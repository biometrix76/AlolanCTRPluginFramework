#!/usr/bin/env python3
"""Build and run portable guide tests without devkitARM or console data."""

import argparse
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sanitizers", choices=("auto", "on", "off"), default="auto")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    compiler = shlex.split(os.environ.get("CXX", "c++"))
    if not compiler or not shutil.which(compiler[0]):
        parser.error("No C++ compiler found; install one or set CXX.")
    common = ["-std=c++11", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-g", "-O1"]
    sanitizer_flags = ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    env = os.environ.copy()
    env["ASAN_OPTIONS"] = "halt_on_error=1"
    env["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
    with tempfile.TemporaryDirectory(prefix="guide-tests-") as temporary:
        build = Path(temporary)
        flags = []
        if args.sanitizers != "off":
            probe_source = build / "sanitizer_probe.cpp"
            probe_source.write_text("int main() { return 0; }\n", encoding="utf-8")
            probe_binary = build / "sanitizer_probe"
            probe = subprocess.run(compiler + common + sanitizer_flags
                                   + [str(probe_source), "-o", str(probe_binary)],
                                   capture_output=True, text=True, env=env)
            if probe.returncode == 0:
                probe = subprocess.run([str(probe_binary)], capture_output=True, text=True, env=env)
            if probe.returncode == 0:
                flags = sanitizer_flags
            elif args.sanitizers == "on":
                parser.exit(1, "Address/undefined sanitizers unavailable:\n" + probe.stderr)
            else:
                print("Sanitizers unavailable; using standard native tests.", flush=True)
        print("Compiler: " + " ".join(compiler), flush=True)
        print("Sanitizers: " + ("address, undefined" if flags else "off"), flush=True)
        binary = build / "guide_tests"
        subprocess.run(compiler + common + flags + ["-I", str(root / "Includes"),
                       str(root / "tests" / "guide_tests.cpp"), "-o", str(binary)], check=True)
        # A real test failure is never retried without sanitizers.
        subprocess.run([str(binary)], check=True, env=env)


if __name__ == "__main__":
    main()
