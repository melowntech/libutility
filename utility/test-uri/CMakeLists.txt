define_module(BINARY utility-testuri
  DEPENDS utility
)

add_executable(utility-testuri testuri.cpp)
target_link_libraries(utility-testuri ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(utility-testuri ${MODULE_DEFINITIONS})
buildsys_binary(utility-testuri)
