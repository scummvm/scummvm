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

/* hashed symbol table manipulation functions
 *
 * Implements hashed symbol tables.  A hashed symbol table stores
 * a table of pointers to linked lists of symbols; each entry in
 * the table corresponds to a hash value, allowing a large table
 * to be searched for a symbol rapidly.
 * 
 * Notes: Separated from tokenizer.cpp to allow the run-time to link the hashed
 * symbol table functions without needing to link the rest of the
 * lexical analysis subsystem.
 */

#include "glk/tads/tads2/tokenizer.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* compute a hash value */
uint tokhsh(char *nam)
{
    uint hash = 0;
    
    while (*nam) hash = ((hash + *nam++) & (TOKHASHSIZE - 1));
    return(hash);
}

/* for allocation - size of tokshdef without name portion */
struct toksh1def
{
    tokthpdef tokshnxt;
    toks1def  tokshsc;
};
typedef struct toksh1def toksh1def;

/* initialize a hashed symbol table */
void tokthini(errcxdef *errctx, mcmcxdef *memctx, toktdef *toktab1)
{
    tokthdef *toktab = (tokthdef *)toktab1;      /* convert to correct type */
    int       i;

    CLRSTRUCT(*toktab);
    toktab->tokthsc.toktfadd = tokthadd;           /* set add-symbol method */
    toktab->tokthsc.toktfsea = tokthsea;         /* set search-table method */
    toktab->tokthsc.toktfset = tokthset;                   /* update symbol */
    toktab->tokthsc.toktfeach = toktheach;       /* call fn for all symbols */
    toktab->tokthsc.tokterr = errctx;         /* set error handling context */
    toktab->tokthmem = memctx;                    /* memory manager context */
    toktab->tokthcpool = mcmalo(memctx, (ushort)TOKTHSIZE,
                                &toktab->tokthpool[0]);
    toktab->tokthpcnt = 0;
    toktab->tokthsize = TOKTHSIZE;

    /* set hash table entries to point to nothing (MCMONINV) */
    for (i = 0 ; i < TOKHASHSIZE ; ++i)
        toktab->tokthhsh[i].tokthpobj = MCMONINV;
}

/* add a symbol to a hashed symbol table */
void tokthadd(toktdef *toktab1, char *name, int namel,
              int typ, int val, int hash)
{
    int       siz = sizeof(toksh1def) + namel;
    toksdef  *sym;
    tokshdef *symh;
    tokthdef *toktab = (tokthdef *)toktab1;
    
    if (toktab->tokthsize < siz)
    {
        mcmcxdef *mctx = toktab->tokthmem;
        
        /* insufficient space in current pool; add a new pool */
        if (toktab->tokthpcnt >= TOKPOOLMAX)
            errsig(toktab->tokthsc.tokterr, ERR_MANYSYM);

        /* unlock current pool page, and note its final size */
        mcmunlck(mctx, toktab->tokthpool[toktab->tokthpcnt]);
        toktab->tokthfinal[toktab->tokthpcnt] = toktab->tokthofs;

        /* allocate a new pool page, and leave it locked */
        toktab->tokthcpool = mcmalo(mctx, (ushort)TOKTHSIZE,
                                   &toktab->tokthpool[++(toktab->tokthpcnt)]);
        toktab->tokthsize = TOKTHSIZE;
        toktab->tokthofs = 0;
    }
    symh = (tokshdef *)(toktab->tokthcpool + toktab->tokthofs);
    sym = &symh->tokshsc;
    
    /* link into list for this hash value */
    OSCPYSTRUCT(symh->tokshnxt, toktab->tokthhsh[hash]);
    toktab->tokthhsh[hash].tokthpobj = toktab->tokthpool[toktab->tokthpcnt];
    toktab->tokthhsh[hash].tokthpofs = toktab->tokthofs;
    
    /* fill in rest of toksdef */
    sym->toksval = val;
    sym->tokslen = namel;
    sym->tokstyp = typ;
    sym->tokshsh = hash;
    sym->toksfr  = 0;
    memcpy(sym->toksnam, name, (size_t)namel);
    
    /* update free pool pointer */
    siz = osrndsz(siz);
    toktab->tokthofs += siz;
    if (siz > toktab->tokthsize) toktab->tokthsize = 0;
    else toktab->tokthsize -= siz;
}

/*
 *   Scan a hash chain, calling a callback for each entry.  If the
 *   callback returns TRUE for any symbol, we stop there, and return TRUE.
 *   If the callback returns FALSE for a symbol, we keep going.  If the
 *   callback returns FALSE for all symbols, we return FALSE.  
 */
static int tokthscan(tokthdef *tab, uint hash,
                     int (*cb)(void *, toksdef *, mcmon), 
                     void *cbctx)
{
    tokshdef  *symh;
    toksdef   *sym;
    tokthpdef  p;
    tokthpdef  nxt;
	uchar     *pg = nullptr;
    mcmcxdef  *mctx = tab->tokthmem;
    mcmon      curobj;

    /* get first object, and lock its page if there is one */
    OSCPYSTRUCT(p, tab->tokthhsh[hash]);
    if ((curobj = p.tokthpobj) != MCMONINV)
        pg = mcmlck(mctx, curobj);

    /* look for a match using the callback */
    for ( ; p.tokthpobj != MCMONINV ; OSCPYSTRUCT(p, nxt))
    {
        symh = (tokshdef *)(pg + p.tokthpofs);
        sym = &symh->tokshsc;
        OSCPYSTRUCT(nxt, symh->tokshnxt);

        /* check for a match; copy to return buffer if found */
        if ((*cb)(cbctx, sym, p.tokthpobj))
        {
            mcmunlck(mctx, p.tokthpobj);
            return(TRUE);
        }
        
        /* if the next page is different from this one, get new lock */
        if (nxt.tokthpobj != curobj && nxt.tokthpobj != MCMONINV)
        {
            mcmunlck(mctx, curobj);
            curobj = nxt.tokthpobj;
            pg = mcmlck(mctx, curobj);
        }
    }

    /* unlock last object, if we had a lock at all */
    if (curobj != MCMONINV) mcmunlck(mctx, curobj);
    return(FALSE);
}

