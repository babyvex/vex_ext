#include "main_screen.h"

#include <algorithm>
#include <cmath>

#include "nav_bar.h"
#include "screen_id.h"
#include "../../../features/category.h"
#include "../../../features/module_manager.h"
#include "../../resources/icons/Combat.h"
#include "../../resources/icons/Favorites.h"
#include "../../resources/icons/Magnify.h"
#include "../../resources/icons/Misc.h"
#include "../../resources/icons/Movement.h"
#include "../../resources/icons/RightArrow.h"
#include "../../resources/icons/Settings.h"
#include "../../resources/icons/ToggleBack.h"
#include "../../resources/icons/ToggleFill.h"
#include "../../resources/icons/Vertline.h"
#include "../../resources/icons/Visual.h"
#include "ImGui/imgui_internal.h"

#include "../../components/components.h"
#include "../../../features/impl/settings/theme_manager.hpp"
void main_screen::load_textures()
{
    if (m_icons_loaded)
        return;

    m_icons_loaded = true;

    combat_text      = load_texture_from_memory(Combat,     CombatLen);
    misc_text        = load_texture_from_memory(Misc,       MiscLen);
    movement_text    = load_texture_from_memory(Movement,   MovementLen);
    visual_text      = load_texture_from_memory(Visual,     VisualLen);
    settings_text    = load_texture_from_memory(Settings,   SettingsLen);
    magnify_text     = load_texture_from_memory(Magnify,    MagnifyLen);
    right_arrow_text = load_texture_from_memory(RightArrow, RightArrowLen);
    vertline_text    = load_texture_from_memory(Vertline,   VertlineLen);
    toggle_back      = load_texture_from_memory(ToggleBack, ToggleBackLen);
    toggle_fill      = load_texture_from_memory(ToggleFill, ToggleFillLen);
    favorites_text   = load_texture_from_memory(Favorites,  FavoritesLen);
}

void main_screen::on_release_gpu()
{
    combat_text.release();
    misc_text.release();
    movement_text.release();
    visual_text.release();
    settings_text.release();
    magnify_text.release();
    right_arrow_text.release();
    vertline_text.release();
    toggle_back.release();
    toggle_fill.release();
    favorites_text.release();
}

void main_screen::draw_screen_content()
{
    load_textures();

    g_panel_blocks_input = (m_panel_anim_t > 0.001f);

    render_tabs();

    const int next = nav_bar::get().draw(screen_id::main);
    if (next >= 0)
        request_switch(next);

    render_search_icon();

    render_tabs_content();
}

