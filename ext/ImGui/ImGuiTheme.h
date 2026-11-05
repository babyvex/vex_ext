#ifndef UNIQUE_CC_IMGUITHEME_H
#define UNIQUE_CC_IMGUITHEME_H

#include "imgui.h"

class ImGuiTheme
{
    const char* name = "Default";

    ImVec4 color_active_tab = ImVec4(0.20f, 0.27f, 0.51f, 1.00f);
    ImVec4 color_child_bg = ImVec4(0.12f, 0.11f, 0.21f, 0.31f);
    ImVec4 color_slider_grab = ImVec4(0.31f, 0.42f, 0.83f, 1.00f);
    ImVec4 color_frame_bg = ImVec4(0.20f, 0.20f, 0.20f, 0.31f);
    ImVec4 color_frame_bg_active = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    ImVec4 color_frame_bg_hovered = ImVec4(0.27f, 0.25f, 0.25f, 1.00f);
    ImVec4 color_window_bg = ImVec4(0.07f, 0.06f, 0.11f, 1.00f);
    ImVec4 color_button = ImVec4(0.12f, 0.11f, 0.21f, 0.31f);
    ImVec4 color_button_active = ImVec4(0.31f, 0.42f, 0.83f, 1.00f);
    ImVec4 color_button_hovered = ImVec4(0.20f, 0.27f, 0.51f, 1.00f);
    ImVec4 color_text = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    ImVec4 color_header = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    ImVec4 color_header_active = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    ImVec4 color_header_hovered = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    ImVec4 color_checkmark = ImVec4(0.56f, 0.18f, 0.18f, 1.00f);
    ImVec4 color_border = ImVec4(0.31f, 0.42f, 0.83f, 1.00f);
    ImVec4 color_border_shadow = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    ImVec4 color_title_bg = {};
    ImVec4 color_title_bg_active = {};
    ImVec4 color_title_bg_collapse = {};

    float rounding_frame = 0.0f;
    float rounding_window =  0.0f; //10.0f;
    float rounding_child =  0.0f;// 10.0f;
    float rounding_grab = 0.0f;//20.0f;
    float rounding_popup = 0.0f;
    float rounding_scrollbar = 0.0f;
    float rounding_tab = 0.0f;

    float window_border_size = 1.5f;
    float frame_border_size = 1.0f;
    float child_border_size = 1.0f;
    ImVec2 window_padding = ImVec2(10.0f, 10.0f);

public:
    ImGuiTheme& Apply();
    ImGuiTheme& SetName(const char* theme_name);

    ImGuiTheme& SetColorActiveTab(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorChildBg(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorSliderGrab(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorFrameBg(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorFrameBgActive(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorFrameBgHovered(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorWindowBg(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorButton(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorButtonActive(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorButtonHovered(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorText(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorHeader(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorHeaderActive(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorHeaderHovered(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorCheckmark(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorBorder(float r, float g, float b, float a = 1.0f);
    ImGuiTheme& SetColorBorderShadow(float r, float g, float b, float a = 1.0f);

    ImGuiTheme& SetColorActiveTab(const ImColor& color);
    ImGuiTheme& SetColorChildBg(const ImColor& color);
    ImGuiTheme& SetColorSliderGrab(const ImColor& color);
    ImGuiTheme& SetColorFrameBg(const ImColor& color);
    ImGuiTheme& SetColorFrameBgActive(const ImColor& color);
    ImGuiTheme& SetColorFrameBgHovered(const ImColor& color);
    ImGuiTheme& SetColorWindowBg(const ImColor& color);
    ImGuiTheme& SetColorButton(const ImColor& color);
    ImGuiTheme& SetColorButtonActive(const ImColor& color);
    ImGuiTheme& SetColorButtonHovered(const ImColor& color);
    ImGuiTheme& SetColorText(const ImColor& color);
    ImGuiTheme& SetColorHeader(const ImColor& color);
    ImGuiTheme& SetColorHeaderActive(const ImColor& color);
    ImGuiTheme& SetColorHeaderHovered(const ImColor& color);
    ImGuiTheme& SetColorCheckmark(const ImColor& color);
    ImGuiTheme& SetColorBorder(const ImColor& color);
    ImGuiTheme& SetColorBorderShadow(const ImColor& color);
    ImGuiTheme& SetColorTitleBg(const ImColor& color);
    ImGuiTheme& SetColorTitleBgActive(const ImColor& color);
    ImGuiTheme& SetColorTitleBgCollapse(const ImColor& color);

    ImGuiTheme& SetRoundingFrame(float value);
    ImGuiTheme& SetRoundingWindow(float value);
    ImGuiTheme& SetRoundingChild(float value);
    ImGuiTheme& SetRoundingGrab(float value);
    ImGuiTheme& SetRoundingPopup(float value);
    ImGuiTheme& SetRoundingScrollbar(float value);
    ImGuiTheme& SetRoundingTab(float value);

    ImGuiTheme& SetWindowBorderSize(float size);
    ImGuiTheme& SetFrameBorderSize(float size);
    ImGuiTheme& SetChildBorderSize(float size);
    ImGuiTheme& SetWindowPadding(float x, float y);

    static ImGuiTheme& DefaultImGui();
};

#endif // UNIQUE_CC_IMGUITHEME_H