#include "Game.h"
#include "Menu.h"
#include "Formation.h"
#include "Player.h"
#include "Bullet.h"
#include "Shield.h"
#include <iostream>
#include <string>
#include <algorithm>

Game::Game(unsigned int windowWidth, unsigned int windowHeight)
: windowWidth_(windowWidth)
, windowHeight_(windowHeight)
, window_(sf::VideoMode({ windowWidth_, windowHeight_ }), "Naves")
, VIRTUAL_WIDTH_(windowWidth)
, VIRTUAL_HEIGHT_(windowHeight)
, rng_(static_cast<unsigned int>(std::random_device{}()))
{
    window_.setVerticalSyncEnabled(true);
    gameView_.setCenter(sf::Vector2f(static_cast<float>(VIRTUAL_WIDTH_)/2.f, static_cast<float>(VIRTUAL_HEIGHT_)/2.f));
    gameView_.setSize(sf::Vector2f(static_cast<float>(VIRTUAL_WIDTH_), static_cast<float>(VIRTUAL_HEIGHT_)));
    playerStart_ = sf::Vector2f(MARGIN_.x + (WINDOW_COLS * CELL_SIZE) / 2.f,
                                MARGIN_.y + HUD_HEIGHT + (WINDOW_ROWS * CELL_SIZE) - CELL_SIZE * 1.5f);
}

Game::~Game() {
    delete menu_;
    delete pauseMenu_;
}

bool Game::loadAssets() {
    bool ok = true;
    hasFont_ = font_.openFromFile("assets/fonts/font.ttf");
    if (!hasFont_) { std::cerr << "[WARN] could not load font\n"; ok = false; }
    if (!texPlayer_.loadFromFile("assets/textures/player.png")) { std::cerr << "[WARN] could not load player.png\n"; ok = false; }
    if (!texBulletPlayer_.loadFromFile("assets/textures/bullet.png")) { std::cerr << "[WARN] could not load bullet.png\n"; ok = false; }
    if (!texBulletEnemy_.loadFromFile("assets/textures/bullet_2.png")) { std::cerr << "[WARN] could not load bullet_2.png\n"; ok = false; }
    if (!texAlienTop_.loadFromFile("assets/textures/alien_top.png")) { std::cerr << "[WARN] could not load alien_top.png\n"; ok = false; }
    if (!texAlienMid_.loadFromFile("assets/textures/alien_mid.png")) { std::cerr << "[WARN] could not load alien_mid.png\n"; ok = false; }
    if (!texAlienBot_.loadFromFile("assets/textures/alien_bottom.png")) { std::cerr << "[WARN] could not load alien_bottom.png\n"; ok = false; }
    if (!texShield_.loadFromFile("assets/textures/shield.png")) { std::cerr << "[WARN] could not load shield.png\n"; ok = false; }

    if (laserBuf_.loadFromFile("assets/sounds/laser_sound.mp3")) laserSound_.emplace(laserBuf_);
    else std::cerr << "[WARN] could not load laser_sound.mp3\n";

    explosionLoaded_ = explosionBuf_.loadFromFile("assets/sounds/explosion_enemy.mp3");
    if (!explosionLoaded_) std::cerr << "[WARN] could not load explosion_enemy.mp3\n";

    return ok;
}

