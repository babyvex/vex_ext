



#include "screen_manager.h"

void screen_manager::register_screen(int id, std::unique_ptr<abstract_screen> screen) {

    if (!screen) {
        return;
    }
    screens[id] = std::move(screen);
}

void screen_manager::switch_to(int id)
{
    auto it = screens.find(id);
    if (it == screens.end())
        return;

    if (it->second.get() == current)
        return;

    if (current)
        current->exit();

    current = it->second.get();
    current_screen_id = id;

    current->take_switch_request();
    current->enter();
}


void screen_manager::tick() {
    if (!current)
        return;

    current->draw();

    const int next = current->take_switch_request();
    if (next >= 0 && next != current_screen_id)
        switch_to(next);
}

void screen_manager::release_all() {
    for (auto& kv : screens)
        kv.second->release_gpu();
}