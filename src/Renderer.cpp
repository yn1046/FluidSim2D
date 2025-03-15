#include "Renderer.h"
#include <d3dcompiler.h>
#include <iostream>
#include "d3dx12.h"

#pragma comment(lib, "d3dcompiler.lib")

Renderer::Renderer(ID3D12Device* device) : device(device) {
    CreatePipeline();
    CreateVertexBuffer();
}

void Renderer::CreatePipeline() {
    // Компиляция шейдеров
    ID3DBlob* vertexShaderBlob = nullptr;
    ID3DBlob* pixelShaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    // Компиляция вершинного шейдера
    if (FAILED(D3DCompileFromFile(
        L"shaders/vertex.hlsl",
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        0,
        0,
        &vertexShaderBlob,
        &errorBlob
    ))) {
        std::cerr << "Failed to compile vertex shader!" << std::endl;
        if (errorBlob) {
            std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        return;
    }

    // Компиляция пиксельного шейдера
    if (FAILED(D3DCompileFromFile(
        L"shaders/pixel.hlsl",
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &pixelShaderBlob,
        &errorBlob
    ))) {
        std::cerr << "Failed to compile pixel shader!" << std::endl;
        if (errorBlob) {
            std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        return;
    }

    // Описание корневой сигнатуры
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* rootSignatureBlob = nullptr;
    if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob))) {
        std::cerr << "Failed to serialize root signature!" << std::endl;
        if (errorBlob) {
            std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        return;
    }

    if (FAILED(device->CreateRootSignature(
        0,
        rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)
    ))) {
        std::cerr << "Failed to create root signature!" << std::endl;
        return;
    }

    // Описание входного слоя (Input Layout)
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Инициализация Rasterizer State
    D3D12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    // Инициализация Blend State
    D3D12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    // Описание Graphics Pipeline
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
    pipelineStateDesc.pRootSignature = rootSignature;
    pipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    pipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    pipelineStateDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    pipelineStateDesc.RasterizerState = rasterizerDesc;
    pipelineStateDesc.BlendState = blendDesc;
    pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
    pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
    pipelineStateDesc.SampleMask = UINT_MAX;
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateDesc.SampleDesc.Count = 1;

    if (FAILED(device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState)))) {
        std::cerr << "Failed to create graphics pipeline state!" << std::endl;
        return;
    }

    // Освобождение ресурсов
    if (vertexShaderBlob) vertexShaderBlob->Release();
    if (pixelShaderBlob) pixelShaderBlob->Release();
    if (rootSignatureBlob) rootSignatureBlob->Release();
}

void Renderer::CreateVertexBuffer() {
    // Создание вершинного буфера
    const UINT vertexBufferSize = sizeof(Particle) * maxParticles;

    // Промежуточные переменные
    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    if (FAILED(device->CreateCommittedResource(
        &heapProperties, // Используем промежуточную переменную
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc, // Используем промежуточную переменную
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)
    ))) {
        std::cerr << "Failed to create vertex buffer!" << std::endl;
        return;
    }

    // Отображение вершинного буфера в CPU-память
    if (FAILED(vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexBufferData)))) {
        std::cerr << "Failed to map vertex buffer!" << std::endl;
        return;
    }
}

void Renderer::Render(ID3D12GraphicsCommandList* commandList, const std::vector<Particle>& particles) {
    // Копирование данных частиц в вершинный буфер
    memcpy(vertexBufferData, particles.data(), particles.size() * sizeof(Particle));

    // Установка Graphics Pipeline
    commandList->SetPipelineState(pipelineState);
    commandList->SetGraphicsRootSignature(rootSignature);

    // Установка вершинного буфера
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Particle);
    vertexBufferView.SizeInBytes = static_cast<UINT>(particles.size() * sizeof(Particle));

    // Используем промежуточную переменную
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // Установка топологии
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

    // Отрисовка частиц
    commandList->DrawInstanced(static_cast<UINT>(particles.size()), 1, 0, 0);
}


Renderer::~Renderer() {
    if (vertexBuffer) vertexBuffer->Release();
    if (pipelineState) pipelineState->Release();
    if (rootSignature) rootSignature->Release();
}