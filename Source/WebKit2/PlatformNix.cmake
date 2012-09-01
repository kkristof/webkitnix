LIST(APPEND WebKit2_LINK_FLAGS
    ${CAIRO_LDFLAGS}
)

LIST(APPEND WebKit2_SOURCES
    Platform/gtk/ModuleGtk.cpp
    Platform/gtk/WorkQueueGtk.cpp
    Platform/unix/SharedMemoryUnix.cpp

    Platform/CoreIPC/unix/ConnectionUnix.cpp
    Platform/CoreIPC/unix/AttachmentUnix.cpp

    Shared/API/c/cairo/WKImageCairo.cpp

    Shared/API/c/gtk/WKGraphicsContextGtk.cpp

    Shared/cairo/ShareableBitmapCairo.cpp

    Shared/efl/LayerTreeContextEfl.cpp
    Shared/efl/ProcessExecutablePathEfl.cpp

    Shared/soup/PlatformCertificateInfo.cpp
    Shared/soup/WebCoreArgumentCodersSoup.cpp

    UIProcess/API/C/soup/WKContextSoup.cpp
    UIProcess/API/C/soup/WKSoupRequestManager.cpp

    UIProcess/API/nix/WebView.cpp

    UIProcess/cairo/BackingStoreCairo.cpp

    UIProcess/efl/TextCheckerEfl.cpp
    UIProcess/nix/WebContextNix.cpp
    UIProcess/efl/WebFullScreenManagerProxyEfl.cpp
    UIProcess/nix/WebInspectorProxyNix.cpp
    UIProcess/nix/WebPageProxyNix.cpp
    UIProcess/efl/WebPreferencesEfl.cpp

    UIProcess/soup/WebCookieManagerProxySoup.cpp
    UIProcess/soup/WebSoupRequestManagerClient.cpp
    UIProcess/soup/WebSoupRequestManagerProxy.cpp

    UIProcess/Launcher/efl/ProcessLauncherEfl.cpp

    UIProcess/Plugins/unix/PluginInfoStoreUnix.cpp

    WebProcess/Cookies/soup/WebCookieManagerSoup.cpp
    WebProcess/Cookies/soup/WebKitSoupCookieJarSqlite.cpp

    WebProcess/Downloads/efl/DownloadSoupErrorsEfl.cpp
    WebProcess/Downloads/soup/DownloadSoup.cpp

    WebProcess/nix/WebProcessNix.cpp
    WebProcess/nix/WebProcessMainNix.cpp

    WebProcess/InjectedBundle/gtk/InjectedBundleGtk.cpp

    WebProcess/WebCoreSupport/efl/WebContextMenuClientEfl.cpp
    WebProcess/WebCoreSupport/efl/WebErrorsEfl.cpp
    WebProcess/WebCoreSupport/efl/WebPopupMenuEfl.cpp

    WebProcess/WebPage/efl/WebInspectorEfl.cpp
    WebProcess/WebPage/nix/WebPageNix.cpp

    WebProcess/soup/WebSoupRequestManager.cpp
    WebProcess/soup/WebKitSoupRequestGeneric.cpp
    WebProcess/soup/WebKitSoupRequestInputStream.cpp
)

LIST(APPEND WebKit2_MESSAGES_IN_FILES
    UIProcess/soup/WebSoupRequestManagerProxy.messages.in
    WebProcess/soup/WebSoupRequestManager.messages.in
)

