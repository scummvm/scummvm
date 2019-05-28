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

#ifndef GLK_TADS_TADS2_OBJECT
#define GLK_TADS_TADS2_OBJECT

#include "glk/tads/tads.h"
#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/memory_cache.h"
#include "glk/tads/tads2/property.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/**
 * object number
 */
typedef ushort objnum;

/**
 * For non-class objects, we'll leave some space free in the object so
 * that a few properties can be added without having to resize the
 * object.  Class objects will probably never have anything added, so
 * there's no need for extra space.  
 */
#define OBJEXTRA 64

#   define   OBJFCLS   0x01                            /* object is a class */

/**
 * The object structure is actually laid out portably, using unaligned
 * 2-byte arrays, stored least significant byte first, for each ushort
 * (including the objnum array for the superclasses).  The actual
 * entries are at these offsets on all machines: 
 *
 *      objws      0
 *      objflg     2
 *      objnsc     4
 *      objnprop   6
 *      objfree    8
 *      objrst     10
 *      objstat    12
 *      objsc[0]   14
 *      objsc[1]   16
 *      etc
 *      
 * If the OBJFINDEX flag is set, the object has a property index.
 * The index occurs after the last superclass (so it's where the
 * property data would go if there were no index), and the property
 * data follows.  Each index entry consists of a pair of two-byte
 * entries:  the first is the property number, and the second is
 * its offset within the object.  For performance reasons, an index
 * is only built on a class object -- whenever a property is changed
 * within an object, the entire index must be rebuilt, because the
 * locations of many properties within the object can be changed by
 * a single property change in the object.  The index is ordered by
 * property number, so it can be searched using a binary search.
 * Furthermore, "ignored" properties are excluded from the index;
 * only the active instance of a particular property is stored.
 * The index must be maintained by all routines that can change
 * property information:  setp, delp, revert, etc.
 * 
 * Preceding the index table is a two-byte entry that gives the
 * offset of the properties.  Since the properties immediately
 * follow the index, this can be used to deduce how large a space
 * is available for the index itself.
 */
typedef uchar objdef;
#define OBJDEFSIZ 14   /* "sizeof(objdef)" - size of object header w/o sc's */

/* object flags */
#define OBJFCLASS  1                                   /* object is a class */
#define OBJFINDEX  2                         /* object has a property index */
#define OBJFMOD    4      /* object has been modified by a newer definition */

/* undo context */
struct objucxdef {
    mcmcxdef *objucxmem;                           /* cache manager context */
    errcxdef *objucxerr;                                   /* error context */
    ushort    objucxsiz;                         /* size of the undo buffer */
    ushort    objucxhead;                  /* head (position of next write) */
    ushort    objucxtail;               /* tail (position of oldest record) */
    ushort    objucxprv;                           /* previous head pointer */
    ushort    objucxtop;                      /* highest head value written */
    void    (*objucxcun)(void *ctx, uchar *data);
                                              /* apply a client undo record */
    ushort  (*objucxcsz)(void *ctx, uchar *data);
                                        /* get size of a client undo record */
    void     *objucxccx;                             /* client undo context */
    uchar     objucxbuf[1];                                  /* undo buffer */
};

/*
 *   Undo records are kept in a circular buffer allocated as part of an
 *   undo context.  Offsets within the buffer are kept for the head, tail,
 *   and previous head records.  The head always points to the byte at
 *   which the next undo record will be written.  The previous head points
 *   to the most recently written undo record; it contains a back link to
 *   the undo record before that, and so forth back through the entire
 *   chain.  (These reverse links are necessary because undo records vary
 *   in size depending on the data contained within.)  The tail points to
 *   the oldest undo record that's still in the buffer.  Conceptually, the
 *   head is always "above" the tail in the buffer; since the buffer is
 *   circular, the tail may have a higher address, but this just means
 *   that the buffer wraps around at the top.  When the head bumps into
 *   the tail (i.e., the head address is physically below or equal to the
 *   tail address, and the head is then advanced so that its address
 *   becomes higher than the tail's), the tail is advanced by discarding
 *   as many of the least recent undo records as necessary to make room
 *   for the new head position.  When the head and the previous head point
 *   to the same place, we have no undo records in the buffer.  
 */
/**
 *   The first byte of an undo record specifies what action is to be
 *   undone.  If a property was added, it is undone merely by deleting the
 *   property.  If a property was changed, it is undone by setting the
 *   property back to its old value.  An additional special flag indicates
 *   a "savepoint."  Normally, all changes back to a savepoint will be
 *   undone.  
 */
#define OBJUADD    1             /* a property was added (undo by deleting) */
#define OBJUCHG    2   /* a property was changed (change back to old value) */
#define OBJUSAV    3          /* savepoint marker (no property information) */
#define OBJUOVR    4     /* override original property (set orig to IGNORE) */
#define OBJUCLI    5                /* client undo record (any client data) */

