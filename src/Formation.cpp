#include "Formation.h"
#include <algorithm>

Formation::Formation(const sf::Texture* topTex,
                     const sf::Texture* midTex,
                     const sf::Texture* botTex,
                     int cols, int rows,
                     const sf::Vector2f& startPos,
                     float spacingX, float spacingY,
                     float speed, float dropAmount)
: topTex_(topTex), midTex_(midTex), botTex_(botTex),
  cols_(cols), rows_(rows), startPos_(startPos),
  spacingX_(spacingX), spacingY_(spacingY),
  speed_(speed), dropAmount_(dropAmount)
{
    enemies_.clear();

    int topCount = 1;
    int midCount = 0;
    if (rows_ > 1) {
        midCount = std::max(1, (rows_ - topCount) / 2); // al menos 1 fila mid si hay más de 1 fila
    }
    int botCount = rows_ - topCount - midCount;
    if (botCount < 0) { botCount = 0; midCount = rows_ - topCount; }

    for (int r = 0; r < rows_; ++r) {
        const sf::Texture* tex = midTex_;
        if (r < topCount) {
            tex = topTex_;
        } else if (r < topCount + midCount) {
            tex = midTex_;
        } else {
            tex = botTex_;
        }

        for (int c = 0; c < cols_; ++c) {
            sf::Vector2f pos{ startPos_.x + c * spacingX_, startPos_.y + r * spacingY_ };
            enemies_.emplace_back(tex, pos);
        }
    }
    computeBounds();
}

void Formation::computeBounds() {
    bool first = true;
    float minx = 0.f, maxx = 0.f;
    for (const auto &e : enemies_) {
        if (!e.isActive()) continue;
        auto b = e.bounds();
        if (first) {
            minx = b.position.x;
            maxx = b.position.x + b.size.x;
            first = false;
        } else {
            minx = std::min(minx, b.position.x);
            maxx = std::max(maxx, b.position.x + b.size.x);
        }
    }
    if (first) {
        minX_ = maxX_ = 0.f;
    } else {
        minX_ = minx;
        maxX_ = maxx;
    }
}

void Formation::update(float dt, float screenLeft, float screenRight) {
    if (enemies_.empty()) return;

    float moveX = dir_ * speed_ * dt;
    for (auto &e : enemies_) {
        if (e.isActive()) e.moveBy({ moveX, 0.f });
    }

    computeBounds();

    if (minX_ < screenLeft || maxX_ > screenRight) {
        for (auto &e : enemies_) {
            if (e.isActive()) e.moveBy({ -moveX, 0.f });
        }
        // invertir y aplicar drop
        dir_ *= -1;
        for (auto &e : enemies_) {
            if (e.isActive()) e.moveBy({ 0.f, dropAmount_ });
        }
        // aumentar velocidad
        speed_ *= 1.07f;
        computeBounds();
    }
}

void Formation::draw(sf::RenderWindow& window) const {
    for (const auto &e : enemies_) {
        if (e.isActive()) e.draw(window);
    }
}

void Formation::reset() {
    enemies_.clear();
    int topCount = 1;
    int midCount = 0;
    if (rows_ > 1) {
        midCount = std::max(1, (rows_ - topCount) / 2);
    }
    int botCount = rows_ - topCount - midCount;
    if (botCount < 0) { botCount = 0; midCount = rows_ - topCount; }

    for (int r = 0; r < rows_; ++r) {
        const sf::Texture* tex = midTex_;
        if (r < topCount) {
            tex = topTex_;
        } else if (r < topCount + midCount) {
            tex = midTex_;
        } else {
            tex = botTex_;
        }

        for (int c = 0; c < cols_; ++c) {
            sf::Vector2f pos{ startPos_.x + c * spacingX_, startPos_.y + r * spacingY_ };
            enemies_.emplace_back(tex, pos);
        }
    }

    dir_ = 1;
    computeBounds();
}

int Formation::aliveCount() const {
    int cnt = 0;
    for (const auto &e : enemies_) if (e.isActive()) ++cnt;
    return cnt;
}