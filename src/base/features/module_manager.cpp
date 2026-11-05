//
// Created by natty on 25.09.2026.
//

#include "module_manager.h"

void module_manager::add_module(abstract_module *module) {
    global_module_list.push_back(module);
}

void module_manager::remove_module(abstract_module *module) {
    auto it = std::find(global_module_list.begin(), global_module_list.end(), module);
    if (it != global_module_list.end()) {
        global_module_list.erase(it);
    }
}

abstract_module * module_manager::get_module_by_name(const std::string name) {
    for (auto& module : global_module_list) {
        if (module->get_name() == name) {
            return module;
        }
    }
}

std::vector<abstract_module*> module_manager::get_module_by_category(category cat) const
{
    std::vector<abstract_module*> result;
    for (const auto& m : global_module_list)
        if (m->get_category() == cat)
            result.push_back(m);
    return result;
}

void module_manager::tick_render_modules() {
    for (auto& module : global_module_list) {
        if (module->get_category() == category::visual || module->get_category() == category::settings) {
            module->on_render_tick();
        }
    }
}

void module_manager::tick_all_modules() {
    for (auto& module : global_module_list) {
        if (module->get_category() != category::visual) {
            module->on_tick();
        }
    }
}
