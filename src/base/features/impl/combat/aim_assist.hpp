#pragma once

#include "../../abstract_module.h"
#include "../../../renderer/components/Components.h"
#include <chrono>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <random>
#include <ImGui/imgui.h>
#include "../../../sdk/minecraft.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint64_t aim_get_time_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

static float aim_wrap_degrees(float angle) {
    angle = std::fmod(angle, 360.0f);
    if (angle >= 180.0f) angle -= 360.0f;
    if (angle < -180.0f) angle += 360.0f;
    return angle;
}

static float aim_dist_vec(double x1, double y1, double z1, double x2, double y2, double z2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    double dz = z1 - z2;
    return static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
}

static std::mt19937& aim_rng() {
    static std::mt19937 g(std::random_device{}());
    return g;
}

static double aim_rnd(double lo, double hi) {
    std::uniform_real_distribution<double> d(lo, hi);
    return d(aim_rng());
}

static int aim_rnd_int(int lo, int hi) {
    std::uniform_int_distribution<int> d(lo, hi);
    return d(aim_rng());
}

static double aim_apply_curve(double t, int type, double power) {
    t = std::clamp(t, 0.0, 1.0);
    switch (type) {
    case 0:
        return 1.0 - std::pow(1.0 - t, power);
    case 1:
        if (t < 0.5)
            return std::pow(2.0 * t, power) / 2.0;
        else
            return 1.0 - std::pow(2.0 * (1.0 - t), power) / 2.0;
    default:
        return t * t * (3.0 - 2.0 * t);
    }
}

struct aim_smart_rot_result {
    float yaw;
    float pitch;
    double targetX;
    double targetY;
    double targetZ;
};

static aim_smart_rot_result aim_calc_rotations_smart(
    double curX, double curY, double curZ,
    double curEyeHeight, float curYaw, float curPitch,
    double targetX, double targetY, double targetZ,
    double targetWidth, double targetHeight,
    double aimHeightSetting,
    double offX, double offY, double offZ)
{
    double eyeY = curY + curEyeHeight;
    double halfWidth = targetWidth * 0.5;

    double h = std::clamp(aimHeightSetting + offY, 0.05, 0.95);

    double targetPointX = targetX + offX * halfWidth;
    double targetPointY = targetY + targetHeight * h;
    double targetPointZ = targetZ + offZ * halfWidth;

    double deltaX = targetPointX - curX;
    double deltaY = targetPointY - eyeY;
    double deltaZ = targetPointZ - curZ;
    double horizontalDistance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);

    float yaw;
    if (horizontalDistance < 0.001) {
        yaw = curYaw;
    } else {
        yaw = (float)(std::atan2(-deltaX, deltaZ) * 180.0 / M_PI);
    }

    double totalDistance = std::sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
    float pitch;
    if (totalDistance < 0.001) {
        pitch = curPitch;
    } else {
        pitch = (float)(-std::asin(std::clamp(deltaY / totalDistance, -1.0, 1.0)) * 180.0 / M_PI);
    }

    return { yaw, pitch, targetPointX, targetPointY, targetPointZ };
}

static double aim_get_fov_angle(
    double eyeX, double eyeY, double eyeZ,
    float yaw, float pitch,
    double targetPointX, double targetPointY, double targetPointZ)
{
    double cy = std::cos(yaw   * M_PI / 180.0);
    double sy = std::sin(yaw   * M_PI / 180.0);
    double cp = std::cos(pitch * M_PI / 180.0);
    double sp = std::sin(pitch * M_PI / 180.0);

    double lookX = -sy * cp;
    double lookY = -sp;
    double lookZ =  cy * cp;

    double dx = targetPointX - eyeX;
    double dy = targetPointY - eyeY;
    double dz = targetPointZ - eyeZ;
    double len = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (len < 0.001) return 0.0;

    dx /= len; dy /= len; dz /= len;

    double dot = lookX*dx + lookY*dy + lookZ*dz;
    dot = std::clamp(dot, -1.0, 1.0);

    return std::acos(dot) * 180.0 / M_PI;
}

