#ifndef RENDERER_H
#define RENDERER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "Particle.h"

class Renderer {
public:
    Renderer(sf::RenderWindow& window);
    void render(const std::vector<Particle>& particles); // Отрисовка частиц

private:
    sf::RenderWindow& window;
};

#endif