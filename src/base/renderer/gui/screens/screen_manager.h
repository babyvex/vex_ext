//
// Created by natty on 29.09.2026.
//

#pragma once
#include <memory>
#include <unordered_map>
#include "abstract_screen.h"


class screen_manager {

private:

    std::unordered_map<int, std::unique_ptr<abstract_screen>> screens;

    abstract_screen* current   = nullptr;
    int current_screen_id = -1;

public:

    screen_manager() = default;

    screen_manager(const screen_manager&) = delete;
    screen_manager& operator=(const screen_manager&) = delete;

    void register_screen(int id, std::unique_ptr<abstract_screen> screen);

    void switch_to(int id);

    [[nodiscard]] abstract_screen* get_current_screen() const { return current; }
    [[nodiscard]] int get_current_screen_id() const { return current_screen_id; }

    void tick();

    void release_all();
};


