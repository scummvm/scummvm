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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_RMEM_H
#define SAGA2_RMEM_H

namespace Saga2 {

// memory block magic ID's
#define RMEM_MAGIC_ID   ('E' + 'O'*256)
#define RMEM_FREED_ID   ('O' + 'E'*256)
#define RMEM_LEAK_ID    ('O' + 'O'*256)
#define RMEM_WALL_ID    0xAE

// Wiping allocated blocks
#define RMEM_WIPE_ALLOC 1
#define RMEM_ALLOC_WIPE_ID 0x90

// Wiping freed blocks
#define RMEM_WIPE_FREE  1
#define RMEM_FREE_WIPE_ID 0xCD

//  Set this to 1 to display the memory blocks on the screen...
#define RMEM_VISIBLE        0

// Wall Size
#define RMEM_WALL_SIZE      8                       // memory wall size around allocs
#define RMEM_MUNG_ENABLE    1                       // enable munging of free blocks

// enables the automatic setting to NULL of freed RPtrs
#define RMEM_AUTO_NULL      1

// allows the global new and delete to be redirected into the heap
#define RMEM_TRAP_NEW_DELETE  1
// enables malloc and free macros
#define RMEM_TRAP_MALLOC_FREE 1

// enable warnings about things that aren't cleaned up
#define RMEM_WARN_LEAKS 1


/* ===================================================================== *
    Common Memory Allocation Types
    Based on std.h types.
 * ====================================================================== */

typedef int8        *BytePtr,
        * *ByteHandle;

typedef uint8       *UBytePtr,
        * *UByteHandle;

typedef int16       *WordPtr,
        * *WordHandle;

typedef uint16      *UWordPtr,
        * *UWordHandle;

typedef int32       *LongPtr,
        * *LongHandle;

typedef uint32      *ULongPtr,
        * *ULongHandle;

typedef bool        *BoolPtr,
        * *BoolHandle;

typedef void        **RHANDLE;

/* ===================================================================== *
    Some other types
 * ====================================================================== */

typedef struct _RHeap   RHeap,
	        *RHeapPtr;

typedef struct _RHandle RHandle;

/* ===================================================================== *
   Basic functions
 * ===================================================================== */

//  Allocate and deallocate a heap
RHeapPtr _RNewHeap(uint8 *memory, int32 size);
void _RDisposeHeap(RHeapPtr heap);

//  Allocate and Deallocate a block of non-relocatable memory
void *_RNewPtr(int32 size, RHeapPtr heap, const char []);
void *_RNewClearPtr(int32 size, RHeapPtr heap, const char []);
void _RVMMLockPtr(void *ptr);
void _RVMMUnlockPtr(void *ptr);
void _RDisposePtr(void *ptr);

//  Allcate and Deallocate a block of relocatable memory
RHANDLE _RNewHandle(int32 size, RHeapPtr heap, const char []);
RHANDLE _RNewClearHandle(int32 size, RHeapPtr heap, const char []);
void _RDisposeHandle(RHANDLE handle);

//  Allocate and Deallocate a data associated with handle
void *_RAllocHandleData(RHANDLE handle, int32 size, const char []);
void _RDisposeHandleData(RHANDLE handle);

//  Lock a handle (prevent from moving)
void *_RLockHandle(RHANDLE handle);
void _RUnlockHandle(RHANDLE handle);

//  Get and Set the attributes of a handle
//void RSetHandleFlags( void *handle, uint16 newflags );
uint16 _RGetHandleFlags(RHANDLE handle);

//  Set this handle as cacheable
void _RCacheHandle(RHANDLE handle);

//  Functions to mark a handle as "loading"
void _RHandleStartLoading(RHANDLE handle);
void _RHandleDoneLoading(RHANDLE handle);

//  Return true if loaded data is valid
bool _RHandleLoaded(RHANDLE handle);

//  Return true if loaded data will be valid soon.
bool _RHandleLoading(RHANDLE handle);

void RShowMem(void);
RHeapPtr whichHeapPublic(void *ptr);
RHeapPtr getHeap(void);

/* ===================================================================== *
   Advanced Functions (do not use in portable applications!)
 * ===================================================================== */

//  Link a handle into a chain of handles
void _RLinkHandle(RHandle *r, RHandle *prev);
void _RDelinkHandle(RHandle *r);

//  Get the size of the allocated block
uint32 _RPtrSize(void *ptr);

//  Get the size of the allocated block
bool _RPtrVMMLocked(void *ptr);

//  Test memory heap integrity
bool _RMemIntegrity(void);

void heapCheck(void);
void _RCheckPtr(void *ptr);


/* ===================================================================== *
   Include logging routines
 * ===================================================================== */

//
// Would the VMM be more effective if major memory
//   reorganizations were triggered by the application?
//   This would allow complete cleanups to be done
//   at times when the system was least stressed

//
// The logging of memory actions is done in a
//   rather brute force manner.  The original routines
//   have been renamed. Macros have been defined
//   to redirect those calls. If logging is not in use
//   the original calls will be run. When logging is
//   enabled routines are called which do the dumping.
//

//
// Using these routines:
//   To start logging memory manager calls use:
//     startLogging();
//   To pause/stop logging use:
//     stopLogging();
//   To resume a paused log use:
//     continueLogging();
//

//
// Logging options
//
//   Routines Logged
//     All the RMEM.C routines which are public can
//       be logged.
//
//   Log Elements
//     Module dump : shows what call is being made and
//       where in the source code it was called from
//     In : The values of the parameters on entry to
//       the routine
//     Out : The values of the parameters when returning
//       from the routine
//     Return : shows the value returned by the routine
//
//

#if DEBUG


// Log file initialization / termination

void initRMemLogging(void);
void startLog(char *filename);
void startLogging(void);
void continueLogging(void);
void stopLogging(void);

//
// These are the logging versions
//

RHeapPtr logRNewHeap(uint8 *m, int32 s, const int l, const char f[]);
void logRDisposeHeap(RHeapPtr h, const int l, const char f[]);
void *logRNewPtr(int32 s, RHeapPtr h, const char desc[], const int l, const char f[]);
void *logRNewClearPtr(int32 s, RHeapPtr h, const char desc[], const int l, const char f[]);
void *logRDisposePtr(void *p, const int l, const char f[]);
RHANDLE logRNewHandle(int32 s, RHeapPtr h, const char desc[], const int l, const char f[]);
RHANDLE logRNewClearHandle(int32 s, RHeapPtr h, const char desc[], const int l, const char f[]);
void logRDisposeHandle(RHANDLE h, const int l, const char f[]);
void *logRAllocHandleData(RHANDLE h, int32 s, const char desc[], const int l, const char f[]);
void logRDisposeHandleData(RHANDLE h, const int l, const char f[]);
void *logRLockHandle(RHANDLE h, const int l, const char f[]);
void logRUnlockHandle(RHANDLE h, const int l, const char f[]);
uint16 logRGetHandleFlags(RHANDLE h, const int l, const char f[]);
void logRCacheHandle(RHANDLE h, const int l, const char f[]);
void logRHandleStartLoading(RHANDLE h, const int l, const char f[]);
void logRHandleDoneLoading(RHANDLE h, const int l, const char f[]);
bool logRHandleLoaded(RHANDLE h, const int l, const char f[]);
bool logRHandleLoading(RHANDLE h, const int l, const char f[]);
void logRLinkHandle(RHandle *r, RHandle *p, const int l, const char f[]);
void logRDelinkHandle(RHandle *r, const int l, const char f[]);
uint32 logRPtrSize(void *p, const int l, const char f[]);
bool logRMemIntegrity(const int l, const char f[]);

//
// These are the remapped memory management routines
//

#define RNewHeap(m,s)           (logRNewHeap(m,s,__LINE__,__FILE__))
#define RDisposeHeap(h)         (logRDisposeHeap(h,__LINE__,__FILE__))
#define RNewPtr(s,h,d)          (logRNewPtr(s,h,d,__LINE__,__FILE__))
#define RNewClearPtr(s,h,d)     (logRNewClearPtr(s,h,d,__LINE__,__FILE__))
#if RMEM_AUTO_NULL
#define RDisposePtr(p)          (logRDisposePtr(p,__LINE__,__FILE__),p=NULL)
#else
#define RDisposePtr(p)          (logRDisposePtr(p,__LINE__,__FILE__))
#endif
#define RNewHandle(s,h,d)       (logRNewHandle(s,h,d,__LINE__,__FILE__))
#define RNewClearHandle(s,h,d)  (logRNewClearHandle(s,h,d,__LINE__,__FILE__))
#define RDisposeHandle(h)       (logRDisposeHandle(h,__LINE__,__FILE__))
#define RAllocHandleData(h,s,d) (logRAllocHandleData(h,s,d,__LINE__,__FILE__))
#define RDisposeHandleData(h)   (logRDisposeHandleData(h,__LINE__,__FILE__))
#define RLockHandle(h)          (logRLockHandle(h,__LINE__,__FILE__))
#define RUnlockHandle(h)        (logRUnlockHandle(h,__LINE__,__FILE__))
#define RGetHandleFlags(h)      (logRGetHandleFlags(h,__LINE__,__FILE__))
#define RCacheHandle(h)         (logRCacheHandle(h,__LINE__,__FILE__))
#define RHandleStartLoading(h)  (logRHandleStartLoading(h,__LINE__,__FILE__))
#define RHandleDoneLoading(h)   (logRHandleDoneLoading(h,__LINE__,__FILE__))
#define RHandleLoaded(h)        (logRHandleLoaded(h,__LINE__,__FILE__))
#define RHandleLoading(h)       (logRHandleLoading(h,__LINE__,__FILE__))
#define RLinkHandle(r,p)        (logRLinkHandle(r,p,__LINE__,__FILE__))
#define RDelinkHandle(r)        (logRDelinkHandle(r,__LINE__,__FILE__))
#define RPtrSize(p)             (logRPtrSize(p,__LINE__,__FILE__))
#define RMemIntegrity()         (logRMemIntegrity(__LINE__,__FILE__))

#else   // DEBUG

#define RNewHeap(m,s)           (_RNewHeap(m,s))
#define RDisposeHeap(h)         (_RDisposeHeap(h))
#define RNewPtr(s,h,d)          (_RNewPtr(s,h,d))
#define RNewClearPtr(s,h,d)     (_RNewClearPtr(s,h,d))
#if RMEM_AUTO_NULL
typedef void *pVOID;
#define RDisposePtr(p)          (_RDisposePtr(p),p=NULL)
#else
#define RDisposePtr(p)          (_RDisposePtr(p))
#endif
#define RNewHandle(s,h,d)       (_RNewHandle(s,h,d))
#define RNewClearHandle(s,h,d)  (_RNewClearHandle(s,h,d))
#define RDisposeHandle(h)       (_RDisposeHandle(h))
#define RAllocHandleData(h,s,d) (_RAllocHandleData(h,s,d))
#define RDisposeHandleData(h)   (_RDisposeHandleData(h))
#define RLockHandle(h)          (_RLockHandle(h))
#define RUnlockHandle(h)        (_RUnlockHandle(h))
#define RGetHandleFlags(h)      (_RGetHandleFlags(h))
#define RCacheHandle(h)         (_RCacheHandle(h))
#define RHandleStartLoading(h)  (_RHandleStartLoading(h))
#define RHandleDoneLoading(h)   (_RHandleDoneLoading(h))
#define RHandleLoaded(h)        (_RHandleLoaded(h))
#define RHandleLoading(h)       (_RHandleLoading(h))
#define RLinkHandle(r,p)        (_RLinkHandle(r,p))
#define RDelinkHandle(r)        (_RDelinkHandle(r))
#define RPtrSize(p)             (_RPtrSize(p))
#define RMemIntegrity           (_RMemIntegrity)

#endif // if/else DEBUG

} // end of namespace Saga2

#endif
