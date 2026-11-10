//
// Created by natty on 26.09.2026.
//

#include "mapping_parser.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <utility>

#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

const char* version_to_str(mapping_versions v)
{
    switch (v)
    {
        case mapping_versions::v_1_21:    return "1.21";
        case mapping_versions::v_1_21_1:  return "1.21.1";
        case mapping_versions::v_1_21_2:  return "1.21.2";
        case mapping_versions::v_1_21_3:  return "1.21.3";
        case mapping_versions::v_1_21_4:  return "1.21.4";
        case mapping_versions::v_1_21_5:  return "1.21.5";
        case mapping_versions::v_1_21_6:  return "1.21.6";
        case mapping_versions::v_1_21_7:  return "1.21.7";
        case mapping_versions::v_1_21_8:  return "1.21.8";
        case mapping_versions::v_1_21_9:  return "1.21.9";
        case mapping_versions::v_1_21_10: return "1.21.10";
        case mapping_versions::v_1_21_11: return "1.21.11";
    }
    return "1.21";
}

namespace
{
    const char* DEFAULT_MAPPINGS_ROOT_A = MAPPINGS_ROOT_DIR;

    const char* targetFileName(acceptable_launcher_mapping_type t)
    {
        switch (t)
        {
            case acceptable_launcher_mapping_type::mojang:       return "mappings.tiny";
            case acceptable_launcher_mapping_type::intermediary: return "mappings (1).tiny";
            case acceptable_launcher_mapping_type::obfuscate:    return "mappings (2).tiny";
        }
        return "mappings.tiny";
    }

    const char* targetName(acceptable_launcher_mapping_type t)
    {
        switch (t)
        {
            case acceptable_launcher_mapping_type::mojang:       return "MOJANG";
            case acceptable_launcher_mapping_type::intermediary: return "INTERMEDIARY";
            case acceptable_launcher_mapping_type::obfuscate:    return "OBFUSCATED";
        }
        return "?";
    }
}

void mapping_parser::download_to_bytes(const char* url)
{
    std::vector<unsigned char> out;

    HINTERNET hInternet = InternetOpenA("vex-mappings",
                                        INTERNET_OPEN_TYPE_DIRECT,
                                        nullptr, nullptr, 0);
    if (!hInternet) return;

    HINTERNET hFile = InternetOpenUrlA(
        hInternet, url, nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);

    if (hFile)
    {
        unsigned char buf[4096];
        DWORD bytesRead = 0;

        while (InternetReadFile(hFile, buf, sizeof(buf), &bytesRead) && bytesRead > 0)
            out.insert(out.end(), buf, buf + bytesRead);

        InternetCloseHandle(hFile);
    }
    InternetCloseHandle(hInternet);

    if (!out.empty())
        parse_from_bytes(out.data(), out.size());
}

void mapping_parser::parse_from_file(const std::filesystem::path& file_path)
{
    std::ifstream f(file_path, std::ios::binary);
    if (!f)
        return;

    std::vector<unsigned char> bytes(std::istreambuf_iterator<char>(f), {});
    parse_from_bytes(bytes.data(), bytes.size());
}

void mapping_parser::parse_from_bytes(const unsigned char* data, size_t size)
{
    if (!data || size == 0)
        return;

    class_map.clear();
    field_map.clear();
    method_map.clear();
    reverse_class_map.clear();

    class_map.reserve(8192);
    field_map.reserve(8192);
    method_map.reserve(8192);
    reverse_class_map.reserve(8192);

    const char* p   = reinterpret_cast<const char*>(data);
    const char* end = p + size;

    std::vector<field_info>*  curFields  = nullptr;
    std::vector<method_info>* curMethods = nullptr;

    auto tabAt = [](const char* s, const char* e) -> const char* {
        return static_cast<const char*>(std::memchr(s, '\t', e - s));
    };

    while (p < end)
    {
        const char* lineStart = p;
        while (p < end && *p != '\n') ++p;
        const char* lineEnd = p;
        if (p < end) ++p;
        if (lineEnd > lineStart && *(lineEnd - 1) == '\r')
            --lineEnd;

        const size_t len = static_cast<size_t>(lineEnd - lineStart);
        if (len == 0)
            continue;

        if (len >= 2 && lineStart[0] == 'c' && lineStart[1] == '\t')
        {
            const char* t1    = lineStart + 2;
            const char* t1End = tabAt(t1, lineEnd);
            if (!t1End) continue;
            const char* t2    = t1End + 1;

            std::string yarn(t1, t1End - t1);
            std::string target(t2, lineEnd - t2);

            class_map[yarn]           = { yarn, target };
            reverse_class_map[target] = yarn;
            curFields                 = &field_map[yarn];
            curMethods                = &method_map[yarn];
        }
        else if (len >= 3 && lineStart[0] == '\t'
                 && lineStart[1] == 'f' && lineStart[2] == '\t'
                 && curFields)
        {
            const char* t1    = lineStart + 3;
            const char* t1End = tabAt(t1, lineEnd);
            if (!t1End) continue;
            const char* t2    = t1End + 1;
            const char* t2End = tabAt(t2, lineEnd);
            if (!t2End) continue;
            const char* t3    = t2End + 1;

            field_info info;
            info.desc.assign(t1, t1End - t1);
            info.yarn.assign(t2, t2End - t2);
            info.target.assign(t3, lineEnd - t3);
            curFields->push_back(std::move(info));
        }
        else if (len >= 3 && lineStart[0] == '\t'
                 && lineStart[1] == 'm' && lineStart[2] == '\t'
                 && curMethods)
        {
            const char* t1    = lineStart + 3;
            const char* t1End = tabAt(t1, lineEnd);
            if (!t1End) continue;
            const char* t2    = t1End + 1;
            const char* t2End = tabAt(t2, lineEnd);
            if (!t2End) continue;
            const char* t3    = t2End + 1;

            method_info info;
            info.desc.assign(t1, t1End - t1);
            info.yarn.assign(t2, t2End - t2);
            info.target.assign(t3, lineEnd - t3);
            curMethods->push_back(std::move(info));
        }
    }
}

