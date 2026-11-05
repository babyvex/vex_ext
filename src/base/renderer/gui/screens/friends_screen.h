//
// Created by natty on 29.09.2026.
//

#pragma once

#include "abstract_screen.h"

class friends_screen : public abstract_screen
{
public:
    ~friends_screen() override { on_release_gpu(); }

protected:
    void draw_screen_content() override;

private:
    void load_textures();
    void on_release_gpu();

    void render_tabs();
    void render_tabs_content();

    static const char* tab_name(int i);

    static constexpr int kMaxTabs = 2;

    int   m_current_tab = 0;
    float m_tab_line_anim[kMaxTabs] = {};

    texture friends_icon;
    texture requests_icon;

    bool m_icons_loaded = false;
};

