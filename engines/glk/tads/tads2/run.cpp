/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/data.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/list.h"
#include "glk/tads/tads2/os.h"
#include "glk/tads/tads2/post_compilation.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* forward declarations */
struct bifcxdef;

/*
*   Create a new object
*/
static void run_new(runcxdef *ctx, uchar *noreg *codepp,
	objnum callobj, prpnum callprop)
{
	objnum   sc = 0;
	objnum   objn;
	objdef  *objp;
	int      sccnt;
	vocidef *voci;

	/* get the superclass (nil means no superclass) */
	if (runtostyp(ctx) == DAT_NIL)
		sccnt = 0;
	else
	{
		/* get the superclass */
		sc = runpopobj(ctx);
		sccnt = 1;

		/* make sure it's not a dynamically-allocated object */
		voci = vocinh(ctx->runcxvoc, sc);
		if (voci->vociflg & VOCIFNEW)
			runsig(ctx, ERR_BADNEWSC);
	}

	/* create a new object and set its superclass */
	objp = objnew(ctx->runcxmem, sccnt, 64, &objn, FALSE);
	if (sccnt) oswp2(objsc(objp), sc);

	/* save undo for the object creation */
	vocdusave_newobj(ctx->runcxvoc, objn);

	/* touch and unlock the object */
	mcmtch(ctx->runcxmem, (mcmon)objn);
	mcmunlck(ctx->runcxmem, (mcmon)objn);

	/* add a vocabulary inheritance record for the new object */
	vociadd(ctx->runcxvoc, objn, MCMONINV, sccnt, &sc, VOCIFNEW | VOCIFVOC);

	/* set up its vocabulary, inheriting from the class */
	if (sccnt)
		supivoc1((struct supcxdef *)0, ctx->runcxvoc,
			vocinh(ctx->runcxvoc, objn), objn, TRUE, VOCFNEW);

	/* run the constructor */
	runpprop(ctx, codepp, callobj, callprop, objn, PRP_CONSTRUCT,
		FALSE, 0, objn);
#ifdef NEVER
	/*
	*   add it to its location's contents list by calling
	*   newobj.moveInto(newobj.location)
	*/
	runppr(ctx, objn, PRP_LOCATION, 0);
	if (runtostyp(ctx) == DAT_OBJECT)
		runppr(ctx, objn, PRP_MOVEINTO, 1);
	else
		rundisc(ctx);
#endif

	/* return the new object */
	runpobj(ctx, objn);
}

/*
*   Delete an object
*/
static void run_delete(runcxdef *ctx, uchar *noreg *codepp,
	objnum callobj, prpnum callprop)
{
	objnum    objn;
	vocidef  *voci;
	int       i;
	voccxdef *vctx = ctx->runcxvoc;

	/* get the object to be deleted */
	objn = runpopobj(ctx);

	/* make sure it was allocated with "new" */
	voci = vocinh(vctx, objn);
	if (voci == 0 || !(voci->vociflg & VOCIFNEW))
		runsig(ctx, ERR_BADDEL);

	/* run the destructor */
	runpprop(ctx, codepp, callobj, callprop, objn, PRP_DESTRUCT,
		FALSE, 0, objn);
#ifdef NEVER
	/* remove it from its location, if any, by using moveInto(nil) */
	runpnil(ctx);
	runppr(ctx, objn, PRP_MOVEINTO, 1);
#endif

	/* save undo for the object deletion */
	vocdusave_delobj(vctx, objn);

	/* delete the object's inheritance and vocabulary records */
	vocdel(vctx, objn);
	vocidel(vctx, objn);

	/* forget 'it' if the deleted object is 'it' (or 'them', etc) */
	if (vctx->voccxit == objn) vctx->voccxit = MCMONINV;
	if (vctx->voccxhim == objn) vctx->voccxhim = MCMONINV;
	if (vctx->voccxher == objn) vctx->voccxher = MCMONINV;
	for (i = 0; i < vctx->voccxthc; ++i)
	{
		if (vctx->voccxthm[i] == objn)
		{
			/* forget the entire 'them' list when deleting from it */
			vctx->voccxthc = 0;
			break;
		}
	}

	/* forget the 'again' statistics if necessary */
	if (vctx->voccxlsd.vocolobj == objn
		|| vctx->voccxlsi.vocolobj == objn
		|| vctx->voccxlsa == objn
		|| vctx->voccxlsv == objn
		|| vctx->voccxlsp == objn)
	{
		/* forget the verb */
		vctx->voccxlsv = MCMONINV;

		/*
		*   note in the flags why we lost the "again" verb, for better
		*   error reporting if the player tries to type "again"
		*/
		vctx->voccxflg |= VOCCXAGAINDEL;
	}

	/* delete the memory manager object */
	mcmfre(ctx->runcxmem, (mcmon)objn);
}


/*
*   invoke a function
*/
void runfn(runcxdef *ctx, noreg objnum  objn, int argc)
{
	uchar *fn;
	int    err;

	NOREG((&objn))

		/* get a lock on the object */
		fn = mcmlck(ctx->runcxmem, objn);

	/* catch any errors, so we can unlock the object */
	ERRBEGIN(ctx->runcxerr)

		/* execute the object */
		runexe(ctx, fn, MCMONINV, objn, (prpnum)0, argc);

	/* in case of error, unlock the object and resignal the error */
	ERRCATCH(ctx->runcxerr, err)
		mcmunlck(ctx->runcxmem, objn);    /* release the lock on the object */
	if (err < ERR_RUNEXIT || err > ERR_RUNEXITOBJ)
		dbgdump(ctx->runcxdbg);                       /* dump the stack */
	errrse(ctx->runcxerr);
	ERREND(ctx->runcxerr)

		/* we're done with the object, so unlock it */
		mcmunlck(ctx->runcxmem, objn);
}

/*
*   compress the heap - remove unreferenced items
*/
void runhcmp(runcxdef *ctx, uint siz, uint below,
	runsdef *val1, runsdef *val2, runsdef *val3)
{
	uchar   *hp = ctx->runcxheap;
	uchar   *htop = ctx->runcxhp;
	runsdef *stop = ctx->runcxsp + below;
	runsdef *stk = ctx->runcxstk;
	runsdef *sp;
	uchar   *dst = hp;
	uchar   *hnxt;
	int      ref;

	/* go through heap, finding references on stack */
	for (; hp < htop; hp = hnxt)
	{
		hnxt = hp + osrp2(hp);                /* remember next heap element */

		for (ref = FALSE, sp = stk; sp < stop; ++sp)
		{
			switch (sp->runstyp)
			{
			case DAT_SSTRING:
			case DAT_LIST:
				if (sp->runsv.runsvstr == hp)    /* reference to this item? */
				{
					ref = TRUE;             /* this heap item is referenced */
					sp->runsv.runsvstr = dst;      /* reflect imminent move */
				}
				break;

			default:                /* other types do not refer to the heap */
				break;
			}
		}

		/* check the explicitly referenced value pointers as well */
#define CHECK_VAL(val) \
        if (val && val->runsv.runsvstr == hp) \
            ref = TRUE, val->runsv.runsvstr = dst;
		CHECK_VAL(val1);
		CHECK_VAL(val2);
		CHECK_VAL(val3);
#undef CHECK_VAL

		/* if referenced, copy it to dst and advance dst */
		if (ref)
		{
			if (hp != dst) memmove(dst, hp, (size_t)osrp2(hp));
			dst += osrp2(dst);
		}
	}

	/* set heap pointer based on shuffled heap */
	ctx->runcxhp = dst;

	/* check for space requested, and signal error if not available */
	if ((uint)(ctx->runcxhtop - ctx->runcxhp) < siz)
		runsig(ctx, ERR_HPOVF);
}

/*
*   push a value onto the stack that's already been allocated in heap
*/
void runrepush(runcxdef *ctx, runsdef *val)
{
	/* check for stack overflow */
	runstkovf(ctx);

	OSCPYSTRUCT(*(ctx->runcxsp), *val);

	/* increment stack pointer */
	++(ctx->runcxsp);
}

/* push a counted-length string onto the stack */
void runpstr(runcxdef *ctx, const char *str, int len, int sav)
{
	runsdef val;

	/* allocate space and set up new string */
	runhres(ctx, len + 2, sav);
	oswp2(ctx->runcxhp, len + 2);
	memcpy(ctx->runcxhp + 2, str, (size_t)len);

	/* push return value */
	val.runsv.runsvstr = ctx->runcxhp;
	val.runstyp = DAT_SSTRING;
	ctx->runcxhp += len + 2;
	runrepush(ctx, &val);
}

