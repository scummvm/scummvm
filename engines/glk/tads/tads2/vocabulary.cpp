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

#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/*
 *   Main vocabulary context.  This can be saved globally if desired, so
 *   that routines that don't have any other access to it (such as
 *   Unix-style signal handlers) can reach it.
 */
voccxdef *main_voc_ctx = 0;

#ifdef VOCW_IN_CACHE
vocwdef *vocwget(voccxdef *ctx, uint idx)
{
    uint pg;
    
    if (idx == VOCCXW_NONE)
        return 0;

    /* get the page we need */
    pg = idx/VOCWPGSIZ;

    /* if it's not locked, lock it */
    if (pg != ctx->voccxwplck)
    {
        /* unlock the old page */
        if (ctx->voccxwplck != MCMONINV)
            mcmunlck(ctx->voccxmem, ctx->voccxwp[ctx->voccxwplck]);

        /* lock the new page */
        ctx->voccxwpgptr = (vocwdef *)mcmlck(ctx->voccxmem,
                                             ctx->voccxwp[pg]);
        ctx->voccxwplck = pg;
    }

    /* return the entry on that page */
    return ctx->voccxwpgptr + (idx % VOCWPGSIZ);
}
#endif /*VOCW_IN_CACHE */

/* hash value is based on first 6 characters only to allow match-in-6 */
uint vochsh(const uchar *t, int len)
{
    uint ret = 0;
    
    if (len > 6) len = 6;
    for ( ; len ; --len, ++t)
        ret = (ret + (uint)(vocisupper(*t) ? tolower(*t) : *t))
               & (VOCHASHSIZ - 1);
    return(ret);
}

/* copy vocabulary word, and convert to lower case */
static void voccpy(uchar *dst, const uchar *src, int len)
{
    for ( ; len ; --len, ++dst, ++src)
        *dst = vocisupper(*src) ? tolower(*src) : *src;
}

/* allocate and set up a new vocwdef record, linking into a vocdef's list */
static void vocwset(voccxdef *ctx, vocdef *v, prpnum p, objnum objn,
                    int classflg)
{
    vocwdef *vw;
    uint     inx;
    vocwdef *vw2;

    /*
     *   look through the vocdef list to see if there's an existing entry
     *   with the DELETED marker -- if so, simply undelete it 
     */
    for (inx = v->vocwlst, vw = vocwget(ctx, inx) ; vw ;
         inx = vw->vocwnxt, vw = vocwget(ctx, inx))
    {
        /* if this entry was deleted, and otherwise matches, undelete it */
        if ((vw->vocwflg & VOCFDEL)
            && vw->vocwobj == objn && vw->vocwtyp == p)
        {
            /*
             *   Remove the deleted flag.  We will otherwise leave the
             *   flags unchanged, since the VOCFDEL flag applies only to
             *   statically allocated objects, and hence the original
             *   flags should take precedence over any run-time flags. 
             */
            vw->vocwflg &= ~VOCFDEL;

            /* we're done */
            return;
        }
    }

    /* make sure the word+object+type record isn't already defined */
    for (inx = v->vocwlst, vw = vocwget(ctx, inx) ; vw ;
         inx = vw->vocwnxt, vw = vocwget(ctx, inx))
    {
        if (vw->vocwobj == objn && vw->vocwtyp == p
            && (vw->vocwflg & VOCFCLASS) == (classflg & VOCFCLASS))
        {
            /* it matches - don't add a redundant record */
            return;
        }
    }
    
    /* look in the free list for an available vocwdef */
    if (ctx->voccxwfre != VOCCXW_NONE)
    {
        inx = ctx->voccxwfre;
        vw = vocwget(ctx, inx);                     /* get the free vocwdef */
        ctx->voccxwfre = vw->vocwnxt;              /* unlink from free list */
    }
    else
    {
        /* allocate another page of vocwdef's if necssary */
        if ((ctx->voccxwalocnt % VOCWPGSIZ) == 0)
        {
            int pg = ctx->voccxwalocnt / VOCWPGSIZ;
            
            /* make sure we haven't exceeded the available page count */
            if (pg >= VOCWPGMAX) errsig(ctx->voccxerr, ERR_VOCMNPG);
            
            /* allocate on the new page */
#ifdef VOCW_IN_CACHE
            mcmalo(ctx->voccxmem, (ushort)(VOCWPGSIZ * sizeof(vocwdef)),
                                           &ctx->voccxwp[pg]);
            mcmunlck(ctx->voccxmem, ctx->voccxwp[pg]);
#else
            ctx->voccxwp[pg] =
                (vocwdef *)mchalo(ctx->voccxerr,
                                  (VOCWPGSIZ * sizeof(vocwdef)),
                                  "vocwset");
#endif
        }

        /* get the next entry, and increment count of used entries */
        inx = ctx->voccxwalocnt++;
        vw = vocwget(ctx, inx);
    }

    /* link the new vocwdef into the vocdef's relation list */
    vw->vocwnxt = v->vocwlst;
    v->vocwlst = inx;

    /* set up the new vocwdef */
    vw->vocwtyp = (uchar)p;
    vw->vocwobj = objn;
    vw->vocwflg = classflg;

    /* 
     *   Scan the list and make sure we're not adding a redundant verb.
     *   Don't bother with the warning if this is a class. 
     */
    if (p == PRP_VERB && (ctx->voccxflg & VOCCXFVWARN)
        && (vw->vocwflg & VOCFCLASS) == 0)
    {
        for (vw2 = vocwget(ctx, v->vocwlst) ; vw2 ;
             vw2 = vocwget(ctx, vw2->vocwnxt))
        {
            /* 
             *   if this is a different object, and it's not a class, and
             *   it's defined as a verb, warn about it 
             */
            if (vw2 != vw
                && (vw2->vocwflg & VOCFCLASS) == 0
                && vw2->vocwtyp == PRP_VERB)
            {
                if (v->vocln2 != 0)
                    errlog2(ctx->voccxerr, ERR_VOCREVB,
                            ERRTSTR,
                            errstr(ctx->voccxerr,
                                   (char *)v->voctxt, v->voclen),
                            ERRTSTR,
                            errstr(ctx->voccxerr,
                                   (char *)v->voctxt + v->voclen, v->vocln2));
                else
                    errlog1(ctx->voccxerr, ERR_VOCREVB,
                            ERRTSTR,
                            errstr(ctx->voccxerr,
                                   (char *)v->voctxt, v->voclen));
                break;
            }
        }
    }
}

