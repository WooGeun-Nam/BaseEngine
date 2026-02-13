#pragma once
#include <Windows.h>
#include "Graphics/D3DDevice.h"
#include "Input/Input.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "Core/SceneManager.h"
#include "Graphics/DebugRenderer.h"

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

    // Game View offset for UI mouse coordinate conversion
    void SetGameViewOffset(float x, float y) { gameViewOffsetX = x; gameViewOffsetY = y; }
    float GetGameViewOffsetX() const { return gameViewOffsetX; }
    float GetGameViewOffsetY() const { return gameViewOffsetY; }

    // Game View size for mouse coordinate scaling
    void SetGameViewSize(int w, int h) { gameViewWidth = w; gameViewHeight = h; }
    int GetGameViewWidth() const { return gameViewWidth; }
    int GetGameViewHeight() const { return gameViewHeight; }

private:
    HWND windowHandle = nullptr;

    SceneManager sceneManager;
    Input input;
    D3DDevice d3dDevice;
    ShaderManager shaderManager;

    int windowWidth;
    int windowHeight;

    float gameViewOffsetX = 0.0f;
    float gameViewOffsetY = 0.0f;

    int gameViewWidth = 1280;
    int gameViewHeight = 720;

    float clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.f };

    bool imguiInitialized;

    void InitializeImGui();
    void ShutdownImGui();
    void AutoCompileScripts();  // Auto-compile scripts on startup
};