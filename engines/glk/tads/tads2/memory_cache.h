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
 * Memory cache manager
 */

#ifndef GLK_TADS_TADS2_MEMORY_CACHE
#define GLK_TADS_TADS2_MEMORY_CACHE

#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/memory_cache_loader.h"
#include "glk/tads/tads2/memory_cache_swap.h"
#include "glk/tads/os_frob_tads.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* if the os layer doesn't want long mcm macros, we don't either */
#ifdef OS_MCM_NO_MACRO
# define MCM_NO_MACRO
#endif

/*
 *   mcmon - cache object number.  Each object in the cache is referenced
 *   by an object number, which is a number of this type.
 */
typedef ushort mcmon;

/* invalid object number - used to indicate N/A or no object */
#define MCMONINV ((mcmon)~0)

/* invalid page number */
#define MCMPINV ((uint)~0)

union mcmodefloc {
	mcsseg  mcmolocs;                            /* swap segment handle */
	mclhd   mcmolocl;                        /* load file object handle */
};

/*
 *   mcmodef - cache object definition.  Each cache object is managed by
 *   this structure.  Pointers for a doubly-linked list are provided;
 *   these are used to maintain a recent-use list for objects in memory,
 *   and to maintain a free list for cache object entries not in use.  A
 *   set of flag bits is provided, as is the size of the object and its
 *   location (which is a memory pointer for objects in memory, a swap
 *   file handle for swapped-out objects, or a load-file handle for
 *   objects that are unloaded). 
 */
struct mcmodef {
    uchar  *mcmoptr;               /* memory pointer (if object is present) */
	mcmodefloc mcmoloc;                       /* object's external location */
#define  mcmoswh  mcmoloc.mcmolocs
#define  mcmoldh  mcmoloc.mcmolocl
    mcmon   mcmonxt;                            /* next object in this list */
    mcmon   mcmoprv;                        /* previous object in this list */
    ushort  mcmoflg;                         /* flags for this cache object */
#define  MCMOFDIRTY  0x01                     /* object has been written */
#define  MCMOFNODISC 0x02       /* not in load file (can't be discarded) */
#define  MCMOFLOCK   0x04                            /* object is locked */
#define  MCMOFPRES   0x08                 /* object is present in memory */
#define  MCMOFLRU    0x10                      /* object is in LRU chain */
#define  MCMOFPAGE   0x20              /* object is a cache manager page */
#define  MCMOFNOSWAP 0x40                /* object cannot be swapped out */
#define  MCMOFFREE   0x80      /* entry refers to a free block of memory */
#define  MCMOFREVRT 0x100           /* call revert callback upon loading */
    uchar   mcmolcnt;                                         /* lock count */
    ushort  mcmosiz;                                  /* size of the object */
};

/* heap header - allocate one of these in each heap */
struct mcmhdef {
    mcmhdef *mcmhnxt;                            /* next heap in this chain */
};

/* GLOBAL cache manager context:  tracks cache manager state */
struct mcmcx1def {
    mcmodef  **mcmcxtab;                        /* page table for the cache */
    errcxdef  *mcmcxerr;                          /* error handling context */
    mcmhdef   *mcmcxhpch;                             /* heap chain pointer */
    mcscxdef   mcmcxswc;                            /* swap manager context */
    mclcxdef   mcmcxldc;                                  /* loader context */
    ulong      mcmcxmax; /* maximum amount of actual heap we can ever alloc */
    mcmon      mcmcxlru;      /* least recently used object still in memory */
    mcmon      mcmcxmru;                       /* most recently used object */
    mcmon      mcmcxfre;                               /* head of free list */
    mcmon      mcmcxunu;                             /* head of unused list */
    ushort     mcmcxpage;                      /* last page table slot used */
    ushort     mcmcxpgmx;        /* maximum number of pages we can allocate */
    void     (*mcmcxcsw)(mcmcx1def *, mcmon, mcsseg, mcsseg);
                         /* change swap handle in object to new swap handle */
};

