#include <type_traits>

#include <magic.h>

#include "dbglog/dbglog.hpp"

#include "magic.hpp"

namespace utility {

typedef std::remove_pointer< ::magic_t>::type MagicCookie;

Magic::Magic(bool followSymlinks)
    : magic_(magic_open(MAGIC_MIME_TYPE
                        | (followSymlinks ? MAGIC_SYMLINK : 0))
             , [] (magic_t m) {
                 if (m) ::magic_close(m);
             })
{
    auto cookie(std::static_pointer_cast<MagicCookie>(magic_).get());
    if (::magic_load(cookie, nullptr) == -1) {
        LOGTHROW(err1, MagicError)
            << "Cannot load magic database: <"
            << ::magic_error(cookie) << ">.";
    }
}

std::string Magic::mime(const void *buffer, std::size_t length) const
{
    auto cookie(std::static_pointer_cast<MagicCookie>(magic_).get());
    auto res(magic_buffer(cookie, buffer, length));

    if (!res) {
        LOGTHROW(err1, MagicError)
            << "Cannot determine magic of given buffer: <"
            << ::magic_error(cookie) << ">.";
    }

    return res;
}

std::string Magic::mime(const boost::filesystem::path &path) const
{
    auto cookie(std::static_pointer_cast<MagicCookie>(magic_).get());
    auto res(magic_file(cookie, path.string().c_str()));

    if (!res) {
        LOGTHROW(err1, MagicError)
            << "Cannot determine magic of given buffer: <"
            << ::magic_error(cookie) << ">.";
    }

    return res;
}

} // namespace utility
