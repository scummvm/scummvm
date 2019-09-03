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

#include "glk/tads/tads2/tokenizer.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/tads2/os.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* special temporary buffers for <<expr>> macro expansion */
static char tokmac1[]  = ",say((";
static char tokmac1s[] = "(say((";
static char tokmac2[]  = "),nil),\"";
static char tokmac3[]  = "),nil))";
static char tokmac4[]  = ")";

/* forward definition of static functions */
static int tokdfhsh(const char *sym, int len);


/* find a #define symbol */
static tokdfdef *tok_find_define(tokcxdef *ctx, const char *sym, int len)
{
    int       hsh;
    tokdfdef *df;

    /* find the appropriate chain the hash table */
    hsh = tokdfhsh(sym, len);

    /* search the chain for this symbol */
    for (df = ctx->tokcxdf[hsh] ; df ; df = df->nxt)
    {
        /* if this one matches, return it */
        if (df->len == len && !memcmp(df->nm, sym, (size_t)len))
        {
            /* fix it up if it's the special __FILE__ or __LINE__ symbol */
            if (len == 8)
            {
                if (!memcmp(sym, "__FILE__", (size_t)8))
                {
                    size_t elen;

                    /* 
                     *   put in the opening single quote, since we want
                     *   the expanded result to be a string 
                     */
                    df->expan[0] = '\'';

                    /* get the name */
                    linnam(ctx->tokcxlin, df->expan+1);

                    /* get the length, and add the closing quote */
                    elen = strlen(df->expan);
                    df->expan[elen] = '\'';

                    /* 
                     *   set the length of the expansion, including the
                     *   quotes (the first quote was measured in the
                     *   length originally, but the second quote hasn't
                     *   been counted yet, so add one to our original
                     *   length) 
                     */
                    df->explen = (int)elen + 1;

                    /* if the expansion is too long, it's an error */
                    if (df->explen >= OSFNMAX)
                        errsig(ctx->tokcxerr, ERR_LONG_FILE_MACRO);
                }
                else if (!memcmp(sym, "__LINE__", (size_t)8))
                {
                    ulong l;

                    /* get the line number */
                    l = linlnum(ctx->tokcxlin);

                    /* convert it to a textual format for the expansion */
                    sprintf(df->expan, "%lu", l);

                    /* set the expanded value's length */
                    df->explen = strlen(df->expan);

                    /* make sure the expansion isn't too long */
                    if (df->explen >= 40)
                        errsig(ctx->tokcxerr, ERR_LONG_LINE_MACRO);
                }
            }
            
            /* return it */
            return df;
        }
    }

    /* didn't find anything */
    return 0;
}

/*
 *   Write preprocessor state to a file 
 */
void tok_write_defines(tokcxdef *ctx, osfildef *fp, errcxdef *ec)
{
    int        i;
    tokdfdef **dfp;
    tokdfdef  *df;
    char       buf[4];

    /* write each element of the hash chains */
    for (i = TOKDFHSHSIZ, dfp = ctx->tokcxdf ; i ; ++dfp, --i)
    {
        /* write each entry in this hash chain */
        for (df = *dfp ; df ; df = df->nxt)
        {
            /* write this entry */
            oswp2(buf, df->len);
            oswp2(buf + 2, df->explen);
            if (osfwb(fp, buf, 4)
                || osfwb(fp, df->nm, df->len)
                || (df->explen != 0 && osfwb(fp, df->expan, df->explen)))
                errsig(ec, ERR_WRTGAM);
        }

        /* write a zero-length entry to indicate the end of this chain */
        oswp2(buf, 0);
        if (osfwb(fp, buf, 4)) errsig(ec, ERR_WRTGAM);
    }
}

/* compute a #define symbol's hash value */
static int tokdfhsh(const char *sym, int len)
{
    uint hsh;

    for (hsh = 0 ; len ; ++sym, --len)
        hsh = (hsh + *sym) & TOKDFHSHMASK;
    return hsh;
}

/* convert a #define symbol to lower case if folding case */
static const char *tok_casefold_defsym(tokcxdef *ctx, char *outbuf,
                                 const char *src, int len)
{
    if (ctx->tokcxflg & TOKCXCASEFOLD)
    {
        char *dst;
        int   rem;

        /* make a lower-case copy of the symbol */
        rem = (len > TOKNAMMAX ? TOKNAMMAX : len);
        for (dst = outbuf ; rem > 0 ; ++dst, ++src, --rem)
            *dst = (Common::isUpper((uchar)*src) ? Common::isLower((uchar)*src) : *src);

        /* use the lower-case copy instead of the original */
        return outbuf;
    }
    else
    {
        /* return the original unchanged */
        return src;
    }
}

/*
 *   convert a token to lower-case if we're folding case 
 */
void tok_case_fold(tokcxdef *ctx, tokdef *tok)
{
    /* if we're in case-insensitive mode, convert the token to lower-case */
    if (ctx->tokcxflg & TOKCXCASEFOLD)
    {
        char *p;
        int   len;

        /* convert each character in the token to lower-case */
        for (p = tok->toknam, len = tok->toklen ; len != 0 ; ++p, --len)
        {
            /* if this character is upper-case, convert it to lower-case */
            if (Common::isUpper((uchar)*p))
                *p = Common::isLower((uchar)*p);
        }
    }
}

/* add a symbol to the #define symbol table, folding case if necessary */
void tok_add_define_cvtcase(tokcxdef *ctx, const char *sym, int len,
                            const char *expan, int explen)
{
    char mysym[TOKNAMMAX];
    
    /* convert to lower-case if necessary */
    sym = tok_casefold_defsym(ctx, mysym, sym, len);

    /* add the symbol */
    tok_add_define(ctx, sym, len, expan, explen);
}

