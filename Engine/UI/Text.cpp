#include "UI/Text.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Core/Application.h"
#include <SpriteBatch.h>
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// GDI+ 초기화 (전역)
static ULONG_PTR gdiplusToken = 0;
static bool gdiplusInitialized = false;

Text::~Text()
{
    textTexture.Reset();
    textureSRV.Reset();
}

void Text::Awake()
{
    UIBase::Awake();
    
    // GDI+ 초기화 (1회만)
    if (!gdiplusInitialized)
    {
        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
        gdiplusInitialized = true;
    }
}

void Text::Update(float deltaTime)
{
    if (needsUpdate)
    {
        UpdateTexture();
        needsUpdate = false;
    }
}

void Text::SetText(const std::wstring& newText)
{
    if (text != newText)
    {
        text = newText;
        needsUpdate = true;
    }
}

void Text::SetFont(std::shared_ptr<::Font> fontAsset, float size)
{
    font = fontAsset;
    fontSize = size;
    needsUpdate = true;
}

void Text::UpdateTexture()
{
    if (text.empty() || !font)
        return;

    // 폰트 로드
    PrivateFontCollection fontCollection;
    Status status = fontCollection.AddFontFile(font->GetPath().c_str());
    if (status != Ok)
    {
        OutputDebugStringA("[Text] Failed to load font file\n");
        return;
    }

    int fontCount = fontCollection.GetFamilyCount();
    if (fontCount == 0)
        return;

    FontFamily fontFamily;
    fontCollection.GetFamilies(1, &fontFamily, &fontCount);

    Gdiplus::Font gdipFont(&fontFamily, fontSize, FontStyleRegular, UnitPixel);

    // 텍스트 크기 측정
    Bitmap tempBitmap(1, 1);
    Graphics tempGraphics(&tempBitmap);
    tempGraphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    RectF bounds;
    tempGraphics.MeasureString(text.c_str(), -1, &gdipFont, PointF(0, 0), &bounds);

    textureWidth = static_cast<int>(bounds.Width) + 4;
    textureHeight = static_cast<int>(bounds.Height) + 4;

    // 비트맵 렌더
    Bitmap bitmap(textureWidth, textureHeight, PixelFormat32bppARGB);
    Graphics graphics(&bitmap);
    graphics.Clear(Color(0, 0, 0, 0));  // 투명 배경
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    SolidBrush brush(Color(255, 255, 255, 255));  // 흰색 (코드에서 틴팅)
    graphics.DrawString(text.c_str(), -1, &gdipFont, PointF(2, 2), &brush);

    // GDI+ Bitmap → D3D11 Texture2D
    BitmapData bmpData;
    Rect rect(0, 0, textureWidth, textureHeight);
    status = bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
    
    if (status != Ok)
        return;

    // D3D11 텍스처 생성
    auto* app = gameObject->GetApplication();
    if (!app)
    {
        bitmap.UnlockBits(&bmpData);
        return;
    }

    ID3D11Device* device = app->GetDevice();
    
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = textureWidth;
    desc.Height = textureHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = bmpData.Scan0;
    initData.SysMemPitch = bmpData.Stride;

    textTexture.Reset();
    HRESULT hr = device->CreateTexture2D(&desc, &initData, textTexture.GetAddressOf());
    
    bitmap.UnlockBits(&bmpData);

    if (FAILED(hr))
    {
        OutputDebugStringA("[Text] Failed to create texture\n");
        return;
    }

    // ShaderResourceView 생성
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    textureSRV.Reset();
    hr = device->CreateShaderResourceView(textTexture.Get(), &srvDesc, textureSRV.GetAddressOf());

    if (FAILED(hr))
    {
        OutputDebugStringA("[Text] Failed to create SRV\n");
    }
}

void Text::RenderUI()
{
    if (!textureSRV || !rectTransform || !canvas)
        return;

    auto* spriteBatch = canvas->GetSpriteBatch();
    if (!spriteBatch)
        return;

    // 화면 좌표 계산
    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();
    XMFLOAT2 topLeft = rectTransform->GetTopLeftPosition(screenW, screenH);

    // 색상 변환
    XMVECTOR colorVec = XMLoadFloat4(&color);

    // RECT 설정
    RECT destRect;
    destRect.left = static_cast<LONG>(topLeft.x);
    destRect.top = static_cast<LONG>(topLeft.y);
    destRect.right = static_cast<LONG>(topLeft.x + textureWidth);
    destRect.bottom = static_cast<LONG>(topLeft.y + textureHeight);

    // 텍스트 렌더링
    spriteBatch->Draw(
        textureSRV.Get(),
        destRect,
        colorVec
    );
}
