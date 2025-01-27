#include "Game.h"

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <sstream>


/**
 * Returns a random number within the given range.
 * 
 * The range is from min (included) to max (included).
 */
int randFromRange(int min, int max)
{
    return (rand() % (1 + max - min)) + min;
}

/**
 * Checks if two circles are overlapping.
 * 
 * Use this function instead of overlap() if all you need to known is if two circles overlap
 * because it is more efficient at doing this calculation.
 * 
 * pos1 - center of circle 1
 * pos2 - center of circle 2
 * r1   - radius of circle 1
 * r2   - radius of circle 2
 */
bool isOverlap(Vec2 pos1, Vec2 pos2, float r1, float r2)
{
    // Apparently, calculating the square root of a number is very expensive. So instead we just compare distances squared.
    return pos1.distSqr(pos2) < (r1 + r2) * (r1 + r2);
}

/**
 * Returns the overlap or distance between the two circles.
 * 
 * If the overlap is greater than 0, then the returned value is the overlap of the two circles.
 * 
 * If the overlap is less then or equal to 0, then it means the two circles are not overlapping,
 * and the absolute value of this overlap is the distance between the two circles. To be specific,
 * this distance is the distance between the two centers of the circles minus both their radiuses.
 * 
 * pos1 - center of circle 1
 * pos2 - center of circle 2
 * r1   - radius of circle 1
 * r2   - radius of circle 2
 */
float overlap(Vec2 pos1, Vec2 pos2, float r1, float r2)
{
    return (r1 + r2) - pos1.dist(pos2);
}

/**
 * Creates instance of Game and initializes it.
 */
Game::Game()
{
    init();
}

/**
 * Runs the main game loop.
 */
void Game::run()
{
    // Spawn 1 enemy for start menu scene so that it bounces and moves around in the background
    spawnEnemy();

    // While game is running
    while (m_running)
    {
        m_entities.update();

        // Start menu scene
        if (m_startMenu)
        {
            sRender();
            sUserInput();
            sMovement();
        }
        else if (m_endGameMenu)
        {
            sUserInput();
            sMovement();
            sCollision();
            sLifespan();
            sRender();
        }
        // Pause scene
        else if (m_paused)
        {
            sUserInput();
            sRender();
        }
        // In-game scene
        else
        {

            sEnemySpawner();
            sMovement();
            sCollision();
            sLifespan(); // must be last system call (in order for nuke to work) [What?]
            sUserInput();
            sRender();

            m_currentFrame++;
        }
    }
    
    // Close window
    m_window.close();
}

/**
 * Initializes the window, loads text font, and spawns the player.
 */
void Game::init()
{
    const int windowWidth = 1280;
    const int windowHeight = 720;
    const int frameLimit = 60;

    // Initialize the window
    m_window.create(sf::VideoMode(windowWidth, windowHeight), "GeoWars");
    m_window.setFramerateLimit(frameLimit);
    m_window.setKeyRepeatEnabled(false);

    // Load text font
    if (!m_font.loadFromFile("/home/rose/Projects/geometry-wars/sofachromergit.otf")) {
        std::cout << "Error with loading font.\n";
    }

    spawnPlayer();
}

/**
 * System for collisions.
 */
