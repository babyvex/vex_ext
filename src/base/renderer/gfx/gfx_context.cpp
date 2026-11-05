#include "gfx_context.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include <cstdio>

bool gfx::create_3d_device(gfx_context& ctx, bool transparent) {
    if (!ctx.hwnd) return false;
    return create_hwnd_swapchain(ctx, transparent);
}

bool gfx::create_hwnd_swapchain(gfx_context& ctx, bool transparent) {

    if (!ctx.hwnd) return false;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = 0;
    sd.BufferDesc.Height                  = 0;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = ctx.hwnd;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        levels, 2, D3D11_SDK_VERSION,
        &sd, &ctx.swapChain, &ctx.device, nullptr, &ctx.context);

    if (FAILED(hr)) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            levels, 2, D3D11_SDK_VERSION,
            &sd, &ctx.swapChain, &ctx.device, nullptr, &ctx.context);
    }

    if (FAILED(hr) || !ctx.swapChain || !ctx.device || !ctx.context) {
        return false;
    }

    ctx.transparent = transparent;
    ID3D11Texture2D* back = nullptr;
    hr = ctx.swapChain->GetBuffer(0, IID_PPV_ARGS(&back));
    if (FAILED(hr) || !back) {
        return false;
    }

    hr = ctx.device->CreateRenderTargetView(back, nullptr, &ctx.rtv);
    back->Release();
    if (FAILED(hr) || !ctx.rtv) {
        return false;
    }


    return true;
}

void gfx::cleanup_3d_device(gfx_context& ctx) {
    if (ctx.context) {
        ID3D11RenderTargetView* nulls[1] = { nullptr };
        ctx.context->OMSetRenderTargets(1, nulls, nullptr);
    }
    if (ctx.rtv)       { ctx.rtv->Release();       ctx.rtv       = nullptr; }
    if (ctx.swapChain) { ctx.swapChain->Release(); ctx.swapChain = nullptr; }
    if (ctx.context)   { ctx.context->Release();   ctx.context   = nullptr; }
    if (ctx.device)    { ctx.device->Release();    ctx.device    = nullptr; }
}

void gfx::init_imgui(gfx_context& ctx, void (*load_fonts)(ImGuiIO&, void*), void* user) {
    IMGUI_CHECKVERSION();
    ctx.imgui = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx.imgui);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    if (load_fonts) load_fonts(io, user);

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(ctx.hwnd);
    ImGui_ImplDX11_Init(ctx.device, ctx.context);
}

void gfx::destroy_imgui(gfx_context& ctx) {
    if (!ctx.imgui) return;
    ImGui::SetCurrentContext(ctx.imgui);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext(ctx.imgui);
    ctx.imgui = nullptr;
}

void gfx::begin_frame(gfx_context& ctx) {
    ImGui::SetCurrentContext(ctx.imgui);
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void gfx::end_frame(gfx_context& ctx) {
    ImGui::Render();

    if (!ctx.rtv) return;

    const float clear_opaque[4] = { 20.0f/255.0f, 19.0f/255.0f, 24.0f/255.0f, 1.0f };
    const float clear_transp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float* clear = ctx.transparent ? clear_transp : clear_opaque;

    ctx.context->OMSetRenderTargets(1, &ctx.rtv, nullptr);
    ctx.context->ClearRenderTargetView(ctx.rtv, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ctx.swapChain->Present(0, 0);
}

void gfx::resize(gfx_context& ctx, UINT w, UINT h) {
    if (w == 0 || h == 0) return;
    if (!ctx.swapChain || !ctx.device) return;

    if (ctx.context) {
        ID3D11RenderTargetView* nulls[1] = { nullptr };
        ctx.context->OMSetRenderTargets(1, nulls, nullptr);
    }
    if (ctx.rtv) { ctx.rtv->Release(); ctx.rtv = nullptr; }

    ctx.swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* back = nullptr;
    if (SUCCEEDED(ctx.swapChain->GetBuffer(0, IID_PPV_ARGS(&back))) && back) {
        ctx.device->CreateRenderTargetView(back, nullptr, &ctx.rtv);
        back->Release();
    }
    ctx.width  = w;
    ctx.height = h;
}

std::optional<LRESULT> gfx::message_handler(gfx_context& ctx, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ctx.imgui)
        ImGui::SetCurrentContext(ctx.imgui);

    if (ctx.imgui && ImGui::GetCurrentContext() &&
        ImGui_ImplWin32_WndProcHandler(ctx.hwnd, msg, wParam, lParam))
        return static_cast<LRESULT>(1);

    switch (msg) {
        case WM_SIZE:
            if (ctx.device && wParam != SIZE_MINIMIZED)
                resize(ctx, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
            return 0;
        case WM_ERASEBKGND:
            return 1;
        default:
            return std::nullopt;
    }
}