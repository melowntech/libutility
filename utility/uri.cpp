#include <cctype>
#include <boost/filesystem/path.hpp>

#include "./uri.hpp"

namespace fs = boost::filesystem;

namespace utility {

namespace {
    const char *alphabet("0123456789abcdef");
} // namespace

std::string urlEncode(const std::string &in, bool plus)
{
    std::string out;
    for (char c : in) {
        if (std::isalnum(c)) {
            out.push_back(c);
        } else if (plus && (c == ' ')) {
            out.push_back('+');
        } else {
            out.push_back('%');
            out.push_back(alphabet[(c >> 4) & 0x0f]);
            out.push_back(alphabet[c & 0x0f]);
        }
    }
    return out;
}

Uri join(const Uri &base, const Uri &uri)
{
    if (!uri.host.empty()) {
        // we have some host
        if (uri.schema.empty()) {
            // no schema, copy from base
            auto tmp(uri);
            tmp.schema = base.schema;
            return tmp;
        }

        // as-is
        return uri;
    }

    // no host, either full path or relative path
    if (!uri.path.empty() && (uri.path[0] != '/')) {
        // absolute path, replace
        auto tmp(base);
        tmp.path = uri.path;
        return tmp;
    }

    // some magic
    std::vector<fs::path> out;
    auto append([&](const fs::path path) -> bool
    {
        bool lastEmpty(false);
        for (const auto &element : path) {
            if (element.empty() || (element == ".")) {
                // nothing to do
                lastEmpty = true;
            } else if (element == "..") {
                out.pop_back();
                lastEmpty = true;
            } else {
                out.push_back(element);
                lastEmpty = false;
            }
        }
        return lastEmpty;
    });

    // append base path, handle dangling filename
    if (!append(base.path)) {
        // remove last element since it is not a directory
        out.pop_back();
    }

    // append uri path
    append(uri.path);

    // join path
    fs::path outPath("/");
    for (const auto &element : out) {
        outPath /= element;
    }

    auto tmp(base);
    tmp.path = outPath.string();
    return tmp;
}

} // utility
