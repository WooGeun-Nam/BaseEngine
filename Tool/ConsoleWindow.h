#pragma once
#include "EditorWindow.h"
#include <string>
#include <vector>
#include <DirectXMath.h>

enum class LogType
{
    Info,
    Warning,
    Error
};

struct LogMessage
{
    std::string message;
    LogType type;
    DirectX::XMFLOAT4 color;
};

class ConsoleWindow : public EditorWindow
{
public:
    ConsoleWindow();

    void Render() override;

    // 로그 추가
    static void Log(const std::string& message, LogType type = LogType::Info);
    static void Clear();

private:
    void RenderToolbar();
    void RenderLogList();

    static std::vector<LogMessage> logs;
    static bool autoScroll;
    
    bool showInfo = true;
    bool showWarning = true;
    bool showError = true;

    char searchBuffer[256] = {0};
    
    // Buffer for displaying all logs as plain text
    std::string logTextBuffer;
    void UpdateLogTextBuffer();
};
