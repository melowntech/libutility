define_module(BINARY test-base64
  DEPENDS utility
)

set(test-base64_SOURCES base64.cpp)
add_executable(test-base64 ${test-base64_SOURCES})
target_link_libraries(test-base64 ${MODULE_LIBRARIES})
buildsys_binary(test-base64)
