#!/usr/bin/env python3
"""Write validated USCATCH1 companions for an existing Game Guide image pack."""

import argparse
import json
import os
from pathlib import Path, PurePosixPath
import struct
import sys
import tempfile

MAX_SPECIES = 807


def species_list(value, name, limit, unique=False):
    if not isinstance(value, list) or len(value) > limit:
        raise ValueError("{} must be a list with at most {} entries".format(name, limit))
    if any(type(item) is not int or not 1 <= item <= MAX_SPECIES for item in value):
        raise ValueError("{} must contain integer species IDs from 1 to 807".format(name))
    if unique and len(set(value)) != len(value):
        raise ValueError("{} must not contain duplicate species".format(name))
    return value


def encode_page(entry):
    """Match UltraSunCaught::ParsePage, including repeated visible row species."""
    if not isinstance(entry, dict):
        raise ValueError("each page must be an object")
    allowed = {"path", "rows", "area_species", "diagnostic"}
    if set(entry) - allowed:
        raise ValueError("unknown page fields: {}".format(", ".join(sorted(set(entry) - allowed))))
    rows = species_list(entry.get("rows"), "rows", 6)
    area = species_list(entry.get("area_species"), "area_species", MAX_SPECIES, unique=True)
    diagnostic = entry.get("diagnostic", False)
    if type(diagnostic) is not bool:
        raise ValueError("diagnostic must be true or false")
    if diagnostic and (rows or area):
        raise ValueError("diagnostic pages must have empty rows and area_species")
    if not set(rows).issubset(area):
        raise ValueError("every row species must be included in area_species")
    header = struct.pack("<8sHHHH", b"USCATCH1", len(rows), len(area), int(diagnostic), 0)
    return header + b"".join(struct.pack("<H", value) for value in rows + area)


def within_root(path, root):
    try:
        path.resolve().relative_to(root)
    except ValueError:
        raise ValueError("path escapes the guide directory: {}".format(path))


def plan_pages(manifest, guide):
    """Validate the whole manifest before any output is changed."""
    if not isinstance(manifest, list):
        raise ValueError("manifest must be a JSON list of page objects")
    root = guide.resolve(strict=True)
    if not root.is_dir():
        raise ValueError("guide must be an existing directory")
    planned = []
    seen = set()
    for number, entry in enumerate(manifest, 1):
        try:
            data = encode_page(entry)
            name = entry.get("path")
            if not isinstance(name, str) or not name or "\\" in name or ":" in name:
                raise ValueError("path must be a relative path using forward slashes")
            if any(ord(char) < 32 for char in name):
                raise ValueError("path contains control characters")
            parts = name.split("/")
            if any(part in ("", ".", "..") for part in parts):
                raise ValueError("path must not contain empty, dot, or parent components")
            relative = PurePosixPath(name)
            if relative.is_absolute() or relative.suffix != ".bmp" or relative.name.startswith("._"):
                raise ValueError("path must name a visible, lowercase .bmp image")
            # SD cards commonly use a case-insensitive filesystem.
            key = name.casefold()
            if key in seen:
                raise ValueError("duplicate image path: {}".format(name))
            seen.add(key)
            image = root.joinpath(*parts)
            target = image.with_name(image.name + ".live")
            within_root(image, root)
            within_root(target, root)
            if not image.is_file():
                raise ValueError("image does not exist: {}".format(name))
            if target.is_symlink() or (target.exists() and not target.is_file()):
                raise ValueError("metadata destination must be a regular file: {}".format(name))
            planned.append((target, data))
        except ValueError as error:
            raise ValueError("page {}: {}".format(number, error))
    return planned


def write_atomic(path, data):
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix=".caught-", delete=False) as output:
            temporary = output.name
            output.write(data)
        os.replace(temporary, path)
    finally:
        if temporary is not None and os.path.exists(temporary):
            os.unlink(temporary)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path, help="JSON list of image paths and species IDs")
    parser.add_argument("guide", type=Path, help="existing Guide directory containing the BMP images")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", action="store_true", help="validate only; do not write files")
    mode.add_argument("--check", action="store_true", help="fail if any companion is missing or differs")
    args = parser.parse_args()
    try:
        manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
        planned = plan_pages(manifest, args.guide)
        if args.check:
            mismatches = [path for path, data in planned if not path.is_file() or path.read_bytes() != data]
            for path in mismatches:
                print("Missing or different: {}".format(path), file=sys.stderr)
            if mismatches:
                return 1
        elif not args.dry_run:
            for path, data in planned:
                write_atomic(path, data)
        action = "Validated" if args.dry_run else "Checked" if args.check else "Wrote"
        print("{} {} metadata files ({} bytes).".format(action, len(planned), sum(len(data) for _, data in planned)))
        return 0
    except (OSError, ValueError) as error:
        print("error: {}".format(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
