define_module(BINARY test-copytree
  DEPENDS utility
)

set(test-copytree_SOURCES
  main.cpp
  )

add_executable(test-copytree ${test-copytree_SOURCES})
target_link_libraries(test-copytree ${MODULE_LIBRARIES})
buildsys_binary(test-copytree)
