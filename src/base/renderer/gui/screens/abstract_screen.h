#pragma once
#include <Windows.h>

#include "../../windows/texture.h"
#include "Imgui/imgui.h"

extern bool g_panel_blocks_input;

class abstract_screen
{
public:


    static constexpr float kDefaultWidth   = 1080.0f;
    static constexpr float kDefaultHeight  = 675.0f;
    static constexpr float kTitleBarHeight = 50.0f;

    virtual ~abstract_screen()
    {
        m_icon_close.release();
        m_icon_minimize.release();
    }

    abstract_screen(const abstract_screen&)            = delete;
    abstract_screen& operator=(const abstract_screen&) = delete;

    virtual void enter() {}
    virtual void exit()  {}

    void draw();




    int take_switch_request()
    {
        const int id = m_switch_request;
        m_switch_request = -1;
        return id;
    }


    void release_gpu()
    {
        m_icon_close.release();
        m_icon_minimize.release();
        on_release_gpu();
    }

protected:
    abstract_screen() = default;

    virtual void draw_screen_content() = 0;
    virtual void draw_title_bar_content() {}
    virtual void on_release_gpu() {}

    void request_switch(int screen) { m_switch_request = screen; }

    virtual ImVec2 get_screen_size() const { return ImVec2(kDefaultWidth, kDefaultHeight); }
    virtual ImGuiWindowFlags get_window_flags() const
    {
        return ImGuiWindowFlags_NoResize
             | ImGuiWindowFlags_NoMove
             | ImGuiWindowFlags_NoTitleBar
             | ImGuiWindowFlags_NoCollapse
             | ImGuiWindowFlags_NoSavedSettings;
    }
    virtual bool should_show_title_bar() const { return true; }
    virtual const char* get_window_name() const { return "##abstract_screen"; }

    ImVec2 window_pos()  const { return m_position; }
    ImVec2 window_size() const { return m_size; }

private:
    void begin_window();
    void end_window();
    void draw_title_bar();
    void draw_window_buttons();
    void draw_title_bar_drag_area();

    ImVec2 m_position{};
    ImVec2 m_size{};

    texture m_icon_close;
    texture m_icon_minimize;
    bool    m_icons_loaded = false;

    int     m_switch_request = -1;

    POINT   m_drag_cursor_start{};
    POINT   m_drag_window_start{};
};

