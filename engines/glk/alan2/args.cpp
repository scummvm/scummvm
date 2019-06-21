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

#include <string>
#ifndef __PACIFIC__
#include "glk/alan2/args.h"
#else
/* I have no idea at all why the include does not work in Pacific C ... */
extern void args(int argc, char* argv[]);
#endif

#include "glk/alan2/main.h"

#ifdef __mac__
#include "glk/alan2/macArgs.h"
#endif

#ifdef __amiga__
#include <libraries/dosextens.h>
#ifdef AZTEC_C
struct FileHandle *con = NULL;
#else
/* Geek Gadgets GCC */
BPTR window;
BPTR cd;
#endif
#endif

#ifdef GLK
#include "glk/alan2/alan2.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/sysdep.h"
#endif

namespace Glk {
namespace Alan2 {

static void switches(unsigned argc, char *argv[]) {
#ifndef GLK
  uint i;
  
  advnam = "";
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
#ifdef GLK
      switch (g_vm->glk_char_to_lower(argv[i][1])) {
#else
      switch (tolower(argv[i][1])) {
#endif
      case 'i':
	errflg = FALSE;
	break;
      case 't':
	trcflg = TRUE;
	break;
      case 'd':
	dbgflg = TRUE;
	break;
      case 's':
	trcflg = TRUE;
	stpflg = TRUE;
	break;
      case 'l':
	logflg = TRUE;
	break;
      case 'v':
	verbose = TRUE;
	break;
      case 'n':
	statusflg = FALSE;
	break;
      default:
	printf("Unrecognized switch, -%c\n", argv[i][1]);
	usage();
	terminate(0);
      }
    } else {
      advnam = argv[i];
      if (strcmp(&advnam[strlen(advnam)-4], ".acd") == 0
	  || strcmp(&advnam[strlen(advnam)-4], ".ACD") == 0
	  || strcmp(&advnam[strlen(advnam)-4], ".dat") == 0
	  || strcmp(&advnam[strlen(advnam)-4], ".DAT") == 0)
		advnam[strlen(advnam)-4] = '\0';
    }
  }
#endif
}



#ifdef __amiga__

#include <intuition/intuition.h>
#include <workbench/workbench.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>

#include <fcntl.h>

extern struct Library *IconBase;

#ifndef AZTEC_C
/* Actually Geek Gadgets GCC with libnix */

/* Aztec C has its own pre-main wbparse which was used in Arun 2.7, with GCC we
   need to do it ourselves. */

#include <clib/intuition_protos.h>

extern unsigned long *__stdfiledes; /* The libnix standard I/O file descriptors */

void
wb_parse(void)
{
  char *cp;
  struct DiskObject *dop;
  struct FileHandle *fhp;

  if (_WBenchMsg->sm_NumArgs == 1) /* If no argument use program icon/info */
    dop = GetDiskObject((UBYTE *)_WBenchMsg->sm_ArgList[0].wa_Name);
  else {
    BPTR olddir = CurrentDir(_WBenchMsg->sm_ArgList[1].wa_Lock);
    dop = GetDiskObject((UBYTE *)_WBenchMsg->sm_ArgList[1].wa_Name);
    CurrentDir(olddir);
  }
  if (dop != 0 && (cp = (char *)FindToolType((UBYTE **)dop->do_ToolTypes, 
					     (UBYTE *)"WINDOW")) != NULL)
    ;
  else /* Could not find a WINDOW tool type */
    cp = "CON:10/10/480/160/Arun:Default Window/CLOSE";
  if ((window = Open((UBYTE *)cp, (long)MODE_OLDFILE))) {
    fhp = (struct FileHandle *) ((long)window << 2);
    SetConsoleTask(fhp->fh_Type);
    SelectInput(window);
    SelectOutput(window);
    __stdfiledes[0] = Input();
    __stdfiledes[1] = Output();
  } else
    exit(-1L);
  FreeDiskObject(dop);
}
#endif
#endif

void args(int argc, char * argv[]) {
  char *prgnam;

#ifdef __mac__
#include <console.h>
#ifdef __MWERKS__
#include <SIOUX.h>
#endif
  short msg, files;
  static char advbuf[256], prgbuf[256];
  /*AppFile af;*/
  OSErr oe;

#ifdef __MWERKS__
  /*SIOUXSettings.setupmenus = FALSE;*/
  SIOUXSettings.autocloseonquit = FALSE;
  SIOUXSettings.asktosaveonclose = FALSE;
  SIOUXSettings.showstatusline = FALSE;
#endif

	GetMacArgs(advbuf);
	advnam = advbuf;

#else
#ifdef __amiga__

  if (argc == 0) { /* If started from Workbench get WbArgs : Aztec C & GG GCC */
    struct WBStartup *WBstart;

    if ((IconBase = OpenLibrary("icon.library", 0)) == NULL)
      syserr("Could not open 'icon.library'");
    /* If started from WB normal main is called with argc == 0 and argv = WBstartup message */
    WBstart = (struct WBStartup *)argv;
#ifndef AZTEC_C
    /* Geek Gadgets GCC */
    wb_parse();
#endif
    advnam = prgnam = WBstart->sm_ArgList[0].wa_Name;
    if (WBstart->sm_NumArgs > 0) {
      cd = CurrentDir(DupLock(WBstart->sm_ArgList[1].wa_Lock));
      advnam = WBstart->sm_ArgList[1].wa_Name;
    }
    /* Possibly other tooltypes ... */
  } else {
    /* Started from a CLI */
    if ((prgnam = strrchr(argv[0], '/')) == NULL
	&& (prgnam = strrchr(argv[0], ':')) == NULL)
      prgnam = argv[0];
    else
      prgnam++;
    /* Now look at the switches and arguments */
    switches(argc, argv);
    if (advnam[0] == '\0')
      /* No game given, try program name */
      if (stricmp(prgnam, PROGNAME) != 0
          && strstr(prgnam, PROGNAME) == 0)
	advnam = strdup(argv[0]);
  }
#else
#if defined(__dos__) || defined(__win__)
  if ((prgnam = strrchr(argv[0], '\\')) == NULL
      && (prgnam = strrchr(argv[0], '/')) == NULL
      && (prgnam = strrchr(argv[0], ':')) == NULL)
    prgnam = argv[0];
  else
    prgnam++;
  if (strlen(prgnam) > 4
      && (strcmp(&prgnam[strlen(prgnam)-4], ".EXE") == 0
	  || strcmp(&prgnam[strlen(prgnam)-4], ".exe") == 0))
    prgnam[strlen(prgnam)-4] = '\0';
  /* Now look at the switches and arguments */
  switches(argc, argv);
  if (advnam[0] == '\0')
    /* No game given, try program name */
    if (stricmp(prgnam, PROGNAME) != 0
        && strstr(prgnam, PROGNAME) == 0)
      advnam = strdup(argv[0]);
#else
#if defined __vms__
  if ((prgnam = strrchr(argv[0], ']')) == NULL
      && (prgnam = strrchr(argv[0], '>')) == NULL
      && (prgnam = strrchr(argv[0], ':')) == NULL)
    prgnam = argv[0];
  else
    prgnam++;
  if (strrchr(prgnam, ';') != NULL)
    *strrchr(prgnam, ';') = '\0';
  if (strlen(prgnam) > 4
      && (strcmp(&prgnam[strlen(prgnam)-4], ".EXE") == 0
	  || strcmp(&prgnam[strlen(prgnam)-4], ".exe") == 0))
    prgnam[strlen(prgnam)-4] = '\0';
  /* Now look at the switches and arguments */
  switches(argc, argv);
  if (advnam[0] == '\0')
    /* No game given, try program name */
    if (strcmp(prgnam, PROGNAME) != 0
        && strstr(prgnam, PROGNAME) == 0)
      advnam = strdup(argv[0]);
#else
#if defined(__unix__) || defined(__APPLE__)
  if ((prgnam = strrchr(argv[0], '/')) == NULL)
    prgnam = strdup(argv[0]);
  else
    prgnam = strdup(&prgnam[1]);
  if (strrchr(prgnam, ';') != NULL)
    *strrchr(prgnam, ';') = '\0';
  /* Now look at the switches and arguments */
  switches(argc, argv);
  if (advnam[0] == '\0')
    /* No game given, try program name */
    if (strcmp(prgnam, PROGNAME) != 0
        && strstr(prgnam, PROGNAME) == 0)
      advnam = strdup(argv[0]);
#else
  Unimplemented OS!
#endif
#endif
#endif
#endif
#endif
}

} // End of namespace Alan2
} // End of namespace Glk
