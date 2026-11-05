#pragma once

#include "../../abstract_module.h"
#include "../../../renderer/components/components.h"

#include <windows.h>
#include <atomic>

extern HWND get_attached_minecraft_window();

class borderless_module : public abstract_module {
private:
    std::atomic<bool> m_toggle_requested{false};
    std::atomic<bool> m_disable_requested{false};

    bool m_borderless_active = false;
    bool m_key_held = false;

    int key_bind = VK_INSERT;

    static constexpr int kWindowedWidth  = 900;
    static constexpr int kWindowedHeight = 600;

public:
    borderless_module()
        : abstract_module("Borderless", "Borderless fullscreen toggle", category::settings)
    {
        add_component(new KeybindComponent(
            "Toggle Key",
            &key_bind,
            [this] {
                m_toggle_requested = true;
                this->toggle();
            }
        ));
    }

    ~borderless_module() noexcept override = default;

    void on_enable()  override { }
    void on_disable() override { }
    void on_tick()    override {  }

    void on_render_tick() override {
        HWND mc = get_attached_minecraft_window();
        if (!mc || !IsWindow(mc)) return;

        if (m_disable_requested.exchange(false)) {
            if (m_borderless_active) {
                apply(mc, false);
                m_borderless_active = false;
            }
            return;
        }

        if (m_toggle_requested.exchange(false)) {
            m_borderless_active = !m_borderless_active;
            apply(mc, m_borderless_active);
        }
    }

    bool is_enabled() const { return m_borderless_active; }

    void request_disable() {
        m_disable_requested = true;
        if (m_borderless_active) {
            HWND mc = get_attached_minecraft_window();
            if (mc && IsWindow(mc)) {
                apply(mc, false);
                m_borderless_active = false;
                printf("sync on close\n");
            }
        }
    }

private:
    void apply(HWND mc, bool borderless) {
        const int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        LONG_PTR style = GetWindowLongPtrA(mc, GWL_STYLE);

        if (borderless) {
            style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
            SetWindowLongPtrA(mc, GWL_STYLE, style);

            SetWindowPos(mc, HWND_NOTOPMOST, 0, 0, screenWidth, screenHeight,
                         SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            //printf("enabled\n");
        }
        else {
            style |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
            SetWindowLongPtrA(mc, GWL_STYLE, style);

            int spawnX = (screenWidth - kWindowedWidth) / 2;
            int spawnY = (screenHeight - kWindowedHeight) / 2;

            SetWindowPos(mc, HWND_NOTOPMOST, spawnX, spawnY,
                         kWindowedWidth, kWindowedHeight,
                         SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            //printf("disabled\n");
        }
    }
};