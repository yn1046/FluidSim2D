#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <iostream>
#include "Simulation.h"
#include "Renderer.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// ���������� ���������� ��� DirectX 12
ID3D12Device* device = nullptr;
IDXGIFactory4* factory = nullptr;
ID3D12CommandQueue* commandQueue = nullptr;
ID3D12CommandAllocator* commandAllocator = nullptr;
ID3D12GraphicsCommandList* commandList = nullptr;
Simulation* simulation = nullptr;
Renderer* renderer = nullptr;

// �������� ������� ��������� ���������
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ����������� ������ ����
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DirectXWindowClass";

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class!" << std::endl;
        return -1;
    }

    // �������� ����
    HWND hwnd = CreateWindowEx(
        0,
        L"DirectXWindowClass",
        L"DirectX 12 SPH",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1024, 768,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hwnd) {
        std::cerr << "Failed to create window!" << std::endl;
        return -1;
    }

    ShowWindow(hwnd, nCmdShow);

    // ������������� DirectX 12
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        std::cerr << "Failed to create DXGI factory!" << std::endl;
        return -1;
    }

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
        std::cerr << "Failed to create D3D12 device!" << std::endl;
        return -1;
    }

    // �������� Command Queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)))) {
        std::cerr << "Failed to create command queue!" << std::endl;
        return -1;
    }

    // �������� Command Allocator
    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)))) {
        std::cerr << "Failed to create command allocator!" << std::endl;
        return -1;
    }

    // �������� Command List
    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)))) {
        std::cerr << "Failed to create command list!" << std::endl;
        return -1;
    }

    // �������� Command List (�� ����� ������� ����� ��������������)
    commandList->Close();

    // ������������� ��������� � ���������
    simulation = new Simulation(device);
    renderer = new Renderer(device);

    // �������� ����
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // ����� Command Allocator � Command List
            commandAllocator->Reset();
            commandList->Reset(commandAllocator, nullptr);

            // ���������� ���������
            simulation->Update(commandList);

            // ���������
            renderer->Render(commandList, simulation->GetParticles());

            // �������� Command List
            commandList->Close();

            // ���������� ������
            ID3D12CommandList* commandLists[] = { commandList };
            commandQueue->ExecuteCommandLists(1, commandLists);

            // �������� ���������� ���������� ������
            // (����� ����� �������� �������������, ��������, � �������������� Fence)
        }
    }

    // ������������ ��������
    delete simulation;
    delete renderer;

    if (commandList) commandList->Release();
    if (commandAllocator) commandAllocator->Release();
    if (commandQueue) commandQueue->Release();
    if (device) device->Release();
    if (factory) factory->Release();

    return 0;
}

// ������� ��������� ���������
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // ��������� (���� �����)
        EndPaint(hwnd, &ps);
        return 0;
    }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}