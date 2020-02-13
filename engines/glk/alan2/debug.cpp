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

#include "glk/alan2/types.h"
#include "glk/alan2/alan_version.h"
#include "glk/alan2/debug.h"
#include "glk/alan2/exe.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/inter.h"
#include "glk/alan2/main.h"
#include "glk/alan2/parse.h"

namespace Glk {
namespace Alan2 {

static void showatrs(Aword atradr) {
	AtrElem *at;
	int i;
	char str[80];

	if (atradr == 0) return;

	i = 1;
	for (at = (AtrElem *) addrTo(atradr); !endOfTable(at); at++) {
		sprintf(str, "$i%3ld: %ld (%s)", (long) i, (unsigned long) at->val, (char *) addrTo(at->stradr));
		output(str);
		i++;
	}
}

static void showobjs() {
	char str[80];
	uint obj;

	output("OBJECTS:");
	for (obj = OBJMIN; obj <= OBJMAX; obj++) {
		sprintf(str, "$i%3ld: ", (long) obj);
		output(str);
		say(obj);
	}
}

static void showobj(int obj) {
	char str[80];
#define OBJ (obj-OBJMIN)


	if (!isObj(obj)) {
		sprintf(str, "Object number out of range. Between %ld and %ld, please.", (unsigned long) OBJMIN, (unsigned long) OBJMAX);
		output(str);
		return;
	}

	sprintf(str, "OBJECT %d :", obj);
	output(str);
	say(obj);

	sprintf(str, "$iLocation = %ld", (unsigned long) where(obj));
	output(str);
	if (isLoc(objs[OBJ].loc))
		say(objs[OBJ].loc);
	else if (isCnt(objs[OBJ].loc)) {
		if (isObj(objs[OBJ].loc)) {
			output("in");
			say(objs[OBJ].loc);
		} else if (isAct(objs[OBJ].loc)) {
			output("carried by");
			say(objs[OBJ].loc);
		} else
			interpret(cnts[objs[OBJ].loc - CNTMIN].nam);
	} else if (objs[OBJ].loc == 0)
		output("nowhere");
	else
		output("Illegal location!");


	output("$iAttributes =");
	showatrs(objs[OBJ].atrs);

#undef OBJ
}

static void showcnts() {
	char str[80];
	uint cnt;
#define  CNT (cnt-CNTMIN)

	output("CONTAINERS:");
	for (cnt = CNTMIN; cnt <= CNTMAX; cnt++) {
		sprintf(str, "$i%3ld: ", (long) cnt);
		output(str);
		if (cnts[CNT].nam != 0)
			interpret(cnts[CNT].nam);
		if (cnts[CNT].parent != 0)
			say(cnts[CNT].parent);
	}

#undef CNT
}

static void showcnt(int cnt) {
	char str[80];
	uint i;
	Abool found = FALSE;
#define  CNT (int)(cnt - CNTMIN)

	if (cnt < (int)CNTMIN || cnt >(int)CNTMAX) {
		sprintf(str, "Container number out of range. Between %ld and %ld, please.", (unsigned long) CNTMIN, (unsigned long) CNTMAX);
		output(str);
		return;
	}

	sprintf(str, "CONTAINER %d :", cnt);
	output(str);
	if (cnts[CNT].nam != 0)
		interpret(cnts[CNT].nam);
	if (cnts[CNT].parent != 0) {
		cnt = cnts[CNT].parent;
		say(cnt);
		sprintf(str, "$iLocation = %ld", (unsigned long) where(cnt));
		output(str);
	}
	output("$iContains ");
	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (in(i, cnt)) { /* Yes, it's in this container */
			if (!found) {
				output("$n");
				found = TRUE;
			}
			sprintf(str, "$t$t%d: ", i);
			output(str);
			say(i);
		}
	}
	if (!found)
		output("nothing");

#undef CNT
}

static void showlocs() {
	char str[80];
	uint loc;

	output("LOCATIONS:");
	for (loc = LOCMIN; loc <= LOCMAX; loc++) {
		sprintf(str, "$i%3ld: ", (long) loc);
		output(str);
		say(loc);
	}
}

static void showloc(int loc) {
	char str[80];


	if (!isLoc(loc)) {
		sprintf(str, "Location number out of range. Between %ld and %ld, please.", (unsigned long) LOCMIN, (unsigned long) LOCMAX);
		output(str);
		return;
	}

	sprintf(str, "LOCATION %d :", loc);
	output(str);
	say(loc);

	output("$iAttributes =");
	showatrs(locs[loc - LOCMIN].atrs);
}

static void showacts() {
	char str[80];
	uint act;

	output("ACTORS:");
	for (act = ACTMIN; act <= ACTMAX; act++) {
		sprintf(str, "$i%3ld:", (long) act);
		output(str);
		say(act);
	}
}

static void showact(int act) {
	char str[80];
	Boolean oldstp;

	if (!isAct(act)) {
		sprintf(str, "Actor number out of range. Between %ld and %ld, please.", (unsigned long) ACTMIN, (unsigned long) ACTMAX);
		output(str);
		return;
	}

	sprintf(str, "ACTOR %d :", act);
	output(str);
	oldstp = stpflg;
	stpflg = FALSE; /* Make sure not to trace this! */
	say(act);
	stpflg = oldstp;

	sprintf(str, "$iLocation = %ld", (unsigned long) acts[act - ACTMIN].loc);
	output(str);
	if (isLoc(acts[act - ACTMIN].loc))
		say(acts[act - ACTMIN].loc);
	else if (acts[act - ACTMIN].loc == 0)
		output("nowhere");
	else
		output("Illegal location!");

	sprintf(str, "$iScript = %ld", (unsigned long) acts[act - ACTMIN].script);
	output(str);

	sprintf(str, "$iStep = %ld", (unsigned long) acts[act - ACTMIN].step);
	output(str);

	output("$iAttributes =");
	showatrs(acts[act - ACTMIN].atrs);
}

static void showevts() {
	int i;
	char str[80];
	Boolean scheduled;

	output("EVENTS:");
	for (uint evt = EVTMIN; evt <= EVTMAX; evt++) {
		sprintf(str, "$i%d (%s):", evt, (char *)addrTo(evts[evt - EVTMIN].stradr));
		output(str);
		scheduled = FALSE;
		for (i = 0; i < etop; i++)
			if ((scheduled = (eventq[i].event == (int)evt)))
				break;
		if (scheduled) {
			sprintf(str, "Scheduled for +%d, at ", eventq[i].time - cur.tick);
			output(str);
			say(eventq[i].where);
		} else
			output("Not scheduled.");
	}
}


static Boolean trc, stp;
static int loc;

void saveInfo() {
	/* Save some important things */
	trc = trcflg;
	trcflg = FALSE;
	stp = stpflg;
	stpflg = FALSE;
	loc = cur.loc;
	cur.loc = where(HERO);
}

void restoreInfo() {
	/* Restore! */
	trcflg = trc;
	stpflg = stp;
	cur.loc = loc;
}

void debug() {
	char buf[256];
	char c;
	int i;

	saveInfo();
	while (TRUE) {
		if (anyOutput)
			para();
		do {
			output("ABUG> ");
			(void)readline(buf);

			lin = 1;
			c = buf[0];
			i = 0;
			(void)sscanf(&buf[1], "%d", &i);
		} while (/*buf &&*/ c == '\0');

		switch (toUpper(c)) {
		case 'H':
		case '?':
			output(alan.longHeader);
			output("$nABUG Commands:\
      $iO [n] -- show object[s]\
      $iA [n] -- show actor[s]\
      $iL [n] -- show location[s]\
      $iC [n] -- show container[s]\
      $iE -- show events\
      $iG -- go on\
      $iT -- toggle trace mode\
      $iS -- toggle step mode\
      $iX -- exit debug mode\
      $iQ -- quit game");
			break;
		case 'Q': {
			Context ctx;
			terminate(ctx, 0);
			break;
		}
		case 'X':
			dbgflg = FALSE;
			restoreInfo();
			return;
		case 'G':
			restoreInfo();
			return;
		case 'O':
			if (i == 0)
				showobjs();
			else
				showobj(i);
			break;
		case 'C':
			if (i == 0)
				showcnts();
			else
				showcnt(i);
			break;
		case 'A':
			if (i == 0)
				showacts();
			else
				showact(i);
			break;
		case 'L':
			if (i == 0)
				showlocs();
			else
				showloc(i);
			break;
		case 'E':
			showevts();
			break;
		case 'S':
			if ((stp = !stp))
				printf("Step on.");
			else
				printf("Step off.");
			break;
		case 'T':
			if ((trc = !trc))
				printf("Trace on.");
			else
				printf("Trace off.");
			break;
		default:
			output("Unknown ABUG command. ? for help.");
			break;
		}
	}
}


/*======================================================================

  debugsay()

  Say somethin, but make sure we don't disturb anything and that it is
  shown to the player.

*/
void debugsay(int item) {
	saveInfo();
	needsp = FALSE;
	col = 1;
	if (item == 0)
		printf("$null$");
	else
		say(item);
	needsp = FALSE;
	col = 1;
	restoreInfo();
}

} // End of namespace Alan2
} // End of namespace Glk