/* add a symbol to the #define symbol table */
void tok_add_define(tokcxdef *ctx, const char *sym, int len,
                    const char *expan, int explen)
{
    int       hsh;
    tokdfdef *df;

    /* if it's already defined, ignore it */
    if (tok_find_define(ctx, sym, len))
        return;

    /* find the appropriate entry in the hash table */
    hsh = tokdfhsh(sym, len);

    /* allocate space for the symbol */
    df = (tokdfdef *)mchalo(ctx->tokcxerr,
                            (sizeof(tokdfdef) + len + explen - 1),
                            "tok_add_define");

    /* set up the new symbol */
    df->nm = df->expan + explen;
    df->len = len;
    df->explen = explen;
    memcpy(df->expan, expan, explen);
    memcpy(df->nm, sym, len);

    /* link it into the hash chain */
    df->nxt = ctx->tokcxdf[hsh];
    ctx->tokcxdf[hsh] = df;
}

/* add a #define symbol with a numeric value */
void tok_add_define_num_cvtcase(tokcxdef *ctx, char *sym, int len, int num)
{
    char buf[20];
    
    /* convert the value to a string */
    sprintf(buf, "%d", num);

    /* add the text value */
    tok_add_define_cvtcase(ctx, sym, len, buf, strlen(buf));
}

/* undefine a #define symbol */
void tok_del_define(tokcxdef *ctx, const char *sym, int len)
{
    int       hsh;
    tokdfdef *df;
    tokdfdef *prv;
    
    /* find the appropriate chain the hash table */
    hsh = tokdfhsh(sym, len);

    /* search the chain for this symbol */
    for (prv = 0, df = ctx->tokcxdf[hsh] ; df ; prv = df, df = df->nxt)
    {
        /* if this one matches, delete it */
        if (df->len == len && !memcmp(df->nm, sym, (size_t)len))
        {
            /* unlink it from the chain */
            if (prv)
                prv->nxt = df->nxt;
            else
                ctx->tokcxdf[hsh] = df->nxt;

            /* delete this symbol, and we're done */
            mchfre(df);
            break;
        }
    }
}

/* scan a #define symbol to see how long it is */
static int tok_scan_defsym(tokcxdef *ctx, const char *p, int len)
{
    int symlen;

    /* make sure it's a valid symbol */
    if (!(Common::isAlpha((uchar)*p) || *p == '_' || *p == '$'))
    {
        errlog(ctx->tokcxerr, ERR_REQSYM);
        return 0;
    }

    /* count characters as long as we have valid symbol characters */
    for (symlen = 0 ; len && TOKISSYM(*p) ; ++p, --len, ++symlen) ;
    return symlen;
}

/* process a #define */
static void tokdefine(tokcxdef *ctx, const char *p, int len)
{
    const char *sym;
    int   symlen;
    const char *expan;
    char  mysym[TOKNAMMAX];
    
    /* get the symbol */
    sym = p;
    if (!(symlen = tok_scan_defsym(ctx, p, len)))
        return;

    /* if it's already in the table, log an error */
    if (tok_find_define(ctx, sym, symlen))
    {
        errlog(ctx->tokcxerr, ERR_DEFREDEF);
        return;
    }

    /* skip whitespace following the symbol */
    expan = sym + symlen;
    len -= symlen;
    while (len && t_isspace(*expan)) --len, ++expan;

    /* if we're folding case, convert the symbol to lower case */
    sym = tok_casefold_defsym(ctx, mysym, sym, symlen);

    /* define the symbol */
    tok_add_define(ctx, sym, symlen, expan, len);
}

/* 
 *   Update the #if status for the current nesting.  Any enclosing
 *   negative #if will override everything inside, so we need to look
 *   through the nesting from the outside in until we either determine
 *   that everything is affirmative or we find a negative anywhere in the
 *   nesting.  
 */
static void tok_update_if_stat(tokcxdef *ctx)
{
    int i;

    /* look through nesting from the outermost level */
    for (i = 0 ; i < ctx->tokcxifcnt ; ++i)
    {
        /* assume this level will apply to everything inside */
        ctx->tokcxifcur = ctx->tokcxif[i];

        /* if this level is off, everything inside is off */
        switch (ctx->tokcxif[i])
        {
        case TOKIF_IF_NO:
        case TOKIF_ELSE_NO:
            /* 
             *   this level is off, hence everything inside is off -- stop
             *   here with the current (negative) determination 
             */
            return;

        default:
            /* so far we're in the "on" section, so keep looking */
            break;
        }
    }
}

/* process an #ifdef or a #ifndef */
static void tok_ifdef_ifndef(tokcxdef *ctx, const char *p, int len, int is_ifdef)
{
    int   symlen;
    const char *sym;
    int   stat;
    int   found;
    char  mysym[TOKNAMMAX];

    /* get the symbol */
    sym = p;
    if (!(symlen = tok_scan_defsym(ctx, p, len)))
        return;

    /* if we're folding case, convert the symbol to lower case */
    sym = tok_casefold_defsym(ctx, mysym, sym, symlen);

    /* see if we can find it in the table, and set the status accordingly */
    found = (tok_find_define(ctx, sym, symlen) != 0);

    /* invert the test if this is an ifndef */
    if (!is_ifdef) found = !found;

    /* set the #if status accordingly */
    if (found)
        stat = TOKIF_IF_YES;
    else
        stat = TOKIF_IF_NO;
    ctx->tokcxif[ctx->tokcxifcnt] = stat;

    /* allocate a new #if level (making sure we have room) */
    if (ctx->tokcxifcnt >= TOKIFNEST)
    {
        errlog(ctx->tokcxerr, ERR_MANYPIF);
        return;
    }
    ctx->tokcxifcnt++;

    /* update the current status */
    tok_update_if_stat(ctx);
}

/* process a #error */
static void tok_p_error(tokcxdef *ctx, const char *p, int len)
{
    errlog1(ctx->tokcxerr, ERR_P_ERROR,
            ERRTSTR, errstr(ctx->tokcxerr, p, len));
}

