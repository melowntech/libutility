aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../test UTILITY_TEST_SOURCES)

define_module(BINARY utility-tests
  DEPENDS utility dbglog Boost_UNIT_TEST_FRAMEWORK
  DEFINITIONS BOOST_TEST_DYN_LINK)

add_executable(utility-tests EXCLUDE_FROM_ALL
  main.cpp ${UTILITY_TEST_SOURCES})
buildsys_binary(utility-tests)
target_link_libraries(utility-tests ${MODULE_LIBRARIES})
target_compile_definitions(utility-tests PRIVATE ${MODULE_DEFINITIONS})
