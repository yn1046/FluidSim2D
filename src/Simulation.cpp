#include "Simulation.h"
#include <cmath>
#include <iostream>
#include <numbers>

// Константы
constexpr float GRAVITY = 9.81f; // Ускорение свободного падения
constexpr float BOUNDARY_DAMPING = 0.5f; // Затухание при столкновении с границами
constexpr float PARTICLE_RADIUS = 5.0f; // Радиус частицы (для отрисовки и столкновений)
constexpr float KERNEL_RADIUS = 20.0f; // Радиус сглаживания (для SPH)
constexpr float REST_DENSITY = 1000.0f; // Плотность в состоянии покоя (например, вода)
constexpr float PRESSURE_CONSTANT = 100.0f; // Константа для расчёта давления
constexpr float VISCOSITY_CONSTANT = 0.1f; // Константа для вязкости

// Вспомогательная функция для ядра сглаживания (Spiky Kernel)
float kernel(float distance, float h) {
    if (distance >= h) return 0.0f;
    float volume = (std::numbers::pi * std::pow(h, 5)) / 6.0f;
    return std::pow(h - distance, 3) / volume;
}

// Вспомогательная функция для градиента ядра (Spiky Kernel Gradient)
sf::Vector2f kernelGradient(sf::Vector2f r, float h) {
    float distance = std::hypot(r.x, r.y);
    if (distance >= h || distance == 0.0f) return { 0.0f, 0.0f };
    float scale = -3.0f * std::pow(h - distance, 2) / (std::numbers::pi * std::pow(h, 5));
    return { r.x * scale / distance, r.y * scale / distance };
}

// Реализация Uniform Grid
Simulation::Grid::Grid(int w, int h, float size) : width(w), height(h), cellSize(size) {
    int numCellsX = static_cast<int>(std::ceil(width / cellSize));
    int numCellsY = static_cast<int>(std::ceil(height / cellSize));
    cells.resize(numCellsX * numCellsY);
}

int Simulation::Grid::getCellIndex(sf::Vector2f pos) const {
    int x = static_cast<int>(pos.x / cellSize);
    int y = static_cast<int>(pos.y / cellSize);
    int numCellsX = static_cast<int>(std::ceil(width / cellSize));
    int numCellsY = static_cast<int>(std::ceil(height / cellSize));

    // Ограничиваем индексы в пределах сетки
    x = std::max(0, std::min(x, numCellsX - 1));
    y = std::max(0, std::min(y, numCellsY - 1));

    return y * numCellsX + x;
}

void Simulation::Grid::addParticle(int particleIndex, sf::Vector2f pos) {
    cells[getCellIndex(pos)].push_back(particleIndex);
}

void Simulation::Grid::clear() {
    for (auto& cell : cells) cell.clear();
}

std::vector<int> Simulation::Grid::getNeighbors(sf::Vector2f pos) const {
    std::vector<int> neighbors;
    int x = static_cast<int>(pos.x / cellSize);
    int y = static_cast<int>(pos.y / cellSize);
    int numCellsX = static_cast<int>(std::ceil(width / cellSize));
    int numCellsY = static_cast<int>(std::ceil(height / cellSize));

    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int cellX = x + i;
            int cellY = y + j;

            // Проверяем, что ячейка находится в пределах сетки
            if (cellX >= 0 && cellX < numCellsX && cellY >= 0 && cellY < numCellsY) {
                int cellIndex = cellY * numCellsX + cellX;
                for (int particleIndex : cells[cellIndex]) {
                    neighbors.push_back(particleIndex);
                }
            }
        }
    }

    return neighbors;
}

// Конструктор симуляции
Simulation::Simulation() : grid(1024, 768, KERNEL_RADIUS) {}

void Simulation::update(float dt, bool isLeftMousePressed, sf::Vector2f mousePosition) {
    if (isLeftMousePressed) {
        // Создаём частицы в круге вокруг позиции курсора
        for (int i = 0; i < MAX_PARTICLES_PER_FRAME; ++i) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2 * std::numbers::pi; // Случайный угол
            float radius = static_cast<float>(rand()) / RAND_MAX * SPAWN_RADIUS; // Случайный радиус
            sf::Vector2f offset(radius * std::cos(angle), radius * std::sin(angle));
            sf::Vector2f spawnPosition = mousePosition + offset;

            Particle p;
            p.position = spawnPosition;
            p.velocity = { 0.0f, 100.0f }; // Начальная скорость вниз
            p.color = sf::Color::Blue; // Цвет частиц
            particles.push_back(p);
        }
    }

    updateGrid();
    updateDensity();
    updateForces(dt);
    integrate(dt);
    handleBoundaryCollisions();
}

