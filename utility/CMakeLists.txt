# bump version here
set(utility_VERSION 1.42)

set(utility_DEPENDS
  dbglog>=1.7
  Boost_THREAD
  Boost_PROGRAM_OPTIONS
  Boost_FILESYSTEM

  # we need pthread_* stuff
  THREADS
)

if(MYSQL++_FOUND)
  message(STATUS "utility: compiling in MySQL++ support")

  list(APPEND utility_DEPENDS MYSQL++)
  list(APPEND utility_DEFINITIONS UTILITY_HAS_MYSQL=1)

  set(utility_MYSQL++_SOURCES mysqldb.hpp mysqldb.cpp)
else()
  message(STATUS "utility: compiling without MySQL++ support")
endif()

if(LIBPROC_FOUND)
  message(STATUS "utility: compiling in libproc support")

  list(APPEND utility_DEPENDS LIBPROC)
  list(APPEND utility_DEFINITIONS UTILITY_HAS_PROC=1)

  set(utility_LIBPROC_SOURCES procstat.hpp procstat.cpp)
else()
  message(STATUS "utility: compiling without libproc support")
endif()

if(MAGIC_FOUND)
  message(STATUS "utility: compiling in magic support")

  list(APPEND utility_DEPENDS MAGIC)

  set(utility_MAGIC_SOURCES magic.hpp magic.cpp)
else()
  message(STATUS "utility: compiling without magic support")
endif()

if(Boost_IOSTREAMS_FOUND)
  message(STATUS "utility: compiling in boost iostreams support")

  list(APPEND utility_DEPENDS Boost_IOSTREAMS)
  list(APPEND utility_DEFINITIONS UTILITY_HAS_BOOST_IOSTREAMS=1)

  set(utility_IOSTREAMS_SOURCES
    substream.hpp substream.cpp
    zip.hpp zip.cpp)
else()
  message(STATUS "utility: compiling without boost iostreams support")
endif()

if(ICU_FOUND)
  message(STATUS "utility: compiling in ICU support")

  list(APPEND utility_DEPENDS ICU)
  list(APPEND utility_DEFINITIONS UTILITY_HAS_ICU=1)

  set(utility_ICU_SOURCES
    path.icu.cpp)

else()
  message(STATUS "utility: compiling without ICU support")
endif()

define_module(LIBRARY utility=${utility_VERSION}
  DEPENDS ${utility_DEPENDS}
  DEFINITIONS ${utility_DEFINITIONS})

set(utility_SOURCES
  duration.hpp
  time.hpp # implementation is system dependent
  steady-clock.hpp
  gl.hpp
  has_member.hpp
  map.hpp
  small_list.hpp
  small_set.hpp
  small_map.hpp
  streams.hpp detail/streams.hpp
  utility.cpp
  config.hpp detail/config.hpp detail/config.cpp
  multivalue.hpp detail/multivalue.hpp
  scopedguard.hpp
  interprocess.hpp
  runnable.hpp runnable.cpp
  ctrlcommand.hpp
  iohelpers.hpp

  uri.hpp uri.cpp
  parse.hpp
  base64.hpp
  md5.hpp

  format.hpp
  raise.hpp
  expect.hpp
  logging.hpp

  path.hpp path.cpp # implementation is system-dependent
  filesystem.hpp filesystem.cpp # some implementation is system-dependent
  detail/filesystem.hpp detail/filesystem.cpp
  tcpendpoint.hpp tcpendpoint-io.hpp tcpendpoint-io.cpp
  udpendpoint.hpp udpendpoint-io.hpp udpendpoint-io.cpp
  detail/iface.hpp
  meminfo.hpp
  cpuinfo.hpp cpuinfo.cpp

  guarded-call.hpp

  naturalsort.hpp

  progress.hpp progress.cpp
  environment.hpp

  lrucache.hpp
  limits.hpp

  hostname.hpp # implementation is system dependent

  openmp.hpp

  atomic.hpp

  algorithm.hpp

  filedes.hpp # implementation is system dependent

  premain.hpp

  thread.hpp thread.cpp

  resourcefetcher.hpp
  httpcode.hpp httpcode.cpp
  httpquery.hpp httpquery.cpp

  typeinfo.hpp typeinfo.cpp

  archive-loading.hpp

  implicit-value.hpp

  eventcounter.hpp eventcounter.cpp

  gccversion.hpp
  cppversion.hpp

  getenv.hpp # deprecated
  env.hpp

  memoryfile.hpp

  tar.hpp tar.cpp

  unistd_compat.hpp

  po.hpp po.cpp

  vercmp.hpp vercmp.cpp

  uncaught-exception.hpp uncaught-exception.cpp

  hostname.hpp # implementation is system dependent

  iothreads.hpp iothreads.cpp
  )

