//
// Created by natty on 25.09.2026.
//

#include "main_window.h"

#include <dwmapi.h>

#include "../../features/module_manager.h"
#include "../../features/impl/settings/borderless.hpp"
#include "../components/components.h"
#include "../gui/screens/configs_screen.h"
#include "../gui/screens/friends_screen.h"
#include "../gui/screens/main_screen.h"
#include "../gui/screens/screen_id.h"
#include "../resources/font/Inter28pt_Regular.h"

HWND main_window::hwnd =  nullptr;
ID3D11Device* main_window::d3d11_device =  nullptr;
ImGuiContext* main_window::imgui_ctx = nullptr;

static borderless_module* get_borderless_module() {
    if (!global_module_manager) return nullptr;
    auto* m = global_module_manager->get_module_by_name("Borderless");
    return static_cast<borderless_module*>(m);
}

main_window::main_window()
    : abstract_window({
        GetModuleHandleW(nullptr),
        "main_mullo_window",
        "vex",
        WS_POPUP,
        WS_EX_TOPMOST  | WS_EX_APPWINDOW,
        0, 0,
        static_cast<int>(abstract_screen::kDefaultWidth),
        static_cast<int>(abstract_screen::kDefaultHeight),
        &main_window::WndProc,
        true, false
      }) {
    scr_manager.register_screen(static_cast<int>(screen_id::main), std::make_unique<main_screen>());
    scr_manager.register_screen(static_cast<int>(screen_id::friends), std::make_unique<friends_screen>());
    scr_manager.register_screen(static_cast<int>(screen_id::configs), std::make_unique<configs_screen>());
}

static void release_imgui_mouse(HWND hwnd, gfx_context& ctx)
{
    if (ctx.imgui)
    {
        ImGui::SetCurrentContext(ctx.imgui);
        ImGuiIO& io = ImGui::GetIO();
        for (int b = 0; b < ImGuiMouseButton_COUNT; ++b)
            io.AddMouseButtonEvent(b, false);
    }

    if (GetCapture() == hwnd)
        ReleaseCapture();
}

LRESULT main_window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    main_window* self = static_cast<main_window*>(from_hwnd(hwnd, msg, lParam));
    if (!self)
        return DefWindowProcA(hwnd, msg, wParam, lParam);


    static UINT s_wmTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    if (s_wmTaskbarCreated != 0 && msg == s_wmTaskbarCreated)
    {
        if (self->tray_active)
        {
            self->show_tray_icon(false);
            self->show_tray_icon(true);
        }
        return 0;
    }


    if (msg == WM_TRAYICON)
    {
        switch (LOWORD(lParam))
        {
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            return 0;

        case WM_RBUTTONUP:
        {
            POINT pt; GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            if (hMenu)
            {
                InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_OPEN, L"Open GUI");
                InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
                InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, L"Exit");

                SetForegroundWindow(hwnd);
                int cmd = TrackPopupMenu(hMenu,
                    TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON,
                    pt.x, pt.y, 0, hwnd, nullptr);
                PostMessageW(hwnd, WM_NULL, 0, 0);
                DestroyMenu(hMenu);

                if (cmd == ID_TRAY_OPEN)
                {
                    ShowWindow(hwnd, SW_RESTORE);
                    SetForegroundWindow(hwnd);
                }
                else if (cmd == ID_TRAY_EXIT)
                {
                    self->request_close();
                }
            }
            return 0;
        }
        default:
            break;
        }
        return 0;
    }

    if (msg == WM_SYSCOMMAND && (wParam & 0xfff0) == SC_KEYMENU)
        return 0;

    if (msg == WM_ACTIVATE || msg == WM_SIZE || msg == WM_KILLFOCUS)
        release_imgui_mouse(hwnd, self->gfx_ctx);


    if (auto r = gfx::message_handler(self->gfx_ctx, msg, wParam, lParam))
        return *r;

    if (msg == WM_CLOSE || msg == WM_DESTROY)
    {
        self->on_close();
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void main_window::load_fonts(ImGuiIO &io) {
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(
        (void*)FontBytes_InterRegular24, FontSize_InterRegular24,
        18.0f, &cfg, io.Fonts->GetGlyphRangesDefault());
}

void main_window::render_frame() {
    scr_manager.tick();
}

void main_window::apply_window_attributes(HWND hwnd) {
    //SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
   // const MARGINS margin = { -1, -1, -1, -1 };
    //DwmExtendFrameIntoClientArea(hwnd, &margin);
}

void main_window::on_tick() {
    KeybindComponent::PollAllHotkeys();
}

void main_window::on_after_create() {
    show_tray_icon(true);

    hwnd = gfx_ctx.hwnd;
    d3d11_device = gfx_ctx.device;

    scr_manager.switch_to(static_cast<int>(screen_id::main));

    ShowWindow(gfx_ctx.hwnd, SW_SHOW);
    SetWindowPos(gfx_ctx.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    SetForegroundWindow(gfx_ctx.hwnd);

    imgui_ctx = gfx_ctx.imgui;
}

void main_window::on_close() {
    if (auto* bm = get_borderless_module())
        bm->request_disable();
    request_close();
}

void main_window::on_before_destroy() {

    if (auto* bm = get_borderless_module())
        bm->request_disable();
    show_tray_icon(false);
    hwnd = nullptr;
    d3d11_device = nullptr;

    show_tray_icon(false);
    hwnd = nullptr;
    d3d11_device = nullptr;
    imgui_ctx = nullptr;
}

void main_window::show_tray_icon(bool show) {
    if (show)
    {
        if (tray_active || !gfx_ctx.hwnd) return;

        ZeroMemory(&tray_nid, sizeof(tray_nid));
        tray_nid.cbSize           = sizeof(NOTIFYICONDATAW);
        tray_nid.hWnd             = gfx_ctx.hwnd;
        tray_nid.uID              = 1;
        tray_nid.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        tray_nid.uCallbackMessage = WM_TRAYICON;

        tray_nid.hIcon = (HICON)GetClassLongPtrA(gfx_ctx.hwnd, GCLP_HICONSM);
        if (!tray_nid.hIcon) tray_nid.hIcon = (HICON)GetClassLongPtrA(gfx_ctx.hwnd, GCLP_HICON);
        if (!tray_nid.hIcon) tray_nid.hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1));
        //if (!tray_nid.hIcon) tray_nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);

        wcscpy_s(tray_nid.szTip, L"vex");

        if (Shell_NotifyIconW(NIM_ADD, &tray_nid))
            tray_active = true;
    }
    else
    {
        if (!tray_active) return;
        Shell_NotifyIconW(NIM_DELETE, &tray_nid);
        tray_active = false;
    }
}
