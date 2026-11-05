#include "friends_screen.h"

#include <algorithm>
#include <cmath>

#include "nav_bar.h"
#include "screen_id.h"
#include "../../resources/icons/Profiles.h"
#include "ImGui/imgui_internal.h"

const char* friends_screen::tab_name(int i)
{
    switch (i)
    {
        case 0: return "All friends";
        default: return "";
    }
}

void friends_screen::draw_screen_content()
{
    render_tabs();

    const int next = nav_bar::get().draw(screen_id::friends);
    if (next >= 0)
        request_switch(next);

    render_tabs_content();
}

void friends_screen::load_textures()
{
    if (m_icons_loaded)
        return;

    m_icons_loaded = true;

    friends_icon = load_texture_from_memory(Profiles, ProfilesLen);
}

void friends_screen::on_release_gpu()
{
    friends_icon.release();
}

void friends_screen::render_tabs()
{
    load_textures();

    constexpr float row_height    = 70.0f;
    constexpr float font_size     = 25.0f;
    constexpr float start_x       = 40.0f;
    constexpr float underline_gap = 6.0f;
    constexpr float underline_h   = 2.0f;
    constexpr float anim_speed    = 10.0f;
    constexpr float anim_epsilon  = 0.001f;

    constexpr float icon_size     = 20.0f;
    constexpr float icon_spacing  = 10.0f;

    const ImU32 accent_color = ImGui::GetColorU32(ImGuiCol_Button);
    const ImU32 white        = IM_COL32(255, 255, 255, 255);
    const ImU32 inactive     = IM_COL32(44, 44, 49, 255);

    ImFont*     font = ImGui::GetFont();
    ImDrawList* dl   = ImGui::GetWindowDrawList();

    const float k = 1.0f - std::exp(-anim_speed * ImGui::GetIO().DeltaTime);

    float t_y = ImGui::GetCursorPosY() + 95.0f;

    for (int i = 0; i < kMaxTabs; ++i)
    {
        const bool  selected = (i == m_current_tab);
        const char* label    = tab_name(i);

        const ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, label);
        if (text_size.x <= 0 || text_size.y <= 0)
            continue;

        const texture* icon = &friends_icon;

        const float total_w = icon_size + icon_spacing + text_size.x;
        const float total_h = (std::max)(icon_size, text_size.y);

        ImGui::SetCursorPos(ImVec2(start_x, t_y));
        ImGui::PushID(i);
        const bool clicked = ImGui::InvisibleButton("##tab", ImVec2(total_w, total_h));
        ImGui::PopID();

        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        if (clicked)
            m_current_tab = i;

        const ImVec2 item_pos = ImGui::GetItemRectMin();
        const ImU32  color    = selected ? white : inactive;

        if (icon && icon->src)
        {
            const float icon_y = item_pos.y + (total_h - icon_size) * 0.5f;
            dl->AddImage((ImTextureID)icon->src,
                         ImVec2(item_pos.x, icon_y),
                         ImVec2(item_pos.x + icon_size, icon_y + icon_size),
                         ImVec2(0, 0), ImVec2(1, 1), color);
        }

        const ImVec2 text_pos(item_pos.x + icon_size + icon_spacing,
                              item_pos.y + (total_h - text_size.y) * 0.5f);
        dl->AddText(font, font_size, text_pos, color, label);

        float& line_anim = m_tab_line_anim[i];
        const float target = selected ? 1.0f : 0.0f;
        line_anim += (target - line_anim) * k;
        if (std::fabs(line_anim - target) < anim_epsilon)
            line_anim = target;

        if (line_anim > anim_epsilon)
        {
            const float half_width = total_w * 0.6f * 0.5f * line_anim;
            const float center_x   = item_pos.x + total_w * 0.5f;
            const float line_y     = item_pos.y + total_h + underline_gap;

            dl->AddRectFilled(ImVec2(center_x - half_width, line_y),
                              ImVec2(center_x + half_width, line_y + underline_h),
                              selected ? accent_color : color,
                              1.0f);
        }

        t_y += row_height;
    }

    ImGui::Dummy(ImVec2(0, 10));
}

void friends_screen::render_tabs_content()
{
    constexpr float start_x = 220.0f;
    constexpr float start_y = 140.0f;

    ImGui::SetCursorPos(ImVec2(start_x, start_y));

    const char* hint = "no friends yet";
    ImGui::PushStyleColor(ImGuiCol_Text,
        ImGui::ColorConvertU32ToFloat4(IM_COL32(80, 80, 95, 255)));
    ImGui::PopStyleColor();
}