/*
 *   After the control byte (OBJUxxx), the object number, property
 *   number, datatype, and data value will follow; some or all of these
 *   may be omitted, depending on the control byte. 
 */

/* get object flags */
#define objflg(o) ((ushort)osrp2(((char *)(o)) + 2))

/* get object flags */
#define objsflg(o, val) oswp2(((char *)(o)) + 2, val)

/* given an object pointer, get a pointer to the first prpdef */
/* prpdef *objprp(objdef *objptr); */
#define objprp(o) ((prpdef *)(objsc(o) + 2*objnsc(o)))

/* given an object pointer, get number of properties in the prpdef */
/* int objnprop(objdef *objptr); */
#define objnprop(o) ((ushort)osrp2(((char *)(o)) + 6))

/* set number of properties */
/* void objsnp(objdef *objptr, int newnum); */
#define objsnp(o,n) oswp2(((char *)(o)) + 6, n)

/* given an object pointer, get offset of free space */
/* int objfree(objdef *objptr); */
#define objfree(o) ((ushort)osrp2(((char *)(o)) + 8))

/* set free space pointer */
/* void objsfree(objdef *objptr, int newfree); */
#define objsfree(o,n) oswp2(((char *)(o)) + 8, n)

/* get number of static properties */
/* ushort objstat(objdef *objptr); */
#define objstat(o) ((ushort)osrp2(((char *)(o)) + 10))

/* set number of static properties */
/* void objsetst(objdef *objptr, int newstat); */
#define objsetst(o,n) oswp2(((char *)(o)) + 10, n)

/* get reset size (size of static properties) */
/* ushort objrst(objdef *objptr); */
#define objrst(o) ((ushort)osrp2(((char *)(o)) + 12))

/* set reset size */
/* void objsetrst(objdef *objptr, uint newrst); */
#define objsetrst(o,n) oswp2(((char *)(o)) + 12, n)
 
/* given an object pointer, get first superclass pointer */
/* uchar *objsc(objdef *objptr); */
#define objsc(o) (((uchar *)(o)) + OBJDEFSIZ)

/* given an object pointer, get number of superclasses */
/* int objnsc(objdef *objptr); */
#define objnsc(o) ((ushort)osrp2(((char *)(o)) + 4))

/* set number of superclasses */
/* void objsnsc(objdef *objptr, int num); */
#define objsnsc(o,n) oswp2(((char *)(o)) + 4, n)

/* given a prpdef, get the next prpdef */
/* prpdef *objpnxt(prpdef *p); */
#define objpnxt(p) \
 ((prpdef *)(((uchar *)(p)) + PRPHDRSIZ + prpsize(p)))

/* get pointer to free prpdef */
/* prpdef *objpfre(objdef *objptr); */
#define objpfre(o) ((prpdef *)(((uchar *)(o)) + objfree(o)))

/* given a prpdef and an object pointer, compute the prpdef offset */
/* uint objpofs(objdef *objptr, prpdef *propptr); */
#define objpofs(o,p) ((uint)((p) ? (((uchar *)(p)) - ((uchar *)(o))) : 0))

/* given an object pointer and a property offset, get prpdef pointer */
/* prpdef *objofsp(objdef *objptr, uint propofs); */
#define objofsp(o,ofs) ((prpdef *)((ofs) ? (((uchar *)(o)) + (ofs)) : 0))

/*
 *   Get the first superclass of an object.  If it doesn't have any
 *   superclasses, return invalid.
 */
objnum objget1sc(mcmcxdef *ctx, objnum objn);

/*
 *   Get an object's property WITHOUT INHERITANCE.  If the object has the
 *   indicated property set, the byte OFFSET of the prpdef within the
 *   object is returned.  The offset will remain valid until any type of
 *   operation that sets a property in the object (such as objdelp,
 *   objsetp, or an undo operation).  An offset of zero means that the
 *   property was not set in the object.
 */
uint objgetp(mcmcxdef *ctx, objnum objn, prpnum prop,
             dattyp *typptr);

/*
 *   Get the *ending* offset of the given property's value, without any
 *   inheritance.  Returns the byte offset one past the end of the
 *   property's data.  
 */
uint objgetp_end(mcmcxdef *ctx, objnum objn, prpnum prop);

/*
 *   Get a property of an object, either from the object or from a
 *   superclass (inherited).  If the inh flag is TRUE, we do not look
 *   at all in the object itself, but restrict our search to inherited
 *   properties only.  We return the byte OFFSET of the prpdef within
 *   the object in which the prpdef is found; the superclass object
 *   itself is NOT locked upon return, but we will NOT unlock the
 *   object passed in.  If the offset is zero, the property was not
 *   found.  The offset returned is valid until any operation that
 *   sets a property in the object (such as objdelp, objsetp, or an
 *   undo operation).
 */
uint objgetap(mcmcxdef *ctx, noreg objnum objn, prpnum prop,
              objnum *orn, int inh);

