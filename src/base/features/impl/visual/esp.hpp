#pragma once

#include "../../abstract_module.h"
#include "../../../renderer/components/components.h"
#include "../../../renderer/windows/overlay_window.h"

#include <ImGui/imgui.h>
#include <cmath>
#include <vector>
#include <utility>

class esp : public abstract_module
{
public:
    struct render_player { double x, y, z; };

private:
    struct camera_state {
        double cx = 0, cy = 0, cz = 0;
        float  yaw = 0, pitch = 0;
        double fov = 70;
        int    sw = 0, sh = 0;

        float  look_x = 0, look_y = 0, look_z = 0;
        float  right_x = 0, right_y = 0, right_z = 0;
        float  up_x = 0, up_y = 0, up_z = 0;
        float  fov_scale_x = 1, fov_scale_y = 1;
        bool   valid = false;
    };

    camera_state m_cam;

    static constexpr double k_pi = 3.14159265358979323846;

    void update_camera(double cx, double cy, double cz,
                       float yaw, float pitch, double fov,
                       int sw, int sh)
    {
        if (sw <= 0 || sh <= 0 || fov <= 0.0) { m_cam.valid = false; return; }

        m_cam.cx = cx; m_cam.cy = cy; m_cam.cz = cz;
        m_cam.yaw = yaw; m_cam.pitch = pitch;
        m_cam.fov = fov; m_cam.sw = sw; m_cam.sh = sh;

        const float yaw_rad   = (float)(yaw   * (k_pi / 180.0));
        const float pitch_rad = (float)(pitch * (k_pi / 180.0));

        const float cos_y = cosf(yaw_rad);
        const float sin_y = sinf(yaw_rad);
        const float cos_p = cosf(pitch_rad);
        const float sin_p = sinf(pitch_rad);

        m_cam.look_x  = -sin_y * cos_p;
        m_cam.look_y  = -sin_p;
        m_cam.look_z  =  cos_y * cos_p;

        m_cam.right_x = -cos_y;
        m_cam.right_y =  0.0f;
        m_cam.right_z = -sin_y;

        m_cam.up_x = -sin_y * sin_p;
        m_cam.up_y =  cos_p;
        m_cam.up_z =  cos_y * sin_p;

        const float fov_rad  = (float)(fov * (k_pi / 180.0));
        const float tan_half = tanf(fov_rad * 0.5f);

        m_cam.fov_scale_y = 1.0f / (tan_half <= 0.0001f ? 1.0f : tan_half);
        m_cam.fov_scale_x = m_cam.fov_scale_y / ((float)sw / (float)sh);

        m_cam.valid = true;
    }

    bool w2s(double wx, double wy, double wz,
             float& out_x, float& out_y, float& out_z) const
    {
        if (!m_cam.valid) return false;

        const float dx = (float)(wx - m_cam.cx);
        const float dy = (float)(wy - m_cam.cy);
        const float dz = (float)(wz - m_cam.cz);

        const float view_x = dx * m_cam.right_x + dy * m_cam.right_y + dz * m_cam.right_z;
        const float view_y = dx * m_cam.up_x    + dy * m_cam.up_y    + dz * m_cam.up_z;
        const float view_z = dx * m_cam.look_x  + dy * m_cam.look_y  + dz * m_cam.look_z;

        out_z = view_z;
        if (view_z <= 0.05f) return false;

        const float ndc_x = (view_x / view_z) * m_cam.fov_scale_x;
        const float ndc_y = (view_y / view_z) * m_cam.fov_scale_y;

        out_x = (1.0f + ndc_x) * 0.5f * (float)m_cam.sw;
        out_y = (1.0f - ndc_y) * 0.5f * (float)m_cam.sh;
        return true;
    }