void Game::sCollision()
{
    // Bullet-enemy collision
    for (auto b : m_entities.getEntities("bullet"))
    {
        for (auto e : m_entities.getEntities("enemy"))
        {
            if (isOverlap(b->cTransform->pos, e->cTransform->pos, b->cCollision->radius, e->cCollision->radius)) // collision
            {
                // Big enemies spawn smaller enemies
                if (e->cLifespan == nullptr)
                {
                    spawnSmallEnemies(e);
                }

                if (m_player != nullptr)
                {
                    // Player scores points for killing enemy
                    m_player->cScore->score += e->cScore->score;
                }

                b->destroy();
                e->destroy();

                break;
            }
        }
    }

    if (m_player != nullptr)
    {
        // Player-enemy collision
        for (auto e : m_entities.getEntities("enemy"))
        {
            if (isOverlap(m_player->cTransform->pos, e->cTransform->pos, m_player->cCollision->radius, e->cCollision->radius)) // collision
            {
                m_endGameMenu = true;

                if (m_player->cScore->score > m_highScore)
                {
                    m_diffNewHighScorePrevHighScore = m_player->cScore->score - m_highScore;
                    m_highScore = m_player->cScore->score;
                    m_isNewHighScore = true;
                }

                m_gameScore = m_player->cScore->score;
                m_player->destroy();
                m_player = nullptr;

                break;
            }
        }
    }

    // Nuke-Enemy collision
    for (auto n : m_entities.getEntities("nuke"))
    {
        // Nuke only works during its first frame (not the best way to do this, but it works)
        if (n->cLifespan->remaining == n->cLifespan->total)
        {
            for (auto e : m_entities.getEntities("enemy"))
            {
                // Enemy is in explosion if its center is inside the explosion radius
                bool isInExplosion = isOverlap(e->cTransform->pos, n->cTransform->pos, m_nukeConfig.ER, 0);
                // Enemy is in blast (shockwave) if its center is inside the blast (shockwave) radius
                bool isInBlast = isOverlap(e->cTransform->pos, n->cTransform->pos, m_nukeConfig.BR, 0);

                if (isInExplosion || (isInBlast && e->cLifespan != nullptr))
                {
                    // Any enemy in the explosion radius dies
                    // Enemies with 'lifespan' die if they are in blast or explosion radius

                    if (m_player != nullptr)
                    {
                        m_player->cScore->score += e->cScore->score;
                    }

                    e->destroy();
                }
                else if (isInBlast)
                {
                    // A enemy in blast radius is given a 'lifespan' (i.e they will die after a certain amount of time passes),
                    // their speed is multiplied by the blast speed multiplier (BVM) (i.e their given a speed boost),
                    // and they are given a higher score value (i.e player gets more for killing these types of enemies)

                    float newSpeed = e->cTransform->velocity.length() * m_nukeConfig.BVM;
                    Vec2 newVelocity = e->cTransform->pos - n->cTransform->pos;
                    newVelocity.normalize();
                    newVelocity *= newSpeed;

                    e->cLifespan = std::make_shared<CLifespan>(m_nukeConfig.REL);
                    e->cTransform->velocity = newVelocity;
                    e->cScore->score = m_enemyConfig.SSE;
                    e->cTransform->angularVel *= -5;
                }
            }
        }
    }

    // Enemy-enemy collision
    for (auto e1 : m_entities.getEntities("enemy"))
    {
        // Enemies with lifespans can not collide with other enemies (they are ghosts)
        if (e1->cLifespan != nullptr)
        {
            continue;
        }

        for (auto e2 : m_entities.getEntities("enemy"))
        {
            // Enemies with lifespans can not collide with other enemies (they are ghosts)
            if (e1->id() == e2->id() || e2->cLifespan != nullptr)
            {
                continue;
            }

            if (isOverlap(e1->cTransform->pos, e2->cTransform->pos, e1->cCollision->radius, e2->cCollision->radius)) // collision
            {
                // Enemies that collide change direction and go in exact opposite directions of each other, but same speed as each started with

                Vec2 newDirectionForE1 = e1->cTransform->pos - e2->cTransform->pos;
                newDirectionForE1.normalize();

                e1->cTransform->velocity = newDirectionForE1 * e1->cTransform->velocity.length();
                e2->cTransform->velocity = newDirectionForE1 * (e2->cTransform->velocity.length() * -1);

                // Separate the two so that their is no overlap anymore
                float halfOverlap = overlap(e1->cTransform->pos, e2->cTransform->pos, e1->cCollision->radius, e2->cCollision->radius)/2; 
                e1->cTransform->pos += e1->cTransform->velocity * (halfOverlap/e1->cTransform->velocity.length());
                e2->cTransform->pos += e2->cTransform->velocity * (halfOverlap/e2->cTransform->velocity.length());
            }
        }
    }       
}

/**
 * System for movement.
 */
void Game::sMovement()
{
    // Player movement
    if (m_player != nullptr)
    {
        std::shared_ptr<CInput> playerCI = m_player->cInput;
        std::shared_ptr<CTransform> playerCT = m_player->cTransform;
        const float radius = m_player->cShape->circle.getRadius();
        CInput actualMovementInput;

        playerCT->velocity = {0,0}; // zero out player velocity

        // Things to take into account when determining actual movement input:
        // 1) Directions that are opposite of each other cancel each other (like up and down)
        // 2) Player can not move out of bounds, so cancel movement that would move player out of bounds
        actualMovementInput.up = (playerCI->up && !playerCI->down) && (playerCT->pos.y - radius >= 0);
        actualMovementInput.down = (playerCI->down && !playerCI->up) && (playerCT->pos.y + radius <= m_window.getSize().y);
        actualMovementInput.left = (playerCI->left && !playerCI->right) && (playerCT->pos.x - radius >= 0);
        actualMovementInput.right = (playerCI->right && !playerCI->left) && (playerCT->pos.x + radius <= m_window.getSize().x);

        if ((actualMovementInput.up || actualMovementInput.down) && (actualMovementInput.left || actualMovementInput.right)) // moving diagonally
        {
            // Moving diagonally should have same speed as moving horizontally or vertically

            const float componentSpeed = std::sqrt(m_playerConfig.S * 2);

            if (actualMovementInput.up) // up
            {
                playerCT->velocity.y -= componentSpeed;
            }
            else // down
            {
                playerCT->velocity.y += componentSpeed;
            }

            if (actualMovementInput.left) // left
            {
                playerCT->velocity.x -= componentSpeed;
            }
            else // right
            {
                playerCT->velocity.x += componentSpeed;
            }
        }
        else // moving horizontally or vertically
        {
            if (actualMovementInput.up) // moving up
            {
                playerCT->velocity.y -= m_playerConfig.S;
            }
            else if (actualMovementInput.down) // moving down
            {
                playerCT->velocity.y += m_playerConfig.S;
            }
            else if (actualMovementInput.left) // moving left
            {
                playerCT->velocity.x -= m_playerConfig.S;
            }
            else if (actualMovementInput.right) // moving right
            {
                playerCT->velocity.x += m_playerConfig.S;
            }
        }

        // Move the player
        playerCT->pos += playerCT->velocity;
    }

    // Enemy movement
    for (auto e : m_entities.getEntities("enemy")) 
    {
        // Enemies travel in straight directions, and bounce of the walls and other enemies

        const float radius = e->cShape->circle.getRadius();
        Vec2& pos = e->cTransform->pos;
        Vec2& vel = e->cTransform->velocity;

        // Enemies bounce off the walls (they shouldn't go outside window)
        if (pos.x - radius <= 0 || pos.x + radius >= m_window.getSize().x)
        {
            vel.x *= -1;
            e->cTransform->angularVel *= -1;
        }
        if (pos.y - radius <= 0 || pos.y + radius >= m_window.getSize().y)
        {
            vel.y *= -1;
            e->cTransform->angularVel *= -1;
        }

        // Move the enemy
        pos += vel;
    }

    // Bullet movement
    for (auto b : m_entities.getEntities("bullet")) 
    {
        // Bullets travel in straight directions (they don't bounce of walls. they can go outside the window)

        // Move the bullet
        b->cTransform->pos += b->cTransform->velocity;
    }
}

