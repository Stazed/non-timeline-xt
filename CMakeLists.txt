#CMake file for non-timeline-xt

cmake_minimum_required(VERSION 3.5.1...3.31.1)
project (non-timeline-xt)

set (PACKAGE_VERSION "2.0.5")

set (CMAKE_SKIP_RULE_DEPENDENCY OFF)

add_definitions(-D'VERSION="${PACKAGE_VERSION}"')
add_definitions(-D'WEBSITE="https://github.com/Stazed/non-timeline-xt"')
add_definitions(-D'PACKAGE="non-timeline-xt"')
add_definitions(-D'PIXMAP_PATH="${CMAKE_INSTALL_PREFIX}/share/pixmaps"')
add_definitions(-D'DOCUMENT_PATH="${CMAKE_INSTALL_PREFIX}/share/doc"')

include(CheckCSourceCompiles)
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-msse2" SUPPORT_SSE)

option (BuildForDebug "Include gdb debugging support" OFF)
option (EnableOptimizations "Enable optimizations" ON)
option (EnableSSE "Enable SSE (default=automatic check)" ON)
option (EnableSSE2 "Enable SSE2 (default=automatic check)" ON)
option (NativeOptimizations "Enable native CPU optimizations" ON)
option (EnableNTK "Use NTK toolkit for build" OFF)
option (EnableFLTKStatic "Build using FLTK static libraries" OFF)


set(CMAKE_BUILD_TYPE "Release")

set (BuildOptions_SSE
    "-msse -mfpmath=sse"
    CACHE STRING "SSE compiler options"
)

set (BuildOptions_SSE2
    "-msse -msse2 -mfpmath=sse"
    CACHE STRING "SSE2 compiler options"
)

set (BuildOptions_Native
    "-march=native -mtune=native"
    CACHE STRING "Native compiler options"
)
set (BuildOptionsBasic
    "-std=c++11 -Wno-unused-parameter -Wno-unused-result -O3 -fomit-frame-pointer -ffast-math -pipe"
    CACHE STRING "basic X86 complier options"
)

set (BuildOptionsDebug
    "-std=c++11 -Wno-cast-function-type -O0 -g3 -ggdb -Wall -Wextra -Wpointer-arith"
#    "-std=c++11 -Wno-cast-function-type -Wno-unused-result -O0 -g3 -ggdb -Wall -Wextra -Wpointer-arith  -flto=auto -ffat-lto-objects -fexceptions -g -grecord-gcc-switches -pipe -Wall -Werror=format-security -Wp,-U_FORTIFY_SOURCE,-D_FORTIFY_SOURCE=3 -Wp,-D_GLIBCXX_ASSERTIONS -m64 -march=x86-64 -mtune=generic -fasynchronous-unwind-tables -fstack-clash-protection -fcf-protection -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer"
    CACHE STRING "Debug build flags"
)

find_package (PkgConfig REQUIRED)

#dependencies
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET non-timeline-xt PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET non-timeline-xt PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()