    void render_2d(ImDrawList* dl, const render_player& p)
    {
        float bx, by, bz, tx, ty, tz;

        const bool bottom_visible = w2s(p.x, p.y,       p.z, bx, by, bz);
        const bool top_visible    = w2s(p.x, p.y + 1.8, p.z, tx, ty, tz);

        if (!bottom_visible && !top_visible) return;

        float height = fabsf(by - ty);
        if (height < 10.0f) height = 10.0f;
        const float width    = height * 0.4f;
        const float center_x = (bx + tx) * 0.5f;

        const ImU32 color = get_color();
        const ImU32 fill_color = IM_COL32(
            (color >> 0)  & 0xFF,
            (color >> 8)  & 0xFF,
            (color >> 16) & 0xFF,
            40);

        dl->AddRectFilled(
            ImVec2(center_x - width * 0.5f, ty),
            ImVec2(center_x + width * 0.5f, by),
            fill_color);

        dl->AddRect(
            ImVec2(center_x - width * 0.5f - 1.0f, ty - 1.0f),
            ImVec2(center_x + width * 0.5f + 1.0f, by + 1.0f),
            IM_COL32(0, 0, 0, 200), 0.0f, 0, 1.0f);

        dl->AddRect(
            ImVec2(center_x - width * 0.5f, ty),
            ImVec2(center_x + width * 0.5f, by),
            color, 0.0f, 0, 1.5f);
    }

    void render_3d(ImDrawList* dl, const render_player& p)
    {
        constexpr float player_height = 1.8f;
        constexpr float player_width  = 0.6f;
        constexpr float half_width    = player_width / 2.0f;

        struct vertex { double wx, wy, wz; float sx, sy, sz; bool visible; };

        vertex verts[8] = {
            { p.x - half_width, p.y,                  p.z - half_width, 0, 0, 0, false },
            { p.x + half_width, p.y,                  p.z - half_width, 0, 0, 0, false },
            { p.x + half_width, p.y,                  p.z + half_width, 0, 0, 0, false },
            { p.x - half_width, p.y,                  p.z + half_width, 0, 0, 0, false },
            { p.x - half_width, p.y + player_height,  p.z - half_width, 0, 0, 0, false },
            { p.x + half_width, p.y + player_height,  p.z - half_width, 0, 0, 0, false },
            { p.x + half_width, p.y + player_height,  p.z + half_width, 0, 0, 0, false },
            { p.x - half_width, p.y + player_height,  p.z + half_width, 0, 0, 0, false },
        };

        for (auto& v : verts)
            v.visible = w2s(v.wx, v.wy, v.wz, v.sx, v.sy, v.sz);

        const ImU32 color = get_color();
        const ImU32 fill_color = IM_COL32(
            (color >> 0) & 0xFF, (color >> 8) & 0xFF,
            (color >> 16) & 0xFF, 50);

        struct edge { int a, b; };
        edge edges[12] = {
            {0,1},{1,2},{2,3},{3,0},
            {4,5},{5,6},{6,7},{7,4},
            {0,4},{1,5},{2,6},{3,7},
        };

        struct face { int a, b, c, d; };
        face faces[6] = {
            {0,1,5,4},{2,3,7,6},{3,0,4,7},
            {1,2,6,5},{4,5,6,7},{0,3,2,1},
        };

        for (auto& f : faces)
            if (verts[f.a].visible && verts[f.b].visible &&
                verts[f.c].visible && verts[f.d].visible)
                dl->AddQuadFilled(
                    ImVec2(verts[f.a].sx, verts[f.a].sy),
                    ImVec2(verts[f.b].sx, verts[f.b].sy),
                    ImVec2(verts[f.c].sx, verts[f.c].sy),
                    ImVec2(verts[f.d].sx, verts[f.d].sy),
                    fill_color);

        for (auto& e : edges)
            if (verts[e.a].visible && verts[e.b].visible)
                dl->AddLine(
                    ImVec2(verts[e.a].sx, verts[e.a].sy),
                    ImVec2(verts[e.b].sx, verts[e.b].sy),
                    color, 2.0f);
    }

