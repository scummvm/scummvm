/* Header is not present in Windows CE SDK */

/* This stuff will live here until port configuration file is in place */
#define stricmp _stricmp
#define strdup _strdup
#define _HEAPOK 0
#define _heapchk() 0

#ifndef _FILE_DEFINED
   typedef void FILE;
   #define _FILE_DEFINED
#endif
FILE* wce_fopen(const char* fname, const char* fmode);
#define fopen wce_fopen

