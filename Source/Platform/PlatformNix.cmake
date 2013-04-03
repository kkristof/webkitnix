set (Platform_INCLUDES
    nix
    chromium
    ${WEBCORE_DIR}/platform/graphics/ # For IntRect.h
    ${WTF_DIR} # For config.h
    ${CMAKE_BINARY_DIR} # For cmakeconfig.h
)

set (Platform_HEADERS
    nix/public/Platform.h
    nix/public/WebFFTFrame.h
    nix/public/WebRect.h
    nix/public/WebSize.h
    nix/public/WebThemeEngine.h
    chromium/public/WebAudioBus.h
    chromium/public/WebAudioDevice.h
    chromium/public/WebCommon.h
    chromium/public/WebData.h
    chromium/public/WebGamepads.h
    chromium/public/WebGamepad.h
    chromium/public/WebVector.h
)

set (Platform_SOURCES
    nix/src/Platform.cpp
    nix/src/DefaultWebThemeEngine.cpp
)

install(FILES ${Platform_HEADERS} DESTINATION include/${WebKit2_LIBRARY_NAME}-${PROJECT_VERSION_MAJOR}/NixPlatform)
