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

#include "glk/tads/tads2/line_source_file.h"
#include "glk/tads/tads2/character_map.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/tads2/tokenizer.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

enum { BYTE_MAX = 0xff };

/* initialize a pre-allocated linfdef, skipping debugger page setup */
void linfini2(mcmcxdef *mctx, linfdef *linf,
              const char *filename, int flen, osfildef *fp, int new_line_records)
{
    /* set up method pointers */
    linf->linflin.lingetp = linfget;
    linf->linflin.linclsp = linfcls;
    linf->linflin.linppos = linfppos;
    linf->linflin.linglop = (new_line_records ? linfglop2 : linfglop);
    linf->linflin.linwrtp = linfwrt;
    linf->linflin.lincmpp = linfcmp;
    linf->linflin.linactp = linfact;
    linf->linflin.lindisp = linfdis;
    linf->linflin.lintellp = linftell;
    linf->linflin.linseekp = linfseek;
    linf->linflin.linreadp = linfread;
    linf->linflin.linpaddp = linfpadd;
    linf->linflin.linqtopp = linfqtop;
    linf->linflin.lingetsp = linfgets;
    linf->linflin.linnamp = linfnam;
    linf->linflin.linlnump = linflnum;
    linf->linflin.linfindp = linffind;
    linf->linflin.lingotop = linfgoto;
    linf->linflin.linofsp = linfofs;
    linf->linflin.linrenp = linfren;
    linf->linflin.lindelp = linfdelnum;

    /* set up instance data */
    linf->linflin.linbuf = linf->linfbuf;
    linf->linflin.linflg = 0;
    memcpy(linf->linfnam, filename, (size_t)flen);
    linf->linfnam[flen] = '\0';
    linf->linfbuf[0] = '\0';
    linf->linfbufnxt = 0;
    linf->linfnxtlen = 0;
    linf->linffp = fp;
    linf->linfnum = 0;
    linf->linflin.linlln = 4;     /* OPCLINE operand is seek offset in file */
    linf->linfmem = mctx;                    /* save memory manager context */
    linf->linfcrec = 0;                  /* no debugger records written yet */
}

/* 
 *   Initialize a file line source object.  If must_find_file is true,
 *   we'll fail if we can't find the file.  Otherwise, we'll create the
 *   linfdef even if we can't find the file, reserving the maximum space
 *   for its path name to be filled in later. 
 */