bool Game::init() {
    if (!loadAssets()) std::cerr << "Continuing in degraded mode\n";
    createView();

    if (bgMusic_.openFromFile("assets/music/bg_music.ogg")) { bgMusic_.setLooping(true); bgMusic_.play(); musicOn_ = true; }

    menu_ = new Menu(hasFont_ ? &font_ : nullptr, 80);
    menu_->setOptions({ "NEW GAME", "EXIT" }, { static_cast<float>(VIRTUAL_WIDTH_) / 2.f, static_cast<float>(VIRTUAL_HEIGHT_) / 2.f }, 140.f);

    pauseMenu_ = new Menu(hasFont_ ? &font_ : nullptr, 56);
    pauseMenu_->setOptions({ "RESUME", "RESTART", "EXIT TO MENU" }, { static_cast<float>(VIRTUAL_WIDTH_) / 2.f, static_cast<float>(VIRTUAL_HEIGHT_) / 2.f }, 96.f);

    musicBtn_.setSize({48.f, 48.f});
    musicBtn_.setPosition(MARGIN_);
    musicBtn_.setFillColor(sf::Color(40,40,50));
    musicBtn_.setOutlineColor(sf::Color(200,200,200));
    musicBtn_.setOutlineThickness(-2.f);

    if (hasFont_) {
        musicIcon_.emplace(font_, musicOn_ ? "Off" : "On", 22);
        musicIcon_->setFillColor(sf::Color::White);
        auto rt = musicIcon_->getLocalBounds();
        float ox = rt.position.x + rt.size.x * 0.5f;
        float oy = rt.position.y + rt.size.y * 0.5f;
        musicIcon_->setOrigin(sf::Vector2f(ox, oy));
        sf::Vector2f bpos = musicBtn_.getPosition();
        sf::Vector2f bsize = musicBtn_.getSize();
        musicIcon_->setPosition(sf::Vector2f(bpos.x + bsize.x * 0.5f, bpos.y + bsize.y * 0.5f));
    }

    bullets_.reserve(64);
    for (size_t i = 0; i < 64; ++i) bullets_.emplace_back(texBulletPlayer_.getSize().x ? &texBulletPlayer_ : nullptr);
    enemyBullets_.reserve(32);
    for (size_t i = 0; i < 32; ++i) enemyBullets_.emplace_back(texBulletEnemy_.getSize().x ? &texBulletEnemy_ : nullptr);

    if (hasFont_) {
        scoreText_.emplace(font_, "Score: 0", 28);
        scoreText_->setFillColor(sf::Color::White);
        livesText_.emplace(font_, "Lives: 3", 28);
        livesText_->setFillColor(sf::Color::White);
        overlayTitle_.emplace(font_, "", 64);
        overlayTitle_->setFillColor(sf::Color::White);
        overlaySub_.emplace(font_, "", 28);
        overlaySub_->setFillColor(sf::Color(200,200,200));
    }

    player_ = std::make_unique<Player>(texPlayer_.getSize().x ? &texPlayer_ : nullptr, playerStart_);
    formation_ = createFormation();

    // prepare explosion sounds pool
    explosionSounds_.clear();
    if (explosionLoaded_) {
        const size_t POOL_SIZE = 8;
        explosionSounds_.reserve(POOL_SIZE);
        for (size_t i = 0; i < POOL_SIZE; ++i) explosionSounds_.emplace_back(explosionBuf_);
        explosionSoundIndex_ = 0;
    }

    resetGameState();
    return true;
}

void Game::createView() {
    updateGameViewForWindow(window_.getSize().x, window_.getSize().y);
}

void Game::updateGameViewForWindow(unsigned int winW, unsigned int winH) {
    float scaleX = static_cast<float>(winW) / static_cast<float>(VIRTUAL_WIDTH_);
    if (winW > MAX_CONTENT_WIDTH_) scaleX = static_cast<float>(MAX_CONTENT_WIDTH_) / static_cast<float>(VIRTUAL_WIDTH_);
    float scaleY = static_cast<float>(winH) / static_cast<float>(VIRTUAL_HEIGHT_);
    float scale = std::min(scaleX, scaleY);
    float viewWpx = static_cast<float>(VIRTUAL_WIDTH_) * scale;
    float viewHpx = static_cast<float>(VIRTUAL_HEIGHT_) * scale;
    float vpW = viewWpx / static_cast<float>(winW);
    float vpH = viewHpx / static_cast<float>(winH);
    float vpL = (1.f - vpW) * 0.5f;
    float vpT = (1.f - vpH) * 0.5f;
    gameView_.setViewport(sf::FloatRect({vpL, vpT}, {vpW, vpH}));
}

