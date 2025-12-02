cmake_minimum_required(VERSION 3.14)
project(IOC_Earth VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard (Mapnik 3.1 uses C++14)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Mapnik using mapnik-config
find_program(MAPNIK_CONFIG mapnik-config REQUIRED)

execute_process(
    COMMAND ${MAPNIK_CONFIG} --cflags
    OUTPUT_VARIABLE MAPNIK_CFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${MAPNIK_CONFIG} --libs
    OUTPUT_VARIABLE MAPNIK_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${MAPNIK_CONFIG} --includes
    OUTPUT_VARIABLE MAPNIK_INCLUDES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Create the library
add_library(ioc_earth
    src/MapPathRenderer.cpp
)

# Include directories
target_include_directories(ioc_earth
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Apply Mapnik compiler flags
separate_arguments(MAPNIK_CFLAGS_LIST UNIX_COMMAND "${MAPNIK_CFLAGS}")
target_compile_options(ioc_earth
    PRIVATE
        ${MAPNIK_CFLAGS_LIST}
)

# Link libraries
separate_arguments(MAPNIK_LIBS_LIST UNIX_COMMAND "${MAPNIK_LIBS}")
target_link_libraries(ioc_earth
    PUBLIC
        ${MAPNIK_LIBS_LIST}
)

# Set library properties
set_target_properties(ioc_earth PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Optional: Create an example executable
option(BUILD_EXAMPLES "Build example executables" OFF)

if(BUILD_EXAMPLES)
    add_executable(map_renderer_example
        examples/main.cpp
    )
    target_link_libraries(map_renderer_example
        PRIVATE
            ioc_earth
    )
endif()

# Install targets
include(GNUInstallDirs)

install(TARGETS ioc_earth
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES src/MapPathRenderer.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ioc_earth
)

# Print configuration summary
message(STATUS "Mapnik found")
message(STATUS "  CFLAGS: ${MAPNIK_CFLAGS}")
message(STATUS "  LIBS: ${MAPNIK_LIBS}")
