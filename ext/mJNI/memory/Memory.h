//
// Created by natty on 14.07.2026.
//

#pragma once

#include <cstdint>
#include "../utils/Utils.h"

struct process_data {
    HANDLE jvm_handle = nullptr;
    void* jvm_dll_address = nullptr;
    DWORD jvm_pid = 0;
    HWND jvm_hwnd = nullptr;
};

extern process_data global_process_data;

BOOL ReadMem(HANDLE handle, void *buffer, const void *address, size_t size, size_t* read);
BOOL WriteMem(HANDLE handle, void *buffer,  void *address, size_t size, size_t* bRead);

BOOL mReadMemory(void*, const void* , size_t );
BOOL mWriteMemory(void*, void* , size_t );

BOOL QueryInformationProcess(HANDLE handle, ULONG u1, PVOID p1, ULONG p2, PULONG p3);

void* readPointer(void* address);

uintptr_t ScanSignature(HANDLE hProcess, uintptr_t module, const std::string& pattern);
uintptr_t JVMFindPattern(const std::string &);

void mAttachToJvm(HANDLE);