/**
 * System for user input.
 */
void Game::sUserInput()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_running = false;
            break;
        }

        if (m_startMenu) // Start menu
        {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
            {
                m_startMenu = false;
            }
        }
        else if (m_endGameMenu) // Game over menu
        {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
            {
                m_endGameMenu = false;

                spawnPlayer();

                for (auto e : m_entities.getEntities("enemy"))
                {
                    e->destroy();
                }

                m_entities.update();
            }
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace)
            {
                m_endGameMenu = false;
                m_startMenu = true;
                m_paused = false;

                m_highScore = 0;
                m_diffNewHighScorePrevHighScore = 0;
                m_isNewHighScore = false;
                m_currentFrame = 0;
                m_lastEnemySpawnTime = 0;
                m_lastNukeTime = 0;

                for (auto e : m_entities.getEntities("enemy"))
                {
                    e->destroy();
                }

                spawnEnemy();
                spawnPlayer();
            }
        }
        else if (m_paused) // Paused
        {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
            {
                m_paused = false;
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::W)
                {
                    m_player->cInput->up = true;
                }
                else if (event.key.code == sf::Keyboard::A)
                {
                    m_player->cInput->left = true;
                }
                else if (event.key.code == sf::Keyboard::S)
                {
                    m_player->cInput->down = true;
                }
                else if (event.key.code == sf::Keyboard::D)
                {
                    m_player->cInput->right = true;
                }
            }
            else if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::W)
                {
                    m_player->cInput->up = false;
                }
                else if (event.key.code == sf::Keyboard::A)
                {
                    m_player->cInput->left = false;
                }
                else if (event.key.code == sf::Keyboard::S)
                {
                    m_player->cInput->down = false;
                }
                else if (event.key.code == sf::Keyboard::D)
                {
                    m_player->cInput->right = false;
                }
            }
        }
        else // In game 
        {
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::P)
                {
                    m_paused = true;
                }
                else if (event.key.code == sf::Keyboard::W)
                {
                    m_player->cInput->up = true;
                }
                else if (event.key.code == sf::Keyboard::A)
                {
                    m_player->cInput->left = true;
                }
                else if (event.key.code == sf::Keyboard::S)
                {
                    m_player->cInput->down = true;
                }
                else if (event.key.code == sf::Keyboard::D)
                {
                    m_player->cInput->right = true;
                }
            }
            else if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::W)
                {
                    m_player->cInput->up = false;
                }
                else if (event.key.code == sf::Keyboard::A)
                {
                    m_player->cInput->left = false;
                }
                else if (event.key.code == sf::Keyboard::S)
                {
                    m_player->cInput->down = false;
                }
                else if (event.key.code == sf::Keyboard::D)
                {
                    m_player->cInput->right = false;
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed) 
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
                }
                if (event.mouseButton.button == sf::Mouse::Right)
                {
                    if (m_currentFrame - m_lastNukeTime >= m_nukeConfig.CDI) {
                        spawnSpecialWeapon(m_player);
                        m_lastNukeTime = m_currentFrame;
                    }
                }
            }
        }
    }
}

/**
 * System for lifespan.
 */
void Game::sLifespan()
{
    // Entities with lifespans will die once their lifespan is over.

    for (auto e : m_entities.getEntities())
    {
        if (e->cLifespan != nullptr)
        {
            if (e->cLifespan->remaining > 0) 
            {
                e->cLifespan->remaining--;
            } else 
            {
                e->destroy();
            }
        }
    }
}

