#include "Shield.h"
#include <algorithm>
#include <cstdint>

Shield::Shield(const sf::Texture* tex, const sf::Vector2f& position, int hp, const sf::Vector2f& size)
: tex_(tex), hp_(hp), maxHp_(hp), active_(hp > 0) {
    if (tex_) sprite_ = std::make_unique<sf::Sprite>(*tex_);
    if (sprite_) {
        sprite_->setPosition(position);
        if (size.x > 0.f || size.y > 0.f) {
            sf::Vector2u ts = tex_->getSize();
            float scaleX = 1.f;
            float scaleY = 1.f;
            if (ts.x > 0 && size.x > 0.f) scaleX = size.x / static_cast<float>(ts.x);
            if (ts.y > 0 && size.y > 0.f) scaleY = size.y / static_cast<float>(ts.y);
            if (size.x > 0.f && size.y == 0.f && ts.y > 0) scaleY = scaleX;
            if (size.y > 0.f && size.x == 0.f && ts.x > 0) scaleX = scaleY;
            sprite_->setScale(sf::Vector2f{scaleX, scaleY});
        }
    }
}

void Shield::draw(sf::RenderWindow& window) const {
    if (!active_) return;
    if (sprite_) window.draw(*sprite_);
}

sf::FloatRect Shield::bounds() const {
    if (sprite_) return sprite_->getGlobalBounds();
    return sf::FloatRect{};
}

bool Shield::takeDamage(int dmg) {
    if (!active_) return false;
    hp_ -= dmg;
    if (hp_ <= 0) {
        active_ = false;
        return true;
    }
    float t = static_cast<float>(hp_) / static_cast<float>(std::max(1, maxHp_));
    uint8_t alpha = static_cast<uint8_t>(std::max(64.0f, 255.0f * t));
    if (sprite_) sprite_->setColor(sf::Color(255,255,255, alpha));
    return false;
}

bool Shield::isActive() const {
    return active_;
}