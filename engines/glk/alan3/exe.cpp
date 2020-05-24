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

#include "glk/alan3/exe.h"
#include "glk/alan3/actor.h"
#include "glk/alan3/alan3.h"
#include "glk/alan3/current.h"
#include "glk/alan3/decode.h"
#include "glk/alan3/event.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/output.h"
#include "glk/alan3/options.h"
#include "glk/alan3/save.h"
#include "glk/alan3/score.h"
#include "glk/alan3/state.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/sysdep.h"
#include "glk/alan3/types.h"
#include "glk/alan3/utils.h"
#include "glk/alan3/word.h"
#include "common/stream.h"

namespace Glk {
namespace Alan3 {

// PUBLIC DATA
Common::SeekableReadStream *textFile;

// PUBLIC DATA - formerly method statics
bool printFlag;				// Printing already?

/* PRIVATE CONSTANTS */

#define WIDTH 80

/*======================================================================*/
void print(Aword fpos, Aword len) {
	char str[2 * WIDTH];          /* String buffer */
	uint outlen = 0;              /* Current output length */
	int ch = 0;
	int i;
	long savfp = 0;     /* Temporary saved text file position */
	bool savedPrintFlag = printFlag;
	void *info = NULL;      /* Saved decoding info */


	if (len == 0) return;

	if (isHere(HERO, /*TRUE*/ DIRECT)) {   /* Check if the player will see it */
		if (printFlag) {            /* Already printing? */
			/* Save current text file position and/or decoding info */
			if (header->pack)
				info = pushDecode();
			else
				savfp = textFile->pos();
		}
		printFlag = TRUE;           /* We're printing now! */

		/* Position to start of text */
		textFile->seek(fpos + header->stringOffset);

		if (header->pack)
			startDecoding();
		for (outlen = 0; outlen != len; outlen = outlen + strlen(str)) {
			/* Fill the buffer from the beginning */
			for (i = 0; i <= WIDTH || (i > WIDTH && ch != ' '); i++) {
				if (outlen + i == len)  /* No more characters? */
					break;
				if (header->pack)
					ch = decodeChar();
				else
					ch = textFile->readByte();

				if (ch == EOFChar)
					break;				// Or end of text?

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
				textFile->seek(savfp);
		}
	}
}


/*======================================================================*/
void sys(Aword fpos, Aword len) {
	syserr("sys calls are unsupported");
}


/*======================================================================*/
char *getStringFromFile(Aword fpos, Aword len) {
	char *buf = (char *)allocate(len + 1);
	char *bufp = buf;

	/* Position to start of text */
	textFile->seek(fpos + header->stringOffset);

	if (header->pack)
		startDecoding();
	while (len--)
		if (header->pack)
			*(bufp++) = decodeChar();
		else
			*(bufp++) = textFile->readByte();

	/* Terminate string with zero */
	*bufp = '\0';

	return buf;
}



/*======================================================================*/
void score(Aword sc) {
	if (sc == 0) {
		ParameterArray messageParameters = newParameterArray();
		addParameterForInteger(messageParameters, current.score);
		addParameterForInteger(messageParameters, header->maximumScore);
		addParameterForInteger(messageParameters, current.tick);
		printMessageWithParameters(M_SCORE, messageParameters);
		freeParameterArray(messageParameters);
	} else {
		current.score += scores[sc - 1];
		scores[sc - 1] = 0;
		gameStateChanged = TRUE;
	}
}


/*======================================================================*/
void visits(Aword v) {
	current.visits = v;
}


/*----------------------------------------------------------------------*/
static void sayUndoneCommand(char *words) {
	static Parameter *messageParameters = NULL;
	messageParameters = (Parameter *)ensureParameterArrayAllocated(messageParameters);

	current.location = where(HERO, DIRECT);
	clearParameterArray(messageParameters);
	addParameterForString(&messageParameters[0], words);
	setEndOfArray(&messageParameters[1]);
	printMessageWithParameters(M_UNDONE, messageParameters);
}


/*======================================================================*/
void undo(CONTEXT) {
	forgetGameState();
	if (anySavedState()) {
		recallGameState();
		sayUndoneCommand(recreatePlayerCommand());
	} else {
		printMessage(M_NO_UNDO);
	}

	LONG_JUMP_LABEL("returnUndo")
}


/*======================================================================*/
void quitGame(CONTEXT) {
	char buf[80];
	bool flag;

	current.location = where(HERO, DIRECT);
	para();
	while (!g_vm->shouldQuit()) {
		col = 1;
		CALL0(g_io->statusLine)
		printMessage(M_QUITACTION);

		FUNC2(g_io->readLine, flag, buf, 80)
		if (!flag)
			CALL1(terminate, 0)

		if (scumm_stricmp(buf, "restart") == 0) {
			LONG_JUMP_LABEL("restart")

		} else if (scumm_stricmp(buf, "restore") == 0) {
			g_vm->loadGame();
			return;

		} else if (scumm_stricmp(buf, "quit") == 0) {
			CALL1(terminate, 0)

		} else if (scumm_stricmp(buf, "undo") == 0) {
			if (gameStateChanged) {
				rememberCommands();
				rememberGameState();
				CALL0(undo)
			} else {
				if (anySavedState()) {
					recallGameState();
					sayUndoneCommand(playerWordsAsCommandString());
				} else {
					printMessage(M_NO_UNDO);
				}

				LONG_JUMP_LABEL("returnUndo")
			}
		}
	}

	syserr("Fallthrough in QUIT");
}



/*======================================================================*/
void restartGame(CONTEXT) {
	Aint previousLocation = current.location;
	current.location = where(HERO, DIRECT);
	para();

	bool flag;
	FUNC1(confirm, flag, M_REALLY)
	if (flag) {
		LONG_JUMP_LABEL("restart")
	}

	current.location = previousLocation;
}



/*======================================================================*/
void cancelEvent(Aword theEvent) {
	int i;

	for (i = eventQueueTop - 1; i >= 0; i--)
		if (eventQueue[i].event == (int)theEvent) {
			while (i < eventQueueTop - 1) {
				eventQueue[i].event = eventQueue[i + 1].event;
				eventQueue[i].after = eventQueue[i + 1].after;
				eventQueue[i].where = eventQueue[i + 1].where;
				i++;
			}
			eventQueueTop--;
			return;
		}
}


/*----------------------------------------------------------------------*/
static void increaseEventQueue(void) {
	eventQueue = (EventQueueEntry *)realloc(eventQueue, (eventQueueTop + 2) * sizeof(EventQueueEntry));
	if (eventQueue == NULL) syserr("Out of memory in increaseEventQueue()");

	eventQueueSize = eventQueueTop + 2;
}


/*----------------------------------------------------------------------*/
static void moveEvent(int to, int from) {
	eventQueue[to].event = eventQueue[from].event;
	eventQueue[to].after = eventQueue[from].after;
	eventQueue[to].where = eventQueue[from].where;
}


/*======================================================================*/
void schedule(Aword event, Aword where, Aword after) {
	uint i;

	if (event == 0) syserr("NULL event");

	cancelEvent(event);
	/* Check for overflow */
	if (eventQueue == nullptr || eventQueueTop == eventQueueSize) {
		increaseEventQueue();
		assert(eventQueue);
	}

	/* Bubble this event down */
	for (i = eventQueueTop; i >= 1 && eventQueue[i - 1].after <= (int)after; i--) {
		moveEvent(i, i - 1);
	}

	eventQueue[i].after = after;
	eventQueue[i].where = where;
	eventQueue[i].event = event;
	eventQueueTop++;
}


// TODO Move to string.c?
/*======================================================================*/
Aptr concat(Aptr as1, Aptr as2) {
	char *s1 = (char *)fromAptr(as1);
	char *s2 = (char *)fromAptr(as2);
	char *result = (char *)allocate(strlen((char *)s1) + strlen((char *)s2) + 1);
	strcpy(result, s1);
	strcat(result, s2);
	return toAptr(result);
}


/*----------------------------------------------------------------------*/
static char *stripCharsFromStringForwards(int count, char *initialString, char **theRest) {
	int stripPosition;
	char *strippedString;
	char *rest;

	if (count > (int)strlen(initialString))
		stripPosition = strlen(initialString);
	else
		stripPosition = count;
	rest = scumm_strdup(&initialString[stripPosition]);
	strippedString = scumm_strdup(initialString);
	strippedString[stripPosition] = '\0';
	*theRest = rest;
	return strippedString;
}

/*----------------------------------------------------------------------*/
static char *stripCharsFromStringBackwards(Aint count, char *initialString, char **theRest) {
	int stripPosition;
	char *strippedString;
	char *rest;

	if (count > (int)strlen(initialString))
		stripPosition = 0;
	else
		stripPosition = strlen(initialString) - count;
	strippedString = scumm_strdup(&initialString[stripPosition]);
	rest = scumm_strdup(initialString);
	rest[stripPosition] = '\0';
	*theRest = rest;
	return strippedString;
}


/*----------------------------------------------------------------------*/
static int countLeadingBlanks(char *string, int position) {
	static char blanks[] = " ";
	return strspn(&string[position], blanks);
}


/*----------------------------------------------------------------------*/
static int skipWordForwards(char *string, int position) {
	char separators[] = " .,?";

	uint i;

	for (i = position; i <= strlen(string) && strchr(separators, string[i]) == NULL; i++)
		;
	return i;
}


/*----------------------------------------------------------------------*/
static char *stripWordsFromStringForwards(Aint count, char *initialString, char **theRest) {
	int skippedChars;
	int position = 0;
	char *stripped;
	int i;

	for (i = count; i > 0; i--) {
		/* Ignore any initial blanks */
		skippedChars = countLeadingBlanks(initialString, position);
		position += skippedChars;
		position = skipWordForwards(initialString, position);
	}

	stripped = (char *)allocate(position + 1);
	strncpy(stripped, initialString, position);
	stripped[position] = '\0';

	skippedChars = countLeadingBlanks(initialString, position);
	*theRest = scumm_strdup(&initialString[position + skippedChars]);

	return (stripped);
}


/*----------------------------------------------------------------------*/
static int skipWordBackwards(char *string, int position) {
	char separators[] = " .,?";
	int i;

	for (i = position; i > 0 && strchr(separators, string[i - 1]) == NULL; i--)
		;
	return i;
}


/*----------------------------------------------------------------------*/
static int countTrailingBlanks(char *string, int position) {
	int skippedChars, i;
	skippedChars = 0;

	if (position > (int)strlen(string) - 1)
		syserr("position > length in countTrailingBlanks");
	for (i = position; i >= 0 && string[i] == ' '; i--)
		skippedChars++;
	return (skippedChars);
}


/*----------------------------------------------------------------------*/
static char *stripWordsFromStringBackwards(Aint count, char *initialString, char **theRest) {
	int skippedChars;
	char *stripped;
	int strippedLength;
	int position = strlen(initialString);
	int i;

	for (i = count; i > 0 && position > 0; i--) {
		position -= 1;
		/* Ignore trailing blanks */
		skippedChars = countTrailingBlanks(initialString, position);
		if (position - skippedChars < 0) break; /* No more words to strip */
		position -= skippedChars;
		position = skipWordBackwards(initialString, position);
	}

	skippedChars = countLeadingBlanks(initialString, 0);
	strippedLength = strlen(initialString) - position - skippedChars;
	stripped = (char *)allocate(strippedLength + 1);
	strncpy(stripped, &initialString[position + skippedChars], strippedLength);
	stripped[strippedLength] = '\0';

	if (position > 0) {
		skippedChars = countTrailingBlanks(initialString, position - 1);
		position -= skippedChars;
	}
	*theRest = scumm_strdup(initialString);
	(*theRest)[position] = '\0';
	return (stripped);
}



/*======================================================================*/
Aptr strip(bool stripFromBeginningNotEnd, int count, bool stripWordsNotChars, int id, int atr) {
	char *initialString = (char *)fromAptr(getInstanceAttribute(id, atr));
	char *theStripped;
	char *theRest;

	if (stripFromBeginningNotEnd) {
		if (stripWordsNotChars)
			theStripped = stripWordsFromStringForwards(count, initialString, &theRest);
		else
			theStripped = stripCharsFromStringForwards(count, initialString, &theRest);
	} else {
		if (stripWordsNotChars)
			theStripped = stripWordsFromStringBackwards(count, initialString, &theRest);
		else
			theStripped = stripCharsFromStringBackwards(count, initialString, &theRest);
	}
	setInstanceStringAttribute(id, atr, theRest);
	return toAptr(theStripped);
}


/*======================================================================*/
int getContainerMember(int container, int index, bool directly) {
	uint i;
	Aint count = 0;

	for (i = 1; i <= header->instanceMax; i++) {
		if (isIn(i, container, DIRECT)) {
			count++;
			if (count == index)
				return i;
		}
	}
	apperr("Index not in container in 'containerMember()'");
	return 0;
}


/***********************************************************************\

  Description Handling

\***********************************************************************/


/*======================================================================*/
void empty(CONTEXT, int cnt, int whr) {
	uint i;

	for (i = 1; i <= header->instanceMax; i++)
		if (isIn(i, cnt, DIRECT))
			CALL2(locate, i, whr)
}



/*======================================================================*/
void use(CONTEXT, int actor, int script) {
	char str[80];
	StepEntry *step;

	if (!isAActor(actor)) {
		sprintf(str, "Instance is not an Actor (%d).", actor);
		syserr(str);
	}

	admin[actor].script = script;
	admin[actor].step = 0;
	step = stepOf(actor);
	if (step != NULL && step->after != 0) {
		FUNC1(evaluate, admin[actor].waitCount, step->after)
	}

	gameStateChanged = TRUE;
}

/*======================================================================*/
void stop(int act) {
	char str[80];

	if (!isAActor(act)) {
		sprintf(str, "Instance is not an Actor (%d).", act);
		syserr(str);
	}

	admin[act].script = 0;
	admin[act].step = 0;

	gameStateChanged = TRUE;
}



static int randomValue = 0;
/*----------------------------------------------------------------------*/
int randomInteger(int from, int to) {
	if (regressionTestOption) {
		int ret = from + randomValue;
		/* Generate them in sequence */
		if (ret > to) {
			ret = from;
			randomValue = 1;
		} else if (ret == to)
			randomValue = 0;
		else
			randomValue++;
		return ret;
	} else {
		if (to == from)
			return to;
		else if (to > from)
			return (g_vm->getRandomNumber(0x7fffffff) / 10) % (to - from + 1) + from;
		else
			return (g_vm->getRandomNumber(0x7fffffff) / 10) % (from - to + 1) + to;
	}
}



/*----------------------------------------------------------------------*/
bool between(int val, int low, int high) {
	if (high > low)
		return low <= val && val <= high;
	else
		return high <= val && val <= low;
}



/*======================================================================*/
bool contains(Aptr string, Aptr substring) {
	bool found;

	strlow((char *)fromAptr(string));
	strlow((char *)fromAptr(substring));

	found = (strstr((char *)fromAptr(string), (char *)fromAptr(substring)) != 0);

	return found;
}


/*======================================================================*/
bool streq(char a[], char b[]) {
	bool eq;

	strlow(a);
	strlow(b);

	eq = (strcmp(a, b) == 0);

	return eq;
}



/*======================================================================*/
void startTranscript(void) {
	if (logFile == NULL) {
		Common::String filename = g_vm->getTargetName() + ".log";

		uint fileUsage = transcriptOption ? fileusage_Transcript : fileusage_InputRecord;
		frefid_t logFileRef = g_vm->glk_fileref_create_by_name(fileUsage, filename.c_str(), 0);
		logFile = g_vm->glk_stream_open_file(logFileRef, filemode_Write, 0);

		if (logFile == NULL) {
			transcriptOption = FALSE;
			logOption = FALSE;
		} else {
			transcriptOption = TRUE;
		}
	}
}


/*======================================================================*/
void stopTranscript(void) {
	if (logFile != NULL) {
		if (transcriptOption || logOption)
			delete logFile;

		logFile = NULL;
		transcriptOption = FALSE;
		logOption = FALSE;
	}
}

} // End of namespace Alan3
} // End of namespace Glk
