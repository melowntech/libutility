set(utility_SOURCES
)

add_library(utility STATIC ${utility_SOURCES})
set_target_properties(utility PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/lib)
