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

#include "glk/tads/tads2/built_in.h"
#include "glk/tads/tads2/appctx.h"
#include "glk/tads/tads2/character_map.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/file_io.h"
#include "glk/tads/tads2/list.h"
#include "glk/tads/tads2/os.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/string_resources.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* yorn - yes or no */
void bifyon(bifcxdef *ctx, int argc)
{
    char     rsp[128];
    char    *p;
    runsdef  val;
    char     yesbuf[64];
    char     nobuf[64];
    re_context  rectx;
    int      match_yes;
    int      match_no;

    bifcntargs(ctx, 0, argc);            /* check for proper argument count */

    /* load the "yes" and "no" reply patterns */
    if (os_get_str_rsc(RESID_YORN_YES, yesbuf, sizeof(yesbuf)))
        strcpy(yesbuf, "[Yy].*");
    if (os_get_str_rsc(RESID_YORN_NO, nobuf, sizeof(nobuf)))
        strcpy(nobuf, "[Nn].*");
    
    /* if we're in HTML mode, switch to input font */
    if (tio_is_html_mode())
        tioputs(ctx->bifcxtio, "<font face='TADS-Input'>");

    /* ensure the prompt is displayed */
    tioflushn(ctx->bifcxtio, 0);

    /* reset count of lines since the last keyboard input */
    tioreset(ctx->bifcxtio);

    /* read a line of text */
    if (tiogets(ctx->bifcxtio, (char *)0, rsp, (int)sizeof(rsp)))
        runsig(ctx->bifcxrun, ERR_RUNQUIT);

    /* if we're in HTML mode, close the input font tag */
    if (tio_is_html_mode())
        tioputs(ctx->bifcxtio, "</font>");

    /* scan off leading spaces */
    for (p = rsp ; t_isspace(*p) ; ++p) ;

    /* set up our regex context */
    re_init(&rectx, ctx->bifcxerr);

    /* check for a "yes" response */
    match_yes = re_compile_and_match(&rectx, yesbuf, strlen(yesbuf),
                                     p, strlen(p));

    /* check for a "no" response */
    match_no = re_compile_and_match(&rectx, nobuf, strlen(nobuf),
                                    p, strlen(p));

    /* check the result */
    if (match_yes == (int)strlen(p))
        val.runsv.runsvnum = 1;
    else if (match_no == (int)strlen(p))
        val.runsv.runsvnum = 0;
    else
        val.runsv.runsvnum = -1;

    /* delete our regex context */
    re_delete(&rectx);

    /* push the result */
    runpush(ctx->bifcxrun, DAT_NUMBER, &val);
}

/* setfuse */
void bifsfs(bifcxdef *ctx, int argc)
{
    objnum    func;
    uint      tm;
    runsdef   val;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    
    bifcntargs(ctx, 3, argc);            /* check for proper argument count */
    func = runpopfn(ctx->bifcxrun);
    tm = runpopnum(ctx->bifcxrun);
    runpop(ctx->bifcxrun, &val);
    
    /* limitation:  don't allow string or list for value */
    if (val.runstyp == DAT_LIST || val.runstyp == DAT_SSTRING)
        runsig(ctx->bifcxrun, ERR_FUSEVAL);
    
    vocsetfd(voc, voc->voccxfus, func, (prpnum)0,
             tm, &val, ERR_MANYFUS);
}

/* remfuse */
void bifrfs(bifcxdef *ctx, int argc)
{
    objnum    func;
    runsdef   val;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    
    bifcntargs(ctx, 2, argc);
    func = runpopfn(ctx->bifcxrun);
    runpop(ctx->bifcxrun, &val);
    vocremfd(voc, voc->voccxfus, func, (prpnum)0,
             &val, ERR_NOFUSE);
}

/* setdaemon */
void bifsdm(bifcxdef *ctx, int argc)
{
    objnum    func;
    runsdef   val;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    
    bifcntargs(ctx, 2, argc);            /* check for proper argument count */
    func = runpopfn(ctx->bifcxrun);
    runpop(ctx->bifcxrun, &val);
    
    /* limitation:  don't allow string or list for value */
    if (val.runstyp == DAT_LIST || val.runstyp == DAT_SSTRING)
        runsig(ctx->bifcxrun, ERR_FUSEVAL);
    
    vocsetfd(voc, voc->voccxdmn, func, (prpnum)0, 0,
             &val, ERR_MANYDMN);
}

/* remdaemon */
void bifrdm(bifcxdef *ctx, int argc)
{
    objnum    func;
    runsdef   val;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    
    bifcntargs(ctx, 2, argc);
    func = runpopfn(ctx->bifcxrun);
    runpop(ctx->bifcxrun, &val);
    vocremfd(voc, voc->voccxdmn, func, (prpnum)0,
             &val, ERR_NODMN);
}

/* incturn */
void bifinc(bifcxdef *ctx, int argc)
{
    int turncnt;
    
    if (argc == 1)
    {
        /* get the number of turns to skip */
        turncnt = runpopnum(ctx->bifcxrun);
        if (turncnt < 1)
            runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "incturn");
    }
    else
    {
        /* no arguments -> increment by one turn */
        bifcntargs(ctx, 0, argc);
        turncnt = 1;
    }

    /* skip the given number of turns */
    vocturn(ctx->bifcxrun->runcxvoc, turncnt, TRUE);
}

/* skipturn */
void bifskt(bifcxdef *ctx, int argc)
{
    int turncnt;

    bifcntargs(ctx, 1, argc);
    turncnt = runpopnum(ctx->bifcxrun);
    if (turncnt < 1)
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "skipturn");
    vocturn(ctx->bifcxrun->runcxvoc, turncnt, FALSE);
}

/* quit */
void bifqui(bifcxdef *ctx, int argc)
{
    /* check for proper argument count */
    bifcntargs(ctx, 0, argc);

    /* flush output buffer, and signal the end of the game */
    tioflush(ctx->bifcxtio);
    errsig(ctx->bifcxerr, ERR_RUNQUIT);
}

/* internal function to convert a TADS string into a C-string */
static void bifcstr(bifcxdef *ctx, char *buf, size_t bufsiz, uchar *str)
{
    size_t  srcrem;
    size_t  dstrem;
    uchar  *src;
    char   *dst;

    /* get the length and text portion of the string */
    srcrem = osrp2(str) - 2;
    str += 2;

    /* scan the string, and convert escapes */
    for (src = str, dst = buf, dstrem = bufsiz ;
         srcrem != 0 && dstrem != 0 ; ++src, --srcrem)
    {
        /* if we have an escape sequence, convert it */
        if (*src == '\\')
        {
            /* skip the backslash in the input */
            ++src;
            --srcrem;

            /* if there's nothing left, store the backslash */
            if (srcrem == 0)
            {
                /* store the backslash */
                *dst++ = '\\';
                --dstrem;

                /* there's nothing left to scan */
                break;
            }

            /* see what the second half of the escape sequence is */
            switch(*src)
            {
            case 'n':
                /* store a C-style newline character */
                *dst++ = '\n';
                --dstrem;
                break;

            case 't':
                /* store a C-style tab */
                *dst++ = '\t';
                --dstrem;
                break;

            case '(':
            case ')':
                /* entirely omit the highlighting sequences */
                break;

            default:
                /* store everything else unchanged */
                *dst++ = *src;
                --dstrem;
                break;
            }
        }
        else
        {
            /* copy this character unchanged */
            *dst++ = *src;
            --dstrem;
        }
    }

    /* if the buffer wasn't big enough, signal an error */
    if (dstrem == 0)
        runsig(ctx->bifcxrun, ERR_BIFCSTR);

    /* null-terminate the result string */
    *dst = '\0';
}

/* save */
void bifsav(bifcxdef *ctx, int argc)
{
    uchar   *fn;
    char     buf[OSFNMAX];
    int      err;
    runsdef  val;
    
    bifcntargs(ctx, 1, argc);
    fn = runpopstr(ctx->bifcxrun);
    bifcstr(ctx, buf, (size_t)sizeof(buf), fn);
    os_defext(buf, ctx->bifcxsavext != 0 ? ctx->bifcxsavext : "sav");
    err = fiosav(ctx->bifcxrun->runcxvoc, buf, ctx->bifcxrun->runcxgamename);
    runpush(ctx->bifcxrun, runclog(err), &val);
}

/* restore */
void bifrso(bifcxdef *ctx, int argc)
{
    uchar    *fn;
    char      buf[OSFNMAX];
    int       err;
    voccxdef *vctx = ctx->bifcxrun->runcxvoc;
    
    bifcntargs(ctx, 1, argc);
    
    /* check for special restore(nil) - restore game given as parameter */
    if (runtostyp(ctx->bifcxrun) == DAT_NIL)
    {
        /* get filename from startup parameter, if any */
        if (!os_paramfile(buf))
        {
            /* no startup parameter */
            runpnum(ctx->bifcxrun, FIORSO_NO_PARAM_FILE);
            return;
        }
    }
    else
    {
        /* get string parameter - it's the filename */
        fn = runpopstr(ctx->bifcxrun);
        bifcstr(ctx, buf, (size_t)sizeof(buf), fn);
        os_defext(buf, ctx->bifcxsavext != 0 ? ctx->bifcxsavext : "sav");
    }

    /* try restoring the file */
    err = fiorso(vctx, buf);

    /* blow away all undo records */
    objulose(vctx->voccxundo);

    /* return the result code from fiorso */
    runpnum(ctx->bifcxrun, err);

    /* note that the rest of the command line is to be ignored */
    vctx->voccxflg |= VOCCXFCLEAR;
}

/* logging */
void biflog(bifcxdef *ctx, int argc)
{
    char   buf[OSFNMAX];
    uchar *str;
    
    bifcntargs(ctx, 1, argc);
    if (runtostyp(ctx->bifcxrun) == DAT_NIL)
    {
        rundisc(ctx->bifcxrun);
        tiologcls(ctx->bifcxtio);
    }
    else
    {
        str = runpopstr(ctx->bifcxrun);
        bifcstr(ctx, buf, (size_t)sizeof(buf), str);
        tiologopn(ctx->bifcxtio, buf);
    }
}

/* restart */
void bifres(bifcxdef *ctx, int argc)
{
    voccxdef *vctx = ctx->bifcxrun->runcxvoc;
    objnum    fn;

    if (argc == 2)
        fn = runpopfn(ctx->bifcxrun);            /* get function if present */
    else
    {
        bifcntargs(ctx, 0, argc);        /* check for proper argument count */
        fn = MCMONINV;                         /* no function was specified */
    }

    objulose(vctx->voccxundo);                /* blow away all undo records */
    vocrevert(vctx);                /* revert all objects to original state */
    vocdmnclr(vctx);                   /* clear out fuses/deamons/notifiers */

    /* restore the original "Me" object */
    vctx->voccxme = vctx->voccxme_init;

    /* call preinit if necessary (call it before invoking the user callback) */
    if (vctx->voccxpreinit != MCMONINV)
        runfn(ctx->bifcxrun, vctx->voccxpreinit, 0);

    /*
     *   If a restart function was provided, call it.  Note that we left
     *   the argument for the function on the stack, so there's no need to
     *   re-push it!  
     */
    if (fn != MCMONINV) runfn(ctx->bifcxrun, fn, 1);

    /* restart the game */
    errsig(ctx->bifcxerr, ERR_RUNRESTART);
}

/* input - get a line of input from the keyboard */
void bifinp(bifcxdef *ctx, int argc)
{
    char inbuf[128];

    /* check for proper argument count */
    bifcntargs(ctx, 0, argc);

    /* make sure the prompt is displayed */
    tioflushn(ctx->bifcxtio, 0);

     /* reset count of lines since the last keyboard input */
    tioreset(ctx->bifcxtio);

    /* read a line of text */
    if (tiogets(ctx->bifcxtio, (char *)0, inbuf, (int)sizeof(inbuf)))
        runsig(ctx->bifcxrun, ERR_RUNQUIT);

    /* push the string, converting escapes */
    runpushcstr(ctx->bifcxrun, inbuf, strlen(inbuf), 0);
}

/* notify */
void bifnfy(bifcxdef *ctx, int argc)
{
    objnum    objn;
    prpnum    prp;
    uint      tm;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    
    bifcntargs(ctx, 3, argc);            /* check for proper argument count */
    objn = runpopobj(ctx->bifcxrun);
    prp = runpopprp(ctx->bifcxrun);
    tm = runpopnum(ctx->bifcxrun);

    /* a time of zero means every turn */
    if (tm == 0)
        tm = VOCDTIM_EACH_TURN;
    
    vocsetfd(voc, voc->voccxalm, objn, prp, tm,
             (runsdef *)0, ERR_MANYNFY);
}


/* unnotify */
void bifunn(bifcxdef *ctx, int argc)
{
    objnum    objn;
    prpnum    prop;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    
    bifcntargs(ctx, 2, argc);
    objn = runpopobj(ctx->bifcxrun);
    prop = runpopprp(ctx->bifcxrun);
    vocremfd(voc, voc->voccxalm, objn, prop,
             (runsdef *)0, ERR_NONFY);
}

/* trace on/off */
void biftrc(bifcxdef *ctx, int argc)
{
    runsdef val;
    int     n;
    int     flag;

    if (argc == 2)
    {
        /* get the type indicator and the on/off status */
        n = runpopnum(ctx->bifcxrun);
        flag = runpoplog(ctx->bifcxrun);

        /* see what type of debugging they want to turn on or off */
        switch(n)
        {
        case 1:
            /* turn on parser tracing */
            if (flag)
                ctx->bifcxrun->runcxvoc->voccxflg |= VOCCXFDBG;
            else
                ctx->bifcxrun->runcxvoc->voccxflg &= ~VOCCXFDBG;
            break;

        default:
            /* ignore other requests */
            runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "debugTrace");
        }
    }
    else
    {
        /* break into debugger; return whether debugger is present */
        bifcntargs(ctx, 0, argc);
        runpush(ctx->bifcxrun, runclog(dbgstart(ctx->bifcxrun->runcxdbg)),
                &val);
    }
}

/* say */
void bifsay(bifcxdef *ctx, int argc)
{
    uchar *str;
    long   num;
    char   numbuf[30];

    if (argc != 2) bifcntargs(ctx, 1, argc);
    
    switch(runtostyp(ctx->bifcxrun))
    {
    case DAT_NUMBER:
        num = runpopnum(ctx->bifcxrun);
        sprintf(numbuf, "%ld", num);
        tioputs(ctx->bifcxtio, numbuf);
        break;
        
    case DAT_SSTRING:
        str = runpopstr(ctx->bifcxrun);
        outfmt(ctx->bifcxtio, str);
        break;
        
    case DAT_NIL:
        (void)runpoplog(ctx->bifcxrun);
        break;
        
    default:
        runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "say");
    }
}

/* car */
void bifcar(bifcxdef *ctx, int argc)
{
    uchar   *lstp;
    uint     lstsiz;
    runsdef  val;
    
    bifcntargs(ctx, 1, argc);
    bifchkarg(ctx, DAT_LIST);
    
    lstp = runpoplst(ctx->bifcxrun);
    
    /* get list's size, and point to its data string */
    lstsiz = osrp2(lstp) - 2;
    lstp += 2;
    
    /* push first element if one is present, otherwise push nil */
    if (lstsiz)
        runpbuf(ctx->bifcxrun, *lstp, lstp+1);
    else
        runpush(ctx->bifcxrun, DAT_NIL, &val);
}