struct tokseadef
{
    char     *tokseanam;
    toksdef   tokseasym;
    toksdef  *toksearet;
    mcmcxdef *tokseamctx;
};
typedef struct tokseadef tokseadef;

/* search callback */
static int tokthsea1(void *ctx0, toksdef *sym, mcmon objn)
{
    tokseadef *ctx = (tokseadef *)ctx0;
    
    VARUSED(objn);
    
    if (sym->tokslen == ctx->tokseasym.tokslen &&
        !memcmp(sym->toksnam, ctx->tokseanam, ctx->tokseasym.tokslen))
    {
        memcpy(ctx->toksearet, sym,
               (size_t)(sizeof(toks1def) + ctx->tokseasym.tokslen));
        return(TRUE);
    }
    else
        return(FALSE);
}

/* search a hashed symbol table for a symbol */
int tokthsea(toktdef *tab1, char *name, int namel, int hash, toksdef *ret)
{
    tokseadef ctx;

    ctx.tokseanam = name;
    ctx.tokseasym.tokslen = namel;
    ctx.toksearet = ret;
    return(tokthscan((tokthdef *)tab1, hash, tokthsea1, &ctx));
}

/* callback for tokthset */
static int tokthset1(void *ctx0, toksdef *sym, mcmon objn)
{
    tokseadef *ctx = (tokseadef *)ctx0;
    
    if (sym->tokslen == ctx->tokseasym.tokslen
        && !memcmp(sym->toksnam, ctx->tokseasym.toksnam,
                   ctx->tokseasym.tokslen))
    {
        sym->toksval = ctx->tokseasym.toksval;
        sym->tokstyp = ctx->tokseasym.tokstyp;
            
        /* touch object, since it's been changed */
        mcmtch(ctx->tokseamctx, objn);
        return(TRUE);
    }
    else
        return(FALSE);
}

/* update a symbol in a hashed symbol table */
void tokthset(toktdef *tab1, toksdef *newsym)
{
    tokseadef  ctx;
    tokthdef  *tab = (tokthdef *)tab1;
    
    OSCPYSTRUCT(ctx.tokseasym, *newsym);
    ctx.tokseamctx = tab->tokthmem;
    tokthscan((tokthdef *)tab1, newsym->tokshsh, tokthset1, &ctx);
}

/* callback for tokthfind */
static int tokthfind1(void *ctx0, toksdef *sym, mcmon objn)
{
    tokseadef *ctx = (tokseadef *)ctx0;
    
    VARUSED(objn);
    
    if (sym->toksval == ctx->tokseasym.toksval
        && sym->tokstyp == ctx->tokseasym.tokstyp)
    {
        memcpy(ctx->toksearet, sym,
               (size_t)(sizeof(toks1def) + sym->tokslen));
        return(TRUE);
    }
    else
        return(FALSE);
}

/* find a symbol of a particular type and value */
int tokthfind(toktdef *tab1, int typ, uint val, toksdef *ret)
{
    tokseadef ctx;
    int       i;
    
    ctx.tokseasym.tokstyp = typ;
    ctx.tokseasym.toksval = val;
    ctx.toksearet = ret;
    
    for (i = 0 ; i < TOKHASHSIZE ; ++i)
    {
        if (tokthscan((tokthdef *)tab1, i, tokthfind1, &ctx))
            return(TRUE);
    }
    return(FALSE);
}

/* call a callback for each function in a hashed symbol table */
void toktheach(toktdef *tab1,
               void (*cb)(void *, toksdef *), void *ctx)
{
    tokthdef *tab = (tokthdef *)tab1;
    uchar    *p;
    uint      max;
    uint      ofs;
    tokshdef *symh;
    toksdef  *sym;
    uint      siz;
    uint      i;
    
    for (i = 0 ; i <= tab->tokthpcnt ; ++i)
    {
        /* lock the current page */
        p = mcmlck(tab->tokthmem, tab->tokthpool[i]);
        
        ERRBEGIN(tab1->tokterr)

        max = (i == tab->tokthpcnt ? tab->tokthofs : tab->tokthfinal[i]);
        for (ofs = 0 ; ofs < max ; )
        {
            /* get this symbol */
            symh = (tokshdef *)(p + ofs);
            sym = &symh->tokshsc;
            
            /* call the user callback */
            (*cb)(ctx, sym);
            
            /* advance to the next symbol on this page */
            siz = sizeof(toksh1def) + sym->tokslen;
            ofs += osrndsz(siz);
        }
            
        ERRCLEAN(tab1->tokterr)
            mcmunlck(tab->tokthmem, tab->tokthpool[i]);
        ERRENDCLN(tab1->tokterr)
            
        /* done with current page; unlock it */
        mcmunlck(tab->tokthmem, tab->tokthpool[i]);
    }
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
