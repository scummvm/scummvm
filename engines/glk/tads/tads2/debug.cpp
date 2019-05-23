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

#include "glk/tads/tads2/debug.h"
#include "glk/tads/tads2/list.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* add a string to the history buffer */
void dbgaddhist(dbgcxdef *ctx, char *buf, int l)
{
    char *p;
    int   dell;

    if (ctx->dbgcxhstf + l + 1 >= ctx->dbgcxhstl)
    {
        /* delete first lines until we have enough space */
        for (dell = 0, p = ctx->dbgcxhstp ; *p || dell < l ; ++p, ++dell) ;
        if (*p) ++p;
        memmove(ctx->dbgcxhstp, ctx->dbgcxhstp + dell,
                (size_t)(ctx->dbgcxhstf - dell));
        ctx->dbgcxhstf -= dell;
    }
    memcpy(ctx->dbgcxhstp + ctx->dbgcxhstf, buf, (size_t)l);
    ctx->dbgcxhstf += l;
}

/* callback for dbgent - saves history line to a char buffer */
static void dbgent1(void *ctx0, const char *str, int strl)
{
    char **ctx = (char **)ctx0;
    memcpy(*ctx, str, (size_t)strl);
    *ctx += strl;
}

void dbgent(dbgcxdef *ctx, runsdef *bp, objnum self, objnum target,
            prpnum prop, int binum, int argc)
{
    dbgfdef *p;
    
    ++(ctx->dbgcxdep);                            /* increment actual depth */
    if (ctx->dbgcxfcn == DBGMAXFRAME)
    {
        --(ctx->dbgcxfcn);                             /* back to top frame */
        memmove(ctx->dbgcxfrm, ctx->dbgcxfrm + 1,
                (size_t)((DBGMAXFRAME - 1) * sizeof(dbgfdef)));
    }
    p = &ctx->dbgcxfrm[ctx->dbgcxfcn];
    ++(ctx->dbgcxfcn);                           /* increment frame pointer */

    p->dbgfbp = bp;
    p->dbgfself = self;
    p->dbgftarg = target;
    p->dbgfprop = prop;
    p->dbgfbif  = binum;
    p->dbgfargc = argc;
    p->dbgffr = 0;                        /* no frame has yet been recorded */
    p->dbgflin = 0;

    /* save call history */
    if (ctx->dbgcxflg & DBGCXFTRC)
    {
        char  buf[128];
        char *tmp;
        int   l;

        tmp = buf;
        dbgstktr(ctx, dbgent1, &tmp, -1, TRUE, FALSE);
        if ((l = (tmp - buf)) > 0 && buf[l-1] == '\n') --l;
        buf[l++] = '\0';
        dbgaddhist(ctx, buf, l);
    }
}

void dbglv(dbgcxdef *ctx, int exittype)
{
    --(ctx->dbgcxdep);                            /* decrement actual depth */
    if (ctx->dbgcxfcn) --(ctx->dbgcxfcn);        /* decrement frame pointer */

    /* 
     *   if we're in STEP OUT/OVER mode, and the target context is level
     *   0, and we're now at level 0, it means that we are stepping out of
     *   a routine called directly by the system and the debugger is
     *   supposed to break when that happens -- return to single-stepping
     *   mode so that we break into the debugger the next time the system
     *   calls a method 
     */
    if ((ctx->dbgcxflg & DBGCXFSS) != 0
        && (ctx->dbgcxflg & DBGCXFSO) != 0
        && ctx->dbgcxsof == 0 && ctx->dbgcxdep == 0)
    {
        /* 
         *   stepping out/over at level 0 - go to normal single-step mode
         *   (clear the out/over flag) 
         */
        ctx->dbgcxflg &= ~DBGCXFSO;
    }

    /* record exit in call history if appropriate */
    if (ctx->dbgcxflg & DBGCXFTRC)
    {
        char   buf[128];
        char  *p;

        switch(exittype)
        {
        case DBGEXVAL:
            if (ctx->dbgcxfcn > 1)
            {
                memset(buf, ' ', (size_t)(ctx->dbgcxfcn - 1));
                dbgaddhist(ctx, buf, (int)ctx->dbgcxfcn - 1);
            }
            memcpy(buf, " => ", (size_t)4);
            p = buf + 4;
            dbgpval(ctx, ctx->dbgcxrun->runcxsp - 1,
                    dbgent1, &p, TRUE);
            *p++ = '\0';
            dbgaddhist(ctx, buf, (int)(p - buf));
            break;

        case DBGEXPASS:
            memcpy(buf, " [pass]", (size_t)8);
            dbgaddhist(ctx, buf, 8);
            break;
        }
    }
}

