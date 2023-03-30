# - Find p8g
# Find the p8g files and includes
#
# p8g_INCLUDE_DIRS - where to find p8g.hpp, etc.
# p8g_LIBRARIES - List of libraries when using p8g.
# p8g_FOUND - True if p8g found.

find_path(p8g_INCLUDE_DIRS
  NAMES p8g.hpp
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_library(p8g_LIBRARIES
  NAMES p8g
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(p8g DEFAULT_MSG p8g_LIBRARIES p8g_INCLUDE_DIRS)

mark_as_advanced(
  p8g_LIBRARIES
  p8g_INCLUDE_DIRS)

if(p8g_FOUND AND NOT (TARGET p8g::p8g))
  add_library (p8g::p8g UNKNOWN IMPORTED)
  set_target_properties(p8g::p8g
    PROPERTIES
      IMPORTED_LOCATION ${p8g_LIBRARIES}
      INTERFACE_INCLUDE_DIRECTORIES ${p8g_INCLUDE_DIRS})
endif()
