#pragma once
#include "../../abstract_module.h"
#include "../../../renderer/components/components.h"
#include "../../../sdk/minecraft.hpp"

class fast_place : public abstract_module {

private:

    int key_bind = -1;
public:

    fast_place()
        : abstract_module("Fast place", "Remove block placement delay", category::misc) {

        add_component(new KeybindComponent(
          "Toggle Key",
          &key_bind,
          [this] { this->toggle(); }
      ));
    }

    ~fast_place() noexcept override = default;

    void on_tick() override {
        if (!is_enabled()) return;
        Minecraft::getInstance().setRightClickDelay(false);
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
