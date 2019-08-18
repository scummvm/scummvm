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

#include "glk/tads/tads2/memory_cache.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* get an unused object cache entry, allocating a new page if needed */
static mcmodef *mcmoal(mcmcx1def *ctx, mcmon *objnum);

/* split a (previously free) block into two pieces */
static void mcmsplt(mcmcx1def *ctx, mcmon n, ushort siz);

/* unlink an object from a doubly-linked list */
static void mcmunl(mcmcx1def *ctx, mcmon n, mcmon *lst);

/* initialize a cache, return cache context */
/* find free block: find a block from the free pool to satisfy a request */
static mcmodef *mcmffb(mcmcx1def *ctx, ushort siz, mcmon *nump);

/* add page pagenum, initializing entries after firstunu to unused */
static void mcmadpg(mcmcx1def *ctx, uint pagenum, mcmon firstunu);

/* link an object into a doubly-linked list at the head of the list */
static void mcmlnkhd(mcmcx1def *ctx, mcmon *lst, mcmon n);

/* try to allocate a new chunk from the heap */
static uchar *mcmhalo(mcmcx1def *ctx);

/* relocate blocks in a heap */
static uchar *mcmreloc(mcmcx1def *ctx, uchar *start, uchar *end);

/* find next free heap block */
static uchar *mcmffh(mcmcx1def *ctx, uchar *p);

#ifdef NEVER
/* update entry to account for a block relocation */
static void mcmmove(mcmcx1def *ctx, mcmodef *obj, uchar *newaddr);
#else /* NEVER */
#define mcmmove(ctx, o, new) ((o)->mcmoptr = (new))
#endif /* NEVER */

/* consolidate two contiguous free blocks into a single block */
static void mcmconsol(mcmcx1def *ctx, uchar *p);

/* collect garbage in all heaps */
static void mcmgarb(mcmcx1def *ctx);

/* make some room by swapping or discarding objects */
static int mcmswap(mcmcx1def *ctx, ushort siz);

/* toss out an object; returns TRUE if successful */
static int mcmtoss(mcmcx1def *ctx, mcmon objnum);

/* next heap block, given a heap block (points to header) */
/* uchar *mcmhnxt(mcmcx1def *ctx, uchar *p) */

#define mcmnxh(ctx, p) \
 ((p) + osrndsz(sizeof(mcmon)) + mcmgobje(ctx, *(mcmon*)(p))->mcmosiz)

#ifdef DEBUG
# define MCMCLICTX(ctx) assert(*(((ulong *)ctx) - 1) == 0x02020202)
# define MCMGLBCTX(ctx) assert(*(((ulong *)ctx) - 1) == 0x01010101)
#else /* DEBUG */
# define MCMCLICTX(ctx)
# define MCMGLBCTX(ctx)
#endif /* DEBUG */

/* initialize a new client context */
mcmcxdef *mcmcini(mcmcx1def *globalctx, uint pages,
                  void (*loadfn)(void *, mclhd, uchar *, ushort),
                  void *loadctx,
                  void (*revertfn)(void *, mcmon), void *revertctx)
{
    mcmcxdef *ret;
    ushort    siz;
    
    siz = sizeof(mcmcxdef) + sizeof(mcmon *) * (pages - 1);
    IF_DEBUG(siz += sizeof(ulong));
    
    ret = (mcmcxdef *)mchalo(globalctx->mcmcxerr, siz, "mcm client context");
    IF_DEBUG((*(ulong *)ret = 0x02020202,
              ret = (mcmcxdef *)((uchar *)ret + sizeof(ulong))));

    ret->mcmcxmsz = pages;
    ret->mcmcxgl = globalctx;
    ret->mcmcxldf = loadfn;
    ret->mcmcxldc = loadctx;
    ret->mcmcxrvf = revertfn;
    ret->mcmcxrvc = revertctx;
    ret->mcmcxflg = 0;
    memset(ret->mcmcxmtb, 0, (size_t)(pages * sizeof(mcmon *)));
    return(ret);
}

/* uninitialize a client context */
void mcmcterm(mcmcxdef *ctx)
{
    /* delete the context memory */
    mchfre(ctx);
}

