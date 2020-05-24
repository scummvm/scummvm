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

#include "glk/alan2/alan2.h"
#include "glk/alan2/types.h"
#include "glk/alan2/exe.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/inter.h"
#include "glk/alan2/main.h"
#include "glk/alan2/parse.h"
#include "glk/alan2/stack.h"
#include "glk/alan2/decode.h"

namespace Glk {
namespace Alan2 {

#define WIDTH 80

#define N_EVTS 100


/* PUBLIC DATA */

/* The event queue */
EvtqElem eventq[N_EVTS];        /* Event queue */
int etop = 0;                   /* Event queue top pointer */

Boolean looking = FALSE;        /* LOOKING? flag */

int dscrstkp = 0;               /* Describe-stack pointer */


void dscrobjs();
void dscracts();


void print(Aword fpos, Aword len) {
	char str[2 * WIDTH];              // String buffer
	int outlen = 0;                   // Current output length
	int ch = 0;
	int i;
	long savfp = 0;                   // Temporary saved text file position
	static Boolean printFlag = FALSE; // Printing already?
	Boolean savedPrintFlag = printFlag;
	void *info = nullptr;             // Saved decoding info


	if (len == 0) return;

	if (isHere(HERO)) {           /* Check if the player will see it */
		if (printFlag) {            /* Already printing? */
			/* Save current text file position and/or decoding info */
			if (header->pack)
				info = pushDecode();
			else
				savfp = ftell(txtfil);
		}
		printFlag = TRUE;           /* We're printing now! */
		fseek(txtfil, fpos, 0);     /* Position to start of text */
		if (header->pack)
			startDecoding();
		for (outlen = 0; outlen != (int)len; outlen = outlen + strlen(str)) {
			/* Fill the buffer from the beginning */
			for (i = 0; i <= WIDTH || (i > WIDTH && ch != ' '); i++) {
				if (outlen + i == (int)len)  /* No more characters? */
					break;
				if (header->pack)
					ch = decodeChar();
				else
					ch = getc(txtfil);
				if (ch == EOFChar)      /* Or end of text? */
					break;
				str[i] = ch;
			}
			str[i] = '\0';
			output(str);
		}
		/* And restore */
		printFlag = savedPrintFlag;
		if (printFlag) {
			if (header->pack)
				popDecode(info);
			else
				fseek(txtfil, savfp, 0);
		}
	}
}

void sys(Aword fpos, Aword len) {
#ifdef GLK
	::error("system calls aren't supported");
#else
	char *command;

	getstr(fpos, len);            /* Returns address to string on stack */
	command = (char *)pop();
	int tmp = system(command);
	free(command);
#endif
}

void getstr(Aword fpos, Aword len) {
	char *buf = (char *)allocate(len + 1);

	push((Aptr) buf);            /* Push the address to the string */
	fseek(txtfil, fpos, 0);       /* Position to start of text */
	if (header->pack)
		startDecoding();
	while (len--)
		if (header->pack)
			*(buf++) = decodeChar();
		else
			*(buf++) = getc(txtfil);
	*buf = '\0';
}

void score(Aword sc) {
	char buf[80];

	if (sc == 0) {
		prmsg(M_SCORE1);
		sprintf(buf, "%d", cur.score);
		output(buf);
		prmsg(M_SCORE2);
		sprintf(buf, "%ld.", (unsigned long) header->maxscore);
		output(buf);
	} else {
		cur.score += scores[sc - 1];
		scores[sc - 1] = 0;
	}
}

void visits(Aword v) {
	cur.visits = v;
}

Boolean confirm(MsgKind msgno) {
	char buf[80];

	/* This is a bit of a hack since we really want to compare the input,
	   it could be affirmative, but for now any input is NOT! */
	prmsg(msgno);

	if (!readline(buf)) return TRUE;
	col = 1;

	return (buf[0] == '\0');
}

void quit(CONTEXT) {
	char buf[80];

	para();
	while (!g_vm->shouldQuit()) {
		col = 1;
		statusline();
		prmsg(M_QUITACTION);
		if (!readline(buf)) {
			CALL1(terminate, 0)
		}

		if (scumm_stricmp(buf, "restart") == 0) {
			g_vm->setRestart(true);
			LONG_JUMP
		} else if (scumm_stricmp(buf, "restore") == 0) {
			restore();
			LONG_JUMP
		} else if (scumm_stricmp(buf, "quit") == 0) {
			CALL1(terminate, 0)
		}
	}
}

void restart() {
	para();
	if (confirm(M_REALLY)) {
		//longjmp(restart_label, TRUE);
		::error("TODO: restart");
	} else
		return;
	syserr("Fallthrough in RESTART");
}

void cancl(Aword evt) {
	int i;

	for (i = etop - 1; i >= 0; i--)
		if (eventq[i].event == (int)evt) {
			while (i < etop - 1) {
				eventq[i].event = eventq[i + 1].event;
				eventq[i].time = eventq[i + 1].time;
				eventq[i].where = eventq[i + 1].where;
				i++;
			}
			etop--;
			return;
		}
}

void schedule(Aword evt, Aword whr, Aword aft) {
	int i;
	int time;

	cancl(evt);
	/* Check for overflow */
	if (etop == N_EVTS) syserr("Out of event space.");

	time = cur.tick + aft;

	/* Bubble this event down */
	for (i = etop; i >= 1 && eventq[i - 1].time <= time; i--) {
		eventq[i].event = eventq[i - 1].event;
		eventq[i].time = eventq[i - 1].time;
		eventq[i].where = eventq[i - 1].where;
	}

	eventq[i].time = time;
	eventq[i].where = whr;
	eventq[i].event = evt;
	etop++;
}


/*----------------------------------------------------------------------

  getatr()

  Get an attribute value from an attribute list

 */
static Aptr getatr(
    Aaddr atradr,              /* IN - ACODE address to attribute table */
    Aaddr atr                  /* IN - The attribute to read */
) {
	AtrElem *at;

	at = (AtrElem *) addrTo(atradr);
	return at[atr - 1].val;
}


/*----------------------------------------------------------------------

  setatr()

  Set a particular attribute to a value.

 */
static void setatr(
	Aaddr atradr,              /* IN - ACODE address to attribute table */
	Aword atr,                 /* IN - attribute code */
	Aword val                  /* IN - new value */
) {
	AtrElem *at;

	at = (AtrElem *) addrTo(atradr);
	at[atr - 1].val = val;
}


/*----------------------------------------------------------------------

  make()

  */

static void makloc(Aword loc, Aword atr, Aword val) {
	setatr(locs[loc - LOCMIN].atrs, atr, val);
}

static void makobj(Aword obj, Aword atr, Aword val) {
	setatr(objs[obj - OBJMIN].atrs, atr, val);
}

static void makact(Aword act, Aword atr, Aword val) {
	setatr(acts[act - ACTMIN].atrs, atr, val);
}

void make(Aword id, Aword atr, Aword val) {
	char str[80];

	if (isObj(id))
		makobj(id, atr, val);
	else if (isLoc(id))
		makloc(id, atr, val);
	else if (isAct(id))
		makact(id, atr, val);
	else {
		sprintf(str, "Can't MAKE item (%ld).", (unsigned long) id);
		syserr(str);
	}
}


/*----------------------------------------------------------------------------

  set()

 */

static void setloc(Aword loc, Aword atr, Aword val) {
	setatr(locs[loc - LOCMIN].atrs, atr, val);
	locs[loc - LOCMIN].describe = 0;
}

static void setobj(Aword obj, Aword atr, Aword val) {
	setatr(objs[obj - OBJMIN].atrs, atr, val);
}

static void setact(Aword act, Aword atr, Aword val) {
	setatr(acts[act - ACTMIN].atrs, atr, val);
}

void set(Aword id, Aword atr, Aword val) {
	char str[80];

	if (isObj(id))
		setobj(id, atr, val);
	else if (isLoc(id))
		setloc(id, atr, val);
	else if (isAct(id))
		setact(id, atr, val);
	else {
		sprintf(str, "Can't SET item (%ld).", (unsigned long) id);
		syserr(str);
	}
}

void setstr(Aword id, Aword atr, Aword str) {
	free((char *)attribute(id, atr));
	set(id, atr, str);
}



/*-----------------------------------------------------------------------------

  incr/decr

  */

/*----------------------------------------------------------------------

  incratr()

  Increment a particular attribute by a value.

 */
static void incratr(
    Aaddr atradr,           /* IN - ACODE address to attribute table */
    Aword atr,              /* IN - attribute code */
    Aword step              /* IN - step to increment by */
) {
	AtrElem *at;

	at = (AtrElem *) addrTo(atradr);
	at[atr - 1].val += step;
}

static void incrloc(Aword loc, Aword atr, Aword step) {
	incratr(locs[loc - LOCMIN].atrs, atr, step);
	locs[loc - LOCMIN].describe = 0;
}

static void incrobj(Aword obj, Aword atr, Aword step) {
	incratr(objs[obj - OBJMIN].atrs, atr, step);
}

static void incract(Aword act, Aword atr, Aword step) {
	incratr(acts[act - ACTMIN].atrs, atr, step);
}

void incr(Aword id, Aword atr, Aword step) {
	char str[80];

	if (isObj(id))
		incrobj(id, atr, step);
	else if (isLoc(id))
		incrloc(id, atr, step);
	else if (isAct(id))
		incract(id, atr, step);
	else {
		sprintf(str, "Can't INCR item (%ld).", (unsigned long) id);
		syserr(str);
	}
}

void decr(Aword id, Aword atr, Aword step) {
	char str[80];

	// TODO: Original did explicit negation on an unsigned value. Make sure that the
	// casts added to ignore the warnings are okay
	if (isObj(id))
		incrobj(id, atr, static_cast<uint>(-(int)step));
	else if (isLoc(id))
		incrloc(id, atr, static_cast<uint>(-(int)step));
	else if (isAct(id))
		incract(id, atr, static_cast<uint>(-(int)step));
	else {
		sprintf(str, "Can't DECR item (%ld).", (unsigned long) id);
		syserr(str);
	}
}


/*----------------------------------------------------------------------

  attribute()

  */

static Aptr locatr(Aword loc, Aword atr) {
	return getatr(locs[loc - LOCMIN].atrs, atr);
}

static Aptr objatr(Aword obj, Aword atr) {
	return getatr(objs[obj - OBJMIN].atrs, atr);
}

static Aptr actatr(Aword act, Aword atr) {
	return getatr(acts[act - ACTMIN].atrs, atr);
}

static Aptr litatr(Aword lit, Aword atr) {
	char str[80];

	if (atr == 1)
		return litValues[lit - LITMIN].value;
	else {
		sprintf(str, "Unknown attribute for literal (%ld).", (unsigned long) atr);
		syserr(str);
	}
	return (Aptr)EOD;
}

Aptr attribute(Aword id, Aword atr) {
	char str[80];

	if (isObj(id))
		return objatr(id, atr);
	else if (isLoc(id))
		return locatr(id, atr);
	else if (isAct(id))
		return actatr(id, atr);
	else if (isLit(id))
		return litatr(id, atr);
	else {
		sprintf(str, "Can't ATTRIBUTE item (%ld).", (unsigned long) id);
		syserr(str);
	}
	return (Aptr)EOD;
}

Aptr strattr(Aword id, Aword atr) {
	return (Aptr)scumm_strdup((char *)attribute(id, atr));
}


/*----------------------------------------------------------------------

  where()

  */

static Aword objloc(Aword obj) {
	if (isCnt(objs[obj - OBJMIN].loc)) /* In something ? */
		if (isObj(objs[obj - OBJMIN].loc) || isAct(objs[obj - OBJMIN].loc))
			return (where(objs[obj - OBJMIN].loc));
		else /* Containers not anywhere is where the hero is! */
			return (where(HERO));
	else
		return (objs[obj - OBJMIN].loc);
}

static Aword actloc(Aword act) {
	return (acts[act - ACTMIN].loc);
}

Aword where(Aword id) {
	char str[80];

	if (isObj(id))
		return objloc(id);
	else if (isAct(id))
		return actloc(id);
	else {
		sprintf(str, "Can't WHERE item (%ld).", (unsigned long) id);
		syserr(str);
	}
	return (Aptr)EOD;
}


/*----------------------------------------------------------------------

  aggregates

  */

Aint agrmax(Aword atr, Aword whr) {
	Aword i;
	Aint max = 0;

	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (isLoc(whr)) {
			if (where(i) == whr && (int)attribute(i, atr) > max)
				max = attribute(i, atr);
		} else if (objs[i - OBJMIN].loc == whr && (int)attribute(i, atr) > max)
			max = attribute(i, atr);
	}
	return (max);
}

Aint agrsum(Aword atr, Aword whr) {
	Aword i;
	Aint sum = 0;

	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (isLoc(whr)) {
			if (where(i) == whr)
				sum += attribute(i, atr);
		} else if (objs[i - OBJMIN].loc == whr)
			sum += attribute(i, atr);
	}
	return (sum);
}

