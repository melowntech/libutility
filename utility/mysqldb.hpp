#ifndef utility_mysqldb_hpp_included_
#define utility_mysqldb_hpp_included_

#include <ctime>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <exception>
#include <ostream>
#include <thread>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

#include <mysql++.h>

#include "dbglog/dbglog.hpp"

namespace utility { namespace mysql {

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

        int connectTimeout;
        int readTimeout;
        int writeTimeout;

        Parameters()
            : port(0), connectTimeout(-1), readTimeout(-1), writeTimeout(-1)
        {}

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

template <typename Connector>
class Tx {
public:
    Tx(Connector &connector);
    Tx(Tx &&other)
        : connector_(other.connector_)
        , tx_(std::move(other.tx_))
    {
    }

    inline void commit() { tx_->commit(); }
    inline void rollback() { tx_->rollback(); }

    inline typename Connector::DbType& db() { return connector_->db(); }

    inline Db::Query query() { return connector_->db().query(); }

    inline Db::SimpleResult execute(Db::Query &query) {
        return connector_->db().execute(query);
    }

    inline Db::StoreQueryResult store(Db::Query &query) {
        return connector_->db().store(query);
    }

    inline operator mysqlpp::Transaction&() { return *tx_; }

private:
    Connector *connector_;
    std::unique_ptr<mysqlpp::Transaction> tx_;
};

template <typename Connector>
Tx<Connector>::Tx(Connector &connector)
    : connector_(&connector)
{
    for (int i(5); i; --i) {
        try {
            // connect to the db(if not connected)
            connector_->connectDb();
            tx_.reset(new mysqlpp::Transaction(connector_->dbconn(), false));

            // done
            return;
        } catch (const mysqlpp::Exception &e) {
            if (i == 1) {
                // too many failures -> fail
                throw;
            }
            LOG(warn3)
                << "Error starting transaction: <" << e.what()
                << ">; retrying.";
            connector_->closeDb();
            ::std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

template <typename Derived> struct TxProxyTraits;

template <typename Derived>
struct TxProxy
{
    typedef typename TxProxyTraits<Derived>::DbType DbType;

    Tx<TxProxy<Derived> > openTx() {
        return Tx<TxProxy<Derived> >(*this);
    }

    inline DbType& db() {
        return static_cast<Derived*>(this)->db();
    }

    inline typename Db::Connection& dbconn() {
        return static_cast<Derived*>(this)->dbconn();
    }

    void closeDb() {
        return static_cast<Derived*>(this)->closeDb();
    }

    void connectDb() {
        return static_cast<Derived*>(this)->connectDb();
    }
};

void restartOnDeadlock(const std::function<void()> &f);

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
{}

inline Db::SimpleResult Db::execute(Query &query) {
    try {
        LOG(info1) << "Executing query: " << query;
        return query.execute();
    } catch (const mysqlpp::Exception &e) {
        QueryError qe(e, query.str());
        LOG(err2) << qe.what();
        std::throw_with_nested(qe);
    }
}

inline Db::StoreQueryResult Db::store(Query &query) {
    try {
        LOG(info1) << "Executing query: " << query;
        return query.store();
    } catch (const mysqlpp::Exception &e) {
        QueryError qe(e, query.str());
        LOG(err2) << qe.what();
        std::throw_with_nested(qe);
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
              << section << "port = " << port << '\n'
              << section << "connectTimeout = " << connectTimeout << '\n'
              << section << "readTimeout = " << readTimeout << '\n'
              << section << "writeTimeout = " << writeTimeout << '\n'
        ;
}

} } // namespace utility::mysql

#endif // utility_mysqldb_hpp_included_

