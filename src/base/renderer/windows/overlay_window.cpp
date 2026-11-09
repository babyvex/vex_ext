//
// Created by natty on 25.09.2026.
//

#include "overlay_window.h"

#include <dwmapi.h>

#include "../../features/module_manager.h"
#include "../utils/win_utils.h"
ImGuiContext* overlay_window::imgui_ctx = nullptr;
HWND overlay_window::hwnd = nullptr;
overlay_window::overlay_window() :
abstract_window({
    GetModuleHandleW(nullptr),
    "overlay_mullo_window",
    "vex",
    WS_POPUP,
    WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
    0, 0,
    GetSystemMetrics(SM_CXSCREEN),
    GetSystemMetrics(SM_CYSCREEN),
    &overlay_window::WndProc,
    false, true
  })
{ }
LRESULT overlay_window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    overlay_window* self = static_cast<overlay_window*>(from_hwnd(hwnd, msg, lParam));
    if (!self)
        return DefWindowProcA(hwnd, msg, wParam, lParam);

    if (auto r = gfx::message_handler(self->gfx_ctx, msg, wParam, lParam))
        return *r;

    if (msg == WM_CLOSE || msg == WM_DESTROY) {
        self->on_close();
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void overlay_window::load_fonts(ImGuiIO &io) {
}

void overlay_window::render_frame() {
    global_module_manager->tick_render_modules();
}

void overlay_window::apply_window_attributes(HWND hwnd) {
    const MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
}

void overlay_window::on_tick() {
    HWND mc = get_attached_minecraft_window();
    if (!mc || !IsWindow(mc) || IsIconic(mc)) {
        if (IsWindowVisible(gfx_ctx.hwnd)) ShowWindow(gfx_ctx.hwnd, SW_HIDE);
        return;
    }

    if (GetForegroundWindow() != mc) {
        if (IsWindowVisible(gfx_ctx.hwnd)) ShowWindow(gfx_ctx.hwnd, SW_HIDE);
        return;
    }

    if (!IsWindowVisible(gfx_ctx.hwnd)) ShowWindow(gfx_ctx.hwnd, SW_SHOWNOACTIVATE);

    RECT destRect;
    GetWindowRect(mc, &destRect);
    int x = destRect.left;
    int y = destRect.top;
    int w = destRect.right - destRect.left;
    int h = destRect.bottom - destRect.top;

    if (w <= 0 || h <= 0) return;

    static int last_x = -1, last_y = -1, last_w = -1, last_h = -1;
    if (x != last_x || y != last_y || w != last_w || h != last_h) {
        SetWindowPos(gfx_ctx.hwnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
        last_x = x; last_y = y; last_w = w; last_h = h;
        gfx::resize(gfx_ctx, static_cast<UINT>(w), static_cast<UINT>(h));
    }
}

void overlay_window::on_after_create() {
    SetLayeredWindowAttributes(gfx_ctx.hwnd, 0, 255, LWA_ALPHA);
    imgui_ctx = gfx_ctx.imgui;
    hwnd = gfx_ctx.hwnd;
}

void overlay_window::on_close() {
}

void overlay_window::on_before_destroy() {
    imgui_ctx = nullptr;
    hwnd = nullptr;   
}
