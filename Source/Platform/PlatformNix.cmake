SET (Platform_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/nix/NixPlatform.h"
)

SET (Platform_SOURCES )

IF (ENABLE_GAMEPAD)
   # Nowadays NixPlatform has just the gamepad stuff
   LIST(APPEND Platform_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/nix/NixPlatform.cpp")
ENDIF ()

INSTALL(FILES ${Platform_HEADERS} DESTINATION include/${WebKit2_LIBRARY_NAME}-${PROJECT_VERSION_MAJOR}/Platform)