/**
 * System for rendering.
 */
void Game::sRender()
{
    m_window.clear(); // clear the window
    const sf::Vector2u WINDOW_SIZE = m_window.getSize();

    // 3 Scenes: start menu, in-game, and game-over
    // Only one scene will be rendered

    // Render start menu scene
    if (m_startMenu)
    {
        for (auto e : m_entities.getEntities("enemy")) // Draw enemies
        {
            std::shared_ptr<CTransform> transform = e->cTransform;
            std::shared_ptr<CShape> shape = e->cShape;
            std::shared_ptr<CLifespan> lifespan = e->cLifespan;

            transform->angle += transform->angularVel;
            shape->circle.setPosition(transform->pos.x, transform->pos.y);
            shape->circle.setRotation(transform->angle);

            if (lifespan != nullptr)
            {
                // Enemies with lifespans fade as their lifespan shrinks

                const int MAX_ALPHA = 255;
                sf::Color updatedFill = shape->circle.getFillColor();
                sf::Color updatedOutline = shape->circle.getOutlineColor();

                // MAX_ALPHA * (<lifespan percentage>)
                const int alpha = MAX_ALPHA * ((float) lifespan->remaining / (float) lifespan->total);

                updatedFill.a = alpha;
                updatedOutline.a = alpha;

                shape->circle.setFillColor(updatedFill);
                shape->circle.setOutlineColor(updatedOutline);
            }

            m_window.draw(shape->circle);
        }

        // Semi-transparent background (overlayed over enemies in background)
        const sf::Color overlayBackground(50, 50, 50, 120);
        sf::RectangleShape overlay;
        overlay.setFillColor(overlayBackground);
        overlay.setSize(sf::Vector2f(WINDOW_SIZE.x, WINDOW_SIZE.y));
        m_window.draw(overlay);

        // Main title (game name)
        sf::Text title;
        title.setFont(m_font);
        title.setString("Geometry Wars");
        title.setCharacterSize(50);
        title.setColor(sf::Color::Cyan);
        title.setOrigin(sf::Vector2f(title.getLocalBounds().left, title.getLocalBounds().top));
        title.setPosition(sf::Vector2f(WINDOW_SIZE.x/2 - title.getLocalBounds().width/2, WINDOW_SIZE.y/2 - title.getLocalBounds().height/2));
        m_window.draw(title);

        // Line under main title
        const sf::Vector2f lineSize(WINDOW_SIZE.x, 1);
        sf::RectangleShape line(lineSize);
        line.setFillColor(sf::Color::Cyan);
        line.setOrigin(sf::Vector2f(lineSize.x/2, lineSize.y/2));
        line.setPosition(sf::Vector2f(WINDOW_SIZE.x/2, title.getPosition().y + title.getLocalBounds().height + lineSize.y/2 + 8));
        m_window.draw(line);

        // How to start game instruction
        float margin = 30;
        sf::Text enterGame;
        enterGame.setFont(m_font);
        enterGame.setString("press enter to play");
        enterGame.setCharacterSize(16);
        // Make it blink
        enterGame.setColor(sf::Color(255, 255, 255, 255*m_startMenuInstructionAlphaPercent));
        m_startMenuInstructionAlphaPercent -= 0.01;
        if (m_startMenuInstructionAlphaPercent < 0)
        {
            m_startMenuInstructionAlphaPercent = 1;
        }
        enterGame.setOrigin(sf::Vector2f(enterGame.getLocalBounds().left, enterGame.getLocalBounds().top));
        enterGame.setPosition(sf::Vector2f(WINDOW_SIZE.x/2 - enterGame.getLocalBounds().width/2, title.getPosition().y + title.getLocalBounds().height + margin));
        m_window.draw(enterGame);


        // Key mappings guide
        sf::Text up;
        sf::Text left;
        sf::Text down;
        sf::Text right;
        sf::Text mainWeapon;
        sf::Text specialWeapon;
        sf::Text pause;

        up.setFont(m_font);
        left.setFont(m_font);
        down.setFont(m_font);
        right.setFont(m_font);
        mainWeapon.setFont(m_font);
        specialWeapon.setFont(m_font);
        pause.setFont(m_font);

        up.setString("W - up");
        left.setString("A - left");
        down.setString("S - down");
        right.setString("D - right");
        mainWeapon.setString("LEFT CLICK - main weapon");
        specialWeapon.setString("RIGHT CLICK - special weapon");
        pause.setString("P - pause/unpause");

        up.setCharacterSize(12);
        left.setCharacterSize(12);
        down.setCharacterSize(12);
        right.setCharacterSize(12);
        mainWeapon.setCharacterSize(12);
        specialWeapon.setCharacterSize(12);
        pause.setCharacterSize(12);

        up.setColor(sf::Color::White);
        left.setColor(sf::Color::White);
        down.setColor(sf::Color::White);
        right.setColor(sf::Color::White);
        mainWeapon.setColor(sf::Color::White);
        specialWeapon.setColor(sf::Color::White);
        pause.setColor(sf::Color::White);

        up.setOrigin(sf::Vector2f(up.getLocalBounds().left, up.getLocalBounds().top));
        left.setOrigin(sf::Vector2f(left.getLocalBounds().left, left.getLocalBounds().top));
        down.setOrigin(sf::Vector2f(down.getLocalBounds().left, down.getLocalBounds().top));
        right.setOrigin(sf::Vector2f(right.getLocalBounds().left, right.getLocalBounds().top));
        mainWeapon.setOrigin(sf::Vector2f(mainWeapon.getLocalBounds().left, mainWeapon.getLocalBounds().top));
        specialWeapon.setOrigin(sf::Vector2f(specialWeapon.getLocalBounds().left, specialWeapon.getLocalBounds().top));
        pause.setOrigin(sf::Vector2f(pause.getLocalBounds().left, pause.getLocalBounds().top));

        up.setPosition(sf::Vector2f(10,10));
        left.setPosition(sf::Vector2f(10, up.getPosition().y + up.getLocalBounds().height + 10));
        down.setPosition(sf::Vector2f(10, left.getPosition().y + left.getLocalBounds().height + 10));
        right.setPosition(sf::Vector2f(10, down.getPosition().y + down.getLocalBounds().height + 10));
        mainWeapon.setPosition(sf::Vector2f(10, right.getPosition().y + right.getLocalBounds().height + 10));
        specialWeapon.setPosition(sf::Vector2f(10, mainWeapon.getPosition().y + mainWeapon.getLocalBounds().height + 10));
        pause.setPosition(sf::Vector2f(10, specialWeapon.getPosition().y + specialWeapon.getLocalBounds().height + 10));

        m_window.draw(up);
        m_window.draw(left);
        m_window.draw(down);
        m_window.draw(right);
        m_window.draw(mainWeapon);
        m_window.draw(specialWeapon);
        m_window.draw(pause);
    }
    else if (m_endGameMenu) // End game (game over) scene
    {
        for (auto e : m_entities.getEntities("enemy")) // Draw enemies
        {
            std::shared_ptr<CTransform> transform = e->cTransform;
            std::shared_ptr<CShape> shape = e->cShape;
            std::shared_ptr<CLifespan> lifespan = e->cLifespan;

            transform->angle += transform->angularVel;
            shape->circle.setPosition(transform->pos.x, transform->pos.y);
            shape->circle.setRotation(transform->angle);

            if (lifespan != nullptr)
            {
                // Enemies with lifespans fade as their lifespan shrinks

                const int MAX_ALPHA = 255;
                sf::Color updatedFill = shape->circle.getFillColor();
                sf::Color updatedOutline = shape->circle.getOutlineColor();

                // MAX_ALPHA * (<lifespan percentage>)
                const int alpha = MAX_ALPHA * ((float) lifespan->remaining / (float) lifespan->total);

                updatedFill.a = alpha;
                updatedOutline.a = alpha;

                shape->circle.setFillColor(updatedFill);
                shape->circle.setOutlineColor(updatedOutline);
            }

            m_window.draw(shape->circle);
        }

        // Semi-transparent background (overlayed over enemies in background)
        const sf::Color overlayBackground(50, 50, 50, 120);
        sf::RectangleShape overlay;
        overlay.setFillColor(overlayBackground);
        overlay.setSize(sf::Vector2f(WINDOW_SIZE.x, WINDOW_SIZE.y));
        m_window.draw(overlay);

        // Show player previous highest score
        std::ostringstream scoreTrackSS;
        if (!m_isNewHighScore)
        {
            // They did not beat it this game
            scoreTrackSS << "High score:" << m_highScore;
        }
        else
        {
            // They beat it, so its now the previous highest score
            scoreTrackSS << "Previous High Score:  " << m_highScore - m_diffNewHighScorePrevHighScore << "  (+" << m_diffNewHighScorePrevHighScore << ")"; 
        }
        sf::Text scoreTracker;
        scoreTracker.setColor(sf::Color::White);
        scoreTracker.setString(scoreTrackSS.str());
        scoreTracker.setFont(m_font);
        scoreTracker.setCharacterSize(12);
        scoreTracker.setOrigin(sf::Vector2f(scoreTracker.getLocalBounds().left, scoreTracker.getLocalBounds().top));
        scoreTracker.setPosition(sf::Vector2f(10,10));
        m_window.draw(scoreTracker);

        // How to return to start menu instruction
        sf::Text returnToStartMenu;
        returnToStartMenu.setColor(sf::Color::White);
        returnToStartMenu.setString("Press backspace to go to start menu");
        returnToStartMenu.setFont(m_font);
        returnToStartMenu.setCharacterSize(12);
        returnToStartMenu.setOrigin(sf::Vector2f(returnToStartMenu.getLocalBounds().left, returnToStartMenu.getLocalBounds().top));
        returnToStartMenu.setPosition(sf::Vector2f(WINDOW_SIZE.x - returnToStartMenu.getLocalBounds().width - 10, 10));
        m_window.draw(returnToStartMenu);

        // Show the player the score they got this game
        std::ostringstream gameScoreSS;
        sf::Text gameScore;
        if (m_isNewHighScore)
        {
            // They got new high score, so make it blink

            sf::Color cyan(sf::Color::Cyan);
            cyan.a = 255 * m_startMenuInstructionAlphaPercent;
            m_startMenuInstructionAlphaPercent -= 0.02;
            if (m_startMenuInstructionAlphaPercent < 0)
            {
                m_startMenuInstructionAlphaPercent = 1;
            }
            gameScoreSS << "High Score: " << m_gameScore;
            gameScore.setColor(cyan);
        }
        else
        {
            // They did not get a new high score

            gameScoreSS << "Score: " << m_gameScore;
            gameScore.setColor(sf::Color::Cyan);
        }
        gameScore.setString(gameScoreSS.str());
        gameScore.setFont(m_font);
        gameScore.setCharacterSize(50);
        gameScore.setOrigin(sf::Vector2f(gameScore.getLocalBounds().left, gameScore.getLocalBounds().top));
        gameScore.setPosition(sf::Vector2f(WINDOW_SIZE.x/2 - gameScore.getLocalBounds().width/2, WINDOW_SIZE.y/2 - gameScore.getLocalBounds().height/2));
        m_window.draw(gameScore);

        // Line that is drawn under score player got this game
        const sf::Vector2f lineSize(WINDOW_SIZE.x, 1);
        sf::RectangleShape line(lineSize);
        line.setFillColor(sf::Color::Cyan);
        line.setOrigin(sf::Vector2f(lineSize.x/2, lineSize.y/2));
        line.setPosition(sf::Vector2f(WINDOW_SIZE.x/2, gameScore.getPosition().y + gameScore.getLocalBounds().height + lineSize.y/2 + 8));
        m_window.draw(line);

        // Instructions for how to play again
        float margin = 30;
        sf::Text enterGame;
        enterGame.setFont(m_font);
        enterGame.setString("press enter to play again");
        enterGame.setCharacterSize(16);
        if (m_isNewHighScore)
        {
            enterGame.setColor(sf::Color(255, 255, 255, 255));
        }
        else 
        {
            // Blinks if player did not get a high score
        
            enterGame.setColor(sf::Color(255, 255, 255, 255*m_startMenuInstructionAlphaPercent));
            m_startMenuInstructionAlphaPercent -= 0.01;
            if (m_startMenuInstructionAlphaPercent < 0)
            {
                m_startMenuInstructionAlphaPercent = 1;
            }
        }
        enterGame.setOrigin(sf::Vector2f(enterGame.getLocalBounds().left, enterGame.getLocalBounds().top));
        enterGame.setPosition(sf::Vector2f(WINDOW_SIZE.x/2 - enterGame.getLocalBounds().width/2, gameScore.getPosition().y + gameScore.getLocalBounds().height + margin));
        m_window.draw(enterGame);
    }
    else // in game
    {
        // Draw all entities (player, enemies, bullets, and nukes)
        for (auto e : m_entities.getEntities())
        {
            std::shared_ptr<CTransform> transform = e->cTransform;
            std::shared_ptr<CShape> shape = e->cShape;
            std::shared_ptr<CLifespan> lifespan = e->cLifespan;

            transform->angle += transform->angularVel;
            shape->circle.setPosition(transform->pos.x, transform->pos.y);
            shape->circle.setRotation(transform->angle);

            if (lifespan != nullptr)
            {
                // Entities (enemies, bullets, and nukes) with lifespans fade as their lifespan shrinks

                const int MAX_ALPHA = 255;
                sf::Color updatedFill = shape->circle.getFillColor();
                sf::Color updatedOutline = shape->circle.getOutlineColor();

                // Prevents enemies from fading too much and becoming invisible yet still alive
                const int alpha = MAX_ALPHA * ((float) lifespan->remaining / (float) lifespan->total) > 80 ? MAX_ALPHA * ((float) lifespan->remaining / (float) lifespan->total) : 80;

                updatedFill.a = alpha;
                updatedOutline.a = alpha;

                shape->circle.setFillColor(updatedFill);
                shape->circle.setOutlineColor(updatedOutline);
            }

            m_window.draw(shape->circle);
        }

        // Show the player's current score
        std::ostringstream currentScoreSS;
        currentScoreSS << "Score: " << m_player->cScore->score;
        sf::Text score;
        score.setFont(m_font);
        score.setString(currentScoreSS.str());
        score.setCharacterSize(30);
        score.setColor(sf::Color::Cyan);
        m_window.draw(score);

        // The special weapon (nuke) cool down indicator (faded when its not available, its a miniature version of actual nuke)
        sf::CircleShape nukeCoolDownIndicator;
        int alpha = m_currentFrame - m_lastNukeTime >= m_nukeConfig.CDI ? 255 : 255 * 0.40;
        sf::Color fill = sf::Color(m_nukeConfig.FR, m_nukeConfig.FG, m_nukeConfig.FB, alpha);
        sf::Color outline = sf::Color(m_nukeConfig.OR, m_nukeConfig.OG, m_nukeConfig.OB, alpha);
        // Miniature version w/ same proportions
        float explosionRadius = 10;
        float blastRadius = explosionRadius * m_nukeConfig.BR / m_nukeConfig.ER;
        nukeCoolDownIndicator.setFillColor(fill);
        nukeCoolDownIndicator.setOutlineColor(outline);
        nukeCoolDownIndicator.setRadius(explosionRadius);
        nukeCoolDownIndicator.setOutlineThickness(blastRadius - explosionRadius);
        nukeCoolDownIndicator.setPointCount(m_nukeConfig.V);
        nukeCoolDownIndicator.setPosition(15,50);
        m_window.draw(nukeCoolDownIndicator);

        // no spawn zone around player (for debugging)
        // draw a circle with radius of no spawn zone on player
        // const float noSpawnZoneRadius = m_player->cCollision->radius * 3;
        // sf::CircleShape noSpawnZone;
        // noSpawnZone.setFillColor(sf::Color(255, 0, 0, 100));
        // noSpawnZone.setRadius(noSpawnZoneRadius);
        // noSpawnZone.setOrigin(sf::Vector2f(noSpawnZoneRadius, noSpawnZoneRadius));
        // noSpawnZone.setPosition(sf::Vector2f(m_player->cTransform->pos.x, m_player->cTransform->pos.y));
        // m_window.draw(noSpawnZone);

        if (m_paused) // paused (just renders an overlay over the in-game scene)
        {
            // Draw overlay, and pause symbol

            float width = 25.f;
            float height = 5.f * width;
            float margin = 20.f;
            sf::RectangleShape leftBar;
            sf::RectangleShape rightBar;
            sf::CircleShape circleAroundBars;
            sf::RectangleShape overlay;

            leftBar.setFillColor(sf::Color::White);
            rightBar.setFillColor(sf::Color::White);
            circleAroundBars.setFillColor(sf::Color(255, 255, 255, 0));
            overlay.setFillColor(sf::Color(50, 50, 50, 120));
            circleAroundBars.setOutlineColor(sf::Color::White);
            circleAroundBars.setOutlineThickness(10);
            circleAroundBars.setPointCount(10);

            leftBar.setSize(sf::Vector2f(width,height));
            rightBar.setSize(sf::Vector2f(width,height));
            overlay.setSize(sf::Vector2f(WINDOW_SIZE.x, WINDOW_SIZE.y));
            circleAroundBars.setRadius(height - 30);

            leftBar.setPosition(sf::Vector2f(WINDOW_SIZE.x/2 - width - margin, WINDOW_SIZE.y/2 - height/2));
            rightBar.setPosition(sf::Vector2f(WINDOW_SIZE.x/2 + margin, WINDOW_SIZE.y/2 - height/2));
            circleAroundBars.setPosition(sf::Vector2f(WINDOW_SIZE.x/2 - circleAroundBars.getRadius(), WINDOW_SIZE.y/2 - circleAroundBars.getRadius()));

            m_window.draw(overlay);
            m_window.draw(leftBar);
            m_window.draw(rightBar);
            m_window.draw(circleAroundBars);
        }
    }

    m_window.display();
}

