#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Enemy.h"

class Formation {
public:
    Formation(const sf::Texture* topTex,
              const sf::Texture* midTex,
              const sf::Texture* botTex,
              int cols, int rows,
              const sf::Vector2f& startPos,
              float spacingX, float spacingY,
              float speed = 60.f,
              float dropAmount = 16.f);

    void update(float dt, float screenLeft, float screenRight);

    void draw(sf::RenderWindow& window) const;

    std::vector<Enemy>& enemies() { return enemies_; }
    const std::vector<Enemy>& enemies() const { return enemies_; }

    void reset();
    int aliveCount() const;

private:
    void computeBounds();

    const sf::Texture* topTex_;
    const sf::Texture* midTex_;
    const sf::Texture* botTex_;

    std::vector<Enemy> enemies_;
    int cols_;
    int rows_;
    sf::Vector2f startPos_;
    float spacingX_;
    float spacingY_;

    int dir_ = 1; // 1 right, -1 left
    float speed_;
    float dropAmount_;

    float minX_ = 0.f;
    float maxX_ = 0.f;
};