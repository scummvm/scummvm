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

#ifndef GLK_TADS_OS_FROB_TADS
#define GLK_TADS_OS_FROB_TADS

#include "common/fs.h"
#include "common/stream.h"
#include "glk/glk_api.h"
#include "glk/tads/os_filetype.h"

namespace Glk {
namespace TADS {

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

 /* ============= File Access ================ */

 /*
 *   Test access to a file - i.e., determine if the file exists.  Returns
 *   zero if the file exists, non-zero if not.  (The semantics may seem
 *   backwards, but this is consistent with the conventions used by most of
 *   the other osfxxx calls: zero indicates success, non-zero indicates an
 *   error.  If the file exists, "accessing" it was successful, so osfacc
 *   returns zero; if the file doesn't exist, accessing it gets an error,
 *   hence a non-zero return code.)
 */
int osfacc(const char *fname);


 /*
 *   Open text file for reading.  This opens the file with read-only access;
 *   we're not allowed to write to the file using this handle.  Returns NULL
 *   on error.
 *
 *   A text file differs from a binary file in that some systems perform
 *   translations to map between C conventions and local file system
 *   conventions; for example, on DOS, the stdio library maps the DOS CR-LF
 *   newline convention to the C-style '\n' newline format.  On many systems
 *   (Unix, for example), there is no distinction between text and binary
 *   files.
 *
 *   On systems that support file sharing and locking, this should open the
 *   file in "shared read" mode - this means that other processes are allowed
 *   to simultaneously read from the file, but no other processs should be
 *   allowed to write to the file as long as we have it open.  If another
 *   process already has the file open with write access, this routine should
 *   return failure, since we can't take away the write privileges the other
 *   process already has and thus we can't guarantee that other processes
 *   won't write to the file while we have it open.
 */
osfildef *osfoprt(const char *fname, os_filetype_t typ);

/*
*   Open a text file for "volatile" reading: we open the file with read-only
*   access, and we explicitly accept instability in the file's contents due
*   to other processes simultaneously writing to the file.  On systems that
*   support file sharing and locking, the file should be opened in "deny
*   none" mode, meaning that other processes can simultaneously open the
*   file for reading and/or writing even while have the file open.
*/
osfildef *osfoprtv(const char *fname, os_filetype_t typ);

/*
*   Open text file for writing; returns NULL on error.  If the file already
*   exists, this truncates the file to zero length, deleting any existing
*   contents.
*/
osfildef *osfopwt(const char *fname, os_filetype_t typ);

/*
*   Open text file for reading and writing, keeping the file's existing
*   contents if the file already exists or creating a new file if no such
*   file exists.  Returns NULL on error.
*/
osfildef *osfoprwt(const char *fname, os_filetype_t typ);

/*
*   Open text file for reading/writing.  If the file already exists,
*   truncate the existing contents to zero length.  Create a new file if it
*   doesn't already exist.  Return null on error.
*/
osfildef *osfoprwtt(const char *fname, os_filetype_t typ);

/*
*   Open binary file for writing; returns NULL on error.  If the file
*   exists, this truncates the existing contents to zero length.
*/
osfildef *osfopwb(const char *fname, os_filetype_t typ);

/*
*   Open source file for reading - use the appropriate text or binary
*   mode.
*/
osfildef *osfoprs(const char *fname, os_filetype_t typ);

/*
*   Open binary file for reading; returns NULL on error.
*/
osfildef *osfoprb(const char *fname, os_filetype_t typ);

/*
*   Open binary file for 'volatile' reading; returns NULL on error.
*   ("Volatile" means that we'll accept writes from other processes while
*   reading, so the file should be opened in "deny none" mode or the
*   equivalent, to the extent that the local system supports file sharing
*   modes.)
*/
osfildef *osfoprbv(const char *fname, os_filetype_t typ);

/*
*   Open binary file for random-access reading/writing.  If the file already
*   exists, keep the existing contents; if the file doesn't already exist,
*   create a new empty file.
*
*   The caller is allowed to perform any mixture of read and write
*   operations on the returned file handle, and can seek around in the file
*   to read and write at random locations.
*
*   If the local file system supports file sharing or locking controls, this
*   should generally open the file in something equivalent to "exclusive
*   write, shared read" mode ("deny write" in DENY terms), so that other
*   processes can't modify the file at the same time we're modifying it (but
*   it doesn't bother us to have other processes reading from the file while
*   we're working on it, as long as they don't mind that we could change
*   things on the fly).  It's not absolutely necessary to assert these
*   locking semantics, but if there's an option to do so this is preferred.
*   Stricter semantics (such as "exclusive" or "deny all" mode) are better
*   than less strict semantics.  Less strict semantics are dicey, because in
*   that case the caller has no way of knowing that another process could be
*   modifying the file at the same time, and no way (through osifc) of
*   coordinating that activity.  If less strict semantics are implemented,
*   the caller will basically be relying on luck to avoid corruptions due to
*   writing by other processes.
*
*   Return null on error.
*/
osfildef *osfoprwb(const char *fname, os_filetype_t typ);

/*
*   Open binary file for random-access reading/writing.  If the file already
*   exists, truncate the existing contents (i.e., delete the contents of the
*   file, resetting it to a zero-length file).  Create a new file if it
*   doesn't already exist.  The caller is allowed to perform any mixture of
*   read and write operations on the returned handle, and can seek around in
*   the file to read and write at random locations.
*
*   The same comments regarding sharing/locking modes for osfoprwb() apply
*   here as well.
*
*   Return null on error.
*/
osfildef *osfoprwtb(const char *fname, os_filetype_t typ);

/*
*   Duplicate a file handle.  Returns a new osfildef* handle that accesses
*   the same open file as an existing osfildef* handle.  The new handle is
*   independent of the original handle, with its own seek position,
*   buffering, etc.  The new handle and the original handle must each be
*   closed separately when the caller is done with them (closing one doesn't
*   close the other).  The effect should be roughly the same as the Unix
*   dup() function.
*
*   On success, returns a new, non-null osfildef* handle duplicating the
*   original handle.  Returns null on failure.
*
*   'mode' is a simplified stdio fopen() mode string.  The first
*   character(s) indicate the access type: "r" for read access, "w" for
*   write access, or "r+" for read/write access.  Note that "w+" mode is
*   specifically not defined, since the fopen() handling of "w+" is to
*   truncate any existing file contents, which is not desirable when
*   duplicating a handle.  The access type can optionally be followed by "t"
*   for text mode, "s" for source file mode, or "b" for binary mode, with
*   the same meanings as for the various osfop*() functions.  The default is
*   't' for text mode if none of these are specified.
*
*   If the osfop*() functions are implemented in terms of stdio FILE*
*   objects, this can be implemented as fdopen(dup(fileno(orig)), mode), or
*   using equivalents if the local stdio library uses different names for
*   these functions.  Note that "s" (source file format) isn't a stdio mode,
*   so implementations must translate it to the appropriate "t" or "b" mode.
*   (For that matter, "t" and "b" modes aren't universally supported either,
*   so some implementations may have to translate these, or more likely
*   simply remove them, as most platforms don't distinguish text and binary
*   modes anyway.)
*/
osfildef *osfdup(osfildef *orig, const char *mode);

/*
*   Set a file's type information.  This is primarily for implementations on
*   Mac OS 9 and earlier, where the file system keeps file-type metadata
*   separate from the filename.  On such systems, this can be used to set
*   the type metadata after a file is created.  The system should map the
*   os_filetype_t values to the actual metadata values on the local system.
*   On most systems, there's no such thing as file-type metadata, in which
*   case this function should simply be stubbed out with an empty function.
*/
void os_settype(const char *f, os_filetype_t typ);

/*
 *   Get a line of text from a text file.  Uses fgets semantics.
 */
char *osfgets(char *buf, size_t len, osfildef *fp);

/*
*   Write a line of text to a text file.  Uses fputs semantics.
*/
int osfputs(const char *buf, osfildef *fp);

/*
*   Write to a text file.  os_fprintz() takes a null-terminated string,
*   while os_fprint() takes an explicit separate length argument that might
*   not end with a null terminator.
*/
void os_fprintz(osfildef *fp, const char *str);
void os_fprint(osfildef *fp, const char *str, size_t len);

/*
*   Write bytes to file.  Return 0 on success, non-zero on error.
*/
int osfwb(osfildef *fp, const void *buf, size_t bufl);

/*
*   Flush buffered writes to a file.  This ensures that any bytes written to
*   the file (with osfwb(), os_fprint(), etc) are actually sent out to the
*   operating system, rather than being buffered in application memory for
*   later writing.
*
*   Note that this routine only guarantees that we write through to the
*   operating system.  This does *not* guarantee that the data will actually
*   be committed to the underlying physical storage device.  Such a
*   guarantee is hard to come by in general, since most modern systems use
*   multiple levels of software and hardware buffering - the OS might buffer
*   some data in system memory, and the physical disk drive might itself
*   buffer data in its own internal cache.  This routine thus isn't good
*   enough, for example, to protect transactional data that needs to survive
*   a power failure or a serious system crash.  What this routine *does*
*   ensure is that buffered data are written through to the OS; in
*   particular, this ensures that another process that's reading from the
*   same file will see all updates we've made up to this point.
*
*   Returns 0 on success, non-zero on error.  Errors can occur for any
*   reason that they'd occur on an ordinary write - a full disk, a hardware
*   failure, etc.
*/
int osfflush(osfildef *fp);

/*
*   Read a character from a file.  Provides the same semantics as fgetc().
*/
int osfgetc(osfildef *fp);

/*
*   Read bytes from file.  Return 0 on success, non-zero on error.
*/
int osfrb(osfildef *fp, void *buf, size_t bufl);

/*
*   Read bytes from file and return the number of bytes read.  0
*   indicates that no bytes could be read.
*/
size_t osfrbc(osfildef *fp, void *buf, size_t bufl);

/*
*   Get the current seek location in the file.  The first byte of the
*   file has seek position 0.
*/
long osfpos(osfildef *fp);

/*
*   Seek to a location in the file.  The first byte of the file has seek
*   position 0.  Returns zero on success, non-zero on error.
*
*   The following constants must be defined in your OS-specific header;
*   these values are used for the "mode" parameter to indicate where to
*   seek in the file:
*
*   OSFSK_SET - set position relative to the start of the file
*.  OSFSK_CUR - set position relative to the current file position
*.  OSFSK_END - set position relative to the end of the file
*/
int osfseek(osfildef *fp, long pos, int mode);

/*
*   Close a file.
*
*   If the OS implementation uses buffered writes, this routine guarantees
*   that any buffered data are flushed to the underlying file.  So, it's not
*   necessary to call osfflush() before calling this routine.  However,
*   since this function doesn't return any error indication, a caller could
*   use osfflush() first to check for errors on any final buffered writes.
*/
void osfcls(osfildef *fp);

/*
*   Delete a file.  Returns zero on success, non-zero on error.
*/
int osfdel(const char *fname);

/*
*   Rename/move a file.  This should apply the usual C rename() behavior.
*   Renames the old file to the new name, which may be in a new directory
*   location if supported on the local system; moves across devices,
*   volumes, file systems, etc may or may not be supported according to the
*   local system's rules.  If the new file already exists, results are
*   undefined.  Returns true on success, false on failure.
*/
int os_rename_file(const char *oldname, const char *newname);

/* ------------------------------------------------------------------------ */
/*
 *   Look for a file in the "standard locations": current directory, program
 *   directory, PATH-like environment variables, etc.  The actual standard
 *   locations are specific to each platform; the implementation is free to
 *   use whatever conventions are appropriate to the local system.  On
 *   systems that have something like Unix environment variables, it might be
 *   desirable to define a TADS-specific variable (TADSPATH, for example)
 *   that provides a list of directories to search for TADS-related files.
 *   
 *   On return, fill in 'buf' with the full filename of the located copy of
 *   the file (if a copy was indeed found), in a format suitable for use with
 *   the osfopxxx() functions; in other words, after this function returns,
 *   the caller should be able to pass the contents of 'buf' to an osfopxxx()
 *   function to open the located file.
 *   
 *   Returns true (non-zero) if a copy of the file was located, false (zero)
 *   if the file could not be found in any of the standard locations.  
 */
bool os_locate(const char *fname, int flen, const char *arg0,
              char *buf, size_t bufsiz);


/* ------------------------------------------------------------------------ */
/*
 *   Create and open a temporary file.  The file must be opened to allow
 *   both reading and writing, and must be in "binary" mode rather than
 *   "text" mode, if the system makes such a distinction.  Returns null on
 *   failure.
 *   
 *   If 'fname' is non-null, then this routine should create and open a file
 *   with the given name.  When 'fname' is non-null, this routine does NOT
 *   need to store anything in 'buf'.  Note that the routine shouldn't try
 *   to put the file in a special directory or anything like that; just open
 *   the file with the name exactly as given.
 *   
 *   If 'fname' is null, this routine must choose a file name and fill in
 *   'buf' with the chosen name; if possible, the file should be in the
 *   conventional location for temporary files on this system, and should be
 *   unique (i.e., it shouldn't be the same as any existing file).  The
 *   filename stored in 'buf' is opaque to the caller, and cannot be used by
 *   the caller except to pass to osfdel_temp().  On some systems, it may
 *   not be possible to determine the actual filename of a temporary file;
 *   in such cases, the implementation may simply store an empty string in
 *   the buffer.  (The only way the filename would be unavailable is if the
 *   implementation uses a system API that creates a temporary file, and
 *   that API doesn't return the name of the created temporary file.  In
 *   such cases, we don't need the name; the only reason we need the name is
 *   so we can pass it to osfdel_temp() later, but since the system is going
 *   to delete the file automatically, osfdel_temp() doesn't need to do
 *   anything and thus doesn't need the name.)
 *   
 *   After the caller is done with the file, it should close the file (using
 *   osfcls() as normal), then the caller MUST call osfdel_temp() to delete
 *   the temporary file.
 *   
 *   This interface is intended to take advantage of systems that have
 *   automatic support for temporary files, while allowing implementation on
 *   systems that don't have any special temp file support.  On systems that
 *   do have automatic delete-on-close support, this routine should use that
 *   system-level support, because it helps ensure that temp files will be
 *   deleted even if the caller fails to call osfdel_temp() due to a
 *   programming error or due to a process or system crash.  On systems that
 *   don't have any automatic delete-on-close support, this routine can
 *   simply use the same underlying system API that osfoprwbt() normally
 *   uses (although this routine must also generate a name for the temp file
 *   when the caller doesn't supply one).
 *   
 *   This routine can be implemented using ANSI library functions as
 *   follows: if 'fname' is non-null, return fopen(fname,"w+b"); otherwise,
 *   set buf[0] to '\0' and return tmpfile().  
 */
osfildef *os_create_tempfile(const char *fname, char *buf);

/*
 *   Delete a temporary file - this is used to delete a file created with
 *   os_create_tempfile().  For most platforms, this can simply be defined
 *   the same way as osfdel().  For platforms where the operating system or
 *   file manager will automatically delete a file opened as a temporary
 *   file, this routine should do nothing at all, since the system will take
 *   care of deleting the temp file.
 *   
 *   Callers are REQUIRED to call this routine after closing a file opened
 *   with os_create_tempfile().  When os_create_tempfile() is called with a
 *   non-null 'fname' argument, the same value should be passed as 'fname' to
 *   this function.  When os_create_tempfile() is called with a null 'fname'
 *   argument, then the buffer passed in the 'buf' argument to
 *   os_create_tempfile() must be passed as the 'fname' argument here.  In
 *   other words, if the caller explicitly names the temporary file to be
 *   opened in os_create_tempfile(), then that same filename must be passed
 *   here to delete the named file; if the caller lets os_create_tempfile()
 *   generate a filename, then the generated filename must be passed to this
 *   routine.
 *   
 *   If os_create_tempfile() is implemented using ANSI library functions as
 *   described above, then this routine can also be implemented with ANSI
 *   library calls as follows: if 'fname' is non-null and fname[0] != '\0',
 *   then call remove(fname); otherwise do nothing.  
 */
int osfdel_temp(const char *fname);

/*
 *   Get the temporary file path.  This should fill in the buffer with a
 *   path prefix (suitable for strcat'ing a filename onto) for a good
 *   directory for a temporary file, such as the swap file.  
 */
void os_get_tmp_path(char *buf);

/* 
 *   Generate a name for a temporary file.  This constructs a random file
 *   path in the system temp directory that isn't already used by an existing
 *   file.
 *   
 *   On systems with long filenames, this can be implemented by selecting a
 *   GUID-strength random name (such as 32 random hex digits) with a decent
 *   random number generator.  That's long enough that the odds of a
 *   collision are essentially zero.  On systems that only support short
 *   filenames, the odds of a collision are non-zero, so the routine should
 *   actually check that the chosen filename doesn't exist.
 *   
 *   Optionally, before returning, this routine *may* create (and close) an
 *   empty placeholder file to "reserve" the chosen filename.  This isn't
 *   required, and on systems with long filenames it's usually not necessary
 *   because of the negligible chance of a collision.  On systems with short
 *   filenames, a placeholder can be useful to prevent a subsequent call to
 *   this routine, or a separate process, from using the same filename before
 *   the caller has had a chance to use the returned name to create the
 *   actual temp file.
 *   
 *   Returns true on success, false on failure.  This can fail if there's no
 *   system temporary directory defined, or the temp directory is so full of
 *   other files that we can't find an unused filename.  
 */
int os_gen_temp_filename(char *buf, size_t buflen);


/* ------------------------------------------------------------------------ */
/*
*   Basic directory/folder management routines
*/

/*
*   Switch to a new working directory.
*
*   This is meant to behave similarly to the Unix concept of a working
*   directory, in that it sets the base directory assumed for subsequent
*   file operations (e.g., the osfopxx() functions, osfdel(), etc - anything
*   that takes a filename or directory name as an argument).  The working
*   directory applies to filenames specified with relative paths in the
*   local system notation.  File operations on filenames specified with
*   absolute paths, of course, ignore the working directory.
*/
void os_set_pwd(const char *dir);

/*
*   Switch the working directory to the directory containing the given
*   file.  Generally, this routine should only need to parse the filename
*   enough to determine the part that's the directory path, then use
*   os_set_pwd() to switch to that directory.
*/
void os_set_pwd_file(const char *filename);

/*
*   Create a directory.  This creates a new directory/folder with the given
*   name, which may be given as a relative or absolute path.  Returns true
*   on success, false on failure.
*
*   If 'create_parents' is true, and the directory has mulitiple path
*   elements, this routine should create each enclosing parent that doesn't
*   already exist.  For example, if the path is specified as "a/b/c", and
*   there exists a folder "a" in the working directory, but "a" is empty,
*   this should first create "b" and then create "c".  If an error occurs
*   creating any parent, the routine should simply stop and return failure.
*   (Optionally, the routine may attempt to behave atomically by undoing any
*   parent folder creations it accomplished before failing on a nested
*   folder, but this isn't required.  To reduce the chances of a failure
*   midway through the operation, the routine might want to scan the
*   filename before starting to ensure that it contains only valid
*   characters, since an invalid character is the most likely reason for a
*   failure part of the way through.)
*
*   We recommend making the routine flexible in terms of the notation it
*   accepts; e.g., on Unix, "/dir/sub/folder" and "/dir/sub/folder/" should
*   be considered equivalent.
*/
bool os_mkdir(const char *dir, int create_parents);

/*
*   Remove a directory.  Returns true on success, false on failure.
*
*   If the directory isn't already empty, this routine fails.  That is, the
*   routine does NOT recursively delete the contents of a non-empty
*   directory.  It's up to the caller to delete any contents before removing
*   the directory, if that's the caller's intention.  (Note to implementors:
*   most native OS APIs to remove directories fail by default if the
*   directory isn't empty, so it's usually safe to implement this simply by
*   calling the native API.  However, if your system's version of this API
*   can remove a non-empty directory, you MUST add an extra test before
*   removing the directory to ensure it's empty, and return failure if it's
*   not.  For the purposes of this test, "empty" should of course ignore any
*   special objects that are automatically or implicitly present in all
*   directories, such as the Unix "." and ".." relative links.)
*/
bool os_rmdir(const char *dir);


/* ------------------------------------------------------------------------ */
/*
*   Filename manipulation routines
*/

/* apply a default extension to a filename, if it doesn't already have one */
void os_defext(char *fname, const char *ext);

/* unconditionally add an extention to a filename */
void os_addext(char *fname, const char *ext);

/* remove the extension from a filename */
void os_remext(char *fname);

/*
*   Compare two file names/paths for syntactic equivalence.  Returns true if
*   the names are equivalent names according to the local file system's
*   syntax conventions, false if not.  This does a syntax-only comparison of
*   the paths, without looking anything up in the file system.  This means
*   that a false return doesn't guarantee that the paths don't point to the
*   same file.
*
*   This routine DOES make the following equivalences:
*
*   - if the local file system is insensitive to case, the names are
*   compared ignoring case
*
*   - meaningless path separator difference are ignored: on Unix, "a/b" ==
*   "a//b" == "a/b/"; on Windows, "a/b" == "a\\b"
*
*   - relative links that are strictly structural or syntactic are applied;
*   for example, on Unix or Windows, "a/./b" == "a/b" = "a/b/c/..".  This
*   only applies for special relative links that can be resolved without
*   looking anything up in the file system.
*
*   This DOES NOT do the following:
*
*   - it doesn't apply working directories/volums to relative paths
*
*   - it doesn't follow symbolic links in the file system
*/
bool os_file_names_equal(const char *a, const char *b);

/*
*   Get a pointer to the root name portion of a filename.  This is the part
*   of the filename after any path or directory prefix.  For example, on
*   Unix, given the string "/home/mjr/deep.gam", this function should return
*   a pointer to the 'd' in "deep.gam".  If the filename doesn't appear to
*   have a path prefix, it should simply return the argument unchanged.
*
*   IMPORTANT: the returned pointer MUST point into the original 'buf'
*   string, and the contents of that buffer must NOT be modified.  The
*   return value must point into the same buffer because there are no
*   allowances for the alternatives.  In particular, (a) you can't return a
*   pointer to newly allocated memory, because callers won't free it, so
*   doing so would cause a memory leak; and (b) you can't return a pointer
*   to an internal static buffer, because callers might call this function
*   more than once and still rely on a value returned on an older call,
*   which would be invalid if a static buffer could be overwritten on each
*   call.  For these reasons, it's required that the return value point to a
*   position within the original string passed in 'buf'.
*/
const char *os_get_root_name(const char *buf);

/*
*   Determine whether a filename specifies an absolute or relative path.
*   This is used to analyze filenames provided by the user (for example,
*   in a #include directive, or on a command line) to determine if the
*   filename can be considered relative or absolute.  This can be used,
*   for example, to determine whether to search a directory path for a
*   file; if a given filename is absolute, a path search makes no sense.
*   A filename that doesn't specify an absolute path can be combined with
*   a path using os_build_full_path().
*
*   Returns true if the filename specifies an absolute path, false if
*   not.
*/
bool os_is_file_absolute(const char *fname);

/*
*   Extract the path from a filename.  Fills in pathbuf with the path
*   portion of the filename.  If the filename has no path, the pathbuf
*   should be set appropriately for the current directory (on Unix or DOS,
*   for example, it can be set to an empty string).
*
*   The result can end with a path separator character or not, depending on
*   local OS conventions.  Paths extracted with this function can only be
*   used with os_build_full_path(), so the conventions should match that
*   function's.
*
*   Unix examples:
*
*.   /home/mjr/deep.gam -> /home/mjr
*.   games/deep.gam -> games
*.   deep.gam -> [empty string]
*
*   Mac examples:
*
*    :home:mjr:deep.gam -> :home:mjr
*.   Hard Disk:games:deep.gam -> Hard Disk:games
*.   Hard Disk:deep.gam -> Hard Disk:
*.   deep.gam -> [empty string]
*
*   VMS examples:
*
*.   SYS$DISK:[mjr.games]deep.gam -> SYS$DISK:[mjr.games]
*.   SYS$DISK:[mjr.games] -> SYS$DISK:[mjr]
*.   deep.gam -> [empty string]
*
*   Note in the last example that we've retained the trailing colon in the
*   path, whereas we didn't in the others; although the others could also
*   retain the trailing colon, it's required only for the last case.  The
*   last case requires the colon because it would otherwise be impossible to
*   determine whether "Hard Disk" was a local subdirectory or a volume name.
*
*/
void os_get_path_name(char *pathbuf, size_t pathbuflen, const char *fname);

/*
*   Build a full path name, given a path and a filename.  The path may have
*   been specified by the user, or may have been extracted from another file
*   via os_get_path_name().  This routine must take care to add path
*   separators as needed, but also must take care not to add too many path
*   separators.
*
*   This routine should reformat the path into canonical format to the
*   extent possible purely through syntactic analysis.  For example, special
*   relative links, such as Unix "." and "..", should be resolved; for
*   example, combining "a/./b/c" with ".." on Unix should yield "a/b".
*   However, symbolic links that require looking up names in the file system
*   should NOT be resolved.  We don't want to perform any actual file system
*   lookups because might want to construct hypothetical paths that don't
*   necessarily relate to files on the local system.
*
*   Note that relative path names may require special care on some
*   platforms.  In particular, if the source path is relative, the result
*   should also be relative.  For example, on the Macintosh, a path of
*   "games" and a filename "deep.gam" should yield ":games:deep.gam" - note
*   the addition of the leading colon to make the result path relative.
*
*   Note also that the 'filename' argument is not only allowed to be an
*   ordinary file, possibly qualified with a relative path, but is also
*   allowed to be a subdirectory.  The result in this case should be a path
*   that can be used as the 'path' argument to a subsequent call to
*   os_build_full_path; this allows a path to be built in multiple steps by
*   descending into subdirectories one at a time.
*
*   Unix examples:
*
*.   /home/mjr + deep.gam -> /home/mjr/deep.gam"
*.   /home/mjr + .. -> /home
*.   /home/mjr + ../deep.gam -> /home/deep.gam
*.   /home/mjr/ + deep.gam -> /home/mjr/deep.gam"
*.   games + deep.gam -> games/deep.gam"
*.   games/ + deep.gam -> games/deep.gam"
*.   /home/mjr + games/deep.gam -> /home/mjr/games/deep.gam"
*.   games + scifi/deep.gam -> games/scifi/deep.gam"
*.   /home/mjr + games -> /home/mjr/games"
*
*   Mac examples:
*
*.   Hard Disk: + deep.gam -> Hard Disk:deep.gam
*.   :games: + deep.gam -> :games:deep.gam
*.   :games:deep + ::test.gam -> :games:test.gam
*.   games + deep.gam -> :games:deep.gam
*.   Hard Disk: + :games:deep.gam -> Hard Disk:games:deep.gam
*.   games + :scifi:deep.gam -> :games:scifi:deep.gam
*.   Hard Disk: + games -> Hard Disk:games
*.   Hard Disk:games + scifi -> Hard Disk:games:scifi
*.   Hard Disk:games:scifi + deep.gam -> Hard Disk:games:scifi:deep.gam
*.   Hard Disk:games + :scifi:deep.gam -> Hard Disk:games:scifi:deep.gam
*
*   VMS examples:
*
*.   [home.mjr] + deep.gam -> [home.mjr]deep.gam
*.   [home.mjr] + [-]deep.gam -> [home]deep.gam
*.   mjr.dir + deep.gam -> [.mjr]deep.gam
*.   [home]mjr.dir + deep.gam -> [home.mjr]deep.gam
*.   [home] + [.mjr]deep.gam -> [home.mjr]deep.gam
*/
void os_build_full_path(char *fullpathbuf, size_t fullpathbuflen,
	const char *path, const char *filename);

/*
*   Combine a path and a filename to form a full path to the file.  This is
*   *almost* the same as os_build_full_path(), but if the 'filename' element
*   is a special relative link, such as Unix '.' or '..', this preserves
*   that special link in the final name.
*
*   Unix examples:
*
*.    /home/mjr + deep.gam -> /home/mjr/deep.gam
*.    /home/mjr + . -> /home/mjr/.
*.    /home/mjr + .. -> /home/mjr/..
*
*   Mac examples:
*
*.    Hard Disk:games + deep.gam -> HardDisk:games:deep.gam
*.    Hard Disk:games + :: -> HardDisk:games::
*
*   VMS exmaples:
*
*.    [home.mjr] + deep.gam -> [home.mjr]deep.gam
*.    [home.mjr] + [-] -> [home.mjr.-]
*/
void os_combine_paths(char *fullpathbuf, size_t pathbuflen,
	const char *path, const char *filename);


/*
*   Get the absolute, fully qualified filename for a file.  This fills in
*   'result_buf' with the absolute path to the given file, taking into
*   account the current working directory and any other implied environment
*   information that affects the way the file system would resolve the given
*   file name to a specific file on disk if we opened the file now using
*   this name.
*
*   The returned path should be in absolute path form, meaning that it's
*   independent of the current working directory or any other environment
*   settings.  That is, this path should still refer to the same file even
*   if the working directory changes.
*
*   Note that it's valid to get the absolute path for a file that doesn't
*   exist, or for a path with directory components that don't exist.  For
*   example, a caller might generate the absolute path for a file that it's
*   about to create, or a hypothetical filename for path comparison
*   purposes.  The function should succeed even if the file or any path
*   components don't exist.  If the file is in relative format, and any path
*   elements don't exist but are syntactically well-formed, the result
*   should be the path obtained from syntactically combining the working
*   directory with the relative path.
*
*   On many systems, a given file might be reachable through more than one
*   absolute path.  For example, on Unix it might be possible to reach a
*   file through symbolic links to the file itself or to parent directories,
*   or hard links to the file.  It's up to the implementation to determine
*   which path to use in such cases.
*
*   On success, returns true.  If it's not possible to resolve the file name
*   to an absolute path, the routine copies the original filename to the
*   result buffer exactly as given, and returns false.
*/
bool os_get_abs_filename(char *result_buf, size_t result_buf_size,
	const char *filename);

/*
*   Get the relative version of the given filename path 'filename', relative
*   to the given base directory 'basepath'.  Both paths must be given in
*   absolute format.
*
*   Returns true on success, false if it's not possible to rewrite the path
*   in relative terms.  For example, on Windows, it's not possible to
*   express a path on the "D:" drive as relative to a base path on the "C:"
*   drive, since each drive letter has an independent root folder; there's
*   no namespace entity enclosing a drive letter's root folder.  On
*   Unix-like systems where the entire namespace has a single hierarchical
*   root, it should always be possible to express any path relative to any
*   other.
*
*   The result should be a relative path that can be combined with
*   'basepath' using os_build_full_path() to reconstruct a path that
*   identifies the same file as the original 'filename' (it's not important
*   that this procedure would result in the identical string - it just has
*   to point to the same file).  If it's not possible to express the
*   filename relative to the base path, fill in 'result_buf' with the
*   original filename and return false.
*
*   Windows examples:
*
*.    c:\mjr\games | c:\mjr\games\deep.gam  -> deep.gam
*.    c:\mjr\games | c:\mjr\games\tads\deep.gam  -> tads\deep.gam
*.    c:\mjr\games | c:\mjr\tads\deep.gam  -> ..\tads\deep.gam
*.    c:\mjr\games | d:\deep.gam  ->  d:\deep.gam (and return false)
*
*   Mac OS examples:
*
*.    Mac HD:mjr:games | Mac HD:mjr:games:deep.gam -> deep.gam
*.    Mac HD:mjr:games | Mac HD:mjr:games:tads:deep.gam -> :tads:deep.gam
*.    Mac HD:mjr:games | Ext Disk:deep.gam -> Ext Disk:deep.gam (return false)
*
*   VMS examples:
*
*.    SYS$:[mjr.games] | SYS$:[mjr.games]deep.gam -> deep.gam
*.    SYS$:[mjr.games] | SYS$:[mjr.games.tads]deep.gam -> [.tads]deep.gam
*.    SYS$:[mjr.games] | SYS$:[mjr.tads]deep.gam -> [-.tads]deep.gam
*.    SYS$:[mjr.games] | DISK$:[mjr]deep.gam -> DISK$[mjr]deep.gam (ret false)
*/
bool os_get_rel_path(char *result_buf, size_t result_buf_size,
	const char *basepath, const char *filename);

/*
*   Determine if the given file is in the given directory.  Returns true if
*   so, false if not.  'filename' is a relative or absolute file name;
*   'path' is a relative or absolute directory path, such as one returned
*   from os_get_path_name().
*
*   If 'include_subdirs' is true, the function returns true if the file is
*   either directly in the directory 'path', OR it's in any subdirectory of
*   'path'.  If 'include_subdirs' is false, the function returns true only
*   if the file is directly in the given directory.
*
*   If 'match_self' is true, the function returns true if 'filename' and
*   'path' are the same directory; otherwise it returns false in this case.
*
*   This routine is allowed to return "false negatives" - that is, it can
*   claim that the file isn't in the given directory even when it actually
*   is.  The reason is that it's not always possible to determine for sure
*   that there's not some way for a given file path to end up in the given
*   directory.  In contrast, a positive return must be reliable.
*
*   If possible, this routine should fully resolve the names through the
*   file system to determine the path relationship, rather than merely
*   analyzing the text superficially.  This can be important because many
*   systems have multiple ways to reach a given file, such as via symbolic
*   links on Unix; analyzing the syntax alone wouldn't reveal these multiple
*   pathways.
*
*   SECURITY NOTE: If possible, implementations should fully resolve all
*   symbolic links, relative paths (e.g., Unix ".."), etc, before rendering
*   judgment.  One important application for this routine is to determine if
*   a file is in a sandbox directory, to enforce security restrictions that
*   prevent a program from accessing files outside of a designated folder.
*   If the implementation fails to resolve symbolic links or relative paths,
*   a malicious program or user could bypass the security restriction by,
*   for example, creating a symbolic link within the sandbox directory that
*   points to the root folder.  Implementations can avoid this loophole by
*   converting the file and directory names to absolute paths and resolving
*   all symbolic links and relative notation before comparing the paths.
*/
bool os_is_file_in_dir(const char *filename, const char *path,
	bool include_subdirs, bool match_self);



/* ------------------------------------------------------------------------ */
/*
 *   Convert an OS filename path to URL-style format.  This isn't a true URL
 *   conversion; rather, it simply expresses a filename in Unix-style
 *   notation, as a series of path elements separated by '/' characters.
 *   Unlike true URLs, we don't use % encoding or a scheme prefix (file://,
 *   etc).
 *   
 *   The result path never ends in a trailing '/', unless the entire result
 *   path is "/".  This is for consistency; even if the source path ends with
 *   a local path separator, the result doesn't.
 *   
 *   If the local file system syntax uses '/' characters as ordinary filename
 *   characters, these must be replaced with some other suitable character in
 *   the result, since otherwise they'd be taken as path separators when the
 *   URL is parsed.  If possible, the substitution should be reversible with
 *   respect to os_cvt_dir_url(), so that the same URL read back in on this
 *   same platform will produce the same original filename.  One particular
 *   suggestion is that if the local system uses '/' to delimit what would be
 *   a filename extension on other platforms, replace '/' with '.', since
 *   this will provide reversibility as well as a good mapping if the URL is
 *   read back in on another platform.
 *   
 *   The local equivalents of "." and "..", if they exist, are converted to
 *   "." and ".." in the URL notation.
 *   
 *   Examples:
 *   
 *.   Windows: images\rooms\startroom.jpg -> images/rooms/startroom.jpg
 *.   Windows: ..\startroom.jpg -> ../startroom.jpg
 *.   Mac:     :images:rooms:startroom.jpg -> images/rooms/startroom.jpg
 *.   Mac:     ::startroom.jpg -> ../startroom.jpg
 *.   VMS:     [.images.rooms]startroom.jpg -> images/rooms/startroom.jpg
 *.   VMS:     [-.images]startroom.jpg -> ../images/startroom.jpg
 *.   Unix:    images/rooms/startroom.jpg -> images/rooms/startroom.jpg
 *.   Unix:    ../images/startroom.jpg -> ../images/startroom.jpg
 *   
 *   If the local name is an absolute path in the local file system (e.g.,
 *   Unix /file, Windows C:\file), translate as follows.  If the local
 *   operating system uses a volume or device designator (Windows C:, VMS
 *   SYS$DISK:, etc), make the first element of the path the exact local
 *   syntax for the device designator: /C:/ on Windows, /SYS$DISK:/ on VMS,
 *   etc.  Include the local syntax for the device prefix.  For a system like
 *   Unix with a unified file system root ("/"), simply start with the root
 *   directory.  Examples:
 *   
 *.    Windows:  C:\games\deep.gam         -> /C:/games/deep.gam
 *.    Windows:  C:games\deep.gam          -> /C:./games/deep.gam
 *.    Windows:  \\SERVER\DISK\games\deep.gam -> /\\SERVER/DISK/games/deep.gam
 *.    Mac OS 9: Hard Disk:games:deep.gam  -> /Hard Disk:/games/deep.gam
 *.    VMS:      SYS$DISK:[games]deep.gam  -> /SYS$DISK:/games/deep.gam
 *.    Unix:     /games/deep.gam           -> /games/deep.gam
 *   
 *   Rationale: it's effectively impossible to create a truly portable
 *   representation of an absolute path.  Operating systems are too different
 *   in the way they represent root paths, and even if that were solvable, a
 *   root path is essentially unusable across machines anyway because it
 *   creates a dependency on the contents of a particular machine's disk.  So
 *   if we're called upon to translate an absolute path, we can forget about
 *   trying to be truly portable and instead focus on round-trip fidelity -
 *   i.e., making sure that applying os_cvt_url_dir() to our result recovers
 *   the exact original path, assuming it's done on the same operating
 *   system.  The approach outlined above should achieve round-trip fidelity
 *   when a local path is converted to a URL and back on the same machine,
 *   since the local URL-to-path converter should recognize its own special
 *   type of local absolute path prefix.  It also produces reasonable results
 *   on other platforms - see the os_cvt_url_dir() comments below for
 *   examples of the decoding results for absolute paths moved to new
 *   platforms.  The result when a device-rooted absolute path is encoded on
 *   one machine and then decoded on another will generally be a local path
 *   with a root on the default device/volume and an outermost directory with
 *   a name based on the original machine's device/volume name.  This
 *   obviously won't reproduce the exact original path, but since that's
 *   impossible anyway, this is probably as good an approximation as we can
 *   create.
 *   
 *   Character sets: the input could be in local or UTF-8 character sets.
 *   The implementation shouldn't care, though - just treat bytes in the
 *   range 0-127 as plain ASCII, and everything else as opaque.  I.e., do not
 *   quote or otherwise modify characters outside the 0-127 range.  
 */
void os_cvt_dir_url(char *result_buf, size_t result_buf_size,
                    const char *src_path);

/*
 *   Convert a URL-style path into a filename path expressed in the local
 *   file system's syntax.  Fills in result_buf with a file path, constructed
 *   using the local file system syntax, that corresponds to the path in
 *   src_url expressed in URL-style syntax.  Examples:
 *   
 *   images/rooms/startroom.jpg -> 
 *.   Windows   -> images\rooms\startroom.jpg
 *.   Mac OS 9  -> :images:rooms:startroom.jpg
 *.   VMS       -> [.images.rooms]startroom.jpg
 *   
 *   The source format isn't a true URL; it's simply a series of path
 *   elements separated by '/' characters.  Unlike true URLs, our input
 *   format doesn't use % encoding and doesn't have a scheme (file://, etc).
 *   (Any % in the source is treated as an ordinary character and left as-is,
 *   even if it looks like a %XX sequence.  Anything that looks like a scheme
 *   prefix is left as-is, with any // treated as path separators.
 *   
 *   images/file%20name.jpg ->
 *.   Windows   -> images\file%20name.jpg
 *   
 *   file://images/file.jpg ->
 *.   Windows   -> file_\\images\file.jpg
 *   
 *   Any characters in the path that are invalid in the local file system
 *   naming rules are converted to "_", unless "_" is itself invalid, in
 *   which case they're converted to "X".  One exception is that if '/' is a
 *   valid local filename character (rather than a path separator as it is on
 *   Unix and Windows), it can be used as the replacement for the character
 *   that os_cvt_dir_url uses as its replacement for '/', so that this
 *   substitution is reversible when a URL is generated and then read back in
 *   on this same platform.
 *   
 *   images/file:name.jpg ->
 *.   Windows   -> images\file_name.jpg
 *.   Mac OS 9  -> :images:file_name.jpg
 *.   Unix      -> images/file:name.jpg
 *   
 *   The path elements "." and ".." are specifically defined as having their
 *   Unix meanings: "." is an alias for the preceding path element, or the
 *   working directory if it's the first element, and ".." is an alias for
 *   the parent of the preceding element.  When these appear as path
 *   elements, this routine translates them to the appropriate local
 *   conventions.  "." may be translated simply by removing it from the path,
 *   since it reiterates the previous path element.  ".." may be translated
 *   by removing the previous element - HOWEVER, if ".." appears as the first
 *   element, it has to be retained and translated to the equivalent local
 *   notation, since it will have to be applied later, when the result_buf
 *   path is actually used to open a file, at which point it will combined
 *   with the working directory or another base path.
 *   
 *.  /images/../file.jpg -> [Windows] file.jpg
 *.  ../images/file.jpg ->
 *.   Windows  -> ..\images\file.jpg
 *.   Mac OS 9 -> ::images:file.jpg
 *.   VMS      -> [-.images]file.jpg
 *   
 *   If the URL path is absolute (starts with a '/'), the routine inspects
 *   the path to see if it was created by the same OS, according to the local
 *   rules for converting absolute paths in os_cvt_dir_url() (see).  If so,
 *   we reverse the encoding done there.  If it doesn't appear that the name
 *   was created by the same operating system - that is, if reversing the
 *   encoding doesn't produce a valid local filename - then we create a local
 *   absolute path as follows.  If the local system uses device/volume
 *   designators, we start with the current working device/volume or some
 *   other suitable default volume.  We then add the first element of the
 *   path, if any, as the root directory name, applying the usual "_" or "X"
 *   substitution for any characters that aren't allowed in local names.  The
 *   rest of the path is handled in the usual fashion.
 *   
 *.  /images/file.jpg ->
 *.    Windows -> \images\file.jpg
 *.    Unix    -> /images/file.jpg
 *   
 *.  /c:/images/file.jpg ->
 *.    Windows -> c:\images\file.jpg
 *.    Unix    -> /c:/images/file.jpg
 *.    VMS     -> SYS$DISK:[c__.images]file.jpg
 *   
 *.  /Hard Disk:/images/file.jpg ->
 *.    Windows -> \Hard Disk_\images\file.jpg
 *.    Unix    -> SYS$DISK:[Hard_Disk_.images]file.jpg
 *   
 *   Note how the device/volume prefix becomes the top-level directory when
 *   moving a path across machines.  It's simply not possible to reconstruct
 *   the exact original path in such cases, since device/volume syntax rules
 *   have little in common across systems.  But this seems like a good
 *   approximation in that (a) it produces a valid local path, and (b) it
 *   gives the user a reasonable basis for creating a set of folders to mimic
 *   the original source system, if they want to use that approach to port
 *   the data rather than just changing the paths internally in the source
 *   material.
 *   
 *   Character sets: use the same rules as for os_cvt_dir_url().  
 */
void os_cvt_url_dir(char *result_buf, size_t result_buf_size,
                    const char *src_url);


} // End of namespace TADS
} // End of namespace Glk

#endif
