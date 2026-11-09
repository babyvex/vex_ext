#pragma once
#include "../../abstract_module.h"
#include "../../../renderer/components/components.h"
#include "../../../sdk/minecraft.hpp"


class auto_sprint : public abstract_module {

private:

    int key_bind = -1;
public:

    auto_sprint()
        : abstract_module("Autosprint", "Automatically keeps sprinting", category::movement) {

        add_component(new KeybindComponent(
          "Toggle Key",
          &key_bind,
          [this] { this->toggle(); }
      ));
    }

    ~auto_sprint() noexcept override = default;

    void on_tick() override {
        if (!is_enabled()) return;
        Minecraft::getInstance().player().setSprint(true);
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
