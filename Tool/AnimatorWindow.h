#pragma once
#include "EditorWindow.h" // 추가
#include <memory>
#include <string>
#include <vector>
#include <DirectXMath.h>
#include <ImGui/imgui.h>

class AnimatorController;
class AnimationState;
class AnimationTransition;
class AnimationClip;

// 상태 노드 위치
struct StateNode
{
    AnimationState* state;
    DirectX::XMFLOAT2 position;
    
    StateNode(AnimationState* s, DirectX::XMFLOAT2 pos) 
        : state(s), position(pos) {}
};

// Controller 파일 정보
struct ControllerFileInfo
{
    std::wstring filename;      // 파일명 (확장자 포함)
    std::wstring fullPath;      // 전체 경로
    std::wstring stem;          // 확장자 제외 파일명
};

// Animator Window - 애니메이션 상태머신 에디터
class AnimatorWindow : public EditorWindow // 상속 추가
{
public:
    AnimatorWindow();
    ~AnimatorWindow();

    // ImGui 렌더링 (오버라이드)
    void Render() override;

    // 현재 열린 컨트롤러
    std::shared_ptr<AnimatorController> GetCurrentController() const { return currentController; }

private:
    // 현재 편집 중인 컨트롤러
    std::shared_ptr<AnimatorController> currentController;
    std::wstring currentControllerPath;
    std::wstring currentControllerName;

    // UI 상태
    bool isOpen = true;
    
    // 선택된 항목
    AnimationState* selectedState = nullptr;
    AnimationTransition* selectedTransition = nullptr;
    
    // 상태 노드 목록
    std::vector<StateNode> stateNodes;

    // 드래그 상태
    bool isDraggingNode = false;
    StateNode* draggedNode = nullptr;
    DirectX::XMFLOAT2 dragStartPos;
    
    // 전환 생성 드래그
    bool isDraggingTransition = false;
    StateNode* transitionStartNode = nullptr;
    ImVec2 transitionEndPos;
    
    // 전환 생성 모드 (우클릭 메뉴)
    bool isCreatingTransition = false;
    AnimationState* transitionSourceState = nullptr;
    
    // 컨텍스트 메뉴
    ImVec2 contextMenuPos;
    StateNode* contextMenuNode = nullptr;  // 우클릭된 노드

    // 그리드 설정
    float gridSize = 50.0f;
    DirectX::XMFLOAT2 canvasOffset = { 0, 0 };
    float canvasZoom = 1.0f;
    
    // Controller 파일 목록
    std::vector<ControllerFileInfo> controllerFiles;
    int selectedControllerIndex = -1;

    // UI 렌더링 함수
    void RenderControllerList();
    void RenderParameters();
    void RenderStateGraph();
    void RenderInspector();

    // 상태 그래프 렌더
    void RenderStateNode(StateNode& node);
    void RenderTransitionArrow(AnimationTransition* transition);
    void HandleNodeDragging();
    void HandleNodeSelection();
    void HandleContextMenu();

    // Inspector 패널
    void InspectState(AnimationState* state);
    void InspectTransition(AnimationTransition* transition);
    
    // 클립 선택 UI
    void RenderClipSelector(AnimationState* state);
    
    // 조건 편집 UI
    void RenderConditionEditor(AnimationTransition* transition);
    void AddConditionToTransition(AnimationTransition* transition);
    void RemoveConditionFromTransition(AnimationTransition* transition, int index);

    // 파라미터 관리
    void RenderParameterList();
    void AddParameter(const std::wstring& name, int type);
    void RemoveParameter(const std::wstring& name);

    // 상태 관리
    void CreateNewState(const std::wstring& name, DirectX::XMFLOAT2 position);
    void DeleteState(AnimationState* state);
    void SetDefaultState(AnimationState* state);
    void CreateTransition(AnimationState* from, AnimationState* to);
    void DeleteTransition(AnimationTransition* transition);
    
    // Controller 관리
    void NewController();
    void OpenController(const std::wstring& path);
    void SaveController();
    void ScanControllerFolder();
    void OpenControllerFolder();
    
    // 노드 위치 저장/로드
    void SaveNodePositions(const std::wstring& path);
    void LoadNodePositions(const std::wstring& path);

    // 유틸리티
    DirectX::XMFLOAT2 ScreenToCanvas(DirectX::XMFLOAT2 screenPos);
    DirectX::XMFLOAT2 CanvasToScreen(DirectX::XMFLOAT2 canvasPos);
    bool IsPointInRect(DirectX::XMFLOAT2 point, DirectX::XMFLOAT2 rectMin, DirectX::XMFLOAT2 rectMax);
    StateNode* GetNodeAtPosition(DirectX::XMFLOAT2 position);
    AnimationTransition* GetTransitionAtPosition(DirectX::XMFLOAT2 position);
    float DistancePointToLine(DirectX::XMFLOAT2 point, DirectX::XMFLOAT2 lineStart, DirectX::XMFLOAT2 lineEnd);
    
    // 모든 AnimationClip 가져오기
    std::vector<std::shared_ptr<AnimationClip>> GetAllAnimationClips();
    
    // Helper
    std::wstring CharToWString(const char* str);
    std::string WStringToString(const std::wstring& wstr);

    // 임시 입력 버퍼
    char controllerNameBuffer[128] = "";
    char stateNameBuffer[128] = "";
    char parameterNameBuffer[128] = "";
    int parameterTypeIndex = 0;
    
    // 조건 추가용 임시 버퍼
    int selectedParameterIndex = 0;
    int selectedConditionMode = 0;
    float conditionThreshold = 0.0f;
    
    // 상태 메시지
    bool showSuccessMessage = false;
    bool showErrorMessage = false;
    std::string statusMessage;
};