linfdef *linfini(mcmcxdef *mctx, errcxdef *ec, const char *filename,
                 int flen, tokpdef *path, int must_find_file,
                 int new_line_records)
{
    int       i;
    objnum   *objp;
    linfdef  *linf;
    osfildef *fp = nullptr;
    char      fbuf[OSFNMAX + 1];
    tokpdef   fakepath;
    int       len;
    
    if (!path)
    {
        path = &fakepath;
        fakepath.tokpnxt = (tokpdef *)0;
        fakepath.tokplen = 0;
    }

    /* search through the path list */
    for ( ; path ; path = path->tokpnxt)
    {
        char last;

        /* prefix the current path */
        if ((len = path->tokplen) != 0)
        {
            memcpy(fbuf, path->tokpdir, (size_t)len);
            last = fbuf[len - 1];
            if (last == OSPATHCHAR ||
                (OSPATHALT && strchr(OSPATHALT, last)))
                /* do nothing */ ;
            else
            {
                /* append path separator character */
                fbuf[len++] = OSPATHCHAR;
            }
        }

        /* add the filename and null-terminate */
        memcpy(fbuf + len, filename, (size_t)flen);
        fbuf[len + flen] = '\0';
        
        /* attempt to open this file */
        if ((fp = osfoprs(fbuf, OSFTTEXT)) != 0)
            break;
    }

    /* 
     *   If no file opened yet, search tads path; if that doesn't work,
     *   let the debugger UI try to find the file.  If nothing works, give
     *   up and return failure.  
     */
    if (fp == 0
        && (!os_locate(filename, flen, (char *)0, fbuf, sizeof(fbuf))
            || (fp = osfoprs(fbuf, OSFTTEXT)) == 0))
    {
        /*
         *   Ask the debugger UI for advice.  If the debugger isn't
         *   present, we'll get a failure code from this routine. 
         */
        if (!dbgu_find_src(filename, flen, fbuf, sizeof(fbuf),
                           must_find_file))
            return 0;

        /* try opening the file */
        if (fbuf[0] == '\0')
        {
            /* 
             *   we didn't get a filename - the UI wants to defer finding
             *   the file until later 
             */
            fp = 0;
        }
        else
        {
            /* we got a filename from the UI - try opening it */
            fp = osfoprs(fbuf, OSFTTEXT);
        }

        /* 
         *   if the file isn't present, and we're required to find it,
         *   return failure 
         */
        if (fp == 0 && must_find_file)
            return 0;
    }
    
    /* figure out how much space we need for the file's full name */
    if (fp == 0)
    {
        /* 
         *   we didn't find the file, so we don't yet know its name - use
         *   the maximum possible filename length for the buffer size, so
         *   that we can store the final filename if we should figure out
         *   where the file is later on 
         */
        fbuf[0] = '\0';
        len = sizeof(fbuf);
    }
    else
    {
        /* 
         *   we found the file, so we have its final name - allocate space
         *   for the known name 
         */
        len = (int)strlen(fbuf);
    }

    /* allocate the linfdef */
    linf = (linfdef *)mchalo(ec, (ushort)(sizeof(linfdef) + flen
                                          + len + 1), "linfini");

    /* do the basic initialization */
    linfini2(mctx, linf, filename, flen, fp, new_line_records);

    memcpy(linf->linfnam + flen + 1, fbuf, (size_t)len);
    linf->linfnam[flen + 1 + len] = '\0';
    
    /* set all debugger pages to not-yet-allocated */
    for (i = LINFPGMAX, objp = linf->linfpg ; i ; ++objp, --i)
        *objp = MCMONINV;

    /* return the new line source object */
    return linf;
}

