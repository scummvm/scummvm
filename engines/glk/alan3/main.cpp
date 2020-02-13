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

#include "glk/alan3/main.h"
#include "glk/alan3/alan_version.h"
#include "glk/alan3/class.h"
#include "glk/alan3/compatibility.h"
#include "glk/alan3/container.h"
#include "glk/alan3/current.h"
#include "glk/alan3/debug.h"
#include "glk/alan3/decode.h"
#include "glk/alan3/dictionary.h"
#include "glk/alan3/event.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/inter.h"
#include "glk/jumps.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/literal.h"
#include "glk/alan3/location.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/options.h"
#include "glk/alan3/output.h"
#include "glk/alan3/parse.h"
#include "glk/alan3/reverse.h"
#include "glk/alan3/rules.h"
#include "glk/alan3/scan.h"
#include "glk/alan3/score.h"
#include "glk/alan3/state.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/syntax.h"
#include "glk/alan3/utils.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */

/* Amachine structures - Static */
VerbEntry *vrbs;        /* Verb table pointer */


/* PRIVATE DATA */
#define STACKSIZE 100

/*----------------------------------------------------------------------*
 *
 * Event Handling
 *
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
static char *eventName(int event) {
	return stringAt(events[event].id);
}


/*----------------------------------------------------------------------*/
static void runPendingEvents(CONTEXT) {
	int i;

	resetRules();
	while (eventQueueTop != 0 && eventQueue[eventQueueTop - 1].after == 0) {
		eventQueueTop--;
		if (isALocation(eventQueue[eventQueueTop].where))
			current.location = eventQueue[eventQueueTop].where;
		else
			current.location = where(eventQueue[eventQueueTop].where, TRANSITIVE);
		if (traceSectionOption) {
			printf("\n<EVENT %s[%d] (at ", eventName(eventQueue[eventQueueTop].event),
			       eventQueue[eventQueueTop].event);
			CALL1(traceSay, current.location)
			printf(" [%d]):>\n", current.location);
		}
		CALL1(interpret, events[eventQueue[eventQueueTop].event].code)
		CALL1(evaluateRules, rules)
	}

	for (i = 0; i < eventQueueTop; i++)
		eventQueue[i].after--;
}


/*----------------------------------------------------------------------*\

  Main program and initialisation

\*----------------------------------------------------------------------*/

Common::SeekableReadStream *codfil;

/*----------------------------------------------------------------------
  Calculate where to start calculating the CRC. Is different for
  different versions. CRC is calculated from pre-beta2 memory start to
  be compatible.  If header size changes this should return beta2
  header size for later versions.
*/
static int crcStart(const byte version[4]) {
	/* Some earlier versions had a shorter header */
	if (isPreAlpha5(version))
		return sizeof(Pre3_0alpha5Header) / sizeof(Aword);
	else if (isPreBeta2(version))
		return sizeof(Pre3_0beta2Header) / sizeof(Aword);
	else
		return sizeof(ACodeHeader) / sizeof(Aword);
}


/*----------------------------------------------------------------------*/
static void readTemporaryHeader(CONTEXT, ACodeHeader *tmphdr) {
	codfil->seek(0);
	if (codfil->read(&tmphdr->tag[0], sizeof(ACodeHeader)) != sizeof(ACodeHeader) ||
			strncmp((char *)tmphdr, "ALAN", 4) != 0)
		playererr(context, "Not an Alan game file, does not start with \"ALAN\"");
}


/*----------------------------------------------------------------------*/
#ifdef SCUMM_LITTLE_ENDIAN
static void reverseMemory() {
	if (debugOption || traceSectionOption || traceInstructionOption)
		output("<Hmm, this is a little-endian machine, fixing byte ordering....");
	reverseACD();           /* Reverse content of the ACD file */
	if (debugOption || traceSectionOption || traceInstructionOption)
		output("OK.>$n");
}
#endif

/*----------------------------------------------------------------------*/
static void setupHeader(ACodeHeader tmphdr) {
	if (isPreBeta2(tmphdr.version)) {
		header = (ACodeHeader *)duplicate(&memory[0], sizeof(ACodeHeader));
		if (isPreAlpha5(tmphdr.version)) {
			header->ifids = 0;
		}
		header->prompt = 0;
	} else if (isPreBeta3(tmphdr.version)) {
		header = (ACodeHeader *)pointerTo(0);
	} else {
		header = (ACodeHeader *)pointerTo(0);
	}
}


