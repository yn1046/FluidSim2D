#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "Particle.h"

class Simulation {
public:
    Simulation();
    void update(float dt, bool isLeftMousePressed, sf::Vector2f mousePosition); // ��������� ��������� ��� ����
    const std::vector<Particle>& getParticles() const;
    void spawnParticles(sf::Vector2f position, sf::Color color); // �������, ��� ��� ������ ���������

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

    // ��������� ��� ������ ������
    const float SPAWN_RADIUS = 10.0f; // ������ ����� ��� ������ ������
    const int MAX_PARTICLES_PER_FRAME = 5; // ������������ ���������� ������ �� ����
};

#endif