#include "Game.h"

int main() {
    const int WINDOW_COLS = 24;
    const int WINDOW_ROWS = 25;
    const int CELL_SIZE = 32;
    const float HUD_HEIGHT = 64.f;
    const sf::Vector2f MARGIN{12.f, 12.f};

    unsigned int windowWidth = static_cast<unsigned int>(MARGIN.x * 2 + WINDOW_COLS * CELL_SIZE);
    unsigned int windowHeight = static_cast<unsigned int>(MARGIN.y + HUD_HEIGHT + WINDOW_ROWS * CELL_SIZE + MARGIN.y);

    Game game(windowWidth, windowHeight);
    if (!game.init()) return 1;
    game.run();
    return 0;
}