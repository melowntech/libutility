#include <pthread.h>

#include "dbglog/dbglog.hpp"

#include "./thread.hpp"

namespace utility { namespace thread {

void setName(const std::string &name)
{
#ifndef _GNU_SOURCE
    LOG(warn3) << "pthread_setname_np unsupported";
#else
    auto useName((name.size() > 15) ? name.substr(0, 15) : name);
    if (::pthread_setname_np(::pthread_self(), useName.c_str())) {
        LOG(warn3) << "pthread_setname_np failed";
    } else {
        LOG(info4) << "set name to: <" << useName << ">";
    }
#endif
}

void appendName(const std::string &name)
{
#ifndef _GNU_SOURCE
    LOG(warn3) << "pthread_setname_np unsupported";
#else
    // fetch original name
    char buf[16];
    if (::pthread_getname_np(::pthread_self(), buf, sizeof(buf))) {
        LOG(warn3) << "pthread_getname_np failed";
        return;
    }

    auto useName(buf + name);
    if (useName.size() > 15) { useName = useName.substr(0, 15); }

    if (::pthread_setname_np(::pthread_self(), useName.c_str())) {
        LOG(warn3) << "pthread_setname_np failed";
    }
#endif
}

} } // namespace utility::thread
