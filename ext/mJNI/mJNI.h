//
// Created by natty on 14.07.2026.
//

#pragma once
#include "jvm/Structs.hpp"

#include <vector>

#include "jvm/Offsets.h"
#include "memory/Memory.h"

// utility functions
namespace mJNIUtilityFunctions {

    jclass decodeKlass(jclass klass);
    jclass encodeKlass(jclass klass);

    jobject decodeOop(jobject object);
    jobject encodeOop(jobject object);
    size_t objectReferenceSize();
    size_t arrayDataOffset();

    uint32_t readU5( const uint8_t* data, size_t& pos);
    void readFieldInfoStream(FieldInfo20& out, const uint8_t* buffer, size_t& pos);
    int copyFieldInfo(void* address, std::vector<FieldInfo20>& outFields);
    bool writeBytecode(void* address,  void* buffer, size_t size);

    std::string readSymbol(void* address);
    std::string read_symbol_direct(uintptr_t symbol_ptr);

    jobject getClassOOP(jclass klass);


    // utility for method deopt
    void clearCode(jmethodID);
    void makeNoEntrant(jmethodID );
    void removeOSRNmethod(void* poolholder, jmethodID);

    bool patchEntryPoint(void* entry, void* verEntry, void* dst);

    uint32_t get_method_flags(void* m) ;

}

// main  function
namespace mJNI {

    jclass findClass(const char* name);

    jfieldID findFieldID(jclass klass, const char* name, const char* sig);
    jmethodID findMethodID(jclass klass, const char* name, const char* sig);

    jobject getStaticObjectField(jclass klass, jfieldID fieldID);
    void setStaticObjectField(jclass klass, jfieldID fieldID, jobject var);

    jobject getObjectField(jobject object, jfieldID fieldID);
    void setObjectField(jobject object, jfieldID fieldID, jobject var);

    jclass getObjectClass(jobject object);

    jboolean instanceOfKlass(jclass klass1, jclass klass2);
    jboolean instanceOf(jobject object, jclass klass);
    jboolean isSameObject(jobject object1, jobject object2);

    jint getArrayLength(jobject array);

    std::string getClassName(jclass klass);

    void getObjectArrayElement(jobject oop, jobject* array, int start, int end);

    std::vector<uint8_t> getBytecode(jmethodID method);

    void deoptimization(jmethodID method);

    bool retransformMethod(jmethodID method,  std::vector<uint8_t>& newBytecode);

    uint16_t get_klass_cp_index(void* klass, const std::string& targetClassName);

    void set_dont_inline(void* method, bool var) ;

    bool is_dont_inline(void* method) ;

    // defl
    template<typename type>
    type getField(jobject object, jfieldID fieldID) {
        type var{};
        if (!object) return type {};
        mReadMemory(&var, (void*)((uintptr_t)object + (uintptr_t)fieldID), sizeof(var));
        return var;
    }

    template<typename type>
    void setField(jobject object, jfieldID fieldID, type var) {
        if (!object) return;
        mWriteMemory(&var, (void*)((uintptr_t)object + fieldID), sizeof(var));
    }

    // static
    template<typename type>
    type getStaticField(jclass klass, jfieldID fieldID) {
        type var{};
        if (!klass) return type {};
        mReadMemory(&var, (void*)((uintptr_t) mJNIUtilityFunctions::getClassOOP(klass) + fieldID), sizeof(var));
        return var;
    }

    template<typename type>
    void setStaticField(jclass klass, jfieldID fieldID, type var) {
        if (!klass) return;
        mWriteMemory(&var, (void*)((uintptr_t) mJNIUtilityFunctions::getClassOOP(klass) + fieldID), sizeof(var));
    }

    // for arrays
     template<typename type>
     void getArrayElement(jobject object, type* array, int start, int end) {

         const auto base = mJNIUtilityFunctions::arrayDataOffset();
         auto length = getArrayLength(object);

         if (!object || !array || start < 0 || start >= length || end <= 0) {
             return;
         }

         if (end > length - start) {
             end = length - start;
         }

         mReadMemory(array, (void*)((uintptr_t)object + base + start * sizeof(*array)), end * sizeof(*array));
     }