/* set up a vocdef record, and link into hash table */
static void vocset(voccxdef *ctx, vocdef *v, prpnum p, objnum objn,
                   int classflg, uchar *wrdtxt, int len,
                   uchar *wrd2, int len2)
{
    uint hshval = vochsh(wrdtxt, len);
    
    v->vocnxt = ctx->voccxhsh[hshval];
    ctx->voccxhsh[hshval] = v;
    
    v->voclen = len;
    v->vocln2 = len2;
    voccpy(v->voctxt, wrdtxt, len);
    if (wrd2) voccpy(v->voctxt + len, wrd2, len2);

    /* allocate and initialize a vocwdef for the object */
    vocwset(ctx, v, p, objn, classflg);
}

/* internal addword - already parsed into two words and have lengths */
void vocadd2(voccxdef *ctx, prpnum p, objnum objn, int classflg,
             uchar *wrdtxt, int len, uchar *wrd2, int len2)
{
    vocdef  *v;
    vocdef  *prv;
    uint     need;
    uint     hshval;

    /* if the word is null, ignore it entirely */
    if (len == 0 && len2 == 0)
        return;

    /* look for a vocdef entry with the same word text */
    hshval = vochsh(wrdtxt, len);
    for (v = ctx->voccxhsh[hshval] ; v ; v = v->vocnxt)
    {
        /* if it matches on both words, use this entry */
        if (v->voclen == len && !memcmp(wrdtxt, v->voctxt, (size_t)len)
            && ((!wrd2 && v->vocln2 == 0)
                || (v->vocln2 == len2 &&
                    !memcmp(wrd2, v->voctxt + len, (size_t)len2))))
        {
            vocwset(ctx, v, p, objn, classflg);
            return;
        }
    }

    /* look for a free vocdef entry of the same size */
    for (prv = (vocdef *)0, v = ctx->voccxfre ; v ; prv = v, v = v->vocnxt)
        if (v->voclen == len + len2) break;
    
    if (v)
    {
        /* we found something - unlink from free list */
        if (prv) prv->vocnxt = v->vocnxt;
        else ctx->voccxfre = v->vocnxt;
        
        /* reuse the entry */
        v->vocwlst = VOCCXW_NONE;
        vocset(ctx, v, p, objn, classflg, wrdtxt, len, wrd2, len2);
        return;
    }
    
    /* didn't find an existing vocdef; allocate a new one */
    need = sizeof(vocdef) + len + len2 - 1;
    if (ctx->voccxrem < need)
    {
        /* not enough space in current page; allocate a new one */
        ctx->voccxpool = mchalo(ctx->voccxerr, VOCPGSIZ, "vocadd2");
        ctx->voccxrem = VOCPGSIZ;
    }
    
    /* use top of current pool, and update pool pointer and size */
    v = (vocdef *)ctx->voccxpool;
    need = osrndsz(need);
    ctx->voccxpool += need;
    if (ctx->voccxrem > need) ctx->voccxrem -= need;
    else ctx->voccxrem = 0;
    
    /* set up new vocdef */
    v->vocwlst = VOCCXW_NONE;
    vocset(ctx, v, p, objn, classflg, wrdtxt, len, wrd2, len2);
}