/* process a #ifdef */
static void tokifdef(tokcxdef *ctx, const char *p, int len)
{
    tok_ifdef_ifndef(ctx, p, len, TRUE);
}

/* process a #ifndef */
static void tokifndef(tokcxdef *ctx, const char *p, int len)
{
    tok_ifdef_ifndef(ctx, p, len, FALSE);
}

/* process a #if */
static void tokif(tokcxdef *ctx, const char *p, int len)
{
    errsig(ctx->tokcxerr, ERR_PIF_NA);
}

/* process a #elif */
static void tokelif(tokcxdef *ctx, const char *p, int len)
{
    errsig(ctx->tokcxerr, ERR_PELIF_NA);
}

/* process a #else */
static void tokelse(tokcxdef *ctx, const char *p, int len)
{
    int cnt;
    
    /* if we're not expecting #else, it's an error */
    cnt = ctx->tokcxifcnt;
    if (cnt == 0 || ctx->tokcxif[cnt-1] == TOKIF_ELSE_YES
        || ctx->tokcxif[cnt-1] == TOKIF_ELSE_NO)
    {
        errlog(ctx->tokcxerr, ERR_BADPELSE);
        return;
    }

    /* switch to the appropriate #else state (opposite the #if state) */
    if (ctx->tokcxif[cnt-1] == TOKIF_IF_YES)
        ctx->tokcxif[cnt-1] = TOKIF_ELSE_NO;
    else
        ctx->tokcxif[cnt-1] = TOKIF_ELSE_YES;

    /* update the current status */
    tok_update_if_stat(ctx);
}

/* process a #endif */
static void tokendif(tokcxdef *ctx, const char *p, int len)
{
    /* if we're not expecting #endif, it's an error */
    if (ctx->tokcxifcnt == 0)
    {
        errlog(ctx->tokcxerr, ERR_BADENDIF);
        return;
    }

    /* remove the #if level */
    ctx->tokcxifcnt--;

    /* update the current status */
    tok_update_if_stat(ctx);
}

/* process a #undef */
static void tokundef(tokcxdef *ctx, const char *p, int len)
{
    const char *sym;
    int   symlen;
    char  mysym[TOKNAMMAX];
    
    /* get the symbol */
    sym = p;
    if (!(symlen = tok_scan_defsym(ctx, p, len)))
        return;

    /* if we're folding case, convert the symbol to lower case */
    sym = tok_casefold_defsym(ctx, mysym, sym, symlen);

    /* if it's not defined, log a warning */
    if (!tok_find_define(ctx, sym, symlen))
    {
        errlog(ctx->tokcxerr, ERR_PUNDEF);
        return;
    }

    /* undefine the symbol */
    tok_del_define(ctx, sym, symlen);
}

/* process a #pragma directive */
static void tokpragma(tokcxdef *ctx, const char *p, int len)
{
    /* ignore empty pragmas */
    if (len == 0)
    {
        errlog(ctx->tokcxerr, ERR_PRAGMA);
        return;
    }

    /* see what we have */
    if (len > 1
        && (*p == 'c' || *p == 'C')
        && (*(p+1) == '+' || *(p+1) == '-' || t_isspace(*(p+1))))
    {
        /* skip spaces after the 'C', if any */
        for (++p, --len ; len && t_isspace(*p) ; ++p, --len) ;

        /* look for the + or - flag */
        if (len && *p == '+')
            ctx->tokcxflg |= TOKCXFCMODE;
        else if (len && *p == '-')
            ctx->tokcxflg &= ~TOKCXFCMODE;
        else
        {
            errlog(ctx->tokcxerr, ERR_PRAGMA);
            return;
        }
    }
    else
    {
        errlog(ctx->tokcxerr, ERR_PRAGMA);
    }
}

/* process a #include directive */
static void tokinclude(tokcxdef *ctx, const char *p, int len)
{
    linfdef *child;
    tokpdef *path;
    const char *fname;
    int      match;
    int      flen;
    linfdef *lin;
    const char *q;
    size_t   flen2;

    /* find the filename portion */
    fname = p + 1;                            /* remember start of filename */
    path = ctx->tokcxinc;                    /* start with first path entry */

    if (!len)
    {
        errlog(ctx->tokcxerr, ERR_INCNOFN);
        return;
    }
    
    switch(*p)
    {
    case '<':
        match = '>';
        if (path && path->tokpnxt) path = path->tokpnxt;   /* skip 1st path */
        goto find_matching_delim;

    case '"':
        match = '"';

    find_matching_delim:
        for (++p, --len ; len && *p != match ; --len, ++p) ;
        if (len == 0 || *p != match) errlog(ctx->tokcxerr, ERR_INCMTCH);
        break;
        
    default:
        errlog(ctx->tokcxerr, ERR_INCSYN);
        return;
    }
    
    flen = p - fname;                         /* compute length of filename */
    for (q = p, flen2 = 0 ;
         q > fname && *(q-1) != OSPATHCHAR && !strchr(OSPATHALT, *(q-1)) ;
         --q, ++flen2) ;
    
    /* check to see if this file has already been included */
    for (lin = ctx->tokcxhdr ; lin ; lin = (linfdef *)lin->linflin.linnxt)
    {
        char *p2 = lin->linfnam;
        
        p2 += strlen(p2);
        
        while (p2 > lin->linfnam && *(p2-1) != OSPATHCHAR
               && !strchr(OSPATHALT, *(p2-1)))
            --p2;
        if (strlen(p2) == flen2
            && !memicmp(p2, q, flen2))
        {
            errlog1(ctx->tokcxerr, ERR_INCRPT, ERRTSTR,
                    errstr(ctx->tokcxerr, fname, flen));
            return;
        }
    }
    
    /* initialize the line source */
    child = linfini(ctx->tokcxmem, ctx->tokcxerr, fname, flen, path, TRUE,
                    (ctx->tokcxflg & TOKCXFLIN2) != 0);
    
    /* if not found, signal an error */
    if (!child) errsig1(ctx->tokcxerr, ERR_INCSEAR,
                        ERRTSTR, errstr(ctx->tokcxerr, fname, flen));
    
    /* link into tokenizer list of line records */
    child->linflin.linnxt = (lindef *)ctx->tokcxhdr;
    ctx->tokcxhdr = child;

    /* if we're tracking sources for debugging, add into the chain */
    if (ctx->tokcxdbg)
    {
        ctx->tokcxdbg->dbgcxlin = &child->linflin;
        child->linflin.linid = ctx->tokcxdbg->dbgcxfid++;
    }

    /* remember my C-mode setting */
    if (ctx->tokcxflg & TOKCXFCMODE)
        ctx->tokcxlin->linflg |= LINFCMODE;
    else
        ctx->tokcxlin->linflg &= ~LINFCMODE;
    
    child->linflin.linpar = ctx->tokcxlin;   /* remember parent line source */
    ctx->tokcxlin = &child->linflin;   /* make the child the current source */
}