std::unique_ptr<Formation> Game::createFormation() {
    const float formationStartX = MARGIN_.x + 2.f * CELL_SIZE;
    const float formationStartY = MARGIN_.y + HUD_HEIGHT + 1.f * CELL_SIZE;
    const float spacingX = static_cast<float>(CELL_SIZE) * 1.65f;
    const float spacingY = static_cast<float>(CELL_SIZE) * 1.15f;
    return std::make_unique<Formation>(
        texAlienTop_.getSize().x ? &texAlienTop_ : nullptr,
        texAlienMid_.getSize().x ? &texAlienMid_ : nullptr,
        texAlienBot_.getSize().x ? &texAlienBot_ : nullptr,
        ENEMY_COLS, ENEMY_ROWS,
        sf::Vector2f{ formationStartX, formationStartY },
        spacingX, spacingY,
        40.f, 18.f
    );
}

void Game::resetGameState() {
    player_->setPosition(playerStart_);
    formation_ = createFormation();
    for (auto &b : bullets_) b.deactivate();
    for (auto &b : enemyBullets_) b.deactivate();
    shields_.clear();
    pausedForResult_ = false;
    paused_ = false;
    score_ = 0;
    lives_ = 3;
    if (scoreText_) scoreText_->setString("Score: 0");
    if (livesText_) livesText_->setString("Lives: 3");

    float shieldsY = player_->bounds().position.y - 120.f;
    sf::Vector2f desiredSize{ 140.f, 70.f };
    float padding = 48.f;
    float available = static_cast<float>(VIRTUAL_WIDTH_) - 2.f * padding;
    float totalW = 4.f * desiredSize.x;
    float gapBetween = 0.f;
    if (available > totalW) gapBetween = (available - totalW) / 3.f + desiredSize.x;
    else gapBetween = desiredSize.x + 12.f;
    float firstCenterX = padding + desiredSize.x * 0.5f;
    for (int i = 0; i < 4; ++i) {
        float centerX = firstCenterX + static_cast<float>(i) * gapBetween;
        if (texShield_.getSize().x) shields_.emplace_back(&texShield_, sf::Vector2f{ centerX - desiredSize.x / 2.f, shieldsY }, SHIELD_HP, desiredSize);
    }

    shootTimer_ = 0.f;
    enemyShootTimer_ = enemyShootDist_(rng_);
}

