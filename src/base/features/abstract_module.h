//
// Created by natty on 25.09.2026.
//

#pragma once
#include <vector>

#include <string>

#include "category.h"
#include "../renderer/components/abstract_component.h"


class abstract_module {
private:

    const char* module_name;
    const char* module_description;
    category module_category;

    bool module_state = false;

    std::vector<abstract_component*> components;

public:
    virtual ~abstract_module() = default;

    abstract_module(const char* module_name, const char* module_description, category category) {
        this->module_name = module_name;
        this->module_description = module_description;
        this->module_category = category;
    }

    [[nodiscard]] const char* get_name() const;
    [[nodiscard]] const char* get_description() const;
    [[nodiscard]] category get_category() const;
    void toggle() { module_state = !module_state; }

    virtual void on_tick() {};
    virtual void on_render_tick() {} ;
    virtual void on_enable() {};
    virtual void on_disable() {} ;

    [[nodiscard]] bool is_enabled() const;
    void set_enabled(bool);

    void render_tile_components();
    void render_panel_components();

    void add_component(abstract_component*);

};


