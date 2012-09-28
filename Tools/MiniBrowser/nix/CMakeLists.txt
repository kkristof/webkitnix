SET(MiniBrowser_DIR "${TOOLS_DIR}/MiniBrowser/nix")

SET(MiniBrowser_SOURCES
    ${MiniBrowser_DIR}/main.cpp
    ${MiniBrowser_DIR}/LinuxWindow.cpp
    ${MiniBrowser_DIR}/XlibEventSource.cpp
)

SET(MiniBrowser_LIBRARIES
    ${WebKit2_LIBRARY_NAME}
    ${OPENGL_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GLIB_GTHREAD_LIBRARIES}
)

SET(MiniBrowser_INCLUDE_DIRECTORIES
    ${GLIB_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${DERIVED_SOURCES_WEBKIT2_DIR}/include
    ${WEBKIT2_DIR}/UIProcess/API/nix
    ${WEBKIT2_DIR}
    ${CMAKE_SOURCE_DIR}/Source
    ${WTF_DIR}
    ${EGL_INCLUDE_DIR}
)

IF (WTF_USE_GLX)
  LIST(APPEND MiniBrowser_SOURCES ${MiniBrowser_DIR}/LinuxWindowGLX.cpp)
ELSE ()
  LIST(APPEND MiniBrowser_SOURCES ${MiniBrowser_DIR}/LinuxWindowEGL.cpp)
  LIST(APPEND MiniBrowser_LIBRARIES ${EGL_LIBRARY})
ENDIF ()

INCLUDE_DIRECTORIES(${MiniBrowser_INCLUDE_DIRECTORIES})

ADD_EXECUTABLE(MiniBrowser ${MiniBrowser_SOURCES})
TARGET_LINK_LIBRARIES(MiniBrowser ${MiniBrowser_LIBRARIES})
SET_TARGET_PROPERTIES(MiniBrowser PROPERTIES FOLDER "Tools")