/* initialize a new global context */
mcmcx1def *mcmini(ulong max, uint pages, ulong swapsize,
                  osfildef *swapfp, char *swapfilename, errcxdef *errctx)
{
    mcmcx1def *ctx;                /* newly-allocated cache manager context */
    uchar     *noreg chunk;/* 1st chunk of memory managed by this cache mgr */
    mcmodef   *obj;                      /* pointer to a cache object entry */
    ushort     siz;                /* size of current thing being allocated */
    ushort     rem;                             /* bytes remaining in chunk */
    int        err;
    
    NOREG((&chunk))
    
    /* make sure 'max' is big enough - must be at least one chunk */
    if (max < (ulong)MCMCHUNK) max = (ulong)MCMCHUNK;
    
    /* allocate space for control structures from low-level heap */
    rem = MCMCHUNK;
    
    IF_DEBUG(rem += sizeof(long));
    chunk = mchalo(errctx, rem, "mcmini");
    IF_DEBUG((*(ulong *)chunk = 0x01010101, chunk += sizeof(ulong),
        rem -= sizeof(ulong)));
    
    ctx = (mcmcx1def *)chunk;              /* put context at start of chunk */
    
    /* initialize swapper; clean up if it fails */
    ERRBEGIN(errctx)
        mcsini(&ctx->mcmcxswc, ctx, swapsize, swapfp, swapfilename, errctx);
    ERRCATCH(errctx, err)
        mcsclose(&ctx->mcmcxswc);
        mchfre(chunk);
        errsig(errctx, err);
    ERREND(errctx)
    
    chunk += sizeof(mcmcx1def);           /* rest of chunk is after context */
    rem -= sizeof(mcmcx1def);         /* remove from remaining size counter */

    /* allocate the page table (an array of pointers to pages) */
    ctx->mcmcxtab = (mcmodef **)chunk;            /* put at bottom of chunk */
    siz = pages * sizeof(mcmodef *);              /* calcuate size of table */

    memset(ctx->mcmcxtab, 0, (size_t)siz);            /* clear entire table */
    chunk += siz;                                  /* reflect size of table */
    rem -= siz;                       /* take it out of the remaining count */

    /* here we begin normal heap marking with object references */
    ctx->mcmcxhpch = (mcmhdef *)chunk;           /* set start of heap chain */
    chunk += sizeof(mcmhdef);
    rem -= sizeof(mcmhdef);
    ctx->mcmcxhpch->mcmhnxt = (mcmhdef *)0;    /* no next heap in chain yet */
    
    /* allocate the first page */
    *(mcmon *)chunk = 0;               /* set object number header in chunk */
    chunk += osrndsz(sizeof(mcmon));
    rem -= osrndsz(sizeof(mcmon));

    ctx->mcmcxtab[0] = (mcmodef *)chunk;          /* put at bottom of chunk */
    memset(ctx->mcmcxtab[0], 0, (size_t)MCMPAGESIZE);
    chunk += MCMPAGESIZE;                           /* reflect size of page */
    rem -= MCMPAGESIZE;                     /* take it out of the remainder */
    
    /* set up the first page with an entry for itself */
    obj = mcmgobje(ctx, (mcmon)0);             /* point to first page entry */
    obj->mcmoflg = MCMOFPRES | MCMOFNODISC | MCMOFPAGE | MCMOFNOSWAP;
    obj->mcmoptr = (uchar *)ctx->mcmcxtab[0];
    obj->mcmosiz = MCMPAGESIZE;
    
    /* set up the rest of the context */
    ctx->mcmcxlru = ctx->mcmcxmru = MCMONINV;        /* no mru/lru list yet */
    ctx->mcmcxmax = max - (ulong)MCMCHUNK;
    ctx->mcmcxpage = 1;        /* next page slot to be allocated will be #1 */
    ctx->mcmcxpgmx = pages;          /* max number of pages we can allocate */
    ctx->mcmcxerr = errctx;
    ctx->mcmcxcsw = mcmcswf;
    
    /* set up the free list with the remainder of the chunk */
    ctx->mcmcxfre = 1;     /* we've allocated object 0; obj 1 is free space */
    obj = mcmgobje(ctx, ctx->mcmcxfre);       /* point to free object entry */
    obj->mcmonxt = obj->mcmoprv = MCMONINV;             /* end of free list */
    obj->mcmoflg = MCMOFFREE;                /* mark the free block as such */
    *(mcmon *)chunk = ctx->mcmcxfre;                /* set free list header */

    chunk += osrndsz(sizeof(mcmon));
    rem -= osrndsz(sizeof(mcmon));
    obj->mcmoptr = chunk;                                  /* rest of chunk */

    obj->mcmosiz = rem - osrndsz(sizeof(mcmon));          /* remaining size in chunk */

    /* set flag for end of chunk (invalid object header) */
    *((mcmon *)(chunk + rem - osrndsz(sizeof(mcmon)))) = MCMONINV;
    
    /* set up the unused entry list with the remaining headers in the page */
    mcmadpg(ctx, 0, 2);
    
    return(ctx);
}

/*
 *   Uninitialize the cache manager.  Frees the memory allocated for the
 *   cache, including the context structure itself.  
 */
void mcmterm(mcmcx1def *ctx)
{
    mcmhdef *cur, *nxt;
    
    /* 
     *   Free each chunk in the cache block list, *except* the last one.  The
     *   last one is special: it's actually the first chunk allocated, since
     *   we build the list in reverse order, and the first chunk pointer
     *   points into the middle of the actual allocation block, since we
     *   sub-allocated the context structure itself and the page table out of
     *   that memory. 
     */
    for (cur = ctx->mcmcxhpch ; cur != 0 && cur->mcmhnxt != 0 ; cur = nxt)
    {
        /* remember the next chunk, and delete this one */
        nxt = cur->mcmhnxt;
        mchfre(cur);
    }

    /* 
     *   As described above, the last chunk in the list is the first
     *   allocated, and it points into the middle of the actual allocated
     *   memory block.  Luckily, we do have a handy pointer to the start of
     *   the memory block, namely the context pointer - it's the first thing
     *   allocated out of the block, so it's the same as the block pointer.
     *   Freeing the context frees this last/first chunk. 
     */
    mchfre(ctx);
}

/*
 *   Allocate a new object, returning a pointer to its memory.  The new
 *   object is locked upon return.  The object number for the new object
 *   is returned at *nump.
 */