static void voc_parse_words(char **wrdtxt, int *len, char **wrd2, int *len2)
{
    /* get length and pointer to actual text */
    *len = osrp2(*wrdtxt) - 2;
    *wrdtxt += 2;
    
    /* see if there's a second word - look for a space */
    for (*wrd2 = *wrdtxt, *len2 = *len ; *len2 && !vocisspace(**wrd2) ;
         ++*wrd2, --*len2) ;
    if (*len2)
    {
        *len -= *len2;
        while (*len2 && vocisspace(**wrd2)) ++*wrd2, --*len2;
    }
    else
    {
        /* no space ==> no second word */
        *wrd2 = (char *)0;
    }
}

void vocadd(voccxdef *ctx, prpnum p, objnum objn, int classflg, char *wrdtxt)
{
    int     len;
    char   *wrd2;
    int     len2;

    voc_parse_words(&wrdtxt, &len, &wrd2, &len2);
    vocadd2(ctx, p, objn, classflg, (uchar *)wrdtxt, len, (uchar *)wrd2, len2);
}

/* make sure we have a page table entry for an object, allocating one if not */
void vocialo(voccxdef *ctx, objnum obj)
{
    if (!ctx->voccxinh[obj >> 8])
    {
        ctx->voccxinh[obj >> 8] =
            (vocidef **)mchalo(ctx->voccxerr,
                               (256 * sizeof(vocidef *)), "vocialo");
        memset(ctx->voccxinh[obj >> 8], 0, (size_t)(256 * sizeof(vocidef *)));
    }
}

/* add an inheritance/location record */
void vociadd(voccxdef *ctx, objnum obj, objnum loc,
             int numsc, objnum *sc, int flags)
{
    vocidef *v;
    vocidef *min;
    vocidef *prv;
	vocidef *minprv = nullptr;

    /* make sure we have a page table entry for this object */
    vocialo(ctx, obj);

    /* look in free list for an entry that's big enough */
    for (prv = (vocidef *)0, min = (vocidef *)0, v = ctx->voccxifr ; v ;
         prv = v, v = v->vocinxt)
    {
        if (v->vocinsc == numsc)
        {
            min = v;
            minprv = prv;
            break;
        }
        else if (v->vocinsc > numsc)
        {
            if (!min || v->vocinsc < min->vocinsc)
            {
                min = v;
                minprv = prv;
            }
        }
    }
    
    if (!min)
    {
        uint need;
        
        /* nothing in free list; allocate a new entry */
        need = osrndsz(sizeof(vocidef) + (numsc - 1)*sizeof(objnum));
        if (ctx->voccxilst + need >= VOCISIZ)
        {
            /* nothing left on current page; allocate a new page */
            ctx->voccxip[++(ctx->voccxiplst)] =
                mchalo(ctx->voccxerr, VOCISIZ, "vociadd");
            ctx->voccxilst = 0;
        }

        /* allocate space out of current page */
        v = (vocidef *)(ctx->voccxip[ctx->voccxiplst] + ctx->voccxilst);
        ctx->voccxilst += need;
    }
    else
    {
        /* unlink from chain and use */
        v = min;
        if (minprv)
            minprv->vocinxt = v->vocinxt;
        else
            ctx->voccxifr = v->vocinxt;
    }

    /* set up the entry */
    if (vocinh(ctx, obj) != (vocidef *)0) errsig(ctx->voccxerr, ERR_VOCINUS);
    v->vociloc = loc;
    v->vociilc = MCMONINV;
    v->vociflg = (flags & ~VOCIFXLAT);
    v->vocinsc = numsc;
    if (numsc)
    {
        if (flags & VOCIFXLAT)
        {
            int i;
            
            for (i = 0 ; i < numsc ; ++i)
                v->vocisc[i] = osrp2(&sc[i]);
        }
        else
            memcpy(v->vocisc, sc, (size_t)(numsc * sizeof(objnum)));
    }
    vocinh(ctx, obj) = v;                           /* set page table entry */
}