/* get a new line from line source, processing '#' directives */
static int tokgetlin(tokcxdef *ctx, int dopound)
{
    for (;;)
    {
        if (linget(ctx->tokcxlin))
        {
            /* at eof in current source; resume parent if there is one */
            if (ctx->tokcxlin->linpar)
            {
                lindef *parent;
                
                parent = ctx->tokcxlin->linpar;          /* remember parent */
                lincls(ctx->tokcxlin);               /* close included file */
                if (!ctx->tokcxdbg)               /* if no debug context... */
                    mchfre(ctx->tokcxlin);              /* free line source */
                ctx->tokcxlin = parent;      /* reset to parent line source */
                if (parent->linflg & LINFCMODE)
                    ctx->tokcxflg |= TOKCXFCMODE;
                else
                    ctx->tokcxflg &= ~TOKCXFCMODE;
                continue;                       /* back for another attempt */
            }
            else
            {
                /* check for outstanding #if/#ifdef */
                if (ctx->tokcxifcnt)
                    errlog(ctx->tokcxerr, ERR_NOENDIF);

                /* return end-of-file indication */
                return TRUE;
            }
        }
        
        /* if this is a multi-segment line, copy it into our own buffer */
        if (ctx->tokcxlin->linflg & LINFMORE)
        {
            char *p;
            uint  rem;
            int   done;
            
            if (!ctx->tokcxbuf)
            {
                /* allocate 1k as a default buffer */
                ctx->tokcxbuf = (char *)mchalo(ctx->tokcxerr, 1024,
                                               "tok");
                ctx->tokcxbsz = 1024;
            }
            ctx->tokcxlen = 0;
            
            for (done = FALSE, p = ctx->tokcxbuf, rem = ctx->tokcxbsz ;
                 !done ; )
            {
                size_t len = ctx->tokcxlin->linlen;

                /* add the current segment's length into line length */
                ctx->tokcxlen += len;
                
                /* we're done after this piece if the last fetch was all */
                done = !(ctx->tokcxlin->linflg & LINFMORE);
                if (len + 1 > rem)
                {
                    char *newp;

                    /* increase the size of the buffer */
                    if (ctx->tokcxbsz > (unsigned)0x8000)
                        errsig(ctx->tokcxerr, ERR_LONGLIN);
                    rem += 4096;
                    ctx->tokcxbsz += 4096;
                    
                    /* allocate a new buffer and copy line into it */
                    newp = (char *)mchalo(ctx->tokcxerr, ctx->tokcxbsz, "tok");
                    memcpy(newp, ctx->tokcxbuf, (size_t)(p - ctx->tokcxbuf));
                    
                    /* free the original buffer, and use the new one */
                    p = (p - ctx->tokcxbuf) + newp;
                    mchfre(ctx->tokcxbuf);
                    ctx->tokcxbuf = newp;
                }
                
                /* add the line to the buffer */
                memcpy(p, ctx->tokcxlin->linbuf, len);
                p += len;
                rem -= len;
                
                /* get the next piece of the line if there is one */
                if (!done)
                {
                    if (linget(ctx->tokcxlin)) break;
                }
            }
            
            /* null-terminate the buffer, and use it for input */
            *p = '\0';
            ctx->tokcxptr = ctx->tokcxbuf;
        }
        else
        {
            ctx->tokcxptr = ctx->tokcxlin->linbuf;
            ctx->tokcxlen = ctx->tokcxlin->linlen;
        }
        
        /* check for preprocessor directives */
        if (dopound && ctx->tokcxlen != 0 && ctx->tokcxptr[0] == '#'
            && !(ctx->tokcxlin->linflg & LINFNOINC))
        {
            const char *p;
            int     len;
            static const struct
            {
                const char *nm;
                int    len;
                int    ok_in_if;
                void (*fn)(tokcxdef *, const char *, int);
            }
            *dirp, dir[] =
            {
                { "include", 7, FALSE, tokinclude },
                { "pragma",  6, FALSE, tokpragma },
                { "define",  6, FALSE, tokdefine },
                { "ifdef",   5, TRUE, tokifdef },
                { "ifndef",  6, TRUE, tokifndef },
                { "if",      2, TRUE, tokif },
                { "else",    4, TRUE, tokelse },
                { "elif",    4, TRUE, tokelif },
                { "endif",   5, TRUE, tokendif },
                { "undef",   5, FALSE, tokundef },
                { "error",   5, FALSE, tok_p_error }
            };
            int  i;

            /* scan off spaces between '#' and directive */
            for (len = ctx->tokcxlen - 1, p = &ctx->tokcxptr[1] ;
                 len && t_isspace(*p) ; --len, ++p) ;

            /* find and process the directive */
            for (dirp = dir, i = sizeof(dir)/sizeof(dir[0]) ; i ; --i, ++dirp)
            {
                /* compare this directive; if it wins, call its function */
                if (len >= dirp->len && !memcmp(p, dirp->nm, (size_t)dirp->len)
                    && (len == dirp->len || t_isspace(*(p + dirp->len))))
                {
                    int cnt;
                    int stat;
                    
                    /*
                     *   if we're not in a #if's false part, or if the
                     *   directive is processed even in #if false parts,
                     *   process the line, otherwise skip it 
                     */
                    cnt = ctx->tokcxifcnt;
                    if (dirp->ok_in_if || cnt == 0
                        || ((stat = ctx->tokcxifcur) == TOKIF_IF_YES
                            || stat == TOKIF_ELSE_YES))
                    {
                        /* skip whitespace following the directive */
                        for (p += dirp->len, len -= dirp->len ;
                             len && t_isspace(*p) ;
                             --len, ++p) ;

                        /* invoke the function to process this directive */
                        (*dirp->fn)(ctx, p, len);
                    }

                    /* there's no need to look at more directives */
                    break;
                }
            }

            /* if we didn't find anything, flag the error */
            if (i == 0)
                errlog(ctx->tokcxerr, ERR_PRPDIR);

            /* ignore this line */
            continue;
        }
        else
        {
            /*
             *   Check the #if level.  If we're in an #if, and we're to
             *   ignore lines (because of a false condition or an #else
             *   part for a true condition), skip this line. 
             */
            if (ctx->tokcxifcnt != 0)
            {
                switch(ctx->tokcxifcur)
                {
                case TOKIF_IF_NO:
                case TOKIF_ELSE_NO:
                    /* ignore this line */
                    continue;

                default:
                    /* we're in a true part - keep the line */
                    break;
                }
            }
            
            ctx->tokcxlin->linflg &= ~LINFDBG;       /* no debug record yet */
            return(FALSE);                      /* return the line we found */
        }
    }
}

