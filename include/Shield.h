#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <cstdint>

class Shield {
public:
    Shield() = default;
    Shield(const sf::Texture* tex, const sf::Vector2f& position, int hp = 3, const sf::Vector2f& size = {0.f,0.f});

    void draw(sf::RenderWindow& window) const;
    sf::FloatRect bounds() const;
    bool takeDamage(int dmg = 1);
    bool isActive() const;

private:
    const sf::Texture* tex_ = nullptr;
    std::unique_ptr<sf::Sprite> sprite_;
    int hp_ = 0;
    int maxHp_ = 0;
    bool active_ = false;
};