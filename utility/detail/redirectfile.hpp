/**
 * Copyright (c) 2017 Melown Technologies SE
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
#ifndef utility_detail_redirectfile_hpp_included_
#define utility_detail_redirectfile_hpp_included_

#include <string>
#include <vector>
#include <utility>
#include <iosfwd>
#include <boost/filesystem/path.hpp>
#include <boost/any.hpp>

namespace utility { namespace detail {

class SystemContext;

struct RedirectFile {
    struct SrcPath {
        SrcPath(const boost::filesystem::path &path, bool out)
            : path(path), out(out) {}
        boost::filesystem::path path;
        bool out;
    };

    struct DstArg {
        DstArg() {}
        DstArg(const std::string &format) : format(format) {}
        std::string format;
    };

    boost::any dst;
    enum class DstType { fd, arg, none };
    DstType dstType;

    boost::any src;
    enum class SrcType { fd, path, istream, ostream };
    SrcType srcType;

    RedirectFile() {}
    RedirectFile(int dst, int src)
        : dst(dst), dstType(DstType::fd)
        , src(src), srcType(SrcType::fd) {}
    RedirectFile(int dst, const boost::filesystem::path &path, bool out)
        : dst(dst), dstType(DstType::fd)
        , src(SrcPath(path, out)), srcType(SrcType::path) {}
    RedirectFile(int dst, std::istream &is)
        : dst(dst), dstType(DstType::fd)
        , src(&is), srcType(SrcType::istream) {}
    RedirectFile(int dst, std::ostream &os)
        : dst(dst), dstType(DstType::fd)
        , src(&os), srcType(SrcType::ostream) {}

    RedirectFile(std::istream &is)
        : dst(), dstType(DstType::none), src(&is), srcType(SrcType::istream) {}
    RedirectFile(std::ostream &os)
        : dst(), dstType(DstType::none), src(&os), srcType(SrcType::ostream) {}

    RedirectFile(const std::string &format, std::istream &is)
        : dst(DstArg(format)), dstType(DstType::arg)
        , src(&is), srcType(SrcType::istream) {}
    RedirectFile(const std::string &format, std::ostream &os)
        : dst(DstArg(format)), dstType(DstType::arg)
        , src(&os), srcType(SrcType::ostream) {}
};

} } // namespace utility::detail

#endif // utility_detail_redirectfile_hpp_included_
