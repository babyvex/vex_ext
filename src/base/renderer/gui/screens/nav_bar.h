#pragma once
#include <array>
#include "screen_id.h"
#include "../../windows/texture.h"

class nav_bar {
public:
    static constexpr int kTabCount = 3;

    static nav_bar& get();
    int  draw(screen_id current);
    void release();

private:
    void load_icons();

    std::array<texture, kTabCount> m_icons;
    std::array<float,   kTabCount> m_width{};
    bool m_loaded = false;
};