#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "Particle.h"

class Simulation {
public:
    Simulation();
    void update(float dt); // Обновление состояния частиц
    const std::vector<Particle>& getParticles() const; // Получение частиц для рендеринга
    void spawnParticles(sf::Vector2f position, sf::Color color); // Генерация частиц

private:
    std::vector<Particle> particles;

    // Uniform Grid
    struct Grid {
        int width, height;
        float cellSize;
        std::vector<std::vector<int>> cells;

        Grid(int w, int h, float size);
        int getCellIndex(sf::Vector2f pos) const;
        void addParticle(int particleIndex, sf::Vector2f pos);
        void clear();
        std::vector<int> getNeighbors(sf::Vector2f pos) const;
    };

    Grid grid;
    void updateGrid();
    void updateDensity();
    void updateForces(float dt);
    void integrate(float dt);
    void handleBoundaryCollisions();
};

#endif