/* push a C-style string, converting escape codes */
void runpushcstr(runcxdef *ctx, const char *str, size_t len, int sav)
{
	const char *p;
	char    *dst;
	size_t   need;
	runsdef  val;

	/* determine how much space we'll need after converting escapes */
	for (p = str, need = len; p < str + len; ++p)
	{
		switch (*p)
		{
		case '\\':
		case '\n':
		case '\r':
		case '\t':
			/* these characters need to be escaped */
			++need;
			break;

		default:
			break;
		}
	}

	/* reserve space */
	runhres(ctx, need + 2, sav);

	/* set up the length prefix */
	oswp2(ctx->runcxhp, need + 2);

	/* copy the string, expanding escapes */
	for (p = str, dst = (char *)ctx->runcxhp + 2; p < str + len; ++p)
	{
		switch (*p)
		{
		case '\\':
			*dst++ = '\\';
			*dst++ = '\\';
			break;

		case '\n':
		case '\r':
			*dst++ = '\\';
			*dst++ = 'n';
			break;

		case '\t':
			*dst++ = '\\';
			*dst++ = '\t';
			break;

		default:
			*dst++ = *p;
			break;
		}
	}

	/* push the return value */
	val.runsv.runsvstr = ctx->runcxhp;
	val.runstyp = DAT_SSTRING;
	ctx->runcxhp += need + 2;
	runrepush(ctx, &val);
}

/* push a value onto the stack */
void runpush(runcxdef *ctx, dattyp typ, runsdef *val)
{
	int len;

	/* check for stack overflow */
	runstkovf(ctx);

	OSCPYSTRUCT(*(ctx->runcxsp), *val);
	ctx->runcxsp->runstyp = typ;

	/* variable-length data must be copied into the heap */
	if (typ == DAT_SSTRING || typ == DAT_LIST)
	{
		len = osrp2(val->runsv.runsvstr);
		runhres(ctx, len, 0);                      /* reserve space in heap */
		memcpy(ctx->runcxhp, val->runsv.runsvstr, (size_t)len);
		ctx->runcxsp->runsv.runsvstr = ctx->runcxhp;
		ctx->runcxhp += len;
	}

	/* increment stack pointer */
	++(ctx->runcxsp);
}

/* push a number onto the stack */
void runpnum(runcxdef *ctx, long num)
{
	runsdef val;

	val.runsv.runsvnum = num;
	runpush(ctx, DAT_NUMBER, &val);
}

/* push an object onto the stack (or nil if obj is MCMONINV) */
void runpobj(runcxdef *ctx, objnum obj)
{
	runsdef val;

	if (obj == MCMONINV)
		runpnil(ctx);
	else
	{
		val.runsv.runsvobj = obj;
		runpush(ctx, DAT_OBJECT, &val);
	}
}

/* push nil */
void runpnil(runcxdef *ctx)
{
	runsdef val;
	runpush(ctx, DAT_NIL, &val);
}

/* copy datatype + value from a runsdef into a buffer (such as list) */
static void runputbuf(uchar *dstp, runsdef *val)
{
	*dstp++ = val->runstyp;
	switch (val->runstyp)
	{
	case DAT_LIST:
	case DAT_SSTRING:
		memcpy(dstp, val->runsv.runsvstr, (size_t)osrp2(val->runsv.runsvstr));
		break;

	case DAT_NUMBER:
		oswp4s(dstp, val->runsv.runsvnum);
		break;

	case DAT_PROPNUM:
		oswp2(dstp, val->runsv.runsvprp);
		break;

	case DAT_OBJECT:
	case DAT_FNADDR:
		oswp2(dstp, val->runsv.runsvobj);
		break;
	}
}

/* push a value from a buffer (list, property, etc) onto stack */
void runpbuf(runcxdef *ctx, int typ, void *valp)
{
	runsdef val;

	switch (typ)
	{
	case DAT_NUMBER:
		val.runsv.runsvnum = osrp4s(valp);
		break;

	case DAT_OBJECT:
	case DAT_FNADDR:
		val.runsv.runsvobj = osrp2(valp);
		break;

	case DAT_PROPNUM:
		val.runsv.runsvprp = osrp2(valp);
		break;

	case DAT_SSTRING:
	case DAT_LIST:
		val.runsv.runsvstr = (uchar *)valp;
		break;

	case DAT_NIL:
	case DAT_TRUE:
		break;
	}
	runpush(ctx, typ, &val);
}

/* compare items at top of stack for equality; TRUE->equal, FALSE->unequal */
int runeq(runcxdef *ctx)
{
	runsdef val1, val2;

	/* get values, and see if they have identical type; not equal if not */
	runpop(ctx, &val1);
	runpop(ctx, &val2);
	if (val1.runstyp != val2.runstyp) return(FALSE);

	/* types match, so check values */
	switch (val1.runstyp)
	{
	case DAT_NUMBER:
		return(val1.runsv.runsvnum == val2.runsv.runsvnum);

	case DAT_SSTRING:
	case DAT_LIST:
		return(osrp2(val1.runsv.runsvstr) == osrp2(val2.runsv.runsvstr)
			&& !memcmp(val1.runsv.runsvstr, val2.runsv.runsvstr,
			(size_t)osrp2(val1.runsv.runsvstr)));

	case DAT_PROPNUM:
		return(val1.runsv.runsvprp == val2.runsv.runsvprp);

	case DAT_OBJECT:
	case DAT_FNADDR:
		return(val1.runsv.runsvobj == val2.runsv.runsvobj);

	default:
		return(TRUE);
	}
}

/* compare magnitudes of numbers/strings at top of stack; strcmp-like value */
int runmcmp(runcxdef *ctx)
{
	if (runtostyp(ctx) == DAT_NUMBER)
	{
		long num2 = runpopnum(ctx);
		long num1 = runpopnum(ctx);

		if (num1 > num2) return(1);
		else if (num1 < num2) return(-1);
		else return(0);
	}
	else if (runtostyp(ctx) == DAT_SSTRING)
	{
		uchar *str2 = runpopstr(ctx);
		uchar *str1 = runpopstr(ctx);
		uint   len1 = osrp2(str1) - 2;
		uint   len2 = osrp2(str2) - 2;

		str1 += 2;
		str2 += 2;
		while (len1 && len2)
		{
			if (*str1 < *str2) return(-1);   /* character from 1 is greater */
			else if (*str1 > *str2) return(1);       /* char from 1 is less */

			++str1;
			++str2;
			--len1;
			--len2;
		}
		if (len1) return(1);    /* match up to len2, but string 1 is longer */
		else if (len2) return(-1);  /* match up to len1, but str2 is longer */
		else return(0);                            /* strings are identical */
	}
	else
	{
		runsig(ctx, ERR_INVCMP);
	}
	return 0;
}

/* determine size of a runsdef item */
int runsiz(runsdef *item) {
	switch (item->runstyp) {
	case DAT_NUMBER:
		return(4);
	case DAT_SSTRING:
	case DAT_LIST:
		return(osrp2(item->runsv.runsvstr));
	case DAT_PROPNUM:
	case DAT_OBJECT:
	case DAT_FNADDR:
		return(2);
	default:
		return(0);
	}
}

/* find a sublist within a list */
uchar *runfind(uchar *lst, runsdef *item)
{
	uint len;
	uint curlen;

	for (len = osrp2(lst) - 2, lst += 2; len; lst += curlen, len -= curlen)
	{
		if (*lst == item->runstyp)
		{
			switch (*lst)
			{
			case DAT_LIST:
			case DAT_SSTRING:
				if (osrp2(lst + 1) == osrp2(item->runsv.runsvstr) &&
					!memcmp(lst + 1, item->runsv.runsvstr, (size_t)osrp2(lst + 1)))
					return(lst);
				break;
			case DAT_NUMBER:
				if (osrp4s(lst + 1) == item->runsv.runsvnum)
					return(lst);
				break;

			case DAT_TRUE:
			case DAT_NIL:
				return(lst);

			case DAT_OBJECT:
			case DAT_FNADDR:
				if (osrp2(lst + 1) == item->runsv.runsvobj)
					return(lst);
				break;

			case DAT_PROPNUM:
				if (osrp2(lst + 1) == item->runsv.runsvprp)
					return(lst);
				break;
			}
		}
		curlen = datsiz(*lst, lst + 1) + 1;
	}
	return((uchar *)0);
}

