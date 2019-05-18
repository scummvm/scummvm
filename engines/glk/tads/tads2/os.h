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

/* Portable interfaces to OS-specific functions
 *
 * This file defines interfaces to certain functions that must be called
 * from portable code, but which must have system-specific implementations.
 */

#ifndef GLK_TADS_TADS2_OS
#define GLK_TADS_TADS2_OS

#include "common/system.h"
#include "glk/tads/os_frob_tads.h"
#include "glk/tads/os_glk.h"
#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/appctx.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* ------------------------------------------------------------------------ */
/*
 *   A note on character sets:
 *   
 *   Except where noted, all character strings passed to and from the
 *   osxxx functions defined herein use the local operating system
 *   representation.  On a Windows machine localized to Eastern Europe,
 *   for example, the character strings passed to and from the osxxx
 *   functions would use single-byte characters in the Windows code page
 *   1250 representation.
 *   
 *   Callers that use multiple character sets must implement mappings to
 *   and from the local character set when calling the osxxx functions.
 *   The osxxx implementations are thus free to ignore any issues related
 *   to character set conversion or mapping.
 *   
 *   The osxxx implementations are specifically not permitted to use
 *   double-byte Unicode as the native character set, nor any other
 *   character set where a null byte could appear as part of a non-null
 *   character.  In particular, callers may assume that null-terminated
 *   strings passed to and from the osxxx functions contain no embedded
 *   null bytes.  Multi-byte character sets (i.e., character sets with
 *   mixed single-byte and double-byte characters) may be used as long as
 *   a null byte is never part of any multi-byte character, since this
 *   would guarantee that a null byte could always be taken as a null
 *   character without knowledge of the encoding or context.  
 */

/* ------------------------------------------------------------------------ */
/*
 *   "Far" Pointers.  Most platforms can ignore this.  For platforms with
 *   mixed-mode addressing models, where pointers of different sizes can
 *   be used within a single program and hence some pointers require
 *   qualification to indicate that they use a non-default addressing
 *   model, the keyword OSFAR should be defined to the appropriate
 *   compiler-specific extension keyword.
 *   
 *   If you don't know what I'm talking about here, you should just ignore
 *   it, because your platform probably doesn't have anything this
 *   sinister.  As of this writing, this applies only to MS-DOS, and then
 *   only to 16-bit implementations that must interact with other 16-bit
 *   programs via dynamic linking or other mechanisms.  
 */


/* ------------------------------------------------------------------------ */
/*
 *   ANSI C99 exact-size integer types.
 *   
 *   C99 defines a set of integer types with exact bit sizes, named intXX_t
 *   for a signed integer with XX bits, and uintXX_t for unsigned.  XX can be
 *   8, 16, 32, or 64.  TADS uses the 16- and 32-bit sizes, so each platform
 *   is responsible for defining the following types:
 *   
 *.    int16_t   - a signed integer type storing EXACTLY 16 bits
 *.    uint16_t  - an unsigned integer type storing EXACTLY 16 bits
 *.    int32_t   - a signed integer type storing EXACTLY 32 bits
 *.    uint32_t  - an unsigned integer type storing EXACTLY 32 bits
 *   
 *   Many modern compilers provide definitions for these types via the
 *   standard header stdint.h.  Where stdint.h is provided, the platform code
 *   can merely #include <stdint.h>.
 *   
 *   For compilers where stdint.h isn't available, you must provide suitable
 *   typedefs.  Note that the types must be defined with the exact bit sizes
 *   specified; it's not sufficient to use a bigger type, because we depend
 *   in some cases on overflow and sign extension behavior at the specific
 *   bit size.
 */


