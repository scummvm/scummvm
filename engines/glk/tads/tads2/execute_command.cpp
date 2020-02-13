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
 * TADS Interpreter Execute user Command
 *
 * Function
 *   Executes a user command after it has been parsed
 * Notes
 *   TADS 2.0 version
 * 
 *   This module contains the implementation of the entire "turn" sequence,
 *   which is:
 * 
 *     preCommand(actor, verb, dobj-list, prep, iobj)
 *     verb.verbAction(actor, do, prep, io)
 *     actor.actorAction( verb, do, prep, io )
 *     actor.location.roomAction( actor, verb, do, prep, io )
 *     if ( io ) 
 *     {
 *       io.iobjCheck(actor, verb, dobj, prep)
 *       if (io does not define verIo<Verb> directly)
 *           io.iobjGen(actor, verb, dobj, prep)
 *       do.dobjCheck(actor, verb, iobj, prep)
 *       if (do does not define do<Verb> directly)
 *           do.dobjGen(actor, verb, iobj, prep)
 *       io.verIo<Verb>( actor, do )
 *       if ( noOutput )
 *       {
 *         do.verDo<Verb>( actor, io )
 *         if ( noOutput ) io.io<Verb>( actor, do )
 *       }
 *     }
 *     else if ( do )
 *     {
 *       do.dobjCheck(actor, verb, nil, nil)
 *       if (do does not define do<Verb> directly)
 *           do.dobjGen(actor, verb, nil, nil)
 *       do.verDo<Verb>( actor )
 *       if ( noOutput )do.do<Verb>( actor )
 *     }
 *     else
 *     {
 *       verb.action( actor )
 *     }
 *     postAction(actor, verb, dobj, prep, iobj, error_code)
 *     daemons
 *     fuses
 *     endCommand(actor, verb, dobj-list, prep, iobj, error_code)
 * 
 *   If an 'exit' or 'exitobj' is encountered, we skip straight to the
 *   daemons.  If an abort is encountered, we skip to endCommand.  If
 *   askio, or askdo is encountered, we skip everything remaining.  Under
 *   any of these exit scenarios, we return success to our caller.
 *   
 *   This module also contains code to set and remove fuses and daemons,
 *   since they are part of the player turn sequence.
 * Returns
 *   0 for success, other for failure.
 */

#include "glk/tads/tads2/built_in.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* allocate and initialize a fuse/daemon/notifier array */
void vocinialo(voccxdef *ctx, vocddef **what, int cnt) {
    vocddef *p;
    
    *what = (vocddef *)mchalo(ctx->voccxerr,
                              (cnt * sizeof(vocddef)), "vocinialo");

    /* set all object/function entries to MCMONINV to indicate not-in-use */
    for (p = *what ; cnt ; ++p, --cnt)
        p->vocdfn = MCMONINV;
}

/* internal service routine to clear one set of fuses/deamons/alerters */
static void vocdmn1clr(vocddef *dmn, uint cnt)
{
    for ( ; cnt ; --cnt, ++dmn) dmn->vocdfn = MCMONINV;
}

/* delete all fuses/daemons/alerters */
void vocdmnclr(voccxdef *ctx)
{
    vocdmn1clr(ctx->voccxfus, ctx->voccxfuc);
    vocdmn1clr(ctx->voccxdmn, ctx->voccxdmc);
    vocdmn1clr(ctx->voccxalm, ctx->voccxalc);
}

/* save undo information for a daemon/fuse/notifier */
static void vocdusav(voccxdef *ctx, vocddef *what)
{
    uchar     *p;
    objucxdef *uc = ctx->voccxundo;
    ushort     siz = sizeof(what) + sizeof(*what) + 1;
    
    /* if we don't need to save undo, quit now */
    if (uc == 0 || !objuok(uc))
        return;

    /* reserve space for our record */
    p = objures(uc, OBJUCLI, siz);
    
    /* set up our undo record */
    *p = VOC_UNDO_DAEMON;
    memcpy(p + 1, &what, (size_t)sizeof(what));
    memcpy(p + 1 + sizeof(what), what, (size_t)sizeof(*what));
    
    uc->objucxhead += siz;
}

/* apply undo information for a daemon/fuse/notifier */
void vocdundo(void *ctx0, uchar *data)
{
    voccxdef *ctx = (voccxdef *)ctx0;
    vocddef  *daemon;
    objnum    objn;
    ushort    siz;
    ushort    wrdsiz;
    uchar    *p;
    int       sccnt;
    objnum    sc;
    int       len1, len2;
    prpnum    prp;
    int       flags;
    uchar    *wrd;

    switch(*data)
    {
    case VOC_UNDO_DAEMON:
        memcpy(&daemon, data + 1, (size_t)sizeof(daemon));
        memcpy(daemon, data + 1 + sizeof(daemon), (size_t)sizeof(*daemon));
        break;

    case VOC_UNDO_NEWOBJ:
        /* get the object number */
        objn = osrp2(data + 1);

        /* delete the object's inheritance and vocabulary records */
        vocdel(ctx, objn);
        vocidel(ctx, objn);

        /* delete the object */
        mcmfre(ctx->voccxmem, (mcmon)objn);
        break;

    case VOC_UNDO_DELOBJ:
        /* get the object's number and size */
        objn = osrp2(data + 1);
        siz = osrp2(data + 3);
        wrdsiz = osrp2(data + 5);

        /* allocate the object with its original number */
        p = mcmalonum(ctx->voccxmem, siz, (mcmon)objn);

        /* copy the contents back to the object */
        memcpy(p, data + 7, (size_t)siz);

        /* get its superclass if it has one */
        sccnt = objnsc(p);
        if (sccnt) sc = osrp2(objsc(p));

        /* unlock the object, and create its inheritance records */
        mcmunlck(ctx->voccxmem, (mcmon)objn);
        vociadd(ctx, objn, MCMONINV, sccnt, &sc, VOCIFNEW | VOCIFVOC);

        /* restore the words as well */
        data += 7 + siz;
        while (wrdsiz)
        {
            /* get the lengths from the buffer */
            len1 = osrp2(data + 2);
            len2 = osrp2(data + 4);

            /* add the word */
            vocadd2(ctx, data[0], objn, data[1], data+6, len1,
                    data+6+len1, len2);
            
            /* remove this object from the word size */
            wrdsiz -= 6 + len1 + len2;
            data += 6 + len1 + len2;
        }
        break;

    case VOC_UNDO_ADDVOC:
    case VOC_UNDO_DELVOC:
        flags = *(data + 1);
        prp = *(data + 2);
        objn = osrp2(data + 3);
        wrd = data + 5;
        if (*data == VOC_UNDO_ADDVOC)
            vocdel1(ctx, objn, (char *)wrd, prp, FALSE, FALSE, FALSE);
        else
            vocadd(ctx, prp, objn, flags, (char *)wrd);
        break;

    case VOC_UNDO_SETME:
        ctx->voccxme = osrp2(data + 1);
        break;
    }
}

/* determine size of one of our client undo records */
ushort OS_LOADDS vocdusz(void *ctx0, uchar *data)
{
    VARUSED(ctx0);

    switch(*data)
    {
    case VOC_UNDO_DAEMON:
        /* it's the size of the structures, plus one for the header */
        return (ushort)((sizeof(vocddef *) + sizeof(vocddef)) + 1);

    case VOC_UNDO_NEWOBJ:
        /* 2 bytes for the objnum plus 1 for the header */
        return 2 + 1;

    case VOC_UNDO_DELOBJ:
        /*
         *   1 (header) + 2 (objnum) + 2 (size) + 2 (word size) + object
         *   data size + word size
         */
        return osrp2(data+3) + osrp2(data+5) + 1+2+2+2;

    case VOC_UNDO_ADDVOC:
    case VOC_UNDO_DELVOC:
        /* 1 (header) + 2 (objnum) + 1 (flags) + 1 (type) + word size */
        return osrp2(data + 5) + 5;

    default:
        return 0;
    }
}

/* save undo for object creation */
void vocdusave_newobj(voccxdef *ctx, objnum objn)
{
    objucxdef *uc = ctx->voccxundo;
    uchar     *p;

    p = objures(uc, OBJUCLI, 3);
    *p = VOC_UNDO_NEWOBJ;
    oswp2(p+1, objn);

    uc->objucxhead += 3;
}

/* save undo information for a change in the "Me" object */
void vocdusave_me(voccxdef *ctx, objnum old_me)
{
    uchar     *p;
    objucxdef *uc = ctx->voccxundo;

    /* if we don't need to save undo, there's nothing to do */
    if (uc == 0 || !objuok(uc))
        return;

    /* reserve space for our record */
    p = objures(uc, OBJUCLI, 3);
    *p = VOC_UNDO_SETME;
    oswp2(p+1, old_me);

    /* absorb the space */
    uc->objucxhead += 3;
}

/* callback context structure */
struct delobj_cb_ctx
{
    uchar *p;
};

/*
 *   Iteration callback to write vocabulary words for an object being
 *   deleted to an undo stream, so that they can be restored if the
 *   deletion is undone. 
 */
static void delobj_cb(void *ctx0, vocdef *voc, vocwdef *vocw)
{
    struct delobj_cb_ctx *ctx = (struct delobj_cb_ctx *)ctx0;
    uchar *p = ctx->p;
    
    /* write this object's header to the stream */
    p[0] = vocw->vocwtyp;
    p[1] = vocw->vocwflg;
    oswp2(p+2, voc->voclen);
    oswp2(p+4, voc->vocln2);

    /* write the words as well */
    memcpy(p+6, voc->voctxt, (size_t)(voc->voclen + voc->vocln2));

    /* advance the pointer */
    ctx->p += 6 + voc->voclen + voc->vocln2;
}

/* save undo for object deletion */
void vocdusave_delobj(voccxdef *ctx, objnum objn)
{
    objucxdef *uc = ctx->voccxundo;
    uchar     *p;
    uchar     *objp;
    uint       siz;
    int        wrdsiz;
    int        wrdcnt;
    struct delobj_cb_ctx fnctx;

    /* figure out how much we need to save */
    objp = mcmlck(ctx->voccxmem, (mcmon)objn);
    siz = objfree(objp);

    /* figure the word size */
    voc_count(ctx, objn, 0, &wrdcnt, &wrdsiz);

    /*
     *   we need to store an additional 6 bytes (2-length1, 2-length2,
     *   1-type, 1-flags) for each word 
     */
    wrdsiz += wrdcnt*6;

    /* set up the undo header */
    p = objures(uc, OBJUCLI, (ushort)(7 + siz + wrdsiz));
    *p = VOC_UNDO_DELOBJ;
    oswp2(p+1, objn);
    oswp2(p+3, siz);
    oswp2(p+5, wrdsiz);

    /* save the object's data */
    memcpy(p+7, objp, (size_t)siz);

    /* write the words */
    fnctx.p = p+7 + siz;
    voc_iterate(ctx, objn, delobj_cb, &fnctx);

    /* unlock the object and advance the undo pointer */
    mcmunlck(ctx->voccxmem, (mcmon)objn);
    uc->objucxhead += 7 + siz + wrdsiz;
}

/* save undo for word creation */
void vocdusave_addwrd(voccxdef *ctx, objnum objn, prpnum typ, int flags,
                      char *wrd)
{
    ushort     wrdsiz;
    uchar     *p;
    objucxdef *uc = ctx->voccxundo;

    /* figure out how much space we need, and reserve it */
    wrdsiz = osrp2(wrd);
    p = objures(uc, OBJUCLI, (ushort)(5 + wrdsiz));

    *p = VOC_UNDO_ADDVOC;
    *(p+1) = flags;
    *(p+2) = (uchar)typ;
    oswp2(p+3, objn);
    memcpy(p+5, wrd, (size_t)wrdsiz);

    uc->objucxhead += 5 + wrdsiz;
}

/* save undo for word deletion */
void vocdusave_delwrd(voccxdef *ctx, objnum objn, prpnum typ, int flags,
                      char *wrd)
{
    ushort     wrdsiz;
    uchar     *p;
    objucxdef *uc = ctx->voccxundo;

    /* figure out how much space we need, and reserve it */
    wrdsiz = osrp2(wrd);
    p = objures(uc, OBJUCLI, (ushort)(5 + wrdsiz));

    *p = VOC_UNDO_DELVOC;
    *(p+1) = flags;
    *(p+2) = (uchar)typ;
    oswp2(p+3, objn);
    memcpy(p+5, wrd, (size_t)wrdsiz);

    uc->objucxhead += 5 + wrdsiz;
}
                      


/* set a fuse/daemon/notifier */
void vocsetfd(voccxdef *ctx, vocddef *what, objnum func, prpnum prop,
              uint tm, runsdef *val, int err)
{
	int      slots = 0;
    
    if (what == ctx->voccxdmn)
        slots = ctx->voccxdmc;
    else if (what == ctx->voccxalm)
        slots = ctx->voccxalc;
    else if (what == ctx->voccxfus)
        slots = ctx->voccxfuc;
    else
        errsig(ctx->voccxerr, ERR_BADSETF);
    
    /* find a free slot, and set up our fuse/daemon */
    for ( ; slots ; ++what, --slots)
    {
        if (what->vocdfn == MCMONINV)
        {
            /* save an undo record for this slot before changing */
            vocdusav(ctx, what);
            
            /* record the information */
            what->vocdfn = func;
            if (val != 0)
                OSCPYSTRUCT(what->vocdarg, *val);
            what->vocdprp = prop;
            what->vocdtim = tm;

            /* 
             *   the fuse/notifier/daemon is set - no need to look further
             *   for an open slot 
             */
            return;
        }
    }

    /* we didn't find an open slot - signal the appropriate error */
    errsig(ctx->voccxerr, err);
}

/* remove a fuse/daemon/notifier */
void vocremfd(voccxdef *ctx, vocddef *what, objnum func, prpnum prop,
              runsdef *val, int err)
{
	int      slots = 0;
    
    if (what == ctx->voccxdmn) slots = ctx->voccxdmc;
    else if (what == ctx->voccxalm) slots = ctx->voccxalc;
    else if (what == ctx->voccxfus) slots = ctx->voccxfuc;
    else errsig(ctx->voccxerr, ERR_BADREMF);
    
    /* find the slot with this same fuse/daemon/notifier, and remove it */
    for ( ; slots ; ++what, --slots)
    {
        if (what->vocdfn == func
            && what->vocdprp == prop
            && (!val || (val->runstyp == what->vocdarg.runstyp
                         && !memcmp(&val->runsv, &what->vocdarg.runsv,
                                    (size_t)datsiz(val->runstyp,
                                                   &val->runsv)))))
        {
            /* save an undo record for this slot before changing */
            vocdusav(ctx, what);

            what->vocdfn = MCMONINV;
            return;
        }
    }

/*    errsig(ctx->voccxerr, err); <<<harmless - don't signal it>>> */
}

