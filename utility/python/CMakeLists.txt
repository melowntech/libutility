if(MYSQL++_FOUND)
  message(STATUS "pyutility: compiling in MySQL++ support")

  list(APPEND pyutility_DEPENDS MYSQL++)
  list(APPEND pyutility_DEFINITIONS UTILITY_HAS_MYSQL=1)
else()
  message(STATUS "pyutility: compiling without MySQL++ support")
endif()

define_module(LIBRARY pyutility=${MODULE_utility_VERSION}
  DEPENDS
  ${pyutility_DEPENDS}
  utility=${MODULE_utility_VERSION}
  pysupport>=1.5
  Boost_THREAD Boost_SYSTEM
  DEFINITIONS ${pyutility_DEFINITIONS})


set(pyutility_SOURCES
  utilitymodule.cpp
  )

py_to_pyc(pyutility_SOURCES utility::py::importsupport importsupport.py)

add_library(pyutility STATIC ${pyutility_SOURCES})
buildsys_library(pyutility)
target_link_libraries(pyutility ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(pyutility ${MODULE_DEFINITIONS})
