//
// Created by natty on 26.09.2026.
//

#include "client_parser.h"

#include <regex>

#include "mJNI/mJNI.h"

namespace command_line_helper {

    std::vector<std::string> tokenize(const std::string& cli) {
        std::vector<std::string> out;
        std::string cur;
        bool inQuote = false;
        for (char c : cli) {
            if (c == '"') { inQuote = !inQuote; continue; }
            if (!inQuote && std::isspace(static_cast<unsigned char>(c))) {
                if (!cur.empty()) { out.push_back(cur); cur.clear(); }
            }
            else {
                cur.push_back(c);
            }
        }
        if (!cur.empty()) out.push_back(cur);
        return out;
    }

    std::string find_flag(const std::vector<std::string>& tokens, const std::string& flag) {
        for (size_t i = 0; i + 1 < tokens.size(); ++i) {
            if (tokens[i] == flag) return tokens[i + 1];
        }
        return {};
    }

    bool isSupported(const std::string& v) {
        for (const auto& s : acceptable_minecraft_version) {
            if (s == v) return true;
        }
        return false;
    }

}

int client_parser::detect_mapping_launcher() {
    if (mJNI::findClass("net/minecraft/client/Minecraft")) {
        return ( current_launcher_mapping_type = mojang ) ;
    }

    if (mJNI::findClass("net/minecraft/class_310")) {
        return ( current_launcher_mapping_type = intermediary ) ;
    }

    if (mJNI::findClass("eog")) {
        return ( current_launcher_mapping_type = obfuscate ) ;
    }

    return ( current_launcher_mapping_type = mojang );
}


bool client_parser::parse_version() {

    std::string version {};

    auto command_line = mJNI::Utils::getCommandLineByHandle(global_process_data.jvm_handle);

    auto tokens = command_line_helper::tokenize(command_line);

    auto extract = [](const std::string& s) -> std::string {
        std::regex rx(R"((?:^|[^\d])(1\.21(?:\.(?:1[01]|[0-9]))?)(?!\d))");
        std::smatch m;
        return std::regex_search(s, m, rx) ? m[1].str() : std::string{};
    };

    auto version_flag = command_line_helper::find_flag(tokens, "--version");

    if (version_flag.empty()) {
        return false;
    }

    version = extract(version_flag);

    auto asset_index = command_line_helper::find_flag(tokens, "--assetIndex");

    if (asset_index.empty()) {
        return false;
    }

    version = extract(version_flag);

    auto cmd_lower = command_line;
    std::ranges::transform(cmd_lower, cmd_lower.begin(), [](unsigned char c) { return std::tolower(c); });

    if (cmd_lower.find("lunar") != std::string::npos) {
        current_client_type = lunar;
    } else if (cmd_lower.find("badlion") != std::string::npos) {
        current_client_type = badlion;
    } else if (cmd_lower.find("feather") != std::string::npos) {
        current_client_type = feather;
    } else if (cmd_lower.find("labymod") != std::string::npos) {
        current_client_type = labymod;
    } else if (cmd_lower.find("fabric") != std::string::npos) {
        current_client_type = fabric;
    } else if (cmd_lower.find("forge") != std::string::npos || cmd_lower.find("neoforge") != std::string::npos) {
        current_client_type = forge;
    } else {
        current_client_type = vanilla;
    }

    current_version = version;

    return true;
}


void client_parser::init() {

    detect_mapping_launcher();

    parse_version();
}

acceptable_launcher_mapping_type client_parser::get_launcher_mapping_type() {
    return current_launcher_mapping_type;
}

acceptable_client_type client_parser::get_client_type() {
    return current_client_type;
}

std::string client_parser::get_version() {
    return current_version;
}
