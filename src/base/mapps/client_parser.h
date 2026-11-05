//
// Created by natty on 26.09.2026.
//

#include <string>
#include <vector>

#pragma once
enum acceptable_launcher_mapping_type {
    mojang = 0,
    intermediary,
    obfuscate
};

enum acceptable_client_type {
    lunar = 0,
    fabric,
    vanilla,
    labymod,
    badlion,
    feather,
    forge
};

inline std::vector<std::string> acceptable_minecraft_version =
{
    "1.21", "1.21.1", "1.21.2", "1.21.3", "1.21.4", "1.21.5",
   "1.21.6", "1.21.7", "1.21.8", "1.21.9", "1.21.10", "1.21.11"
};


inline acceptable_launcher_mapping_type client_type_to_mapping (acceptable_client_type client_type) {

    switch (client_type) {
        case acceptable_client_type::lunar:
        case acceptable_client_type::badlion:
        case acceptable_client_type::forge:   return mojang;
        case acceptable_client_type::feather: return intermediary;
        case acceptable_client_type::fabric:  return intermediary;
        case acceptable_client_type::labymod:
        case acceptable_client_type::vanilla: return obfuscate;
        default:                  return mojang;
    }

}

class client_parser {
private:


    int detect_mapping_launcher();
    bool parse_version();

    acceptable_launcher_mapping_type current_launcher_mapping_type = {};
    acceptable_client_type current_client_type = {};

    std::string current_version {};

public:

    void init();

    acceptable_launcher_mapping_type get_launcher_mapping_type();
    acceptable_client_type get_client_type();
    std::string get_version();

    static client_parser& get() {
        static client_parser instance;
        return instance;
    }
};


