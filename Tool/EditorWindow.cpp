#include "EditorWindow.h"

EditorWindow::EditorWindow(const std::string& windowName, bool openByDefault)
    : windowName(windowName)
    , isOpen(openByDefault)
{
}
