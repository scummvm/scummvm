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

#include "glk/tads/tads2/object.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/*
 *   Get a property WITHOUT INHERITANCE.  The offset of the property's
 *   prpdef is returned.  An offset of zero means the property wasn't
 *   found.
 */
uint objgetp(mcmcxdef *mctx, objnum objn, prpnum prop, dattyp *typptr)
{
    objdef *objptr;
    prpdef *p;
    int     cnt;
    uint    retval;                       /* property offset, if we find it */
    uint    ignprop; /* ignored property - use if real property isn't found */
    uchar   pbuf[2];                  /* property number in portable format */
    uchar  *indp = nullptr;
    uchar  *indbase;
    int     last;
    int     first;
    int     cur = 0;
    
    oswp2(pbuf, prop);            /* get property number in portable foramt */
    objptr = (objdef *)mcmlck(mctx, objn);      /* get a lock on the object */
    ignprop = 0;                   /* assume we won't find ignored property */
    cnt = objnprop(objptr);             /* get number of properties defined */
    retval = 0;                                          /* presume failure */
    
    if (objflg(objptr) & OBJFINDEX)
    {
        /* there's an index -> do a binary search through the index */
        indbase = (uchar *)objpfre(objptr);                   /* find index */
        first = 0;
        last = cnt - 1;
        for (;;)
        {
            if (first > last) break;           /* crossed over -> not found */
            cur = first + (last - first)/2;         /* split the difference */
            indp = indbase + cur*4;            /* get pointer to this entry */
            if (indp[0] == pbuf[0] && indp[1] == pbuf[1])
            {
                retval = osrp2(indp + 2);
                break;
            }
            else if (indp[0] < pbuf[0]
                     || (indp[0] == pbuf[0] && indp[1] < pbuf[1]))
                first = (cur == first ? first + 1 : cur);
            else
                last = (cur == last ? last - 1 : cur);
        }

        /* ignore ignored and deleted properties if possible */
        while (retval
               && ((prpflg(objptr + retval) & PRPFIGN) != 0
                   || ((prpflg(objptr + retval) & PRPFDEL) != 0
                       && (mctx->mcmcxflg & MCMCXF_NO_PRP_DEL) == 0))
               && cur < cnt && indp[0] == indp[4] && indp[1] == indp[5])
        {
            indp += 4;
            retval = osrp2(indp + 2);
        }
        if (retval && osrp2(objptr + retval) != prop)
            assert(FALSE);
    }
    else
    {
        /* there's no index -> do sequential search through properties */
        for (p = objprp(objptr) ; cnt ; p = objpnxt(p), --cnt)
        {
            /* if this is the property, and it's not being ignored, use it */
            if (*(uchar *)p == pbuf[0] && *(((uchar *)p) + 1) == pbuf[1])
            {
                if (prpflg(p) & PRPFIGN)                 /* this is ignored */
                    ignprop = objpofs(objptr, p);  /* ... make a note of it */
                else if ((prpflg(p) & PRPFDEL) != 0         /* it's deleted */
                         && (mctx->mcmcxflg & MCMCXF_NO_PRP_DEL) == 0)
                    /* simply skip it */ ;
                else
                {
                    retval = objpofs(objptr, p);         /* this is the one */
                    break;                                    /* we're done */
                }
            }
        }
    }

    if (!retval) retval = ignprop;     /* use ignored value if nothing else */
    if (retval && typptr) *typptr = prptype(objofsp(objptr, retval));
    
    mcmunlck(mctx, objn);                 /* done with object, so unlock it */
    return(retval);
}

/* get the offset of the end of a property in an object */
uint objgetp_end(mcmcxdef *ctx, objnum objn, prpnum prop)
{
    objdef *objp;
    prpdef *propptr;
    uint    ofs;
    uint    valsiz;
    
    /* get the start of the object */
    ofs = objgetp(ctx, objn, prop, 0);
    if (ofs == 0)
        return 0;

    /* get the object */
    objp = mcmlck(ctx, (mcmon)objn);

    /* get the property */
    propptr = objofsp(objp, ofs);

    /* get the data size */
    valsiz = prpsize(propptr);

    /* done with the object */
    mcmunlck(ctx, (mcmon)objn);

    /* 
     *   return the ending offset - it's the starting offset plus the
     *   property header size plus the size of the property data 
     */
    return ofs + PRPHDRSIZ + valsiz;
}

