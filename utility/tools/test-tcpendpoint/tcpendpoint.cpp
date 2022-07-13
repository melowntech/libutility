/**
 * Copyright (c) 2022 Melown Technologies SE
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
#include "utility/tcpendpoint.hpp"
#include "utility/tcpendpoint-io.hpp"

#include "service/cmdline.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace {

class TcpEndpoint : public service::Cmdline
{
public:
    TcpEndpoint()
        : service::Cmdline("utility-tcpendpoint", BUILD_TARGET_VERSION)
    {}

private:
    void configuration(po::options_description &cmdline
                       , po::options_description &config
                       , po::positional_options_description &pd)
        override;

    void configure(const po::variables_map &vars) override;

    bool help(std::ostream &out, const std::string &what) const
        override;

    int run() override;

    std::string endpoint_;
    utility::TcpEndpoint::ParseFlags flags_ = {};
};

void TcpEndpoint::configuration(po::options_description &cmdline
                        , po::options_description &config
                        , po::positional_options_description &pd)
{
    cmdline.add_options()
        ("endpoint,e", po::value(&endpoint_)->required()
         , "TCP Endpoint.")
        ("allow-hostname-resolution,r", "Allow hostname resolution.")
        ;

    pd.add("endpoint", 1);

    (void) config;
}

void TcpEndpoint::configure(const po::variables_map &vars)
{
    if (vars.count("allow-hostname-resolution")) {
        flags_ |= utility::TcpEndpoint::ParseFlags::allowResolve;
    }
}

bool TcpEndpoint::help(std::ostream &out, const std::string &what) const
{
    if (what.empty()) {
        out << R"RAW(utility-tcpendpoint
usage
    utility-tcpendpoint ENDPOINT [OPTIONS]

)RAW";
    }
    return false;
}

int TcpEndpoint::run()
{
    try {
        utility::TcpEndpoint endpoint(endpoint_, flags_);
        std::cout << endpoint << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "failed to parse endpoint <" << endpoint_
                  << ">: " << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char *argv[])
{
    return TcpEndpoint()(argc, argv);
}