class aim_assist : public abstract_module {

private:
    bool m_state = false;
    int m_key_bind = -1;

    int m_mode = 0;
    double m_speed_yaw = 5.0;
    double m_speed_pitch = 5.0;
    double m_smoothness = 5.0;
    double m_fov = 90.0;
    double m_range = 6.0;
    bool m_draw_fov = false;
    bool m_free_mode = false;
    float m_fov_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    jobject m_current_target_obj = nullptr;
    uint64_t m_last_time_ms = 0;
    float m_last_player_yaw = 0.0f;
    float m_last_player_pitch = 0.0f;

    uint64_t m_aim_capture_time = 0;

    int    m_aim_curve_type   = 0;
    double m_aim_curve_power  = 2.5;
    double m_aim_speed_mul    = 1.0;

    double m_aim_off_target_x  = 0.0;
    double m_aim_off_target_y  = 0.0;
    double m_aim_off_target_z  = 0.0;
    double m_aim_off_current_x = 0.0;
    double m_aim_off_current_y = 0.0;
    double m_aim_off_current_z = 0.0;
    uint64_t m_aim_off_refresh = 0;
    double m_aim_off_lerp      = 0.05;

    double m_aim_last_step_yaw   = 0.0;
    double m_aim_last_step_pitch = 0.0;

    double m_aim_height_noise   = 0.0;
    uint64_t m_aim_noise_refresh = 0;

    double m_aim_height = 0.85;

    void update_aim_engine();
    void reset_target_state();

public:
    aim_assist()
        : abstract_module("Aim assist", "Helps aim automatically", category::combat) {

        add_component(new KeybindComponent(
            "Keybind",
            &m_key_bind,
            [this]() {
                this->toggle();
            }));

        std::vector<std::pair<std::string, int>> mode_options = {
            {"Default", 0},
            {"Smart", 1}
        };
        add_component(new DropdownComponent<int>(
            "Mode",
            &m_mode,
            mode_options));

        add_component(new SliderComponent<double>(
            "Yaw",
            &m_speed_yaw,
            0.0,
            15.0,
            0.1));

        add_component(new SliderComponent<double>(
            "Pitch",
            &m_speed_pitch,
            0.0,
            15.0,
            0.1));

        add_component(new SliderComponent<double>(
            "Smoothness",
            &m_smoothness,
            1.0,
            15.0,
            0.1));

        add_component(new SliderComponent<double>(
            "Fov",
            &m_fov,
            0.0,
            180.0,
            1.0));

        add_component(new SliderComponent<double>(
            "Distance",
            &m_range,
            1.0,
            10.0,
            0.1));

        add_component(new OvalButton(
            "Fov circle",
            &m_draw_fov));

        add_component(new OvalButton(
            "Free mode",
            &m_free_mode));

        add_component(new ColorPickerComponent(
            "Fov circle color",
            m_fov_color,
            true));

        add_component(new SliderComponent<double>(
            "Aim height",
            &m_aim_height,
            0.0,
            1.0,
            0.05));
    }

    void on_tick() override { update_aim_engine(); }
    void on_render_tick() override;

    void on_enable() override {
        m_state = true;
        reset_target_state();
        m_last_time_ms = aim_get_time_ms();
    }

    void on_disable() override {
        m_state = false;
        reset_target_state();
    }

    ~aim_assist() noexcept override = default;
};

inline void aim_assist::reset_target_state() {
    m_current_target_obj  = nullptr;
    m_aim_capture_time    = 0;
    m_aim_last_step_yaw   = 0.0;
    m_aim_last_step_pitch = 0.0;
    m_aim_off_target_x    = 0.0;
    m_aim_off_target_y    = 0.0;
    m_aim_off_target_z    = 0.0;
    m_aim_off_current_x   = 0.0;
    m_aim_off_current_y   = 0.0;
    m_aim_off_current_z   = 0.0;
    m_aim_height_noise    = 0.0;
}

