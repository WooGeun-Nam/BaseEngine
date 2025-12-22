#pragma once
#include "UI/UIBase.h"
#include <DirectXMath.h>
#include <functional>

class Texture;

// Slider: 값 조절 슬라이더 UI Component
class Slider : public UIBase
{
public:
    Slider() = default;
    ~Slider() = default;

    void Awake() override;
    
    void RenderUI() override;

    // 값 설정/가져오기 (0.0 ~ 1.0)
    void SetValue(float value);
    float GetValue() const { return value; }

    // 최소/최대 값 설정
    void SetMinValue(float min) { minValue = min; }
    void SetMaxValue(float max) { maxValue = max; }
    float GetMinValue() const { return minValue; }
    float GetMaxValue() const { return maxValue; }

    // 색상 설정
    void SetBackgroundColor(const DirectX::XMFLOAT4& color) { backgroundColor = color; }
    void SetFillColor(const DirectX::XMFLOAT4& color) { fillColor = color; }
    void SetHandleColor(const DirectX::XMFLOAT4& color) { handleColor = color; }

    // 이벤트 콜백
    std::function<void(float)> onValueChanged;

protected:
    // ? UIBase 이벤트 핸들러 오버라이드
    void OnPointerDown() override;
    void OnDrag(const DirectX::XMFLOAT2& delta) override;
    void OnPointerUp() override;

private:
    void UpdateValueFromMouse();

private:
    float value = 0.5f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    
    // 색상
    DirectX::XMFLOAT4 backgroundColor = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    DirectX::XMFLOAT4 fillColor = DirectX::XMFLOAT4(0.0f, 0.8f, 0.0f, 1.0f);
    DirectX::XMFLOAT4 handleColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    
    float handleWidth = 20.0f;
};