/* get a symbol name; returns length of name */
int dbgnam(dbgcxdef *ctx, char *outbuf, int typ, int val)
{
    toksdef  sym;

    if (!ctx->dbgcxtab || !ctx->dbgcxtab->tokthhsh)
    {
        memcpy(outbuf, "<NO SYMBOL TABLE>", (size_t)17);
        return(17);
    }
    
    if (tokthfind((toktdef *)ctx->dbgcxtab, typ, val, &sym))
    {
        memcpy(outbuf, sym.toksnam, (size_t)sym.tokslen);
        return(sym.tokslen);
    }
    else if (typ == TOKSTOBJ)
    {
        if ((mcmon)val == MCMONINV)
        {
            memcpy(outbuf, "<invalid object>", 16);
            return 16;
        }
        else
        {
            sprintf(outbuf, "<object#%u>", val);
            return strlen(outbuf);
        }
    }
    else
    {
        memcpy(outbuf, "<UNKNOWN>", (size_t)9);
        return(9);
    }
}

/* send a buffer value (as from a list) to ui callback for display */
static void dbgpbval(dbgcxdef *ctx, dattyp typ, const uchar *val,
                     void (*dispfn)(void *, const char *, int),
                     void *dispctx)
{
    char  buf[TOKNAMMAX + 1];
    const char *p = buf;
    uint  len;

    switch(typ)
    {
    case DAT_NUMBER:
        sprintf(buf, "%ld", (long)osrp4s(val));
        len = strlen(buf);
        break;
        
    case DAT_OBJECT:
        len = dbgnam(ctx, buf, TOKSTOBJ, osrp2(val));
        break;
        
    case DAT_SSTRING:
        len = osrp2(val) - 2;
        p = (const char *)val + 2;
        break;
        
    case DAT_NIL:
        p = "nil";
        len = 3;
        break;

    case DAT_LIST:
        (*dispfn)(dispctx, "[", 1);
        len = osrp2(val) - 2;
        p = (const char *)val + 2;
        while (len)
        {
            dbgpbval(ctx, (dattyp)*p, (const uchar *)(p + 1), dispfn, dispctx);
            lstadv((uchar **)&p, &len);
            if (len) (*dispfn)(dispctx, " ", 1);
        }
        (*dispfn)(dispctx, "]", 1);
        len = 0;
        break;
        
    case DAT_TRUE:
        p = "true";
        len = 4;
        break;
        
    case DAT_FNADDR:
        (*dispfn)(dispctx, "&", 1);
        len = dbgnam(ctx, buf, TOKSTFUNC, osrp2(val));
        break;
        
    case DAT_PROPNUM:
        (*dispfn)(dispctx, "&", 1);
        len = dbgnam(ctx, buf, TOKSTPROP, osrp2(val));
        break;
        
    default:
        p = "[unknown type]";
        len = 14;
        break;
    }

    if (typ == DAT_SSTRING) (*dispfn)(dispctx, "'", 1);
    if (len) (*dispfn)(dispctx, p, len);
    if (typ == DAT_SSTRING) (*dispfn)(dispctx, "'", 1);
}

