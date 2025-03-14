#include <SFML/Graphics.hpp>
#include "Simulation.h"
#include "Renderer.h"

int main() {
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "SPH Simulation");
    window.setFramerateLimit(60);

    Simulation simulation;
    Renderer renderer(window);

    sf::Color currentColor = sf::Color::Green;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (event->is<sf::Event::MouseButtonPressed>()) {
                currentColor = sf::Color(rand() % 256, rand() % 256, rand() % 256);
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                simulation.spawnParticles(mousePosition, currentColor);
            }
        }

        simulation.update(1.0f / 60.0f);
        window.clear();
        renderer.render(simulation.getParticles());
        window.display();
    }

    return 0;
}