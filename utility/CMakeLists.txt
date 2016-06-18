set(utility_DEPENDS dbglog Boost_THREAD Boost_PROGRAM_OPTIONS
  Boost_FILESYSTEM
  # we need pthread_* stuff
  THREADS
)

if(MYSQL++_FOUND)
  message(STATUS "utility: compiling in MySQL++ support")

  list(APPEND utility_DEPENDS MYSQL++)

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

define_module(LIBRARY utility DEPENDS ${utility_DEPENDS}
  DEFINITIONS ${utility_DEFINITIONS})

set(utility_SOURCES
  duration.hpp
  time.hpp time.cpp
  steady-clock.hpp
  gl.hpp
  has_member.hpp
  map.hpp
  small_map.hpp
  streams.hpp detail/streams.hpp
  utility.cpp
  config.hpp detail/config.hpp detail/config.cpp
  multivalue.hpp detail/multivalue.hpp
  scopedguard.hpp
  process.hpp process.cpp detail/process.hpp detail/redirectfile.hpp
  interprocess.hpp
  runnable.hpp runnable.cpp
  iohelpers.hpp

  uri.hpp uri.cpp
  parse.hpp
  base64.hpp
  md5.hpp

  format.hpp
  raise.hpp
  expect.hpp

  path.hpp path.cpp
  filesystem.hpp filesystem.cpp
  detail/filesystem.hpp detail/filesystem.cpp
  tcpendpoint.hpp tcpendpoint-io.hpp tcpendpoint-io.cpp
  detail/iface.hpp
  meminfo.hpp
  cpuinfo.hpp cpuinfo.cpp

  atfork.hpp atfork.cpp
  lockfile.hpp lockfile.cpp
  identity.hpp identity.cpp
  guarded-call.hpp

  naturalsort.hpp

  progress.hpp progress.cpp
  environment.hpp

  lrucache.hpp

  openmp.hpp

  atomic.hpp

  algorithm.hpp

  filedes.hpp filedes.cpp
  tar.hpp tar.cpp

  premain.hpp
  )

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND utility_SOURCES
    detail/iface.linux.cpp
    detail/meminfo.linux.cpp
    detail/path.linux.cpp
    detail/rlimit.linux.cpp
    )
else()
  list(APPEND utility_SOURCES
    detail/iface.unsupported.cpp
    detail/path.unsupported.cpp
    detail/rlimit.unsupported.cpp
    )
endif()

add_library(utility STATIC
  ${utility_SOURCES}
  ${utility_MYSQL++_SOURCES}
  ${utility_LIBPROC_SOURCES}
  ${utility_MAGIC_SOURCES}
  )
buildsys_library(utility)
target_link_libraries(utility ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(utility ${MODULE_DEFINITIONS})

# fallback to pre C++11 compiler for Boost.Filesystem operations
if (${CMAKE_C_COMPILER_ID} MATCHES GNU)
  set_source_files_properties(detail/filesystem.cpp PROPERTIES
    COMPILE_FLAGS -std=c++98)
endif()

# test stuff
if (COMPILE_TESTS)
  add_subdirectory(test-copytree)
  add_subdirectory(test-base64)
endif()