/* determine whether an object is a descendant of another object */
static int objisd(mcmcxdef *ctx, objdef *objptr, objnum parentnum)
{
    uchar *sc;
    int    cnt;
    
    for (sc = objsc(objptr), cnt = objnsc(objptr) ; cnt ;
         sc += 2, --cnt)
    {
        int     cursc = osrp2(sc);
        int     ret;
        objdef *curptr;
        
        if (cursc == parentnum) return(TRUE);
        
        curptr = (objdef *)mcmlck(ctx, (mcmon)cursc);
        ret = objisd(ctx, curptr, parentnum);
        mcmunlck(ctx, (mcmon)cursc);
        if (ret) return(TRUE);
    }
    return(FALSE);
}

/*
 *   Get a property of an object, either from the object or from a
 *   superclass (inherited).  If the inh flag is TRUE, we do not look at
 *   all in the object itself, but restrict our search to inherited
 *   properties only.  We return the byte offset of the prpdef within the
 *   object in which the prpdef is found; the superclass object itself is
 *   NOT locked upon return, but we will NOT unlock the object passed in
 *   (in other words, all object locking status is the same as it was on
 *   entry).  If the offset is zero, the property was not found.
 *   
 *   This is an internal helper routine - it's not meant to be called
 *   except by objgetap().  
 */
static uint objgetap0(mcmcxdef *ctx, noreg objnum obj, prpnum prop,
                      objnum *orn, int inh, dattyp *ortyp)
{
    uchar  *sc;
    ushort  sccnt;
    ushort  psav = 0;
    dattyp  typsav = DAT_NIL;
    objnum  osavn = MCMONINV;
    uchar  *o1;
    objnum  o1n;
    ushort  poff;
    int     found;
    uint    retval;
    dattyp  typ;
    uchar   sclist[100];                           /* up to 50 superclasses */
    objdef *objptr;
    
    NOREG((&obj))
    
    /* see if the property is in the current object first */
    if (!inh && (retval = objgetp(ctx, obj, prop, &typ)) != 0)
    {
        /* 
         *   tell the caller which object this came from, if the caller
         *   wants to know 
         */
        if (orn != 0)
            *orn = obj;

        /* if the caller wants to know the type, return it */
        if (ortyp != 0)
            *ortyp = typ;

        /* return the property offset */
        return retval;
    }
    
    /* lock the object, cache its superclass list, and unlock it */
    objptr = (objdef *)mcmlck(ctx, (mcmon)obj);
    sccnt = objnsc(objptr);
    memcpy(sclist, objsc(objptr), (size_t)(sccnt << 1));
    sc = sclist;
    mcmunlck(ctx, (mcmon)obj);
    
    /* try to inherit the property */
    for (found = FALSE ; sccnt != 0 ; sc += 2, --sccnt)
    {
        /* recursively look up the property in this superclass */
        poff = objgetap0(ctx, (objnum)osrp2(sc), prop, &o1n, FALSE, &typ);

        /* if we found the property, remember it */
        if (poff != 0)
        {
            int isdesc = 0;
            
            /* if we have a previous object, determine lineage */
            if (found)
            {
                o1 = mcmlck(ctx, o1n);
                isdesc = objisd(ctx, o1, osavn);
                mcmunlck(ctx, o1n);
            }
            
            /*
             *   if we don't already have a property, or the new object
             *   is a descendant of the previously found object (meaning
             *   that the new object's property should override the
             *   previously found object's property), use this new
             *   property 
             */
            if (!found || isdesc)
            {
                psav = poff;
                osavn = o1n;
                typsav = typ;
                found = TRUE;
            }
        }
    }

    /* set return pointer and return the offset of what we found */
    if (orn != 0)
        *orn = osavn;

    /* return the object type if the caller wanted it */
    if (ortyp != 0)
        *ortyp = typsav;

    /* return the offset of the property if we found one, or zero if not */
    return (found ? psav : 0);
}

/*
 *   Get a property of an object, either from the object or from a
 *   superclass (inherited).  If the inh flag is TRUE, we do not look at
 *   all in the object itself, but restrict our search to inherited
 *   properties only.  We return the byte offset of the prpdef within the
 *   object in which the prpdef is found; the superclass object itself is
 *   NOT locked upon return, but we will NOT unlock the object passed in
 *   (in other words, all object locking status is the same as it was on
 *   entry).  If the offset is zero, the property was not found.  
 */
