//
// Created by natty on 25.09.2026.
//

#include "vex.h"

#include <thread>

#include "features/module_manager.h"
#include "features/impl/combat/aim_assist.hpp"
#include "features/impl/combat/hitboxes.hpp"
#include "features/impl/misc/fast_place.hpp"
#include "features/impl/movement/auto_sprint.hpp"
#include "features/impl/movement/no_jump_delay.hpp"
#include "features/impl/settings/borderless.hpp"
#include "features/impl/settings/stream_spoof.hpp"
#include "features/impl/settings/theme_manager.hpp"
#include "features/impl/visual/esp.hpp"
#include "mapps/client_parser.h"
#include "mapps/mappings_manager.h"
#include "mJNI/mJNI.h"
#include "sdk/minecraft.hpp"

vex & vex::get() {
    static vex m;
    return m;
}
vex & vex::update() {

     auto pid = mJNI::Utils::getPidFromProcessName("javaw.exe");
     if (!pid) {
         return *this;
     }

     if (auto res = mJNI::mJNIAttachToJVM(pid); !res) {
         return *this;
     }

    mappings::init();
    Cacher::updateCache();

    global_module_manager.get()->add_module(std::make_unique<borderless_module>().release());
    global_module_manager.get()->add_module(std::make_unique<theme_manager>().release());
    global_module_manager.get()->add_module(std::make_unique<stream_spoof>().release());

    global_module_manager.get()->add_module(std::make_unique<hitbox>().release());
    global_module_manager.get()->add_module(std::make_unique<aim_assist>().release());

    global_module_manager.get()->add_module(std::make_unique<esp>().release());

    global_module_manager.get()->add_module(std::make_unique<auto_sprint>().release());
    global_module_manager.get()->add_module(std::make_unique<no_jump_delay>().release());

    global_module_manager.get()->add_module(std::make_unique<fast_place>().release());

    std::jthread render_thread([this] {
        if (!main_window.create_window(false)) {
            //printf("failed to create main window\n");
            return;
        }

        if (!overlay_window.create_window(true)) {
           // printf("failed to create overlay window \n" );
            return;
        }

        while (!main_window.should_close() && !overlay_window.should_close()) {
        //global_module_manager.get()->tick_render_modules();

        main_window.tick();
        overlay_window.tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    });

    while (!main_window.should_close() && !overlay_window.should_close()) {
        global_module_manager.get()->tick_all_modules();
    }

    return *this;
}


vex& vex::destroy() {

    mJNI::mJNIDetach();

    main_window.destroy_window();

    return *this;
}