/* cdr */
void bifcdr(bifcxdef *ctx, int argc)
{
    uchar   *lstp;
    uint     siz;
    uint     lstsiz;
    runsdef  val;
    runsdef  stkval;
    
    bifcntargs(ctx, 1, argc);
    bifchkarg(ctx, DAT_LIST);
    
    lstp = runpoplst(ctx->bifcxrun);
    stkval.runstyp = DAT_LIST;
    stkval.runsv.runsvstr = lstp;
    
    /* get list's size, and point to its data string */
    lstsiz = osrp2(lstp) - 2;
    lstp += 2;
    
    if (lstsiz != 0)
    {
        /* deduct size of first element from size of list */
        siz = datsiz(*lstp, lstp+1) + 1;
        lstsiz -= siz;

        /* add in the size prefix for our new list size */
        lstsiz += 2;

        /* allocate space for new list containing rest of list */
        runhres1(ctx->bifcxrun, lstsiz, 1, &stkval);
        lstp = stkval.runsv.runsvstr + siz + 2;

        /* write out size followed by list value string */
        oswp2(ctx->bifcxrun->runcxhp, lstsiz);
        memcpy(ctx->bifcxrun->runcxhp+2, lstp, (size_t)(lstsiz-2));
        
        val.runsv.runsvstr = ctx->bifcxrun->runcxhp;
        val.runstyp = DAT_LIST;
        ctx->bifcxrun->runcxhp += lstsiz;
        runrepush(ctx->bifcxrun, &val);
    }
    else
        runpush(ctx->bifcxrun, DAT_NIL, &val);   /* empty list - cdr is nil */
}

/* caps */
void bifcap(bifcxdef *ctx, int argc)
{
    bifcntargs(ctx, 0, argc);
    tiocaps(ctx->bifxtio);  /* set output driver next-char-capitalized flag */
}

/* nocaps */
void bifnoc(bifcxdef *ctx, int argc)
{
    bifcntargs(ctx, 0, argc);
    tionocaps(ctx->bifxtio);               /* set next-not-capitalized flag */
}

/* length */
void biflen(bifcxdef *ctx, int argc)
{
    uchar   *p;
    runsdef  val;
    long     len = 0;
    int      l;
    
    bifcntargs(ctx, 1, argc);
    switch(runtostyp(ctx->bifcxrun))
    {
    case DAT_SSTRING:
        p = (uchar *)runpopstr(ctx->bifcxrun);
        len = osrp2(p) - 2;
        break;

    case DAT_LIST:
        p = runpoplst(ctx->bifcxrun);
        l = osrp2(p) - 2;
        p += 2;
        
        /* count all elements in list */
        for (len = 0 ; l ; ++len)
        {
            int cursiz;
            
            /* get size of this element, and move past it */
            cursiz = datsiz(*p, p+1) + 1;
            l -= cursiz;
            p += cursiz;
        }
        break;

    default:
        runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "length");
    }
    
    val.runsv.runsvnum = len;
    runpush(ctx->bifcxrun, DAT_NUMBER, &val);
}

/* find */
void biffnd(bifcxdef *ctx, int argc)
{
    uchar   *p1, *p2;
    int      len1, len2;
	int      outv = 0;
    runsdef  val;
    int      typ = 0;
    int      siz;
    
    bifcntargs(ctx, 2, argc);
    switch(runtostyp(ctx->bifcxrun))
    {
    case DAT_SSTRING:
        p1 = runpopstr(ctx->bifcxrun);
        len1 = osrp2(p1) - 2;
        p1 += 2;
        
        p2 = runpopstr(ctx->bifcxrun);
        len2 = osrp2(p2) - 2;
        p2 += 2;

        /* look for p2 within p1 */
        for (typ = DAT_NIL, outv = 1 ; len1 >= len2 ; ++p1, --len1, ++outv)
        {
            if (!memcmp(p1, p2, (size_t)len2))
            {
                typ = DAT_NUMBER;           /* use number in outv after all */
                break;                        /* that's it - we've found it */
            }
        }
        break;
        
    case DAT_LIST:
        p1 = runpoplst(ctx->bifcxrun);
        len1 = osrp2(p1) - 2;
        p1 += 2;

        /* get second item:  any old datatype */
        runpop(ctx->bifcxrun, &val);
        
        for (typ = DAT_NIL, outv = 1 ; len1 ; ++outv, p1 += siz, len1 -= siz)
        {
            siz = datsiz(*p1, p1 + 1) + 1;      /* get size of this element */
            if (val.runstyp != *p1) continue;          /* types don't match */
            
            switch(val.runstyp)
            {
            case DAT_NUMBER:
                if (val.runsv.runsvnum != osrp4s(p1 + 1)) continue;
                break;
                
            case DAT_SSTRING:
            case DAT_LIST:
                if (osrp2(p1 + 1) != osrp2(val.runsv.runsvstr) ||
                    memcmp(p1 + 3, val.runsv.runsvstr + 2,
                           (size_t)(osrp2(p1 + 1) - 2)))
                    continue;
                break;
                
            case DAT_PROPNUM:
                if (osrp2(p1 + 1) != val.runsv.runsvprp) continue;
                break;
                
            case DAT_OBJECT:
            case DAT_FNADDR:
                if (osrp2(p1 + 1) != val.runsv.runsvobj) continue;
                break;
                
            default:
                break;
            }
            
            /* if we got here, it means we found a match */
            typ = DAT_NUMBER;                      /* use the value in outv */
            break;                            /* that's it - we've found it */
        }
        break;
        
    default:
        runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "find");
    }
    
    /* push the value given by typ and outv */
    val.runsv.runsvnum = outv;
    runpush(ctx->bifcxrun, typ, &val);
}

/* setit - set current 'it' */
void bifsit(bifcxdef *ctx, int argc)
{
    objnum    obj;
    int       typ;
    voccxdef *vcx = ctx->bifcxrun->runcxvoc;
    
    /* check for extended version that allows setting him/her */
    if (argc == 2)
    {
        if (runtostyp(ctx->bifcxrun) == DAT_NIL)
        {
            rundisc(ctx->bifcxrun);                      /* discard the nil */
            obj = MCMONINV;                           /* use invalid object */
        }
        else
            obj = runpopobj(ctx->bifcxrun);               /* get the object */

        typ = runpopnum(ctx->bifcxrun);                     /* get the code */
        vcx->voccxthc = 0;                         /* clear the 'them' list */

        switch(typ)
        {
        case 0:                                                 /* set "it" */
            vcx->voccxit = obj;
            break;

        case 1:                                                /* set "him" */
            vcx->voccxhim = obj;
            break;

        case 2:                                                /* set "her" */
            vcx->voccxher = obj;
            break;
        }
        return;
    }

    /* "setit classic" has one argument only */
    bifcntargs(ctx, 1, argc);

    /* check to see if we're setting 'it' or 'them' */
    if (runtostyp(ctx->bifcxrun) == DAT_LIST)
    {
        uchar *lst;
        uint   siz;
        int    cnt;

        lst = runpoplst(ctx->bifcxrun);
        siz = osrp2(lst);
        lst += 2;
        siz -= 2;

        for (cnt = 0 ; siz ; )
        {
            /* if this is an object, add to 'them' list (otherwise ignore) */
            if (*lst == DAT_OBJECT)
                vcx->voccxthm[cnt++] = osrp2(lst+1);

            lstadv(&lst, &siz);
        }
        vcx->voccxthc = cnt;
        vcx->voccxit = MCMONINV;
    }
    else
    {
        /* set 'it', and delete 'them' list */
        if (runtostyp(ctx->bifcxrun) == DAT_NIL)
        {
            vcx->voccxit = MCMONINV;
            rundisc(ctx->bifcxrun);
        }
        else
            vcx->voccxit = runpopobj(ctx->bifcxrun);
        vcx->voccxthc = 0;
    }
}

/* randomize - seed random number generator */
void bifsrn(bifcxdef *ctx, int argc)
{
    bifcntargs(ctx, 0, argc);
    os_rand(&ctx->bifcxrnd);
    ctx->bifcxrndset = TRUE;
}

/* rand - get a random number */
void bifrnd(bifcxdef *ctx, int argc)
{
    unsigned long result, max, randseed;
    int      tmp;
    runsdef  val;

    /* get argument - number giving upper bound of generated number */
    bifcntargs(ctx, 1, argc);
    bifchkarg(ctx, DAT_NUMBER);
    max = runpopnum(ctx->bifcxrun);

    /* if the max is zero, just return zero */
    if (max == 0)
    {
        runpnum(ctx->bifcxrun, 0);
        return;
    }

    /*
     *   If the random number generator has been seeded by a call to
     *   randomize(), use the new, improved random number generator.  If
     *   not, use the old random number generator to ensure that the same
     *   sequence of numbers is generated as always (to prevent breaking
     *   existing test scripts based on the old sequence). 
     */
    if (!ctx->bifcxrndset)
    {
        /* compute the next number in sequence, using old cheesy generator */
        randseed = ctx->bifcxrnd;
        randseed *= 1033;
        randseed += 5;
        tmp = randseed / 16384;
        randseed %= 16384;
        result = tmp / 7;

        /* adjust the result to be in the requested range */
        result = ( randseed % max ) + 1;
        
        /* save the new seed value, and return the value */
        ctx->bifcxrnd = randseed;
        val.runsv.runsvnum = result;
        runpush(ctx->bifcxrun, DAT_NUMBER, &val);
    }
    else
    {
#define BIF_RAND_M  ((ulong)2147483647)
#define BIF_RAND_Q  ((ulong)127773)
#define BIF_RAND_A  ((ulong)16807)
#define BIF_RAND_R  ((ulong)2836)

        long lo, hi, test;

        lo = ctx->bifcxrnd / BIF_RAND_Q;
        hi = ctx->bifcxrnd % BIF_RAND_Q;
        test = BIF_RAND_A*lo - BIF_RAND_R*hi;
        ctx->bifcxrnd = test;
        if (test > 0)
            ctx->bifcxrnd = test;
        else
            ctx->bifcxrnd = test + BIF_RAND_M;
        runpnum(ctx->bifcxrun, (((ulong)ctx->bifcxrnd) % max) + 1);
    }
}

/* 
 *   case-insensitive substring matching 
 */
static const char *bif_stristr(const char *s1, const char *s2)
{
    size_t s1len;
    size_t s2len;

    /* scan for a match */
    for (s1len = strlen(s1), s2len = strlen(s2) ; s1len >= s2len ;
         ++s1, --s1len)
    {
        /* if this is a match, return this substring */
        if (memicmp(s1, s2, s2len) == 0)
            return (const char *)s1;
    }

    return 0;
}

/*
 *   askfile flags 
 */
#define BIF_ASKF_EXT_RET  1                        /* extended return codes */

/* 
 *   askfile 
 */
void bifask(bifcxdef *ctx, int argc)
{
    uchar *prompt;
    char   buf[OSFNMAX + 2];
    char   pbuf[128];
    int    err;
    int    prompt_type;
    int    file_type;
    ulong  flags;

    /* make sure we have an acceptable number of arguments */
    if (argc != 1 && argc != 3 && argc != 4)
        runsig(ctx->bifcxrun, ERR_BIFARGC);

    /* get the first argument - the prompt string */
    prompt = runpopstr(ctx->bifcxrun);
    bifcstr(ctx, pbuf, (size_t)sizeof(pbuf), prompt);

    /* presume we will have no flags */
    flags = 0;

    /* if we have the prompt type and file type parameters, get them */
    if (argc >= 3)
    {
        /* get the prompt-type and the file-type arguments */
        prompt_type = (int)runpopnum(ctx->bifcxrun);
        file_type = (int)runpopnum(ctx->bifcxrun);

        /* if we have a fourth argument, it's the flags */
        if (argc == 4)
            flags = runpopnum(ctx->bifcxrun);
    }
    else
    {
        static const char *save_strs[] =
        {
            "save",
            "write",
            0
        };
        static const char *game_strs[] =
        {
            "restore",
            "game",
            0
        };
        const char **sp;

        /* 
         *   No prompt type or file type were specified.  Try to infer the
         *   dialog type and file type from the text of the prompt.  (This
         *   is mostly to support older games, in particular those based
         *   on older versions of adv.t, since newer games should always
         *   provide explicit values for the file type and dialog type.
         *   We are thus inferring the types based on the prompt strings
         *   that older adv.t's used when calling askfile.)
         *   
         *   If the prompt contains any substring such as "save" or
         *   "write", specify that we're saving; otherwise, assume that
         *   we're opening an existing file for reading.
         *   
         *   If the prompt contains the substrings "restore" AND "game",
         *   assume that we're opening a game file; otherwise, don't make
         *   any assumptions, and use the "unknown" file type.  
         */

        /* presume we're going to open a saved-game file */
        prompt_type = OS_AFP_OPEN;
        file_type = OSFTSAVE;

        /* look for any one of the "save" substrings */
        for (sp = save_strs ; *sp != 0 ; ++sp)
        {
            /* check to see if this substring matches */
            if (bif_stristr(pbuf, *sp))
            {
                /* found it - use the "save" prompt */
                prompt_type = OS_AFP_SAVE;

                /* no need to look any further */
                break;
            }
        }

        /* 
         *   look for *all* of the "restore game" strings - if we fail to
         *   find any of them, be conservative and make no assumptions
         *   about the file type 
         */
        for (sp = game_strs ; *sp != 0 ; ++sp)
        {
            if (bif_stristr(pbuf, *sp) == 0)
            {
                /* 
                 *   this one doesn't match - don't make assumptions about
                 *   the file type 
                 */
                file_type = OSFTUNK;

                /* no need to look any further */
                break;
            }
        }

        /* check for a transcript */
        if (file_type == OSFTUNK
            && prompt_type == OS_AFP_SAVE
            && bif_stristr(pbuf, "script") != 0)
        {
            /* looks like a log file */
            file_type = OSFTLOG;
        }
    }

    /* ask for a file */
    err = tio_askfile(pbuf, buf, (int)sizeof(buf), prompt_type, (os_filetype_t)file_type);

    /* 
     *   if the caller requested extended return codes, return a list
     *   containing the return code as the first element and, if
     *   successful, the string as the second element 
     */
    if ((flags & BIF_ASKF_EXT_RET) != 0)
    {
        ushort len;
        runsdef val;
        uchar *p;
        
        /* 
         *   Allocate space for the starter list - if we have a string to
         *   return, just allocate space for the number element for now;
         *   otherwise, allocate space for the number plus a nil second
         *   element (one byte).
         */
        len = 2 + (1 + 4);
        if (err != OS_AFE_SUCCESS)
            ++len;

        /* allocate the space */
        runhres(ctx->bifcxrun, len, 0);

        /* set up our list pointer */
        val.runstyp = DAT_LIST;
        val.runsv.runsvstr = p = ctx->bifcxrun->runcxhp;

        /* write the length prefix */
        oswp2(p, len);
        p += 2;

        /* write the return code as the first element */
        *p++ = DAT_NUMBER;
        oswp4s(p, err);
        p += 4;

        /* write the 'nil' second element if there's an error */
        if (err != OS_AFE_SUCCESS)
            *p++ = DAT_NIL;

        /* commit the list's memory */
        ctx->bifcxrun->runcxhp = p;

        /* push the list */
        runrepush(ctx->bifcxrun, &val);

        /* if we were successful, add the string to the list */
        if (err == OS_AFE_SUCCESS)
        {
            runsdef val2;
            
            /* push the string value, converting to our string format */
            runpushcstr(ctx->bifcxrun, buf, strlen(buf), 1);

            /* add it to the list already on the stack */
            runpop(ctx->bifcxrun, &val2);
            runpop(ctx->bifcxrun, &val);
            runadd(ctx->bifcxrun, &val, &val2, 2);

            /* re-push the result */
            runrepush(ctx->bifcxrun, &val);
        }
    }
    else
    {
        /* 
         *   use the traditional return codes - if askfile failed, return
         *   nil; otherwise, return the filename 
         */
        if (err)
            runpnil(ctx->bifcxrun);
        else
            runpushcstr(ctx->bifcxrun, buf, strlen(buf), 0);
    }
}