void main_screen::render_module_card(abstract_module* module, ImVec2 pos, float width)
{
    constexpr float card_height = 170.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const bool enabled = module->is_enabled();
    const float anim   = m_toggle_anim[module];
    auto& s            = m_card_spring[module];

    float dt = ImGui::GetIO().DeltaTime;
    if (dt > 0.1f) dt = 0.1f;

    constexpr float stiffness = 320.0f;
    constexpr float damping   = 24.0f;

    if (std::abs(s.value) > 0.0001f || std::abs(s.velocity) > 0.0001f)
    {
        const float displacement = s.value;
        const float spring_force = -stiffness * displacement;
        const float damping_force = -damping * s.velocity;
        const float acceleration = spring_force + damping_force;

        s.velocity += acceleration * dt;
        s.value    += s.velocity * dt;

        if (s.value < 0.0f) { s.value = 0.0f; s.velocity = 0.0f; }
        if (s.value < 0.005f && std::abs(s.velocity) < 0.05f) { s.value = 0.0f; s.velocity = 0.0f; }
    }

    const float squeeze = 0.03f * s.value;
    const ImVec2 center(pos.x + width * 0.5f, pos.y + card_height * 0.5f);
    const float half_w = width * 0.5f * (1.0f - squeeze);
    const float half_h = card_height * 0.5f * (1.0f - squeeze);

    const ImVec2 draw_pos(center.x - half_w, center.y - half_h);
    const ImVec2 draw_end(center.x + half_w, center.y + half_h);
    const float  draw_w = draw_end.x - draw_pos.x;
    const float  draw_h = draw_end.y - draw_pos.y;

    const ImVec2 mouse = ImGui::GetMousePos();
    const bool hovered =
        !g_panel_blocks_input &&
        mouse.x >= pos.x && mouse.x <= pos.x + width &&
        mouse.y >= pos.y && mouse.y <= pos.y + card_height;

    ImU32 card_bg = hovered ? IM_COL32(24, 24, 29, 255) : IM_COL32(22, 22, 26, 255);
    dl->AddRectFilled(draw_pos, draw_end, card_bg, 8.0f);

    const float text_x = draw_pos.x + 16.0f;
    const float name_font_size = ImGui::GetFontSize() * 1.5f;
    const ImVec2 name_size = ImGui::CalcTextSize(module->get_name());
    const float name_y = draw_pos.y + (draw_h - name_size.y) * 0.08f;

    dl->AddText(ImGui::GetFont(), name_font_size, ImVec2(text_x, name_y),
                IM_COL32(255, 255, 255, 255), module->get_name());

    const float desc_font_size = ImGui::GetFontSize() * 1.25f;
    const float desc_y = name_y + name_size.y + 8.0f;

    dl->AddText(nullptr, desc_font_size, ImVec2(text_x, desc_y),
                IM_COL32(58, 58, 65, 255), module->get_description());

    constexpr float toggle_w = 42.0f;
    constexpr float toggle_h = 22.0f;
    constexpr float toggle_right_margin = 16.0f;
    constexpr float dots_w = 24.0f;

    const float tx_hit = pos.x + width - toggle_w - toggle_right_margin - 25.0f;
    const float ty_hit = pos.y + (card_height - toggle_h) * 0.13f;
    const float dots_hit_x = pos.x + width - toggle_right_margin + dots_w * 0.5f - 20.0f;
    const float dots_hit_cy = pos.y + (card_height - toggle_h) * 0.13f + toggle_h * 0.5f;

    {
        constexpr float boxH   = 38.0f;
        constexpr float margin = 16.0f;

        const float comp_x = draw_pos.x + margin;
        const float comp_y = draw_end.y - margin - boxH;

        ImGui::SetCursorScreenPos(ImVec2(comp_x, comp_y));

        g_current_module_enabled = module->is_enabled();
        module->render_tile_components();
        g_current_module_enabled = false;
    }

    {
        const float tx_draw = draw_pos.x + draw_w - toggle_w - toggle_right_margin - 25.0f;
        const float ty_draw = draw_pos.y + (draw_h - toggle_h) * 0.13f;

        // фон тоггла: выкл — тёмный, вкл — акцент приглушённый (×0.75)
        const ImVec4 off_col(20.0f/255.0f, 19.0f/255.0f, 24.0f/255.0f, 1.0f);
        ImVec4 on_col = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        on_col.x *= 0.75f;
        on_col.y *= 0.75f;
        on_col.z *= 0.75f;
        const ImU32 tog_col = ImGui::ColorConvertFloat4ToU32(ImLerp(off_col, on_col, anim));

        dl->AddRectFilled(ImVec2(tx_draw, ty_draw),
                          ImVec2(tx_draw + toggle_w, ty_draw + toggle_h),
                          tog_col, toggle_h * 0.5f);

        // обводка: выкл — серый, вкл — акцент
        const ImU32 off_border = IM_COL32(60, 60, 65, 255);
        const ImU32 on_border  = ImGui::GetColorU32(ImGuiCol_Button);
        const ImU32 border_col = ImGui::ColorConvertFloat4ToU32(ImLerp(
            ImGui::ColorConvertU32ToFloat4(off_border),
            ImGui::ColorConvertU32ToFloat4(on_border), anim));

        dl->AddRect(ImVec2(tx_draw, ty_draw),
                    ImVec2(tx_draw + toggle_w, ty_draw + toggle_h),
                    border_col, toggle_h * 0.5f,
                    ImDrawFlags_None, 2.0f);

        const float knob_r = toggle_h * 0.5f - 3.0f;
        const float kx = tx_draw + toggle_h * 0.5f + (toggle_w - toggle_h) * anim;
        const float ky = ty_draw + toggle_h * 0.5f;

        // кружок — всегда белый, не акцент
        dl->AddCircleFilled(ImVec2(kx, ky), knob_r, IM_COL32(255, 255, 255, 255));

        ImGui::SetCursorScreenPos(ImVec2(tx_hit, ty_hit));
        ImGui::PushID(module);
        ImGui::InvisibleButton("##toggle", ImVec2(toggle_w, toggle_h));

        if (ImGui::IsItemActivated() &&
            !g_panel_blocks_input && !m_click_consumed_by_panel)
        {
            s.value = 1.0f;
            s.velocity = 0.0f;
            module->toggle();
        }
        ImGui::PopID();
    }

    {
        constexpr float dot_radius  = 2.0f;
        constexpr float dot_spacing = 7.0f;

        const float dots_x    = draw_pos.x + draw_w - toggle_right_margin;
        const float toggle_cy = draw_pos.y + (draw_h - toggle_h) * 0.13f + toggle_h * 0.5f;
        const float center_x  = dots_x + dots_w * 0.5f - 20.0f;

        const ImVec2 dots_rect_min(dots_hit_x - dots_w * 0.5f, dots_hit_cy - dots_w * 0.5f);
        const ImVec2 dots_rect_max(dots_hit_x + dots_w * 0.5f, dots_hit_cy + dots_w * 0.5f);

        const bool dots_hovered =
            !g_panel_blocks_input &&
            mouse.x >= dots_rect_min.x && mouse.x <= dots_rect_max.x &&
            mouse.y >= dots_rect_min.y && mouse.y <= dots_rect_max.y;

        const bool panel_open_for_this =
            (m_open_panel_module == module) && (m_panel_anim_t > 0.001f);

        const float target = panel_open_for_this ? 1.0f : (dots_hovered ? 0.5f : 0.0f);
        float& da = m_dots_anim[module];
        const float k_dots = 1.0f - std::exp(-14.0f * ImGui::GetIO().DeltaTime);
        da += (target - da) * k_dots;
        if (std::fabs(target - da) < 0.001f)
            da = target;

        // точки — серый → белый, без акцента
        const int v = static_cast<int>(150 + (255 - 150) * da);
        const ImU32 dots_color = IM_COL32(v, v, v, 255);

        for (int j = 0; j < 3; j++)
        {
            const float y = toggle_cy + (j - 1) * dot_spacing;
            dl->AddCircleFilled(ImVec2(center_x, y), dot_radius, dots_color);
        }

        ImGui::SetCursorScreenPos(dots_rect_min);
        ImGui::PushID(module);
        ImGui::InvisibleButton("##dots", ImVec2(dots_w, dots_w));
        if (ImGui::IsItemClicked() && !g_panel_blocks_input)
        {
            const bool will_open = (m_open_panel_module != module);
            m_open_panel_module = will_open ? module : nullptr;
            m_dots_clicked_this_frame = true;

            if (will_open)
            {
                m_panel_scroll         = 0.0f;
                m_panel_scroll_target  = 0.0f;
                m_panel_content_bottom = 0.0f;
            }
        }
        ImGui::PopID();
    }

    ImGui::SetCursorScreenPos(pos);
    ImGui::PushID(module);
    ImGui::InvisibleButton("##card_click", ImVec2(width, card_height),
                           ImGuiButtonFlags_AllowOverlap);

    const bool mouse_over_toggle =
        ImGui::IsMouseHoveringRect(ImVec2(tx_hit, ty_hit),
                                   ImVec2(tx_hit + toggle_w, ty_hit + toggle_h));
    const bool mouse_over_dots =
        ImGui::IsMouseHoveringRect(
            ImVec2(dots_hit_x - dots_w * 0.5f, dots_hit_cy - dots_w * 0.5f),
            ImVec2(dots_hit_x + dots_w * 0.5f, dots_hit_cy + dots_w * 0.5f));

    if (!mouse_over_toggle && !mouse_over_dots &&
        !g_panel_blocks_input && !m_click_consumed_by_panel)
    {
        if (ImGui::IsItemActivated())
        {
            s.value = 1.0f;
            s.velocity = 0.0f;
            module->toggle();
        }
    }
    ImGui::PopID();

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + card_height));
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
}

