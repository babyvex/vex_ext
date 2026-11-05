#pragma once



#include "abstract_screen.h"


class configs_screen : public abstract_screen
{
public:
    void draw_screen_content() override;
    ~configs_screen() override { on_release_gpu(); }

private:
    void load_textures();
    void on_release_gpu();

    void render_tabs();
    void render_tabs_content();

    static const char* tab_name(int i);

    static constexpr int kMaxTabs = 2;

    int   m_current_tab = 0;
    float m_tab_line_anim[kMaxTabs] = {};

    texture my_config_icon;
    texture public_config_icon;

    bool m_icons_loaded = false;
};