int linfget(lindef *lin)
{
#   define  linf ((linfdef *)lin)
    char   *p;
    size_t  rdlen;
    int     nl_len;

    /* remember seek position of start of current line */
    linf->linfseek = osfpos(linf->linffp);

    /*
     *   if we have data left in the buffer after the end of this line,
     *   move it to the start of the buffer
     */
    if (linf->linfnxtlen != 0)
    {
        /* move the data down */
        memmove(linf->linfbuf, linf->linfbuf + linf->linfbufnxt,
                linf->linfnxtlen);

        /* 
         *   adjust the seek position to account for the fact that we've
         *   read ahead in the file 
         */
        linf->linfseek -= linf->linfnxtlen;

        /* 
         *   Fill up the rest of the buffer.  Leave one byte for a null
         *   terminator and one byte for a possible extra newline pair
         *   character (see below), hence fill to sizeof(buf)-2.  
         */
        rdlen = osfrbc(linf->linffp, linf->linfbuf + linf->linfnxtlen,
                       sizeof(linf->linfbuf) - linf->linfnxtlen - 2);

        /* 
         *   the total space is the amount we had left over plus the
         *   amount we just read 
         */
        rdlen += linf->linfnxtlen;
    }
    else
    {
        /* 
         *   We have nothing in the buffer - fill it up.  Fill to
         *   sizeof(buf)-2 to leave room for a null terminator plus a
         *   possible extra newline pair character (see below). 
         */
        rdlen = osfrbc(linf->linffp, linf->linfbuf,
                       sizeof(linf->linfbuf) - 2);
    }

    /* 
     *   if there's nothing in the buffer at this point, we've reached the
     *   end of the file 
     */
    if (rdlen == 0)
        return TRUE;

    /* 
     *   if the last line was not a continuation line, increment the line
     *   counter for the start of a new line 
     */
    if (!(lin->linflg & LINFMORE))
        ++(linf->linfnum);

    /* null-terminate the buffer contents */
    linf->linfbuf[rdlen] = '\0';

    /* perform character mapping on th new part only */
    for (p = linf->linfbuf + linf->linfnxtlen ; *p != '\0' ; ++p)
        *p = cmap_n2i(*p);

    /* 
     *   scan the for the first newline in the buffer, allowing newline
     *   conventions that involve either CR or LF 
     */
    for (p = linf->linfbuf ; *p != '\n' && *p != '\r' && *p != '\0' ; ++p) ;

    /*
     *   Check to see if this character is followed by its newline pair
     *   complement, to allow for either CR-LF or LF-CR sequences, as well
     *   as plain single-byte newline (CR or LF) sequences.
     *   
     *   First, though, one weird special case: if this character is at
     *   the read limit in the buffer, the complementary character might
     *   be lurking in the next byte that we haven't read.  In this case,
     *   use that one-byte reserve we have left (we filled the buffer only
     *   to length-2 so far) and read the next byte.  
     */
    if (*p != '\0' && p + 1 == linf->linfbuf + sizeof(linf->linfbuf) - 2)
    {
        /* 
         *   we've filled the buffer to but not including the reserve for
         *   just this case - fetch the extra character 
         */
        if (osfrbc(linf->linffp, p + 1, 1) == 1)
        {
            /* increase the total read length for the extra byte */
            ++rdlen;
            *(p+2) = '\0';
        }
    }

    /* 
     *   now we can check for the newline type, since we have definitely
     *   read the full paired sequence 
     */
    if (*p == '\0')
    {
        /* there's no newline in the buffer - we'll return a partial line */
        nl_len = 0;

        /* set the partial line flag */
        lin->linflg |= LINFMORE;

        /* return the entire buffer */
        lin->linlen = rdlen;

        /* there's nothing left for the next time through */
        linf->linfnxtlen = 0;
    }
    else
    {
        /* check for a complementary pair */
        if ((*p == '\n' && *(p+1) == '\r') || (*p == '\r' && *(p+1) == '\n'))
        {
            /* we have a paired newline */
            nl_len = 2;
        }
        else
        {
            /* we have but a single-character newline sequence */
            nl_len = 1;
        }

        /* this is the end of a line */
        lin->linflg &= ~LINFMORE;

        /* 
         *   return only the part of the buffer up to, but not including,
         *   the newline 
         */
        lin->linlen = (p - linf->linfbuf);

        /* null-terminate the buffer at the newline */
        *p = '\0';

        /* 
         *   anything remaining after the newline sequence is available
         *   for reading the next time through 
         */
        linf->linfbufnxt = ((p + nl_len) - linf->linfbuf);
        linf->linfnxtlen = rdlen - linf->linfbufnxt;
    }

    /* make sure buffer pointer is correct */
    lin->linbuf = linf->linfbuf;

    LINFDEBUG(printf("%s\n", linf->linfbuf));

    /* success */
    return FALSE;

#   undef  linf
}

/* make printable string from position in file (for error reporting) */
void linfppos(lindef *lin, char *buf, uint buflen)
{
    VARUSED(buflen);
    
    sprintf(buf, "%s(%lu): ", ((linfdef *)lin)->linfnam,
            ((linfdef *)lin)->linfnum);
}

/* close line source */
void linfcls(lindef *lin)
{
    osfcls(((linfdef *)lin)->linffp);
}

/* generate operand of OPCLINE (source-line debug) instruction */
void linfglop(lindef *lin, uchar *buf)
{
    oswp4(buf, ((linfdef *)lin)->linfseek);   /* save seek position of line */
}

/* generate new-style operand of OPCLINE instruction */
void linfglop2(lindef *lin, uchar *buf)
{
    oswp4(buf, ((linfdef *)lin)->linfnum);    /* save seek position of line */
}

