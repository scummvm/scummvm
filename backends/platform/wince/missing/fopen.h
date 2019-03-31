/* Header is not present in Windows CE SDK */

extern "C" {
/* This stuff will live here until port configuration file is in place */
#ifndef _FILE_DEFINED
typedef void FILE;
#define _FILE_DEFINED
#endif
FILE *wce_fopen(const char *fname, const char *fmode);
#define fopen wce_fopen
}
