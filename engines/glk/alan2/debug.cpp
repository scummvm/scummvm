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
#ifdef HAVE_SHORT_FILENAMES
#include "glk/alan2/av.h"
#else
#include "glk/alan2/alan_version.h"
#endif

#ifdef USE_READLINE
#include "glk/alan2/readline.h"
#endif

#include "glk/alan2/inter.h"
#include "glk/alan2/main.h"
#include "glk/alan2/parse.h"
#include "glk/alan2/exe.h"

#include "glk/alan2/debug.h"

#ifdef GLK
#include "glk/alan2/glkio.h"
#endif

namespace Glk {
namespace Alan2 {

#ifdef _PROTOTYPES_
static void showatrs(
  Aword atradr
)
#else
static void showatrs(atradr)
  Aword atradr;
#endif
{
  AtrElem *at;
  int i;
  char str[80];

  if (atradr == 0) return;

  i = 1;
  for (at = (AtrElem *) addrTo(atradr); !endOfTable(at); at++) {
    sprintf(str, "$i%3ld: %ld (%s)", (long) i, (unsigned long) at->val, (char *) addrTo(at->stradr));
#if ISO == 0
    fromIso(str, str);
#endif
    output(str);
    i++;
  }
}


#ifdef _PROTOTYPES_
static void showobjs(void)
#else
static void showobjs()
#endif
{
  char str[80];
  int obj;

  output("OBJECTS:");
  for (obj = OBJMIN; obj <= OBJMAX; obj++) {
    sprintf(str, "$i%3ld: ", (long) obj);
    output(str);
    say(obj);
  }
}


#ifdef _PROTOTYPES_
static void showobj(
  int obj
)
#else
static void showobj(obj)
  int obj;
#endif
{
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
      interpret(cnts[objs[OBJ].loc-CNTMIN].nam);
  } else if (objs[OBJ].loc == 0)
    output("nowhere");
  else
    output("Illegal location!");


  output("$iAttributes =");
  showatrs(objs[OBJ].atrs);

#undef OBJ
}


#ifdef _PROTOTYPES_
static void showcnts(void)
#else
static void showcnts()
#endif
{
  char str[80];
  int cnt;
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


#ifdef _PROTOTYPES_
static void showcnt(
  int cnt
)
#else
static void showcnt(cnt)
  int cnt;
#endif
{
  char str[80];
  int i;
  Abool found = FALSE;
#define  CNT (cnt-CNTMIN)

  if (cnt < CNTMIN || cnt > CNTMAX) {
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


#ifdef _PROTOTYPES_
static void showlocs(void)
#else
static void showlocs()
#endif
{
  char str[80];
  int loc;

  output("LOCATIONS:");
  for (loc = LOCMIN; loc <= LOCMAX; loc++) {
    sprintf(str, "$i%3ld: ", (long) loc);
    output(str);
    say(loc);
  }
}


#ifdef _PROTOTYPES_
static void showloc(
  int loc
)
#else
static void showloc(loc)
  int loc;
#endif
{
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
  showatrs(locs[loc-LOCMIN].atrs);
}


#ifdef _PROTOTYPES_
static void showacts(void)
#else
static void showacts()
#endif
{
  char str[80];
  int act;

  output("ACTORS:");
  for (act = ACTMIN; act <= ACTMAX; act++) {
    sprintf(str, "$i%3ld:", (long) act);
    output(str);
    say(act);
  }
}


#ifdef _PROTOTYPES_
static void showact(
  int act
)
#else
static void showact(act)
  int act;
#endif
{
  char str[80];
  Boolean oldstp;
  
  if (!isAct(act)) {
    sprintf(str, "Actor number out of range. Between %ld and %ld, please.", (unsigned long) ACTMIN, (unsigned long) ACTMAX);
    output(str);
    return;
  }
  
  sprintf(str, "ACTOR %d :", act);
  output(str);
  oldstp = stpflg; stpflg = FALSE; /* Make sure not to trace this! */
  say(act);
  stpflg = oldstp;

  sprintf(str, "$iLocation = %ld", (unsigned long) acts[act-ACTMIN].loc);
  output(str);
  if (isLoc(acts[act-ACTMIN].loc))
    say(acts[act-ACTMIN].loc);
  else if (acts[act-ACTMIN].loc == 0)
    output("nowhere");
  else
    output("Illegal location!");

  sprintf(str, "$iScript = %ld", (unsigned long) acts[act-ACTMIN].script);
  output(str);

  sprintf(str, "$iStep = %ld", (unsigned long) acts[act-ACTMIN].step);
  output(str);

  output("$iAttributes =");
  showatrs(acts[act-ACTMIN].atrs);
}


#ifdef _PROTOTYPES_
static void showevts(void)
#else
static void showevts()
#endif
{
  int evt, i;
  char str[80];
  Boolean scheduled;

  output("EVENTS:");
  for (evt = EVTMIN; evt <= EVTMAX; evt++) {
    sprintf(str, "$i%d (%s):", evt, (char *)addrTo(evts[evt-EVTMIN].stradr));
#if ISO == 0
    fromIso(str, str);
#endif
    output(str);
    scheduled = FALSE;
    for (i = 0; i < etop; i++)
      if ((scheduled = (eventq[i].event == evt)))
	break;
    if (scheduled) {
      sprintf(str, "Scheduled for +%d, at ", eventq[i].time-cur.tick);
      output(str);
      say(eventq[i].where);
    } else
      output("Not scheduled.");
  }
}


static Boolean trc, stp;
static int loc;

#ifdef _PROTOTYPES_
void saveInfo(void)
#else
void saveInfo()
#endif
{
  /* Save some important things */
  trc = trcflg; trcflg = FALSE;
  stp = stpflg; stpflg = FALSE;
  loc = cur.loc; cur.loc = where(HERO);
}

#ifdef _PROTOTYPES_
void restoreInfo(void)
#else
void restoreInfo()
#endif
{
  /* Restore! */
  trcflg = trc;
  stpflg = stp;
  cur.loc = loc;
}


#ifdef _PROTOTYPES_
void debug(void)
#else
void debug()
#endif
{
  char buf[256];
  char c;
  int i;

  saveInfo();
  while (TRUE) {
    if (anyOutput)
      para();
    do {
      output("ABUG> ");
#ifdef USE_READLINE
      (void) readline(buf);
#else
      fgets(buf, 255, stdin);
#endif
      lin = 1;
      c = buf[0];
      i = 0;
      sscanf(&buf[1], "%d", &i);
    } while (buf && c == '\0');

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
    case 'Q':
      terminate(0);
    case 'X':
      dbgflg = FALSE;		/* Fall through to 'G' */
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
#ifdef _PROTOTYPES_
void debugsay(int item)
#else
void debugsay(item)
     int item;
#endif
{
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
