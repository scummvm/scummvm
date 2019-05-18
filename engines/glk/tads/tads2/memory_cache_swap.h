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
 * Memory cache swap manager
 *
 * The cache swap manager provides swap file services to the memory
 * cache manager.  The cache manager calls the swap manager to write
 * objects to the swap file and read in previously swapped-out objects.
 */

#ifndef GLK_TADS_TADS2_MEMORY_CACHE_SWAP
#define GLK_TADS_TADS2_MEMORY_CACHE_SWAP

#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/error_handling.h"
#include "glk/tads/os_frob_tads.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* Forward declarations */
struct mcmcx1def;

/**
 * swap segment descriptor
 */
struct mcsdsdef {
    ulong    mcsdsptr;                         /* seek pointer in swap file */
    ushort   mcsdssiz;                         /* size of this swap segment */
    ushort   mcsdsosz;                 /* size of object written to segment */
    uint     mcsdsobj;                                  /* client object ID */
    ushort   mcsdsflg;                                             /* flags */
#define      MCSDSFINUSE   0x01                        /* segment is in use */
};

/**
 * mcsseg - swap segment handle.  All swap-file segments are addressed
 * through this handle type.  
 */
typedef ushort mcsseg;

/**
 * Swap manager context
 */
struct mcscxdef {
    osfildef   *mcscxfp;                                /* swap file handle */
    char       *mcscxfname;                            /* name of swap file */
    errcxdef   *mcscxerr;                         /* error handling context */
    ulong       mcscxtop;              /* top of swap file allocated so far */
    ulong       mcscxmax;        /* maximum size of swap file we're allowed */
    mcsdsdef  **mcscxtab;                     /* swap descriptor page table */
    mcsseg      mcscxmsg;               /* maximum segment allocated so far */
    mcmcx1def *mcscxmem;                   /* memory manager context */
};

#define MCSSEGINV ((mcsseg)~0)      /* invalid segment ID - error indicator */

/* initialize swapper - returns 0 for success, other for error */
void mcsini(struct mcscxdef *ctx, struct mcmcx1def *gmemctx, ulong maxsiz,
            osfildef *fp, char *swapfilename, struct errcxdef *errctx);

/* close swapper (release memory areas) */
void mcsclose(struct mcscxdef *ctx);

/**
 *   Swap an object out.  The caller specifies the location and size of
 *   the object, as well as a unique handle (arbitrary, up to the caller;
 *   the only requirement is that it be unique among all caller objects
 *   and always the same for a particular caller's object) and the
 *   previous swap handle if the object ever had one.  If the object is
 *   not dirty (it hasn't been written since being swapped in), and the
 *   swap manager hasn't reused the swap slot, the swap manager doesn't
 *   need to write the memory, since it already has a copy on disk;
 *   instead, it can just mark the slot as back in use.  If the caller
 *   doesn't wish to take advantage of this optimization, always pass in
 *   dirty == TRUE, which will force a write regardless of the object ID.
 */
mcsseg mcsout(struct mcscxdef *ctx, uint objid, uchar *objptr,
              ushort objsize, mcsseg oldswapseg, int dirty);
            
/* Swap an object in */
void mcsin(struct mcscxdef *ctx, mcsseg swapseg, uchar *objptr, ushort size);


/* number of page pointers in page table (max number of pages) */
#define MCSPAGETAB 256

/* number of swap descriptors in a page */
#define MCSPAGECNT 256

/* find swap descriptor corresponding to swap segment number */
#define mcsdsc(ctx,seg) (&(ctx)->mcscxtab[(seg)>>8][(seg)&255])

/* write out a swap segment */
void mcswrt(mcscxdef *ctx, mcsdsdef *desc, uchar *buf, ushort bufl);

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
