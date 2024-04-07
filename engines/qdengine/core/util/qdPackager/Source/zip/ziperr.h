/*
  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2005-Feb-10 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
 *  ziperr.h by Mark Adler
 */

/* Error return values.  The values 0..4 and 12..18 follow the conventions
   of PKZIP.   The values 4..10 are all assigned to "insufficient memory"
   by PKZIP, so the codes 5..10 are used here for other purposes. */
#define ZE_MISS         -1      /* used by procname(), zipbare() */
#define ZE_OK           0       /* success */
#define ZE_EOF          2       /* unexpected end of zip file */
#define ZE_FORM         3       /* zip file structure error */
#define ZE_MEM          4       /* out of memory */
#define ZE_LOGIC        5       /* internal logic error */
#define ZE_BIG          6       /* entry too large to split, read, or write */
#define ZE_NOTE         7       /* invalid comment format */
#define ZE_TEST         8       /* zip test (-T) failed or out of memory */
#define ZE_ABORT        9       /* user interrupt or termination */
#define ZE_TEMP         10      /* error using a temp file */
#define ZE_READ         11      /* read or seek error */
#define ZE_NONE         12      /* nothing to do */
#define ZE_NAME         13      /* missing or empty zip file */
#define ZE_WRITE        14      /* error writing to a file */
#define ZE_CREAT        15      /* couldn't open to write */
#define ZE_PARMS        16      /* bad command line */
#define ZE_OPEN         18      /* could not open a specified file to read */

#define ZE_MAXERR       18      /* the highest error number */

/* Macro to determine whether to call perror() or not */
#define PERR(e) (e==ZE_READ||e==ZE_WRITE||e==ZE_CREAT||e==ZE_TEMP||e==ZE_OPEN)

#ifdef GLOBALS
/* Error messages for the ziperr() function in the zip programs */
char *ziperrors[ZE_MAXERR] = {
/*  1 */  "",
/*  2 */  "Unexpected end of zip file",
/*  3 */  "Zip file structure invalid",
/*  4 */  "Out of memory",
/*  5 */  "Internal logic error",
/*  6 */  "Entry too big to split, read, or write",
/*  7 */  "Invalid comment format",
/*  8 */  "Zip file invalid or could not spawn unzip",
/*  9 */  "Interrupted",
/* 10 */  "Temporary file failure",
/* 11 */  "Input file read failure",
/* 12 */  "Nothing to do!",
/* 13 */  "Missing or empty zip file",
/* 14 */  "Output file write failure",
/* 15 */  "Could not create output file",
/* 16 */  "Invalid command arguments",
/* 17 */  "",
/* 18 */  "File not found or no read permission"
#  ifdef AZTEC_C
          ,     /* extremely lame compiler bug workaround */
#  endif
};
#else /* !GLOBALS */
extern char *ziperrors[ZE_MAXERR]; /* Error messages for ziperr() */
#endif /* ?GLOBALS */
