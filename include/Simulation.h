#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include "Particle.h"

class Simulation {
public:
    Simulation();
    void update(float dt); // Обновление состояния частиц
    const std::vector<Particle>& getParticles() const; // Получение частиц для рендеринга
    void spawnParticles(sf::Vector2f position, sf::Color color); // Генерация частиц

private:
    std::vector<Particle> particles;
    void updateDensity();
    void updateForces(float dt);
    void integrate(float dt);
    void handleBoundaryCollisions();
};

#endif