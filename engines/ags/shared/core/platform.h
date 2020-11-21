#ifndef __AC_PLATFORM_H
#define __AC_PLATFORM_H

// platform definitions. Not intended for replacing types or checking for libraries.

// check Android first because sometimes it can get confused with host OS
#if defined(__ANDROID__) || defined(ANDROID)
    #define AGS_PLATFORM_OS_WINDOWS (0)
    #define AGS_PLATFORM_OS_LINUX   (0)
    #define AGS_PLATFORM_OS_MACOS   (0)
    #define AGS_PLATFORM_OS_ANDROID (1)
    #define AGS_PLATFORM_OS_IOS     (0)
    #define AGS_PLATFORM_OS_PSP     (0)
#elif defined(_WIN32)
    //define something for Windows (32-bit and 64-bit)
    #define AGS_PLATFORM_OS_WINDOWS (1)
    #define AGS_PLATFORM_OS_LINUX   (0)
    #define AGS_PLATFORM_OS_MACOS   (0)
    #define AGS_PLATFORM_OS_ANDROID (0)
    #define AGS_PLATFORM_OS_IOS     (0)
    #define AGS_PLATFORM_OS_PSP     (0)
#elif defined(__APPLE__)
    #include "TargetConditionals.h"
    #ifndef TARGET_OS_SIMULATOR
        #define TARGET_OS_SIMULATOR (0)
    #endif
    #ifndef TARGET_OS_IOS
        #define TARGET_OS_IOS (0)
    #endif
    #ifndef TARGET_OS_OSX
        #define TARGET_OS_OSX (0)
    #endif

    #if TARGET_OS_SIMULATOR || TARGET_IPHONE_SIMULATOR
        #define AGS_PLATFORM_OS_WINDOWS (0)
        #define AGS_PLATFORM_OS_LINUX   (0)
        #define AGS_PLATFORM_OS_MACOS   (0)
        #define AGS_PLATFORM_OS_ANDROID (0)
        #define AGS_PLATFORM_OS_IOS     (1)
        #define AGS_PLATFORM_OS_PSP     (0)
    #elif TARGET_OS_IOS || TARGET_OS_IPHONE
        #define AGS_PLATFORM_OS_WINDOWS (0)
        #define AGS_PLATFORM_OS_LINUX   (0)
        #define AGS_PLATFORM_OS_MACOS   (0)
        #define AGS_PLATFORM_OS_ANDROID (0)
        #define AGS_PLATFORM_OS_IOS     (1)
        #define AGS_PLATFORM_OS_PSP     (0)
    #elif TARGET_OS_OSX || TARGET_OS_MAC
        #define AGS_PLATFORM_OS_WINDOWS (0)
        #define AGS_PLATFORM_OS_LINUX   (0)
        #define AGS_PLATFORM_OS_MACOS   (1)
        #define AGS_PLATFORM_OS_ANDROID (0)
        #define AGS_PLATFORM_OS_IOS     (0)
        #define AGS_PLATFORM_OS_PSP     (0)
    #else
        #error "Unknown Apple platform"
    #endif
#elif defined(__linux__)
    #define AGS_PLATFORM_OS_WINDOWS (0)
    #define AGS_PLATFORM_OS_LINUX   (1)
    #define AGS_PLATFORM_OS_MACOS   (0)
    #define AGS_PLATFORM_OS_ANDROID (0)
    #define AGS_PLATFORM_OS_IOS     (0)
    #define AGS_PLATFORM_OS_PSP     (0)
#else
    #error "Unknown platform"
#endif


#if defined(__LP64__)
    // LP64 machine, OS X or Linux
    // int 32bit | long 64bit | long long 64bit | void* 64bit
    #define AGS_PLATFORM_64BIT (1)
#elif defined(_WIN64)
    // LLP64 machine, Windows
    // int 32bit | long 32bit | long long 64bit | void* 64bit
    #define AGS_PLATFORM_64BIT (1)
#else
    // 32-bit machine, Windows or Linux or OS X
    // int 32bit | long 32bit | long long 64bit | void* 32bit
    #define AGS_PLATFORM_64BIT (0)
#endif

#if defined(_WIN32)
    #define AGS_PLATFORM_ENDIAN_LITTLE  (1)
    #define AGS_PLATFORM_ENDIAN_BIG     (0)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define AGS_PLATFORM_ENDIAN_LITTLE  (1)
    #define AGS_PLATFORM_ENDIAN_BIG     (0)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define AGS_PLATFORM_ENDIAN_LITTLE  (0)
    #define AGS_PLATFORM_ENDIAN_BIG     (1)
#else
    #error "Unknown platform"
#endif

#if defined(_DEBUG)
    #define AGS_PLATFORM_DEBUG  (1)
#elif ! defined(NDEBUG)
    #define AGS_PLATFORM_DEBUG  (1)
#else
    #define AGS_PLATFORM_DEBUG  (0)
#endif

#endif // __AC_PLATFORM_H