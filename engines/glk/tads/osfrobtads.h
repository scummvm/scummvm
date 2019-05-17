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

/* OS-layer functions and macros.
 *
 * This file does not introduce any curses (or other screen-API)
 * dependencies; it can be used for both the interpreter as well as the
 * compiler.
 */

#ifndef GLK_TADS_OSFROBTADS
#define GLK_TADS_OSFROBTADS

#include "common/fs.h"
#include "common/stream.h"
#include "glk/glk_api.h"
#include "glk/tads/os_filetype.h"

namespace Glk {
namespace TADS {

#define OSPATHCHAR '/'
#define OSPATHALT ""
#define OSPATHURL "/"
#define OSPATHSEP ':'
#define OS_NEWLINE_SEQ "\n"


/* Defined for Gargoyle. */
#define HAVE_STDINT_

#if 0
#include "common.h"
#endif

/* Used by the base code to inhibit "unused parameter" compiler warnings. */
#ifndef VARUSED
#define VARUSED(var) (void)var
#endif

/* We assume that the C-compiler is mostly ANSI compatible. */
#define OSANSI

/* Special function qualifier needed for certain types of callback
 * functions.  This is for old 16-bit systems; we don't need it and
 * define it to nothing. */
#define OS_LOADDS

/* Unices don't suffer the near/far pointers brain damage (thank God) so
 * we make this a do-nothing macro. */
#define osfar_t

/* This is used to explicitly discard computed values (some compilers
 * would otherwise give a warning like "computed value not used" in some
 * cases).  Casting to void should work on every ANSI-Compiler. */
#define DISCARD (void)

/* Copies a struct into another.  ANSI C allows the assignment operator
 * to be used with structs. */
#define OSCPYSTRUCT(x,y) ((x)=(y))

/* Link error messages into the application. */
#define ERR_LINK_MESSAGES

/* Program Exit Codes. */
#define OSEXSUCC 0 /* Successful completion. */
#define OSEXFAIL 1 /* Failure. */

/* Here we configure the osgen layer; refer to tads2/osgen3.c for more
 * information about the meaning of these macros. */
#define USE_DOSEXT
#define USE_NULLSTYPE

/* Theoretical maximum osmalloc() size.
 * Unix systems have at least a 32-bit memory space.  Even on 64-bit
 * systems, 2^32 is a good value, so we don't bother trying to find out
 * an exact value. */
#define OSMALMAX 0xffffffffL

#define OSFNMAX 255

/**
 * File handle structure for osfxxx functions
 * Note that we need to define it as a Common::Stream since the type is used by
 * TADS for both reading and writing files
 */
typedef Common::Stream osfildef;

/* Directory handle for searches via os_open_dir() et al. */
typedef Common::FSNode *osdirhdl_t;

/* file type/mode bits */
#define OSFMODE_FILE    S_IFREG
#define OSFMODE_DIR     S_IFDIR
#define OSFMODE_CHAR    S_IFCHR
#define OSFMODE_BLK     S_IFBLK
#define OSFMODE_PIPE    S_IFIFO
#ifdef S_IFLNK
#define OSFMODE_LINK    S_IFLNK
#else
#define OSFMODE_LINK    0
#endif
#ifdef S_IFSOCK
#define OSFMODE_SOCKET  S_IFSOCK
#else
#define OSFMODE_SOCKET  0
#endif

/* File attribute bits. */
#define OSFATTR_HIDDEN  0x0001
#define OSFATTR_SYSTEM  0x0002
#define OSFATTR_READ    0x0004
#define OSFATTR_WRITE   0x0008

/* Get a file's stat() type. */
int osfmode( const char* fname, int follow_links, unsigned long* mode,
             unsigned long* attr );

#if 0
/* The maximum width of a line of text.
 *
 * We ignore this, but the base code needs it defined.  If the
 * interpreter is run inside a console or terminal with more columns
 * than the value defined here, weird things will happen, so we go safe
 * and use a large value. */
#define OS_MAXWIDTH 255
#endif

/* Disable the Tads swap file; computers have plenty of RAM these days.
 */
#define OS_DEFAULT_SWAP_ENABLED 0

/* TADS 2 macro/function configuration.  Modern configurations always
 * use the no-macro versions, so these definitions should always be set
 * as shown below. */
#define OS_MCM_NO_MACRO
#define ERR_NO_MACRO

/* These values are used for the "mode" parameter of osfseek() to
 * indicate where to seek in the file. */
#define OSFSK_SET SEEK_SET /* Set position relative to the start of the file. */
#define OSFSK_CUR SEEK_CUR /* Set position relative to the current file position. */
#define OSFSK_END SEEK_END /* Set position relative to the end of the file. */


/* ============= Functions follow ================ */

/* Allocate a block of memory of the given size in bytes. */
#define osmalloc malloc

/* Free memory previously allocated with osmalloc(). */
#define osfree free

/* Reallocate memory previously allocated with osmalloc() or osrealloc(),
 * changing the block's size to the given number of bytes. */
#define osrealloc realloc

/* Open text file for reading. */
#define osfoprt(fname,typ) (fopen((fname),"r"))

/* Open text file for writing. */
#define osfopwt(fname,typ) (fopen((fname),"w"))

/* Open text file for reading and writing, keeping the file's existing
 * contents if the file already exists or creating a new file if no
 * such file exists. */
osfildef *osfoprwt(const char *fname, os_filetype_t typ);

/* Open text file for reading/writing.  If the file already exists,
 * truncate the existing contents.  Create a new file if it doesn't
 * already exist. */
#define osfoprwtt(fname,typ) (fopen((fname),"w+"))

/* Open binary file for writing. */
#define osfopwb(fname,typ) (fopen((fname),"wb"))

/* Open source file for reading - use the appropriate text or binary
 * mode. */
#define osfoprs osfoprt

/* Open binary file for reading. */
inline osfildef *osfoprb(const char *fname, os_filetype_t typ);

/* Open binary file for reading/writing.  If the file already exists,
 * keep the existing contents.  Create a new file if it doesn't already
 * exist. */
osfildef*
osfoprwb(const char *fname, os_filetype_t typ);

/* Open binary file for writing.  If the file already exists,
 * truncate the existing contents.  Create a new file if it doesn't
 * already exist. */
inline osfildef *osfoprwtb(const char *fname, os_filetype_t typ);

/* Get a line of text from a text file. */
#define osfgets fgets

/* Write a line of text to a text file. */
#define osfputs fputs

/* Write bytes to file. */
inline bool osfwb(osfildef *fp, void *buf, size_t count);

/* Flush buffered writes to a file. */
#define osfflush fflush

/* Read bytes from file. */
int osfrb(osfildef *fp, void *buf, size_t count);

/* Read bytes from file and return the number of bytes read. */
#define osfrbc(fp,buf,bufl) (fread((buf),1,(bufl),(fp)))

/* Get the current seek location in the file. */
#define osfpos ftell

/* Seek to a location in the file. */
#define osfseek fseek

/* Close a file. */
#define osfcls delete

/* Delete a file. */
#define osfdel remove

/* Access a file - determine if the file exists.
 *
 * We map this to the access() function.  It should be available in
 * virtually every system out there, as it appears in many standards
 * (SVID, AT&T, POSIX, X/OPEN, BSD 4.3, DOS, MS Windows, maybe more). */
#define osfacc(fname) (access((fname), F_OK))

/* Rename a file. */
#define os_rename_file(from, to) (rename(from, to) == 0)

/* Get a file's stat() type. */
struct os_file_stat_t;
int os_file_stat( const char* fname, int follow_links,
                  struct os_file_stat_t* s );

/* Get a character from a file. */
#define osfgetc fgetc

/* Set busy cursor.
 *
 * We don't have a mouse cursor so there's no need to implement this. */
#define os_csr_busy(a)

/* Update progress display.
 *
 * We don't provide any kind of "compilation progress display", so we
 * just define this as an empty macro.
 */
#define os_progress(fname,linenum)

} // End of namespace TADS
} // End of namespace Glk

#endif
