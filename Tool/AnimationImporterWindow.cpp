#include "AnimationImporterWindow.h"
#include "AnimationImporter.h"
#include "Resource/Resources.h"
#include <ImGui/imgui.h>
#include <Windows.h>
#include <commdlg.h>
#include <DirectXTex.h>
#include <d3d11.h>
#include <shellapi.h>

AnimationImporterWindow::AnimationImporterWindow(ID3D11Device* device, ID3D11DeviceContext* context)
    : d3dDevice(device)
    , d3dContext(context)
    , isOpen(true)
    , showSuccessMessage(false)
    , showErrorMessage(false)
{
}

AnimationImporterWindow::~AnimationImporterWindow()
{

}

void AnimationImporterWindow::Render()
{
    if (!isOpen)
        return;
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Animation Importer Tool", &isOpen);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Import Animation");
    ImGui::Separator();
    ImGui::Spacing();
	ImGui::End();
}