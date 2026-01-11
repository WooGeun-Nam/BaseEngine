#include "AnimatorWindow.h"
#include "Animation/AnimatorController.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationState.h"
#include "Animation/AnimationTransition.h"
#include "Animation/AnimatorParameter.h"
#include "Resource/Resources.h"
#include "Resource/AnimationClip.h"
#include "ConsoleWindow.h"

#include <ImGui/imgui.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <Windows.h>
#include <shellapi.h>
#include <fstream>

using json = nlohmann::json;

AnimatorWindow::AnimatorWindow()
    : EditorWindow("Animator", false) // EditorWindow 생성자 호출
{
    // 기본 컨트롤러 생성하지 않음 - 사용자가 New 버튼으로 생성
    ScanControllerFolder();
}

AnimatorWindow::~AnimatorWindow()
{
}

void AnimatorWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(1200, 700), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Animator", &isOpen))
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Animator Controller Editor");
        ImGui::Separator();
        ImGui::Spacing();
        
        // 컬럼 생성 전에 사용 가능한 너비를 먼저 계산
        float availableWidth = ImGui::GetContentRegionAvail().x;
        
        // 3단 레이아웃: Controllers List | State Graph | Inspector
        ImGui::Columns(3, "MainColumns", true);
        
        // 컬럼 너비 설정: 왼쪽(250) + 중앙(나머지 - 300) + 오른쪽(300)
        ImGui::SetColumnWidth(0, 250);
        if (availableWidth > 550)  // 최소 너비 확인 (250 + 300)
        {
            ImGui::SetColumnWidth(1, availableWidth - 550);
        }
        
        // 왼쪽: Controller 파일 목록
        RenderControllerList();
        
        ImGui::NextColumn();
        
        // 중앙: 상태 그래프
        RenderStateGraph();
        
        ImGui::NextColumn();
        
        // 오른쪽: Inspector
        ImGui::BeginChild("InspectorPanel", ImVec2(300, 0), true);
        
        // Parameters 섹션
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Parameters");
        ImGui::Separator();
        RenderParameters();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Inspector 섹션
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Inspector");
        ImGui::Separator();
        RenderInspector();
        
        ImGui::EndChild();
        
        ImGui::Columns(1);
    }
    ImGui::End();
}

void AnimatorWindow::RenderControllerList()
{
    ImGui::BeginChild("ControllerList", ImVec2(0, 0), true);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Controllers");
    ImGui::Separator();
    
    // New, Refresh, Open Folder 버튼
    if (ImGui::Button("New", ImVec2(ImGui::GetContentRegionAvail().x * 0.33f - 2, 0)))
    {
        ImGui::OpenPopup("NewControllerPopup");
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Create new controller");
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Refresh", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 2, 0)))
    {
        ScanControllerFolder();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Refresh controller list");
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Open Folder", ImVec2(-1, 0)))
    {
        OpenControllerFolder();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Open Assets/Controllers folder");
    }
    
    ImGui::Separator();
    ImGui::Spacing();
    
    // New Controller 팝업
    if (ImGui::BeginPopup("NewControllerPopup"))
    {
        ImGui::Text("New Controller Name:");
        ImGui::InputText("##NewControllerName", controllerNameBuffer, sizeof(controllerNameBuffer));
        
        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            if (strlen(controllerNameBuffer) > 0)
            {
                std::wstring name = CharToWString(controllerNameBuffer);
                NewController();
                currentControllerName = name;
                
                // 즉시 저장
                SaveController();
                
                // 목록 새로고침
                ScanControllerFolder();
                
                memset(controllerNameBuffer, 0, sizeof(controllerNameBuffer));
                ImGui::CloseCurrentPopup();
                
                showSuccessMessage = true;
                statusMessage = "Controller created: " + WStringToString(name);
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            memset(controllerNameBuffer, 0, sizeof(controllerNameBuffer));
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // Controller 파일 목록
    for (size_t i = 0; i < controllerFiles.size(); ++i)
    {
        ImGui::PushID((int)i);
        
        bool isSelected = (selectedControllerIndex == (int)i);
        std::string displayName = WStringToString(controllerFiles[i].filename);
        
        if (ImGui::Selectable(displayName.c_str(), isSelected))
        {
            selectedControllerIndex = (int)i;
            OpenController(controllerFiles[i].fullPath);
            currentControllerName = controllerFiles[i].stem;
        }
        
        ImGui::PopID();
    }
    
    // 상태 메시지
    if (showSuccessMessage)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[SUCCESS]");
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }
    
    if (showErrorMessage)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ERROR]");
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }
    
    ImGui::EndChild();
}

void AnimatorWindow::RenderParameters()
{
    ImGui::Text("Parameters");
    ImGui::Separator();
    
    if (!currentController)
        return;
    
    RenderParameterList();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Add Parameter
    ImGui::Text("Add Parameter");
    ImGui::InputText("Name##Param", parameterNameBuffer, sizeof(parameterNameBuffer));
    
    const char* types[] = { "Float", "Int", "Bool", "Trigger" };
    ImGui::Combo("Type##Param", &parameterTypeIndex, types, 4);
    
    if (ImGui::Button("Add##Param"))
    {
        if (strlen(parameterNameBuffer) > 0)
        {
            std::string nameStr(parameterNameBuffer);
            std::wstring name(nameStr.begin(), nameStr.end());
            AddParameter(name, parameterTypeIndex);
            memset(parameterNameBuffer, 0, sizeof(parameterNameBuffer));
        }
    }
}

void AnimatorWindow::RenderParameterList()
{
    if (!currentController)
        return;
    
    const auto& params = currentController->GetParameters().GetAllParameters();
    
    std::vector<std::wstring> keysToRemove;
    
    for (const auto& pair : params)
    {
        std::wstring wname = pair.first;
        std::string name = WStringToString(wname);  // 변환 함수 사용
        
        ImGui::PushID(name.c_str());
        
        // 파라미터 타입에 따라 다른 UI 표시
        switch (pair.second.type)
        {
        case AnimatorParameterType::Float:
        {
            float value = currentController->GetFloat(wname);
            if (ImGui::DragFloat(name.c_str(), &value, 0.1f))
            {
                currentController->SetFloat(wname, value);
            }
            break;
        }
        case AnimatorParameterType::Int:
        {
            int value = currentController->GetInt(wname);
            if (ImGui::DragInt(name.c_str(), &value))
            {
                currentController->SetInt(wname, value);
            }
            break;
        }
        case AnimatorParameterType::Bool:
        {
            bool value = currentController->GetBool(wname);
            if (ImGui::Checkbox(name.c_str(), &value))
            {
                currentController->SetBool(wname, value);
            }
            break;
        }
        case AnimatorParameterType::Trigger:
        {
            if (ImGui::Button(name.c_str()))
            {
                currentController->SetTrigger(wname);
            }
            ImGui::SameLine();
            ImGui::Text("(Trigger)");
            break;
        }
        }
        
        // 삭제 버튼
        ImGui::SameLine();
        if (ImGui::SmallButton("X"))
        {
            keysToRemove.push_back(wname);
        }
        
        ImGui::PopID();
    }
    
    // 삭제 처리 (반복문 밖에서)
    for (const auto& key : keysToRemove)
    {
        RemoveParameter(key);
    }
}

void AnimatorWindow::RenderStateGraph()
{
    ImGui::BeginChild("StateGraphPanel", ImVec2(0, 0), true);
    
    // 상단 툴바
    if (currentController)
    {
        std::string title = "State Graph";
        if (!currentControllerName.empty())
        {
            title += " - " + WStringToString(currentControllerName);
        }
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", title.c_str());
        
        // Save 버튼
        if (ImGui::Button("Save", ImVec2(100, 0)))
        {
            SaveController();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Save controller (Ctrl+S)");
        }
        
        ImGui::SameLine();
        
        // Create State 버튼
        if (ImGui::Button("Create State", ImVec2(120, 0)))
        {
            ImGui::OpenPopup("CreateStatePopup");
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Create new state in center of canvas");
        }
        
        // Create State 팝업
        if (ImGui::BeginPopup("CreateStatePopup"))
        {
            ImGui::Text("New State Name:");
            ImGui::InputText("##NewStateName", stateNameBuffer, sizeof(stateNameBuffer));
            
            if (ImGui::Button("Create", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter))
            {
                if (strlen(stateNameBuffer) > 0)
                {
                    std::string nameStr(stateNameBuffer);
                    std::wstring name(nameStr.begin(), nameStr.end());
                    
                    // 캔버스 중앙에 생성
                    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
                    DirectX::XMFLOAT2 centerPos(canvasSize.x * 0.5f - 75.0f, canvasSize.y * 0.5f - 40.0f);
                    
                    CreateNewState(name, centerPos);
                    memset(stateNameBuffer, 0, sizeof(stateNameBuffer));
                    ImGui::CloseCurrentPopup();
                }
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                memset(stateNameBuffer, 0, sizeof(stateNameBuffer));
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        
        // Ctrl+S 단축키
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S))
        {
            SaveController();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No Controller Selected");
        ImGui::Text("Create new controller or select from the list");
        ImGui::EndChild();
        return;
    }
    
    ImGui::Separator();
    
    // UI 안내 메시지 - 전환 생성 모드에 따라 다르게 표시
    if (isCreatingTransition && transitionSourceState)
    {
        std::string sourceName = WStringToString(transitionSourceState->GetName());
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 
            "Creating Transition from '%s' - Click destination state (ESC to cancel)", 
            sourceName.c_str());
    }
    else
    {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
            "Right-click canvas: Create State | Right-click state: Make Transition");
    }
    
    ImGui::Separator();
    
    auto* stateMachine = currentController->GetStateMachine();
    if (!stateMachine)
    {
        ImGui::EndChild();
        return;
    }
    
    // 상태 노드들의 동기화 (새로 추가된 상태만 처리)
    const auto& states = stateMachine->GetAllStates();
    if (stateNodes.size() < states.size())
    {
        // 새로 추가된 상태 찾아서 추가
        float offsetX = 100.0f;
        float offsetY = 100.0f;
        
        // 기존 노드가 있으면 다음 위치 계산
        if (!stateNodes.empty())
        {
            for (const auto& node : stateNodes)
            {
                if (node.position.x + 200.0f > offsetX)
                    offsetX = node.position.x + 200.0f;
                if (node.position.y > offsetY)
                    offsetY = node.position.y;
            }
        }
        
        // 새로운 상태만 추가
        for (auto* state : states)
        {
            bool exists = false;
            for (const auto& node : stateNodes)
            {
                if (node.state == state)
                {
                    exists = true;
                    break;
                }
            }
            
            if (!exists)
            {
                stateNodes.emplace_back(state, DirectX::XMFLOAT2(offsetX, offsetY));
                offsetX += 200.0f;
                if (offsetX > 800.0f)
                {
                    offsetX = 100.0f;
                    offsetY += 150.0f;
                }
            }
        }
    }
    else if (stateNodes.size() > states.size())
    {
        // 삭제된 상태 제거
        stateNodes.erase(
            std::remove_if(stateNodes.begin(), stateNodes.end(),
                [&states](const StateNode& node) {
                    return std::find(states.begin(), states.end(), node.state) == states.end();
                }),
            stateNodes.end()
        );
    }
    
    // 캔버스 영역 시작
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    
    // Invisible button for canvas interaction
    ImGui::InvisibleButton("StateGraphCanvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    bool isCanvasHovered = ImGui::IsItemHovered();
    bool isCanvasActive = ImGui::IsItemActive();
    
    // DrawList 가져오기
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // 클리핑 영역 설정
    drawList->PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);
    
    // 그리드 배경
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), 
                            IM_COL32(30, 30, 30, 255));
    
    // 그리드 라인
    for (float x = fmod(canvasOffset.x, gridSize); x < canvasSize.x; x += gridSize)
    {
        drawList->AddLine(ImVec2(canvasPos.x + x, canvasPos.y), 
                         ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y), 
                         IM_COL32(50, 50, 50, 255));
    }
    for (float y = fmod(canvasOffset.y, gridSize); y < canvasSize.y; y += gridSize)
    {
        drawList->AddLine(ImVec2(canvasPos.x, canvasPos.y + y), 
                         ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), 
                         IM_COL32(50, 50, 50, 255));
    }
    
    // 전환 화살표 그리기
    for (auto& node : stateNodes)
    {
        for (auto* transition : node.state->GetTransitions())
        {
            RenderTransitionArrow(transition);
        }
    }
    
    // 상태 노드 그리기
    for (auto& node : stateNodes)
    {
        RenderStateNode(node);
    }
    
    // 클리핑 영역 해제
    drawList->PopClipRect();
    
    // 인터랙션 핸들링
    HandleNodeDragging();
    HandleNodeSelection();
    HandleContextMenu();
    
    ImGui::EndChild();
}

