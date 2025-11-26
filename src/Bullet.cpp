#include "Bullet.h"
#include <memory>

static constexpr float TARGET_BULLET_W = 15.f;
static constexpr float TARGET_BULLET_H = 15.f;

Bullet::Bullet(const sf::Texture* texture) {
    if (texture) {
        sprite_ = std::make_unique<sf::Sprite>(*texture);
        auto local = sprite_->getLocalBounds();

        float scaleX = TARGET_BULLET_W / local.size.x;
        float scaleY = TARGET_BULLET_H / local.size.y;
        float scale = std::min(scaleX, scaleY);
        sprite_->setScale({ scale, scale });

        auto newLocal = sprite_->getLocalBounds();
        sprite_->setOrigin({ newLocal.size.x / 2.f, newLocal.size.y / 2.f });
    } else {
        fallbackRect_.setSize({TARGET_BULLET_W, TARGET_BULLET_H});
        fallbackRect_.setOrigin(fallbackRect_.getSize() / 2.f);
        fallbackRect_.setFillColor(sf::Color::Yellow);
    }
}

void Bullet::spawn(const sf::Vector2f& pos, float speedY) {
    active_ = true;
    speedY_ = speedY;
    if (sprite_) sprite_->setPosition(pos);
    else fallbackRect_.setPosition(pos);
}

void Bullet::update(float dt) {
    if (!active_) return;
    sf::Vector2f move(0.f, speedY_ * dt);
    if (sprite_) sprite_->move(move);
    else fallbackRect_.move(move);

    sf::FloatRect r = bounds();
    if (r.position.y + r.size.y < -200.f || r.position.y > 5000.f) {
        active_ = false;
    }
}

void Bullet::deactivate() { active_ = false; }

bool Bullet::isActive() const { return active_; }

sf::FloatRect Bullet::bounds() const {
    if (sprite_) return sprite_->getGlobalBounds();
    return fallbackRect_.getGlobalBounds();
}

void Bullet::draw(sf::RenderWindow& window) const {
    if (!active_) return;
    if (sprite_) window.draw(*sprite_);
    else window.draw(fallbackRect_);
}