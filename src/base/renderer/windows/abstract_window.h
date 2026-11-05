//
// Created by natty on 25.09.2026.
//

#pragma once

#include "../utils/band_window.h"

#include <string>

#include "../gfx/gfx_context.h"

class abstract_window {

public:
    struct config {
        HINSTANCE hinstance = nullptr;
        std::string class_name;
        std::string title;
        DWORD style = 0;
        DWORD ex_style = 0;
        int x = 0, y = 0, w = 0, h = 0;
        WNDPROC wnd_proc = nullptr;
        bool center = false;
        bool transparent = false;
    };

    explicit abstract_window(config config);
    virtual ~abstract_window() = default;

    abstract_window(const abstract_window&) = delete;
    abstract_window& operator=(const abstract_window&) = delete;

    bool create_window(bool transparent = false);

    void tick();

    void destroy_window();

    [[nodiscard]] HWND get_hwnd() const { return gfx_ctx.hwnd; }

protected:

    virtual void load_fonts(ImGuiIO& io) = 0;
    virtual void render_frame() = 0;

    virtual HWND create_native_window(const config& config, int w, int h);

    virtual void on_tick() {}  // for window tracking , for check keybinding
    virtual void on_close() {}
    virtual void on_after_create() {}
    virtual void on_before_destroy() {}

    virtual void apply_window_attributes(HWND hwnd) {}

    static abstract_window* from_hwnd(HWND hwnd, UINT msg, LPARAM lparam);

    gfx_context gfx_ctx;
private:

    config cfg;

    bool initialized = false;
    static void load_fonts_callback(ImGuiIO& io, void* user);

};
