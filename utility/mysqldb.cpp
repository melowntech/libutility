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
#include <mysql/mysqld_error.h>

#include <functional>

#include <boost/filesystem.hpp>

#include "dbglog/dbglog.hpp"
#include "./config.hpp"

#include "./mysqldb.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace utility { namespace mysql {

void Db::Parameters::configuration(const std::string &section
                                   , po::options_description &config)
{
    config.add_options()
        ((section + "database").c_str()
         , po::value(&database)->required()
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

std::vector<std::string> Db::initCommands()
{
    std::vector<std::string> init;
    init.push_back("SET NAMES utf8");
    init.push_back("SET sql_mode='STRICT_TRANS_TABLES'");
    init.push_back("SET AUTOCOMMIT=0");
    return init;
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

    for (const auto &cmd : initCommands()) {
        conn_.query(cmd).exec();
    }
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

Db::Parameters Db::fromConfig(const fs::path &configPath, const fs::path &root
                              , fs::path *outPath)
{
    auto path(fs::absolute(configPath, root));
    if (outPath) { *outPath = path; }

    // read db configuration
    Db::Parameters dbconf;
    utility::readConfig(path, dbconf, "db.");
    return dbconf;
}

Db::Parameters Db::fromConfig(const po::variables_map &vars
                              , const fs::path &root
                              , fs::path *outPath)
{
    // load db configuration from vars
    return fromConfig(vars["dbconf"].as<fs::path>(), root, outPath);
}

bool isRestartable(const mysqlpp::BadQuery &e)
{
    switch (e.errnum()) {
    case ER_LOCK_WAIT_TIMEOUT:
    case ER_LOCK_DEADLOCK:
        return true;

    default:
        return false;
    }
}

bool Db::QueryError::isRestartable() const
{
    try {
        // rethrow nested exception
        rethrow_if_nested();

        // no nested exception
    } catch (const mysqlpp::BadQuery &e) {
        // bad query! check whether it is restartable
        return mysql::isRestartable(e);
    } catch (...) {
        // some other error...
    }

    // not restartable tx
    return false;
}

bool isRestartable(const Db::QueryError &e)
{
    return e.isRestartable();
}

} } // namespace utility::mysql
