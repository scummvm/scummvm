/*
  Copyright (c) 1990-2006 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2004-May-22 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/

/* Automatic setting of the common Microsoft C idenfifier MSC.
 * NOTE: Watcom also defines M_I*86 !
 */
#if defined(_MSC_VER) || (defined(M_I86) && !defined(__WATCOMC__))
#  ifndef MSC
#    define MSC                 /* This should work for older MSC, too!  */
#  endif
#endif

/* Tell Microsoft Visual C++ 2005 to leave us alone and
 * let us use standard C functions the way we're supposed to.
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  ifndef _CRT_SECURE_NO_DEPRECATE
#    define _CRT_SECURE_NO_DEPRECATE
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#endif

#if defined(__WATCOMC__) && defined(__386__)
#  define WATCOMC_386
#endif

#if (defined(__CYGWIN32__) && !defined(__CYGWIN__))
#  define __CYGWIN__            /* compatibility for CygWin B19 and older */
#endif

/* enable multibyte character set support by default */
#ifndef _MBCS
#  define _MBCS
#endif
#if defined(__CYGWIN__)
#  undef _MBCS
#endif

#ifndef MSDOS
/*
 * Windows 95 (and Windows NT) file systems are (to some extend)
 * extensions of MSDOS. Common features include for example:
 *      FAT or (FAT like) file systems,
 *      '\\' as directory separator in paths,
 *      "\r\n" as record (line) terminator in text files, ...
 */
#  define MSDOS
/* inherit MS-DOS file system etc. stuff */
#endif

#define USE_CASE_MAP
#define PROCNAME(n) (action == ADD || action == UPDATE ? wild(n) : \
                     procname(n, 1))
#define BROKEN_FSEEK
#ifndef __RSXNT__
#  define HAVE_FSEEKABLE
#endif

/* File operations--use "b" for binary if allowed or fixed length 512 on VMS
 *                  use "S" for sequential access on NT to prevent the NT
 *                  file cache eating up memory with large .zip files
 */
#define FOPR "rb"
#define FOPM "r+b"
#define FOPW "wbS"

#if (defined(__CYGWIN__) && !defined(NO_MKTIME))
#  define NO_MKTIME             /* Cygnus' mktime() implementation is buggy */
#endif
#if (!defined(NT_TZBUG_WORKAROUND) && !defined(NO_NT_TZBUG_WORKAROUND))
#  define NT_TZBUG_WORKAROUND
#endif
#if (defined(UTIL) && defined(NT_TZBUG_WORKAROUND))
#  undef NT_TZBUG_WORKAROUND    /* the Zip utilities do not use time-stamps */
#endif
#if !defined(NO_EF_UT_TIME) && !defined(USE_EF_UT_TIME)
#  define USE_EF_UT_TIME
#endif
#if (!defined(NO_NTSD_EAS) && !defined(NTSD_EAS))
#  define NTSD_EAS
#endif

#if (defined(NTSD_EAS) && !defined(ZP_NEED_MEMCOMPR))
#  define ZP_NEED_MEMCOMPR
#endif

#ifdef WINDLL
# ifndef NO_ASM
#   define NO_ASM
# endif
# ifndef MSWIN
#   define MSWIN
# endif
# ifndef REENTRANT
#   define REENTRANT
# endif
#endif /* WINDLL */

/* Enable use of optimized x86 assembler version of longest_match() for
   MSDOS, WIN32 and OS2 per default.  */
#if !defined(NO_ASM) && !defined(ASMV)
#  define ASMV
#endif

/* Enable use of optimized x86 assembler version of crc32() for
   MSDOS, WIN32 and OS2 per default.  */
#if !defined(NO_ASM) && !defined(ASM_CRC)  && !defined(NO_ASM_CRC)
#  define ASM_CRC
#endif

#if !defined(__GO32__) && !defined(__EMX__) && !defined(__CYGWIN__)
#  define NO_UNISTD_H
#endif

/* the following definitions are considered as "obsolete" by Microsoft and
 * might be missing in some versions of <windows.h>
 */
#ifndef AnsiToOem
#  define AnsiToOem CharToOemA
#endif
#ifndef OemToAnsi
#  define OemToAnsi OemToCharA
#endif

#if (defined(__RSXNT__) && defined(__CRTRSXNT__))
#  include <crtrsxnt.h>
#endif

/* Get types and stat */
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#ifdef _MBCS
#  if (!defined(__EMX__) && !defined(__MINGW32__) && !defined(__CYGWIN__))
#    include <stdlib.h>
#    include <mbstring.h>
#  endif
#  if (defined(__MINGW32__) && !defined(MB_CUR_MAX))
#    ifdef __MSVCRT__
       extern int *__p___mb_cur_max(void);
#      define MB_CUR_MAX (*__p___mb_cur_max())
#    else
       extern int *_imp____mb_cur_max_dll;