/*
 *   Count one or more turns - burn all fuses down by the given number of
 *   turns.  Execute any fuses that expire within the given interval, but
 *   not any that expire at the end of the last turn counted here.  (If
 *   incrementing by one turn only, no fuses will be executed.)  If the
 *   do_fuses flag is false, fuses are simply deleted if they burn down
 *   within the interval.  
 */
void vocturn(voccxdef *ctx, int turncnt, int do_fuses)
{
    vocddef *p;
    int      i;
    int      do_exe;

    while (turncnt--)
    {
        /* presume we won't find anything to execute */
        do_exe = FALSE;
        
        /* go through notifiers, looking for fuse-type notifiers */
        for (i = ctx->voccxalc, p = ctx->voccxalm ; i ; ++p, --i)
        {
            if (p->vocdfn != MCMONINV
                && p->vocdtim != VOCDTIM_EACH_TURN
                && p->vocdtim != 0)
            {
                /* save an undo record for this slot before changing */
                vocdusav(ctx, p);
                
                if (--(p->vocdtim) == 0)
                    do_exe = TRUE;
            }
        }
        
        /* now go through the fuses */
        for (i = ctx->voccxfuc, p = ctx->voccxfus ; i ; ++p, --i)
        {
            if (p->vocdfn != MCMONINV && p->vocdtim != 0)
            {
                /* save an undo record for this slot before changing */
                vocdusav(ctx, p);

                if (--(p->vocdtim) == 0)
                    do_exe = TRUE;
            }
        }

        /*
         *   if we'll be doing more, and anything burned down, run
         *   current fuses before going on to the next turn 
         */
        if ((!do_fuses || turncnt) && do_exe)
            exefuse(ctx, do_fuses);
    }
}

/*
 *   display a default error message for a verb/dobj/iobj combo.
 *   The message is "I don't know how to <verb.sdesc> <dobj.thedesc>" if
 *   the dobj is present, and "I don't know how to <verb.sdesc> anything
 *   <prep.sdesc> <iobj.thedesc>" if the iobj is present.  Such a message
 *   is displayed when the objects in the command don't handle the verb
 *   (i.e., don't have any methods for verification of the verb:  they
 *   lack verDo<verb> or verIo<verb>).
 */
static void exeperr(voccxdef *ctx, objnum verb, objnum dobj,
                    objnum prep, objnum iobj)
{
    if (ctx->voccxper2 != MCMONINV)
    {
        runpobj(ctx->voccxrun, iobj);
        runpobj(ctx->voccxrun, prep);
        runpobj(ctx->voccxrun, dobj);
        runpobj(ctx->voccxrun, verb);
        runfn(ctx->voccxrun, ctx->voccxper2, 4);
        return;
    }
    
    vocerr(ctx, VOCERR(110), "I don't know how to ");
    runppr(ctx->voccxrun, verb, PRP_SDESC, 0);
    
    if (dobj != MCMONINV)
    {
        vocerr(ctx, VOCERR(111), " ");
        runppr(ctx->voccxrun, dobj, PRP_THEDESC, 0);
    }
    else
    {
        vocerr(ctx, VOCERR(112), " anything ");
        if (prep != MCMONINV)
            runppr(ctx->voccxrun, prep, PRP_SDESC, 0);
        else
            vocerr(ctx, VOCERR(113), "to");
        vocerr(ctx, VOCERR(114), " ");
        runppr(ctx->voccxrun, iobj, PRP_THEDESC, 0);
    }
    vocerr(ctx, VOCERR(115), ".");
}


/*
 *   Execute daemons 
 */
void exedaem(voccxdef *ctx)
{
    runcxdef *rcx = ctx->voccxrun;
    vocddef  *daemon;
    int       i;
    runsdef   val;
    int       err;

    for (i = ctx->voccxdmc, daemon = ctx->voccxdmn ; i ; ++daemon, --i)
    {
        if (daemon->vocdfn != MCMONINV)
        {
            objnum thisd = daemon->vocdfn;
            
            ERRBEGIN(ctx->voccxerr)

            OSCPYSTRUCT(val, daemon->vocdarg);
            runpush(rcx, val.runstyp, &val);
            runfn(rcx, thisd, 1);
            
            ERRCATCH(ctx->voccxerr, err)
                if (err != ERR_RUNEXIT && err != ERR_RUNEXITOBJ)
                    errrse(ctx->voccxerr);
            ERREND(ctx->voccxerr)
        }
    }
    for (i = ctx->voccxalc, daemon = ctx->voccxalm ; i ; ++daemon, --i)
    {
        if (daemon->vocdfn != MCMONINV
            && daemon->vocdtim == VOCDTIM_EACH_TURN)
        {
            ERRBEGIN(ctx->voccxerr)

            runppr(rcx, daemon->vocdfn, daemon->vocdprp, 0);
            
            ERRCATCH(ctx->voccxerr, err)
                if (err != ERR_RUNEXIT && err != ERR_RUNEXITOBJ)
                    errrse(ctx->voccxerr);
            ERREND(ctx->voccxerr)
        }
    }
}

/*
 *   Execute any pending fuses.  Return TRUE if any fuses were executed,
 *   FALSE otherwise.  
 */
int exefuse(voccxdef *ctx, int do_run)
{
    runcxdef *rcx = ctx->voccxrun;
    vocddef  *daemon;
    int       i;
    int       found = FALSE;
    runsdef   val;
    int       err;

    /* first, execute any expired function-based fuses */
    for (i = ctx->voccxfuc, daemon = ctx->voccxfus ; i ; ++daemon, --i)
    {
        if (daemon->vocdfn != MCMONINV && daemon->vocdtim == 0)
        {
            objnum thisf = daemon->vocdfn;
         
            found = TRUE;
            ERRBEGIN(ctx->voccxerr)

            /* save an undo record for this slot before changing */
            vocdusav(ctx, daemon);

            /* remove the fuse prior to running  */
            daemon->vocdfn = MCMONINV;

            if (do_run)
            {
                OSCPYSTRUCT(val, daemon->vocdarg);
                runpush(rcx, val.runstyp, &val);
                runfn(rcx, thisf, 1);
            }
            
            ERRCATCH(ctx->voccxerr, err)
                if (err != ERR_RUNEXIT && err != ERR_RUNEXITOBJ)
                    errrse(ctx->voccxerr);
            ERREND(ctx->voccxerr)
        }
    }

    /* next, execute any expired method-based notifier fuses */
    for (i = ctx->voccxalc, daemon = ctx->voccxalm ; i ; ++daemon, --i)
    {
        if (daemon->vocdfn != MCMONINV && daemon->vocdtim == 0)
        {
            objnum thisa = daemon->vocdfn;

            found = TRUE;
            ERRBEGIN(ctx->voccxerr)

            /* save an undo record for this slot before changing */
            vocdusav(ctx, daemon);

            /* delete it prior to running it */
            daemon->vocdfn = MCMONINV;

            if (do_run)
                runppr(rcx, thisa, daemon->vocdprp, 0);
            
            ERRCATCH(ctx->voccxerr, err)
                if (err != ERR_RUNEXIT && err != ERR_RUNEXITOBJ)
                    errrse(ctx->voccxerr);
            ERREND(ctx->voccxerr)
        }
    }

    /* return true if we found any expired fuses */
    return found;
}

/* ------------------------------------------------------------------------ */
/*
 *   Find the action routine template for a verb.  Fills in *tplofs with
 *   the offset of the template property within the verb object, and fills
 *   in actofs with the offset of the "action" property within the verb
 *   object.  Sets *tplofs to zero if there's no template, and sets
 *   *actofs to zero if there's no action routine.
 */
static void exe_get_tpl(voccxdef *ctx, objnum verb,
                        uint *tplofs, uint *actofs)
{
    /* look up the new-style template first */
    *tplofs = objgetap(ctx->voccxmem, verb, PRP_TPL2, (objnum *)0, FALSE);

    /* if there's no new-style template, look up the old-style template */
    if (*tplofs == 0)
        *tplofs = objgetap(ctx->voccxmem, verb, PRP_TPL, (objnum *)0, FALSE);

    /* also look to see if the verb has an Action method */
    *actofs = objgetap(ctx->voccxmem, verb, PRP_ACTION, (objnum *)0, FALSE);
}


/* ------------------------------------------------------------------------ */
/*
 *   Execute fuses and daemons.  Returns zero on success, or ERR_ABORT if
 *   'abort' was thrown during execution.  
 */
int exe_fuses_and_daemons(voccxdef *ctx, int err, int do_fuses,
                          objnum actor, objnum verb,
                          vocoldef *dobj_list, int dobj_cnt,
                          objnum prep, objnum iobj)
{
    int err2;

    /* presume no error */
    err2 = 0;
    
    /* execute fuses and daemons if desired - trap any errors that occur */
    if (do_fuses)
    {
        ERRBEGIN(ctx->voccxerr)
        {
            /* execute daemons */
            exedaem(ctx);
            
            /* execute fuses */
            (void)exefuse(ctx, TRUE);
        }
        ERRCATCH(ctx->voccxerr, err2)
        {
            /* 
             *   if 'abort' was invoked, ignore it, since it's now had the
             *   desired effect of skipping any remaining fuses and
             *   daemons; resignal any other error 
             */
            if (err2 != ERR_RUNABRT)
                errrse(ctx->voccxerr);
            
            /* replace any previous error with the new error code */
            err = err2;
        }
        ERREND(ctx->voccxerr);
    }

    /* execute endCommand if it's defined */
    if (ctx->voccxendcmd != MCMONINV)
    {
        /* push the arguments */
        runpnum(ctx->voccxrun, err);
        runpobj(ctx->voccxrun, iobj);
        runpobj(ctx->voccxrun, prep);
        voc_push_vocoldef_list(ctx, dobj_list, dobj_cnt);
        runpobj(ctx->voccxrun, verb);
        runpobj(ctx->voccxrun, actor);

        /* call endCommand */
        runfn(ctx->voccxrun, ctx->voccxendcmd, 6);
    }

    /* return the error status */
    return err;
}

/* ------------------------------------------------------------------------ */
/* 
 *   execute iobjGen/dobjGen methods, if appropriate 
 */
