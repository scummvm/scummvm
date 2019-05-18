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

/*
 * file i/o interface
 */

#ifndef GLK_TADS_TADS2_FILE_IO
#define GLK_TADS_TADS2_FILE_IO

#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/memory_cache_loader.h"
#include "glk/tads/tads2/object.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* forward declarations */
struct voccxdef;
struct tokpdef;
struct tokthdef;
struct tokcxdef;

/* load-on-demand context (passed in by mcm in load callback) */
struct fiolcxdef {
    osfildef *fiolcxfp;                        /* file pointer of load file */
    errcxdef *fiolcxerr;                          /* error handling context */
    ulong     fiolcxst;                          /* starting offset in file */
    uint      fiolcxflg;                   /* flags from original load file */
    uint      fiolcxseed;                                    /* fioxor seed */
    uint      fiolcxinc;                                /* fioxor increment */
};

/* write game to binary file */
void fiowrt(struct mcmcxdef *mctx, voccxdef *vctx,
            struct tokcxdef *tokctx, struct tokthdef *tab,
            uchar *fmts, uint fmtl, char *fname, uint flags, objnum preinit,
            int extc, uint prpcnt, char *filever);

/* flag values for use with fiowrt */
#define FIOFSYM   0x01               /* include symbol table in output file */
#define FIOFLIN   0x02          /* include source file tracking information */
#define FIOFPRE   0x04        /* preinit needs to be run after reading game */
#define FIOFCRYPT 0x08           /* "encrypt" objects prior to writing them */
#define FIOFBIN   0x10                        /* writing precompiled header */
#define FIOFFAST  0x20                     /* fast-load records are in file */
#define FIOFCASE  0x40    /* case folding was turned on in original compile */
#define FIOFLIN2  0x80                            /* new-style line records */

/* read game from binary file; sets up loader callback context */
void fiord(mcmcxdef *mctx, voccxdef *vctx, tokcxdef *tctx, const char *fname,
	const char *exename, fiolcxdef *setupctx, objnum *preinit, uint *flagp,
	tokpdef *path, uchar **fmtsp, uint *fmtlp, uint *pcntptr, int flags,
	appctxdef *appctx, char *argv0);

/* shut down load-on-demand subsystem, close load file */
void fiorcls(fiolcxdef *ctx);

/* loader callback - load an object on demand */
void OS_LOADDS fioldobj(void *ctx, mclhd handle, uchar *ptr, ushort siz);

/* 
 *   Save a game - returns TRUE on failure.  We'll save the file to
 *   'fname'.  'game_fname' is the name of the game file; if this is not
 *   null, we'll save it to the saved game file so that the player can
 *   later start the game by specifying only the saved game file to the
 *   run-time.  'game_fname' can be null, in which case we'll omit the
 *   game file information.  
 */
int fiosav(voccxdef *vctx, char *fname, char *game_fname);

/*
 *   fiorso() result codes 
 */
#define FIORSO_SUCCESS          0                                /* success */
#define FIORSO_FILE_NOT_FOUND   1                         /* file not found */
#define FIORSO_NOT_SAVE_FILE    2                  /* not a saved game file */
#define FIORSO_BAD_FMT_VSN      3       /* incompatible file format version */
#define FIORSO_BAD_GAME_VSN     4  /* file saved by another game or version */
#define FIORSO_READ_ERROR       5            /* error reading from the file */
#define FIORSO_NO_PARAM_FILE    6   /* no parameter file (for restore(nil)) */

/* restore a game - returns TRUE on failure */
int fiorso(voccxdef *vctx, char *fname);

/*
 *   Look in a saved game file to determine if it has information on which
 *   GAM file created it.  If the GAM file information is available, this
 *   routine returns true and stores the game file name in the given
 *   buffer; if the information isn't available, we'll return false.  
 */
int fiorso_getgame(char *saved_file, char *buf, size_t buflen);

/* encrypt/decrypt an object */
void fioxor(uchar *p, uint siz, uint seed, uint inc);

/* strings stored in binary game file for identification and validation */

/* file header string */
#define FIOFILHDR    "TADS2 bin\012\015\032"

/* resource file header string */
#define FIOFILHDRRSC "TADS2 rsc\012\015\032"

/* CURRENT file format version string */
#define FIOVSNHDR  "v2.2.0"

/* other file format versions that can be READ by this version */
#define FIOVSNHDR2 "v2.0.0"
#define FIOVSNHDR3 "v2.0.1"

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