/* save line source information to binary (.gam) file; TRUE ==> error */
int linfwrt(lindef *lin, osfildef *fp)
{
#define linf ((linfdef *)lin)
    uchar   buf[BYTE_MAX + 6];
    size_t  len;
    uint    pgcnt;
    uchar  *objp;
    mcmon  *objn;

    buf[0] = lin->linid;
    len = strlen(linf->linfnam);
    if (len > BYTE_MAX)
        return FALSE;
    buf[1] = (uchar)len;
    oswp4(buf + 2, linf->linfcrec);
    memcpy(buf + 6, linf->linfnam, (size_t)buf[1]);
    if (osfwb(fp, buf, (int)(buf[1] + 6))) return(TRUE);
    
    /* write the debug source pages */
    if (!linf->linfcrec) return(FALSE);          /* no debug records at all */
    pgcnt = 1 + ((linf->linfcrec - 1) >> 10);     /* figure number of pages */
    
    for (objn = linf->linfpg ; pgcnt ; ++objn, --pgcnt)
    {
        objp = mcmlck(linf->linfmem, *objn);
        if (osfwb(fp, objp, (1024 * DBGLINFSIZ))) return(TRUE);
        mcmunlck(linf->linfmem, *objn);
    }
    
    return(FALSE);

#   undef  linf
}

/* load a file-line-source from binary (.gam) file */
int linfload(osfildef *fp, dbgcxdef *dbgctx, errcxdef *ec, tokpdef *path)
{
    linfdef *linf;
    uchar    buf[BYTE_MAX + 6];
    uint     pgcnt;
    uchar   *objp;
    mcmon   *objn;
    
    /* read the source's description from the file */
    if (osfrb(fp, buf, 6)
        || osfrb(fp, buf + 6, (int)buf[1]))
        return TRUE;
    
    /* initialize the linfdef */
    if (!(linf = linfini(dbgctx->dbgcxmem, ec, (char *)buf + 6,
                         (int)buf[1], path, FALSE, FALSE)))
    {
        errlog1(ec, ERR_NOSOURC, ERRTSTR,
                errstr(ec, (char *)buf+6, (int)buf[1]));
        return TRUE;
    }
    
    /* if we opened the file, close it - don't hold all files open */
    if (linf->linffp != 0)
    {
        osfcls(linf->linffp);
        linf->linffp = 0;
    }
    
    /* link into debug line source chain */
    linf->linflin.linnxt = dbgctx->dbgcxlin;
    dbgctx->dbgcxlin = &linf->linflin;
    linf->linflin.linid = buf[0];
    linf->linfcrec = osrp4(buf + 2);
    
    /* make sure the max line id is set above current line */
    if (buf[0] >= dbgctx->dbgcxfid)
        dbgctx->dbgcxfid = buf[0] + 1;
    
    /* make sure we have some debug records */
    if (!linf->linfcrec)
        return FALSE;

    /* figure number of pages */
    pgcnt = 1 + ((linf->linfcrec - 1) >> 10);
    
    /* allocate and read the debug source pages */
    for (objn = linf->linfpg ; pgcnt ; ++objn, --pgcnt)
    {
        objp = mcmalo(linf->linfmem, (ushort)(1024 * DBGLINFSIZ), objn);
        if (osfrb(fp, objp, (1024 * DBGLINFSIZ))) return(TRUE);
        mcmunlck(linf->linfmem, *objn);
    }

    /* success */
    return FALSE;
}

