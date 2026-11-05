//
// Created by natty on 25.09.2026.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "abstract_module.h"


class module_manager {
private:

    std::vector<abstract_module*> global_module_list;

public:

    void add_module(abstract_module *module);
    void remove_module(abstract_module *module);

    abstract_module* get_module_by_name(std::string name);

    [[nodiscard]] std::vector<abstract_module*> get_module_by_category(category cat) const;

    void tick_render_modules();
    void tick_all_modules();

};

inline auto global_module_manager = std::make_shared<module_manager>();