uint objgetap(mcmcxdef *ctx, noreg objnum obj, prpnum prop,
              objnum *ornp, int inh)
{
    uint    retval;
    dattyp  typ;
    objnum  orn;

    /* 
     *   even if the caller doesn't care about the original object number,
     *   we do, so provide our own location to store it if necessary 
     */
    if (ornp == 0)
        ornp = &orn;

    /* keep going until we've finished translating synonyms */
    for (;;)
    {
        /* look up the property */
        retval = objgetap0(ctx, obj, prop, ornp, inh, &typ);
        
        /* 
         *   If we found something (i.e., retval != 0), check to see if we
         *   have a synonym; if so, synonym translation is required 
         */
        if (retval != 0 && typ == DAT_SYN)
        {
            prpnum  prvprop;
            objdef *objptr;
            prpdef *p;

            /* 
             *   Translation is required - get new property and try again.
             *   First, remember the original property, so we can make
             *   sure we're not going to loop (at least, not in this one
             *   synonym definition).  
             */
            prvprop = prop;

            objptr = (objdef *)mcmlck(ctx, (mcmon)*ornp);
            p = objofsp(objptr, retval);
            prop = osrp2(prpvalp(p));
            mcmunlck(ctx, (mcmon)*ornp);

            /* check for direct circularity */
            if (prop == prvprop)
                errsig(ctx->mcmcxgl->mcmcxerr, ERR_CIRCSYN);

            /* go back for another try with the new property */
            continue;
        }

        /* we don't have to perform a translation; return the result */
        return retval;
    }
}


/*
 *   Expand an object by a requested size, and return a pointer to the
 *   object's location.  The object will be unlocked and relocked by this
 *   call.  The new size is written to the *siz argument.
 */
objdef *objexp(mcmcxdef *ctx, objnum obj, ushort *siz)
{
    ushort  oldsiz;
    uchar  *p;
    
    oldsiz = mcmobjsiz(ctx, (mcmon)obj);
    p = mcmrealo(ctx, (mcmon)obj, (ushort)(oldsiz + *siz));
    *siz = mcmobjsiz(ctx, (mcmon)obj) - oldsiz;
    return((objdef *)p);
}

/*
 *   Delete a property in an object.  Note that we never actually remove
 *   anything marked as an original property, but just mark it 'ignore'.
 *   This way, it's easy to restore the entire original state of the
 *   objects, simply by deleting everything not marked original and
 *   clearing the 'ignore' flag on the remaining properties.  If
 *   'mark_only' is true, we'll only mark the property as deleted without
 *   actually reclaiming the space; this is necessary when deleting a
 *   method when other methods may follow, since p-code is not entirely
 *   self-relative and thus can't always be relocated within an object.  
 */
void objdelp(mcmcxdef *mctx, objnum objn, prpnum prop, int mark_only)
{
    objdef *objptr;
    uint    pofs;
    prpdef *p;
    prpdef *nxt;
    size_t  movsiz;
    
    pofs = objgetp(mctx, objn, prop, (dattyp *)0);  /* try to find property */
    if (!pofs) return;                   /* not defined - nothing to delete */
    
    objptr = (objdef *)mcmlck(mctx, objn);            /* get lock on object */
    p = objofsp(objptr, pofs);                 /* get actual prpdef pointer */
    nxt = objpnxt(p);                    /* find next prpdef after this one */
    
    /* if this is original, just mark 'ignore' */
    if (prpflg(p) & PRPFORG)
    {
        prpflg(p) |= PRPFIGN;                    /* mark this as overridden */
    }
    else if (mark_only)
    {
        prpflg(p) |= PRPFDEL;     /* mark as deleted without removing space */
    }
    else
    {
        /* move prpdef's after current one down over current one */
        movsiz = (uchar *)objptr + objfree(objptr) - (uchar *)nxt;
        memmove(p, nxt, movsiz);

        objsnp(objptr, objnprop(objptr)-1);
        objsfree(objptr, objfree(objptr) - (((uchar *)nxt) - ((uchar *)p)));
    }
    
    /* tell cache manager this object has been changed, and unlock it */
    mcmtch(mctx, objn);
    mcmunlck(mctx, objn);
}

/*
 *   Set a property of an object to a new value, overwriting the original
 *   value (if any); the object must be unlocked coming in.  If an undo
 *   context is provided, an undo record is written; if the undo context
 *   pointer is null, no undo information is kept.  
 */