void main_screen::render_search_icon()
{
    constexpr float kTabHeight   = 48.0f;
    constexpr float kIconSize    = 26.0f;
    constexpr float kBaseY       = 60.0f;
    constexpr float kPadX        = 18.0f;
    constexpr float kRounding    = 8.0f;
    constexpr float kAnimSpeed   = 10.0f;

    constexpr ImU32 kColActive   = IM_COL32(20, 19, 24, 255);
    constexpr ImU32 kColInactive = IM_COL32(20, 19, 24, 160);
    constexpr ImU32 kColIconOn   = IM_COL32(255, 255, 255, 255);
    constexpr ImU32 kColIconOff  = IM_COL32(160, 160, 160, 255);

    const ImVec2 win_pos  = ImGui::GetWindowPos();
    const ImVec2 win_size = ImGui::GetWindowSize();

    constexpr float right_pad = 20.0f;

    const float collapsed_w = kIconSize + kPadX * 2.0f;
    constexpr float input_w_target = 200.0f;
    constexpr float input_gap      = 10.0f;
    const float expanded_w = input_w_target + input_gap + collapsed_w;

    const float tab_x1_final = win_pos.x + win_size.x - right_pad;
    const float tab_x0_final = tab_x1_final - expanded_w;
    const float tab_y0       = win_pos.y + kBaseY;
    const float tab_y1       = tab_y0 + kTabHeight;

    const float icon_x = tab_x1_final - kPadX - kIconSize;
    const float icon_y = tab_y0 + (kTabHeight - kIconSize) * 0.5f;

    ImGui::SetCursorScreenPos(ImVec2(icon_x, icon_y));
    ImGui::PushID("##search_icon_click");
    ImGui::InvisibleButton("##hit", ImVec2(kIconSize, kIconSize));
    const bool icon_hovered = ImGui::IsItemHovered() && !g_panel_blocks_input;
    const bool icon_clicked = ImGui::IsItemClicked();
    ImGui::PopID();

    const bool want_open = (m_search_icon_hovered_prev || icon_hovered || m_search_open);

    const float k = 1.0f - std::exp(-kAnimSpeed * ImGui::GetIO().DeltaTime);
    const float target = want_open ? 1.0f : 0.0f;

    if (m_search_anim_t < 0.001f && target > 0.5f)
        m_search_anim_t = 0.001f;
    else
        m_search_anim_t += (target - m_search_anim_t) * k;

    if (m_search_anim_t < 0.001f) m_search_anim_t = 0.0f;

    const float cur_w = collapsed_w + (expanded_w - collapsed_w) * m_search_anim_t;
    const float tab_x1 = tab_x1_final;
    const float tab_x0 = tab_x1 - cur_w;

    const bool open = (m_search_anim_t > 0.5f);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(ImVec2(tab_x0, tab_y0),
                      ImVec2(tab_x1, tab_y1),
                      open ? kColActive : kColInactive,
                      kRounding);

    bool input_active = false;

    if (m_search_anim_t > 0.3f)
    {
        const float input_x = tab_x0 + kPadX;
        const float input_w = cur_w - kPadX * 2.0f - kIconSize - input_gap;
        const float input_y = tab_y0 + (kTabHeight - 22.0f) * 0.5f;

        if (input_w > 20.0f)
        {
            ImGui::SetCursorScreenPos(ImVec2(input_x, input_y));
            ImGui::PushStyleColor(ImGuiCol_FrameBg,        IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_Border,         IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_Text,           IM_COL32(230, 230, 235, 255));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     ImVec2(0, 2));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  0.0f);
            ImGui::SetNextItemWidth(input_w);

            ImGui::PushID("##search_input");
            if (m_search_active)
            {
                ImGui::SetKeyboardFocusHere();
                m_search_active = false;
            }

            ImGui::InputTextWithHint("##buf", "Search...",
                                     m_search_buf, sizeof(m_search_buf));

            input_active = ImGui::IsItemActive();
            ImGui::PopID();

            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(5);
        }
    }

    {
        const ImU32 icon_col = (icon_hovered || m_search_open)
            ? kColIconOn
            : kColIconOff;

        if (magnify_text.src)
        {
            dl->AddImage((ImTextureID)magnify_text.src,
                         ImVec2(icon_x, icon_y),
                         ImVec2(icon_x + kIconSize, icon_y + kIconSize),
                         ImVec2(0, 0), ImVec2(1, 1), icon_col);
        }
    }

    if (icon_clicked)
    {
        if (!m_search_open)
        {
            m_search_open   = true;
            m_search_active = true;
        }
        else
        {
            m_search_open   = false;
            m_search_active = false;
            m_search_buf[0] = '\0';
        }
    }

    constexpr float row_h      = 34.0f;
    constexpr float list_pad_y = 6.0f;

    bool clicked_any_module = false;

    if (open && m_search_buf[0] != '\0')
    {
        auto* mgr = global_module_manager.get();
        if (mgr)
        {
            struct hit_t { abstract_module* mod; int cat; };
            std::vector<hit_t> hits;

            std::string query = m_search_buf;
            for (auto& c : query)
                c = (char)std::tolower((unsigned char)c);

            for (int ci = 0; ci < categories::count(); ++ci)
            {
                const auto mods = mgr->get_module_by_category(categories::id(ci));
                for (auto* m : mods)
                {
                    std::string name = m->get_name();
                    for (auto& c : name)
                        c = (char)std::tolower((unsigned char)c);

                    if (name.find(query) != std::string::npos)
                        hits.push_back({ m, ci });
                }
            }

            const float list_w = expanded_w;
            const float list_x = tab_x1_final - list_w;
            const float list_y = tab_y1 + list_pad_y;

            if (hits.empty())
            {
                const ImVec2 p0(list_x, list_y);
                const ImVec2 p1(list_x + list_w, list_y + row_h);

                dl->AddRectFilled(p0, p1, kColActive, kRounding);

                const char* hint = "no results";
                const ImVec2 ts = ImGui::CalcTextSize(hint);
                dl->AddText(ImVec2(p0.x + (list_w - ts.x) * 0.5f,
                                   p0.y + (row_h - ts.y) * 0.5f),
                            IM_COL32(120, 120, 135, 255), hint);
            }
            else
            {
                const float total_h = row_h * (float)hits.size();

                dl->AddRectFilled(
                    ImVec2(list_x, list_y),
                    ImVec2(list_x + list_w, list_y + total_h),
                    kColActive, kRounding);

                abstract_module* hit_module = nullptr;
                int              hit_cat    = 0;

                for (size_t i = 0; i < hits.size(); ++i)
                {
                    const auto& h = hits[i];

                    const ImVec2 p0(list_x, list_y + row_h * (float)i);
                    const ImVec2 p1(p0.x + list_w, p0.y + row_h);

                    ImGui::SetCursorScreenPos(p0);
                    ImGui::PushID((int)(1000 + i));
                    ImGui::InvisibleButton("##hit_row", ImVec2(list_w, row_h));
                    const bool row_hovered = ImGui::IsItemHovered();
                    const bool row_clicked = ImGui::IsItemClicked();
                    ImGui::PopID();

                    if (row_hovered)
                        dl->AddRectFilled(p0, p1,
                                          IM_COL32(30, 30, 36, 255), kRounding);

                    const char* name = h.mod->get_name();
                    const ImVec2 nts = ImGui::CalcTextSize(name);

                    dl->AddText(
                        ImVec2(p0.x + 12.0f,
                               p0.y + (row_h - nts.y) * 0.5f),
                        row_hovered
                            ? IM_COL32(255, 255, 255, 255)
                            : IM_COL32(220, 220, 225, 255),
                        name);

                    const char* cat_name = categories::name(h.cat);
                    const ImVec2 cts = ImGui::CalcTextSize(cat_name);

                    dl->AddText(
                        ImVec2(p1.x - cts.x - 12.0f,
                               p0.y + (row_h - cts.y) * 0.5f),
                        IM_COL32(120, 120, 135, 255),
                        cat_name);

                    if (row_clicked)
                    {
                        hit_module = h.mod;
                        hit_cat    = h.cat;
                        clicked_any_module = true;
                    }
                }

                if (hit_module)
                {
                    m_current_tab = hit_cat;

                    m_open_panel_module = hit_module;
                    m_panel_scroll         = 0.0f;
                    m_panel_scroll_target  = 0.0f;
                    m_panel_content_bottom = 0.0f;

                    m_search_open   = false;
                    m_search_active = false;
                    m_search_buf[0] = '\0';
                }
            }
        }
    }

    if (!clicked_any_module &&
        m_search_open &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        const ImVec2 mp = ImGui::GetMousePos();
        const bool inside =
            mp.x >= tab_x0_final && mp.x <= tab_x1_final &&
            mp.y >= tab_y0 && mp.y <= tab_y1;

        if (!inside && !icon_hovered)
        {
            m_search_open   = false;
            m_search_active = false;
            m_search_buf[0] = '\0';
        }
    }

    m_search_icon_hovered_prev = icon_hovered;
}