/* send a value to ui callback for display */
void dbgpval(dbgcxdef *ctx, runsdef *val,
             void (*dispfn)(void *ctx, const char *str, int strl),
             void *dispctx,
             int showtype)
{
    uchar   buf[TOKNAMMAX + 1];
    uint    len;
    const uchar  *p = buf;
    const char *typ = 0;
    
    switch(val->runstyp)
    {
    case DAT_NUMBER:
        sprintf((char *)buf, "%ld", val->runsv.runsvnum);
        len = strlen((char *)buf);
        typ = "number";
        break;
        
    case DAT_OBJECT:
        len = dbgnam(ctx, (char *)buf, TOKSTOBJ, val->runsv.runsvobj);
        typ = "object";
        break;
        
    case DAT_SSTRING:
        len = osrp2(val->runsv.runsvstr) - 2;
        p = val->runsv.runsvstr + 2;
        typ = "string";
        break;
        
    case DAT_NIL:
        p = (const uchar *)"nil";
        len = 3;
        break;

    case DAT_LIST: {
        if (showtype) (*dispfn)(dispctx, "list: ", 6);
        (*dispfn)(dispctx, "[", 1);
        len = osrp2(val->runsv.runsvstr) - 2;
        uchar *up = val->runsv.runsvstr + 2;
        while (len)
        {
            dbgpbval(ctx, (dattyp)*up, (const uchar *)(up + 1), dispfn, dispctx);
            lstadv(&up, &len);
            if (len) (*dispfn)(dispctx, " ", 1);
        }
        (*dispfn)(dispctx, "]", 1);
        len = 0;
	p = up;
        break;
    } 
    case DAT_TRUE:
        p = (const uchar *)"true";
        len = 4;
        break;
        
    case DAT_FNADDR:
        len = dbgnam(ctx, (char *)buf, TOKSTFUNC, val->runsv.runsvobj);
        typ = "function pointer";
        break;
        
    case DAT_PROPNUM:
        len = dbgnam(ctx, (char *)buf, TOKSTPROP, val->runsv.runsvprp);
        typ = "property pointer";
        break;
        
    default:
        p = (const uchar *)"[unknown type]";
        len = 14;
        break;
    }

    /* show the type prefix if desired, or add a quote if it's a string */
    if (typ && showtype)
    {
        /* show the type prefix */
        (*dispfn)(dispctx, typ, (int)strlen(typ));
        (*dispfn)(dispctx, ": ", 2);
    }
    else if (val->runstyp == DAT_SSTRING)
    {
        /* it's a string, and we're not showing a type - add a quote */
        (*dispfn)(dispctx, "'", 1);
    }

    /* 
     *   if possible, null-terminate the buffer - do this only if the
     *   length is actually within the buffer, which won't be the case if
     *   the text comes from someplace outside the buffer (which is the
     *   case if it's a string, for example) 
     */
    if (len < sizeof(buf))
        buf[len] = '\0';

    /* display a "&" prefix if it's an address of some kind */
    if (val->runstyp == DAT_PROPNUM || val->runstyp == DAT_FNADDR)
        (*dispfn)(dispctx, "&", 1);

    /* display the text */
    if (len != 0)
        (*dispfn)(dispctx, (const char *)p, len);

    /* add a closing quote if it's a string and we showed an open quote */
    if (val->runstyp == DAT_SSTRING && !(typ && showtype))
        (*dispfn)(dispctx, "'", 1);
}