void AnimatorWindow::RenderStateNode(StateNode& node)
{
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    DirectX::XMFLOAT2 screenPosXM = CanvasToScreen(node.position);
    ImVec2 screenPos(screenPosXM.x + canvasPos.x, screenPosXM.y + canvasPos.y);
    
    ImVec2 nodeSize(150, 80);
    ImVec2 nodePosMax(screenPos.x + nodeSize.x, screenPos.y + nodeSize.y);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // 노드 색상
    ImU32 nodeColor = IM_COL32(60, 60, 60, 255);
    ImU32 borderColor = IM_COL32(100, 100, 100, 255);
    
    if (node.state == selectedState)
    {
        borderColor = IM_COL32(0, 150, 255, 255); // 파란색 테두리
    }
    
    auto* stateMachine = currentController->GetStateMachine();
    if (stateMachine && node.state == stateMachine->GetDefaultState())
    {
        nodeColor = IM_COL32(80, 80, 50, 255); // 노란색 기본 상태
        borderColor = IM_COL32(200, 200, 100, 255);
    }
    
    // 노드 박스
    drawList->AddRectFilled(screenPos, nodePosMax, nodeColor, 5.0f);
    drawList->AddRect(screenPos, nodePosMax, borderColor, 5.0f, 0, 2.0f);
    
    // 상태 이름 - 변환 함수 사용
    std::string stateName = WStringToString(node.state->GetName());
    ImVec2 textSize = ImGui::CalcTextSize(stateName.c_str());
    ImVec2 textPos(screenPos.x + (nodeSize.x - textSize.x) * 0.5f, 
                   screenPos.y + 10);
    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), stateName.c_str());
    
    // 클립 이름
    auto clip = node.state->GetClip();
    if (clip)
    {
        std::wstring clipPath = clip->Path();
        size_t lastSlash = clipPath.find_last_of(L"/\\");
        std::wstring clipName = (lastSlash != std::wstring::npos) ? clipPath.substr(lastSlash + 1) : clipPath;
        
        std::string clipStr = WStringToString(clipName);  // 변환 함수 사용
        ImVec2 clipTextSize = ImGui::CalcTextSize(clipStr.c_str());
        ImVec2 clipTextPos(screenPos.x + (nodeSize.x - clipTextSize.x) * 0.5f, 
                          screenPos.y + 35);
        drawList->AddText(clipTextPos, IM_COL32(180, 180, 180, 255), clipStr.c_str());
    }
    else
    {
        ImVec2 noClipTextPos(screenPos.x + 35, screenPos.y + 35);
        drawList->AddText(noClipTextPos, IM_COL32(150, 150, 150, 255), "(No Clip)");
    }
}