# system-dependent stuff
if(CMAKE_SYSTEM_NAME MATCHES Linux)
  message(STATUS "utility: adding linux-specific sources")
  list(APPEND utility_SOURCES
    detail/iface.linux.cpp
    detail/meminfo.linux.cpp
    detail/path.linux.cpp
    detail/path.posix.cpp
    detail/rlimit.linux.cpp
    detail/filesystem.linux.cpp
    glob.hpp glob.cpp
    )
  if(BUILDSYS_EMBEDDED)
    list(APPEND utility_SOURCES
      detail/memoryfile.unsupported.cpp
      )
  else()
    list(APPEND utility_SOURCES
      detail/memoryfile.linux.cpp
      )
  endif()
elseif(CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES iOS)
  message(STATUS "utility: adding darwin- or ios-specific sources")
  list(APPEND utility_SOURCES
    detail/iface.unsupported.cpp
    detail/path.posix.cpp
    detail/path.unsupported.cpp
    detail/rlimit.unsupported.cpp
    detail/filesystem.linux.cpp
    detail/memoryfile.unsupported.cpp
    )
elseif(WIN32)
  message(STATUS "utility: adding windows-specific sources")
  list(APPEND utility_SOURCES
    detail/iface.unsupported.cpp
    detail/path.windows.cpp
    detail/rlimit.windows.cpp
    detail/filesystem.windows.cpp
    detail/memoryfile.unsupported.cpp
    )
else()
  message(STATUS "utility: adding sources for unsupported system")
  list(APPEND utility_SOURCES
    detail/iface.unsupported.cpp
    detail/path.unsupported.cpp
    detail/rlimit.unsupported.cpp
    detail/filesystem.unsupported.cpp
    detail/memoryfile.unsupported.cpp
    )
endif()

# system-dependent stuff: posix/windows
if(WIN32)

  # windows
  list(APPEND utility_SOURCES
    detail/time.windows.cpp
    detail/filedes.windows.cpp
    detail/limits.unsupported.cpp
    detail/unistd_compat.windows.cpp
    detail/hostname.windows.cpp
    )

else()

  # posix
  list(APPEND utility_SOURCES
    process.hpp process.cpp detail/process.hpp detail/redirectfile.hpp
    atfork.hpp atfork.cpp
    atexit.hpp atexit.cpp
    lockfile.hpp lockfile.cpp
    identity.hpp identity.cpp
    persona.hpp switchpersona.cpp

    detail/limits.posix.cpp
    detail/time.posix.cpp
    detail/filedes.posix.cpp
    detail/hostname.linux.cpp
    )
endif()

if(C++_VERSION VERSION_GREATER_EQUAL 14)
  list(APPEND utility_SOURCES
    po-alias.hpp po-alias.cpp
    )
endif()

add_library(utility STATIC
  ${utility_SOURCES}
  ${utility_MYSQL++_SOURCES}
  ${utility_LIBPROC_SOURCES}
  ${utility_MAGIC_SOURCES}
  ${utility_IOSTREAMS_SOURCES}
  ${utility_ICU_SOURCES}
  )
buildsys_library(utility)
target_link_libraries(utility ${MODULE_LIBRARIES})
target_compile_definitions(utility PRIVATE ${MODULE_DEFINITIONS})

# fallback to pre C++11 compiler for Boost.Filesystem operations
if (${CMAKE_C_COMPILER_ID} MATCHES GNU)
  set_source_files_properties(detail/filesystem.cpp PROPERTIES
    COMPILE_FLAGS -std=c++98)
endif()

# test stuff
if (COMPILE_TESTS)
  add_subdirectory(test-copytree)
  add_subdirectory(test-base64)
  add_subdirectory(test-utility)
endif()

if(NOT BUILDSYS_EMBEDDED)
  add_subdirectory(test-uri EXCLUDE_FROM_ALL)
endif()

if(LIBPROC_FOUND)
  add_subdirectory(test-procstat EXCLUDE_FROM_ALL)
endif()