/* setscore */
void bifssc(bifcxdef *ctx, int argc)
{
    int s1, s2;

    /* optional new way - string argument */
    if (argc == 1 && runtostyp(ctx->bifcxrun) == DAT_SSTRING)
    {
        char   buf[80];
        uchar *p;
        
        p = runpopstr(ctx->bifcxrun);
        bifcstr(ctx, buf, (size_t)sizeof(buf), p);
        tiostrsc(ctx->bifcxtio, buf);
    }
    else
    {
        /* old way - two numeric arguments (displays: x/y) */
        bifcntargs(ctx, 2, argc);
        s1 = runpopnum(ctx->bifcxrun);
        s2 = runpopnum(ctx->bifcxrun);
        tioscore(ctx->bifcxtio, s1, s2);
    }
}

/* substr */
void bifsub(bifcxdef *ctx, int argc)
{
    uchar   *p;
    int      ofs;
    int      asklen;
    int      outlen;
    int      len;

    bifcntargs(ctx, 3, argc);

    /* get the string argument */
    bifchkarg(ctx, DAT_SSTRING);
    p = runpopstr(ctx->bifcxrun);
    len = osrp2(p) - 2;
    p += 2;
    
    /* get the offset argument */
    bifchkarg(ctx, DAT_NUMBER);
    ofs = runpopnum(ctx->bifcxrun);
    if (ofs < 1) runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "substr");
    
    /* get the length argument */
    bifchkarg(ctx, DAT_NUMBER);
    asklen = runpopnum(ctx->bifcxrun);
    if (asklen < 0) runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "substr");

    --ofs;          /* convert offset to a zero bias (user provided 1-bias) */
    p += ofs;                           /* advance string pointer by offset */

    if (ofs >= len)
        outlen = 0;                         /* offset is past end of string */
    else if (asklen > len - ofs)
        outlen = len - ofs;                      /* just use rest of string */
    else
        outlen = asklen;                /* requested length can be provided */
    
    runpstr(ctx->bifcxrun, (char *)p, outlen, 3);
}

/* cvtstr - convert value to a string */
void bifcvs(bifcxdef *ctx, int argc)
{
    const char *p = nullptr;
    int   len = 0;
    char  buf[30];
    
    bifcntargs(ctx, 1, argc);
    switch(runtostyp(ctx->bifcxrun))
    {
    case DAT_NIL:
        p = "nil";
        len = 3;
        (void)runpoplog(ctx->bifcxrun);
        break;
        
    case DAT_TRUE:
        p = "true";
        len = 4;
        (void)runpoplog(ctx->bifcxrun);
        break;
        
    case DAT_NUMBER:
        sprintf(buf, "%ld", runpopnum(ctx->bifcxrun));
        p = buf;
        len = strlen(buf);
        break;

    case DAT_SSTRING:
        /* leave the string value on the stack unchanged */
        return;

    default:
        /* throw the RUNEXITOBJ error */
        runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "cvtstr");
    }
    
    runpstr(ctx->bifcxrun, p, len, 0);
}

/* cvtnum  - convert a value to a number */
void bifcvn(bifcxdef *ctx, int argc)
{
    runsdef  val;
    uchar   *p;
    int      len;
    int      typ;
    long     acc;
    int      neg;
    
    bifcntargs(ctx, 1, argc);
    p = runpopstr(ctx->bifcxrun);
    len = osrp2(p) - 2;
    p += 2;
    
    if (len == 3 && !memcmp(p, "nil", (size_t)3))
        typ = DAT_NIL;
    else if (len == 4 && !memcmp(p, "true", (size_t)4))
        typ = DAT_TRUE;
    else
    {
        typ = DAT_NUMBER;
        for ( ; len != 0 && t_isspace(*p) ; ++p, --len) ;
        if (len != 0 && *p == '-')
        {
            neg = TRUE;
            for (++p, --len ; len != 0 && t_isspace(*p) ; ++p, --len) ;
        }
        else neg = FALSE;

        /* accumulate the number digit by digit */
        for (acc = 0 ; len != 0 && Common::isDigit(*p) ; ++p, --len)
            acc = (acc << 3) + (acc << 1) + ((*p) - '0');

        if (neg) acc = -acc;
        val.runsv.runsvnum = acc;
    }
    
    runpush(ctx->bifcxrun, typ, &val);
}

/* general string conversion function */
static void bifcvtstr(bifcxdef *ctx, void (*cvtfn)(uchar *, int), int argc)
{
    uchar   *p;
    int      len;
    runsdef  val;
    runsdef  stkval;
    
    bifcntargs(ctx, 1, argc);
    bifchkarg(ctx, DAT_SSTRING);
    
    p = runpopstr(ctx->bifcxrun);
    stkval.runstyp = DAT_SSTRING;
    stkval.runsv.runsvstr = p;
    len = osrp2(p);
    
    /* allocate space in heap for the string and convert */
    runhres1(ctx->bifcxrun, len, 1, &stkval);
    p = stkval.runsv.runsvstr;
    memcpy(ctx->bifcxrun->runcxhp, p, (size_t)len);
    (*cvtfn)(ctx->bifcxrun->runcxhp + 2, len - 2);
    
    val.runsv.runsvstr = ctx->bifcxrun->runcxhp;
    val.runstyp = DAT_SSTRING;
    ctx->bifcxrun->runcxhp += len;
    runrepush(ctx->bifcxrun, &val);
}

/* routine to convert a counted-length string to uppercase */
static void bifstrupr(uchar *str, int len)
{
    for ( ; len ; --len, ++str)
    {
        if (*str == '\\' && len > 1)
            --len, ++str;
        else if (Common::isLower(*str))
            *str = toupper(*str);
    }
}

/* upper */
void bifupr(bifcxdef *ctx, int argc)
{
    bifcvtstr(ctx, bifstrupr, argc);
}

/* convert a counted-length string to lowercase */
static void bifstrlwr(uchar *str, int len)
{
    for ( ; len ; --len, ++str)
    {
        if (*str == '\\' && len > 1)
            --len, ++str;
        else if (Common::isUpper(*str))
            *str = tolower(*str);
    }
}

/* lower */
void biflwr(bifcxdef *ctx, int argc)
{
    bifcvtstr(ctx, bifstrlwr, argc);
}

/* internal check to determine if object is of a class */
int bifinh(voccxdef *voc, vocidef *v, objnum cls)
{
    int     i;
    objnum *sc;

    if (!v) return(FALSE);
    for (i = v->vocinsc, sc = v->vocisc ; i ; ++sc, --i)
    {
        if (*sc == cls
            || bifinh(voc, vocinh(voc, *sc), cls))
            return(TRUE);
    }
    return(FALSE);
}

/* isclass(obj, cls) */
void bifisc(bifcxdef *ctx, int argc)
{
    objnum    obj;
    objnum    cls;
    runsdef   val;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    
    bifcntargs(ctx, 2, argc);

    /* if checking for nil, return nil */
    if (runtostyp(ctx->bifcxrun) == DAT_NIL)
    {
        rundisc(ctx->bifcxrun);
        rundisc(ctx->bifcxrun);
        runpnil(ctx->bifcxrun);
        return;
    }

    /* get the arguments:  object, class */
    obj = runpopobj(ctx->bifcxrun);
    cls = runpopobj(ctx->bifcxrun);

    /* return the result from bifinh() */
    runpush(ctx->bifcxrun, runclog(bifinh(voc, vocinh(voc, obj), cls)), &val);
}

/* firstsc(obj) - get the first superclass of an object */
void bif1sc(bifcxdef *ctx, int argc)
{
    objnum obj;
    objnum sc;

    bifcntargs(ctx, 1, argc);
    obj = runpopobj(ctx->bifcxrun);
    sc = objget1sc(ctx->bifcxrun->runcxmem, obj);
    runpobj(ctx->bifcxrun, sc);
}

/* firstobj */
void biffob(bifcxdef *ctx, int argc)
{
    vocidef ***vpg;
    vocidef  **v;
    objnum     obj;
    int        i;
    int        j;
    objnum     cls = 0;
    voccxdef  *voc = ctx->bifcxrun->runcxvoc;

    /* get class to search for, if one is specified */
    if (argc == 0)
        cls = MCMONINV;
    else if (argc == 1)
        cls = runpopobj(ctx->bifcxrun);
    else
        runsig(ctx->bifcxrun, ERR_BIFARGC);
    
    for (vpg = voc->voccxinh, i = 0 ; i < VOCINHMAX ; ++vpg, ++i)
    {
        if (!*vpg) continue;
        for (v = *vpg, obj = (i << 8), j = 0 ; j < 256 ; ++v, ++obj, ++j)
        {
            if (!*v || ((*v)->vociflg & VOCIFCLASS)
                || (cls != MCMONINV && !bifinh(voc, *v, cls)))
                continue;
            
            /* this is an object we can use - push it */
            runpobj(ctx->bifcxrun, obj);
            return;
        }
    }
    
    /* no objects found at all - return nil */
    runpnil(ctx->bifcxrun);
}

/* nextobj */
void bifnob(bifcxdef *ctx, int argc)
{
    objnum     prv;
    vocidef ***vpg;
    vocidef  **v;
    objnum     obj;
    int        i;
    int        j;
    objnum     cls = 0;
    voccxdef  *voc = ctx->bifcxrun->runcxvoc;

    /* get last position in search */
    prv = runpopobj(ctx->bifcxrun);
    
    /* get class to search for, if one is specified */
    if (argc == 1)
        cls = MCMONINV;
    else if (argc == 2)
        cls = runpopobj(ctx->bifcxrun);
    else
        runsig(ctx->bifcxrun, ERR_BIFARGC);
    
    /* start at previous object plus 1 */
    i = (prv >> 8);
    vpg = voc->voccxinh + i;
    j = (prv & 255);
    obj = prv;
    v = (*vpg) + j;
    
    for (;;)
    {
        ++j;
        ++obj;
        ++v;
        if (j == 256)
        {
            j = 0;
            ++i;
            ++vpg;
            if (!*vpg)
            {
                obj += 255;
                j += 255;
                continue;
            }
            v = (*vpg);
        }
        if (i >= VOCINHMAX)
        {
            runpnil(ctx->bifcxrun);
            return;
        }
        
        if (!*v || ((*v)->vociflg & VOCIFCLASS)
            || (cls != MCMONINV && !bifinh(voc, *v, cls)))
            continue;
            
        /* this is an object we can use - push it */
        runpobj(ctx->bifcxrun, obj);
        return;
    }
}

/* setversion */
void bifsvn(bifcxdef *ctx, int argc)
{
    bifcntargs(ctx, 1, argc);
    (void)runpopstr(ctx->bifcxrun);
    /* note - setversion doesn't do anything in v2; uses timestamp instead */
}

/* getarg */
void bifarg(bifcxdef *ctx, int argc)
{
    int argnum;
    
    bifcntargs(ctx, 1, argc);
    bifchkarg(ctx, DAT_NUMBER);
    
    /* get and verify argument number */
    argnum = runpopnum(ctx->bifcxrun);
    if (argnum < 1) runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "getarg");

    runrepush(ctx->bifcxrun, ctx->bifcxrun->runcxbp - argnum - 1);
}

/* datatype */
void biftyp(bifcxdef *ctx, int argc)
{
    runsdef val;
    
    bifcntargs(ctx, 1, argc);
    
    /* get whatever it is, and push the type */
    runpop(ctx->bifcxrun, &val);
    val.runsv.runsvnum = val.runstyp;          /* new value is the datatype */
    runpush(ctx->bifcxrun, DAT_NUMBER, &val);
}

/* undo */
void bifund(bifcxdef *ctx, int argc)
{
    objucxdef *ucx = ctx->bifcxrun->runcxvoc->voccxundo;
    mcmcxdef  *mcx = ctx->bifcxrun->runcxmem;
    errcxdef  *ec  = ctx->bifcxerr;
    int        err;
    int        undone;
    runsdef    val;

    bifcntargs(ctx, 0, argc);                               /* no arguments */

    ERRBEGIN(ec)
        if (ucx)
        {
            objundo(mcx, ucx);         /* try to undo to previous savepoint */
            undone = TRUE;                       /* looks like we succeeded */
        }
        else
            undone = FALSE;                  /* no undo context; can't undo */
    ERRCATCH(ec, err)
        if (err == ERR_NOUNDO || err == ERR_ICUNDO)
            undone = FALSE;
        else
            errrse(ec);            /* don't know how to handle other errors */
    ERREND(ec)

    /* return a value indicating whether the undo operation succeeded */
    runpush(ctx->bifcxrun, runclog(undone), &val);

    /* note that the rest of the command line is to be ignored */
    ctx->bifcxrun->runcxvoc->voccxflg |= VOCCXFCLEAR;
}

/* flags for defined() function */
#define BIFDEF_DEFINED_ANY           1
#define BIFDEF_DEFINED_DIRECTLY      2
#define BIFDEF_DEFINED_INHERITS      3
#define BIFDEF_DEFINED_GET_CLASS     4

/* defined */
void bifdef(bifcxdef *ctx, int argc)
{
    prpnum  prpn;
    objnum  objn;
    uint    ofs;
    runsdef val;
    objnum  def_objn;
    int     flag;

    /* get object and property arguments */
    objn = runpopobj(ctx->bifcxrun);
    prpn = runpopprp(ctx->bifcxrun);

    /* if there's a flag argument, get it as well */
    if (argc == 3)
    {
        /* get the flag */
        flag = (int)runpopnum(ctx->bifcxrun);
    }
    else
    {
        /* check the argument count */
        bifcntargs(ctx, 2, argc);

        /* use the default flag value (DEFINES_OR_INHERITS) */
        flag = BIFDEF_DEFINED_ANY;
    }

    /* get the offset of the property and the defining object */
    ofs = objgetap(ctx->bifcxrun->runcxmem, objn, prpn, &def_objn, FALSE);

    /* determine the type of information they want */
    switch(flag)
    {
    case BIFDEF_DEFINED_ANY:
        /* if the property is defined, return true, else return nil */
        runpush(ctx->bifcxrun, runclog(ofs != 0), &val);
        break;

    case BIFDEF_DEFINED_DIRECTLY:
        /* if the property is defined directly by the object, return true */
        runpush(ctx->bifcxrun, runclog(ofs != 0 && def_objn == objn), &val);
        break;
        
    case BIFDEF_DEFINED_INHERITS:
        /* if the property is inherited, return true */
        runpush(ctx->bifcxrun, runclog(ofs != 0 && def_objn != objn), &val);
        break;

    case BIFDEF_DEFINED_GET_CLASS:
        /* if it's defined, return the defining object, otherwise nil */
        if (ofs == 0)
            runpnil(ctx->bifcxrun);
        else
            runpobj(ctx->bifcxrun, def_objn);
        break;

    default:
        /* invalid flag value */
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "defined");
    }
}

