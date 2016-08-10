#ifndef utility_uri_hpp_included_
#define utility_uri_hpp_included_

#include <memory>

#include <boost/filesystem/path.hpp>

#ifndef UTILITY_HAS_URI
#error uri.hpp cannot be included without compiling with liburiparser
#endif

namespace utility {

class Uri {
public:
    Uri() : port_() {}
    Uri(std::string uriString);

    const std::string& scheme() const;
    const std::string& host() const;
    int port() const;

    boost::filesystem::path path() const;

    /** Returns slice of path starting at given index.
     *
     * \param index start index
     * \param absolutize make path absolute (starting with /) if true
     */
    boost::filesystem::path path(std::size_t index
                                 , bool absolutize = false) const;

    /** Returns path component at given index.
     *  Returns empty string if index is out of bounds.
     */
    std::string pathComponent(std::size_t index) const;

    /** Returns count of path components.
     */
    std::size_t pathComponentCount() const;

    bool absolutePath() const;

    bool absolute() const;

    /** Reconstructs URI in string representation.
     */
    std::string str() const;

    /** Resolves URI reference.
     *  returns this + relative
     */
    Uri resolve(const Uri &relative) const;

    struct Storage;

private:
    Uri(const std::shared_ptr<Storage> &uri);
    std::shared_ptr<Storage> storage_;

    std::string scheme_;
    std::string host_;
    int port_;
};

Uri operator+(const Uri &base, const Uri &relative);

/** Reconstructs URI in string representation.
 */
inline std::string str(const Uri &uri) { return uri.str(); }

inline const std::string& Uri::scheme() const { return scheme_; }
inline const std::string& Uri::host() const { return host_; }
inline int Uri::port() const { return port_; }

// inlines

inline Uri operator+(const Uri &base, const Uri &relative) {
    return base.resolve(relative);
}

inline bool Uri::absolute() const { return !host().empty(); }

} // namespace utility

#endif // utility_uri_hpp_included_
