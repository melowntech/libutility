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
#include <cstdlib>
#include <iostream>

#include <boost/filesystem.hpp>

#include "utility/buildsys.hpp"
#include "utility/gccversion.hpp"
#include "utility/streams.hpp"
#include "utility/zip.hpp"

#include "service/cmdline.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace {

class Zip : public service::Cmdline
{
public:
    Zip()
        : service::Cmdline("utility-zip", BUILD_TARGET_VERSION)
        , overwrite_(false), verbose_(false)
        , method_(utility::zip::Compression::store)
    {}

private:
    virtual void configuration(po::options_description &cmdline
                               , po::options_description &config
                               , po::positional_options_description &pd)
        UTILITY_OVERRIDE;

    virtual void configure(const po::variables_map &vars)
        UTILITY_OVERRIDE;

    virtual bool help(std::ostream &out, const std::string &what) const
        UTILITY_OVERRIDE;

    virtual int run() UTILITY_OVERRIDE;

    void add(utility::zip::Writer &zip, const fs::path &file);

    fs::path zip_;
    bool overwrite_;
    std::vector<fs::path> files_;
    boost::optional<fs::path> cdir_;
    bool verbose_;
    utility::zip::Compression method_;
};

void Zip::configuration(po::options_description &cmdline
                        , po::options_description &config
                        , po::positional_options_description &pd)
{
    cmdline.add_options()
        ("zip,o", po::value(&zip_)->required()
         , "Path to output zip file.")
        ("overwrite", "Overwrite existing zip archive if empty.")
        ("file", po::value(&files_)->required()
         , "File to place into the zip archive, can be used multiple times.")
        ("directory,C", po::value<fs::path>()
         , "Change to given directory.")
        ("verbose", "Show what is going on.")
        ("method", po::value(&method_)
         , utility::concat
         ("Compression method, one of "
          , enumerationString(method_), ".").c_str())
        ;

    pd.add("zip", 1)
        .add("file", -1);

    (void) config;
}

void Zip::configure(const po::variables_map &vars)
{
    overwrite_ = vars.count("overwrite");
    verbose_ = vars.count("verbose");
    if (vars.count("directory")) {
        cdir_ = vars["directory"].as<fs::path>();
    }
}

bool Zip::help(std::ostream &out, const std::string &what) const
{
    if (what.empty()) {
        out << R"RAW(utility-zip
usage
    utility-zip OUTPUT INPUT+ [OPTIONS]

)RAW";
    }
    return false;
}

void copy(const fs::path &path
          , const utility::zip::Writer::OStream::pointer &os)
{
    utility::ifstreambuf is(path.string());
    os->get() << is.rdbuf();
    os->close();
}

class Packer {
public:
    Packer(utility::zip::Writer zip
           , utility::zip::Compression method, bool verbose)
        : zip_(zip), method_(method), verbose_(verbose)
    {}

    void add(const fs::path &file);

private:
    void store(const fs::path &file);

    utility::zip::Writer &zip_;
    const utility::zip::Compression method_;
    const bool verbose_;
};

void Packer::store(const fs::path &file)
{
    if (verbose_) {
        std::cout << file.string() << '\n';
    }
    copy(file, zip_.ostream(file, method_));
    return;
}

void Packer::add(const fs::path &file)
{
    if (!fs::is_directory(file)) {
        store(file);
        return;
    }

    for (fs::recursive_directory_iterator i(file), e; i != e; ++i) {
        const auto &path(i->path());
        if (!fs::is_directory(path)) { store(path); }
    }
}

int Zip::run()
{
    utility::zip::Writer zip(zip_, overwrite_);

    if (cdir_) {
        // switch to configured directory
        fs::current_path(*cdir_);
    }

    Packer packer(zip, method_, verbose_);
    for (const auto &file : files_) { packer.add(file); }

    zip.close();

    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char *argv[])
{
    return Zip()(argc, argv);
}