/* proptype */
void bifpty(bifcxdef *ctx, int argc)
{
    prpnum   prpn;
    objnum   objn;
    uint     ofs;
    runsdef  val;
    objnum   orn;
    objdef  *objptr;
    prpdef  *propptr;

    bifcntargs(ctx, 2, argc);
    
    /* get offset of obj.prop */
    objn = runpopobj(ctx->bifcxrun);
    prpn = runpopprp(ctx->bifcxrun);
    ofs = objgetap(ctx->bifcxrun->runcxmem, objn, prpn, &orn, FALSE);
    
    if (ofs)
    {
        /* lock the object, read the prpdef, and unlock it */
        objptr = (objdef *)mcmlck(ctx->bifcxrun->runcxmem, (mcmon)orn);
        propptr = objofsp(objptr, ofs);
        val.runsv.runsvnum = prptype(propptr);
        mcmunlck(ctx->bifcxrun->runcxmem, (mcmon)orn);
    }
    else
    {
        /* property is not defined by object - indicate that type is nil */
        val.runsv.runsvnum = DAT_NIL;
    }
    
    /* special case:  DAT_DEMAND -> DAT_LIST (for contents properties) */
    if (val.runsv.runsvnum == DAT_DEMAND)
        val.runsv.runsvnum = DAT_LIST;

    /* return the property type as a number */
    runpush(ctx->bifcxrun, DAT_NUMBER, &val);
}

/* outhide */
void bifoph(bifcxdef *ctx, int argc)
{
    runsdef val;
    int     hidden, output_occurred;

    bifcntargs(ctx, 1, argc);
    outstat(&hidden, &output_occurred);
    if (runtostyp(ctx->bifcxrun) == DAT_TRUE)
    {
        /* throw away the flag */
        rundisc(ctx->bifcxrun);
        
        /* figure out appropriate return value */
        if (!hidden)
            val.runsv.runsvnum = 0;
        else if (!output_occurred)
            val.runsv.runsvnum = 1;
        else
            val.runsv.runsvnum = 2;
        runpush(ctx->bifcxrun, DAT_NUMBER, &val);

        /* actually hide the output, resetting count flag */
        outhide();
    }
    else if (runtostyp(ctx->bifcxrun) == DAT_NIL)
    {
        /* throw away the flag */
        rundisc(ctx->bifcxrun);

        /* show output, returning status */
        runpush(ctx->bifcxrun, runclog(outshow()), &val);
    }
    else if (runtostyp(ctx->bifcxrun) == DAT_NUMBER)
    {
        int n = runpopnum(ctx->bifcxrun);

        if (n == 0)
        {
            /* output was not hidden - show output and return status */
            runpush(ctx->bifcxrun, runclog(outshow()), &val);
        }
        else if (n == 1)
        {
            /*
             *   Output was hidden, but no output had occurred yet.
             *   Leave output hidden and return whether any output has
             *   occurred.
             */
            runpush(ctx->bifcxrun, runclog(output_occurred), &val);
        }
        else if (n == 2)
        {
            /*
             *   Output was hidden, and output had already occurred.  If
             *   more output has occurred, return true, else return nil.
             *   In either case, set the output_occurred flag back to
             *   true, since it was true before the outhide(true).  
             */
            runpush(ctx->bifcxrun, runclog(output_occurred), &val);
            outsethidden();
        }
        else
            errsig1(ctx->bifcxerr, ERR_INVVBIF, ERRTSTR, "outhide");
    }
    else
        errsig(ctx->bifcxerr, ERR_REQNUM);
}

/* put a numeric value in a list */
static uchar *bifputnum(uchar *lstp, uint val)
{
    *lstp++ = DAT_NUMBER;
    oswp4s(lstp, (long)val);
    return(lstp + 4);
}

/* gettime */
void biftim(bifcxdef *ctx, int argc) {
	TimeDate tm;
	uint timer;
    uchar      ret[80];
    uchar     *p;
    runsdef    val;
    int        typ;
	int tm_yday;
	const int MONTH_DAYS[11] = { 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30 };

    if (argc == 1)
    {
        /* get the time type */
        typ = (int)runpopnum(ctx->bifcxrun);
    }
    else
    {
        /* make sure no arguments are specified */
        bifcntargs(ctx, 0, argc);

        /* use the default time type */
        typ = 1;
    }

    switch(typ)
    {
    case 1:
        /* 
         *   default information format - list format with current system
         *   time and date 
         */

        /* make sure the time zone is set up properly */
        os_tzset();
        
        /* get the local time information */
		g_system->getTimeAndDate(tm);
        
        /* adjust values for return format */
        tm.tm_wday++;

		// Get number of days since start of year
		tm_yday = tm.tm_mday;
		for (int monthNum = 1; monthNum < tm.tm_mon; ++monthNum) {
			int daysInMonth = MONTH_DAYS[monthNum - 1];
			if (monthNum == 2)
				daysInMonth = (tm.tm_year % 4) == 0 && (((tm.tm_year % 100) != 0) || ((tm.tm_year % 400) == 0)) ? 29 : 28;

			tm_yday += daysInMonth;
		}

		// TODO: Convert dae/tme to Unix style local time
		timer = 0;
        
        /* build return list value */
        oswp2(ret, 47);
        p = ret + 2;
        p = bifputnum(p, tm.tm_year);
        p = bifputnum(p, tm.tm_mon);
        p = bifputnum(p, tm.tm_mday);
        p = bifputnum(p, tm.tm_wday);
        p = bifputnum(p, tm_yday);
        p = bifputnum(p, tm.tm_hour);
        p = bifputnum(p, tm.tm_min);
        p = bifputnum(p, tm.tm_sec);
        *p++ = DAT_NUMBER;
        oswp4s(p, (long)timer);
        
        val.runstyp = DAT_LIST;
        val.runsv.runsvstr = ret;
        runpush(ctx->bifcxrun, DAT_LIST, &val);
        break;

    case 2:
        /*
         *   High-precision system timer value - returns the system time
         *   in milliseconds, relative to an arbitrary zero point 
         */
        runpnum(ctx->bifcxrun, os_get_sys_clock_ms());
        break;

    default:
        /* other types are invalid */
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "gettime");
        break;
    }
}

/* getfuse */
void bifgfu(bifcxdef *ctx, int argc)
{
    vocddef  *daem;
    objnum    func;
    runsdef   val;
    runcxdef *rcx = ctx->bifcxrun;
    int       slots;
    prpnum    prop;
    voccxdef *vcx = ctx->bifcxrun->runcxvoc;

    bifcntargs(ctx, 2, argc);
 
    if (runtostyp(rcx) == DAT_FNADDR)
    {
        /* check on a setfuse()-style fuse: get fnaddr, parm */
        func = runpopfn(rcx);
        runpop(rcx, &val);

        for (slots = vcx->voccxfuc, daem = vcx->voccxfus ;
             slots ; ++daem, --slots)
        {
            if (daem->vocdfn == func
                && daem->vocdarg.runstyp == val.runstyp
                && !memcmp(&val.runsv, &daem->vocdarg.runsv,
                           (size_t)datsiz(val.runstyp, &val.runsv)))
                goto ret_num;
        }
    }
    else
    {
        /* check on a notify()-style fuse: get object, &message */
        func = runpopobj(rcx);
        prop = runpopprp(rcx);

        for (slots = vcx->voccxalc, daem = vcx->voccxalm ;
             slots ; ++daem, --slots)
        {
            if (daem->vocdfn == func && daem->vocdprp == prop)
                goto ret_num;
        }
    }
    
    /* didn't find anything - return nil */
    runpush(rcx, DAT_NIL, &val);
    return;
    
ret_num:
    /* return current daem->vocdtim */
    runpnum(rcx, (long)daem->vocdtim);
    return;
}

/* runfuses */
void bifruf(bifcxdef *ctx, int argc)
{
    int     ret;
    runsdef val;

    bifcntargs(ctx, 0, argc);
    ret = exefuse(ctx->bifcxrun->runcxvoc, TRUE);
    runpush(ctx->bifcxrun, runclog(ret), &val);
}

/* rundaemons */
void bifrud(bifcxdef *ctx, int argc)
{
    bifcntargs(ctx, 0, argc);
    exedaem(ctx->bifcxrun->runcxvoc);
}

/* intersect */
void bifsct(bifcxdef *bifctx, int argc)
{
    runcxdef *ctx = bifctx->bifcxrun;
    uchar    *l1;
    uchar    *l2;
    uchar    *l3;
    uint      siz1;
    uint      siz2;
    uint      siz3;
    uchar    *p;
    uint      l;
    uint      dsz1;
    uint      dsz2;
    runsdef   val;
    runsdef   stk1, stk2;
    
    bifcntargs(bifctx, 2, argc);
    l1 = runpoplst(ctx);
    siz1 = osrp2(l1);
    l2 = runpoplst(ctx);
    siz2 = osrp2(l2);

    /* make sure the first list is smaller - if not, switch them */
    if (siz1 > siz2)
        l3 = l1, l1 = l2, l2 = l3, siz3 = siz1, siz1 = siz2, siz2 = siz3;
    
    /* size of result is at most size of smaller list (which is now siz1) */
    stk1.runstyp = stk2.runstyp = DAT_LIST;
    stk1.runsv.runsvstr = l1;
    stk2.runsv.runsvstr = l2;
    runhres2(ctx, siz1, 2, &stk1, &stk2);
    l1 = stk1.runsv.runsvstr;
    l2 = stk2.runsv.runsvstr;
    l3 = ctx->runcxhp + 2;

    /* go through list1, and copy each element that is found in list2 */
    for (l1 += 2, l2 += 2, siz1 -= 2, siz2 -= 2 ; siz1 ; lstadv(&l1, &siz1))
    {
        dsz1 = datsiz(*l1, l1 + 1) + 1;
        for (l = siz2, p = l2 ; l ; lstadv(&p, &l))
        {
            dsz2 = datsiz(*p, p + 1) + 1;
#ifndef AMIGA
            if (dsz1 == dsz2 && !memcmp(l1, p, (size_t)dsz1))
#else /* AMIGA */
            if (!memcmp(l1, p, (size_t)dsz1) && (dsz1 == dsz2) )
#endif /* AMIGA */
            {
                memcpy(l3, p, (size_t)dsz1);
                l3 += dsz1;
                break;
            }
        }
    }
    
    /* set up return value, take it out of the heap, and push value */
    val.runsv.runsvstr = ctx->runcxhp;
    val.runstyp = DAT_LIST;
    oswp2(ctx->runcxhp, (uint)(l3 - ctx->runcxhp));
    ctx->runcxhp = l3;
    runrepush(ctx, &val);
}

/*
 *   Portable keystroke mappings.  We map the extended key codes to these
 *   strings, so that the TADS code can access arrow keys and the like.  
 */
static const char *ext_key_names[] =
{
    "[up]",                                                   /* CMD_UP - 1 */
    "[down]",                                               /* CMD_DOWN - 2 */
    "[right]",                                             /* CMD_RIGHT - 3 */
    "[left]",                                               /* CMD_LEFT - 4 */
    "[end]",                                                 /* CMD_END - 5 */
    "[home]",                                               /* CMD_HOME - 6 */
    "[del-eol]",                                            /* CMD_DEOL - 7 */
    "[del-line]",                                           /* CMD_KILL - 8 */
    "[del]",                                                 /* CMD_DEL - 9 */
    "[scroll]",                                             /* CMD_SCR - 10 */
    "[page up]",                                           /* CMD_PGUP - 11 */
    "[page down]",                                         /* CMD_PGDN - 12 */
    "[top]",                                                /* CMD_TOP - 13 */
    "[bottom]",                                             /* CMD_BOT - 14 */
    "[f1]",                                                  /* CMD_F1 - 15 */
    "[f2]",                                                  /* CMD_F2 - 16 */
    "[f3]",                                                  /* CMD_F3 - 17 */
    "[f4]",                                                  /* CMD_F4 - 18 */
    "[f5]",                                                  /* CMD_F5 - 19 */
    "[f6]",                                                  /* CMD_F6 - 20 */
    "[f7]",                                                  /* CMD_F7 - 21 */
    "[f8]",                                                  /* CMD_F8 - 22 */
    "[f9]",                                                  /* CMD_F9 - 23 */
    "[f10]",                                                /* CMD_F10 - 24 */
    "[?]",                                  /* invalid key - CMD_CHOME - 25 */
    "[tab]",                                                /* CMD_TAB - 26 */
    "[?]",                                   /* invalid key - shift-F2 - 27 */
    "[?]",                                      /* not used (obsolete) - 28 */
    "[word-left]",                                    /* CMD_WORD_LEFT - 29 */
    "[word-right]",                                  /* CMD_WORD_RIGHT - 30 */
    "[del-word]",                                      /* CMD_WORDKILL - 31 */
    "[eof]",                                                /* CMD_EOF - 32 */
    "[break]"                                             /* CMD_BREAK - 33 */
};

/*
 *   Get the name of a keystroke.  Pass in the one or two characters
 *   returned by os_getc(), and we'll fill in the buffer with the
 *   inputkey() name of the keystroke.  Returns true if the key was valid,
 *   false if not.  'c' is the first character returned by os_getc() for
 *   the keystroke; if 'c' is zero, then 'extc' is the character returned
 *   by the second call to os_getc() to get the CMD_xxx code for the
 *   keystroke.
 *   
 *   The name buffer should be 20 characters long - this will ensure that
 *   any name will fit.  
 *   
 *   For ordinary, printable characters, we'll simply return the
 *   character; the letter 'a', for example, is returned as the string "a".
 *   
 *   For extended keys, we'll look up the CMD_xxx code and return the name
 *   of the command, enclosed in square brackets; see the ext_key_names
 *   table for the mappings.  The left-arrow cursor key, for example,
 *   returns "[left]".
 *   
 *   For control characters, we'll generate a name like "[ctrl-a]", except
 *   for the following characters:
 *   
 *.  ascii 10 returns "\n"
 *.  ascii 13 returns "\n"
 *.  ascii 9 returns "\t"
 *.  ascii 8 returns "[bksp]" 
 */
static int get_ext_key_name(char *namebuf, int c, int extc)
{
    /* if it's a control character, translate it */
    if (c >= 1 && c <= 27)
    {
        switch(c)
        {
        case 10:
        case 13:
            /* return '\n' for LF and CR characters */
            strcpy(namebuf, "\\n");
            return TRUE;

        case 9:
            /* return '\t' for TAB characters */
            strcpy(namebuf, "\\t");
            return TRUE;

        case 8:
            /* return '[bksp]' for backspace characters */
            strcpy(namebuf, "[bksp]");
            return TRUE;

        case 27:
            /* return '[esc]' for the escape key */
            strcpy(namebuf, "[esc]");
            return TRUE;

        default:
            /* return '[ctrl-X]' for other control characters */
            strcpy(namebuf, "[ctrl-X]");
            namebuf[6] = (char)(c + 'a' - 1);
            return TRUE;
        }
    }

    /* if it's any other non-extended key, return it as-is */
    if (c != 0)
    {
        namebuf[0] = c;
        namebuf[1] = '\0';
        return TRUE;
    }
    
    /* if it's in the key name array, use the array entry */
    if (extc >= 1
        && extc <= (int)(sizeof(ext_key_names)/sizeof(ext_key_names[0])))
    {
        /* use the array name */
        strcpy(namebuf, ext_key_names[extc - 1]);
        return TRUE;
    }

    /* if it's in the ALT key range, generate an ALT key name */
    if (extc >= CMD_ALT && extc <= CMD_ALT + 25)
    {
        /* generate an ALT key name */
        strcpy(namebuf, "[alt-X]");
        namebuf[5] = (char)(extc - CMD_ALT + 'a');
        return TRUE;
    }

    /* it's not a valid key - use '[?]' as the name */
    strcpy(namebuf, "[?]");
    return FALSE;
}


