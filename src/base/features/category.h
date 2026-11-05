#pragma once



#include <array>
#include <string_view>

enum class category {
    combat,
    visual,
    movement,
    misc,
    settings,
    count
};

struct category_entry {
    category    id;
    const char* name;
};

inline constexpr std::array<category_entry, 5> g_categories = {{
    { category::combat,  "Combat"  },
    { category::visual,  "Visual"  },
    { category::movement, "Movement" },
    { category::misc, "Misc" },
    { category::settings, "Settings" },
}};

namespace categories {
    inline constexpr int count() {
        return static_cast<int>(g_categories.size());
    }

    inline const char* name(int index) {
        if (index < 0 || index >= count()) return "";
        return g_categories[index].name;
    }

    inline category id(int index) {
        if (index < 0 || index >= count()) return category::count;
        return g_categories[index].id;
    }

    inline int index_of(category cat) {
        for (int i = 0; i < count(); i++)
            if (g_categories[i].id == cat) return i;
        return -1;
    }
}