void dbgstktr(dbgcxdef *ctx,
              void (*dispfn)(void *ctx, const char *str, int strl),
              void *dispctx,
              int level, int toponly, int include_markers)
{
    dbgfdef *f;
    int      i;
    int      j;
    int      k;
    char     buf[128];
    char    *p;
    char     c;

    for (i = ctx->dbgcxfcn, j = ctx->dbgcxdep, f = &ctx->dbgcxfrm[i-1]
         ; i ; --f, --j, --i)
    {
        p = buf;
        if (toponly)
        {
            int v = (i < 50 ? i : 50);

            if (v > 1)
            {
                memset(buf, ' ', (size_t)(v - 1));
                dbgaddhist(ctx, buf, v-1);
            }
        }
        else if (include_markers)
        {
            c = (i == level + 1 ? '*' : ' ');
            sprintf(buf, "%3d%c  ", j, c);
            p += 4;
        }

        if (f->dbgftarg == MCMONINV)
            p += dbgnam(ctx, p, TOKSTBIFN, f->dbgfbif);
        else
            p += dbgnam(ctx, p,
                        (f->dbgfself == MCMONINV ? TOKSTFUNC : TOKSTOBJ),
                        (int)f->dbgftarg);

        if (f->dbgfself != MCMONINV && f->dbgfself != f->dbgftarg)
        {
            memcpy(p, "<self=", (size_t)6);
            p += 6;
            p += dbgnam(ctx, p, TOKSTOBJ, (int)f->dbgfself);
            *p++ = '>';
        }
        if (f->dbgfprop)
        {
            *p++ = '.';
            p += dbgnam(ctx, p, TOKSTPROP, (int)f->dbgfprop);
        }
        
        /* display what we have so far */
        *p++ = '\0';
        (*dispfn)(dispctx, buf, (int)strlen(buf));

        /* display arguments if there are any */
        if (f->dbgfself == MCMONINV || f->dbgfargc != 0)
        {
            (*dispfn)(dispctx, "(", 1);
            for (k = 0 ; k < f->dbgfargc ; ++k)
            {
                dbgpval(ctx, f->dbgfbp - k - 2, dispfn, dispctx, FALSE);
                if (k + 1 < f->dbgfargc) (*dispfn)(dispctx, ", ", 2);
            }
            (*dispfn)(dispctx, ")", 1);
        }
        
        /* send out a newline, then move on to next frame */
        (*dispfn)(dispctx, "\n", 1);

        /* we're done if doing one function only */
        if (toponly) break;
    }
}

static void dbgdsdisp(void *ctx, const char *buf, int bufl)
{
    if (buf[0] == '\n')
        tioflush((tiocxdef *)ctx);
    else
        tioputslen((tiocxdef *)ctx, (const char *)buf, bufl);
}

/* dump the stack */
void dbgds(dbgcxdef *ctx)
{
    /* don't do stack dumps if we're running from the debugger command line */
    if (ctx->dbgcxflg & DBGCXFIND) return;

    tioflush(ctx->dbgcxtio);
    tioshow(ctx->dbgcxtio);
    dbgstktr(ctx, dbgdsdisp, ctx->dbgcxtio, -1, FALSE, TRUE);
    tioflush(ctx->dbgcxtio);
    ctx->dbgcxfcn = ctx->dbgcxdep = 0;
}

/* get information about the currently executing source line */
void dbglget(dbgcxdef *ctx, uchar *buf)
{
    dbglgetlvl(ctx, buf, 0);
}

/* 
 *   Get information about a source line at a particular stack level into
 *   the buffer; leaves out frame info.  level 0 is the currently
 *   executing line, 1 is the first enclosing level, and so on.  
 */
int dbglgetlvl(dbgcxdef *ctx, uchar *buf, int level)
{
    uchar   *linrec;
    uchar   *obj;
    dbgfdef *fr;

    /* make sure the level is valid */
    if (level > ctx->dbgcxfcn - 1)
        return 1;

    /* get the frame at the given level */
    fr = &ctx->dbgcxfrm[ctx->dbgcxfcn - level - 1];

    /* if we're in an intrinsic, go to enclosing frame */
    if (fr->dbgftarg == MCMONINV) --fr;
    
    /* make sure we've encountered an OPCLINE in this frame */
    if (fr->dbgflin == 0)
        return 1;

    /* we need to read from the target object - lock it */
    obj = mcmlck(ctx->dbgcxmem, (mcmon)fr->dbgftarg);

    linrec = obj + fr->dbgflin;
    memcpy(buf, linrec + 3, (size_t)(*linrec - 3));
    
    /* no longer need the target object locked */
    mcmunlck(ctx->dbgcxmem, (mcmon)fr->dbgftarg);

    /* success */
    return 0;
}

/* tell the line source the location of the current line being compiled */
void dbgclin(tokcxdef *tokctx, objnum objn, uint ofs)
{
    uchar buf[4];
    
    /* package the information and send it to the line source */
    oswp2(buf, objn);
    oswp2(buf + 2, ofs);
    lincmpinf(tokctx->tokcxlin, buf);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
