#include <mysql/mysqld_error.h>

#include <functional>

#include <boost/filesystem.hpp>

#include "utility/config.hpp"
#include "dbglog/dbglog.hpp"

#include "mysqldb.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace utility { namespace mysql {

void Db::Parameters::configuration(const std::string &section
                                   , po::options_description &config)
{
    config.add_options()
        ((section + "database").c_str()
         , po::value(&database)
         , "Name of database to select upon connection.")
        ((section + "host").c_str()
         , po::value(&host)->default_value(host)
         , ("Specifies the IPC method and parameters for "
            "contacting the server."))
        ((section + "user").c_str()
         , po::value(&user)->default_value(user)
         , ("User name to log in under, or \"\" to use the user name this "
            "program is running under."))
        ((section + "password").c_str()
         , po::value(&password)
         , "Password to use when logging in.")
        ((section + "port").c_str()
         , po::value(&port)->default_value(port)
         , ("TCP port number database server is listening on, "
            "or 0 to use default value; note that you may also give this as "
            "part of the server parameter."))

        ((section + "connectTimeout").c_str()
         , po::value(&connectTimeout)
         , "Connect timeout (-1 to use default).")
        ((section + "readTimeout").c_str()
         , po::value(&readTimeout)
         , "Read timeout (-1 to use default).")
        ((section + "writeTimeout").c_str()
         , po::value(&writeTimeout)
         , "Write timeout (-1 to use default).")
        ;
}

Db::Db(const Parameters &params)
    : params_(params)
    , conn_()
{
    conn_.set_option(new mysqlpp::MultiStatementsOption(true));
    if (params_.connectTimeout) {
        conn_.set_option
            (new mysqlpp::ConnectTimeoutOption(params_.connectTimeout));
    }
    if (params_.readTimeout) {
        conn_.set_option
            (new mysqlpp::ReadTimeoutOption(params_.readTimeout));
    }
    if (params_.writeTimeout) {
        conn_.set_option
            (new mysqlpp::WriteTimeoutOption(params_.writeTimeout));
    }

    conn_.connect(params_.database.c_str()
                  , (params_.host.empty() ? nullptr : params_.host.c_str())
                  , (params_.user.empty() ? nullptr : params_.user.c_str())
                  , params_.password.c_str()
                  , params_.port);

    LOG(info2) << "Connected to database <" << params_.database
               << "> at <" << params_.host << "> as <"
               << params_.user << ">.";

    conn_.query("SET NAMES utf8").exec();
    conn_.query("SET sql_mode='STRICT_TRANS_TABLES'").exec();
    conn_.query("SET AUTOCOMMIT=0").exec();
}

Db::~Db()
{
    LOG(info2) << "Disconnected from database <" << params_.database
               << "> at <" << params_.host << ">.";
}

void Db::prepareConfig(po::options_description &config
                       , const std::string &rootRef)
{
    config.add_options()
        ("dbconf", po::value<fs::path>()->required()
         , ("Path to DB configuration (relative path is appended to "
            + rootRef + ").").c_str())
        ;
}

void Db::configHelp(std::ostream &out)
{
    Parameters dbconf;
    po::options_description config("dbconf configuration file");
    dbconf.configuration("db.", config);
    out << config;
}

Db::Parameters Db::fromConfig(const po::variables_map &vars, fs::path root
                              , fs::path *outPath)
{
    // read db configuration
    Db::Parameters dbconf;
    fs::path path(vars["dbconf"].as<fs::path>());
    if (!path.is_absolute()) {
        path = absolute(path, root);
    }

    if (outPath) { *outPath = path; }
    utility::readConfig(path, dbconf, "db.");
    return dbconf;
}

void restartOnDeadlock(const std::function<void()> &f)
{
    for (;;) {
        try {
            f();
            return;
        } catch (const utility::mysql::Db::QueryError &exc) {
            // rethrow inner mysqlpp exception
            try {
                // rethrow nested exception
                std::rethrow_if_nested(exc);
                // if not nested -> rethrow current exception
                throw;
            } catch (const mysqlpp::BadQuery &e) {
                switch (e.errnum()) {
                case ER_LOCK_WAIT_TIMEOUT:
                case ER_LOCK_DEADLOCK:
                    break;

                default:
                    throw;
                }
                // deadlock -> try again
                LOG(warn3)
                    << "Deadlock while executing query: <"
                    << exc.what() << ">; retrying.";
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
}

} } // namespace utility::mysql
