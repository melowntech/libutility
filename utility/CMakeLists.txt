define_module(LIBRARY utility DEPENDS dbglog Boost_THREAD Boost_PROGRAM_OPTIONS)

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
  )

add_library(utility STATIC ${utility_SOURCES})
set_target_properties(utility PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(utility ${MODULE_LIBRARIES})
