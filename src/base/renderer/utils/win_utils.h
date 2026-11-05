//
// Created by natty on 26.09.2026.
//

#pragma once

#include <string>
#include <windows.h>

namespace win_utils {

    DWORD get_pid_by_hwnd(HWND hwnd);

    std::string to_lower(std::string str);

    std::wstring to_wide(const std::string& s);

    bool enable_privilege(LPCSTR name);

    bool enable_ui_access();

}

HWND get_attached_minecraft_window();

static HWND s_mc_target = nullptr;


