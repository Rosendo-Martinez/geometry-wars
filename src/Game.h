#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"


struct PlayerConfig { int SR = 32, CR = 32, FR = 5, FG = 5, FB = 5, OR = 255, OG = 0, OB = 0, OT = 4, V = 8; float S = 5; };
struct EnemyConfig { int SR = 32, CR = 32, OR = 255, OG = 255, OB = 255, OT = 2, VMIN = 3, VMAX = 8, L = 90, SI = 60, SNE = 50, SSE = 125; float SMIN = 3, SMAX = 3; };
struct BulletConfig { int SR = 10, CR = 10, FR = 255, FG = 255, FB = 255, OR = 255, OG = 255, OB = 255, OT = 2, V = 20, L = 90; float S = 20; };
struct NukeConfig { int V = 20, ER = 150, BR = 300, L = 40, REL = 150, FR = 232, FG = 100, FB = 61, OR = 192, OG = 192, OB = 192, CDI = 100; float BVM = 3; };

class Game
{
public:
    Game();
    void run();

private:
    sf::RenderWindow    m_window;
    EntityManager       m_entities;
    sf::Font            m_font;
    sf::Text            m_text;
    PlayerConfig        m_playerConfig;
    EnemyConfig         m_enemyConfig;
    BulletConfig        m_bulletConfig;
    NukeConfig          m_nukeConfig;
    int                 m_currentFrame          = 0;
    int                 m_lastEnemySpawnTime    = 0;
    int                 m_lastNukeTime          = 0;
    bool                m_paused                = false;
    bool                m_running               = true;
    bool                m_startMenu             = true;
    bool                m_endGameMenu           = false;
    float               m_startMenuInstructionAlphaPercent = 1;
    int                 m_highScore             = 0;

    std::shared_ptr<Entity> m_player;

    void init();
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