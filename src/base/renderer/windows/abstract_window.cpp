//
// Created by natty on 25.09.2026.
//

#include "abstract_window.h"

#include <utility>

#include "../utils/win_utils.h"
#include "ImGui/imgui_impl_win32.h"

abstract_window::abstract_window(config config)
    : cfg(std::move(config)) {
}

HWND abstract_window::create_native_window(const config &c, int w, int h) {
    return ::CreateWindowExA(
          c.ex_style, c.class_name.c_str(), c.title.c_str(), c.style,
          c.x, c.y, w, h,
          nullptr, nullptr, c.hinstance, this);
}

abstract_window * abstract_window::from_hwnd(HWND hwnd, UINT msg, LPARAM lparam) {
    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTA*>(lparam);
        auto* self = static_cast<abstract_window*>(cs->lpCreateParams);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        return self;
    }
    return reinterpret_cast<abstract_window *>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
}

void abstract_window::load_fonts_callback(ImGuiIO &io, void *user) {
    static_cast<abstract_window*>(user)->load_fonts(io);
}

bool abstract_window::create_window(bool f) {
    if (initialized) return true;

    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEXA wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = cfg.wnd_proc;
    wc.hInstance     = cfg.hinstance;
    wc.hbrBackground = nullptr;
    wc.hCursor       = LoadCursorA(nullptr, IDC_ARROW);
    wc.hIcon         = LoadIconA(nullptr, IDI_APPLICATION);
    wc.hIconSm       = LoadIconA(nullptr, IDI_APPLICATION);
    wc.lpszClassName = cfg.class_name.c_str();
    ::RegisterClassExA(&wc);

    const int screen_w = GetSystemMetrics(SM_CXSCREEN);
    const int screen_h = GetSystemMetrics(SM_CYSCREEN);
    const int w = cfg.w ? cfg.w : screen_w;
    const int h = cfg.h ? cfg.h : screen_h;

    if (cfg.center) {
        cfg.x = (screen_w - w) / 2;
        cfg.y = (screen_h - h) / 2;
    }

    gfx_ctx.hwnd = create_native_window(cfg, w, h);
    if (!gfx_ctx.hwnd) {
        ::UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    apply_window_attributes(gfx_ctx.hwnd);

    if (!gfx::create_3d_device(gfx_ctx, f)) {
        gfx::cleanup_3d_device(gfx_ctx);
        ::DestroyWindow(gfx_ctx.hwnd);
        gfx_ctx.hwnd = nullptr;
        ::UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    ::ShowWindow(gfx_ctx.hwnd, SW_SHOWNOACTIVATE);
    ::UpdateWindow(gfx_ctx.hwnd);

    gfx::init_imgui(gfx_ctx, &abstract_window::load_fonts_callback, this);

    initialized = true;
    on_after_create();
    return true;
}

void abstract_window::tick() {
    if (!initialized) return;

    MSG msg;
    while (::PeekMessageA(&msg,  gfx_ctx.hwnd, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageA(&msg);

        if (msg.message == WM_QUIT)
        {
            on_close();
            return;
        }
    }

    if (!gfx_ctx.hwnd || ::IsIconic(gfx_ctx.hwnd))
    {
        ::Sleep(10);
        return;
    }

    on_tick();

    gfx::begin_frame(gfx_ctx);
    render_frame();
    gfx::end_frame(gfx_ctx);
}

void abstract_window::destroy_window() {

    if (!initialized) return;

    on_before_destroy();

    HWND hwnd = gfx_ctx.hwnd;

    gfx::destroy_imgui(gfx_ctx);
    gfx::cleanup_3d_device(gfx_ctx);

    if (hwnd)
        ::DestroyWindow(hwnd);

    gfx_ctx.hwnd = nullptr;

    ::UnregisterClassA(cfg.class_name.c_str(), cfg.hinstance);

    initialized = false;
}