void objsetp(mcmcxdef *ctx, objnum objn, prpnum prop, dattyp typ,
             const void *val, objucxdef *undoctx)
{
    objdef *objptr;
    prpdef *p;
    uint    pofs;
    uint    siz;
    ushort  newsiz;
    int     indexed;
    int     prop_was_set;
    
    /* get a lock on the object */
    objptr = (objdef *)mcmlck(ctx, objn);
    indexed = objflg(objptr) & OBJFINDEX;

    /* catch any errors so we can unlock the object */
    ERRBEGIN(ctx->mcmcxgl->mcmcxerr)
    {
        /* get the previous value of the property, if any */
        pofs = objgetp(ctx, objn, prop, (dattyp *)0);
        p = objofsp(objptr, pofs);
        prop_was_set = (p != 0);

        /* start the undo record if we are keeping undo information */
        if (undoctx && objuok(undoctx))
        {
            uchar  *up;
            uchar   cmd;

            if (p)
            {
                if (prpflg(p) & PRPFORG)
                {
                    cmd = OBJUOVR;                     /* override original */
                    p = (prpdef *)0;       /* pretend it doesn't even exist */
                }
                else cmd = OBJUCHG;                      /* change property */
            }
            else cmd = OBJUADD;            /* prop didn't exist - adding it */

            /* write header, reserve space, and get a pointer to the space */
            up = objures(undoctx, cmd,
                         (ushort)(sizeof(mcmon) + sizeof(prpnum)
                                  + (p ? PRPHDRSIZ + prpsize(p) : 0)));

            /* write the object and property numbers */
            memcpy(up, &objn, (size_t)sizeof(objn));
            up += sizeof(mcmon);
            memcpy(up, &prop, (size_t)sizeof(prop));
            up += sizeof(prop);

            /* if there's existing data, write it */
            if (p)
            {
                memcpy(up, p, (size_t)(PRPHDRSIZ + prpsize(p)));
                up += PRPHDRSIZ + prpsize(p);
            }

            /* update the undo context's head offset for the new value */
            undoctx->objucxhead = up - undoctx->objucxbuf;
        }

        /* get the size of the data */
        siz = datsiz(typ, val);

        /*
         *   If the property is already set, and the new data fits, use the
         *   existing slot.  However, do not use existing slot if it's
         *   in the non-mutable portion of the object.
         */
        if (!p || (uint)prpsize(p) < siz || pofs < (uint)objrst(objptr))
        {
            uint   avail;

            /* delete any existing value */
            if (prop_was_set)
                objdelp(ctx, objn, prop, FALSE);

            /* get the top of the property area */
            p = objpfre(objptr);        

            /* make sure there's room at the top */
            avail = mcmobjsiz(ctx, (mcmon)objn) - objfree(objptr);
            if (avail < siz + PRPHDRSIZ)
            {
                newsiz = 64 + ((objfree(objptr) + siz + PRPHDRSIZ) -
                               mcmobjsiz(ctx, (mcmon)objn));
                objptr = objexp(ctx, objn, &newsiz);
                p = objpfre(objptr);       /* reset pointer if object moved */
                /* NOTE! Index (if present) is now invalid! */
            }

            prpsetsize(p, siz);                /* set the new property size */
            prpsetprop(p, prop);                     /* ... and property id */
            prpflg(p) = 0;                         /* no property flags yet */
            objsnp(objptr, objnprop(objptr) + 1);          /* one more prop */
            objsfree(objptr, objfree(objptr) + siz + PRPHDRSIZ);
        }

        /* copy the new data to top of object's free space */
        prptype(p) = typ;
        if (siz != 0) memcpy(prpvalp(p), val, (size_t)siz);
    }
    ERRCLEAN(ctx->mcmcxgl->mcmcxerr)
    {
        mcmunlck(ctx, objn);                           /* unlock the object */
    }
    ERRENDCLN(ctx->mcmcxgl->mcmcxerr)
        
    /* dirty the object, and release lock on object before return */
    mcmtch(ctx, objn);                        /* mark the object as changed */
    mcmunlck(ctx, objn);                                       /* unlock it */

    /* if necessary, rebuild the property index */
    if (indexed) objindx(ctx, objn);
}