#      define MB_CUR_MAX (*_imp____mb_cur_max_dll)
#    endif
#  endif
#  if (defined(__LCC__) && !defined(MB_CUR_MAX))
     extern int *_imp____mb_cur_max;
#    define MB_CUR_MAX (*_imp____mb_cur_max)
#  endif
#endif

#ifdef __LCC__
#  include <time.h>
#  ifndef tzset
#    define tzset _tzset
#  endif
#  ifndef utime
#    define utime _utime
#  endif
#endif
#ifdef __MINGW32__
   extern void _tzset(void);            /* this is missing in <time.h> */
#  ifndef tzset
#    define tzset _tzset
#  endif
#endif
#if (defined(__RSXNT__) || defined(__EMX__)) && !defined(tzset)
#  define tzset _tzset
#endif
#ifdef W32_USE_IZ_TIMEZONE
#  ifdef __BORLANDC__
#    define tzname tzname
#    define IZTZ_DEFINESTDGLOBALS
#  endif
#  ifndef tzset
#    define tzset _tzset
#  endif
#  ifndef timezone
#    define timezone _timezone
#  endif
#  ifndef daylight
#    define daylight _daylight
#  endif
#  ifndef tzname
#    define tzname _tzname
#  endif
#  if (!defined(NEED__ISINDST) && !defined(__BORLANDC__))
#    define NEED__ISINDST
#  endif
#  ifdef IZTZ_GETLOCALETZINFO
#    undef IZTZ_GETLOCALETZINFO
#  endif
#  define IZTZ_GETLOCALETZINFO GetPlatformLocalTimezone
#endif /* W32_USE_IZ_TIMEZONE */

#ifdef MATCH
#  undef MATCH
#endif
#define MATCH dosmatch          /* use DOS style wildcard matching */

#ifdef ZCRYPT_INTERNAL
#  ifdef WINDLL
#    define ZCR_SEED2     (unsigned)3141592654L /* use PI as seed pattern */
#  else
#    include <process.h>        /* getpid() declaration for srand seed */
#  endif
#endif

/* Up to now, all versions of Microsoft C runtime libraries lack the support
 * for customized (non-US) switching rules between daylight saving time and
 * standard time in the TZ environment variable string.
 * But non-US timezone rules are correctly supported when timezone information
 * is read from the OS system settings in the Win32 registry.
 * The following work-around deletes any TZ environment setting from
 * the process environment.  This results in a fallback of the RTL time
 * handling code to the (correctly interpretable) OS system settings, read
 * from the registry.
 */
#ifdef USE_EF_UT_TIME
# if (defined(__WATCOMC__) || defined(__CYGWIN__) || \
      defined(W32_USE_IZ_TIMEZONE))
#   define iz_w32_prepareTZenv()
# else
#   define iz_w32_prepareTZenv()        putenv("TZ=")
# endif
#endif

/* This patch of stat() is useful for at least three compilers.  It is   */
/* difficult to take a stat() of a root directory under Windows95, so  */
/* zstat_zipwin32() detects that case and fills in suitable values.    */
#ifndef __RSXNT__
#  ifndef W32_STATROOT_FIX
#    define W32_STATROOT_FIX
#  endif
#endif /* !__RSXNT__ */

#if (defined(NT_TZBUG_WORKAROUND) || defined(W32_STATROOT_FIX))
#  define W32_STAT_BANDAID
   int zstat_zipwin32(const char *path, struct stat *buf);
#  ifdef SSTAT
#    undef SSTAT
#  endif
#  define SSTAT zstat_zipwin32
#endif /* NT_TZBUG_WORKAROUND || W32_STATROOT_FIX */

int getch_win32(void);

#ifdef __GNUC__
# define IZ_PACKED      __attribute__((packed))
#else
# define IZ_PACKED
#endif

/* for some (all ?) versions of IBM C Set/2 and IBM C Set++ */
#ifndef S_IFMT
#  define S_IFMT 0xF000
#endif /* !S_IFMT */

#ifdef __WATCOMC__
#  include <stdio.h>    /* PATH_MAX is defined here */
#  define NO_MKTEMP

/* Get asm routines to link properly without using "__cdecl": */
#  ifdef __386__
#    ifdef ASMV
#      pragma aux match_init    "_*" parm caller [] modify []
#      pragma aux longest_match "_*" parm caller [] value [eax] \
                                      modify [eax ecx edx]
#    endif
#    if defined(ASM_CRC) && !defined(USE_ZLIB)
#      pragma aux crc32         "_*" parm caller [] value [eax] modify [eax]
#      pragma aux get_crc_table "_*" parm caller [] value [eax] \
                                      modify [eax ecx edx]
#    endif /* ASM_CRC && !USE_ZLIB */
#  endif /* __386__ */
#endif /* __WATCOMC__ */
