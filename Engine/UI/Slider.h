#pragma once
#include "UI/UIBase.h"
#include <DirectXMath.h>
#include <memory>
#include <functional>

class Texture;

// Slider: 값 조절 슬라이더 UI Component
// 
// 사용 방법:
// 1. GameObject에 Slider Component 추가
// 2. SetValue(0.5f) 등으로 초기 값 설정
// 3. onValueChanged 콜백 등록
// 
// 특징:
// - 값 범위: 0.0 ~ 1.0
// - 마우스 드래그로 값 조절
// - 배경 바, 채워지는 바, 핸들
// - 볼륨 조절, 체력바, 진행도 표시 등에 사용
//
// 예시:
// auto slider = sliderObj->AddComponent<Slider>();
// slider->SetValue(0.7f);
// slider->onValueChanged = [](float value) {
//     AudioManager::SetVolume(value);
// };
class Slider : public UIBase
{
public:
    Slider() = default;
    ~Slider() = default;

    void Awake() override;
    void Update(float deltaTime) override;
    
    // Component::Render() 오버라이드
    void Render() override;

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

private:
    bool IsPointerInside();
    void UpdateValueFromMouse();

private:
    float value = 0.5f;          // 현재 값 (0.0 ~ 1.0)
    float minValue = 0.0f;       // 최소 값
    float maxValue = 1.0f;       // 최대 값
    
    bool isDragging = false;     // 드래그 중 여부
    
    // 색상
    DirectX::XMFLOAT4 backgroundColor = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);  // 배경 바
    DirectX::XMFLOAT4 fillColor = DirectX::XMFLOAT4(0.0f, 0.8f, 0.0f, 1.0f);        // 채워지는 바
    DirectX::XMFLOAT4 handleColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);      // 핸들
    
    // 크기 설정
    float handleWidth = 20.0f;   // 핸들 너비
};