/* set an undo savepoint */
void objusav(objucxdef *undoctx)
{
    /* the only thing in this record is the OBJUSAV header */
    objures(undoctx, OBJUSAV, (ushort)0);
}

/* reserve space in an undo buffer, and write header */
uchar *objures(objucxdef *undoctx, uchar cmd, ushort siz)
{
    ushort  prv;
    uchar  *p;
    
    /* adjust size to include header information */
    siz += 1 + sizeof(ushort);
    
    /* make sure there's enough room overall for the record */
    if (siz > undoctx->objucxsiz) errsig(undoctx->objucxerr, ERR_UNDOVF);
    
    /* if there's no information, reset buffers */
    if (undoctx->objucxhead == undoctx->objucxprv)
    {
        undoctx->objucxhead = undoctx->objucxprv = undoctx->objucxtail = 0;
        undoctx->objucxtop = 0;
        goto done;
    }
    
    /* if tail is below head, we can use to top of entire buffer */
    if (undoctx->objucxtail < undoctx->objucxhead)
    {
        /* if there's enough space left after head, we're done */
        if (undoctx->objucxsiz - undoctx->objucxhead >= siz)
            goto done;
        
        /* insufficient space:  wrap head down to bottom of buffer */
        undoctx->objucxtop = undoctx->objucxprv;            /* last was top */
        undoctx->objucxhead = 0;
    }
    
    /* head is below tail:  delete records until we have enough room */
    while (undoctx->objucxtail - undoctx->objucxhead < siz)
    {
        objutadv(undoctx);
        
        /* if the tail wrapped, advancing won't do any more good */
        if (undoctx->objucxtail <= undoctx->objucxhead)
        {
            /* if there's enough room at the top, we're done */
            if (undoctx->objucxsiz - undoctx->objucxhead >= siz)
                goto done;
            
            /* still not enough room; wrap the head this time */
            undoctx->objucxtop = undoctx->objucxprv;        /* last was top */
            undoctx->objucxhead = 0;
        }
    }
    
done:
    /* save back-link, and set objucxprv pointer to the new record */
    prv = undoctx->objucxprv;
    undoctx->objucxprv = undoctx->objucxhead;
    
    /* write the header:  command byte, back-link to previous record */
    p = &undoctx->objucxbuf[undoctx->objucxhead];
    *p++ = cmd;
    memcpy(p, &prv, sizeof(prv));
    
    /* advance the head pointer past the header */
    undoctx->objucxhead += 1 + sizeof(prv);
    
    /* set the high-water mark if we've exceeded the old one */
    if (undoctx->objucxprv > undoctx->objucxtop)
        undoctx->objucxtop = undoctx->objucxprv;

    /* return the reserved space */
    return &undoctx->objucxbuf[undoctx->objucxhead];
}

/* advance the undo tail pointer over the record it points to */
void objutadv(objucxdef *undoctx)
{
    uchar  *p;
    ushort  siz;
    uchar   pr[PRPHDRSIZ];                   /* space for a property header */
    uchar   cmd;
    
    /* if we're at the most recently written record, flush buffer */
    if (undoctx->objucxtail == undoctx->objucxprv)
    {
        undoctx->objucxtail = 0;
        undoctx->objucxprv = 0;
        undoctx->objucxhead = 0;
        undoctx->objucxtop = 0;
    }

    /* if we've reached high water mark, wrap back to bottom */
    if (undoctx->objucxtail == undoctx->objucxtop)
    {
        undoctx->objucxtail = 0;
        return;
    }
    
    /* determine size by inspecting current record */
    p = undoctx->objucxbuf + undoctx->objucxtail;
    siz = 1 + sizeof(ushort);                          /* basic header size */
    
    cmd = *p++;
    p += sizeof(ushort);                       /* skip the previous pointer */
    
    switch(cmd)
    {
    case OBJUCHG:
        /* change:  property header (added below) plus data value */
        memcpy(pr, p + sizeof(mcmon) + sizeof(prpnum), (size_t)PRPHDRSIZ);
        siz += PRPHDRSIZ + prpsize(pr);
        /* FALLTHROUGH */

    case OBJUADD:
    case OBJUOVR:
        /* add/override:  property header only */
        siz += sizeof(mcmon) + sizeof(prpnum);
        break;
        
    case OBJUCLI:
        siz += (*undoctx->objucxcsz)(undoctx->objucxccx, p);
        break;

    case OBJUSAV:
        break;
    }
    
    undoctx->objucxtail += siz;
}

