//
// Created by natty on 25.09.2026.
//

#include "abstract_module.h"

const char * abstract_module::get_name() const {
    return this->module_name;
}

const char * abstract_module::get_description() const {
    return this->module_description;
}

category abstract_module::get_category() const {
    return this->module_category;
}

bool abstract_module::is_enabled() const {
    return this->module_state;
}

void abstract_module::set_enabled(const bool state) {
    this->module_state = state;
}

void abstract_module::render_tile_components() {
    {
        for (auto* c : components)
            if (c && c->wants_tile_render())
                c->render_in_tile_module();
    }
}

void abstract_module::render_panel_components() {
    {
        for (auto* c : components)
            if (c && c->wants_panel_render())
                c->render_in_settings_panel();
    }
}

void abstract_module::add_component(abstract_component * component) {
    components.push_back(component);
}
