#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"


// Default values are temporary, will read in actual values from file later
struct PlayerConfig { int SR = 32, CR = 32, FR = 5, FG = 5, FB = 5, OR = 255, OG = 0, OB = 0, OT = 4, V = 8; float S = 5; };
struct EnemyConfig { int SR = 32, CR = 32, OR = 255, OG = 255, OB = 255, OT = 2, VMIN = 3, VMAX = 8, L = 90, SI = 60; float SMIN = 3, SMAX = 3; };
struct BulletConfig { int SR = 10, CR = 10, FR = 255, FG = 255, FB = 255, OR = 255, OG = 255, OB = 255, OT = 2, V = 20, L = 90; float S = 20; };


class Game
{
public:
    Game(const std::string & config);

    void run();

private:
    sf::RenderWindow    m_window;
    EntityManager       m_entities;
    sf::Font            m_font;
    sf::Text            m_text;
    PlayerConfig        m_playerConfig;
    EnemyConfig         m_enemyConfig;
    BulletConfig        m_bulletConfig;
    int                 m_score = 0;
    int                 m_currentFrame = 0;
    int                 m_lastEnemySpawnTime = 0;
    bool                m_paused = false;
    bool                m_running = true;

    std::shared_ptr<Entity> m_player;

    void init(const std::string & config);
    void setPaused(bool paused);

    void sMovement();
    void sUserInput();
    void sLifespan();
    void sRender();
    void sEnemySpawner();
    void sCollision();

    void spawnPlayer();
    void spawnEnemy();
    void spawnSmallEnemies(std:: shared_ptr<Entity> entity);
    void spawnBullet(std::shared_ptr<Entity> entity, const Vec2 & mousePos);
    void spawnSpecialWeapon(std::shared_ptr<Entity> entity);
};