/* CLIENT cache manager context: used by client to request mcm services */
struct mcmcxdef {
    mcmcx1def *mcmcxgl;                     /* global cache manager context */
    uint       mcmcxflg;                                           /* flags */
    uint       mcmcxmsz;                   /* maximum size of mapping table */
    void     (*mcmcxldf)(void *ctx, mclhd handle, uchar *ptr,
                         ushort siz);           /* callback to load objects */
    void      *mcmcxldc;                       /* context for load callback */
    void     (*mcmcxrvf)(void *ctx, mcmon objn);           /* revert object */
    void      *mcmcxrvc;                     /* context for revert callback */
    mcmon     *mcmcxmtb[1];                                /* mapping table */
};

/* context flags */
#define MCMCXF_NO_PRP_DEL   0x0001  /* PRPFDEL is invalid in this game file */

/* convert from a client object number to a global object number */
/* mcmon mcmc2g(mcmcxdef *ctx, mcmon objn); */
#define mcmc2g(ctx, objn) ((ctx)->mcmcxmtb[(objn)>>8][(objn)&255])

/*
 *   FREE LIST: this is a list, headed by context->mcmcxfre and chained
 *   forward and back by mcmonxt and mcmoprv, consisting of free memory
 *   blocks.  These refer to blocks in the heap that are not used by any
 *   client objects. 
 */
/*
 *   UNUSED LIST: this is a list, headed by context->mcmcxunu and chained
 *   forward by mcmonxt (not back, because it's never necessary to take
 *   anything out of the list except at the head, nor to search the list
 *   backwards), of unused cache object entries.  These entries are not
 *   associated with any client object or with any heap memory.  This list
 *   is used to get a new cache object header, and deleted cache objects
 *   are placed onto this list.  
 */
/*
 *   LRU LIST: this is a list of in-memory blocks in ascending order of
 *   recency of use by the client.  Each time a client unlocks a block,
 *   the block is moved to the most recent position in the list (the end
 *   of the list).  To make it fast to add a new object, we keep a pointer
 *   to the end of the list as well as to the beginning.  The start of the
 *   list is at context->mcmcxlru, and is the least recently unlocked
 *   block still in memory.  The end of the list is at context->mcmcxmru,
 *   and is the most recently unlocked block.  
 */

/*
 *   initialize the cache manager, returning a context for cache manager
 *   operations; a null pointer is returned if insufficient heap memory is
 *   available for initialization.  The 'max' argument specifies the
 *   maximum amount of actual low-level heap memory that the cache manager
 *   can ever allocate on behalf of this context (of course, it can
 *   overcommit the heap through swapping).  If 'max' is less than the
 *   size of a single heap allocation, it is adjusted upwards to that
 *   minimum.  
 */
mcmcx1def *mcmini(ulong max, uint pages, ulong swapsize,
                  osfildef *swapfp, char *swapfilename, errcxdef *errctx);

/* terminate the cache manager - frees the structure and all cache memory */
void mcmterm(mcmcx1def *ctx);

/* allocate a client context */
mcmcxdef *mcmcini(mcmcx1def *globalctx, uint pages,
                  void (*loadfn)(void *, mclhd, uchar *, ushort),
                  void *loadctx,
                  void (*revertfn)(void *, mcmon), void *revertctx);

/* terminate a client context - frees the structure memory */
void mcmcterm(mcmcxdef *ctx);


/*
 *   Lock a cache object, bringing it into memory if necessary.  Returns
 *   a pointer to the memory containing the object.  A null pointer is
 *   returned in case of error.  The object remains fixed in memory at the
 *   returned location until unlocked.  Locks are stacked; that is, if
 *   an object is locked twice in succession, it needs to be unlocked
 *   twice in succession before it is actually unlocked.
 */
#ifdef MCM_NO_MACRO
uchar *mcmlck(mcmcxdef *ctx, mcmon objnum);
#else /* MCM_NO_MACRO */

/* uchar *mcmlck(mcmcxdef *ctx, mcmon objnum); */
#define mcmlck(ctx,num) \
 ((mcmobje(ctx,num)->mcmoflg & MCMOFPRES ) ? \
 ((mcmobje(ctx,num)->mcmoflg|=MCMOFLOCK), \
  ++(mcmobje(ctx,num)->mcmolcnt), mcmobje(ctx,num)->mcmoptr) \
 : mcmload(ctx,num))

#endif /* MCM_NO_MACRO */