/* get the next token, removing it from the input stream */
int toknext(tokcxdef *ctx)
{
    const char *p;
    tokdef *tok = &ctx->tokcxcur;
    int     len;

    /* 
     *   Check for the special case that we pushed an open paren prior to
     *   a string containing an embedded expression.  If this is the case,
     *   immediately return the string we previously parsed. 
     */
    if ((ctx->tokcxflg & TOKCXF_EMBED_PAREN_PRE) != 0)
    {
        /* 
         *   convert the token to a string - note that the offset
         *   information for the string is already in the current token
         *   structure, since we set everything up for it on the previous
         *   call where we actually parsed the beginning of the string 
         */
        tok->toktyp = TOKTDSTRING;

        /* clear the special flag - we've now consumed the pushed string */
        ctx->tokcxflg &= ~TOKCXF_EMBED_PAREN_PRE;

        /* immediately return the string */
        return tok->toktyp;
    }

    /* set up at the current scanning position */
    p = ctx->tokcxptr;
    len = ctx->tokcxlen;

    /* scan off whitespace and comments until we find something */
    do
    {
    skipblanks:
        /* if there's nothing on this line, get the next one */
        if (len == 0)
        {
            /* if we're in a macro expansion, continue after it */
            if (ctx->tokcxmlvl)
            {
                ctx->tokcxmlvl--;
                p = ctx->tokcxmsav[ctx->tokcxmlvl];
                len = ctx->tokcxmsvl[ctx->tokcxmlvl];
            }
            else
            {
                if (tokgetlin(ctx, TRUE))
                {
                    tok->toktyp = TOKTEOF;
                    goto done;
                }
                p = ctx->tokcxptr;
                len = ctx->tokcxlen;
            }
        }
        while (len && t_isspace(*p)) ++p, --len;     /* scan off whitespace */
        
        /* check for comments, and remove if present */
        if (len >= 2 && *p == '/' && *(p+1) == '/')
            len = 0;
        else if (len >= 2 && *p == '/' && *(p+1) == '*')
        {
            while (len < 2 || *p != '*' || *(p+1) != '/')
            {
                if (len != 0)
                    ++p, --len;

                if (len == 0)
                {
                    if (ctx->tokcxmlvl != 0)
                    {
                        ctx->tokcxmlvl--;
                        p = ctx->tokcxmsav[ctx->tokcxmlvl];
                        len = ctx->tokcxmsvl[ctx->tokcxmlvl];
                    }
                    else
                    {
                        if (tokgetlin(ctx, FALSE))
                        {
                            ctx->tokcxptr = p;
                            tok->toktyp = TOKTEOF;
                            goto done;
                        }
                        p = ctx->tokcxptr;
                        len = ctx->tokcxlen;
                    }
                }
            }
            p += 2;
            len -= 2;
            goto skipblanks;
        }
    } while (len == 0);
    
nexttoken:
    if (Common::isAlpha((uchar)*p) || *p == '_' || *p == '$')
    {
        int       l;
        int       hash;
        const char *q;
	char     *tq;
        toktdef  *tab;
        int       found = FALSE;
        uchar     thischar;
        tokdfdef *df;
        
        for (hash = 0, l = 0, tq = tok->toknam ;
             len != 0 && TOKISSYM(*p) && l < TOKNAMMAX ;
             (thischar = ((Common::isUpper((uchar)*p)
                           && (ctx->tokcxflg & TOKCXCASEFOLD))
                          ? Common::isLower((uchar)*p) : *p)),
             (hash = ((hash + thischar) & (TOKHASHSIZE - 1))),
             (*tq++ = thischar), ++p, --len, ++l) ;
        *tq = '\0';
        if (len != 0 && TOKISSYM(*p))
        {
            while (len != 0 && TOKISSYM(*p)) ++p, --len;
            errlog1(ctx->tokcxerr, ERR_TRUNC, ERRTSTR,
                    errstr(ctx->tokcxerr, tok->toknam, tok->toklen));
        }
        tok->toklen = l;
        tok->tokhash = hash;

        /*
         *   check for the special defined() preprocessor operator 
         */
        if (l == 9 && !memcmp(tok->toknam,
                              ((ctx->tokcxflg & TOKCXCASEFOLD)
                               ? "__defined" : "__DEFINED"),
                              (size_t)9)
            && len > 2 && *p == '(' && TOKISSYM(*(p+1))
            && !Common::isDigit((uchar)*(p+1)))
        {
            int symlen;
            char mysym[TOKNAMMAX];
            
            /* find the matching ')', allowing only symbolic characters */
            ++p, --len;
            for (symlen = 0, q = p ; len && *p != ')' && TOKISSYM(*p) ;
                 ++p, --len, ++symlen) ;

            /* make sure we found the closing paren */
            if (!len || *p != ')')
                errsig(ctx->tokcxerr, ERR_BADISDEF);
            ++p, --len;

            /* if we're folding case, convert the symbol to lower case */
            q = tok_casefold_defsym(ctx, mysym, q, symlen);

            /* check to see if it's defined */
            tok->toktyp = TOKTNUMBER;
            tok->tokval = (tok_find_define(ctx, q, symlen) != 0);
            goto done;
        }

        /* substitute the preprocessor #define, if any */
        if ((df = tok_find_define(ctx, tok->toknam, l)) != 0)
        {
            /* save the current parsing position */
            if (ctx->tokcxmlvl >= TOKMACNEST)
                errsig(ctx->tokcxerr, ERR_MACNEST);
            ctx->tokcxmsav[ctx->tokcxmlvl] = p;
            ctx->tokcxmsvl[ctx->tokcxmlvl] = len;
            ctx->tokcxmlvl++;

            /* point to the token's expansion and keep going */
            p = df->expan;
            len = df->explen;
            goto nexttoken;
        }
        
        /* look up in symbol table(s), if any */
        for (tab = ctx->tokcxstab ; tab ; tab = tab->toktnxt)
        {
            if ((found = (*tab->toktfsea)(tab, tok->toknam, l, hash,
                                          &tok->toksym)) != 0)
                break;
        }
        
        if (found && tok->toksym.tokstyp == TOKSTKW)
            tok->toktyp = tok->toksym.toksval;
        else
        {
            tok->toktyp = TOKTSYMBOL;
            if (!found) tok->toksym.tokstyp = TOKSTUNK;
        }
        goto done;
    }
    else if (Common::isDigit((uchar)*p))
    {
        long acc = 0;
        
        /* check for octal/hex */
        if (*p == '0')
        {
            ++p, --len;
            if (len && (*p == 'x' || *p == 'X'))
            {
                /* hex */
                ++p, --len;
                while (len && TOKISHEX(*p))
                {
                    acc = (acc << 4) + TOKHEX2INT(*p);
                    ++p, --len;
                }
            }
            else
            {
                /* octal */
                while (len && TOKISOCT(*p))
                {
                    acc = (acc << 3) + TOKOCT2INT(*p);
                    ++p, --len;
                }
            }
        }
        else
        {
            /* decimal */
            while (len && Common::isDigit((uchar)*p))
            {
                acc = (acc << 1) + (acc << 3) + TOKDEC2INT(*p);
                ++p, --len;
            }
        }
        tok->tokval = acc;
        tok->toktyp = TOKTNUMBER;
        goto done;
    }
    else if (*p == '"' || *p == '\'')
    {
        char  delim;                 /* closing delimiter we're looking for */
        const char *strstart;                 /* pointer to start of string */
        int   warned;
        
        delim = *p;
        --len;
        strstart = ++p;

        if (delim == '"' && len >= 2 && *p == '<' && *(p+1) == '<')
        {
            /* save the current parsing position */
            if (ctx->tokcxmlvl >= TOKMACNEST)
                errsig(ctx->tokcxerr, ERR_MACNEST);
            ctx->tokcxmsav[ctx->tokcxmlvl] = p + 2;
            ctx->tokcxmsvl[ctx->tokcxmlvl] = len - 2;
            ctx->tokcxmlvl++;

            /* 
             *   read from the special "<<" expansion string - use the
             *   version for a "<<" at the very beginning of the string 
             */
            p = tokmac1s;
            len = strlen(p);
            ctx->tokcxflg |= TOKCXFINMAC;
            goto nexttoken;
        }
        tok->toktyp = (delim == '"' ? TOKTDSTRING : TOKTSSTRING);
        
        tok->tokofs = (*ctx->tokcxsst)(ctx->tokcxscx);  /* start the string */
        for (warned = FALSE ;; )
        {
            if (len >= 2 && *p == '\\')
            {
                if (*(p+1) == '"' || *(p+1) == '\'')
                {
                    (*ctx->tokcxsad)(ctx->tokcxscx, strstart,
                                     (ushort)(p - strstart));
                    strstart = p + 1;
                }
                p += 2;
                len -= 2;
            }
            else if (len == 0 || *p == delim ||
                     (delim == '"' && len >= 2 && *p == '<' && *(p+1) == '<'
                      && !(ctx->tokcxflg & TOKCXFINMAC)))
            {
                (*ctx->tokcxsad)(ctx->tokcxscx, strstart,
                                 (ushort)(p - strstart));
                if (len == 0)
                {
                    if (ctx->tokcxmlvl)
                    {
                        ctx->tokcxmlvl--;
                        p = ctx->tokcxmsav[ctx->tokcxmlvl];
                        len = ctx->tokcxmsvl[ctx->tokcxmlvl];
                    }
                    else
                        (*ctx->tokcxsad)(ctx->tokcxscx, " ", (ushort)1);
                    
                    while (len == 0)
                    {
                        if (tokgetlin(ctx, FALSE))
                            errsig(ctx->tokcxerr, ERR_STREOF);
                        p = ctx->tokcxptr;
                        len = ctx->tokcxlen;

                        /* warn if it looks like the end of an object */
                        if (!warned && len && (*p == ';' || *p == '}'))
                        {
                            errlog(ctx->tokcxerr, ERR_STREND);
                            warned = TRUE;     /* warn only once per string */
                        }

                        /* scan past whitespace at start of line */
                        while (len && t_isspace(*p)) ++p, --len;
                    }
                    strstart = p;
                }
                else break;
            }
            else
                ++p, --len;
        }

        /* end the string */
        (*ctx->tokcxsend)(ctx->tokcxscx);

        /* check to see how it ended */
        if (len != 0 && *p == delim)
        {
            /* 
             *   We ended with the matching delimiter.  Move past the
             *   closing delimiter. 
             */
            ++p;
            --len;

            /*
             *   If we have a pending close paren we need to put in
             *   because of an embedded expression that occurred earlier
             *   in the string, parse the macro to provide the paren.  
             */
            if ((ctx->tokcxflg & TOKCXF_EMBED_PAREN_AFT) != 0
                && !(ctx->tokcxflg & TOKCXFINMAC))
            {
                /* clear the flag */
                ctx->tokcxflg &= ~TOKCXF_EMBED_PAREN_AFT;

                /* push the current parsing position */
                if (ctx->tokcxmlvl >= TOKMACNEST)
                    errsig(ctx->tokcxerr, ERR_MACNEST);
                ctx->tokcxmsav[ctx->tokcxmlvl] = p;
                ctx->tokcxmsvl[ctx->tokcxmlvl] = len;
                ctx->tokcxmlvl++;

                /* parse the macro */
                p = tokmac4;
                len = strlen(p);
            }
        }
        else if (len != 0 && *p == '<')
        {
            /* save the current parsing position */
            if (ctx->tokcxmlvl >= TOKMACNEST)
                errsig(ctx->tokcxerr, ERR_MACNEST);
            ctx->tokcxmsav[ctx->tokcxmlvl] = p + 2;
            ctx->tokcxmsvl[ctx->tokcxmlvl] = len - 2;
            ctx->tokcxmlvl++;

            /* read from the "<<" expansion */
            p = tokmac1;
            len = strlen(p);
            ctx->tokcxflg |= TOKCXFINMAC;

            /* 
             *   Set the special push-a-paren flag: we'll return an open
             *   paren now, so that we have an open paren before the
             *   string, and then on the next call to toknext() we'll
             *   immediately return the string we've already parsed here.
             *   This will ensure that everything in the string is
             *   properly grouped together as a single indivisible
             *   expression.
             *   
             *   Note that we only need to do this for the first embedded
             *   expression in a string.  Once we have a close paren
             *   pending, we don't need more open parens.  
             */
            if (!(ctx->tokcxflg & TOKCXF_EMBED_PAREN_AFT))
            {
                ctx->tokcxflg |= TOKCXF_EMBED_PAREN_PRE;
                tok->toktyp = TOKTLPAR;
            }
        }
        goto done;
    }
    else if (len >= 2 && *p == '>' && *(p+1) == '>'
             && (ctx->tokcxflg & TOKCXFINMAC) != 0)
    {
        /* skip the ">>" */
        ctx->tokcxflg &= ~TOKCXFINMAC;
        p += 2;
        len -= 2;

        /* save the current parsing position */
        if (ctx->tokcxmlvl >= TOKMACNEST)
            errsig(ctx->tokcxerr, ERR_MACNEST);
        ctx->tokcxmsav[ctx->tokcxmlvl] = p;
        ctx->tokcxmsvl[ctx->tokcxmlvl] = len;
        ctx->tokcxmlvl++;

        if (*p == '"')
        {
            ++(ctx->tokcxmsav[ctx->tokcxmlvl - 1]);
            --(ctx->tokcxmsvl[ctx->tokcxmlvl - 1]);
            p = tokmac3;

            /* 
             *   we won't need an extra closing paren now, since tokmac3
             *   provides it 
             */
            ctx->tokcxflg &= ~TOKCXF_EMBED_PAREN_AFT;
        }
        else
        {
            /* 
             *   The string is continuing.  Set a flag to note that we
             *   need to provide a close paren after the end of the
             *   string, and parse the glue (tokmac2) that goes between
             *   the expression and the resumption of the string. 
             */
            ctx->tokcxflg |= TOKCXF_EMBED_PAREN_AFT;
            p = tokmac2;
        }

        len = strlen(p);
        goto nexttoken;
    }
    else
    {
        tokscdef *sc;
        
        for (sc = ctx->tokcxsc[ctx->tokcxinx[(uchar)*p]] ; sc ;
             sc = sc->tokscnxt)
        {
            if (toksceq(sc->tokscstr, p, sc->toksclen, len))
            {
                tok->toktyp = sc->toksctyp;
                p += sc->toksclen;
                len -= sc->toksclen;
                goto done;
            }
        }
        errsig(ctx->tokcxerr, ERR_INVTOK);
    }
    
done:
    ctx->tokcxptr = p;
    ctx->tokcxlen = len;
    return(tok->toktyp);
}

