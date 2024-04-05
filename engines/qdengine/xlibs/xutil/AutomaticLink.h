// define parameter _LIB_NAME

#if defined(_AFXDLL)
#define _MT_DLL_SUFFIX "Dll"
#elif defined(_DLL) || defined(_MTD)
#define _MT_DLL_SUFFIX "Dll"
#elif defined(_MT)
#define _MT_DLL_SUFFIX "Mt"
#else
#define _MT_DLL_SUFFIX 
#endif 

#ifdef _DEBUG
#define _DEBUG_SUFFIX "Dbg"
#else 
#define _DEBUG_SUFFIX 
#endif 

#define _FULL_NAME_ _LIB_NAME _MT_DLL_SUFFIX _DEBUG_SUFFIX ".lib"
#pragma message("Automatically linking with " _FULL_NAME_) 
#pragma comment(lib, _FULL_NAME_) 

#undef _FULL_NAME_
#undef _DEBUG_SUFFIX
#undef _MT_DLL_SUFFIX
#undef _LIB_NAME

