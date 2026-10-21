#ifndef ULTRASUN_CAUGHT_CORE_HPP
#define ULTRASUN_CAUGHT_CORE_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include "CaughtFormsUSUM.hpp"

namespace UltraSunCaught {
constexpr unsigned MaxSpecies = 807;
constexpr unsigned CaughtOffset = 0x88;
constexpr unsigned CaughtBytes = 0x68;
constexpr unsigned SeenOffset = 0xF0;
constexpr unsigned SeenStride = 0x8C;
constexpr unsigned SnapshotBytes = SeenOffset + 4 * SeenStride;
constexpr std::uint32_t DexMagic = 0x2F120F17;

inline std::uint16_t Read16(const std::uint8_t *p) {
    return static_cast<std::uint16_t>(p[0] | (p[1] << 8));
}
inline std::uint32_t Read32(const std::uint8_t *p) {
    return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
           (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
}
inline bool Bit(const std::uint8_t *p, unsigned bit) {
    return (p[bit >> 3] & (1u << (bit & 7))) != 0;
}
inline bool IsCaught(const std::uint8_t *caught, unsigned species) {
    return species >= 1 && species <= MaxSpecies && Bit(caught, species - 1);
}
inline unsigned Count(const std::uint8_t *caught) {
    unsigned n = 0;
    for (unsigned s = 1; s <= MaxSpecies; ++s) n += IsCaught(caught, s);
    return n;
}

inline bool IsSeen(const std::uint8_t *data, unsigned species) {
    if (species < 1 || species > MaxSpecies) return false;
    for (unsigned sexShiny = 0; sexShiny < 4; ++sexShiny)
        if (Bit(data + SeenOffset + sexShiny * SeenStride, species - 1))
            return true;

    // A legitimately caught alternate form may have only its form-specific
    // seen bit set. In particular, Alolan forms need not mark the Kanto form.
    for (const SeenFormRange &form : USUMSeenForms) {
        if (form.species > species) break;
        if (form.species != species) continue;
        for (unsigned sexShiny = 0; sexShiny < 4; ++sexShiny)
            for (unsigned i = 0; i < form.count; ++i)
                if (Bit(data + SeenOffset + sexShiny * SeenStride, form.firstBit + i))
                    return true;
        break;
    }
    return false;
}

// A signature alone is not enough: each caught species must have a seen base
// or alternate form, and unused caught bits must be clear. Reject empty or
// uninitialised blocks rather than showing a misleading missing-Pokemon list.
enum class DexCheck : unsigned { Valid, TooShort, WrongMagic, EmptyCaught, PaddingSet, CaughtNotSeen };
inline DexCheck CheckDex(const std::uint8_t *data, std::size_t size) {
    if (size < SnapshotBytes) return DexCheck::TooShort;
    if (Read32(data) != DexMagic) return DexCheck::WrongMagic;
    const std::uint8_t *caught = data + CaughtOffset;
    if (Count(caught) == 0) return DexCheck::EmptyCaught;
    for (unsigned bit = MaxSpecies; bit < CaughtBytes * 8; ++bit)
        if (Bit(caught, bit)) return DexCheck::PaddingSet;
    for (unsigned bit = 0; bit < MaxSpecies; ++bit) {
        if (!Bit(caught, bit)) continue;
        if (!IsSeen(data, bit + 1)) return DexCheck::CaughtNotSeen;
    }
    return DexCheck::Valid;
}
inline bool ValidateDex(const std::uint8_t *data, std::size_t size) {
    return CheckDex(data, size) == DexCheck::Valid;
}

struct Page {
    bool valid = false;
    bool diagnostic = false;
    std::vector<std::uint16_t> rows;
    std::vector<std::uint16_t> area;
};

inline Page ParsePage(const std::uint8_t *data, std::size_t size) {
    Page page;
    if (size < 16 || std::memcmp(data, "USCATCH1", 8) != 0) return page;
    const unsigned rowCount = Read16(data + 8);
    const unsigned areaCount = Read16(data + 10);
    const unsigned flags = Read16(data + 12);
    if (rowCount > 6 || areaCount > MaxSpecies || flags > 1 || Read16(data + 14) != 0)
        return page;
    if (size != 16 + 2 * (rowCount + areaCount)) return page;
    if (flags == 1 && (rowCount || areaCount)) return page;
    bool present[MaxSpecies + 1] = {};
    for (unsigned i = 0; i < areaCount; ++i) {
        unsigned species = Read16(data + 16 + 2 * (rowCount + i));
        if (species < 1 || species > MaxSpecies || present[species]) return Page();
        present[species] = true;
        page.area.push_back(species);
    }
    for (unsigned i = 0; i < rowCount; ++i) {
        unsigned species = Read16(data + 16 + 2 * i);
        if (species < 1 || species > MaxSpecies || !present[species]) return Page();
        page.rows.push_back(species);
    }
    page.valid = true;
    page.diagnostic = flags == 1;
    return page;
}
inline unsigned CountArea(const std::uint8_t *caught, const Page &page) {
    unsigned n = 0;
    for (std::uint16_t species : page.area) n += IsCaught(caught, species);
    return n;
}
}
#endif
