#include "ImGuiTheme.h"

ImGuiTheme& ImGuiTheme::Apply()
{
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.Colors[ImGuiCol_CheckMark] = color_checkmark;
    style.Colors[ImGuiCol_SliderGrab] = color_slider_grab;
    style.Colors[ImGuiCol_SliderGrabActive] = color_slider_grab;
    style.Colors[ImGuiCol_FrameBg] = color_frame_bg;
    style.Colors[ImGuiCol_FrameBgActive] = color_frame_bg_active;
    style.Colors[ImGuiCol_FrameBgHovered] = color_frame_bg_hovered;
    style.Colors[ImGuiCol_WindowBg] = color_window_bg;
    style.Colors[ImGuiCol_Button] = color_button;
    style.Colors[ImGuiCol_ButtonHovered] = color_button_hovered;
    style.Colors[ImGuiCol_ButtonActive] = color_button_active;
    style.Colors[ImGuiCol_Header] = color_header;
    style.Colors[ImGuiCol_Text] = color_text;
    style.Colors[ImGuiCol_HeaderActive] = color_header_active;
    style.Colors[ImGuiCol_HeaderHovered] = color_header_hovered;
    style.Colors[ImGuiCol_ChildBg] = color_child_bg;
    style.Colors[ImGuiCol_Border] = color_border;
    style.Colors[ImGuiCol_BorderShadow] = color_border_shadow;
    style.Colors[ImGuiCol_TitleBg] = color_title_bg;
    style.Colors[ImGuiCol_TitleBgActive] = color_title_bg_active;
    style.Colors[ImGuiCol_TitleBgCollapsed] = color_title_bg_collapse;


    style.FrameRounding = rounding_frame;
    style.WindowRounding = rounding_window;
    style.ChildRounding = rounding_child;
    style.GrabRounding = rounding_grab;
    style.PopupRounding = rounding_popup;
    style.ScrollbarRounding = rounding_scrollbar;
    style.TabRounding = rounding_tab;
    style.WindowBorderSize = window_border_size;
    style.FrameBorderSize = frame_border_size;
    style.ChildBorderSize = child_border_size;
    style.WindowPadding = window_padding;
    
    return *this;
}

ImGuiTheme& ImGuiTheme::SetName(const char* theme_name)
{
    name = theme_name;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorActiveTab(float r, float g, float b, float a)
{
    color_active_tab = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorChildBg(float r, float g, float b, float a)
{
    color_child_bg = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorSliderGrab(float r, float g, float b, float a)
{
    color_slider_grab = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorFrameBg(float r, float g, float b, float a)
{
    color_frame_bg = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorFrameBgActive(float r, float g, float b, float a)
{
    color_frame_bg_active = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorFrameBgHovered(float r, float g, float b, float a)
{
    color_frame_bg_hovered = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorWindowBg(float r, float g, float b, float a)
{
    color_window_bg = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorButton(float r, float g, float b, float a)
{
    color_button = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorButtonActive(float r, float g, float b, float a)
{
    color_button_active = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorButtonHovered(float r, float g, float b, float a)
{
    color_button_hovered = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorText(float r, float g, float b, float a)
{
    color_text = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorHeader(float r, float g, float b, float a)
{
    color_header = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorHeaderActive(float r, float g, float b, float a)
{
    color_header_active = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorHeaderHovered(float r, float g, float b, float a)
{
    color_header_hovered = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorCheckmark(float r, float g, float b, float a)
{
    color_checkmark = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorBorder(float r, float g, float b, float a)
{
    color_border = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorBorderShadow(float r, float g, float b, float a)
{
    color_border_shadow = ImVec4(r, g, b, a);
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorActiveTab(const ImColor& color)
{
    color_active_tab = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorChildBg(const ImColor& color)
{
    color_child_bg = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorSliderGrab(const ImColor& color)
{
    color_slider_grab = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorFrameBg(const ImColor& color)
{
    color_frame_bg = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorFrameBgActive(const ImColor& color)
{
    color_frame_bg_active = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorFrameBgHovered(const ImColor& color)
{
    color_frame_bg_hovered = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorWindowBg(const ImColor& color)
{
    color_window_bg = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorButton(const ImColor& color)
{
    color_button = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorButtonActive(const ImColor& color)
{
    color_button_active = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorButtonHovered(const ImColor& color)
{
    color_button_hovered = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorText(const ImColor& color)
{
    color_text = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorHeader(const ImColor& color)
{
    color_header = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorHeaderActive(const ImColor& color)
{
    color_header_active = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorHeaderHovered(const ImColor& color)
{
    color_header_hovered = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorCheckmark(const ImColor& color)
{
    color_checkmark = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorBorder(const ImColor& color)
{
    color_border = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetColorBorderShadow(const ImColor& color)
{
    color_border_shadow = color.Value;
    return *this;
}

ImGuiTheme & ImGuiTheme::SetColorTitleBg(const ImColor &color) {
    color_title_bg = color.Value;
    return *this;
}

ImGuiTheme & ImGuiTheme::SetColorTitleBgActive(const ImColor &color) {
    color_title_bg_active = color.Value;
    return *this;
}

ImGuiTheme & ImGuiTheme::SetColorTitleBgCollapse(const ImColor &color) {
    color_title_bg_collapse = color.Value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetRoundingFrame(float value)
{
    rounding_frame = value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetRoundingWindow(float value)
{
    rounding_window = value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetRoundingChild(float value)
{
    rounding_child = value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetRoundingGrab(float value)
{
    rounding_grab = value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetRoundingPopup(float value)
{
    rounding_popup = value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetRoundingScrollbar(float value)
{
    rounding_scrollbar = value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetRoundingTab(float value)
{
    rounding_tab = value;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetWindowBorderSize(float size)
{
    window_border_size = size;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetFrameBorderSize(float size)
{
    frame_border_size = size;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetChildBorderSize(float size)
{
    child_border_size = size;
    return *this;
}

ImGuiTheme& ImGuiTheme::SetWindowPadding(float x, float y)
{
    window_padding = ImVec2(x, y);
    return *this;
}

ImGuiTheme& ImGuiTheme::DefaultImGui()
{
    static ImGuiTheme defaultTheme;
    ImGui::StyleColorsDark();
    return defaultTheme.SetName("Default ImGui");
}