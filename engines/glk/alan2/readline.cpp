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

#ifdef GLK

#include "glk/alan2/alan2.h"
#include "glk/alan2/readline.h"
#include "glk/alan2/main.h"
#include "glk/alan2/glkio.h"

namespace Glk {
namespace Alan2 {

/*======================================================================

  readline()

  Read a line from the user, with history and editing

  */

/* 4f - length of user buffer should be used */
Boolean readline(char usrbuf[])
{
  event_t event;
  g_vm->glk_request_line_event(glkMainWin, usrbuf, 255, 0);
  /* FIXME: buffer size should be infallible: all existing calls use 256 or
     80 character buffers, except parse which uses LISTLEN (currently 100)
   */
  do
  {
	  g_vm->glk_select(&event);
    if (evtype_Arrange == event.type)
      statusline();
	if (g_vm->shouldQuit())
		return false;

  } while (event.type != evtype_LineInput);

  usrbuf[event.val1] = 0;
  return TRUE;
}

} // End of namespace Alan2
} // End of namespace Glk

#else

#include "glk/alan2/sysdep.h"

#ifdef HAVE_TERMIO
#include <termios.h>
#endif

#ifdef __PACIFIC__
#include <unixio.h>
#else
//#include <unistd.h>
#endif

#include "glk/alan2/readline.h"

#include "glk/alan2/main.h"

