define_module(BINARY utility-testuri
  DEPENDS utility
)

add_executable(utility-testuri testuri.cpp)
target_link_libraries(utility-testuri ${MODULE_LIBRARIES})
target_compile_definitions(utility-testuri PRIVATE ${MODULE_DEFINITIONS})
buildsys_binary(utility-testuri)
