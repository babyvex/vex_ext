//
// Created by natty on 09.10.2026.
//

#include "mappings_manager.h"

#include <mutex>

#include "client_parser.h"
#include "mapping_parser.h"
#include "mJNI/mJNI.h"

std::unique_ptr<mapping_parser> global_mapping_parser;


static std::unordered_map<std::string, jclass>    global_klasses;
static std::unordered_map<std::string, jfieldID>  global_fields;
static std::unordered_map<std::string, jmethodID> global_methods;
static std::mutex                                 global_mappings_mutex;

void mappings::init() {
    client_parser::get().init();

    auto version = client_parser::get().get_version();
    auto client = client_parser::get().get_client_type();
    auto launcher = client_parser::get().get_launcher_mapping_type();

    global_mapping_parser = std::make_unique<mapping_parser>();

    auto target = client_type_to_mapping(client);

    if (launcher == intermediary && client == vanilla) {
        target = intermediary;
    }

    bool loaded = global_mapping_parser->load(version.c_str(), target);

    // if (!loaded) {
    //     std::string customPath = "D:\\Mappings\\" + version + "\\mappings.tiny";
    //     if (target == intermediary) {
    //         customPath = "D:\\Mappings\\" + version + "\\mappings (1).tiny";
    //     } else if (target == obfuscate) {
    //         customPath = "D:\\Mappings\\" + version + "\\mappings (2).tiny";
    //     }
    //     loaded = global_mapping_parser->load(version.c_str(), target, customPath.c_str());
    // }

}

jfieldID mappings::find_minecraft_field_with_owner_klass(jclass klass, const char *ownerClass, const char *fieldName,
    const char *fieldDesc) {
    if (!klass || !ownerClass || !fieldName) return 0;

    std::string key = std::string(ownerClass) + "|" + fieldName + "|" + (fieldDesc ? fieldDesc : "");
    {
        std::lock_guard<std::mutex> lock(global_mappings_mutex);
        auto it = global_fields.find(key);
        if (it != global_fields.end()) return it->second;
    }

    const char* targetFieldName = fieldName;
    const char* targetDesc = fieldDesc;

    if (global_mapping_parser) {
        auto fInfo = global_mapping_parser->find_field(ownerClass, fieldName);
        if (fInfo) {
            targetFieldName = fInfo->get_name();
            if (fInfo->get_desc() && strlen(fInfo->get_desc()) > 0) {
                targetDesc = fInfo->get_desc();
            }
        }
    }

    jfieldID fid = mJNI::findFieldID(klass, targetFieldName, targetDesc);
    if (!fid && targetFieldName != fieldName) {
        fid = mJNI::findFieldID(klass, fieldName, fieldDesc);
    }
    if (!fid && targetDesc) {
        fid = mJNI::findFieldID(klass, targetFieldName, nullptr);
    }

    {
        std::lock_guard<std::mutex> lock(global_mappings_mutex);
        global_fields[key] = fid;
    }
    return fid;
}

jmethodID mappings::find_minecraft_method_with_owner_klass(jclass klass, const char *ownerClass, const char *methodName,
const char *methodDesc) {if (!klass || !ownerClass || !methodName) return 0;

    std::string key = std::string(ownerClass) + "|" + methodName + "|" + (methodDesc ? methodDesc : "");
    {
    std::lock_guard<std::mutex> lock(global_mappings_mutex);
    auto it = global_methods.find(key);
    if (it != global_methods.end()) return it->second;
    }

    const char* targetMethodName = methodName;
    const char* targetDesc = methodDesc;

    if (global_mapping_parser) {
        auto mInfo = global_mapping_parser->find_method(ownerClass, methodName);
        if (mInfo) {
            targetMethodName = mInfo->get_name();
            if (mInfo->get_desc() && strlen(mInfo->get_desc()) > 0) {
                targetDesc = mInfo->get_desc();
            }
        }
    }

    jmethodID mid = mJNI::findMethodID(klass, targetMethodName, targetDesc);
    if (!mid && targetMethodName != methodName) {
        mid = mJNI::findMethodID(klass, methodName, methodDesc);
    }
    if (!mid && targetDesc) {
        mid = mJNI::findMethodID(klass, targetMethodName, nullptr);
    }

    {
    std::lock_guard<std::mutex> lock(global_mappings_mutex);
    global_methods[key] = mid;
    }
    return mid;
}

jclass mappings::find_minecraft_klass(const char *name) {
    if (!name) return nullptr;

    std::string key(name);
    {
        std::lock_guard<std::mutex> lock(global_mappings_mutex);
        auto it = global_klasses.find(key);
        if (it != global_klasses.end()) return it->second;
    }

    const char* targetName = name;

    if (global_mapping_parser) {
        auto clsInfo = global_mapping_parser->find_class(name);
        if (clsInfo) {
            targetName = clsInfo->get_name();

        }

        jclass klass = mJNI::findClass(targetName);
        if (!klass && targetName != name) {
            klass = mJNI::findClass(name);
        }

        {
            std::lock_guard<std::mutex> lock(global_mappings_mutex);
            global_klasses[key] = klass;
        }
        return klass;
    }
}