    template<typename type>
    type setArrayElement(jobject object, type* array, int start, int end) {

        const auto base = mJNIUtilityFunctions::arrayDataOffset();
        auto length = getArrayLength(object);

        if (!object || !array || start < 0 || start >= length || end <= 0) {
            return type{};
        }

        if (end > length - start) {
            end = length - start;
        }

        mWriteMemory(array, (void*)((uintptr_t)object + base + start * sizeof(*array)), end * sizeof(*array));
    }

    bool mJNIAttachToJVM(int PID);
    bool mJNIDetach();
}

namespace mJNIHook {

    struct buffer_context {
        uint64_t call_counter = 0;
        uint64_t time_stamp = 0;
        //uint64_t last_arg1= 0 ;
        //uint64_t last_arg2 = 0;
        //uint64_t additional_shellcode_flag = 0;
    };
    struct method_hook {

        void* target_method;

        void* i2i_entry;
        void* from_compiled_entry;
        void* from_interpreter_entry;

        void* buffer_context_address;
        void* shellcode_context_address;
        void* trampoline_address;
    };


    inline bool hook(void* method, method_hook* o_hook)
    {
    method_hook mh{};
    mh.target_method = method;

    void* code = nullptr;
    mReadMemory(&code, (void*)(uintptr_t(method) + Offsets::Method::code), sizeof(code));
    if (code != nullptr) {
        mJNI::deoptimization(method);
    }

    void* i2i_entry              = nullptr;
    void* from_compiled_entry    = nullptr;
    void* from_interpreter_entry = nullptr;
    mReadMemory(&i2i_entry,              (void*)(uintptr_t(method) + Offsets::Method::i2iEntry),              sizeof(i2i_entry));
    mReadMemory(&from_compiled_entry,    (void*)(uintptr_t(method) + Offsets::Method::fromCompiledEntry),    sizeof(from_compiled_entry));
    mReadMemory(&from_interpreter_entry, (void*)(uintptr_t(method) + Offsets::Method::fromInterpretedEntry), sizeof(from_interpreter_entry));

    mh.i2i_entry              = i2i_entry;
    mh.from_compiled_entry    = from_compiled_entry;
    mh.from_interpreter_entry = from_interpreter_entry;

    auto hook_buffer = VirtualAllocEx(global_process_data.jvm_handle, nullptr,
                                      sizeof(buffer_context),
                                      MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!hook_buffer) return false;
    mh.buffer_context_address = hook_buffer;

    buffer_context bc{};
    mWriteMemory(&bc, mh.buffer_context_address, sizeof(buffer_context));

    uint64_t buf_v = (uint64_t)mh.buffer_context_address;

    // i2i
    void* tramp_i2i = VirtualAllocEx(global_process_data.jvm_handle, nullptr, 4096,
                                     MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!tramp_i2i) return false;
    {
        std::vector<uint8_t> sc;
        sc.reserve(128);
        sc.push_back(0x9c);                              // pushfq
        sc.push_back(0x50);                              // push rax
        sc.push_back(0x51);                              // push rcx
        sc.push_back(0x52);                              // push rdx
        sc.push_back(0x41); sc.push_back(0x50);          // push r8
        sc.push_back(0x41); sc.push_back(0x51);          // push r9
        sc.push_back(0x41); sc.push_back(0x52);          // push r10
        sc.push_back(0x41); sc.push_back(0x53);          // push r11
        sc.push_back(0x53);                              // push rbx
        sc.push_back(0x49); sc.push_back(0xba);          // mov r10, buf
        size_t p_buf = sc.size();
        for (int i = 0; i < 8; ++i) sc.push_back(0);
        sc.push_back(0xf0); sc.push_back(0x49); sc.push_back(0xff); sc.push_back(0x02); // lock inc [r10]
        sc.push_back(0x0f); sc.push_back(0x31);          // rdtsc
        sc.push_back(0x48); sc.push_back(0xc1); sc.push_back(0xe2); sc.push_back(0x20); // shl rdx,32
        sc.push_back(0x48); sc.push_back(0x09); sc.push_back(0xd0);                     // or rax,rdx
        sc.push_back(0x49); sc.push_back(0x89); sc.push_back(0x42); sc.push_back(0x08); // mov [r10+8],rax
        sc.push_back(0x5b);                              // pop rbx
        sc.push_back(0x41); sc.push_back(0x5b);          // pop r11
        sc.push_back(0x41); sc.push_back(0x5a);          // pop r10
        sc.push_back(0x41); sc.push_back(0x59);          // pop r9
        sc.push_back(0x41); sc.push_back(0x58);          // pop r8
        sc.push_back(0x5a);                              // pop rdx
        sc.push_back(0x59);                              // pop rcx
        sc.push_back(0x58);                              // pop rax
        sc.push_back(0x9d);                              // popfq
        sc.push_back(0x48); sc.push_back(0xb8);          // mov rax, jmp_target
        size_t p_jmp = sc.size();
        for (int i = 0; i < 8; ++i) sc.push_back(0);
        sc.push_back(0xff); sc.push_back(0xe0);          // jmp rax

        uint64_t jmp_v = (uint64_t)i2i_entry;
        for (int i = 0; i < 8; ++i) {
            sc[p_buf + i] = (uint8_t)(buf_v >> (8 * i));
            sc[p_jmp + i] = (uint8_t)(jmp_v >> (8 * i));
        }
        mWriteMemory(sc.data(), tramp_i2i, sc.size());
        DWORD op = 0;
        VirtualProtectEx(global_process_data.jvm_handle, tramp_i2i, 4096, PAGE_EXECUTE_READ, &op);
    }

    // from_compiled
    void* tramp_cmp = VirtualAllocEx(global_process_data.jvm_handle, nullptr, 4096,
                                     MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!tramp_cmp) return false;
    {
        std::vector<uint8_t> sc;
        sc.reserve(128);
        sc.push_back(0x9c);
        sc.push_back(0x50);
        sc.push_back(0x51);
        sc.push_back(0x52);
        sc.push_back(0x41); sc.push_back(0x50);
        sc.push_back(0x41); sc.push_back(0x51);
        sc.push_back(0x41); sc.push_back(0x52);
        sc.push_back(0x41); sc.push_back(0x53);
        sc.push_back(0x53);
        sc.push_back(0x49); sc.push_back(0xba);
        size_t p_buf = sc.size();
        for (int i = 0; i < 8; ++i) sc.push_back(0);
        sc.push_back(0xf0); sc.push_back(0x49); sc.push_back(0xff); sc.push_back(0x02);
        sc.push_back(0x0f); sc.push_back(0x31);
        sc.push_back(0x48); sc.push_back(0xc1); sc.push_back(0xe2); sc.push_back(0x20);
        sc.push_back(0x48); sc.push_back(0x09); sc.push_back(0xd0);
        sc.push_back(0x49); sc.push_back(0x89); sc.push_back(0x42); sc.push_back(0x08);
        sc.push_back(0x5b);
        sc.push_back(0x41); sc.push_back(0x5b);
        sc.push_back(0x41); sc.push_back(0x5a);
        sc.push_back(0x41); sc.push_back(0x59);
        sc.push_back(0x41); sc.push_back(0x58);
        sc.push_back(0x5a);
        sc.push_back(0x59);
        sc.push_back(0x58);
        sc.push_back(0x9d);
        sc.push_back(0x48); sc.push_back(0xb8);
        size_t p_jmp = sc.size();
        for (int i = 0; i < 8; ++i) sc.push_back(0);
        sc.push_back(0xff); sc.push_back(0xe0);

        uint64_t jmp_v = (uint64_t)from_compiled_entry;
        for (int i = 0; i < 8; ++i) {
            sc[p_buf + i] = (uint8_t)(buf_v >> (8 * i));
            sc[p_jmp + i] = (uint8_t)(jmp_v >> (8 * i));
        }
        mWriteMemory(sc.data(), tramp_cmp, sc.size());
        DWORD op = 0;
        VirtualProtectEx(global_process_data.jvm_handle, tramp_cmp, 4096, PAGE_EXECUTE_READ, &op);
    }

    //  from_interpreted
    void* tramp_int = VirtualAllocEx(global_process_data.jvm_handle, nullptr, 4096,
                                     MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!tramp_int) return false;
    {
        std::vector<uint8_t> sc;
        sc.reserve(128);
        sc.push_back(0x9c);
        sc.push_back(0x50);
        sc.push_back(0x51);
        sc.push_back(0x52);
        sc.push_back(0x41); sc.push_back(0x50);
        sc.push_back(0x41); sc.push_back(0x51);
        sc.push_back(0x41); sc.push_back(0x52);
        sc.push_back(0x41); sc.push_back(0x53);
        sc.push_back(0x53);
        sc.push_back(0x49); sc.push_back(0xba);
        size_t p_buf = sc.size();
        for (int i = 0; i < 8; ++i) sc.push_back(0);
        sc.push_back(0xf0); sc.push_back(0x49); sc.push_back(0xff); sc.push_back(0x02);
        sc.push_back(0x0f); sc.push_back(0x31);
        sc.push_back(0x48); sc.push_back(0xc1); sc.push_back(0xe2); sc.push_back(0x20);
        sc.push_back(0x48); sc.push_back(0x09); sc.push_back(0xd0);
        sc.push_back(0x49); sc.push_back(0x89); sc.push_back(0x42); sc.push_back(0x08);
        sc.push_back(0x5b);
        sc.push_back(0x41); sc.push_back(0x5b);
        sc.push_back(0x41); sc.push_back(0x5a);
        sc.push_back(0x41); sc.push_back(0x59);
        sc.push_back(0x41); sc.push_back(0x58);
        sc.push_back(0x5a);
        sc.push_back(0x59);
        sc.push_back(0x58);
        sc.push_back(0x9d);
        sc.push_back(0x48); sc.push_back(0xb8);
        size_t p_jmp = sc.size();
        for (int i = 0; i < 8; ++i) sc.push_back(0);
        sc.push_back(0xff); sc.push_back(0xe0);

        uint64_t jmp_v = (uint64_t)from_interpreter_entry;
        for (int i = 0; i < 8; ++i) {
            sc[p_buf + i] = (uint8_t)(buf_v >> (8 * i));
            sc[p_jmp + i] = (uint8_t)(jmp_v >> (8 * i));
        }
        mWriteMemory(sc.data(), tramp_int, sc.size());
        DWORD op = 0;
        VirtualProtectEx(global_process_data.jvm_handle, tramp_int, 4096, PAGE_EXECUTE_READ, &op);
    }

    mh.trampoline_address = tramp_i2i;

    struct PatchTarget { uintptr_t addr; void* value; };
    PatchTarget patches[3] = {
        { (uintptr_t)method + Offsets::Method::i2iEntry,               tramp_i2i },
        { (uintptr_t)method + Offsets::Method::fromCompiledEntry,     tramp_cmp },
        { (uintptr_t)method + Offsets::Method::fromInterpretedEntry,  tramp_int },
    };

    for (int i = 0; i < 3; ++i) {
        DWORD oldProt = 0;
        if (!VirtualProtectEx(global_process_data.jvm_handle,
                              (void*)patches[i].addr, sizeof(void*),
                              PAGE_EXECUTE_READWRITE, &oldProt)) {
            return false;
        }
        if (!mWriteMemory(&patches[i].value, (void*)patches[i].addr, sizeof(void*))) {
            VirtualProtectEx(global_process_data.jvm_handle,
                             (void*)patches[i].addr, sizeof(void*), oldProt, &oldProt);
            return false;
        }
        VirtualProtectEx(global_process_data.jvm_handle,
                         (void*)patches[i].addr, sizeof(void*), oldProt, &oldProt);
    }

    if (o_hook) *o_hook = mh;
    return true;
    }
}