void main_screen::render_tabs_content()
{
    auto* mgr = global_module_manager.get();
    if (!mgr) return;

    const int category_index = m_current_tab;
    if (category_index < 0 || category_index >= categories::count()) return;

    std::vector<abstract_module*> modules =
        mgr->get_module_by_category(categories::id(category_index));

    if (modules.empty()) return;

    std::stable_sort(modules.begin(), modules.end(),
        [this](abstract_module* a, abstract_module* b)
        {
            const bool pa = m_pinned.count(a) > 0;
            const bool pb = m_pinned.count(b) > 0;
            return pa > pb;
        });

    constexpr float card_width   = 380.0f;
    constexpr float card_height  = 170.0f;
    constexpr float card_spacing = 15.0f;
    constexpr float start_x      = 220.0f;
    constexpr float start_y      = 140.0f;
    constexpr float container_h  = 480.0f;
    constexpr float padding_x    = 10.0f;
    constexpr float padding_y    = 10.0f;

    constexpr float sb_width       = 10.0f;
    constexpr float sb_min_thumb   = 20.0f;
    constexpr float sb_extra_space = 30.0f;

    const int columns = 2;

    ImGui::SetCursorPos(ImVec2(start_x, start_y));

    const float container_w =
        columns * card_width + (columns - 1) * card_spacing + padding_x * 2.0f
        + sb_extra_space;

    constexpr float panel_open_w   = 400.0f;
    constexpr float panel_duration = 0.12f;

    {
        const bool want_open = (m_open_panel_module != nullptr);

        if (want_open)
            m_panel_rendering_module = m_open_panel_module;

        const float step = ImGui::GetIO().DeltaTime / panel_duration;
        if (want_open)
            m_panel_anim_t = (std::min)(m_panel_anim_t + step, 1.0f);
        else
            m_panel_anim_t = (std::max)(m_panel_anim_t - step, 0.0f);

        const float inv   = 1.0f - m_panel_anim_t;
        const float eased = 1.0f - inv * inv * inv;

        m_module_panel_width = panel_open_w * eased;

        if (m_panel_anim_t <= 0.0f)
            m_panel_rendering_module = nullptr;
    }

    g_panel_blocks_input = (m_panel_anim_t > 0.001f);

    {
        const float dt_anim  = ImGui::GetIO().DeltaTime;
        const float k_toggle = 1.0f - std::exp(-12.0f * dt_anim);

        for (auto* m : modules)
        {
            float& a = m_toggle_anim[m];
            const float target = m->is_enabled() ? 1.0f : 0.0f;
            a += (target - a) * k_toggle;
            if (std::fabs(target - a) < 0.001f)
                a = target;
        }
    }

    m_click_consumed_by_panel = false;
    m_dots_clicked_this_frame = false;

    const int rows = (static_cast<int>(modules.size()) + columns - 1) / columns;
    const float content_h =
        padding_y * 2.0f + rows * card_height + (rows - 1) * card_spacing;
    const float max_scroll = (std::max)(0.0f, content_h - container_h);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::BeginChild("##modules_grid",
                      ImVec2(container_w, container_h),
                      false,
                      ImGuiWindowFlags_NoNav |
                      ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoScrollWithMouse);

    const float dt       = ImGui::GetIO().DeltaTime;
    const float k_scroll = 1.0f - std::exp(-22.0f * dt);

    const ImVec2 child_screen = ImGui::GetWindowPos();
    const ImVec2 mouse        = ImGui::GetMousePos();

    const bool mouse_inside =
        mouse.x >= child_screen.x && mouse.x <= child_screen.x + container_w &&
        mouse.y >= child_screen.y && mouse.y <= child_screen.y + container_h;

    if (mouse_inside && !ImGui::IsAnyItemActive() && !g_panel_blocks_input)
    {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f)
            m_target_scroll -= wheel * 60.0f;
    }

    m_target_scroll = ImClamp(m_target_scroll, 0.0f, max_scroll);

    m_current_scroll += (m_target_scroll - m_current_scroll) * k_scroll;
    if (std::fabs(m_target_scroll - m_current_scroll) < 0.05f)
        m_current_scroll = m_target_scroll;

    const float base_x = ImGui::GetCursorPosX();
    const float base_y = ImGui::GetCursorPosY();

    for (size_t i = 0; i < modules.size(); i++)
    {
        const int col = static_cast<int>(i) % columns;
        const int row = static_cast<int>(i) / columns;

        const float x = base_x + padding_x + col * (card_width + card_spacing);
        const float y = base_y + padding_y + row * (card_height + card_spacing)
                        - m_current_scroll;

        ImGui::SetCursorPos(ImVec2(x, y));

        const ImVec2 card_pos = ImGui::GetCursorScreenPos();

        const float card_bottom = card_pos.y + card_height;
        const float clip_top    = child_screen.y;
        const float clip_bottom = child_screen.y + container_h;
        if (card_bottom < clip_top || card_pos.y > clip_bottom)
            continue;

        ImGui::PushID(static_cast<int>(i));
        render_module_card(modules[i], card_pos, card_width);
        ImGui::PopID();
    }

    if (m_panel_anim_t >= 0.999f && !m_dots_clicked_this_frame)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            const ImVec2 m   = ImGui::GetMousePos();
            const ImVec2 dsp = ImGui::GetIO().DisplaySize;

            constexpr float panel_open_w = 400.0f;
            constexpr float titlebar_h   = 50.0f;

            const float panel_x0 = dsp.x - panel_open_w;

            const bool in_titlebar  = (m.y < titlebar_h);
            const bool inside_panel = (m.x >= panel_x0 && m.y >= titlebar_h);

            if (!in_titlebar && !inside_panel)
            {
                m_open_panel_module = nullptr;
                m_click_consumed_by_panel = true;
                g_panel_blocks_input = false;
            }
        }
    }

    if (content_h > container_h)
    {
        ImDrawList* dl_sb = ImGui::GetWindowDrawList();

        const float track_x0 = child_screen.x + container_w - sb_width;
        const float track_x1 = track_x0 + sb_width;
        const float track_y0 = child_screen.y + 0.0f;
        const float track_y1 = child_screen.y + container_h - 2.0f;
        const float track_h  = track_y1 - track_y0;

        const float view_ratio = container_h / content_h;
        float thumb_h = track_h * view_ratio;
        if (thumb_h < sb_min_thumb) thumb_h = sb_min_thumb;
        if (thumb_h > track_h)      thumb_h = track_h;

        const float scroll_t = (max_scroll > 0.0f)
                             ? (m_current_scroll / max_scroll)
                             : 0.0f;
        const float thumb_y = track_y0 + scroll_t * (track_h - thumb_h);

        constexpr float hit_extra = 5.0f;
        ImGui::SetCursorScreenPos(ImVec2(track_x0 - hit_extra, track_y0));
        ImGui::PushID("##custom_sb");
        ImGui::InvisibleButton("##sb_hit",
                               ImVec2(sb_width + hit_extra * 2.0f, track_h));

        static bool  s_dragging    = false;
        static float s_drag_offset = 0.0f;

        if (ImGui::IsItemActivated() && !g_panel_blocks_input)
        {
            const float my = ImGui::GetMousePos().y;
            if (my >= thumb_y && my <= thumb_y + thumb_h)
                s_drag_offset = my - thumb_y;
            else
                s_drag_offset = thumb_h * 0.5f;
            s_dragging = true;
        }
        if (s_dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !g_panel_blocks_input)
        {
            const float my = ImGui::GetMousePos().y;
            const float new_thumb_y = my - s_drag_offset;
            const float denom = (track_h - thumb_h);
            const float t = (denom > 0.0f)
                          ? (new_thumb_y - track_y0) / denom
                          : 0.0f;
            const float clamped = ImClamp(t, 0.0f, 1.0f);
            const float new_scroll = clamped * max_scroll;

            m_target_scroll  = new_scroll;
            m_current_scroll = new_scroll;
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            s_dragging = false;

        const ImU32 thumb_col = IM_COL32(20, 19, 24, 255);

        dl_sb->AddRectFilled(ImVec2(track_x0, thumb_y),
                             ImVec2(track_x1, thumb_y + thumb_h),
                             thumb_col,
                             sb_width * 0.5f);

        ImGui::PopID();

        ImGui::SetCursorPos(ImVec2(0.0f, content_h));
        ImGui::Dummy(ImVec2(1.0f, 1.0f));
    }

    ImGui::EndChild();

    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(1);

    if (m_panel_rendering_module && m_module_panel_width > 0.5f)
    {
        render_module_panel();
    }
}