    void render_skeleton(ImDrawList* dl, const render_player& p)
    {
        constexpr float h = 1.8f;
        constexpr float w = 0.4f;

        struct bone_point { double x, y, z; };
        bone_point head   = { p.x, p.y + h,          p.z };
        bone_point neck   = { p.x, p.y + h * 0.85f,  p.z };
        bone_point l_sh   = { p.x - w,        p.y + h * 0.8f,  p.z };
        bone_point r_sh   = { p.x + w,        p.y + h * 0.8f,  p.z };
        bone_point l_hand = { p.x - w * 1.2f, p.y + h * 0.4f,  p.z };
        bone_point r_hand = { p.x + w * 1.2f, p.y + h * 0.4f,  p.z };
        bone_point waist  = { p.x, p.y + h * 0.5f,   p.z };
        bone_point l_knee = { p.x - w * 0.5f, p.y + h * 0.25f, p.z };
        bone_point r_knee = { p.x + w * 0.5f, p.y + h * 0.25f, p.z };
        bone_point l_foot = { p.x - w * 0.5f, p.y,             p.z };
        bone_point r_foot = { p.x + w * 0.5f, p.y,             p.z };

        const ImU32 color       = get_color();
        const ImU32 joint_color = IM_COL32(255, 255, 255, 255);

        auto proj = [&](bone_point& b, float& sx, float& sy) -> bool {
            float sz; return w2s(b.x, b.y, b.z, sx, sy, sz);
        };

        float hx, hy, nx, ny, lsx, lsy, rsx, rsy, lhx, lhy, rhx, rhy;
        float wx, wy, lkx, lky, rkx, rky, lfx, lfy, rfx, rfy;

        const bool hv  = proj(head,   hx, hy);
        const bool nv  = proj(neck,   nx, ny);
        const bool lsv = proj(l_sh,   lsx, lsy);
        const bool rsv = proj(r_sh,   rsx, rsy);
        const bool lhv = proj(l_hand, lhx, lhy);
        const bool rhv = proj(r_hand, rhx, rhy);
        const bool wv  = proj(waist,  wx, wy);
        const bool lkv = proj(l_knee, lkx, lky);
        const bool rkv = proj(r_knee, rkx, rky);
        const bool lfv = proj(l_foot, lfx, lfy);
        const bool rfv = proj(r_foot, rfx, rfy);

        auto bone = [&](float x1, float y1, float x2, float y2, bool v1, bool v2) {
            if (v1 && v2) dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, 2.0f);
        };

        if (hv) {
            dl->AddCircleFilled(ImVec2(hx, hy), 8.0f, color);
            dl->AddCircle(ImVec2(hx, hy), 8.0f, joint_color, 64, 2.0f);
        }

        bone(nx, ny, hx, hy, nv, hv);
        bone(nx, ny, wx, wy, nv, wv);
        bone(lsx, lsy, rsx, rsy, lsv, rsv);
        bone(lsx, lsy, lhx, lhy, lsv, lhv);
        bone(rsx, rsy, rhx, rhy, rsv, rhv);
        bone(wx, wy, lkx, lky, wv, lkv);
        bone(lkx, lky, lfx, lfy, lkv, lfv);
        bone(wx, wy, rkx, rky, wv, rkv);
        bone(rkx, rky, rfx, rfy, rkv, rfv);

