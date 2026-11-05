//
// Created by natty on 25.09.2026.
//
#pragma once
#include "renderer/windows/main_window.h"
#include "renderer/windows/overlay_window.h"


class vex {

private:
    main_window main_window;
    overlay_window  overlay_window;
public:

    static vex& get();

    vex& update();

    vex& destroy();

};

