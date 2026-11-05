//
// Created by natty on 26.09.2026.
//

#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "client_parser.h"

struct field_info {
    std::string yarn;
    std::string target;
    std::string desc;
    bool desc_resolved{ false };

    [[nodiscard]] const char* get_name() const { return target.c_str(); }
    [[nodiscard]] const char* get_desc() const { return desc.c_str(); }
};

struct method_info {
    std::string yarn;
    std::string target;
    std::string desc;
    bool desc_resolved{ false };

    [[nodiscard]] const char* get_name() const { return target.c_str(); }
    [[nodiscard]] const char* get_desc() const { return desc.c_str(); }
};

struct class_info {
    std::string yarn;
    std::string target;

    [[nodiscard]] const char* get_name() const { return target.c_str(); }
};

enum class mapping_versions {
    v_1_21,
    v_1_21_1,
    v_1_21_2,
    v_1_21_3,
    v_1_21_4,
    v_1_21_5,
    v_1_21_6,
    v_1_21_7,
    v_1_21_8,
    v_1_21_9,
    v_1_21_10,
    v_1_21_11
};

const char* version_to_str(mapping_versions v);


class mapping_parser {
private:
    std::unordered_map<std::string, class_info>                                       class_map;
    std::unordered_map<std::string, std::vector<field_info>>                          field_map;
    std::unordered_map<std::string, std::vector<method_info>>                         method_map;
    std::unordered_map<std::string, std::unordered_map<std::string, field_info*>>     field_index;
    std::unordered_map<std::string, std::unordered_map<std::string, method_info*>>    method_index;
    std::unordered_map<std::string, std::string>                                      reverse_class_map;
    std::unordered_map<std::string, std::string>                                      target_to_yarn_cache;


    acceptable_launcher_mapping_type loaded_mappings {};

    void download_to_bytes(const char* url);
    void parse_from_file(const std::filesystem::path& file_path);
    void parse_from_bytes(const unsigned char* bytes, size_t size);

    std::string replace_classes_descriptor(const char* descriptor);
    void build_lookup_indexes();
    

public:

    bool load(const char* version, acceptable_launcher_mapping_type target, const char* custom_path = " " );
    bool load(mapping_versions versions, acceptable_launcher_mapping_type target, const char* custom_path = " " );

    class_info* find_class(const char* class_name);
    field_info* find_field(const char* class_name, const char* field_name);
    method_info* find_method(const char* class_name, const char* method_name);

    const char* target_to_yarn(const char* target);

};