void AnimatorWindow::RenderTransitionArrow(AnimationTransition* transition)
{
    if (!transition)
        return;
    
    // 안전성 체크: 소스와 목적지 상태가 유효한지 확인
    auto* sourceState = transition->GetSourceState();
    auto* destState = transition->GetDestinationState();
    
    if (!sourceState || !destState)
        return;
    
    // 출발/도착 노드 찾기
    StateNode* fromNode = nullptr;
    StateNode* toNode = nullptr;
    
    for (auto& node : stateNodes)
    {
        if (node.state == sourceState)
            fromNode = &node;
        if (node.state == destState)
            toNode = &node;
    }
    
    if (!fromNode || !toNode)
        return;
    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    
    // 노드 크기
    const float nodeWidth = 150.0f;
    const float nodeHeight = 80.0f;
    
    // 노드 중앙점 계산 (캔버스 좌표)
    DirectX::XMFLOAT2 fromCenter(fromNode->position.x + nodeWidth * 0.5f, fromNode->position.y + nodeHeight * 0.5f);
    DirectX::XMFLOAT2 toCenter(toNode->position.x + nodeWidth * 0.5f, toNode->position.y + nodeHeight * 0.5f);
    
    // 역방향 트랜지션이 있는지 확인 (양방향 트랜지션 감지)
    bool hasReverseTransition = false;
    for (auto* reverseTransition : toNode->state->GetTransitions())
    {
        if (reverseTransition && reverseTransition->GetDestinationState() == fromNode->state)
        {
            hasReverseTransition = true;
            break;
        }
    }
    
    // 방향 벡터 계산
    float dx = toCenter.x - fromCenter.x;
    float dy = toCenter.y - fromCenter.y;
    float length = sqrtf(dx * dx + dy * dy);
    
    if (length < 1.0f)
        return;
    
    // 정규화된 방향 벡터
    float dirX = dx / length;
    float dirY = dy / length;
    
    // 노드 경계까지의 교점 계산 (박스의 가장자리)
    auto calculateBoxIntersection = [](DirectX::XMFLOAT2 center, float width, float height, float dirX, float dirY) -> DirectX::XMFLOAT2 {
        // 박스의 반 크기
        float halfW = width * 0.5f;
        float halfH = height * 0.5f;
        
        // 각 방향에 대한 교점 시간(t) 계산
        float tX = (dirX > 0) ? halfW / dirX : (dirX < 0 ? -halfW / dirX : FLT_MAX);
        float tY = (dirY > 0) ? halfH / dirY : (dirY < 0 ? -halfH / dirY : FLT_MAX);
        
        // 더 작은 t 값이 실제 교점
        float t = (tX < tY) ? tX : tY;
        
        return DirectX::XMFLOAT2(center.x + dirX * t, center.y + dirY * t);
    };
    
    // 출발점과 도착점을 노드 경계로 조정
    DirectX::XMFLOAT2 fromEdge = calculateBoxIntersection(fromCenter, nodeWidth, nodeHeight, dirX, dirY);
    DirectX::XMFLOAT2 toEdge = calculateBoxIntersection(toCenter, nodeWidth, nodeHeight, -dirX, -dirY);
    
    // 캔버스 좌표를 화면 좌표로 변환
    DirectX::XMFLOAT2 fromPosScreen = CanvasToScreen(fromEdge);
    DirectX::XMFLOAT2 toPosScreen = CanvasToScreen(toEdge);
    
    ImVec2 fromPos(fromPosScreen.x + canvasPos.x, fromPosScreen.y + canvasPos.y);
    ImVec2 toPos(toPosScreen.x + canvasPos.x, toPosScreen.y + canvasPos.y);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // 색상 설정
    ImU32 arrowColor = IM_COL32(150, 150, 150, 255);
    ImU32 arrowHeadColor = IM_COL32(200, 200, 200, 255);
    
    if (transition == selectedTransition)
    {
        arrowColor = IM_COL32(255, 150, 0, 255); // 주황색
        arrowHeadColor = IM_COL32(255, 200, 100, 255);
    }
    
    // 양방향 트랜지션인 경우 곡선으로 그리기
    if (hasReverseTransition)
    {
        // 수직 벡터 (곡선을 위한 오프셋 방향)
        float perpX = -dirY;
        float perpY = dirX;
        
        float offsetAmount = 20.0f; // 오프셋 크기
        
        // 베지어 곡선을 위한 제어점 계산
        float midX = (fromPos.x + toPos.x) * 0.5f;
        float midY = (fromPos.y + toPos.y) * 0.5f;
        
        ImVec2 controlPoint1(fromPos.x * 0.75f + midX * 0.25f + perpX * offsetAmount,
                             fromPos.y * 0.75f + midY * 0.25f + perpY * offsetAmount);
        
        ImVec2 controlPoint2(toPos.x * 0.75f + midX * 0.25f + perpX * offsetAmount,
                             toPos.y * 0.75f + midY * 0.25f + perpY * offsetAmount);
        
        // 베지어 곡선 그리기
        drawList->AddBezierCubic(fromPos, controlPoint1, controlPoint2, toPos, arrowColor, 2.5f);
        
        // 화살표 머리를 곡선의 끝 방향으로 조정
        float endDx = toPos.x - controlPoint2.x;
        float endDy = toPos.y - controlPoint2.y;
        float endLength = sqrtf(endDx * endDx + endDy * endDy);
        
        if (endLength > 0.1f)
        {
            dirX = endDx / endLength;
            dirY = endDy / endLength;
        }
    }
    else
    {
        // 직선 그리기
        drawList->AddLine(fromPos, toPos, arrowColor, 2.5f);
    }
    
    // 화살표 머리 그리기
    float angle = atan2f(dirY, dirX);
    float arrowSize = 15.0f;
    float arrowAngle = 0.4f;
    
    ImVec2 arrowTip = toPos;
    ImVec2 arrowP1(
        arrowTip.x - arrowSize * cosf(angle - arrowAngle),
        arrowTip.y - arrowSize * sinf(angle - arrowAngle)
    );
    ImVec2 arrowP2(
        arrowTip.x - arrowSize * cosf(angle + arrowAngle),
        arrowTip.y - arrowSize * sinf(angle + arrowAngle)
    );
    
    // 화살표 머리 그리기 (채워진 삼각형)
    drawList->AddTriangleFilled(arrowTip, arrowP1, arrowP2, arrowHeadColor);
    
    // 화살표 외곽선
    drawList->AddTriangle(arrowTip, arrowP1, arrowP2, arrowColor, 2.0f);
}

void AnimatorWindow::RenderInspector()
{
    ImGui::Text("Inspector");
    ImGui::Separator();
    
    if (selectedState)
    {
        InspectState(selectedState);
    }
    else if (selectedTransition)
    {
        InspectTransition(selectedTransition);
    }
    else
    {
        ImGui::TextDisabled("No selection");
    }
}