bool Game::trySpawnFromColumn(int col) {
    if (!formation_) return false;
    auto &en = formation_->enemies();
    for (int r = ENEMY_ROWS - 1; r >= 0; --r) {
        int idx = r * ENEMY_COLS + col;
        if (idx < 0 || idx >= static_cast<int>(en.size())) continue;
        auto &enemy = en[idx];
        if (enemy.isActive()) {
            sf::FloatRect eb = enemy.bounds();
            sf::Vector2f shotPos{ eb.position.x + eb.size.x / 2.f, eb.position.y + eb.size.y + 4.f };
            for (auto &b : enemyBullets_) {
                if (!b.isActive()) {
                    b.spawn(shotPos, 220.f);
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

bool Game::rectsIntersect(const sf::FloatRect& a, const sf::FloatRect& b) {
    return !(a.position.x + a.size.x < b.position.x ||
             b.position.x + b.size.x < a.position.x ||
             a.position.y + a.size.y < b.position.y ||
             b.position.y + b.size.y < a.position.y);
}

void Game::handleEvents() {
    while (auto evOpt = window_.pollEvent()) {
        const sf::Event& ev = *evOpt;
        if (ev.is<sf::Event::Closed>()) { window_.close(); break; }

        if (ev.is<sf::Event::Resized>()) {
            auto r = ev.getIf<sf::Event::Resized>();
            if (r) updateGameViewForWindow(static_cast<unsigned int>(r->size.x), static_cast<unsigned int>(r->size.y));
        }

        if (ev.is<sf::Event::KeyPressed>()) {
            auto k = ev.getIf<sf::Event::KeyPressed>();
            if (!k) continue;
            if (k->code == sf::Keyboard::Key::Escape && !pausedForResult_) paused_ = !paused_;
            if (pausedForResult_) {
                if (k->code == sf::Keyboard::Key::Enter || k->code == sf::Keyboard::Key::Space) resetGameState();
            }
        }

        if (ev.is<sf::Event::MouseButtonPressed>()) {
            auto mb = ev.getIf<sf::Event::MouseButtonPressed>();
            if (!mb) continue;
            if (mb->button == sf::Mouse::Button::Left) {
                sf::Vector2i pix = sf::Mouse::getPosition(window_);
                sf::Vector2f mp = window_.mapPixelToCoords(pix, window_.getDefaultView());
                if (musicBtn_.getGlobalBounds().contains(mp)) {
                    if (bgMusic_.getStatus() == sf::SoundSource::Status::Playing) { bgMusic_.pause(); musicOn_ = false; }
                    else { bgMusic_.play(); musicOn_ = true; }
                    if (musicIcon_) musicIcon_->setString(musicOn_ ? "Off" : "On");
                }
            }
        }

        if (state_ == AppState::Menu) {
            sf::View prev = window_.getView();
            window_.setView(window_.getDefaultView());
            if (menu_) menu_->processEvent(ev, window_);
            window_.setView(prev);
            continue;
        }

        if (paused_ && !pausedForResult_) {
            sf::View prev = window_.getView();
            window_.setView(window_.getDefaultView());
            if (pauseMenu_) {
                pauseMenu_->processEvent(ev, window_);
                if (pauseMenu_->consumeConfirm()) {
                    int sel = pauseMenu_->getSelectedIndex();
                    if (sel == 0) paused_ = false;
                    else if (sel == 1) { resetGameState(); state_ = AppState::Playing; paused_ = false; }
                    else if (sel == 2) { paused_ = false; state_ = AppState::Menu; }
                }
            }
            window_.setView(prev);
            continue;
        }

        // no further in-game event handling needed here (realtime input handled in update)
    }
}

void Game::update(float dt) {
    if (state_ == AppState::Menu) {
        if (menu_) {
            menu_->update(dt);
            if (menu_->consumeConfirm()) {
                int sel = menu_->getSelectedIndex();
                if (sel == 0) { resetGameState(); state_ = AppState::Playing; }
                else if (sel == 1) { window_.close(); }
            }
        }
        return;
    }

    if (paused_ || pausedForResult_) return;

    shootTimer_ -= dt; if (shootTimer_ < 0.f) shootTimer_ = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) player_->moveLeft(dt);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) player_->moveRight(dt);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && shootTimer_ <= 0.f) {
        sf::FloatRect pb = player_->bounds();
        sf::Vector2f bulletPos{ pb.position.x + pb.size.x / 2.f, pb.position.y - 6.f };
        for (auto &b : bullets_) {
            if (!b.isActive()) { b.spawn(bulletPos, -480.f); if (laserSound_) laserSound_->play(); shootTimer_ = SHOOT_COOLDOWN; break; }
        }
    }

    player_->update(dt);
    for (auto &b : bullets_) b.update(dt);
    for (auto &b : enemyBullets_) b.update(dt);
    if (formation_) formation_->update(dt, MARGIN_.x, static_cast<float>(VIRTUAL_WIDTH_) - MARGIN_.x);

    enemyShootTimer_ -= dt;
    if (enemyShootTimer_ <= 0.f) {
        int tries = ENEMY_COLS; bool spawned = false;
        while (tries-- > 0 && !spawned) {
            int col = enemyColDist_(rng_);
            spawned = trySpawnFromColumn(col);
        }
        enemyShootTimer_ = enemyShootDist_(rng_);
    }

    for (auto &b : bullets_) {
        if (!b.isActive()) continue;
        bool hitShield = false;
        for (auto &s : shields_) {
            if (!s.isActive()) continue;
            if (rectsIntersect(s.bounds(), b.bounds())) { b.deactivate(); hitShield = true; break; }
        }
        if (hitShield) continue;
        for (auto &e : formation_->enemies()) {
            if (!e.isActive()) continue;
            if (rectsIntersect(b.bounds(), e.bounds())) {
                b.deactivate();
                e.setActive(false);

                // play explosion sound
                if (explosionLoaded_ && !explosionSounds_.empty()) {
                    explosionSounds_[explosionSoundIndex_].setBuffer(explosionBuf_);
                    explosionSounds_[explosionSoundIndex_].play();
                    explosionSoundIndex_ = (explosionSoundIndex_ + 1) % explosionSounds_.size();
                }

                score_ += 10;
                if (scoreText_) scoreText_->setString("Score: " + std::to_string(score_));
                break;
            }
        }
    }

    for (auto &b : enemyBullets_) {
        if (!b.isActive()) continue;
        bool hitShield = false;
        for (auto &s : shields_) {
            if (!s.isActive()) continue;
            if (rectsIntersect(s.bounds(), b.bounds())) { b.deactivate(); s.takeDamage(1); hitShield = true; break; }
        }
        if (hitShield) continue;
        if (rectsIntersect(b.bounds(), player_->bounds())) {
            b.deactivate();
            lives_ -= 1;
            if (livesText_) livesText_->setString("Lives: " + std::to_string(lives_));
            if (lives_ <= 0) {
                pausedForResult_ = true;
                if (overlayTitle_) { overlayTitle_->setString("GAME OVER"); overlayTitle_->setFillColor(sf::Color::Red); }
                if (overlaySub_) overlaySub_->setString("Press ENTER to restart");
            } else {
                player_->setPosition(playerStart_);
            }
        }
    }

    for (auto &e : formation_->enemies()) {
        if (!e.isActive()) continue;
        sf::FloatRect eb = e.bounds();
        if (eb.position.y + eb.size.y >= playerStart_.y - CELL_SIZE * 0.5f) {
            pausedForResult_ = true;
            if (overlayTitle_) { overlayTitle_->setString("GAME OVER"); overlayTitle_->setFillColor(sf::Color::Red); }
            if (overlaySub_) overlaySub_->setString("Press ENTER to restart");
            break;
        }
    }

    bool anyAlive = false;
    for (auto &e : formation_->enemies()) { if (e.isActive()) { anyAlive = true; break; } }
    if (!anyAlive) {
        pausedForResult_ = true;
        if (overlayTitle_) { overlayTitle_->setString("YOU WIN"); overlayTitle_->setFillColor(sf::Color::Yellow); }
        if (overlaySub_) overlaySub_->setString("Press ENTER to restart");
    }

    // music handling: pause/resume depending on menu visibility
    bool menuVisible = (state_ == AppState::Menu) || (state_ == AppState::Playing && (paused_ || pausedForResult_));
    if (bgMusic_.getStatus() == sf::SoundSource::Status::Playing) {
        if (menuVisible && musicWasPlayingBeforeMenu_ == false) {
            musicWasPlayingBeforeMenu_ = true;
            bgMusic_.pause();
        }
    } else {
        if (!menuVisible && musicWasPlayingBeforeMenu_ && musicOn_) {
            bgMusic_.play();
            musicWasPlayingBeforeMenu_ = false;
        }
    }
}

void Game::render() {
    window_.clear(sf::Color(18,18,28));

    if (state_ == AppState::Menu) {
        window_.setView(window_.getDefaultView());
        sf::RectangleShape fullBg(sf::Vector2f(static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)));
        fullBg.setFillColor(sf::Color(8,8,12));
        window_.draw(fullBg);
        if (menu_) menu_->draw(window_);
        window_.display();
        return;
    }

    // If a menu is visible (pause or end-game), draw only default-view overlay as in original main
    if (paused_ || pausedForResult_) {
        window_.setView(window_.getDefaultView());
        sf::RectangleShape fullBg(sf::Vector2f(static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)));
        fullBg.setFillColor(sf::Color(0,0,0,200));
        window_.draw(fullBg);

        if (paused_ && !pausedForResult_) {
            if (pauseMenu_) pauseMenu_->draw(window_);
        }

        if (pausedForResult_) {
            if (hasFont_ && overlayTitle_ && overlaySub_) {
                sf::Vector2u cur = window_.getSize();
                sf::FloatRect rt = overlayTitle_->getLocalBounds();
                float ox = rt.position.x + rt.size.x * 0.5f;
                float oy = rt.position.y + rt.size.y * 0.5f;
                overlayTitle_->setOrigin(sf::Vector2f(ox, oy));
                overlayTitle_->setPosition(sf::Vector2f(static_cast<float>(cur.x)/2.f, static_cast<float>(cur.y)/2.f - 24.f));

                sf::FloatRect rs = overlaySub_->getLocalBounds();
                float sox = rs.position.x + rs.size.x * 0.5f;
                float soy = rs.position.y + rs.size.y * 0.5f;
                overlaySub_->setOrigin(sf::Vector2f(sox, soy));
                overlaySub_->setPosition(sf::Vector2f(static_cast<float>(cur.x)/2.f, static_cast<float>(cur.y)/2.f + 40.f));

                window_.draw(*overlayTitle_);
                window_.draw(*overlaySub_);
            }
        }

        window_.display();
        return;
    }

    // Normal gameplay rendering
    window_.setView(gameView_);
    for (auto &s : shields_) s.draw(window_);
    if (formation_) formation_->draw(window_);
    for (auto &b : bullets_) if (b.isActive()) b.draw(window_);
    for (auto &b : enemyBullets_) if (b.isActive()) b.draw(window_);
    if (player_) player_->draw(window_);

    window_.setView(window_.getDefaultView());
    sf::Vector2u curSize = window_.getSize();

    // Draw music button + icon
    window_.draw(musicBtn_);
    if (musicIcon_) window_.draw(*musicIcon_);

    // Draw score and lives to the right of music button
    {
        sf::Vector2f btnPos = musicBtn_.getPosition();
        sf::Vector2f btnSize = musicBtn_.getSize();
        const float paddingX = 12.f;
        const float spacing = 12.f;
        const float startX = btnPos.x + btnSize.x + paddingX;
        const float centerY = btnPos.y + btnSize.y * 0.5f;

        if (scoreText_) {
            sf::FloatRect tb = scoreText_->getLocalBounds();
            scoreText_->setOrigin(sf::Vector2f(0.f, tb.position.y + tb.size.y * 0.5f));
            scoreText_->setPosition(sf::Vector2f(startX, centerY));
            window_.draw(*scoreText_);
        }

        float nextX = startX;
        if (scoreText_) nextX += scoreText_->getGlobalBounds().size.x + spacing;
        if (livesText_) {
            sf::FloatRect tb2 = livesText_->getLocalBounds();
            livesText_->setOrigin(sf::Vector2f(0.f, tb2.position.y + tb2.size.y * 0.5f));
            livesText_->setPosition(sf::Vector2f(nextX, centerY));
            window_.draw(*livesText_);
        }
    }

    // Pause overlay/menu if needed (draw above HUD)
    if (paused_ && !pausedForResult_) {
        sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(curSize.x), static_cast<float>(curSize.y)));
        overlay.setFillColor(sf::Color(0,0,0,160));
        window_.draw(overlay);
        if (pauseMenu_) pauseMenu_->draw(window_);
    }

    if (pausedForResult_) {
        sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(curSize.x), static_cast<float>(curSize.y)));
        overlay.setFillColor(sf::Color(0,0,0,160));
        window_.draw(overlay);
        if (overlayTitle_ && overlaySub_) {
            sf::FloatRect rt = overlayTitle_->getLocalBounds();
            float ox = rt.position.x + rt.size.x * 0.5f;
            float oy = rt.position.y + rt.size.y * 0.5f;
            overlayTitle_->setOrigin(sf::Vector2f(ox, oy));
            overlayTitle_->setPosition(sf::Vector2f(static_cast<float>(curSize.x)/2.f, static_cast<float>(curSize.y)/2.f - 24.f));
            sf::FloatRect rs = overlaySub_->getLocalBounds();
            float sox = rs.position.x + rs.size.x * 0.5f;
            float soy = rs.position.y + rs.size.y * 0.5f;
            overlaySub_->setOrigin(sf::Vector2f(sox, soy));
            overlaySub_->setPosition(sf::Vector2f(static_cast<float>(curSize.x)/2.f, static_cast<float>(curSize.y)/2.f + 40.f));
            window_.draw(*overlayTitle_);
            window_.draw(*overlaySub_);
        }
    }

    window_.display();
}

void Game::run() {
    while (window_.isOpen()) {
        handleEvents();
        float dt = clock_.restart().asSeconds();
        update(dt);
        render();
    }
}