/**
 * System for spawning enemies.
 */
void Game::sEnemySpawner()
{
    // 1 enemy is spawned after one 'spawn interval' has passed
    
    if (m_currentFrame - m_lastEnemySpawnTime >= m_enemyConfig.SI) {
        spawnEnemy();
        m_lastEnemySpawnTime = m_currentFrame;
    };
}

/**
 * Spawns the player.
 */
void Game::spawnPlayer()
{
    auto entity = m_entities.addEntity("player");

    entity->cTransform = std::make_shared<CTransform>(Vec2(m_window.getSize().x / 2.0f, m_window.getSize().y / 2.0f), Vec2(3.0f,3.0f), 0.0f);
    entity->cShape = std::make_shared<CShape>(32.0f, 8, sf::Color(10,10,10), sf::Color(255,0,0), 4.0f);
    entity->cInput = std::make_shared<CInput>();
    entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
    entity->cScore = std::make_shared<CScore>(0);

    m_player = entity;
    m_isNewHighScore = false;
}

/**
 * Spawns 'small' enemies.
 */
void Game::spawnSmallEnemies(std:: shared_ptr<Entity> bigEnemy)
{
    // When a big enemy is killed, it will break up into smaller enemies

    // The number of small enemies to spawn is equal to the number of vertices the big enemy has
    const int numberOfSmallEnemies = bigEnemy->cShape->circle.getPointCount();
    const float speed = bigEnemy->cTransform->velocity.length();

    for (int i = 0; i < numberOfSmallEnemies; i++) 
    {
        // Each small enemy goes off in the direction of a vertex (starting from the center of big enemy)

        const float angle = 360/numberOfSmallEnemies * (i) + bigEnemy->cTransform->angle;
        Vec2 vel;
        Vec2 pos;
        vel.polar(angle, speed);

        auto smallEnemy = m_entities.addEntity("enemy");

        // These smaller enemies spawn where the big enemy died, they are worth double
        // the points of the big enemy, and have a lifespan
        smallEnemy->cTransform = std::make_shared<CTransform>(bigEnemy->cTransform->pos, vel, 0);
        smallEnemy->cCollision = std::make_shared<CCollision>(bigEnemy->cCollision->radius/2);
        smallEnemy->cShape = std::make_shared<CShape>(bigEnemy->cShape->circle.getRadius()/2, bigEnemy->cShape->circle.getPointCount(), bigEnemy->cShape->circle.getFillColor(), bigEnemy->cShape->circle.getOutlineColor(), bigEnemy->cShape->circle.getOutlineThickness()/2);
        smallEnemy->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
        smallEnemy->cScore = std::make_shared<CScore>(m_enemyConfig.SSE);
    }
}

