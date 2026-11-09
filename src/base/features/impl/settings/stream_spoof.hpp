#pragma once
#include "../../abstract_module.h"
#include "../../../renderer/windows/main_window.h"
#include "../../../renderer/windows/overlay_window.h"

class stream_spoof : public abstract_module {
private:
    int key_bind = -1;

public:
    stream_spoof()
        : abstract_module("Stream spoof", "Hide on rec and demostr", category::settings)
    {
        add_component(new KeybindComponent(
            "Toggle Key",
            &key_bind,
            [this] { this->toggle(); }
        ));
    }

    ~stream_spoof() noexcept override = default;

    void on_tick() override { }

    void on_render_tick() override { }

    void on_enable() override {
        auto win_hwnd = main_window::get_main_hwnd();
        auto overlay_hwnd = overlay_window::get_main_hwnd();
        BOOL ok1 = SetWindowDisplayAffinity(win_hwnd, WDA_EXCLUDEFROMCAPTURE);
        BOOL ok2 = SetWindowDisplayAffinity(overlay_hwnd, WDA_EXCLUDEFROMCAPTURE);
        //printf("win=%d err=%lu, overlay=%d err=%lu\n", ok1, GetLastError(), ok2, GetLastError());
    }

    void on_disable() override {
        auto win_hwnd = main_window::get_main_hwnd();
        auto overlay_hwnd = overlay_window::get_main_hwnd();

        if (win_hwnd)     SetWindowDisplayAffinity(win_hwnd, WDA_NONE);
        if (overlay_hwnd) SetWindowDisplayAffinity(overlay_hwnd, WDA_NONE);
    }
};