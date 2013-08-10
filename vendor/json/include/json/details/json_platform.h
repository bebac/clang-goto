// ----------------------------------------------------------------------------
#ifndef __json__platform_h__
#define __json__platform_h__
// ----------------------------------------------------------------------------

#ifdef _WIN32

    // Visual Studio does not support noexcept.
    #define noexcept

    // Visual Studio does not support std::initializer_list.
    #undef __JSON__HAS_STD_INITIALIZER_LIST

    // Visual Studio does not support using.
    #undef __JSON__HAS_CPP_USING_SUPPORT

#else
    // Assume GCC - TODO: Need to check version should probably be >= 4.7
    
    #define __JSON__HAS_STD_INITIALIZER_LIST
    #define __JSON__HAS_CPP_USING_SUPPORT

#endif

// ----------------------------------------------------------------------------
#endif // __json__platform_h__