/**
 * Spawns a bullet.
 */
void Game::spawnBullet(std::shared_ptr<Entity> player, const Vec2 & mousePos)
{
    // Bullet starts off at center of player
    // It goes towards the direction of the mouse

    auto bullet = m_entities.addEntity("bullet");

    Vec2 vel = mousePos - player->cTransform->pos;
    vel.normalize();
    vel *= m_bulletConfig.S;

    bullet->cTransform = std::make_shared<CTransform>(player->cTransform->pos, vel, 0);
    bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
    bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB, 255), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB, 255), m_bulletConfig.OT);
    bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

/**
 * Spawn special weapon (nuke).
 * 
 * Spawns special weapon on top of player.
 */
void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // Nuke spawns on top of player.

    auto nuke = m_entities.addEntity("nuke");
    sf::Color fill = sf::Color(m_nukeConfig.FR, m_nukeConfig.FG, m_nukeConfig.FB);
    sf::Color outline = sf::Color(m_nukeConfig.OR, m_nukeConfig.OG, m_nukeConfig.OB);

    nuke->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, Vec2(0,0), 0);
    nuke->cShape = std::make_shared<CShape>(m_nukeConfig.ER, m_nukeConfig.V, fill, outline, m_nukeConfig.BR - m_nukeConfig.ER);
    nuke->cLifespan = std::make_shared<CLifespan>(m_nukeConfig.L);
}