static uchar *mcmalo1(mcmcx1def *ctx, ushort siz, mcmon *nump)
{
    mcmon    n;
    mcmodef *o;
    uchar   *chunk;
    
    MCMGLBCTX(ctx);

    /* round size to appropriate multiple */
    siz = osrndsz(siz);

    /* if it's bigger than the chunk size, we can't allocate it */
    if (siz > MCMCHUNK)
        errsig(ctx->mcmcxerr, ERR_BIGOBJ);

startover:
    /* look in the free block chain for a fit to the request */
    o = mcmffb(ctx, siz, &n);
    if (n != MCMONINV)
    {
        mcmsplt(ctx, n, siz);               /* split the block if necessary */
        mcmgobje(ctx, n)->mcmoflg = MCMOFNODISC | MCMOFLOCK | MCMOFPRES;
        mcmgobje(ctx, n)->mcmolcnt = 1;                /* one locker so far */
        *nump = n;
        return(o->mcmoptr);
    }
    
    /* nothing found; we must get space out of the heap if possible */
    chunk = mcmhalo(ctx);                            /* get space from heap */
    if (!chunk) goto error;           /* can't get any more space from heap */
    o = mcmoal(ctx, &n);               /* set up cache entry for free space */
    if (n == MCMONINV)
    {
        mcmhdef *chunk_hdr = ((mcmhdef *)chunk) - 1;
        ctx->mcmcxhpch = chunk_hdr->mcmhnxt;
        mchfre(chunk_hdr);
        goto error;         /* any error means we can't allocate the memory */
    }
    
    *(mcmon *)chunk = n;                               /* set object header */
    chunk += osrndsz(sizeof(mcmon));
    o->mcmoptr = chunk;
    o->mcmosiz = MCMCHUNK - osrndsz(sizeof(mcmon));
    o->mcmoflg = MCMOFFREE;
    mcmlnkhd(ctx, &ctx->mcmcxfre, n);
    goto startover;              /* try again, now that we have some memory */
    
error:
    *nump = MCMONINV;
    return((uchar *)0);
}

static void mcmcliexp(mcmcxdef *cctx, mcmon clinum)
{
    /* add global number to client mapping table at client number */
    if (cctx->mcmcxmtb[clinum >> 8] == (mcmon *)0)
    {
        mcmcx1def *ctx = cctx->mcmcxgl;
        int        i;
        mcmon     *p;
        
        /* this page is not allocated - allocate it */
        p = (mcmon *)mchalo(ctx->mcmcxerr, (256 * sizeof(mcmon)),
                            "client mapping page");
        cctx->mcmcxmtb[clinum >> 8] = p;
        for (i = 0 ; i < 256 ; ++i) *p++ = MCMONINV;
    }
}

/* high-level allocate:  try, collect garbage, then try again */
uchar *mcmalo0(mcmcxdef *cctx, ushort siz, mcmon *nump,
               mcmon clinum, int noclitrans)
{
    uchar     *ret;
    mcmcx1def *ctx = cctx->mcmcxgl;                       /* global context */
    mcmon      glb;                       /* global object number allocated */
    
    MCMCLICTX(cctx);
    MCMGLBCTX(ctx);
    
    /* try once */
    if ((ret = mcmalo1(ctx, siz, &glb)) != 0)
        goto done;

    /* collect some garbage */
    mcmgarb(ctx);
    
    /* try swapping until we get the memory or have nothing left to swap */
    for ( ;; )
    {
        /* try again */
        if ((ret = mcmalo1(ctx, siz, &glb)) != 0)
            goto done;

        /* nothing left to swap? */
        if (!mcmswap(ctx, siz))
            break;

        /* try yet again */
        if ((ret = mcmalo1(ctx, siz, &glb)) != 0)
            goto done;

        /* collect garbage once again */        
        mcmgarb(ctx);
    }
    
    /* try again */
    if ((ret = mcmalo1(ctx, siz, &glb)) != 0)
        goto done;
    
    /* we have no other way of getting more memory, so signal an error */
    errsig(ctx->mcmcxerr, ERR_NOMEM1);
    NOTREACHEDV(uchar *);
    
done:
    if (noclitrans)
    {
        *nump = glb;
        return(ret);
    }
    
    /* we have an object - generate client number */
    if (clinum == MCMONINV)
    {
        /* find a free number */
        mcmon **p;
        uint    i;
        mcmon   j = 0;
        mcmon  *q;
        int     found = FALSE;
        int     unused = -1;
        
        for (i = 0, p = cctx->mcmcxmtb ; i < cctx->mcmcxmsz ; ++i, ++p)
        {
            if (*p)
            {
                for (j = 0, q = *p ; j < 256 ; ++j, ++q)
                {
                    if (*q == MCMONINV)
                    {
                        found = TRUE;
                        break;
                    }
                }
            }
            else if (unused == -1)
                unused = i;            /* note an unused page mapping table */

            if (found) break;
        }
        
        if (found)
            clinum = (i << 8) + j;
        else if (unused != -1)
            clinum = (unused << 8);
        else
            errsig(ctx->mcmcxerr, ERR_CLIFULL);
    }

    /* expand client mapping table if necessary */
    mcmcliexp(cctx, clinum);

    /* make sure the entry isn't already in use */
    if (mcmc2g(cctx, clinum) != MCMONINV)
        errsig(ctx->mcmcxerr, ERR_CLIUSE);

    cctx->mcmcxmtb[clinum >> 8][clinum & 255] = glb;
    if (nump) *nump = clinum;
    return(ret);
}

/* reserve space for an object at a client object number */
void mcmrsrv(mcmcxdef *cctx, ushort siz, mcmon clinum, mclhd loadhd)
{
    mcmcx1def *ctx = cctx->mcmcxgl;                       /* global context */
    mcmon      glb;                       /* global object number allocated */
    mcmodef   *o;
    
    MCMCLICTX(cctx);
    MCMGLBCTX(ctx);
    
    o = mcmoal(ctx, &glb);                       /* get a new object header */
    if (!o) errsig(ctx->mcmcxerr, ERR_NOHDR);     /* can't get a new header */
    
    o->mcmoldh = loadhd;
    o->mcmoflg = 0;
    o->mcmosiz = siz;
    
    mcmcliexp(cctx, clinum);
    if (mcmc2g(cctx, clinum) != MCMONINV)
        errsig(ctx->mcmcxerr, ERR_CLIUSE);
    
    cctx->mcmcxmtb[clinum >> 8][clinum & 255] = glb;
}

