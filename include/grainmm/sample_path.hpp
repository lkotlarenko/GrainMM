// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <string_view>
namespace grainmm {
// Byte-preserving relative path for a rooted sample library; no allocation.
inline bool safe_sample_path(std::string_view name) noexcept {
    if (name.empty() || name.size() > 512)
        return false;
    std::size_t start = 0, count = 0;
    for (;;) {
        const auto end = name.find('/', start);
        const auto length = end == std::string_view::npos ? name.size() - start : end - start;
        if (length == 0 || length > 128 || ++count > 8 || (length == 1 && name[start] == '.') ||
            (length == 2 && name[start] == '.' && name[start + 1] == '.'))
            return false;
        for (std::size_t i = start; i < start + length; ++i)
            if (static_cast<unsigned char>(name[i]) < 32 || name[i] == 127 || name[i] == '\\')
                return false;
        if (end == std::string_view::npos)
            return true;
        start = end + 1;
    }
}
} // namespace grainmm
