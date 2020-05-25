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

#define V27COMPATIBLE

#include "glk/alan2/alan2.h"
#include "glk/alan2/alan_version.h"
#include "glk/alan2/args.h"
#include "glk/alan2/debug.h"
#include "glk/alan2/exe.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/inter.h"
#include "glk/alan2/main.h"
#include "glk/alan2/parse.h"
#include "glk/alan2/reverse.h"
#include "glk/alan2/rules.h"
#include "glk/alan2/stack.h"
#include "glk/alan2/sysdep.h"
#include "glk/alan2/types.h"
#include "common/file.h"

namespace Glk {
namespace Alan2 {

/* PUBLIC DATA */

/* The Amachine memory */
Aword *memory;
//static AcdHdr dummyHeader;    // Dummy to use until memory allocated
AcdHdr *header;

Aaddr memTop;           // Top of load memory

int conjWord;           // First conjunction in dictonary, for ','


/* Amachine variables */
CurVars cur;

/* Amachine structures */
WrdElem *dict;          /* Dictionary pointer */
ActElem *acts;          /* Actor table pointer */
LocElem *locs;          /* Location table pointer */
VrbElem *vrbs;          /* Verb table pointer */
StxElem *stxs;          /* Syntax table pointer */
ObjElem *objs;          /* Object table pointer */
CntElem *cnts;          /* Container table pointer */
RulElem *ruls;          /* Rule table pointer */
EvtElem *evts;          /* Event table pointer */
MsgElem *msgs;          /* Message table pointer */
Aword *scores;          /* Score table pointer */
Aword *freq;            /* Cumulative character frequencies */

int dictsize;

Boolean verbose = FALSE;
Boolean errflg = TRUE;
Boolean trcflg = FALSE;
Boolean dbgflg = FALSE;
Boolean stpflg = FALSE;
Boolean logflg = FALSE;
Boolean statusflg = TRUE;
Boolean fail = FALSE;
Boolean anyOutput = FALSE;


/* The files and filenames */
const char *advnam;
Common::File *txtfil;
Common::WriteStream *logfil;


/* Screen formatting info */
int col, lin;

Boolean needsp = FALSE;
Boolean skipsp = FALSE;

/*======================================================================

  terminate()

  Terminate the execution of the adventure, e.g. close windows,
  return buffers...

 */
void terminate(CONTEXT, int code) {
	newline();

	g_vm->glk_exit();
	LONG_JUMP
}

/*======================================================================

  usage()

  */
void usage() {
	printf("Usage:\n\n");
	printf("    %s [<switches>] <adventure>\n\n", PROGNAME);
	printf("where the possible optional switches are:\n");
	g_vm->glk_set_style(style_Preformatted);

	printf("    -v    verbose mode\n");
	printf("    -l    log player commands and game output to a file\n");
	printf("    -i    ignore version and checksum errors\n");
	printf("    -n    no Status Line\n");
	printf("    -d    enter debug mode\n");
	printf("    -t    trace game execution\n");
	printf("    -s    single instruction trace\n");
	g_vm->glk_set_style(style_Normal);
}


/*======================================================================

  syserr()

  Print a little text blaming the user for the system error.

 */
void syserr(const char *str) {
	::error("%s", str);
}


/*======================================================================

  error()

  Print an error message, force new player input and abort.
  */
 /* IN - The error message number */
void error(CONTEXT, MsgKind msgno) {
	if (msgno != MSGMAX)
		prmsg(msgno);
	wrds[wrdidx] = EOD;       /* Force new player input */
	dscrstkp = 0;         /* Reset describe stack */

	LONG_JUMP
}


/*======================================================================

  statusline()

  Print the the status line on the top of the screen.

  */
void statusline() {
	uint glkWidth;
	char line[100];
	int pcol = col;
	uint i;

	if (NULL == glkStatusWin)
		return;

	g_vm->glk_set_window(glkStatusWin);
	g_vm->glk_window_clear(glkStatusWin);
	g_vm->glk_window_get_size(glkStatusWin, &glkWidth, NULL);

	g_vm->glk_set_style(style_User1);
	for (i = 0; i < glkWidth; i++)
		g_vm->glk_put_char(' ');

	col = 1;
	g_vm->glk_window_move_cursor(glkStatusWin, 1, 0);
	needsp = FALSE;
	say(where(HERO));
	if (header->maxscore > 0)
		sprintf(line, "Score %d(%d)/%d moves", cur.score, (int)header->maxscore, cur.tick);
	else
		sprintf(line, "%d moves", cur.tick);
	g_vm->glk_window_move_cursor(glkStatusWin, glkWidth - col - strlen(line), 0);
	printf(line);
	needsp = FALSE;

	col = pcol;

	g_vm->glk_set_window(glkMainWin);
}


/*======================================================================

  logprint()

  Print some text and log it if logging is on.

 */
static void logprint(const char str[]) {
	printf(str);
	if (logflg)
		fprintf(logfil, "%s", str);
}


/*======================================================================

  newline()

  Make a newline, but check for screen full.

 */
void newline() {
	g_vm->glk_put_char('\n');
}


/*======================================================================

  para()

  Make a new paragraph, i.e one empty line (one or two newlines).

 */
void para() {
	if (col != 1)
		newline();
	newline();
}


/*======================================================================

  clear()

  Clear the screen.

 */
void clear() {
	g_vm->glk_window_clear(glkMainWin);
}


/*======================================================================

  allocate()

  Safely allocate new memory.

*/
void *allocate(unsigned long len /* IN - Length to allocate */) {
	void *p = (void *)malloc((size_t)len);

	if (p == NULL)
		syserr("Out of memory.");

	return p;
}


/*----------------------------------------------------------------------

  just()

  Justify a string so that it wraps at end of screen.

 */
static void just(const char str[]) {
	logprint(str);
}


/*----------------------------------------------------------------------

  space()

  Output a space if needed.

 */
static void space() {
	if (skipsp)
		skipsp = FALSE;
	else {
		if (needsp) {
			logprint(" ");
			col++;
		}
	}
	needsp = FALSE;
}


/*----------------------------------------------------------------------

  sayparam()

  A parameter needs to be said, check for words the player used and use
  them if possible.

*/
static void sayparam(int p) {
	for (int i = 0; i <= p; i++) {
		if (params[i].code == EOD)
			syserr("Nonexistent parameter referenced.");
	}

	// Any words he used?
	if (params[p].firstWord == EOD) {
		say(params[p].code);
	} else {
		// Yes, so use them...
		for (uint i = params[p].firstWord; i <= params[p].lastWord; i++) {
			just((char *)addrTo(dict[wrds[i]].wrd));
			if (i < params[p].lastWord)
				just(" ");
		}
	}
}


/*----------------------------------------------------------------------

  prsym()

  Print an expanded symbolic reference.

  N = newline
  I = indent on a new line
  P = new paragraph
  L = current location name
  O = current object -> first parameter!
  V = current verb
  A = current actor
  T = tabulation
  $ = no space needed after this
 */
static void prsym(
    char *str   /* IN - The string starting with '$' */
) {
	switch (toLower(str[1])) {
	case 'n':
		newline();
		needsp = FALSE;
		break;
	case 'i':
		newline();
		logprint("    ");
		col = 5;
		needsp = FALSE;
		break;
	case 'o':
		sayparam(0);
		needsp = TRUE;      /* We did print something non-white */
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		sayparam(str[1] - '1');
		needsp = TRUE;      /* We did print something non-white */
		break;
	case 'l':
		say(cur.loc);
		needsp = TRUE;      /* We did print something non-white */
		break;
	case 'a':
		say(cur.act);
		needsp = TRUE;      /* We did print something non-white */
		break;
	case 'v':
		just((char *)addrTo(dict[vrbwrd].wrd));
		needsp = TRUE;      /* We did print something non-white */
		break;
	case 'p':
		para();
		needsp = FALSE;
		break;
	case 't': {
		int i;
		int spaces = 4 - (col - 1) % 4;

		for (i = 0; i < spaces; i++) logprint(" ");
		col = col + spaces;
		needsp = FALSE;
		break;
	}
	case '$':
		skipsp = TRUE;
		break;
	default:
		logprint("$");
		break;
	}
}



/*======================================================================

  output()

  Output a string to suit the screen. Any symbolic inserts ('$') are
  recogniced and performed.

 */
void output(const char original[]) {
	char ch;
	char *str, *copy;
	char *symptr;

	copy = scumm_strdup(original);
	str = copy;

	if (str[0] != '$' || str[1] != '$')
		space();            /* Output space if needed (& not inhibited) */

	while ((symptr = strchr(str, '$')) != (char *) NULL) {
		ch = *symptr;       /* Terminate before symbol */
		*symptr = '\0';
		if (strlen(str) > 0) {
			just(str);        /* Output part before '$' */
			if (str[strlen(str) - 1] == ' ')
				needsp = FALSE;
		}
		*symptr = ch;       /* restore '$' */
		prsym(symptr);      /* Print the symbolic reference */
		str = &symptr[2];       /* Advance to after symbol and continue */
	}
	if (str[0] != 0) {
		just(str);          /* Output trailing part */
		skipsp = FALSE;
		if (str[strlen(str) - 1] != ' ')
			needsp = TRUE;
	}
	anyOutput = TRUE;
	free(copy);
}


/*======================================================================

  prmsg()

  Print a message from the message table.

  */
void prmsg(MsgKind msg /* IN - message number */) {
	interpret(msgs[msg].stms);
}


/*----------------------------------------------------------------------*\

  Various check functions

  endOfTable()
  isObj, isLoc, isAct, IsCnt & isNum

\*----------------------------------------------------------------------*/

/* How to know we are at end of a table */
Boolean eot(Aword *adr) {
	return *adr == EOD;
}

Boolean isObj(Aword x) {
	return x >= OBJMIN && x <= OBJMAX;
}

Boolean isCnt(Aword x) {
	return (x >= CNTMIN && x <= CNTMAX) ||
	       (isObj(x) && objs[x - OBJMIN].cont != 0) ||
	       (isAct(x) && acts[x - ACTMIN].cont != 0);
}

Boolean isAct(Aword x) {
	return x >= ACTMIN && x <= ACTMAX;
}

Boolean isLoc(Aword x) {
	return x >= LOCMIN && x <= LOCMAX;
}

Boolean isNum(Aword x) {
	return x >= LITMIN && x <= LITMAX && litValues[x - LITMIN].type == TYPNUM;
}

Boolean isStr(Aword x) {
	return x >= LITMIN && x <= LITMAX && litValues[x - LITMIN].type == TYPSTR;
}

Boolean isLit(Aword x) {
	return x >= LITMIN && x <= LITMAX;
}


/*======================================================================

  exitto()

  Is there an exit from one location to another?

  */
Boolean exitto(int to, int from) {
	ExtElem *ext;

	if (locs[from - LOCMIN].exts == 0)
		return (FALSE); /* No exits */

	for (ext = (ExtElem *) addrTo(locs[from - LOCMIN].exts); !endOfTable(ext); ext++)
		if ((int)ext->next == to)
			return (TRUE);

	return (FALSE);
}


/*----------------------------------------------------------------------
  count()

  Count the number of items in a container.

  */
static int count(int cnt /* IN - the container to count */) {
	int j = 0;

	for (uint i = OBJMIN; i <= OBJMAX; i++)
		if (in(i, cnt))
			/* Then it's in this container also */
			j++;
	return j;
}


/*----------------------------------------------------------------------
  sumatr()

  Sum the values of one attribute in a container. Recursively.

  */
static int sumatr(
    Aword atr,         /* IN - the attribute to sum over */
    Aword cnt          /* IN - the container to sum */
) {
	uint i;
	int sum = 0;

	for (i = OBJMIN; i <= OBJMAX; i++)
		if (objs[i - OBJMIN].loc == cnt) {  /* Then it's in this container */
			if (objs[i - OBJMIN].cont != 0) /* This is also a container! */
				sum = sum + sumatr(atr, i);
			sum = sum + attribute(i, atr);
		}
	return (sum);
}


/*======================================================================
  checklim()

  Checks if a limit for a container is exceeded.

  */
Boolean checklim(
    Aword cnt,         /* IN - Container code */
    Aword obj          /* IN - The object to add */
) {
	LimElem *lim;
	Aword props;

	fail = TRUE;
	if (!isCnt(cnt))
		syserr("Checking limits for a non-container.");

	/* Find the container properties */
	if (isObj(cnt))
		props = objs[cnt - OBJMIN].cont;
	else if (isAct(cnt))
		props = acts[cnt - ACTMIN].cont;
	else
		props = cnt;

	if (cnts[props - CNTMIN].lims != 0) { /* Any limits at all? */
		for (lim = (LimElem *) addrTo(cnts[props - CNTMIN].lims); !endOfTable(lim); lim++)
			if (lim->atr == 0) {
				if (count(cnt) >= (int)lim->val) {
					interpret(lim->stms);
					return (TRUE);    /* Limit check failed */
				}
			} else {
				if (sumatr(lim->atr, cnt) + attribute(obj, lim->atr) > lim->val) {
					interpret(lim->stms);
					return (TRUE);
				}
			}
	}
	fail = FALSE;
	return (FALSE);
}


/*----------------------------------------------------------------------*\

  Action routines

\*----------------------------------------------------------------------*/



/*----------------------------------------------------------------------
  trycheck()

  Tries a check, returns TRUE if it passed, FALSE else.

  */
static Boolean trycheck(
    Aaddr adr,         /* IN - ACODE address to check table */
    Boolean act        /* IN - Act if it fails ? */
) {
	ChkElem *chk;

	chk = (ChkElem *) addrTo(adr);
	if (chk->exp == 0) {
		interpret(chk->stms);
		return (FALSE);
	} else {
		while (!endOfTable(chk)) {
			interpret(chk->exp);
			if (!(Abool)pop()) {
				if (act)
					interpret(chk->stms);
				return (FALSE);
			}
			chk++;
		}
		return (TRUE);
	}
}


/*======================================================================
  go()

  Move hero in a direction.

  */
void go(CONTEXT, int dir) {
	ExtElem *ext;
	Boolean ok;
	Aword oldloc;

	ext = (ExtElem *) addrTo(locs[cur.loc - LOCMIN].exts);
	if (locs[cur.loc - LOCMIN].exts != 0) {
		while (!endOfTable(ext)) {
			if ((int)ext->code == dir) {
				ok = TRUE;
				if (ext->checks != 0) {
					if (trcflg) {
						printf("\n<EXIT %d (%s) from %d (", dir,
						       (char *)addrTo(dict[wrds[wrdidx - 1]].wrd), cur.loc);
						debugsay(cur.loc);
						printf("), Checking:>\n");
					}
					ok = trycheck(ext->checks, TRUE);
				}
				if (ok) {
					oldloc = cur.loc;
					if (ext->action != 0) {
						if (trcflg) {
							printf("\n<EXIT %d (%s) from %d (", dir,
							       (char *)addrTo(dict[wrds[wrdidx - 1]].wrd), cur.loc);
							debugsay(cur.loc);
							printf("), Executing:>\n");
						}
						interpret(ext->action);
					}
					/* Still at the same place? */
					if (where(HERO) == oldloc) {
						if (trcflg) {
							printf("\n<EXIT %d (%s) from %d (", dir,
							       (char *)addrTo(dict[wrds[wrdidx - 1]].wrd), cur.loc);
							debugsay(cur.loc);
							printf("), Moving:>\n");
						}
						locate(HERO, ext->next);
					}
				}
				return;
			}
			ext++;
		}
	}

	CALL1(error, M_NO_WAY)
}


/*----------------------------------------------------------------------

  findalt()

  Find the verb alternative wanted in a verb list and return
  the address to it.

 */
static AltElem *findalt(
    Aword vrbsadr,     /* IN - Address to start of list */
    Aword param        /* IN - Which parameter to match */
) {
	VrbElem *vrb;
	AltElem *alt;

	if (vrbsadr == 0)
		return (NULL);

	for (vrb = (VrbElem *) addrTo(vrbsadr); !endOfTable(vrb); vrb++)
		if ((int)vrb->code == cur.vrb) {
			for (alt = (AltElem *) addrTo(vrb->alts); !endOfTable(alt); alt++)
				if (alt->param == param || alt->param == 0)
					return alt;
			return NULL;
		}
	return NULL;
}


/*======================================================================

  possible()

  Check if current action is possible according to the CHECKs.

  */
Boolean possible() {
	AltElem *alt[MAXPARAMS + 2];  /* List of alt-pointers, one for each param */
	int i;            /* Parameter index */

	fail = FALSE;
	alt[0] = findalt(header->vrbs, 0);
	/* Perform global checks */
	if (alt[0] != 0 && alt[0]->checks != 0) {
		if (!trycheck(alt[0]->checks, FALSE)) return FALSE;
		if (fail) return FALSE;
	}

	/* Now CHECKs in this location */
	alt[1] = findalt(locs[cur.loc - LOCMIN].vrbs, 0);
	if (alt[1] != 0 && alt[1]->checks != 0)
		if (!trycheck(alt[1]->checks, FALSE))
			return FALSE;

	for (i = 0; params[i].code != EOD; i++) {
		alt[i + 2] = findalt(objs[params[i].code - OBJMIN].vrbs, i + 1);
		/* CHECKs in a possible parameter */
		if (alt[i + 2] != 0 && alt[i + 2]->checks != 0)
			if (!trycheck(alt[i + 2]->checks, FALSE))
				return FALSE;
	}

	for (i = 0; i < 2 || params[i - 2].code != EOD; i++)
		if (alt[i] != 0 && alt[i]->action != 0)
			break;
	if (i >= 2 && params[i - 2].code == EOD)
		/* Didn't find any code for this verb/object combination */
		return FALSE;
	else
		return TRUE;
}


/*----------------------------------------------------------------------

  do_it()

  Execute the action commanded by hero.

  */
static void do_it(CONTEXT) {
	AltElem *alt[MAXPARAMS + 2];  /* List of alt-pointers, one for each param */
	Boolean done[MAXPARAMS + 2];  /* Is it done */
	int i;            /* Parameter index */
	char trace[80];       /* Trace string buffer */

	fail = FALSE;
	alt[0] = findalt(header->vrbs, 0);
	/* Perform global checks */
	if (alt[0] != 0 && alt[0]->checks != 0) {
		if (trcflg)
			printf("\n<VERB %d, CHECK, GLOBAL:>\n", cur.vrb);
		if (!trycheck(alt[0]->checks, TRUE)) return;
		if (fail) return;
	}

	/* Now CHECKs in this location */
	alt[1] = findalt(locs[cur.loc - LOCMIN].vrbs, 0);
	if (alt[1] != 0 && alt[1]->checks != 0) {
		if (trcflg)
			printf("\n<VERB %d, CHECK, in LOCATION:>\n", cur.vrb);
		if (!trycheck(alt[1]->checks, TRUE)) return;
		if (fail) return;
	}

	for (i = 0; params[i].code != EOD; i++) {
		if (isLit(params[i].code))
			alt[i + 2] = 0;
		else {
			if (isObj(params[i].code))
				alt[i + 2] = findalt(objs[params[i].code - OBJMIN].vrbs, i + 1);
			else if (isAct(params[i].code))
				alt[i + 2] = findalt(acts[params[i].code - ACTMIN].vrbs, i + 1);
			else
				syserr("Illegal parameter type.");
			/* CHECKs in the parameters */
			if (alt[i + 2] != 0 && alt[i + 2]->checks != 0) {
				if (trcflg)
					printf("\n<VERB %d, CHECK, in Parameter #%d:>\n", cur.vrb, i);
				if (!trycheck(alt[i + 2]->checks, TRUE)) return;
				if (fail) return;
			}
		}
	}

	/* Check for anything to execute... */
	for (i = 0; i < 2 || params[i - 2].code != EOD; i++)
		if (alt[i] != 0 && alt[i]->action != 0)
			break;
	if (i >= 2 && params[i - 2].code == EOD) {
		// Didn't find any code for this verb/object combination
		CALL1(error, M_CANT0)
	}

	/* Perform actions! */

	/* First try any BEFORE or ONLY from outside in */
	done[0] = FALSE;
	done[1] = FALSE;
	for (i = 2; params[i - 2].code != EOD; i++)
		done[i] = FALSE;
	i--;
	while (i >= 0) {
		if (alt[i] != 0)
			if (alt[i]->qual == (Aword)Q_BEFORE || alt[i]->qual == (Aword)Q_ONLY) {
				if (alt[i]->action != 0) {
					if (trcflg) {
						if (i == 0)
							strcpy(trace, "GLOBAL");
						else if (i == 1)
							strcpy(trace, "in LOCATION");
						else
							sprintf(trace, "in PARAMETER %d", i - 1);
						if (alt[i]->qual == (Aword)Q_BEFORE)
							printf("\n<VERB %d, %s (BEFORE), Body:>\n", cur.vrb, trace);
						else
							printf("\n<VERB %d, %s (ONLY), Body:>\n", cur.vrb, trace);
					}
					CALL1(interpret, alt[i]->action)
					if (fail) return;
					if (alt[i]->qual == (Aword)Q_ONLY) return;
				}
				done[i] = TRUE;
			}
		i--;
	}

	/* Then execute any not declared as AFTER, i.e. the default */
	for (i = 0; i < 2 || params[i - 2].code != EOD; i++) {
		if (alt[i] != 0)
			if (alt[i]->qual != (Aword)Q_AFTER) {
				if (!done[i] && alt[i]->action != 0) {
					if (trcflg) {
						if (i == 0)
							strcpy(trace, "GLOBAL");
						else if (i == 1)
							strcpy(trace, "in LOCATION");
						else
							sprintf(trace, "in PARAMETER %d", i - 1);
						printf("\n<VERB %d, %s, Body:>\n", cur.vrb, trace);
					}
					CALL1(interpret, alt[i]->action)
					if (fail) return;
				}
				done[i] = TRUE;
			}
	}

	/* Finally, the ones declared as after */
	i--;
	while (i >= 0) {
		if (alt[i] != 0)
			if (!done[i] && alt[i]->action != 0) {
				if (trcflg) {
					if (i == 0)
						strcpy(trace, "GLOBAL");
					else if (i == 1)
						strcpy(trace, "in LOCATION");
					else
						sprintf(trace, "in PARAMETER %d", i - 1);
					printf("\n<VERB %d, %s (AFTER), Body:>\n", cur.vrb, trace);
				}
				CALL1(interpret, alt[i]->action)
				if (fail) return;
			}
		i--;
	}
}


/*======================================================================

  action()

  Execute all activities commanded. Handles possible multiple actions
  such as THEM or lists of objects.

  */
void action(CONTEXT, ParamElem plst[] /* IN - Plural parameter list */) {
	int i, mpos;
	char marker[10];

	if (plural) {
		/*
		   The code == 0 means this is a multiple position. We must loop
		   over this position (and replace it by each present in the plst)
		 */
		for (mpos = 0; params[mpos].code != 0; mpos++); /* Find multiple position */
		sprintf(marker, "($%d)", mpos + 1); /* Prepare a printout with $1/2/3 */
		for (i = 0; plst[i].code != EOD; i++) {
			params[mpos] = plst[i];
			output(marker);
			CALL0(do_it)
			if (plst[i + 1].code != EOD)
				para();
		}
		params[mpos].code = 0;
	} else {
		CALL0(do_it)
	}
}


/*----------------------------------------------------------------------*\

  Event Handling

  eventchk()

\*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------
  eventchk()

  Check if any events are pending. If so execute them.
  */
static void eventchk() {
	while (etop != 0 && eventq[etop - 1].time == cur.tick) {
		etop--;
		if (isLoc(eventq[etop].where))
			cur.loc = eventq[etop].where;
		else
			cur.loc = where(eventq[etop].where);
		if (trcflg) {
			printf("\n<EVENT %d (at ", eventq[etop].event);
			debugsay(cur.loc);
			printf("):>\n");
		}
		interpret(evts[eventq[etop].event - EVTMIN].code);
	}
}


/*----------------------------------------------------------------------*\

  Main program and initialisation

  codfil
  filenames

  checkvers()
  load()
  checkdebug()
  initheader()
  initstrings()
  start()
  init()
  main()

\*----------------------------------------------------------------------*/

Common::SeekableReadStream *codfil;

/*----------------------------------------------------------------------

  checkvers()

 */
static void checkvers(AcdHdr *hdr) {
	char vers[4];
	char state[2];

	/* Construct our own version */
	vers[0] = alan.version.version;
	vers[1] = alan.version.revision;

	/* Check version of .ACD file */
	if (dbgflg) {
		state[0] = hdr->vers[3];
		state[1] = '\0';
		printf("<Version of '%s' is %d.%d(%d)%s>",
		       advnam,
		       (int)(hdr->vers[0]),
		       (int)(hdr->vers[1]),
		       (int)(hdr->vers[2]),
		       (hdr->vers[3]) == 0 ? "" : state);
		newline();
	}

	/* Compatible if version and revision match... */
	if (strncmp(hdr->vers, vers, 2) != 0) {
#ifdef V25COMPATIBLE
		if (hdr->vers[0] == 2 && hdr->vers[1] == 5) /* Check for 2.5 version */
			/* This we can convert later if needed... */;
		else
#endif
#ifdef V27COMPATIBLE
			if (hdr->vers[0] == 2 && hdr->vers[1] == 7) /* Check for 2.7 version */
				/* This we can convert later if needed... */;
			else
#endif
				if (errflg) {
					char str[80];
					sprintf(str, "Incompatible version of ACODE program. Game is %ld.%ld, interpreter %ld.%ld.",
					        (long)(hdr->vers[0]),
					        (long)(hdr->vers[1]),
					        (long) alan.version.version,
					        (long) alan.version.revision);
					syserr(str);
				} else
					output("<WARNING! Incompatible version of ACODE program.>\n");
	}
}


/*----------------------------------------------------------------------

  load()

 */
static void load() {
	AcdHdr tmphdr;
	Aword crc = 0;
	uint i;
	char err[100];

	Aword *ptr = (Aword *)&tmphdr + 1;
	codfil->seek(0);
	codfil->read(&tmphdr.vers[0], 4);
	for (i = 1; i < sizeof(tmphdr) / sizeof(Aword); ++i, ++ptr)
		*ptr = codfil->readUint32BE();
	checkvers(&tmphdr);

	/* Allocate and load memory */

	/* No memory allocated yet? */
	if (memory == NULL) {
#ifdef V25COMPATIBLE
		if (tmphdr.vers[0] == 2 && tmphdr.vers[1] == 5)
			/* We need some more memory to expand 2.5 format*/
			memory = allocate((tmphdr.size + tmphdr.objmax - tmphdr.objmin + 1 + 2) * sizeof(Aword));
		else
#endif
			memory = (Aword *)allocate(tmphdr.size * sizeof(Aword));
	}
	memTop = tmphdr.size;
	header = (AcdHdr *) addrTo(0);

	if ((int)(tmphdr.size * sizeof(Aword)) > codfil->size())
		::error("Header size is greater than filesize");

	codfil->seek(0);
	codfil->read(&header->vers[0], sizeof(Aword) * tmphdr.size);

	/* Calculate checksum */
	for (i = sizeof(tmphdr) / sizeof(Aword); i < memTop; i++) {
		crc += memory[i] & 0xff;
		crc += (memory[i] >> 8) & 0xff;
		crc += (memory[i] >> 16) & 0xff;
		crc += (memory[i] >> 24) & 0xff;
	}
	if (crc != tmphdr.acdcrc) {
		sprintf(err, "Checksum error in .ACD file (0x%lx instead of 0x%lx).",
		        (unsigned long) crc, (unsigned long) tmphdr.acdcrc);
		if (errflg)
			syserr(err);
		else {
			output("<WARNING! $$");
			output(err);
			output("$$ Ignored, proceed at your own risk.>$n");
		}
	}

#if defined(SCUMM_LITTLE_ENDIAN)
	if (dbgflg || trcflg || stpflg)
		output("<Hmm, this is a little-endian machine, fixing byte ordering....");
	reverseACD(tmphdr.vers[0] == 2 && tmphdr.vers[1] == 5); /* Reverse all words in the ACD file */
	if (dbgflg || trcflg || stpflg)
		output("OK.>$n");
#endif

#ifdef V25COMPATIBLE
	/* Check for 2.5 version */
	if (tmphdr.vers[0] == 2 && tmphdr.vers[1] == 5) {
		if (dbgflg || trcflg || stpflg)
			output("<Hmm, this is a v2.5 game, please wait while I convert it...");
		c25to26ACD();
		if (dbgflg || trcflg || stpflg)
			output("OK.>$n");
	}
#endif

}


/*----------------------------------------------------------------------

  checkdebug()

 */
static void checkdebug() {
	/* Make sure he can't debug if not allowed! */
	if (!header->debug) {
		if (dbgflg | trcflg | stpflg)
			printf("<Sorry, '%s' is not compiled for debug!>\n", advnam);
		para();
		dbgflg = FALSE;
		trcflg = FALSE;
		stpflg = FALSE;
	}
}


/*----------------------------------------------------------------------

  initheader()

 */
static void initheader() {
	dict = (WrdElem *) addrTo(header->dict);
	/* Find out number of entries in dictionary */
	for (dictsize = 0; !endOfTable(&dict[dictsize]); dictsize++);
	vrbs = (VrbElem *) addrTo(header->vrbs);
	stxs = (StxElem *) addrTo(header->stxs);
	locs = (LocElem *) addrTo(header->locs);
	acts = (ActElem *) addrTo(header->acts);
	objs = (ObjElem *) addrTo(header->objs);
	evts = (EvtElem *) addrTo(header->evts);
	cnts = (CntElem *) addrTo(header->cnts);
	ruls = (RulElem *) addrTo(header->ruls);
	msgs = (MsgElem *) addrTo(header->msgs);
	scores = (Aword *) addrTo(header->scores);

	if (header->pack)
		freq = (Aword *) addrTo(header->freq);
}


/*----------------------------------------------------------------------

  initstrings()

  */
static void initstrings() {
	IniElem *init;

	for (init = (IniElem *) addrTo(header->init); !endOfTable(init); init++) {
		getstr(init->fpos, init->len);
		memory[init->adr] = pop();
	}
}


/*----------------------------------------------------------------------

  start()

 */
static void start() {
	int startloc;

	cur.tick = -1;
	cur.loc = startloc = where(HERO);
	cur.act = HERO;
	cur.score = 0;
	if (trcflg)
		printf("\n<START:>\n");
	interpret(header->start);
	para();

	acts[HERO - ACTMIN].loc = 0;
	locate(HERO, startloc);
}



/*----------------------------------------------------------------------
  init()

  Initialization, program load etc.

  */
static void init() {
	int i;

	/* Initialise some status */
	etop = 0;         /* No pending events */
	looking = FALSE;      /* Not looking now */
	dscrstkp = 0;         /* No describe in progress */

	load();

	initheader();
	checkdebug();

	/* Initialise string attributes */
	initstrings();

	/* Find first conjunction and use that for ',' handling */
	for (i = 0; i < dictsize; i++)
		if (isConj(i)) {
			conjWord = i;
			break;
		}

	/* Start the adventure */
	clear();
	start();
}



/*----------------------------------------------------------------------
  movactor()

  Let the current actor move. If player, ask him.

 */
static void movactor(CONTEXT) {
	ScrElem *scr;
	StepElem *step;
	ActElem *act = (ActElem *) &acts[cur.act - ACTMIN];

	cur.loc = where(cur.act);
	if (cur.act == (int)HERO) {
		CALL0(parse)
		if (g_vm->shouldQuit())
			return;
		fail = FALSE;           /* fail only aborts one actor */
		rules();
	} else if (act->script != 0) {
		for (scr = (ScrElem *) addrTo(act->scradr); !endOfTable(scr); scr++)
			if (scr->code == act->script) {
				/* Find correct step in the list by indexing */
				step = (StepElem *) addrTo(scr->steps);
				step = (StepElem *) &step[act->step];
				/* Now execute it, maybe. First check wait count */
				if (step->after > act->count) {
					/* Wait some more */
					if (trcflg) {
						printf("\n<ACTOR %d, ", cur.act);
						debugsay(cur.act);
						printf(" (at ");
						debugsay(cur.loc);
						printf("), SCRIPT %u, STEP %u, Waiting %d more>\n",
						       act->script, act->step + 1, step->after - act->count);
					}
					act->count++;
					rules();
					return;
				} else
					act->count = 0;
				/* Then check possible expression */
				if (step->exp != 0) {
					if (trcflg) {
						printf("\n<ACTOR %d, ", cur.act);
						debugsay(cur.act);
						printf(" (at ");
						debugsay(cur.loc);
						printf("), SCRIPT %u, STEP %u, Evaluating:>\n",
						       act->script, act->step + 1);
					}
					interpret(step->exp);
					if (!(Abool)pop()) {
						rules();
						return; /* Hadn't happened yet */
					}
				}
				/* OK, so finally let him do his thing */
				act->step++;        /* Increment step number before executing... */
				if (trcflg) {
					printf("\n<ACTOR %d, ", cur.act);
					debugsay(cur.act);
					printf(" (at ");
					debugsay(cur.loc);
					printf("), SCRIPT %u, STEP %u, Executing:>\n",
					       act->script, act->step);
				}
				interpret(step->stm);
				step++;
				/* ... so that we can see if he is USEing another script now */
				if (act->step != 0 && endOfTable(step))
					/* No more steps in this script, so stop him */
					act->script = 0;
				fail = FALSE;           /* fail only aborts one actor */
				rules();
				return;
			}
		syserr("Unknown actor script.");
	} else if (trcflg) {
		printf("\n<ACTOR %d, ", cur.act);
		debugsay(cur.act);
		printf(" (at ");
		debugsay(cur.loc);
		printf("), Idle>\n");
		rules();
		return;
	}
}

/*----------------------------------------------------------------------

  openFiles()

  Open the necessary files.

  */
static void openFiles() {
	// If logging open log file
	if (logflg) {
		Common::String filename = Common::String::format("%s.log", advnam);
		logfil = g_system->getSavefileManager()->openForSaving(filename);

		logflg = logfil != nullptr;
	}
}


/*======================================================================

  run()

  Run the adventure

  */
void run() {
	openFiles();

	// Set default line and column
	col = lin = 1;

	while (!g_vm->shouldQuit()) {
		// Load, initialise and start the adventure
		g_vm->setRestart(false);
		init();

		if (g_vm->_saveSlot != -1) {
			if (g_vm->loadGameState(g_vm->_saveSlot).getCode() != Common::kNoError)
				return;
			g_vm->_saveSlot = -1;
			g_vm->_pendingLook = true;
		}

		Context ctx;
		while (!g_vm->shouldQuit() && !g_vm->shouldRestart()) {
			if (!ctx._break) {
				if (dbgflg)
					debug();

				eventchk();
				cur.tick++;
			}

			// Execution ends up here after calls to the error method

			// Move all characters
			ctx._break = false;
			for (cur.act = ACTMIN; cur.act <= (int)ACTMAX && !ctx._break; cur.act++) {
				movactor(ctx);

				if (g_vm->shouldQuit())
					return;
			}
		}
	}
}

} // End of namespace Alan2
} // End of namespace Glk
