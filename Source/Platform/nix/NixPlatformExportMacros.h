#ifndef NixPlatformExportMacros_h
#define NixPlatformExportMacros_h

#undef NIX_PLATFORM_EXPORT
#if defined(__GNUC__) && !defined(__CC_ARM) && !defined(__ARMCC__)
#define NIX_PLATFORM_EXPORT __attribute__((visibility("default")))
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN32_WCE) || defined(__CC_ARM) || defined(__ARMCC__)
#if BUILDING_WEBKIT
#define NIX_PLATFORM_EXPORT __declspec(dllexport)
#else
#define NIX_PLATFORM_EXPORT __declspec(dllimport)
#endif
#else
#define NIX_PLATFORM_EXPORT
#endif

#endif