/*
 *   Unlock a cache object, allowing it to be moved and swapped.
 *   Unlocking an object moves it to the end (i.e., most recently used)
 *   position on the LRU chain, making it the least favorable to swap out
 *   or discard.  This happens at unlock time (rather than lock time)
 *   because presumably the client has been using the object the entire
 *   time it was locked. For this reason, and to keep memory unfragmented
 *   as much as possible, objects should not be kept locked except when
 *   actually in use.  Note that locks nest; if an object is locked three
 *   times without an intervening unlock, it must be unlocked three times
 *   in a row.  An object can be unlocked even if it's not locked; doing
 *   so has no effect.
 */
#ifdef MCM_NO_MACRO
void mcmunlck(mcmcxdef *ctx, mcmon objnum);
#else /* MCM_NO_MACRO */

/* void mcmunlck(mcmcxdef *ctx, mcmon objnum); */
#define mcmunlck(ctx,obj) \
 ((mcmobje(ctx,obj)->mcmoflg & MCMOFLOCK) ? \
  (--(mcmobje(ctx,obj)->mcmolcnt) ? (void)0 : \
  ((mcmobje(ctx,obj)->mcmoflg&=(~MCMOFLOCK)), \
    mcmuse((ctx)->mcmcxgl,mcmc2g(ctx,obj)))) : (void)0)

#endif /* MCM_NO_MACRO */

/*
 *   Allocate a new cache object.  The new object is locked upon return.
 *   A pointer to the memory for the new object is returned, and
 *   the object number is returned at *nump.  A null pointer is returned
 *   if the object cannot be allocated.
 */
/* uchar *mcmalo(mcmcxdef *ctx, ushort siz, mcmon *nump); */
#define mcmalo(ctx, siz, nump) mcmalo0(ctx, siz, nump, MCMONINV, FALSE)

/*
 *   Reserve space for an object, giving it a particular client object
 *   number.  This doesn't actually allocate any space for the object, but
 *   just sets it up so that it can be loaded by the client when it's
 *   needed.  
 */
void mcmrsrv(mcmcxdef *ctx, ushort siz, mcmon clinum, mclhd loadhd);

/*
 *   Allocate a new cache object, and associate it with a particular
 *   client object number.  An error is signalled if the client object
 *   number is already in use.
 */
/* uchar *mcmalonum(mcmcxdef *ctx, ushort siz, mcmon num); */
#define mcmalonum(ctx, siz, num) mcmalo0(ctx, siz, (mcmon *)0, num, FALSE)

/*
 *   Reallocate an existing object.  The object's size is increased
 *   or reduced according to newsize.  The object is locked if it is
 *   not already, and the address of the object's memory is returned.
 *   Note that the object can move when reallocated, even if it was
 *   locked before the call.
 */
uchar *mcmrealo(mcmcxdef *ctx, mcmon objnum, ushort newsize);

/*
 *   Touch a cache object (rendering it dirty).  When an object is
 *   written, the client must touch it to ensure that the version in
 *   memory is not discarded.  The cache manager attempts to optimize
 *   activity by not writing objects that can be reconstructed from the
 *   load or swap file.  Touching the object informs the cache manager
 *   that the object is different from any version it has in a swap or
 *   load file.  
 */
/* void mcmtch(mcmcxdef *ctx, mcmon objnum); */
#define mcmtch(ctx,obj) \
  (mcmobje(ctx,obj)->mcmoflg |= MCMOFDIRTY)
/* was: (mcmobje(ctx,obj)->mcmoflg |= (MCMOFDIRTY | MCMOFNODISC)) */

/* get size of a cache manager object - object need not be locked */
/* ushort mcmobjsiz(mcmcxdef *ctx, mcmon objn); */
#define mcmobjsiz(ctx, objn) (mcmobje(ctx, objn)->mcmosiz)

/* determine if object has ever been touched */
/* int mcmobjdirty(mcmcxdef *ctx, mcmon objn); */
#define mcmobjdirty(ctx, objn) \
 (mcmobje(ctx, objn)->mcmoflg & (MCMOFDIRTY | MCMOFNODISC))

/* get object's memory pointer - object must be locked for valid result */
/* uchar *mcmobjptr(mcmcxdef *ctx, mcmon objn); */
#define mcmobjptr(ctx, objn) (mcmobje(ctx, objn)->mcmoptr)

/*
 *   Free an object.  The memory occupied by the object is discarded, and
 *   the object may no longer be referenced.  
 */
void mcmfre(mcmcxdef *ctx, mcmon obj);

