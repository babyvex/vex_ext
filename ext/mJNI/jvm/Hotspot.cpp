//
// Created by natty on 14.07.2026.
//

#include "Hotspot.h"

#include <vector>
#include <Windows.h>

#include "../memory/Memory.h"
#include "../utils/Utils.h"

static std::vector<ExportFunction> mExports; // g

HotspotVM::structMap HotspotVM::mStructMap;
HotspotVM::typeMap HotspotVM::mTypeMap;

HotspotVM::HotspotVM() {
    instance = this;
}

bool HotspotVM::Initialize() {
    for (auto& [name, fields] : mStructMap) {
        for (auto& [field, value] : fields) {
            free(const_cast<char*>(value->typeName));
            free(const_cast<char*>(value->fieldName));
            free(const_cast<char*>(value->typeString));
            delete value;
        }
    }
    mStructMap.clear();
    for (auto& [name, value] : mTypeMap) {
        free(const_cast<char*>(value->type_name));
        free(const_cast<char*>(value->superclass_name));
        delete value;
    }
    mTypeMap.clear();
    return parseStructs();
}

std::optional<std::reference_wrapper<HotspotVM::structEntry>> HotspotVM::findTypeFields(const std::string &typeName) {
    auto it = mStructMap.find(typeName);
    if (it != mStructMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::reference_wrapper<VMTypeEntry>>
HotspotVM::findType(const std::string& typeName) {
    auto it = mTypeMap.find(typeName);
    if (it != mTypeMap.end()) {
        return std::ref(*it->second);
    }
    return std::nullopt;
}

int HotspotVM::JvmVersion() {

    auto strct = HotspotVM::findTypeFields("JDK_Version");
    if (!strct.has_value()) {
        auto offset = findTypeFields("Abstract_VM_Version").value().get().at("_vm_major_version")->address;
        int version = 0;
        mReadMemory(&version, offset, sizeof(version));

        return version;
    }

    auto current = strct.value().get().at("_current")->address;
    auto majorOffset = strct.value().get().at("_major")->offset;
    unsigned char version = 0;
    mReadMemory(&version, (void*)((uintptr_t)current + majorOffset), sizeof(version));

    return version;
}

void* HotspotVM::findFlagAddress(const char* name) {
    const auto fields = findTypeFields("JVMFlag");
    const auto type = findType("JVMFlag");
    if (!fields || !type) return nullptr;
    const auto& entries = fields->get();
    for (const auto* field : {"flags", "numFlags", "_name", "_addr"}) {
        if (!entries.contains(field)) return nullptr;
    }

    uintptr_t flags = 0;
    size_t count = 0;
    const auto stride = type->get().size;
    if (!stride || !mReadMemory(&flags, entries.at("flags")->address, sizeof(flags)) || !flags ||
        !mReadMemory(&count, entries.at("numFlags")->address, sizeof(count)) || count > 65536) return nullptr;

    for (size_t i = 0; i < count; ++i) {
        void* nameAddress = nullptr;
        const auto address = flags + i * stride;
        if (!mReadMemory(&nameAddress, reinterpret_cast<void*>(address + entries.at("_name")->offset),
                        sizeof(nameAddress)) || !nameAddress) continue;
        char flagName[256]{};
        if (!mReadMemory(flagName, nameAddress, sizeof(flagName) - 1)) continue;
        if (std::string(flagName) == name) {
            return readPointer(reinterpret_cast<void*>(address + entries.at("_addr")->offset));
        }
    }
    return nullptr;
}


void * HotspotVM::findExport(const char *name) {
    for (auto& e : mExports) {
        if (e.name == name) {
            return (void*)((uintptr_t)global_process_data.jvm_dll_address + e.address);
        }
    }
    return nullptr;
}

bool HotspotVM::parseStructs() {

    mExports = mJNI::Utils::copyExports(global_process_data.jvm_dll_address);

    void* gHotSpotVMStructs = findExport("gHotSpotVMStructs");
    void* gHotSpotVMTypes = findExport("gHotSpotVMTypes");

    uintptr_t oRes = 0;
    uintptr_t tRes = 0;
    uint64_t structStride = 0, typeStride = 0;
    if (!mReadMemory(&oRes, gHotSpotVMStructs, sizeof(oRes)) || !oRes ||
        !mReadMemory(&tRes, gHotSpotVMTypes, sizeof(tRes)) || !tRes ||
        !mReadMemory(&structStride, findExport("gHotSpotVMStructEntryArrayStride"), sizeof(structStride)) ||
        !mReadMemory(&typeStride, findExport("gHotSpotVMTypeEntryArrayStride"), sizeof(typeStride)) ||
        structStride != sizeof(VMStructEntry) || typeStride != sizeof(VMTypeEntry)) return false;

    bool structsComplete = false;
    for (size_t i = 0; i < 65536; ++i) {

        VMStructEntry entry{};
        uintptr_t structAddr = oRes + i * structStride;
        if (!mReadMemory(&entry, reinterpret_cast<void *>(structAddr), sizeof(entry))) return false;

        if (entry.typeName == nullptr) {
            structsComplete = true;
            break;
        }

        char typeName[256] = { 0 };
        char fieldName[256] = { 0 };
        char typeString[256] = { 0 };

        if (!mReadMemory(typeName, entry.typeName, sizeof(typeName) - 1) ||
            !mReadMemory(fieldName, entry.fieldName, sizeof(fieldName) - 1)) return false;
        if (entry.typeString && !mReadMemory(typeString, entry.typeString, sizeof(typeString) - 1)) return false;

        VMStructEntry* out = new VMStructEntry();
        out->typeName = _strdup(typeName);
        out->fieldName = _strdup(fieldName);
        out->typeString = _strdup(typeString);
        out->isStatic = entry.isStatic;
        out->offset = entry.offset;
        out->address = entry.address;

        mStructMap[typeName][fieldName] = out;
    }

    bool typesComplete = false;
    for (size_t i = 0; i < 65536; ++i) {
        uintptr_t typeAddr = tRes + i * typeStride;

        VMTypeEntry entry{};
        if (!mReadMemory(&entry, reinterpret_cast<void*>(typeAddr), sizeof(entry))) return false;
        if (!entry.type_name) {
            typesComplete = true;
            break;
        }

        char typeName[256] = { 0 };
        char superclassName[256] = { 0 };

        if (!mReadMemory(typeName, entry.type_name, sizeof(typeName) - 1)) return false;
        if (entry.superclass_name) {
            if (!mReadMemory(superclassName, entry.superclass_name, sizeof(superclassName) - 1)) return false;
        }

        VMTypeEntry* out = new VMTypeEntry();
        out->type_name = _strdup(typeName);
        out->superclass_name = entry.superclass_name ? _strdup(superclassName) : nullptr;
        out->is_oop_type = entry.is_oop_type;
        out->is_integer_type = entry.is_integer_type;
        out->is_unsigned = entry.is_unsigned;
        out->size = entry.size;

        mTypeMap[typeName] = out;
    }

    return structsComplete && typesComplete;
}
