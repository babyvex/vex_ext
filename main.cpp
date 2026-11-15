#include <iostream>
#include <memory>

#include "src/base/vex.h"
#include "src/base/renderer/windows/main_window.h"
#include "src/base/features/abstract_module.h"
#include "src/base/features/module_manager.h"
#include "src/base/mapps/client_parser.h"

#include "src/base/renderer/components/components.h"

int main()
{
    vex::get().update();

    return 0;
}