#ifndef RENDERER_H
#define RENDERER_H

#include <d3d12.h>
#include <vector>
#include "Particle.h"

class Renderer {
public:
    Renderer(ID3D12Device* device);
    ~Renderer();

    void Render(ID3D12GraphicsCommandList* commandList, const std::vector<Particle>& particles);

private:
    void CreatePipeline();
    void CreateVertexBuffer();

    ID3D12Device* device;
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    ID3D12Resource* vertexBuffer = nullptr;
    UINT8* vertexBufferData = nullptr; // Указатель на данные вершинного буфера
    static constexpr UINT maxParticles = 10000; // Максимальное количество частиц
};

#endif