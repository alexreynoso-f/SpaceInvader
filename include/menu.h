#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class Menu {
public:
    Menu(const sf::Font* font = nullptr, unsigned int charSize = 36);

    // options: textos de las opciones
    // center: punto central horizontal a partir del cual se colocan las opciones verticalmente
    // spacing: separación vertical entre opciones (px)
    void setOptions(const std::vector<std::string>& options, const sf::Vector2f& center, float spacing);

    // establece textura de fondo (puede ser nullptr)
    void setBackground(const sf::Texture* tex);

    // procesamiento de eventos (teclado/ratón)
    void processEvent(const sf::Event& ev, sf::RenderWindow& window);

    // update por frame (dt en segundos)
    void update(float dt);

    // dibuja el menú (fondo, items, indicador)
    void draw(sf::RenderWindow& window) const;

    // cuando el usuario confirma (Enter o click), consumeConfirm devuelve true en el frame de la confirmación
    bool consumeConfirm();

    int getSelectedIndex() const;

private:
    const sf::Font* font_;
    unsigned int charSize_;
    std::vector<sf::Text> items_;
    std::vector<std::string> labels_;
    sf::Vector2f center_;
    float spacing_ = 64.f;
    int selected_ = 0;

    // fondo opcional (gestión dinámica para evitar default ctor issues)
    const sf::Texture* bgTex_ = nullptr;
    mutable std::unique_ptr<sf::Sprite> bgSprite_;

    // indicador triangular
    sf::ConvexShape pointer_;
    float pointerOffsetX_ = -48.f; // distancia relativa al borde izquierdo del texto

    // confirm flag
    mutable bool confirmFlag_ = false;

    // colores
    sf::Color colorNormal_ = sf::Color(140, 140, 140);
    sf::Color colorSelected_ = sf::Color(230, 230, 230);

    void rebuild();
};