/* ------------------------------------------------------------------------ */
/*
 *   Thread-local storage (TLS).
 *   
 *   When TADS is compiled with threading support, it requires some variables
 *   to be "thread-local".  This means that the variables have global scope
 *   (so they're not stored in "auto" variables on the stack), but each
 *   thread has a private copy of each such variable.
 *   
 *   Nearly all systems that support threads also support thread-local
 *   storage.  Like threading support itself, though, TLS support is at
 *   present implemented only in non-portable OS APIs rather than standard C
 *   language features.  TLS is a requirement if TADS is compiled with
 *   threading, but it's not needed for non-threaded builds.  TADS only
 *   requires threading at present (version 3.1) for its network features;
 *   since these features are optional, systems that don't have threading and
 *   TLS support will simply need to disable the network features, which will
 *   allow all of the threading and TLS definitions in osifc to be omitted.
 *   
 *   There appear to be two common styles of TLS programming models.  The
 *   first provides non-standard compiler syntax for declarative creation of
 *   thread-local variables.  The Microsoft (on Windows) and Gnu compilers
 *   (on Linux and Unix) do this: they provide custom storage class modifiers
 *   for declaring thread locals (__declspec(thread) for MSVC, __thread for
 *   gcc).  Compilers that support declarative thread locals handle the
 *   implementation details through code generation, so the program merely
 *   needs to add the special TLS storage class qualifier to an otherwise
 *   ordinary global variable declaration, and then can access the thread
 *   local as though it were an ordinary global.
 *   
 *   The second programming model is via explicit OS API calls to create,
 *   initialize, and access thread locals.  pthreads provides such an API, as
 *   does Win32.  In fact, when you use the declarative syntax with MSVC or
 *   gcc, the compiler generates the appropriate API calls, but the details
 *   are transparent to the program; in contrast, when using pthreads
 *   directly, the program must actively call the relevant APIs.
 *   
 *   It's probably the case that every system that has compiler-level support
 *   for declarative thread local creation also has procedural APIs, so the
 *   simplest way to abstract the platform differences would be to do
 *   everything in terms of APIs.  However, it seems likely that compilers
 *   with declarative syntax might be able to generate more efficient code,
 *   since optimizers always benefit from declarative information.  So we'd
 *   like to use declarative syntax whenever it's available, but fall back on
 *   explicit API calls when it's not.  So our programming model is a union
 *   of the two styles:
 *   
 *   1. For each thread local, declare the thread local:
 *.      OS_DECL_TLS(char *, my_local);
 *   
 *   2. At main program startup (for the main thread only), initialize each
 *   thread local:
 *.      os_tls_create(my_local);
 *   
 *   3. Never get or set the value of a thread local directly; instead, use
 *   the get/set functions:
 *.      char *x = os_tls_get(char *, my_local);
 *.      os_tls_set(my_local, "hello");
 *   
 *   One key feature of this implementation is that each thread local is
 *   stored as a (void *) value.  We do it this way to allow a simple direct
 *   mapping to the pthreads APIs, since that's going to be the most common
 *   non-declarative implementation.  This means that a thread local variable
 *   can contain any pointer type, but *only* a pointer type.  The standard
 *   pattern for dealing with anything more ocmplex is the same as in
 *   pthreads: gather up the data into a structure, malloc() an instance of
 *   that structure at entry to each thread (including the main thread), and
 *   os_tls_set() the variable to contain a pointer to that structure.  From
 *   then on, use os_tls_set(my_struct *, my_local)->member to access the
 *   member variables in the structure.  And finally, each thread must delete
 *   the structure at thread exit.
 */

/*   
 *   
 *   Declare a thread local.
 *   
 *   - For compilers that support declarative TLS variables, the local OS
 *   headers should use the compiler support by #defining OS_DECL_TLS to the
 *   appropriate local declarative keyword.
 *   
 *   - For systems without declarative TLS support but with TLS APIs, the
 *   global declared by this macro actually stores the slot ID (what pthreads
 *   calls the "key") for the variable.  This macro should therefore expand
 *   to a declaration of the appropriate API type for a slot ID; for example,
 *   on pthreads, #define OS_DECL_TLS(t, v) pthread_key_t v.
 *   
 *   - For builds with no thread support, simply #define this to declare the
 *   variable as an ordinary global: #define OS_DECL_TLS(t, v) t v.
 */
/* #define OS_DECL_TLS(typ, varname)  __thread typ varname */

