cmake_minimum_required(VERSION 3.2)

# Set the version number.
set(EXECSVR_VERSION_MAJOR 0)
set(EXECSVR_VERSION_MINOR 3)
set(EXECSVR_VERSION_REVISION 1)
set(EXECSVR_VERSION "${EXECSVR_VERSION_MAJOR}.${EXECSVR_VERSION_MINOR}.${EXECSVR_VERSION_REVISION}")

set(VER 1.00)
set(MACRO "-D DEF_USE_SSL")

set(exectype 0)

# find module
find_package(Threads REQUIRED)
#find_package(ZLIB    REQUIRED)
# sudo apt install zlib1g-de
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

#find_package(RdKafka REQUIRED)
#    message(STATUS "Build will generate a static library.")
#    set(CPPKAFKA_LIBRARY_TYPE STATIC)
#    add_definitions("-DCPPKAFKA_STATIC=1")
#    add_definitions("-DLIBRDKAFKA_STATICLIB")

# -Og -g
set(CMAKE_CXX_STANDARD 14)
if (exectype)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -g ${MACRO}")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2 -shared ${MACRO}")
endif()
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

set(EXECSVR_INSTALL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(EXECSVR_INSTALL_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(EXECSVR_INSTALL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

#include_directories(${EXECSVR_INSTALL_INCLUDE_DIR})

if (exectype)
    SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
else()	
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
endif()

set(EXEC_NAME http-keeper-${VER})
add_subdirectory(src)

