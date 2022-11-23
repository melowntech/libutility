/**
 * Copyright (c) 2018 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <algorithm>
#include <boost/filesystem.hpp>

#include "path.hpp"

namespace fs = boost::filesystem;

namespace utility {

#if BOOST_VERSION < 106100

namespace detail {

inline std::pair<fs::path::iterator, fs::path::iterator>
mismatch(fs::path::iterator first1, fs::path::iterator last1
         , fs::path::iterator first2, fs::path::iterator last2)
{
#if __cplusplus < 201402L
    while ((first1 != last1) && (first2 != last2) && (*first1 == *first2)) {
        ++first1; ++first2;
    }
    return std::make_pair(first1, first2);

#else

    // simple as that
    return std::mismatch(first1, last1, first2, last2);
#endif
}

} // namespace detail

fs::path lexically_relative(const fs::path &path, const fs::path &base)
{
    auto mm(detail::mismatch
            (path.begin(), path.end(), base.begin(), base.end()));

    if ((mm.first == path.begin()) && (mm.second == base.begin())) {
        return {};
    }

    if ((mm.first == path.end()) && (mm.second == base.end())) {
        return ".";
    }

    fs::path tmp;
    for (; mm.second != base.end(); ++mm.second) {
      tmp /= "..";
    }

    for (; mm.first != path.end(); ++mm.first) {
      tmp /= *mm.first;
    }

    return tmp;
}

#else

// it cannot be simpler!
fs::path lexically_relative(const fs::path &path, const fs::path &base)
{
    return path.lexically_relative(base);
}

#endif

void rename(const fs::path& oldPath, const fs::path& newPath)
{
#ifdef _WIN32
    // use copy/delete instead of problematic rename
    auto copyOptions(fs::copy_options::recursive
                     | fs::copy_options::overwrite_existing
                     | fs::copy_options::copy_symlinks);
    copy(oldPath, newPath, copyOptions);
    remove_all(oldPath);
#else
    fs::rename(oldPath, newPath);
#endif
}

} // namespace utility
