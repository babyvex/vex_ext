//
// Created by natty on 12.10.2026.
//

#ifndef MULLO_D2D1_RENDERER_H
#define MULLO_D2D1_RENDERER_H

#include <d2d1.h>
#include <dwrite.h>
#include <d2d1_1.h>

// thanks https://github.com/CovERUshKA

using d_color = D2D1_COLOR_F;
using d_point_2f = D2D1_POINT_2F;


struct vector2d {
    float x {} ,y {} ;
};

struct text_information {
    DWORD x_align {} , y_align {};

    d_color color {} ;

    bool multiline{}, clip {true};
};

class d2d1_drawer {

    public:

    ID2D1Factory* factory {};
    IDWriteFactory* write_factory {};

    ID2D1PathGeometry* geometry {};
    ID2D1SolidColorBrush* solid_color_brush {};

    ID2D1GeometrySink* geometry_sink {};
    ID2D1RenderTarget* render_target {};
    ID2D1DeviceContext* device_context {};


    d2d1_drawer() = default;

    ~d2d1_drawer() {
        destroy();
    }

    void line(const vector2d& cords1, const vector2d& cords2, float width, d_color color, bool antialias = false );
    void rectangle(const vector2d& cords1, const vector2d& rect_parms, d_color color );
    void rectangle_bordered(const vector2d& cords1, const vector2d& rect_parms, float width, d_color color, bool antialias );

    void circle(const vector2d& c, float radius, d_color color );
    void triangle(d_point_2f a, d_point_2f b, d_point_2f c, d_color color );

    void str_text(const wchar_t* text, const vector2d& xy, const text_information& information , const wchar_t* font = L"Segoe UI", float font_size = 14.0f, const vector2d& wh = {0,0 });
    void str_text(const char* text, const vector2d& xy, const text_information& information , const char* font = "Segoe UI", float font_size = 14.0f, const vector2d& wh = {0,0 });

    void clip_begin(d_point_2f rect);
    void clip_end();

    HRESULT  init();
    void destroy();

    private:
    ID2D1SolidColorBrush* make_brush_brush(d_color color);
};



#endif //MULLO_D2D1_RENDERER_H
