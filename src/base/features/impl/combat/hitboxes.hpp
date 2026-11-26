#pragma once
#include "../../abstract_module.h"
#include "../../../renderer/components/components.h"
#include "../../../sdk/minecraft.hpp"


class hitbox : public abstract_module {

private:

    float expand = 0.3f;

public:
    hitbox()
        : abstract_module("Hitbox", "Expands target hitbox", category::combat) {
        add_component(new SliderComponent<float>("Size", &expand, expand, 6.0f, 0.1f));
    }

    ~hitbox() noexcept override = default;

    void on_tick() override {

        if (!is_enabled()) {return;}

        auto mc = Minecraft::getInstance();

        auto world = mc.level();

        for (auto& player : world.players()) {

            if (player == mc.player()) {
                continue;
            }

            auto position = player.position();
            auto bb = player.bb();

            if (!position.getObject() || !bb.getObject()) {
                continue;
            }

            double curX = position.getX();
            double curZ = position.getZ();

            bb.setMinX(curX - expand);
            bb.setMinZ(curZ - expand);

            bb.setMaxX(curX + expand);
            bb.setMaxZ(curZ + expand);
        }

    }

    void on_render_tick() override {};

    void on_enable() override {
        set_enabled(true) ;
    }

    void on_disable() override {
        set_enabled(false) ;
    }
};
