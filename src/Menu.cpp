#include "Menu.h"

Menu::Menu()
{
    elements.bIsShown = bShowMenu;
    elements.Menu = std::bind(&Menu::Draw, this);
    elements.Shroud = std::bind(&Menu::SHROUD, this);
    elements.Hud = std::bind(&Menu::HUD, this);
}

void Menu::Draw()
{
    if (bShowMenu)
        MainMenu();

    if (bShowDemoWindow && bShowMenu)
        ImGui::ShowDemoWindow();

    if (bShowStyleEditor && bShowMenu)
        ImGui::ShowStyleEditor();
}

void Menu::MainMenu()
{
    static ImVec2 MenuSize = ImVec2(800, 600);
    static ImVec2 ScreenSize = g_dxWindow->GetScreenSize();
    ImVec2 start = { ScreenSize.x / 2 - MenuSize.x / 2, ScreenSize.y / 2 - MenuSize.y / 2 };
    ImGui::SetNextWindowPos(start);
    ImGui::SetNextWindowSize(MenuSize);
    if (!ImGui::Begin("(DX11) ImGui External Base", &bShowMenu, 96))
    {
        ImGui::End();
        return;
    }            
    
    ImGui::Text("BASE MENU (PREVIEW)");
    ImGui::Text("BUILD VERSION: v1.0.0");
    ImGui::Text("BUILD DATE: 8/11/2024");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    static int BytesPerLine{ 4 };
    static int BytesSpacing{ 2 };
    static unsigned int MemScanReadSize{ 0x1000 };
    static unsigned int MemScanStarAddress{ 0x0 };
    ImGui::InputInt("##MEMSCAN_START", (int*)&MemScanStarAddress, 0, 0, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::SameLine();
    ImGui::InputInt("##MEMSCAN_SIZE", (int*)&MemScanReadSize, 0, 0, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::BeginChild("MemoryScanner", ImVec2(ImGui::GetContentRegionAvail()), ImGuiChildFlags_Border);
    {
        int pad{ 0 };
        for (int i = 0; i < MemScanReadSize; i += BytesPerLine)
        {
            static unsigned long long base = reinterpret_cast<unsigned long long>(GetModuleHandle(0));
            if (!base)
            {
                ImGui::Text("Failed to get module base address.");
                break;
            }
        
            ImGui::Text("%p: ", base + i);
            ImGui::SameLine();
            for (int j = 0; j < BytesPerLine; j++)
            {
                unsigned long long addr = (base + i + j);
                if (addr > 0)
                    ImGui::Text("0x%02X ", *(unsigned __int8*)addr);
                else
                    ImGui::Text("?? ");
        
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void Menu::SHROUD()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(g_dxWindow->GetScreenSize());
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    if (!ImGui::Begin("##SHROUDWINDOW", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs)) 
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}

void Menu::HUD()
{
    ImVec2 draw_pos = g_dxWindow->GetCloneWindowPos();
    ImVec2 draw_size = g_dxWindow->GetCloneWindowSize();
    ImGui::SetNextWindowPos(draw_pos);
    ImGui::SetNextWindowSize(draw_size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(IM_COL32_RED));
    if (!ImGui::Begin("##HUDWINDOW", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs))
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();


    ImDrawList* ImDraw = ImGui::GetWindowDrawList();
    auto center = ImVec2({ draw_size.x * .5f, draw_size.y * .5f });
    auto top_center = ImVec2({ draw_size.x * .5f, draw_size.y * 0.0f });
    GUI::DrawTextCentered(top_center + draw_pos, IM_COL32_RED, "https://github.com/NightFyre/DX11-ImGui-External-Base", 12.f);

    ImGui::End();
}

DxWindow::SOverlay Menu::GetOverlay() { return elements; }

void Menu::UpdateOverlayViewState(bool bState) { elements.bIsShown = bState; }

void GUI::TextCentered(const char* pText)
{
    ImVec2 textSize = ImGui::CalcTextSize(pText);
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 textPos = ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f);
    ImGui::SetCursorPos(textPos);
    ImGui::Text("%s", pText);
}

//  @ATTN: max buffer is 256chars
void GUI::TextCenteredf(const char* pText, ...)
{
    va_list args;
    va_start(args, pText);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), pText, args);
    va_end(args);

    TextCentered(buffer);
}

void GUI::DrawText_(ImVec2 pos, ImColor color, const char* pText, float fontSize)
{
    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), fontSize, pos, color, pText, pText + strlen(pText), 800, 0);
}

//  @ATTN: max buffer is 256chars
void GUI::DrawTextf(ImVec2 pos, ImColor color, const char* pText, float fontSize, ...)
{
    va_list args;
    va_start(args, fontSize);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), pText, args);
    va_end(args);

    DrawText_(pos, color, buffer, fontSize);
}

void GUI::DrawTextCentered(ImVec2 pos, ImColor color, const char* pText, float fontSize)
{
    float textSize = ImGui::CalcTextSize(pText).x;
    ImVec2 textPosition = ImVec2(pos.x - (textSize * 0.5f), pos.y);
    DrawText_(textPosition, color, pText, fontSize);
}

//  void GUI::DrawTextCentered(ImVec2 pos, ImColor color, const char* pText, float fontSize)
//  {
//      std::string txt(pText);
//  
//      ImVec2 textSize = ImGui::CalcTextSize(pText);
//  
//      const char* start = txt.c_str();
//      const char* end = start + txt.size();
//  
//      ImVec2 textPos(pos.x - (textSize.x * .5f), pos.y);
//      DrawText_(textPos, color, pText, fontSize);
//  }

//  @ATTN: max buffer is 256chars
void GUI::DrawTextCenteredf(ImVec2 pos, ImColor color, const char* pText, float fontSize, ...)
{
    va_list args;
    va_start(args, fontSize);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), pText, args);
    va_end(args);

    DrawTextCentered(pos, color, pText, fontSize);
}