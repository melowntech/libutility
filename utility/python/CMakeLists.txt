define_module(LIBRARY pyutility=${MODULE_utility_VERSION}
  DEPENDS
  utility=${MODULE_utility_VERSION}
  pysupport>=1.4
  Boost_THREAD Boost_SYSTEM)


set(pyutility_SOURCES
  utilitymodule.cpp
  )

py_to_pyc(pyutility_SOURCES utility::py::importsupport importsupport.py)

add_library(pyutility STATIC ${pyutility_SOURCES})
buildsys_library(pyutility)
target_link_libraries(pyutility ${MODULE_LIBRARIES})