/* inputkey */
void bifink(bifcxdef *ctx, int argc)
{
    int    c;
    int    extc;
    char   str[20];
    size_t len;
    
    bifcntargs(ctx, 0, argc);
    tioflushn(ctx->bifcxtio, 0);

    /* get a key */
    c = os_getc_raw();

    /* if it's extended, get the second part of the extended sequence */
    extc = (c == 0 ? os_getc_raw() : 0);

    /* map the extended key name */
    get_ext_key_name(str, c, extc);

    /* get the length of the name */
    len = strlen(str);

    /* reset the [more] counter */
    outreset();

    /* return the string, translating escapes */
    runpstr(ctx->bifcxrun, str, len, 0);
}

/* get direct/indirect object word list */
void bifwrd(bifcxdef *ctx, int argc)
{
    int       ob;
    vocoldef *v;
    uchar     buf[128];
    uchar *dst;
    const uchar *src;
    uint      len;
    runsdef   val;
    
    bifcntargs(ctx, 1, argc);

    /* figure out what word list to get */
    ob = runpopnum(ctx->bifcxrun);
    switch(ob)
    {
    case 1:
        v = ctx->bifcxrun->runcxvoc->voccxdobj;
        break;

    case 2:
        v = ctx->bifcxrun->runcxvoc->voccxiobj;
        break;

    default:
        runpnil(ctx->bifcxrun);
        return;
    }

    /* now build a list of strings from the words, if there are any */
    if (v != 0 && voclistlen(v) != 0 && v->vocolfst != 0 && v->vocollst != 0)
    {
        for (dst = buf + 2, src = (const uchar *)v->vocolfst ;
             src <= (const uchar *)v->vocollst ; src += len + 1)
        {
            *dst++ = DAT_SSTRING;
            len = strlen((const char *)src);
            oswp2(dst, len + 2);
            strcpy((char *)dst + 2, (const char *)src);
            dst += len + 2;
        }
    }
    else
        dst = buf + 2;

    /* finish setting up the list length and return it */
    len = dst - buf;
    oswp2(buf, len);
    val.runsv.runsvstr = buf;
    val.runstyp = DAT_LIST;
    runpush(ctx->bifcxrun, DAT_LIST, &val);
}

/* add a vocabulary word to an object */
void bifadw(bifcxdef *ctx, int argc)
{
    uchar    *wrd;
    objnum    objn;
    prpnum    prpn;
    vocidef  *voci;
    int       classflg;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;

    bifcntargs(ctx, 3, argc);

    /* get the arguments */
    objn = runpopobj(ctx->bifcxrun);
    prpn = runpopprp(ctx->bifcxrun);
    wrd = runpopstr(ctx->bifcxrun);

    /* make sure the property is a valid part of speech property */
    if (!prpisvoc(prpn))
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "addword");

    /* get the vocidef for the object, and see if it's a class object */
    voci = vocinh(voc, objn);

    classflg = VOCFNEW;
    if (voci->vociflg & VOCIFCLASS) classflg |= VOCFCLASS;

    /* add the word */
    vocadd(voc, prpn, objn, classflg, (char *)wrd);

    /* generate undo for the operation */
    vocdusave_addwrd(voc, objn, prpn, classflg, (char *)wrd);
}

/* delete a vocabulary word from an object */
void bifdlw(bifcxdef *ctx, int argc)
{
    uchar    *wrd;
    objnum    objn;
    prpnum    prpn;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;

    bifcntargs(ctx, 3, argc);

    /* get the arguments */
    objn = runpopobj(ctx->bifcxrun);
    prpn = runpopprp(ctx->bifcxrun);
    wrd = runpopstr(ctx->bifcxrun);

    /* make sure the property is a valid part of speech property */
    if (!prpisvoc(prpn))
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "delword");

    /* delete the word */
    vocdel1(voc, objn, (char *)wrd, prpn, FALSE, FALSE, TRUE);
}

/* callback context for word list builder */
struct bifgtw_cb_ctx
{
    uchar *p;
    int    typ;
};

/* callback for word list builder */
static void bifgtw_cb(void *ctx0, vocdef *voc, vocwdef *vocw)
{
    struct bifgtw_cb_ctx *ctx = (struct bifgtw_cb_ctx *)ctx0;

    /* ignore deleted objects */
    if (vocw->vocwflg & VOCFDEL)
        return;

    /* ignore objects of the inappropriate type */
    if (vocw->vocwtyp != ctx->typ)
        return;
    
    /* the datatype is string */
    *ctx->p = DAT_SSTRING;

    /* copy the first word */
    memcpy(ctx->p + 3, voc->voctxt, (size_t)voc->voclen);

    /* if there are two words, add a space and the second word */
    if (voc->vocln2)
    {
        *(ctx->p + 3 + voc->voclen) = ' ';
        memcpy(ctx->p + 4 + voc->voclen, voc->voctxt + voc->voclen,
               (size_t)voc->vocln2);
        oswp2(ctx->p + 1, voc->voclen + voc->vocln2 + 3);
        ctx->p += voc->voclen + voc->vocln2 + 4;
    }
    else
    {
        oswp2(ctx->p + 1, voc->voclen+2);
        ctx->p += voc->voclen + 3;
    }
}

/* get the list of words for an object for a particular part of speech */
void bifgtw(bifcxdef *ctx, int argc)
{
    objnum    objn;
    prpnum    prpn;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    int       cnt;
    int       siz;
    runsdef   val;
    struct bifgtw_cb_ctx fnctx;

    bifcntargs(ctx, 2, argc);

    /* get the arguments */
    objn = runpopobj(ctx->bifcxrun);
    prpn = runpopprp(ctx->bifcxrun);
    
    /* make sure the property is a valid part of speech property */
    if (!prpisvoc(prpn))
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "delword");

    /* get the size of the list we'll need to build */
    voc_count(voc, objn, prpn, &cnt, &siz);

    /*
     *   calculate how much space it will take to make a list out of all
     *   these words: 2 bytes for the list length header; plus, for each
     *   entry, 1 byte for the type header, 2 bytes for the string size
     *   header, and possibly one extra byte for the two-word separator --
     *   a total of 4 bytes extra per word.  
     */
    siz += 2 + 4*cnt;

    /* reserve the space */
    runhres(ctx->bifcxrun, siz, 0);

    /* set up our callback context, and build the list */
    fnctx.p = ctx->bifcxrun->runcxhp + 2;
    fnctx.typ = prpn;
    voc_iterate(voc, objn, bifgtw_cb, &fnctx);

    /* set up the return value */
    val.runstyp = DAT_LIST;
    val.runsv.runsvstr = ctx->bifcxrun->runcxhp;

    /* write the list length, and advance past the space we used */
    oswp2(ctx->bifcxrun->runcxhp, fnctx.p - ctx->bifcxrun->runcxhp);
    ctx->bifcxrun->runcxhp = fnctx.p;

    /* return the list */
    runrepush(ctx->bifcxrun, &val);
}

/* verbinfo service routine - add an object to the output list */
static uchar *bifvin_putprpn(uchar *p, prpnum prpn)
{
    *p++ = DAT_PROPNUM;
    oswp2(p, prpn);
    return p + 2;
}

/* verbinfo */
void bifvin(bifcxdef *ctx, int argc)
{
    objnum  verb;
    objnum  prep;
    uchar   tplbuf[VOCTPL2SIZ];
    int     newstyle;

    /* get the verb */
    verb = runpopobj(ctx->bifcxrun);
    
    /* check for the presence of a preposition */
    if (argc == 1)
    {
        /* no preposition */
        prep = MCMONINV;
    }
    else
    {
        /* the second argument is the preposition */
        bifcntargs(ctx, 2, argc);
        prep = runpopobj(ctx->bifcxrun);
    }

    /* look up the template */
    if (voctplfnd(ctx->bifcxrun->runcxvoc, verb, prep, tplbuf, &newstyle))
    {
        prpnum   prp_do, prp_verdo, prp_io, prp_verio;
        int      flg_dis_do;
        ushort   siz;
        uchar   *p;
        runsdef  val;

        /* get the information from the template */
        prp_do     = voctpldo(tplbuf);
        prp_verdo  = voctplvd(tplbuf);
        prp_io     = voctplio(tplbuf);
        prp_verio  = voctplvi(tplbuf);
        flg_dis_do = (voctplflg(tplbuf) & VOCTPLFLG_DOBJ_FIRST) != 0;

        /*
         *   figure space for the return value: if there's a prep, three
         *   property pointers plus a boolean, otherwise just two property
         *   pointers 
         */
        siz = 2 + 2*(2+1);
        if (prep != MCMONINV)
            siz += (2+1) + 1;

        /* reserve the space */
        runhres(ctx->bifcxrun, siz, 0);

        /* build the output list */
        p = ctx->bifcxrun->runcxhp;
        oswp2(p, siz);
        p += 2;

        p = bifvin_putprpn(p, prp_verdo);
        if (prep == MCMONINV)
        {
            p = bifvin_putprpn(p, prp_do);
        }
        else
        {
            p = bifvin_putprpn(p, prp_verio);
            p = bifvin_putprpn(p, prp_io);
            *p++ = runclog(flg_dis_do);
        }

        /* build the return value */
        val.runstyp = DAT_LIST;
        val.runsv.runsvstr = ctx->bifcxrun->runcxhp;

        /* consume the space */
        ctx->bifcxrun->runcxhp += siz;

        /* return the list */
        runrepush(ctx->bifcxrun, &val);
    }
    else
    {
        /* no template for this verb - return nil */
        runpnil(ctx->bifcxrun);
    }
}


/* clearscreen */
void bifcls(bifcxdef *ctx, int argc)
{
    /* this takes no arguments */
    bifcntargs(ctx, 0, argc);

    /* flush any pending output */
    tioflushn(ctx->bifcxtio, 0);
    
    /* clear the screen */
    oscls();
}

/*
 *   File operations 
 */

/*
 *   fopen(file, mode).
 *   
 *   Operations are allowed only if they conform to the current I/O safety
 *   level.  The safety level can be set by the user on the command line
 *   when running the game, and some implementations may allow the setting
 *   to be saved as a preference.  The possible levels are:
 *   
 *.  0 - minimum safety - read and write in any directory
 *.  1 - read in any directory, write in current directory
 *.  2 - read/write access in current directory only
 *.  3 - read-only access in current directory only
 *.  4 - maximum safety - no file I/O allowed
 *   
 *   When operations are allowed only in the current directory, the
 *   operations will fail if the filename contains any sort of path
 *   specifier (for example, on Unix, any file that contains a '/' is
 *   considered to have a path specifier, and will always fail if
 *   operations are only allowed in the current directory).  
 */
void biffopen(bifcxdef *ctx, int argc)
{
    char      fname[OSFNMAX];
    uchar    *p;
    uchar    *mode;
    int       modelen;
    int       fnum;
    osfildef *fp;
    int       bin_mode = TRUE;   /* flag: mode is binary (rather than text) */
    int       rw_mode = FALSE;     /* flag: both read and write are allowed */
    char      main_mode;                     /* 'r' for read, 'w' for write */
    int       in_same_dir;            /* flag: file is in current directory */
    appctxdef *appctx;
    
    bifcntargs(ctx, 2, argc);

    /* get the filename */
    p = runpopstr(ctx->bifcxrun);
    bifcstr(ctx, fname, (size_t)sizeof(fname), p);

    /* 
     *   If it's a relative path, combine it with the game file path to form
     *   the absolute path.  This ensures that relative paths are always
     *   relative to the original working directory if the OS-level working
     *   directory has changed. 
     */
    if (!os_is_file_absolute(fname))
    {
        /* combine the game file path with the relative filename */
        char newname[OSFNMAX];
        os_build_full_path(newname, sizeof(newname),
                           ctx->bifcxrun->runcxgamepath, fname);

        /* replace the original filename with the full path */
        strcpy(fname, newname);
    }

    /* get the mode string */
    mode = runpopstr(ctx->bifcxrun);
    modelen = osrp2(mode) - 2;
    mode += 2;
    if (modelen < 1)
        goto bad_mode;

    /* allocate a filenum for the file */
    for (fnum = 0 ; fnum < BIFFILMAX ; ++fnum)
    {
        if (ctx->bifcxfile[fnum].fp == 0)
            break;
    }
    if (fnum == BIFFILMAX)
    {
        /* return nil to indicate failure */
        runpnil(ctx->bifcxrun);
        return;
    }

    /* parse the main mode */
    switch(*mode)
    {
    case 'w':
    case 'W':
        main_mode = 'w';
        break;

    case 'r':
    case 'R':
        main_mode = 'r';
        break;

    default:
        goto bad_mode;
    }

    /* skip the main mode, and check for a '+' flag */
    ++mode;
    --modelen;
    if (modelen > 0 && *mode == '+')
    {
        /* note the read/write mode */
        rw_mode = TRUE;

        /* skip the speciifer */
        ++mode;
        --modelen;
    }

    /* check for a binary/text specifier */
    if (modelen > 0)
    {
        switch(*mode)
        {
        case 'b':
        case 'B':
            bin_mode = TRUE;
            break;

        case 't':
        case 'T':
            bin_mode = FALSE;
            break;

        default:
            goto bad_mode;
        }

        /* skip the binary/text specifier */
        ++mode;
        --modelen;
    }

    /* it's an error if there's anything left unparsed */
    if (modelen > 0)
        goto bad_mode;

    /*
     *   If we have a host application context, and it provides a file
     *   safety level callback function, ask the host system for its
     *   current file safety level, which overrides our current setting.  
     */
    appctx = ctx->bifcxappctx;
    if (appctx != 0 && appctx->get_io_safety_level != 0)
    {
        /* 
         *   ask the host system for the current level, and override any
         *   setting we previously had 
         */
        (*appctx->get_io_safety_level)(
            appctx->io_safety_level_ctx,
            &ctx->bifcxsafetyr, &ctx->bifcxsafetyw);
    }

    /* 
     *   Check to see if the file is in the current working directory - if
     *   not, we may have to disallow the operation based on safety level
     *   settings.
     */
    in_same_dir = os_is_file_in_dir(
        fname, ctx->bifcxrun->runcxgamepath, TRUE, FALSE);

    /* check file safety settings */
    switch(main_mode)
    {
    case 'w':
        /* 
         *   writing - we must be at a safety level no higher than 2
         *   (read/write current directory) to write at all, and we must be
         *   level 0 to write a file that's not in the current directory 
         */
        if (ctx->bifcxsafetyw > 2
            || (!in_same_dir && ctx->bifcxsafetyw > 0))
        {
            /* this operation is not allowed - return failure */
            runpnil(ctx->bifcxrun);
            return;
        }
        break;

    case 'r':
        /*
         *   reading - we must be at a safety level no higher than 3 (read
         *   current directory) to read at all, and we must be at safety
         *   level 1 (read any directory) or lower to read a file that's not
         *   in the current directory 
         */
        if (ctx->bifcxsafetyr > 3
            || (!in_same_dir && ctx->bifcxsafetyr > 1))
        {
            /* this operation is not allowed - return failure */
            runpnil(ctx->bifcxrun);
            return;
        }
        break;

    default:
        /* 
         *   fail the operation, as a code maintenance measure to make
         *   sure that we add appropriate cases to this switch (even if
         *   merely to allow the operation unconditionally) in the event
         *   that more modes are added in the future 
         */
        goto bad_mode;
    }

    /* try opening the file */
    switch(main_mode)
    {
    case 'w':
        /* check for binary vs text mode */
        if (bin_mode)
        {
            /* 
             *   binary mode -- allow read/write or just writing, but in
             *   either case truncate the file if it already exists, and
             *   create a new file if it doesn't exist 
             */
            if (rw_mode)
                fp = osfoprwtb(fname, OSFTDATA);
            else
                fp = osfopwb(fname, OSFTDATA);
        }
        else
        {
            /* text mode - don't allow read/write on a text file */
            if (rw_mode)
                goto bad_mode;

            /* open the file */
            fp = osfopwt(fname, OSFTTEXT);
        }
        break;

    case 'r':
        /* check for binary vs text mode */
        if (bin_mode)
        {
            /*
             *   Binary mode -- allow read/write or just reading; leave
             *   any existing file intact.
             */
            if (rw_mode)
            {
                /* open for reading and writing, keeping existing data */
                fp = osfoprwb(fname, OSFTDATA);
            }
            else
            {
                /* open for read-only */
                fp = osfoprb(fname, OSFTDATA);
            }
        }
        else
        {
            /* text mode -- only allow reading */
            if (rw_mode)
                goto bad_mode;

            /* open the file */
            fp = osfoprt(fname, OSFTTEXT);
        }
        break;

    default:
        goto bad_mode;
    }

    /* if we couldn't open it, return nil */
    if (fp == 0)
    {
        runpnil(ctx->bifcxrun);
        return;
    }

    /* store the flags */
    ctx->bifcxfile[fnum].flags = 0;
    if (bin_mode)
        ctx->bifcxfile[fnum].flags |= BIFFIL_F_BINARY;

    /* remember the file handle */
    ctx->bifcxfile[fnum].fp = fp;

    /* return the file number (i.e., the slot number) */
    runpnum(ctx->bifcxrun, (long)fnum);
    return;


    /* come here on a mode error */
bad_mode:
    runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "fopen");
}
              
