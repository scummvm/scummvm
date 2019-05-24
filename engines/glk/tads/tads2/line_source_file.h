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

#ifndef GLK_TADS_TADS2_LINE_SOURCE_FILE
#define GLK_TADS_TADS2_LINE_SOURCE_FILE

#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/debug.h"
#include "glk/tads/tads2/line_source.h"
#include "glk/tads/tads2/object.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

struct tokpdef;

/* maximum number of pages of debugging records we can keep */
#define LINFPGMAX 128

/* 
 *   executable line information structure: this record relates one
 *   executable line to the object containing the p-code, and the offset
 *   in the object of the p-code for the start of the line 
 */
struct linfinfo {
    /* 
     *   OPCLINE data (file seek position or line number, depending on how
     *   the game was compiled: -ds -> file seek offset, -ds2 -> line
     *   number) 
     */
    ulong fpos;
    
    /* object number */
    objnum objn;

    /* offset from start of code */
    uint ofs;
};

/* 
 *   file line source 
 */
struct linfdef {
    lindef    linflin;                                   /* superclass data */
    osfildef *linffp;                  /* file pointer for this line source */
    char      linfbuf[100];                 /* buffer for the line contents */
    int       linfbufnxt;         /* offset in buffer of start of next line */
    int       linfnxtlen;                /* length of data after linfbufnxt */
    ulong     linfnum;                               /* current line number */
    ulong     linfseek;                    /* seek position of current line */
    mcmcxdef *linfmem;                            /* memory manager context */
    mcmon     linfpg[LINFPGMAX];             /* pages for debugging records */
    ulong     linfcrec;        /* number of debugger records written so far */
    char      linfnam[1];                        /* name of file being read */
};

/* initialize a file line source, opening the file for the line source */
linfdef *linfini(mcmcxdef *mctx, errcxdef *errctx, const char *filename,
                 int flen, tokpdef *path, int must_find_file,
                 int new_line_records);

/* initialize a pre-allocated linfdef, skipping debugger page setup */
void linfini2(mcmcxdef *mctx, linfdef *linf,
              const char *filename, int flen, osfildef *fp, int new_line_records);

/* get next line from line source */
int linfget(lindef *lin);

/* generate printable rep of current position in source (for errors) */
void linfppos(lindef *lin, char *buf, uint bufl);

/* close line source */
void linfcls(lindef *lin);

/* generate source-line debug instruction operand */
void linfglop(lindef *lin, uchar *buf);

/* generate new-style source-line debug instructino operand */
void linfglop2(lindef *lin, uchar *buf);

/* save line source to binary (.gam) file */
int linfwrt(lindef *lin, osfildef *fp);

/* load a file-line-source from binary (.gam) file */
int linfload(osfildef *fp, dbgcxdef *dbgctx, errcxdef *ec,
             tokpdef *path);

/* add a debugger line record for the current line */
void linfcmp(lindef *lin, uchar *buf);

/* find nearest line record to a file seek location */
void linffind(lindef *lin, char *buf, objnum *objp, uint *ofsp);

/* activate line source for debugging */
void linfact(lindef *lin);

/* disactivate line source */
void linfdis(lindef *lin);

/* get current seek position */
void linftell(lindef *lin, uchar *pos);

/* seek */
void linfseek(lindef *lin, uchar *pos);

/* read */
int linfread(lindef *lin, uchar *buf, uint siz);

/* add a signed delta to a seek positon */
void linfpadd(lindef *lin, uchar *pos, long delta);

/* query whether at top of file */
int linfqtop(lindef *lin, uchar *pos);

/* read one line at current seek position */
int linfgets(lindef *lin, uchar *buf, uint bufsiz);

/* get name of line source */
void linfnam(lindef *lin, char *buf);

/* get the current line number */
ulong linflnum(lindef *lin);

/* go to top or bottom */
void linfgoto(lindef *lin, int where);

/* return the current offset in the line source */
long linfofs(lindef *lin);

/* renumber an object */
void linfren(lindef *lin, objnum oldnum, objnum newnum);

/* delete an object */
void linfdelnum(lindef *lin, objnum objn);

/* copy line records to an array of linfinfo structures */
void linf_copy_linerecs(linfdef *linf, linfinfo *info);

/* debugging echo */
#ifdef DEBUG
# define LINFDEBUG(x) x
#else /* DEBUG */
# define LINFDEBUG(x)
#endif /* DEBUG */

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
