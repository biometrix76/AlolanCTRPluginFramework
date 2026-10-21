#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace GuideFiles {
    inline void FilterByExtension(std::vector<std::string> &files, const std::string &extension) {
        // Directory::ListFiles uses substring matching. Sidecars such as
        // page.bmp.live and macOS ._ companions are not guide pictures/text.
        files.erase(std::remove_if(files.begin(), files.end(),
            [&extension](const std::string &name) {
                return name.compare(0, 2, "._") == 0
                    || name.size() < extension.size()
                    || name.compare(name.size() - extension.size(), extension.size(), extension) != 0;
            }), files.end());
    }
}