Aint agrcount(Aword whr) {
	Aword i;
	Aword count = 0;

	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (isLoc(whr)) {
			if (where(i) == whr)
				count++;
		} else if (objs[i - OBJMIN].loc == whr)
			count++;
	}
	return (count);
}


/*----------------------------------------------------------------------

  locate()

  */

static void locobj(Aword obj, Aword whr) {
	if (isCnt(whr)) { /* Into a container */
		if (whr == obj)
			syserr("Locating something inside itself.");
		if (checklim(whr, obj))
			return;
		else
			objs[obj - OBJMIN].loc = whr;
	} else {
		objs[obj - OBJMIN].loc = whr;
		/* Make sure the location is described since it's changed */
		locs[whr - LOCMIN].describe = 0;
	}
}

static void locact(Aword act, Aword whr) {
	Aword prevact = cur.act;
	Aword prevloc = cur.loc;

	cur.loc = whr;
	acts[act - ACTMIN].loc = whr;
	if (act == HERO) {
		if (locs[acts[act - ACTMIN].loc - LOCMIN].describe % (cur.visits + 1) == 0)
			look();
		else {
			if (anyOutput)
				para();
			say(where(HERO));
			prmsg(M_AGAIN);
			newline();
			dscrobjs();
			dscracts();
		}
		locs[where(HERO) - LOCMIN].describe++;
		locs[where(HERO) - LOCMIN].describe %= (cur.visits + 1);
	} else
		locs[whr - LOCMIN].describe = 0;
	if (locs[cur.loc - LOCMIN].does != 0) {
		cur.act = act;
		interpret(locs[cur.loc - LOCMIN].does);
		cur.act = prevact;
	}

	if (cur.act != (int)act)
		cur.loc = prevloc;
}