inline void aim_assist::on_render_tick() {
    if (!m_state) return;

    if (m_draw_fov) {
        Minecraft mc = Minecraft::getInstance();
        LocalPlayer main = mc.player();
        if (main.getObject()) {
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            if (drawList) {
                ImVec2 displaySize = ImGui::GetIO().DisplaySize;
                float fov = static_cast<float>(m_fov);
                float radius = (fov / 90.0f) * (std::min(displaySize.x, displaySize.y) / 2.0f);

                ImU32 col = IM_COL32(
                    static_cast<int>(m_fov_color[0] * 255.0f),
                    static_cast<int>(m_fov_color[1] * 255.0f),
                    static_cast<int>(m_fov_color[2] * 255.0f),
                    static_cast<int>(m_fov_color[3] * 255.0f)
                );

                drawList->AddCircle(
                    ImVec2(displaySize.x / 2.0f, displaySize.y / 2.0f),
                    radius,
                    col,
                    64,
                    2.0f
                );
            }
        }
    }

    update_aim_engine();
}

inline void aim_assist::update_aim_engine() {
    if (!m_state) return;

    Minecraft mc = Minecraft::getInstance();
    LocalPlayer main = mc.player();
    ClientLevel level = mc.level();

    if (!main.getObject() || !level.getObject()) return;

    Vec3 mainPos = main.position();
    double mainX = mainPos.getX();
    double mainY = mainPos.getY();
    double mainZ = mainPos.getZ();
    double mainEyeHeight = 1.62;
    double mainEyeY = mainY + mainEyeHeight;

    float mainYaw   = main.getYaw();
    float mainPitch = main.getPitch();

    uint64_t nowMs = aim_get_time_ms();

    if (nowMs - m_aim_noise_refresh > (uint64_t)aim_rnd(180, 450)) {
        m_aim_height_noise += aim_rnd(-0.04, 0.04);
        m_aim_height_noise = std::clamp(m_aim_height_noise, -0.12, 0.10);
        m_aim_noise_refresh = nowMs;
    }

    std::vector<Player> players = level.players();

    float bestDistance = FLT_MAX;
    jobject bestTargetObj = nullptr;
    Vec3 bestTargetPos(nullptr);
    AABB bestTargetBox(nullptr);
    double maxFovHalf = m_fov * 0.5;

    for (auto& player : players) {
        jobject pObj = player.getObject();
        if (!pObj || pObj == main.getObject()) continue;
        if (!player.isAlive()) continue;

        Vec3 pPos = player.position();
        float distance = aim_dist_vec(mainX, mainY, mainZ, pPos.getX(), pPos.getY(), pPos.getZ());
        if (distance > m_range) continue;
        if (distance < 0.5f) continue;

        AABB box = player.bb();
        if (!box.getObject()) continue;

        double targetH = std::clamp(m_aim_height, 0.05, 0.95);
        double tpX = pPos.getX();
        double boxH = box.maxY() - box.minY();
        double tpY = pPos.getY() + boxH * targetH;
        double tpZ = pPos.getZ();

        double fovScore = aim_get_fov_angle(mainX, mainEyeY, mainZ, mainYaw, mainPitch, tpX, tpY, tpZ);
        if (m_fov < 360.0 && fovScore > maxFovHalf) continue;

        if (distance < bestDistance) {
            bestDistance  = distance;
            bestTargetObj = pObj;
            bestTargetPos = pPos;
            bestTargetBox = box;
        }
    }

    if (!bestTargetObj || bestDistance > m_range) {
        reset_target_state();
        return;
    }

    if (bestTargetObj != m_current_target_obj) {
        m_current_target_obj = bestTargetObj;
        m_aim_capture_time   = nowMs;

        m_aim_curve_type  = aim_rnd_int(0, 2);
        m_aim_curve_power = aim_rnd(1.8, 3.5);
        m_aim_speed_mul   = aim_rnd(0.85, 1.15);

        m_aim_off_target_x = aim_rnd(-0.20, 0.20);
        m_aim_off_target_y = aim_rnd(-0.06, 0.06);
        m_aim_off_target_z = aim_rnd(-0.20, 0.20);
        m_aim_off_refresh  = nowMs + (uint64_t)aim_rnd(500, 1200);
        m_aim_off_lerp     = aim_rnd(0.015, 0.04);

        m_aim_last_step_yaw   = 0.0;
        m_aim_last_step_pitch = 0.0;

        m_aim_height_noise  = aim_rnd(-0.08, 0.06);
        m_aim_noise_refresh = nowMs;
    }

    if (nowMs >= m_aim_off_refresh) {
        m_aim_off_target_x = aim_rnd(-0.20, 0.20);
        m_aim_off_target_y = aim_rnd(-0.06, 0.06);
        m_aim_off_target_z = aim_rnd(-0.20, 0.20);
        m_aim_off_refresh  = nowMs + (uint64_t)aim_rnd(500, 1200);
        m_aim_off_lerp     = aim_rnd(0.015, 0.04);
    }
    double lf = m_aim_off_lerp;
    m_aim_off_current_x += (m_aim_off_target_x - m_aim_off_current_x) * lf;
    m_aim_off_current_y += (m_aim_off_target_y - m_aim_off_current_y) * lf;
    m_aim_off_current_z += (m_aim_off_target_z - m_aim_off_current_z) * lf;

    double boxW = bestTargetBox.maxX() - bestTargetBox.minX();
    double boxH = bestTargetBox.maxY() - bestTargetBox.minY();
    double aimH = m_aim_height + m_aim_height_noise;

    aim_smart_rot_result rot = aim_calc_rotations_smart(
        mainX, mainY, mainZ, mainEyeHeight, mainYaw, mainPitch,
        bestTargetPos.getX(), bestTargetPos.getY(), bestTargetPos.getZ(),
        boxW, boxH,
        aimH,
        0.0, 0.0, 0.0);

    double dYaw   = aim_wrap_degrees(rot.yaw - mainYaw);
    double dPitch = rot.pitch - mainPitch;

    double dAngle = std::sqrt(dYaw * dYaw + dPitch * dPitch);
    if (dAngle < 0.05) return;

    double tProgress = std::clamp((nowMs - m_aim_capture_time) / 500.0, 0.0, 1.0);

    double curve = aim_apply_curve(tProgress, m_aim_curve_type, m_aim_curve_power);
    curve = std::clamp(curve, 0.05, 1.8);

    double d9;
    if      (dAngle < 3.0)  d9 = 0.50;
    else if (dAngle < 10.0) d9 = 0.85;
    else if (dAngle < 25.0) d9 = 1.00;
    else if (dAngle < 50.0) d9 = 1.30;
    else                    d9 = 1.60;

    double d10 = 1.0;
    if (dAngle > 15.0) {
        double k = std::clamp((dAngle - 15.0) / 60.0, 0.0, 1.0);
        d10 = 1.0 + k * 1.4;
    }

    double speedYawBase   = (m_speed_yaw   * 3.0) / 200.0;
    double speedPitchBase = (m_speed_pitch * 3.0) / 200.0;

    double fallMul = 1.0;
    float fallDist = main.fallDistance();
    if (fallDist > 0.0f) {
        fallMul = 1.0 + std::clamp((double)fallDist * 0.15, 0.0, 0.7);
    }

    if (!main.isOnGround() && main.position().getY() > 0.1) {
        fallMul *= 0.90;
    }

    double stepYaw   = dYaw   * speedYawBase   * curve * d9 * d10 * m_aim_speed_mul * fallMul;
    double stepPitch = dPitch * speedPitchBase * curve * d9 * d10 * m_aim_speed_mul * 0.85 * fallMul;

    m_aim_last_step_yaw   = stepYaw;
    m_aim_last_step_pitch = stepPitch;

    float newYaw   = mainYaw   + (float)stepYaw;
    float newPitch = std::clamp(mainPitch + (float)stepPitch, -90.0f, 90.0f);

    if (!std::isnan(newYaw) && !std::isnan(newPitch) &&
        !std::isinf(newYaw) && !std::isinf(newPitch)) {
        main.setYaw(newYaw);
        main.setPitch(newPitch);
    }
}