/* add a debugger line record for the current line being compiled */
void linfcmp(lindef *lin, uchar *buf)
{
    uint    pg;
    uchar  *objptr;
#   define  linf ((linfdef *)lin)

    /* figure out which page to use, and lock it */
    pg = linf->linfcrec >> 10;                     /* 2^10 records per page */
    if (pg >= LINFPGMAX)
        errsig(linf->linfmem->mcmcxgl->mcmcxerr, ERR_MANYDBG);
    if (linf->linfpg[pg] == MCMONINV)
        objptr = mcmalo(linf->linfmem, (ushort)(1024 * DBGLINFSIZ),
                        &linf->linfpg[pg]);
    else
        objptr = mcmlck(linf->linfmem, linf->linfpg[pg]);
    
    /* write the record to the appropriate offset within the page */
    memcpy(objptr + (linf->linfcrec & 1023) * DBGLINFSIZ, buf,
           (size_t)DBGLINFSIZ);
    
    /* increment counter of line records so far */
    ++(linf->linfcrec);
    
    /* done with page - touch it and unlock it */
    mcmtch(linf->linfmem, linf->linfpg[pg]);
    mcmunlck(linf->linfmem, linf->linfpg[pg]);
    
#   undef linf
}

/*
 *   Renumber an existing object.  Searches through all line records for
 *   any with the given object number, and changes the number to the new
 *   number if found.  
 */
void linfren(lindef *lin, objnum oldnum, objnum newnum)
{
#   define  linf ((linfdef *)lin)
    uint    pgcnt;
    uchar  *objp;
    mcmon  *pgobjn;
    int     i;
    int     pgtot;
    int     tot;

    /* figure the number of pages - if no lines, stop now */
    tot = linf->linfcrec;
    if (tot == 0)
        return;

    /* calculate the number of pages to check */
    pgcnt = 1 + ((tot - 1) >> 10);

    /* scan each page */
    for (pgobjn = linf->linfpg ; pgcnt ; ++pgobjn, --pgcnt, tot -= 1024)
    {
        /* lock the page */
        objp = mcmlck(linf->linfmem, *pgobjn);

        /* figure the number on this page */
        pgtot = (tot > 1024 ? 1024 : tot);

        /* scan each record on this page */
        for (i = 0 ; i < pgtot ; ++i, objp += DBGLINFSIZ)
        {
            /* check this one */
            if (osrp2(objp) == oldnum)
            {
                /* it matches - renumber it */
                oswp2(objp, newnum);
            }
        }

        /* done with the page - touch it and unlock it */
        mcmtch(linf->linfmem, *pgobjn);
        mcmunlck(linf->linfmem, *pgobjn);
    }

#   undef  linf
}

/*
 *   Delete an existing object.  Searches through all line records for any
 *   with the given object number, and removes line records for the object
 *   number if found.  
 */
