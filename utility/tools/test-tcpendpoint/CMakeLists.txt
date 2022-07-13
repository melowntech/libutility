# used indirectly by module service (from libservice repo)

define_module(BINARY utility-tcpendpoint
  DEPENDS service utility
  )

set(utility-tcpendpoint_SOURCES
  tcpendpoint.cpp
  )

add_executable(utility-tcpendpoint ${utility-tcpendpoint_SOURCES})
target_link_libraries(utility-tcpendpoint ${MODULE_LIBRARIES})
buildsys_binary(utility-tcpendpoint)
