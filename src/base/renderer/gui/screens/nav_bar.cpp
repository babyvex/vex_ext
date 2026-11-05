#include "nav_bar.h"

#include <cfloat>
#include <cmath>

#include "../../../features/module_manager.h"
#include "../../../features/impl/settings/theme_manager.hpp"
#include "Imgui/imgui.h"
#include "../screens/abstract_screen.h"

#include "../../resources/icons/Friends.h"
#include "../../resources/icons/Home.h"
#include "../../resources/icons/Macros.h"

namespace
{
    struct tab_def { const char* label; screen_id target; };

    constexpr tab_def kTabs[nav_bar::kTabCount] = {
        { "Home",    screen_id::main    },
        { "Configs", screen_id::configs },
        { "Friends", screen_id::friends },
    };

    constexpr float kBaseX       = 20.0f;
    constexpr float kBaseY       = 60.0f;
    constexpr float kTabHeight   = 48.0f;
    constexpr float kIconSize    = 26.0f;
    constexpr float kIconSpacing = 12.0f;
    constexpr float kPadX        = 18.0f;
    constexpr float kRounding    = 8.0f;
    constexpr float kGap         = 12.0f;
    constexpr float kAnimSpeed   = 10.0f;
    constexpr float kFontSize    = 22.0f;

    constexpr ImU32 kColActive   = IM_COL32(20, 19, 24, 255);
    constexpr ImU32 kColInactive = IM_COL32(20, 19, 24, 160);
    constexpr ImU32 kColIconOff  = IM_COL32(160, 160, 160, 255);
    constexpr ImU32 kColText     = IM_COL32(255, 255, 255, 255);

    theme_manager* get_theme_manager() {
        if (!global_module_manager) return nullptr;
        auto* m = global_module_manager->get_module_by_name("Theme manager");
        return static_cast<theme_manager*>(m);
    }
}

nav_bar& nav_bar::get()
{
    static nav_bar instance;
    return instance;
}

void nav_bar::load_icons()
{
    if (m_loaded)
        return;

    m_loaded = true;
    m_icons[0] = load_texture_from_memory(Home,    HomeLen);
    m_icons[1] = load_texture_from_memory(Macros,  MacrosLen);
    m_icons[2] = load_texture_from_memory(Friends, FriendsLen);
}

void nav_bar::release()
{
    for (auto& icon : m_icons)
        icon.release();

    m_loaded = false;
}

int nav_bar::draw(screen_id current)
{
    load_icons();

    ImFont*     font    = ImGui::GetFont();
    ImDrawList* dl      = ImGui::GetWindowDrawList();
    const ImVec2 win_pos = ImGui::GetWindowPos();

    const float k = 1.0f - std::exp(-kAnimSpeed * ImGui::GetIO().DeltaTime);

    auto* tm = get_theme_manager();
    const bool navbar_accent = (tm == nullptr) || tm->navbar_accent_enabled();

    const ImU32 kColIconOn = navbar_accent
        ? ImGui::GetColorU32(ImGuiCol_Button)
        : IM_COL32(255, 255, 255, 255);

    int   request  = -1;
    float cursor_x = kBaseX;

    for (int i = 0; i < kTabCount; ++i)
    {
        const bool  selected = (kTabs[i].target == current);
        const char* label    = kTabs[i].label;
        const texture& icon  = m_icons[i];

        const ImVec2 text_size = font->CalcTextSizeA(kFontSize, FLT_MAX, -1.0f, label);

        const float collapsed_w = kIconSize + kPadX * 2.0f;
        const float expanded_w  = kIconSize + kIconSpacing + text_size.x + kPadX * 2.0f;
        const float target_w    = selected ? expanded_w : collapsed_w;

        float& w = m_width[i];
        if (w < 1.0f)
            w = target_w;
        else
            w += (target_w - w) * k;

        const ImVec2 tab_min(win_pos.x + cursor_x, win_pos.y + kBaseY);
        const ImVec2 tab_max(tab_min.x + w, tab_min.y + kTabHeight);

        ImGui::SetCursorScreenPos(tab_min);
        ImGui::PushID(i);
        const bool clicked = ImGui::InvisibleButton("##nav_tab", ImVec2(w, kTabHeight));
        ImGui::PopID();

        const bool hovered = ImGui::IsItemHovered() && !g_panel_blocks_input;

        if (hovered)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        if (clicked && !selected && !g_panel_blocks_input)
            request = static_cast<int>(kTabs[i].target);

        // фон таба
        dl->AddRectFilled(tab_min, tab_max, selected ? kColActive : kColInactive, kRounding);

        // иконка: выкл — серый, вкл — accent (если включён) или белый
        if (icon.src)
        {
            const float icon_y = tab_min.y + (kTabHeight - kIconSize) * 0.5f;
            dl->AddImage((ImTextureID)icon.src,
                         ImVec2(tab_min.x + kPadX, icon_y),
                         ImVec2(tab_min.x + kPadX + kIconSize, icon_y + kIconSize),
                         ImVec2(0, 0), ImVec2(1, 1),
                         selected ? kColIconOn : kColIconOff);
        }

        if (w > collapsed_w + text_size.x * 0.3f)
        {
            const ImVec2 text_pos(
                tab_min.x + kPadX + kIconSize + kIconSpacing,
                tab_min.y + (kTabHeight - text_size.y) * 0.5f);
            dl->AddText(font, kFontSize, text_pos, kColText, label);
        }

        cursor_x += w + kGap;
    }

    return request;
}