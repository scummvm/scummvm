/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GLK_AGT_CONFIG
#define GLK_AGT_CONFIG

#include "glk/glk_api.h"
#include "common/stream.h"

namespace Glk {
namespace AGT {

/*
   This file contains most of the configuration information
      including the platform-dependent #define statements
   It's in three major sections:
     --Platform specific defines for various platforms, each
       surrounded by "#ifdef <platform>" and "#endif"
     --Various defaults
     --Filename extensions

   Ideally, a port to a new platform should only need to modify this
    file, the makefile, os_<whatever>.c, and possibly filename.c.  (In
    practice, you may also need to tweak the the high-level I/O code
    in interface.c or the memory-allocation code in util.c.  If you
    find yourself needing to do more than that, get in touch with me.)  */

#undef _WIN32 /* GARGLK */


/* Default to PLAIN platform */
/* At the moment, you can replace this with LINUX, HPUX, AMIGA,     */
/*   MSDOS, SUN, or NEXT; some of these may require the correct os_... */
/*   file to work                                                   */
/*   (In particular, AMIGA requires David Kinder's os_amiga.c file) */
/* The actual platform specific defines don't start until a few     */
/*   lines down, past the #includes and the definition of global    */
#ifndef PLAIN
#define PLAIN
#endif

/* ------------------------------------------------------------------- */
/* PLATFORM SPECIFIC DEFINITIONS, ETC. */
/*  See agility.doc or porting.txt for more information. */
/* Things you can currently define: */
/*    fix_ascii: 1=translate IBM character set, 0=don't */
/*    NEED_STR_CMP: define if strcasecmp() not defined on your system */
/*    NEED_STRN_CMP: define if strncasecmp() not defined on your system */
/*    HAVE_STRDUP: define if strdup() exists on your system */
/*    REPLACE_GETFILE: define if you replace the default get_user_file(). */
/*    REPLACE_MENU if you replace agt_menu().                 */
/*    REPLACE_MAIN: define if you replace the default main(). */
/*  (replacements should be defined in the relevant os_<platform>.c file) */
/*    DA1,DA2,...DA6,DSS,pTTL: file name extensions for the various AGT
            files */
/*   HAVE_SLEEP if your platform has the sleep() function */
/*   BUFF_SIZE is the maximum size of the buffer to use when reading
       in files. Regardless, it will be made no bigger than the file
       being read in and no smaller than the record size; thus setting
       it to 0 will cause the smallest buffer to always be used and
       setting this to 1MB will in practice always use a buffer the
       sizs of the file. It defaults to 32K */
/*   CBUF_SIZE is the maximum size of the buffer used for reading in
       the Master's Edition DA6 files; the size of the buffer in bytes
       is twice this value (since an individual token is two bytes long). */
/*   DESCR_BUFFSIZE is the maximum size of the description text block before
       the interpreter will read it from disk rather than storing it in
       memory during play. At the moment this only affects AGX games;
       original AGT games always use the disk. */
/*   DOHASH to use a hash table for dictionary searches; the only
       reason not to have this would be memory */
/*   HASHBITS determines the size of the hash table: (2^HASHBITS)*sizeof(word);
       the hash table must be at least as large as the dictionary.
       In practice this means HASHBITS should be at least 12;
       this is the current default. */
/*   MAXSTRUC The maximum size (in chars) which a single data structure can
       be on this platform. This defaults to 1MB (i.e. no limit for
       practical purposes). In practice I know of no game files that
       require any structures bigger than about 30K.  */
/*   LOWMEM Define this if you are low on memory. At the moment this
       only saves a few K.*/
/*   PORTSTR Is the string describing this particular port.
        e.g. #define PORTSTR "OrfDOS Port by R.J. Wright" */
/*   UNIX_IO  if you have Unix-like low level file I/O functions.
       (MS-DOS, for example, does). This speeds up the reading
       of the large game data files on some platforms. If this is
       defined, READFLAG, WRITEFLAG, and FILE_PERM also need to
       be defined. (Giving the flags needed for opening a file for
       reading or writing, and the file permissions to be given to newly
       created files. */
/*   OPEN_AS_TEXT  Define to cause text files to be opened as text files. */
/*   PREFIX_EXT  Add filename extensions at the beginning of the name,
       rather than at the end. */
/*   PATH_SEP, if defined, is a string containing all characters which
       can be used to separate the path from the filename. */
/*   pathtest(s) is a macro that should check whether the given string
        is an absolute path.  If this is left undefined, then _all_
        paths will be treated as absolute. You don't need to define
        this if you are replacing filename.c.  */
/* ------------------------------------------------------------------- */

/* force16 is used purely for debugging purposes, to make sure that
   everything works okay even with 16-bit ints */
/* #define force16  */

#define DOHASH

/*
 * The Glk port is very similar to plain ASCII, to give it the best
 * chance at success on multiple Glk platforms.  The only basic change
 * is to turn off IBM character translations; Glk works in ISO 8859
 * Latin-1, which can offer slightly closer translation of the IBM
 * code page 437 characters that the simpler mappings in the core
 * AGiliTy code.  The os_glk.c module handles the translations.
 */
#ifdef GARGLK
#define NEED_STR_CMP            /* Inherited from PLAIN. */
#define NEED_STRN_CMP           /* Inherited from PLAIN. */
#define BUFF_SIZE   0       /* Inherited from PLAIN. */
#define CBUF_SIZE   (5000L)     /* Inherited from PLAIN. */
#define INBUFF_SIZE (1024)      /* Inherited from PLAIN. */
#define fix_ascii   0       /* os_glk.c does translations. */
#define MAXSTRUC    (1024L*1024L)   /* 32Kb from PLAIN is too small for
                       several games (including Soggy). */
#define PORTSTR     "Glk version"   /* Identify ourselves discreetly. */
#define REPLACE_GETFILE         /* Override get_user_file. */
#define REPLACE_MAIN            /* Override main. */
#define fnamecmp    strcasecmp  /* Case insensitive filename compare. */
#undef PLAIN

#endif

/* PLAIN should always come last, giving everyone else a chance
 to #undef it. */
#ifdef PLAIN  /* This should work if nothing else does */
#define NEED_STR_CMP
#define NEED_STRN_CMP
#define BUFF_SIZE 0
#define CBUF_SIZE (5000L)
#define INBUFF_SIZE (1024)     /* Used by Magx */
#define MAXSTRUC (32L*1024L) /* IIRC, 32K is the minimum required by
                the ANSI standard */
#define PORTSTR "Pure ANSI C version"
#endif


/* __GNUC__ */


/* ------------------------------------------------------------------- */
/* DEFAULTS FOR "PLATFORM SPECIFIC" DEFINES */
/* ------------------------------------------------------------------- */

#ifdef __STRICT_ANSI__
#define NEED_STR_CMP
#define NEED_STRN_CMP
#endif

#ifndef fix_ascii
#define fix_ascii 1  /* Translate IBM character set by default */
#endif

#ifndef BUFF_SIZE
#ifdef LOWMEM
#define BUFF_SIZE 0   /* i.e. unbuffered */
#else
#define BUFF_SIZE (32L*1024L)  /* 32K */
#endif
#endif /* BUFF_SIZE */

#ifndef MAXSTRUC
#define MAXSTRUC (1024L*1024L)
#endif

#ifndef DESCR_BUFFSIZE
#define DESCR_BUFFSIZE 0  /* Always load descriptions from disk */
#endif

#ifndef HASHBITS
#ifdef LOWMEM
#define HASHBITS 12   /* 4K entries */
#else
#define HASHBITS 13   /* 8K entries in hash table */
#endif
#endif /* HASHBITS */

#ifndef fnamecmp   /* Used to compare filenames */
#define fnamecmp strcmp
#endif

#ifndef fnamencmp  /* Also used to compare filenames */
#define fnamencmp strncmp
#endif

/* If DOSFARDATA hasn't been defined, define it as the empty string. */
#ifndef DOSFARDATA
#define DOSFARDATA
#endif

/* ---------------------------------------------------------------------- */
/* FILENAME EXTENSIONS                                */
/* These are the various filename extensions for the different data files.*/
/* ---------------------------------------------------------------------- */

/* The following are only used by the interpreter, agtout, and agt2agx */
#ifndef DA1
#define DA1 ".da1"   /* General info (text file) */
#define DA2 ".da2"   /* Rooms */
#define DA3 ".da3"   /* Items */
#define DA4 ".da4"   /* Creatures */
#define DA5 ".da5"   /* Commands, headers */
#define DA6 ".da6"   /* Commands, code (Master's Ed only) */
#define DSS ".d$$"   /* Description strings */
#define pHNT ".hnt"  /* Popup hint file; not used yet. */
#define pOPT ".opt"  /* Interface specification file */
#endif

/* The following are only used by the Magx compiler */
#ifndef pAGT
#define pAGT ".agt"
#define pDAT ".dat"
#define pMSG ".msg"
#define pCMD ".cmd"
#define pSTD ".std"
#define AGTpSTD "agt.std"  /* Default error message file */
#endif

/* The following are used by both the interpreter and the compiler */
#ifndef pAGX
#define pAGX ".agx"  /* Extension for new Adventure Game eXecutable format */
#define pTTL ".ttl"  /* Title file */
#define pINS ".ins"  /* Instruction file */
#define pVOC ".voc"  /* Menu vocabulary file */
#define pCFG ".cfg"  /* Game configuration file */
#define pEXT "."  /* Separator between extension and base of filename */
#endif


#ifndef pSAV
#define pSAV ".sav"  /* Extension for save files */
#endif

#ifndef pSCR
#define pSCR ".scr"  /* Script file */
#endif

#ifndef pLOG
#define pLOG ".log"  /* LOG/REPLAY file */
#endif





/* Finally, two potentially platform dependent type defintions,
   for binary and text files respectively.  Don't change these
   unless you are also changing filename.c */

typedef Common::Stream *genfile;
typedef char *file_id_type;  /* i.e. the filename */

#define NO_FILE_ID NULL
#define BAD_TEXTFILE NULL
#define BAD_BINFILE  NULL

} // End of namespace AGT
} // End of namespace Glk

#endif
