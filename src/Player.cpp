#include "Player.h"
#include <memory>

static constexpr float TARGET_PLAYER_W = 50.f; // ancho
static constexpr float TARGET_PLAYER_H = 50.f; // alto

Player::Player(const sf::Texture* texture, const sf::Vector2f& startPos)
    : position_(startPos)
{
    if (texture) {
        sprite_ = std::make_unique<sf::Sprite>(*texture);
        auto local = sprite_->getLocalBounds();

        float scaleX = TARGET_PLAYER_W / local.size.x;
        float scaleY = TARGET_PLAYER_H / local.size.y;
        float scale = std::min(scaleX, scaleY);
        sprite_->setScale({ scale, scale });

        auto newLocal = sprite_->getLocalBounds();
        sprite_->setOrigin({ newLocal.size.x / 2.f, newLocal.size.y / 2.f });
        sprite_->setPosition(position_);
    }
}

void Player::update(float /*dt*/) {
    if (sprite_) sprite_->setPosition(position_);
    else fallbackRect_.setPosition(position_);
}

void Player::draw(sf::RenderWindow& window) const {
    if (sprite_) window.draw(*sprite_);
    else window.draw(fallbackRect_);
}

void Player::moveLeft(float dt) {
    position_.x -= speed_ * dt;
    if (position_.x < 16.f) position_.x = 16.f;
}

void Player::setPosition(const sf::Vector2f& pos) {
    position_ = pos;
    if (sprite_) sprite_->setPosition(position_);
    else fallbackRect_.setPosition(position_);
}

sf::FloatRect Player::bounds() const {
    if (sprite_) return sprite_->getGlobalBounds();
    return fallbackRect_.getGlobalBounds();
}
void Player::setHorizontalLimits(float left, float right) {
    leftLimit_ = left;
    rightLimit_ = right;
}


void Player::moveRight(float dt) {
    position_.x += speed_ * dt;
    float halfW = 3.f;
    if (sprite_) {
        auto gb = sprite_->getGlobalBounds();
        halfW = gb.size.x / 3.f;
    } else {
        halfW = fallbackRect_.getSize().x / 3.f;
    }
    if (position_.x > rightLimit_ - halfW) position_.x = rightLimit_ - halfW;
}