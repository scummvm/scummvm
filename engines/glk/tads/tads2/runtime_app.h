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

#ifndef GLK_TADS_TADS2_RUNTIME_APP
#define GLK_TADS_TADS2_RUNTIME_APP

#include "glk/tads/tads2/appctx.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* forward-declare structure types */
struct runcxdef;

/*
 *   Run-time version number 
 */
#define  TADS_RUNTIME_VERSION   "2.5.17"

extern char G_tads_oem_app_name[];
extern char G_tads_oem_display_mode[];
extern char G_tads_oem_dbg_name[];
extern char G_tads_oem_author[];
extern int G_tads_oem_copyright_prefix;


/*
 *   Main run-time subsystem entrypoint.  Runs the game specified in the
 *   argument vector; does not return until the game terminates.  The
 *   application container context is optional; pass null if no context is
 *   required.  
 */
int trdmain(int argc, char **argv, appctxdef *appctx, const char *save_ext);

/*
 *   Main debugger subsystem entrypoint.  Works like trdmain(), but starts
 *   the game under the debugger.  
 */
int tddmain(int argc, char **argv, appctxdef *appctx, const char *save_ext);

/*
 *   close and delete the swap file 
 */
void trd_close_swapfile(struct runcxdef *runctx);

/*
 *   Define default memory sizes if no one else has.
 */
#ifndef TRD_HEAPSIZ
# define TRD_HEAPSIZ  4096
#endif
#ifndef TRD_STKSIZ
# define TRD_STKSIZ   200
#endif
#ifndef TRD_UNDOSIZ
# define TRD_UNDOSIZ  (16 * 1024)
#endif


#ifndef TDD_HEAPSIZ
# define TDD_HEAPSIZ  4096
#endif
#ifndef TDD_STKSIZ
# define TDD_STKSIZ   200
#endif
#ifndef TDD_UNDOSIZ
# define TDD_UNDOSIZ  (16 * 1024)
#endif
#ifndef TDD_POOLSIZ
# define TDD_POOLSIZ  (2 * 1024)
#endif
#ifndef TDD_LCLSIZ
# define TDD_LCLSIZ   0
#endif

/*
 *   If the OS headers haven't defined any system-specific option usage
 *   messages, set up a dummy list.  The usage display routine will show
 *   messages starting from the lower number up to and including the higher
 *   number; by default we'll make the ending number lower than the starting
 *   number so that we don't display any messages at all.  
 */
#ifndef ERR_TRUS_OS_FIRST
# define ERR_TRUS_OS_FIRST    100
# define ERR_TRUS_OS_LAST      99
#endif

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
