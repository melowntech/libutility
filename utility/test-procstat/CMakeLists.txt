define_module(BINARY test-procstat
  DEPENDS utility
)

set(test-procstat_SOURCES
  main.cpp
  )

add_executable(test-procstat ${test-procstat_SOURCES})
target_link_libraries(test-procstat ${MODULE_LIBRARIES})
buildsys_binary(test-procstat)