std::string mapping_parser::replace_classes_descriptor(const char* descriptor)
{
    if (!descriptor) return {};
    std::string desc(descriptor);

    if (desc.find('L') == std::string::npos)
        return desc;

    std::string result;
    result.reserve(desc.size());

    const size_t n = desc.size();
    size_t i = 0;

    while (i < n)
    {
        const char c = desc[i];
        if (c == 'L')
        {
            const size_t semi = desc.find(';', i + 1);
            if (semi != std::string::npos)
            {
                const std::string yarnCls = desc.substr(i + 1, semi - i - 1);

                if (const class_info* ci = find_class(yarnCls.c_str()))
                {
                    result.push_back('L');
                    result.append(ci->target);
                    result.push_back(';');
                    i = semi + 1;
                    continue;
                }
            }
        }
        result.push_back(c);
        ++i;
    }

    return result;
}

void mapping_parser::build_lookup_indexes()
{
    field_index.clear();
    method_index.clear();

    for (auto& [clsYarn, fVec] : field_map)
    {
        auto& subMap = field_index[clsYarn];
        subMap.reserve(fVec.size());
        for (auto& f : fVec)
            subMap[f.yarn] = &f;
    }

    for (auto& [clsYarn, mVec] : method_map)
    {
        auto& subMap = method_index[clsYarn];
        subMap.reserve(mVec.size());
        for (auto& m : mVec)
            subMap[m.yarn] = &m;
    }
}

bool mapping_parser::load(const char* version,
                          acceptable_launcher_mapping_type target,
                          const char* custom_path)
{
    loaded_mappings = target;

    std::filesystem::path file;
    if (custom_path && *custom_path && std::strcmp(custom_path, " ") != 0)
    {
        file = custom_path;
    }
    else
    {
        std::filesystem::path root = std::filesystem::u8path(DEFAULT_MAPPINGS_ROOT_A);
        file = root / (version ? version : "") / targetFileName(target);
    }

    if (!std::filesystem::exists(file))
        return false;

    parse_from_file(file);

    if (class_map.empty())
        return false;

    build_lookup_indexes();
    return true;
}

bool mapping_parser::load(mapping_versions versions,
                          acceptable_launcher_mapping_type target,
                          const char* custom_path)
{
    return load(version_to_str(versions), target, custom_path);
}

class_info* mapping_parser::find_class(const char* class_name)
{
    if (!class_name) return nullptr;
    auto it = class_map.find(class_name);
    return (it != class_map.end()) ? &it->second : nullptr;
}

field_info* mapping_parser::find_field(const char* class_name,
                                       const char* field_name)
{
    if (!class_name || !field_name) return nullptr;

    auto clsIt = field_index.find(class_name);
    if (clsIt == field_index.end()) return nullptr;

    auto fIt = clsIt->second.find(field_name);
    if (fIt != clsIt->second.end())
    {
        field_info* info = fIt->second;
        if (info && !info->desc_resolved)
        {
            info->desc = replace_classes_descriptor(info->desc.c_str());
            info->desc_resolved = true;
        }
        return info;
    }

    for (auto& [name, info] : clsIt->second)
    {
        if (_stricmp(name.c_str(), field_name) == 0)
        {
            if (info && !info->desc_resolved)
            {
                info->desc = replace_classes_descriptor(info->desc.c_str());
                info->desc_resolved = true;
            }
            return info;
        }
    }

    return nullptr;
}

method_info* mapping_parser::find_method(const char* class_name,
                                         const char* method_name)
{
    if (!class_name || !method_name) return nullptr;

    auto clsIt = method_index.find(class_name);
    if (clsIt == method_index.end()) return nullptr;

    auto mIt = clsIt->second.find(method_name);
    if (mIt == clsIt->second.end()) return nullptr;

    method_info* info = mIt->second;
    if (info && !info->desc_resolved)
    {
        info->desc = replace_classes_descriptor(info->desc.c_str());
        info->desc_resolved = true;
    }
    return info;
}

const char* mapping_parser::target_to_yarn(const char* target)
{
    if (!target) return nullptr;

    auto cacheIt = target_to_yarn_cache.find(target);
    if (cacheIt != target_to_yarn_cache.end())
        return cacheIt->second.c_str();

    auto it = reverse_class_map.find(target);
    if (it != reverse_class_map.end())
    {
        auto [ins, _] = target_to_yarn_cache.emplace(target, it->second);
        return ins->second.c_str();
    }

    auto [ins, _] = target_to_yarn_cache.emplace(target, target);
    return ins->second.c_str();
}