void AnimatorWindow::InspectState(AnimationState* state)
{
    if (!state || !currentController)
        return;
    
    // 상태가 현재 컨트롤러에 속해있는지 확인
    auto* stateMachine = currentController->GetStateMachine();
    if (!stateMachine)
    {
        selectedState = nullptr;
        return;
    }
    
    // 상태가 유효한지 확인
    const auto& states = stateMachine->GetAllStates();
    bool stateExists = std::find(states.begin(), states.end(), state) != states.end();
    if (!stateExists)
    {
        selectedState = nullptr;
        return;
    }
    
    std::string stateName = WStringToString(state->GetName());
    ImGui::Text("State: %s", stateName.c_str());
    ImGui::Separator();
    
    // 기본 상태로 설정
    bool isDefault = (state == stateMachine->GetDefaultState());
    
    if (isDefault)
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[Default State]");
    }
    else
    {
        if (ImGui::Button("Set as Default"))
        {
            SetDefaultState(state);
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Clip 선택
    RenderClipSelector(state);
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Speed
    float speed = state->GetSpeed();
    if (ImGui::DragFloat("Speed", &speed, 0.01f, 0.1f, 10.0f))
    {
        state->SetSpeed(speed);
    }
    
    // Loop
    bool loop = state->IsLoop();
    if (ImGui::Checkbox("Loop", &loop))
    {
        state->SetLoop(loop);
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // 전환 목록
    const auto& transitions = state->GetTransitions();
    ImGui::Text("Transitions (%zu)", transitions.size());
    
    AnimationTransition* transitionToDelete = nullptr;
    
    for (auto* trans : transitions)
    {
        if (!trans) continue; // null 체크
        
        ImGui::PushID(trans);
        
        auto* destState = trans->GetDestinationState();
        if (destState)
        {
            std::string destName = WStringToString(destState->GetName());
            
            if (ImGui::Selectable(("-> " + destName).c_str(), trans == selectedTransition))
            {
                selectedTransition = trans;
                selectedState = nullptr;
            }
            
            ImGui::SameLine();
            if (ImGui::SmallButton("X"))
            {
                transitionToDelete = trans;
            }
        }
        
        ImGui::PopID();
    }
    
    // 삭제는 반복문 밖에서 처리 (안전성)
    if (transitionToDelete)
    {
        DeleteTransition(transitionToDelete);
    }
    
    ImGui::Spacing();
    if (ImGui::Button("Delete State"))
    {
        DeleteState(state);
        selectedState = nullptr;
    }
}

void AnimatorWindow::InspectTransition(AnimationTransition* transition)
{
    if (!transition || !currentController)
        return;
    
    // 트랜지션의 소스와 목적지 상태가 유효한지 확인
    auto* sourceState = transition->GetSourceState();
    auto* destState = transition->GetDestinationState();
    
    if (!sourceState || !destState)
    {
        selectedTransition = nullptr;
        return;
    }
    
    // 상태들이 현재 컨트롤러에 속해있는지 확인
    auto* stateMachine = currentController->GetStateMachine();
    if (!stateMachine)
    {
        selectedTransition = nullptr;
        return;
    }
    
    const auto& states = stateMachine->GetAllStates();
    bool sourceExists = std::find(states.begin(), states.end(), sourceState) != states.end();
    bool destExists = std::find(states.begin(), states.end(), destState) != states.end();
    
    if (!sourceExists || !destExists)
    {
        selectedTransition = nullptr;
        return;
    }
    
    std::string fromName = WStringToString(sourceState->GetName());
    std::string toName = WStringToString(destState->GetName());
    
    ImGui::Text("Transition");
    ImGui::Text("%s -> %s", fromName.c_str(), toName.c_str());
    ImGui::Separator();
    
    // Duration
    float duration = transition->GetDuration();
    if (ImGui::DragFloat("Duration", &duration, 0.01f, 0.0f, 5.0f))
    {
        transition->SetDuration(duration);
    }
    
    // Has Exit Time
    bool hasExitTime = transition->HasExitTime();
    if (ImGui::Checkbox("Has Exit Time", &hasExitTime))
    {
        transition->SetHasExitTime(hasExitTime);
    }
    
    // Exit Time
    if (hasExitTime)
    {
        float exitTime = transition->GetExitTime();
        if (ImGui::DragFloat("Exit Time", &exitTime, 0.01f, 0.0f, 1.0f))
        {
            transition->SetExitTime(exitTime);
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // 조건 편집
    RenderConditionEditor(transition);
    
    ImGui::Spacing();
    if (ImGui::Button("Delete Transition"))
    {
        DeleteTransition(transition);
        
        // 삭제 후 출발 상태 선택 (사용자 편의성)
        if (sourceState)
        {
            selectedState = sourceState;
        }
    }
}

void AnimatorWindow::RenderClipSelector(AnimationState* state)
{
    if (!state)
        return;
    
    ImGui::Text("Animation Clip:");
    
    auto allClips = GetAllAnimationClips();
    
    auto currentClip = state->GetClip();
    std::string currentClipName = "(None)";
    
    if (currentClip)
    {
        std::wstring clipPath = currentClip->Path();
        size_t lastSlash = clipPath.find_last_of(L"/\\");
        std::wstring clipFileName = (lastSlash != std::wstring::npos) ? clipPath.substr(lastSlash + 1) : clipPath;
        currentClipName = WStringToString(clipFileName);
    }
    
    if (ImGui::BeginCombo("##ClipSelector", currentClipName.c_str()))
    {
        // None 옵션
        if (ImGui::Selectable("(None)", !currentClip))
        {
            state->SetClip(nullptr);
        }
        
        // 모든 클립 표시
        for (const auto& clip : allClips)
        {
            std::wstring clipPath = clip->Path();
            size_t lastSlash = clipPath.find_last_of(L"/\\");
            std::wstring clipFileName = (lastSlash != std::wstring::npos) ? clipPath.substr(lastSlash + 1) : clipPath;
            std::string clipName = WStringToString(clipFileName);
            
            bool isSelected = (currentClip && currentClip->Path() == clip->Path());
            
            if (ImGui::Selectable(clipName.c_str(), isSelected))
            {
                state->SetClip(clip);
            }
        }
        
        ImGui::EndCombo();
    }
}

void AnimatorWindow::RenderConditionEditor(AnimationTransition* transition)
{
    if (!transition)
        return;
    
    ImGui::Text("Conditions:");
    ImGui::Separator();
    
    const auto& conditions = transition->GetConditions();
    
    int indexToRemove = -1;
    
    for (size_t i = 0; i < conditions.size(); ++i)
    {
        const auto& cond = conditions[i];
        std::string paramName = WStringToString(cond.parameterName);  // 변환 함수 사용
        
        ImGui::PushID((int)i);
        
        // 조건 표시
        std::string condStr = paramName;
        switch (cond.mode)
        {
        case TransitionConditionMode::If:
            condStr += " = true";
            break;
        case TransitionConditionMode::IfNot:
            condStr += " = false";
            break;
        case TransitionConditionMode::Greater:
            condStr += " > " + std::to_string(cond.threshold);
            break;
        case TransitionConditionMode::Less:
            condStr += " < " + std::to_string(cond.threshold);
            break;
        case TransitionConditionMode::Equals:
            condStr += " == " + std::to_string((int)cond.threshold);
            break;
        case TransitionConditionMode::NotEquals:
            condStr += " != " + std::to_string((int)cond.threshold);
            break;
        }
        
        ImGui::Text("%s", condStr.c_str());
        ImGui::SameLine();
        
        if (ImGui::SmallButton("X"))
        {
            indexToRemove = (int)i;
        }
        
        ImGui::PopID();
    }
    
    if (indexToRemove >= 0)
    {
        RemoveConditionFromTransition(transition, indexToRemove);
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Add Condition:");
    
    // 파라미터 선택
    const auto& params = currentController->GetParameters().GetAllParameters();
    std::vector<std::string> paramNames;
    for (const auto& pair : params)
    {
        std::string name = WStringToString(pair.first);
        paramNames.push_back(name);
    }
    
    if (!paramNames.empty())
    {
        if (selectedParameterIndex >= paramNames.size())
            selectedParameterIndex = 0;
        
        if (ImGui::BeginCombo("Parameter##Cond", paramNames[selectedParameterIndex].c_str()))
        {
            for (size_t i = 0; i < paramNames.size(); i++)
            {
                bool isSelected = (i == selectedParameterIndex);
                if (ImGui::Selectable(paramNames[i].c_str(), isSelected))
                {
                    selectedParameterIndex = (int)i;
                }
            }
            ImGui::EndCombo();
        }
        
        // 선택된 파라미터의 타입에 따라 조건 모드 선택
        std::wstring selectedParamName(paramNames[selectedParameterIndex].begin(), 
                                      paramNames[selectedParameterIndex].end());
        auto paramType = currentController->GetParameterType(selectedParamName);
        
        const char* modes[] = { "If (true)", "If Not (false)", "Greater", "Less", "Equals", "NotEqual" };
        int modeCount = 6;
        
        if (paramType == AnimatorParameterType::Bool || paramType == AnimatorParameterType::Trigger)
        {
            modeCount = 2; // If, IfNot만
        }
        
        ImGui::Combo("Mode##Cond", &selectedConditionMode, modes, modeCount);
        
        // Float/Int 타입이고 Greater/Less/Equals/NotEqual인 경우 threshold 입력
        if ((paramType == AnimatorParameterType::Float || paramType == AnimatorParameterType::Int) &&
            selectedConditionMode >= 2)
        {
            if (paramType == AnimatorParameterType::Float)
            {
                ImGui::DragFloat("Threshold##Cond", &conditionThreshold, 0.1f);
            }
            else
            {
                int intThreshold = (int)conditionThreshold;
                if (ImGui::DragInt("Threshold##Cond", &intThreshold))
                {
                    conditionThreshold = (float)intThreshold;
                }
            }
        }
        
        if (ImGui::Button("Add Condition"))
        {
            AddConditionToTransition(transition);
        }
    }
    else
    {
        ImGui::TextDisabled("No parameters available");
    }
}

void AnimatorWindow::HandleNodeDragging()
{
    ImGuiIO& io = ImGui::GetIO();
    
    // 드래그 시작 - 드래그 임계값 체크
    if (!isDraggingNode && ImGui::IsMouseDragging(0, 5.0f))  // 5픽셀 이상 드래그 시에만
    {
        ImVec2 mousePos(io.MousePos.x, io.MousePos.y);
        StateNode* node = GetNodeAtPosition(DirectX::XMFLOAT2(mousePos.x, mousePos.y));
        
        if (node)
        {
            isDraggingNode = true;
            draggedNode = node;
            dragStartPos = node->position;
        }
    }
    
    // 드래그 중
    if (isDraggingNode && ImGui::IsMouseDragging(0))
    {
        if (draggedNode)
        {
            ImVec2 delta = io.MouseDelta;
            draggedNode->position.x += delta.x;
            draggedNode->position.y += delta.y;
        }
    }
    
    // 드래그 종료
    if (ImGui::IsMouseReleased(0))
    {
        isDraggingNode = false;
        draggedNode = nullptr;
    }
}

void AnimatorWindow::HandleNodeSelection()
{
    ImGuiIO& io = ImGui::GetIO();
    
    // ESC 키로 전환 생성 모드 취소
    if (isCreatingTransition && ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        isCreatingTransition = false;
        transitionSourceState = nullptr;
        return;
    }
    
    // 캔버스 영역 체크 - InvisibleButton이 클릭되었는지 확인
    bool canvasClicked = ImGui::IsItemClicked(0);
    
    // 전환 생성 모드 처리
    if (isCreatingTransition && canvasClicked && !isDraggingNode)
    {
        ImVec2 mousePos(io.MousePos.x, io.MousePos.y);
        StateNode* node = GetNodeAtPosition(DirectX::XMFLOAT2(mousePos.x, mousePos.y));
        
        if (node && node->state != transitionSourceState)
        {
            // 전환 생성
            CreateTransition(transitionSourceState, node->state);
            isCreatingTransition = false;
            transitionSourceState = nullptr;
            return;
        }
        else if (!node)
        {
            // 빈 곳 클릭 시 전환 생성 모드 취소
            isCreatingTransition = false;
            transitionSourceState = nullptr;
        }
    }
    
    // 캔버스 내에서 마우스 클릭 시 노드 또는 트랜지션 선택 (드래그가 아닌 경우에만)
    if (canvasClicked && !isDraggingNode && !isCreatingTransition)
    {
        ImVec2 mousePos(io.MousePos.x, io.MousePos.y);
        
        // 먼저 트랜지션 클릭 확인
        AnimationTransition* clickedTransition = GetTransitionAtPosition(DirectX::XMFLOAT2(mousePos.x, mousePos.y));
        
        if (clickedTransition)
        {
            // 트랜지션 선택
            selectedTransition = clickedTransition;
            selectedState = nullptr;
        }
        else
        {
            // 노드 클릭 확인
            StateNode* node = GetNodeAtPosition(DirectX::XMFLOAT2(mousePos.x, mousePos.y));
            
            if (node)
            {
                // 클릭한 노드 선택
                selectedState = node->state;
                selectedTransition = nullptr;
            }
            else
            {
                // 빈 곳을 클릭하면 선택 해제
                selectedState = nullptr;
                selectedTransition = nullptr;
            }
        }
    }
}

void AnimatorWindow::HandleContextMenu()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    
    // 캔버스 영역에서 우클릭
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        // 우클릭한 위치 저장
        contextMenuPos = ImVec2(io.MousePos.x, io.MousePos.y);
        
        // 노드 위에서 우클릭했는지 확인
        StateNode* node = GetNodeAtPosition(DirectX::XMFLOAT2(contextMenuPos.x, contextMenuPos.y));
        
        if (node)
        {
            // 노드 컨텍스트 메뉴
            contextMenuNode = node;
            ImGui::OpenPopup("StateNodeContextMenu");
        }
        else
        {
            // 캔버스 컨텍스트 메뉴
            contextMenuNode = nullptr;
            ImGui::OpenPopup("StateGraphContextMenu");
        }
    }
    
    // 노드 컨텍스트 메뉴 팝업
    if (ImGui::BeginPopup("StateNodeContextMenu"))
    {
        if (contextMenuNode)
        {
            std::string stateName = WStringToString(contextMenuNode->state->GetName());
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "State: %s", stateName.c_str());
            ImGui::Separator();
            
            // Make Transition 옵션
            if (ImGui::MenuItem("Make Transition"))
            {
                isCreatingTransition = true;
                transitionSourceState = contextMenuNode->state;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::Separator();
            
            // Set as Default
            auto* stateMachine = currentController->GetStateMachine();
            bool isDefault = (stateMachine && contextMenuNode->state == stateMachine->GetDefaultState());
            
            if (!isDefault)
            {
                if (ImGui::MenuItem("Set as Default"))
                {
                    SetDefaultState(contextMenuNode->state);
                    ImGui::CloseCurrentPopup();
                }
            }
            
            ImGui::Separator();
            
            // Delete State
            if (ImGui::MenuItem("Delete State"))
            {
                DeleteState(contextMenuNode->state);
                if (selectedState == contextMenuNode->state)
                    selectedState = nullptr;
                contextMenuNode = nullptr;
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::EndPopup();
    }
    
    // 캔버스 컨텍스트 메뉴 팝업
    if (ImGui::BeginPopup("StateGraphContextMenu"))
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Create New State");
        ImGui::Separator();
        
        ImGui::Text("State Name:");
        ImGui::SetNextItemWidth(200);
        ImGui::InputText("##ContextStateName", stateNameBuffer, sizeof(stateNameBuffer));
        
        ImGui::Spacing();
        
        if (ImGui::Button("Create", ImVec2(100, 0)) || (ImGui::IsItemFocused() == false && ImGui::IsKeyPressed(ImGuiKey_Enter)))
        {
            if (strlen(stateNameBuffer) > 0)
            {
                std::string nameStr(stateNameBuffer);
                std::wstring name(nameStr.begin(), nameStr.end());
                
                // 우클릭한 위치를 캔버스 좌표로 변환
                DirectX::XMFLOAT2 localPos(contextMenuPos.x - canvasPos.x, 
                                          contextMenuPos.y - canvasPos.y);
                DirectX::XMFLOAT2 canvasCoord = ScreenToCanvas(localPos);
                
                CreateNewState(name, canvasCoord);
                memset(stateNameBuffer, 0, sizeof(stateNameBuffer));
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(100, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            memset(stateNameBuffer, 0, sizeof(stateNameBuffer));
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void AnimatorWindow::AddParameter(const std::wstring& name, int typeIndex)
{
    if (!currentController)
        return;
    
    AnimatorParameterType type = AnimatorParameterType::Float;
    switch (typeIndex)
    {
    case 0: type = AnimatorParameterType::Float; break;
    case 1: type = AnimatorParameterType::Int; break;
    case 2: type = AnimatorParameterType::Bool; break;
    case 3: type = AnimatorParameterType::Trigger; break;
    }
    
    currentController->AddParameter(name, type);
}

void AnimatorWindow::RemoveParameter(const std::wstring& name)
{
    if (!currentController)
        return;
    
    currentController->RemoveParameter(name);
}

void AnimatorWindow::CreateNewState(const std::wstring& name, DirectX::XMFLOAT2 position)
{
    if (!currentController)
        return;
    
    auto* stateMachine = currentController->GetStateMachine();
    if (!stateMachine)
        return;
    
    auto* newState = new AnimationState(name);
    stateMachine->AddState(newState);
    stateNodes.emplace_back(newState, position);
    
    // 첫 번째 상태면 자동으로 기본 상태로 설정
    if (stateNodes.size() == 1)
    {
        SetDefaultState(newState);
    }
}

void AnimatorWindow::DeleteState(AnimationState* state)
{
    if (!currentController || !state)
        return;
    
    auto* stateMachine = currentController->GetStateMachine();
    if (!stateMachine)
        return;
    
    stateMachine->RemoveState(state->GetName());
    
    // 노드 목록에서도 제거
    auto it = std::find_if(stateNodes.begin(), stateNodes.end(),
        [state](const StateNode& node) { return node.state == state; });
    
    if (it != stateNodes.end())
        stateNodes.erase(it);
    
    if (selectedState == state)
        selectedState = nullptr;
}

void AnimatorWindow::SetDefaultState(AnimationState* state)
{
    if (!currentController || !state)
        return;
    
    auto* stateMachine = currentController->GetStateMachine();
    if (stateMachine)
    {
        stateMachine->SetDefaultState(state);
    }
}

void AnimatorWindow::CreateTransition(AnimationState* from, AnimationState* to)
{
    if (!from || !to)
        return;
    
    // 자기 자신으로의 전환 체크
    if (from == to)
    {
        showErrorMessage = true;
        showSuccessMessage = false;
        statusMessage = "Cannot create transition to same state!";
        return;
    }
    
    // 중복 트랜지션 체크
    const auto& existingTransitions = from->GetTransitions();
    for (auto* transition : existingTransitions)
    {
        if (transition->GetDestinationState() == to)
        {
            // 이미 동일한 트랜지션이 존재함
            showErrorMessage = true;
            showSuccessMessage = false;
            std::string fromName = WStringToString(from->GetName());
            std::string toName = WStringToString(to->GetName());
            statusMessage = "Transition '" + fromName + "' -> '" + toName + "' already exists!";
            return;
        }
    }
    
    auto* transition = new AnimationTransition(from, to);
    from->AddTransition(transition);
    
    showSuccessMessage = true;
    showErrorMessage = false;
    std::string fromName = WStringToString(from->GetName());
    std::string toName = WStringToString(to->GetName());
    statusMessage = "Transition created: '" + fromName + "' -> '" + toName + "'";
}

void AnimatorWindow::OpenController(const std::wstring& path)
{
    // 기존 선택 상태 초기화 (댕글링 포인터 방지)
    selectedState = nullptr;
    selectedTransition = nullptr;
    
    // 기존 노드 목록 클리어
    stateNodes.clear();
    
    currentController = std::make_shared<AnimatorController>();
    if (currentController->Load(path))
    {
        currentControllerPath = path;
        
        // 파일명 추출
        size_t lastSlash = path.find_last_of(L"/\\");
        if (lastSlash != std::wstring::npos)
        {
            currentControllerName = path.substr(lastSlash + 1);
            size_t lastDot = currentControllerName.find_last_of(L'.');
            if (lastDot != std::wstring::npos)
            {
                currentControllerName = currentControllerName.substr(0, lastDot);
            }
        }
        
        // State Machine에서 모든 상태 가져오기
        auto* stateMachine = currentController->GetStateMachine();
        if (stateMachine)
        {
            const auto& states = stateMachine->GetAllStates();
            
            // 노드 위치 로드 시도
            std::wstring layoutPath = path;
            size_t extPos = layoutPath.find_last_of(L'.');
            if (extPos != std::wstring::npos)
            {
                layoutPath = layoutPath.substr(0, extPos) + L".layout";
            }
            
            LoadNodePositions(layoutPath);
            
            // LoadNodePositions에서 제대로 로드되지 않았거나 일부 상태가 누락된 경우
            // 누락된 상태만 추가 (기존 위치는 유지)
            if (stateNodes.size() < states.size())
            {
                float offsetX = 100.0f;
                float offsetY = 100.0f;
                
                // 기존 노드들이 있으면 그 다음 위치부터 시작
                if (!stateNodes.empty())
                {
                    // 가장 오른쪽/아래 노드 찾기
                    for (const auto& node : stateNodes)
                    {
                        if (node.position.x + 200.0f > offsetX)
                            offsetX = node.position.x + 200.0f;
                        if (node.position.y > offsetY)
                            offsetY = node.position.y;
                    }
                }
                
                // 로드되지 않은 상태들 추가
                for (auto* state : states)
                {
                    bool found = false;
                    for (const auto& node : stateNodes)
                    {
                        if (node.state == state)
                        {
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found)
                    {
                        stateNodes.emplace_back(state, DirectX::XMFLOAT2(offsetX, offsetY));
                        offsetX += 200.0f;
                        if (offsetX > 800.0f)
                        {
                            offsetX = 100.0f;
                            offsetY += 150.0f;
                        }
                    }
                }
            }
        }
        
        showSuccessMessage = true;
        showErrorMessage = false;
        statusMessage = "Controller loaded: " + WStringToString(currentControllerName);
    }
    else
    {
        showErrorMessage = true;
        showSuccessMessage = false;
        statusMessage = "Failed to load controller";
    }
}

void AnimatorWindow::SaveController()
{
    if (!currentController)
        return;
    
    showSuccessMessage = false;
    showErrorMessage = false;
    
    if (currentControllerPath.empty())
    {
        // 새 파일 경로 생성
        if (currentControllerName.empty())
        {
            showErrorMessage = true;
            statusMessage = "Controller name is empty";
            return;
        }
        
        currentControllerPath = L"Assets/Controllers/" + currentControllerName + L".controller";
    }
    
    // Controllers 폴더 생성
    std::wstring dirPath = L"Assets/Controllers";
    std::filesystem::create_directories(dirPath);
    
    if (currentController->Save(currentControllerPath))
    {
        // 노드 위치 저장
        std::wstring layoutPath = currentControllerPath;
        size_t extPos = layoutPath.find_last_of(L'.');
        if (extPos != std::wstring::npos)
        {
            layoutPath = layoutPath.substr(0, extPos) + L".layout";
        }
        SaveNodePositions(layoutPath);
        
        // Resources 리로드
        Resources::LoadAllAssetsFromFolder(L"Assets");
        
        showSuccessMessage = true;
        statusMessage = "Controller saved: " + WStringToString(currentControllerName);
    }
    else
    {
        showErrorMessage = true;
        statusMessage = "Failed to save controller";
    }
}

void AnimatorWindow::SaveNodePositions(const std::wstring& path)
{
    if (!currentController)
        return;
    
    try
    {
        json data;
        json nodesArray = json::array();
        
        for (const auto& node : stateNodes)
        {
            json nodeData;
            std::string stateName = WStringToString(node.state->GetName());
            nodeData["state"] = stateName;
            nodeData["x"] = node.position.x;
            nodeData["y"] = node.position.y;
            nodesArray.push_back(nodeData);
        }
        
        data["nodes"] = nodesArray;
        
        std::ofstream file(path);
        if (file.is_open())
        {
            file << data.dump(4);
        }
    }
    catch (...)
    {
        // 레이아웃 저장 실패는 치명적이지 않음
    }
}

void AnimatorWindow::LoadNodePositions(const std::wstring& path)
{
    if (!currentController)
        return;
    
    try
    {
        std::ifstream file(path);
        if (!file.is_open())
            return;
        
        json data;
        file >> data;
        
        if (!data.contains("nodes"))
            return;
        
        auto* stateMachine = currentController->GetStateMachine();
        if (!stateMachine)
            return;
        
        stateNodes.clear();
        
        for (const auto& nodeData : data["nodes"])
        {
            std::string stateNameUtf8 = nodeData["state"];
            std::wstring stateName(stateNameUtf8.begin(), stateNameUtf8.end());
            
            auto* state = stateMachine->GetState(stateName);
            if (state)
            {
                float x = nodeData["x"];
                float y = nodeData["y"];
                stateNodes.emplace_back(state, DirectX::XMFLOAT2(x, y));
            }
        }
    }
    catch (...)
    {
        // 레이아웃 로드 실패는 치명적이지 않음
        stateNodes.clear();
    }
}

void AnimatorWindow::DeleteTransition(AnimationTransition* transition)
{
    if (!transition)
        return;
    
    auto* sourceState = transition->GetSourceState();
    auto* destState = transition->GetDestinationState();
    
    // 상태 메시지 준비 (삭제 전에 정보 저장)
    std::string fromName = sourceState ? WStringToString(sourceState->GetName()) : "Unknown";
    std::string toName = destState ? WStringToString(destState->GetName()) : "Unknown";
    
    // 선택된 트랜지션이 삭제되는 경우 선택 해제 (삭제 전에!)
    if (selectedTransition == transition)
    {
        selectedTransition = nullptr;
    }
    
    // 트랜지션 삭제 (실제 메모리 해제는 RemoveTransition 내부에서 발생)
    if (sourceState)
    {
        sourceState->RemoveTransition(transition);
    }
    
    // 상태 메시지 표시
    showSuccessMessage = true;
    showErrorMessage = false;
    statusMessage = "Transition deleted: '" + fromName + "' -> '" + toName + "'";
}

void AnimatorWindow::AddConditionToTransition(AnimationTransition* transition)
{
    if (!transition)
        return;
    
    const auto& params = currentController->GetParameters().GetAllParameters();
    std::vector<std::wstring> paramNames;
    for (const auto& pair : params)
    {
        paramNames.push_back(pair.first);
    }
    
    if (selectedParameterIndex >= paramNames.size())
        return;
    
    std::wstring paramName = paramNames[selectedParameterIndex];
    
    TransitionConditionMode mode = TransitionConditionMode::If;
    switch (selectedConditionMode)
    {
    case 0: mode = TransitionConditionMode::If; break;
    case 1: mode = TransitionConditionMode::IfNot; break;
    case 2: mode = TransitionConditionMode::Greater; break;
    case 3: mode = TransitionConditionMode::Less; break;
    case 4: mode = TransitionConditionMode::Equals; break;
    case 5: mode = TransitionConditionMode::NotEquals; break;
    }
    
    transition->AddCondition(paramName, mode, conditionThreshold);
}

void AnimatorWindow::RemoveConditionFromTransition(AnimationTransition* transition, int index)
{
    if (!transition || index < 0)
        return;
    
    transition->RemoveCondition(index);
}

std::vector<std::shared_ptr<AnimationClip>> AnimatorWindow::GetAllAnimationClips()
{
    std::vector<std::shared_ptr<AnimationClip>> clips;
    
    // Assets/Animations 폴더에서 모든 .anim 파일 찾기
    try
    {
        std::filesystem::path animPath = std::filesystem::current_path() / L"Assets" / L"Animations";
        if (std::filesystem::exists(animPath))
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(animPath))
            {
                if (entry.is_regular_file())
                {
                    std::wstring ext = entry.path().extension().wstring();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
                    
                    if (ext == L".anim")
                    {
                        // 파일명 추출 (확장자 제외)
                        std::wstring stem = entry.path().stem().wstring();
                        
                        // Resources 캐시에서 가져오기 시도
                        auto clip = Resources::Get<AnimationClip>(stem);
                        
                        // 캐시에 없으면 직접 로드
                        if (!clip)
                        {
                            clip = Resources::Load<AnimationClip>(stem, entry.path().wstring());
                        }
                        
                        if (clip)
                        {
                            clips.push_back(clip);
                        }
                    }
                }
            }
        }
    }
    catch (...)
    {
    }
    
    return clips;
}

void AnimatorWindow::ScanControllerFolder()
{
    controllerFiles.clear();
    
    try
    {
        std::filesystem::path controllerPath = std::filesystem::current_path() / L"Assets" / L"Controllers";
        
        // 폴더가 없으면 생성
        if (!std::filesystem::exists(controllerPath))
        {
            std::filesystem::create_directories(controllerPath);
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(controllerPath))
        {
            if (!entry.is_regular_file())
                continue;
            
            std::wstring ext = entry.path().extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
            
            if (ext == L".controller")
            {
                ControllerFileInfo info;
                info.filename = entry.path().filename().wstring();
                info.fullPath = entry.path().wstring();
                info.stem = entry.path().stem().wstring();
                
                controllerFiles.push_back(info);
            }
        }
    }
    catch (...)
    {
    }
}

void AnimatorWindow::OpenControllerFolder()
{
    // Assets/Controllers 폴더를 윈도우 탐색기로 열기
    std::filesystem::path controllerPath = std::filesystem::current_path() / L"Assets" / L"Controllers";
    
    // 폴더가 없으면 생성
    if (!std::filesystem::exists(controllerPath))
    {
        std::filesystem::create_directories(controllerPath);
    }
    
    // 윈도우 탐색기로 폴더 열기
    ShellExecuteW(NULL, L"open", controllerPath.wstring().c_str(), NULL, NULL, SW_SHOW);
}

void AnimatorWindow::NewController()
{
    currentController = std::make_shared<AnimatorController>();
    currentControllerPath.clear();
    stateNodes.clear();
    selectedState = nullptr;
    selectedTransition = nullptr;
}

std::wstring AnimatorWindow::CharToWString(const char* str)
{
    if (str == nullptr || strlen(str) == 0)
        return L"";

    int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    if (sizeNeeded <= 0)
        return L"";
    
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_ACP, 0, str, -1, &wstrTo[0], sizeNeeded);
    
    if (!wstrTo.empty() && wstrTo.back() == L'\0')
        wstrTo.pop_back();
    
    return wstrTo;
}

std::string AnimatorWindow::WStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
        return "";

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (sizeNeeded <= 0)
        return "";

    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &strTo[0], sizeNeeded, NULL, NULL);

    if (!strTo.empty() && strTo.back() == '\0')
        strTo.pop_back();

    return strTo;
}

DirectX::XMFLOAT2 AnimatorWindow::ScreenToCanvas(DirectX::XMFLOAT2 screenPos)
{
    return DirectX::XMFLOAT2(
        (screenPos.x - canvasOffset.x) / canvasZoom,
        (screenPos.y - canvasOffset.y) / canvasZoom
    );
}

DirectX::XMFLOAT2 AnimatorWindow::CanvasToScreen(DirectX::XMFLOAT2 canvasPos)
{
    return DirectX::XMFLOAT2(
        canvasPos.x * canvasZoom + canvasOffset.x,
        canvasPos.y * canvasZoom + canvasOffset.y
    );
}

bool AnimatorWindow::IsPointInRect(DirectX::XMFLOAT2 point, DirectX::XMFLOAT2 rectMin, DirectX::XMFLOAT2 rectMax)
{
    return point.x >= rectMin.x && point.x <= rectMax.x &&
           point.y >= rectMin.y && point.y <= rectMax.y;
}

StateNode* AnimatorWindow::GetNodeAtPosition(DirectX::XMFLOAT2 position)
{
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    
    for (auto& node : stateNodes)
    {
        DirectX::XMFLOAT2 screenPosXM = CanvasToScreen(node.position);
        DirectX::XMFLOAT2 rectMin(screenPosXM.x + canvasPos.x, screenPosXM.y + canvasPos.y);
        DirectX::XMFLOAT2 rectMax(rectMin.x + 150, rectMin.y + 80);
        
        if (IsPointInRect(position, rectMin, rectMax))
        {
            return &node;
        }
    }
    
    return nullptr;
}

AnimationTransition* AnimatorWindow::GetTransitionAtPosition(DirectX::XMFLOAT2 position)
{
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    const float clickThreshold = 10.0f; // 10픽셀 이내면 클릭으로 인정
    
    const float nodeWidth = 150.0f;
    const float nodeHeight = 80.0f;
    
    for (auto& node : stateNodes)
    {
        for (auto* transition : node.state->GetTransitions())
        {
            // 출발/도착 노드 찾기
            StateNode* fromNode = nullptr;
            StateNode* toNode = nullptr;
            
            for (auto& n : stateNodes)
            {
                if (n.state == transition->GetSourceState())
                    fromNode = &n;
                if (n.state == transition->GetDestinationState())
                    toNode = &n;
            }
            
            if (!fromNode || !toNode)
                continue;
            
            // 노드 중앙점 계산 (캔버스 좌표)
            DirectX::XMFLOAT2 fromCenter(fromNode->position.x + nodeWidth * 0.5f, fromNode->position.y + nodeHeight * 0.5f);
            DirectX::XMFLOAT2 toCenter(toNode->position.x + nodeWidth * 0.5f, toNode->position.y + nodeHeight * 0.5f);
            
            // 역방향 트랜지션이 있는지 확인 (양방향 트랜지션 감지)
            bool hasReverseTransition = false;
            for (auto* reverseTransition : toNode->state->GetTransitions())
            {
                if (reverseTransition && reverseTransition->GetDestinationState() == fromNode->state)
                {
                    hasReverseTransition = true;
                    break;
                }
            }
            
            // 방향 벡터 계산
            float dx = toCenter.x - fromCenter.x;
            float dy = toCenter.y - fromCenter.y;
            float length = sqrtf(dx * dx + dy * dy);
            
            if (length < 1.0f)
                continue;
            
            // 정규화된 방향 벡터
            float dirX = dx / length;
            float dirY = dy / length;
            
            // 노드 경계까지의 교점 계산 (박스의 가장자리)
            auto calculateBoxIntersection = [](DirectX::XMFLOAT2 center, float width, float height, float dirX, float dirY) -> DirectX::XMFLOAT2 {
                // 박스의 반 크기
                float halfW = width * 0.5f;
                float halfH = height * 0.5f;
                
                // 각 방향에 대한 교점 시간(t) 계산
                float tX = (dirX > 0) ? halfW / dirX : (dirX < 0 ? -halfW / dirX : FLT_MAX);
                float tY = (dirY > 0) ? halfH / dirY : (dirY < 0 ? -halfH / dirY : FLT_MAX);
                
                // 더 작은 t 값이 실제 교점
                float t = (tX < tY) ? tX : tY;
                
                return DirectX::XMFLOAT2(center.x + dirX * t, center.y + dirY * t);
            };
            
            // 출발점과 도착점을 노드 경계로 조정
            DirectX::XMFLOAT2 fromEdge = calculateBoxIntersection(fromCenter, nodeWidth, nodeHeight, dirX, dirY);
            DirectX::XMFLOAT2 toEdge = calculateBoxIntersection(toCenter, nodeWidth, nodeHeight, -dirX, -dirY);
            
            // 캔버스 좌표를 화면 좌표로 변환
            DirectX::XMFLOAT2 fromPosScreen = CanvasToScreen(fromEdge);
            DirectX::XMFLOAT2 toPosScreen = CanvasToScreen(toEdge);
            
            ImVec2 fromPos(fromPosScreen.x + canvasPos.x, fromPosScreen.y + canvasPos.y);
            ImVec2 toPos(toPosScreen.x + canvasPos.x, toPosScreen.y + canvasPos.y);
            
            float distance;
            
            if (hasReverseTransition)
            {
                // 베지어 곡선의 경우 - 곡선 상의 여러 점을 샘플링하여 거리 계산
                float perpX = -dirY;
                float perpY = dirX;
                float offsetAmount = 20.0f; // 오프셋 크기
        
                // 베지어 곡선을 위한 제어점 계산
                float midX = (fromPos.x + toPos.x) * 0.5f;
                float midY = (fromPos.y + toPos.y) * 0.5f;
        
                DirectX::XMFLOAT2 cp1(fromPos.x * 0.75f + midX * 0.25f + perpX * offsetAmount,
                                      fromPos.y * 0.75f + midY * 0.25f + perpY * offsetAmount);
        
                DirectX::XMFLOAT2 cp2(toPos.x * 0.75f + midX * 0.25f + perpX * offsetAmount,
                                      toPos.y * 0.75f + midY * 0.25f + perpY * offsetAmount);
        
                // 베지어 곡선 상의 여러 점을 샘플링
                distance = FLT_MAX;
                const int samples = 20; // 샘플링 수
        
                for (int i = 0; i <= samples; ++i)
                {
                    float t = static_cast<float>(i) / samples;
                    float invT = 1.0f - t;
        
                    // 3차 베지어 곡선 공식
                    float x = invT * invT * invT * fromPos.x +
                             3.0f * invT * invT * t * cp1.x +
                             3.0f * invT * t * t * cp2.x +
                             t * t * t * toPos.x;
        
                    float y = invT * invT * invT * fromPos.y +
                             3.0f * invT * invT * t * cp1.y +
                             3.0f * invT * t * t * cp2.y +
                             t * t * t * toPos.y;
        
                    float distX = position.x - x;
                    float distY = position.y - y;
                    float pointDist = sqrtf(distX * distX + distY * distY);
        
                    if (pointDist < distance)
                    {
                        distance = pointDist;
                    }
                }
            }
            else
            {
                // 직선의 경우 - 기존 방식
                distance = DistancePointToLine(position, fromPosScreen, toPosScreen);
            }
            
            if (distance < clickThreshold)
            {
                return transition;
            }
        }
    }
    
    return nullptr;
}

float AnimatorWindow::DistancePointToLine(DirectX::XMFLOAT2 point, DirectX::XMFLOAT2 lineStart, DirectX::XMFLOAT2 lineEnd)
{
    float dx = lineEnd.x - lineStart.x;
    float dy = lineEnd.y - lineStart.y;
    
    if (dx == 0 && dy == 0)
    {
        // 선분이 점인 경우
        float pdx = point.x - lineStart.x;
        float pdy = point.y - lineStart.y;
        return sqrtf(pdx * pdx + pdy * pdy);
    }
    
    // 선분의 길이의 제곱
    float lengthSquared = dx * dx + dy * dy;
    
    // 점을 선분에 투영했을 때의 t 값 (0~1 사이)
    float t = ((point.x - lineStart.x) * dx + (point.y - lineStart.y) * dy) / lengthSquared;
    
    // t를 0~1로 클램프 (선분 밖으로 나가지 않도록)
    t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
    
    // 선분 위의 가장 가까운 점
    float closestX = lineStart.x + t * dx;
    float closestY = lineStart.y + t * dy;
    
    // 점과 가장 가까운 점 사이의 거리
    float distX = point.x - closestX;
    float distY = point.y - closestY;
    
    return sqrtf(distX * distX + distY * distY);
}

void AnimatorWindow::OpenControllerFile(const std::wstring& filePath)
{
    // 로그 추가 - 디버깅용
    ConsoleWindow::Log("OpenControllerFile called", LogType::Info);
    
    // 창이 닫혀있으면 열기
    SetOpen(true);
    
    // 상태 확인 로그
    if (IsOpen())
    {
        ConsoleWindow::Log("AnimatorWindow is now open", LogType::Info);
    }
    else
    {
        ConsoleWindow::Log("AnimatorWindow failed to open", LogType::Error);
    }
    
    // 컨트롤러 파일 열기
    OpenController(filePath);
}
