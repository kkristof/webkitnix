include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nix)

SET (Platform_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/nix/public/Platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/nix/public/WebFFTFrame.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/chromium/public/WebAudioBus.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/chromium/public/WebAudioDevice.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/chromium/public/WebCommon.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/chromium/public/WebData.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/chromium/public/WebGamepads.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/chromium/public/WebGamepad.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/chromium/public/WebVector.h"
)

SET (Platform_SOURCES "nix/public/Platform.cpp")

install(FILES ${Platform_HEADERS} DESTINATION include/${WebKit2_LIBRARY_NAME}-${PROJECT_VERSION_MAJOR}/NixPlatform)
