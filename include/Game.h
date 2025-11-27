#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <optional>
#include <random>

class Game {
public:
    Game(unsigned int windowWidth, unsigned int windowHeight);
    ~Game();

    bool init();
    void run();

private:
    unsigned int windowWidth_;
    unsigned int windowHeight_;
    sf::RenderWindow window_;
    sf::View gameView_;
    unsigned int VIRTUAL_WIDTH_;
    unsigned int VIRTUAL_HEIGHT_;
    unsigned int MAX_CONTENT_WIDTH_ = 1280u;

    sf::Font font_;
    bool hasFont_ = false;
    sf::Texture texPlayer_, texBulletPlayer_, texBulletEnemy_;
    sf::Texture texAlienTop_, texAlienMid_, texAlienBot_, texShield_;
    sf::Music bgMusic_;
    sf::SoundBuffer laserBuf_;
    std::optional<sf::Sound> laserSound_;

    sf::SoundBuffer explosionBuf_;
    bool explosionLoaded_ = false;
    std::vector<sf::Sound> explosionSounds_;
    size_t explosionSoundIndex_ = 0;

    std::unique_ptr<class Menu> menu_;
    std::unique_ptr<class Menu> pauseMenu_;

    std::unique_ptr<class Formation> formation_;
    std::vector<class Bullet> bullets_;
    std::vector<class Bullet> enemyBullets_;
    std::vector<class Shield> shields_;
    std::unique_ptr<class Player> player_;

    sf::RectangleShape musicBtn_;
    std::optional<sf::Text> musicIcon_;
    bool musicOn_ = false;
    bool musicWasPlayingBeforeMenu_ = false;

    std::optional<sf::Text> scoreText_;
    std::optional<sf::Text> livesText_;
    int score_ = 0;
    int lives_ = 0;

    bool pausedForResult_ = false;
    bool paused_ = false;
    std::optional<sf::Text> overlayTitle_;
    std::optional<sf::Text> overlaySub_;

    sf::Clock clock_;
    float shootTimer_ = 0.f;
    const float SHOOT_COOLDOWN = 0.6f;
    const int SHIELD_HP = 15;

    std::mt19937 rng_;
    std::uniform_real_distribution<float> enemyShootDist_{0.8f, 1.8f};
    std::uniform_int_distribution<int> enemyColDist_{0, 10};
    float enemyShootTimer_ = 0.f;

    enum class AppState { Menu, Playing };
    AppState state_ = AppState::Menu;

    sf::Vector2f MARGIN_{12.f, 12.f};
    const int WINDOW_COLS = 24;
    const int WINDOW_ROWS = 25;
    const int CELL_SIZE = 32;
    const float HUD_HEIGHT = 64.f;
    sf::Vector2f playerStart_;

    static constexpr int ENEMY_COLS = 11;
    static constexpr int ENEMY_ROWS = 5;

    int wave_ = 1;

    bool loadAssets();
    void createView();
    void updateGameViewForWindow(unsigned int winW, unsigned int winH);
    std::unique_ptr<class Formation> createFormation();
    void resetGameState();
    bool trySpawnFromColumn(int col);
    static bool rectsIntersect(const sf::FloatRect& a, const sf::FloatRect& b);

    void spawnNextWave();

    void handleEvents();
    void update(float dt);
    void render();
};