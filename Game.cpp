#include "Game.h"

#include <cstdlib>
#include <iostream>
#include <cmath>

// Helper functions

int randRange(int min, int max)
{
    return (rand() % (1 + max - min)) + min;
}

// Public Class Methods

Game::Game(const std::string & config)
{
    init(config);
}


void Game::run()
{
    while (m_running)
    {
        m_entities.update();
        // std::cout << "Entities Count: " << m_entities.getEntities().size() << " Bullets Count: " << m_entities.getEntities("bullet").size() << " Enemies Count: " << m_entities.getEntities("enemy").size() << std::endl;

        if (!m_paused)
        {
            // ?
        }

        sEnemySpawner();
        sMovement();
        sCollision();
        sUserInput();
        sRender();
        sLifespan();

        m_currentFrame++;
    }
}


// Private Class Methods

void Game::init(const std::string & path)
{
    // TODO: read in config file here
    //      store values in structs
    // std::ifstream fin(path);

    m_window.create(sf::VideoMode(1280, 720), "GeoWars");
    m_window.setFramerateLimit(60);
    m_window.setKeyRepeatEnabled(false);

    spawnPlayer();
}


void Game::sCollision()
{
    // bullet-enemy collision
    for (auto b : m_entities.getEntities("bullet"))
    {
        for (auto e : m_entities.getEntities("enemy"))
        {
            const bool isCollision = b->cTransform->pos.distSqr(e->cTransform->pos) < (b->cCollision->radius + e->cCollision->radius) * (b->cCollision->radius + e->cCollision->radius);
            if (isCollision)
            {
                b->destroy();
                e->destroy();
                break;
            }
        }
    }

    // player-enemy collision
    for (auto e : m_entities.getEntities("enemy"))
    {
        const bool isCollision = m_player->cTransform->pos.distSqr(e->cTransform->pos) < (m_player->cCollision->radius + e->cCollision->radius) * (m_player->cCollision->radius + e->cCollision->radius);
        if (isCollision)
        {
            float mx = m_window.getSize().x / 2.0f;
            float my = m_window.getSize().y / 2.0f;

            m_player->cTransform->pos.x = mx;
            m_player->cTransform->pos.y = my;
            break;
        }
    }
}


void Game::sMovement()
{
    // TODO: imp. all entity movement in this function

    m_player->cTransform->velocity = {0,0}; // reset player vel. before every frame to zero
    
    CInput playerInput;

    // cancel directions that are canceling each other out
    playerInput.up = (m_player->cInput->up && !m_player->cInput->down);
    playerInput.down = (m_player->cInput->down && !m_player->cInput->up);
    playerInput.left = (m_player->cInput->left && !m_player->cInput->right);
    playerInput.right = (m_player->cInput->right && !m_player->cInput->left);

    // prevent player from going out of bounds
    if (m_player->cTransform->pos.x - m_player->cShape->circle.getRadius() <= 0)
    {
        playerInput.left = false;
    }
    if (m_player->cTransform->pos.x + m_player->cShape->circle.getRadius() >= m_window.getSize().x)
    {
        playerInput.right = false;
    }
    if (m_player->cTransform->pos.y - m_player->cShape->circle.getRadius() <= 0)
    {
        playerInput.up = false;
    }
    if (m_player->cTransform->pos.y + m_player->cShape->circle.getRadius() >= m_window.getSize().y)
    {
        playerInput.down = false;
    }

    const int movementInputCount = playerInput.up + playerInput.down + playerInput.left + playerInput.right;

    // player moving diagonally
    if (movementInputCount == 2) 
    {
        const float componentSpeed = std::sqrt(m_playerConfig.S * 2);
        if (playerInput.up) {
            m_player->cTransform->velocity.y -= componentSpeed;
        }
        if (playerInput.down) {
            m_player->cTransform->velocity.y += componentSpeed;
        }
        if (playerInput.left) {
            m_player->cTransform->velocity.x -= componentSpeed;
        }
        if (playerInput.right) {
            m_player->cTransform->velocity.x += componentSpeed;
        }
    }
    // player moving horizontally or vertically
    else if (movementInputCount == 1) 
    {
        if (playerInput.up) {
            m_player->cTransform->velocity.y -= m_playerConfig.S;
        } else if (playerInput.down) {
            m_player->cTransform->velocity.y += m_playerConfig.S;
        } else if (playerInput.left) {
            m_player->cTransform->velocity.x -= m_playerConfig.S;
        } else if (playerInput.right) {
            m_player->cTransform->velocity.x += m_playerConfig.S;
        }
    }

    // move the player
    m_player->cTransform->pos.x += m_player->cTransform->velocity.x; 
    m_player->cTransform->pos.y += m_player->cTransform->velocity.y;

    // enemy movement
    for (auto e : m_entities.getEntities("enemy")) 
    {
        const float r = e->cShape->circle.getRadius();
        Vec2& pos = e->cTransform->pos;
        Vec2& vel = e->cTransform->velocity;

        // check if enemy is touching window walls
        if (pos.x - r <= 0 || pos.x + r >= m_window.getSize().x)
        {
            vel.x *= -1;
        }
        if (pos.y - r <= 0 || pos.y + r >= m_window.getSize().y)
        {
            vel.y *= -1;
        }

        pos.x += vel.x;
        pos.y += vel.y;
    }

    // bullet movement
    for (auto e : m_entities.getEntities("bullet")) 
    {
        Vec2& pos = e->cTransform->pos;
        Vec2& vel = e->cTransform->velocity;

        pos.x += vel.x;
        pos.y += vel.y;
    }
}

