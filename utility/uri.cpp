#include <cctype>

#include "./uri.hpp"

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

} // utility
