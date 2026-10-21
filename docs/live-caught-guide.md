# Live caught status in the Game Guide

The Game Guide can show species-level caught checkboxes and an area total over
compatible encounter pictures in **Pokémon Ultra Sun** (title ID
`00040000001B5000`). It reads the loaded game's Pokédex in memory. It never edits
game memory, caught flags, or save files. Other games can still use the guide,
but their live caught status is unavailable.

Load your save and reach normal gameplay before opening the plugin's Game Guide.
A green check means caught; an empty box means not caught. An amber dash means
the reader could not validate the data, so the result is unknown. Regional forms
share their species' caught flag. A completely empty or inconsistent Pokédex is
also shown as unknown.

The reader discovers and validates the Pokédex address on first use, then reads
fresh flags from that address on subsequent openings and once a second while
the guide is open. Cached addresses are revalidated; invalid addresses are
rediscovered when reopening. The existing guide menu and current image remain
in memory, without preloading the entire image pack. Normal opening does not
write diagnostic files or query the game's version.

Press **X** inside the guide to force discovery and write `CaughtTracker.log`
in the plugin's working directory. If validation fails, it also writes a bounded
memory diagnostic, `CaughtTracker-memory.bin`. These diagnostics may contain
personal game state; inspect them before sharing. A diagnostic picture can show
the reader status, caught count, and last opening time.

## Preparing a compatible guide

This repository provides the reader and metadata writer, not an illustrated
encounter database. Existing plain text and BMP guides remain usable. Pictures
without metadata have no caught overlay. Adding metadata does not rearrange a
picture, determine encounter rates, or infer its Pokémon: the author supplies
that information and a compatible layout.

Place the guide in the plugin's existing `Guide` directory. Every annotated
picture has a companion with the **entire image name** plus `.live`:

```text
Guide/
  00 Reader status.bmp
  00 Reader status.bmp.live
  Example Area/
    00 Encounters.bmp
    00 Encounters.bmp.live
    Area info.txt
```

The guide lists folders and `.txt` entries on the top screen and displays the
current folder's BMPs on the bottom screen. Use **L/R** or swipe to switch
pictures. Only exact lowercase `.bmp` and `.txt` suffixes are listed; `.bmp.live`
companions and macOS `._` resource files are excluded.

Create a JSON list like [caught-pages.example.json](caught-pages.example.json).
Paths are relative to the `Guide` directory and must point to existing BMPs.
`rows` lists the visible species in top-to-bottom order, using National Pokédex
IDs from 1 through 807. It accepts up to six rows. Repeated row species are
allowed, for example when one species has two encounter methods.
`area_species` is the unique set of species used for the area total; include all
row species, and use the same area list across pages that share an area total.
The example values illustrate the format and are not an encounter table.

With Python 3, run these commands from the repository root, substituting your
manifest and local guide path:

```sh
python3 tools/write_caught_metadata.py pages.json /path/to/Guide --dry-run
python3 tools/write_caught_metadata.py pages.json /path/to/Guide
python3 tools/write_caught_metadata.py pages.json /path/to/Guide --check
```

The writer validates every page and path before writing anything, replaces each
companion atomically, and does not modify images or text. `--check` returns a
nonzero status for missing or different companions. All paths stay inside the
guide directory, and duplicate paths are rejected without regard to case.
Unknown JSON fields are rejected to catch misspellings. Existing manifests using
`path`, `rows`, and `area_species` are accepted directly.

### Picture layout

Author a **280 × 200** BMP, displayed at bottom-screen position `(20, 20)`.
The overlay positions are fixed, so leave these image-local rectangles clear:

| Element | Image-local rectangle (x, y, width, height) |
| --- | --- |
| Area total | `(164, 20, 90, 11)` |
| Row 0 checkbox | `(222, 37, 14, 14)` |
| Row n checkbox | `(222, 37 + 25*n, 14, 14)`, n = 0…5 |
| Diagnostic panel | `(7, 41, 266, 50)` |

Use the normal encounter layout or the diagnostic layout, not both in one
picture. To enable the diagnostic panel, set `diagnostic` to `true` and leave
both lists empty. It replaces part of that picture with the live status. Omit
`diagnostic` or set it to `false` for ordinary encounter cards. The plugin's
existing close button remains above the image.

### Binary companion format

All numbers are unsigned little-endian 16-bit values. The file length must match
exactly; trailing bytes are rejected.

| Offset | Length | Meaning |
| --- | --- | --- |
| 0 | 8 bytes | Magic `USCATCH1` |
| 8 | 2 bytes | Row count, 0–6 |
| 10 | 2 bytes | Area species count, 0–807 |
| 12 | 2 bytes | Flags: 0 = encounter, 1 = diagnostic |
| 14 | 2 bytes | Reserved, must be zero |
| 16 | 2 × row count | Row species IDs |
| Following rows | 2 × area count | Unique area species IDs |

Every row ID must occur in the area list. A diagnostic page must have zero rows
and zero area species. `Includes/CaughtCore.hpp` contains the parser and
validation rules; `Sources/LiveCaught.cpp` contains the fixed drawing layout.

## Build compatibility

The bundled CTRPluginFramework static library requires a compatible devkitARM
C/C++ runtime. This implementation was tested with **GCC 14.1 / devkitARM r64**,
using this pinned official image:

```text
devkitpro/devkitarm@sha256:0feed092a21607d3c67b4a15fa6dd1b82b2a7542ed2a52c1f5b31e37985aae78
```

A newer compiler is not automatically binary-compatible with the bundled
library. In particular, GCC 16's runtime layout differs; mixing it with this
library can compile successfully and still crash on the console.

For an ELF-only compile check, run from the repository root:

```sh
docker run --rm -v "$PWD:/project" -w /project \
  devkitpro/devkitarm@sha256:0feed092a21607d3c67b4a15fa6dd1b82b2a7542ed2a52c1f5b31e37985aae78 \
  make -j2 BUILD_TARGET=/project/AlolanCTRPluginFramework.elf
```

This produces an ELF, not an installable plugin. Packaging also requires a
compatible native `3gxtool` and the repository's `.plgInfo` file; the tested
package uses 3gxtool 1.3 and the `3GX$0002` format. Keep the toolchain compatible
when following the main README's release instructions.
