# used indirectly by module service (from libservice repo)

define_module(BINARY utility-vercmp
  DEPENDS service utility
  )

set(utility-vercmp_SOURCES
  vercmp.cpp
  )

add_executable(utility-vercmp ${utility-vercmp_SOURCES})
target_link_libraries(utility-vercmp ${MODULE_LIBRARIES})
buildsys_binary(utility-vercmp)