void locate(Aword id, Aword whr) {
	char str[80];

	if (isObj(id))
		locobj(id, whr);
	else if (isAct(id))
		locact(id, whr);
	else {
		sprintf(str, "Can't LOCATE item (%ld).", (unsigned long) id);
		syserr(str);
	}
}


/*----------------------------------------------------------------------

  isHere()

  */

static Abool objhere(Aword obj) {
	if (isCnt(objs[obj - OBJMIN].loc)) {  /* In something? */
		if (isObj(objs[obj - OBJMIN].loc) || isAct(objs[obj - OBJMIN].loc))
			return (isHere(objs[obj - OBJMIN].loc));
		else /* If the container wasn't anywhere, assume where HERO is! */
			return ((int)where(HERO) == cur.loc);
	} else {
		return (int)(objs[obj - OBJMIN].loc) == cur.loc;
	}
}

static Aword acthere(Aword act) {
	return (int)(acts[act - ACTMIN].loc) == cur.loc;
}

Abool isHere(Aword id) {
	char str[80];

	if (isObj(id))
		return objhere(id);
	else if (isAct(id))
		return acthere(id);
	else {
		sprintf(str, "Can't HERE item (%ld).", (unsigned long) id);
		syserr(str);
	}
	return (Abool)EOD;
}

