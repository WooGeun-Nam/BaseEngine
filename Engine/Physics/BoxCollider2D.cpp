#include "BoxCollider2D.h"
#include "CircleCollider.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include "Graphics/SpriteRenderDevice.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/SpriteRenderer.h"
#include "Resource/Texture.h"
#include <d3d11.h>

using namespace DirectX;

void BoxCollider2D::FitToTexture()
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
            
            // halfSize = 텍스처 크기 / 2
            halfSize.x = desc.Width * 0.5f;
            halfSize.y = desc.Height * 0.5f;
            
            texture2D->Release();
        }
        
        resource->Release();
    }
}

XMFLOAT2 BoxCollider2D::GetCenter() const
{
    return gameObject->transform.GetPosition();
}

float BoxCollider2D::GetRotation() const
{
    return gameObject->transform.GetRotation();
}

void BoxCollider2D::GetVertices(std::array<XMFLOAT2, 4>& out) const
{
    XMFLOAT2 pos = gameObject->transform.GetPosition();
    XMFLOAT2 scale = gameObject->transform.GetScale();
    float rot = gameObject->transform.GetRotation();

    float cosR = cosf(rot);
    float sinR = sinf(rot);

    // === 스케일 적용된 halfSize ===
    float hx = halfSize.x * scale.x;
    float hy = halfSize.y * scale.y;

    XMFLOAT2 pts[4] =
    {
        { -hx, -hy },
        {  hx, -hy },
        {  hx,  hy },
        { -hx,  hy }
    };

    for (int i = 0; i < 4; i++)
    {
        float x = pts[i].x;
        float y = pts[i].y;

        float wx = x * cosR - y * sinR + pos.x;
        float wy = x * sinR + y * cosR + pos.y;

        out[i] = { wx, wy };
    }
}

bool BoxCollider2D::Intersects(BaseCollider* other)
{
    if (!enabled || !other->IsEnabled())
        return false;

    if (auto box = dynamic_cast<BoxCollider2D*>(other))
        return IntersectsOBB(box);

    if (auto circle = dynamic_cast<CircleCollider*>(other))
        return IntersectsCircle(circle);

    return false;
}

static void ProjectOntoAxis(
    const std::array<XMFLOAT2, 4>& verts,
    XMFLOAT2 axis,
    float& outMin, float& outMax)
{
    float min = FLT_MAX;
    float max = -FLT_MAX;

    for (auto& v : verts)
    {
        float proj = v.x * axis.x + v.y * axis.y;
        min = (std::min)(min, proj);
        max = (std::max)(max, proj);
    }

    outMin = min;
    outMax = max;
}

bool BoxCollider2D::IntersectsOBB(BoxCollider2D* other)
{
    std::array<XMFLOAT2, 4> a, b;
    GetVertices(a);
    other->GetVertices(b);

    XMFLOAT2 axes[4] =
    {
        { a[1].x - a[0].x, a[1].y - a[0].y },
        { a[3].x - a[0].x, a[3].y - a[0].y },
        { b[1].x - b[0].x, b[1].y - b[0].y },
        { b[3].x - b[0].x, b[3].y - b[0].y }
    };

    for (auto& ax : axes)
    {
        float len = sqrtf(ax.x * ax.x + ax.y * ax.y);
        if (len < 0.0001f) continue;

        XMFLOAT2 axis = { ax.x / len, ax.y / len };

        float aMin, aMax, bMin, bMax;
        ProjectOntoAxis(a, axis, aMin, aMax);
        ProjectOntoAxis(b, axis, bMin, bMax);

        if (aMax < bMin || bMax < aMin)
            return false;
    }

    return true;
}

bool BoxCollider2D::IntersectsCircle(CircleCollider* circle)
{
    std::array<XMFLOAT2, 4> verts;
    GetVertices(verts);

    XMFLOAT2 c = circle->GetWorldCenter();
    float r = circle->GetWorldRadius();

    float minX = verts[0].x, maxX = verts[0].x;
    float minY = verts[0].y, maxY = verts[0].y;

    for (int i = 1; i < 4; i++)
    {
        minX = (std::min)(minX, verts[i].x);
        maxX = (std::max)(maxX, verts[i].x);
        minY = (std::min)(minY, verts[i].y);
        maxY = (std::max)(maxY, verts[i].y);
    }

    float clampedX = (std::max)(minX, (std::min)(c.x, maxX));
    float clampedY = (std::max)(minY, (std::min)(c.y, maxY));

    float dx = c.x - clampedX;
    float dy = c.y - clampedY;

    return (dx * dx + dy * dy) <= (r * r);
}

void BoxCollider2D::DebugDraw()
{
    
    if (!enabled) return;

    std::array<XMFLOAT2, 4> v;
    GetVertices(v);

    XMFLOAT4 col = { 0,1,0,1 }; // 밝은 초록
    // DebugRenderer의 DrawBox를 호출합니다.
    DebugRenderer::Instance().DrawBox(v, col);
}