/* service routine for file routines - get and validate a file number */
static osfildef *bif_get_file(bifcxdef *ctx, int *fnump, int *bin_modep)
{
    long fnum;

    /* get the file number and make sure it's valid */
    fnum = runpopnum(ctx->bifcxrun);
    if (fnum < 0 || fnum >= BIFFILMAX || ctx->bifcxfile[fnum].fp == 0)
        runsig(ctx->bifcxrun, ERR_BADFILE);

    /* put the validated file number, if the caller wants it */
    if (fnump != 0)
        *fnump = (int)fnum;

    /* set the binary-mode flag, if the caller wants it */
    if (bin_modep != 0)
        *bin_modep = ((ctx->bifcxfile[fnum].flags & BIFFIL_F_BINARY) != 0);

    /* return the file array pointer */
    return ctx->bifcxfile[fnum].fp;
}

void biffclose(bifcxdef *ctx, int argc)
{
    int       fnum;
    osfildef *fp;

    /* get the file number */
    bifcntargs(ctx, 1, argc);
    fp = bif_get_file(ctx, &fnum, 0);

    /* close the file and release the slot */
    osfcls(fp);
    ctx->bifcxfile[fnum].fp = 0;
}
              
void bifftell(bifcxdef *ctx, int argc)
{
    osfildef *fp;

    /* get the file number */
    bifcntargs(ctx, 1, argc);
    fp = bif_get_file(ctx, (int *)0, 0);

    /* return the seek position */
    runpnum(ctx->bifcxrun, osfpos(fp));
}
              
void biffseek(bifcxdef *ctx, int argc)
{
    osfildef *fp;
    long      pos;

    /* get the file pointer */
    bifcntargs(ctx, 2, argc);
    fp = bif_get_file(ctx, (int *)0, 0);

    /* get the seek position, and seek there */
    pos = runpopnum(ctx->bifcxrun);
    osfseek(fp, pos, OSFSK_SET);
}
              
void biffseekeof(bifcxdef *ctx, int argc)
{
    osfildef *fp;

    /* get the file pointer */
    bifcntargs(ctx, 1, argc);
    fp = bif_get_file(ctx, (int *)0, 0);

    /* seek to the end */
    osfseek(fp, 0L, OSFSK_END);
}
              
void biffwrite(bifcxdef *ctx, int argc)
{
    osfildef *fp;
    char      typ;
    char      buf[32];
    runsdef   val;
    int       bin_mode;
    
    /* get the file */
    bifcntargs(ctx, 2, argc);
    fp = bif_get_file(ctx, (int *)0, &bin_mode);

    /* get the value to write */
    runpop(ctx->bifcxrun, &val);
    typ = val.runstyp;

    if (bin_mode)
    {
        /* put a byte indicating the type */
        if (osfwb(fp, &typ, 1))
            goto ret_error;
        
        /* see what type of data we want to put */
        switch(typ)
        {
        case DAT_NUMBER:
            oswp4s(buf, val.runsv.runsvnum);
            if (osfwb(fp, buf, 4))
                goto ret_error;
            break;
            
        case DAT_SSTRING:
            /* write the string, including the length prefix */
            if (osfwb(fp, val.runsv.runsvstr, osrp2(val.runsv.runsvstr)))
                goto ret_error;
            break;
            
        case DAT_TRUE:
            /* all we need for this is the type prefix */
            break;
            
        default:
            /* other types are not acceptable */
            runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "fwrite");
        }
    }
    else
    {
        uint rem;
        uchar *p;
        
        switch(typ)
        {
        case DAT_SSTRING:
            /*
             *   Copy and translate the string to our buffer, in pieces if
             *   the size of the string exceeds that of our buffer.  If we
             *   encounter any escape codes, translate them.  
             */
            rem = osrp2(val.runsv.runsvstr) - 2;
            p = val.runsv.runsvstr + 2;
            while (rem > 0)
            {
                uchar *dst;
                uchar dbuf[256];

                /* fill up the buffer */
                for (dst = dbuf ;
                     rem != 0 && (size_t)(dst - dbuf) < sizeof(dbuf) - 1 ;
                     ++p, --rem)
                {
                    /* if we have an escape character, translate it */
                    if (*p == '\\' && rem > 1)
                    {
                        /* skip the opening slash */
                        ++p;
                        --rem;

                        /* translate it */
                        switch(*p)
                        {
                        case 'n':
                            *dst++ = '\n';
                            break;

                        case 't':
                            *dst++ = '\t';
                            break;

                        default:
                            *dst++ = *p;
                            break;
                        }
                    }
                    else
                    {
                        /* copy this character directly */
                        *dst++ = *p;
                    }
                }

                /* null-terminate the buffer */
                *dst = '\0';

                /* write it out */
                if (osfputs((char *)dbuf, fp) == EOF)
                    goto ret_error;
            }

            /* done */
            break;

        default:
            /* other types are not allowed */
            runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "fwrite");
        }
    }

    /* success */
    runpnil(ctx->bifcxrun);
    return;

ret_error:
    val.runstyp = DAT_TRUE;
    runpush(ctx->bifcxrun, DAT_TRUE, &val);
}
              
void biffread(bifcxdef *ctx, int argc)
{
    osfildef *fp;
    char      typ;
    char      buf[32];
    runsdef   val;
    ushort    len;
    int       bin_mode;

    /* get the file pointer */
    bifcntargs(ctx, 1, argc);
    fp = bif_get_file(ctx, (int *)0, &bin_mode);

    if (bin_mode)
    {
        /* binary file - read the type byte */
        if (osfrb(fp, &typ, 1))
            goto ret_error;
        
        /* read the data according to the type */
        switch(typ)
        {
        case DAT_NUMBER:
            if (osfrb(fp, buf, 4))
                goto ret_error;
            runpnum(ctx->bifcxrun, osrp4s(buf));
            break;
            
        case DAT_SSTRING:
            /* get the size */
            if (osfrb(fp, buf, 2))
                goto ret_error;
            len = osrp2(buf);
            
            /* reserve space */
            runhres(ctx->bifcxrun, len, 0);
            
            /* read the string into the reserved space */
            if (osfrb(fp, ctx->bifcxrun->runcxhp + 2, len - 2))
                goto ret_error;
            
            /* set up the string */
            oswp2(ctx->bifcxrun->runcxhp, len);
            val.runstyp = DAT_SSTRING;
            val.runsv.runsvstr = ctx->bifcxrun->runcxhp;
            
            /* consume the space */
            ctx->bifcxrun->runcxhp += len;
            
            /* push the value */
            runrepush(ctx->bifcxrun, &val);
            break;
            
        case DAT_TRUE:
            val.runstyp = DAT_TRUE;
            runpush(ctx->bifcxrun, DAT_TRUE, &val);
            break;
            
        default:
            goto ret_error;
        }
    }
    else
    {
        uchar  dbuf[257];
        uchar *dst;
        uchar *src;
        uint   dlen;
        uint   res_total;
        int    found_nl;

        /* 
         *   reserve some space in the heap - we'll initially reserve
         *   space for twice our buffer, in case every single character
         *   needs to be expanded into an escape sequence
         */
        res_total = sizeof(dbuf) * 2;
        runhres(ctx->bifcxrun, res_total, 0);

        /* set up our output value */
        val.runstyp = DAT_SSTRING;
        val.runsv.runsvstr = ctx->bifcxrun->runcxhp;
        dst = ctx->bifcxrun->runcxhp + 2;

        /* keep going until we find a newline or run out of data */
        for (found_nl = FALSE ; !found_nl ; )
        {
            /* text-mode - read the result into our buffer */
            if (!osfgets((char *)dbuf, sizeof(dbuf) - 1, fp))
            {
                /* 
                 *   if we found a newline, return what we have;
                 *   otherwise, return an error 
                 */
                if (found_nl)
                    break;
                else
                    goto ret_error;
            }
            
            /* 
             *   make sure it's null-terminated, in case the buffer was
             *   full 
             */
            dbuf[256] = '\0';
            
            /* translate into the heap area we've reserved */
            for (src = dbuf ; *src != '\0' ; ++src, ++dst)
            {
                /* determine if we need translations */
                switch(*src)
                {
                case '\n':
                case '\r':
                    /* translate to a newline sequence */
                    *dst++ = '\\';
                    *dst = 'n';
                    
                    /* note that we've found our newline */
                    found_nl = TRUE;
                    break;
                    
                case '\t':
                    /* translate to a tab sequence */
                    *dst++ = '\\';
                    *dst = 't';
                    break;
                    
                case '\\':
                    /* expand to a double-backslash sequence */
                    *dst++ = '\\';
                    *dst = '\\';
                    break;

                default:
                    /* leave other characters intact */
                    *dst = *src;
                    break;
                }
            }

            /*
             *   If we didn't find the newline, we'll need more space.
             *   This is a bit tricky, because the space we've already set
             *   up may move if we compact the heap while asking for more
             *   space.  So, remember our current length, reserve another
             *   buffer-full of space, and set everything up at the new
             *   output location if necessary.  
             */
            if (!found_nl)
            {
                /* reserve another buffer-full (double for expansion) */
                res_total += sizeof(dbuf) * 2;

                /* note our current offset */
                dlen = dst - val.runsv.runsvstr;
                oswp2(val.runsv.runsvstr, dlen);

                /* ask for the space */
                runhres(ctx->bifcxrun, res_total, 0);

                /* 
                 *   Since we were at the top of the heap before, we
                 *   should still be at the top of the heap.  If not,
                 *   we'll have to copy from our old location to the new
                 *   top of the heap.  
                 */
                if (val.runsv.runsvstr != ctx->bifcxrun->runcxhp)
                {
                    /* copy our existing text to our new location */
                    memmove(ctx->bifcxrun->runcxhp, val.runsv.runsvstr, dlen);

                    /* fix up our pointer */
                    val.runsv.runsvstr = ctx->bifcxrun->runcxhp;
                }

                /* re-establish our output pointer at our new location */
                dst = val.runsv.runsvstr + dlen;
            }
        }

        /* finish setting up the string */
        dlen = dst - val.runsv.runsvstr;
        oswp2(val.runsv.runsvstr, dlen);

        /* consume the space */
        ctx->bifcxrun->runcxhp += dlen;

        /* push the value */
        runrepush(ctx->bifcxrun, &val);
    }

    /* success - we've already pushed the return value */
    return;

ret_error:
    runpnil(ctx->bifcxrun);
}

void bifcapture(bifcxdef *ctx, int argc)
{
    mcmcxdef *mcx = ctx->bifcxrun->runcxmem;
    mcmon     obj;
    uint      siz;
    uint      ofs;
    uchar    *p;

    /* get the capture on/off flag */
    bifcntargs(ctx, 1, argc);
    switch(runtostyp(ctx->bifcxrun))
    {
    case DAT_TRUE:
        /* turn on capturing */
        tiocapture(ctx->bifcxtio, mcx, TRUE);

        /*
         *   The return value is a status code used to restore the
         *   original status on the bracketing call to turn off output.
         *   The only status necessary is the current output size. 
         */
        siz = tiocapturesize(ctx->bifcxtio);
        runpnum(ctx->bifcxrun, (long)siz);
        break;

    case DAT_NUMBER:
        /* get the original offset */
        ofs = runpopnum(ctx->bifcxrun);

        /* get the capture object and size */
        obj = tiogetcapture(ctx->bifcxtio);
        siz = tiocapturesize(ctx->bifcxtio);
        if (obj == MCMONINV)
        {
            runpnil(ctx->bifcxrun);
            return;
        }

        /* turn off capturing and reset the buffer on the outermost call */
        if (ofs == 0)
        {
            tiocapture(ctx->bifcxtio, mcx, FALSE);
            tioclrcapture(ctx->bifcxtio);
        }

        /* lock the object */
        p = mcmlck(mcx, obj);

        /* include only the part that happened after the matching call */
        p += ofs;
        siz = (ofs > siz) ? 0 : siz - ofs;

        ERRBEGIN(ctx->bifcxerr)

        /* push the string onto the stack */
        runpstr(ctx->bifcxrun, (char *)p, siz, 0);
        
        ERRCLEAN(ctx->bifcxerr)
            /* done with the object - unlock it */
            mcmunlck(mcx, obj);
        ERRENDCLN(ctx->bifcxerr)

        /* done with the object - unlock it */
        mcmunlck(mcx, obj);
        break;

    default:
        runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "outcapture");
    }
}

/*
 *   systemInfo 
 */