/* resize an existing object */
uchar *mcmrealo(mcmcxdef *cctx, mcmon cliobj, ushort newsize)
{
    mcmcx1def *ctx = cctx->mcmcxgl;                       /* global context */
    mcmon      obj = mcmc2g(cctx, cliobj); 
    mcmodef   *o = mcmgobje(ctx, obj);
    mcmon      nxt;
    mcmodef   *nxto;
    uchar     *p;
    int        local_lock;
    
    MCMCLICTX(cctx);
    MCMGLBCTX(ctx);
    
    newsize = osrndsz(newsize);
    
    /* make sure the object is locked, and note if we locked it */
    if ((local_lock = !(o->mcmoflg & MCMOFLOCK)) != 0)
        (void)mcmlck(cctx, cliobj);
    
    ERRBEGIN(ctx->mcmcxerr)
    
    if (newsize < o->mcmosiz)
        mcmsplt(ctx, obj, newsize);            /* smaller; just split block */
    else
    {
        /* see if there's a free block after this block */
        p = o->mcmoptr;
        nxt = *(mcmon *)(p + o->mcmosiz);
        nxto = (nxt == MCMONINV) ? (mcmodef *)0 : mcmgobje(ctx, nxt);
        
        if (nxto && ((nxto->mcmoflg & MCMOFFREE)
                     && nxto->mcmosiz >= newsize - o->mcmosiz))
        {
            /* sanity check - make sure heap and page table agree */
            assert(nxto->mcmoptr == p + o->mcmosiz + osrndsz(sizeof(mcmon)));
            /* annex the free block */
            o->mcmosiz += nxto->mcmosiz + osrndsz(sizeof(mcmon));
            /* move the free block to the unused list */
            mcmunl(ctx, nxt, &ctx->mcmcxfre);
            nxto->mcmonxt = ctx->mcmcxunu;
            ctx->mcmcxunu = nxt;
            nxto->mcmoflg = 0;
            
            /* split the newly grown block if necessary */
            mcmsplt(ctx, obj, newsize);
        }
        else
        {
            /* can't annex; allocate new memory and copy */
            
            if (o->mcmolcnt != 1)           /* if anyone else has a lock... */
                errsig(ctx->mcmcxerr, ERR_REALCK);      /* we can't move it */
    
            p = mcmalo0(cctx, newsize, &nxt, MCMONINV, TRUE);
            if (nxt == MCMONINV) errsig(ctx->mcmcxerr, ERR_NOMEM2);
            memcpy(p, o->mcmoptr, (size_t)o->mcmosiz);
            
            /* adjust the object entries */
            nxto = mcmgobje(ctx, nxt);          /* get pointer to new entry */
            newsize = nxto->mcmosiz;        /* get actual size of new block */
            nxto->mcmoptr = o->mcmoptr;   /* copy current block info to new */
            nxto->mcmosiz = o->mcmosiz;
            o->mcmoptr = p;        /* copy new block info to original entry */
            o->mcmosiz = newsize;
            
            /* now fix up the heap pointers, and free the temp object */
            *(mcmon *)(p - osrndsz(sizeof(mcmon))) = obj;
            *(mcmon *)(nxto->mcmoptr - osrndsz(sizeof(mcmon))) = nxt;
            mcmgunlck(ctx, nxt);
            mcmgfre(ctx, nxt);
        }
    }
    
    ERRCLEAN(ctx->mcmcxerr)
        /* release our lock, if we had to obtain one */
        if (local_lock) mcmunlck(cctx, cliobj);
    ERRENDCLN(ctx->mcmcxerr)
    
    /* return the address of the object */
    return(o->mcmoptr);
}

/*
 *   Free an object by GLOBAL number:  move object to free list.
 */
void mcmgfre(mcmcx1def *ctx, mcmon obj)
{
    mcmodef   *o = mcmgobje(ctx, obj);
    
    MCMGLBCTX(ctx);
    
    /* signal an error if the object is locked */
    if (o->mcmolcnt) errsig(ctx->mcmcxerr, ERR_LCKFRE);

    /* take out of LRU chain if it's in the chain */
    if (o->mcmoflg & MCMOFLRU) mcmunl(ctx, obj, &ctx->mcmcxlru);
    
    /* put it in the free list */
    mcmlnkhd(ctx, &ctx->mcmcxfre, obj);
    o->mcmoflg = MCMOFFREE;
}

/*
 *   load and lock an object that has been swapped out or discarded 
 */
