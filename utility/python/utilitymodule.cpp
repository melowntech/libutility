/**
 * Copyright (c) 2018 Melown Technologies SE
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

#include <cstring>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/slice.hpp>
#include <boost/python/call.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/scope.hpp>

#include <stdint.h>

#include "dbglog/dbglog.hpp"

#include "pysupport/package.hpp"
#include "pysupport/class.hpp"
#include "pysupport/hasattr.hpp"
#include "pysupport/iostreams.hpp"
#include "pysupport/dump.hpp"
#include "pysupport/load.hpp"

#include "../mysqldb.hpp"
#include "../memoryfile.hpp"

#include "importsupport.py.pyc.hpp"

namespace fs = boost::filesystem;
namespace bp = boost::python;

namespace utility { namespace py {

bp::object mysqlModule;
bp::object mysql_cursorsModule;

const utility::mysql::Db::Parameters
Parameters_fromConfig(const fs::path &path
                      , const fs::path &root = fs::path())
{
    return utility::mysql::Db::fromConfig(path, root);
}

BOOST_PYTHON_FUNCTION_OVERLOADS
(Parameters_fromConfig_overloads, Parameters_fromConfig, 1, 2)

std::string Parameters_configHelp0()
{
    std::ostringstream os;
    utility::mysql::Db::configHelp(os);
    return os.str();
}

void Parameters_configHelp1(const pysupport::OStream::pointer &os)
{
    utility::mysql::Db::configHelp(os->ostream());
}

bp::dict
Parameters_asDict(const utility::mysql::Db::Parameters &parameters)
{
    bp::dict dict;

    dict["host"] = parameters.host;
    dict["user"] = parameters.user;
    dict["passwd"] = parameters.password;
    dict["port"] = parameters.port;
    dict["db"] = parameters.database;

    dict["connect_timeout"] = parameters.connectTimeout;

    dict["charset"] = "utf8";

    return dict;
}

bp::object Db_connect(const utility::mysql::Db::Parameters &parameters)
{
    if (!mysqlModule) {
        LOGTHROW(err1, std::runtime_error)
            << "MySQLdb module not found";
    }

    bp::list empty;
    auto options(Parameters_asDict(parameters));

    {
        std::ostringstream os;
        for (const auto &cmd : utility::mysql::Db::initCommands()) {
            os << cmd << ';';
        }

        options["init_command"] = os.str();
    }

    return mysqlModule.attr("connect")(*empty, **options);
}

class Db {
public:
    typedef utility::mysql::Db::Parameters Parameters;
    Db(const Parameters &params)
        : params_(params)
        , conn_(Db_connect(params))
    {}

    const Parameters& params() const { return params_; }

    bp::object conn() { return conn_; }

    bp::object cursor() {
        return conn_.attr("cursor")();
    }

    bp::object dictCursor() {
        if (!mysql_cursorsModule) {
            LOGTHROW(err1, std::runtime_error)
                << "MySQLdb.cursors module not found";
        }

        return conn_.attr("cursor")
            (bp::object(mysql_cursorsModule.attr("DictCursor")));
    }

private:
    Parameters params_;
    bp::object conn_;
};

struct MemoryFileFlag {};

std::shared_ptr<utility::Filedes>
memoryFile(const std::string &name, int flags)
{
    return std::make_shared<utility::Filedes>
        (utility::memoryFile(name, flags));
}

} } // namespace utility::py

BOOST_PYTHON_MODULE(melown_utility_mysql)
{
    using namespace bp;
    namespace py = utility::py;

    const return_internal_reference<> InternalRef;

    auto Db = class_<py::Db>
        ("Db", init<const py::Db::Parameters&>())
        .def("fromConfig", &py::Parameters_fromConfig
             , py::Parameters_fromConfig_overloads())
        .def("configHelp", &py::Parameters_configHelp0)
        .def("configHelp", &py::Parameters_configHelp1)
        .def("params", &py::Db::params, InternalRef)
        .def("conn", &py::Db::conn)
        .def("cursor", &py::Db::cursor)
        .def("dictCursor", &py::Db::dictCursor)
        ;

    {
        bp::scope scope(Db);
        auto Parameters = class_<py::Db::Parameters>
            ("Parameters", init<>())
            .def_readwrite("database", &py::Db::Parameters::database)
            .def_readwrite("host", &py::Db::Parameters::host)
            .def_readwrite("user", &py::Db::Parameters::user)
            .def_readwrite("password", &py::Db::Parameters::password)
            .def_readwrite("port", &py::Db::Parameters::port)
            .def_readwrite("connectTimeout"
                           , &py::Db::Parameters::connectTimeout)
            .def_readwrite("readTimeout", &py::Db::Parameters::readTimeout)
            .def_readwrite("writeTimeout", &py::Db::Parameters::writeTimeout)
            .def("asDict", &py::Parameters_asDict)
            PYSUPPORT_DUMPABLE(py::Db::Parameters)
            ;
    }

    try {
        py::mysqlModule = import("MySQLdb");
        py::mysql_cursorsModule = import("MySQLdb.cursors");
    } catch (const error_already_set&) {}
}

BOOST_PYTHON_MODULE(melown_utility)
{
    using namespace bp;
    namespace py = utility::py;

    auto Filedes = class_<utility::Filedes, std::shared_ptr<utility::Filedes>
                          , boost::noncopyable>
        ("Filedes", init<>())
        .def("fileno", &utility::Filedes::get)
        .def("path", &utility::Filedes::path
             , bp::return_value_policy<bp::return_by_value>())
        ;

    def("memoryFile", &py::memoryFile);

    auto MemoryFileFlag = class_<py::MemoryFileFlag>
        ("MemoryFileFlag", init<>())
        ;

    MemoryFileFlag.attr("closeOnExec")
        = utility::MemoryFileFlag::closeOnExec;
    MemoryFileFlag.attr("allowSealing")
        = utility::MemoryFileFlag::allowSealing;

    {
        auto importsupport
            (pysupport::load("importsupport", py::importsupport));

        scope().attr("import_extension")
            = importsupport.attr("import_extension");
        scope().attr("file_from_archive")
            = importsupport.attr("file_from_archive");
    }
}

namespace utility { namespace py {

namespace {

std::once_flag utility_mysql_onceFlag;

void injectMysqlModule(const bp::object&, const bp::object &module)
{
    std::call_once(utility_mysql_onceFlag, [&]()
    {
        pysupport::addModuleToPackage("mysql", PyInit_melown_utility_mysql()
                                      , "melown.utility", module);
        bp::import("melown.utility.mysql");
    });
}

} // namespace

PYSUPPORT_MODULE_IMPORT_CALLBACK(utility, injectMysqlModule)

} } // namespace utility::py