/* add values */
void runadd(runcxdef *ctx, runsdef *val, runsdef *val2, uint below)
{
	if (val->runstyp == DAT_LIST)
	{
		int     len1 = osrp2(val->runsv.runsvstr);
		int     len2 = runsiz(val2);
		int     newlen;

		/* if concatenating a list, take out length + datatype from 2nd */
		if (val2->runstyp == DAT_LIST)
			newlen = len1 + len2 - 2;          /* leave out second list len */
		else
			newlen = len1 + len2 + 1;             /* add in datatype header */

												  /* get space in heap, copy first list, and set new length */
		runhres2(ctx, newlen, below, val, val2);
		memcpy(ctx->runcxhp, val->runsv.runsvstr, (size_t)len1);
		oswp2(ctx->runcxhp, newlen);

		/* append the new element or list of elements */
		if (val2->runstyp == DAT_LIST)
			memcpy(ctx->runcxhp + len1, val2->runsv.runsvstr + 2,
			(size_t)(len2 - 2));
		else
			runputbuf(ctx->runcxhp + len1, val2);

		/* set up return value and update heap pointer */
		val->runsv.runsvstr = ctx->runcxhp;
		ctx->runcxhp += newlen;
	}
	else if (val->runstyp == DAT_SSTRING && val2->runstyp == DAT_SSTRING)
	{
		int len1 = osrp2(val->runsv.runsvstr);
		int len2 = osrp2(val2->runsv.runsvstr);

		/* reserve space, and concatenate the two strings */
		runhres2(ctx, len1 + len2 - 2, below, val, val2);
		memcpy(ctx->runcxhp, val->runsv.runsvstr, (size_t)len1);
		memcpy(ctx->runcxhp + len1, val2->runsv.runsvstr + 2,
			(size_t)len2 - 2);

		/* set length to sum of two lengths, minus 2nd length word */
		oswp2(ctx->runcxhp, len1 + len2 - 2);
		val->runsv.runsvstr = ctx->runcxhp;
		ctx->runcxhp += len1 + len2 - 2;
	}
	else if (val->runstyp == DAT_NUMBER && val2->runstyp == DAT_NUMBER)
		val->runsv.runsvnum += val2->runsv.runsvnum;
	else
		runsig(ctx, ERR_INVADD);
}

/* returns TRUE if value changed */
int runsub(runcxdef *ctx, runsdef *val, runsdef *val2, uint below)
{
	if (val->runstyp == DAT_LIST)
	{
		uchar *sublist;
		int    subsize;
		int    listsize;
		int    part1sz;

		if (val2->runstyp == DAT_LIST)
		{
			uchar *p1;
			uchar *p2;
			uint   rem1;
			uint   rem2;
			uchar *dst;

			/* reserve space for another copy of first list */
			listsize = runsiz(val);
			runhres2(ctx, listsize, below, val, val2);
			dst = ctx->runcxhp + 2;

			/* get pointer to first list */
			p1 = val->runsv.runsvstr;
			rem1 = osrp2(p1) - 2;
			p1 += 2;

			/*
			*   loop through left list, copying elements to output if
			*   not in the right list
			*/
			for (; rem1; lstadv(&p1, &rem1))
			{
				int found = FALSE;

				/* find current element of first list in second list */
				p2 = val2->runsv.runsvstr;
				rem2 = osrp2(p2) - 2;
				p2 += 2;
				for (; rem2; lstadv(&p2, &rem2))
				{
					if (*p1 == *p2)
					{
						int siz1 = datsiz(*p1, p1 + 1);
						int siz2 = datsiz(*p2, p2 + 1);

						if (siz1 == siz2 &&
							(siz1 == 0 || !memcmp(p1 + 1, p2 + 1, (size_t)siz1)))
						{
							found = TRUE;
							break;
						}
					}
				}

				/* if this element wasn't found, copy to output list */
				if (!found)
				{
					uint siz;

					*dst++ = *p1;
					if ((siz = datsiz(*p1, p1 + 1)) != 0)
					{
						memcpy(dst, p1 + 1, siz);
						dst += siz;
					}
				}
			}

			/* we've built the list; write size and we're done */
			oswp2(ctx->runcxhp, dst - ctx->runcxhp);
			val->runsv.runsvstr = ctx->runcxhp;
			ctx->runcxhp = dst;
		}
		else if ((sublist = runfind(val->runsv.runsvstr, val2)) != 0)
		{
			subsize = datsiz(*sublist, sublist + 1) + 1;
			listsize = runsiz(val);
			part1sz = sublist - (uchar *)val->runsv.runsvstr;

			runhres2(ctx, listsize - subsize, below, val, val2);
			memcpy(ctx->runcxhp, val->runsv.runsvstr, (size_t)part1sz);
			memcpy(ctx->runcxhp + part1sz, sublist + subsize,
				(size_t)(listsize - subsize - part1sz));
			oswp2(ctx->runcxhp, listsize - subsize);
			val->runsv.runsvstr = ctx->runcxhp;
			ctx->runcxhp += listsize - subsize;
		}
		else
		{
			return(FALSE);            /* no change - value can be re-pushed */
		}
	}
	else if (val->runstyp == DAT_NUMBER && val2->runstyp == DAT_NUMBER)
		val->runsv.runsvnum -= val2->runsv.runsvnum;
	else
		runsig(ctx, ERR_INVSUB);

	return(TRUE);                 /* value has changed; must be pushed anew */
}

/* return code pointer offset */
static uint runcpsav(runcxdef *ctx, uchar *noreg *cp, objnum obj, prpnum prop)
{
	uint ofs;

	VARUSED(prop);

	/* get offset from start of object */
	ofs = *cp - mcmobjptr(ctx->runcxmem, (mcmon)obj);

	/* clear the pointer so the caller knows the object is unlocked */
	*cp = 0;

	/* unlock the object, and return the derived offset */
	mcmunlck(ctx->runcxmem, (mcmon)obj);
	return(ofs);
}

/* restore code pointer based on object.property */
uchar *runcprst(runcxdef *ctx, uint ofs, objnum obj, prpnum prop)
{
	uchar *ptr;

	VARUSED(prop);

	/* lock object, and get pointer based on offset */
	ptr = mcmlck(ctx->runcxmem, (mcmon)obj) + ofs;

	return(ptr);
}

/* get offset of an element within a list */
static uint runindofs(runcxdef *ctx, uint indx, uchar *lstp)
{
	uint   lstsiz;
	uchar *orgp = lstp;

	/* verify that index is in range */
	if (indx <= 0) runsig(ctx, ERR_LOWINX);

	/* get list's size, and point to its data string */
	lstsiz = osrp2(lstp) - 2;
	lstp += 2;

	/* skip the first indx-1 elements */
	for (--indx; indx && lstsiz; --indx) lstadv(&lstp, &lstsiz);

	/* if we ran out of list, the index is out of range */
	if (!lstsiz) runsig(ctx, ERR_HIGHINX);

	/* return the offset */
	return((uint)(lstp - orgp));
}

/* push an indexed element of a list; index is tos, list is next on stack */
static void runpind(runcxdef *ctx, uint indx, uchar *lstp)
{
	uchar   *ele;
	runsdef  val;

	/* find the element we want to push */
	ele = lstp + runindofs(ctx, indx, lstp);

	/* reserve space first, in case lstp gets moved around */
	val.runstyp = DAT_LIST;
	val.runsv.runsvstr = lstp;
	runhres1(ctx, datsiz(*ele, ele + 1), 0, &val);
	if (val.runsv.runsvstr != lstp)
		ele = val.runsv.runsvstr + runindofs(ctx, indx, val.runsv.runsvstr);

	/* push the operand */
	runpbuf(ctx, *ele, ele + 1);
}

/*
*   Check a property to ensure that it's a data property.  Throws an
*   error if the property contains a method.  This is used for debugger
*   speculative evaluation to ensure that we don't call any methods from
*   within speculative expressions.
*/
static void runcheckpropdata(runcxdef *ctx, objnum obj, prpnum prop)
{
	uint    pofs;
	objnum  target;
	objdef *objptr;
	prpdef *prpptr;
	int     typ;

	/* if the object is invalid, it's an error */
	if (obj == MCMONINV)
		errsig(ctx->runcxerr, ERR_REQVOB);

	/* get the property */
	pofs = objgetap(ctx->runcxmem, obj, prop, &target, FALSE);

	/* if there's no property, it's okay - it will just return nil */
	if (pofs == 0)
		return;

	/* get the object */
	objptr = mcmlck(ctx->runcxmem, target);

	/* get the property */
	prpptr = (prpdef *)(((uchar *)objptr) + pofs);
	typ = prptype(prpptr);

	/* we're done with the object's memory now */
	mcmunlck(ctx->runcxmem, target);

	/* check the type */
	switch (typ)
	{
	case DAT_CODE:
	case DAT_DSTRING:
		/*
		*   we can't call code or evaluate (i.e., print) double-quoted
		*   strings during speculative evaluation
		*/
		errsig(ctx->runcxerr, ERR_RTBADSPECEXPR);

	default:
		/* other types do not involve method calls, so they're okay */
		break;
	}
}

