#include "Enemy.h"
#include <memory>


static constexpr float TARGET_ENEMY_W = 50.f;
static constexpr float TARGET_ENEMY_H = 45.f;

Enemy::Enemy(const sf::Texture* texture, const sf::Vector2f& startPos) {
    if (texture) {
        sprite_ = std::make_unique<sf::Sprite>(*texture);
        auto local = sprite_->getLocalBounds();

        float scaleX = TARGET_ENEMY_W / local.size.x;
        float scaleY = TARGET_ENEMY_H / local.size.y;
        float scale = std::min(scaleX, scaleY);
        sprite_->setScale({ scale, scale });

        auto newLocal = sprite_->getLocalBounds();
        sprite_->setOrigin({ newLocal.size.x / 2.f, newLocal.size.y / 2.f });
        sprite_->setPosition(startPos);
    } else {
        fallbackRect_.setSize({TARGET_ENEMY_W, TARGET_ENEMY_H});
        fallbackRect_.setOrigin(fallbackRect_.getSize() / 2.f);
        fallbackRect_.setFillColor(sf::Color(200,80,80));
        fallbackRect_.setPosition(startPos);
    }
    leftLimit_ = startPos.x - 80.f;
    rightLimit_ = startPos.x + 80.f;
}

void Enemy::update(float dt) {
    if (!active_) return;
}

void Enemy::draw(sf::RenderWindow& window) const {
    if (!active_) return;
    if (sprite_) window.draw(*sprite_);
    else window.draw(fallbackRect_);
}

void Enemy::setActive(bool v) { active_ = v; }
bool Enemy::isActive() const { return active_; }

sf::FloatRect Enemy::bounds() const {
    if (sprite_) return sprite_->getGlobalBounds();
    return fallbackRect_.getGlobalBounds();
}

void Enemy::setPosition(const sf::Vector2f& pos) {
    if (sprite_) sprite_->setPosition(pos);
    else fallbackRect_.setPosition(pos);
}

sf::Vector2f Enemy::getPosition() const {
    if (sprite_) return sprite_->getPosition();
    return fallbackRect_.getPosition();
}

void Enemy::moveBy(const sf::Vector2f& delta) {
    if (sprite_) sprite_->move(delta);
    else fallbackRect_.move(delta);
}