/* revert all objects to original state, using inheritance records */
void vocrevert(voccxdef *vctx)
{
    vocidef ***vpg;
    vocidef  **v;
    int        i;
    int        j;
    objnum     obj;

    /*
     *   Go through the inheritance records.  Delete each dynamically
     *   allocated object, and revert each static object to its original
     *   load state. 
     */
    for (vpg = vctx->voccxinh, i = 0 ; i < VOCINHMAX ; ++vpg, ++i)
    {
        if (!*vpg) continue;
        for (v = *vpg, obj = (i << 8), j = 0 ; j < 256 ; ++v, ++obj, ++j)
        {
            if (*v)
            {
                /* if the object was dynamically allocated, delete it */
                if ((*v)->vociflg & VOCIFNEW)
                {
                    /* delete vocabulary and inheritance data for the object */
                    vocidel(vctx, obj);
                    vocdel(vctx, obj);

                    /* delete the object */
                    mcmfre(vctx->voccxmem, (mcmon)obj);
                }
                else
                {
                    /* revert the object */
                    mcmrevert(vctx->voccxmem, (mcmon)obj);
                }
            }
        }
    }

    /*
     *   Revert the vocabulary list: delete all newly added words, and
     *   undelete all original words marked as deleted.  
     */
    vocdel1(vctx, MCMONINV, (char *)0, 0, TRUE, TRUE, FALSE);
}

/* initialize voc context */
void vocini(voccxdef *vocctx, errcxdef *errctx, mcmcxdef *memctx,
            runcxdef *runctx, objucxdef *undoctx,
            int fuses, int daemons, int notifiers)
{
    CLRSTRUCT(*vocctx);
    vocctx->voccxerr = errctx;
    vocctx->voccxiplst = (uint)-1;
    vocctx->voccxilst = VOCISIZ;
    vocctx->voccxmem = memctx;
    vocctx->voccxrun = runctx;
    vocctx->voccxundo = undoctx;

    vocctx->voccxme  =
    vocctx->voccxme_init = 
    vocctx->voccxvtk =
    vocctx->voccxstr =
    vocctx->voccxnum =
    vocctx->voccxit  =
    vocctx->voccxhim =
    vocctx->voccxprd =
    vocctx->voccxpre =
    vocctx->voccxpre2 =
    vocctx->voccxppc =
    vocctx->voccxlsv =
    vocctx->voccxpreinit =
    vocctx->voccxper =
    vocctx->voccxprom =
    vocctx->voccxpostprom =
    vocctx->voccxpdis =
    vocctx->voccxper2 =
    vocctx->voccxperp =
    vocctx->voccxpdef =
    vocctx->voccxpdef2 =
    vocctx->voccxpask =
    vocctx->voccxpask2 =
    vocctx->voccxpask3 =
    vocctx->voccxinitrestore =
    vocctx->voccxpuv =
    vocctx->voccxpnp =
    vocctx->voccxpostact =
    vocctx->voccxendcmd =
    vocctx->voccxher = MCMONINV;
    vocctx->voccxthc = 0;
#ifdef VOCW_IN_CACHE
    vocctx->voccxwplck = MCMONINV;
#endif

    vocctx->voccxactor = MCMONINV;
    vocctx->voccxverb = MCMONINV;
    vocctx->voccxprep = MCMONINV;
    vocctx->voccxdobj = 0;
    vocctx->voccxiobj = 0;

    vocctx->voccxunknown = 0;
    vocctx->voccxlastunk = 0;

    vocctx->voc_stk_ptr = 0;
    vocctx->voc_stk_cur = 0;
    vocctx->voc_stk_end = 0;

    /* allocate fuses, daemons, notifiers */
    vocinialo(vocctx, &vocctx->voccxfus, (vocctx->voccxfuc = fuses));
    vocinialo(vocctx, &vocctx->voccxdmn, (vocctx->voccxdmc = daemons));
    vocinialo(vocctx, &vocctx->voccxalm, (vocctx->voccxalc = notifiers));

    /* no entries in vocwdef free list yet */
    vocctx->voccxwfre = VOCCXW_NONE;
}

