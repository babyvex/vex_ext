#pragma once
#include "ImGui/ImGuiTheme.h"

#include "../../../renderer/windows/main_window.h"
#include "../../../renderer/components/components.h"

class theme_manager : public abstract_module {
private:
    int key_bind = -1;
    float color[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
    float last[4]   = { -1, -1, -1, -1 };
    bool accent_navbar             = true;
    bool accent_category_underline = true;
    bool rainbow                   = false;
    float rainbow_speed            = 0.25f;

    bool  last_navbar             = true;
    bool  last_category_underline = true;
    bool  last_rainbow            = false;
    float last_rainbow_speed      = 0.25f;

    float m_rainbow_hue = 0.0f;

public:
    theme_manager()
        : abstract_module("Theme manager", "Change your theme", category::settings)
    {
        add_component(new ColorPickerComponent(
            "Accent color",
            color,
            true
        ));

        add_component(new OvalButton(
            "Accent: navbar icons",
            &accent_navbar
        ));
        add_component(new OvalButton(
            "Accent: category underline",
            &accent_category_underline
        ));
        add_component(new OvalButton(
            "Rainbow mode",
            &rainbow
        ));
    }

    void on_tick() override {  }

    void on_render_tick() override {
        const bool color_changed =
            last[0] != color[0] || last[1] != color[1] ||
            last[2] != color[2] || last[3] != color[3];

        const bool settings_changed =
            last_navbar             != accent_navbar ||
            last_category_underline != accent_category_underline ||
            last_rainbow            != rainbow ||
            last_rainbow_speed      != rainbow_speed;

        if (!color_changed && !settings_changed && !rainbow) return;

        last[0] = color[0]; last[1] = color[1];
        last[2] = color[2]; last[3] = color[3];

        last_navbar             = accent_navbar;
        last_category_underline = accent_category_underline;
        last_rainbow            = rainbow;
        last_rainbow_speed      = rainbow_speed;

        ImGuiContext* ctx = main_window::get_imgui_context();
        if (!ctx) return;

        ImGuiContext* prev = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(ctx);

        ImVec4 c;

        if (rainbow) {
            m_rainbow_hue += rainbow_speed * ImGui::GetIO().DeltaTime;
            if (m_rainbow_hue > 1.0f) m_rainbow_hue -= 1.0f;

            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(m_rainbow_hue, 1.0f, 1.0f, r, g, b);
            c = ImVec4(r, g, b, 1.0f);
        } else {
            c = ImVec4(color[0], color[1], color[2], color[3]);
        }

        ImGuiStyle& style = ImGui::GetStyle();

        style.Colors[ImGuiCol_Button]           = c;
        style.Colors[ImGuiCol_ButtonHovered]    = c;
        style.Colors[ImGuiCol_ButtonActive]     = c;
        style.Colors[ImGuiCol_SliderGrab]       = c;
        style.Colors[ImGuiCol_SliderGrabActive] = c;

        if (prev && prev != ctx) ImGui::SetCurrentContext(prev);
    }

    void on_enable()  override { }
    void on_disable() override { }

    bool navbar_accent_enabled()             const { return accent_navbar; }
    bool category_underline_accent_enabled() const { return accent_category_underline; }
    bool rainbow_enabled()                   const { return rainbow; }
};