# Guide regression tests

Run from the repository root with Python 3 and a C++11 compiler:

```sh
python3 tests/run_tests.py
```

The runner builds in a temporary directory and enables AddressSanitizer and
UndefinedBehaviorSanitizer when the local compiler and runtime support them.
Use `--sanitizers on` to require both (as CI does), or `--sanitizers off` for
environments without them. Set `CXX` to choose another compiler.

These tests use synthetic data only. They cover caught/seen bounds and malformed
dex blocks; alternate-form seen flags, including Alolan Rattata, Dugtrio, and
Grimer; metadata counts, duplicate handling and area membership; and exact file
extension filtering that excludes `.bmp.live` and macOS `._` companions while
preserving menu order.

They do not emulate the 3DS memory service, graphics renderer, or menu lifecycle.
Those paths still require a plugin build and on-console testing.
