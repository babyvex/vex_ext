#include "Offsets.h"

#include <stdexcept>

#include "../memory/Memory.h"

namespace {
    const VMStructEntry& field(const char* type, const char* name) {
        const auto fields = HotspotVM::findTypeFields(type);
        if (fields) {
            const auto it = fields->get().find(name);
            if (it != fields->get().end()) return *it->second;
        }
        throw std::runtime_error(std::string("Missing HotSpot field: ") + type + "::" + name);
    }

    uint64_t offset(const char* type, const char* name) { return field(type, name).offset; }
    uint64_t address(const char* type, const char* name) {
        return reinterpret_cast<uint64_t>(field(type, name).address);
    }

    uint64_t compressedAddress(const char* type, const char* current, const char* legacy) {
        const auto fields = HotspotVM::findTypeFields(type);
        if (!fields) return 0;
        const auto& entries = fields->get();
        for (const auto* name : {current, legacy}) {
            const auto it = entries.find(name);
            if (it != entries.end()) return reinterpret_cast<uint64_t>(it->second->address);
        }
        return 0;
    }
}

void Offsets::InitializeOffsets() {
    const int version = HotspotVM::JvmVersion();
    if (version < 17) throw std::runtime_error("HotSpot 17 or newer is required.");

    // These are independent JVM options on both x64 and AArch64.
    CompressedOops::useCompressedOops = reinterpret_cast<uint64_t>(HotspotVM::findFlagAddress("UseCompressedOops"));
    CompressedKlassPointers::useCompressedClassPointers =
        reinterpret_cast<uint64_t>(HotspotVM::findFlagAddress("UseCompressedClassPointers"));
    if (!CompressedOops::useCompressedOops || !CompressedKlassPointers::useCompressedClassPointers) {
        throw std::runtime_error("Cannot read HotSpot compressed pointer options.");
    }
    if (auto flag = HotspotVM::findFlagAddress("UseCompactObjectHeaders")) {
        bool compact = false;
        if (!mReadMemory(&compact, flag, sizeof(compact)) || compact) {
            throw std::runtime_error("Compact object headers are unsupported; use -XX:-UseCompactObjectHeaders.");
        }
    }

    OopDesc::klass = offset("oopDesc", "_metadata._klass");
    OopDesc::compressedKlass = offset("oopDesc", "_metadata._compressed_klass");

    InstanceKlass::name = offset("Klass", "_name");
    InstanceKlass::nextLink = offset("Klass", "_next_link");
    InstanceKlass::constants = offset("InstanceKlass", "_constants");
    InstanceKlass::methods = offset("InstanceKlass", "_methods");
    InstanceKlass::fields = version <= 17 ? offset("InstanceKlass", "_fields") : 0;
    InstanceKlass::fieldsInfoStream = version >= 21 ? offset("InstanceKlass", "_fieldinfo_stream") : 0;
    InstanceKlass::fieldsCount = version <= 17 ? offset("InstanceKlass", "_java_fields_count") : 0;
    InstanceKlass::javaMirror = offset("Klass", "_java_mirror");
    InstanceKlass::superKlass = offset("Klass", "_super");
    InstanceKlass::subKlass = offset("Klass", "_subklass");
    InstanceKlass::localInterfaces = offset("InstanceKlass", "_local_interfaces");
    InstanceKlass::osrNmethodsHead = offset("InstanceKlass", "_osr_nmethods_head");

    Method::code = offset("Method", "_code");
    Method::i2iEntry = offset("Method", "_i2i_entry");
    Method::fromCompiledEntry = offset("Method", "_from_compiled_entry");
    Method::fromInterpretedEntry = offset("Method", "_from_interpreted_entry");
    Method::constMethod = offset("Method", "_constMethod");
    // _adapter follows _method_counters in the shared HotSpot Method layout.
    Method::adapter = offset("Method", "_method_counters") + sizeof(void*);
    AdapterHandlerEntry::c2iEntry = version > 17 ? 2 * sizeof(void*) : 4 * sizeof(void*);

    ConstMethod::nameIndex = offset("ConstMethod", "_name_index");
    ConstMethod::signatureIndex = offset("ConstMethod", "_signature_index");
    ConstMethod::codeSize = offset("ConstMethod", "_code_size");
    ConstMethod::constants = offset("ConstMethod", "_constants");
    ConstMethod::constMethodSize = HotspotVM::findType("ConstMethod").value().get().size;

    ClassLoaderData::head = address("ClassLoaderDataGraph", "_head");
    ClassLoaderData::klasses = offset("ClassLoaderData", "_klasses");
    ClassLoaderData::next = offset("ClassLoaderData", "_next");

    nmethod::verifiedEntryPoint = offset("nmethod", "_verified_entry_point");
    nmethod::osrEntryPoint = offset("nmethod", "_osr_entry_point");
    nmethod::entryPoint = offset("nmethod", "_entry_point");
    nmethod::entryBCI = offset("nmethod", "_entry_bci");
    nmethod::osrLink = offset("nmethod", "_osr_link");
    nmethod::state = offset("nmethod", "_state");
    nmethod::compileID = offset("nmethod", "_compile_id");
    nmethod::compileLVL = offset("nmethod", "_comp_level");

    CodeBlob::codeBegin = offset("CodeBlob", "_code_begin");
    CodeBlob::codeEnd = offset("CodeBlob", "_code_end");
    SharedRuntime::wrongMethodBlob = address("SharedRuntime", "_wrong_method_blob");

    CompressedOops::narrowOopBase = compressedAddress("CompressedOops", "_base", "_narrow_oop._base");
    CompressedOops::narrowOopShift = compressedAddress("CompressedOops", "_shift", "_narrow_oop._shift");
    CompressedKlassPointers::narrowKlassBase = compressedAddress("CompressedKlassPointers", "_base", "_narrow_klass._base");
    CompressedKlassPointers::narrowKlassShift = compressedAddress("CompressedKlassPointers", "_shift", "_narrow_klass._shift");

    ConstantPool::length = offset("ConstantPool", "_length");
    ConstantPool::poolHolder = offset("ConstantPool", "_pool_holder");
    ConstantPool::constPoolSize = HotspotVM::findType("ConstantPool").value().get().size;
    Symbol::length = offset("Symbol", "_length");
    Symbol::body = offset("Symbol", "_body");
}