/* initialize a linear symbol table */
void toktlini(errcxdef *errctx, toktldef *toktab, uchar *mem, uint siz)
{
    CLRSTRUCT(*toktab);
    
    /* initialize superclass data */
    toktab->toktlsc.toktfadd = toktladd;           /* set add-symbol method */
    toktab->toktlsc.toktfsea = toktlsea;         /* set search-table method */
    toktab->toktlsc.toktfeach = toktleach;             /* set 'each' method */
    toktab->toktlsc.toktfset = toktlset;             /* set 'update' method */
    toktab->toktlsc.tokterr = errctx;         /* set error handling context */
    
    /* initialize class data */
    toktab->toktlptr = mem;
    toktab->toktlnxt = mem;
    toktab->toktlsiz = siz;
}

/* add a symbol to a linear symbol table */
void toktladd(toktdef *toktab1, char *name, int namel,
              int typ, int val, int hash)
{
    uint      siz = sizeof(toks1def) + namel;
    toksdef  *sym;
    toktldef *toktab = (toktldef *)toktab1;
    
    VARUSED(hash);
    
    if (toktab->toktlsiz < siz)
        errsig(toktab->toktlsc.tokterr, ERR_NOLCLSY);
    
    sym = (toksdef *)toktab->toktlnxt;
    siz = osrndsz(siz);
    toktab->toktlnxt += siz;
    if (siz > toktab->toktlsiz) toktab->toktlsiz = 0;
    else toktab->toktlsiz -= siz;

    /* set up symbol */
    sym->toksval = val;
    sym->tokslen = namel;
    sym->tokstyp = typ;
    sym->toksfr  = 0;
    memcpy(sym->toksnam, name, (size_t)(namel + 1));
    
    /* indicate there's one more symbol in the table */
    ++(toktab->toktlcnt);
}

