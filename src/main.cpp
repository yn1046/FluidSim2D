#include <SFML/Graphics.hpp>
#include "Simulation.h"
#include "Renderer.h"

int main() {
    sf::RenderWindow window(sf::VideoMode({ 1024, 768 }), "SPH Simulation");
    window.setFramerateLimit(60);

    Simulation simulation;
    Renderer renderer(window);

    sf::Color currentColor = sf::Color::Blue; // ���� ������
    bool isLeftMousePressed = false; // ��������� ���

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto *keyPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (keyPressed->button == sf::Mouse::Button::Left) {
                    isLeftMousePressed = true; // ��� ������
                }
            }
            if (const auto* keyPressed = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (keyPressed->button == sf::Mouse::Button::Left) {
                    isLeftMousePressed = false; // ��� ��������
                }
            }
        }

        // �������� ������� �������
        sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // ��������� ���������
        simulation.update(1.0f / 60.0f, isLeftMousePressed, mousePosition);

        // ���������
        window.clear();
        renderer.render(simulation.getParticles());
        window.display();
    }

    return 0;
}