/*----------------------------------------------------------------------

  isNear()

  */

static Aword objnear(Aword obj) {
	if (isCnt(objs[obj - OBJMIN].loc)) {  /* In something? */
		if (isObj(objs[obj - OBJMIN].loc) || isAct(objs[obj - OBJMIN].loc))
			return (isNear(objs[obj - OBJMIN].loc));
		else  /* If the container wasn't anywhere, assume here, so not nearby! */
			return (FALSE);
	} else
		return (exitto(where(obj), cur.loc));
}

static Aword actnear(Aword act) {
	return (exitto(where(act), cur.loc));
}

Abool isNear(Aword id) {
	char str[80];

	if (isObj(id))
		return objnear(id);
	else if (isAct(id))
		return actnear(id);
	else {
		sprintf(str, "Can't NEAR item (%ld).", (unsigned long) id);
		syserr(str);
	}
	return (Abool)EOD;
}


/*----------------------------------------------------------------------

  in()

  */

Abool in(Aword obj, Aword cnt) {
	if (!isObj(obj))
		return (FALSE);
	if (!isCnt(cnt))
		syserr("IN in a non-container.");

	return (objs[obj - OBJMIN].loc == cnt);
}


/*----------------------------------------------------------------------

  say()

  */

static void sayloc(Aword loc) {
	interpret(locs[loc - LOCMIN].nams);
}

