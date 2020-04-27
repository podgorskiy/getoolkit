cmake_minimum_required(VERSION 3.6)

project(ImageAnnotationToolkit)

option (GLFW_BUILD_EXAMPLES OFF)
option (GLFW_BUILD_TESTS OFF)
option (GLFW_INSTALL OFF)
option (USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL)

if (OPENGL_FOUND)
	message("OpenGL Correctly Found")
	include_directories(${OPENGL_INCLUDE_DIR})
else (OPENGL_FOUND)
	message("OpenGL environment missing")
endif (OPENGL_FOUND)


find_package (Python 3 COMPONENTS Interpreter Development REQUIRED)
find_package(PythonLibs 3 REQUIRED)

message("Python_FOUND:${Python_FOUND}")
message("Python_VERSION:${Python_VERSION}")
message("Python_Development_FOUND:${Python_Development_FOUND}")
message("Python_LIBRARIES:${Python_LIBRARIES}")
message("PYTHON_LIBRARY_DIR:${PYTHON_LIBRARY_DIR}")

include_directories(${Python_INCLUDE_DIRS} )

##############################################################
# Generating OpenGL profile
##############################################################
message("Executing gl3w_gen.py")
execute_process(COMMAND python gl3w_gen.py --root ${PROJECT_BINARY_DIR}/libs/gl3w WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/gl3w)


##############################################################
# Setup compiler options and configurations
##############################################################

if(MSVC)
    set(COMMON_OPTIONS /W3 /wd4996 /we4239 /we4002 /we4700 /we4305 /EHsc /MP)
    set(DEBUG_OPTIONS /MTd /Od /Zi)
    set(RELEASE_OPTIONS /MT /Ox /fp:fast /GL /GR-)

    add_definitions(-D_SCL_SECURE)
    add_definitions(-D_SECURE_SCL)
    add_definitions(-D_ALLOW_KEYWORD_MACROS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-DWINVER=0x0602)
    add_definitions(-D_WIN32_WINNT=0x0602)
    add_definitions(-DNTDDI_VERSION=0x06020000)

elseif(APPLE)
    set(COMMON_OPTIONS -DHAVE_PTHREAD -DGUID_CFUUID -lstdc++fs)
    set(DEBUG_OPTIONS -g)
    set(RELEASE_OPTIONS -Ofast )
else()
    set(COMMON_OPTIONS -DHAVE_PTHREAD  -lstdc++fs)
    set(DEBUG_OPTIONS -g)
    set(RELEASE_OPTIONS -O2)
endif()

set(DEBUG_OPTIONS ${DEBUG_OPTIONS} ${COMMON_OPTIONS})
set(RELEASE_OPTIONS ${RELEASE_OPTIONS} ${COMMON_OPTIONS})

add_compile_options(
        "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>"
        "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO")
endif()

add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)

add_definitions(-DHAVE_SSE42)

add_definitions(-D_HAS_EXCEPTIONS=0)
add_definitions(-DCPPHEAPWATCH=1)

set (CMAKE_CXX_STANDARD 14)

set (CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "-fPIC -g  ${CMAKE_C_FLAGS}")
set (CMAKE_CXX_FLAGS "-fPIC -g -fno-strict-aliasing -msse2 -msse3 -msse4 -funsafe-math-optimizations ${CMAKE_CXX_FLAGS}")

#####################################################################
# Includes
#####################################################################
include_directories(libs/glfw/include)
include_directories(libs/pybind11/include)
include_directories(libs/gl3w/include)
include_directories(libs/glm)
include_directories(${CMAKE_BINARY_DIR}/libs/gl3w/include)
include_directories(${Python_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIR})
include_directories(sources)
include_directories("${PROJECT_BINARY_DIR}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/spdlog/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/doctest/doctest)

link_directories("${PYTHON_LIBRARY_DIR}")
#####################################################################


##############################################################
# Externals
##############################################################
add_subdirectory (libs/glfw)
add_subdirectory (libs/gl3w)


#####################################################################
# Main sources
#####################################################################
file(GLOB_RECURSE SOURCES sources/*.cpp sources/*.h sources/*.c sources/*.cc)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sources/libjpeg-turbo_CU.cpp)
#####################################################################

#####################################################################
# Targets
#####################################################################
add_library(anntoolkit SHARED ${SOURCES})
#####################################################################

#####################################################################
# Linkage
#####################################################################
set(LIBRARIES rt m  stdc++fs gomp  ${PYTHON_LIBRARY} glfw gl3w   ${OPENGL_LIBRARIES} )
target_link_libraries(anntoolkit ${LIBRARIES})
SET_TARGET_PROPERTIES(anntoolkit PROPERTIES PREFIX "_")
#####################################################################