/* uninitialize the voc context */
void vocterm(voccxdef *ctx)
{
    /* delete the fuses, daemons, and notifiers */
    voctermfree(ctx->voccxfus);
    voctermfree(ctx->voccxdmn);
    voctermfree(ctx->voccxalm);

    /* delete the private stack */
    if (ctx->voc_stk_ptr != 0)
        mchfre(ctx->voc_stk_ptr);
}

/* clean up the vocab context */
void voctermfree(vocddef *what)
{
    if (what != 0)
        mchfre(what);
}

/*
 *   Iterate through all words for a particular object, calling a
 *   function with each vocwdef found.  If objn == MCMONINV, we'll call
 *   the callback for every word.  
 */
void voc_iterate(voccxdef *ctx, objnum objn,
                 void (*fn)(void *, vocdef *, vocwdef *), void *fnctx)
{
    int       i;
    vocdef   *v;
    vocdef  **vp;
    vocwdef  *vw;
    uint      idx;

    /* go through each hash value looking for matching words */
    for (i = VOCHASHSIZ, vp = ctx->voccxhsh ; i ; ++vp, --i)
    {
        /* go through all words in this hash chain */
        for (v = *vp ; v ; v = v->vocnxt)
        {
            /* go through each object relation for this word */
            for (idx = v->vocwlst, vw = vocwget(ctx, idx) ; vw ;
                 idx = vw->vocwnxt, vw = vocwget(ctx, idx))
            {
                /*
                 *   if this word is for this object, call the callback
                 */
                if (objn == MCMONINV || vw->vocwobj == objn)
                    (*fn)(fnctx, v, vw);
            }
        }
    }
}

/* callback context for voc_count */
struct voc_count_ctx
{
    int     cnt;
    int     siz;
    prpnum  prp;
};

/* callback for voc_count */
static void voc_count_cb(void *ctx0, vocdef *voc, vocwdef *vocw)
{
    struct voc_count_ctx *ctx = (struct voc_count_ctx *)ctx0;
    
    VARUSED(vocw);

    /*
     *   If it matches the property (or we want all properties), count
     *   it.  Don't count deleted objects. 
     */
    if ((ctx->prp == 0 || ctx->prp == vocw->vocwtyp)
        && !(vocw->vocwflg & VOCFDEL))
    {
        /* count the word */
        ctx->cnt++;
        
        /* count the size */
        ctx->siz += voc->voclen + voc->vocln2;
    }
}

/*
 *   Get the number and size of words defined for an object.  The size
 *   returns the total byte count from all the words involved.  Do not
 *   include deleted words in the count.  
 */
void voc_count(voccxdef *ctx, objnum objn, prpnum prp, int *cnt, int *siz)
{
    struct voc_count_ctx fnctx;

    /* set up the context with zero initial counts */
    fnctx.cnt = 0;
    fnctx.siz = 0;
    fnctx.prp = prp;

    /* iterate over all words for the object with our callback */
    voc_iterate(ctx, objn, voc_count_cb, &fnctx);

    /* return the data */
    if (cnt) *cnt = fnctx.cnt;
    if (siz) *siz = fnctx.siz;
}


/*
 *   Delete a particular word associated with an object, or all words if
 *   the word pointer is null.  If the word isn't marked as added at
 *   runtime (i.e., the VOCFNEW flag is not set for the word), the word is
 *   simply marked as deleted, rather than being actually deleted.
 *   However, if the 'really_delete' flag is set, the word is actually
 *   deleted.  If the 'revert' flag is true, this routine deletes _every_
 *   dynamically created word, and undeletes all dynamically deleted words
 *   that were in the original vocabulary.  
 */
