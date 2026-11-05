//
// Created by natty on 27.09.2026.
//

#include "band_window.h"
#include "win_utils.h"

typedef HWND (__stdcall *p_create_window_in_band)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID, DWORD);
typedef HWND (__stdcall *p_create_window_in_band_ex)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID, DWORD, DWORD);

typedef BOOL (__stdcall *p_set_window_band)(HWND, HWND, DWORD);
typedef BOOL (__stdcall *p_get_window_band)(HWND, PDWORD);


static p_create_window_in_band m_create_window_in_band = nullptr;
static p_create_window_in_band_ex m_create_window_in_band_ex = nullptr;
static p_set_window_band set_window_band = nullptr;
static p_get_window_band get_window_band = nullptr;


bool band_window::init()
{
    auto user32 = GetModuleHandleA("user32.dll");
    if (!user32) return false;

    m_create_window_in_band = reinterpret_cast<p_create_window_in_band>(GetProcAddress(user32, "CreateWindowInBand"));
    m_create_window_in_band_ex = reinterpret_cast<p_create_window_in_band_ex>(GetProcAddress(user32, "CreateWindowInBandEx"));
    set_window_band = reinterpret_cast<p_set_window_band>(GetProcAddress(user32, "SetWindowBand"));
    get_window_band = reinterpret_cast<p_get_window_band>(GetProcAddress(user32, "GetWindowBand"));

    printf("create_in_band %p \n",    m_create_window_in_band );
    printf("create_in_band_ex %p \n", m_create_window_in_band_ex );

    if (!win_utils::enable_ui_access()) {
        return false;
    }

    return true;
}

HWND band_window::create_window_in_band_ex(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x,
    int y, int w, int h, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam, ZBID band) {

    if (!m_create_window_in_band_ex) return nullptr;

    HWND hwnd = m_create_window_in_band_ex(
            dwExStyle, lpClassName, lpWindowName, dwStyle,
            x, y, w, h,
            hWndParent, hMenu, hInstance, lpParam,
            static_cast<DWORD>(band), 0);

    return hwnd;
}

HWND band_window::create_window_in_band(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int w, int h,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam, ZBID band) {

    if (!m_create_window_in_band) return nullptr;

    HWND hwnd = m_create_window_in_band(
           0,
           lpClassName, lpWindowName, dwStyle,
           x, y, w, h,
           hWndParent, hMenu, hInstance, lpParam,
           static_cast<DWORD>(band)
       );

    return hwnd;
}

