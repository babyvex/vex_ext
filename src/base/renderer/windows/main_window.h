//
// Created by natty on 25.09.2026.
//

#pragma once

#include "abstract_window.h"
#include "../gui/screens/screen_manager.h"

static constexpr UINT WM_TRAYICON   = WM_APP + 101;
static constexpr UINT ID_TRAY_OPEN  = 1001;
static constexpr UINT ID_TRAY_EXIT  = 1002;

class main_window : public abstract_window {

public:
    static LRESULT __stdcall WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    main_window();

    void request_close()      { state = true; }
    [[nodiscard]] bool should_close() const
    {
        return state;
    }

    static HWND get_main_hwnd() { return hwnd; }
    static ImGuiContext* get_imgui_context() { return imgui_ctx; }
    static ID3D11Device* get_d3d11_device() { return d3d11_device; }

protected:

    void load_fonts(ImGuiIO& io) override;
    void render_frame() override;

    void apply_window_attributes(HWND hwnd) override;

    void on_tick() override;
    void on_after_create() override;
    void on_close() override;
    void on_before_destroy() override;

private:

    bool state = false;

    ImFont* main_window_font = nullptr;

    screen_manager scr_manager;

    static HWND hwnd;

    static ID3D11Device* d3d11_device;

    NOTIFYICONDATAW tray_nid{};
    bool            tray_active = false;

    static ImGuiContext* imgui_ctx;

    void show_tray_icon(bool show);
};

