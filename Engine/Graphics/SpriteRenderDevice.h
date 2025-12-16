#pragma once
#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>
#include <SpriteBatch.h>

using namespace DirectX;

class Camera2D;

class SpriteRenderDevice
{
public:
    static SpriteRenderDevice& Instance()
    {
        static SpriteRenderDevice instance;
        return instance;
    }

    struct DrawInfo
    {
        ID3D11ShaderResourceView* texture = nullptr;

        XMFLOAT2 position{ 0,0 };
        XMFLOAT2 scale{ 1,1 };
        float rotation = 0.0f;

        XMFLOAT4 color{ 1,1,1,1 };

        bool flipX = false;
        bool flipY = false;

        bool useSource = false; 
        RECT sourceRect{};

        XMFLOAT2 origin{ 0,0 };
        float layer = 0.0f;
    };

public:
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Begin();
    void End();
    void Draw(const DrawInfo& info);

    void SetCamera(Camera2D* cam) { camera = cam; }

private:
    SpriteRenderDevice() = default;

private:
    std::unique_ptr<SpriteBatch> spriteBatch;
    ID3D11DeviceContext* context = nullptr;

    Camera2D* camera = nullptr;
};
