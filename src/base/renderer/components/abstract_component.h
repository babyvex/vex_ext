#pragma once


#include <string>

#include "component_types.h"

class abstract_component
{
public:
    explicit abstract_component(component_type type);
    virtual ~abstract_component();

    component_type get_type() const;

    virtual std::string serialize() const;
    virtual void deserialize(const std::string& data);

    virtual void render_in_settings_panel() = 0;
    virtual void render_in_tile_module()     = 0;

    virtual bool wants_tile_render()   const { return false; }
    virtual bool wants_panel_render()  const { return true;  }

    virtual float get_height() const { return 32.0f; }

protected:
    component_type m_type;
};

