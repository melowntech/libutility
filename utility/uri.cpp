#include <cstdlib>

#include <uriparser/Uri.h>

#include "dbglog/dbglog.hpp"

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

struct Uri::Storage {
    ::UriUriA uri;
    std::string raw;

    Storage(std::string &&raw) : raw(std::move(raw)) {
        std::memset(&uri, 0x0, sizeof(::UriUriA));
    }

    Storage() {
        std::memset(&uri, 0x0, sizeof(::UriUriA));
    }
};

namespace {

std::shared_ptr<Uri::Storage> allocateUri()
{
    std::unique_ptr<Uri::Storage> tmp(new Uri::Storage());

    return std::shared_ptr<Uri::Storage>
        (tmp.release(), [](Uri::Storage *storage)
    {
        if (storage) { ::uriFreeUriMembersA(&storage->uri); }
        delete storage;
    });
}

std::shared_ptr<Uri::Storage> allocateUri(std::string &&raw)
{
    std::unique_ptr<Uri::Storage> tmp(new Uri::Storage(std::move(raw)));

    return std::shared_ptr<Uri::Storage>
        (tmp.release(), [](Uri::Storage *storage)
    {
        if (storage) { ::uriFreeUriMembersA(&storage->uri); }
        delete storage;
    });
}

} // namespace

Uri::Uri(std::string uriString)
    : storage_(allocateUri(std::move(uriString))), port_()
{
    ::UriParserStateA state;
    state.uri = &storage_->uri;
    if (::uriParseUriA(&state, storage_->raw.c_str()) != URI_SUCCESS) {
        LOGTHROW(err1, std::runtime_error)
            << "Cannot parse uri <" << storage_->raw << ">";
    }

    host_.assign(state.uri->hostText.first, state.uri->hostText.afterLast);
    if (state.uri->portText.first != state.uri->portText.afterLast) {
        port_ = std::atoi
            (std::string(state.uri->portText.first
                         , state.uri->portText.afterLast).c_str());
    }
}

Uri::Uri(const std::shared_ptr<Storage> &storage)
    : storage_(storage)
{
    if (!storage_) { return; }
    auto &u(storage->uri);

    host_.assign(u.hostText.first, u.hostText.afterLast);
    if (u.portText.first != u.portText.afterLast) {
        port_ = std::atoi
            (std::string(u.portText.first
                         , u.portText.afterLast).c_str());
    }
}

bool Uri::absolutePath() const
{
    return (storage_
            ? (storage_->uri.absolutePath || !host_.empty())
            : false);
}

boost::filesystem::path Uri::path() const
{
    if (!storage_) { return {}; }
    const auto &uri(storage_->uri);

    boost::filesystem::path out;
    if (absolutePath()) { out /= "/"; }

    for (auto segment(uri.pathHead); segment; segment = segment->next) {
        out.append(segment->text.first, segment->text.afterLast);
    }

    return out;
}

boost::filesystem::path Uri::path(std::size_t index, bool absolutize) const
{
    if (!storage_) { return {}; }
    const auto &uri(storage_->uri);

    boost::filesystem::path out;

    for (auto segment(uri.pathHead); segment; segment = segment->next) {
        if (index > 0) { --index; continue; }
        if (absolutize) { out /= "/"; absolutize = false; }
        out.append(segment->text.first, segment->text.afterLast);
    }

    return out;
}

std::string Uri::pathComponent(std::size_t index) const
{
    if (!storage_) { return {}; }
    const auto &uri(storage_->uri);

    for (auto segment(uri.pathHead); segment;
         segment = segment->next, --index)
    {
        if (!index) {
            return { segment->text.first, segment->text.afterLast };
        }
    }

    return {};
}

std::size_t Uri::pathComponentCount() const
{
    if (!storage_) { return {}; }
    const auto &uri(storage_->uri);

    std::size_t count(0);
    for (auto segment(uri.pathHead); segment; segment = segment->next) {
        ++count;
    }
    return count;
}

std::string Uri::str() const
{
    if (!storage_) { return {}; }
    const auto &uri(storage_->uri);

    int length;
    if (::uriToStringCharsRequiredA(&uri, &length) != URI_SUCCESS) {
        LOGTHROW(err1, std::runtime_error)
            << "Cannot recompose uri.";
    }

    std::vector<char> tmp(length + 1, 0);
    if (::uriToStringA(tmp.data(), &uri, length + 1, nullptr) != URI_SUCCESS) {
        LOGTHROW(err1, std::runtime_error)
            << "Cannot recompose uri.";
    }

    return { tmp.data(), std::string::size_type(length) };
}

Uri Uri::resolve(const Uri &relative) const
{
    // get source
    if (!relative.storage_) { return *this; }
    auto &src(relative.storage_->uri);

    if (!storage_) { return relative; }
    auto &uri(storage_->uri);

    // get output
    auto dstStorage(allocateUri());
    auto &dst(dstStorage->uri);

    auto res(::uriAddBaseUriA(&dst, &src, &uri));
    if (res != URI_SUCCESS) {
        LOGTHROW(err1, std::runtime_error)
            << "Cannot resolve uri (err=" << res << ").";
    }

    // done
    return Uri(dstStorage);
}

} // utility