void bifsysinfo(bifcxdef *ctx, int argc)
{
    runsdef val;
    int id;
    long result;

    /* see what we have */
    switch(id = (int)runpopnum(ctx->bifcxrun))
    {
    case SYSINFO_SYSINFO:
        /* systemInfo call is supported in this version - return true */
        bifcntargs(ctx, 1, argc);
        val.runstyp = DAT_TRUE;
        runpush(ctx->bifcxrun, DAT_TRUE, &val);
        break;

    case SYSINFO_VERSION:
        /* get the run-time version string */
        bifcntargs(ctx, 1, argc);
        runpushcstr(ctx->bifcxrun, TADS_RUNTIME_VERSION,
                    strlen(TADS_RUNTIME_VERSION), 0);
        break;

    case SYSINFO_OS_NAME:
        /* get the operating system name */
        bifcntargs(ctx, 1, argc);
        runpushcstr(ctx->bifcxrun, OS_SYSTEM_NAME, strlen(OS_SYSTEM_NAME), 0);
        break;
        
    case SYSINFO_HTML:
    case SYSINFO_JPEG:
    case SYSINFO_PNG:
    case SYSINFO_WAV:
    case SYSINFO_MIDI:
    case SYSINFO_WAV_MIDI_OVL:
    case SYSINFO_WAV_OVL:
    case SYSINFO_PREF_IMAGES:
    case SYSINFO_PREF_SOUNDS:
    case SYSINFO_PREF_MUSIC:
    case SYSINFO_PREF_LINKS:
    case SYSINFO_MPEG:
    case SYSINFO_MPEG1:
    case SYSINFO_MPEG2:
    case SYSINFO_MPEG3:
    case SYSINFO_LINKS_HTTP:
    case SYSINFO_LINKS_FTP:
    case SYSINFO_LINKS_NEWS:
    case SYSINFO_LINKS_MAILTO:
    case SYSINFO_LINKS_TELNET:
    case SYSINFO_PNG_TRANS:
    case SYSINFO_PNG_ALPHA:
    case SYSINFO_OGG:
    case SYSINFO_MNG:
    case SYSINFO_MNG_TRANS:
    case SYSINFO_MNG_ALPHA:
    case SYSINFO_TEXT_HILITE:
    case SYSINFO_INTERP_CLASS:
        /* 
         *   these information types are all handled by the OS layer, and
         *   take no additional arguments 
         */
        bifcntargs(ctx, 1, argc);
        if (os_get_sysinfo(id, 0, &result))
        {
            /* we got a valid result - return it */
            runpnum(ctx->bifcxrun, result);
        }
        else
        {
            /* the code was unknown - return nil */
            runpnil(ctx->bifcxrun);
        }
        break;

    case SYSINFO_HTML_MODE:
        /* ask the output formatter for its current HTML setting */
        bifcntargs(ctx, 1, argc);
        val.runstyp = runclog(tio_is_html_mode());
        runpush(ctx->bifcxrun, val.runstyp, &val);
        break;

    case SYSINFO_TEXT_COLORS:
        /* 
         *   Text colors are only supported in full HTML interpreters.  If
         *   this is an HTML interpreter, ask the underlying OS layer about
         *   color support; otherwise, colors are not available, since we
         *   don't handle colors in our text-only HTML subset.
         *   
         *   Colors are NOT supported in the HTML mini-parser in text-only
         *   interpreters in TADS 2.  So, even if we're running in HTML
         *   mode, if this is a text-only interpreter, we can't display text
         *   colors.  
         */
        bifcntargs(ctx, 1, argc);
        if (os_get_sysinfo(SYSINFO_HTML, 0, &result) && result != 0)
        {
            /* 
             *   we're in HTML mode, so ask the underlying HTML OS
             *   implementation for its level of text color support 
             */
            if (os_get_sysinfo(id, 0, &result))
            {
                /* push the OS-level result */
                runpnum(ctx->bifcxrun, result);
            }
            else
            {
                /* the OS code doesn't recognize it; assume no support */
                runpnum(ctx->bifcxrun, SYSINFO_TXC_NONE);
            }
        }
        else
        {
            /* we're a text-only interpreter - no color support */
            runpnum(ctx->bifcxrun, SYSINFO_TXC_NONE);
        }
        break;

    case SYSINFO_BANNERS:
        /* TADS 2 does not offer banner support */
        bifcntargs(ctx, 1, argc);
        runpnum(ctx->bifcxrun, 0);
        break;
        
    default:
        /* 
         *   Other codes fail harmlessly with a nil return value.  Pop all
         *   remaining arguments and return nil. 
         */
        for ( ; argc > 1 ; --argc)
            rundisc(ctx->bifcxrun);
        runpnil(ctx->bifcxrun);
        break;
    }
}

/*
 *   morePrompt - display the more prompt and wait for the user to respond 
 */
void bifmore(bifcxdef *ctx, int argc)
{
    /* this function takes no arguments */
    bifcntargs(ctx, 0, argc);

    /* display the MORE prompt */
    tioflushn(ctx->bifcxtio, 1);
    out_more_prompt();
}

/*
 *   parserSetMe 
 */
void bifsetme(bifcxdef *ctx, int argc)
{
    objnum new_me;
    
    /* this function takes one argument */
    bifcntargs(ctx, 1, argc);

    /* get the new "Me" object */
    new_me = runpopobj(ctx->bifcxrun);

    /* "Me" cannot be nil */
    if (new_me == MCMONINV)
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "parserSetMe");

    /* set the current "Me" object in the parser */
    voc_set_me(ctx->bifcxrun->runcxvoc, new_me);
}

/*
 *   parserGetMe 
 */
void bifgetme(bifcxdef *ctx, int argc)
{
    /* this function takes no arguments */
    bifcntargs(ctx, 0, argc);

    /* return the current Me object */
    runpobj(ctx->bifcxrun, ctx->bifcxrun->runcxvoc->voccxme);
}

/*
 *   reSearch 
 */
void bifresearch(bifcxdef *ctx, int argc)
{
    uchar  *patstr;
    size_t  patlen;
    uchar  *searchstr;
    size_t  searchlen;
    int     result_len;
    int     match_ofs;
    
    /* this function takes two parameters: pattern, string */
    bifcntargs(ctx, 2, argc);

    /* get the pattern string */
    patstr = runpopstr(ctx->bifcxrun);
    patlen = osrp2(patstr) - 2;
    patstr += 2;

    /* get the search string */
    searchstr = runpopstr(ctx->bifcxrun);
    searchlen = osrp2(searchstr) - 2;
    searchstr += 2;

    /* search for the pattern in the string */
    match_ofs = re_compile_and_search(&ctx->bifcxregex,
                                      (char *)patstr, patlen,
                                      (char *)searchstr, searchlen,
                                      &result_len);

    /* 
     *   if we didn't match, return nil; otherwise, return a list with the
     *   match offset and length 
     */
    if (match_ofs < 0)
    {
        /* no match - return nil */
        runpnil(ctx->bifcxrun);
    }
    else
    {
        ushort listsiz;
        runsdef val;
        uchar *p;
        
        /* 
         *   build a list consisting of two numbers and a string: two
         *   bytes for the list header, then two elements at (one byte for
         *   the datatype header, four bytes for the number), then the
         *   string element with (one byte for the datatype, two bytes for
         *   the string length prefix, and the bytes of the string) 
         */
        listsiz = 2 + (1+4)*2 + (1 + 2 + (ushort)(result_len));

        /* allocate the space */
        runhres(ctx->bifcxrun, listsiz, 0);

        /* set up the list stack item */
        val.runstyp = DAT_LIST;
        p = val.runsv.runsvstr = ctx->bifcxrun->runcxhp;

        /* set the list's length */
        oswp2(p, listsiz);
        p += 2;

        /* 
         *   Add the offset element.  For consistency with TADS
         *   conventions, use 1 as the offset of the first character in
         *   the string - this makes it easy to use the offset value with
         *   substr(). 
         */
        *p++ = DAT_NUMBER;
        oswp4s(p, match_ofs + 1);
        p += 4;

        /* add the length element */
        *p++ = DAT_NUMBER;
        oswp4s(p, result_len);
        p += 4;

        /* add the result string */
        *p++ = DAT_SSTRING;
        oswp2(p, result_len + 2);
        p += 2;
        memcpy(p, ctx->bifcxregex.strbuf + match_ofs, result_len);

        /* reserve the space in the heap */
        ctx->bifcxrun->runcxhp += listsiz;

        /* return the list */
        runrepush(ctx->bifcxrun, &val);
    }
}

/* reGetGroup */
void bifregroup(bifcxdef *ctx, int argc)
{
    int grp;
    size_t len;
    re_group_register *reg;
    ushort hplen;
    runsdef val;
    uchar *p;
    long numval;
    
    /* this function takes one parameter: the group number to retrieve */
    bifcntargs(ctx, 1, argc);

    /* get the group number */
    grp = (int)runpopnum(ctx->bifcxrun);

    /* make sure it's within range */
    if (grp < 1 || grp > RE_GROUP_REG_CNT)
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "reGetGroup");

    /* adjust from a 1-bias to an array index */
    --grp;

    /* if the group was never set, return nil */
    if (grp >= ctx->bifcxregex.cur_group)
    {
        runpnil(ctx->bifcxrun);
        return;
    }

    /* get the register */
    reg = &ctx->bifcxregex.regs[grp];

    /* if the group wasn't set, return nil */
    if (reg->start_ofs == 0 || reg->end_ofs == 0)
    {
        runpnil(ctx->bifcxrun);
        return;
    }

    /* calculate the length of the string in this group */
    len = reg->end_ofs - reg->start_ofs;

    /* 
     *   reserve the necessary heap space: two bytes for the list length
     *   prefix, two number elements (one byte each for the type, four
     *   bytes each for the value), and the string element (one byte for
     *   the type, two bytes for the length prefix, plus the string
     *   itself).  
     */
    hplen = (ushort)(2 + 2*(1+4) + (1 + 2 + len));
    runhres(ctx->bifcxrun, hplen, 0);

    /* set up the stack value */
    val.runstyp = DAT_LIST;
    p = val.runsv.runsvstr = ctx->bifcxrun->runcxhp;

    /* put in the list length prefix */
    oswp2(p, hplen);
    p += 2;

    /* add the starting character position of the group - adjust to 1-bias */
    *p++ = DAT_NUMBER;
    numval = (long)(reg->start_ofs - ctx->bifcxregex.strbuf) + 1;
    oswp4s(p, numval);
    p += 4;

    /* add the length of the group */
    *p++ = DAT_NUMBER;
    numval = (long)(reg->end_ofs - reg->start_ofs);
    oswp4s(p, numval);
    p += 4;

    /* set up the string */
    *p++ = DAT_SSTRING;
    oswp2(p, len+2);
    p += 2;
    memcpy(p, reg->start_ofs, len);

    /* consume the heap space */
    ctx->bifcxrun->runcxhp += hplen;

    /* push the result */
    runrepush(ctx->bifcxrun, &val);
}


/*
 *   inputevent 
 */
void bifinpevt(bifcxdef *ctx, int argc)
{
	unsigned long timeout = 0;
    int use_timeout = FALSE;
    os_event_info_t info;
    int evt;
    uchar *p;
    ushort lstsiz;
    runsdef val;
    size_t paramlen = 0;
    char keyname[20];
    
    /* check for a timeout value */
    if (argc == 0)
    {
        /* there's no timeout */
        use_timeout = FALSE;
        timeout = 0;
    }
    else if (argc >= 1)
    {
        /* get the timeout value */
        use_timeout = TRUE;
        timeout = (unsigned long)runpopnum(ctx->bifcxrun);
    }

    /* ensure we don't have too many arguments */
    if (argc > 1)
        runsig(ctx->bifcxrun, ERR_BIFARGC);

    /* flush any pending output */
    tioflushn(ctx->bifcxtio, 0);

    /* reset count of lines since keyboard input */
    tioreset(ctx->bifcxtio);

    /* ask the OS code for an event */
    evt = os_get_event(timeout, use_timeout, &info);

    /* 
     *   the list always minimally needs two bytes of length prefix plus a
     *   number with the event code (one byte for the type, four bytes for
     *   the value) 
     */
    lstsiz = 2 + (1 + 4);

    /* figure out how much space we'll need based on the event type */
    switch(evt)
    {
    case OS_EVT_KEY:
        /* 
         *   we need space for a string with one or two bytes (depending
         *   on whether or not we have an extended key code) - 1 byte for
         *   type code, 2 for length prefix, and 1 or 2 for the string's
         *   contents 
         */
        paramlen = (info.key[0] == 0 ? 2 : 1);

        /* map the extended key */
        get_ext_key_name(keyname, info.key[0], info.key[1]);

        /* determine the length we need for the string */
        paramlen = strlen(keyname);

        /* add it into the list */
        lstsiz += 1 + 2 + paramlen;
        break;

    case OS_EVT_HREF:
        /* 
         *   we need space for the href string - 1 byte for type code, 2
         *   for length prefix, plus the string's contents 
         */
        paramlen = strlen(info.href);
        lstsiz += 1 + 2 + (ushort)paramlen;
        break;

    default:
        /* other event types have no extra data */
        break;
    }

    /* allocate space for the list */
    runhres(ctx->bifcxrun, lstsiz, 0);

    /* set up the stack value */
    val.runstyp = DAT_LIST;
    p = val.runsv.runsvstr = ctx->bifcxrun->runcxhp;

    /* set up the list length prefix */
    oswp2(p, lstsiz);
    p += 2;

    /* set up the event type element */
    *p++ = DAT_NUMBER;
    oswp4s(p, evt);
    p += 4;

    /* add the event parameters, if any */
    switch(evt)
    {
    case OS_EVT_KEY:
        /* set up the string for the key */
        *p++ = DAT_SSTRING;
        oswp2(p, paramlen + 2);
        p += 2;

        /* add the characters to the string */
        memcpy(p, keyname, paramlen);
        p += paramlen;
        break;

    case OS_EVT_HREF:
        /* add the string for the href */
        *p++ = DAT_SSTRING;
        oswp2(p, paramlen + 2);
        memcpy(p + 2, info.href, paramlen);
        break;
    }

    /* consume the heap space */
    ctx->bifcxrun->runcxhp += lstsiz;

    /* push the result */
    runrepush(ctx->bifcxrun, &val);
}

/* timeDelay */
void bifdelay(bifcxdef *ctx, int argc)
{
    long delay;
    
    /* ensure we have the right number of arguments */
    bifcntargs(ctx, 1, argc);

    /* flush any pending output */
    tioflushn(ctx->bifcxtio, 0);

    /* get the delay time */
    delay = runpopnum(ctx->bifcxrun);

    /* let the system perform the delay */
    os_sleep_ms(delay);
}

/* setOutputFilter */
void bifsetoutfilter(bifcxdef *ctx, int argc)
{
    /* ensure we have the right number of arguments */
    bifcntargs(ctx, 1, argc);

    /* see what we have */
    switch(runtostyp(ctx->bifcxrun))
    {
    case DAT_NIL:
        /* remove the current filter */
        out_set_filter(MCMONINV);

        /* discard the argument */
        rundisc(ctx->bifcxrun);
        break;

    case DAT_FNADDR:
        /* set the filter to the given function */
        out_set_filter(runpopfn(ctx->bifcxrun));
        break;

    default:
        /* anything else is invalid */
        runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "setOutputFilter");
    }
}

/*
 *   Get an optional object argument.  If the next argument is not an
 *   object value, or we're out of arguments, we'll return MCMONINV.
 *   Otherwise, we'll pop the object value and return it, decrementing the
 *   remaining argument counter provided. 
 */