LIST(APPEND WebKit2_INCLUDE_DIRECTORIES
    "${JAVASCRIPTCORE_DIR}/llint"
    "${WEBCORE_DIR}/platform/efl"
    "${WEBCORE_DIR}/platform/graphics/cairo"
    "${WEBCORE_DIR}/platform/network/soup"
    "${WEBCORE_DIR}/svg/graphics"
    "${WEBKIT2_DIR}/Shared/efl"
    "${WEBKIT2_DIR}/Shared/soup"
    "${WEBKIT2_DIR}/UIProcess/API/C/soup"
    "${WEBKIT2_DIR}/UIProcess/API/cpp/efl"
    "${WEBKIT2_DIR}/UIProcess/API/efl"
    "${WEBKIT2_DIR}/UIProcess/soup"
    "${WEBKIT2_DIR}/WebProcess/Downloads/soup"
    "${WEBKIT2_DIR}/WebProcess/efl"
    "${WEBKIT2_DIR}/WebProcess/nix"
    "${WEBKIT2_DIR}/WebProcess/soup"
    "${WEBKIT2_DIR}/WebProcess/WebCoreSupport/efl"
    "${WTF_DIR}/wtf/gobject"
    ${CAIRO_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
    ${LIBXSLT_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${LIBSOUP_INCLUDE_DIRS}
    ${WTF_DIR}
    ${HARFBUZZ_INCLUDE_DIRS}
)

LIST(APPEND WebKit2_LIBRARIES
    ${CAIRO_LIBRARIES}
    ${Freetype_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${PNG_LIBRARY}
    ${JPEG_LIBRARY}
    ${CMAKE_DL_LIBS}
    ${GLIB_LIBRARIES}
    ${GLIB_GIO_LIBRARIES}
    ${GLIB_GOBJECT_LIBRARIES}
    ${LIBSOUP_LIBRARIES}
)

LIST (APPEND WebProcess_SOURCES
    nix/MainNix.cpp
)

LIST (APPEND WebProcess_LIBRARIES
    ${CAIRO_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${LIBXSLT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SQLITE_LIBRARIES}
)

ADD_DEFINITIONS(-DDEFAULT_THEME_PATH=\"${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/themes\")

ADD_CUSTOM_TARGET(forwarding-headerEfl
    COMMAND ${PERL_EXECUTABLE} ${WEBKIT2_DIR}/Scripts/generate-forwarding-headers.pl ${WEBKIT2_DIR} ${DERIVED_SOURCES_WEBKIT2_DIR}/include efl
)
SET(ForwardingHeaders_NAME forwarding-headerEfl)

ADD_CUSTOM_TARGET(forwarding-headerSoup
    COMMAND ${PERL_EXECUTABLE} ${WEBKIT2_DIR}/Scripts/generate-forwarding-headers.pl ${WEBKIT2_DIR} ${DERIVED_SOURCES_WEBKIT2_DIR}/include soup
)
SET(ForwardingNetworkHeaders_NAME forwarding-headerSoup)

CONFIGURE_FILE(efl/ewebkit2.pc.in ${CMAKE_BINARY_DIR}/WebKit2/efl/ewebkit2.pc @ONLY)
SET (WebKitNix_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/UIProcess/API/nix/WebView.h"
)

INSTALL(FILES ${CMAKE_BINARY_DIR}/WebKit2/efl/ewebkit2.pc DESTINATION lib/pkgconfig)
INSTALL(FILES ${WebKitNix_HEADERS} DESTINATION include/${WebKit2_LIBRARY_NAME}-${PROJECT_VERSION_MAJOR})

ADD_DEFINITIONS(-DLIBEXECDIR=\"${CMAKE_INSTALL_PREFIX}/${EXEC_INSTALL_DIR}\"
    -DWEBPROCESSNAME=\"${WebProcess_EXECUTABLE_NAME}\"
    -DPLUGINPROCESSNAME=\"${PluginProcess_EXECUTABLE_NAME}\"
)

IF (ENABLE_INSPECTOR)
    SET(WK2_WEB_INSPECTOR_DIR ${CMAKE_BINARY_DIR}/WebKit2/efl/webinspector)
    SET(WK2_WEB_INSPECTOR_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${WebKit2_LIBRARY_NAME}-${PROJECT_VERSION_MAJOR})
    ADD_DEFINITIONS(-DWK2_WEB_INSPECTOR_DIR="${WK2_WEB_INSPECTOR_DIR}")
    ADD_DEFINITIONS(-DWK2_WEB_INSPECTOR_INSTALL_DIR="${WK2_WEB_INSPECTOR_INSTALL_DIR}/webinspector")
    ADD_CUSTOM_TARGET(
        wk2-web-inspector-resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${WEBCORE_DIR}/inspector/front-end ${WK2_WEB_INSPECTOR_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${WEBCORE_DIR}/English.lproj/localizedStrings.js ${WK2_WEB_INSPECTOR_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${DERIVED_SOURCES_WEBCORE_DIR}/InspectorBackendCommands.js ${WK2_WEB_INSPECTOR_DIR}/InspectorBackendCommands.js
        DEPENDS ${WebCore_LIBRARY_NAME}
    )
    INSTALL(DIRECTORY ${WK2_WEB_INSPECTOR_DIR}
        DESTINATION ${WK2_WEB_INSPECTOR_INSTALL_DIR}
        FILES_MATCHING PATTERN "*.js"
                       PATTERN "*.html"
                       PATTERN "*.css"
                       PATTERN "*.gif"
                       PATTERN "*.png")
ENDIF ()