void main_screen::render_module_panel()
{
    if (!m_panel_rendering_module) return;

    constexpr float panel_w_full  = 400.0f;
    constexpr float header_h      = 45.0f;
    constexpr float titlebar_h    = 50.0f;
    constexpr float footer_h      = 12.0f;

    const ImVec2 win_pos  = ImGui::GetWindowPos();
    const ImVec2 win_size = ImGui::GetWindowSize();

    const float panel_h  = win_size.y - titlebar_h;
    const float panel_x1 = win_pos.x + win_size.x;
    const float panel_y0 = win_pos.y + titlebar_h;
    const float panel_y1 = win_pos.y + win_size.y;

    const float visible_w = panel_w_full * m_panel_anim_t;
    const float panel_x0  = panel_x1 - visible_w;

    const float alpha    = m_panel_anim_t;
    const float alpha_sq = alpha * alpha;
    const int   a255     = static_cast<int>(255 * alpha_sq);

    constexpr ImU32 col_titlebar = IM_COL32(23, 23, 28, 255);
    constexpr ImU32 col_body     = IM_COL32(20, 19, 24, 255);

    bool request_close = false;

    ImGui::SetCursorScreenPos(ImVec2(panel_x0, panel_y0));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha_sq);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, col_body);

    ImGui::BeginChild("##module_settings_panel",
                      ImVec2(visible_w, panel_h),
                      false,
                      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoScrollWithMouse);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    const float full_x0 = panel_x1 - panel_w_full;

    dl->PushClipRect(ImVec2(panel_x0, panel_y0),
                     ImVec2(panel_x1, panel_y1),
                     true);

    {
        const ImVec2 mouse = ImGui::GetMousePos();
        const bool inside =
            mouse.x >= panel_x0 && mouse.x <= panel_x1 &&
            mouse.y >= panel_y0 && mouse.y <= panel_y1;

        const float content_top    = panel_y0 + header_h + 12.0f;
        const float content_bottom = panel_y1;
        const float visible_h      = content_bottom - content_top;

        float content_h = 0.0f;
        if (m_panel_content_bottom > content_top)
            content_h = m_panel_content_bottom - content_top;

        constexpr float bottom_pad = 20.0f;
        const float max_scroll = (std::max)(0.0f, content_h - visible_h + bottom_pad);

        if (inside)
        {
            const float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f)
                m_panel_scroll_target -= wheel * 40.0f;
        }

        m_panel_scroll_target = ImClamp(m_panel_scroll_target, 0.0f, max_scroll);

        const float dt = ImGui::GetIO().DeltaTime;
        const float k  = 1.0f - std::exp(-22.0f * dt);
        m_panel_scroll += (m_panel_scroll_target - m_panel_scroll) * k;
        if (std::fabs(m_panel_scroll_target - m_panel_scroll) < 0.05f)
            m_panel_scroll = m_panel_scroll_target;
    }

    {
        dl->AddRectFilled(ImVec2(full_x0, panel_y0),
                          ImVec2(panel_x1, panel_y0 + header_h),
                          col_titlebar);

        dl->AddRectFilled(ImVec2(full_x0, panel_y0 + header_h),
                          ImVec2(panel_x1, panel_y1),
                          col_body);

        dl->AddLine(ImVec2(full_x0, panel_y0 + header_h),
                    ImVec2(panel_x1, panel_y0 + header_h),
                    IM_COL32(40, 42, 46, 255), 1.0f);

        constexpr float arrow_size = 18.0f;
        constexpr float arrow_left = 14.0f;
        constexpr float title_gap  = 24.0f;
        constexpr float hit_pad    = 6.0f;

        const ImVec2 arrow_pos(full_x0 + arrow_left,
                               panel_y0 + (header_h - arrow_size) * 0.5f);
        const ImVec2 arrow_end(arrow_pos.x + arrow_size,
                               arrow_pos.y + arrow_size);

        ImGui::SetCursorScreenPos(ImVec2(arrow_pos.x - hit_pad,
                                         arrow_pos.y - hit_pad));
        ImGui::PushID("##panel_close_btn");
        ImGui::InvisibleButton("##hit",
                               ImVec2(arrow_size + hit_pad * 2.0f,
                                      arrow_size + hit_pad * 2.0f));
        const bool hover_arrow = ImGui::IsItemHovered();
        const bool click_arrow = ImGui::IsItemClicked();
        ImGui::PopID();

        if (right_arrow_text.src)
        {
            dl->AddImage((ImTextureID)right_arrow_text.src,
                         arrow_pos, arrow_end,
                         ImVec2(0, 0), ImVec2(1, 1),
                         hover_arrow ? IM_COL32(255, 255, 255, a255)
                                     : IM_COL32(180, 180, 180, a255));
        }

        if (click_arrow)
            request_close = true;

        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() + 5,
                    ImVec2(arrow_end.x + title_gap,
                           panel_y0 + (header_h - ImGui::GetFontSize()) * 0.5f - 3),
                    IM_COL32(255, 255, 255, a255),
                    m_panel_rendering_module->get_name());

        if (favorites_text.src)
        {
            const bool pinned = m_pinned.count(m_panel_rendering_module) > 0;

            constexpr float fav_size  = 18.0f;
            constexpr float fav_right = 14.0f;

            const ImVec2 fav_pos(panel_x1 - fav_size - fav_right,
                                 panel_y0 + (header_h - fav_size) * 0.5f);
            const ImVec2 fav_end(fav_pos.x + fav_size, fav_pos.y + fav_size);

            ImGui::SetCursorScreenPos(ImVec2(fav_pos.x - hit_pad,
                                             fav_pos.y - hit_pad));
            ImGui::PushID("##panel_fav_btn");
            ImGui::InvisibleButton("##hit",
                                   ImVec2(fav_size + hit_pad * 2.0f,
                                          fav_size + hit_pad * 2.0f));
            const bool hover_fav = ImGui::IsItemHovered();
            const bool click_fav = ImGui::IsItemClicked();
            ImGui::PopID();

            const ImU32 fav_col = pinned
                ? IM_COL32(255, 200, 60, a255)
                : (hover_fav ? IM_COL32(200, 200, 205, a255)
                             : IM_COL32(150, 150, 155, a255));

            dl->AddImage((ImTextureID)favorites_text.src,
                         fav_pos, fav_end,
                         ImVec2(0, 0), ImVec2(1, 1), fav_col);

            if (click_fav)
            {
                if (pinned)
                    m_pinned.erase(m_panel_rendering_module);
                else
                    m_pinned.insert(m_panel_rendering_module);
            }
        }

        if (alpha > 0.3f)
        {
            const float content_x = full_x0;
            const float content_y = panel_y0 + header_h + 12.0f - m_panel_scroll;

            dl->PushClipRect(ImVec2(full_x0, panel_y0 + header_h),
                             ImVec2(panel_x1, panel_y1),
                             true);

            ImGui::SetCursorScreenPos(ImVec2(content_x, content_y));

            g_current_module_enabled = m_panel_rendering_module->is_enabled();
            m_panel_rendering_module->render_panel_components();
            g_current_module_enabled = false;

            const float cursor_after = ImGui::GetCursorScreenPos().y;
            m_panel_content_bottom = cursor_after + m_panel_scroll;

            dl->PopClipRect();
        }
    }

    dl->PopClipRect();

    ImGui::EndChild();

    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(3);

    if (request_close)
        m_open_panel_module = nullptr;
}
void main_screen::render_tabs()
{
    constexpr float row_height     = 70.0f;
    constexpr float skipped_row    = 35.0f;
    constexpr float font_size      = 25.0f;
    constexpr float start_x        = 40.0f;
    constexpr float underline_gap  = 6.0f;
    constexpr float underline_h    = 2.0f;
    constexpr float anim_speed     = 20.0f;
    constexpr float anim_epsilon   = 0.001f;

    constexpr float icon_size      = 20.0f;
    constexpr float icon_spacing   = 10.0f;

    auto* tm = [&]() -> theme_manager* {
        if (!global_module_manager) return nullptr;
        auto* m = global_module_manager->get_module_by_name("Theme manager");
        return static_cast<theme_manager*>(m);
    }();

    const bool category_accent =
        (tm == nullptr) || tm->category_underline_accent_enabled();

    const ImU32 accent_color = category_accent
        ? ImGui::GetColorU32(ImGuiCol_Button)
        : IM_COL32(47, 122, 229, 255);

    const ImU32 white        = IM_COL32(255, 255, 255, 255);
    const ImU32 inactive     = IM_COL32(44, 44, 49, 255);

    ImFont*     font = ImGui::GetFont();
    ImDrawList* dl   = ImGui::GetWindowDrawList();

    const float k   = 1.0f - std::exp(-anim_speed * ImGui::GetIO().DeltaTime);
    const int count = std::min<int>(categories::count(), kMaxTabs);

    float t_y = ImGui::GetCursorPosY() + 95.0f;

    for (int i = 0; i < count; i++)
    {
        const bool   selected  = (i == m_current_tab);
        const char*  label     = categories::name(i);
        const ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, label);

        if (text_size.x <= 0 || text_size.y <= 0)
        {
            t_y += skipped_row;
            continue;
        }

        const texture* icon = nullptr;
        switch (categories::id(i))
        {
            case category::combat:   icon = &combat_text;   break;
            case category::visual:   icon = &visual_text;   break;
            case category::movement: icon = &movement_text; break;
            case category::misc:     icon = &misc_text;     break;
            case category::settings: icon = &settings_text; break;
            default: break;
        }

        const float total_w = icon_size + icon_spacing + text_size.x;
        const float total_h = (std::max)(icon_size, text_size.y);

        ImGui::SetCursorPos(ImVec2(start_x, t_y));
        ImGui::PushID(i);
        const bool clicked = ImGui::InvisibleButton("##tab", ImVec2(total_w, total_h));
        ImGui::PopID();

        if (ImGui::IsItemHovered() && !g_panel_blocks_input)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        if (clicked && !g_panel_blocks_input)
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

        if (category_accent && line_anim > anim_epsilon)
        {
            const float half_width = total_w * 0.6f * 0.5f * line_anim;
            const float center_x   = item_pos.x + total_w * 0.5f;
            const float line_y     = item_pos.y + total_h + underline_gap;

            dl->AddRectFilled(ImVec2(center_x - half_width, line_y),
                              ImVec2(center_x + half_width, line_y + underline_h),
                              ImGui::GetColorU32(ImGuiCol_Button),
                              1.0f);
        }

        t_y += row_height;
    }

    ImGui::Dummy(ImVec2(0, 10));
}