uchar *mcmload(mcmcxdef *cctx, mcmon cnum)
{
    mcmcx1def   *ctx = cctx->mcmcxgl;
    mcmodef     *o = mcmobje(cctx, cnum);
    mcmodef     *newdef;
    mcmon        newn;
    mcmon        num = mcmc2g(cctx, cnum);
    
    MCMCLICTX(cctx);
    MCMGLBCTX(ctx);

    /* we first need to obtain some memory for this object */
    (void)mcmalo0(cctx, o->mcmosiz, &newn, MCMONINV, TRUE);
    newdef = mcmgobje(ctx, newn);
    
    /* use memory block from our new object */
    o->mcmoptr = newdef->mcmoptr;
    o->mcmosiz = newdef->mcmosiz;

    /* load or swap the object in */
    ERRBEGIN(ctx->mcmcxerr)
        if (o->mcmoflg & (MCMOFNODISC | MCMOFDIRTY))
            mcsin(&ctx->mcmcxswc, o->mcmoswh, o->mcmoptr, o->mcmosiz);
        else if (cctx->mcmcxldf)
            (*cctx->mcmcxldf)(cctx->mcmcxldc, o->mcmoldh, o->mcmoptr,
                              o->mcmosiz);
        else
            errsig(ctx->mcmcxerr, ERR_NOLOAD);
    ERRCLEAN(ctx->mcmcxerr)
        mcmgunlck(ctx, newn);                          /* unlock the object */
        mcmgfre(ctx, newn);              /* don't need new memory after all */
    ERRENDCLN(ctx->mcmcxerr)

    /* unuse the new cache entry we obtained (we just wanted the memory) */
/* @@@ */
    *(mcmon *)(o->mcmoptr - osrndsz(sizeof(mcmon))) = num;      /* set obj# */
    newdef->mcmoflg = 0;                        /* mark new block as unused */
    newdef->mcmonxt = ctx->mcmcxunu;                /* link to unused chain */
    ctx->mcmcxunu = newn;

    /* set flags in the newly loaded object and return */
    o->mcmoflg |= MCMOFPRES | MCMOFLOCK; /* object is now present in memory */
    o->mcmoflg &= ~MCMOFDIRTY;         /* not written since last swapped in */
    o->mcmoflg |= MCMOFNODISC; /* don't discard once it's been to swap file */
    o->mcmolcnt = 1;                                   /* one locker so far */
    
    /* if the object is to be reverted upon loading, revert it now */
    if (o->mcmoflg & MCMOFREVRT)
    {
        (*cctx->mcmcxrvf)(cctx->mcmcxrvc, cnum);
        o->mcmoflg &= ~MCMOFREVRT;
    }

    return(o->mcmoptr);
}

/*
 *   Allocate a new object header.  This doesn't allocate an object, just
 *   the header for one. 
 */
static mcmodef *mcmoal(mcmcx1def *ctx, mcmon *nump)
{
    mcmodef  *ret;
    uint      pagenum;
    
    MCMGLBCTX(ctx);
    
    /* look first in list of unused headers */
startover:
    if (ctx->mcmcxunu != MCMONINV)
    {
        /* we have something in the unused list; return it */
        *nump = ctx->mcmcxunu;
        ret = mcmgobje(ctx, *nump);
        ctx->mcmcxunu = ret->mcmonxt;
        ret->mcmoswh = MCSSEGINV;
        return(ret);
    }
    
    /*
     *   No unused entries: we must create a new page.  To do so, we
     *   simply allocate memory for a new page.  Allocate the memory
     *   ourselves, to avoid deadlocking with the allocator (which can
     *   try to get a new entry to satisfy our request for memory).
     */
    if (ctx->mcmcxpage == ctx->mcmcxpgmx) goto error;      /* no more pages */
    pagenum = ctx->mcmcxpage++;                      /* get a new page slot */
    
    ctx->mcmcxtab[pagenum] =
         (mcmodef *)mchalo(ctx->mcmcxerr, MCMPAGESIZE, "mcmoal");
    mcmadpg(ctx, pagenum, MCMONINV);
    goto startover;

error:
    *nump = MCMONINV;
    return((mcmodef *)0);
}

/* find free block:  find a block from the free pool to satisfy allocation */
static mcmodef *mcmffb(mcmcx1def *ctx, ushort siz, mcmon *nump)
{
    mcmon    n;
    mcmodef *o;
    mcmon    minn;
    mcmodef *mino;
    ushort   min = 0;
    
    MCMGLBCTX(ctx);

    for (minn = MCMONINV, mino = 0, n = ctx->mcmcxfre ; n != MCMONINV ;
         n = o->mcmonxt)
    {
        o = mcmgobje(ctx, n);
        if (o->mcmosiz == siz)
        {
            /* found exact match - use it immediately */
            minn = n;
            min = siz;
            mino = o;
            break;
        }
        else if (o->mcmosiz > siz)
        {
            /* found something at least as big; is it smallest yet? */
            if (minn == MCMONINV || o->mcmosiz < min)
            {
                /* yes, best fit so far, use it; but keep looking */
                minn = n;
                mino = o;
                min = o->mcmosiz;
            }
        }
    }
    
    /* if we found something, remove from the free list */
    if (minn != MCMONINV)
    {
        mcmunl(ctx, minn, &ctx->mcmcxfre);
        mino->mcmoflg &= ~MCMOFFREE;
        mino->mcmoswh = MCSSEGINV;
    }
    
    *nump = minn;
    return mino;
}

/*
 *   unlink an object header from one of the doubly-linked lists 
 */
static void mcmunl(mcmcx1def *ctx, mcmon n, mcmon *lst)
{
    mcmodef *o = mcmgobje(ctx, n);
    mcmodef *nxt;
    mcmodef *prv;
    
    MCMGLBCTX(ctx);

    /* see if this is LRU chain - must deal with MRU pointer if so */
    if (lst == &ctx->mcmcxlru)
    {
        /* if it's at MRU, set MRU pointer to previous object in list */
        if (ctx->mcmcxmru == n)
        {
            ctx->mcmcxmru = o->mcmoprv;     /* set MRU to previous in chain */
            if (ctx->mcmcxmru != MCMONINV)           /* set nxt for new MRU */
                mcmgobje(ctx, ctx->mcmcxmru)->mcmonxt = MCMONINV;
            else
                ctx->mcmcxlru = MCMONINV;     /* nothing in list; clear LRU */
        }
        o->mcmoflg &= ~MCMOFLRU;
    }
    
    nxt = o->mcmonxt == MCMONINV ? (mcmodef *)0 : mcmgobje(ctx, o->mcmonxt);
    prv = o->mcmoprv == MCMONINV ? (mcmodef *)0 : mcmgobje(ctx, o->mcmoprv);
    
    /* set back link for next object, if there is a next object */
    if (nxt) nxt->mcmoprv = o->mcmoprv;
    
    /* set forward link for previous object, or head if no previous object */
    if (prv) prv->mcmonxt = o->mcmonxt;
    else *lst = o->mcmonxt;
    
    o->mcmonxt = o->mcmoprv = MCMONINV;
}

