#include "CaughtCore.hpp"
#include "GuideFiles.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {
using namespace UltraSunCaught;
using Bytes = std::vector<std::uint8_t>;
unsigned checks = 0;

void Require(bool condition, const char *description) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void Put16(Bytes &data, std::size_t offset, unsigned value) {
    data[offset] = static_cast<std::uint8_t>(value);
    data[offset + 1] = static_cast<std::uint8_t>(value >> 8);
}

void SetBit(Bytes &data, unsigned offset, unsigned bit) {
    data[offset + bit / 8] |= static_cast<std::uint8_t>(1u << (bit % 8));
}

Bytes EmptyDex() {
    Bytes dex(SnapshotBytes, 0);
    for (unsigned byte = 0; byte < 4; ++byte)
        dex[byte] = static_cast<std::uint8_t>(DexMagic >> (8 * byte));
    return dex;
}

void CatchBase(Bytes &dex, unsigned species, unsigned region = 0) {
    SetBit(dex, CaughtOffset, species - 1);
    SetBit(dex, SeenOffset + region * SeenStride, species - 1);
}

Bytes Metadata(const std::vector<std::uint16_t> &rows,
               const std::vector<std::uint16_t> &area, unsigned flags = 0) {
    Bytes data(16 + 2 * (rows.size() + area.size()), 0);
    std::memcpy(data.data(), "USCATCH1", 8);
    Put16(data, 8, static_cast<unsigned>(rows.size()));
    Put16(data, 10, static_cast<unsigned>(area.size()));
    Put16(data, 12, flags);
    std::size_t cursor = 16;
    for (auto species : rows) { Put16(data, cursor, species); cursor += 2; }
    for (auto species : area) { Put16(data, cursor, species); cursor += 2; }
    return data;
}

Page Parse(const Bytes &data) { return ParsePage(data.data(), data.size()); }

void TestCaughtAndDex() {
    Bytes flags(CaughtBytes, 0);
    SetBit(flags, 0, 0);
    SetBit(flags, 0, MaxSpecies - 1);
    Require(IsCaught(flags.data(), 1) && IsCaught(flags.data(), 807), "caught edge species");
    Require(!IsCaught(flags.data(), 0) && !IsCaught(flags.data(), 808)
            && !IsCaught(flags.data(), std::numeric_limits<unsigned>::max()), "out-of-range species");
    Require(!IsCaught(flags.data(), 2) && Count(flags.data()) == 2, "caught count and uncaught species");

    Bytes dex = EmptyDex();
    Require(CheckDex(dex.data(), dex.size()) == DexCheck::EmptyCaught, "empty dex stays unavailable");
    CatchBase(dex, 1);
    CatchBase(dex, 807, 3);
    const Bytes original = dex;
    Require(ValidateDex(dex.data(), dex.size()), "valid dex with endpoint species");
    Require(dex == original, "dex validation does not modify input");
    Require(!IsSeen(dex.data(), 0) && !IsSeen(dex.data(), 808), "seen species bounds");
    for (std::size_t size = 0; size < SnapshotBytes; ++size) {
        Bytes shortDex(dex.begin(), dex.begin() + size);
        Require(CheckDex(shortDex.data(), shortDex.size()) == DexCheck::TooShort, "truncated dex rejected");
    }
    dex[0] ^= 1;
    Require(CheckDex(dex.data(), dex.size()) == DexCheck::WrongMagic, "wrong dex magic");
    for (unsigned bit = MaxSpecies; bit < CaughtBytes * 8; ++bit) {
        dex = original;
        SetBit(dex, CaughtOffset, bit);
        Require(CheckDex(dex.data(), dex.size()) == DexCheck::PaddingSet, "every unused caught bit rejected");
    }
    for (unsigned region = 0; region < 4; ++region) {
        dex = EmptyDex();
        for (unsigned species = 1; species <= MaxSpecies; ++species) CatchBase(dex, species, region);
        Require(ValidateDex(dex.data(), dex.size()), "all species accepted in each seen region");
        Require(Count(dex.data() + CaughtOffset) == MaxSpecies, "all-species count");
    }
    dex = EmptyDex();
    SetBit(dex, CaughtOffset, 24);
    Require(CheckDex(dex.data(), dex.size()) == DexCheck::CaughtNotSeen, "caught species requires seen evidence");
}

void TestForms() {
    // These three Alolan species exposed the base-form-only validation bug.
    // Keep the expected bits independent of the generated lookup table.
    const unsigned regressions[][2] = {{19, 813}, {51, 829}, {88, 837}};
    for (const auto &entry : regressions) {
        for (unsigned region = 0; region < 4; ++region) {
            Bytes dex = EmptyDex();
            SetBit(dex, CaughtOffset, entry[0] - 1);
            SetBit(dex, SeenOffset + region * SeenStride, entry[1]);
            Require(ValidateDex(dex.data(), dex.size()), "Alolan-only seen flag validates caught species");
            dex[SeenOffset + region * SeenStride + entry[1] / 8] = 0;
            Require(CheckDex(dex.data(), dex.size()) == DexCheck::CaughtNotSeen, "missing Alolan seen flag rejected");
            SetBit(dex, SeenOffset + region * SeenStride, 807); // Venusaur's alternate form.
            Require(CheckDex(dex.data(), dex.size()) == DexCheck::CaughtNotSeen, "unrelated alternate form rejected");
        }
    }
    unsigned previousSpecies = 0;
    unsigned nextBit = MaxSpecies;
    for (const SeenFormRange &form : USUMSeenForms) {
        Require(form.species > previousSpecies && form.species <= MaxSpecies, "form table sorted by unique species");
        Require(form.count > 0 && form.firstBit == nextBit
                && form.firstBit + form.count <= SeenStride * 8, "form ranges contiguous and in bounds");
        for (unsigned region = 0; region < 4; ++region) {
            for (unsigned i = 0; i < form.count; ++i) {
                Bytes dex = EmptyDex();
                SetBit(dex, CaughtOffset, form.species - 1);
                SetBit(dex, SeenOffset + region * SeenStride, form.firstBit + i);
                Require(ValidateDex(dex.data(), dex.size()), "alternate seen form accepted in each region");
            }
        }
        previousSpecies = form.species;
        nextBit = form.firstBit + form.count;
    }
}

