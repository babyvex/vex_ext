//
// Created by natty on 14.07.2026.
//

#include "Utils.h"

#include <memory>

#include "../memory/Memory.h"

#include <tlhelp32.h>
#include <psapi.h>
#include <vector>
#include <winternl.h>


int mJNI::Utils::getPidFromProcessName(const char *processName) {
    DWORD pid = 0;
    PROCESSENTRY32 entry;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    entry.dwSize = sizeof(PROCESSENTRY32);
    BOOL result = Process32First(hSnapshot, &entry);

    while (result) {
        if (strcmp(processName, entry.szExeFile) == 0) {
            pid = entry.th32ProcessID;
            break;
        }
        result = Process32Next(hSnapshot, &entry);
    }

    CloseHandle(hSnapshot);
    return pid;
}


void* mJNI::Utils::getProcessExport(void *process, const char* exportName) {
    std::vector<HMODULE> mods(128);
    DWORD count = 0;
    for (;;) {
        const auto capacity = static_cast<DWORD>(mods.size() * sizeof(HMODULE));
        if (!EnumProcessModulesEx(process, mods.data(), capacity, &count, LIST_MODULES_64BIT)) {
            return nullptr;
        }
        if (count <= capacity) break;
        mods.resize(count / sizeof(HMODULE) + 1);
    }

    for (size_t i = 0; i < count / sizeof(HMODULE); ++i) {
        char name[MAX_PATH]{};
        if (GetModuleBaseNameA(process, mods[i], name, MAX_PATH) && _stricmp(name, exportName) == 0) {
            return mods[i];
        }
    }

    return 0;
}

namespace {
    bool readModuleHeaders(HANDLE process, const void* base, IMAGE_NT_HEADERS64& headers) {
        IMAGE_DOS_HEADER dos{};
        if (!base || !ReadMem(process, &dos, base, sizeof(dos), nullptr) ||
            dos.e_magic != IMAGE_DOS_SIGNATURE || dos.e_lfanew < static_cast<LONG>(sizeof(dos))) return false;

        const auto address = reinterpret_cast<uintptr_t>(base) + dos.e_lfanew;
        if (address < reinterpret_cast<uintptr_t>(base) ||
            !ReadMem(process, &headers, reinterpret_cast<void*>(address), sizeof(headers), nullptr)) return false;

        return headers.Signature == IMAGE_NT_SIGNATURE &&
            headers.FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER64) &&
            headers.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    }
}

mJNI::Architecture mJNI::Utils::getModuleArchitecture(HANDLE process, const void* base) {
    IMAGE_NT_HEADERS64 headers{};
    if (!readModuleHeaders(process, base, headers)) return Architecture::unknown;
    return architectureFromMachine(headers.FileHeader.Machine);
}

std::vector<ExportFunction> mJNI::Utils::copyExports(void * base) {

    std::vector<ExportFunction> exports;

    IMAGE_NT_HEADERS64 ntHeader{};
    if (!readModuleHeaders(global_process_data.jvm_handle, base, ntHeader) ||
        architectureFromMachine(ntHeader.FileHeader.Machine) == Architecture::unknown ||
        ntHeader.OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT) return exports;

    IMAGE_DATA_DIRECTORY directory = ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (directory.Size == 0) return exports;

    IMAGE_EXPORT_DIRECTORY exportDirectory;
    if (!mReadMemory(&exportDirectory, (void*)((uintptr_t)base + directory.VirtualAddress), sizeof(exportDirectory))) return exports; // gets virtual address

    std::unique_ptr<DWORD[]> names = std::make_unique<DWORD[]>(exportDirectory.NumberOfNames);
    std::unique_ptr<unsigned short[]> ordinals = std::make_unique<unsigned short[]>(exportDirectory.NumberOfNames);
    std::unique_ptr<DWORD[]> func = std::make_unique<DWORD[]>(exportDirectory.NumberOfFunctions);

    if (!mReadMemory(names.get(), (void*)(uintptr_t(base) + exportDirectory.AddressOfNames), exportDirectory.NumberOfNames * sizeof(DWORD))) return exports;
    if (!mReadMemory(ordinals.get(), (void*)(uintptr_t(base) + exportDirectory.AddressOfNameOrdinals), exportDirectory.NumberOfNames * sizeof(unsigned short))) return exports;
    if (!mReadMemory(func.get(), (void*)(uintptr_t(base) + exportDirectory.AddressOfFunctions), exportDirectory.NumberOfFunctions * sizeof(DWORD))) return exports;

    for (DWORD i = 0; i < exportDirectory.NumberOfNames; ++i) {
        char eName[256] = {0};
        uintptr_t pName = (reinterpret_cast<uintptr_t>(base) + names[i]);
        if (mReadMemory(eName, (void*)pName, sizeof(eName))) {

            auto ordinal = ordinals[i];
            if (ordinal >= exportDirectory.NumberOfFunctions) continue;
            auto rva = func[ordinal];
            eName[sizeof(eName) - 1] = '\0';
            exports.push_back({ ordinal + exportDirectory.Base, rva, std::string(eName) });
        }
    }
    return exports;
}