/* link an item to the head of a doubly-linked list */
static void mcmlnkhd(mcmcx1def *ctx, mcmon *lst, mcmon n)
{
    MCMGLBCTX(ctx);

    if (*lst != MCMONINV) mcmgobje(ctx, *lst)->mcmoprv = n;
    mcmgobje(ctx, n)->mcmonxt = *lst;      /* next is previous head of list */
    *lst = n;                               /* make object new head of list */
    mcmgobje(ctx, n)->mcmoprv = MCMONINV;     /* there is no previous entry */
}

/* add page pagenum, initializing entries after firstunu to unused */
static void mcmadpg(mcmcx1def *ctx, uint pagenum, mcmon firstunu)
{
    mcmon    unu;
    mcmodef *obj;
    mcmon    lastunu;
    
    MCMGLBCTX(ctx);

    unu = (firstunu == MCMONINV ? pagenum * MCMPAGECNT : firstunu);
    ctx->mcmcxunu = unu;
    lastunu = (pagenum * MCMPAGECNT) + MCMPAGECNT - 1;
    for (obj = mcmgobje(ctx, unu) ; unu < lastunu ; ++obj)
        obj->mcmonxt = ++unu;
    obj->mcmonxt = MCMONINV;
}

/*
 *   split a previously-free block into two chunks, adding the remainder
 *   back into the free list, if there's enough left over 
 */ 
static void mcmsplt(mcmcx1def *ctx, mcmon n, ushort siz)
{
    mcmodef *o = mcmgobje(ctx, n);
    mcmon    newn;
    mcmodef *newp;
    
    MCMGLBCTX(ctx);

    if (o->mcmosiz < siz + MCMSPLIT) return;     /* don't split; we're done */

    newp = mcmoal(ctx, &newn);
    if (newn == MCMONINV) return;         /* ignore error - just skip split */

    /* set up the new entry, and link into free list */
    *(mcmon *)(o->mcmoptr + siz) = newn;
    newp->mcmoptr = o->mcmoptr + siz + osrndsz(sizeof(mcmon));
    newp->mcmosiz = o->mcmosiz - siz - osrndsz(sizeof(mcmon));
    newp->mcmoflg = MCMOFFREE;
    mcmlnkhd(ctx, &ctx->mcmcxfre, newn);
    
    o->mcmosiz = siz;       /* size of new object is now exactly as request */
}

/* allocate a new chunk from the heap if possible */
static uchar *mcmhalo(mcmcx1def *ctx)
{
    uchar  *chunk;
    int     err;
#define  size (MCMCHUNK + sizeof(mcmhdef) + 2*osrndsz(sizeof(mcmon)))

    MCMGLBCTX(ctx);

    if (ctx->mcmcxmax < MCMCHUNK) return((uchar *)0);

    ERRBEGIN(ctx->mcmcxerr)
        chunk = mchalo(ctx->mcmcxerr, size, "mcmhalo");
    ERRCATCH(ctx->mcmcxerr, err)
        ctx->mcmcxmax = 0;      /* remember we can't allocate anything more */
        return((uchar *)0);                             /* return no memory */
    ERREND(ctx->mcmcxerr)

    ctx->mcmcxmax -= MCMCHUNK;
    
    /* link into heap chain */
    ((mcmhdef *)chunk)->mcmhnxt = ctx->mcmcxhpch;
    ctx->mcmcxhpch = (mcmhdef *)chunk;
/*@@@@*/
    *(mcmon *)(chunk + osrndsz(sizeof(mcmhdef) + MCMCHUNK)) = MCMONINV;
	VARUSED(err);

	return(chunk + sizeof(mcmhdef));

#undef size
}

/* "use" an object - move to most-recent position in LRU chain */
void mcmuse(mcmcx1def *ctx, mcmon obj)
{
    mcmodef   *o = mcmgobje(ctx, obj);
    
    MCMGLBCTX(ctx);

    if (ctx->mcmcxmru == obj) return;         /* already MRU; nothing to do */
    
    /* remove from LRU chain if it's in it */
    if (o->mcmoflg & MCMOFLRU) mcmunl(ctx, obj, &ctx->mcmcxlru);

    /* set forward pointer of last block, if there is one */
    if (ctx->mcmcxmru != MCMONINV)
        mcmgobje(ctx, ctx->mcmcxmru)->mcmonxt = obj;
    
    o->mcmoprv = ctx->mcmcxmru;               /* point back to previous MRU */
    o->mcmonxt = MCMONINV;                /* nothing in list after this one */
    ctx->mcmcxmru = obj;                          /* point MRU to new block */
    
    /* if there's nothing in the chain at all, set LRU to this block, too */
    if (ctx->mcmcxlru == MCMONINV) ctx->mcmcxlru = obj;

    /* note that object is in LRU chain */
    o->mcmoflg |= MCMOFLRU;
}

