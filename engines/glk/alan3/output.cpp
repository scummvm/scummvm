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

#include "common/stream.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/options.h"
#include "glk/alan3/output.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/word.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/dictionary.h"
#include "glk/alan3/current.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/sysdep.h"
#include "glk/alan3/instance.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
bool anyOutput;
bool capitalize;
bool needSpace;
bool skipSpace;

/* Screen formatting info */
int col, lin;
int pageLength, pageWidth;

/* Logfile */
strid_t logFile;


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
  getPageSize()

  Try to get the current page size from the system, else use the ones
  from the header.

 */
void getPageSize(void) {
	pageLength = 0;
	pageWidth = 0;
}


/*----------------------------------------------------------------------*/
static int updateColumn(int currentColumn, const char *string) {
	const char *newlinePosition = strrchr(string, '\n');
	if (newlinePosition != NULL)
		return &string[strlen(string)] - newlinePosition;
	else
		return currentColumn + strlen(string);
}


/*======================================================================*/
void setSubHeaderStyle(void) {
	g_io->glk_set_style(style_Subheader);
}


/*======================================================================*/
void setNormalStyle(void) {
	g_io->glk_set_style(style_Normal);
}

/*======================================================================*/
void newline(void) {
	printAndLog("\n");
	col = 1;
	needSpace = FALSE;
}


/*======================================================================*/
void para(void) {
	g_io->flowBreak();

	if (col != 1)
		newline();
	newline();
	capitalize = TRUE;
}


/*======================================================================*/
void clear(void) {
	g_io->clear();
}


/*----------------------------------------------------------------------*/
static void capitalizeFirst(Common::String &str) {
	uint i = 0;

	// Skip over space...
	while (i < str.size() && isSpace(str[i])) i++;

	if (i < str.size()) {
		str.setChar(toUpper(str[i]), i);
		capitalize = false;
	}
}


/*======================================================================*/
void printAndLog(const char *string) {
	static int column = 0;
	char *stringCopy;
	char *stringPart;

	printf("%s", string);
	if (!g_io->onStatusLine && transcriptOption) {
		// TODO Is this assuming only 70-char wide windows for GLK?
		if ((int)strlen(string) > 70 - column) {
			stringCopy = scumm_strdup(string);  /* Make sure we can write NULLs */
			stringPart = stringCopy;
			while ((int)strlen(stringPart) > 70 - column) {
				int p;
				for (p = 70 - column; p > 0 && !Common::isSpace((int)stringPart[p]); p--);
				stringPart[p] = '\0';
				g_io->glk_put_string_stream(logFile, stringPart);
				g_io->glk_put_char_stream(logFile, '\n');
				column = 0;
				stringPart = &stringPart[p + 1];
			}
			g_io->glk_put_string_stream(logFile, stringPart);
			column = updateColumn(column, stringPart);
			free(stringCopy);
		} else {
			g_io->glk_put_string_stream(logFile, string);
			column = updateColumn(column, string);
		}
	}
}


/*----------------------------------------------------------------------*/
static void justify(const char *str) {
	Common::String tempStr(str);

	if (capitalize)
		capitalizeFirst(tempStr);

	printAndLog(tempStr.c_str());
	col = col + tempStr.size();  // Update column
}


/*----------------------------------------------------------------------*/
static void space(void) {
	if (skipSpace)
		skipSpace = FALSE;
	else {
		if (needSpace) {
			printAndLog(" ");
			col++;
		}
	}
	needSpace = FALSE;
}


/*----------------------------------------------------------------------*/
static void sayPlayerWordsForParameter(int p) {
	int i;

	for (i = globalParameters[p].firstWord; i <= globalParameters[p].lastWord; i++) {
		justify((char *)pointerTo(dictionary[playerWords[i].code].string));
		if (i < globalParameters[p].lastWord)
			justify(" ");
	}
}


