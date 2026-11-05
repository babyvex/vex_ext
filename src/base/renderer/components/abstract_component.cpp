#include "abstract_component.h"

abstract_component::abstract_component(component_type type)
    : m_type(type) {}

abstract_component::~abstract_component() = default;

component_type abstract_component::get_type() const { return m_type; }

std::string abstract_component::serialize() const { return {}; }

void abstract_component::deserialize(const std::string& ) {}