static objnum bif_get_optional_obj_arg(bifcxdef *ctx, int *rem_argc)
{
    /* if we're out of arguments, there's no object value */
    if (*rem_argc == 0)
        return MCMONINV;

    /* 
     *   if the next argument is not an object or nil, we're out of object
     *   arguments 
     */
    if (runtostyp(ctx->bifcxrun) != DAT_OBJECT
        && runtostyp(ctx->bifcxrun) != DAT_NIL)
        return MCMONINV;

    /* we have an object - remove it from the remaining argument count */
    --(*rem_argc);

    /* pop and return the object value */
    return runpopobjnil(ctx->bifcxrun);
}

/*
 *   execCommand flag values 
 */
#define EC_HIDE_SUCCESS     0x00000001
#define EC_HIDE_ERROR       0x00000002
#define EC_SKIP_VALIDDO     0x00000004
#define EC_SKIP_VALIDIO     0x00000008

/*
 *   execCommand - execute a recursive command 
 */
void bifexec(bifcxdef *ctx, int argc)
{
    objnum actor;
    objnum verb;
    objnum dobj;
    objnum prep;
    objnum iobj;
    int    err;
	uint   capture_start = 0;
    uint   capture_end;
    objnum capture_obj;
    ulong  flags;
    int    hide_any;
    int    rem_argc;
    
    /* 
     *   Check for the correct argument count.  The first two arguments
     *   are required; additional arguments are optional. 
     */
    if (argc < 2 || argc > 6)
        runsig(ctx->bifcxrun, ERR_BIFARGC);

    /* pop the arguments - actor, verb, dobj, prep, iobj */
    actor = runpopobjnil(ctx->bifcxrun);
    verb = runpopobjnil(ctx->bifcxrun);

    /* 
     *   The other object arguments are optional.  If we run into a
     *   numeric argument, it's the flags value, in which case we're out
     *   of objects. 
     */
    rem_argc = argc - 2;
    dobj = bif_get_optional_obj_arg(ctx, &rem_argc);
    prep = bif_get_optional_obj_arg(ctx, &rem_argc);
    iobj = bif_get_optional_obj_arg(ctx, &rem_argc);

    /* if we have a flags argument, pop it */
    if (rem_argc > 0)
    {
        /* the last argument is the flags - pop the numeric value */
        flags = runpopnum(ctx->bifcxrun);

        /* remove it from the remaining argument counter */
        --rem_argc;
    }
    else
    {
        /* no flags specified - use zero by default */
        flags = 0;
    }

    /* 
     *   make sure we don't have any arguments left - if we do, then it
     *   means that we got an incorrect type and skipped an argument when
     *   we were trying to sense the meanings of the arguments from their
     *   types 
     */
    if (rem_argc != 0)
        runsig1(ctx->bifcxrun, ERR_INVTBIF, ERRTSTR, "execCommand");

    /* if we're hiding any output, start output capture */
    hide_any = ((flags & (EC_HIDE_SUCCESS | EC_HIDE_ERROR)) != 0);
    if (hide_any)
    {
        /* start capturing */
        tiocapture(ctx->bifcxtio, ctx->bifcxrun->runcxmem, TRUE);

        /* note the current output position */
        capture_start = tiocapturesize(ctx->bifcxtio);
    }

    /* execute the command */
    err = execmd_recurs(ctx->bifcxrun->runcxvoc,
                        actor, verb, dobj, prep, iobj,
                        (flags & EC_SKIP_VALIDDO) == 0,
                        (flags & EC_SKIP_VALIDIO) == 0);

    /* if we're hiding any output, end hiding */
    if (hide_any)
    {
        uchar *p;
        int hide;
        
        /* get the capture buffer size */
        capture_end = tiocapturesize(ctx->bifcxtio);

        /* turn off capture if it wasn't already on when we started */
        if (capture_start == 0)
            tiocapture(ctx->bifcxtio, ctx->bifcxrun->runcxmem, FALSE);

        /* determine whether we're hiding or showing the result */
        if (err == 0)
            hide = ((flags & EC_HIDE_SUCCESS) != 0);
        else
            hide = ((flags & EC_HIDE_ERROR) != 0);

        /* show or hide the result, as appropriate */
        if (hide)
        {
            /* 
             *   We're hiding this result, so do not display the string.
             *   If there's an enclosing capture, remove the string from
             *   the enclosing capture.  
             */
            if (capture_start != 0)
                tiopopcapture(ctx->bifcxtio, capture_start);
        }
        else
        {
            /* 
             *   We're showing the text.  If we're in an enclosing
             *   capture, do nothing - simply leave the string in the
             *   enclosing capture buffer; otherwise, actually display it 
             */
            if (capture_start == 0)
            {
                /* lock the capture object */
                capture_obj = tiogetcapture(ctx->bifcxtio);
                p = mcmlck(ctx->bifcxrun->runcxmem, capture_obj);
                
                ERRBEGIN(ctx->bifcxerr)
                {
                    /* display the string */
                    outformatlen((char *)p + capture_start,
                                 capture_end - capture_start);
                }
                ERRCLEAN(ctx->bifcxerr)
                {
                    /* unlock the capture object before signalling out */
                    mcmunlck(ctx->bifcxrun->runcxmem, capture_obj);
                }
                ERRENDCLN(ctx->bifcxerr);

                /* unlock the capture object */
                mcmunlck(ctx->bifcxrun->runcxmem, capture_obj);
            }
        }
        
        /* clear the capture buffer if it wasn't on when we started */
        if (capture_start == 0)
            tioclrcapture(ctx->bifcxtio);
    }

    /* push the result code */
    runpnum(ctx->bifcxrun, err);
}

/* 
 *   parserGetObj - get one of the objects associated with the command 
 */
void bifgetobj(bifcxdef *ctx, int argc)
{
    int id;
	objnum obj = 0;
    voccxdef *voc = ctx->bifcxrun->runcxvoc;
    
    /* check the argument count */
    bifcntargs(ctx, 1, argc);

    /* get the argument */
    id = (int)runpopnum(ctx->bifcxrun);

    /* get the appropriate object */
    switch(id)
    {
    case 1:
        /* get the current actor */
        obj = voc->voccxactor;

        /* if there's no current actor, use the current 'me' by default */
        if (obj == MCMONINV)
            obj = voc->voccxme;

        /* done */
        break;

    case 2:
        /* verb */
        obj = voc->voccxverb;
        break;

    case 3:
        /* direct object */
        obj = (voc->voccxdobj == 0 ? MCMONINV : voc->voccxdobj->vocolobj);
        break;

    case 4:
        /* preposition */
        obj = voc->voccxprep;
        break;

    case 5:
        /* indirect object */
        obj = (voc->voccxiobj == 0 ? MCMONINV : voc->voccxiobj->vocolobj);
        break;

    case 6:
        /* "it" */
        obj = voc->voccxit;
        break;

    case 7:
        /* "him" */
        obj = voc->voccxhim;
        break;

    case 8:
        /* "her" */
        obj = voc->voccxher;
        break;

    case 9:
        /* them */
        voc_push_objlist(voc, voc->voccxthm, voc->voccxthc);

        /* 
         *   return directly, since we've already pushed the result (it's
         *   a list, not an object) 
         */
        return;

    default:
        /* invalid argument */
        runsig1(ctx->bifcxrun, ERR_INVVBIF, ERRTSTR, "parserGetObj");
        break;
    }

    /* return the object */
    runpobj(ctx->bifcxrun, obj);
}

/*
 *   parseNounList - parse a noun list.  Call like this:
 *   
 *   parserParseNounList(wordlist, typelist, starting_index, complain,
 *   multi, check_actor); 
 */
void bifparsenl(bifcxdef *ctx, int argc)
{
    /* check the argument count */
    bifcntargs(ctx, 6, argc);

    /* call the parser */
    voc_parse_np(ctx->bifcxrun->runcxvoc);
}

/*
 *   parserTokenize - given a string, produce a list of tokens.  Returns
 *   nil on error, or a list of token strings.
 *   
 *   parserTokenize(commandString); 
 */
void bifprstok(bifcxdef *ctx, int argc)
{
    /* check arguments */
    bifcntargs(ctx, 1, argc);

    /* call the parser */
    voc_parse_tok(ctx->bifcxrun->runcxvoc);
}

/*
 *   parserGetTokTypes - given a list of tokens (represented as strings),
 *   get a corresponding list of token types.
 *   
 *   parserGetTokTypes(tokenList); 
 */
void bifprstoktyp(bifcxdef *ctx, int argc)
{
    /* check arguments */
    bifcntargs(ctx, 1, argc);

    /* call the parser */
    voc_parse_types(ctx->bifcxrun->runcxvoc);
}

/*
 *   parserDictLookup - given a list of tokens and their types, produce a
 *   list of all of the objects that match all of the words.
 *   
 *   parserDictLookup(tokenList, typeList); 
 */
void bifprsdict(bifcxdef *ctx, int argc)
{
    /* check arguments */
    bifcntargs(ctx, 2, argc);

    /* call the parser */
    voc_parse_dict_lookup(ctx->bifcxrun->runcxvoc);
}

/*
 *   parserResolveObjects - resolve an object list of the sort returned by
 *   parseNounList.  Validates and disambiguates the objects.
 *   
 *   parserResolveObjects(actor, verb, prep, otherobj, usageType,
 *   verprop, tokenList, objList, silent) 
 */
void bifprsrslv(bifcxdef *ctx, int argc)
{
    /* check arguments */
    bifcntargs(ctx, 9, argc);

    /* call the parser */
    voc_parse_disambig(ctx->bifcxrun->runcxvoc);
}

/*
 *   parserReplaceCommand - replace the current command line with a new
 *   string.  Aborts the current command. 
 */
void bifprsrplcmd(bifcxdef *ctx, int argc)
{
    /* check arguments */
    bifcntargs(ctx, 1, argc);

    /* call the parser */
    voc_parse_replace_cmd(ctx->bifcxrun->runcxvoc);
}

/*
 *   exitobj - throw a RUNEXITOBJ error 
 */
void bifexitobj(bifcxdef *ctx, int argc)
{
    /* no arguments are allowed */
    bifcntargs(ctx, 0, argc);

    /* throw the RUNEXITOBJ error */
    errsig(ctx->bifcxerr, ERR_RUNEXITOBJ);
}

/*
 *   Standard system button labels for bifinpdlg() 
 */
#define BIFINPDLG_LBL_OK      1
#define BIFINPDLG_LBL_CANCEL  2
#define BIFINPDLG_LBL_YES     3
#define BIFINPDLG_LBL_NO      4

/*
 *   inputdialog 
 */
void bifinpdlg(bifcxdef *ctx, int argc)
{
    uchar *p;
    char prompt[256];
    char lblbuf[256];
    const char *labels[10];
    char *dst;
    char *xp;
    uint len;
    size_t bcnt;
    int default_resp, cancel_resp;
    int resp;
    int std_btns;
    int icon_id;

    /* check for proper arguments */
    bifcntargs(ctx, 5, argc);

    /* get the icon number */
    icon_id = runpopnum(ctx->bifcxrun);

    /* get the prompt string */
    p = runpopstr(ctx->bifcxrun);
    bifcstr(ctx, prompt, (size_t)sizeof(prompt), p);

    /* translate from internal to local characters */
    for (xp = prompt ; *xp != '\0' ; xp++)
        *xp = (char)cmap_i2n(*xp);

    /* check for a standard button set selection */
    if (runtostyp(ctx->bifcxrun) == DAT_NUMBER)
    {
        /* get the standard button set ID */
        std_btns = runpopnum(ctx->bifcxrun);

        /* there are no actual buttons */
        bcnt = 0;
    }
    else
    {
        /* we're not using standard buttons */
        std_btns = 0;
        
        /* get the response string list */
        p = runpoplst(ctx->bifcxrun);
        len = osrp2(p);
        p += 2;
        
        /* build our internal button list */
        for (bcnt = 0, dst = lblbuf ; len != 0 ; lstadv(&p, &len))
        {
            /* see what we have */
            if (*p == DAT_SSTRING)
            {
                /* it's a label string - convert to a C string */
                bifcstr(ctx, dst, sizeof(lblbuf) - (dst - lblbuf), p + 1);

                /* translate from internal to local characters */
                for (xp = dst ; *xp != '\0' ; xp++)
                    *xp = (char)cmap_i2n(*xp);
                
                /* set this button to point to the converted text */
                labels[bcnt++] = dst;
                
                /* move past this label in the button buffer */
                dst += strlen(dst) + 1;
            }
            else if (*p == DAT_NUMBER)
            {
                int id;
                int resid;
                
                /* it's a standard system label ID - get the ID */
                id = (int)osrp4s(p + 1);

                /* translate it to the appropriate string resource */
                switch(id)
                {
                case BIFINPDLG_LBL_OK:
                    resid = RESID_BTN_OK;
                    break;
                    
                case BIFINPDLG_LBL_CANCEL:
                    resid = RESID_BTN_CANCEL;
                    break;
                    
                case BIFINPDLG_LBL_YES:
                    resid = RESID_BTN_YES;
                    break;
                    
                case BIFINPDLG_LBL_NO:
                    resid = RESID_BTN_NO;
                    break;

                default:
                    resid = 0;
                    break;
                }

                /* 
                 *   if we got a valid resource ID, load the resource;
                 *   otherwise, skip this button
                 */
                if (resid != 0
                    && !os_get_str_rsc(resid, dst,
                                       sizeof(lblbuf) - (dst - lblbuf)))
                {
                    /* set this button to point to the converted text */
                    labels[bcnt++] = dst;

                    /* move past this label in the button buffer */
                    dst += strlen(dst) + 1;
                }
            }
            
            /* if we have exhausted our label array, stop now */
            if (bcnt >= sizeof(labels)/sizeof(labels[0])
                || dst >= lblbuf + sizeof(lblbuf))
                break;
        }
    }

    /* get the default response */
    if (runtostyp(ctx->bifcxrun) == DAT_NIL)
    {
        rundisc(ctx->bifcxrun);
        default_resp = 0;
    }
    else
        default_resp = runpopnum(ctx->bifcxrun);

    /* get the cancel response */
    if (runtostyp(ctx->bifcxrun) == DAT_NIL)
    {
        rundisc(ctx->bifcxrun);
        cancel_resp = 0;
    }
    else
        cancel_resp = runpopnum(ctx->bifcxrun);

    /* flush output before showing the dialog */
    tioflushn(ctx->bifcxtio, 0);

    /* show the dialog */
    resp = tio_input_dialog(icon_id, prompt, std_btns,
                            (const char **)labels, bcnt,
                            default_resp, cancel_resp);

    /* return the result */
    runpnum(ctx->bifcxrun, resp);
}

/*
 *   Determine if a resource exists 
 */
void bifresexists(bifcxdef *ctx, int argc)
{
    uchar *p;
    char resname[OSFNMAX];
    appctxdef *appctx;
    int found;
    runsdef val;
    
    /* check for proper arguments */
    bifcntargs(ctx, 1, argc);

    /* get the resource name string */
    p = runpopstr(ctx->bifcxrun);
    bifcstr(ctx, resname, (size_t)sizeof(resname), p);

    /* 
     *   if we have a host application context, and it provides a resource
     *   finder function, ask the resource finder if the resource is
     *   available; otherwise, report that the resource is not loadable,
     *   since we must not be running a version of the interpreter that
     *   supports external resource loading 
     */
    appctx = ctx->bifcxappctx;
    found = (appctx != 0
             && appctx->resfile_exists != 0
             && (*appctx->resfile_exists)(appctx->resfile_exists_ctx,
                                          resname, strlen(resname)));

    /* push the result */
    runpush(ctx->bifcxrun, runclog(found), &val);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
