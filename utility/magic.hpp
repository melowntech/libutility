#ifndef utility_magic_hpp_included_
#define utility_magic_hpp_included_

#include <cstddef>
#include <stdexcept>
#include <memory>
#include <string>

#include <boost/filesystem/path.hpp>

namespace utility {

struct MagicError : std::runtime_error {
    MagicError(const std::string &msg) : std::runtime_error(msg) {}
};

class Magic {
public:
    Magic(bool followSymlinks = true);

    std::string mime(const void *buffer, std::size_t length) const;

    std::string mime(const boost::filesystem::path &path) const;

private:
    std::shared_ptr<void> magic_;
};

} // namespace utility

#endif // utility_magic_hpp_included_
