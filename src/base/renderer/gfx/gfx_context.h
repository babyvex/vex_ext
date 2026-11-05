#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <optional>
#include <Imgui/imgui.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
struct gfx_context {
    HWND                        hwnd        = nullptr;
    ID3D11Device*               device      = nullptr;
    ID3D11DeviceContext*        context     = nullptr;
    IDXGISwapChain*             swapChain   = nullptr;
    ID3D11RenderTargetView*     rtv         = nullptr;

    ImGuiContext*               imgui       = nullptr;
    UINT                        width       = 0;
    UINT                        height      = 0;
    bool                        transparent = false;
};

namespace gfx {
    bool create_3d_device(gfx_context& ctx, bool transparent);
    bool create_hwnd_swapchain(gfx_context& ctx, bool transparent);

    void cleanup_3d_device(gfx_context& ctx);

    void init_imgui(gfx_context& ctx, void (*load_fonts)(ImGuiIO&, void*), void* user);
    void destroy_imgui(gfx_context& ctx);

    void begin_frame(gfx_context& ctx);
    void end_frame(gfx_context& ctx);
    void resize(gfx_context& ctx, UINT w, UINT h);

    std::optional<LRESULT> message_handler(gfx_context& ctx, UINT msg, WPARAM wParam, LPARAM lParam);
}