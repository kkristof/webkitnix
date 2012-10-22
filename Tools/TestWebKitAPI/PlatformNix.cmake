ADD_CUSTOM_TARGET(forwarding-headersNixForTestWebKitAPI
    COMMAND ${PERL_EXECUTABLE} ${WEBKIT2_DIR}/Scripts/generate-forwarding-headers.pl ${TESTWEBKITAPI_DIR} ${DERIVED_SOURCES_WEBKIT2_DIR}/include nix
)
SET(ForwardingHeadersForTestWebKitAPI_NAME forwarding-headersNixForTestWebKitAPI)

ADD_CUSTOM_TARGET(forwarding-headersSoupForTestWebKitAPI
    COMMAND ${PERL_EXECUTABLE} ${WEBKIT2_DIR}/Scripts/generate-forwarding-headers.pl ${TESTWEBKITAPI_DIR} ${DERIVED_SOURCES_WEBKIT2_DIR}/include soup
)
SET(ForwardingNetworkHeadersForTestWebKitAPI_NAME forwarding-headersSoupForTestWebKitAPI)

INCLUDE_DIRECTORIES(${LIBSOUP_INCLUDE_DIRS}
    ${WEBKIT2_DIR}/UIProcess/API/C/soup
    ${WEBKIT2_DIR}/UIProcess/API/nix
    ${GLIB_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
)

SET(test_main_SOURCES
    ${TESTWEBKITAPI_DIR}/nix/main.cpp
    ${TESTWEBKITAPI_DIR}/nix/MainLoop.cpp
)

SET(bundle_harness_SOURCES
    ${TESTWEBKITAPI_DIR}/nix/InjectedBundleControllerNix.cpp
    ${TESTWEBKITAPI_DIR}/nix/PlatformUtilitiesNix.cpp
    ${TESTWEBKITAPI_DIR}/nix/MainLoop.cpp
)

SET(webkit2_api_harness_SOURCES
    ${TESTWEBKITAPI_DIR}/nix/PlatformUtilitiesNix.cpp
    ${TESTWEBKITAPI_DIR}/nix/PlatformWebViewNix.cpp
)

# The list below works like a test expectation. Tests in the
# test_webkit2_api_BINARIES list are added to the test runner and
# tried on the bots on every build. Tests in test_webkit2_api_fail_BINARIES
# are compiled and suffixed with fail and skipped from the test runner.
#
# Make sure that the tests are passing on both Debug and
# Release builds before adding it to test_webkit2_api_BINARIES.

SET(test_webkit2_api_BINARIES
    CookieManager
    DOMWindowExtensionNoCache
    DocumentStartUserScriptAlertCrash
    EvaluateJavaScript
    FailedLoad
    Find
    FrameMIMETypeHTML
    FrameMIMETypePNG
    GetInjectedBundleInitializationUserDataCallback
    InjectedBundleBasic
    InjectedBundleInitializationUserDataCallbackWins
    LoadAlternateHTMLStringWithNonDirectoryURL
    LoadCanceledNoServerRedirectCallback
    NewFirstVisuallyNonEmptyLayout
    PageLoadBasic
    PageLoadDidChangeLocationWithinPageForFrame
    ParentFrame
    PreventEmptyUserAgent
    PrivateBrowsingPushStateNoHistoryCallback
    RestoreSessionStateContainingFormData
    WKConnection
    WKString
    WKStringJSString
    WillSendSubmitEvent
)

SET(test_webkit2_api_fail_BINARIES
    CanHandleRequest
    DOMWindowExtensionBasic
    DownloadDecideDestinationCrash
    ForceRepaint
    NewFirstVisuallyNonEmptyLayoutForImages
    NewFirstVisuallyNonEmptyLayoutFrames
    NewFirstVisuallyNonEmptyLayoutFails
    ShouldGoToBackForwardListItem
    WKPageGetScaleFactorNotZero
)

# Tests disabled because of missing features on the test harness:
#
#   AboutBlankLoad
#   HitTestResultNodeHandle
#   MouseMoveAfterCrash
#   ResponsivenessTimerDoesntFireEarly
#   SpacebarScrolling
#   WKPreferences
#
# Flaky test, fails on Release but passes on Debug:
#
#   UserMessage


# WebKitNix API tests
SET(TestWebKitNixAPIBase_SOURCES
    ${test_main_SOURCES}
    ${TESTWEBKITAPI_DIR}/nix/PlatformUtilitiesNix.cpp
    ${TESTWEBKITAPI_DIR}/nix/PageLoader.cpp
    ${TESTWEBKITAPI_DIR}/JavaScriptTest.cpp
    ${TESTWEBKITAPI_DIR}/PlatformUtilities.cpp
    ${TESTWEBKITAPI_DIR}/TestsController.cpp
)

SET(TestWebKitNixAPIBase_LIBRARIES)

IF (WTF_USE_EGL)
  LIST(APPEND TestWebKitNixAPIBase_SOURCES ${TESTWEBKITAPI_DIR}/nix/GLUtilitiesEGL.cpp)
  LIST(APPEND TestWebKitNixAPIBase_LIBRARIES ${EGL_LIBRARY})
ELSE ()
  LIST(APPEND TestWebKitNixAPIBase_SOURCES ${TESTWEBKITAPI_DIR}/nix/GLUtilitiesGLX.cpp)
ENDIF ()

ADD_LIBRARY(TestWebKitNixAPIBase ${TestWebKitNixAPIBase_SOURCES})
TARGET_LINK_LIBRARIES(TestWebKitNixAPIBase ${TestWebKitNixAPIBase_LIBRARIES})
ADD_DEPENDENCIES(TestWebKitNixAPIBase ${WebKit2_LIBRARY_NAME} ${ForwardingHeadersForTestWebKitAPI_NAME} ${ForwardingNetworkHeadersForTestWebKitAPI_NAME})

LIST(APPEND bundle_harness_SOURCES
    ${TESTWEBKITAPI_DIR}/Tests/nix/WebViewWebProcessCrashed_Bundle.cpp
)

INCLUDE_DIRECTORIES(
    ${TESTWEBKITAPI_DIR}/nix
)

SET(test_webkitnix_api_LIBRARIES
    TestWebKitNixAPIBase
    ${OPENGL_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${WTF_LIBRARY_NAME}
    ${JavaScriptCore_LIBRARY_NAME}
    ${WebKit2_LIBRARY_NAME}
    gtest
)

SET(test_webkitnix_api_BINARIES
    WebViewPaintToCurrentGLContext
    WebViewTranslated
)

SET(test_webkitnix_api_fail_BINARIES
    WebViewWebProcessCrashed
)

FOREACH(testName ${test_webkitnix_api_BINARIES})
    ADD_EXECUTABLE(test_webkitnix_api_${testName} ${TESTWEBKITAPI_DIR}/Tests/nix/${testName}.cpp)
    ADD_TEST(test_webkitnix_api_${testName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_webkitnix_api_${testName})
    SET_TESTS_PROPERTIES(test_webkitnix_api_${testName} PROPERTIES TIMEOUT 60)
    TARGET_LINK_LIBRARIES(test_webkitnix_api_${testName} ${test_webkitnix_api_LIBRARIES})
ENDFOREACH()

# We don't run tests that are expected to fail. We could use the WILL_FAIL
# property, but it reports failure when the test crashes or timeouts and would
# make the bot red.

FOREACH(testName ${test_webkitnix_api_fail_BINARIES})
    ADD_EXECUTABLE(test_webkitnix_api_fail_${testName} ${TESTWEBKITAPI_DIR}/Tests/nix/${testName}.cpp)
    TARGET_LINK_LIBRARIES(test_webkitnix_api_fail_${testName} ${test_webkitnix_api_LIBRARIES})
ENDFOREACH()
