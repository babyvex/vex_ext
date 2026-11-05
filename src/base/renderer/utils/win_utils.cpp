//
// Created by natty on 26.09.2026.
//

#include "win_utils.h"

#include <tlhelp32.h>
#include <algorithm>

#include "mJNI/memory/Memory.h"



DWORD win_utils::get_pid_by_hwnd(const HWND hwnd) {
    DWORD pid = 0;
    if (hwnd)
        GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

std::string win_utils::to_lower(std::string s) {
    std::ranges::transform(s, s.begin(),
                           [](const char c) { return static_cast<char>(towlower(c)); });
    return s;
}

std::wstring win_utils::to_wide(const std::string &s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring out(n, 0);
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), out.data(), n);
    return out;
}

bool win_utils::enable_privilege(LPCSTR name) {

    HANDLE token {};
    LUID luid{};
    TOKEN_PRIVILEGES token_privileges{};

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        return false;
    }

    if (!LookupPrivilegeValueA(nullptr, name, &luid)) {
        CloseHandle(token);
        return false;
    }
    token_privileges.PrivilegeCount = 1;
    token_privileges.Privileges[0].Luid = luid;
    token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL ok = AdjustTokenPrivileges(token, FALSE, &token_privileges, sizeof(token_privileges), nullptr, nullptr);

    DWORD err = GetLastError();

    CloseHandle(token);

    return (ok && err == ERROR_SUCCESS);
}

bool win_utils::enable_ui_access() {

    enable_privilege(SE_DEBUG_NAME);
    enable_privilege(SE_TCB_NAME);

    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &token)) {
        DWORD ui_access = 1;
        if (SetTokenInformation(token, TokenUIAccess, &ui_access, sizeof(ui_access))) {
            CloseHandle(token);
            return true;
        }
        CloseHandle(token);
    }
    return false;
}

HWND get_attached_minecraft_window()
{
    DWORD target_pid = global_process_data.jvm_pid;
    if (!target_pid) return nullptr;

    HWND fg = GetForegroundWindow();
    if (fg && IsWindow(fg))
    {
        DWORD fg_pid = 0;
        GetWindowThreadProcessId(fg, &fg_pid);
        if (fg_pid == target_pid) return fg;
    }

    if (s_mc_target && IsWindow(s_mc_target) && IsWindowVisible(s_mc_target) && !IsIconic(s_mc_target))
    {
        DWORD pid = 0;
        GetWindowThreadProcessId(s_mc_target, &pid);
        if (pid == target_pid) return s_mc_target;
    }

    struct search_ctx { DWORD pid; HWND best; int area; } ctx{ target_pid, nullptr, 0 };

    EnumWindows([](HWND hwnd, LPARAM lp) -> BOOL
    {
        auto* c = reinterpret_cast<search_ctx*>(lp);
        if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) return TRUE;

        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid != c->pid) return TRUE;
        if (GetWindow(hwnd, GW_OWNER)) return TRUE;

        RECT rc{};
        if (GetClientRect(hwnd, &rc))
        {
            int a = (rc.right - rc.left) * (rc.bottom - rc.top);
            if (a > c->area) { c->area = a; c->best = hwnd; }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&ctx));

    if (ctx.best)
    {
        s_mc_target = ctx.best;
        return ctx.best;
    }

    return nullptr;
}