void Game::sUserInput()
{
    // TODO: handle user input here
    //          no movement logic
    //          only set players input component data

    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_running = false;
        }

        // Key pressed
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:
                    m_player->cInput->up = true;
                    break;
                case sf::Keyboard::A:
                    m_player->cInput->left = true;
                    break;
                case sf::Keyboard::S:
                    m_player->cInput->down = true;
                    break;
                case sf::Keyboard::D:
                    m_player->cInput->right = true;
                    break;
                default: break;
            }
        }

        // Key released
        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:
                    m_player->cInput->up = false;
                    break;
                case sf::Keyboard::A:
                    m_player->cInput->left = false;
                    break;
                case sf::Keyboard::S:
                    m_player->cInput->down = false;
                    break;
                case sf::Keyboard::D:
                    m_player->cInput->right = false;
                    break;
                default: break;
            }
        }

        if (event.type == sf::Event::MouseButtonPressed) 
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                // TODO: add cool down
                spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
            }
            if (event.mouseButton.button == sf::Mouse::Right)
            {
                // special weapon
            }
        }
    }
}

void Game::sLifespan()
{
    for (auto b : m_entities.getEntities("bullet"))
    {
        if (b->cLifespan->remaining > 0) {
            b->cLifespan->remaining--;
        } else {
            b->destroy();
        }
    }
}

void Game::sRender()
{
    m_window.clear();

    for (auto e : m_entities.getEntities()) {
        e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

        e->cTransform->angle += 1.0f;
        e->cShape->circle.setRotation(e->cTransform->angle);

        if (e->tag() == "bullet")
        {
            const float lifespanPercent = ((float) e->cLifespan->remaining / (float) e->cLifespan->total);
            const int alpha = 255 * lifespanPercent;

            e->cShape->circle.setFillColor(sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB, alpha));
            e->cShape->circle.setOutlineColor(sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB, alpha));
        }

        m_window.draw(e->cShape->circle);
    }

    m_window.display();
}

void Game::sEnemySpawner()
{
    // TODO: code which imp. enemy spawning should go here
    //         use (m_currentFrame - m_lastEnemySpawnTime) to determine
    //         how long its has been since the last enemy spawned

    if (m_currentFrame - m_lastEnemySpawnTime >= m_enemyConfig.SI) {
        spawnEnemy();
        m_lastEnemySpawnTime = m_currentFrame;
    };
}

void Game::spawnPlayer()
{
    // TODO: Finish adding all properties of the player with the correct values from the config

    auto entity = m_entities.addEntity("player");

    float mx = m_window.getSize().x / 2.0f;
    float my = m_window.getSize().y / 2.0f;

    entity->cTransform = std::make_shared<CTransform>(Vec2(mx,my), Vec2(3.0f,3.0f), 0.0f);

    entity->cShape = std::make_shared<CShape>(32.0f, 8, sf::Color(10,10,10), sf::Color(255,0,0), 4.0f);

    entity->cInput = std::make_shared<CInput>();

    entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

    m_player = entity;
}

void Game::spawnSmallEnemies(std:: shared_ptr<Entity> entity)
{
    // - small enemies are worth double points of the original enemy

    const int n = entity->cShape->circle.getPointCount();
    const float speed = entity->cTransform->velocity.length();
    for (int i = 0; i < n; i++) 
    {
        const float angle = 360/n * (i) + entity->cTransform->angle;
        Vec2 vel;
        vel.polar(angle, speed);

        auto se = m_entities.addEntity("small enemy");

        se->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, vel, 0);

        se->cCollision = std::make_shared<CCollision>(entity->cCollision->radius/2);

        se->cShape = std::make_shared<CShape>(entity->cShape->circle.getRadius()/2, entity->cShape->circle.getPointCount(), entity->cShape->circle.getFillColor(), entity->cShape->circle.getOutlineColor(), entity->cShape->circle.getOutlineThickness()/2);

        se->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
    }
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2 & mousePos)
{
    auto bullet = m_entities.addEntity("bullet");

    Vec2 vel = mousePos - entity->cTransform->pos;
    vel.normalize();
    vel *= m_bulletConfig.S;

    bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, vel, 0);

    bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);

    bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB, 255), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB, 255), m_bulletConfig.OT);

    bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
}

void Game::spawnEnemy()
{
    // TODO: make sure the enemy is spawned properly with the m_enemyConfig variables
    //      enemy must be spawned completely within the bounds of the window

    // auto e = m_entities.addEntity("enemy");
    // e->cTransform = std::make_shared<CTransform>(Vec2(100.0, 100.0), Vec2(0.2,0.2), 0.0);
    // e->cShape = std::make_shared<CShape>(20.f, 3, sf::Color::Magenta, sf::Color::Red, 5.f);
    auto entity = m_entities.addEntity("enemy");

    
    const float ex = randRange(m_enemyConfig.SR, m_window.getSize().x - m_enemyConfig.SR);
    const float ey = randRange(m_enemyConfig.SR, m_window.getSize().y - m_enemyConfig.SR);
    const float componentSpeed = std::sqrt(randRange(m_enemyConfig.SMIN, m_enemyConfig.SMAX) * 2);
    const int velXSign = std::rand() % 2 == 0 ? 1 : -1;
    const int velYSign = std::rand() % 2 == 0 ? 1 : -1;
    const int shapePoints = randRange(m_enemyConfig.VMIN, m_enemyConfig.VMAX);

    entity->cTransform = std::make_shared<CTransform>(Vec2(ex,ey), Vec2(componentSpeed * velXSign, componentSpeed * velYSign), 0.0f);

    entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, shapePoints, sf::Color(randRange(0,255),randRange(0,255),randRange(0,255)), sf::Color(m_enemyConfig.OR,m_enemyConfig.OG,m_enemyConfig.OB), m_enemyConfig.OT);

    entity->cInput = std::make_shared<CInput>();

    entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::setPaused(bool paused)
{
    // todo
}