/*----------------------------------------------------------------------*/
static void loadAndCheckMemory(ACodeHeader tmphdr, Aword crc, char err[]) {
	int i;
	/* No memory allocated yet? */
	if (memory == NULL) {
		memory = (Aword *)allocate(tmphdr.size * sizeof(Aword));
	}

	memTop = tmphdr.size;
	codfil->seek(0);
	if (codfil->read(memory, sizeof(Aword) * memTop) != (sizeof(Aword) * memTop))
		syserr("Could not read all ACD code.");

	/* Calculate checksum */
	for (i = crcStart(tmphdr.version); i < memTop; i++) {
		crc += memory[i] & 0xff;
		crc += (memory[i] >> 8) & 0xff;
		crc += (memory[i] >> 16) & 0xff;
		crc += (memory[i] >> 24) & 0xff;
	}
	if (crc != tmphdr.acdcrc) {
		sprintf(err, "Checksum error in Acode (.a3c) file (0x%lx instead of 0x%lx).",
		        (unsigned long) crc, (unsigned long) tmphdr.acdcrc);
		if (!ignoreErrorOption)
			syserr(err);
		else {
			output("<WARNING! $$");
			output(err);
			output("$$ Ignored, proceed at your own risk.>$n");
		}
	}
}


/*----------------------------------------------------------------------*/
static const char *decodeState(int c) {
	static char state[3] = "\0\0";
	switch (c) {
	case 0:
		return ".";
	case 'd':
		return "dev";
	case 'a':
		return "alpha";
	case 'b':
		return "beta";
	default:
		state[0] = header->version[3];
		return state;
	}
}

/*======================================================================*/
char *decodedGameVersion(const byte version[]) {
	static char str[100];
	sprintf(str, "%d.%d%s%d",
	        (int)version[3],
	        (int)version[2],
	        decodeState(version[0]),
	        (int)version[1]);
	return str;
}

/*----------------------------------------------------------------------*/
static void incompatibleDevelopmentVersion(ACodeHeader *hdr) {
	Common::String msg = Common::String::format("Incompatible version of ACODE program. "
		"Development versions always require exact match. Game is %ld.%ld%s%ld, interpreter %ld.%ld%s%ld!",
	        (long)(hdr->version[0]),
	        (long)(hdr->version[1]),
	        decodeState(hdr->version[3]),
	        (long)(hdr->version[2]),
	        (long)alan.version.version,
	        (long)alan.version.revision,
	        alan.version.state,
	        (long)alan.version.correction);
	apperr(msg.c_str());
}


/*----------------------------------------------------------------------*/
static void incompatibleVersion(ACodeHeader *hdr) {
	Common::String msg = Common::String::format("Incompatible version of ACODE program. Game is %ld.%ld, interpreter %ld.%ld.",
	        (long)(hdr->version[0]),
	        (long)(hdr->version[1]),
	        (long)alan.version.version,
	        (long)alan.version.revision);
	apperr(msg.c_str());
}


/*----------------------------------------------------------------------*/
static void alphaRunningLaterGame(char gameState) {
	output("<WARNING! You are running an alpha interpreter, but the game is generated by a");
	if (gameState == 'b')
		output("beta");
	else
		output("release");
	output("state compiler which was released later. This might cause the game to not work fully as intended. Look for an upgraded game file.>\n");
}

/*----------------------------------------------------------------------*/
static void nonDevelopmentRunningDevelopmentStateGame(const byte version[]) {
	char errorMessage[200];
	char versionString[100];

	strcpy(errorMessage, "Games generated by a development state compiler");
	sprintf(versionString, "(this game is v%d.%d.%d%s)", version[0], version[1],
	        version[2], decodeState(version[3]));
	strcat(errorMessage, versionString);
	strcat(errorMessage, "can only be run with a matching interpreter. Look for a game file generated with an alpha, beta or release state compiler.>\n");
	apperr(errorMessage);
}