void linfdelnum(lindef *lin, objnum objn)
{
#   define  linf ((linfdef *)lin)
    uint    pgcnt;
    uchar  *objp;
    uchar  *objp_orig;
    mcmon  *pgobjn;
    int     i;
    int     pgtot;
    int     tot;

    /* figure the number of pages - if no lines, stop now */
    tot = linf->linfcrec;
    if (tot == 0)
        return;

    /* calculate the number of pages to check */
    pgcnt = 1 + ((tot - 1) >> 10);

    /* scan each page */
    for (pgobjn = linf->linfpg ; pgcnt ; ++pgobjn, --pgcnt, tot -= 1024)
    {
        /* lock the page */
        objp = objp_orig = mcmlck(linf->linfmem, *pgobjn);

        /* figure the number on this page */
        pgtot = (tot > 1024 ? 1024 : tot);

        /* scan each record on this page */
        for (i = 0 ; i < pgtot ; ++i, objp += DBGLINFSIZ)
        {
            int j;
            
            /* check this one */
            if (osrp2(objp) == objn)
            {
                uchar *nxtp;
                uint pg;
                int delcnt;
                int totrem;
                
                /* 
                 *   it matches - delete it, along with any subsequent
                 *   contiguous entries that also match it 
                 */
                for (delcnt = 1, j = i + 1 ; j < pgtot ; ++j, ++delcnt)
                {
                    /* 
                     *   if this one doesn't match, we've found the end of
                     *   the contiguous records for this object 
                     */
                    if (osrp2(objp + (j - i)*DBGLINFSIZ) != objn)
                        break;
                }

                /* close up the gap on this page */
                if (j < pgtot)
                    memmove(objp, objp + delcnt*DBGLINFSIZ,
                            (pgtot - j)*DBGLINFSIZ);

                /* 
                 *   if this isn't the last page, copy the bottom of the
                 *   next page to the gap at the top of this page 
                 */
                if (pgcnt > 1)
                {
                    /* lock the next page */
                    nxtp = mcmlck(linf->linfmem, *(pgobjn + 1));

                    /* 
                     *   copy from the beginning of the next page to the
                     *   end of this page 
                     */
                    memcpy(objp_orig + (pgtot - delcnt)*DBGLINFSIZ,
                           nxtp, delcnt*DBGLINFSIZ);

                    /* done with the page */
                    mcmunlck(linf->linfmem, *(pgobjn + 1));
                }
                else
                {
                    /* 
                     *   this is the last page, so there's no next page to
                     *   copy items from - reduce the count of items on
                     *   this page accordingly 
                     */
                    pgtot -= delcnt;
                }

                /*
                 *   Now rearrange all subsequent pages to accommodate the
                 *   gap we just created 
                 */
                for (totrem = tot, pg = 1 ; pg < pgcnt ;
                     totrem -= 1024, ++pg)
                {
                    uchar *curp;
                    int curtot;

                    /* figure how many we have on this page */
                    curtot = (totrem > 1024 ? 1024 : totrem);
                    
                    /* lock this page */
                    curp = mcmlck(linf->linfmem, *(pgobjn + pg));

                    /* delete from the start of this page */
                    memmove(curp, curp + delcnt*DBGLINFSIZ,
                            (curtot - delcnt)*DBGLINFSIZ);

                    /* if there's another page, copy from it */
                    if (pg + 1 < pgcnt)
                    {
                        /* lock the next page */
                        nxtp = mcmlck(linf->linfmem, *(pgobjn + pg + 1));

                        /* 
                         *   copy from the start of the next page to the
                         *   end of this page 
                         */
                        memcpy(curp + (curtot - delcnt)*DBGLINFSIZ,
                               nxtp, delcnt*DBGLINFSIZ);

                        /* unlock it */
                        mcmunlck(linf->linfmem, *(pgobjn + pg + 1));
                    }

                    /* done with the page - touch it and unlock it */
                    mcmtch(linf->linfmem, *(pgobjn + pg));
                    mcmunlck(linf->linfmem, *(pgobjn + pg));
                }

                /* deduct the removed records from the total */
                linf->linfcrec -= delcnt;
            }
        }

        /* done with the page - touch it and unlock it */
        mcmtch(linf->linfmem, *pgobjn);
        mcmunlck(linf->linfmem, *pgobjn);
    }

#   undef  linf
}


