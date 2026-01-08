#include "InspectorWindow.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/Camera2D.h"
#include "Physics/BoxCollider2D.h"
#include "Physics/CircleCollider.h"
#include "Physics/Rigidbody2D.h"
#include "Core/Animator.h"
#include "Animation/AnimatorController.h"
#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include "Resource/SpriteSheet.h"
#include <ImGui/imgui.h>
#include <DirectXMath.h>
#include <typeinfo>
#include <filesystem>
#include <Windows.h>

// Helper function for safe wstring to string conversion
static std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
        return std::string();
    
    // Use WideCharToMultiByte for Windows
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
    return result;
}

InspectorWindow::InspectorWindow()
    : EditorWindow("Inspector", true) // 기본적으로 열림
{
}

void InspectorWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(350, 600), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(windowName.c_str(), &isOpen))
    {
        if (!selectedObject)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No object selected");
            ImGui::End();
            return;
        }

        // GameObject 이름 편집
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "GameObject");
        ImGui::Separator();
        
        // 이름 입력 필드
        std::string name = "GameObject";
        if (!selectedObject->GetName().empty())
        {
            std::wstring wname = selectedObject->GetName();
            name = WStringToString(wname); // 올바른 변환 함수 사용
        }
        
        static char nameBuffer[256] = "";
        strncpy_s(nameBuffer, name.c_str(), sizeof(nameBuffer) - 1);
        
        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer)))
        {
            // 이름 변경
            std::string newName(nameBuffer);
            std::wstring wNewName(newName.begin(), newName.end());
            selectedObject->SetName(wNewName);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Transform 편집
        RenderTransform(selectedObject);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 컴포넌트 목록
        RenderComponents(selectedObject);
    }

    ImGui::End();
}

void InspectorWindow::RenderTransform(GameObject* obj)
{
    if (!obj)
        return;

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Transform");
    ImGui::Separator();

    Transform& transform = obj->transform;

    // Position
    DirectX::XMFLOAT2 pos = transform.GetPosition();
    if (ImGui::DragFloat2("Position", &pos.x, 0.5f))
    {
        transform.SetPosition(pos.x, pos.y);
    }

    // Scale
    DirectX::XMFLOAT2 scale = transform.GetScale();
    if (ImGui::DragFloat2("Scale", &scale.x, 0.01f, 0.01f, 100.0f))
    {
        transform.SetScale(scale.x, scale.y);
    }

    // Rotation (라디안 -> 도)
    float rotation = transform.GetRotation();
    float rotationDeg = DirectX::XMConvertToDegrees(rotation);
    if (ImGui::DragFloat("Rotation", &rotationDeg, 0.5f, -360.0f, 360.0f))
    {
        transform.SetRotation(DirectX::XMConvertToRadians(rotationDeg));
    }
    
    // Size (Width/Height) - 편집 가능, Scale을 역계산
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Size");
    
    // SpriteRenderer가 있으면 텍스처 크기 기준, 없으면 기본 100 픽셀
    float baseWidth = 100.0f;
    float baseHeight = 100.0f;
    
    auto* spriteRenderer = obj->GetComponent<SpriteRenderer>();
    if (spriteRenderer && spriteRenderer->GetTexture())
    {
        baseWidth = static_cast<float>(spriteRenderer->GetTexture()->Width());
        baseHeight = static_cast<float>(spriteRenderer->GetTexture()->Height());
    }
    
    float width = scale.x * baseWidth;
    float height = scale.y * baseHeight;
    
    bool sizeChanged = false;
    if (ImGui::DragFloat("Width", &width, 1.0f, 1.0f, 10000.0f))
    {
        sizeChanged = true;
    }
    if (ImGui::DragFloat("Height", &height, 1.0f, 1.0f, 10000.0f))
    {
        sizeChanged = true;
    }
    
    // Size가 변경되면 Scale 업데이트
    if (sizeChanged && baseWidth > 0 && baseHeight > 0)
    {
        float newScaleX = width / baseWidth;
        float newScaleY = height / baseHeight;
        transform.SetScale(newScaleX, newScaleY);
    }
    
    if (ImGui::IsItemHovered())
    {
        if (spriteRenderer && spriteRenderer->GetTexture())
        {
            ImGui::SetTooltip("Base texture size: %.0fx%.0f\nCurrent scale: %.2fx%.2f", 
                baseWidth, baseHeight, scale.x, scale.y);
        }
        else
        {
            ImGui::SetTooltip("No texture assigned.\nUsing default base size: 100x100");
        }
    }
}