/*----------------------------------------------------------------------*/
static void sayParameter(CONTEXT, int p, int form) {
	int i;

	for (i = 0; i <= p; i++)
		if (isEndOfArray(&globalParameters[i]))
			apperr("Nonexistent parameter referenced.");

#ifdef ALWAYS_SAY_PARAMETERS_USING_PLAYER_WORDS
	if (params[p].firstWord != EOD) /* Any words he used? */
		/* Yes, so use them... */
		sayPlayerWordsForParameter(p);
	else
		CALL2(sayForm(params[p].code, form)
#else
	if (globalParameters[p].useWords) {
		/* Ambiguous instance referenced, so use the words he used */
		sayPlayerWordsForParameter(p);
	} else {
		CALL2(sayForm, globalParameters[p].instance, (SayForm)form)
	}
#endif
}


/*----------------------------------------------------------------------

  Print an expanded symbolic reference.

  N = newline
  I = indent on a new line
  P = new paragraph
  L = current location name
  O = current object -> first parameter!
  <n> = n:th parameter
  +<n> = definite form of n:th parameter
  0<n> = indefinite form of n:th parameter
  !<n> = pronoun for the n:th parameter
  V = current verb
  A = current actor
  T = tabulation
  $ = no space needed after this, and don't capitalize
  _ = interpret this as a single dollar, if in doubt or conflict with other symbols

  str - The string starting with '$'
  */
static char *printSymbol(CONTEXT, char str[]) {
	int advance = 2;

	if (*str == '\0') printAndLog("$");
	else switch (toLower(str[1])) {
		case 'n':
			newline();
			needSpace = FALSE;
			break;
		case 'i':
			newline();
			printAndLog("    ");
			col = 5;
			needSpace = FALSE;
			break;
		case 'o':
			space();
			R0CALL2(sayParameter, 0, 0)
			needSpace = TRUE;       /* We did print something non-white */
			break;
		case '+':
		case '0':
		case '-':
		case '!':
			space();
			if (Common::isDigit((int)str[2])) {
				int form;
				switch (str[1]) {
				case '+':
					form = SAY_DEFINITE;
					break;
				case '0':
					form = SAY_INDEFINITE;
					break;
				case '-':
					form = SAY_NEGATIVE;
					break;
				case '!':
					form = SAY_PRONOUN;
					break;
				default:
					form = SAY_SIMPLE;
					break;
				}
				R0CALL2(sayParameter, str[2] - '1', form)
				needSpace = TRUE;
			}
			advance = 3;
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
			space();
			R0CALL2(sayParameter, str[1] - '1', SAY_SIMPLE)
			needSpace = TRUE;       /* We did print something non-white */
			break;
		case 'l':
			space();
			R0CALL1(say, current.location)
			needSpace = TRUE;       /* We did print something non-white */
			break;
		case 'a':
			space();
			R0CALL1(say, current.actor)
			needSpace = TRUE;       /* We did print something non-white */
			break;
		case 'v':
			space();
			justify((const char *)pointerTo(dictionary[verbWord].string));
			needSpace = TRUE;       /* We did print something non-white */
			break;
		case 'p':
			para();
			needSpace = FALSE;
			break;
		case 't': {
			int i;
			int spaces = 4 - (col - 1) % 4;

			for (i = 0; i < spaces; i++) printAndLog(" ");
			col = col + spaces;
			needSpace = FALSE;
			break;
		}
		case '$':
			skipSpace = TRUE;
			capitalize = FALSE;
			break;
		case '_':
			advance = 2;
			printAndLog("$");
			break;
		default:
			advance = 1;
			printAndLog("$");
			break;
		}

	return &str[advance];
}


/*----------------------------------------------------------------------*/
static bool inhibitSpace(char *str) {
	return str[0] != '\0' && str[0] == '$' && str[1] == '$';
}


/*----------------------------------------------------------------------*/
static bool isSpaceEquivalent(char str[]) {
	if (str[0] == ' ')
		return TRUE;
	else
		return strncmp(str, "$p", 2) == 0
		       || strncmp(str, "$n", 2) == 0
		       || strncmp(str, "$i", 2) == 0
		       || strncmp(str, "$t", 2) == 0;
}


/*----------------------------------------------------------------------*/
static bool punctuationNext(char *str) {
	const char *punctuation = strchr(".,!?", str[0]);
	bool end = str[1] == '\0';
	bool space = isSpaceEquivalent(&str[1]);
	return (punctuation != NULL && (end || space));
}


/*----------------------------------------------------------------------*/
static char lastCharOf(char *str) {
	return str[strlen(str) - 1];
}


/*======================================================================*/
void output(const char *original) {
	char ch;
	char *str, *copy;
	char *symptr;
	Context ctx;

	copy = scumm_strdup(original);
	str = copy;

	if (inhibitSpace(str) || punctuationNext(str))
		needSpace = FALSE;
	else
		space();            /* Output space if needed (& not inhibited) */

	/* Output string up to symbol and handle the symbol */
	while ((symptr = strchr(str, '$')) != (char *) NULL) {
		ch = *symptr;       /* Terminate before symbol */
		*symptr = '\0';
		if (strlen(str) > 0) {
			skipSpace = FALSE;    /* Only let skipSpace through if it is
                                     last in the string */
			if (lastCharOf(str) == ' ') {
				str[strlen(str) - 1] = '\0'; /* Truncate space character */
				justify(str);       /* Output part before '$' */
				needSpace = TRUE;
			} else {
				justify(str);       /* Output part before '$' */
				needSpace = FALSE;
			}
		}
		*symptr = ch;       /* restore '$' */
		str = printSymbol(ctx, symptr);  /* Print the symbolic reference and advance */
	}

	if (str[0] != 0) {
		justify(str);           /* Output trailing part */
		skipSpace = FALSE;
		if (lastCharOf(str) != ' ')
			needSpace = TRUE;
	}

	if (needSpace)
		capitalize = strchr("!?.", str[strlen(str) - 1]) != 0;

	anyOutput = TRUE;
	free(copy);
}


/*======================================================================*/
bool confirm(CONTEXT, MsgKind msgno) {
	char buf[80];
	bool flag;

	/* This is a bit of a hack since we really want to compare the input,
	   it could be affirmative, but for now any input is NOT! */
	printMessage(msgno);

	R0FUNC2(g_io->readLine, flag, buf, 80)
	if (!flag)
		return TRUE;
	col = 1;

	return (buf[0] == '\0');
}

} // End of namespace Alan3
} // End of namespace Glk
