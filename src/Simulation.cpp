#include "Simulation.h"
#include <cmath>
#include <iostream>
#include <numbers>

// ���������
constexpr float GRAVITY = 9.81f; // ��������� ���������� �������
constexpr float BOUNDARY_DAMPING = 0.5f; // ��������� ��� ������������ � ���������
constexpr float PARTICLE_RADIUS = 5.0f; // ������ ������� (��� ��������� � ������������)
constexpr float KERNEL_RADIUS = 20.0f; // ������ ����������� (��� SPH)
constexpr float REST_DENSITY = 1000.0f; // ��������� � ��������� ����� (��������, ����)
constexpr float PRESSURE_CONSTANT = 100.0f; // ��������� ��� ������� ��������
constexpr float VISCOSITY_CONSTANT = 0.1f; // ��������� ��� ��������

// ��������������� ������� ��� ���� ����������� (Spiky Kernel)
float kernel(float distance, float h) {
    if (distance >= h) return 0.0f;
    float volume = (std::numbers::pi * std::pow(h, 5)) / 6.0f;
    return std::pow(h - distance, 3) / volume;
}

// ��������������� ������� ��� ��������� ���� (Spiky Kernel Gradient)
sf::Vector2f kernelGradient(sf::Vector2f r, float h) {
    float distance = std::hypot(r.x, r.y);
    if (distance >= h || distance == 0.0f) return { 0.0f, 0.0f };
    float scale = -3.0f * std::pow(h - distance, 2) / (std::numbers::pi * std::pow(h, 5));
    return { r.x * scale / distance, r.y * scale / distance };
}

Simulation::Simulation() {
    // ������������� ���������
}

void Simulation::update(float dt) {
    updateDensity();
    updateForces(dt);
    integrate(dt);
    handleBoundaryCollisions();
}

const std::vector<Particle>& Simulation::getParticles() const {
    return particles;
}

void Simulation::spawnParticles(sf::Vector2f position, sf::Color color) {
    for (int i = 0; i < 10; ++i) {
        Particle p;
        p.position = position;
        p.velocity = { 0, 0 };
        p.color = color;
        particles.push_back(p);
    }
}

void Simulation::updateDensity() {
    for (auto& p : particles) {
        p.density = 0.0f;
        for (const auto& neighbor : particles) {
            float distance = std::hypot(p.position.x - neighbor.position.x, p.position.y - neighbor.position.y);
            if (distance < KERNEL_RADIUS) {
                p.density += kernel(distance, KERNEL_RADIUS);
            }
        }
    }
}

void Simulation::updateForces(float dt) {
    for (auto& p : particles) {
        sf::Vector2f pressureForce = { 0.0f, 0.0f };
        sf::Vector2f viscosityForce = { 0.0f, 0.0f };

        for (const auto& neighbor : particles) {
            if (&p == &neighbor) continue; // �� ��������� ���� �������

            sf::Vector2f r = p.position - neighbor.position;
            float distance = std::hypot(r.x, r.y);

            if (distance < KERNEL_RADIUS) {
                // ��������
                float pressure = PRESSURE_CONSTANT * (p.density + neighbor.density - 2 * REST_DENSITY);
                pressureForce += kernelGradient(r, KERNEL_RADIUS) * pressure;

                // ��������
                viscosityForce += (neighbor.velocity - p.velocity) * VISCOSITY_CONSTANT * kernel(distance, KERNEL_RADIUS);
            }
        }

        // ����������
        sf::Vector2f gravityForce = { 0.0f, GRAVITY * p.density };

        // ����� ����
        sf::Vector2f totalForce = pressureForce + viscosityForce + gravityForce;

        // ���������� ��������
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
        // ������������ � ����� ��������
        if (p.position.x < PARTICLE_RADIUS) {
            p.position.x = PARTICLE_RADIUS;
            p.velocity.x *= -BOUNDARY_DAMPING;
        }
        // ������������ � ������ ��������
        if (p.position.x > 1024 - PARTICLE_RADIUS) {
            p.position.x = 1024 - PARTICLE_RADIUS;
            p.velocity.x *= -BOUNDARY_DAMPING;
        }
        // ������������ � ������� ��������
        if (p.position.y < PARTICLE_RADIUS) {
            p.position.y = PARTICLE_RADIUS;
            p.velocity.y *= -BOUNDARY_DAMPING;
        }
        // ������������ � ������ ��������
        if (p.position.y > 768 - PARTICLE_RADIUS) {
            p.position.y = 768 - PARTICLE_RADIUS;
            p.velocity.y *= -BOUNDARY_DAMPING;
        }
    }
}