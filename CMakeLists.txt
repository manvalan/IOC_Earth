cmake_minimum_required(VERSION 3.10)
project(IOC_Earth VERSION 1.0.0 LANGUAGES CXX)

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Opzioni di compilazione
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Trova le dipendenze richieste
find_package(PkgConfig REQUIRED)

# Trova Mapnik
pkg_check_modules(MAPNIK REQUIRED libmapnik>=3.0)

# Trova Boost (richiesto da Mapnik)
# Usa il nuovo approccio di Boost che non richiede componenti specifici
find_package(Boost REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${MAPNIK_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# Link directories
link_directories(
    ${MAPNIK_LIBRARY_DIRS}
    ${Boost_LIBRARY_DIRS}
)

# Compila flags
add_compile_options(
    ${MAPNIK_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wpedantic
)

# Definisci la libreria
set(LIBRARY_SOURCES
    src/MapPathRenderer.cpp
)

set(LIBRARY_HEADERS
    include/MapPathRenderer.h
)

# Crea la libreria
add_library(ioc_earth ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

# Imposta le proprietà della libreria
set_target_properties(ioc_earth PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${LIBRARY_HEADERS}"
)

# Link delle librerie
target_link_libraries(ioc_earth
    PUBLIC
        ${MAPNIK_LIBRARIES}
        ${Boost_LIBRARIES}
)

# Compila gli esempi se richiesto
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installazione
include(GNUInstallDirs)

install(TARGETS ioc_earth
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ioc_earth
)

# Crea file di configurazione per pkg-config
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ioc_earth.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/ioc_earth.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ioc_earth.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Informazioni di build
message(STATUS "")
message(STATUS "IOC_Earth Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "  Build shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "  Mapnik version: ${MAPNIK_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