/*
 *   "Revert" an object - convert it back to original state.  This
 *   routine just invokes a client callback to do the actual reversion
 *   work.  The callback is called immediately if the object is already
 *   present in memory, but is deferred until the object is loaded/swapped
 *   in if the object is not in memory. 
 */
/* void mcmrevert(mcmcxdef *ctx, mcmon objn); */
#define mcmrevert(ctx, objn) \
 ((mcmobje(ctx, objn)->mcmoflg & MCMOFPRES) ? \
  ((*(ctx)->mcmcxrvf)((ctx)->mcmcxrvc, objn), DISCARD 0) \
  : DISCARD (mcmobje(ctx, objn)->mcmoflg |= MCMOFREVRT))

/* get current size of object cache */
ulong mcmcsiz(mcmcxdef *ctx);

/* change an object's swap handle (used by swapper) */
/* void mcmcsw(mcmcx1def *ctx, ushort objn, mcsseg swapn, mcsseg oldswn); */
#define mcmcsw(ctx, objn, swapn, oldswapn) \
   ((*(ctx)->mcmcxcsw)(ctx, objn, swapn, oldswapn))

/* ------------------------------- PRIVATE ------------------------------- */

/* Unlock an object by its global handle */
#ifdef MCM_NO_MACRO
void mcmgunlck(mcmcx1def *ctx, mcmon objnum);
#else /* MCM_NO_MACRO */

/* void mcmgunlck(mcmcx1def *ctx, mcmon objnum); */
#define mcmgunlck(ctx,obj) \
 ((mcmgobje(ctx,obj)->mcmoflg & MCMOFLOCK) ? \
  (--(mcmgobje(ctx,obj)->mcmolcnt) ? (void)0 : \
    ((mcmgobje(ctx,obj)->mcmoflg&=(~MCMOFLOCK)), mcmuse(ctx,obj))) : \
  (void)0)

#endif /* MCM_NO_MACRO */

/* real memory allocator; clients use cover macros */
uchar *mcmalo0(mcmcxdef *ctx, ushort siz, mcmon *nump, mcmon clinum,
               int noclitrans);

/* free an object by global object number */
void mcmgfre(mcmcx1def *ctx, mcmon obj);

/* "use" an object (move to most-recent position in LRU chain) */
void mcmuse(mcmcx1def *ctx, mcmon n);

/*
 *   Load or swap in a cache object which is currently unloaded, locking
 *   it before returning.  Returns a pointer to the memory containing
 *   the object, or a null pointer in case of error.  The object
 *   remains fixed in memory at the returned location until unlocked.
 */
uchar *mcmload(mcmcxdef *ctx, mcmon objnum);

/*
 *   Size of each chunk of memory we'll request from the heap manager.
 *   To cut down on wasted memory from the heap manager, we'll always make
 *   our requests in this large size, then sub-allocate out of these
 *   chunks. 
 */
#define MCMCHUNK  32768

/*
 *   number of cache entries in a page - make this a power of 2 to keep
 *   the arithmetic to find a cache object entry simple 
 */
#define MCMPAGECNT 256

/*
 *   size of a page, in bytes
 */
#define MCMPAGESIZE (MCMPAGECNT * sizeof(mcmodef))

/*
 *   When allocating memory, and we find a free block satisfying the
 *   request, we will split the free block if doing so would result in
 *   enough space in the second block.  MCMSPLIT specifies the minimum
 *   size left over that will allow a split to occur.
 */
#define MCMSPLIT 64

/* get an object cache entyr given a GLOBAL object number */
#define mcmgobje(ctx,num) (&((ctx)->mcmcxtab[(num)>>8][(num)&255]))

/* get an object cache entry given a CLIENT object number */
/* mcmodef *mcmobje(mcmcxdef *ctx, mcmon objnum) */
#define mcmobje(ctx,num) mcmgobje((ctx)->mcmcxgl,mcmc2g(ctx,num))

/* allocate a block that will be locked for its entire lifetime */
void *mcmptralo(mcmcxdef *ctx, ushort siz);

/* free a block allocated with mcmptralo */
void mcmptrfre(mcmcxdef *ctx, void *block);

/* change an object's swap handle */
void mcmcswf(mcmcx1def *ctx, mcmon objn, mcsseg swapn, mcsseg oldswapn);

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