const std::vector<Particle>& Simulation::getParticles() const {
    return particles;
}

void Simulation::spawnParticles(sf::Vector2f position, sf::Color color) {
    // Ограничиваем позицию в пределах окна
    position.x = std::max(0.0f, std::min(position.x, 1024.0f));
    position.y = std::max(0.0f, std::min(position.y, 768.0f));

    for (int i = 0; i < 10; ++i) {
        Particle p;
        // Добавляем небольшое случайное смещение по горизонтали
        p.position.x = position.x + (rand() % 10 - 5); // Смещение от -5 до +5
        p.position.y = position.y;
        // Начальная скорость вниз
        p.velocity = { 0.0f, 100.0f }; // Скорость вниз
        p.color = color;
        particles.push_back(p);
    }
}

void Simulation::updateGrid() {
    grid.clear();
    for (int i = 0; i < particles.size(); ++i) {
        // Ограничиваем позиции частиц в пределах окна
        particles[i].position.x = std::max(0.0f, std::min(particles[i].position.x, 1024.0f));
        particles[i].position.y = std::max(0.0f, std::min(particles[i].position.y, 768.0f));
        grid.addParticle(i, particles[i].position);
    }
}

void Simulation::updateDensity() {
    if (particles.empty()) return;

    for (int i = 0; i < particles.size(); ++i) {
        auto& p = particles[i];
        p.density = 0.0f;

        auto neighbors = grid.getNeighbors(p.position);
        for (int neighborIndex : neighbors) {
            const auto& neighbor = particles[neighborIndex];
            float distance = std::hypot(p.position.x - neighbor.position.x, p.position.y - neighbor.position.y);
            if (distance < KERNEL_RADIUS) {
                p.density += kernel(distance, KERNEL_RADIUS);
            }
        }
    }
}

void Simulation::updateForces(float dt) {
    for (int i = 0; i < particles.size(); ++i) {
        auto& p = particles[i];
        sf::Vector2f pressureForce = { 0.0f, 0.0f };
        sf::Vector2f viscosityForce = { 0.0f, 0.0f };

        auto neighbors = grid.getNeighbors(p.position);
        for (int neighborIndex : neighbors) {
            if (neighborIndex == i) continue; // Не учитываем саму частицу

            const auto& neighbor = particles[neighborIndex];
            sf::Vector2f r = p.position - neighbor.position;
            float distance = std::hypot(r.x, r.y);

            if (distance < KERNEL_RADIUS) {
                // Давление
                float pressure = PRESSURE_CONSTANT * (p.density + neighbor.density - 2 * REST_DENSITY);
                pressureForce += kernelGradient(r, KERNEL_RADIUS) * pressure;

                // Вязкость
                viscosityForce += (neighbor.velocity - p.velocity) * VISCOSITY_CONSTANT * kernel(distance, KERNEL_RADIUS);
            }
        }

        // Гравитация
        sf::Vector2f gravityForce = { 0.0f, GRAVITY * p.density };

        // Общая сила
        sf::Vector2f totalForce = pressureForce + viscosityForce + gravityForce;

        // Обновление скорости
        p.velocity += totalForce * dt;
    }
}

void Simulation::integrate(float dt) {
    for (auto& p : particles) {
        p.position += p.velocity * dt;
    }
}

void Simulation::handleBoundaryCollisions() {
    for (auto& p : particles) {
        // Ограничиваем позиции частиц в пределах окна
        p.position.x = std::max(0.0f, std::min(p.position.x, 1024.0f));
        p.position.y = std::max(0.0f, std::min(p.position.y, 768.0f));

        // Столкновение с левой границей
        if (p.position.x < PARTICLE_RADIUS) {
            p.position.x = PARTICLE_RADIUS;
            p.velocity.x *= -BOUNDARY_DAMPING;
        }
        // Столкновение с правой границей
        if (p.position.x > 1024 - PARTICLE_RADIUS) {
            p.position.x = 1024 - PARTICLE_RADIUS;
            p.velocity.x *= -BOUNDARY_DAMPING;
        }
        // Столкновение с верхней границей
        if (p.position.y < PARTICLE_RADIUS) {
            p.position.y = PARTICLE_RADIUS;
            p.velocity.y *= -BOUNDARY_DAMPING;
        }
        // Столкновение с нижней границей
        if (p.position.y > 768 - PARTICLE_RADIUS) {
            p.position.y = 768 - PARTICLE_RADIUS;
            p.velocity.y *= -BOUNDARY_DAMPING;
        }
    }
}