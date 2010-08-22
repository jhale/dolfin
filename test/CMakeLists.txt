# Require CMake 2.8
cmake_minimum_required(VERSION 2.8)

# Set special link option, see `cmake --help-policy CMP0003`
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif()

# Set verbose output while testing CMake
set(CMAKE_VERBOSE_MAKEFILE 1)

# Find DOLFIN config file (not used here, but check that the demo will be able
# to find it
find_package(dolfin REQUIRED)

# If config file is found, add all demo sub-directories, else print helper
# message
if (dolfin_FOUND)

  # Build list of all cpp directories
  file( GLOB_RECURSE list "test.cpp")
  list( SORT list )
  string(REGEX REPLACE "/test.cpp" "" list "${list}")

  # Add each C++ demo directory
  foreach( cpp_dir ${list} )
    message(STATUS "Add C++ test: ${cpp_dir}")
    add_subdirectory( ${cpp_dir} )
  endforeach()

else()

  message(STATUS "Could not locate dolfin-config.cmake file. Did you do 'make install' for the DOLFIN library and set the appropriate paths (source <build_dir>/dolfin.conf)?")

endif()

