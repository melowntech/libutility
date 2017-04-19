/**
 * Copyright (c) 2017 Melown Technologies SE
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
#ifndef utility_resourcefetcher_hpp_included_
#define utility_resourcefetcher_hpp_included_

#include <ctime>
#include <string>
#include <exception>
#include <system_error>
#include <future>

#include "./uri.hpp"
#include "./supplement.hpp"
#include "./errorcode.hpp"

namespace utility {

/** Fetches resource(s) from a URL(s).
 */
class ResourceFetcher
{
public:
    /** Query with a reply.
     */
    class Query : public utility::Supplement<Query> {
    public:
        struct Body {
            std::time_t lastModified;
            std::time_t expires;
            bool redirect;
            std::string contentType;
            std::string data;

            Body() : lastModified(-1), expires(-1), redirect(false) {}
        };

        Query(Query&&) = default;
        Query(const Query&) = default;

        Query() : empty_(true), followRedirects_(true) {}
        Query(const std::string &location, bool followRedirects = true);


        void assign(const std::string &location, bool followRedirects = true);

        bool reuse() const { return reuse_; }
        Query& reuse(bool reuse) { reuse_ = reuse; return *this; }

        /** Total timeout for this query in ms. Zero or negative means no
         *  timeout.
         */
        long timeout() const { return timeout_; }
        /** Total timeout for this query in ms. Zero or negative means no
         *  timeout.
         */
        Query& timeout(long timeout) { timeout_ = timeout; return *this; }

        void set(std::time_t lastModified, std::time_t expires
                 , const void *data, std::size_t size
                 , const std::string &contentType);

        void redirect(const std::string &url);

        void error(std::exception_ptr exc) { exc_ = std::move(exc); }

        void error(std::error_code ec) {ec_ = std::move(ec); }

        const Body& get() const;

        Body&& moveOut();

        bool check(const std::error_code &ec) const { return (ec_ == ec); }

        const std::error_code& ec() const { return ec_; }

        const std::exception_ptr& exc() const { return exc_; }

        /** Nonthrowing body getter.
         */
        template <typename Sink> const Body* get(Sink &sink) const;

        const std::string& location() const { return location_; }

        operator bool() const { return !empty_; }

        bool valid() const;

        typedef std::vector<Query> list;

    private:
        bool empty_;
        std::string location_;
        Body body_;
        std::exception_ptr exc_;
        std::error_code ec_;
        bool followRedirects_;
        bool reuse_;

        long timeout_;
    };

    class MultiQuery : public utility::Supplement<MultiQuery> {
    public:
        MultiQuery() {}
        MultiQuery(std::size_t expected) { queries_.reserve(expected); }
        MultiQuery(Query query) { queries_.push_back(std::move(query)); }
        MultiQuery(Query::list query) : queries_(std::move(query)) {}
        MultiQuery(MultiQuery&&) = default;

        Query& add(Query query) {
            queries_.push_back(std::move(query));
            return queries_.back();
        }

        void addIfValid(Query query) {
            if (query) { queries_.push_back(std::move(query)); }
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
        const Query& back() const { return queries_.back(); }
        Query& back() { return queries_.back(); }
        const Query& operator[](int i) const { return queries_[i]; }
        Query& operator[](int i) { return queries_[i]; }

        operator bool() const { return !empty(); }

    private:
        Query::list queries_;
    };

    typedef std::function<void(MultiQuery &&query)> Done;

    virtual ~ResourceFetcher() {}

    /** Perform single-query and fill in respons.
     */
    void perform(Query query, const Done &done) const;

    /** Perform multi-query and fill in responses.
     */
    void perform(MultiQuery query, const Done &done) const;

    /** Blocking query operation.
     */
    Query perform(Query query) const;

    /** Blocking multi-query operation.
     */
    MultiQuery perform(MultiQuery query) const;

private:
    virtual void perform_impl(MultiQuery query, const Done &done)
        const = 0;
};

// inlines

inline void ResourceFetcher::perform(Query query, const Done &done)
    const
{
    perform_impl({std::move(query)}, done);
}

inline void ResourceFetcher::perform(MultiQuery query, const Done &done)
    const
{
    perform_impl(std::move(query), done);
}

inline ResourceFetcher::Query::Query(const std::string &location
                                     , bool followRedirects)
    : empty_(false), location_(location), exc_(), ec_()
    , followRedirects_(followRedirects)
    , reuse_(true)
{}

inline void ResourceFetcher::Query::assign(const std::string &location
                                           , bool followRedirects)
{
    empty_ = false;
    location_ = location;
    exc_ = {};
    ec_ = {};
    followRedirects_ = followRedirects;
    reuse_ = true;
}

inline void ResourceFetcher::Query::set(std::time_t lastModified
                                        , std::time_t expires
                                        , const void *data, std::size_t size
                                        , const std::string &contentType)
{
    body_.lastModified = lastModified;
    body_.expires = expires;
    body_.contentType = contentType;
    body_.data.assign(static_cast<const char*>(data), size);
    body_.redirect = false;
    exc_ = {};
    ec_ = {};
}

inline void ResourceFetcher::Query::redirect(const std::string &url)
{
    body_.lastModified = body_.expires = -1;
    body_.data = url;
    body_.contentType.clear();
    body_.redirect = true;
    exc_ = {};
    ec_ = {};
}

inline const ResourceFetcher::Query::Body& ResourceFetcher::Query::get() const
{
    if (exc_) { std::rethrow_exception(exc_); }
    if (ec_) { throwErrorCode(ec_); }
    return body_;
}

inline ResourceFetcher::Query::Body&& ResourceFetcher::Query::moveOut()
{
    if (exc_) { std::rethrow_exception(exc_); }
    if (ec_) { throwErrorCode(ec_); }
    return std::move(body_);
}

template <typename Sink>
inline const ResourceFetcher::Query::Body*
ResourceFetcher::Query::get(Sink &sink) const
{
    if (exc_) { sink(exc_); return nullptr; }
    if (ec_) { sink(ec_); }
    return &body_;
}

inline bool ResourceFetcher::Query::valid() const
{
    return (!exc_ && !ec_);
}

namespace detail {
template <typename T>
class Promise {
public:
    Promise() {}
    Promise(const Promise &o)
        : p_(std::move(o.promise()))
    {}

    std::promise<T>& promise() const {
            return const_cast<std::promise<T>&>(p_);
    }

private:
    std::promise<T> p_;
};

} // namespace detail

inline ResourceFetcher::Query ResourceFetcher::perform(Query query) const
{

    detail::Promise<Query> promise;
    auto future(promise.promise().get_future());
    perform_impl({std::move(query)}, [promise](MultiQuery &&q)
    {
        promise.promise().set_value(std::move(q.front()));
    });

    return future.get();
}

inline ResourceFetcher::MultiQuery ResourceFetcher::perform(MultiQuery query)
    const
{
    detail::Promise<MultiQuery> promise;
    auto future(promise.promise().get_future());
    perform_impl({std::move(query)}, [promise](MultiQuery &&q)
    {
        promise.promise().set_value(std::move(q));
    });

    return future.get();
}

} // namespace utility

#endif // utility_resourcefetcher_hpp_included_
