#include <Headers.hpp>
#include "CaughtCore.hpp"
#include "LiveCaught.hpp"

namespace CTRPluginFramework { namespace LiveCaught {
namespace {
    UltraSunCaught::Page page;
    std::vector<u32> candidates;
    u8 caught[UltraSunCaught::CaughtBytes] = {};
    bool ready = false;
    const char *status = "Not checked";
    u16 gameVersion = 0;
    Clock refreshClock;
    bool attempted = false;
    unsigned lastOpenMs = 0;
    bool lastOpenCached = false;
    struct ScanStats {
        u32 queries = 0, processErrors = 0, selfErrors = 0;
        u32 firstProcessError = 0, firstSelfError = 0;
        u32 scannedBytes = 0, signatures = 0;
    };
    struct Region { u32 base, size, permission, state; };
    struct Probe {
        u32 address, check;
        u8 bytes[UltraSunCaught::SnapshotBytes];
    };
    ScanStats stats;
    std::vector<Region> regions;
    std::vector<Probe> probes;

    bool QueryRange(u32 address, MemInfo &info) {
        PageInfo pageInfo;
        std::memset(&info, 0, sizeof(info));
        ++stats.queries;
        // Use the same explicit game-process handle as CTRPF's memory map.
        Result result = svcQueryProcessMemory(&info, &pageInfo, Process::GetHandle(), address);
        if (R_SUCCEEDED(result)) return true;
        if (!stats.processErrors++) stats.firstProcessError = static_cast<u32>(result);
        result = svcQueryMemory(&info, &pageInfo, address);
        if (R_SUCCEEDED(result)) return true;
        if (!stats.selfErrors++) stats.firstSelfError = static_cast<u32>(result);
        return false;
    }

    // Read only a fully mapped, readable range. This feature never changes
    // game memory, memory permissions, caught flags, or the game's save file.
    bool ReadRange(u32 address, void *destination, u32 size) {
        MemInfo info;
        if (!QueryRange(address, info)) return false;
        const u64 end = static_cast<u64>(address) + size;
        const u64 regionEnd = static_cast<u64>(info.base_addr) + info.size;
        if (!(info.perm & MEMPERM_READ) || address < info.base_addr || end > regionEnd)
            return false;
        std::memcpy(destination, reinterpret_cast<const void *>(address), size);
        return true;
    }

    void Scan(u32 start, u32 end) {
        u8 buffer[0x1000];
        u8 snapshot[UltraSunCaught::SnapshotBytes];
        u32 address = start;
        while (address < end && candidates.size() < 16) {
            MemInfo info;
            if (!QueryRange(address, info) || info.size == 0) {
                address += 0x1000;
                continue;
            }
            const u64 queryEnd = static_cast<u64>(info.base_addr) + info.size;
            u32 regionEnd = static_cast<u32>(std::min<u64>(queryEnd, end));
            if (regionEnd <= address) { address += 0x1000; continue; }
            if (!(info.perm & MEMPERM_READ) ||
                info.state == MEMSTATE_FREE || info.state == MEMSTATE_IO ||
                info.state == MEMSTATE_SHARED || info.state == MEMSTATE_RESERVED) {
                address = regionEnd; continue;
            }
            bool recorded = false;
            for (const Region &region : regions) recorded |= region.base == info.base_addr;
            if (!recorded && regions.size() < 96)
                regions.push_back({info.base_addr, info.size, info.perm, info.state});
            while (address < regionEnd && candidates.size() < 16) {
                const u32 size = std::min<u32>(sizeof(buffer), regionEnd - address);
                // QueryRange above already bounds this entire readable region.
                // Avoid a kernel memory-map query for every 4 KiB page.
                {
                    std::memcpy(buffer, reinterpret_cast<const void *>(address), size);
                    stats.scannedBytes += size;
                    for (u32 offset = 0; offset + 4 <= size; offset += 4) {
                        if (UltraSunCaught::Read32(buffer + offset) != UltraSunCaught::DexMagic) continue;
                        ++stats.signatures;
                        const u32 possible = address + offset;
                        if (ReadRange(possible, snapshot, sizeof(snapshot))) {
                            const auto check = UltraSunCaught::CheckDex(snapshot, sizeof(snapshot));
                            bool recordedProbe = false;
                            for (const Probe &probe : probes) recordedProbe |= probe.address == possible;
                            if (!recordedProbe && probes.size() < 16) {
                                Probe probe;
                                probe.address = possible;
                                probe.check = static_cast<u32>(check);
                                std::memcpy(probe.bytes, snapshot, sizeof(snapshot));
                                probes.push_back(probe);
                            }
                            if (check == UltraSunCaught::DexCheck::Valid) {
                                bool duplicate = false;
                                for (u32 candidate : candidates) duplicate |= candidate == possible;
                                if (!duplicate) candidates.push_back(possible);
                                if (candidates.size() >= 16) break;
                            }
                        }
                    }
                }
                address += size;
            }
        }
    }