/**
 * Spawns a big enemy.
 */
void Game::spawnEnemy()
{
    auto enemy = m_entities.addEntity("enemy");

    // Spawn position
    float x, y;

    if (m_player != nullptr && !m_startMenu)
    {
        // Enemy can't spawn on top or near the player (just reroll a new random spawn point)

        float reroll = true;
        const float noSpawnZoneRadius = m_player->cCollision->radius * 3;
        while (reroll)
        {
            // enemies can't spawn outside or PARTLY outside map, must be fully in
            x = randFromRange(m_enemyConfig.SR, m_window.getSize().x - m_enemyConfig.SR);
            y = randFromRange(m_enemyConfig.SR, m_window.getSize().y - m_enemyConfig.SR);
            
            if (!isOverlap(m_player->cTransform->pos, Vec2(x,y), noSpawnZoneRadius, m_enemyConfig.SR))
            {
                reroll = false;
            }
        }
    }
    else
    {
        // Player is not spawned, so it safe to spawn anywhere

        // enemies can't spawn outside or PARTLY outside map, must be fully in
        x = randFromRange(m_enemyConfig.SR, m_window.getSize().x - m_enemyConfig.SR);
        y = randFromRange(m_enemyConfig.SR, m_window.getSize().y - m_enemyConfig.SR);
    }

    // Random speed, random diagonal direction, and random number of vertices
    const float componentSpeed = std::sqrt(randFromRange(m_enemyConfig.SMIN, m_enemyConfig.SMAX) * 2);
    const int velXSign = std::rand() % 2 == 0 ? 1 : -1;
    const int velYSign = std::rand() % 2 == 0 ? 1 : -1;
    const int shapePoints = randFromRange(m_enemyConfig.VMIN, m_enemyConfig.VMAX);

    enemy->cTransform = std::make_shared<CTransform>(Vec2(x,y), Vec2(componentSpeed * velXSign, componentSpeed * velYSign), 0.0f);
    enemy->cShape = std::make_shared<CShape>(m_enemyConfig.SR, shapePoints, sf::Color(randFromRange(0,255),randFromRange(0,255),randFromRange(0,255)), sf::Color(m_enemyConfig.OR,m_enemyConfig.OG,m_enemyConfig.OB), m_enemyConfig.OT);
    enemy->cInput = std::make_shared<CInput>();
    enemy->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
    enemy->cScore = std::make_shared<CScore>(m_enemyConfig.SNE);

    m_lastEnemySpawnTime = m_currentFrame;
}
