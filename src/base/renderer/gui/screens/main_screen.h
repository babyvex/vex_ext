


#pragma once
#include <unordered_map>
#include <unordered_set>

#include "abstract_screen.h"

class abstract_module;

class main_screen : public abstract_screen
{
public:
    ~main_screen() override { on_release_gpu(); }

    void render_tabs_content();

protected:
    void draw_screen_content() override;
    void on_release_gpu() override;


private:
    static constexpr int kMaxTabs = 16;

    void load_textures();
    void render_tabs();

    texture combat_text;
    texture misc_text;
    texture movement_text;
    texture visual_text;
    texture settings_text;
    texture magnify_text;
    texture right_arrow_text;
    texture vertline_text;
    texture toggle_back;
    texture toggle_fill;
    texture favorites_text;

    bool  m_icons_loaded = false;

    int   m_current_tab = 0;
    float m_tab_line_anim[kMaxTabs] = {};

    void render_module_card(abstract_module* module, ImVec2 pos, float width);

    std::unordered_map<abstract_module*, float> m_toggle_anim;

    abstract_module* m_opened_module   = nullptr;
    bool             m_module_panel_open = false;

    std::unordered_set<abstract_module*> m_pinned;

    abstract_module* m_open_panel_module = nullptr;
    std::unordered_map<abstract_module*, float> m_panel_anim;
    ImVec2 m_open_panel_pos{0.0f, 0.0f};
    float m_module_panel_width = 0.0f;

    float m_target_scroll = 0.0f;
    float m_current_scroll = 0.0f;
    void render_module_panel();
    std::unordered_map<abstract_module*, float> m_bump_anim;

    struct spring_state { float value = 0.0f; float velocity = 0.0f; };
    std::unordered_map<abstract_module*, spring_state> m_card_spring;
    float m_card_spring_time = 0.0f;

    float m_panel_dim_alpha = 0.0f;
    bool m_click_consumed_by_panel = false;
    bool m_panel_blocks_input   = false;

    bool m_dots_clicked_this_frame = false;
    std::unordered_map<abstract_module*, float> m_dots_anim;
    float m_panel_anim_t = 0.0f;
    abstract_module* m_panel_rendering_module = nullptr;

    float m_panel_scroll       = 0.0f;
    float m_panel_scroll_target= 0.0f;

    float m_panel_content_bottom = 0.0f;


    bool  m_search_open     = false;
    float m_search_anim_t   = 0.0f;
    char  m_search_buf[128] = {};
    bool  m_search_active   = false;

    void render_search_icon();
    void render_search_panel();
    bool  m_search_icon_hovered_prev = false;


};