void vocdel1(voccxdef *ctx, objnum objn, char *wrd1, prpnum prp,
             int really_delete, int revert, int keep_undo)
{
    int       i;
    vocdef   *v;
    vocdef   *prv;
    vocdef   *nxt;
    vocdef  **vp;
    vocwdef  *vw;
    vocwdef  *prvw;
    vocwdef  *nxtw;
    uint      nxtidx;
    uint      idx;
    int       deleted_vocdef;
	char     *wrd2 = nullptr;
    int       len1 = 0, len2 = 0;
    int       do_del;
    char     *orgwrd;

    /* parse the word if provided */
    orgwrd = wrd1;
    if (wrd1)
        voc_parse_words(&wrd1, &len1, &wrd2, &len2);
    
    /* go through each hash value looking for matching words */
    for (i = VOCHASHSIZ, vp = ctx->voccxhsh ; i ; ++vp, --i)
    {
        /* go through all words in this hash chain */
        for (prv = (vocdef *)0, v = *vp ; v ; v = nxt)
        {
            /* remember next word in hash chain */
            nxt = v->vocnxt;

            /* if this word doesn't match, skip it */
            if (wrd1)
            {
                /* compare the first word */
                if (v->voclen != len1
                    || memicmp((char *)v->voctxt, wrd1, (size_t)len1))
                {
                    prv = v;
                    continue;
                }

                /* if there's a second word, compare it as well */
                if (wrd2 && (v->vocln2 != len2
                             || memicmp((char *)v->voctxt + len1,
                                        wrd2, (size_t)len2)))
                {
                    prv = v;
                    continue;
                }
            }

            /* presume we're not going to delete this vocdef */
            deleted_vocdef = FALSE;
            
            /* go through all object relations for this word */
            for (prvw = 0, idx = v->vocwlst, vw = vocwget(ctx, idx) ; vw ;
                 vw = nxtw, idx = nxtidx)
            {
                /* remember next word in relation list */
                nxtidx = vw->vocwnxt;
                nxtw = vocwget(ctx, nxtidx);

                /*
                 *   figure out whether to delete this word, based on the
                 *   caller's specified operating mode 
                 */
                if (revert)
                {
                    /* if reverting, delete all new words */
                    do_del = (vw->vocwflg & VOCFNEW);

                    /* also, remove the DELETED flag if present */
                    vw->vocwflg &= ~VOCFDEL;
                }
                else
                {
                    /*
                     *   delete the word if the object number matches,
                     *   AND either we're not searching for a specific
                     *   vocabulary word (in which case wrd1 will be null)
                     *   or the word matches the vocabulary word we're
                     *   seeking (in which case the part of speech must
                     *   match) 
                     */
                    do_del = (vw->vocwobj == objn
                              && (wrd1 == 0 || vw->vocwtyp == prp));

                    /*
                     *   if we're not in really_delete mode, and the word
                     *   matches and it wasn't dynamically added at
                     *   run-time, simply mark it as deleted -- this will
                     *   allow it to be undeleted if the game is reverted
                     *   to RESTART conditions 
                     */
                    if (do_del && !really_delete && !(vw->vocwflg & VOCFNEW))
                    {
                        /* geneate undo for the operation */
                        if (keep_undo && orgwrd)
                            vocdusave_delwrd(ctx, objn, prp,
                                             vw->vocwflg, orgwrd);
                        
                        /* just mark the word for deletion, but keep vw */
                        vw->vocwflg |= VOCFDEL;
                        do_del = FALSE;
                    }
                }

                /* now delete the structure if we decided we should */
                if (do_del)
                {
                    /* geneate undo for the operation */
                    if (keep_undo && orgwrd)
                        vocdusave_delwrd(ctx, objn, prp, vw->vocwflg, orgwrd);
                                         
                    /* unlink this vocwdef from the vocdef's list */
                    if (prvw)
                        prvw->vocwnxt = vw->vocwnxt;
                    else
                        v->vocwlst = vw->vocwnxt;

                    /* link the vocwdef into the vocwdef free list */
                    vw->vocwnxt = ctx->voccxwfre;
                    ctx->voccxwfre = idx;

                    /*
                     *   if there's nothing left in the vocdef's list,
                     *   delete the entire vocdef as well 
                     */
                    if (v->vocwlst == VOCCXW_NONE)
                    {
                        if (prv) prv->vocnxt = v->vocnxt;
                        else *vp = v->vocnxt;
                    
                        /* link into free chain */
                        v->vocnxt = ctx->voccxfre;
                        ctx->voccxfre = v;

                        /* note that it's been deleted */
                        deleted_vocdef = TRUE;
                    }
                }
                else
                {
                    /* we're not deleting the word, so move prvw forward */
                    prvw = vw;
                }
            }

            /* if we didn't delete this vocdef, move prv forward onto it */
            if (!deleted_vocdef)
                prv = v;
        }
    }
}