/*======================================================================*/
void checkVersion(ACodeHeader *hdr) {
	/* Strategy for version matching is:
	   1) Development interpreters/games require exact match
	   2) Alpha, Beta and Release interpreters will not run development games
	   3) Alpha interpreters must warn if they run beta or release games
	   4) Beta interpreters may introduce changes which are not alpha compatible,
	   if the change is a strict addition (i.e. if not used will not affect
	   alpha interpreters, example is introduction of a new opcode if it is
	   done at the end of the list)
	   5) Release interpreters should run alpha and beta games without problems

	   NOTE that we are working with a non-reversed version string/word here.
	*/

	char interpreterVersion[4];
	bool developmentVersion;
	bool alphaVersion;
	int compareLength;
	char gameState = hdr->version[3];

	/* Construct our own version */
	interpreterVersion[0] = alan.version.version;
	interpreterVersion[1] = alan.version.revision;
	interpreterVersion[2] = alan.version.correction;
	interpreterVersion[3] = alan.version.state[0];

	/* Check version of .ACD file */
	if (debugOption && !regressionTestOption) {
		printf("<Version of '%s' is %d.%d%s%d!>\n",
		       g_vm->getFilename().c_str(),
		       (int)hdr->version[0],
		       (int)hdr->version[1],
		       decodeState(hdr->version[3]),
		       (int)hdr->version[2]);
		newline();
	}

	/* Development version require exact match, else only 2 digit match */
	developmentVersion = (strcmp(alan.version.state, "dev") == 0);
	alphaVersion = (strcmp(alan.version.state, "alpha") == 0);
	compareLength = (developmentVersion ? 3 : 2);

	if (gameState == 'd' && !developmentVersion)
		/* Development state game requires development state interpreter... */
		nonDevelopmentRunningDevelopmentStateGame(hdr->version);
	else {
		/* Compatible if version, revision (and correction if dev state) match... */
		if (memcmp(hdr->version, interpreterVersion, compareLength) != 0) {
			/* Mismatch! */
			if (!ignoreErrorOption) {
				if (developmentVersion)
					incompatibleDevelopmentVersion(hdr);
				else
					incompatibleVersion(hdr);
			} else
				output("<WARNING! Incompatible version of ACODE program.>\n");
		} else if (developmentVersion && gameState != 'd')
			/* ... unless interpreter is development and game not */
			incompatibleDevelopmentVersion(hdr);
		else if (alphaVersion && gameState != 'a') {
			/* If interpreter is alpha version and the game is later, warn! */
			alphaRunningLaterGame(gameState);
		}
	}
}

/*----------------------------------------------------------------------*/
static void load(CONTEXT) {
	ACodeHeader tmphdr;
	Aword crc = 0;
	char err[100];

	CALL1(readTemporaryHeader, &tmphdr)
	checkVersion(&tmphdr);

	/* Allocate and load memory */
#ifdef SCUMM_LITTLE_ENDIAN
	reverseHdr(&tmphdr);
#endif

	if (tmphdr.size <= sizeof(ACodeHeader) / sizeof(Aword))
		syserr("Malformed game file. Too small.");

	loadAndCheckMemory(tmphdr, crc, err);

#ifdef SCUMM_LITTLE_ENDIAN
	reverseMemory();
#endif
	setupHeader(tmphdr);
}


/*----------------------------------------------------------------------*/
static void checkDebug(CONTEXT) {
	/* Make sure he can't debug if not allowed! */
	if (!header->debug) {
		if (debugOption | traceSectionOption | traceInstructionOption) {
			printf("<Sorry, '%s' is not compiled for debug! Exiting.>\n", g_vm->getFilename().c_str());
			CALL1(terminate, 0)
		}
		para();
		debugOption = FALSE;
		traceSectionOption = FALSE;
		traceInstructionOption = FALSE;
		tracePushOption = FALSE;
	}

	// If debugging, use no randomization
	if (debugOption || regressionTestOption)
		g_vm->setRandomNumberSeed(1);
}