/* delete all symbols from a linear symbol table */
void toktldel(toktldef *tab)
{
    tab->toktlcnt = 0;
    tab->toktlsiz += tab->toktlnxt - tab->toktlptr;
    tab->toktlnxt = tab->toktlptr;
}

/* call a function for every symbol in a linear symbol table */
void toktleach(toktdef *tab1,
               void (*cb)(void *ctx, toksdef *sym), void *ctx)
{
    toksdef  *p;
    uint      cnt;
    toktldef *tab = (toktldef *)tab1;
    
    for (p = (toksdef *)tab->toktlptr, cnt = tab->toktlcnt ; cnt ; --cnt )
    {
        (*cb)(ctx, p);
        p = (toksdef *)(((uchar *)p)
                        + osrndsz(p->tokslen + sizeof(toks1def)));
    }
}

/* search a linear symbol table */
int toktlsea(toktdef *tab1, char *name, int namel, int hash, toksdef *ret)
{
    toksdef  *p;
    uint      cnt;
    toktldef *tab = (toktldef *)tab1;
    
    VARUSED(hash);
    
    for (p = (toksdef *)tab->toktlptr, cnt = tab->toktlcnt ; cnt ; --cnt )
    {
        if (p->tokslen == namel && !memcmp(p->toksnam, name, (size_t)namel))
        {
            memcpy(ret, p, (size_t)(sizeof(toks1def) + namel));
            return(TRUE);
        }
        
        p = (toksdef *)(((uchar *)p)
                        + osrndsz(p->tokslen + sizeof(toks1def)));
    }

    /* nothing found - indicate by returning FALSE */
    return(FALSE);
}

