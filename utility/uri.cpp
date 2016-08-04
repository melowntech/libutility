#include <cstdlib>

#include <uriparser/Uri.h>

#include "dbglog/dbglog.hpp"

#include "./uri.hpp"

namespace utility {

namespace {

std::shared_ptr<void> allocateUri()
{
    std::unique_ptr< ::UriUriA> tmp(new ::UriUriA());
    std::memset(tmp.get(), 0x0, sizeof(::UriUriA));

    return std::shared_ptr<void>(tmp.release(), [](::UriUriA *uri)
    {
        if (uri) { ::uriFreeUriMembersA(uri); }
        delete uri;
    });
}

::UriUriA* getUri(std::shared_ptr<void> &uri)
{
    return static_cast< ::UriUriA*>(uri.get());
}

const ::UriUriA* getUri(const std::shared_ptr<void> &uri)
{
    return static_cast< ::UriUriA*>(uri.get());
}

} // namespace

Uri::Uri(const std::string &uriString)
    : uri_(allocateUri()), port_()
{
    ::UriParserStateA state;
    state.uri = getUri(uri_);
    if (::uriParseUriA(&state, uriString.c_str()) != URI_SUCCESS) {
        LOGTHROW(err1, std::runtime_error)
            << "Cannot parse uri <" << uriString << ">";
    }

    host_.assign(state.uri->hostText.first, state.uri->hostText.afterLast);
    if (state.uri->portText.first != state.uri->portText.afterLast) {
        port_ = std::stoi(std::string(state.uri->portText.first
                                      , state.uri->portText.afterLast));
    }
}

bool Uri::absolutePath() const
{
    if (auto uri = getUri(uri_)) { return uri->absolutePath; }
    return false;
}

boost::filesystem::path Uri::path() const
{
    auto uri(getUri(uri_));
    if (!uri) { return {}; }

    boost::filesystem::path out;
    if (uri->absolutePath) { out /= "/"; }

    for (auto segment(uri->pathHead); segment; segment = segment->next) {
        out.append(segment->text.first, segment->text.afterLast);
    }

    return out;
}

boost::filesystem::path Uri::path(std::size_t index, bool absolutize) const
{
    auto uri(getUri(uri_));
    if (!uri) { return {}; }

    boost::filesystem::path out;

    for (auto segment(uri->pathHead); segment; segment = segment->next) {
        if (index > 0) { --index; continue; }
        if (absolutize) { out /= "/"; absolutize = false; }
        out.append(segment->text.first, segment->text.afterLast);
    }

    return out;
}

std::string Uri::pathComponent(std::size_t index) const
{
    auto uri(getUri(uri_));
    if (!uri) { return {}; }

    for (auto segment(uri->pathHead); segment;
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
    auto uri(getUri(uri_));
    if (!uri) { return {}; }

    std::size_t count;
    for (auto segment(uri->pathHead); segment; segment = segment->next) {
        ++count;
    }
    return count;
}

} // utility
