#ifndef SIMULATION_H
#define SIMULATION_H

#include <d3d12.h>
#include <vector>
#include "Particle.h"

class Simulation {
public:
    Simulation(ID3D12Device* device);
    ~Simulation();

    void Update(ID3D12GraphicsCommandList* commandList);
    const std::vector<Particle>& GetParticles() const;

private:
    void CreateBuffers();
    void CreateComputePipeline();

    ID3D12Device* device;
    ID3D12RootSignature* computeRootSignature = nullptr;
    ID3D12PipelineState* computePipelineState = nullptr;
    ID3D12Resource* particleBuffer = nullptr; // Буфер для частиц на GPU
    ID3D12Resource* particleBufferUpload = nullptr; // Буфер для загрузки данных на GPU
    ID3D12CommandAllocator* commandAllocator = nullptr; // Command Allocator для копирования данных
    static constexpr UINT maxParticles = 10000; // Максимальное количество частиц
};

#endif