pkg_check_modules(JACK REQUIRED jack>=0.115.6)
pkg_check_modules(LIBLO liblo>=0.26 REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

if(EnableNTK)
    pkg_check_modules(NTK ntk REQUIRED)
    pkg_check_modules(NTK_IMAGES ntk_images REQUIRED)
    add_definitions(-D'BUILD_TYPE="Built with NTK Library"')
else(EnableNTK)
    SET(FLTK_SKIP_OPENGL TRUE)
    if(EnableFLTKStatic)
        find_library(FLTK_STATIC libfltk.a REQUIRED)
        find_library(FLTK_STATIC_IMAGES libfltk_images.a REQUIRED)
        pkg_check_modules(XFIXES REQUIRED xfixes)
        pkg_check_modules(XINERAMA REQUIRED xinerama)
        pkg_check_modules(FONTCONFIG REQUIRED fontconfig>=0.22)
        pkg_check_modules(XFT REQUIRED xft)
        pkg_check_modules(XRENDER REQUIRED xrender)
        add_definitions(-D'BUILD_TYPE="Built with FLTK static library"')
    else(EnableFLTKStatic)
        add_definitions(-D'BUILD_TYPE="Built with FLTK Library"')
    endif(EnableFLTKStatic)

    find_package(FLTK REQUIRED)
    pkg_check_modules (LIBCAIRO REQUIRED cairo)
    if(FLTK_FOUND)
        mark_as_advanced(FORCE FLTK_BASE_LIBRARY)
        mark_as_advanced(FORCE FLTK_CONFIG_SCRIPT)
        mark_as_advanced(FORCE FLTK_DIR)
        mark_as_advanced(FORCE FLTK_FLUID_EXECUTABLE)
        mark_as_advanced(FORCE FLTK_FORMS_LIBRARY)
        mark_as_advanced(FORCE FLTK_GL_LIBRARY)
        mark_as_advanced(FORCE FLTK_IMAGES_LIBRARY)
        mark_as_advanced(FORCE FLTK_INCLUDE_DIR)
        mark_as_advanced(FORCE FLTK_MATH_LIBRARY)
        add_definitions(-D'FLTK_SUPPORT=1')
    endif(FLTK_FOUND)
endif(EnableNTK)


include(CheckSymbolExists)

set(CMAKE_REQUIRED_LIBRARIES "jack")
check_symbol_exists(jack_get_property "jack/metadata.h" JACK_METADATA)
if(JACK_METADATA)
    add_definitions(-D'HAVE_JACK_METADATA=1')
    message(STATUS "Found jack metadata")
else(JACK_METADATA)
    message(STATUS "Jack metadata not found")
endif(JACK_METADATA)

check_symbol_exists(jack_port_get_latency_range "jack/jack.h" JACK_LATENCY_RANGE)
if(JACK_LATENCY_RANGE)
    add_definitions(-D'HAVE_JACK_PORT_GET_LATENCY_RANGE=1')
    message(STATUS "Found jack_port_get_latency_range")
else(JACK_LATENCY_RANGE)
    message(STATUS "jack_port_get_latency_range not found")
endif(JACK_LATENCY_RANGE)

CHECK_CXX_SOURCE_COMPILES(
    "int main ( char**argv, int argc ) {
    const char *s = (const char*)
    __builtin_assume_aligned( 0, 16 );
    return 0; }" HAVE_BUILTIN_ALIGNED)

if(HAVE_BUILTIN_ALIGNED)
    add_definitions(-D'HAS_BUILTIN_ASSUME_ALIGNED=1')
endif(HAVE_BUILTIN_ALIGNED)

CHECK_CXX_SOURCE_COMPILES(
    "#include <sndfile.h>\nint main ( char **argv, int argc ) {
    return SF_FORMAT_VORBIS; }" HAS_SF_FORMAT_VORBIS)

if(HAS_SF_FORMAT_VORBIS)
    add_definitions(-D'HAVE_SF_FORMAT_VORBIS=1')
endif(HAS_SF_FORMAT_VORBIS)

check_symbol_exists(mlock "sys/mman.h" MLOCK)
if(MLOCK)
    add_definitions(-D'HAVE_MLOCK=1')
else (MLOCK)
    message(STATUS "mlock not found")
endif (MLOCK)

find_library (LIBDL dl)
if (LIBDL)
    message(STATUS "Found dl in ${LIBDL}")
else (LIBDL)
    message(FATAL_ERROR "dl required but not found")
endif (LIBDL)

find_library (LIBM m)
if (LIBM)
    message(STATUS "Found m in ${LIBM}")
else (LIBM)
    message(FATAL_ERROR "m required but not found")
endif (LIBM)

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

mark_as_advanced (CMAKE_BUILD_TYPE)
mark_as_advanced (CMAKE_INSTALL_PREFIX)
mark_as_advanced (LIBDL)
mark_as_advanced (LIBM)

if (BuildForDebug)
    set (CMAKE_BUILD_TYPE "Debug")
    set (CMAKE_CXX_FLAGS_DEBUG ${BuildOptionsDebug})
else (BuildForDebug)
    set (CMAKE_BUILD_TYPE "Release")

    set (CMAKE_CXX_FLAGS_RELEASE ${BuildOptionsBasic})
    add_definitions(-D'NDEBUG=1')
    if(EnableOptimizations)
        if (SUPPORT_SSE)
            if(EnableSSE)
                if(EnableSSE2)
                    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${BuildOptions_SSE2}")
                    set (USE_SSE2 "${SUPPORT_SSE}")
                    set (USE_SSE "${SUPPORT_SSE}") #uses both
                else(EnableSSE2)
                    set (USE_SSE "${SUPPORT_SSE}")
                    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${BuildOptions_SSE}")
                endif(EnableSSE2)
            endif(EnableSSE)
        endif (SUPPORT_SSE)

        if(BuildOptions_Native)
            set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${BuildOptions_Native}")
            set (USE_NATIVE "")
        endif(BuildOptions_Native)

    else(EnableOptimizations)
        set (USE_SSE2 "")
        set (USE_SSE "")
    endif(EnableOptimizations)
endif (BuildForDebug)

add_definitions(-D'BUILD_TYPE_CMAKE="${CMAKE_BUILD_TYPE}"')

add_subdirectory(timeline)
add_subdirectory(timeline/icons)
add_subdirectory(timeline/doc)


##Summarize The Full Configuration
message(STATUS)
message(STATUS "=========================================================")
message(STATUS "Build Configuration for non-timeline-xt-${PACKAGE_VERSION} for ${CMAKE_SYSTEM_NAME}")
message(STATUS "=========================================================")
message(STATUS)

macro(package_status foundvar pkg)
    if(${foundvar})
        message(STATUS "${pkg} Yes")
    else()
        message(STATUS "${pkg} No")
    endif()
endmacro()

message (STATUS                    "___________________ Optimizations________________________"  )
package_status(USE_SSE             "Use sse. . . . . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(USE_SSE2            "Use sse2 . . . . . . . . . . . . . . . . . . . . . . . .:"  )

message (STATUS)
message (STATUS                    "______________________Library____________________________"  )
package_status(PKG_CONFIG_FOUND    "PkgConfig. . . . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(FLTK_FOUND          "FLTK toolkit . . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(NTK_FOUND           "NTK toolkit. . . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(LIBCAIRO_FOUND      "Cairo support. . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(Threads_FOUND       "pthread support (libpthread) . . . . . . . . . . . . . .:"  )
package_status(MLOCK               "mlock support. . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(LIBM                "libm support . . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(LIBDL               "libdl support. . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(JACK_FOUND          "JACK Audio Connection Kit library. . . . . . . . . . . .:"  )
package_status(SNDFILE_FOUND       "General audio file (libsndfile). . . . . . . . . . . . .:"  )
package_status(LIBLO_FOUND         "Liblo support. . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(JACK_METADATA       "Jack metadata support. . . . . . . . . . . . . . . . . .:"  )
package_status(JACK_LATENCY_RANGE  "Jack port latency range support. . . . . . . . . . . . .:"  )
package_status(HAVE_BUILTIN_ALIGNED "Has builtin assume aligned . . . . . . . . . . . . . . .:"  )
package_status(HAS_SF_FORMAT_VORBIS    "Have SF format vorbis. . . . . . . . . . . . . . . . . .:"  )

message (STATUS)
message (STATUS                    "____________________ User Options________________________"  )
package_status(EnableOptimizations "Use optimizations. . . . . . . . . . . . . . . . . . . .:"  )
package_status(EnableSSE           "Use sse. . . . . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(EnableSSE2          "Use sse2 . . . . . . . . . . . . . . . . . . . . . . . .:"  )
package_status(EnableNTK           "Enable NTK build . . . . . . . . . . . . . . . . . . . .:"  )
package_status(EnableFLTKStatic    "Enable FLTK Static build . . . . . . . . . . . . . . . .:"  )
package_status(NativeOptimizations "Native optimizations . . . . . . . . . . . . . . . . . .:"  )
package_status(BuildForDebug       "Build for debug. . . . . . . . . . . . . . . . . . . . .:"  )


message (STATUS)
message (STATUS)
message (STATUS "Building for ${CMAKE_BUILD_TYPE}")
if (BuildForDebug)
    message (STATUS "Flags: ${CMAKE_CXX_FLAGS_DEBUG}")
else (BuildForDebug)
    message (STATUS "Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
endif (BuildForDebug)