/* push an object's property */
void runpprop(runcxdef *ctx, uchar *noreg *codepp,
	objnum callobj, prpnum callprop,
	noreg objnum obj, prpnum prop, int inh, int argc, objnum self)
{
	uint     pofs;
	uint     saveofs = 0;
	objdef  *objptr;
	prpdef  *prpptr;
	uchar   *val;
	int      typ;
	runsdef  sval;
	objnum   target;
	int      times_through = 0;
	int      err;
	objnum   otherobj = 0;

	NOREG((&obj, &codepp));

	if (obj == MCMONINV) runsig(ctx, ERR_RUNNOBJ);

startover:
	pofs = objgetap(ctx->runcxmem, obj, prop, &target, inh);

	/* if nothing was found, push nil */
	if (!pofs)
	{
		runpush(ctx, DAT_NIL, &sval);
		return;
	}

	/* found a property; get the prpdef, and the value and type of data */
	objptr = mcmlck(ctx->runcxmem, target);
	ERRBEGIN(ctx->runcxerr)         /* catch errors so we can unlock object */

		prpptr = (prpdef *)(((uchar *)objptr) + pofs);
	val = prpvalp(prpptr);
	typ = prptype(prpptr);

	/* determine what to do based on property type */
	switch (typ)
	{
	case DAT_CODE:
		/* save caller's code offset - caller's object may move */
		if (codepp)
			saveofs = runcpsav(ctx, codepp, callobj, callprop);

		/* execute the code */
		runexe(ctx, val, self, target, prop, argc);

		/* restore caller's code pointer in case object moved */
		if (codepp)
			*codepp = runcprst(ctx, saveofs, callobj, callprop);
		break;

	case DAT_REDIR:
		otherobj = osrp2(val);
		break;

	case DAT_DSTRING:
		outfmt(ctx->runcxtio, val);
		break;

	case DAT_DEMAND:
		break;

	default:
		runpbuf(ctx, typ, val);
		break;
	}

	/* we're done - unlock the object */
	mcmunlck(ctx->runcxmem, target);

	/* if it's redirected, redirect it now */
	if (typ == DAT_REDIR)
	{
		runpprop(ctx, codepp, callobj, callprop, otherobj, prop,
			FALSE, argc, otherobj);
	}

	/* if an error occurs, unlock the object, and resignal the error */
	ERRCATCH(ctx->runcxerr, err)
		mcmunlck(ctx->runcxmem, target);
	if (err < ERR_RUNEXIT || err > ERR_RUNEXITOBJ)
		dbgdump(ctx->runcxdbg);                       /* dump the stack */
	errrse(ctx->runcxerr);
	ERREND(ctx->runcxerr)

		/* apply special handling for set-on-first-use data */
		if (typ == DAT_DEMAND)
		{
			/*
			*   if we've already done this, the property isn't being set by
			*   the callback, so we'll never get out of this loop - abort if
			*   so
			*/
			if (++times_through != 1)
				runsig(ctx, ERR_DMDLOOP);

			/* save caller's code offset - caller's object may move */
			if (codepp)
				saveofs = runcpsav(ctx, codepp, callobj, callprop);

			/* invoke the callback to set the property on demand */
			(*ctx->runcxdmd)(ctx->runcxdmc, obj, prop);

			/* restore caller's code pointer */
			if (codepp)
				*codepp = runcprst(ctx, saveofs, callobj, callprop);

			/* try again now that it's been set up */
			goto startover;
		}
}

/* ======================================================================== */
/*
*   user exit callbacks
*/

/* External fnctions are now obsolete */
#if 0
static int runuftyp(runuxdef *ctx)
{
	return(runtostyp(ctx->runuxctx));
}

static long runufnpo(runuxdef *ctx)
{
	return(runpopnum(ctx->runuxctx));
}

static uchar *runufspo(runuxdef *ctx)
{
	return(runpopstr(ctx->runuxctx));
}

static void runufdsc(runuxdef *ctx)
{
	rundisc(ctx->runuxctx);
}

static void runufnpu(runuxdef *ctx, long num)
{
	runpnum(ctx->runuxctx, num);
}

static void runufspu(runuxdef *ctx, uchar *str)
{
	runsdef val;

	val.runstyp = DAT_SSTRING;
	val.runsv.runsvstr = str - 2;
	runrepush(ctx->runuxctx, &val);
}

static void runufcspu(runuxdef *ctx, char *str)
{
	runpstr(ctx->runuxctx, str, (int)strlen(str), ctx->runuxargc);
}

static uchar *runufsal(runuxdef *ctx, int len)
{
	uchar *ret;

	len += 2;
	runhres(ctx->runuxctx, len, ctx->runuxargc);
	ret = ctx->runuxctx->runcxhp;
	oswp2(ret, len);
	ret += 2;

	ctx->runuxctx->runcxhp += len;
	return(ret);
}

static void runuflpu(runuxdef *ctx, int typ)
{
	runsdef val;

	val.runstyp = typ;
	runrepush(ctx->runuxctx, &val);
}

#endif

/* convert an osrp2 value to a signed short value */
#define runrp2s(p) ((short)(ushort)osrp2(p))