/*----------------------------------------------------------------------*/
static void initStaticData(void) {
	/* Dictionary */
	dictionary = (DictionaryEntry *) pointerTo(header->dictionary);
	/* Find out number of entries in dictionary */
	for (dictionarySize = 0; !isEndOfArray(&dictionary[dictionarySize]); dictionarySize++);

	/* Scores */


	/* All addresses to tables indexed by ids are converted to pointers,
	   then adjusted to point to the (imaginary) element before the
	   actual table so that [0] does not exist. Instead indices goes
	   from 1 and we can use [1]. */

	if (header->instanceTableAddress == 0)
		syserr("Instance table pointer == 0");
	instances = (InstanceEntry *) pointerTo(header->instanceTableAddress);
	instances--;            /* Back up one so that first is no. 1 */


	if (header->classTableAddress == 0)
		syserr("Class table pointer == 0");
	classes = (ClassEntry *) pointerTo(header->classTableAddress);
	classes--;          /* Back up one so that first is no. 1 */

	if (header->containerTableAddress != 0) {
		containers = (ContainerEntry *) pointerTo(header->containerTableAddress);
		containers--;
	}

	if (header->eventTableAddress != 0) {
		events = (EventEntry *) pointerTo(header->eventTableAddress);
		events--;
	}

	/* Scores, if already allocated, copy initial data */
	if (scores == NULL)
		scores = (Aword *)duplicate((Aword *) pointerTo(header->scores), header->scoreCount * sizeof(Aword));
	else
		memcpy(scores, pointerTo(header->scores), header->scoreCount * sizeof(Aword));

	if (literals == NULL)
		literals = (LiteralEntry *)allocate(sizeof(Aword) * (MAXPARAMS + 1));

	stxs = (SyntaxEntry *) pointerTo(header->syntaxTableAddress);
	vrbs = (VerbEntry *) pointerTo(header->verbTableAddress);
	msgs = (MessageEntry *) pointerTo(header->messageTableAddress);
	initRules(header->ruleTableAddress);

	if (header->pack)
		freq = (Aword *) pointerTo(header->freq);
}


/*----------------------------------------------------------------------*/
static void initStrings(void) {
	StringInitEntry *init;

	for (init = (StringInitEntry *) pointerTo(header->stringInitTable); !isEndOfArray(init); init++)
		setInstanceAttribute(init->instanceCode, init->attributeCode, toAptr(getStringFromFile(init->fpos, init->len)));
}

/*----------------------------------------------------------------------*/
static Aint sizeOfAttributeData(void) {
	uint i;
	int size = 0;

	for (i = 1; i <= header->instanceMax; i++) {
		AttributeEntry *attribute = (AttributeEntry *)pointerTo(instances[i].initialAttributes);
		while (!isEndOfArray(attribute)) {
			size += AwordSizeOf(AttributeEntry);
			attribute++;
		}
		size += 1;          /* For EOD */
	}

	if (size != header->attributesAreaSize
	        && (sizeof(AttributeHeaderEntry) == sizeof(AttributeEntry)))
		syserr("Attribute area size calculated wrong.");
	return size;
}


/*----------------------------------------------------------------------*/
static AttributeEntry *initializeAttributes(int awordSize) {
	Aword *attributeArea = (Aword *)allocate(awordSize * sizeof(Aword));
	Aword *currentAttributeArea = attributeArea;
	uint i;

	for (i = 1; i <= header->instanceMax; i++) {
		AttributeHeaderEntry *originalAttribute = (AttributeHeaderEntry *)pointerTo(instances[i].initialAttributes);
		admin[i].attributes = (AttributeEntry *)currentAttributeArea;
		while (!isEndOfArray(originalAttribute)) {
			((AttributeEntry *)currentAttributeArea)->code = originalAttribute->code;
			((AttributeEntry *)currentAttributeArea)->value = originalAttribute->value;
			((AttributeEntry *)currentAttributeArea)->id = originalAttribute->id;
			currentAttributeArea += AwordSizeOf(AttributeEntry);
			originalAttribute++;
		}
		*((Aword *)currentAttributeArea) = EOD;
		currentAttributeArea += 1;
	}

	return (AttributeEntry *)attributeArea;
}




/*----------------------------------------------------------------------*/
static void initDynamicData(void) {
	uint instanceId;

	/* Allocate for administrative table */
	admin = (AdminEntry *)allocate((header->instanceMax + 1) * sizeof(AdminEntry));

	/* Create game state copy of attributes */
	attributes = initializeAttributes(sizeOfAttributeData());

	/* Initialise string & set attributes */
	initStrings();
	initSets((SetInitEntry *)pointerTo(header->setInitTable));

	/* Set initial locations */
	for (instanceId = 1; instanceId <= header->instanceMax; instanceId++)
		admin[instanceId].location = instances[instanceId].initialLocation;
}


