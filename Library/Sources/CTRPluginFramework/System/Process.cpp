#include <Headers.hpp>

extern Handle gspThreadEventHandle;

namespace CTRPluginFramework {
    Process::ExceptionCallback Process::exceptionCallback = nullptr;
    Process::OnPauseResumeCallback Process::OnPauseResume = nullptr;
    bool Process::ThrowOldExceptionOnCallbackException = false;

    Handle Process::GetHandle(void) {
        return ProcessImpl::ProcessHandle;
    }

    u32 Process::GetProcessID(void) {
        return ProcessImpl::ProcessId;
    }

    u64 Process::GetTitleID(void) {
        return ProcessImpl::TitleId;
    }

    void Process::GetTitleID(string &output) {
        char tid[17] = {0};
        sprintf(tid, "%016llX", ProcessImpl::TitleId);

        for (int i = 0; i < 16;)
            output += tid[i++];
    }

    void Process::GetName(string &output) {
        for (int i = 0; i < 8;) {
            char c = ProcessImpl::CodeSet.processName[i++];

            if (c)
                output += c;
        }
    }

    u16 Process::GetVersion(void) {
        AM_TitleEntry entry = {0};
        AM_TitleEntry entryUpd = {0};
        AM_TitleEntry entryCard = {0};

        u64 tid = Process::GetTitleID();
        u64 tidupdate = tid | 0x000000E00000000;

        AM_GetTitleInfo(MEDIATYPE_SD, 1, &tid, &entry);
        AM_GetTitleInfo(MEDIATYPE_SD, 1, &tidupdate, &entryUpd);

        if (R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_GAME_CARD, 1, &tid, &entryCard)))
            return (max(entryUpd.version, entryCard.version));

        return max(entryUpd.version, entry.version);
    }

    u32 Process::GetTextSize(void) {
        return ProcessImpl::CodeSet.textPages * 0x1000;
    }

    u32 Process::GetRoDataSize(void) {
        return ProcessImpl::CodeSet.rodataPages * 0x1000;
    }

    u32 Process::GetRwDataSize(void) {
        return ProcessImpl::CodeSet.rwPages * 0x1000;
    }

    bool Process::IsPaused(void) {
        return ProcessImpl::IsPaused > 0;
    }

    void Process::Pause() {
        ProcessImpl::Pause(false);
    }

    void Process::Play(const u32 frames) {
        if (frames)
            OSDImpl::ResumeFrame(frames);

        else ProcessImpl::Play(false);
    }

    void Process::WaitForExit() {
        ProcessImpl::WaitForExit();
    }

    bool Process::Patch(u32 addr, void *patch, u32 length, void *original) {
        return ProcessImpl::PatchProcess(addr, static_cast<u8*>(patch), length, static_cast<u8*>(original));
    }

    bool Process::Patch(u32 addr, u32 patch, void *original) {
        return ProcessImpl::PatchProcess(addr, reinterpret_cast<u8*>(&patch), 4, static_cast<u8*>(original));
    }

    bool Process::ProtectMemory(u32 addr, u32 size, int perm) {
        if (addr & 0xFFF) {
            addr &= ~0xFFF;
            size += 0x1000;
            size &= ~0xFFF;
        }

        svcControlProcess(ProcessImpl::ProcessHandle, PROCESSOP_SET_MMU_TO_RWX, 0, 0);
        return true;
    }

    bool Process::ProtectRegion(u32 addr, int perm) {
        MemInfo minfo;
        PageInfo pinfo;

        if (R_FAILED(svcQueryProcessMemory(&minfo, &pinfo, ProcessImpl::ProcessHandle, addr)))
            goto error;

        if (minfo.state == MEMSTATE_FREE)
            goto error;

        if (addr < minfo.base_addr || addr > minfo.base_addr + minfo.size)
            goto error;

        return ProtectMemory(minfo.base_addr, minfo.size, perm);
        error:
        return false;
    }

    void Process::ProtectRegionInRange(u32 startAddress, u32 endAddress, int perm) {}

    bool Process::CopyMemory(void *dst, const void *src, u32 size) {
        u32 src32 = (u32)src;
        u32 dst32 = (u32)dst;

        if (!CheckAddress(src32))
            goto error;

        if (!CheckAddress(dst32))
            goto error;

        if (!CheckAddress(dst32 + size))
            goto error;

        if (!CheckAddress(src32 + size))
            goto error;

        svcFlushProcessDataCache(ProcessImpl::ProcessHandle, src32, size);
        svcInvalidateProcessDataCache(ProcessImpl::ProcessHandle, dst32, size);
        memcpy(dst, src, size);
        svcFlushProcessDataCache(ProcessImpl::ProcessHandle, dst32, size);

        return true;
        error:
        return false;
    }

    bool Process::CheckAddress(u32 address, u32 perm) {
        return ProcessImpl::IsValidAddress(address);
    }

    bool Process::CheckRegion(u32 address, u32 &size, u32 perm) {
        MemInfo mInfo = ProcessImpl::GetMemRegion(address);
        size = mInfo.size;
        return mInfo != ProcessImpl::InvalidRegion;
    }

    u32 Process::GetFreeMemoryRegion(u32 size, u32 startAddress) {
        return ProcessImpl::GetFreeMemRegion(size, startAddress);
    }

    static u8 data8; static u16 data16; static u32 data32;

    bool Process::Write64(u32 address, u64 value) {
        if (CheckAddress(address, MEMPERM_WRITE)) {
            *reinterpret_cast<u64*>(address) = value;
            return (true);
        }

        return (false);
    }

    bool Process::Write32(u32 address, u32 value) {
        if (CheckAddress(address, MEMPERM_WRITE)) {
            *reinterpret_cast<u32*>(address) = value;
            return (true);
        }

        return (false);
    }


    void Process::Write32(u32 pointer, u32 offset, u32 value) {
        u32 address;

        if (Process::Read32(pointer, address)) {
            if (Process::Read32(address + offset, data32) && data32 != value)
                Process::Write32(address + offset, value);
        }
    }

    void Process::Write32(u32 pointer, u32 offset, u32 value, int length) {
        u32 address;

        for (int i = 0; i < length; i++) {
            if (Process::Read32(pointer + (i * 4), address)) {
                if (Process::Read32(address + offset, data32) && data32 != value)
                    Process::Write32(address + offset, value);
            }
        }
    }


    bool Process::Write32(u32 address, vector<u32> value) {
        for (unsigned int i = 0; i < value.size(); i++) {
            if (Process::Read32(address + (i * 4), data32) && data32 != value[i]) {
                if (!Process::Write32(address + (i * 4), value[i]))
                    return false;
            }

            else return false;
        }

        return true;
    }

    void Process::Write32(u32 address, vector<u32> value, int length) {
        for (int i = 0; i < (length / 4); i++) {
            if (Process::Read32(address + (i * 4), data32) && data32 != value[i])
                Process::Write32(address + (i * 4), value[i]);
        }
    }

    void Process::Write32(vector<u32> address, vector<u32> value) {
        for (unsigned int i = 0; i < address.size(); i++) {
            if (Process::Read32(address[i], data32) && data32 != value[i])
                Process::Write32(address[i], value[i]);
        }
    }

    bool Process::Write32(u32 address, u32 value, u32 &original, MenuEntry *entry, bool &saved) {
        if (entry->IsActivated()) {
            if (!saved) {
                original = *(u32*)address;
                saved = true;
                return true;
            }

            if (Process::Read32(address, data32) && data32 != value)
                Process::Write32(address, value);

            return true;
        }

        if (!entry->IsActivated()) {
            if (saved) {
                if (Process::Read32(address, data32) && data32 != original)
                    Process::Write32(address, original);

                original = 0;
                saved = false;
                return true;
            }
        }

        return false;
    }

    bool Process::Write32(u32 address, vector<u32> value, vector<u32> &original, MenuEntry *entry, bool &saved) {
        if (entry->IsActivated()) {
            if (!saved) {
                for (unsigned int i = 0; i < value.size(); i++)
                    original[i] = *(u32*)(address + (i * 4));

                saved = true;
                return true;
            }

            for (unsigned int j = 0; j < value.size(); j++) {
                if (Process::Read32(address + (j * 4), data32) && data32 != value[j])
                    Process::Write32(address + (j * 4), value[j]);
            }

            return true;
        }

        if (!entry->IsActivated()) {
            if (saved) {
                Process::Write32(address, original);
                fill(original.begin(), original.end(), 0);
                saved = false;
                return true;
            }
        }

        return false;
    }

    bool Process::Write32(vector<u32> address, vector<u32> value, vector<u32> &original, MenuEntry *entry, bool &saved) {
        if (entry->IsActivated()) {
            if (!saved) {
                for (unsigned int i = 0; i < value.size(); i++)
                    original[i] = *(u32*)address[i];

                saved = true;
                return true;
            }

            for (unsigned int j = 0; j < address.size(); j++) {
                if (Process::Read32(address[j], data32) && data32 != value[j])
                    Process::Write32(address[j], value[j]);
            }

            return true;
        }

        if (!entry->IsActivated()) {
            if (saved) {
                Process::Write32(address, original);
                fill(original.begin(), original.end(), 0);
                saved = false;
                return true;
            }
        }

        return false;
    }

    bool Process::Write16(u32 address, u16 value) {
        if (CheckAddress(address, MEMPERM_WRITE)) {
            *reinterpret_cast<u16*>(address) = value;
            return (true);
        }

        return (false);
    }

    void Process::Write16(u32 pointer, u32 offset, u16 value) {
        u32 address;

        if (Process::Read32(pointer, address)) {
            if (Process::Read16(address + offset, data16) && data16 != value)
                Process::Write16(address + offset, value);
        }
    }

    void Process::Write16(u32 pointer, u32 offset, u16 value, int length) {
        u32 address;

        for (int i = 0; i < length; i++) {
            if (Process::Read32(pointer + (i * 4), address)) {
                if (Process::Read16(address + offset, data16) && data16 != value)
                    Process::Write16(address + offset, value);
            }
        }
    }

    bool Process::Write8(u32 address, u8 value) {
        if (CheckAddress(address, MEMPERM_WRITE)) {
            *reinterpret_cast<u8*>(address) = value;
            return (true);
        }

        return (false);
    }

    void Process::Write8(u32 pointer, u32 offset, u8 value) {
        u32 address;

        if (Process::Read32(pointer, address)) {
            if (Process::Read8(address + offset, data8) && data8 != value)
                Process::Write8(address + offset, value);
        }
    }

    void Process::Write8(u32 pointer, u32 offset, u8 value, int length) {
        u32 address;

        for (int i = 0; i < length; i++) {
            if (Process::Read32(pointer + (i * 4), address)) {
                if (Process::Read8(address + offset, data8) && data8 != value)
                    Process::Write8(address + offset, value);
            }
        }
    }

    bool Process::WriteFloat(u32 address, float value) {
        if (CheckAddress(address, MEMPERM_WRITE)) {
            *reinterpret_cast<float*>(address) = value;
            return (true);
        }

        return (false);
    }

    bool Process::WriteDouble(u32 address, double value) {
        if (CheckAddress(address, MEMPERM_WRITE)) {
            *reinterpret_cast<double*>(address) = value;
            return (true);
        }

        return (false);
    }

    bool Process::Read64(u32 address, u64 &value) {
        if (CheckAddress(address, MEMPERM_READ)) {
            value = *reinterpret_cast<u64*>(address);
            return (true);
        }

        return (false);
    }

    bool Process::Read32(u32 address, u32 &value) {
        if (CheckAddress(address, MEMPERM_READ)) {
            value = *reinterpret_cast<vu32*>(address);
            return (true);
        }

        return (false);
    }

    u32 Process::Read32(u32 address) {
        return *(volatile unsigned int*)(address);
    }

    vector<u32> Process::Read32(u32 address, int length) {
        vector<u32> vals(length);

        for (int i = 0; i < length; i++) {
            vals[i] = *(u32*)address;
            address += 0x4;
        }

        return vals;
    }

    bool Process::Read16(u32 address, u16 &value) {
        if (CheckAddress(address, MEMPERM_READ)) {
            value = *reinterpret_cast<vu16*>(address);
            return (true);
        }

        return (false);
    }

    u16 Process::Read16(u32 address) {
        return *(volatile unsigned short*)(address);
    }

    u16 Process::Read16(u32 pointer, u32 offset) {
        u16 data;
        Process::Read16(pointer, data);
        return *(volatile unsigned short*)(data + offset);
    }

    bool Process::Read8(u32 address, u8 &value) {
        if (CheckAddress(address, MEMPERM_READ)) {
            value = *reinterpret_cast<vu8*>(address);
            return (true);
        }

        return (false);
    }

    u8 Process::Read8(u32 address) {
        return *(volatile unsigned char*)(address);
    }

    bool Process::Read8(u32 address, u8 &value, bool check) {
        if (check) {
            if (!System::IsConnectedToInternet()) {
                if (Process::Read8(address, data8)) {
                    value = data8;
                    return true;
                }
            }
        }

        return false;
    }

    bool Process::ReadFloat(u32 address, float &value) {
        if (CheckAddress(address, MEMPERM_READ)) {
            value = *reinterpret_cast<volatile float*>(address);
            return (true);
        }

        return (false);
    }

    bool Process::ReadDouble(u32 address, double &value) {
        if (CheckAddress(address, MEMPERM_READ)) {
            value = *reinterpret_cast<volatile double*>(address);
            return (true);
        }

        return (false);
    }

    static bool ConvertString(void *output, const u8 *input, u32 size, StringFormat outfmt) {
        if (outfmt == StringFormat::Utf16) {
            u16 buffer[0x10];
            u16 *out = reinterpret_cast<u16*>(output);
            u16 *buf;
            u32 code;
            int units;

            size >>= 1;
            size <<= 1;

            do {
                buf = buffer;
                units = decode_utf8(&code, input);

                if (units == -1)
                    return (false);

                input += units;
                units = encode_utf16(buf, code);
                size -= units;

                if (!size)
                    *out = 0;

                else {
                    while (units--)
                        *out++ = *buf++;
                }

            }

            while (size && code > 0);
        }

        else {
            u32 *out = reinterpret_cast<u32*>(output);
            u32 code;
            int units;

            size >>= 2;
            size <<= 2;

            do {
                units = decode_utf8(&code, input);

                if (units == -1)
                    return (false);

                input += units;
                size -= 4;

                if (!size)
                    *out = 0;

                else {
                    while (units--)
                        *out++ = code;
                }

            }

            while (size && code > 0);
        }

        return (true);
    }

    bool Process::ReadString(u32 address, string &output, u32 size, StringFormat format) {
        if (!CheckAddress(address, MEMPERM_READ))
            return (false);

        u8 buffer[0x10];

        if (format == StringFormat::Utf8) {
            u32 code = 0;
            u8 *p = reinterpret_cast<u8*>(address);
            int unit = 0;

            do {
                unit = decode_utf8(&code, p);

                if (unit == -1)
                    return (false);

                size -= unit;

                if (code > 0) {
                    while (unit--)
                        output += *p++;
                }
            }

            while (code > 0 && size);
        }

        else if (format == StringFormat::Utf16) {
            u32 code = 0;
            u16 *p = reinterpret_cast<u16*>(address);
            u8 *buf;
            int unit = 0;

            size >>= 1;
            size <<= 1;

            do {
                buf = buffer;
                unit = decode_utf16(&code, p);

                if (unit == -1)
                    return (false);

                p += unit;
                size -= unit * 2;
                unit = encode_utf8(buf, code);

                if (unit == -1)
                    return (false);

                if (code > 0) {
                    while (unit--)
                        output += *buf++;
                }
            }

            while (code > 0 && size);
        }

        else {
            u32 code = 0;
            u32 *p = reinterpret_cast<u32*>(address);
            u8 *buf;
            int unit = 0;

            size >>= 2;
            size <<= 2;

            do {
                buf = buffer;
                code = *p++;
                size -= 4;
                unit = encode_utf8(buf, code);

                if (unit == -1)
                    return (false);

                if (code > 0) {
                    while (unit--)
                        output += *buf++;
                }
            }

            while (code > 0 && size);
        }

        return (true);
    }

    bool Process::WriteString(u32 address, const string &input, StringFormat outFmt) {
        if (!CheckAddress(address, MEMPERM_READ | MEMPERM_WRITE) || input.empty())
            return (false);

        if (outFmt == StringFormat::Utf8) {
            u8 *p = reinterpret_cast<u8*>(address);

            for (char c : input)
                *p++ = c;

            return (true);
        }

        else if (outFmt == StringFormat::Utf16) {
            u32 size = (input.size() + 1) * 2;
            const u8 *in = reinterpret_cast<const u8*>(input.c_str());
            u16 *out = reinterpret_cast<u16*>(address);
            return (ConvertString(out, in, size, outFmt));
        }

        {
            u32 size = (input.size() + 1) * 4;
            const u8 *in = reinterpret_cast<const u8*>(input.c_str());
            u32 *out = reinterpret_cast<u32*>(address);
            return (ConvertString(out, in, size, outFmt));
        }
    }

    bool Process::WriteString(u32 address, const string &input, u32 size, StringFormat outFmt) {
        if (!CheckAddress(address, MEMPERM_READ | MEMPERM_WRITE) || input.empty())
            return (false);

        if (outFmt == StringFormat::Utf8) {
            u8 *p = reinterpret_cast<u8*>(address);

            for (char c : input) {
                size--;

                if (!size) {
                    *p = 0;
                    break;
                }

                *p++ = c;
            }

            return (true);
        }

        else if (outFmt == StringFormat::Utf16) {
            const u8 *in = reinterpret_cast<const u8*>(input.c_str());
            u16 *out = reinterpret_cast<u16*>(address);
            return (ConvertString(out, in, size, outFmt));
        }

        {
            const u8 *in = reinterpret_cast<const u8*>(input.c_str());
            u32 *out = reinterpret_cast<u32*>(address);
            return (ConvertString(out, in, size, outFmt));
        }
    }

    void Process::ReturnToHomeMenu(void) {
        APT_PrepareToCloseApplication(true);
        APT_CloseApplication(NULL, 0, 0);
        ProcessImpl::UnlockGameThreads();
        svcExitProcess();
        for (;;);
    }
}