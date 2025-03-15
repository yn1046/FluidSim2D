#ifndef PARTICLE_H
#define PARTICLE_H

#include <DirectXMath.h>

struct Particle {
    DirectX::XMFLOAT2 position;
    DirectX::XMFLOAT2 velocity;
    float density;
    float pressure;
};

#endif