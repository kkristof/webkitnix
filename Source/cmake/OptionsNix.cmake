set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Set default values for features based on Features${PORT}.config
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/Source/cmake/Features${PORT}.config" featureData NEWLINE_CONSUME)
string(REPLACE ";" "\\\\;" featureData "${featureData}")
string(REPLACE "\n" ";" featureData "${featureData}")

set(_WEBKIT_AVAILABLE_OPTIONS "")
foreach (feature IN ITEMS ${featureData})
    string(REGEX MATCH "^[^\#].*" validRecord ${feature})
    if (validRecord)
        string(REGEX REPLACE "([0-9A-Z_-]+).*" "\\1" defineName "${feature}")
        string(REGEX REPLACE "[0-9A-Z_-]+ *= *(.*)" "\\1" value "${feature}")
        option(${defineName} "Toggle ${defineName}" ${value})
        list(APPEND _WEBKIT_AVAILABLE_OPTIONS ${defineName})
    endif ()
endforeach ()

add_definitions(-DBUILDING_NIX__=1)
add_definitions(-DWTF_PLATFORM_NIX=1)
set(WTF_PLATFORM_NIX 1)

find_package(Cairo 1.10 REQUIRED)
find_package(Fontconfig 2.8.0 REQUIRED)
find_package(Sqlite REQUIRED)
find_package(LibXml2 2.6 REQUIRED)
find_package(LibXslt 1.1.7 REQUIRED)
find_package(ICU REQUIRED)
find_package(Threads REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

if (WTF_USE_OPENGL_ES_2)
    set(WTF_USE_EGL 1)
    find_package(OpenGLES2 REQUIRED)
else ()
    find_package(OpenGL REQUIRED)
endif ()

find_package(GLIB 2.31.8 REQUIRED COMPONENTS gio gobject gmodule gthread)
find_package(LibSoup 2.39.4.1 REQUIRED)
add_definitions(-DENABLE_GLIB_SUPPORT=1)

set(SHARED_CORE 0)

set(ENABLE_WEBKIT 0)
set(ENABLE_WEBKIT2 1)

set(WTF_USE_SOUP 1)
add_definitions(-DWTF_USE_SOUP=1)

add_definitions(-DENABLE_CONTEXT_MENUS=0)

set(WTF_USE_PTHREADS 1)
add_definitions(-DWTF_USE_PTHREADS=1)

set(WTF_USE_ICU_UNICODE 1)
add_definitions(-DWTF_USE_ICU_UNICODE=1)

set(WTF_USE_CAIRO 1)
add_definitions(-DWTF_USE_CAIRO=1)

set(JSC_EXECUTABLE_NAME jsc)

set(WTF_LIBRARY_NAME wtf_nix)
set(JavaScriptCore_LIBRARY_NAME javascriptcore_nix)
set(WebCore_LIBRARY_NAME webcore_nix)
set(WebKit2_LIBRARY_NAME WebKitNix)


find_package(Freetype REQUIRED)
find_package(HarfBuzz REQUIRED)
set(WTF_USE_FREETYPE 1)
set(WTF_USE_HARFBUZZ_NG 1)
add_definitions(-DWTF_USE_FREETYPE=1)
add_definitions(-DWTF_USE_HARFBUZZ_NG=1)

if (NOT ENABLE_SVG)
    set(ENABLE_SVG_FONTS 0)
endif ()

if (ENABLE_BATTERY_STATUS)
    find_package(DBus REQUIRED)
endif ()

if (ENABLE_VIDEO OR ENABLE_WEB_AUDIO)
    set(GSTREAMER_COMPONENTS app interfaces pbutils)
    set(WTF_USE_GSTREAMER 1)
    add_definitions(-DWTF_USE_GSTREAMER=1)

    if (ENABLE_VIDEO)
        list(APPEND GSTREAMER_COMPONENTS video)
    endif ()

    if (ENABLE_WEB_AUDIO)
        list(APPEND GSTREAMER_COMPONENTS audio fft)
        add_definitions(-DWTF_USE_WEBAUDIO_GSTREAMER=1)
    endif ()

    find_package(GStreamer REQUIRED COMPONENTS ${GSTREAMER_COMPONENTS})
endif ()

if (WTF_USE_OPENGL_ES_2)
    add_definitions(-DWTF_USE_OPENGL_ES_2=1)
else ()
    set(WTF_USE_OPENGL 1)
    add_definitions(-DWTF_USE_OPENGL=1)
endif ()

if (WTF_USE_EGL)
    find_package(EGL REQUIRED)
endif ()

set(CPACK_SOURCE_GENERATOR TBZ2)

set(WTF_USE_ACCELERATED_COMPOSITING 1)
add_definitions(-DWTF_USE_ACCELERATED_COMPOSITING=1)

set(WTF_USE_COORDINATED_GRAPHICS 1)
add_definitions(-DWTF_USE_COORDINATED_GRAPHICS=1)

set(WTF_USE_TEXTURE_MAPPER 1)
add_definitions(-DWTF_USE_TEXTURE_MAPPER=1)

set(WTF_USE_TEXTURE_MAPPER_GL 1)
add_definitions(-DWTF_USE_TEXTURE_MAPPER_GL=1)

set(WTF_USE_3D_GRAPHICS 1)
add_definitions(-DWTF_USE_3D_GRAPHICS=1)

macro(PROCESS_WEBKIT_OPTIONS)
    # Show all options on screen
    message(STATUS "Enabled features:")

    set(_MAX_FEATURE_LENGTH 0)
    foreach (_name ${_WEBKIT_AVAILABLE_OPTIONS})
        string(LENGTH ${_name} _NAME_LENGTH)
        if (_NAME_LENGTH GREATER _MAX_FEATURE_LENGTH)
            set(_MAX_FEATURE_LENGTH ${_NAME_LENGTH})
        endif ()
    endforeach ()

    set(_SHOULD_PRINT_POINTS OFF)
    set(_0 "OFF")
    set(_1 "ON")
    foreach (_name ${_WEBKIT_AVAILABLE_OPTIONS})
        string(LENGTH ${_name} _NAME_LENGTH)

        set(_MESSAGE " ${_name} ")

        if (_SHOULD_PRINT_POINTS)
            foreach (IGNORE RANGE ${_NAME_LENGTH} ${_MAX_FEATURE_LENGTH})
                set(_MESSAGE "${_MESSAGE} ")
            endforeach ()
            set(_SHOULD_PRINT_POINTS OFF)
        else ()
            foreach (IGNORE RANGE ${_NAME_LENGTH} ${_MAX_FEATURE_LENGTH})
                set(_MESSAGE "${_MESSAGE}.")
            endforeach ()
            set(_SHOULD_PRINT_POINTS ON)
        endif ()

        if (${_name})
            list(APPEND FEATURE_DEFINES ${_name})
            set(FEATURE_DEFINES_WITH_SPACE_SEPARATOR "${FEATURE_DEFINES_WITH_SPACE_SEPARATOR} ${_name}")
        endif ()

        message(STATUS "${_MESSAGE} ${${_name}}")
    endforeach ()

    # Create cmakeconfig.h.in based on the features we have available
    set(CMK_CONFIG_H_IN "${CMAKE_BINARY_DIR}/cmakeconfig.h.in")

    file(WRITE ${CMK_CONFIG_H_IN}
        "#ifndef CMAKECONFIG_H\n"
        "#define CMAKECONFIG_H\n\n")
    foreach (_name ${_WEBKIT_AVAILABLE_OPTIONS})
        file(APPEND ${CMK_CONFIG_H_IN} "#cmakedefine01 ${_name}\n")
    endforeach ()
    file(APPEND ${CMK_CONFIG_H_IN} "\n#endif // CMAKECONFIG_H\n")

    file(MD5 ${CMK_CONFIG_H_IN} CMK_CONFIG_H_IN_NEW_MD5)
    if (NOT "${CMK_CONFIG_H_IN_MD5}" STREQUAL "${CMK_CONFIG_H_IN_NEW_MD5}")
        message(STATUS "Creating cmakeconfig.h")
        configure_file(${CMK_CONFIG_H_IN} "${CMAKE_BINARY_DIR}/cmakeconfig.h" @ONLY)
        set(CMK_CONFIG_H_IN_MD5 "${CMK_CONFIG_H_IN_NEW_MD5}" CACHE STRING "Md5 of ${CMK_CONFIG_H_IN}" FORCE)
    endif ()
endmacro()

PROCESS_WEBKIT_OPTIONS()
