#include "Menu.h"
#include <cmath>
#include <iostream>

Menu::Menu(const sf::Font* font, unsigned int charSize)
: font_(font), charSize_(charSize)
{
    pointer_.setPointCount(3);
    // triángulo base (apuntando a la derecha); lo reposicionamos al dibujar
    pointer_.setPoint(0, sf::Vector2f(0.f, -12.f));
    pointer_.setPoint(1, sf::Vector2f(18.f, 0.f));
    pointer_.setPoint(2, sf::Vector2f(0.f, 12.f));
    pointer_.setFillColor(sf::Color(200,200,200));
    pointer_.setOutlineColor(sf::Color(80,80,80));
    pointer_.setOutlineThickness(-2.f);
}

void Menu::setOptions(const std::vector<std::string>& options, const sf::Vector2f& center, float spacing) {
    labels_ = options;
    center_ = center;
    spacing_ = spacing;
    items_.clear();

    if (!font_) {
        std::cerr << "[WARN] Menu::setOptions: font_ is null — cannot construct sf::Text items\n";
        // No hacemos nada más para evitar construir sf::Text sin parámetros
        return;
    }

    items_.reserve(options.size());
    for (const auto& s : options) {
        // sf::Text requires a Font reference in your SFML build, so construct with font_
        sf::Text t(*font_, s, charSize_);
        t.setFillColor(colorNormal_);
        items_.push_back(std::move(t));
    }
    selected_ = 0;
    rebuild();
}

void Menu::setBackground(const sf::Texture* tex) {
    bgTex_ = tex;
    if (bgTex_) {
        bgSprite_ = std::make_unique<sf::Sprite>(*bgTex_);
    } else {
        bgSprite_.reset();
    }
}

void Menu::processEvent(const sf::Event& ev, sf::RenderWindow& window) {
    // Use ev.is / ev.getIf (your project uses that API)
    if (ev.is<sf::Event::KeyPressed>()) {
        auto k = ev.getIf<sf::Event::KeyPressed>();
        if (!k) return;
        if (k->code == sf::Keyboard::Key::Up) {
            selected_ = (selected_ - 1 + static_cast<int>(items_.size())) % static_cast<int>(items_.size());
            rebuild();
        } else if (k->code == sf::Keyboard::Key::Down) {
            selected_ = (selected_ + 1) % static_cast<int>(items_.size());
            rebuild();
        } else if (k->code == sf::Keyboard::Key::Enter || k->code == sf::Keyboard::Key::Space) {
            confirmFlag_ = true;
        }
    } else if (ev.is<sf::Event::MouseMoved>()) {
        // Some builds don't expose x/y on the variant event; use current mouse position in window
        sf::Vector2i pix = sf::Mouse::getPosition(window);
        sf::Vector2f mpf = window.mapPixelToCoords(pix);
        for (size_t i = 0; i < items_.size(); ++i) {
            if (!items_[i].getString().isEmpty() && items_[i].getGlobalBounds().contains(mpf)) {
                if (static_cast<int>(i) != selected_) {
                    selected_ = static_cast<int>(i);
                    rebuild();
                }
                break;
            }
        }
    } else if (ev.is<sf::Event::MouseButtonPressed>()) {
        auto mb = ev.getIf<sf::Event::MouseButtonPressed>();
        if (!mb) return;
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2i pix = sf::Mouse::getPosition(window);
            sf::Vector2f mpf = window.mapPixelToCoords(pix);
            for (size_t i = 0; i < items_.size(); ++i) {
                if (!items_[i].getString().isEmpty() && items_[i].getGlobalBounds().contains(mpf)) {
                    selected_ = static_cast<int>(i);
                    confirmFlag_ = true;
                    rebuild();
                    break;
                }
            }
        }
    } else if (ev.is<sf::Event::Resized>()) {
        // nothing here; draw will re-scale the background according to window size
    }
}

void Menu::update(float dt) {
    (void)dt;
    // no animations for now
}

void Menu::draw(sf::RenderWindow& window) const {
    // Draw background if present, scaled to window preserving aspect and centered
    if (bgTex_ && bgSprite_) {
        sf::Vector2u ts = bgTex_->getSize();
        if (ts.x > 0 && ts.y > 0) {
            float ww = static_cast<float>(window.getSize().x);
            float wh = static_cast<float>(window.getSize().y);
            float sx = ww / static_cast<float>(ts.x);
            float sy = wh / static_cast<float>(ts.y);
            float scale = std::max(sx, sy);
            bgSprite_->setScale(sf::Vector2f(scale, scale));
            sf::FloatRect b = bgSprite_->getGlobalBounds();
            bgSprite_->setPosition(sf::Vector2f((ww - b.size.x) / 2.f, (wh - b.size.y) / 2.f));
            window.draw(*bgSprite_);
        }
    } else {
        // dark background fallback
        // Use mapPixelToCoords so the rectangle covers the full window regardless of the active view or viewport.
        sf::Vector2i topLeftPixel{0, 0};
        sf::Vector2i bottomRightPixel{ static_cast<int>(window.getSize().x), static_cast<int>(window.getSize().y) };
        sf::Vector2f topLeft = window.mapPixelToCoords(topLeftPixel);
        sf::Vector2f bottomRight = window.mapPixelToCoords(bottomRightPixel);
        sf::RectangleShape rect(bottomRight - topLeft);
        rect.setPosition(topLeft);
        rect.setFillColor(sf::Color::Black);
        window.draw(rect);
    }

    // Draw items
    for (size_t i = 0; i < items_.size(); ++i) {
        if (!items_[i].getString().isEmpty()) window.draw(items_[i]);
    }

    // Draw pointer triangle next to selected text
    if (!items_.empty() && !items_[selected_].getString().isEmpty()) {
        const sf::Text& t = items_[selected_];
        sf::FloatRect tb = t.getGlobalBounds();
        sf::Vector2f ppos(tb.position.x + pointerOffsetX_, tb.position.y + tb.size.y * 0.5f);
        sf::ConvexShape ptr = pointer_;
        ptr.setFillColor(colorSelected_);
        ptr.setPosition(ppos);
        window.draw(ptr);
    }
}

bool Menu::consumeConfirm() {
    if (confirmFlag_) {
        confirmFlag_ = false;
        return true;
    }
    return false;
}

int Menu::getSelectedIndex() const {
    return selected_;
}

void Menu::rebuild() {
    if (items_.empty()) return;
    float totalH = static_cast<float>(items_.size() - 1) * spacing_;
    float startY = center_.y - totalH * 0.5f;
    for (size_t i = 0; i < items_.size(); ++i) {
        sf::Text& t = items_[i];
        if (static_cast<int>(i) == selected_) t.setFillColor(colorSelected_);
        else t.setFillColor(colorNormal_);
        sf::FloatRect r = t.getLocalBounds();
        float originX = std::floor(r.position.x + r.size.x * 0.5f);
        float originY = std::floor(r.position.y + r.size.y * 0.5f);
        t.setOrigin(sf::Vector2f(originX, originY));
        t.setPosition(sf::Vector2f(center_.x, startY + static_cast<float>(i) * spacing_));
    }
}