std::string mJNI::Utils::getCommandLineByHandle(HANDLE handle) {
    PROCESS_BASIC_INFORMATION pbi{};
    ULONG len = 0;
    if (!QueryInformationProcess(handle, 0, &pbi, sizeof(pbi), &len)) return {};
    if (!pbi.PebBaseAddress) return {};

    PEB peb{};
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(handle, pbi.PebBaseAddress, &peb, sizeof(peb), &bytesRead)) return {};

    RTL_USER_PROCESS_PARAMETERS procParams{};
    if (!ReadProcessMemory(handle, peb.ProcessParameters, &procParams, sizeof(procParams), &bytesRead)) return {};

    if (procParams.CommandLine.Length == 0 || !procParams.CommandLine.Buffer) return {};

    std::vector<wchar_t> wbuf(procParams.CommandLine.Length / sizeof(wchar_t) + 1, 0);
    if (!ReadProcessMemory(handle, procParams.CommandLine.Buffer, wbuf.data(), procParams.CommandLine.Length, &bytesRead)) return {};

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), static_cast<int>(procParams.CommandLine.Length / sizeof(wchar_t)), nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) return {};

    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), static_cast<int>(procParams.CommandLine.Length / sizeof(wchar_t)), &result[0], sizeNeeded, nullptr, nullptr);

    return result;
}

struct WindowSearchData {
    DWORD pid;
    std::vector<HWND> windows;
    bool onlyMainWindow = false;
};




BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* data = reinterpret_cast<WindowSearchData*>(lParam);
    if (!data) return TRUE;

    DWORD windowPid = 0;
    GetWindowThreadProcessId(hwnd, &windowPid);
    if (windowPid != data->pid) return TRUE;

    if (!IsWindowVisible(hwnd)) return TRUE;

    if (data->onlyMainWindow) {
        if (GetParent(hwnd) == NULL && GetWindowTextLengthA(hwnd) > 0) {
            data->windows.push_back(hwnd);
            return FALSE;
        }
        return TRUE;
    }

    data->windows.push_back(hwnd);
    return TRUE;
}

std::vector<HWND> getWindowsByPID(DWORD pid) {
    WindowSearchData data;
    data.pid = pid;
    data.onlyMainWindow = false;

    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&data));
    return data.windows;
}

HWND mJNI::Utils::getWindowByPID(DWORD pid) {
    HWND main = getMainWindowByPID(pid);
    if (main) return main;

    auto windows = getWindowsByPID(pid);

    if (!windows.empty()) {
        return windows[0];
    }

    return nullptr;
}
HWND mJNI::Utils::getMainWindowByPID(DWORD pid) {
    WindowSearchData data;
    data.pid = pid;
    data.onlyMainWindow = true;

    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&data));

    if (!data.windows.empty()) {
        return data.windows[0];
    }
    return nullptr;
}
