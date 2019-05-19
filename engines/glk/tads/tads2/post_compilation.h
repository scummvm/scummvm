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

/* definitions for post-compilation setup
 */

#ifndef GLK_TADS_TADS2_POST_COMPILATION
#define GLK_TADS_TADS2_POST_COMPILATION

#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/tokenizer.h"
#include "glk/tads/tads2/vocabulary.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* setup context */
struct supcxdef {
    errcxdef *supcxerr;
    mcmcxdef *supcxmem;                    /* memory manager client context */
    voccxdef *supcxvoc;                   /* player command parsing context */
    tokthdef *supcxtab;                           /* top-level symbol table */
    runcxdef *supcxrun;                                /* execution context */
    uchar    *supcxbuf;                        /* space for building a list */
    ushort    supcxlen;                                   /* size of buffer */
};

/* set up contents list for one object for demand-on-load */
extern void supcont(void *ctx, objnum obj, prpnum prp);

/* set up inherited vocabulary (called before executing game) */
extern void supivoc(supcxdef *ctx);

/* find required objects/functions */
extern void supfind(errcxdef *ctx, tokthdef *tab, voccxdef *voc,
             objnum *preinit, int warnlevel, int casefold);

/* set up reserved words */
extern void suprsrv(supcxdef *sup, void (*bif[])(struct bifcxdef *, int),
             toktdef *tab, int fncntmax, int v1compat, char *new_do,
             int casefold);

/* set up built-in functions without symbol table (for run-time) */
extern void supbif(supcxdef *sup, void (*bif[])(struct bifcxdef *, int),
            int bifsiz);

/* log an undefined-object error */
extern void sup_log_undefobj(mcmcxdef *mctx, errcxdef *ec, int err,
                      char *sym_name, int sym_name_len, objnum objn);

/* set up inherited vocabulary for a particular object */
extern void supivoc1(supcxdef *sup, voccxdef *ctx, vocidef *v, objnum target,
              int inh_from_obj, int flags);

/* get name of an object out of symbol table */
extern void supgnam(char *buf, tokthdef *tab, objnum objn);

/* table of built-in functions */
struct supbidef {
    char  *supbinam;                                    /* name of function */
    void (*supbifn)(struct bifcxdef *, int);           /* C routine to call */
};

/* external definition for special token table */
//extern tokldef supsctab[];

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
