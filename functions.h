#pragma once
#include "librarys.h"



float len(const sf::Vector2f& v) {
    return sqrt(v.x*v.x + v.y*v.y);
}

float lenSquared(const sf::Vector2f& v) {
    return (v.x*v.x + v.y*v.y);
}

float dot(const sf::Vector2f& v1, const sf::Vector2f& v2) {
    return float(v1.x*v2.x + v1.y*v2.y);
}


sf::Vector2f normalize(const sf::Vector2f& v) {
    float length = len(v);
    if (length == 1) return v;
    else if (length < 1e-6) return sf::Vector2f(0, 0);
    else return v/length;
}


struct Line {
    public:
        sf::Vector2f posA, posB;
        sf::Color color;
        float lineWidth;

        Line(sf::Vector2f _posA, sf::Vector2f _posB, sf::Color _color = sf::Color::Red, float _lineWidth = 2.0f) : posA(_posA), posB(_posB), color(_color), lineWidth(_lineWidth) {}

        void render(sf::RenderWindow& window) {
            float dx = posA.x - posB.x;
            float dy = posA.y - posB.y;
            float length = len({dx, dy});
            float rotation = atan2(dy, dx);

            sf::RectangleShape obj;
            obj.setSize({length, lineWidth});
            obj.setOrigin(length, lineWidth/2.0f);
            obj.setRotation(rotation*180.0f/M_PI);
            obj.setPosition(posA);
            obj.setFillColor(color);

            window.draw(obj);
        }
};