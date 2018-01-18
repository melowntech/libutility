# used indirectly by module service (from libservice repo)

define_module(BINARY utility-zip
  DEPENDS service utility
  )

set(utility-zip_SOURCES
  zip.cpp
  )

add_executable(utility-zip ${utility-zip_SOURCES})
target_link_libraries(utility-zip ${MODULE_LIBRARIES})
buildsys_binary(utility-zip)


set(utility-lszip_SOURCES
  lszip.cpp
  )

add_executable(utility-lszip ${utility-lszip_SOURCES})
target_link_libraries(utility-lszip ${MODULE_LIBRARIES})
buildsys_binary(utility-lszip)

set(utility-unzip_SOURCES
  unzip.cpp
  )

add_executable(utility-unzip ${utility-unzip_SOURCES})
target_link_libraries(utility-unzip ${MODULE_LIBRARIES})
buildsys_binary(utility-unzip)