        if (nv)  dl->AddCircleFilled(ImVec2(nx, ny),   3.0f, joint_color);
        if (lsv) dl->AddCircleFilled(ImVec2(lsx, lsy), 3.0f, joint_color);
        if (rsv) dl->AddCircleFilled(ImVec2(rsx, rsy), 3.0f, joint_color);
        if (lhv) dl->AddCircleFilled(ImVec2(lhx, lhy), 3.0f, joint_color);
        if (rhv) dl->AddCircleFilled(ImVec2(rhx, rhy), 3.0f, joint_color);
        if (wv)  dl->AddCircleFilled(ImVec2(wx, wy),   3.0f, joint_color);
        if (lkv) dl->AddCircleFilled(ImVec2(lkx, lky), 3.0f, joint_color);
        if (rkv) dl->AddCircleFilled(ImVec2(rkx, rky), 3.0f, joint_color);
        if (lfv) dl->AddCircleFilled(ImVec2(lfx, lfy), 3.0f, joint_color);
        if (rfv) dl->AddCircleFilled(ImVec2(rfx, rfy), 3.0f, joint_color);
    }

    ImU32 get_color() const
    {
        return IM_COL32(
            (int)(my_color_with_alpha[0] * 255),
            (int)(my_color_with_alpha[1] * 255),
            (int)(my_color_with_alpha[2] * 255),
            (int)(my_color_with_alpha[3] * 255));
    }

    static constexpr float max_dist_sq = 150.0f * 150.0f;
    int m_key_bind = -1;

    int m_current_mode = 0;   // 0 = 2D, 1 = 3D, 2 = Skeleton

    float my_color_with_alpha[4] = { 1.0f, 0.0f, 0.0f, 0.5f };

public:
    esp()
        : abstract_module("Esp", "Render nearly players", category::visual)
    {
        add_component(new KeybindComponent(
            "keybind", &m_key_bind, [this]() { this->toggle(); }));

        add_component(new ChooseButtonsComponent<int>(
            "Mode", &m_current_mode,
            std::vector<std::pair<std::string, int>>{
                {"2D",       0},
                {"3D",       1},
                {"Skeleton", 2},
            },
            68.0f));

        add_component(new ColorPickerComponent(
            "ESP Color", my_color_with_alpha, true));
    }

    ~esp() override = default;

    void on_enable()  override { set_enabled(true) ; }
    void on_disable() override { set_enabled(false) ; }
    void on_tick()    override { }void on_render_tick() override
{
    ImGuiContext* ctx = overlay_window::get_imgui_context();
    if (!ctx) return;
    if (ImGui::GetCurrentContext() != ctx) return;
    if (!is_enabled()) return;

    Minecraft mc = Minecraft::getInstance();
    if (!mc.getObject()) return;
    ClientLevel level = mc.level();
    if (!level.getObject()) return;
    LocalPlayer self = mc.player();
    if (!self.getObject()) return;
    GameRenderer game_renderer = mc.gameRenderer();
    if (!game_renderer.getObject()) return;
    Camera camera = game_renderer.mainCamera();
    if (!camera.getObject()) return;

    Vec3 cam_pos = camera.getPosition();
    const double cam_x = cam_pos.getX();
    const double cam_y = cam_pos.getY();
    const double cam_z = cam_pos.getZ();
    const float cam_yaw   = camera.yRotation();
    const float cam_pitch = camera.xRotation();

    Options options = mc.options();
    int fov_value = 70;
    if (options.getObject()) fov_value = options.fov().getInt();
    const double fov = (fov_value > 0 && fov_value < 180) ? (double)fov_value : 70.0;

    const ImGuiIO& io = ImGui::GetIO();
    const int scr_w = (int)io.DisplaySize.x;
    const int scr_h = (int)io.DisplaySize.y;
    if (scr_w <= 0 || scr_h <= 0) return;

    update_camera(cam_x, cam_y, cam_z, cam_yaw, cam_pitch, fov, scr_w, scr_h);

    ImDrawList* dl = ImGui::GetForegroundDrawList();

    for (auto& player : level.players())
    {
        jobject p_obj = player.getObject();
        if (!p_obj || p_obj == self.getObject()) continue;

        Entity::EntityRenderPos r_pos = player.getRenderPos();
        const double p_x = r_pos.x;
        const double p_y = r_pos.y;
        const double p_z = r_pos.z;

        const double dx = p_x - cam_x;
        const double dy = p_y - cam_y;
        const double dz = p_z - cam_z;
        if (dx * dx + dy * dy + dz * dz > max_dist_sq) continue;

        const render_player rp{ p_x, p_y, p_z };

        switch (m_current_mode)
        {
            case 0: render_2d(dl, rp);       break;
            case 1: render_3d(dl, rp);       break;
            case 2: render_skeleton(dl, rp); break;
            default: break;
        }
    }
}
};