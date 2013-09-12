define_module(LIBRARY utility DEPENDS dbglog Boost_THREAD Boost_PROGRAM_OPTIONS
  # we need pthread_* stuff
  THREADS
)

set(utility_SOURCES
  duration.hpp
  gl.hpp
  has_member.hpp
  map.hpp
  small_map.hpp
  streams.hpp detail/streams.hpp
  utility.cpp
  config.hpp detail/config.hpp detail/config.cpp
  multivalue.hpp detail/multivalue.hpp
  scopedguard.hpp
  process.hpp process.cpp
  interprocess.hpp
  runnable.hpp runnable.cpp

  uri.hpp
  parse.hpp
  base64.hpp

  filesystem.hpp detail/filesystem.hpp detail/filesystem.cpp
  tcpendpoint.hpp tcpendpoint-io.hpp tcpendpoint-io.cpp
  detail/iface.hpp

  lockfile.hpp lockfile.cpp

  naturalsort.hpp
  )

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND utility_SOURCES
    detail/iface.linux.cpp
    )
else()
  list(APPEND utility_SOURCES
    detail/iface.unsupported.cpp
    )
endif()

add_library(utility STATIC ${utility_SOURCES})
buildsys_library(utility)
target_link_libraries(utility ${MODULE_LIBRARIES})

# fallback to pre C++11 compiler for Boost.Filesystem operations
if (${CMAKE_C_COMPILER_ID} MATCHES GNU)
  set_source_files_properties(detail/filesystem.cpp PROPERTIES
    COMPILE_FLAGS -std=c++98)
endif()