/* ======================================================================== */
/*
*   execute p-code
*/
void runexe(runcxdef *ctx, uchar *p0, objnum self, objnum target,
	prpnum targprop, int argc)
{
	uchar    *noreg p = p0;
	uchar     opc;                     /* opcode we're currently working on */
	runsdef   val;                           /* stack element (for pushing) */
	runsdef   val2;     /* another one (for popping in two-op instructions) */
	uint      ofs;                   /* offset in code of current execution */
	prpnum    prop = 0;                     /* property number, when needed */
	objnum    obj = 0;                        /* object number, when needed */
	runsdef  *noreg rstsp;        /* sp to reset to on DISCARD instructions */
	uchar    *lstp = nullptr;                               /* list pointer */
	int       nargc;                   /* argument count of called function */
	runsdef  *valp;
	runsdef  *stkval;
	int       i = 0;
	int       brkchk;
	runsdef  val3;
	int      asityp;
	int      asiext = 0;
	int      lclnum = 0;

#ifndef DBG_OFF
	int       err;
#endif

	NOREG((&rstp, &p));

	/* save entry SP - this is reset point until ENTER */
	rstsp = ctx->runcxsp;

#ifndef DBG_OFF
	/*
	*   For the debugger's sake, set up an error frame so that we catch
	*   any errors thrown during p-code execution within this function.
	*   If an error occurs, and the debugger is present, we'll set the
	*   instruction pointer back to the start of the line that caused the
	*   error and enter the debugger with the error indication.  If the
	*   debugger isn't present, we'll simply re-throw the error.  This
	*   entire block can be compiled out of the execution engine when
	*   linking a stand-alone (non-debug) version of the run-time.
	*/
resume_from_error:
	ERRBEGIN(ctx->runcxerr)
#endif /* DBG_OFF */

		for (brkchk = 0;; ++brkchk)
		{
			/* check for break - signal if user has hit break */
			if (brkchk == 1000)
			{
				brkchk = 0;
				if (os_break()) runsig(ctx, ERR_USRINT);
			}

			opc = *p++;

			switch (opc)
			{
			case OPCPUSHNUM:
				val.runsv.runsvnum = osrp4s(p);
				runpush(ctx, DAT_NUMBER, &val);
				p += 4;
				break;

			case OPCPUSHOBJ:
				val.runsv.runsvobj = osrp2(p);
				runpush(ctx, DAT_OBJECT, &val);
				p += 2;
				break;

			case OPCPUSHSELF:
				val.runsv.runsvobj = self;
				runpush(ctx, DAT_OBJECT, &val);
				break;

			case OPCPUSHSTR:
				val.runsv.runsvstr = p;
				runpush(ctx, DAT_SSTRING, &val);
				p += osrp2(p);                              /* skip past string */
				break;

			case OPCPUSHLST:
				val.runsv.runsvstr = p;
				runpush(ctx, DAT_LIST, &val);
				p += osrp2(p);                                /* skip past list */
				break;

			case OPCPUSHNIL:
				runpush(ctx, DAT_NIL, &val);
				break;

			case OPCPUSHTRUE:
				runpush(ctx, DAT_TRUE, &val);
				break;

			case OPCPUSHFN:
				val.runsv.runsvobj = osrp2(p);
				runpush(ctx, DAT_FNADDR, &val);
				p += 2;
				break;

			case OPCPUSHPN:
				val.runsv.runsvprp = osrp2(p);
				runpush(ctx, DAT_PROPNUM, &val);
				p += 2;
				break;

			case OPCNEG:
				val.runstyp = DAT_NUMBER;
				val.runsv.runsvnum = -runpopnum(ctx);
				runrepush(ctx, &val);
				break;

			case OPCBNOT:
				val.runstyp = DAT_NUMBER;
				val.runsv.runsvnum = ~runpopnum(ctx);
				runrepush(ctx, &val);
				break;

			case OPCNOT:
				if (runtoslog(ctx))
					runpush(ctx, runclog(!runpoplog(ctx)), &val);
				else
					runpush(ctx, runclog(runpopnum(ctx)), &val);
				break;

			case OPCADD:
				runpop(ctx, &val2);    /* right op is pushed last -> popped 1st */
				runpop(ctx, &val);
				runadd(ctx, &val, &val2, 2);
				runrepush(ctx, &val);
				break;

			case OPCSUB:
				runpop(ctx, &val2);    /* right op is pushed last -> popped 1st */
				runpop(ctx, &val);
				(void)runsub(ctx, &val, &val2, 2);
				runrepush(ctx, &val);
				break;

			case OPCMUL:
				val.runstyp = DAT_NUMBER;
				val.runsv.runsvnum = runpopnum(ctx);
				val.runsv.runsvnum *= runpopnum(ctx);
				runrepush(ctx, &val);
				break;

			case OPCBAND:
				val.runstyp = DAT_NUMBER;
				val.runsv.runsvnum = runpopnum(ctx);
				val.runsv.runsvnum &= runpopnum(ctx);
				runrepush(ctx, &val);
				break;

			case OPCBOR:
				val.runstyp = DAT_NUMBER;
				val.runsv.runsvnum = runpopnum(ctx);
				val.runsv.runsvnum |= runpopnum(ctx);
				runrepush(ctx, &val);
				break;

			case OPCSHL:
				val.runstyp = DAT_NUMBER;
				val.runsv.runsvnum = runpopnum(ctx);
				val.runsv.runsvnum = runpopnum(ctx) << val.runsv.runsvnum;
				runrepush(ctx, &val);
				break;

			case OPCSHR:
				val.runstyp = DAT_NUMBER;
				val.runsv.runsvnum = runpopnum(ctx);
				val.runsv.runsvnum = runpopnum(ctx) >> val.runsv.runsvnum;
				runrepush(ctx, &val);
				break;

			case OPCXOR:
				/* allow logical ^ logical or number ^ number */
				if (runtoslog(ctx))
				{
					int a, b;

					/* logicals - return a logical value */
					a = runpoplog(ctx);
					b = runpoplog(ctx);
					val.runstyp = runclog(a ^ b);
				}
				else
				{
					/* numeric value - return binary xor */
					val.runstyp = DAT_NUMBER;
					val.runsv.runsvnum = runpopnum(ctx);
					val.runsv.runsvnum ^= runpopnum(ctx);
				}
				runrepush(ctx, &val);
				break;

			case OPCDIV:
				val.runsv.runsvnum = runpopnum(ctx);
				if (val.runsv.runsvnum == 0)
					runsig(ctx, ERR_DIVZERO);
				val.runsv.runsvnum = runpopnum(ctx) / val.runsv.runsvnum;
				val.runstyp = DAT_NUMBER;
				runrepush(ctx, &val);
				break;

			case OPCMOD:
				val.runsv.runsvnum = runpopnum(ctx);
				if (val.runsv.runsvnum == 0)
					runsig(ctx, ERR_DIVZERO);
				val.runsv.runsvnum = runpopnum(ctx) % val.runsv.runsvnum;
				val.runstyp = DAT_NUMBER;
				runrepush(ctx, &val);
				break;

#ifdef NEVER
			case OPCAND:
				if (runtostyp(ctx) == DAT_LIST)
					runlstisect(ctx);
				else
					runpush(ctx, runclog(runpoplog(ctx) && runpoplog(ctx)), &val);
				break;

			case OPCOR:
				runpush(ctx, runclog(runpoplog(ctx) || runpoplog(ctx)), &val);
				break;
#endif /* NEVER */

			case OPCEQ:
				runpush(ctx, runclog(runeq(ctx)), &val);
				break;

			case OPCNE:
				runpush(ctx, runclog(!runeq(ctx)), &val);
				break;

			case OPCLT:
				runpush(ctx, runclog(runmcmp(ctx) < 0), &val);
				break;

			case OPCLE:
				runpush(ctx, runclog(runmcmp(ctx) <= 0), &val);
				break;

			case OPCGT:
				runpush(ctx, runclog(runmcmp(ctx) > 0), &val);
				break;

			case OPCGE:
				runpush(ctx, runclog(runmcmp(ctx) >= 0), &val);
				break;

			case OPCCALL:
			{
				objnum o;

				/* get the argument count */
				nargc = *p++;

				/* ensure we have enough values to pass as arguments */
				runcheckargc(ctx, &nargc);

				/* object could move--save offset to restore 'p' after call */
				o = osrp2(p);
				ofs = runcpsav(ctx, &p, target, targprop);

				/* execute the function */
				runfn(ctx, o, nargc);

				/* restore code pointer in case target object moved */
				p = runcprst(ctx, ofs, target, targprop) + 2;
				break;
			}

			case OPCGETP:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				prop = osrp2(p);
				p += 2;
				obj = runpopobj(ctx);
				runpprop(ctx, &p, target, targprop, obj, prop, FALSE, nargc,
					obj);
				break;

			case OPCGETPDATA:
				prop = osrp2(p);
				p += 2;
				obj = runpopobj(ctx);
				runcheckpropdata(ctx, obj, prop);
				runpprop(ctx, &p, target, targprop, obj, prop, FALSE, 0, obj);
				break;

			case OPCGETDBLCL:
#ifdef DBG_OFF
				/* non-debug mode - this will always throw an error */
				dbgfrfind(ctx->runcxdbg, 0, 0);
#else
				/* debug mode - look up the local in the stack frame */
				{
					objnum   frobj;
					uint     frofs;
					runsdef *otherbp;

					frobj = osrp2(p);
					frofs = osrp2(p + 2);
					otherbp = dbgfrfind(ctx->runcxdbg, frobj, frofs);
					runrepush(ctx, otherbp + runrp2s(p + 4) - 1);
					p += 6;
				}
#endif
				break;

			case OPCGETLCL:
				runrepush(ctx, ctx->runcxbp + runrp2s(p) - 1);
				p += 2;
				break;

			case OPCRETURN:
				runleave(ctx, argc /* was: osrp2(p) */);
				dbgleave(ctx->runcxdbg, DBGEXRET);
				goto done;

			case OPCRETVAL:
				/* if there's nothing on the stack, return nil */
				if (runtostyp(ctx) != DAT_BASEPTR)
					runpop(ctx, &val);
				else
					val.runstyp = DAT_NIL;

				runleave(ctx, argc /* was: osrp2(p) */);
				runrepush(ctx, &val);
				dbgleave(ctx->runcxdbg, DBGEXVAL);
				goto done;

			case OPCENTER:
				/* push old base pointer and set up new one */
				ctx->runcxsp = rstsp;
				val.runsv.runsvstr = (uchar *)ctx->runcxbp;
				runpush(ctx, DAT_BASEPTR, &val);
				ctx->runcxbp = ctx->runcxsp;

				/* add a trace record */
				dbgenter(ctx->runcxdbg, ctx->runcxbp, self, target, targprop,
					0, argc);

				/* initialize locals to nil */
				for (i = osrp2(p); i; --i) runpush(ctx, DAT_NIL, &val);
				p += 2;                         /* skip the local count operand */

												/* save stack pointer - reset sp to this value on DISCARD */
				rstsp = ctx->runcxsp;
				break;

			case OPCDISCARD:
				ctx->runcxsp = rstsp;
				break;

			case OPCSWITCH:
			{
				int      tostyp;
				int      match, typmatch;

				runpop(ctx, &val);
				tostyp = val.runstyp;
				switch (tostyp)
				{
				case DAT_SSTRING:
					tostyp = OPCPUSHSTR;
					break;
				case DAT_LIST:
					tostyp = OPCPUSHLST;
					break;
				case DAT_PROPNUM:
					tostyp = OPCPUSHPN;
					break;
				case DAT_FNADDR:
					tostyp = OPCPUSHFN;
					break;
				case DAT_TRUE:
					tostyp = OPCPUSHTRUE;
					break;
				case DAT_NIL:
					tostyp = OPCPUSHNIL;
					break;
				}

				p += osrp2(p);                         /* find the switch table */
				i = osrp2(p);                            /* get number of cases */

														 /* look for a matching case */
				for (match = FALSE; i && !match; --i)
				{
					p += 2;                     /* skip previous jump/size word */
					typmatch = (*p == tostyp);
					switch (*p++)
					{
					case OPCPUSHNUM:
						match = (typmatch
							&& val.runsv.runsvnum == osrp4s(p));
						p += 4;
						break;

					case OPCPUSHLST:
					case OPCPUSHSTR:
						match = (typmatch
							&& osrp2(val.runsv.runsvstr) == osrp2(p)
							&& !memcmp(val.runsv.runsvstr,
								p, (size_t)osrp2(p)));
						p += runrp2s(p);
						break;

					case OPCPUSHPN:
						match = (typmatch
							&& val.runsv.runsvprp == osrp2(p));
						p += 2;
						break;

					case OPCPUSHOBJ:
					case OPCPUSHFN:
						match = (typmatch
							&& val.runsv.runsvobj == osrp2(p));
						p += 2;
						break;

					case OPCPUSHSELF:
						match = (typmatch && val.runsv.runsvobj == self);
						break;

					case OPCPUSHTRUE:
					case OPCPUSHNIL:
						match = typmatch;
						break;
					}
				}

				if (!match) p += 2;         /* if default, skip to default case */
				p += runrp2s(p);      /* wherever we left off, p points to jump */
				break;
			}

			case OPCJMP:
				p += runrp2s(p);
				break;

			case OPCJT:
				if (runtoslog(ctx))
					p += (runpoplog(ctx) ? runrp2s(p) : 2);
				else
					p += (runpopnum(ctx) != 0 ? runrp2s(p) : 2);
				break;

			case OPCJF:
				if (runtoslog(ctx))
					p += ((!runpoplog(ctx)) ? runrp2s(p) : 2);
				else if (runtostyp(ctx) == DAT_NUMBER)
					p += ((runpopnum(ctx) == 0) ? runrp2s(p) : 2);
				else                      /* consider any other type to be true */
				{
					rundisc(ctx);  /* throw away the item considered to be true */
					p += 2;
				}
				break;

			case OPCSAY:
				outfmt(ctx->runcxtio, p);
				p += osrp2(p);                              /* skip past string */
				break;

			case OPCBUILTIN:
			{
				int      binum;
				runsdef *stkp;

				nargc = *p++;
				runcheckargc(ctx, &nargc);
				binum = osrp2(p);
				ofs = runcpsav(ctx, &p, target, targprop);
				stkp = ctx->runcxsp - nargc;

				dbgenter(ctx->runcxdbg, ctx->runcxsp + 1, MCMONINV, MCMONINV,
					(prpnum)0, binum, nargc);
				(*ctx->runcxbi[binum])((struct bifcxdef *)ctx->runcxbcx,
					nargc);
				dbgleave(ctx->runcxdbg,
					ctx->runcxsp != stkp ? DBGEXVAL : DBGEXRET);

				p = runcprst(ctx, ofs, target, targprop);
				p += 2;
				break;
			}

			case OPCPTRCALL:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				ofs = runcpsav(ctx, &p, target, targprop);
				runfn(ctx, runpopfn(ctx), nargc);
				p = runcprst(ctx, ofs, target, targprop);
				break;

			case OPCINHERIT:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				prop = osrp2(p);
				p += 2;
				runpprop(ctx, &p, target, targprop, target, prop, TRUE, nargc,
					self);
				break;

			case OPCPTRINH:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				prop = runpopprp(ctx);
				runpprop(ctx, &p, target, targprop, target, prop, TRUE, nargc,
					self);
				break;

			case OPCPTRGETP:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				prop = runpopprp(ctx);
				obj = runpopobj(ctx);
				runpprop(ctx, &p, target, targprop, obj, prop, FALSE, nargc,
					obj);
				break;

			case OPCPTRGETPDATA:
				prop = runpopprp(ctx);
				obj = runpopobj(ctx);
				runcheckpropdata(ctx, obj, prop);
				runpprop(ctx, &p, target, targprop, obj, prop, FALSE, 0, obj);
				break;

			case OPCEXPINH:
				/* inheritance from explicit superclass */
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				prop = osrp2(p);
				obj = osrp2(p + 2);
				p += 4;

				/*
				*   Evaluate the property of the given object, but keeping
				*   the same 'self' as is currently in effect.  Note that the
				*   'inherit' flag is FALSE in this call, even though we're
				*   inheriting, because the opcode explicitly specifies the
				*   object we want to inherit from.
				*/
				runpprop(ctx, &p, target, targprop, obj, prop, FALSE,
					nargc, self);
				break;

			case OPCEXPINHPTR:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				prop = runpopprp(ctx);
				obj = osrp2(p);
				p += 2;
				runpprop(ctx, &p, target, targprop, obj, prop, FALSE,
					nargc, self);
				break;

			case OPCPASS:
				prop = osrp2(p);
				runleave(ctx, 0);
				dbgleave(ctx->runcxdbg, DBGEXPASS);
				runpprop(ctx, &p, target, targprop, target, prop, TRUE, argc,
					self);
				goto done;

			case OPCEXIT:
				errsig(ctx->runcxerr, ERR_RUNEXIT);
				break;

			case OPCABORT:
				errsig(ctx->runcxerr, ERR_RUNABRT);
				break;

			case OPCASKDO:
				errsig(ctx->runcxerr, ERR_RUNASKD);
				break;

			case OPCASKIO:
				errsig1(ctx->runcxerr, ERR_RUNASKI, ERRTINT, osrp2(p));
				break;

			case OPCJE:
				p += (runeq(ctx) ? runrp2s(p) : 2);
				break;

			case OPCJNE:
				p += (!runeq(ctx) ? runrp2s(p) : 2);
				break;

			case OPCJGT:
				p += (runmcmp(ctx) > 0 ? runrp2s(p) : 2);
				break;

			case OPCJGE:
				p += (runmcmp(ctx) >= 0 ? runrp2s(p) : 2);
				break;

			case OPCJLT:
				p += (runmcmp(ctx) < 0 ? runrp2s(p) : 2);
				break;

			case OPCJLE:
				p += (runmcmp(ctx) <= 0 ? runrp2s(p) : 2);
				break;

			case OPCJNAND:
				p += (!(runpoplog(ctx) && runpoplog(ctx)) ? runrp2s(p) : 2);
				break;

			case OPCJNOR:
				p += (!(runpoplog(ctx) || runpoplog(ctx)) ? runrp2s(p) : 2);
				break;

			case OPCGETPSELF:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				prop = osrp2(p);
				p += 2;
				runpprop(ctx, &p, target, targprop, self, prop, FALSE, nargc,
					self);
				break;

			case OPCGETPSELFDATA:
				prop = osrp2(p);
				p += 2;
				runcheckpropdata(ctx, self, prop);
				runpprop(ctx, &p, target, targprop, self, prop, FALSE, 0, self);
				break;

			case OPCGETPPTRSELF:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				prop = runpopprp(ctx);
				runpprop(ctx, &p, target, targprop, self, prop, FALSE, nargc,
					self);
				break;

			case OPCGETPOBJ:
				nargc = *p++;
				runcheckargc(ctx, &nargc);
				obj = osrp2(p);
				prop = osrp2(p + 2);
				p += 4;
				runpprop(ctx, &p, target, targprop, obj, prop, FALSE, nargc,
					obj);
				break;

			case OPCINDEX:
				i = runpopnum(ctx);                                /* get index */
				lstp = runpoplst(ctx);                          /* get the list */
				runpind(ctx, i, lstp);
				break;

			case OPCJST:
				if (runtostyp(ctx) == DAT_TRUE)
					p += runrp2s(p);
				else
				{
					(void)runpoplog(ctx);
					p += 2;
				}
				break;

			case OPCJSF:
				if (runtostyp(ctx) == DAT_NIL ||
					(runtostyp(ctx) == DAT_NUMBER &&
					(ctx->runcxsp - 1)->runsv.runsvnum == 0))
					p += runrp2s(p);
				else
				{
					runpop(ctx, &val);
					p += 2;
				}
				break;

			case OPCCALLEXT:
			{
#if 0 // external functions are now obsolete
				static runufdef uf =
				{
					runuftyp,  runufnpo,  runufspo,  runufdsc,
					runufnpu,  runufspu,  runufcspu, runufsal,
					runuflpu
				};
				int        fn;
				runxdef   *ex;

				runuxdef   ux;

				/* set up callback context */
				ux.runuxctx = ctx;
				ux.runuxvec = &uf;
				ux.runuxargc = *p++;

				fn = osrp2(p);
				p += 2;
				ex = &ctx->runcxext[fn];

				if (!ex->runxptr)
				{
					if ((ex->runxptr = os_exfil(ex->runxnam)) == 0)
						runsig1(ctx, ERR_EXTLOAD, ERRTSTR, ex->runxnam);
				}
				if (os_excall(ex->runxptr, &ux))
					runsig1(ctx, ERR_EXTRUN, ERRTSTR, ex->runxnam);
#else
				/* external functions are obsolete - throw an error */
				runxdef *ex;
				p += 1;
				ex = &ctx->runcxext[osrp2(p)];
				p += 2;
				runsig1(ctx, ERR_EXTRUN, ERRTSTR, ex->runxnam);
#endif
			}
			break;

			case OPCDBGRET:
				goto done;

			case OPCCONS:
			{
				uint    totsiz;
				uint    oldsiz;
				uint    tot;
				uint    cursiz;
				runsdef lstend;

				tot = i = osrp2(p);    /* get # of items to build into list */
				p += 2;

				/* reserve space for initial list (w/length word only) */
				runhres(ctx, 2, 0);

				/*
				*   Set up value to point to output list, making room
				*   for length prefix.  Remember size-so-far separately.
				*/
				lstend.runstyp = DAT_LIST;
				lstend.runsv.runsvstr = ctx->runcxhp;
				ctx->runcxhp += 2;
				totsiz = 2;

				while (i--)
				{
					runpop(ctx, &val);          /* get next value off stack */
					cursiz = runsiz(&val);

					/*
					*   Set up to allocate space.  Before doing so, make
					*   sure the list under construction is valid, to
					*   ensure that it stays around after garbage
					*   collection.
					*/
					oldsiz = totsiz;
					totsiz += cursiz + 1;
					oswp2(lstend.runsv.runsvstr, oldsiz);
					ctx->runcxhp = lstend.runsv.runsvstr + oldsiz;
					runhres2(ctx, cursiz + 1, tot - i, &val, &lstend);

					/* write this item to the list */
					runputbuf(lstend.runsv.runsvstr + oldsiz, &val);
				}
				oswp2(lstend.runsv.runsvstr, totsiz);
				ctx->runcxhp = lstend.runsv.runsvstr + totsiz;
				runrepush(ctx, &lstend);
			}
			break;

			case OPCARGC:
				val.runsv.runsvnum = argc;
				runpush(ctx, DAT_NUMBER, &val);
				break;

			case OPCCHKARGC:
				if ((*p & 0x80) ? argc < (*p & 0x7f) : argc != *p)
				{
					char namebuf[128];
					size_t namelen;

					/*
					*   debugger is present - look up the name of the current
					*   function or method, so that we can report it in the
					*   error message
					*/
					if (targprop == 0)
					{
						/* we're in a function */
						namelen = dbgnam(ctx->runcxdbg, namebuf, TOKSTFUNC,
							target);
					}
					else
					{
						/* we're in an object.method */
						namelen = dbgnam(ctx->runcxdbg, namebuf, TOKSTOBJ,
							target);
						namebuf[namelen++] = '.';
						namelen += dbgnam(ctx->runcxdbg, namebuf + namelen,
							TOKSTPROP, targprop);
					}
					namebuf[namelen] = '\0';
					runsig1(ctx, ERR_ARGC, ERRTSTR, namebuf);
				}
				++p;
				break;

			case OPCLINE:
			case OPCBP:
			{
				uchar *ptr = mcmobjptr(ctx->runcxmem, (mcmon)target);
				uchar  instr;

				/* set up the debugger frame record for this line */
				dbgframe(ctx->runcxdbg, osrp2(p + 1), p - ptr);

				/* remember the instruction */
				instr = *(p - 1);

				/* remember the offset of the line record */
				ctx->runcxlofs = ofs = (p + 2 - ptr);

				/* skip to the next instruction */
				p += *p;

				/* let the debugger take over, if it wants to */
				dbgssi(ctx->runcxdbg, ofs, instr, 0, &p);
				break;
			}

			case OPCFRAME:
				/* this is a frame record - just jump past it */
				p += osrp2(p);
				break;

			case OPCASI_MASK | OPCASIDIR | OPCASILCL:
				runpop(ctx, &val);
				OSCPYSTRUCT(*(ctx->runcxbp + runrp2s(p) - 1), val);
				stkval = &val;
				p += 2;
				goto no_assign;

			case OPCASI_MASK | OPCASIDIR | OPCASIPRP:
				obj = runpopobj(ctx);
				prop = osrp2(p);
				p += 2;
				runpop(ctx, &val);
				stkval = valp = &val;
				goto assign_property;

			case OPCASI_MASK | OPCASIDIR | OPCASIPRPPTR:
				prop = runpopprp(ctx);
				obj = runpopobj(ctx);
				runpop(ctx, &val);
				stkval = valp = &val;
				goto assign_property;

			case OPCNEW:
				run_new(ctx, &p, target, targprop);
				break;

			case OPCDELETE:
				run_delete(ctx, &p, target, targprop);
				break;

			default:
				if ((opc & OPCASI_MASK) == OPCASI_MASK)
				{
					valp = &val;
					stkval = &val;

					asityp = (opc & OPCASITYP_MASK);
					if (asityp == OPCASIEXT)
						asiext = *p++;

					/* get list element/property number if needed */
					switch (opc & OPCASIDEST_MASK)
					{
					case OPCASIPRP:
						obj = runpopobj(ctx);
						prop = osrp2(p);
						p += 2;
						break;

					case OPCASIPRPPTR:
						prop = runpopprp(ctx);
						obj = runpopobj(ctx);
						break;

					case OPCASIIND:
						i = runpopnum(ctx);
						lstp = runpoplst(ctx);
						break;

					case OPCASILCL:
						lclnum = runrp2s(p);
						p += 2;
						break;
					}

					if (asityp != OPCASIDIR)
					{
						/* we have an <op>= operator - get lval, modify, & set */
						switch (opc & OPCASIDEST_MASK)
						{
						case OPCASILCL:
							OSCPYSTRUCT(val, *(ctx->runcxbp + lclnum - 1));
							break;

						case OPCASIPRP:
						case OPCASIPRPPTR:
							runpprop(ctx, &p, target, targprop, obj, prop,
								FALSE, 0, obj);
							runpop(ctx, &val);
							break;

						case OPCASIIND:
							runpind(ctx, i, lstp);
							runpop(ctx, &val);
							break;
						}

						/* if saving pre-inc/dec value, get the value now */
						if ((opc & OPCASIPRE_MASK) == OPCASIPOST)
						{
							OSCPYSTRUCT(val3, val);
							stkval = &val3;
						}
					}

					/* get rvalue, except for inc/dec operations */
					if (asityp != OPCASIINC && asityp != OPCASIDEC)
						runpop(ctx, &val2);

					/* now apply operation to lvalue using rvalue */
					switch (asityp)
					{
					case OPCASIADD:
						if ((opc & OPCASIIND) != 0)
						{
							runsdef val4;

							/*
							*   we're adding to an indexed value out of a list -
							*   we need to make sure the list is protected from
							*   garbage collection, so push it back on the stack
							*   while we're working
							*/
							val4.runstyp = DAT_LIST;
							val4.runsv.runsvstr = lstp;
							runrepush(ctx, &val4);

							/* carry out the addition */
							runadd(ctx, &val, &val2, 2);

							/*
							*   in case the list got moved during garbage
							*   collection, retrieve it from the stack
							*/
							lstp = runpoplst(ctx);
						}
						else
						{
							/* no list indexing - just carry out the addition */
							runadd(ctx, &val, &val2, 2);
						}
						break;

					case OPCASISUB:
						if ((opc & OPCASIIND) != 0)
						{
							runsdef val4;
							int result;

							/* as with adding, protect the list from GC */
							val4.runstyp = DAT_LIST;
							val4.runsv.runsvstr = lstp;
							runrepush(ctx, &val4);

							/* carry out the subtraction and note the result */
							result = runsub(ctx, &val, &val2, 2);

							/* recover the list pointer */
							lstp = runpoplst(ctx);

							/* check to see if we have an assignment */
							if (!result)
								goto no_assign;
						}
						else
						{
							/* no list indexing - just do the subtraction */
							if (!runsub(ctx, &val, &val2, 2))
								goto no_assign;
						}
						break;

					case OPCASIMUL:
						if (val.runstyp != DAT_NUMBER
							|| val2.runstyp != DAT_NUMBER)
							runsig(ctx, ERR_REQNUM);
						val.runsv.runsvnum *= val2.runsv.runsvnum;
						break;

					case OPCASIDIV:
						if (val.runstyp != DAT_NUMBER
							|| val2.runstyp != DAT_NUMBER)
							runsig(ctx, ERR_REQNUM);
						if (val2.runsv.runsvnum == 0)
							runsig(ctx, ERR_DIVZERO);
						val.runsv.runsvnum /= val2.runsv.runsvnum;
						break;

					case OPCASIINC:
						if (val.runstyp != DAT_NUMBER)
							runsig(ctx, ERR_REQNUM);
						++(val.runsv.runsvnum);
						break;

					case OPCASIDEC:
						if (val.runstyp != DAT_NUMBER)
							runsig(ctx, ERR_REQNUM);
						--(val.runsv.runsvnum);
						break;

					case OPCASIDIR:
						valp = stkval = &val2;
						break;

					case OPCASIEXT:
						switch (asiext)
						{
						case OPCASIMOD:
							if (val.runstyp != DAT_NUMBER
								|| val2.runstyp != DAT_NUMBER)
								runsig(ctx, ERR_REQNUM);
							if (val2.runsv.runsvnum == 0)
								runsig(ctx, ERR_DIVZERO);
							val.runsv.runsvnum %= val2.runsv.runsvnum;
							break;

						case OPCASIBAND:
							if ((val.runstyp == DAT_TRUE
								|| val.runstyp == DAT_NIL)
								&& (val2.runstyp == DAT_TRUE
									|| val2.runstyp == DAT_NIL))
							{
								int a, b;

								a = (val.runstyp == DAT_TRUE ? 1 : 0);
								b = (val2.runstyp == DAT_TRUE ? 1 : 0);
								val.runstyp = runclog(a && b);
							}
							else if (val.runstyp == DAT_NUMBER
								&& val2.runstyp == DAT_NUMBER)
								val.runsv.runsvnum &= val2.runsv.runsvnum;
							else
								runsig(ctx, ERR_REQNUM);
							break;

						case OPCASIBOR:
							if ((val.runstyp == DAT_TRUE
								|| val.runstyp == DAT_NIL)
								&& (val2.runstyp == DAT_TRUE
									|| val2.runstyp == DAT_NIL))
							{
								int a, b;

								a = (val.runstyp == DAT_TRUE ? 1 : 0);
								b = (val2.runstyp == DAT_TRUE ? 1 : 0);
								val.runstyp = runclog(a || b);
							}
							else if (val.runstyp == DAT_NUMBER
								&& val2.runstyp == DAT_NUMBER)
								val.runsv.runsvnum |= val2.runsv.runsvnum;
							else
								runsig(ctx, ERR_REQNUM);
							break;

						case OPCASIXOR:
							if ((val.runstyp == DAT_TRUE || val.runstyp == DAT_NIL)
								&& (val2.runstyp == DAT_TRUE
									|| val2.runstyp == DAT_NIL))
							{
								int a, b;

								a = (val.runstyp == DAT_TRUE ? 1 : 0);
								b = (val2.runstyp == DAT_TRUE ? 1 : 0);
								val.runstyp = runclog(a ^ b);
							}
							else if (val.runstyp == DAT_NUMBER
								&& val2.runstyp == DAT_NUMBER)
								val.runsv.runsvnum ^= val2.runsv.runsvnum;
							else
								runsig(ctx, ERR_REQNUM);
							break;

						case OPCASISHL:
							if (val.runstyp != DAT_NUMBER
								|| val2.runstyp != DAT_NUMBER)
								runsig(ctx, ERR_REQNUM);
							val.runsv.runsvnum <<= val2.runsv.runsvnum;
							break;

						case OPCASISHR:
							if (val.runstyp != DAT_NUMBER
								|| val2.runstyp != DAT_NUMBER)
								runsig(ctx, ERR_REQNUM);
							val.runsv.runsvnum >>= val2.runsv.runsvnum;
							break;

						default:
							runsig(ctx, ERR_INVOPC);
						}
						break;

					default:
						runsig(ctx, ERR_INVOPC);
					}

					/* write the rvalue at *valp to the lvalue */
					switch (opc & OPCASIDEST_MASK)
					{
					case OPCASILCL:
						OSCPYSTRUCT(*(ctx->runcxbp + lclnum - 1), *valp);
						break;

					case OPCASIPRP:
					case OPCASIPRPPTR:
					assign_property:
					{
						void    *valbuf;
						uchar    outbuf[4];

						switch (valp->runstyp)
						{
						case DAT_LIST:
						case DAT_SSTRING:
							valbuf = valp->runsv.runsvstr;
							break;

						case DAT_NUMBER:
							valbuf = outbuf;
							oswp4s(outbuf, valp->runsv.runsvnum);
							break;

						case DAT_OBJECT:
						case DAT_FNADDR:
							valbuf = outbuf;
							oswp2(outbuf, valp->runsv.runsvobj);
							break;

						case DAT_PROPNUM:
							valbuf = outbuf;
							oswp2(outbuf, valp->runsv.runsvprp);
							break;

						default:
							valbuf = &valp->runsv;
							break;
						}

						ofs = runcpsav(ctx, &p, target, targprop);
						objsetp(ctx->runcxmem, obj, prop, valp->runstyp,
							valbuf, ctx->runcxundo);
						p = runcprst(ctx, ofs, target, targprop);
						break;
					}

					case OPCASIIND:
					{
						uint   newtot;
						uint   newsiz;
						uint   remsiz;
						uint   delsiz;
						uchar *delp;
						uchar *remp;

						/* compute sizes and pointers to various parts */
						ofs = runindofs(ctx, i, lstp);
						delp = lstp + ofs;        /* ptr to item to replace */
						delsiz = datsiz(*delp, delp + 1);  /* size of *delp */
						remsiz = osrp2(lstp) - ofs - delsiz - 1;
						newsiz = runsiz(valp);          /* size of new item */
						newtot = osrp2(lstp) + newsiz - delsiz;  /* new tot */

						/* reserve space for the new list & copy first part */
						{
							runsdef rval3;

							/* make sure lstp stays valid before and after */
							rval3.runstyp = DAT_LIST;
							rval3.runsv.runsvstr = lstp;
							runhres3(ctx, newtot, 3, &val, &val2, &rval3);

							/* update all of the pointers within lstp */
							lstp = rval3.runsv.runsvstr;
							delp = lstp + ofs;
							remp = lstp + ofs + delsiz + 1;
						}
						memcpy(ctx->runcxhp + 2, lstp + 2, (size_t)(ofs - 2));

						/* set size of new list */
						oswp2(ctx->runcxhp, newtot);

						/* copy new item into buffer */
						runputbuf(ctx->runcxhp + ofs, valp);

						/* copy remainder and update heap pointer */
						memcpy(ctx->runcxhp + ofs + newsiz + 1, remp,
							(size_t)remsiz);
						val.runstyp = DAT_LIST;
						val.runsv.runsvstr = ctx->runcxhp;
						stkval = &val;
						ctx->runcxhp += newtot;
						break;
					}
					}

				no_assign:   /* skip assignment - operation didn't change value */
					if (*p == OPCDISCARD)
					{
						/* next assignment is DISCARD - deal with it now */
						++p;
						ctx->runcxsp = rstsp;
					}
					else
						runrepush(ctx, stkval);
				}
				else
					errsig(ctx->runcxerr, ERR_INVOPC);
			}
		}

	/*
	*   come here to return - don't use 'return' directly, since that
	*   would not properly exit the error frame
	*/
done:;

#ifndef DBG_OFF
	/*
	*   Come here to catch any errors that occur during execution of this
	*   p-code
	*/
	ERRCATCH(ctx->runcxerr, err)
	{
		/*
		*   if the debugger isn't present, or we're already in the
		*   debugger, or if the debugger can't resume from errors, or if
		*   we're not in user code (in which case the debugger can't
		*   resume from this error even if it normally could resume from
		*   an error), simply re-signal the error
		*/
		if (!dbgpresent()
			|| ctx->runcxdbg->dbgcxfcn == 0
			|| !dbgu_err_resume(ctx->runcxdbg)
			|| (ctx->runcxdbg->dbgcxflg & DBGCXFIND) != 0)
			errrse(ctx->runcxerr);

		/* check the error code */
		switch (err)
		{
		case ERR_RUNEXIT:
		case ERR_RUNABRT:
		case ERR_RUNASKD:
		case ERR_RUNASKI:
		case ERR_RUNQUIT:
		case ERR_RUNRESTART:
		case ERR_RUNEXITOBJ:
			/* don't trap these errors - resignal it immediately */
			errrse(ctx->runcxerr);

		default:
			/* trap other errors to the debugger */
			break;
		}

		/* if the object was unlocked, re-lock it */
		if (p == 0)
			mcmlck(ctx->runcxmem, target);

		/* set up after the last OPCLINE instruction */
		p = mcmobjptr(ctx->runcxmem, (mcmon)target) + ctx->runcxlofs - 2;
		p += *p;

		/*
		*   Keep the current error's arguments around for handling
		*   outside of this handler, since we'll need them in dbgssi.
		*/
		errkeepargs(ctx->runcxerr);

		/* enter the debugger with the error code */
		dbgssi(ctx->runcxdbg, ctx->runcxlofs, OPCLINE, err, &p);

		/* check the error again */
		switch (err)
		{
		case ERR_ARGC:
			/* we can't continue from this - simply return */
			break;

		default:
			/* resume execution */
			goto resume_from_error;
		}
	}
	ERREND(ctx->runcxerr);
#endif /* DBG_OFF */
}

/*
*   Signal a run-time error.  This function first calls the debugger
*   single-step function to allow the debugger to trap the error, then
*   signals the error as usual when the debugger returns.
*/
void runsign(runcxdef *ctx, int err)
{
	/*
	*   If the debugger isn't capable of resuming from a run-time error,
	*   trap to the debugger now so that the user can see what happened.
	*   Do not trap to the debugger here if the debugger can resume from
	*   an error; instead, we'll trap in the p-code loop, since we'll be
	*   able to resume execution from the point of the error.
	*
	*   Note that we can't resume from an error when there's no stack
	*   frame, so we'll trap to the debugger here in that case.
	*/
	if (ctx->runcxdbg->dbgcxfcn == 0
		|| !dbgu_err_resume(ctx->runcxdbg))
		dbgssi(ctx->runcxdbg, ctx->runcxlofs, OPCLINE, err, 0);

	/* signal the error */
	errsign(ctx->runcxerr, err, "TADS");
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