/* undo one undo record, and remove it from the undo list */
void obj1undo(mcmcxdef *mctx, objucxdef *undoctx)
{
    uchar  *p;
	prpnum  prop = 0;
	objnum  objn = 0;
    uchar   cmd;
    uchar   pr[PRPHDRSIZ];                     /* space for property header */
    ushort  prv;
    ushort  pofs;
    objdef *objptr;
	int     indexed = 0;

    /* if there's no more undo, signal an error */
    if (undoctx->objucxprv == undoctx->objucxhead)
        errsig(undoctx->objucxerr, ERR_NOUNDO);
    
    /* move back to previous record */
    undoctx->objucxhead = undoctx->objucxprv;
    p = &undoctx->objucxbuf[undoctx->objucxprv];
    
    /* get command, and set undocxprv to previous record */
    cmd = *p++;
    memcpy(&prv, p, sizeof(prv));
    p += sizeof(prv);
    
    /* if we're at the tail, no more undo; otherwise, use back link */
    if (undoctx->objucxprv == undoctx->objucxtail)
        undoctx->objucxprv = undoctx->objucxhead;
    else
        undoctx->objucxprv = prv;
    
    if (cmd == OBJUSAV) return;       /* savepointer marker - nothing to do */
    
    /* get object/property information for property-changing undo */
    if (cmd != OBJUCLI)
    {
        memcpy(&objn, p, (size_t)sizeof(objn));
        p += sizeof(objn);
        memcpy(&prop, p, (size_t)sizeof(prop));
        p += sizeof(prop);
        objptr = mcmlck(mctx, objn);
        indexed = (objflg(objptr) & OBJFINDEX);
        mcmunlck(mctx, objn);
    }
    
    switch(cmd)
    {
    case OBJUADD:
        objdelp(mctx, objn, prop, FALSE);
        if (indexed) objindx(mctx, objn);
        break;
        
    case OBJUOVR:
        objdelp(mctx, objn, prop, FALSE);  /* delete the non-original value */
        pofs = objgetp(mctx, objn, prop, (dattyp *)0);  /* get ignored prop */
        objptr = (objdef *)mcmlck(mctx, objn);           /* lock the object */
        prpflg(objofsp(objptr, pofs)) &= ~PRPFIGN;     /* no longer ignored */
        mcmunlck(mctx, objn);                          /* unlock the object */
        break;
        
    case OBJUCHG:
        memcpy(pr, p, (size_t)PRPHDRSIZ);
        p += PRPHDRSIZ;
        objsetp(mctx, objn, prop, prptype(pr), (void *)p, (objucxdef *)0);
        break;
        
    case OBJUCLI:
        (*undoctx->objucxcun)(undoctx->objucxccx, p);
        break;
    }
}

/*
 *   Determine if it's ok to add undo records - returns TRUE if a
 *   savepoint has been stored in the undo log, FALSE if not. 
 */
int objuok(objucxdef *undoctx)
{
    ushort prv;

    /* see if there's any more undo information */
    if (undoctx->objucxprv == undoctx->objucxhead)
        return(FALSE);

    /* look for most recent savepoint marker */
    for (prv = undoctx->objucxprv ;; )
    {
        if (undoctx->objucxbuf[prv] == OBJUSAV)
            return(TRUE);               /* found a savepoint - can add undo */

        /* if we've reached the tail, there are no more undo records */
        if (prv == undoctx->objucxtail)
            return(FALSE);                /* no savepoints - can't add undo */

        /* get previous record */
        memcpy(&prv, &undoctx->objucxbuf[prv+1], sizeof(prv));
    }
}

/*
 *   Undo back to the most recent savepoint.  If there is no savepoint in
 *   the undo list, NOTHING will be undone.  This prevents reaching an
 *   inconsistent state in which some, but not all, of the operations
 *   between two savepoints are undone: either all operations between two
 *   savepoints will be undone, or none will. 
 */
