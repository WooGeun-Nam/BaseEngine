#pragma once
#include <Windows.h>
#include "Graphics/D3DDevice.h"
#include "Input/Input.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "Core/SceneManager.h"
#include "Graphics/DebugRenderer.h"

class SpriteImporterWindow;

class Application
{
public:
    Application();
    ~Application();

    bool initialize(HWND windowHandle, int width, int height);
    void run();

    SceneManager& GetSceneManager() { return sceneManager; }
    Input& GetInput() { return input; }
    ID3D11Device* GetDevice() { return d3dDevice.getDevice(); }
    ID3D11DeviceContext* GetContext() { return d3dDevice.getContext(); }
    ShaderManager& GetShaderManager() { return shaderManager; }

    int GetWindowWidth() const { return windowWidth; }
    int GetWindowHeight() const { return windowHeight; }

private:
    HWND windowHandle = nullptr;

    SceneManager sceneManager;
    Input input;
    D3DDevice d3dDevice;
    ShaderManager shaderManager;

    int windowWidth;
    int windowHeight;

    float clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.f };

    // ImGui µµ±¸ Ã¢
    SpriteImporterWindow* spriteImporterWindow;
    bool imguiInitialized;

    void InitializeImGui();
    void ShutdownImGui();
};