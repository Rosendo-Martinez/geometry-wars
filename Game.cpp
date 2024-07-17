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

        if (!m_paused)
        {
            // ?
        }

        sEnemySpawner();
        sMovement();
        sCollision();
        sUserInput();
        sRender();

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
    // TODO: imp. all proper collisions between entities
    //          be sure to use collision radius, not shape radius

    for (auto b : m_entities.getEntities("bullet"))
    {
        for (auto e : m_entities.getEntities("enemy"))
        {
            // if (Physics::CheckCollision(b, e))
            // {
            //     b->destroy();
            //     e->destroy();
            // }
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
    }
}

void Game::sLifespan()
{
}

void Game::sRender()
{
    m_window.clear();

    for (auto e : m_entities.getEntities()) {
        e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

        e->cTransform->angle += 1.0f;
        e->cShape->circle.setRotation(e->cTransform->angle);

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

    m_player = entity;
}

void spawnSmallEnemies(std:: shared_ptr<Entity> entity)
{
    // TODO: spawn small enemies at the location of the input enemy entity
    // - spawn a number of small enemies equal to the vertices of the original enemy
    // - small enemy same color but half size of enemy entity
    // - small enemies are worth double points of the original enemy
}

void spawnBullet(std::shared_ptr<Entity> entity, const Vec2 & mousePos)
{
    // TODO: imp. spawning of a bullet which travels toward target
    //      - bullet speed is given as a scalar speed
    //      - you must set the velocity by using formula in notes
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

    
    float ex = randRange(m_enemyConfig.SR, m_window.getSize().x - m_enemyConfig.SR);
    float ey = randRange(m_enemyConfig.SR, m_window.getSize().y - m_enemyConfig.SR);

    entity->cTransform = std::make_shared<CTransform>(Vec2(ex,ey), Vec2(2,2), 0.0f);

    entity->cShape = std::make_shared<CShape>(16.0f, 3, sf::Color(0,0,255), sf::Color(255,255,255), 4.0f);

    entity->cInput = std::make_shared<CInput>();

    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::setPaused(bool paused)
{
    // todo
}