/*----------------------------------------------------------------------*/
static void runInheritedInitialize(CONTEXT, Aint theClass) {
	if (theClass == 0) return;
	CALL1(runInheritedInitialize, classes[theClass].parent)

	if (classes[theClass].initialize)
		interpret(context, classes[theClass].initialize);
}


/*----------------------------------------------------------------------*/
static void runInitialize(CONTEXT, Aint theInstance) {
	CALL1(runInheritedInitialize, instances[theInstance].parent)

	if (instances[theInstance].initialize != 0)
		interpret(context, instances[theInstance].initialize);
}


/*----------------------------------------------------------------------*/
static void initializeInstances(CONTEXT) {
	uint instanceId;

	/* Set initial locations */
	for (instanceId = 1; instanceId <= header->instanceMax; instanceId++) {
		current.instance = instanceId;
		CALL1(runInitialize, instanceId)
	}
}


/*----------------------------------------------------------------------*/
static void start(CONTEXT) {
	int startloc;

	current.tick = 0;
	current.location = startloc = where(HERO, TRANSITIVE);
	current.actor = HERO;
	current.score = 0;

	CALL0(initializeInstances)

	if (traceSectionOption)
		printf("\n<START:>\n");
	CALL1(interpret, header->start)
	para();

	if (where(HERO, TRANSITIVE) == startloc) {
		if (traceSectionOption)
			printf("<CURRENT LOCATION:>");
		CALL0(look)
	}

	resetAndEvaluateRules(context, rules, header->version);
}


/*----------------------------------------------------------------------*/
static void openFiles(void) {
	/* If logging open log file */
	if (transcriptOption || logOption) {
		startTranscript();
	}
}


/*----------------------------------------------------------------------*/
static void init(CONTEXT) {
	int i;

	/* Initialise some status */
	eventQueueTop = 0;          /* No pending events */
	initStaticData();
	initDynamicData();
	initParsing();
	CALL0(checkDebug)

	getPageSize();

	/* Find first conjunction and use that for ',' handling */
	for (i = 0; i < dictionarySize; i++)
		if (isConjunction(i)) {
			conjWord = i;
			break;
		}

	/* Start the adventure */
	if (debugOption) {
		CALL3(debug, FALSE, 0, 0)
	} else {
		clear();
	}

	start(context);
}



/*----------------------------------------------------------------------*/
static bool traceActor(CONTEXT, int theActor) {
	if (traceSectionOption) {
		printf("\n<ACTOR ");
		R0CALL1(traceSay, theActor)
		printf("[%d]", theActor);
		if (current.location != 0) {
			printf(" (at ");
			R0CALL1(traceSay, current.location)
		} else
			printf(" (nowhere");
		printf("[%d])", current.location);
	}
	return traceSectionOption;
}


/*----------------------------------------------------------------------*/
static char *scriptName(int theActor, int theScript) {
	ScriptEntry *scriptEntry = (ScriptEntry *)pointerTo(header->scriptTableAddress);

	while (theScript > 1) {
		scriptEntry++;
		theScript--;
	}
	return (char *)pointerTo(scriptEntry->id);
}