    void CaptureDiagnostic() {
        // A bounded, read-only diagnostic of the two known Gen 7 save heaps.
        // No game data, memory permissions, or game save files are modified.
        File file("CaughtTracker-memory.bin", File::RWC | File::TRUNCATE);
        if (!file.IsOpen()) return;
        const char magic[8] = {'U','S','R','A','M','0','0','3'};
        const u64 title = Process::GetTitleID();
        const u32 version = gameVersion;
        file.Write(magic, sizeof(magic));
        file.Write(&title, sizeof(title));
        file.Write(&version, sizeof(version));
        for (const Probe &probe : probes) {
            const u32 record[2] = {probe.address, sizeof(probe.bytes)};
            file.Write(record, sizeof(record));
            file.Write(probe.bytes, sizeof(probe.bytes));
        }
        u8 bytes[0x1000];
        const u32 starts[2] = {0x33010000, 0x330D0000};
        const u32 sizes[2] = {0x8000, 0x10000};
        for (unsigned range = 0; range < 2; ++range) {
            for (u32 offset = 0; offset < sizes[range]; offset += sizeof(bytes)) {
                const u32 address = starts[range] + offset;
                if (!ReadRange(address, bytes, sizeof(bytes))) continue;
                const u32 record[2] = {address, sizeof(bytes)};
                file.Write(record, sizeof(record));
                file.Write(bytes, sizeof(bytes));
            }
        }
        const u32 end[2] = {0, 0};
        file.Write(end, sizeof(end));
        file.Flush();
        file.Close();
    }

    void Log() {
        File file("CaughtTracker.log", File::RWC | File::TRUNCATE);
        if (!file.IsOpen()) return;
        char line[256];
        std::snprintf(line, sizeof(line), "Ultra Sun live caught tracker reader test 6\nTitle: %016llX\nGame version: %u\nStatus: %s\nCandidates: %u\nCaught species: %u\n",
            static_cast<unsigned long long>(Process::GetTitleID()), gameVersion, status,
            static_cast<unsigned>(candidates.size()), ready ? UltraSunCaught::Count(caught) : 0);
        file.Write(line, std::strlen(line));
        std::snprintf(line, sizeof(line), "Last guide-open read: %u ms (%s)\n",
            lastOpenMs, lastOpenCached ? "cached address, fresh flags" : "search");
        file.Write(line, std::strlen(line));
        std::snprintf(line, sizeof(line), "Queries: %lu\nProcess-query failures: %lu (first %08lX)\nSelf-query failures: %lu (first %08lX)\nScanned bytes: %lu\nRaw dex signatures: %lu\n",
            static_cast<unsigned long>(stats.queries), static_cast<unsigned long>(stats.processErrors),
            static_cast<unsigned long>(stats.firstProcessError), static_cast<unsigned long>(stats.selfErrors),
            static_cast<unsigned long>(stats.firstSelfError), static_cast<unsigned long>(stats.scannedBytes),
            static_cast<unsigned long>(stats.signatures));
        file.Write(line, std::strlen(line));
        for (const Region &region : regions) {
            std::snprintf(line, sizeof(line), "Region: %08lX size %08lX perm %lu state %lu\n",
                static_cast<unsigned long>(region.base), static_cast<unsigned long>(region.size),
                static_cast<unsigned long>(region.permission), static_cast<unsigned long>(region.state));
            file.Write(line, std::strlen(line));
        }
        for (const Probe &probe : probes) {
            std::snprintf(line, sizeof(line), "Dex probe: %08lX validation %lu caught %u\n",
                static_cast<unsigned long>(probe.address), static_cast<unsigned long>(probe.check),
                UltraSunCaught::Count(probe.bytes + UltraSunCaught::CaughtOffset));
            file.Write(line, std::strlen(line));
        }
        for (u32 address : candidates) {
            std::snprintf(line, sizeof(line), "Candidate: %08lX\n", static_cast<unsigned long>(address));
            file.Write(line, std::strlen(line));
        }
        if (ready) {
            file.WriteLine("Caught species IDs:");
            for (unsigned species = 1; species <= UltraSunCaught::MaxSpecies; ++species) {
                if (!UltraSunCaught::IsCaught(caught, species)) continue;
                std::snprintf(line, sizeof(line), "%u ", species);
                file.Write(line, std::strlen(line));
            }
            file.WriteLine("");
        }
        file.Flush();
        file.Close();
    }