/* find next free block in a heap, starting with pointer */
static uchar *mcmffh(mcmcx1def *ctx, uchar *p)
{
    mcmodef *o;

    MCMGLBCTX(ctx);

    while (*(mcmon *)p != MCMONINV)
    {
        o = mcmgobje(ctx, *(mcmon *)p);
        assert(o->mcmoptr == p + osrndsz(sizeof(mcmon)));
        if (o->mcmoflg & MCMOFFREE) return(p);
        p += osrndsz(sizeof(mcmon)) + o->mcmosiz;  /* move on to next chunk */
    }
    return((uchar *)0);                      /* no more free blocks in heap */
}

#ifdef NEVER
static void mcmmove(mcmcx1def *ctx, mcmodef *o, uchar *newpage)
{
    mcmodef **page;
    
    MCMGLBCTX(ctx);

    /* see if we need to update page table (we do if moving a page) */
    if (o->mcmoflg & MCMOFPAGE)
    {
        for (page = ctx->mcmcxtab ; *page ; ++page)
        {
            if (*page == (mcmodef *)(o->mcmoptr))
            {
                *page = (mcmodef *)newpag;
                break;
            }
        }
        if (!*page) printf("\n*** internal error - relocating page\n");
    }
    o->mcmoptr = newpage;
}
#endif /* NEVER */

/* relocate blocks from p to (but not including) q */
static uchar *mcmreloc(mcmcx1def *ctx, uchar *p, uchar *q)
{
    mcmodef *o;
    ushort   dist;
    mcmon    objnum;

    MCMGLBCTX(ctx);

    objnum = *(mcmon *)p;      /* get number of free block being bubbled up */
    o = mcmgobje(ctx, objnum);                /* get pointer to free object */
    assert(o->mcmoptr == p + osrndsz(sizeof(mcmon)));
    dist = osrndsz(sizeof(mcmon)) + o->mcmosiz; /* compute distance to move */
    mcmmove(ctx, o, q - dist + osrndsz(sizeof(mcmon)));  /* move obj to top */

    memmove(p, p+dist, (size_t)(q - p - o->mcmosiz));        /* move memory */

    /* update cache entries for the blocks we moved */
    while (p != q - dist)
    {
        mcmmove(ctx, mcmgobje(ctx, *(mcmon *)p), p + osrndsz(sizeof(mcmon)));
        p = mcmnxh(ctx, p);
    }

    *(mcmon *)(q - dist) = objnum;                       /* set bubbled num */
    return(q - dist);               /* return new location of bubbled block */
}

/* consolidate the two (free) blocks starting at p into one block */
static void mcmconsol(mcmcx1def *ctx, uchar *p)
{
    uchar   *q;
    mcmodef *obj1, *obj2;
    
    MCMGLBCTX(ctx);

    q = mcmnxh(ctx, p);
    obj1 = mcmgobje(ctx, *(mcmon *)p);
    obj2 = mcmgobje(ctx, *(mcmon *)q);
    
    assert(obj1->mcmoptr == p + osrndsz(sizeof(mcmon)));
    assert(obj2->mcmoptr == q + osrndsz(sizeof(mcmon)));

    obj1->mcmosiz += osrndsz(sizeof(mcmon)) + obj2->mcmosiz;
    mcmunl(ctx, *(mcmon *)q, &ctx->mcmcxfre);
                    
    /* add second object entry to unused list */
    obj2->mcmonxt = ctx->mcmcxunu;
    ctx->mcmcxunu = *(mcmon *)q;
    obj2->mcmoflg = 0;
}

/* attempt to compact all heaps by consolidating free space */
static void mcmgarb(mcmcx1def *ctx)
{
    mcmhdef *h;
    uchar   *p;
    uchar   *q;
    uchar   *nxt;
    ushort   flags;
    
    MCMGLBCTX(ctx);

    for (h = ctx->mcmcxhpch ; h ; h = h->mcmhnxt)
    {
        p = (uchar *)(h+1);                   /* get pointer to actual heap */
        p = mcmffh(ctx, p);                 /* get first free block in heap */
        if (!p) continue;             /* can't do anything - no free blocks */
        nxt = mcmnxh(ctx, p);              /* remember immediate next block */
        
        for (q=p ;; )
        {
            q = mcmnxh(ctx, q);                  /* find next chunk in heap */
            if (*(mcmon *)q == MCMONINV) break;      /* reached end of heap */
            assert(mcmgobje(ctx, *(mcmon *)q)->mcmoptr
                   == q + osrndsz(sizeof(mcmon)));
            flags = mcmgobje(ctx, *(mcmon *)q)->mcmoflg;       /* get flags */

            /* if the block is locked, p can't be relocated */
            if (flags & MCMOFLOCK)
            {
                p = mcmffh(ctx, q);         /* find next free block after p */
                q = p;
                if (p) continue;   /* try again; start with next free block */
                else break;         /* no more free blocks - done with heap */
            }

            /* if the block is free, we can relocate between p and q */
            if (flags & MCMOFFREE)
            {
                if (q != nxt) p = mcmreloc(ctx, p, q);          /* relocate */
                mcmconsol(ctx, p);           /* consolidate two free blocks */
                
                /* resume looking, starting with consolidated block */
                nxt = mcmnxh(ctx, p);
                q = p;
                continue;
            }
        }
    }
}

