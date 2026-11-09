//
// Created by natty on 25.09.2026.
//
#pragma once
#include "abstract_window.h"
#include "d2d1_render/d2d1_renderer.h"


class overlay_window : public abstract_window {
    public:
    overlay_window();

    static LRESULT __stdcall WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void request_close()      { state = true; }
    [[nodiscard]] bool should_close() const
    {
        return state;
    }

    static HWND get_main_hwnd() { return hwnd; }

    static ID3D11Device* get_d3d11_device() { return d3d11_device; }
    static ImGuiContext* get_imgui_context() { return imgui_ctx; }

protected:

    void load_fonts(ImGuiIO& io) override;
    void render_frame() override;

    void apply_window_attributes(HWND hwnd) override;

    void on_tick() override;
    void on_after_create() override;
    void on_close() override;
    void on_before_destroy() override;

private:

    static ImGuiContext* imgui_ctx;
    bool state = false;

    static HWND hwnd;

    static ID3D11Device* d3d11_device;

    d2d1_drawer* drawer = new d2d1_drawer();

};


