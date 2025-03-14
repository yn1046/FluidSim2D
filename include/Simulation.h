#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include "Particle.h"

class Simulation {
public:
    Simulation();
    void update(float dt); // ���������� ��������� ������
    const std::vector<Particle>& getParticles() const; // ��������� ������ ��� ����������
    void spawnParticles(sf::Vector2f position, sf::Color color); // ��������� ������

private:
    std::vector<Particle> particles;
    void updateDensity();
    void updateForces(float dt);
    void integrate(float dt);
    void handleBoundaryCollisions();
};

#endif