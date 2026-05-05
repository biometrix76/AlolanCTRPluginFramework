#include <Headers.h>

namespace CTRPluginFramework {
    bool Sudo::Toggle(u32 address, bool enabled) {
        // If running on Citra, no need to mess with memory permissions
        if (System::IsCitra())
            return true;

        u32 process;
        Handle handle;
        MemInfo mem;
        PageInfo page;
        bool out = false;

        if (R_FAILED(svcGetProcessId(&process, CUR_PROCESS_HANDLE)))
            return false;

        if (R_FAILED(svcOpenProcess(&handle, process)))
            return false;

        if (R_FAILED(svcQueryMemory(&mem, &page, address))) {
            svcCloseHandle(handle);
            return false;
        }

        if (enabled) {
            // Only apply if current perms < RWX
            if (mem.perm < 2) {
                if (R_SUCCEEDED(svcControlProcessMemory(
                        handle,
                        mem.base_addr,
                        0,
                        mem.size,
                        MemOp(MEMOP_PROT),
                        MemPerm(MEMPERM_READ | MEMPERM_EXECUTE | MEMPERM_WRITE)
                    ))) {
                    out = true;
                }
            }
        }

        else {
            // Only revert if current perms > Read
            if (mem.perm > 2) {
                if (R_SUCCEEDED(svcControlProcessMemory(
                        handle,
                        mem.base_addr,
                        0,
                        mem.size,
                        MemOp(MEMOP_PROT),
                        MEMPERM_READ
                    ))) {
                    out = true;
                }
            }
        }

        svcCloseHandle(handle);
        return out;
    }

    u32 Sudo::Read32(u32 address) {
        u32 value;

        if (Sudo::Toggle(address, true)) {
            value = *(u32*)address;
            Sudo::Toggle(address, false);
            return value;
        }

        return 0;
    }

    vector<u32> Sudo::Read32(u32 address, int length) {
        vector<u32> vals(length);

        if (Sudo::Toggle(address, true)) {
            for (int i = 0; i < length; i++) {
                vals[i] = *(u32*)address;
                address += 0x4;
            }

            Sudo::Toggle(address, false);
        }

        return vals;
    }

    bool Sudo::Write32(u32 address, u32 value) {
        if (Sudo::Toggle(address, true)) {
            *(u32*)address = value;
            Sudo::Toggle(address, false);
            return true;
        }

        return false;
    }

    bool Sudo::Write32(u32 address, u32 value, u32 &original) {
        if (Sudo::Toggle(address, true)) {
            original = *(u32*)address;
            *(u32*)address = value;
            Sudo::Toggle(address, false);
            return true;
        }

        return false;
    }

    bool Sudo::Write32(u32 address, vector<u32> value) {
        if (Sudo::Toggle(address, true)) {
            for (unsigned int i = 0; i < value.size(); i++)
                *(u32*)(address + i * 4) = value[i];

            Sudo::Toggle(address, false);
            return true;
        }

        return false;
    }
}