/* delete all vocabulary for an object */
void vocdel(voccxdef *ctx, objnum objn)
{
    vocdel1(ctx, objn, (char *)0, (prpnum)0, TRUE, FALSE, FALSE);
}

/* delete object inheritance records for a particular object */
void vocidel(voccxdef *ctx, objnum obj)
{
    vocidef *v;
    
    /* get entry out of page table, and clear page table slot */
    v = vocinh(ctx, obj);
    vocinh(ctx, obj) = (vocidef *)0;
    
    /* link into free list */
    if (v)
    {
        v->vocinxt = ctx->voccxifr;
        ctx->voccxifr = v;
    }
}

/*
 *   Find template matching a verb+object+prep combination; return TRUE
 *   if a suitable template is found, FALSE otherwise.
 */
int voctplfnd(voccxdef *ctx, objnum verb_in, objnum prep,
              uchar *tplout, int *newstyle)
{
    uchar  *tplptr;
    uchar  *thistpl;
    int     found;
    int     tplcnt;
    uint    tplofs;
    objnum  verb;

    /* look for a new-style template first */
    tplofs = objgetap(ctx->voccxmem, verb_in, PRP_TPL2, &verb, FALSE);
    if (tplofs)
    {
        /* flag the presence of the new-style template */
        *newstyle = TRUE;
    }
    else
    {
        /* no new-style template - look for old version */
        tplofs = objgetap(ctx->voccxmem, verb_in, PRP_TPL, &verb, FALSE);
        *newstyle = FALSE;
    }

    /* inherit templates until we run out of them */
    for (;;)
    {
        /* if we found something already, use it */
        if (tplofs)
        {
            size_t siz;

            /* figure the size of this template style */
            siz = (*newstyle ? VOCTPL2SIZ : VOCTPLSIZ);
            
            /* lock the verb object, and get the property value pointer */
            tplptr  = mcmlck(ctx->voccxmem, verb);
            thistpl = prpvalp(tplptr + tplofs);
            
            /* first byte is number of templates in array */
            tplcnt = *thistpl++;
            
            /* look for a template that matches the preposition object */
            for (found = FALSE ; tplcnt ; thistpl += siz, --tplcnt)
            {
                if (voctplpr(thistpl) == prep)
                {
                    found = TRUE;
                    break;
                }
            }
            
            /* unlock the object and return the value if we found one */
            mcmunlck(ctx->voccxmem, verb);
            if (found)
            {
                memcpy(tplout, thistpl, siz);
                return(TRUE);
            }
        }

        /* try inheriting a template (new-style, then old-style) */
        tplofs = objgetap(ctx->voccxmem, verb_in, PRP_TPL2, &verb, TRUE);
        if (tplofs)
            *newstyle = TRUE;
        else
        {
            tplofs = objgetap(ctx->voccxmem, verb_in, PRP_TPL, &verb, TRUE);
            *newstyle = FALSE;
        }

        /* return not-found if we couldn't inherit it */
        if (!tplofs)
            return FALSE;

        /* use the newly found verb */
        verb_in = verb;
    }
}

/*
 *   Set the "Me" object 
 */
void voc_set_me(voccxdef *ctx, objnum new_me)
{
    /* save undo for the change */
    vocdusave_me(ctx, ctx->voccxme);

    /* set the new "Me" object */
    ctx->voccxme = new_me;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