/* find the nearest line record to a file seek location */
void linffind(lindef *lin, char *buf, objnum *objp, uint *ofsp)
{
#   define  linf ((linfdef *)lin)
    uint    pg;
    uchar  *objptr;
    uchar  *bufptr;
    long    first;
    long    last;
    long    cur;
    ulong   seekpos;
	ulong   curpos = 0;
    objnum  objn;
    uint    ofs;

    /* get desired seek position out of buffer */
    seekpos = osrp4(buf);

    /* we haven't traversed any records yet */
    objn = MCMONINV;
    ofs = 0;
    
    /* run a binary search for the indicated line record */
    first = 0;
    last = linf->linfcrec - 1;
    for (;;)
    {
        /* make sure we're not out of records entirely */
        if (first > last)
        {
            /* return the most recent record found - it's closest */
            *objp = objn;
            *ofsp = ofs;

            /* set the position to that of the line we actually found */
            oswp4(buf, curpos);
            return;
        }

        /* split the difference */
        cur = first + (last - first)/2;

        /* calculate the page containing this item */
        pg = cur >> 10;
        
        /* get object + offset corresponding to current source line */
        objptr = mcmlck(linf->linfmem, linf->linfpg[pg]);
        bufptr = objptr + ((cur & 1023) * DBGLINFSIZ);
        objn = osrp2(bufptr);
        ofs = osrp2(bufptr + 2);
        mcmunlck(linf->linfmem, linf->linfpg[pg]);

        /* read user data out of the object's OPCLINE record */
        objptr = mcmlck(linf->linfmem, (mcmon)objn);
        bufptr = objptr + ofs + 5;
        curpos = osrp4(bufptr);
        mcmunlck(linf->linfmem, (mcmon)objn);

        /* see what we have */
        if (curpos == seekpos)
        {
            *objp = objn;
            *ofsp = ofs;
            return;
        }
        else if (curpos < seekpos)
            first = (cur == first ? first + 1 : cur);
        else
            last = (cur == last ? last - 1 : cur);
    }
    
#   undef linf
}

/* 
 *   copy line records to an array of linfinfo structures 
 */
void linf_copy_linerecs(linfdef *linf, struct linfinfo *info)
{
    uint    pg;
    uint    prvpg;
    uchar  *objptr;
    uchar  *bufptr;
    long    last;
    long    cur;

    /* note the last element */
    last = linf->linfcrec;

    /* if there are no records, there's nothing to do */
    if (last == 0)
        return;

    /* load the first page of records */
    prvpg = 0;
    pg = 0;
    objptr = mcmlck(linf->linfmem, linf->linfpg[0]);

    /* scan the records */
    for (cur = 0 ; cur < last ; ++cur, ++info)
    {
        uchar *codeptr;
        
        /* calculate the page containing this item */
        pg = cur >> 10;

        /* if it's different than the last page, load the next page */
        if (pg != prvpg)
        {
            /* unlock the previous page */
            mcmunlck(linf->linfmem, linf->linfpg[prvpg]);

            /* load the next page */
            objptr = mcmlck(linf->linfmem, linf->linfpg[pg]);

            /* this is now the previous page */
            prvpg = pg;
        }
            
        /* get object + offset corresponding to current source line */
        bufptr = objptr + ((cur & 1023) * DBGLINFSIZ);
        info->objn = osrp2(bufptr);
        info->ofs = osrp2(bufptr + 2);

        /* read source location data out of the object's OPCLINE record */
        codeptr = mcmlck(linf->linfmem, (mcmon)info->objn);
        bufptr = codeptr + info->ofs + 5;
        info->fpos = osrp4(bufptr);
        mcmunlck(linf->linfmem, (mcmon)info->objn);
    }

    /* unlock the last page */
    mcmunlck(linf->linfmem, linf->linfpg[prvpg]);
}

/* disactivate line source under debugger - close file */
void linfdis(lindef *lin)
{
#   define linf ((linfdef *)lin)

    if (linf->linffp)
    {
        osfcls(linf->linffp);
        linf->linffp = (osfildef *)0;
    }
    
#   undef linf
}

/* activate line source under debugger - open file */
void linfact(lindef *lin)
{
    char   *fname;
#   define  linf ((linfdef *)lin)

    /* get the name buffer, and advance to the full path name portion */
    fname = linf->linfnam;
    fname += strlen(fname) + 1;

    /*
     *   If the full path name is empty, it means that the UI told us to
     *   defer searching for the file until we actually need the file.  At
     *   this point, we actually need the file.  Ask the UI again to find
     *   the file. 
     */
    if (fname[0] != '\0'
        || dbgu_find_src(linf->linfnam, strlen(linf->linfnam),
                         fname, OSFNMAX, TRUE))
    {
        /* open the file */
        linf->linffp = osfoprs(fname, OSFTTEXT);
    }
    else
    {
        /* there's no file to open */
        linf->linffp = 0;
    }

#   undef linf
}

