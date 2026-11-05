#include "abstract_screen.h"

#include "../../resources/icons/Delete.h"
#include "../../resources/icons/Minimize.h"
#include "../../windows/main_window.h"

namespace
{
    constexpr float kButtonSize = 28.0f;
    constexpr float kPadding    = 10.0f;
    constexpr float kSpacing    = 4.0f;

    constexpr float kButtonsAreaWidth = kPadding + kButtonSize * 2.0f + kSpacing * 2.0f;
}


bool g_panel_blocks_input = false;

void abstract_screen::draw()
{
    begin_window();
    draw_screen_content();
    end_window();
}

void abstract_screen::begin_window()
{
    const ImVec2 display = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(display, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin(get_window_name(), nullptr, get_window_flags());

    m_position = ImGui::GetWindowPos();
    m_size     = ImGui::GetWindowSize();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(
        m_position,
        ImVec2(m_position.x + m_size.x, m_position.y + m_size.y),
        IM_COL32(16, 16, 18, 255),
        15.0f
    );

    if (should_show_title_bar())
        draw_title_bar();

    ImGui::SetCursorPos(ImVec2(0.0f, should_show_title_bar() ? kTitleBarHeight : 0.0f));
}

void abstract_screen::end_window()
{
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}

void abstract_screen::draw_title_bar()
{
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(
        m_position,
        ImVec2(m_position.x + m_size.x, m_position.y + kTitleBarHeight),
        IM_COL32(20, 19, 24, 255),
        15.0f,
        ImDrawFlags_RoundCornersTop
    );

    draw_title_bar_content();
    draw_window_buttons();

    draw_title_bar_drag_area();
}

void abstract_screen::draw_window_buttons()
{
    if (!m_icons_loaded)
    {
        m_icons_loaded = true;
        m_icon_close    = load_texture_from_memory(Delete, DeleteLen);
        m_icon_minimize = load_texture_from_memory(Minimize, MinimizeLen);
    }

    HWND hwnd = main_window::get_main_hwnd();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const float btnY = m_position.y + (kTitleBarHeight - kButtonSize) * 0.5f;
    float x = m_position.x + m_size.x - kPadding;

    auto icon_button = [&](const char* id, const texture& tex) -> bool
    {
        x -= kButtonSize;

        ImGui::SetCursorScreenPos(ImVec2(x, btnY));
        const bool clicked = ImGui::InvisibleButton(id, ImVec2(kButtonSize, kButtonSize));
        const bool hovered = ImGui::IsItemHovered();

        if (tex.src)
        {
            dl->AddImage(
                (ImTextureID)tex.src,
                ImVec2(x, btnY),
                ImVec2(x + kButtonSize, btnY + kButtonSize),
                ImVec2(0, 0), ImVec2(1, 1),
                hovered ? IM_COL32(255, 255, 255, 255)
                        : IM_COL32(180, 180, 180, 200)
            );
        }

        x -= kSpacing;
        return clicked;
    };

    if (icon_button("##btn_close", m_icon_close))
    {
        if (hwnd && IsWindow(hwnd))
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
    }


    if (icon_button("##btn_minimize", m_icon_minimize))
    {
        if (hwnd && IsWindow(hwnd))
            ShowWindow(hwnd, SW_MINIMIZE);
    }
}

void abstract_screen::draw_title_bar_drag_area()
{
    HWND hwnd = main_window::get_main_hwnd();
    if (!hwnd || !IsWindow(hwnd))
        return;

    ImGui::SetCursorScreenPos(m_position);
    ImGui::InvisibleButton("##title_drag", ImVec2(m_size.x - kButtonsAreaWidth, kTitleBarHeight));




    if (ImGui::IsItemActivated())
    {
        GetCursorPos(&m_drag_cursor_start);

        RECT rc{};
        GetWindowRect(hwnd, &rc);
        m_drag_window_start = { rc.left, rc.top };
    }

    if (ImGui::IsItemActive())
    {
        POINT cur{};
        GetCursorPos(&cur);

        SetWindowPos(hwnd, nullptr,
                     m_drag_window_start.x + (cur.x - m_drag_cursor_start.x),
                     m_drag_window_start.y + (cur.y - m_drag_cursor_start.y),
                     0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}