/*
 *   expand an object by a requested amount, returning a pointer to the
 *   object's new location if it must be moved.  The object will be
 *   unlocked and relocked by this call.  On return, the actual amount
 *   of space ADDED to the object will be returned.
 */
objdef *objexp(mcmcxdef *ctx, objnum obj, ushort *siz);

/*
 *   Set an object's property, deleting the original value of the
 *   property if it existed.  If an undo context is provided, write an
 *   undo record for the change; if the undo context pointer is null, no
 *   undo information is retained. 
 */
void objsetp(mcmcxdef *ctx, objnum obj, prpnum prop,
             dattyp typ, const void *val, objucxdef *undoctx);

/* 
 *   Delete a property.  If mark_only is true, we'll only mark the
 *   property as deleted without actually reclaiming its space; this is
 *   necessary when removing a code property (type DAT_CODE) any time
 *   other code properties may follow, because p-code is not entirely
 *   self-relative and thus can't always be relocated within an object. 
 */
void objdelp(mcmcxdef *mctx, objnum objn, prpnum prop, int mark_only);

/*
 *   Set up for emitting code into an object.  Writes a property header
 *   of type 'code', and returns the offset of the next free byte in the
 *   object.  Call objendemt when done.  The datatype argument is
 *   provided so that list generation can be done through the same
 *   mechanism, since parser lists must be converted to run-time
 *   lists via the code generator.
 */
uint objemt(mcmcxdef *ctx, objnum objn, prpnum prop, dattyp typ);

/* done emitting code into property, finish setting object info */
void objendemt(mcmcxdef *ctx, objnum objn, prpnum prop, uint endofs);

/*
 *   Determine if undo records should be kept.  Undo records should be
 *   kept only if a savepoint is present in the undo log.  If no savepoint
 *   is present, adding undo records would be useless, since it will not
 *   be possible to apply the undo information. 
 */
int objuok(objucxdef *undoctx);

/*
 *   Reserve space in an undo buffer, deleting old records as needed.
 *   Returns a pointer to the reserved space. 
 */
uchar *objures(objucxdef *undoctx, uchar cmd, ushort siz);

/* advance the tail pointer in an undo buffer over the record it points to */
void objutadv(objucxdef *undoctx);

/* apply one undo record, and remove it from undo list */
void obj1undo(mcmcxdef *mctx, objucxdef *undoctx);

/*
 *   Undo back to the most recent savepoint.  If there is no savepoint in
 *   the undo list, NOTHING will be undone.  This prevents reaching an
 *   inconsistent state in which some, but not all, of the operations
 *   between two savepoints are undone: either all operations between two
 *   savepoints will be undone, or none will. 
 */
void objundo(mcmcxdef *mctx, objucxdef *undoctx);

/* set an undo savepoint */
void objusav(objucxdef *undoctx);

/* initialize undo context */
objucxdef *objuini(mcmcxdef *memctx, ushort undosiz,
                   void (*undocb)(void *ctx, uchar *data),
                   ushort (*sizecb)(void *ctx, uchar *data),
                   void *callctx);

/* free the undo context - releases memory allocated by objuini() */
void objuterm(objucxdef *undoctx);

/* discard all undo context (for times such as restarting) */
void objulose(objucxdef *undoctx);

/*
 *   Allocate and initialize a new object.  The caller specifies the
 *   number of superclasses to be allocated, and the amount of space (in
 *   bytes) for the object's property data.  The caller must fill in the
 *   superclass array.  Upon return, the object is allocated and locked,
 *   and is initialized with no properties.  A pointer to the object's
 *   memory is returned, and *objnptr receives the object number.
 */
objdef *objnew(mcmcxdef *mctx, int sccnt, ushort propspace,
               objnum *objnptr, int classflg);
            
/* initialize an already allocated object */
void objini(mcmcxdef *mctx, int sccnt, objnum objn, int classflg);

/*
 *   Add space for additional superclasses to an object.  The object can
 *   already have some properties set (if it doesn't, it can just be
 *   reinitialized).
 */
void objaddsc(mcmcxdef *mctx, int sccnt, objnum objn);

/*
 *   Delete an object's properties and superclasses.  The 'mindel'
 *   parameter specifies the minimum property number to be deleted.
 *   Properties below this are considered "system" properties that are not
 *   to be deleted.  This could be used by a development environment to
 *   store the source for an object as a special system property in the
 *   object; when the object is recompiled, all of the object's properties
 *   and superclasses must be deleted except the source property, which is
 *   retained even after recompilation. 
 */
void objclr(mcmcxdef *mctx, objnum objn, prpnum mindel);

/* Build or rebuild an object's property index */
void objindx(mcmcxdef *mctx, objnum objn);

/* set up just-compiled object: mark static part and original properties */
void objcomp(mcmcxdef *mctx, objnum objn, int for_debug);

/* revert an object to original post-compilation state */
void objrevert(void *mctx, mcmon objn);

/* reset 'ignore' flags for a newly reconstructed object */
void objsetign(mcmcxdef *mctx, objnum objn);


} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