void TestMetadata() {
    // Repeated display rows are legal (different forms/tables); area totals are unique.
    const Bytes valid = Metadata({19, 19, 51}, {19, 51, 88});
    Page page = Parse(valid);
    Require(page.valid && !page.diagnostic, "ordinary metadata accepted");
    Require(page.rows == std::vector<std::uint16_t>({19, 19, 51})
            && page.area == std::vector<std::uint16_t>({19, 51, 88}), "metadata preserves display order");
    Bytes flags(CaughtBytes, 0);
    SetBit(flags, 0, 18);
    SetBit(flags, 0, 87);
    Require(CountArea(flags.data(), page) == 2, "area caught count ignores duplicate display rows");
    Require(Parse(Metadata({}, {}, 1)).diagnostic, "diagnostic-only metadata");
    Require(Parse(Metadata({}, {})).valid, "empty ordinary metadata");
    Require(!Parse(Metadata({19}, {19}, 1)).valid, "diagnostic metadata cannot contain species");
    Require(!Parse(Metadata({}, {}, 2)).valid, "unknown metadata flags rejected");
    Require(!Parse(Metadata({19}, {19, 19})).valid, "duplicate area species rejected");
    Require(!Parse(Metadata({51}, {19})).valid, "row must belong to area");
    for (std::uint16_t invalid : {0, 808, 65535}) {
        Require(!Parse(Metadata({}, {invalid})).valid, "invalid area species rejected");
        Require(!Parse(Metadata({invalid}, {19})).valid, "invalid row species rejected");
    }
    Require(Parse(Metadata({1, 2, 3, 4, 5, 807}, {1, 2, 3, 4, 5, 807})).valid, "six-row limit accepted");
    Require(!Parse(Metadata({1, 2, 3, 4, 5, 6, 7}, {1, 2, 3, 4, 5, 6, 7})).valid, "seven rows rejected");
    std::vector<std::uint16_t> area;
    for (unsigned species = 1; species <= MaxSpecies; ++species) area.push_back(static_cast<std::uint16_t>(species));
    Require(Parse(Metadata({1, 807}, area)).valid, "maximum area accepted");
    area.push_back(1);
    Require(!Parse(Metadata({}, area)).valid, "oversized area rejected");
    for (std::size_t size = 0; size < valid.size(); ++size) {
        const Bytes truncated(valid.begin(), valid.begin() + size);
        Require(!Parse(truncated).valid, "truncated metadata rejected");
    }
    Bytes changed = valid;
    changed.push_back(0);
    Require(!Parse(changed).valid, "trailing metadata bytes rejected");
    changed = valid; changed[0] ^= 1;
    Require(!Parse(changed).valid, "wrong metadata magic rejected");
    changed = valid; Put16(changed, 14, 1);
    Require(!Parse(changed).valid, "reserved metadata field rejected");
    changed = valid; Put16(changed, 8, 65535);
    Require(!Parse(changed).valid, "oversized declared row count rejected");
    changed = valid; Put16(changed, 10, 65535);
    Require(!Parse(changed).valid, "oversized declared area count rejected");
}

void TestFiles() {
    const std::vector<std::string> input = {
        "02 encounters.bmp", "._02 encounters.bmp", "02 encounters.bmp.live",
        "01 encounters.bmp", "00 welcome.bmp.live", "notes.bmp.txt", "Map.bmp",
        "", "b", "._", "._Map.bmp", "Map.BMP", "Map.bmp.backup", "Map.txt",
        "._Map.txt", "Map.txt.live", "a.bmp.name.bmp", "02 encounters.bmp"
    };
    auto files = input;
    GuideFiles::FilterByExtension(files, ".bmp");
    const std::vector<std::string> expected = {
        "02 encounters.bmp", "01 encounters.bmp", "Map.bmp", "a.bmp.name.bmp", "02 encounters.bmp"
    };
    Require(files == expected, "BMP filter excludes sidecars and companions while preserving order");
    GuideFiles::FilterByExtension(files, ".bmp");
    Require(files == expected, "BMP filtering is idempotent");
    files = input;
    GuideFiles::FilterByExtension(files, ".txt");
    Require(files == std::vector<std::string>({"notes.bmp.txt", "Map.txt"}), "text filter requires exact suffix");
    files.clear();
    GuideFiles::FilterByExtension(files, ".bmp");
    Require(files.empty(), "empty file list");
}
}

int main() {
    TestCaughtAndDex();
    TestForms();
    TestMetadata();
    TestFiles();
    std::cout << "Passed " << checks << " guide regression checks.\n";
}
