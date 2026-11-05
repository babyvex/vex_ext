//
// Created by natty on 12.10.2026.
//

#include "d2d1_renderer.h"

static void safe_release(IUnknown* p)
{
    if (p) p->Release();
}


void d2d1_drawer::line(const vector2d &cords1, const vector2d &cords2, float width, d_color color, bool antialias ) {

    if (!render_target) return;

    render_target->SetAntialiasMode(antialias
        ? D2D1_ANTIALIAS_MODE_ALIASED
        : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    ID2D1SolidColorBrush* br = make_brush_brush(color);
    if (!br) return;

    render_target->DrawLine(
        D2D1::Point2F(cords1.x, cords1.y),
        D2D1::Point2F(cords2.x, cords2.y),
        br, width);
}

HRESULT d2d1_drawer::init() {

    auto hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        __uuidof(ID2D1Factory1), nullptr,
        reinterpret_cast<void**>(&factory));
    if (FAILED(hr)) {return hr; }

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
         __uuidof(IDWriteFactory),
         reinterpret_cast<IUnknown**>(&write_factory));

    if (FAILED(hr)) return hr;

    return S_OK;

}

void d2d1_drawer::destroy() {

    safe_release(solid_color_brush); solid_color_brush = nullptr;
    safe_release(geometry);          geometry          = nullptr;
    safe_release(geometry_sink);     geometry_sink     = nullptr;
    safe_release(write_factory);     write_factory     = nullptr;
    safe_release(factory);           factory           = nullptr;
    render_target = nullptr;

}

ID2D1SolidColorBrush * d2d1_drawer::make_brush_brush(d_color color) {
    if (!render_target) return nullptr;

    if (solid_color_brush) {
        solid_color_brush->SetColor(color);
        return solid_color_brush;
    }

    ID2D1SolidColorBrush* b = nullptr;
    if (FAILED(render_target->CreateSolidColorBrush(color, &b)))
        return nullptr;

    return b;
}
