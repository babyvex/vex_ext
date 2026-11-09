#pragma once
#include "ImGui/ImGuiTheme.h"

#include "../../../renderer/windows/main_window.h"
#include "../../../renderer/components/components.h"

class theme_manager : public abstract_module {
private:
    bool  accent_navbar             = true;
    bool  accent_category_underline = true;
    bool  rainbow                   = false;
    float rainbow_speed             = 0.25f;

    int m_selected_theme = 3;

    ImVec4 m_current_accent = ImVec4(220/255.0f, 220/255.0f, 220/255.0f, 1.0f);
    float  m_rainbow_hue    = 0.0f;

    static constexpr int kThemeCount = 8;

    ThemeSelectorComponent::Entry m_theme_entries[kThemeCount] = {
        { IM_COL32(153, 15, 2, 255),    "Dark Red" },
        { IM_COL32(224, 149, 0, 255),   "Amber" },
        { IM_COL32(0, 166, 128, 255),   "Emerald" },
        { IM_COL32(220, 220, 220, 255), "Gainsboro" },
        { IM_COL32(47, 122, 229, 255),  "Cobalt" },
        { IM_COL32(232, 96, 152, 255),  "Rose" },
        { IM_COL32(162, 84, 217, 255),  "Amethyst" },
        { IM_COL32(0x8F, 0xE3, 0xD6, 255), "Mint" },
    };

public:
    theme_manager()
        : abstract_module("Theme manager", "Change your theme", category::settings)
    {
        add_component(new ThemeSelectorComponent(
            "Themes",
            &m_selected_theme,
            m_theme_entries,
            kThemeCount
        ));

        add_component(new OvalButton(
            "Navbar icons",
            &accent_navbar
        ));
        add_component(new OvalButton(
            "Category underline",
            &accent_category_underline
        ));
        add_component(new OvalButton(
            "Rainbow mode",
            &rainbow
        ));

        add_component(new ConditionalComponent(
            &rainbow,
            new SliderComponent<float>(
                "Rainbow speed",
                &rainbow_speed,
                0.05f, 2.0f, 0.01f
            )
        ));
    }

    void on_tick() override {  }
    void on_render_tick() override {
        ImVec4 target;

        if (rainbow) {
            m_rainbow_hue += rainbow_speed * ImGui::GetIO().DeltaTime;
            if (m_rainbow_hue > 1.0f) m_rainbow_hue -= 1.0f;

            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(m_rainbow_hue, 1.0f, 1.0f, r, g, b);
            target = ImVec4(r, g, b, 1.0f);
        } else {
            const ImU32 c = m_theme_entries[m_selected_theme].color;
            target = ImVec4(((c >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
                            ((c >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
                            ((c >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f, 1.0f);
        }

        const bool changed =
            m_current_accent.x != target.x ||
            m_current_accent.y != target.y ||
            m_current_accent.z != target.z;

        if (!changed && !rainbow) return;

        m_current_accent = target;

        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Button]           = m_current_accent;
        style.Colors[ImGuiCol_ButtonHovered]    = m_current_accent;
        style.Colors[ImGuiCol_ButtonActive]     = m_current_accent;
        style.Colors[ImGuiCol_SliderGrab]       = m_current_accent;
        style.Colors[ImGuiCol_SliderGrabActive] = m_current_accent;
    }

    void on_enable()  override { }
    void on_disable() override { }

    bool navbar_accent_enabled()             const { return accent_navbar; }
    bool category_underline_accent_enabled() const { return accent_category_underline; }
    bool rainbow_enabled()                   const { return rainbow; }
};