void objundo(mcmcxdef *mctx, objucxdef *undoctx)
{
    ushort prv;
    ushort sav;

    /* see if there's any more undo information */
    if (undoctx->objucxprv == undoctx->objucxhead)
        errsig(undoctx->objucxerr, ERR_NOUNDO);

    /* look for most recent savepoint marker */
    for (prv = undoctx->objucxprv ;; )
    {
        if (undoctx->objucxbuf[prv] == OBJUSAV)
        {
            sav = prv;
            break;
        }
        
        /* if we've reached the tail, there are no more undo records */
        if (prv == undoctx->objucxtail)
            errsig(undoctx->objucxerr, ERR_ICUNDO);

        /* get previous record */
        memcpy(&prv, &undoctx->objucxbuf[prv+1], sizeof(prv));
    }
    
    /* now undo everything until we get to the savepoint */
    do { obj1undo(mctx, undoctx); } while (undoctx->objucxhead != sav);
}

/* initialize undo context */
objucxdef *objuini(mcmcxdef *ctx, ushort siz,
                   void (*undocb)(void *, uchar *), 
                   ushort (*sizecb)(void *, uchar *),
                   void *callctx)
{
    objucxdef *ret;
    long       totsiz;

    /* force size into valid range */
    totsiz = (long)siz + sizeof(objucxdef) - 1;
    if (totsiz > 0xff00)
        siz = 0xff00 - sizeof(objucxdef) + 1;

    ret = (objucxdef *)mchalo(ctx->mcmcxgl->mcmcxerr,
                              (sizeof(objucxdef) + siz - 1),
                              "objuini");
    
    ret->objucxmem  = ctx;
    ret->objucxerr  = ctx->mcmcxgl->mcmcxerr;
    ret->objucxsiz  = siz;
    ret->objucxhead = ret->objucxprv = ret->objucxtail = ret->objucxtop = 0;
    
    /* set client callback functions */
    ret->objucxcun = undocb;               /* callback to apply client undo */
    ret->objucxcsz = sizecb;         /* callback to get size of client undo */
    ret->objucxccx = callctx;      /* context for client callback functions */
    
    return(ret);
}

/* discard all undo records */
void objulose(objucxdef *ctx)
{
    if (ctx)
        ctx->objucxhead =
        ctx->objucxprv  =
        ctx->objucxtail =
        ctx->objucxtop  = 0;
}

/* uninitialize the undo context - release allocated memory */
void objuterm(objucxdef *uctx)
{
    /* free the undo memory block */
    mchfre(uctx);
}

/* revert object to original (post-compilation) values */
void objrevert(void *ctx0, mcmon objn)
{
    mcmcxdef *mctx = (mcmcxdef *)ctx0;
    uchar    *p;
    prpdef   *pr;
    int       cnt;
    int       indexed;
    
    p = mcmlck(mctx, objn);
    pr = objprp(p);
    indexed = objflg(p) & OBJFINDEX;
    
    /* restore original settings */
    objsfree(p, objrst(p));
    objsnp(p, objstat(p));

    /* go through original properties and remove 'ignore' flag if set */
    for (cnt = objnprop(p) ; cnt ; pr = objpnxt(pr), --cnt)
        prpflg(pr) &= ~PRPFIGN;
    
    /* touch object and unlock it */
    mcmtch(mctx, objn);
    mcmunlck(mctx, objn);
    
    /* if it's indexed, rebuild the index */
    if (indexed) objindx(mctx, objn);
}

/* set 'ignore' flag for original properties set in mutable part */
void objsetign(mcmcxdef *mctx, objnum objn)
{
    objdef *objptr;
    prpdef *mut;
    prpdef *p;
    int     statcnt;
    int     cnt;
    int     indexed;
    prpdef *p1;
    
    objptr = (objdef *)mcmlck(mctx, (mcmon)objn);
    p1 = objprp(objptr);
    indexed = objflg(objptr) & OBJFINDEX;

    /* go through mutables, and set ignore on duplicates in non-mutables */
    for (mut = (prpdef *)(objptr + objrst(objptr)),
         cnt = objnprop(objptr) - objstat(objptr) ; cnt ;
         mut = objpnxt(mut), --cnt)
    {
        for (p = p1, statcnt = objstat(objptr) ; statcnt ;
             p = objpnxt(p), --statcnt)
        {
            /* if this static prop matches a mutable prop, ignore it */
            if (prpprop(p) == prpprop(mut))
            {
                prpflg(p) |= PRPFIGN;
                break;
            }
        }
    }
    
    mcmtch(mctx, (mcmon)objn);
    mcmunlck(mctx, (mcmon)objn);
    if (indexed) objindx(mctx, objn);
}

/*
 *   Build or rebuild a property index for an object.  
 */