/* update a symbol in a linear symbol table */
void toktlset(toktdef *tab1, toksdef *newsym)
{
    toksdef  *p;
    uint      cnt;
    toktldef *tab = (toktldef *)tab1;
    
    for (p = (toksdef *)tab->toktlptr, cnt = tab->toktlcnt ; cnt ; --cnt )
    {
        if (p->tokslen == newsym->tokslen
            && !memcmp(p->toksnam, newsym->toksnam, (size_t)newsym->tokslen))
        {
            p->toksval = newsym->toksval;
            p->tokstyp = newsym->tokstyp;
            return;
        }
        
        p = (toksdef *)(((uchar *)p)
                        + osrndsz(p->tokslen + sizeof(toks1def)));
    }
}

tokcxdef *tokcxini(errcxdef *errctx, mcmcxdef *mcmctx, tokldef *sctab)
{
    int       i;
    int       cnt;
    tokldef  *p;
    uchar     c;
    uchar     index[256];
    tokcxdef *ret;
    tokscdef *sc;
    ushort    siz;
    
    /* set up index table: finds tokcxsc entry from character value */
    memset(index, 0, (size_t)sizeof(index));
    for (i = cnt = 0, p = sctab ; (c = p->toklstr[0]) != 0 ; ++cnt, ++p)
        if (!index[c]) index[c] = ++i;
    
    /* allocate memory for table plus the tokscdef's */
    siz = sizeof(tokcxdef) + (i * sizeof(tokscdef *))
          + ((cnt + 1) * sizeof(tokscdef));
    ret = (tokcxdef *)mchalo(errctx, siz, "tokcxini");
    memset(ret, 0, (size_t)siz);
    
    /* copy the index, set up fixed part */
    memcpy(ret->tokcxinx, index, sizeof(ret->tokcxinx));
    ret->tokcxerr = errctx;
    ret->tokcxmem = mcmctx;

    /* start out without an #if */
    ret->tokcxifcur = TOKIF_IF_YES;
    
    /* force the first toknext() to read a line */
    ret->tokcxptr = "\000";
    
    /* figure where the tokscdef's go (right after sc pointer array) */
    sc = (tokscdef *)&ret->tokcxsc[i+1];
    
    /* set up the individual tokscdef entries, and link into lists */
    for (p = sctab ; (c = p->toklstr[0]) != 0 ; ++p, ++sc)
    {
        size_t len;
        
        sc->toksctyp = p->tokltyp;
        len = sc->toksclen = strlen(p->toklstr);
        memcpy(sc->tokscstr, p->toklstr, len);
        sc->tokscnxt = ret->tokcxsc[index[c]];
        ret->tokcxsc[index[c]] = sc;
    }
    
    return(ret);
}

/* add an include path to a tokdef */
void tokaddinc(tokcxdef *ctx, char *path, int pathlen)
{
    tokpdef *newpath;
    tokpdef *last;
    
    /* find the tail of the include path list, if any */
    for (last = ctx->tokcxinc ; last && last->tokpnxt ;
         last = last->tokpnxt) ;
    
    /* allocate storage for and set up a new path structure */
    newpath = (tokpdef *)mchalo(ctx->tokcxerr,
                                (sizeof(tokpdef) + pathlen - 1),
                                "tokaddinc");
    newpath->tokplen = pathlen;
    newpath->tokpnxt = (tokpdef *)0;
    memcpy(newpath->tokpdir, path, (size_t)pathlen);
    
    /* link in at end of list (if no list yet, newpath becomes first entry) */
    if (last)
        last->tokpnxt = newpath;
    else
        ctx->tokcxinc = newpath;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
