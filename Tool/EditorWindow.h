#pragma once
#include <string>

// 모든 에디터 창의 기본 클래스
class EditorWindow
{
public:
    EditorWindow(const std::string& windowName, bool openByDefault = false);
    virtual ~EditorWindow() = default;

    // 순수 가상 함수 - 자식 클래스에서 반드시 구현
    virtual void Render() = 0;

    // 윈도우 상태
    bool IsOpen() const { return isOpen; }
    void SetOpen(bool open) { isOpen = open; }
    void ToggleOpen() { isOpen = !isOpen; }

    // 윈도우 이름
    const std::string& GetWindowName() const { return windowName; }

    // 메뉴바에 표시 여부
    virtual bool ShowInMenuBar() const { return true; }

    // 업데이트 우선순위 (낮을수록 먼저 실행)
    virtual int GetRenderOrder() const { return 0; }

protected:
    bool isOpen;
    std::string windowName;
};
