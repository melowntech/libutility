#ifndef utility_resourcefetcher_hpp_included_
#define utility_resourcefetcher_hpp_included_

#include <ctime>
#include <string>
#include <exception>

#include "./uri.hpp"
#include "./supplement.hpp"

namespace utility {

/** Fetches resource(s) from a URL(s).
 */
class ResourceFetcher
{
public:
    /** Query with a reply.
     */
    class Query : public utility::Supplement {
    public:
        struct Body {
            std::time_t lastModified;
            std::time_t expires;
            bool redirect;
            std::string contentType;
            std::string data;

            Body() : lastModified(-1), expires(-1), redirect(false) {}
        };

        Query() : empty_(true), followRedirects_(true) {}

        Query(const std::string &location, bool followRedirects = true)
            : empty_(false), location_(location), exc_()
            , followRedirects_(followRedirects)
        {}

        void assign(const std::string &location, bool followRedirects = true) {
            empty_ = false;
            location_ = location;
            exc_ = {};
            followRedirects_ = followRedirects;
        }

        void set(std::time_t lastModified, std::time_t expires
                 , const void *data, std::size_t size
                 , const std::string &contentType)
        {
            body_.lastModified = lastModified;
            body_.expires = expires;
            body_.contentType = contentType;
            body_.data.assign(static_cast<const char*>(data), size);
            body_.redirect = false;
            exc_ = {};
        }

        void redirect(const std::string &url) {
            body_.lastModified = body_.expires = -1;
            body_.data = url;
            body_.contentType.clear();
            body_.redirect = true;
            exc_ = {};
        }

        void error(std::exception_ptr exc) {
            exc_ = exc;
        }

        const Body& get() const {
            if (exc_) { std::rethrow_exception(exc_); }
            return body_;
        }

        const std::string& location() const { return location_; }

        operator bool() const { return !empty_; }

        typedef std::vector<Query> list;

    private:
        bool empty_;
        std::string location_;
        Body body_;
        std::exception_ptr exc_;
        bool followRedirects_;
    };

    class MultiQuery : public utility::Supplement {
    public:
        MultiQuery() {}
        MultiQuery(std::size_t expectedSize) { queries_.reserve(expectedSize); }
        MultiQuery(const Query &query) : queries_{query} {}
        MultiQuery(const Query::list &query) : queries_(query) {}

        Query& add(const Query &query) {
            queries_.push_back(query);
            return queries_.back();
        }

        template <typename ...Args>
        Query& add(Args &&...args) {
            queries_.emplace_back(std::move(args)...);
            return queries_.back();
        }

        const Query::list& queries() const { return queries_; }

        Query::list::iterator begin() { return queries_.begin(); }
        Query::list::iterator end() { return queries_.end(); }

        Query::list::const_iterator begin() const {
            return queries_.begin();
        }
        Query::list::const_iterator end() const {
            return queries_.end();
        }

        bool empty() const { return queries_.empty(); }
        Query::list::size_type size() const { return queries_.size(); }
        const Query& front() const { return queries_.front(); }
        Query& front() { return queries_.front(); }

    private:
        Query::list queries_;
    };

    typedef std::function<void(const MultiQuery &query)> Done;

    virtual ~ResourceFetcher() {}

    /** Perform single-query and fill in respons.
     */
    void perform(const Query &query, const Done &done) const;

    /** Perform multi-query and fill in responses.
     */
    void perform(const MultiQuery &query, const Done &done) const;

private:
    virtual void perform_impl(const MultiQuery &query, const Done &done)
        const = 0;
};

// inlines

inline void ResourceFetcher::perform(const Query &query, const Done &done)
    const
{
    perform_impl({query}, done);
}

inline void ResourceFetcher::perform(const MultiQuery &query
                                     , const Done &done)
    const
{
    perform_impl(query, done);
}

} // namespace utility

#endif // utility_resourcefetcher_hpp_included_
