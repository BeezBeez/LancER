cmake_minimum_required (VERSION 3.10)
set (PROJECT_NAME LancER)
project (${PROJECT_NAME} C CXX)

# Architecture specific
if (COMPILER_ID MATCHES "MSVC")
    set(MSVC_${lang}_ARCHITECTURE_ID "${ARCHITECTURE_ID}")
endif()

# Import modules 
set (DEP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${DEP_DIR}" "${DEP_DIR}/lib/cmake/")

# Set language specific
set (CMAKE_CXX_EXTENSIONS 1)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_C_STANDARD 11)

#
set(DCMAKE_SH="CMAKE_SH-NOTFOUND")
if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(GNUtoMS TRUE)
endif()

# Set source directories
set (SOURCES_LIST 
    "./include/JiviX/*.hpp"
    "./include/JiviX/*/*.hpp"
    "./include/JiviX/*/vulkan.*"
    "./include/JiviX/*/*/*.hpp"
    "./include/JiviX/*/*/vulkan.*"
    "./include/JiviX/*/*/*/*.hpp"
    "./include/JiviX/*/*/*/vulkan.*"
    "./include/JiviX/*/*/*/*/*.hpp"
    "./include/JiviX/*/*/*/*/vulkan.*"

    "./include/vulkan/*.hpp"
    "./include/vulkan/*.h"
    "./include/vma/*.hpp"
    "./include/vma/*.h"
    "./include/vkh/*.hpp"
    "./include/vkh/*.h"
    "./include/vkt2/*.hpp"
    "./include/vkt2/*.h"
    "./include/glad/*.hpp"
    "./include/glad/*.h"
    "./include/volk/*.hpp"
    "./include/volk/*.h"
    "./include/*.hpp"
    "./include/*.h"

    "./src/*.cpp"
    "./src/*.c"
    "./src/*/*.hpp"
    "./src/*/*.h"
    "./src/*/*/*.cpp"
    "./src/*/*/*.c"
    "./src/*/*.cpp"
    "./src/*/*.c"
    
    #"./src/include/*/*.hpp"
    #"./src/include/*/*.h"
    #"./src/include/*.hpp"
    #"./src/include/*.h"
    #"./src/*/*/*.hpp"
    #"./src/*/*/*.h"
    #"./src/*.hpp"
    #"./src/*.h"
)

# Find packages (Linux Prior)
find_package(Vulkan QUIET)
find_package(GLFW3 QUIET)
find_package(PkgConfig QUIET)
pkg_search_module(GLFW QUIET glfw3)
pkg_search_module(GLFW QUIET renderdoc)

# Set libs and include dirs of projects
set (LIBS glfw3)
set (LIB_DIR ${PROJECT_SOURCE_DIR} "${DEP_DIR}/lib")
set (INC_DIR 
    "${DEP_DIR}/include/JiviX/"
    "${DEP_DIR}/include/*/*/"
    "${DEP_DIR}/include/*/"
    "${DEP_DIR}/include/" 
    "${DEP_DIR}/src/" 
    "${DEP_DIR}/src/base/" 
    "${DEP_DIR}")

# Set definition
set (DEFINES
    -DNOMINMAX
    -DVK_ENABLE_BETA_EXTENSIONS
    -D_CRT_SECURE_NO_WARNINGS
    -D_USE_MATH_DEFINES
    -DGLM_FORCE_SWIZZLE
    -DGLM_FORCE_RADIANS
    -DGLM_ENABLE_EXPERIMENTAL
    -DVMA_DEDICATED_ALLOCATION
    -DGLFW_INCLUDE_NONE
    -DEXPERIMENTAL_GLTF
    -DUSE_VULKAN
)

# Set MSVC/Windows specific 
if (MSVC OR MSVS OR MSYS OR MINGW)
    set(DEFINES ${DEFINES} -DOS_WIN -DWIN32) 
endif()

# Set Linux specific
if (UNIX OR LINUX)
    set(DEFINES ${DEFINES} -DOS_LNX) 
endif()

# Vulkan Directories 
if (Vulkan_INCLUDE_DIR)
    #set (INC_DIR ${INC_DIR} ${Vulkan_INCLUDE_DIR})
endif()

# Vulkan Library 
if (Vulkan_LIBRARY)
    #set (LIBS ${LIBS} ${Vulkan_LIBRARY})
endif()

# GLFW 
if (GLFW_LIBRARIES)
    set (LIBS ${LIBS} ${GLFW_LIBRARIES})
    set (INC_DIR ${INC_DIR} ${GLFW_INCLUDE_DIRS})
endif()

# set GCC8 (Linux) libs
# MingW-W64 TODO
if (UNIX OR LINUX)
    find_package(X11 QUIET)
    if (X11_FOUND)
        set (LIBS ${LIBS} ${X11_LIBRARIES})
        set (INC_DIR ${INC_DIR} ${X11_INCLUDE_DIR})
    endif()
    set (LIBS ${LIBS} -static-libgcc -static-libstdc++ -lX11 -lXrandr -lpthread -ldl)
endif()

# Compiler specific options
if (MSVC) # Clang in Visual Studio 2017
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /diagnostics:caret /std:c++latest /arch:AVX2 /tune:skylake /fp:strict /permissive- /Qspectre /JMC /Oy /Oi /Ot /Ox /GT /GL /Gy /Qpar /W3 /NODEFAULTLIB:library") # Native compiler of Visual Studio 2017
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /diagnostics:caret /std:c++latest /arch:AVX2 /fp:strict /permissive- /Qspectre /JMC /Oy /Oi /Ot /Ox /GT /GL /Gy /Qpar /W3")
    set (CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -march=skylake -mtune=skylake -fpermissive ")
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=c1x   -march=skylake -mtune=skylake -fpermissive ")
endif()

# Set optimizations for build types
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

# Set default compiler options
link_directories(${LIB_DIR})
include_directories(${INC_DIR})
file (GLOB RSOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES_LIST})

# Compute test 
set (APPLICATION_NAME test-app)
add_definitions(${DEFINES})
add_executable(${APPLICATION_NAME} src/gltf-old/main.cpp ${RSOURCES})
target_link_libraries(${APPLICATION_NAME} PRIVATE ${LIBS} Vulkan::Vulkan)

# Add includes to MSVC explorer 
foreach(source IN LISTS RSOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
