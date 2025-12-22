#include "Graphics/RenderManager.h"
#include "Graphics/Camera2D.h"
#include "Graphics/DebugRenderer.h"
#include "Core/GameObject.h"
#include "UI/Canvas.h"
#include <SimpleMath.h>

RenderManager& RenderManager::Instance()
{
    static RenderManager instance;
    return instance;
}

void RenderManager::SetCamera(Camera2D* cam)
{
    camera = cam;
    
    // DebugRenderer에도 카메라 설정
    DebugRenderer::Instance().SetCamera(cam);
}

void RenderManager::SetCanvas(Canvas* c)
{
    canvas = c;
}

bool RenderManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight)
{
    this->device = device;
    this->context = context;
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    
    // SpriteBatch 생성
    spriteBatch = std::make_unique<SpriteBatch>(context);
    
    // DebugRenderer 초기화
    DebugRenderer::Instance().Initialize(device, context);
    
    return true;
}

void RenderManager::BeginFrame()
{
    if (!spriteBatch)
        return;

    // 카메라 view 행렬 적용
    if (camera)
    {
        XMMATRIX view = camera->GetViewMatrix();
        spriteBatch->Begin(SpriteSortMode_BackToFront, nullptr, nullptr,
                          nullptr, nullptr, nullptr, view);
    }
    else
    {
        spriteBatch->Begin(SpriteSortMode_BackToFront);
    }
}

void RenderManager::EndFrame()
{
    if (!spriteBatch)
        return;

    spriteBatch->End();

    // UI 자동 렌더링 (카메라 없음)
    RenderUI();
}

// Canvas가 관리하는 UI GameObject만 순회
void RenderManager::RenderUI()
{
    if (!canvas || !spriteBatch)
        return;

    // UI는 카메라 변환 없이 Screen Space로 렌더링
    spriteBatch->Begin(SpriteSortMode_BackToFront);
    
    // Canvas의 uiObjects 배열 순회 (계층 순서 보장)
    for (auto* obj : canvas->GetUIObjects())
    {
        obj->RenderUI();
    }
    
    spriteBatch->End();
}

void RenderManager::BeginDebug()
{
    // DebugRenderer로 위임
    DebugRenderer::Instance().Begin(screenWidth, screenHeight);
}

void RenderManager::EndDebug()
{
    // DebugRenderer로 위임
    DebugRenderer::Instance().End();
}

void RenderManager::SetScreenSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
}

float RenderManager::GetLayerDepth(RenderLayer layer, float subDepth)
{
    // subDepth는 0~1 범위로 클램핑
    if (subDepth < 0.0f) subDepth = 0.0f;
    if (subDepth > 1.0f) subDepth = 1.0f;

    // 각 레이어의 depth 범위
    const float layerRanges[][2] = {
        { 0.0f, 0.2f },  // Background
        { 0.2f, 0.5f },  // Game
        { 0.5f, 0.8f },  // UI
        { 0.8f, 1.0f }   // Debug
    };

    int layerIndex = static_cast<int>(layer);
    float minDepth = layerRanges[layerIndex][0];
    float maxDepth = layerRanges[layerIndex][1];

    // 선형 보간
    return minDepth + (maxDepth - minDepth) * subDepth;
}