/*
 *   For API-based systems without declarative support in the compiler, the
 *   main program startup code must explicitly create a slot for each thread-
 *   local variable by calling os_tls_create().  The API returns a slot ID,
 *   which is shared among threads and therefore can be stored in an ordinary
 *   global variable.  OS_DECL_TLS will have declared the global variable
 *   name in this case as an ordinary global of the slot ID type.  The
 *   os_tls_create() macro should therefore expand to a call to the slot
 *   creation API, storing the new slot ID in the global.
 *   
 *   Correspondingly, before the main thread exits, it should delete each
 *   slot it created, b calling os_tls_delete().
 *   
 *   For declarative systems, there's no action required here, so these
 *   macros can be defined to empty.
 */
/* #define os_tls_create(varname)  pthread_key_create(&varname, NULL) */
/* #define os_tls_delete(varname)  pthread_key_delete(varname) */


/*
 *   On API-based systems, each access to get or set the thread local
 *   requires an API call, using the slot ID stored in the actual global to
 *   get the per-thread instance of the variable's storage.
 *.    #define os_tls_get(typ, varname) ((typ)pthread_getspecific(varname))
 *.    #define os_tls_set(varname, val) pthread_setspecific(varname, val)
 *   
 *   On declarative systems, the global variable itself is the thread local,
 *   so get/set can be implemented as direct access to the variable.
 *.    #define os_tls_get(typ, varname) varname
 *.    #define os_tls_set(varname, val) varname = (val)
 */

/*
 *   Common TLS definitions - declarative thread locals
 *   
 *   For systems with declarative TLS support in the compiler, the OS header
 *   can #define OS_DECLARATIVE_TLS to pick up suitable definitions for the
 *   os_tls_xxx() macros.  The OS header must separately define OS_DECL_TLS
 *   as appropriate for the local system.
 */
#ifdef OS_DECLARATIVE_TLS
#define os_tls_create(varname)
#define os_tls_delete(varname)
#define os_tls_get(typ, varname) varname
#define os_tls_set(varname, val) varname = (val)
#endif

/*
 *   Common TLS definitions - pthreads
 *   
 *   For pthreads systems without declarative TLS support in the compiler,
 *   the OS header can simply #define OS_PTHREAD_TLS to pick up the standard
 *   definitions below. 
 */
#ifdef OS_PTHREAD_TLS
#include <pthread.h>
#define OS_DECL_TLS(typ, varname) pthread_key_t varname
#define os_tls_create(varname) pthread_key_create(&varname, NULL)
#define os_tls_delete(varname) pthread_key_delete(varname)
#define os_tls_get(typ, varname) ((typ)pthread_getspecific(varname))
#define os_tls_set(varname, val) pthread_setspecific(varname, val)
#endif

/* ------------------------------------------------------------------------ */
/* 
 *   OS main entrypoint 
 */
int os0main(int oargc, char **oargv,
            int (*mainfn)(int, char **, char *), 
            const char *before, const char *config);

/* 
 *   new-style OS main entrypoint - takes an application container context 
 */
int os0main2(int oargc, char **oargv,
             int (*mainfn)(int, char **, appctxdef *, char *),
             const char *before, const char *config,
             appctxdef *appctx);


/* open the error message file for reading */
osfildef *oserrop(const char *arg0);

