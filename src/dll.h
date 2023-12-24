#ifdef _WIN32
    #ifdef __GNUC__
        #define DLL_EXPORT __attribute__ ((dllexport))
        #define DLL_IMPORT __attribute__ ((dllimport))
    #else
        #define DLL_EXPORT __declspec((dllexport))
        #define DLL_IMPORT __declspec((dllimport))
    #endif
#else
    #if __GNUC__ >= 4
        #define DLL_EXPORT __attribute__ ((visibility ("default")))
        #define DLL_IMPORT __attribute__ ((visibility ("default")))
    #else
        #define DLL_EXPORT
        #define DLL_IMPORT
    #endif
#endif