static int exegen(voccxdef *ctx, objnum obj, prpnum genprop,
                  prpnum verprop, prpnum actprop)
{
    int     hasgen;                                 /* has xobjGen property */
    objnum  genobj;                         /* object with xobjGen property */
    int     hasver;                               /* has verXoVerb property */
    objnum  verobj;                       /* object with verXoVerb property */
    int     hasact;                                  /* has xoVerb property */
    objnum  actobj;                          /* object with xoVerb property */
    
    /* ignore it if there's no object here */
    if (obj == MCMONINV) return(FALSE);

    /* look up the xobjGen property, and ignore if not present */
    hasgen = objgetap(ctx->voccxmem, obj, genprop, &genobj, FALSE);
    if (!hasgen) return(FALSE);

    /* look up the verXoVerb and xoVerb properties */
    hasver = objgetap(ctx->voccxmem, obj, verprop, &verobj, FALSE);
    hasact = objgetap(ctx->voccxmem, obj, actprop, &actobj, FALSE);

    /* ignore if verXoVerb or xoVerb "overrides" xobjGen */
    if ((hasver && !bifinh(ctx, vocinh(ctx, genobj), verobj))
        || (hasact && !bifinh(ctx, vocinh(ctx, genobj), actobj)))
        return FALSE;
    
    /* all conditions are met - execute dobjGen */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   Save "again" information for a direct or indirect object 
 */
static void exe_save_again_obj(vocoldef *againv, const vocoldef *objv,
                               char **bufp)
{
    /* if there's an object, save it */
    if (objv != 0)
    {
        /* copy the object information structure */
        memcpy(againv, objv, sizeof(*againv));

        /* copy the original command words to the "again" buffer */
        if (objv->vocolfst != 0 && objv->vocollst != 0)
        {
            size_t copylen;
            
            /* 
             *   Compute the length of the entire list.  The words are
             *   arranged consecutively in the buffer, separated by null
             *   bytes, so we must copy everything from the first word to
             *   the start of the last word, plus the length of the last
             *   word, plus the last word's trailing null byte.  
             */
            copylen = objv->vocollst - objv->vocolfst
                      + strlen(objv->vocollst) + 1;

            /* copy the text */
            memcpy(*bufp, objv->vocolfst, copylen);

            /* 
             *   set the new structure to point into the copy, not the
             *   original 
             */
            againv->vocolfst = *bufp;
            againv->vocollst = *bufp + (objv->vocollst - objv->vocolfst);

            /* skip past the space we've consumed in the buffer */
            *bufp += copylen;
        }
    }
    else
    {
        /* there's nothing to save - just set the object ID to invalid */
        againv->vocolobj = MCMONINV;
    }
}

/*
 *   Restore an "again" object previously saved.  Note that we must copy
 *   the saved data to our 2-element arrays so that we can insert a
 *   terminating element after each restored element - other code
 *   occasionally expects these structures to be stored in the standard
 *   object list array format.  Returns a pointer to the restored object
 *   list, which is the same as the first argument.  
 */
static vocoldef *exe_restore_again_obj(vocoldef again_array[2],
                                       const vocoldef *saved_obj)
{
    /* copy the saved object into the first array element */
    memcpy(&again_array[0], saved_obj, sizeof(again_array[0]));

    /* clear the second element to indicate the end of the object list */
    again_array[1].vocolobj = MCMONINV;
    again_array[1].vocolflg = 0;

    /* return a pointer to the first array element */
    return &again_array[0];
}

/* ------------------------------------------------------------------------ */
/* 
 *   Execute a single command.  'recursive' indicates whether the routine
 *   is being called for normal command processing or as a recursive call
 *   from within the game; if this flag is true, we'll bypass certain
 *   operations that are only appropriate for normal direct player
 *   commands: we won't remember the command for "again" processing, we
 *   won't do end-of-turn processing, and we won't reset the system stack
 *   before each function invocation.  
 */
static int exe1cmd(voccxdef *ctx, objnum actor, objnum verb, vocoldef *dobjv,
                   objnum *prepptr, vocoldef *iobjv, int endturn, uchar *tpl,
                   int newstyle, int recursive,
                   int validate_dobj, int validate_iobj,
                   vocoldef *dobj_list, int cur_dobj_idx, int dobj_cnt,
                   int show_multi_prefix, int multi_flags)
{
    objnum    loc;
    int       err;
    runcxdef *rcx = ctx->voccxrun;
    objnum    prep = *prepptr;
    objnum    dobj = (dobjv != 0 ? dobjv->vocolobj : MCMONINV);
    objnum    iobj = (iobjv != 0 ? iobjv->vocolobj : MCMONINV);
    int       tplflags;
    int       dobj_first;
    objnum    old_tio_actor;
    vocoldef *old_ctx_dobj;
    vocoldef *old_ctx_iobj;
    objnum    old_verb;
    objnum    old_actor;
    objnum    old_prep;
    int       do_fuses;
    int       do_postact;
    vocoldef  again_dobj[2];
    vocoldef  again_iobj[2];

    /* presume no error will occur */
    err = 0;

    /* 
     *   Presume we'll run fuses and daemons if this is the end of the
     *   turn.  We only do fuses and daemons once per command, even if the
     *   command contains multiple objects; 'endturn' will be true only
     *   when this is the last object of the command.  
     */
    do_fuses = endturn;

    /* presume we will call postAction */
    do_postact = TRUE;

    /* remember the original tio-level actor setting */
    old_tio_actor = tiogetactor(ctx->voccxtio);

    /* remember the original command settings (in case this is recursive) */
    old_actor = ctx->voccxactor;
    old_verb = ctx->voccxverb;
    old_prep = ctx->voccxprep;
    old_ctx_dobj = ctx->voccxdobj;
    old_ctx_iobj = ctx->voccxiobj;

    /* the default actor is Me */
    if (actor == MCMONINV)
        actor = ctx->voccxme;

    /* if command is "again", get information from previous command */
    if (verb == ctx->voccxvag)
    {
        /* it's "again" - repeat the last command */
        actor = ctx->voccxlsa;
        verb  = ctx->voccxlsv;
        dobj  = ctx->voccxlsd.vocolobj;
        iobj  = ctx->voccxlsi.vocolobj;
        prep  = ctx->voccxlsp;
        tpl   = ctx->voccxlst;
        newstyle = ctx->voccxlssty;

        /* 
         *   If we have a direct or indirect object, restore the full
         *   object information structure pointers (in particular, this
         *   restores the word lists).
         */
        if (dobj != MCMONINV)
            dobjv = exe_restore_again_obj(again_dobj, &ctx->voccxlsd);
        if (iobj != MCMONINV)
            iobjv = exe_restore_again_obj(again_iobj, &ctx->voccxlsi);

        /*
         *   make sure the command is repeatable: there must have been a
         *   verb, and the objects specified must still be accessible 
         */
        if (verb == MCMONINV)
        {
            /* 
             *   if the last command was lost due to an object deletion,
             *   show the message "you can't repeat that command";
             *   otherwise, show the message "there's no command to
             *   repeat" 
             */
            if ((ctx->voccxflg & VOCCXAGAINDEL) != 0)
                vocerr(ctx, VOCERR(27), "You can't repeat that command.");
            else
                vocerr(ctx, VOCERR(26), "There's no command to repeat.");

            /* flush the output and return failure */
            tioflush(ctx->voccxtio);
            return 0;
        }
        else if ((dobj != MCMONINV &&
                  !vocchkaccess(ctx, dobj, PRP_VALIDDO, 0, actor, verb))
                 || (iobj != MCMONINV &&
                     !vocchkaccess(ctx, iobj, PRP_VALIDIO, 0, actor, verb))
                 || !vocchkaccess(ctx, actor, PRP_VALIDACTOR, 0, actor, verb))
        {
            vocerr(ctx, VOCERR(27), "You can't repeat that command.");
            tioflush(ctx->voccxtio);
            return(0);
        }
    }
    else
    {
        /* verify the direct object if present */
        if (validate_dobj
            && dobj != MCMONINV
            && !vocchkaccess(ctx, dobj, PRP_VALIDDO, 0, actor, verb))
        {
            /* generate an appropriate message */
            if (vocchkvis(ctx, dobj, actor))
            {
                /* it's visible but not accessible */
                vocnoreach(ctx, &dobj, 1, actor, verb, prep,
                           PRP_DODEFAULT, FALSE, 0, 0, 1);
            }
            else
            {
                /* it's not even visible */
                if (recursive)
                    vocerr(ctx, VOCERR(39), "You don't see that here.");
                else
                    vocerr(ctx, VOCERR(38),
                           "You don't see that here any more.");
            }
            
            /* indicate the error */
            return ERR_PRS_VAL_DO_FAIL;
        }
        
        /* verify the indirect object if present */
        if (validate_iobj
            && iobj != MCMONINV
            && !vocchkaccess(ctx, iobj, PRP_VALIDIO, 0, actor, verb))
        {
            /* generate the error message */
            if (vocchkvis(ctx, iobj, actor))
            {
                /* it's visible but not accessible */
                vocnoreach(ctx, &iobj, 1, actor, verb, prep,
                           PRP_IODEFAULT, FALSE, 0, 0, 1);
            }
            else
            {
                /* it's not even visible */
                if (recursive)
                    vocerr(ctx, VOCERR(39), "You don't see that here.");
                else
                    vocerr(ctx, VOCERR(38),
                           "You don't see that here any more.");
            }
            
            /* indicate the error */
            return ERR_PRS_VAL_IO_FAIL;
        }

        /* 
         *   save the command, unless this is a recursive call from the
         *   game, so that we can repeat this command if the next is
         *   "again" 
         */
        if (!recursive)
        {
            char *dst;
            
            /* save the command parameters */
            ctx->voccxlsa = actor;
            ctx->voccxlsv = verb;
            ctx->voccxlsp = prep;
            ctx->voccxlssty = newstyle;
            if (tpl != 0)
                memcpy(ctx->voccxlst, tpl,
                       (size_t)(newstyle ? VOCTPL2SIZ : VOCTPLSIZ));

            /* set up to write into the "again" word buffer */
            dst = ctx->voccxagainbuf;

            /* save the direct object information */
            exe_save_again_obj(&ctx->voccxlsd, dobjv, &dst);

            /* save the indirect object information */
            exe_save_again_obj(&ctx->voccxlsi, iobjv, &dst);

            /* 
             *   clear the flag indicating that "again" was lost due to
             *   object deletion, because we obviously have a valid
             *   "again" at this point 
             */
            ctx->voccxflg &= ~VOCCXAGAINDEL;
        }
    }
    
    /* remember the flags */
    tplflags = (tpl != 0 && newstyle ? voctplflg(tpl) : 0);
    dobj_first = (tplflags & VOCTPLFLG_DOBJ_FIRST);

    /* set up actor for tio subsystem - format strings need to know */
    tiosetactor(ctx->voccxtio, actor);

    /* store current dobj and iobj vocoldef's for later reference */
    ctx->voccxdobj = dobjv;
    ctx->voccxiobj = iobjv;

    /* store the rest of the current command objects for reference */
    ctx->voccxactor = actor;
    ctx->voccxverb = verb;
    ctx->voccxprep = prep;
    
    ERRBEGIN(ctx->voccxerr)

    /* reset the run-time context if this is a top-level call */
    if (!recursive)
        runrst(rcx);

    /* 
     *   if this is the first object, invoke the game's preCommand
     *   function, passing the list of all of the direct objects 
     */
    if (cur_dobj_idx == 0 && ctx->voccxprecmd != MCMONINV)
    {
        /* push the arguments: actor, verb, dobj-list, prep, iobj */
        runpobj(rcx, iobj);
        runpobj(rcx, prep);
        voc_push_vocoldef_list(ctx, dobj_list, dobj_cnt);
        runpobj(rcx, verb);
        runpobj(rcx, actor);

        /* catch errors specially for preCommand */
//        ERRBEGIN(ctx->voccxerr)
//        {
            /* invoke preCommand */
            runfn(rcx, ctx->voccxprecmd, 5);
//        }
//        ERRCATCH(ctx->voccxerr, err)
//        {
            /* 
             *   if the error was 'exit', translate it to EXITPRECMD so
             *   that we handle the outer loop correctly (exiting from
             *   preCommand skips execution for all subsequent objects,
             *   but doesn't skip fuses and daemons) 
             */
            if (err == ERR_RUNEXIT)
                errsig(ctx->voccxerr, ERR_RUNEXITPRECMD);

            /* no special handling - just resignal the error */
            errrse(ctx->voccxerr);
//        }
//        ERREND(ctx->voccxerr);
    }

    /* show the pre-object prefix if the caller instructed us to do so */
    voc_multi_prefix(ctx, dobj, show_multi_prefix, multi_flags,
                     cur_dobj_idx, dobj_cnt);

    /* 
     *   check to see if the verb has verbAction defined - if so, invoke
     *   the method
     */
    if (objgetap(ctx->voccxmem, verb, PRP_VERBACTION, (objnum *)0, FALSE))
    {
        /* call verb.verbAction(actor, dobj, prep, iobj) */
        runpobj(rcx, iobj);
        runpobj(rcx, prep);
        runpobj(rcx, dobj);
        runpobj(rcx, actor);
        runppr(rcx, verb, PRP_VERBACTION, 4);
    }

    /* invoke cmdActor.actorAction(verb, dobj, prep, iobj) */
    runpobj(rcx, iobj);
    runpobj(rcx, prep);
    runpobj(rcx, dobj);
    runpobj(rcx, verb);
    runppr(rcx, actor, PRP_ACTORACTION, 4);

    /* reset the run-time context if this is a top-level call */
    if (!recursive)
        runrst(rcx);

    /* invoke actor.location.roomAction(actor, verb, dobj, prep, iobj) */
    runppr(rcx, actor, PRP_LOCATION, 0);
    if (runtostyp(rcx) == DAT_OBJECT)
    {
        loc = runpopobj(rcx);

        /* reset the run-time context if this is a top-level call */
        if (!recursive)
            runrst(rcx);

        /* call roomAction */
        runpobj(rcx, iobj);
        runpobj(rcx, prep);
        runpobj(rcx, dobj);
        runpobj(rcx, verb);
        runpobj(rcx, actor);
        runppr(rcx, loc, PRP_ROOMACTION, 5);
    }
    else
    {
        /* the location isn't an object, so discard it */
        rundisc(rcx);
    }
    
    /* if there's an indirect object, execute iobjCheck */
    if (iobj != MCMONINV)
    {
        /* reset the run-time context if this is a top-level call */
        if (!recursive)
            runrst(rcx);

        /* invoke iobjCheck */
        runpobj(rcx, prep);
        runpobj(rcx, dobj);
        runpobj(rcx, verb);
        runpobj(rcx, actor);
        runppr(rcx, iobj, PRP_IOBJCHECK, 4);
    }

    /*
     *   If there's an indirect object, and the indirect object doesn't
     *   directly define io<Verb>, call iobj.iobjGen(actor, verb, dobj,
     *   prep) 
     */
    if (iobj != MCMONINV
        && exegen(ctx, iobj, PRP_IOBJGEN, voctplvi(tpl), voctplio(tpl)))
    {
        /* reset the run-time context if this is a top-level call */
        if (!recursive)
            runrst(rcx);

        /* invoke iobjGen */
        runpobj(rcx, prep);
        runpobj(rcx, dobj);
        runpobj(rcx, verb);
        runpobj(rcx, actor);
        runppr(rcx, iobj, PRP_IOBJGEN, 4);
    }

    /* if there's an direct object, execute dobjCheck */
    if (dobj != MCMONINV)
    {
        /* reset the run-time context if this is a top-level call */
        if (!recursive)
            runrst(rcx);

        /* invoke dobjCheck */
        runpobj(rcx, prep);
        runpobj(rcx, iobj);
        runpobj(rcx, verb);
        runpobj(rcx, actor);
        runppr(rcx, dobj, PRP_DOBJCHECK, 4);
    }

    /* 
     *   If there's a direct object, and the direct object doesn't
     *   directly define do<Verb>, call dobj.dobjGen(actor, verb, iobj,
     *   prep) 
     */
    if (dobj != MCMONINV
        && exegen(ctx, dobj, PRP_DOBJGEN, voctplvd(tpl), voctpldo(tpl)))
    {
        /* reset the run-time context if this is a top-level call */
        if (!recursive)
            runrst(rcx);

        /* invoke dobjGen */
        runpobj(rcx, prep);
        runpobj(rcx, iobj);
        runpobj(rcx, verb);
        runpobj(rcx, actor);
        runppr(rcx, dobj, PRP_DOBJGEN, 4);
    }

    /* reset the hidden-text flag */
    tiohide(ctx->voccxtio);
    tioshow(ctx->voccxtio);

    /*
     *   Now do what needs to be done, depending on the sentence structure:
     *   
     *      No objects ==> cmdVerb.action( cmdActor )
     *   
     *      Direct object only ==> cmdDobj.verDo<Verb>( actor )
     *.                            cmdDobj.do<Verb>( actor )
     *   
     *      Indirect + direct ==> cmdDobj.verDo<Verb>( actor, cmdIobj )
     *.                           cmdIobj.verIo<Verb>( actor, cmdDobj )
     *.                           cmdIobj.io<Verb>( actor, cmdDobj ) 
     */
    if (dobj == MCMONINV)
    {
        /* reset the stack for top-level calls */
        if (!recursive)
            runrst(rcx);

        /* invoke verb.action */
        runpobj(rcx, actor);
        runppr(rcx, verb, PRP_ACTION, 1);
    }
    else if (iobj == MCMONINV)
    {
        if (!objgetap(ctx->voccxmem, dobj, voctplvd(tpl), (objnum *)0, FALSE))
        {
            /* display the error */
            exeperr(ctx, verb, dobj, MCMONINV, MCMONINV);

            /* note that verDoVerb failed */
            err = ERR_PRS_NO_VERDO;

            /* we're done with this command */
            goto skipToFuses;
        }
        
        /* reset the stack for top-level calls */
        if (!recursive)
            runrst(rcx);

        /* invoke dobj.verDoVerb */
        runpobj(rcx, actor);
        runppr(rcx, dobj, voctplvd(tpl), 1);

        /* check for an error message from verDoVerb */
        if (!tioshow(ctx->voccxtio))
        {
            /* reset the stack for top-level calls */
            if (!recursive)
                runrst(rcx);

            /* dobj.verDoVerb displayed no output - process dobj.doVerb */
            runpobj(rcx, actor);
            runppr(rcx, dobj, voctpldo(tpl), 1);
        }
        else
        {
            /* note that verDoVerb failed */
            err = ERR_PRS_VERDO_FAIL;
        }
    }
    else
    {
        /* check to see if the verDoVerb and verIoVerb methods exist */
        if (!objgetap(ctx->voccxmem, dobj, voctplvd(tpl), (objnum *)0, FALSE))
        {
            /* no verDoVerb method - show a default message */
            exeperr(ctx, verb, dobj, MCMONINV, MCMONINV);

            /* note the error */
            err = ERR_PRS_NO_VERDO;

            /* skip to the end of the turn */
            goto skipToFuses;
        }
        else if (!objgetap(ctx->voccxmem, iobj, voctplvi(tpl), (objnum *)0,
                           FALSE))
        {
            /* no verIoVerb method - show a default mesage */
            exeperr(ctx, verb, MCMONINV, prep, iobj);

            /* note the error */
            err = ERR_PRS_NO_VERIO;

            /* skip to the end of the turn */
            goto skipToFuses;
        }

        /* reset the stack for top-level calls */
        if (!recursive)
            runrst(rcx);

        /* call verDoVerb(actor [,iobj]) */
        if (!dobj_first)
            runpobj(rcx, iobj);
        runpobj(rcx, actor);
        runppr(rcx, dobj, voctplvd(tpl), (dobj_first ? 1 : 2));

        /* check for error output from verDoVerb */
        if (!tioshow(ctx->voccxtio))
        {
            /* reset the stack for top-level calls */
            if (!recursive)
                runrst(rcx);

            /* no error from verDoVerb - call verIoVerb(actor [,dobj]) */
            if (dobj_first)
                runpobj(rcx, dobj);
            runpobj(rcx, actor);
            runppr(rcx, iobj, voctplvi(tpl), (dobj_first ? 2 : 1));

            /* check for error output from verIoVerb */
            if (!tioshow(ctx->voccxtio))
            {
                /* reset the stack for top-level calls */
                if (!recursive)
                    runrst(rcx);
                
                /* no error from verDoVerb or verIoVerb - call ioVerb */
                runpobj(rcx, dobj);
                runpobj(rcx, actor);
                runppr(rcx, iobj, voctplio(tpl), 2);
            }
            else
            {
                /* note the error */
                err = ERR_PRS_VERIO_FAIL;
            }
        }
        else
        {
            /* note the error */
            err = ERR_PRS_VERDO_FAIL;
        }
    }
    
  skipToFuses:
    ERRCATCH(ctx->voccxerr, err)
    {
        /* if askIo was invoked, get the preposition from the error stack */
        if (err == ERR_RUNASKI)
            *prepptr = errargint(0);

        /*
         *   If we executed 'abort', we'll skip straight to endCommand.
         *   
         *   If we executed askDo or askIo, we won't execute anything
         *   more, because the command is being interrupted.
         *   
         *   If 'exit' or 'exitobj' was executed, proceed through
         *   postAction and subsequent steps.
         *   
         *   If any error occurred other than 'exit' or 'exitobj' being
         *   invoked, resignal the error.
         *   
         *   We don't need to do anything more at this point if 'exit' was
         *   invoked, because 'exit' merely skips to the end-of-turn
         *   phase, which is where we'll go next from here.
         *   
         *   If 'exitobj' was invoked, we don't want to return an error at
         *   all, since we just want to skip the remainder of the normal
         *   processing for the current object and proceed to the next
         *   object (in a command with multiple direct objects).  
         */
        if (err == ERR_RUNABRT)
        {
            /* 
             *   aborting - we're going to call postAction, but we're not
             *   going to execute fuses and daemons 
             */
            do_fuses = FALSE;
            endturn = TRUE;
        }
        else if (err == ERR_RUNASKD || err == ERR_RUNASKI)
        {
            /* we're going to skip all end-of-turn action */
            do_fuses = FALSE;
            do_postact = FALSE;
            endturn = FALSE;
        }
        else if (err == ERR_RUNEXIT)
        {
            /* 
             *   Proceed with the remainder of the processing for this
             *   turn, but retain the error code to return to our caller,
             *   so they know that the rest of the turn is to be skipped.
             *   
             *   In addition, set 'do_fuses' to true, since we want to go
             *   directly to the fuse and daemon processing for this turn,
             *   regardless of whether any other objects are present
             *   (because we'll skip any remaining objects).  
             */
            endturn = TRUE;
            do_fuses = TRUE;
        }
        else if (err == ERR_RUNEXITPRECMD)
        {
            /* 
             *   exited from preCommand - end the turn, but do not run the
             *   postAction routine 
             */
            do_fuses = TRUE;
            do_postact = FALSE;
            endturn = TRUE;
        }
        else if (err == ERR_RUNEXITOBJ)
        {
            /* 
             *   Proceed with the remainder of processing for this turn -
             *   we want to proceed to the next object, if any, and
             *   process it as normal.  We don't need to update 'endturn'
             *   or 'do_fuses', since we want to do all of those in the
             *   normal fashion.  
             */
        }
        else
        {
            /*
             *   We can't handle any other errors.  Restore the enclosing
             *   command context, and resignal the error.  
             */

            /* restore the previous tio actor setting */
            tiosetactor(ctx->voccxtio, old_tio_actor);

            /* restore the original context iobj and dobj settings */
            ctx->voccxdobj = old_ctx_dobj;
            ctx->voccxiobj = old_ctx_iobj;

            /* restore the original context command objects */
            ctx->voccxactor = old_actor;
            ctx->voccxverb = old_verb;
            ctx->voccxprep = old_prep;

            /* resignal the error */
            errrse(ctx->voccxerr);
        }
    }
    ERREND(ctx->voccxerr);

    /*
     *   If desired, call postAction(actor, verb, dobj, prep, iobj,
     *   error_status).  
     */
    if (do_postact && ctx->voccxpostact != MCMONINV)
    {
        int err2;
        
        ERRBEGIN(ctx->voccxerr)
        {
            /* push the arguments */
            runpnum(rcx, err);
            runpobj(rcx, iobj);
            runpobj(rcx, prep);
            runpobj(rcx, dobj);
            runpobj(rcx, verb);
            runpobj(rcx, actor);

            /* invoke postAction */
            runfn(rcx, ctx->voccxpostact, 6);
        }
        ERRCATCH(ctx->voccxerr, err2)
        {
            /* remember the new error condition */
            err = err2;

            /* if we're aborting, skip fuses and daemons */
            if (err == ERR_RUNABRT)
            {
                endturn = TRUE;
                do_fuses = FALSE;
            }
        }
        ERREND(ctx->voccxerr);
    }
    
    /* restore the original context iobj and dobj settings */
    ctx->voccxdobj = old_ctx_dobj;
    ctx->voccxiobj = old_ctx_iobj;

    /* restore the original context command objects */
    ctx->voccxverb = old_verb;
    ctx->voccxprep = old_prep;

    /* reset the stack for top-level calls */
    if (!recursive)
        runrst(rcx);

    /* 
     *   If this is the end of the turn, execute fuses and daemons.  Skip
     *   fuses on recursive calls, since we want to count them as part of
     *   the enclosing turn.  
     */
    if (endturn && !recursive)
    {
        /* catch errors so that we can restore the actor globals */
        ERRBEGIN(ctx->voccxerr)
        {
            /* run fuses, daemons, and endCommand */
            err = exe_fuses_and_daemons(ctx, err, do_fuses, actor, verb,
                                        dobj_list, dobj_cnt, prep, iobj);
        }
        ERRCLEAN(ctx->voccxerr)
        {
            /* restore the previous actor globals */
            ctx->voccxactor = old_actor;
            tiosetactor(ctx->voccxtio, old_tio_actor);
        }
        ERRENDCLN(ctx->voccxerr);
    }

    /* restore the previous actor globals */
    ctx->voccxactor = old_actor;
    tiosetactor(ctx->voccxtio, old_tio_actor);

    /* success */
    return err;
}


/*
 *   saveit stores the current direct object list in 'it' or 'them'.
 */
static void exesaveit(voccxdef *ctx, vocoldef *dolist)
{
    int       cnt;
    int       i;
    int       dbg = ctx->voccxflg & VOCCXFDBG;
    runcxdef *rcx = ctx->voccxrun;

    cnt = voclistlen(dolist);
    if (cnt == 1)
    {
        /*
         *   check to make sure they're not referring to a number or a
         *   string; if so, it doesn't make any sense to save it 
         */
        if (dolist[0].vocolflg == VOCS_STR
            || dolist[0].vocolflg == VOCS_NUM)
        {
            /* 
             *   As of 2.5.11, don't clear 'it' on a number or string;
             *   rather, just leave it as it was from the prior command.
             *   Players will almost never expect a number or string to have
             *   anything to do with pronoun antecedents, and in fact some
             *   players reported finding it confusing to have the antecedant
             *   implied by the second-most-recent command disappear when the
             *   most recent command used a number of string.  
             */
#if 0
            /* save a nil 'it' */
            ctx->voccxit = MCMONINV;
            if (dbg)
                tioputs(ctx->voccxtio,
                        ".. setting 'it' to nil (strObj/numObj)\\n");
#endif

            /* we're done */
            return;
        }

        /* save 'it' */
        ctx->voccxit = dolist[0].vocolobj;
        ctx->voccxthc = 0;

        if (dbg)
        {
            tioputs(ctx->voccxtio, ".. setting it: ");
            runppr(rcx, ctx->voccxit, PRP_SDESC, 0);
            tioputs(ctx->voccxtio, "\\n");
        }

        /* set "him" if appropriate */
        runppr(rcx, ctx->voccxit, PRP_ISHIM, 0);
        if (runtostyp(rcx) == DAT_TRUE)
        {
            ctx->voccxhim = ctx->voccxit;
            if (dbg)
                tioputs(ctx->voccxtio,
                        "... [setting \"him\" to same object]\\n");
        }
        rundisc(rcx);

        /* set "her" if appropriate */
        runppr(rcx, ctx->voccxit, PRP_ISHER, 0);
        if (runtostyp(rcx) == DAT_TRUE)
        {
            ctx->voccxher = ctx->voccxit;
            if (dbg)
                tioputs(ctx->voccxtio,
                        "... [setting \"her\" to same object]\\n");
        }
        rundisc(rcx);
    }
    else if (cnt > 1)
    {
        ctx->voccxthc = cnt;
        ctx->voccxit  = MCMONINV;
        if (dbg) tioputs(ctx->voccxtio, ".. setting \"them\": [");
        for (i = 0 ; i < cnt ; ++i)
        {
            ctx->voccxthm[i] = dolist[i].vocolobj;
            if (dbg)
            {
                runppr(rcx, dolist[i].vocolobj, PRP_SDESC, 0);
                tioputs(ctx->voccxtio, i+1 < cnt ? ", " : "]\\n");
            }
        }
    }
}

/* display a multiple-object prefix */
void voc_multi_prefix(voccxdef *ctx, objnum objn,
                      int show_prefix, int multi_flags,
                      int cur_index, int count)
{
    runcxdef *rcx = ctx->voccxrun;

    /* if the object is invalid, ignore it */
    if (objn == MCMONINV)
        return;

    /* 
     *   if there's a prefixdesc method defined, call it rather than the
     *   older multisdesc (or even older sdesc) approach 
     */
    if (objgetap(ctx->voccxmem, objn, PRP_PREFIXDESC,
                 (objnum *)0, FALSE) != 0)
    {
        runsdef val;

        /* push the word flags */
        runpnum(rcx, multi_flags);

        /* 
         *   push the object count and the current index (adjusted to a
         *   1-based value) 
         */
        runpnum(rcx, count);
        runpnum(rcx, cur_index + 1);
        
        /* push the 'show' flag */
        val.runstyp = runclog(show_prefix);
        runpush(rcx, val.runstyp, &val);

        /* call the method */
        runppr(rcx, objn, PRP_PREFIXDESC, 4);

        /* we're done */
        return;
    }

    /* 
     *   if we're not showing the prefix, don't use the multisdesc/sdesc
     *   display 
     */
    if (!show_prefix)
        return;
    
    /*
     *   use multisdesc if defined (for compatibility with older games,
     *   use sdesc if multisdesc doesn't exist for this object) 
     */
    if (objgetap(ctx->voccxmem, objn, PRP_MULTISDESC,
                 (objnum *)0, FALSE) == 0)
    {
        /* there's no multisdesc defined - use the plain sdesc */
        runppr(rcx, objn, PRP_SDESC, 0);
    }
    else
    {
        /* multisdesc is defined - use it */
        runppr(rcx, objn, PRP_MULTISDESC, 0);
    }

    /* show the colon */
    vocerr_info(ctx, VOCERR(120), ": ");
}

/* execute command for each object in direct object list */
static int exeloop(voccxdef *ctx, objnum actor, objnum verb,
                   vocoldef *dolist, objnum *prep, vocoldef *iobj,
                   int multi_flags, uchar *tpl, int newstyle)
{
    runcxdef *rcx = ctx->voccxrun;
    int       err;
    int       i;
    int       dobj_cnt;
    int       exec_cnt;
    vocoldef *dobj;

    /* 
     *   count the direct objects; we'll iterate over the direct objects,
     *   so we execute the command once per direct object 
     */
    exec_cnt = dobj_cnt = (dolist != 0 ? voclistlen(dolist) : 0);

    /* 
     *   if there are no direct objects, we still must execute the command
     *   once 
     */
    if (exec_cnt < 1)
        exec_cnt = 1;

    /*
     *   If we have multiple direct objects, or we're using "all" with
     *   just one direct object, check with the verb to see if multiple
     *   words are acceptable: call verb.rejectMultiDobj, and see what it
     *   returns; if it returns true, don't allow multiple words, and
     *   expect that rejectMultiDobj displayed an error message.
     *   Otherwise, proceed.  
     */
    if (((multi_flags & VOCS_ALL) != 0 || dobj_cnt > 1)
        && dolist && dolist[0].vocolobj != MCMONINV)
    {
        int typ;

        ERRBEGIN(ctx->voccxerr)
            runrst(rcx);
            if (!prep || *prep == MCMONINV)
                runpnil(rcx);
            else
                runpobj(rcx, *prep);
            runppr(rcx, verb, PRP_REJECTMDO, 1);
            typ = runtostyp(rcx);
            rundisc(rcx);
        ERRCATCH(ctx->voccxerr, err)
            if (err == ERR_RUNEXIT || err == ERR_RUNEXITOBJ
                || err == ERR_RUNABRT)
                return err;
            else
                errrse(ctx->voccxerr);
        ERREND(ctx->voccxerr)

        /* if they returned 'true', don't bother continuing */
        if (typ == DAT_TRUE)
            return 0;
    }

    /* 
     *   execute the command the required number of times 
     */
    for (i = 0 ; i < exec_cnt ; ++i)
    {
        int show_multi_prefix;

        /* get the current direct object, if we have one */
        dobj = (dolist != 0 ? &dolist[i] : 0);

        /*
         *   If we have a number or string, set the current one in
         *   numObj/strObj 
         */
        if (dolist != 0)
        {
            if (dolist[i].vocolflg == VOCS_STR)
            {
                /* it's a string - set strObj.value */
                vocsetobj(ctx, ctx->voccxstr, DAT_SSTRING,
                          dolist[i].vocolfst + 1, &dolist[i], &dolist[i]);
            }
            else if (dolist[i].vocolflg == VOCS_NUM)
            {
                long v1, v2;

                /* it's a number - set numObj.value */
                v1 = atol(dolist[i].vocolfst);
                oswp4s(&v2, v1);
                vocsetobj(ctx, ctx->voccxnum, DAT_NUMBER, &v2,
                          &dolist[i], &dolist[i]);
            }
        }

        /*
         *   For cases where we have a bunch of direct objects (or even
         *   one when "all" was used), we want to preface the output from
         *   each iteration with the name of the object we're acting on
         *   currently.  In other cases, there is no prefix.  
         */
        show_multi_prefix = ((multi_flags != 0 || dobj_cnt > 1) && dobj != 0);

        /* 
         *   Execute the command for this object.  For every object except
         *   the first, re-validate the direct and indirect objects.
         *   There's no need to re-validate the objects on the first
         *   object in a command, because that will already have been done
         *   during object resolution. 
         */
        err = exe1cmd(ctx, actor, verb, dobj, prep, iobj,
                      (i + 1 == exec_cnt), tpl, newstyle, FALSE,
                      i != 0, i != 0, dolist, i, dobj_cnt,
                      show_multi_prefix, multi_flags);

        /* check the error - ignore any verification failures */
        switch(err)
        {
        case ERR_PRS_VERDO_FAIL:
        case ERR_PRS_VERIO_FAIL:
        case ERR_PRS_NO_VERDO:
        case ERR_PRS_NO_VERIO:
        case ERR_RUNEXITOBJ:
        case ERR_RUNEXIT:
            /* ignore the error and continue */
            err = 0;
            break;

        case ERR_RUNEXITPRECMD:
            /* 
             *   exited from preCommand - skip execution of subsequent
             *   objects, but return success 
             */
            return 0;

        case 0:
            /* no error; continue */
            break;

        default:
            /* anything else stops this command */
            return err;
        }

        /* flush output */
        tioflush(ctx->voccxtio);
    }

    /* success */
    return 0;
}

/*
 *   Execute a command recursively.  Game code can call this routine
 *   (indirectly through a built-in function) to execute a command, using
 *   all of the same steps that would be applied for the command if the
 *   player had typed it.
 */
int execmd_recurs(voccxdef *ctx, objnum actor, objnum verb,
                  objnum dobj, objnum prep, objnum iobj,
                  int validate_dobj, int validate_iobj)
{
    int       err;
    int       newstyle;
    uchar     tpl[VOCTPL2SIZ];
    vocoldef  dobjv;
    vocoldef  iobjv;
    voccxdef  ctx_copy;
    runsdef *orig_sp;
    runsdef *orig_bp;

    /*
     *   Save the stack and base pointers as they are on entry.  Since
     *   exe1cmd() is being called recursively, it won't automatically clear
     *   the stack after it's done as it would at the top level; this means
     *   that an aborted frame can be left on the stack if we throw an
     *   'exit' or 'abort' in the course of executing the command.  To make
     *   sure we don't leave any aborted frames on the stack before
     *   returning to our caller, we simply need to restore the stack and
     *   frame pointers on the way out as they were on the way in.  
     */
    orig_sp = ctx->voccxrun->runcxsp;
    orig_bp = ctx->voccxrun->runcxbp;

    /* make a copy of the voc context, so that changes aren't permanent */
    ctx_copy = *ctx;
    ctx = &ctx_copy;

    /* 
     *   there are no unknown words in the recursive command, since the
     *   command was prepared directly from resolved objects 
     */
    ctx->voccxunknown = 0;

    /* set up the vocoldef structure for the direct object, if present */
    if (dobj != MCMONINV)
    {
        dobjv.vocolobj = dobj;
        dobjv.vocolfst = dobjv.vocollst = "";
        dobjv.vocolflg = 0;
    }

    /* set up the vocoldef structure for the indirect object, if present */
    if (iobj != MCMONINV)
    {
        iobjv.vocolobj = iobj;
        iobjv.vocolfst = iobjv.vocollst = "";
        iobjv.vocolflg = 0;
    }
    
    /* figure out which template we need, based on the objects provided */
    if (dobj == MCMONINV)
    {
        uint actofs;
        uint tplofs;
        
        /* 
         *   No objects were provided - use the verb's "action" method.
         *   Make sure that there is in fact an "action" method. 
         */
        exe_get_tpl(ctx, verb, &tplofs, &actofs);
        if (actofs != 0)
        {
            /* execute the "action" method */
            err = exe1cmd(ctx, actor, verb, 0, &prep, 0, FALSE,
                          0, FALSE, TRUE, validate_dobj, validate_iobj,
                          0, 0, 0, FALSE, 0);
        }
        else
        {
            /* indicate that the sentence structure wasn't understood */
            err = ERR_PRS_SENT_UNK;
        }
    }
    else if (iobj == MCMONINV)
    {
        /* 
         *   No indirect object was provided, but a direct object is
         *   present - use the one-object template.  First, look up the
         *   template.  
         */
        if (voctplfnd(ctx, verb, MCMONINV, tpl, &newstyle))
        {
            /* execute the command */
            err = exe1cmd(ctx, actor, verb, &dobjv, &prep, 0, FALSE,
                          tpl, newstyle, TRUE, validate_dobj, validate_iobj,
                          &dobjv, 0, 1, FALSE, 0);
        }
        else
        {
            /* indicate that the sentence structure wasn't understood */
            err = ERR_PRS_SENT_UNK;
        }
    }
    else
    {
        /* 
         *   Both a direct and indirect object were provided - find the
         *   two-object template for the given preposition.
         */
        if (voctplfnd(ctx, verb, prep, tpl, &newstyle))
        {
            /* execute the command */
            err = exe1cmd(ctx, actor, verb, &dobjv, &prep, &iobjv, FALSE,
                          tpl, newstyle, TRUE, validate_dobj, validate_iobj,
                          &dobjv, 0, 1, FALSE, 0);
        }
        else
        {
            /* indicate that the sentence structure wasn't understood */
            err = ERR_PRS_SENT_UNK;
        }
    }

    /* 
     *   if the error was EXITPRECMD, change it to EXIT - EXITPRECMD is a
     *   special flag indicating that we exited from a preCommand
     *   function, which is different than normal exiting internally but
     *   not to the game 
     */
    if (err == ERR_RUNEXITPRECMD)
        err = ERR_RUNEXIT;

    /*
     *   restore the original stack and base pointers, to ensure that we
     *   don't leave any aborted frames on the stack 
     */
    ctx->voccxrun->runcxsp = orig_sp;
    ctx->voccxrun->runcxbp = orig_bp;

    /* return the result code */
    return err;
}


/*
 *   Check for ALL, ANY, or THEM in the list - use multi-mode if found,
 *   even if we have only one object.  Returns a combination of any of the
 *   VOCS_ALL, VOCS_ANY, or VOCS_THEM flags that we find.  
 */
static int check_for_multi(vocoldef *dolist)
{
    int dolen;
    int i;
    int result;

    /* presume we won't find any flags */
    result = 0;

    /* 
     *   scan the list for ALL, ANY, or THEM flags, combining any such
     *   flags we find into the result 
     */
    dolen = voclistlen(dolist);
    for (i = 0 ; i < dolen ; ++i)
        result |= (dolist[i].vocolflg & (VOCS_ALL | VOCS_ANY | VOCS_THEM));

    /* return the result */
    return result;
}

/* ------------------------------------------------------------------------ */
/*
 *   Try running the preparseCmd user function.  Returns 0 if the
 *   function doesn't exist or returns 'true', ERR_PREPRSCMDCAN if it
 *   returns 'nil' (and thus wants to cancel the command), and
 *   ERR_PREPRSCMDREDO if it returns a list (and thus wants to redo the
 *   command). 
 */
int try_preparse_cmd(voccxdef *ctx, char **cmd, int wrdcnt,
                     uchar **preparse_list)
{
    uchar    listbuf[VOCBUFSIZ + 2 + 3*VOCBUFSIZ];
    int      i;
    uchar   *p;
    size_t   len;
    runsdef  val;
    int      typ;
    int      err;

    /* if there's no preparseCmd, keep processing */
    if (ctx->voccxppc == MCMONINV)
        return 0;
    
    /* build a list of the words */
    for (p = listbuf + 2, i = 0 ; i < wrdcnt ; ++i)
    {
        char *src;
        int add_quote;
        
        /* check for strings - they require special handling */
        if (cmd[i][0] == '"')
        {
            /* 
             *   it's a string - what follows is a run-time style string,
             *   with a length prefix followed by the text of the string 
             */
            len = osrp2(cmd[i] + 1) - 2;
            src = cmd[i] + 3;

            /* add quotes to the result */
            add_quote = TRUE;
        }
        else
        {
            /* ordinary word - copy directly */
            src = (char *)cmd[i];

            /* it's a null-terminated string */
            len = strlen(src);

            /* don't add quotes to the result */
            add_quote = FALSE;
        }

        /* write the type prefix */
        *p++ = DAT_SSTRING;

        /* write the length prefix */
        oswp2(p, len + 2 + (add_quote ? 2 : 0));
        p += 2;

        /* add an open quote if necessary */
        if (add_quote)
            *p++ = '"';

        /* copy the text */
        memcpy(p, src, len);
        p += len;

        /* add the closing quote if necessary */
        if (add_quote)
            *p++ = '"';
    }
    
    /* set the length of the whole list */
    len = p - listbuf;
    oswp2(listbuf, len);
    
    /* push the list as the argument, and call the user's preparseCmd */
    val.runstyp = DAT_LIST;
    val.runsv.runsvstr = listbuf;
    runpush(ctx->voccxrun, DAT_LIST, &val);

    /* presume that no error will occur */
    err = 0;

    /* catch errors that occur within preparseCmd */
    ERRBEGIN(ctx->voccxerr)
    {
        /* call preparseCmd */
        runfn(ctx->voccxrun, ctx->voccxppc, 1);
    }
    ERRCATCH(ctx->voccxerr, err)
    {
        /* 
         *   if it's abort/exit/exitobj, just return it; for any other
         *   errors, just re-throw the same error 
         */
        switch(err)
        {
        case ERR_RUNABRT:
        case ERR_RUNEXIT:
        case ERR_RUNEXITOBJ:
            /* simply return these errors to the caller */
            break;

        default:
            /* re-throw anything else */
            errrse(ctx->voccxerr);
        }
    }
    ERREND(ctx->voccxerr);

    /* if an error occurred, return the error code */
    if (err != 0)
        return err;

    /* get the result */
    typ = runtostyp(ctx->voccxrun);
    
    /* if they returned a list, it's a new command to execute */
    if (typ == DAT_LIST)
    {
        /* get the list and give it to the caller */
        *preparse_list = runpoplst(ctx->voccxrun);
        
        /* 
         *   indicate that the command is to be reparsed with the new word
         *   list 
         */
        return ERR_PREPRSCMDREDO;
    }

    /* for any other type, we don't need the value, so discard it */
    rundisc(ctx->voccxrun);

    /* if the result is nil, don't process this command further */
    if (typ == DAT_NIL)
        return ERR_PREPRSCMDCAN;
    else
        return 0;
}


/* ------------------------------------------------------------------------ */
/*
 *   Call parseAskobjIndirect 
 */
static void voc_askobj_indirect(voccxdef *ctx, vocoldef *dolist,
                                objnum actor, objnum verb, objnum prep)
{
    int cnt;
    int i;
    size_t len;
    uchar *lstp;
    
    /*
     *   Generate the direct object list argument.  This argument is a
     *   list of lists.  For each noun phrase, we generate one sublist in
     *   the main list.  Each sublist itself consists of three
     *   sub-sublists: first, a list of strings giving the words in the
     *   noun phrase; second, a list of the objects matching the noun
     *   phrase; third, a list of the flags for the matching objects.
     *   
     *   So, if the player typed "put red box and blue ball", we might
     *   generate a list something like this:
     *   
     *   [ [ ['red', 'box'], [redBox1, redBox2], [0, 0] ], [ ['blue',
     *   'ball'], [blueBall], [0, 0] ] ] 
     */

    /*
     *   First, figure out how much space we need for this list of lists
     *   of lists.  Scan the direct object list for distinct noun phrases
     *   - we need one sublist for each distinct noun phrase.
     */
    cnt = voclistlen(dolist);
    for (len = 0, i = 0 ; i < cnt ; )
    {
        const char *p;
        size_t curlen;
        int j;
            
        /* 
         *   we need the sublist type prefix (one byte) plus the sublist
         *   length prefix (two bytes), plus the type and length prefixes
         *   (one plus two bytes) for each of the three sub-sublist 
         */
        len += (1+2) + 3*(1+2);

        /* 
         *   we need space to store the strings for the words in this noun
         *   phrase 
         */
        for (p = dolist[i].vocolfst ; p != 0 && p <= dolist[i].vocollst ;
             p += curlen + 1)
        {
            /* 
             *   add in the space needed for this string element in the
             *   sub-sublist - we need one byte for the type prefix, two
             *   bytes for the length prefix, and the bytes for the string
             *   itself 
             */
            curlen = strlen(p);
            len += (1+2) + curlen;
        }

        /* 
         *   scan each object for this same noun phrase (i.e., for which
         *   the vocabulary words are the same) 
         */
        for (j = i ; j < cnt && dolist[j].vocolfst == dolist[i].vocolfst ;
             ++j)
        {
            /* 
             *   Add in space for this object in the sub-sublist for the
             *   current noun phrase.  If this object is nil, we need only
             *   one byte for the type; otherwise, we need one byte for
             *   the type prefix plus two bytes for the object ID.  
             */
            if (dolist[i].vocolobj == MCMONINV)
                len += 1;
            else
                len += (1 + 2);
            
            /*
             *   Add in space for the flags sub-sublist for the current
             *   object.  We need one byte for the type and four for the
             *   integer value.  
             */
            len += (1 + 4);
        }

        /* skip to the next distinct noun phrase */
        i = j;
    }

    /* allocate the list */
    lstp = voc_push_list_siz(ctx, len);

    /* 
     *   Go through our object array again, and this time actually build
     *   the list.  
     */
    for (i = 0 ; i < cnt ; )
    {
        const char *p;
        uchar *subp;
        uchar *subsubp;
        size_t curlen;
        int j;

        /* start the sublist with the type prefix */
        *lstp++ = DAT_LIST;

        /* leave a placeholder for our length prefix */
        subp = lstp;
        lstp += 2;

        /* start the sub-sublist with the word strings */
        *lstp++ = DAT_LIST;
        subsubp = lstp;
        lstp += 2;

        /* store the word strings in the sub-sublist */
        for (p = dolist[i].vocolfst ; p != 0 && p <= dolist[i].vocollst ;
             p += curlen + 1)
        {
            /* get this string's length */
            curlen = strlen(p);

            /* store the type and length prefixes */
            *lstp++ = DAT_SSTRING;
            oswp2(lstp, curlen + 2);
            lstp += 2;

            /* store the string */
            memcpy(lstp, p, curlen);
            lstp += curlen;
        }

        /* fix up the string sub-sublist length */
        oswp2(subsubp, lstp - subsubp);

        /* start the second sub-sublist, for the objects */
        *lstp++ = DAT_LIST;
        subsubp = lstp;
        lstp += 2;

        /* write each object */
        for (j = i ; j < cnt && dolist[j].vocolfst == dolist[i].vocolfst ;
             ++j)
        {
            /* 
             *   if this object isn't nil, write it to the sub-sublist;
             *   otherwise, just put nil in the sub-sublist 
             */
            if (dolist[j].vocolobj != MCMONINV)
            {
                *lstp++ = DAT_OBJECT;
                oswp2(lstp, dolist[j].vocolobj);
                lstp += 2;
            }
            else
            {
                /* no object - just store nil */
                *lstp++ = DAT_NIL;
            }
        }

        /* fix up the object sub-sublist length */
        oswp2(subsubp, lstp - subsubp);

        /* start the third sub-sublist, for the flags */
        *lstp++ = DAT_LIST;
        subsubp = lstp;
        lstp += 2;

        /* write each object's flags */
        for (j = i ; j < cnt && dolist[j].vocolfst == dolist[i].vocolfst ;
             ++j)
        {
            /* write the flags */
            *lstp++ = DAT_NUMBER;
            oswp4s(lstp, dolist[j].vocolflg);
            lstp += 4;
        }

        /* fix up the flag sub-sublist length */
        oswp2(subsubp, lstp - subsubp);

        /* skip to the start of the next distinct noun phrase */
        i = j;

        /* fix up the sublist length */
        oswp2(subp, lstp - subp);
    }

    /* push the prep, verb, and actor arguments */
    runpobj(ctx->voccxrun, prep);
    runpobj(ctx->voccxrun, verb);
    runpobj(ctx->voccxrun,
            (objnum)(actor == MCMONINV ? ctx->voccxme : actor));

    /* call the function */
    runfn(ctx->voccxrun, ctx->voccxpask3, 4);
}


/* ------------------------------------------------------------------------ */
/*
 *   execmd() - executes a user's command given the verb's verb and
 *   preposition words, a list of nouns to be used as indirect objects,
 *   and a list to be used for direct objects.  The globals cmdActor and
 *   cmdPrep should already be set.  This routine tries to find a template
 *   for the verb which matches the player's command.  If no template
 *   matches, we try (using default objects and, if that fails, requests
 *   to the player for objects) to fill in any missing information in the
 *   player's command.  If that still fails, we will say we don't
 *   understand the sentence and leave it at that.  
 */
int execmd(voccxdef *ctx, objnum actor, objnum prep,
           char *vverb, char *vprep, vocoldef *dolist, vocoldef *iolist,
           char **cmd, int *typelist,
           char *cmdbuf, int wrdcnt, uchar **preparse_list, int *next_word)
{
    objnum    verb;
    objnum    iobj;
    int       multi_flags = 0;
    vocwdef  *n;
    int       cnt;
    vocoldef *newnoun;
    int       next;
    char     *exenewcmd;
    char     *donewcmd;
    char     *ionewcmd;
    char     *exenewbuf;
    char     *donewbuf;
    char     *ionewbuf;
    char    **exenewlist;
    char    **donewlist;
    char    **ionewlist;
    int      *exenewtype;
    int      *donewtype;
    int      *ionewtype;
    vocoldef *dolist1;
    vocoldef *iolist1;
    uchar     tpl[VOCTPL2SIZ];
    int       foundtpl;        /* used to determine success of tpl searches */
    runcxdef *rcx = ctx->voccxrun;
    uint      tplofs;                          /* offset of template object */
    uint      actofs;                        /* offset of 'action' property */
    int       askflags;                /* flag for what we need to ask user */
    int       newstyle;   /* flag indicating new-style template definitions */
    int       tplflags;
    int       err;
    uchar    *save_sp;

    /* run preparseCmd */
    switch(try_preparse_cmd(ctx, cmd, wrdcnt, preparse_list))
    {
    case 0:
        /* proceed with the command */
        break;

    case ERR_PREPRSCMDCAN:
        /* command cancelled */
        return 0;

    case ERR_RUNEXIT:
    case ERR_RUNABRT:
    case ERR_RUNEXITOBJ:
        /* abort/exit/exitobj - treat this the same as command cancellation */
        return 0;

    case ERR_PREPRSCMDREDO:
        /* redo the command - so indicate to the caller */
        return ERR_PREPRSCMDREDO;
    }

    /* look up the verb based on the verb and verb-prep */
    n = vocffw(ctx, vverb, (int)strlen(vverb),
               vprep, (vprep ? (int)strlen(vprep) : 0), PRP_VERB,
               (vocseadef *)0);

    /* if we didn't find a verb template, we can't process the sentence */
    if (n == 0)
    {
        /* try parseUnknownVerb, and show an error if that doesn't handle it */
        if (try_unknown_verb(ctx, actor, cmd, typelist, wrdcnt, next_word,
                             TRUE, VOCERR(18),
                             "I don't understand that sentence."))
        {
            /* they handled it successfully - end the command with success */
            return 0;
        }
        else
        {
            /* 
             *   parseUnknownVerb failed or aborted - end the command with
             *   an error 
             */
            return 1;
        }
    }

    /* get the deepverb object */
    verb = n->vocwobj;

    /* default actor is "Me" */
    if (actor == MCMONINV)
        actor = ctx->voccxme;
    
    /* set a savepoint, if we're keeping undo information */
    if (ctx->voccxundo)
        objusav(ctx->voccxundo);

    /*
     *   Check that the room will allow this command -- it may not
     *   due to darkness or other ailment.  We can find out with the
     *   roomCheck(verb) message, sent to the meobj.  
     */
    {
        int t;

        /* call roomCheck */
        runrst(rcx);
        runpobj(rcx, verb);
        runppr(rcx, ctx->voccxme, PRP_ROOMCHECK, 1);
        t = runpoplog(rcx);

        /* if they returned nil, stop the command, but indicate success */
        if (!t)
            return 0;
    }

    /* look for a new-style template first, then the old-style template */
    exe_get_tpl(ctx, verb, &tplofs, &actofs);
    
    /* make sure we found a verb */
    if (tplofs == 0 && actofs == 0 && verb != ctx->voccxvag)
    {
        /* try parseUnknownVerb, and show an error if that doesn't handle it */
        if (try_unknown_verb(ctx, actor, cmd, typelist, wrdcnt, next_word,
                             TRUE, VOCERR(23),
                 "internal error: verb has no action, doAction, or ioAction"))
            return 0;
        else
            return 1;
    }

    /*
     *   Check to see if we have an "all" - if we do, we'll need to
     *   display the direct object's name even if only one direct object
     *   comes of it.  
     */
    multi_flags = check_for_multi(dolist);

    /* 
     *   set up dobj word list in case objwords is used in doDefault (the
     *   game may want to check for "all" and disallow it, for example)
     */
    ctx->voccxdobj = dolist;

    /* set up our stack allocations, which we may need from now on */
    voc_enter(ctx, &save_sp);
    VOC_STK_ARRAY(ctx, char,     donewcmd,  VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char,     ionewcmd,  VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char,     donewbuf,  2*VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char,     ionewbuf,  2*VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char *,   donewlist, VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char *,   ionewlist, VOCBUFSIZ);
    VOC_MAX_ARRAY(ctx, int,      donewtype);
    VOC_MAX_ARRAY(ctx, int,      ionewtype);
    VOC_MAX_ARRAY(ctx, vocoldef, dolist1);
    VOC_MAX_ARRAY(ctx, vocoldef, iolist1);

    /* keep going until we're done with the sentence */
    for ( ;; )
    {
        askflags = err = 0;
        
        ERRBEGIN(ctx->voccxerr)
        
        /*
         *   Now see what kind of sentence we have.  If we have no
         *   objects and an action, use the action.  If we have a direct
         *   object and a doAction, use the doAction.  If we have an
         *   indirect object and an ioAction with a matching preposition,
         *   use the ioAction.  If we have an indirect object and no
         *   matching ioAction, complain.  If we have a direct object and
         *   no doAction or ioAction, complain.  If we have fewer objects
         *   than we really want, ask the user for more of them.  
         */
        if (voclistlen(dolist) == 0 && voclistlen(iolist) == 0)
        {
            if (actofs || verb == ctx->voccxvag)
            {
                if ((err = exeloop(ctx, actor, verb, (vocoldef *)0, &prep,
                                   (vocoldef *)0, multi_flags,
                                   (uchar *)0, 0)) != 0)
                    goto exit_error;
            }
            else
            {
                /*
                 *   The player has not specified any objects, but the
                 *   verb seems to require one.  See if there's a unique
                 *   default.  
                 */
                runrst(rcx);
                runpnil(rcx);
                runpobj(rcx, prep);
                runpobj(rcx, actor);
                runppr(rcx, verb, PRP_DODEFAULT, 3);
                
                if (runtostyp(rcx) == DAT_LIST)
                {
                    uchar   *l = runpoplst(rcx);
                    uint     lstsiz;
                    objnum   defobj = 0;
                    int      objcnt;
                    objnum   newprep;
                    runsdef  val;
                    objnum   o;
                    
                    /* push list back on stack, to keep in heap */
                    val.runsv.runsvstr = l;
                    val.runstyp = DAT_LIST;
                    runrepush(rcx, &val);
                    
                    /* get list size out of list */
                    lstsiz = osrp2(l) - 2;
                    l += 2;

                    /* find default preposition for verb, if any */
                    runppr(rcx, verb, PRP_PREPDEFAULT, 0);
                    if (runtostyp(rcx) == DAT_OBJECT)
                        newprep = runpopobj(rcx);
                    else
                    {
                        newprep = MCMONINV;
                        rundisc(rcx);
                    }
                    
                    if (!voctplfnd(ctx, verb, newprep, tpl, &newstyle))
                    {
                        for (objcnt = 0 ; lstsiz && objcnt < 2
                             ; lstadv(&l, &lstsiz))
                        {
                            if (*l == DAT_OBJECT)
                            {
                                ++objcnt;
                                defobj = osrp2(l + 1);
                            }
                        }
                    }
                    else
                    {
                        int dobj_first;
                        
                        /*
                         *   Get the template flags.  If we must
                         *   disambiguate the direct object first for this
                         *   verb, do so now. 
                         */
                        tplflags = (newstyle ? voctplflg(tpl) : 0);
                        dobj_first = (tplflags & VOCTPLFLG_DOBJ_FIRST);

                        for (objcnt = 0 ; lstsiz && objcnt < 2
                             ; lstadv(&l, &lstsiz))
                        {
                            if (*l == DAT_OBJECT)
                            {
                                o = osrp2(l + 1);
                                if (!objgetap(ctx->voccxmem, o, voctplvd(tpl),
                                              (objnum *)0, FALSE))
                                    continue;
                                
                                tiohide(ctx->voccxtio);
                                if (newprep != MCMONINV && !dobj_first)
                                    runpnil(rcx);
                                runpobj(rcx, actor);
                                runppr(rcx, o, voctplvd(tpl),
                                       ((newprep != MCMONINV && !dobj_first)
                                        ? 2 : 1));
                                
                                if (!tioshow(ctx->voccxtio))
                                {
                                    ++objcnt;
                                    defobj = o;
                                }
                            }
                        }
                        
                        /* no longer need list in heap, so discard it */
                        rundisc(rcx);
                
                        /* use default object if there's exactly one */
                        if (objcnt == 1)
                        {
                            dolist[0].vocolobj = defobj;
                            dolist[0].vocolflg = 0;
                            dolist[0].vocolfst = dolist[0].vocollst = 0;
                            dolist[1].vocolobj = MCMONINV;
                            dolist[1].vocolflg = 0;
                            dolist[1].vocolfst = dolist[1].vocollst = 0;

                            runrst(rcx);
                            if (ctx->voccxpdef2 != MCMONINV)
                            {
                                runpnil(rcx);
                                runpobj(rcx, defobj);
                                runpobj(rcx, verb);
                                runpobj(rcx, actor);
                                runfn(rcx, ctx->voccxpdef2, 4);
                            }
                            else if (ctx->voccxpdef != MCMONINV)
                            {
                                runpnil(rcx);
                                runpobj(rcx, defobj);
                                runfn(rcx, ctx->voccxpdef, 2);
                            }
                            else
                            {
                                /* tell the player what we're doing */
                                vocerr_info(ctx, VOCERR(130), "(");
                                runppr(rcx, defobj, PRP_THEDESC, 0);
                                vocerr_info(ctx, VOCERR(131), ")");
                                tioflush(ctx->voccxtio);
                            }
                            err = -2;                         /* "continue" */
                            goto exit_error;
                        }
                    }
                }
                else
                    rundisc(rcx);
            
                /*
                 *   No unique default; ask the player for a direct
                 *   object, and try the command again if he is kind
                 *   enough to provide one.  
                 */
                askflags = ERR_RUNASKD;
            }
        }
        else if (voclistlen(iolist) == 0)
        {
            /* direct object(s), but no indirect object -- find doAction */
            if (voctplfnd(ctx, verb, MCMONINV, tpl, &newstyle))
            {
                /* disambiguate the direct object list, now that we can */
                if (vocdisambig(ctx, dolist1, dolist, PRP_DODEFAULT,
                                PRP_VALIDDO, voctplvd(tpl), cmd, MCMONINV,
                                actor, verb, prep, cmdbuf, FALSE))
                {
                    err = -1;
                    goto exit_error;
                }
                iobj = MCMONINV;

                /*
                 *   save the disambiguated direct object list, in case
                 *   we hit an askio in the course of processing it 
                 */
                memcpy(dolist, dolist1,
                       (size_t)(voclistlen(dolist1) + 1)*sizeof(dolist[0]));

                /* re-check for multi-mode */
                if (multi_flags == 0)
                    multi_flags = check_for_multi(dolist1);
                
                /* save it/them/him/her, and execute the command */
                exesaveit(ctx, dolist1);
                if ((err = exeloop(ctx, actor, verb, dolist1, &prep,
                                   (vocoldef *)0, multi_flags,
                                   tpl, newstyle)) != 0)
                    goto exit_error;
            }
            else
            {
                /* no doAction - we'll need to find an indirect object */
                runrst(rcx);
                runppr(rcx, verb, PRP_PREPDEFAULT, 0);
                if (runtostyp(rcx) != DAT_OBJECT)
                {
                    /* discard the result */
                    rundisc(rcx);

                    /* call parseUnknownVerb to handle it */
                    if (try_unknown_verb(ctx, actor, cmd, typelist,
                                         wrdcnt, next_word, TRUE, VOCERR(24),
                                         "I don't recognize that sentence."))
                    {
                        /* handled - end the command successfully */
                        err = 0;
                    }
                    else
                    {
                        /* not handled - indicate failure */
                        err = -1;
                    }
                    goto exit_error;
                }
                prep = runpopobj(rcx);
            
                runrst(rcx);
                runpobj(rcx, prep);
                runpobj(rcx, actor);
                runppr(rcx, verb, PRP_IODEFAULT, 2);
                
                if (runtostyp(rcx) == DAT_LIST)
                {
                    uchar   *l = runpoplst(rcx);
                    uint     lstsiz;
                    objnum   defobj = 0;
                    int      objcnt;
                    runsdef  val;
                    objnum   o;
                    
                    /* push list back on stack, to keep in heap */
                    val.runsv.runsvstr = l;
                    val.runstyp = DAT_LIST;
                    runrepush(rcx, &val);
                    
                    /* get list size out of list */
                    lstsiz = osrp2(l) - 2;
                    l += 2;
                    
                    if (!voctplfnd(ctx, verb, prep, tpl, &newstyle))
                    {
                        for (objcnt = 0 ; lstsiz && objcnt < 2
                             ; lstadv(&l, &lstsiz))
                        {
                            if (*l == DAT_OBJECT)
                            {
                                objcnt++;
                                defobj = osrp2(l + 1);
                            }
                        }
                    }
                    else
                    {
                        int dobj_first;
                        
                        /*
                         *   Get the template flags.  If we must
                         *   disambiguate the direct object first for this
                         *   verb, do so now. 
                         */
                        tplflags = (newstyle ? voctplflg(tpl) : 0);
                        dobj_first = (tplflags & VOCTPLFLG_DOBJ_FIRST);
                        if (dobj_first)
                        {
                            if (vocdisambig(ctx, dolist1, dolist,
                                            PRP_DODEFAULT, PRP_VALIDDO,
                                            voctplvd(tpl), cmd, MCMONINV,
                                            actor, verb, prep, cmdbuf,
                                            FALSE))
                            {
                                err = -1;
                                goto exit_error;
                            }

                            /* only one direct object is allowed here */
                            if (voclistlen(dolist1) > 1)
                            {
                                vocerr(ctx, VOCERR(28),
                          "You can't use multiple objects with this command.");
                                err = -1;
                                goto exit_error;
                            }

                            /* save the object in the original list */
                            memcpy(dolist, dolist1,
                                   (size_t)(2 * sizeof(dolist[0])));
                        }
                        
                        for (objcnt = 0 ; lstsiz && objcnt < 2
                             ; lstadv(&l, &lstsiz))
                        {
                            if (*l == DAT_OBJECT)
                            {
                                o = osrp2(l + 1);
                                if (!objgetap(ctx->voccxmem, o, voctplvi(tpl),
                                              (objnum *)0, FALSE))
                                    continue;
                                
                                tiohide(ctx->voccxtio);
                                if (dobj_first)
                                    runpobj(rcx, dolist[0].vocolobj);
                                runpobj(rcx, actor);
                                runppr(rcx, o, voctplvi(tpl),
                                       (dobj_first ? 2 : 1));
                                if (!tioshow(ctx->voccxtio))
                                {
                                    objcnt++;
                                    defobj = o;
                                }
                            }
                        }
                    }
                    
                    /* no longer need list in heap, so discard it */
                    rundisc(rcx);

                    /* if there's exactly one default object, use it */
                    if (objcnt == 1)
                    {
                        iolist[0].vocolobj = defobj;
                        iolist[0].vocolflg = 0;
                        iolist[0].vocolfst = iolist[0].vocollst = 0;
                        iolist[1].vocolobj = MCMONINV;
                        iolist[1].vocolflg = 0;
                        iolist[1].vocolfst = iolist[1].vocollst = 0;
                    
                        /* tell the user what we're assuming */
                        runrst(rcx);
                        if (ctx->voccxpdef2 != MCMONINV)
                        {
                            runpobj(rcx, prep);
                            runpobj(rcx, defobj);
                            runpobj(rcx, verb);
                            runpobj(rcx, actor);
                            runfn(rcx, ctx->voccxpdef2, 4);
                        }
                        else if (ctx->voccxpdef != MCMONINV)
                        {
                            runpobj(rcx, prep);
                            runpobj(rcx, defobj);
                            runfn(rcx, ctx->voccxpdef, 2);
                        }
                        else
                        {
                            vocerr_info(ctx, VOCERR(130), "(");
                            runppr(rcx, prep, PRP_SDESC, 0);
                            vocerr_info(ctx, VOCERR(132), " ");
                            runppr(rcx, defobj, PRP_THEDESC, 0);
                            vocerr_info(ctx, VOCERR(131), ")");
                        }
                        tioflush(ctx->voccxtio);
                        err = -2;                             /* "continue" */
                        goto exit_error;
                    }
                }
                else
                    rundisc(rcx);
            
                /*
                 *   We didn't get a unique default indirect object, so
                 *   we should ask the player for an indirct object, and
                 *   repeat the command should he provide one.  
                 */
                askflags = ERR_RUNASKI;
            }
        }
        else
        {
            objnum otherobj;
            
            /* find the template for this verb/prep combination */
            if (!voctplfnd(ctx, verb, prep, tpl, &newstyle))
            {
                vocoldef *np1;
                
                /* 
                 *   If we could have used the preposition in the first noun
                 *   phrase rather than in the verb, and this would have
                 *   yielded a valid verb phrase, the error is "I don't see
                 *   any <noun phrase> here".
                 *   
                 *   Otherwise, it's a verb phrasing error.  In this case,
                 *   call parseUnknownVerb to handle the error; the default
                 *   error is "I don't recognize that sentence".  
                 */
                np1 = dolist[0].vocolfst < iolist[0].vocolfst
                      ? dolist : iolist;
                if ((np1->vocolflg & VOCS_TRIMPREP) != 0)
                {
                    char namebuf[VOCBUFSIZ];
                    
                    /* 
                     *   it's a trimmed prep phrase, so we actually have an
                     *   unmatched object - report the error 
                     */
                    voc_make_obj_name_from_list(
                        ctx, namebuf, cmd, np1->vocolfst, np1->vocolhlst);
                    vocerr(ctx, VOCERR(9), "I don't see any %s here.",
                           namebuf);

                    /* terminate the command with an error */
                    err = -1;
                }
                else if (try_unknown_verb(ctx, actor, cmd, typelist,
                                     wrdcnt, next_word, TRUE,
                                     VOCERR(24),
                                     "I don't recognize that sentence."))
                {
                    /* they handled it - terminate command successfully */
                    err = 0;
                }
                else
                {
                    /* that failed - terminate the command with an error */
                    err = -1;
                }

                /* terminate the command */
                goto exit_error;
            }

            /*
             *   We have both direct and indirect objects.  If we don't
             *   yet have the direct object, go ask for it 
             */
            if (voclistlen(dolist) == 0)
            {
                askflags = ERR_RUNASKD;
                goto exit_error;
            }

            /* get the flags (if old-style, flags are always zero) */
            tplflags = (newstyle ? voctplflg(tpl) : 0);

            /*
             *   the "other" object (dobj if doing iobj, iobj if doing
             *   dobj) is not known when the first object is disambiguated
             */
            otherobj = MCMONINV;

            /* disambiguate the objects in the proper order */
            if (tplflags & VOCTPLFLG_DOBJ_FIRST)
            {
                /* disambiguate the direct object list */
                if (vocdisambig(ctx, dolist1, dolist, PRP_DODEFAULT,
                                PRP_VALIDDO, voctplvd(tpl), cmd, otherobj,
                                actor, verb, prep, cmdbuf, FALSE))
                {
                    err = -1;
                    goto exit_error;
                }

                /*
                 *   only one direct object is allowed if it's
                 *   disambiguated first 
                 */
                if (voclistlen(dolist1) > 1)
                {
                    vocerr(ctx, VOCERR(28),
                         "You can't use multiple objects with this command.");
                    err = -1;
                    goto exit_error;
                }

                /* the other object is now known for iboj disambiguation */
                otherobj = dolist1[0].vocolobj;
            }

            /* disambiguate the indirect object list */
            if (vocdisambig(ctx, iolist1, iolist, PRP_IODEFAULT,
                            PRP_VALIDIO, voctplvi(tpl), cmd, otherobj,
                            actor, verb, prep, cmdbuf, FALSE))
            {
                err = -1;
                goto exit_error;
            }

            /* only one indirect object is allowed */
            if (voclistlen(iolist1) > 1)
            {
                vocerr(ctx, VOCERR(25),
                       "You can't use multiple indirect objects.");
                err = -1;
                goto exit_error;
            }
            otherobj = iobj = iolist1[0].vocolobj;

            /*
             *   disambiguate the direct object list if we haven't
             *   already done so (we might have disambiguated it first due
             *   to the DisambigDobjFirst flag being set in the template)
             */
            if (!(tplflags & VOCTPLFLG_DOBJ_FIRST)
                && vocdisambig(ctx, dolist1, dolist, PRP_DODEFAULT,
                               PRP_VALIDDO, voctplvd(tpl), cmd, otherobj,
                               actor, verb, prep, cmdbuf, FALSE))
            {
                err = -1;
                goto exit_error;
            }
                
            /* re-check for multi-mode */
            if (multi_flags == 0)
                multi_flags = check_for_multi(dolist1);
            
            /* save it/them/him/her, and execute the command */
            exesaveit(ctx, dolist1);
            if ((err = exeloop(ctx, actor, verb, dolist1, &prep, iolist1,
                               multi_flags, tpl, newstyle)) != 0)
                goto exit_error;
        }
        
    exit_error: ;
        
        ERRCATCH(ctx->voccxerr, err)
            if (err == ERR_RUNASKI) prep = errargint(0);
            if (err != ERR_RUNASKD && err != ERR_RUNASKI)
                errrse(ctx->voccxerr);
        ERREND(ctx->voccxerr)

        switch(err)
        {
        case 0:
            break;
            
        case ERR_RUNABRT:
            /* "abort" executed - return the ABORT code */
            VOC_RETVAL(ctx, save_sp, err);
            
        case ERR_RUNEXIT:
            /* 
             *   "exit" executed - terminate the command, but return
             *   success, since we want to process any additional commands 
             */
            VOC_RETVAL(ctx, save_sp, 0);

        case ERR_RUNEXITOBJ:
            /* 
             *   "exitobj" executed - indicate success, since this merely
             *   indicates that the game decided it was done processing an
             *   object early 
             */
            VOC_RETVAL(ctx, save_sp, 0);

        case ERR_RUNASKI:
        case ERR_RUNASKD:
            askflags = err;
            break;
            
        case -2:                   /* special code: continue with main loop */
            continue;

        case -1:                           /* special code: return an error */
        default:
            VOC_RETVAL(ctx, save_sp, 1);
        }
    
        /*
         *   If we got this far, we probably want more information.  The
         *   askflags can tell us what to do from here.  
         */
        if (askflags)
        {
            int old_unknown;
            int exenewpos;
            
            /* 
             *   if we had unknown words, don't ask for more information
             *   at this point; simply give up and report the unknown word 
             */
            if (ctx->voccxunknown != 0)
            {
                VOC_RETVAL(ctx, save_sp, 1);
            }
            
            /* find new template indicated by the additional object */
            foundtpl = voctplfnd(ctx, verb, prep, tpl, &newstyle);
            tplflags = (newstyle ? voctplflg(tpl) : 0);
        
            /* find a default object of the type requested */
            runrst(rcx);
            if (askflags == ERR_RUNASKD) runpnil(rcx);
            runpobj(rcx, prep);
            runpobj(rcx, actor);
            runppr(rcx, verb,
                   (prpnum)(askflags == ERR_RUNASKD
                            ? PRP_DODEFAULT : PRP_IODEFAULT),
                   (askflags == ERR_RUNASKD ? 3 : 2));
            
            /*
             *   If we got a list back from ?oDefault, and we have a new
             *   template for the command, process the list normally with
             *   the object verification routine for this template.  If we
             *   end up with exactly one object, we will assume it is the
             *   object to be used; otherwise, make no assumption and ask
             *   the user for guidance.  
             */
            if (runtostyp(rcx) == DAT_LIST && foundtpl)
            {
                uchar   *l = runpoplst(rcx);
                uint     lstsiz;
                int      objcnt;
                objnum   defobj = 0;
                objnum   o;
                runsdef  val;
                
                /* push list back on stack, to keep it in the heap */
                val.runsv.runsvstr = l;
                val.runstyp = DAT_LIST;
                runrepush(rcx, &val);
                
                /* get list size out of list */
                lstsiz = osrp2(l) - 2;
                l += 2;
                
                for (objcnt = 0 ; lstsiz && objcnt < 2 ; lstadv(&l, &lstsiz))
                {
                    if (*l == DAT_OBJECT)
                    {
                        prpnum verprop;
                        int argc = 1;
                    
                        o = osrp2(l + 1);
                        verprop = (askflags == ERR_RUNASKD ? voctplvd(tpl)
                                                        : voctplvi(tpl));
                
                        if (!objgetap(ctx->voccxmem, o, verprop,
                                      (objnum *)0, FALSE))
                            continue;

                        tiohide(ctx->voccxtio);

                        /*
                         *   In the unlikely event that we have an
                         *   indirect object but no direct object, push
                         *   the iobj.  This can happen when the player
                         *   types a sentence such as "verb prep iobj".  
                         */
                        if (voclistlen(iolist) != 0
                            && askflags == ERR_RUNASKD
                            && !(tplflags & VOCTPLFLG_DOBJ_FIRST))
                        {
                            /* push the indirect object */
                            runpobj(rcx, iolist[0].vocolobj);

                            /* note the second argument */
                            argc = 2;
                        }

                        /*
                         *   If this is a disambigDobjFirst verb, and
                         *   we're validating an indirect object list,
                         *   then we must push the direct object argument
                         *   to the indirect object validation routine.  
                         */
                        if (askflags == ERR_RUNASKI
                            && (tplflags & VOCTPLFLG_DOBJ_FIRST) != 0)
                        {
                            /* push the diret object */
                            runpobj(rcx, dolist[0].vocolobj);

                            /* note the second argument */
                            argc = 2;
                        }

                        /* push the actor and call the verXoVerb routine */
                        runpobj(rcx, actor);
                        runppr(rcx, o, verprop, argc);
                        if (!tioshow(ctx->voccxtio))
                        {
                            ++objcnt;
                            defobj = o;
                        }

                    }
                }
                
                /* no longer need list in heap, so discard it */
                rundisc(rcx);
                
                /* if we found exactly one object, it's the default */
                if (objcnt == 1)
                {
                    if (askflags == ERR_RUNASKD)
                    {
                        dolist[0].vocolobj = defobj;
                        dolist[0].vocolflg = 0;
                        dolist[0].vocolfst = dolist[0].vocollst = 0;
                        dolist[1].vocolobj = MCMONINV;
                        dolist[1].vocolflg = 0;
                        dolist[1].vocolfst = dolist[1].vocollst = 0;
                    }
                    else
                    {
                        iolist[0].vocolobj = defobj;
                        iolist[0].vocolflg = 0;
                        iolist[0].vocolfst = iolist[0].vocollst = 0;
                        iolist[1].vocolobj = MCMONINV;
                        iolist[1].vocolflg = 0;
                        iolist[1].vocolfst = iolist[1].vocollst = 0;
                    }
                    
                    /* tell the user what we're assuming */
                    if (ctx->voccxpdef2 != MCMONINV)
                    {
                        if (askflags == ERR_RUNASKI)
                            runpobj(rcx, prep);
                        else
                            runpnil(rcx);
                        runpobj(rcx, defobj);
                        runpobj(rcx, verb);
                        runpobj(rcx, actor);
                        runfn(rcx, ctx->voccxpdef2, 4);
                    }
                    else if (ctx->voccxpdef != MCMONINV)
                    {
                        if (askflags == ERR_RUNASKI)
                            runpobj(rcx, prep);
                        else
                            runpnil(rcx);
                        runpobj(rcx, defobj);
                        runfn(rcx, ctx->voccxpdef, 2);
                    }
                    else
                    {
                        vocerr_info(ctx, VOCERR(130), "(");
                        if (askflags == ERR_RUNASKI)
                        {
                            runppr(rcx, prep, PRP_SDESC, 0);
                            vocerr_info(ctx, VOCERR(132), " ");
                        }
                        runppr(rcx, defobj, PRP_THEDESC, 0);
                        vocerr_info(ctx, VOCERR(131), ")");
                    }
                    tioflush(ctx->voccxtio);
                    continue;                      /* try the command again */
                }
            }
            else
                rundisc(rcx);

            /* make sure output capturing is off for the prompt */
            tiocapture(ctx->voccxtio, (mcmcxdef *)0, FALSE);
            tioclrcapture(ctx->voccxtio);
            
            /*
             *   If we're asking for an indirect object, and we have a
             *   list of direct objects, and parseAskobjIndirect is
             *   defined, call it.  Otherwise, if there's a
             *   parseAskobjActor routine, call it.  Otherwise, if there's
             *   a parseAskobj routine, use that.  Finally, if none of
             *   those are defined, generate the default phrasing.  
             */
            if (ctx->voccxpask3 != MCMONINV
                && askflags == ERR_RUNASKI
                && voclistlen(dolist) != 0)
            {
                /* call parseAskobjIndirect */
                voc_askobj_indirect(ctx, dolist, actor, verb, prep);
            }
            else if (ctx->voccxpask2 != MCMONINV)
            {
                if (askflags == ERR_RUNASKI)
                    runpobj(ctx->voccxrun, prep);
                runpobj(ctx->voccxrun, verb);
                runpobj(ctx->voccxrun,
                        (objnum)(actor == MCMONINV ? ctx->voccxme : actor));
                runfn(ctx->voccxrun, ctx->voccxpask2,
                      askflags == ERR_RUNASKI ? 3 : 2);
            }
            else if (ctx->voccxpask != MCMONINV)
            {
                if (askflags == ERR_RUNASKI)
                    runpobj(ctx->voccxrun, prep);
                runpobj(ctx->voccxrun, verb);
                runfn(ctx->voccxrun, ctx->voccxpask,
                      askflags == ERR_RUNASKI ? 2 : 1);
            }
            else
            {
                /*
                 *   Phrase the question: askDo: "What do you want
                 *   <actor> to <verb>?"  askIo: "What do you want <actor>
                 *   to <verb> it <prep>?"  If the actor is Me, leave the
                 *   actor out of it.  
                 */
                if (actor != MCMONINV && actor != ctx->voccxme)
                {
                    vocerr_info(ctx, VOCERR(148), "What do you want ");
                    runppr(rcx, actor, PRP_THEDESC, 0);
                    vocerr_info(ctx, VOCERR(149), " to ");
                }
                else
                {
                    /* no actor - don't mention one */
                    vocerr_info(ctx, VOCERR(140), "What do you want to ");
                }

                /* add the verb */
                runppr(rcx, verb, PRP_SDESC, 0);

                /*
                 *   add an appropriate pronoun for the direct object,
                 *   and the preposition, if we're asking for an indirect
                 *   object 
                 */
                if (askflags == ERR_RUNASKI)
                {
                    int   i;
                    int   vcnt;
                    int   distinct;
                    const char *lastfst;

                    /*
                     *   If possible, tailor the pronoun to the situation
                     *   rather than using "it"; if we have multiple
                     *   objects, use "them", and if we have agreement
                     *   with the possible single objects about "him" or
                     *   "her", use that.  Otherwise, use "it".  If "all"
                     *   was specified for any word, automatically assume
                     *   multiple distinct objects were specified.  
                     */
                    vcnt = voclistlen(dolist);
                    for (distinct = 0, i = 0, lastfst = 0 ; i < vcnt ; ++i)
                    {
                        /* if the first word is different here, note it */
                        if (lastfst != dolist[i].vocolfst)
                        {
                            /* this is a different word - count it */
                            ++distinct;
                            lastfst = dolist[i].vocolfst;
                        }

                        /* always assume multiple distinct objects on "all" */
                        if (dolist[i].vocolflg & VOCS_ALL)
                        {
                            distinct = 2;
                            break;
                        }
                    }

                    /*
                     *   If we have multiple words, use "them";
                     *   otherwise, see if we can find agreement about
                     *   using "him" or "her". 
                     */
                    if (distinct > 1)
                    {
                        /* multiple words specified by user - use "them" */
                        vocerr_info(ctx, VOCERR(144), " them ");
                    }
                    else
                    {
                        int is_him = 0;
                        int is_her = 0;
                        int is_them = 0;

                        /* run through the objects and check him/her */
                        for (i = 0 ; i < vcnt ; ++i)
                        {
                            int him1, her1, them1;

                            /* if it's special (number, string), use "it" */
                            if (dolist[i].vocolobj == MCMONINV)
                            {
                                him1 = FALSE;
                                her1 = FALSE;
                                them1 = FALSE;
                            }
                            else
                            {
                                /* check for "him" */
                                runppr(rcx, dolist[i].vocolobj, PRP_ISHIM, 0);
                                him1 = (runtostyp(rcx) == DAT_TRUE);
                                rundisc(rcx);
                                
                                /* check for "her" */
                                runppr(rcx, dolist[i].vocolobj, PRP_ISHER, 0);
                                her1 = (runtostyp(rcx) == DAT_TRUE);
                                rundisc(rcx);

                                /* check for "them" */
                                runppr(rcx, dolist[i].vocolobj,
                                       PRP_ISTHEM, 0);
                                them1 = (runtostyp(rcx) == DAT_TRUE);
                                rundisc(rcx);
                            }

                            /*
                             *   if this is the first object, it
                             *   definitely agrees; otherwise, keep going
                             *   only if it agrees with what we found on
                             *   the last pass 
                             */
                            if (i == 0)
                            {
                                is_him = him1;
                                is_her = her1;
                                is_them = them1;
                            }
                            else
                            {
                                /* turn off either that is no longer true */
                                if (!him1) is_him = FALSE;
                                if (!her1) is_her = FALSE;
                                if (!them1) is_them = FALSE;
                            }

                            /* if all are false, stop now */
                            if (!is_him && !is_her && !is_them)
                                break;
                        }

                        /*
                         *   If we could agree on "him", "her", or "them",
                         *   use that pronoun; otherwise, use "it".  If we
                         *   found both "him" and "her" are acceptable for
                         *   all objects, use "them".  
                         */
                        if ((is_him && is_her) || is_them)
                            vocerr_info(ctx, VOCERR(147), " them ");
                        else if (is_him)
                            vocerr_info(ctx, VOCERR(145), " him ");
                        else if (is_her)
                            vocerr_info(ctx, VOCERR(146), " her ");
                        else
                            vocerr_info(ctx, VOCERR(141), " it ");
                    }

                    /* finish off the question with the prep and a "?" */
                    if (prep != MCMONINV)
                        runppr(rcx, prep, PRP_SDESC, 0);
                    else
                        vocerr_info(ctx, VOCERR(142), "to");
                }
                vocerr_info(ctx, VOCERR(143), "?");
            }
            tioflush(ctx->voccxtio);
                
            /*
             *   Get a new command line.  If the player gives us
             *   something that looks like a noun list, and nothing more,
             *   he anwered our question; otherwise, he's typing a new
             *   command, so we must return to the caller with the reparse
             *   flag set.  
             */
            if (askflags == ERR_RUNASKD)
            {
                exenewbuf = donewbuf;
                exenewcmd = donewcmd;
                exenewlist = donewlist;
                exenewtype = donewtype;
            }
            else
            {
                exenewbuf = ionewbuf;
                exenewcmd = ionewcmd;
                exenewlist = ionewlist;
                exenewtype = ionewtype;
            }

            /* read the new command */
            if (vocread(ctx, actor, verb, exenewcmd, VOCBUFSIZ,
                        askflags == ERR_RUNASKD ? 3 : 4) == VOCREAD_REDO)
            {
                /* 
                 *   we got an input line, but we want to treat it as a brand
                 *   new command line - copy the new text to the command
                 *   buffer, set the 'redo' flag, and give up 
                 */
                strcpy(cmdbuf, exenewcmd);
                ctx->voccxredo = TRUE;
                VOC_RETVAL(ctx, save_sp, 1);
            }
            
            if (!(cnt = voctok(ctx, exenewcmd, exenewbuf, exenewlist,
                               TRUE, FALSE, TRUE)))
            {
                runrst(rcx);
                runfn(rcx, ctx->voccxprd, 0);
                VOC_RETVAL(ctx, save_sp, 1);
            }
            if (cnt < 0)
            {
                ctx->voccxunknown = 0;
                VOC_RETVAL(ctx, save_sp, 1);
            }

            /* 
             *   Save the unknown word count while getting types, and set
             *   the count to a non-zero value - this will force the type
             *   checker to generate an error on an unknown word.  This
             *   removes a little control from the game (since
             *   parseUnknownXobj won't be called), but there's not much
             *   else we can do here. 
             */
            old_unknown = ctx->voccxunknown;
            ctx->voccxunknown = 1;

            /* get the types */
            exenewlist[cnt] = 0;
            if (vocgtyp(ctx, exenewlist, exenewtype, cmdbuf))
            {
                /* 
                 *   clear the unknown word count so that we fail with
                 *   this error rather than trying to deal with unknown
                 *   words 
                 */
                ctx->voccxunknown = 0;

                /* return failure */
                VOC_RETVAL(ctx, save_sp, 1);
            }

            /* restore the unknown word count */
            ctx->voccxunknown = old_unknown;

            /* start at the first word */
            exenewpos = 0;

            /* 
             *   if we're asking for an indirect object, and the first
             *   word is a preposition, and matches the preposition that
             *   we supplied to precede the indirect object, skip the
             *   preposition 
             */
            if (askflags == ERR_RUNASKI
                && prep != MCMONINV
                && (exenewtype[0] & VOCT_PREP) != 0)
            {
                vocwdef *vp;
                
                /* get the preposition */
                vp = vocffw(ctx, exenewlist[0], (int)strlen(exenewlist[0]),
                            (char *)0, 0, PRP_PREP, (vocseadef *)0);
                if (vp != 0 && vp->vocwobj == prep)
                    ++exenewpos;
            }

            /* check for a noun */
            newnoun = (askflags == ERR_RUNASKD ? dolist : iolist);
            cnt = vocchknoun(ctx, exenewlist, exenewtype, exenewpos, &next,
                             newnoun, FALSE);
            
            if (cnt < 0) { VOC_RETVAL(ctx, save_sp, 1); } /* invalid syntax */
            if (cnt == 0
                || (exenewlist[next] && !vocspec(exenewlist[next], VOCW_THEN)
                    && *exenewlist[next] != '\0'))
            {
                strcpy(cmdbuf, exenewcmd);
                ctx->voccxredo = TRUE;
                VOC_RETVAL(ctx, save_sp, 1);
            }

            /* re-check the 'multi' flags */
            multi_flags = check_for_multi(newnoun);
            
            /* give it another go by going back to the top of the loop */
        }
        else
        {
            /* normal exit flags - return success */
            VOC_RETVAL(ctx, save_sp, 0);
        }
    }
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
