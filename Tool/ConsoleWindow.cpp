#include "ConsoleWindow.h"
#include <ImGui/imgui.h>

std::vector<LogMessage> ConsoleWindow::logs;
bool ConsoleWindow::autoScroll = true;

ConsoleWindow::ConsoleWindow()
    : EditorWindow("Console", true)
{
    // 초기 테스트 로그
    Log("Console initialized", LogType::Info);
}

void ConsoleWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(800, 200), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(windowName.c_str(), &isOpen))
    {
        RenderToolbar();
        ImGui::Separator();
        RenderLogList();
    }

    ImGui::End();
}

void ConsoleWindow::RenderToolbar()
{
    // Clear 버튼
    if (ImGui::Button("Clear"))
    {
        Clear();
    }

    ImGui::SameLine();

    // 필터 체크박스
    ImGui::Checkbox("Info", &showInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &showWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &showError);

    ImGui::SameLine();

    // 검색 필드
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("##Search", searchBuffer, sizeof(searchBuffer));

    ImGui::SameLine();

    // Auto Scroll 체크박스
    ImGui::Checkbox("Auto Scroll", &autoScroll);
}

void ConsoleWindow::UpdateLogTextBuffer()
{
    logTextBuffer.clear();
    std::string searchText(searchBuffer);
    
    for (const auto& log : logs)
    {
        // Filter
        if ((log.type == LogType::Info && !showInfo) ||
            (log.type == LogType::Warning && !showWarning) ||
            (log.type == LogType::Error && !showError))
        {
            continue;
        }

        // Search filter
        if (!searchText.empty() && log.message.find(searchText) == std::string::npos)
        {
            continue;
        }

        logTextBuffer += log.message + "\n";
    }
}

void ConsoleWindow::RenderLogList()
{
    // Update the text buffer with current filters
    UpdateLogTextBuffer();
    
    // Use InputTextMultiline for Unity-like console
    // This allows full text selection, Ctrl+A, drag selection, and Ctrl+C
    ImGui::InputTextMultiline(
        "##ConsoleLog",
        const_cast<char*>(logTextBuffer.c_str()),
        logTextBuffer.size() + 1,
        ImVec2(-1, -1),
        ImGuiInputTextFlags_ReadOnly
    );
    
    // Auto Scroll to bottom
    if (autoScroll && logs.size() > 0)
    {
        ImGui::SetScrollHereY(1.0f);
    }
}

void ConsoleWindow::Log(const std::string& message, LogType type)
{
    LogMessage log;
    log.message = message;
    log.type = type;

    // 타입에 따른 색상 설정 (현재는 사용하지 않지만 나중을 위해 보존)
    switch (type)
    {
    case LogType::Info:
        log.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // 흰색
        break;
    case LogType::Warning:
        log.color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 노란색
        break;
    case LogType::Error:
        log.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색
        break;
    }

    logs.push_back(log);

    // 로그가 너무 많으면 오래된 것부터 삭제
    if (logs.size() > 1000)
    {
        logs.erase(logs.begin());
    }
}

void ConsoleWindow::Clear()
{
    logs.clear();
}
