//
// Created by natty on 14.07.2026.
//

#include "Memory.h"
#include <winternl.h>

process_data global_process_data;


typedef NTSTATUS(NTAPI* zNtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);

BOOL ReadMem(HANDLE handle, void *buffer, const void *address, size_t size, size_t* bRead) {
    SIZE_T transferred = 0;
    const BOOL result = ReadProcessMemory(handle, address, buffer, size, &transferred);
    if (bRead) *bRead = transferred;
    return result && transferred == size;
}

BOOL WriteMem(HANDLE handle, const void *buffer, void *address, size_t size, size_t* bRead) {
    SIZE_T transferred = 0;
    const BOOL result = WriteProcessMemory(handle, address, buffer, size, &transferred);
    if (bRead) *bRead = transferred;
    return result && transferred == size;
}


BOOL mReadMemory(void *buffer, const void *address, size_t size) {
    return ReadMem(global_process_data.jvm_handle, buffer, address, size, nullptr);
}


BOOL mWriteMemory(void *buffer, void *address, size_t size) {
    DWORD oldProtect = 0;
    if (!VirtualProtectEx(global_process_data.jvm_handle, address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    const BOOL written = WriteMem(global_process_data.jvm_handle, buffer, address, size, nullptr);
    const BOOL restored = VirtualProtectEx(global_process_data.jvm_handle, address, size, oldProtect, &oldProtect);
    return written && restored;
}

BOOL QueryInformationProcess(HANDLE handle, ULONG u1, PVOID p1, ULONG p2, PULONG p3) {
    static const auto NtQueryInformationProcess = [] {
        auto nt = GetModuleHandleA("ntdll.dll");
        return nt ? reinterpret_cast<zNtQueryInformationProcess>(
            GetProcAddress(nt, "NtQueryInformationProcess")) : nullptr;
    }();
    if (!NtQueryInformationProcess) return FALSE;
    NTSTATUS status = NtQueryInformationProcess(
        handle,
        u1,
        p1,p2,p3);
    return (status == 0);
}

void * readPointer(void *address) {
    void* buffer = nullptr;
    mReadMemory(&buffer, address, sizeof(void*));
    return buffer;
}

bool compareBytes(const uint8_t* data, const std::vector<int>& pattern) {
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (pattern[i] != -1 && data[i] != pattern[i]) {
            return false;
        }
    }
    return true;
}

std::vector<int> patternToBytes(const std::string& pattern) {
    std::vector<int> bytes;
    const char* start = pattern.c_str();
    const char* end = start + pattern.length();

    while (start < end) {
        if (*start == '?') {
            ++start;
            if (*start == '?') ++start;
            bytes.push_back(-1);
        }
        else {
            bytes.push_back(strtoul(start, const_cast<char**>(&start), 16));
        }
        while (*start == ' ') ++start;
    }
    return bytes;
}


uintptr_t ScanSignature(HANDLE hProcess, uintptr_t module, const std::string& pattern) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    uintptr_t start = (uintptr_t)sysInfo.lpMinimumApplicationAddress; // 4
    uintptr_t end = (uintptr_t)sysInfo.lpMaximumApplicationAddress; // 5

    std::vector<int> patternBytes = patternToBytes(pattern);
    SIZE_T bytesRead;
    uint8_t buffer[4096];

    while (start < end) {

        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQueryEx(hProcess, (LPCVOID)start, &mbi, sizeof(mbi)) == 0)
            break;
        // if (start < module)
        // {
        //     start += mbi.RegionSize;
        //     continue;
        // }
        if ((mbi.State == MEM_COMMIT) &&
            (mbi.Protect & (PAGE_EXECUTE_READWRITE | PAGE_READWRITE | PAGE_EXECUTE_READ)) &&
            !(mbi.Protect & PAGE_GUARD)) {

            uint8_t* data = new uint8_t[mbi.RegionSize];
            if (ReadMem(hProcess, data,mbi.BaseAddress, mbi.RegionSize, &bytesRead)) {
                for (SIZE_T i = 0; i < (bytesRead - patternBytes.size()); ++i) {
                    if (compareBytes(data + i, patternBytes)) {
                        std::uintptr_t matchAddr = (uintptr_t)mbi.BaseAddress + i;
                        delete[] data;
                        return matchAddr;
                    }
                }
            }
            delete[] data;
            }

        start += mbi.RegionSize;
    }

    return 0;
}
uintptr_t JVMFindPattern(const std::string &pattern) {
    return ScanSignature(global_process_data.jvm_handle, (uintptr_t)global_process_data.jvm_dll_address, pattern);
}

bool mAttachToJvm(HANDLE handle) {
    void* module = mJNI::Utils::getProcessExport(handle, "jvm.dll");
    if (!module) return false;

    const auto architecture = mJNI::Utils::getModuleArchitecture(handle, module);
    if (architecture == mJNI::Architecture::unknown) {
        std::cerr << "Unsupported JVM architecture; use a 64-bit x64 or ARM64 JVM.\n";
        return false;
    }

    global_process_data.jvm_handle = handle;
    global_process_data.jvm_dll_address = module;
    global_process_data.jvm_pid = GetProcessId(handle);
    global_process_data.jvm_hwnd = mJNI::Utils::getWindowByPID(global_process_data.jvm_pid);
    global_process_data.jvm_architecture = architecture;
    return true;
}

