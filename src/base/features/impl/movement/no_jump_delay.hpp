#pragma once
#include "../../abstract_module.h"
#include "../../../renderer/components/components.h"
#include "../../../sdk/minecraft.hpp"

class no_jump_delay : public abstract_module {

private:

    int key_bind = -1;
public:

    no_jump_delay()
        : abstract_module("No jump delay", "Remove jump delay", category::movement) {

        add_component(new KeybindComponent(
          "Toggle Key",
          &key_bind,
          [this] { this->toggle(); }
      ));
    }

    ~no_jump_delay() noexcept override = default;

    void on_tick() override {
        if (!is_enabled()) return;
        Minecraft::getInstance().player().setNoJumpDelay(false);
    }

    void on_render_tick() override {

    }

    void on_enable() override {
        set_enabled(true);
    }

    void on_disable() override {
        set_enabled(false);
    }
};
