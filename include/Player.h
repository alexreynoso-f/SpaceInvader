#ifndef GALAGA_PLAYER_H
#define GALAGA_PLAYER_H


#pragma once
#include <SFML/Graphics.hpp>

class Player {
public:
    Player(const sf::Texture* texture, const sf::Vector2f& startPos);
    void setHorizontalLimits(float left, float right);

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    void moveLeft(float dt);
    void moveRight(float dt);
    void setPosition(const sf::Vector2f& pos);
    sf::FloatRect bounds() const;

private:
    std::unique_ptr<sf::Sprite> sprite_;
    sf::RectangleShape fallbackRect_;
    sf::Vector2f position_;
    float speed_ = 150.f;
    float leftLimit_ = 16.f;
    float rightLimit_ = 800.f;
};


#endif