#ifndef fetcher_db_hpp_included_
#define fetcher_db_hpp_included_

#include <ctime>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <exception>
#include <ostream>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

#include <mysql++.h>

#include "dbglog/dbglog.hpp"

class Db : boost::noncopyable {
public:
    typedef boost::optional<Db> handle;
    typedef mysqlpp::Connection Connection;
    typedef mysqlpp::Query Query;
    typedef mysqlpp::SimpleResult SimpleResult;
    typedef mysqlpp::StoreQueryResult StoreQueryResult;

    struct Parameters {
        std::string database;
        std::string host;
        std::string user;
        std::string password;
        unsigned int port;

        Parameters() : port(0) {}

        void configuration(const std::string &section
                           , boost::program_options::options_description
                           &config);

        void configure(const boost::program_options::variables_map &) {}

        template <typename E, typename T>
        std::basic_ostream<E, T>& dump(std::basic_ostream<E, T> &os
                                       , const std::string &section = "")
            const;
    };

    struct QueryError : std::runtime_error {
        QueryError(const mysqlpp::Exception &e, const std::string &query);
        std::exception_ptr exc;
    };

    /** Create DB connection. */
    Db(const Parameters &params);

    ~Db();

    inline const Parameters& params() const { return params_; }

    /** Connect to database if not connected yet and return
     * underlying database connection. */
    inline Connection& conn() { return conn_; }

    inline Query query() { return conn_.query(); }

    template <typename T>
    inline Query query(const T &t) { return conn_.query(t); }

    SimpleResult execute(Query &query);

    StoreQueryResult store(Query &query);

    static void prepareConfig(boost::program_options::options_description
                              &config
                              , const std::string &rootRef = "cwd");

    static Parameters
    fromConfig(const boost::program_options::variables_map &vars
               , boost::filesystem::path root = boost::filesystem::path()
               , boost::filesystem::path *path = nullptr);

    static void configHelp(std::ostream &out);

private:
    const Parameters params_;
    Connection conn_;
};

template <typename T>
boost::optional<T> optional(const mysqlpp::String &s)
{
    if (s.is_null()) {
        return boost::none;
    }
    return T(s);
}

// inline method implementation

inline Db::QueryError::QueryError(const mysqlpp::Exception &e
                                  , const std::string &query)
    : std::runtime_error
      (str(boost::format("Query execution failed: <%s>; query: %s")
           % e.what() % query))
    , exc(std::make_exception_ptr(e))
{}

inline Db::SimpleResult Db::execute(Query &query) {
    try {
        LOG(info1) << "Executing query: " << query;
        return query.execute();
    } catch (const mysqlpp::Exception &e) {
        QueryError qe(e, query.str());
        LOG(err2) << qe.what();
        throw qe;
    }
}

inline Db::StoreQueryResult Db::store(Query &query) {
    try {
        LOG(info1) << "Executing query: " << query;
        return query.store();
    } catch (const mysqlpp::Exception &e) {
        QueryError qe(e, query.str());
        LOG(err2) << qe.what();
        throw qe;
    }
}

template <typename E, typename T>
std::basic_ostream<E, T>&
Db::Parameters::dump(std::basic_ostream<E, T> &os, const std::string &section)
    const
{
    // NB: do not print password!
    return os << section << "database = " << database << '\n'
              << section << "host = " << host << '\n'
              << section << "user = " << user << '\n'
              << section << "port = " << port << '\n';
}

#endif // fetcher_db_hpp_included_