/* get current seek position */
void linftell(lindef *lin, uchar *pos)
{
#   define  linf ((linfdef *)lin)
    long    seekpos;

    seekpos = osfpos(linf->linffp);
    oswp4(pos, seekpos);

#   undef linf
}

/* seek to a new position */
void linfseek(lindef *lin, uchar *pos)
{
#   define  linf ((linfdef *)lin)
    long    seekpos;
    
    seekpos = osrp4(pos);
    osfseek(linf->linffp, seekpos, OSFSK_SET);

#   undef linf
}

/* read bytes - fread-style interface */
int linfread(lindef *lin, uchar *buf, uint siz)
{
#   define  linf ((linfdef *)lin)

    return osfrbc(linf->linffp, buf, siz);

#   undef linf
}

/* add a signed delta to a seek position */
void linfpadd(lindef *lin, uchar *pos, long delta)
{
#   define  linf ((linfdef *)lin)
    long    seekpos;

    seekpos = osrp4(pos);
    seekpos += delta;
    if (seekpos < 0) seekpos = 0;
    oswp4(pos, seekpos);

#   undef linf
}

/* query whether we're at top of file */
int linfqtop(lindef *lin, uchar *pos)
{
#   define  linf ((linfdef *)lin)

    return(osrp4(pos) == 0);

#   undef linf
}

/* read one line at current position - fgets-style interface */
int linfgets(lindef *lin, uchar *buf, uint siz)
{
    int    ret;
    long   startpos;
    uchar *p;
#   define  linf ((linfdef *)lin)

    /* note the seek offset at the start of the line */
    startpos = osfpos(linf->linffp);

    /* read the next line */
    ret = (osfgets((char *)buf, siz, linf->linffp) != 0);
    if (!ret)
        return ret;

    /* scan for non-standard line endings */
    for (p = buf ; *p != '\0' && *p != '\r' && *p != '\n' ; ++p) ;
    if (*p != '\0')
    {
        uchar *nxt;
        
        /* 
         *   Scan for non-line-ending characters after this line-ending
         *   character.  If we find any, we must have non-standard newline
         *   conventions in this file.  To be tolerant of these, seek back
         *   to the start of the next line in these cases and read the
         *   next line from the new location. 
         */
        for (nxt = p + 1 ; *nxt == '\r' || *nxt == '\n' ; ++nxt) ;
        if (*nxt == '\0')
        {
            /* 
             *   we had only line-ending characters after the first
             *   line-ending character -- simply end the line after the
             *   first line-ending character 
             */
            *(p+1) = '\0';
        }
        else
        {
            /*
             *   We had a line-ending character in the middle of other
             *   text, so we must have a file that doesn't conform to
             *   local newline conventions.  Seek back to the next
             *   character following the last line-ending character so
             *   that we start the next line here, and end the current
             *   line after the first line-ending character. 
             */
            *(p+1) = '\0';
            osfseek(linf->linffp, startpos + (nxt - buf), OSFSK_SET);
        }
    }

    /* return the result */
    return ret;

#   undef linf
}

/* get name of line source */
void linfnam(lindef *lin, char *buf)
{
#   define  linf ((linfdef *)lin)

    strcpy(buf, linf->linfnam);

#   undef linf
}

/* get the current line number */
ulong linflnum(lindef *lin)
{
#   define  linf ((linfdef *)lin)

    return linf->linfnum;

#   undef linf
}

/* go to top/bottom of line source */
void linfgoto(lindef *lin, int where)
{
#   define  linf ((linfdef *)lin)

    osfseek(linf->linffp, 0L, where);

#   undef linf
}

/* return current seek offset within source */
long linfofs(lindef *lin)
{
#   define linf ((linfdef *)lin)

    return(osfpos(linf->linffp));
    
#   undef  linf
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
