#include "Graphics/RenderManager.h"
#include "Graphics/Camera2D.h"
#include "Graphics/DebugRenderer.h"
#include "UI/Canvas.h"

bool RenderManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight)
{
    this->device = device;
    this->context = context;
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    
    // 싱글톤 SpriteBatch 생성
    spriteBatch = std::make_unique<SpriteBatch>(context);
    
    return true;
}

void RenderManager::BeginFrame()
{
    if (!spriteBatch)
        return;

    // SpriteSortMode_BackToFront: layer depth 기준 정렬 (뒤에서부터 front)
    // 카메라 변환 view matrix 적용 (Game 레이어용)
    if (camera)
    {
        XMMATRIX view = camera->GetViewMatrix();
        spriteBatch->Begin(SpriteSortMode_BackToFront, nullptr, nullptr, nullptr, nullptr, nullptr, view);
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

    // EndFrame 후 UI 렌더링 (카메라 영향 없이)
    RenderCanvas();   // Canvas UI 렌더링
}

void RenderManager::RenderCanvas()
{
    if (!canvas || !spriteBatch)
        return;

    // ===== UI는 카메라 변환 없이 Screen Space로 렌더링 =====
    // 카메라 view matrix 없이 SpriteBatch 시작
    spriteBatch->Begin(SpriteSortMode_BackToFront);
    
    // Canvas의 RenderUI() 호출
    // Canvas는 자식 GameObject의 UIBase Component들을 순회하여 렌더링
    canvas->RenderUI();
    
    spriteBatch->End();
}

void RenderManager::BeginDebug()
{
    // DebugRenderer에 시작
    DebugRenderer::Instance().Begin(screenWidth, screenHeight);
}

void RenderManager::EndDebug()
{
    // DebugRenderer에 종료
    DebugRenderer::Instance().End();
}

void RenderManager::SetCamera(Camera2D* cam)
{
    camera = cam;
    
    // DebugRenderer에도 카메라 설정
    DebugRenderer::Instance().SetCamera(cam);
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
