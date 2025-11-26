#pragma once
#include <SFML/Graphics.hpp>

class Enemy {
public:
    Enemy(const sf::Texture* texture = nullptr, const sf::Vector2f& startPos = {0.f,0.f});

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    void setActive(bool v);
    bool isActive() const;
    sf::FloatRect bounds() const;
    void setPosition(const sf::Vector2f& pos);

    sf::Vector2f getPosition() const;
    void moveBy(const sf::Vector2f& delta);

private:
    std::unique_ptr<sf::Sprite> sprite_;
    sf::RectangleShape fallbackRect_;
    bool active_ = true;

    float speedX_ = 80.f;
    int dir_ = 1; // 1 right, -1 left
    float leftLimit_ = 20.f;
    float rightLimit_ = 800.f;
};