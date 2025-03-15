#include "Simulation.h"
#include <d3dcompiler.h>
#include <iostream>
#include "d3dx12.h"

#pragma comment(lib, "d3dcompiler.lib")

Simulation::Simulation(ID3D12Device* device) : device(device) {
	// Создание Command Allocator
	if (FAILED(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator)
	))) {
		std::cerr << "Failed to create command allocator!" << std::endl;
		return;
	}

	CreateBuffers();
	CreateComputePipeline();
}

void Simulation::CreateBuffers() {
	// Создание буфера для частиц
	const UINT particleBufferSize = sizeof(Particle) * maxParticles;

	// Промежуточные переменные
	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(particleBufferSize);

	// Буфер для частиц на GPU
	if (FAILED(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&particleBuffer)
	))) {
		std::cerr << "Failed to create particle buffer!" << std::endl;
		return;
	}

	// Буфер для загрузки данных частиц на GPU
	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	if (FAILED(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&particleBufferUpload)
	))) {
		std::cerr << "Failed to create particle upload buffer!" << std::endl;
		return;
	}

	// Инициализация частиц
	std::vector<Particle> initialParticles(maxParticles);
	for (UINT i = 0; i < maxParticles; i++) {
		initialParticles[i].position = { 0.0f, 0.0f };
		initialParticles[i].velocity = { 0.0f, 0.0f };
		initialParticles[i].density = 0.0f;
		initialParticles[i].pressure = 0.0f;
	}

	// Копирование данных в буфер загрузки
	D3D12_SUBRESOURCE_DATA particleData = {};
	particleData.pData = initialParticles.data();
	particleData.RowPitch = particleBufferSize;
	particleData.SlicePitch = particleBufferSize;

	// Копирование данных из буфера загрузки в буфер на GPU
	ID3D12GraphicsCommandList* commandList = nullptr;
	if (FAILED(device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator,
		nullptr,
		IID_PPV_ARGS(&commandList)
	))) {
		std::cerr << "Failed to create command list!" << std::endl;
		return;
	}

	commandList->CopyBufferRegion(particleBuffer, 0, particleBufferUpload, 0, particleBufferSize);
	commandList->Close();


	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)))) {
		std::cerr << "Failed to create command queue!" << std::endl;
		return;
	}

	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists);

	// Ожидание завершения копирования
	ID3D12Fence* fence = nullptr;
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
		std::cerr << "Failed to create fence!" << std::endl;
		return;
	}

	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr) {
		std::cerr << "Failed to create fence event!" << std::endl;
		return;
	}

	if (FAILED(commandQueue->Signal(fence, 1))) {
		std::cerr << "Failed to signal fence!" << std::endl;
		return;
	}

	if (fence->GetCompletedValue() < 1) {
		if (FAILED(fence->SetEventOnCompletion(1, fenceEvent))) {
			std::cerr << "Failed to set event on completion!" << std::endl;
			return;
		}
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// Освобождение ресурсов
	CloseHandle(fenceEvent);
	if (fence) fence->Release();
	if (commandQueue) commandQueue->Release();
	if (commandList) commandList->Release();
}

void Simulation::CreateComputePipeline() {
	// Компиляция Compute Shader
	ID3DBlob* computeShaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	if (FAILED(D3DCompileFromFile(
		L"shaders/compute.hlsl",
		nullptr,
		nullptr,
		"main",
		"cs_5_0",
		0,
		0,
		&computeShaderBlob,
		&errorBlob
	))) {
		std::cerr << "Failed to compile compute shader!" << std::endl;
		if (errorBlob) {
			std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;
			errorBlob->Release();
		}
		return;
	}

	// Описание корневой сигнатуры
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

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
		IID_PPV_ARGS(&computeRootSignature)
	))) {
		std::cerr << "Failed to create compute root signature!" << std::endl;
		return;
	}

	// Описание Compute Pipeline
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc = {};
	computePipelineStateDesc.pRootSignature = computeRootSignature;
	computePipelineStateDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };

	if (FAILED(device->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineState)))) {
		std::cerr << "Failed to create compute pipeline state!" << std::endl;
		return;
	}

	// Освобождение ресурсов
	if (computeShaderBlob) computeShaderBlob->Release();
	if (rootSignatureBlob) rootSignatureBlob->Release();
}

void Simulation::Update(ID3D12GraphicsCommandList* commandList) {
	// Установка Compute Pipeline
	commandList->SetPipelineState(computePipelineState);
	commandList->SetComputeRootSignature(computeRootSignature);

	// Привязка буфера частиц
	commandList->SetComputeRootUnorderedAccessView(0, particleBuffer->GetGPUVirtualAddress());

	// Запуск Compute Shader
	commandList->Dispatch(maxParticles / 256, 1, 1);
}

const std::vector<Particle>& Simulation::GetParticles() const {
	// Чтение данных частиц из буфера (для отладки)
	// В реальном проекте это может быть не нужно
	std::vector<Particle> particles(maxParticles);
	D3D12_RANGE readRange = { 0, sizeof(Particle) * maxParticles };
	void* pData = nullptr;

	if (FAILED(particleBuffer->Map(0, &readRange, &pData))) {
		std::cerr << "Failed to map particle buffer!" << std::endl;
		return particles;
	}

	memcpy(particles.data(), pData, sizeof(Particle) * maxParticles);
	particleBuffer->Unmap(0, nullptr);

	return particles;
}

Simulation::~Simulation() {
	if (particleBuffer) particleBuffer->Release();
	if (particleBufferUpload) particleBufferUpload->Release();
	if (computeRootSignature) computeRootSignature->Release();
	if (computePipelineState) computePipelineState->Release();
	if (commandAllocator) commandAllocator->Release();
}