namespace Glk {
namespace Alan2 {

#ifdef HAVE_TERMIO
/*----------------------------------------------------------------------*\

  termio handling

\*----------------------------------------------------------------------*/

static struct termios term;

static void newtermio()
{
  struct termios newterm;
  tcgetattr(0, &term);
  newterm=term;
  newterm.c_lflag&=~(ECHO|ICANON);
  newterm.c_cc[VMIN]=1;
  newterm.c_cc[VTIME]=0;
  tcsetattr(0, TCSANOW, &newterm);
}

static void restoretermio()
{
  tcsetattr(0, TCSANOW, &term);
}

#endif


/*----------------------------------------------------------------------*\

  Global character buffers etc.

\*----------------------------------------------------------------------*/

static unsigned char buffer[LINELENGTH+1];
static int bufidx;

static unsigned char *history[HISTORYLENGTH];
static int histidx;		/* Index where to store next history */
static int histp;		/* Points to the history recalled last */

static unsigned char ch;
static int endOfInput = 0;
static Boolean change;
static Boolean insert = TRUE;


/*----------------------------------------------------------------------*\

  Character map types and maps

\*----------------------------------------------------------------------*/

typedef struct {unsigned char min, max; void (*hook)(char ch);} KeyMap;

/* Forward declaration of hooks */
static void escHook(char ch);
static void insertCh(char ch);
static void arrowHook(char ch);
static void upArrow(char ch);
static void downArrow(char ch);
static void rightArrow(char ch);
static void leftArrow(char ch);
static void insertToggle(char ch);
static void newLine(char ch);
static void delFwd(char ch);
static void delBwd(char ch);

#ifdef __XXunix__
static KeyMap keymap[] = {
  {0x00, 0x07, NULL},
  {0x08, 0x08, delBwd},
  {0x09, 0x09, NULL},
  {0x0a, 0x0a, newLine},
  {0x1b, 0x1b, escHook},
  {0x1c, 0x7e, insertCh},
  {0x7f, 0x7f, delFwd},
  {0x80, 0xff, insertCh},
  {0x00, 0x00, NULL}
};

/* I can't figure out what really coverns the esc-map characters... */
#ifdef __solarisX__
static KeyMap escmap[] = {
  {0x00, 0x4e, NULL},
  {0x4f, 0x4f, arrowHook},
  {0x50, 0xff, NULL},
  {0x00, 0x00, NULL}
};
#else
static KeyMap escmap[] = {
  {0x00, 0x5a, NULL},
  {0x5b, 0x5b, arrowHook},
  {0x5c, 0xff, NULL},
  {0x00, 0x00, NULL}
};
#endif

static KeyMap arrowmap[] = {
  {0x00, 0x31, NULL},
  {0x32, 0x32, insertToggle},
  {0x33, 0x40, NULL},
  {0x41, 0x41, upArrow},
  {0x42, 0x42, downArrow},
  {0x43, 0x43, rightArrow},
  {0x44, 0x44, leftArrow},
  {0x45, 0xff, NULL},
  {0x00, 0x00, NULL}
};

#endif

#ifdef __win__
static KeyMap keymap[] = {
  {0x00, 0x01, NULL},
  {0x02, 0x02, leftArrow},
  {0x03, 0x05, NULL},
  {0x06, 0x06, rightArrow},
  {0x07, 0x07, NULL},
  {0x08, 0x08, delBwd},
  {0x09, 0x09, NULL},
  {0x0a, 0x0a, newLine},
  {0x1b, 0x1b, escHook},
  {0x1c, 0x7e, insertCh},
  {0x7f, 0x7f, delFwd},
  {0x80, 0xff, insertCh},
  {0x00, 0x00, NULL}
};

static KeyMap escmap[] = {
  {0x00, 0x5a, NULL},
  {0x5b, 0x5b, arrowHook},
  {0x5c, 0xff, NULL},
  {0x00, 0x00, NULL}
};

static KeyMap arrowmap[] = {
  {0x00, 0x31, NULL},
  {0x32, 0x32, insertToggle},
  {0x33, 0x40, NULL},
  {0x41, 0x41, upArrow},
  {0x42, 0x42, downArrow},
  {0x43, 0x43, rightArrow},
  {0x44, 0x44, leftArrow},
  {0x45, 0xff, NULL},
  {0x00, 0x00, NULL}
};

#endif

#ifdef __dos__
static KeyMap keymap[] = {
  {0x00, 0x01, NULL},
  {0x02, 0x02, leftArrow},
  {0x03, 0x05, NULL},
  {0x06, 0x06, rightArrow},
  {0x07, 0x07, NULL},
  {0x08, 0x08, delBwd},
  {0x09, 0x09, NULL},
  {0x0a, 0x0a, newLine},
  {0x1b, 0x1b, escHook},
  {0x1c, 0x7e, insertCh},
  {0x7f, 0x7f, delFwd},
  {0x80, 0xff, insertCh},
  {0x00, 0x00, NULL}
};

static KeyMap escmap[] = {
  {0x00, 0x5a, NULL},
  {0x5b, 0x5b, arrowHook},
  {0x5c, 0xff, NULL},
  {0x00, 0x00, NULL}
};

static KeyMap arrowmap[] = {
  {0x00, 0x31, NULL},
  {0x32, 0x32, insertToggle},
  {0x33, 0x40, NULL},
  {0x41, 0x41, upArrow},
  {0x42, 0x42, downArrow},
  {0x43, 0x43, rightArrow},
  {0x44, 0x44, leftArrow},
  {0x45, 0xff, NULL},
  {0x00, 0x00, NULL}
};

#endif


static void doBeep(void)
{
  write(1, "\7", 1);
}


static void backspace(void)
{
  write(1, "\b", 1);
}


static void erase()
{
  int i;

  for (i = 0; i < bufidx; i++) backspace(); /* Backup to beginning of text */
  for (i = 0; i < strlen((char *)buffer); i++) write(1, " ", 1); /* Erase all text */
  for (i = 0; i < strlen((char *)buffer); i++) backspace(); /* Backup to beginning of text */
}

/*----------------------------------------------------------------------*\

  Character handling hook functions

\*----------------------------------------------------------------------*/

static void execute(KeyMap map[], unsigned char ch)
{
  int i = 0;

  for (i = 0; i <= 256; i++) {
    if (i > 0 && map[i].min == 0x00) break; /* End marker is a 0,0,NULL */
    if (map[i].min <= ch && ch <= map[i].max) {
      if (map[i].hook != NULL) {
	map[i].hook(ch);
	return;
      } else
	doBeep();
    }
  }
  doBeep();
}


static void upArrow(char ch)
{
  /* Is there more history ? */
  if (history[(histp+HISTORYLENGTH-1)%HISTORYLENGTH] == NULL ||
      (histp+HISTORYLENGTH-1)%HISTORYLENGTH == histidx) {
    write(1, "\7", 1);
    return;
  }

  erase();

  /* Backup history pointer */
  histp = (histp+HISTORYLENGTH-1)%HISTORYLENGTH;

  /* Copy the history and write it */
  strcpy((char *)buffer, (char *)history[histp]);
  bufidx = strlen((char *)buffer);
  write(1, (void *)buffer, strlen((char *)buffer));

}


static void downArrow(char ch)
{
  /* Is there more history ? */
  if (histp == histidx) {
    write(1, "\7", 1);
    return;
  }

  erase();

  /* Advance history pointer */
  histp = (histp+1)%HISTORYLENGTH;

  /* If we are not at the most recent history entry, copy the history and write it */
  if (histp != histidx) {
    strcpy((char *)buffer, (char *)history[histp]);
    bufidx = strlen((char *)buffer);
    write(1, (void *)buffer, strlen((char *)buffer));
  } else {
    bufidx = 0;
    buffer[0] = '\0';
  }
}


static void rightArrow(char ch)
{
  if (bufidx > LINELENGTH || buffer[bufidx] == '\0')
    doBeep();
  else {
    write(1, (void *)&buffer[bufidx], 1);
    bufidx++;
  }
}


static void leftArrow(char ch)
{
  if (bufidx == 0)
    doBeep();
  else {
    bufidx--;
    backspace();
  }
}


static void insertToggle(char ch)
{
  read(0, &ch, 1);
  if (ch != 'z')
    doBeep();
  else
    insert = !insert;
}


static void delBwd(char ch)
{
  if (bufidx == 0)
    doBeep();
  else {
    int i;

    change = TRUE;
    backspace();
    bufidx--;
    for (i = 0; i <= strlen((char *)&buffer[bufidx+1]); i++)
      buffer[bufidx+i] = buffer[bufidx+1+i];
    write(1, (void *)&buffer[bufidx], strlen((char *)&buffer[bufidx]));
    write(1, " ", 1);
    for (i = 0; i <= strlen((char *)&buffer[bufidx]); i++) backspace();
  }
}  

static void delFwd(char ch)
{
  if (bufidx > LINELENGTH || buffer[bufidx] == '\0')
    doBeep();
  else {
    int i;

    change = TRUE;
    strcpy((char *)&buffer[bufidx], (char *)&buffer[bufidx+1]);
    write(1, (void *)&buffer[bufidx], strlen((char *)&buffer[bufidx]));
    write(1, " ", 1);
    for (i = 0; i <= strlen((char *)&buffer[bufidx]); i++) backspace();
  }
}  

static void escHook(char ch) {
  read(0, &ch, 1);
  execute(escmap, ch);
}

static void arrowHook(char ch) {
  read(0, &ch, 1);
  execute(arrowmap, ch);
}

static void newLine(char ch)
{
  endOfInput = 1;
  write(1, "\n", 1);

  /* If the input is not the same as the previous, save it in the history */
  if (change && strlen((char *)buffer) > 0) {
    if (history[histidx] == NULL)
      history[histidx] = (unsigned char *)allocate(LINELENGTH+1);
    strcpy((char *)history[histidx], (char *)buffer);
    histidx = (histidx+1)%HISTORYLENGTH;
  }
}


static void insertCh(char ch) {
  if (bufidx > LINELENGTH)
    doBeep();
  else {
    /* If at end advance the NULL */
    if (buffer[bufidx] == '\0')
      buffer[bufidx+1] = '\0';
    else if (insert) {
      int i;

      /* If insert mode is on, move the characters ahead */
      for (i = strlen((char *)buffer); i >= bufidx; i--)
	buffer[i+1] = buffer[i];
      write(1, (void *)&buffer[bufidx], strlen((char *)&buffer[bufidx]));
      for (i = strlen((char *)&buffer[bufidx]); i > 0; i--) backspace();
    }
    change = TRUE;
    buffer[bufidx] = ch;
    write(1, &ch, 1);
    bufidx++;
  }
}


/*----------------------------------------------------------------------

  echoOff()

  */
static void echoOff()
{
#ifdef HAVE_TERMIO
  newtermio();
#else
#ifdef __win__
#include <windows.h>
#include <winbase.h>
#include <wincon.h>

  DWORD handle = GetStdHandle(STD_INPUT_HANDLE);

  (void) SetConsoleMode(handle, 0);

#endif
#endif
}


/*----------------------------------------------------------------------

  echoOn()

  */
static void echoOn()
{
#ifdef HAVE_TERMIO
  restoretermio();
#else
#ifdef __win__
#include <windows.h>
#include <winbase.h>
#include <wincon.h>

  DWORD handle = GetStdHandle(STD_INPUT_HANDLE);
  (void) SetConsoleMode(handle, ENABLE_ECHO_INPUT);

#endif
#endif
}


/*======================================================================

  readline()

  Read a line from the user, with history and editing

  */

/* 4f - length of user buffer should be used */
Boolean readline(char usrbuf[])
{
  fflush(stdout);
  bufidx = 0;
  histp = histidx;
  buffer[0] = '\0';
  change = TRUE;
  echoOff();
  endOfInput = 0;
  while (!endOfInput) {
    if (read(0, (void *)&ch, 1) != 1) {
      echoOn();
      return FALSE;
    }
    execute(keymap, ch);
  }
  echoOn();
  strcpy(usrbuf, (char *)buffer);  
  return TRUE;
}

} // End of namespace Alan2
} // End of namespace Glk

#endif
