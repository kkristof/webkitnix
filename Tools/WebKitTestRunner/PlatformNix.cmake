LIST(APPEND WebKitTestRunner_LINK_FLAGS
)

ADD_CUSTOM_TARGET(forwarding-headersNixForWebKitTestRunner
    COMMAND ${PERL_EXECUTABLE} ${WEBKIT2_DIR}/Scripts/generate-forwarding-headers.pl ${WEBKIT_TESTRUNNER_DIR} ${DERIVED_SOURCES_WEBKIT2_DIR}/include nix
)
SET(ForwardingHeadersForWebKitTestRunner_NAME forwarding-headersNixForWebKitTestRunner)

ADD_CUSTOM_TARGET(forwarding-headersSoupForWebKitTestRunner
    COMMAND ${PERL_EXECUTABLE} ${WEBKIT2_DIR}/Scripts/generate-forwarding-headers.pl ${WEBKIT_TESTRUNNER_DIR} ${DERIVED_SOURCES_WEBKIT2_DIR}/include soup
)
SET(ForwardingNetworkHeadersForWebKitTestRunner_NAME forwarding-headersSoupForWebKitTestRunner)

LIST(APPEND WebKitTestRunner_SOURCES
    ${WTF_DIR}/wtf/MD5.cpp
    ${WEBKIT_TESTRUNNER_DIR}/cairo/TestInvocationCairo.cpp

    ${WEBKIT_TESTRUNNER_DIR}/nix/EventSenderProxyNix.cpp
    ${WEBKIT_TESTRUNNER_DIR}/nix/PlatformWebViewNix.cpp
    ${WEBKIT_TESTRUNNER_DIR}/nix/TestControllerNix.cpp
    ${WEBKIT_TESTRUNNER_DIR}/nix/main.cpp
)

LIST(APPEND WebKitTestRunner_INCLUDE_DIRECTORIES
    ${WEBKIT2_DIR}/UIProcess/API/nix
    "${WTF_DIR}/wtf/gobject"

    ${CAIRO_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)

SET(WebKitTestRunner_LIBRARIES
    ${JavaScriptCore_LIBRARY_NAME}
    ${WebCoreTestSupport_LIBRARY_NAME}
    ${WebKit2_LIBRARY_NAME}
    ${WebCore_LIBRARY_NAME}

    ${CAIRO_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

LIST(APPEND WebKitTestRunnerInjectedBundle_SOURCES
    ${WEBKIT_TESTRUNNER_INJECTEDBUNDLE_DIR}/nix/FontManagement.cpp
    ${WEBKIT_TESTRUNNER_INJECTEDBUNDLE_DIR}/nix/ActivateFontsNix.cpp
    ${WEBKIT_TESTRUNNER_INJECTEDBUNDLE_DIR}/efl/InjectedBundleEfl.cpp
    ${WEBKIT_TESTRUNNER_INJECTEDBUNDLE_DIR}/nix/TestRunnerNix.cpp
)

# FIXME: DOWNLOADED_FONTS_DIR should not hardcode the directory
# structure. See <https://bugs.webkit.org/show_bug.cgi?id=81475>.
ADD_DEFINITIONS(-DFONTS_CONF_DIR="${TOOLS_DIR}/DumpRenderTree/gtk/fonts"
                -DDOWNLOADED_FONTS_DIR="${CMAKE_SOURCE_DIR}/WebKitBuild/Dependencies/Source/webkitgtk-test-fonts-0.0.3"
                -DTHEME_DIR="${THEME_BINARY_DIR}")

SET(ImageDiff_SOURCES
    ${WEBKIT_TESTRUNNER_DIR}/nix/ImageDiff.cpp
)
ADD_EXECUTABLE(ImageDiff ${ImageDiff_SOURCES})
TARGET_LINK_LIBRARIES(ImageDiff ${PNG_LIBRARY})
SET_TARGET_PROPERTIES(ImageDiff PROPERTIES FOLDER "Tools")