/* ------------------------------------------------------------------------ */
/*
 *   Special "switch" statement optimization flags.  These definitions are
 *   OPTIONAL - if a platform doesn't provide these definitions, suitable
 *   code that's fully portable will be used.
 *   
 *   On some compilers, the performance of a "switch" statement can be
 *   improved by fully populating the switch with all possible "case"
 *   values.  When the compiler can safely assume that every possible "case"
 *   value is specifically called out in the switch, the compiler can
 *   generate somewhat faster code by omitting any range check for the
 *   controlling expression of the switch: a range check is unnecessary
 *   because the compiler knows that the value can never be outside the
 *   "case" table.
 *   
 *   This type of optimization doesn't apply to all compilers.  When a given
 *   platform's compiler can be coerced to produce faster "switch"
 *   statements, though, there might be some benefit in defining these
 *   symbols according to local platform rules.
 *   
 *   First, #define OS_FILL_OUT_CASE_TABLES if you want this type of switch
 *   optimization at all.  This symbol is merely a flag, so it doesn't need
 *   a value - #defining it is enough to activate the special code.  If you
 *   don't define this symbol, then the code that cares about this will
 *   simply generate ordinary switches, leaving holes in the case table and
 *   using "default:" to cover them.  If the platform's osxxx.h header does
 *   #define OS_FILL_OUT_CASE_TABLES, then the portable code will know to
 *   fill out case tables with all possible alternatives where it's possible
 *   and potentially beneficial to do so.
 *   
 *   Second, if you #define OS_FILL_OUT_CASE_TABLES, you MUST ALSO #define
 *   OS_IMPOSSIBLE_DEFAULT_CASE.  The value for this symbol must be some
 *   code to insert into a "switch" statement at the point where a
 *   "default:" case would normally go.  On some compilers, special
 *   extensions allow the program to explicitly indicate within a switch
 *   that all possible cases are covered, so that the compiler doesn't have
 *   to be relied upon to infer this for itself (which would be impossible
 *   for it to do in many cases anyway).  You can use an empty definition
 *   for this symbol if your compiler doesn't have any special construct for
 *   indicating a fully-populated case table.
 *   
 *   Note that *most* switch statements in portable code won't care one way
 *   or the other about these definitions.  There's a time/space trade-off
 *   in fully populating a switch's case table, so only the most
 *   time-critical code will bother trying.  
 */



/* ------------------------------------------------------------------------ */
/*
 *   TADS 2 swapping configuration.  Define OS_DEFAULT_SWAP_ENABLED to 0
 *   if you want swapping turned off, 1 if you want it turned on.  If we
 *   haven't defined a default swapping mode yet, turn swapping on by
 *   default.  
 */
#ifndef OS_DEFAULT_SWAP_ENABLED
# define OS_DEFAULT_SWAP_ENABLED   1
#endif

/*
 *   If the system "long description" (for the banner) isn't defined, make
 *   it the same as the platform ID string.  
 */
#ifndef OS_SYSTEM_LDESC
# define OS_SYSTEM_LDESC  OS_SYSTEM_NAME
#endif

/*
 *   TADS 2 Usage Lines
 *   
 *   If the "usage" lines (i.e., the descriptive lines of text describing
 *   how to run the various programs) haven't been otherwise defined,
 *   define defaults for them here.  Some platforms use names other than
 *   tc, tr, and tdb for the tools (for example, on Unix they're usually
 *   tadsc, tadsr, and tadsdb), so the usage lines should be adjusted
 *   accordingly; simply define them earlier than this point (in this file
 *   or in one of the files included by this file, such as osunixt.h) for
 *   non-default text.  
 */
#ifndef OS_TC_USAGE
# define OS_TC_USAGE  "usage: tc [options] file"
#endif
#ifndef OS_TR_USAGE
# define OS_TR_USAGE  "usage: tr [options] file"
#endif
#ifndef OS_TDB_USAGE
# define OS_TDB_USAGE  "usage: tdb [options] file"
#endif

/*
 *   Ports can define a special TDB startup message, which is displayed
 *   after the copyright/version banner.  If it's not defined at this
 *   point, define it to an empty string.  
 */
#ifndef OS_TDB_STARTUP_MSG
# define OS_TDB_STARTUP_MSG ""
#endif

/*
 *   If a system patch sub-level isn't defined, define it here as zero.
 *   The patch sub-level is used on some systems where a number of ports
 *   are derived from a base port (for example, a large number of ports
 *   are based on the generic Unix port).  For platforms like the Mac,
 *   where the porting work applies only to that one platform, this
 *   sub-level isn't meaningful.
 */
#ifndef OS_SYSTEM_PATCHSUBLVL
# define OS_SYSTEM_PATCHSUBLVL  "0"
#endif

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