static void sayobj(Aword obj) {
	interpret(objs[obj - OBJMIN].dscr2);
}

static void sayact(Aword act) {
	interpret(acts[act - ACTMIN].nam);
}

void sayint(Aword val) {
	char buf[25];

	if (isHere(HERO)) {
		sprintf(buf, "%ld", (unsigned long) val);
		output(buf);
	}
}

void saystr(char *str) {
	if (isHere(HERO))
		output(str);
	free(str);
}

static void saylit(Aword lit) {
	char *str;

	if (isNum(lit))
		sayint(litValues[lit - LITMIN].value);
	else {
		str = (char *)scumm_strdup((char *)litValues[lit - LITMIN].value);
		saystr(str);
	}
}

void sayarticle(Aword id) {
	if (!isObj(id))
		syserr("Trying to say article of something *not* an object.");
	if (objs[id - OBJMIN].art != 0)
		interpret(objs[id - OBJMIN].art);
	else
		prmsg(M_ARTICLE);
}

void say(Aword id) {
	char str[80];

	if (isHere(HERO)) {
		if (isObj(id))
			sayobj(id);
		else if (isLoc(id))
			sayloc(id);
		else if (isAct(id))
			sayact(id);
		else if (isLit(id))
			saylit(id);
		else {
			sprintf(str, "Can't SAY item (%ld).", (unsigned long) id);
			syserr(str);
		}
	}
}


/*----------------------------------------------------------------------

  describe()

  */

static void dscrloc(Aword loc) {
	if (locs[loc - LOCMIN].dscr != 0)
		interpret(locs[loc - LOCMIN].dscr);
}

static void dscrobj(Aword obj) {
	objs[obj - OBJMIN].describe = FALSE;
	if (objs[obj - OBJMIN].dscr1 != 0)
		interpret(objs[obj - OBJMIN].dscr1);
	else {
		prmsg(M_SEEOBJ1);
		sayarticle(obj);
		say(obj);
		prmsg(M_SEEOBJ4);
		if (objs[obj - OBJMIN].cont != 0)
			list(obj);
	}
}

