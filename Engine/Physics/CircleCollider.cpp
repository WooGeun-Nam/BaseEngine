#include "CircleCollider.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include "Physics/BoxCollider2D.h"
#include "Graphics/SpriteRenderDevice.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/SpriteRenderer.h"
#include "Resource/Texture.h"
#include <d3d11.h>
#include <algorithm>

using namespace DirectX;

void CircleCollider::FitToTexture()
{
    if (!gameObject)
        return;

    // SpriteRenderer 찾기
    auto* spriteRenderer = gameObject->GetComponent<SpriteRenderer>();
    if (!spriteRenderer)
        return;

    // Texture 가져오기
    auto texture = spriteRenderer->GetTexture();
    if (!texture)
        return;

    // SRV에서 텍스처 정보 가져오기
    ID3D11ShaderResourceView* srv = texture->GetSRV();
    if (!srv)
        return;

    ID3D11Resource* resource = nullptr;
    srv->GetResource(&resource);
    
    if (resource)
    {
        ID3D11Texture2D* texture2D = nullptr;
        resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture2D);
        
        if (texture2D)
        {
            D3D11_TEXTURE2D_DESC desc;
            texture2D->GetDesc(&desc);
            
            // radius = 텍스처 크기의 절반 (더 작은 쪽 기준)
            radius = (std::min)(desc.Width, desc.Height) * 0.5f;
            
            texture2D->Release();
        }
        
        resource->Release();
    }
}

float CircleCollider::GetWorldRadius() const
{
    float scaleX = gameObject->transform.GetScale().x;
    return radius * scaleX;
}

XMFLOAT2 CircleCollider::GetWorldCenter() const
{
    return gameObject->transform.GetPosition();
}

bool CircleCollider::Intersects(BaseCollider* other)
{
    if (!enabled || !other->IsEnabled()) return false;

    // Circle ↔ Circle
    if (auto c = dynamic_cast<CircleCollider*>(other))
    {
        XMFLOAT2 a = GetWorldCenter();
        XMFLOAT2 b = c->GetWorldCenter();

        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dist2 = dx * dx + dy * dy;

        float r = GetWorldRadius() + c->GetWorldRadius();
        return dist2 <= (r * r);
    }

    // Circle ↔ Box(OBB)
    if (auto b = dynamic_cast<BoxCollider2D*>(other))
        return b->Intersects(this);

    return false;
}

void CircleCollider::DebugDraw()
{
    if (!enabled) return;

    // DebugRenderer의 DrawCircle을 호출합니다.
    DebugRenderer::Instance().DrawCircle(
        GetWorldCenter(),
        GetWorldRadius(),
        { 0,1,0,1 }
    );
}