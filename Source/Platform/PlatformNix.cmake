SET (Platform_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/nix/NixPlatform.h"
)

SET (Platform_SOURCES
   "${CMAKE_CURRENT_SOURCE_DIR}/nix/NixPlatform.cpp"
)

INSTALL(FILES ${Platform_HEADERS} DESTINATION include/${WebKit2_LIBRARY_NAME}-${PROJECT_VERSION_MAJOR}/Platform)

