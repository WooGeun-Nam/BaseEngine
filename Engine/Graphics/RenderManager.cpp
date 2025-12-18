#include "Graphics/RenderManager.h"
#include "Graphics/Camera2D.h"

bool RenderManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    this->context = context;
    
    // 단일 SpriteBatch 생성
    spriteBatch = std::make_unique<SpriteBatch>(context);
    
    return true;
}

void RenderManager::BeginFrame()
{
    if (!spriteBatch)
        return;

    // SpriteSortMode_BackToFront: layer depth 기반 정렬 (낮은 값이 먼저)
    // 카메라가 있으면 view matrix 적용 (Game 레이어용)
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
}

float RenderManager::GetLayerDepth(RenderLayer layer, float subDepth)
{
    // subDepth를 0~1 범위로 클램프
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