/* toss out a particular object */
static int mcmtoss(mcmcx1def *ctx, mcmon n)
{
    mcmodef *o = mcmgobje(ctx, n);
    mcmodef *newp;
    mcmon    newn;

    MCMGLBCTX(ctx);

    /* make a new block for the free space */
    newp = mcmoal(ctx, &newn);
    if (newn == MCMONINV)
        return(FALSE);           /* ignore the error, but can't toss it out */

    /* write object to swap file if not discardable */
    if (o->mcmoflg & (MCMOFNODISC | MCMOFDIRTY))
    {
        mcsseg old_swap_seg;
        
        /*
         *   If this object was last loaded out of the load file, rather
         *   than the swap file, don't attempt to find it in the swap file
         *   -- so note by setting the old swap segment parameter to null.
         */
        if (!(o->mcmoflg & MCMOFNODISC))
            old_swap_seg = o->mcmoswh;
        else
            old_swap_seg = MCSSEGINV;
        
        o->mcmoswh = mcsout(&ctx->mcmcxswc, (uint)n, o->mcmoptr, o->mcmosiz,
                            old_swap_seg, o->mcmoflg & MCMOFDIRTY);
    }
    
    /* give the object's space to the newly created block */
    newp->mcmoptr = o->mcmoptr;
    newp->mcmosiz = o->mcmosiz;
    newp->mcmoflg = MCMOFFREE;
/*@@@*/
    *(mcmon *)(o->mcmoptr - osrndsz(sizeof(mcmon))) = newn;
    mcmlnkhd(ctx, &ctx->mcmcxfre, newn);
    
    o->mcmoflg &= ~MCMOFPRES;              /* object is no longer in memory */
    mcmunl(ctx, n, &ctx->mcmcxlru);                 /* remove from LRU list */
    return(TRUE);                             /* successful, so return TRUE */
}

/* swap or discard to make room for siz; return 0 if nothing swapped */
static int mcmswap(mcmcx1def *ctx, ushort siz)
{
    mcmon    n;
    mcmodef *o;
    mcmon    nxt;
    int      pass;                     /* pass 1: swap one piece big enough */
                      /* pass 2: swap enough pieces to add up to right size */
    ushort   tot;

    MCMGLBCTX(ctx);

    for (pass = 1, tot = 0 ; pass < 3 && tot < siz ; ++pass)
    {
        for (n = ctx->mcmcxlru ; n != MCMONINV && tot < siz ; n = nxt)
        {
            o = mcmgobje(ctx, n);
            nxt = o->mcmonxt;             /* get next now, as we may unlink */
            if (!(o->mcmoflg & (MCMOFLOCK | MCMOFNOSWAP | MCMOFPAGE))
                 && (pass == 2 || o->mcmosiz >= siz))
            {
                /* toss out, and add into size if successful */
                if (mcmtoss(ctx, n)) tot += o->mcmosiz;
            }
        }
    }
    
    /* if we managed to remove anything, return TRUE, otherwise FALSE */
    return(tot != 0);
}

/* compute size of cache */
ulong mcmcsiz(mcmcxdef *cctx)
{
    mcmcx1def *ctx = cctx->mcmcxgl;
    mcmhdef   *p;
    ulong      tot;
    
    MCMCLICTX(cctx);
    MCMGLBCTX(ctx);
    
    /* count number of heaps, adding in chunk size for each */
    for (tot = 0, p = ctx->mcmcxhpch ; p ; p = p->mcmhnxt)
        tot += MCMCHUNK;
    
    return(tot);
}

#ifdef MCM_NO_MACRO
/* routines that can be either macros or functions */

uchar *mcmlck(mcmcxdef *ctx, mcmon objnum)
{
    mcmodef *o = mcmobje(ctx, objnum);

    if ((o->mcmoflg & MCMOFFREE) != 0 || mcmc2g(ctx, objnum) == MCMONINV)
    {
        errsig(ctx->mcmcxgl->mcmcxerr, ERR_INVOBJ);
        return 0;
    }
    else if (o->mcmoflg & MCMOFPRES)
    {
        o->mcmoflg |= MCMOFLOCK;
        ++(o->mcmolcnt);
        return(o->mcmoptr);
    }
    else
        return(mcmload(ctx, objnum));
}

void mcmunlck(mcmcxdef *ctx, mcmon obj)
{
    mcmodef *o = mcmobje(ctx, obj);
    
    if (o->mcmoflg & MCMOFLOCK)
    {
        if (!(--(o->mcmolcnt)))
        {
            o->mcmoflg &= ~MCMOFLOCK;
            mcmuse(ctx->mcmcxgl, mcmc2g(ctx, obj));
        }
    }
}

void mcmgunlck(mcmcx1def *ctx, mcmon obj)
{
    mcmodef *o = mcmgobje(ctx, obj);
    
    if (o->mcmoflg & MCMOFLOCK)
    {
        if (!(--(o->mcmolcnt)))
        {
            o->mcmoflg &= ~MCMOFLOCK;
            mcmuse(ctx, obj);
        }
    }
}

#endif /* MCM_NO_MACRO */

/*
 *   Change an object's swap file handle.  This routine will only be
 *   called for an object that is either present or swapped out (i.e., an
 *   object with a valid mcsseg number in its swap state).  
 */
void mcmcswf(mcmcx1def *ctx, mcmon objn, mcsseg swapn, mcsseg oldswapn)
{
    mcmodef *o = mcmgobje(ctx, objn);
    
    MCMGLBCTX(ctx);
    
    /*
     *   Reset the swap number only if the object is swapped out and its
     *   swap file number matches the old one, or the object is currently
     *   present (in which case the swap file number is irrelevant and can
     *   be replaced).  
     */
    if (((o->mcmoflg & (MCMOFDIRTY | MCMOFNODISC)) && o->mcmoswh == oldswapn)
        || (o->mcmoflg & MCMOFPRES))
        o->mcmoswh = swapn;
}


void mcmfre(mcmcxdef *ctx, mcmon obj)
{
    /* free the actual object */
    mcmgfre(ctx->mcmcxgl, mcmc2g(ctx, obj));

    /* unmap the client object number */
    mcmc2g(ctx, obj) = MCMONINV;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