static void dscract(Aword act) {
	ScrElem *scr = NULL;

	if (acts[act - ACTMIN].script != 0) {
		for (scr = (ScrElem *) addrTo(acts[act - ACTMIN].scradr); !endOfTable(scr); scr++)
			if (scr->code == acts[act - ACTMIN].script)
				break;
		if (endOfTable(scr)) scr = NULL;
	}
	if (scr != NULL && scr->dscr != 0)
		interpret(scr->dscr);
	else if (acts[act - ACTMIN].dscr != 0)
		interpret(acts[act - ACTMIN].dscr);
	else {
		interpret(acts[act - ACTMIN].nam);
		prmsg(M_SEEACT);
	}
	acts[act - ACTMIN].describe = FALSE;
}


static Aword dscrstk[255];

void describe(Aword id) {
	int i;
	char str[80];

	for (i = 0; i < dscrstkp; i++)
		if (dscrstk[i] == id)
			syserr("Recursive DESCRIBE.");
	dscrstk[dscrstkp++] = id;

	if (isObj(id))
		dscrobj(id);
	else if (isLoc(id))
		dscrloc(id);
	else if (isAct(id))
		dscract(id);
	else {
		sprintf(str, "Can't DESCRIBE item (%ld).", (unsigned long) id);
		syserr(str);
	}

	dscrstkp--;
}


/*----------------------------------------------------------------------

  use()

  */

void use(Aword act, Aword scr) {
	char str[80];

	if (!isAct(act)) {
		sprintf(str, "Item is not an Actor (%ld).", (unsigned long) act);
		syserr(str);
	}

	acts[act - ACTMIN].script = scr;
	acts[act - ACTMIN].step = 0;
}


/*----------------------------------------------------------------------

  list()

  */

void list(Aword cnt) {
	uint i;
	Aword props;
	Aword prevobj = 0;
	Boolean found = FALSE;
	Boolean multiple = FALSE;

	/* Find container properties */
	if (isObj(cnt))
		props = objs[cnt - OBJMIN].cont;
	else if (isAct(cnt))
		props = acts[cnt - ACTMIN].cont;
	else
		props = cnt;

	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (in(i, cnt)) { /* Yes, it's in this container */
			if (!found) {
				found = TRUE;
				if (cnts[props - CNTMIN].header != 0)
					interpret(cnts[props - CNTMIN].header);
				else {
					prmsg(M_CONTAINS1);
					if (cnts[props - CNTMIN].nam != 0) /* It has it's own name */
						interpret(cnts[props - CNTMIN].nam);
					else
						say(cnts[props - CNTMIN].parent); /* It is actually an object or actor */
					prmsg(M_CONTAINS2);
				}
			} else {
				if (multiple) {
					needsp = FALSE;
					prmsg(M_CONTAINS3);
				}
				multiple = TRUE;
				sayarticle(prevobj);
				say(prevobj);
			}
			prevobj = i;
		}
	}

	if (found) {
		if (multiple)
			prmsg(M_CONTAINS4);
		sayarticle(prevobj);
		say(prevobj);
		prmsg(M_CONTAINS5);
	} else {
		if (cnts[props - CNTMIN].empty != 0)
			interpret(cnts[props - CNTMIN].empty);
		else {
			prmsg(M_EMPTY1);
			if (cnts[props - CNTMIN].nam != 0) /* It has it's own name */
				interpret(cnts[props - CNTMIN].nam);
			else
				say(cnts[props - CNTMIN].parent); /* It is actually an actor or object */
			prmsg(M_EMPTY2);
		}
	}
	needsp = TRUE;
}


/*----------------------------------------------------------------------

  empty()

  */

void empty(Aword cnt, Aword whr) {
	for (uint i = OBJMIN; i <= OBJMAX; i++)
		if (in(i, cnt))
			locate(i, whr);
}


/*----------------------------------------------------------------------*\

  Description of current location

  dscrobjs()
  dscracts()
  look()

\*----------------------------------------------------------------------*/

