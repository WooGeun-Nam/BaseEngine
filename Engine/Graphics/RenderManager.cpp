#include "Graphics/RenderManager.h"
#include "Graphics/RenderTexture.h"  // 추가
#include "Graphics/Camera2D.h"
#include "Graphics/DebugRenderer.h"
#include "Core/GameObject.h"
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
}

// Canvas가 관리하는 UI GameObject만 순회
void RenderManager::BeginUI()
{
    if (!spriteBatch)
        return;

    // UI 렌더링은 카메라 위치 적용 X
    spriteBatch->Begin(SpriteSortMode_BackToFront);
}

void RenderManager::EndUI()
{
    if (!spriteBatch)
        return;

    spriteBatch->End();
}

void RenderManager::BeginDebug()
{
    // DebugRenderer 사용
    DebugRenderer::Instance().Begin(screenWidth, screenHeight);
}

void RenderManager::EndDebug()
{
    // DebugRenderer 사용
    DebugRenderer::Instance().End();
}

// === RenderTexture 지원 함수 ===
void RenderManager::BeginSceneRender(RenderTexture* renderTexture)
{
    if (!renderTexture || !context)
        return;

    isRenderingToTexture = true;

    // 현재 렌더 타겟 저장
    UINT numViewports = 1;
    context->OMGetRenderTargets(1, &savedRenderTarget, &savedDepthStencil);
    context->RSGetViewports(&numViewports, &savedViewport);

    // 현재 화면 크기 저장 및 RenderTexture 크기로 업데이트
    int savedWidth = screenWidth;
    int savedHeight = screenHeight;
    
    screenWidth = renderTexture->GetWidth();
    screenHeight = renderTexture->GetHeight();

    // RenderTexture를 렌더 타겟으로 설정
    renderTexture->SetAsRenderTarget(context);
    
    // 클리어
    float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    renderTexture->ClearRenderTarget(context, clearColor);
}

void RenderManager::EndSceneRender()
{
    if (!isRenderingToTexture)
        return;

    // 저장된 렌더 타겟 복원
    RestoreBackBuffer();
    isRenderingToTexture = false;
}

void RenderManager::RestoreBackBuffer()
{
    if (!context)
        return;

    // 백버퍼로 복원
    context->OMSetRenderTargets(1, &savedRenderTarget, savedDepthStencil);
    context->RSSetViewports(1, &savedViewport);
    
    // 화면 크기를 원래 크기로 복원
    screenWidth = static_cast<int>(savedViewport.Width);
    screenHeight = static_cast<int>(savedViewport.Height);

    // Release COM 참조
    if (savedRenderTarget)
    {
        savedRenderTarget->Release();
        savedRenderTarget = nullptr;
    }
    if (savedDepthStencil)
    {
        savedDepthStencil->Release();
        savedDepthStencil = nullptr;
    }
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
