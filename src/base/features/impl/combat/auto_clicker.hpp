#pragma once
#include "../../abstract_module.h"


class auto_clicker : public abstract_module {


private:

public:
    auto_clicker()
        : abstract_module("Auto clicker", "Automates humanized CPS clicking", category::combat) {

    }

    ~auto_clicker() noexcept override = default;

    void on_tick() override;

    void on_render_tick() override;

    void on_enable() override;

    void on_disable() override;
};