void dscrobjs() {
	uint i;
	int prevobj = 0;
	Boolean found = FALSE;
	Boolean multiple = FALSE;

	/* First describe everything here with its own description */
	for (i = OBJMIN; i <= OBJMAX; i++)
		if ((int)objs[i - OBJMIN].loc == cur.loc &&
		        objs[i - OBJMIN].describe &&
		        objs[i - OBJMIN].dscr1)
			describe(i);

	/* Then list everything else here */
	for (i = OBJMIN; i <= OBJMAX; i++)
		if ((int)objs[i - OBJMIN].loc == cur.loc && objs[i - OBJMIN].describe) {
			if (!found) {
				prmsg(M_SEEOBJ1);
				sayarticle(i);
				say(i);
				found = TRUE;
			} else {
				if (multiple) {
					needsp = FALSE;
					prmsg(M_SEEOBJ2);
					sayarticle(prevobj);
					say(prevobj);
				}
				multiple = TRUE;
			}
			prevobj = i;
		}

	if (found) {
		if (multiple) {
			prmsg(M_SEEOBJ3);
			sayarticle(prevobj);
			say(prevobj);
		}
		prmsg(M_SEEOBJ4);
	}

	/* Set describe flag for all objects */
	for (i = OBJMIN; i <= OBJMAX; i++)
		objs[i - OBJMIN].describe = TRUE;
}

void dscracts() {
	uint i;

	for (i = HERO + 1; i <= ACTMAX; i++)
		if ((int)acts[i - ACTMIN].loc == cur.loc && acts[i - ACTMIN].describe)
			describe(i);

	/* Set describe flag for all actors */
	for (i = HERO; i <= ACTMAX; i++)
		acts[i - ACTMIN].describe = TRUE;
}

void look() {
	uint i;

	if (looking)
		syserr("Recursive LOOK.");

	looking = TRUE;
	/* Set describe flag for all objects and actors */
	for (i = OBJMIN; i <= OBJMAX; i++)
		objs[i - OBJMIN].describe = TRUE;
	for (i = ACTMIN; i <= ACTMAX; i++)
		acts[i - ACTMIN].describe = TRUE;

	if (anyOutput)
		para();

	g_vm->glk_set_style(style_Subheader);
	needsp = FALSE;
	say(cur.loc);
	needsp = FALSE;
	output(".");
	g_vm->glk_set_style(style_Normal);
	newline();
	needsp = FALSE;
	describe(cur.loc);
	dscrobjs();
	dscracts();
	looking = FALSE;
}


/*----------------------------------------------------------------------

  save()

  */

void save() {
	(void)g_vm->saveGame();
}


/*----------------------------------------------------------------------

  restore()

  */

void restore() {
	(void)g_vm->loadGame();
}


/*----------------------------------------------------------------------

  rnd()

  */

Aword rnd(Aword from, Aword to) {
	if (to == from)
		return to;
	else if (to > from)
		return (g_vm->getRandomNumber(0x7fffffff) / 10) % (to - from + 1) + from;
	else
		return (g_vm->getRandomNumber(0x7fffffff) / 10) % (from - to + 1) + to;
}

/*----------------------------------------------------------------------

  btw()

  BETWEEN

  */

Abool btw(Aint val, Aint low, Aint high) {
	if (high > low)
		return low <= val && val <= high;
	else
		return high <= val && val <= low;
}



/*----------------------------------------------------------------------

  contains()

  */

Aword contains(Aptr string, Aptr substring) {
	Abool found;

	strlow((char *)string);
	strlow((char *)substring);

	found = (strstr((char *)string, (char *)substring) != 0);

	free((char *)string);
	free((char *)substring);

	return (found);
}


/*----------------------------------------------------------------------

  streq()

  Compare two strings approximately, ignore case

  */
Abool streq(char a[], char b[]) {
	Boolean eq;

	strlow(a);
	strlow(b);

	eq = (strcmp(a, b) == 0);

	free(a);
	free(b);

	return (eq);
}

} // End of namespace Alan2
} // End of namespace Glk
