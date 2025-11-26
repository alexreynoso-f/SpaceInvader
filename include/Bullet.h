#pragma once
#include <SFML/Graphics.hpp>

class Bullet {
public:
    Bullet(const sf::Texture* texture = nullptr);

    void spawn(const sf::Vector2f& pos, float speedY);
    void update(float dt);
    void deactivate();
    bool isActive() const;
    sf::FloatRect bounds() const;
    void draw(sf::RenderWindow& window) const;

private:
    std::unique_ptr<sf::Sprite> sprite_;
    sf::RectangleShape fallbackRect_;
    bool active_ = false;
    float speedY_ = 0.f;
};