void objindx(mcmcxdef *mctx, objnum objn)
{
    uint    newsiz;
    uint    avail;
    objdef *objptr;
    uint    cnt;
    prpdef *p;
	uchar  *indp = nullptr;
    uchar  *indbase;
    uint    icnt;
    uint    first;
    uint    last;
	uint    cur = 0;
    
    objptr = (objdef *)mcmlck(mctx, objn);            /* get object pointer */
    cnt = objnprop(objptr);                     /* get number of properties */
    p = objprp(objptr);         /* get pointer to properties (or old index) */
    newsiz = 2 + 4*cnt;                 /* figure size needed for the index */
    
    avail = mcmobjsiz(mctx, objn) - objfree(objptr);
    
    /* insert space for the index; expand the object if necessary */
    if (avail < newsiz)
    {
        ushort  need;
        
        newsiz += 10*4;                   /* add some extra space for later */
        need = newsiz - avail;            /* compute amount of space needed */
        objptr = objexp(mctx, objn, &need);
        p = objprp(objptr);
    }
    
    /* now build the index */
    indbase = objpfre(objptr);
    for (icnt = 0 ; cnt ; p = objpnxt(p), --cnt, ++icnt)
    {
        uint ofs = (uchar *)p - (uchar *)objptr;
        
        if (icnt)
        {
            /* figure out where to insert this property */
            first = 0;
            last = icnt - 1;
            for (;;)
            {
                if (first > last) break;
                cur = first + (last - first)/2;
                indp = indbase + cur*4;
                if (indp[0] == p[0] && indp[1] == p[1])
                    break;
                else if (indp[0] < p[0]
                         || (indp[0] == p[0] && indp[1] < p[1]))
                    first = (cur == first ? first + 1 : cur);
                else
                    last = (cur == last ? last - 1 : cur);
            }
            
            /* make sure we're positioned just before insertion point */
            while (cur < icnt
                   && (indp[0] <= p[0]
                       || (indp[0] == p[0] && indp[1] <= p[1])))
            {
                indp += 4;
                ++cur;
            }
                
            /* move elements above if any */
            if (cur < icnt)
                memmove(indp + 4, indp, (size_t)((icnt - cur) * 4));
        }
        else
            indp = indbase;
        
        /* insert property into index */
        indp[0] = p[0];
        indp[1] = p[1];
        oswp2(indp+2, ofs);
    }
    
    /* set the index flag, and dirty and free the object */
    objsflg(objptr, objflg(objptr) | OBJFINDEX);
    mcmtch(mctx, (mcmon)objn);
    mcmunlck(mctx, (mcmon)objn);
}

/* allocate and initialize an object */
objdef *objnew(mcmcxdef *mctx, int sccnt, ushort propspace,
               objnum *objnptr, int classflg)
{
    objdef *o;
    mcmon   objn;

    /* allocate cache object */
    o = (objdef *)mcmalo(mctx, (ushort)(OBJDEFSIZ + sccnt * 2 + propspace),
                         &objn);
    
    /* set up object descriptor for the new object */
    objini(mctx, sccnt, (objnum)objn, classflg);

    *objnptr = (objnum)objn;
    return(o);
}

/* initialize an already allocated object */
void objini(mcmcxdef *mctx, int sccnt, objnum objn, int classflg)
{
    objdef *o;
    uint    flags = 0;

    /* get a lock on the object */
    o = (objdef *)mcmlck(mctx, objn);
    
    memset(o, 0, (size_t)10);
    objsnsc(o, sccnt);
    objsfree(o, ((uchar *)objsc(o) + 2*sccnt) - (uchar *)o);
    
    /* set up flags */
    if (classflg) flags |= OBJFCLASS;
    objsflg(o, flags);

    /* tell cache manager that this object has been modified */
    mcmtch(mctx, objn);
    mcmunlck(mctx, objn);
}

/*
 *   Get the first superclass of an object.  If it doesn't have any
 *   superclasses, return invalid.
 */
objnum objget1sc(mcmcxdef *ctx, objnum objn)
{
    objdef *p;
    objnum  retval;

    /* lock the object */
    p = mcmlck(ctx, (mcmon)objn);

    /* get the first superclass if it has any */
    if (objnsc(p) == 0)
        retval = MCMONINV;
    else
        retval = osrp2(objsc(p));

    /* unlock the object and return the superclass value */
    mcmunlck(ctx, (mcmon)objn);
    return retval;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
