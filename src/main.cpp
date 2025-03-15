#include <SFML/Graphics.hpp>
#include "Simulation.h"
#include "Renderer.h"

int main() {
    sf::RenderWindow window(sf::VideoMode({ 1024, 768 }), "SPH Simulation");
    window.setFramerateLimit(60);

    Simulation simulation;
    Renderer renderer(window);

    sf::Color currentColor = sf::Color::Blue; // Цвет частиц
    bool isLeftMousePressed = false; // Состояние ЛКМ

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto *keyPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (keyPressed->button == sf::Mouse::Button::Left) {
                    isLeftMousePressed = true; // ЛКМ зажата
                }
            }
            if (const auto* keyPressed = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (keyPressed->button == sf::Mouse::Button::Left) {
                    isLeftMousePressed = false; // ЛКМ отпущена
                }
            }
        }

        // Получаем позицию курсора
        sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // Обновляем симуляцию
        simulation.update(1.0f / 60.0f, isLeftMousePressed, mousePosition);

        // Отрисовка
        window.clear();
        renderer.render(simulation.getParticles());
        window.display();
    }

    return 0;
}