    void Refresh(bool scan, bool log) {
        ready = false;
        std::memset(caught, 0, sizeof(caught));
        if (Process::GetTitleID() != 0x00040000001B5000ULL) {
            status = "Ultra Sun only";
            if (log) Log();
            return;
        }
        if (scan || candidates.empty()) {
            candidates.clear();
            stats = ScanStats();
            regions.clear();
            probes.clear();
            // Search the observed USUM save area first. Scan the wider save
            // heap only if necessary; unrelated heaps caused ten-second stalls.
            Scan(0x33010000, 0x33018000);
            if (candidates.empty()) Scan(0x33000000, 0x33200000);
        }
        if (candidates.empty()) {
            if (!stats.scannedBytes) status = "RAM not readable";
            else if (!stats.signatures) status = "Dex not found";
            else status = "Dex not validated";
        }
        else if (candidates.size() >= 16) status = "Unavailable: too many matches";
        else {
            u8 snapshot[UltraSunCaught::SnapshotBytes];
            bool first = true;
            bool consistent = true;
            for (u32 address : candidates) {
                if (!ReadRange(address, snapshot, sizeof(snapshot)) ||
                    !UltraSunCaught::ValidateDex(snapshot, sizeof(snapshot))) {
                    consistent = false;
                    break;
                }
                const u8 *flags = snapshot + UltraSunCaught::CaughtOffset;
                if (first) { std::memcpy(caught, flags, sizeof(caught)); first = false; }
                else if (std::memcmp(caught, flags, sizeof(caught)) != 0) {
                    consistent = false;
                    break;
                }
            }
            ready = consistent && !first;
            status = ready ? "Ready" : "Unavailable: dex copies differ";
        }
        if (log) {
            if (!ready) CaptureDiagnostic();
            Log();
        }
    }

    void SmallText(const char *text, int x, int y, Color color) {
        Renderer::DrawString(text, x, y, color);
    }
}

void Tick(bool entering) {
    if (entering || !attempted) {
        Clock openClock;
        const bool hadAddress = !candidates.empty();
        // Cache where the dex is, never its caught flags. Revalidate and read
        // current flags on every opening without service calls or SD writes.
        Refresh(false, false);
        lastOpenCached = hadAddress && ready;
        if (hadAddress && !ready) Refresh(true, false);
        lastOpenMs = static_cast<unsigned>(openClock.GetElapsedTime().AsMilliseconds());
        attempted = true;
        refreshClock.Restart();
    } else if (refreshClock.HasTimePassed(Seconds(1))) {
        // A failed scan is retried on reopening or X, not once a second.
        if (!candidates.empty()) Refresh(false, false);
        refreshClock.Restart();
    }
}
void Rescan() {
    // X explicitly requests a full discovery and a diagnostic SD log.
    gameVersion = Process::GetVersion();
    Refresh(true, true);
    attempted = true;
    refreshClock.Restart();
}

void SelectImage(const std::string &directory, const std::string &image) {
    page = UltraSunCaught::Page();
    if (image.empty()) return;
    File file(directory + "/" + image + ".live", File::READ);
    if (!file.IsOpen()) return;
    const u64 size = file.GetSize();
    if (size < 16 || size > 2048) return;
    std::vector<u8> bytes(static_cast<std::size_t>(size));
    if (file.Read(bytes.data(), static_cast<u32>(size)) != File::SUCCESS) return;
    page = UltraSunCaught::ParsePage(bytes.data(), bytes.size());
}

void Draw() {
    if (!page.valid) return;
    const Color green(137, 226, 168);
    const Color light(235, 241, 249);
    const Color dim(164, 187, 209);
    const Color titleBackground(26, 48, 69);
    char text[96];
    if (page.diagnostic) {
        Renderer::DrawRect(27, 61, 266, 50, Color(17, 25, 38));
        std::snprintf(text, sizeof(text), "Reader: %s", status);
        SmallText(text, 29, 64, ready ? green : dim);
        if (ready)
            std::snprintf(text, sizeof(text), "Caught: %u species | reader 6", UltraSunCaught::Count(caught));
        else
            std::snprintf(text, sizeof(text), "Reader 6 | X: retry");
        SmallText(text, 29, 79, light);
        std::snprintf(text, sizeof(text), "Read: %u ms | %s", lastOpenMs,
            lastOpenCached ? "cached address" : "search");
        SmallText(text, 29, 94, dim);
        return;
    }
    if (page.area.empty()) return;
    // Leave the window's existing top-right close button untouched.
    Renderer::DrawRect(184, 40, 90, 11, titleBackground);
    if (ready)
        std::snprintf(text, sizeof(text), "%u/%u caught", UltraSunCaught::CountArea(caught, page), static_cast<unsigned>(page.area.size()));
    else
        std::snprintf(text, sizeof(text), "Dex: unknown");
    SmallText(text, 185, 40, ready ? green : dim);
    for (std::size_t row = 0; row < page.rows.size(); ++row) {
        const int x = 242;
        const int y = 57 + static_cast<int>(row) * 25;
        Renderer::DrawRect(x, y, 14, 14, ready ? dim : light, false);
        if (!ready) {
            // A solid amber dash remains visible without relying on font glyphs.
            Renderer::DrawRect(x + 4, y + 6, 6, 2, Color(255, 188, 92));
        } else if (UltraSunCaught::IsCaught(caught, page.rows[row])) {
            // CTRPF's vector DrawLine fills a rectangle, not a diagonal.
            // An upward stroke gives it a negative height and overruns VRAM.
            // Rasterize the check with positive 2x2 rectangles inside the box.
            static const u8 mark[][2] = {
                {3, 7}, {4, 8}, {5, 9}, {6, 10}, {7, 9},
                {8, 8}, {9, 7}, {10, 6}, {11, 5}
            };
            for (const auto &pixel : mark)
                Renderer::DrawRect(x + pixel[0], y + pixel[1], 2, 2, green);
        }
    }
}
}}
