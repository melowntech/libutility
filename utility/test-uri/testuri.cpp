#include <cstdlib>

#include "dbglog/dbglog.hpp"
#include "utility/uri.hpp"

void log(const utility::Uri &uri)
{
    auto c(uri.components());
    LOG(info4) << "    str(uri): " << str(uri);
    LOG(info4) << "    uri.scheme: " << c.scheme;
    LOG(info4) << "    uri.user: " << c.user;
    LOG(info4) << "    uri.password: " << c.password;
    LOG(info4) << "    uri.host: " << c.host;
    LOG(info4) << "    uri.port: " << c.port;
    LOG(info4) << "    uri.path: " << c.path;
    LOG(info4) << "    uri.search: " << c.search;
    LOG(info4) << "    uri.fragment: " << c.fragment;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        LOG(fatal) << "Missing parameters.";
        return EXIT_FAILURE;
    }

    utility::Uri base(argv[1]);
    LOG(info4) << "Base:";
    log(base);

    if (argc < 3) { return EXIT_SUCCESS; }

    utility::Uri relative(argv[2]);
    LOG(info4) << "Relative:";
    log(relative);

    auto resolved(base + relative);
    LOG(info4) << "Resolved:";
    log(resolved);

    return EXIT_SUCCESS;
}