/*----------------------------------------------------------------------*/
static void moveActor(CONTEXT, int theActor) {
	ScriptEntry *scr;
	StepEntry *step;
	Aint previousInstance = current.instance;
	bool flag;

	if (context._break) {
		// forfeit setjmp replacement destination
		assert(context._label == "forfeit");
		context.clear();
		current.instance = previousInstance;
		return;
	}

	current.actor = theActor;
	current.instance = theActor;
	current.location = where(theActor, TRANSITIVE);

	if (theActor == (int)HERO) {
		// Ask him!
		CALL0(parse)
		capitalize = TRUE;
		fail = FALSE;           // fail only aborts one actor

	} else if (admin[theActor].script != 0) {
		for (scr = (ScriptEntry *) pointerTo(header->scriptTableAddress); !isEndOfArray(scr); scr++) {
			if (scr->code == admin[theActor].script) {
				/* Find correct step in the list by indexing */
				step = (StepEntry *) pointerTo(scr->steps);
				step = (StepEntry *) &step[admin[theActor].step];
				/* Now execute it, maybe. First check wait count */
				if (admin[theActor].waitCount > 0) { /* Wait some more ? */
					FUNC1(traceActor, flag, theActor)
					if (flag)
						printf(", SCRIPT %s[%ld], STEP %ld, Waiting another %ld turns>\n",
						       scriptName(theActor, admin[theActor].script),
						       (long)admin[theActor].script, (long)admin[theActor].step + 1,
						       (long)admin[theActor].waitCount);

					admin[theActor].waitCount--;
					break;
				}
				/* Then check possible expression to wait for */
				if (step->exp != 0) {
					FUNC1(traceActor, flag, theActor)
					if (flag)
							printf(", SCRIPT %s[%ld], STEP %ld, Evaluating:>\n",
						       scriptName(theActor, admin[theActor].script),
						       (long)admin[theActor].script, (long)admin[theActor].step + 1);
					FUNC1(evaluate, flag, step->exp)
					if (!flag)
						break;      /* Break loop, don't execute step*/
				}
				/* OK, so finally let him do his thing */
				admin[theActor].step++;     /* Increment step number before executing... */
				if (!isEndOfArray(step + 1) && (step + 1)->after != 0) {
					FUNC1(evaluate, admin[theActor].waitCount, (step + 1)->after)
				}

				FUNC1(traceActor, flag, theActor)
				if (flag)
					printf(", SCRIPT %s[%ld], STEP %ld, Executing:>\n",
					       scriptName(theActor, admin[theActor].script),
					       (long)admin[theActor].script,
					       (long)admin[theActor].step);
				CALL1(interpret, step->stms)
				step++;
				/* ... so that we can see if he failed or is USEing another script now */
				if (fail || (admin[theActor].step != 0 && isEndOfArray(step)))
					/* No more steps in this script, so stop him */
					admin[theActor].script = 0;
				fail = FALSE;           /* fail only aborts one actor */
				break;          /* We have executed a script so leave loop */
			}
		}
		if (isEndOfArray(scr))
			syserr("Unknown actor script.");
	} else {
		FUNC1(traceActor, flag, theActor)
		if (flag) {
			printf(", Idle>\n");
		}
	}

	current.instance = previousInstance;
}

/*======================================================================*/
void run(void) {
	Stack theStack = NULL;
	Context ctx;

	openFiles();
	load(ctx);			// Load program

	do {
		ctx.clear();
		if (ctx._break)
			break;

		if (theStack)
			deleteStack(theStack);

		theStack = createStack(STACKSIZE);
		setInterpreterStack(theStack);

		initStateStack();

		// Initialise and start the adventure
		init(ctx);

		while (!g_vm->shouldQuit()) {
			if (!(ctx._break && ctx._label == "forfeit")) {
				if (ctx._break) {
					assert(ctx._label.hasPrefix("return"));

					if (ctx._label == "returnError") {
						forgetGameState();
						forceNewPlayerInput();
					} else if (ctx._label == "returnUndo") {
						forceNewPlayerInput();
					}

					ctx.clear();
				} else {
					if (debugOption)
						debug(ctx, FALSE, 0, 0);

					if (!ctx._break) {
						if (stackDepth(theStack) != 0)
							syserr("Stack is not empty in main loop");

						if (!current.meta)
							runPendingEvents(ctx);
					}
				}

				recursionDepth = 0;

				// Move all characters, hero first
				rememberGameState();
				current.meta = FALSE;
			}

			moveActor(ctx, header->theHero);

			if (!ctx._break) {
				if (gameStateChanged)
					rememberCommands();
				else
					forgetGameState();

				if (!current.meta) {
					current.tick++;

					// Remove this call? Since Eval is done up there after each event...
					resetAndEvaluateRules(ctx, rules, header->version);

					if (!ctx._break) {
						// Then all the other actors...
						for (uint i = 1; i <= header->instanceMax; i++) {
							if (i != header->theHero && isAActor(i)) {
								moveActor(ctx, i);
								if (ctx._break)
									break;

								resetAndEvaluateRules(ctx, rules, header->version);
								if (ctx._break)
									break;
							}
						}
					}
				}
			}

			if (ctx._break && ctx._label == "restart")
				break;
		}
	} while (!g_vm->shouldQuit() && ctx._label == "restart");
}

} // End of namespace Alan3
} // End of namespace Glk