void InspectorWindow::RenderComponents(GameObject* obj)
{
    if (!obj)
        return;

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Components");
    ImGui::Separator();

    const auto& components = obj->GetComponents();

    for (size_t i = 0; i < components.size(); ++i)
    {
        auto* comp = components[i];

        ImGui::PushID((int)i);

        // 컴포넌트 이름
        std::string componentName = typeid(*comp).name();
        if (componentName.find("class ") == 0)
            componentName = componentName.substr(6);

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", componentName.c_str());

        // Enabled 체크박스
        bool enabled = comp->IsEnabled();
        if (ImGui::Checkbox("Enabled", &enabled))
        {
            comp->SetEnabled(enabled);
        }
        
        ImGui::SameLine();
        
        // Delete Component 버튼 (비활성화 - GameObject::RemoveComponent 미구현)
        ImGui::BeginDisabled();
        if (ImGui::Button("Delete##Component"))
        {
            // TODO: GameObject::RemoveComponent 구현 필요
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("RemoveComponent not implemented yet");
        }

        // SpriteRenderer 특정 처리
        if (auto* spriteRenderer = dynamic_cast<SpriteRenderer*>(comp))
        {
            // 텍스처 필드
            ImGui::Text("Texture:");
            ImGui::SameLine();
            
            // 현재 텍스처 표시 - 파일명 표시 (Path에서 파일명 추출)
            if (spriteRenderer->GetTexture())
            {
                std::wstring texturePath = spriteRenderer->GetTexture()->Path();
                std::filesystem::path path(texturePath);
                std::wstring fileName = path.stem().wstring(); // 확장자 제외한 파일명
                std::string displayName = WStringToString(fileName); // 헬퍼 함수 사용
                
                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", displayName.c_str());
            }
            else
            {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[None]");
            }
            
            // 드롭 타겟
            if (ImGui::BeginDragDropTarget())
            {
                // 텍스처 파일 드롭
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_PATH"))
                {
                    const wchar_t* filePath = (const wchar_t*)payload->Data;
                    
                    // 파일 경로에서 파일명 추출 (확장자 제거)
                    std::wstring fullPath(filePath);
                    std::filesystem::path path(fullPath);
                    std::wstring stem = path.stem().wstring();
                    
                    // Resources에서 텍스처 로드
                    auto texture = Resources::Get<Texture>(stem);
                    if (!texture)
                    {
                        // 캐시에 없으면 로드
                        texture = Resources::Load<Texture>(stem, fullPath);
                    }
                    
                    if (texture)
                    {
                        spriteRenderer->SetTexture(texture);
                        // 원본 PNG 이미지이므로 SourceRect 초기화
                        spriteRenderer->ClearSourceRect();
                    }
                }
                
                // Sheet 프레임 드롭
                struct FramePayload
                {
                    wchar_t sheetPath[512];
                    int frameIndex;
                };
                
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHEET_FRAME"))
                {
                    const FramePayload* frameData = (const FramePayload*)payload->Data;
                    
                    // Sheet 파일 경로에서 이름 추출
                    std::wstring sheetPath(frameData->sheetPath);
                    std::filesystem::path path(sheetPath);
                    std::wstring sheetName = path.stem().wstring();
                    
                    // SpriteSheet 로드
                    auto sheet = Resources::Get<SpriteSheet>(sheetName);
                    if (!sheet)
                    {
                        sheet = Resources::Load<SpriteSheet>(sheetName, sheetPath);
                    }
                    
                    if (sheet)
                    {
                        RECT frameRect;
                        if (sheet->GetFrameRect(frameData->frameIndex, frameRect))
                        {
                            // 텍스처와 소스 Rect 설정
                            spriteRenderer->SetTexture(sheet->GetTexture());
                            spriteRenderer->SetSourceRect(frameRect);
                        }
                    }
                }
                
                ImGui::EndDragDropTarget();
            }
        }

        // BoxCollider2D 특정 처리
        if (auto* boxCollider = dynamic_cast<BoxCollider2D*>(comp))
        {
            // Half Size
            if (ImGui::DragFloat2("Half Size", &boxCollider->halfSize.x, 0.5f, 0.1f, 10000.0f))
            {
                // Updated via direct member access
            }
            
            // Offset
            auto offset = boxCollider->GetOffset();
            if (ImGui::DragFloat2("Offset", &offset.x, 0.5f, -10000.0f, 10000.0f))
            {
                boxCollider->SetOffset(offset.x, offset.y);
            }
            
            // Is Trigger checkbox
            bool isTrigger = boxCollider->IsTrigger();
            if (ImGui::Checkbox("Is Trigger##BoxCollider", &isTrigger))
            {
                boxCollider->SetTrigger(isTrigger);
            }
            
            // Fit To Texture button
            if (ImGui::Button("Fit To Texture##BoxCollider"))
            {
                boxCollider->FitToTexture();
            }
        }

        // CircleCollider 특정 처리
        if (auto* circleCollider = dynamic_cast<CircleCollider*>(comp))
        {
            // Radius
            if (ImGui::DragFloat("Radius", &circleCollider->radius, 0.5f, 0.1f, 10000.0f))
            {
                // Updated via direct member access
            }
            
            // Offset
            auto offset = circleCollider->GetOffset();
            if (ImGui::DragFloat2("Offset", &offset.x, 0.5f, -10000.0f, 10000.0f))
            {
                circleCollider->SetOffset(offset.x, offset.y);
            }
            
            // Is Trigger checkbox
            bool isTrigger = circleCollider->IsTrigger();
            if (ImGui::Checkbox("Is Trigger##CircleCollider", &isTrigger))
            {
                circleCollider->SetTrigger(isTrigger);
            }
            
            // Fit To Texture button
            if (ImGui::Button("Fit To Texture##CircleCollider"))
            {
                circleCollider->FitToTexture();
            }
        }

        // Animator 특정 처리
        if (auto* animator = dynamic_cast<Animator*>(comp))
        {
            // Animation Controller 필드
            ImGui::Text("Controller:");
            ImGui::SameLine();
            
            // 현재 컨트롤러 표시
            auto controller = animator->GetController();
            if (controller)
            {
                std::wstring controllerPath = controller->Path();
                std::filesystem::path path(controllerPath);
                std::wstring fileName = path.stem().wstring();
                std::string displayName = WStringToString(fileName); // 헬퍼 함수 사용
                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", displayName.c_str());
            }
            else
            {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[None]");
            }
            
            // 컨트롤러 드롭 타겟
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTROLLER_PATH"))
                {
                    const wchar_t* filePath = (const wchar_t*)payload->Data;
                    
                    // 전체 경로에서 파일명 추출 (확장자 제외)
                    std::wstring fullPath(filePath);
                    std::filesystem::path path(fullPath);
                    std::wstring stem = path.stem().wstring();
                    
                    // Resources에서 AnimatorController 로드
                    auto loadedController = Resources::Get<AnimatorController>(stem);
                    if (!loadedController)
                    {
                        // 캐시에 없으면 로드
                        loadedController = Resources::Load<AnimatorController>(stem, fullPath);
                    }
                    
                    if (loadedController)
                    {
                        animator->SetController(loadedController);
                    }
                }
                
                ImGui::EndDragDropTarget();
            }
            
            ImGui::Spacing();
            
            // AnimatorController의 파라미터 표시 (SetFloat, SetBool 등)
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Parameters");
            ImGui::Text("Use SetFloat/SetInt/SetBool in code");
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Set parameters via script:\n"
                    "animator->SetFloat(L\"Speed\", value);\n"
                    "animator->SetBool(L\"isJumping\", true);\n"
                    "animator->SetTrigger(L\"Attack\");");
            }
        }

        // Camera2D 특정 처리
        if (auto* camera = dynamic_cast<Camera2D*>(comp))
        {
            // 카메라 위치
            auto camPos = camera->GetPosition();
            float posArray[2] = { camPos.x, camPos.y };
            if (ImGui::DragFloat2("Camera Position", posArray, 1.0f))
            {
                camera->SetPosition(posArray[0], posArray[1]);
            }
            
            // 뷰포트 크기
            float viewportWidth = camera->GetViewportWidth();
            float viewportHeight = camera->GetViewportHeight();
            
            if (ImGui::DragFloat("Viewport Width", &viewportWidth, 1.0f, 100.0f, 5000.0f))
            {
                camera->SetViewportSize(viewportWidth, viewportHeight);
            }
            
            if (ImGui::DragFloat("Viewport Height", &viewportHeight, 1.0f, 100.0f, 5000.0f))
            {
                camera->SetViewportSize(viewportWidth, viewportHeight);
            }
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Viewport size shown in Scene View");
        }

        ImGui::Separator();

        ImGui::PopID();
    }
    
    ImGui::Spacing();
    
    // Add Component 버튼
    if (ImGui::Button("Add Component", ImVec2(-1, 30)))
    {
        ImGui::OpenPopup("AddComponentPopup");
    }
    
    // Add Component 팝업
    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Add Component");
        ImGui::Separator();
        
        if (ImGui::Selectable("SpriteRenderer"))
        {
            obj->AddComponent<SpriteRenderer>();
        }
        
        if (ImGui::Selectable("BoxCollider2D"))
        {
            obj->AddComponent<BoxCollider2D>();
        }
        
        if (ImGui::Selectable("CircleCollider"))
        {
            obj->AddComponent<CircleCollider>();
        }
        
        if (ImGui::Selectable("Rigidbody2D"))
        {
            obj->AddComponent<Rigidbody2D>();
        }
        
        if (ImGui::Selectable("Animator"))
        {
            obj->AddComponent<Animator>();
        }
        
        ImGui::EndPopup();
    }
}
