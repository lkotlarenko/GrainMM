// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/ui_theme.hpp"
#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
int main() {
    char pattern[] = "/tmp/grainmm-theme-XXXXXX";
    const auto dir = ::mkdtemp(pattern);
    if (!dir)
        return 1;
    const std::string base = std::string(dir) + "/";
    int result = 0;
    try {
        if (grainmm::load_theme(dir) != 0)
            return 1;
        for (unsigned i = 0; i < grainmm::themes.size(); ++i) {
            grainmm::save_theme(dir, i);
            if (grainmm::load_theme(dir) != i)
                return 1;
        }
        std::ofstream(base + "theme.v1.pending") << "keep";
        bool failed = false;
        try {
            grainmm::save_theme(dir, 0);
        } catch (...) {
            failed = true;
        }
        if (!failed || grainmm::load_theme(dir) != 5)
            return 1;
        ::unlink((base + "theme.v1.pending").c_str());
        std::ofstream(base + "theme.v1") << "corrupt";
        failed = false;
        try {
            (void)grainmm::load_theme(dir);
        } catch (...) {
            failed = true;
        }
        if (!failed)
            return 1;
        ::unlink((base + "theme.v1").c_str());
        ::symlink("protected", (base + "theme.v1").c_str());
        failed = false;
        try {
            grainmm::save_theme(dir, 0);
        } catch (...) {
            failed = true;
        }
        if (!failed)
            return 1;
    } catch (...) {
        result = 1;
    }
    ::unlink((base + "theme.v1").c_str());
    ::unlink((base + "theme.v1.pending").c_str());
    ::rmdir(dir);
    return result;
}
