#include "Renderer.h"

Renderer::Renderer(sf::RenderWindow& window) : window(window) {}

void Renderer::render(const std::vector<Particle>& particles) {
    for (const auto& p : particles) {
        sf::CircleShape circle(2); // Размер частицы
        circle.setPosition(p.position);
        circle.setFillColor(p.color);
        window.draw(circle);
    }
}