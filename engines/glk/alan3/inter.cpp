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

#include "glk/alan3/inter.h"
#include "glk/alan3/alan3.h"
#include "glk/alan3/current.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/debug.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/options.h"
#include "glk/alan3/save.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/output.h"
#include "glk/alan3/score.h"
#include "glk/alan3/params.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/container.h"
#include "glk/alan3/location.h"
#include "glk/alan3/compatibility.h"

#ifdef HAVE_GLK
#define MAP_STDIO_TO_GLK
#include "glk/alan3/glkio.h"
#endif

namespace Glk {
namespace Alan3 {

bool stopAtNextLine = FALSE;
bool fail = FALSE;


/* PRIVATE DATA */

static int pc;
static Stack stack = NULL;

static void (*interpreterMock)(Aaddr adr) = NULL;


/*======================================================================*/
void setInterpreterMock(void (*mock)(Aaddr adr)) {
	interpreterMock = mock;
}


/*======================================================================*/
void setInterpreterStack(Stack theStack) {
	stack = theStack;
}


/*----------------------------------------------------------------------*/
static void traceInstruction(const char *str, ...) {
	va_list args;

	if (traceInstructionOption) {
		va_start(args, str);
		Common::String msg = Common::String::format(str, args);
		va_end(args);

		printf("%s", msg.c_str());
	}
}


/*----------------------------------------------------------------------*/
static void traceSkip() {
	printf("\n    : \t\t\t\t\t\t\t");
}


/*----------------------------------------------------------------------*/
static void interpretIf(Aword v) {
	int lev = 1;
	Aword i;

	if (!v) {
		/* Skip to next ELSE or ENDIF on same level */
		if (traceInstructionOption) traceSkip();
		while (TRUE) {
			i = memory[pc++];
			if (I_CLASS(i) == (Aword)C_STMOP)
				switch (I_OP(i)) {
				case I_ELSE:
					if (lev == 1) {
						if (traceInstructionOption)
							printf("\n%4x: ELSE\t\t\t\t\t\t", pc);
						return;
					}
					break;
				case I_IF:
					lev++;
					break;
				case I_ENDIF:
					lev--;
					if (lev == 0) {
						if (traceInstructionOption)
							printf("\n%4x: ENDIF\t\t\t\t\t\t", pc);
						return;
					}
					break;
				default:
					break;
				}
		}
	}
}


/*----------------------------------------------------------------------*/
static void interpretElse(void) {
	int lev = 1;
	Aword i;

	if (traceInstructionOption) traceSkip();
	while (TRUE) {
		/* Skip to ENDIF on the same level */
		i = memory[pc++];
		if (I_CLASS(i) == (Aword)C_STMOP)
			switch (I_OP(i)) {
			case I_ENDIF:
				lev--;
				if (lev == 0) return;
				break;
			case I_IF:
				lev++;
				break;
			default:
				break;
			}
	}
}


/*----------------------------------------------------------------------*/
static void goToLOOPEND(void) {
	int level = 1;
	int i;

	if (traceInstructionOption) traceSkip();
	while (TRUE) {
		/* Skip past LOOPEND on the same level */
		i = memory[pc];
		if (I_CLASS(i) == (Aword)C_STMOP)
			switch (I_OP(i)) {
			case I_LOOPEND:
				level--;
				if (level == 0)
					return;
				break;
			case I_LOOP:
				level++;
				break;
			default:
				break;
			}
		pc++;
	}
}


/*----------------------------------------------------------------------*/
static void jumpBackToStartOfMatchingLOOP(void) {
	int level = 1;
	int i;

	if (traceInstructionOption) traceSkip();
	pc--;               /* Ignore the instruction we're on */
	while (TRUE) {
		/* Skip back past LOOP on the same level */
		i = memory[--pc];
		if (I_CLASS(i) == (Aword)C_STMOP)
			switch (I_OP(i)) {
			case I_LOOPEND:
				level++;
				break;
			case I_LOOP:
				level--;
				if (level == 0) {
					return;
				}
				break;
			default:
				break;
			}
	}
}


/*----------------------------------------------------------------------*/
static void nextLoop(void) {
	goToLOOPEND();
}


/*----------------------------------------------------------------------*/
static void endLoop(Aint index, Aint limit) {
	if (index < limit) {
		index++;
		push(stack, limit);
		push(stack, index);
		jumpBackToStartOfMatchingLOOP();
		if (traceInstructionOption)
			printf("\n%4x: LOOP\t\t\t\t\t\t", pc);
		pc++;
	}
}


/*----------------------------------------------------------------------*/
static void stackDup(void) {
	push(stack, top(stack));
}


/*----------------------------------------------------------------------*/
static void depexec(Aword v) {
	int lev = 1;
	Aword i;
	const char *instructionString = "DEPELSE";

	if (!v) {
		/* The expression was not true, skip to next CASE on the same
		   level which could be a DEPCASE or DEPELSE */
		if (traceInstructionOption) printf("\n    : ");
		while (TRUE) {
			i = memory[pc++];
			if (I_CLASS(i) == (Aword)C_STMOP)
				switch (I_OP(i)) {
				case I_DEPEND:
					lev++;
					break;
				case I_ENDDEP:
					if (lev == 1) {
						pc--;
						if (traceInstructionOption)
							printf("\n%4x: ENDDEP", pc);
						return;
					} else
						lev--;
					break;
				case I_DEPCASE:
					instructionString = "DEPCASE";
					// fall through

				case I_DEPELSE:
					if (lev == 1) {
						if (traceInstructionOption)
							printf("\n%4x: %s", pc, instructionString);
						return;
					}
					break;
				default:
					break;
				}
		}
	}
}


/*----------------------------------------------------------------------*/
static void depcase(void) {
	int lev = 1;
	Aword i;

	/*
	  We have just executed a DEPCASE/DEPELSE clause as a result of a
	  DEPCASE catching so skip to end of DEPENDING block (next DEPEND
	  on same level) then return.
	*/

	if (traceInstructionOption) printf("\n    : ");
	while (TRUE) {
		i = memory[pc++];
		if (I_CLASS(i) == (Aword)C_STMOP)
			switch (I_OP(i)) {
			case I_DEPEND:
				lev++;
				break;
			case I_ENDDEP:
				lev--;
				if (lev == 0) {
					pc--;
					return;
				}
				break;
			default:
				break;
			}
	}
}


/*----------------------------------------------------------------------*/
static const char *booleanValue(Abool value) {
	if (value) return "   TRUE";
	else return "  FALSE";
}

/*----------------------------------------------------------------------*/
static const char *stringValue(Aptr address) {
	static char string[1000];

	sprintf(string, "0x%lx (\"%s\")\t\t", (unsigned long) address, (char *)fromAptr(address));
	return string;
}

/*----------------------------------------------------------------------*/
static const char *pointerValue(Aptr address) {
	static char string[100];

	sprintf(string, "@%6lx", (unsigned long) address);
	return string;
}

/*----------------------------------------------------------------------*/
static void traceStringTopValue() {
	if (traceInstructionOption)
		printf("\t=%s", stringValue(top(stack)));
}

/*----------------------------------------------------------------------*/
static void tracebooleanTopValue() {
	if (traceInstructionOption) {
		if (top(stack)) printf("\t=TRUE\t");
		else printf("\t=FALSE\t");
	}
}

/*----------------------------------------------------------------------*/
static void traceIntegerTopValue() {
	if (traceInstructionOption)
		printf("\t=%ld\t", (long)top(stack));
}

/*----------------------------------------------------------------------*/
static void tracePointerTopValue() {
	if (traceInstructionOption)
		printf("\t=%s\t", pointerValue(top(stack)));
}

/*----------------------------------------------------------------------*/
static void traceInstanceTopValue(CONTEXT) {
	if (traceInstructionOption) {
		printf("\t=%ld ('", (long)top(stack));
		CALL1(traceSay, top(stack))
		printf("')");
		if (traceStackOption)
			printf("\n\t\t\t\t\t\t\t");
	}
}

/*----------------------------------------------------------------------*/
static const char *transitivityFlag(ATrans value) {
	switch (value) {
	case TRANSITIVE:
		return "Transitive";
	case DIRECT:
		return "Direct";
	case INDIRECT:
		return "Indirect";
	default:
		break;
	}
	syserr("Unexpected transitivity");
	return "ERROR";
}

/*----------------------------------------------------------------------*/
static const char *printForm(SayForm form) {
	switch (form) {
	case SAY_SIMPLE:
		return "-";
	case SAY_INDEFINITE:
		return "An";
	case SAY_DEFINITE:
		return "The";
	case SAY_NEGATIVE:
		return "No";
	case SAY_PRONOUN:
		return "It";
	default:
		break;
	}
	return "**Unknown!!***";
}


static Aaddr invocation[1000];
int recursionDepth = 0;

/*----------------------------------------------------------------------*/
static void checkForRecursion(Aaddr adr) {
	int i;

	for (i = 0; i < recursionDepth; i++)
		if (invocation[i] == adr)
			apperr("Interpreter recursion.");
	invocation[recursionDepth++] = adr;
	if (recursionDepth > 1000)
		syserr("Interpreter call stack too deep.");
}


static bool skipStackDump = FALSE; /* Need to be able to skip it for LINE */


/*----------------------------------------------------------------------*/
static bool stillOnSameLine(Aint line, Aint file) {
	return line != current.sourceLine || file != current.sourceFile;
}


/*======================================================================*/
void interpret(CONTEXT, Aaddr adr) {
	Aaddr oldpc;
	Aword i;

	/* Check for mock implementation */
	if (interpreterMock != NULL) {
		interpreterMock(adr);
		return;
	}

	/* Sanity checks: */
	if (adr == 0) syserr("Interpreting at address 0.");
	checkForRecursion(adr);

	if (traceInstructionOption)
		printf("\n++++++++++++++++++++++++++++++++++++++++++++++++++");

	oldpc = pc;
	pc = adr;
	while (TRUE) {
		if (pc > memTop)
			syserr("Interpreting outside program.");

		i = memory[pc++];

		switch (I_CLASS(i)) {
		case C_CONST:
			if (tracePushOption) printf("\n%4x: PUSH  \t%7ld\t\t\t\t\t", pc - 1, (long)I_OP(i));
			push(stack, I_OP(i));
			if (tracePushOption && traceStackOption)
				dumpStack(stack);
			break;
		case C_CURVAR:
			if (traceInstructionOption) printf("\n%4x: ", pc - 1);
			switch (I_OP(i)) {
			case V_PARAM:
				if (traceInstructionOption) printf("PARAM \t%7ld\t\t\t\t=%ld\t", (long)top(stack),
					                                   (long)globalParameters[top(stack) - 1].instance);
				push(stack, globalParameters[pop(stack) - 1].instance);
				break;
			case V_CURLOC:
				if (traceInstructionOption) printf("CURLOC \t\t\t\t\t=%d\t", current.location);
				push(stack, current.location);
				break;
			case V_CURACT:
				if (traceInstructionOption) printf("CURACT \t\t\t\t\t=%d\t", current.actor);
				push(stack, current.actor);
				break;
			case V_CURVRB:
				if (traceInstructionOption) printf("CURVRB \t\t\t\t\t=%d\t", current.verb);
				push(stack, current.verb);
				break;
			case V_CURRENT_INSTANCE:
				if (traceInstructionOption) printf("CURINS \t\t\t\t\t=%d\t", current.instance);
				push(stack, current.instance);
				break;
			case V_SCORE:
				if (traceInstructionOption) printf("CURSCORE \t\t\t\t\t=%d\t", current.score);
				push(stack, current.score);
				break;
			case V_MAX_INSTANCE: {
				int instanceMax = isPreBeta3(header->version) ? header->instanceMax : header->instanceMax - 1;
				if (traceInstructionOption) printf("MAXINSTANCE \t\t\t\t=%d\t", instanceMax);
				push(stack, instanceMax);
				break;
			}
			default:
				syserr("Unknown CURVAR instruction.");
				break;
			}
			if (traceStackOption)
				dumpStack(stack);
			break;

		case C_STMOP:
			if (traceInstructionOption) printf("\n%4x: ", pc - 1);
			switch (I_OP(i)) {

			case I_DUP:
				if (traceInstructionOption)
					printf("DUP\t\t\t\t\t\t");
				stackDup();
				break;

			case I_DUPSTR:
				if (traceInstructionOption)
					printf("DUPSTR\t\t\t\t\t\t");
				push(stack, toAptr(scumm_strdup((char *)fromAptr(top(stack)))));
				break;

			case I_POP: {
				Aptr top = pop(stack);
				if (traceInstructionOption)
					printf("POP\t%7ld", (long)top);
				break;
			}

			case I_LINE: {
				Aint line = pop(stack);
				Aint file = pop(stack);
				traceInstruction("LINE\t%7ld, %7ld\t\t\t", (long)file, (long)line);
				if (traceStackOption)
					dumpStack(stack);
				skipStackDump = TRUE;
				if (line != 0) {
					bool atNext = stopAtNextLine && line != current.sourceLine;
					bool atBreakpoint =  breakpointIndex(file, line) != -1;
					if (traceSourceOption && stillOnSameLine(line, file)) {
						if (col != 1 || traceInstructionOption)
							printf("\n");
						showSourceLine(file, line);
						if (!traceInstructionOption)
							printf("\n");
					}
					current.sourceLine = line;
					current.sourceFile = file;
					if (atNext || atBreakpoint) {
						stopAtNextLine = FALSE;
						CALL3(debug, TRUE, line, file)
					}
				}
				break;
			}

			case I_PRINT: {
				Aint fpos = pop(stack);
				Aint len = pop(stack);
				if (traceInstructionOption) {
					printf("PRINT \t%7ld, %7ld\t\"", (long)fpos, (long)len);
					col = 41;       /* To break lines better! */
				}
				print(fpos, len);
				if (traceInstructionOption) {
					printf("\"");
					if (traceStackOption)
						printf("\n\t\t\t\t\t\t\t");
				}
				break;
			}

			case I_STYLE: {
				Aint style = pop(stack);
				if (traceInstructionOption) {
					printf("STYLE \t%7ld\t\t\"", (long)style);
				}
				g_io->setStyle(style);
				break;
			}

			case I_SYSTEM: {
				Aint fpos = pop(stack);
				Aint len = pop(stack);
				if (traceInstructionOption) {
					printf("SYSTEM \t%7ld, %7ld\t\"", (long)fpos, (long)len);
					col = 34;       /* To format it better! */
				}
				sys(fpos, len);
				if (traceInstructionOption)
					printf("\"\t\t\t\t\t\t");
				break;
			}

			case I_GETSTR: {
				Aint fpos = pop(stack);
				Aint len = pop(stack);
				if (traceInstructionOption)
					printf("GETSTR\t%7ld, %7ld", (long)fpos, (long)len);
				push(stack, toAptr(getStringFromFile(fpos, len)));
				traceStringTopValue();
				break;
			}

			case I_QUIT: {
				if (traceInstructionOption)
					printf("QUIT\t\t\t\t\t\t");
				CALL0(quitGame)
				break;
			}
			case I_LOOK: {
				if (traceInstructionOption)
					printf("LOOK\t\t\t\t\t\t");
				CALL0(look)
				break;
			}
			case I_SAVE: {
				if (traceInstructionOption)
					printf("SAVE\t\t\t\t\t\t");
				(void)g_vm->saveGame();
				break;
			}
			case I_RESTORE: {
				if (traceInstructionOption)
					printf("RESTORE\t\t\t\t\t\t");
				(void)g_vm->loadGame();
				break;
			}
			case I_RESTART: {
				if (traceInstructionOption)
					printf("RESTART\t\t\t\t\t\t");
				CALL0(restartGame)
				break;
			}

			case I_SCORE: {
				Aint sc = pop(stack);
				if (traceInstructionOption)
					printf("SCORE \t%7ld\t\t=%ld\t\t\t", (long)sc, (long)scores[sc - 1]);
				score(sc);
				break;
			}
			case I_VISITS: {
				Aint v = pop(stack);
				if (traceInstructionOption)
					printf("VISITS \t%7ld\t\t\t\t\t", (long)v);
				visits(v);
				break;
			}

			case I_LIST: {
				Aint cnt = pop(stack);
				if (traceInstructionOption)
					printf("LIST \t%7ld\t\t\t\t\t", (long)cnt);
				CALL1(list, cnt)
				break;
			}
			case I_EMPTY: {
				Aint cnt = pop(stack);
				Aint whr = pop(stack);
				if (traceInstructionOption)
					printf("EMPTY \t%7ld, %7ld\t\t\t\t", (long)cnt, (long)whr);
				CALL2(empty, cnt, whr)
				break;
			}
			case I_SCHEDULE: {
				Aint event = pop(stack);
				Aint where = pop(stack);
				Aint after = pop(stack);
				if (traceInstructionOption)
					printf("SCHEDULE \t%7ld, %7ld, %7ld\t\t\t\t", (long)event, (long)where, (long)after);
				schedule(event, where, after);
				break;
			}
			case I_CANCEL: {
				Aint event = pop(stack);
				if (traceInstructionOption)
					printf("CANCEL \t%7ld\t\t\t\t", (long)event);
				cancelEvent(event);
				break;
			}
			case I_MAKE: {
				Aint atr = pop(stack);
				Aid id = pop(stack);
				Abool val = pop(stack);
				if (traceInstructionOption)
					printf("MAKE \t%7ld, %7ld, %s\t\t\t", (long)id, (long)atr, booleanValue(val));
				setInstanceAttribute(id, atr, val);
				break;
			}
			case I_SET: {
				Aint atr = pop(stack);
				Aid id = pop(stack);
				Aptr val = pop(stack);
				if (traceInstructionOption) {
					printf("SET \t%7ld, %7ld, %7ld\t\t\t\t", (long)id, (long)atr, (long)val);
				}
				setInstanceAttribute(id, atr, val);
				break;
			}
			case I_SETSTR: {
				Aint atr = pop(stack);
				Aid id = pop(stack);
				Aptr str = pop(stack);
				if (traceInstructionOption) {
					printf("SETSTR\t%7ld, %7ld, %s\t\t\t\t", (long)id, (long)atr, stringValue(str));
				}
				setInstanceStringAttribute(id, atr, (char *)fromAptr(str));
				break;
			}
			case I_SETSET: {
				Aint atr = pop(stack);
				Aid id = pop(stack);
				Aptr set = pop(stack);
				if (traceInstructionOption) {
					printf("SETSET\t%7ld, %7ld, %7s\t\t", (long)id, (long)atr, pointerValue(set));
				}
				setInstanceSetAttribute(id, atr, set);
				break;
			}
			case I_NEWSET: {
				Set *set = newSet(0);
				if (traceInstructionOption) {
					printf("NEWSET\t\t\t");
				}
				push(stack, toAptr(set));
				tracePointerTopValue();
				break;
			}
			case I_UNION: {
				Aptr set2 = pop(stack);
				Aptr set1 = pop(stack);
				if (traceInstructionOption) {
					printf("UNION\t%7ld, %7ld\t\t\t\t", (long)set1, (long)set2);
				}
				push(stack, toAptr(setUnion((Set *)fromAptr(set1), (Set *)fromAptr(set2))));
				tracePointerTopValue();
				freeSet((Set *)fromAptr(set1));
				freeSet((Set *)fromAptr(set2));
				break;
			}
			case I_INCR: {
				Aint step = pop(stack);
				if (traceInstructionOption) {
					printf("INCR\t%7ld", (long)step);
				}
				push(stack, pop(stack) + step);
				traceIntegerTopValue();
				break;
			}
			case I_DECR: {
				Aint step = pop(stack);
				if (traceInstructionOption) {
					printf("DECR\t%7ld\t\t\t\t\t", (long)step);
				}
				push(stack, pop(stack) - step);
				traceIntegerTopValue();
				break;
			}
			case I_INCLUDE: {
				Aint member = pop(stack);
				if (traceInstructionOption) {
					printf("INCLUDE\t%7ld\t\t\t\t\t", (long)member);
				}
				addToSet((Set *)fromAptr(top(stack)), member);
				break;
			}
			case I_EXCLUDE: {
				Aint member = pop(stack);
				if (traceInstructionOption) {
					printf("EXCLUDE\t%7ld", (long)member);
				}
				removeFromSet((Set *)fromAptr(top(stack)), member);
				break;
			}
			case I_SETSIZE: {
				Set *set = (Set *)fromAptr(pop(stack));
				if (traceInstructionOption)
					printf("SETSIZE\t%7p\t\t", set);
				push(stack, setSize(set));
				if (traceInstructionOption)
					traceIntegerTopValue();
				break;
			}
			case I_SETMEMB: {
				Set *set = (Set *)fromAptr(pop(stack));
				Aint index = pop(stack);
				if (traceInstructionOption)
					printf("SETMEMB\t%7p, %7ld", set, (long)index);
				push(stack, getSetMember(set, index));
				if (traceInstructionOption)
					traceIntegerTopValue();
				break;
			}
			case I_CONTSIZE: {
				Abool transitivity = pop(stack);
				Aint container = pop(stack);
				if (traceInstructionOption)
					printf("CONTSIZE\t%7ld, %7s\t", (long)container, transitivityFlag((ATrans)transitivity));
				push(stack, containerSize(container, (ATrans)transitivity));
				if (traceInstructionOption)
					traceIntegerTopValue();
				break;
			}
			case I_CONTMEMB: {
				Abool transitivity = pop(stack);
				Aint container = pop(stack);
				Aint index = pop(stack);
				if (traceInstructionOption)
					printf("CONTMEMB\t%7ld, %7ld, %7s", (long)container, (long)index, transitivityFlag((ATrans)transitivity));
				push(stack, getContainerMember(container, index, transitivity));
				if (traceInstructionOption)
					traceIntegerTopValue();
				break;
			}
			case I_ATTRIBUTE: {
				Aint atr = pop(stack);
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("ATTRIBUTE %7ld, %7ld\t", (long)id, (long)atr);
				push(stack, getInstanceAttribute(id, atr));
				traceIntegerTopValue();
				break;
			}
			case I_ATTRSTR: {
				Aint atr = pop(stack);
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("ATTRSTR \t%7ld, %7ld", (long)id, (long)atr);
				push(stack, toAptr(getInstanceStringAttribute(id, atr)));
				traceStringTopValue();
				break;
			}
			case I_ATTRSET: {
				Aint atr = pop(stack);
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("ATTRSET \t%7ld, %7ld", (long)id, (long)atr);
				push(stack, toAptr(getInstanceSetAttribute(id, atr)));
				tracePointerTopValue();
				break;
			}
			case I_SHOW: {
				Aint image = pop(stack);
				Aint align = pop(stack);
				if (traceInstructionOption)
					printf("SHOW \t%7ld, %7ld\t\t\t\t", (long)image, (long)align);
				g_io->showImage(image, align);
				break;
			}
			case I_PLAY: {
				Aint sound = pop(stack);
				if (traceInstructionOption)
					printf("PLAY \t%7ld\t\t\t\t", (long)sound);
				g_io->playSound(sound);
				break;
			}
			case I_LOCATE: {
				Aid id = pop(stack);
				Aint whr = pop(stack);
				if (traceInstructionOption)
					printf("LOCATE \t%7ld, %7ld\t\t\t", (long)id, (long)whr);
				CALL2(locate, id, whr)
				break;
			}
			case I_WHERE: {
				Abool transitivity = pop(stack);
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("WHERE \t%7ld, %7s", (long)id, transitivityFlag((ATrans)transitivity));
				push(stack, where(id, (ATrans)transitivity));
				CALL0(traceInstanceTopValue);
				break;
			}
			case I_LOCATION: {
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("LOCATION \t%7ld\t\t", (long)id);
				push(stack, locationOf(id));
				CALL0(traceInstanceTopValue)
				break;
			}
			case I_HERE: {
				Abool transitivity = pop(stack);
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("HERE \t%7ld, %s\t\t\t", (long)id, transitivityFlag((ATrans)transitivity));
				push(stack, isHere(id, (ATrans)transitivity));
				tracebooleanTopValue();
				break;
			}
			case I_NEARBY: {
				Abool transitivity = pop(stack);
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("NEARBY \t%7ld, %s\t\t\t", (long)id, transitivityFlag((ATrans)transitivity));
				push(stack, isNearby(id, (ATrans)transitivity));
				tracebooleanTopValue();
				break;
			}
			case I_NEAR: {
				Abool transitivity = pop(stack);
				Aint other = pop(stack);
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("NEAR \t%7ld, %7ld, %s\t\t\t", (long)id, (long)other, transitivityFlag((ATrans)transitivity));
				push(stack, isNear(id, other, (ATrans)transitivity));
				tracebooleanTopValue();
				break;
			}
			case I_AT: {
				Abool transitivity = pop(stack);
				Aint other = pop(stack);
				Aint instance = pop(stack);
				if (traceInstructionOption)
					printf("AT \t%7ld, %7ld, %s", (long)instance, (long)other, transitivityFlag((ATrans)transitivity));
				push(stack, isAt(instance, other, (ATrans)transitivity));
				tracebooleanTopValue();
				break;
			}
			case I_IN: {
				Abool transitivity = pop(stack);
				Aint cnt = pop(stack);
				Aint obj = pop(stack);
				if (traceInstructionOption)
					printf("IN \t%7ld, %7ld, %s", (long)obj, (long)cnt, transitivityFlag((ATrans)transitivity));
				push(stack, isIn(obj, cnt, (ATrans)transitivity));
				tracebooleanTopValue();
				break;
			}
			case I_INSET: {
				Aptr set = pop(stack);
				Aword element = pop(stack);
				if (traceInstructionOption)
					printf("INSET \t%7ld, %7ld", (long)element, (long)set);
				push(stack, inSet((Set *)fromAptr(set), element));
				freeSet((Set *)fromAptr(set));
				tracebooleanTopValue();
				break;
			}
			case I_USE: {
				Aid act = pop(stack);
				Aint scr = pop(stack);
				if (traceInstructionOption)
					printf("USE \t%7ld, %7ld\t\t\t\t", (long)act, (long)scr);
				CALL2(use, act, scr)
				break;
			}
			case I_STOP: {
				Aid actor = pop(stack);
				if (traceInstructionOption)
					printf("STOP \t%7ld\t\t\t\t\t", (long)actor);
				stop(actor);
				break;
			}
			case I_DESCRIBE: {
				Aid id = pop(stack);
				if (traceInstructionOption) {
					printf("DESCRIBE \t%7ld\t\t\t", (long)id);
					col = 41;       /* To format it better! */
				}
				CALL1(describe, id)
				if (traceInstructionOption)
					printf("\n\t\t\t\t\t\t");
				break;
			}
			case I_SAY: {
				Aint form = pop(stack);
				Aid id = pop(stack);
				if (traceInstructionOption)
					printf("SAY\t%7s, %7ld\t\t\t", printForm((SayForm)form), (long)id);
				if (form == SAY_SIMPLE) {
					CALL1(say, id)
				} else {
					CALL2(sayForm, id, (SayForm)form)
				}

				if (traceInstructionOption)
					printf("\t\t\t\t\t\t\t");
				break;
			}
			case I_SAYINT: {
				Aword val = pop(stack);
				if (traceInstructionOption)
					printf("SAYINT\t%7ld\t\t\t\"", (long)val);
				sayInteger(val);
				if (traceInstructionOption)
					printf("\"\n\t\t\t\t\t\t\t");
				break;
			}
			case I_SAYSTR: {
				Aptr sayAddr = pop(stack);
				if (traceInstructionOption)
					printf("SAYSTR\t%7ld\t\ty\t", (long)sayAddr);
				sayString((char *)fromAptr(sayAddr));
				if (traceInstructionOption)
					printf("\n\t\t\t\t\t\t");
				break;
			}
			case I_IF: {
				Aword v = pop(stack);
				if (traceInstructionOption)
					printf("IF \t%s\t\t\t\t\t", booleanValue(v));
				interpretIf(v);
				break;
			}
			case I_ELSE: {
				if (traceInstructionOption)
					printf("ELSE\t\t\t\t\t\t");
				interpretElse();
				break;
			}
			case I_ENDIF: {
				if (traceInstructionOption)
					printf("ENDIF\t\t\t\t\t\t");
				break;
			}
			case I_AND: {
				Aword rh = pop(stack);
				Aword lh = pop(stack);
				if (traceInstructionOption)
					printf("AND \t%s, %s", booleanValue(lh), booleanValue(rh));
				push(stack, lh && rh);
				tracebooleanTopValue();
				break;
			}
			case I_OR: {
				Aword rh = pop(stack);
				Aword lh = pop(stack);
				if (traceInstructionOption)
					printf("OR \t%s, %s", booleanValue(lh), booleanValue(rh));
				push(stack, lh || rh);
				tracebooleanTopValue();
				break;
			}
			case I_NE: {
				Aword rh = pop(stack);
				Aword lh = pop(stack);
				if (traceInstructionOption)
					printf("NE \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh != rh);
				tracebooleanTopValue();
				break;
			}
			case I_EQ: {
				Aword rh = pop(stack);
				Aword lh = pop(stack);
				if (traceInstructionOption)
					printf("EQ \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh == rh);
				tracebooleanTopValue();
				break;
			}
			case I_STREQ: {
				Aptr rh = pop(stack);
				Aptr lh = pop(stack);
				if (traceInstructionOption)
					printf("STREQ \t0x%lx, 0x%lx", (long)lh, (long)rh);
				push(stack, streq((char *)fromAptr(lh), (char *)fromAptr(rh)));
				tracebooleanTopValue();
				if (traceInstructionOption)
					printf("\t");
				deallocate(fromAptr(lh));
				deallocate(fromAptr(rh));
				break;
			}
			case I_STREXACT: {
				Aptr rh = pop(stack);
				Aptr lh = pop(stack);
				if (traceInstructionOption)
					printf("STREXACT \t0x%lx, 0x%lx", (long)lh, (long)rh);
				push(stack, strcmp((char *)fromAptr(lh), (char *)fromAptr(rh)) == 0);
				tracebooleanTopValue();
				deallocate(fromAptr(lh));
				deallocate(fromAptr(rh));
				break;
			}
			case I_LE: {
				Aint rh = pop(stack);
				Aint lh = pop(stack);
				if (traceInstructionOption)
					printf("LE \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh <= rh);
				tracebooleanTopValue();
				break;
			}
			case I_GE: {
				Aint rh = pop(stack);
				Aint lh = pop(stack);
				if (traceInstructionOption)
					printf("GE \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh >= rh);
				tracebooleanTopValue();
				break;
			}
			case I_LT: {
				Aint rh = pop(stack);
				Aint lh = pop(stack);
				if (traceInstructionOption)
					printf("LT \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh < rh);
				tracebooleanTopValue();
				break;
			}
			case I_GT: {
				Aint rh = pop(stack);
				Aint lh = pop(stack);
				if (traceInstructionOption)
					printf("GT \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh > rh);
				tracebooleanTopValue();
				break;
			}
			case I_PLUS: {
				Aint rh = pop(stack);
				Aint lh = pop(stack);
				if (traceInstructionOption)
					printf("PLUS \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh + rh);
				traceIntegerTopValue();
				break;
			}
			case I_MINUS: {
				Aint rh = pop(stack);
				Aint lh = pop(stack);
				if (traceInstructionOption)
					printf("MINUS \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh - rh);
				traceIntegerTopValue();
				break;
			}
			case I_MULT: {
				Aint rh = pop(stack);
				Aint lh = pop(stack);
				if (traceInstructionOption)
					printf("MULT \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh * rh);
				traceIntegerTopValue();
				break;
			}
			case I_DIV: {
				Aint rh = pop(stack);
				Aint lh = pop(stack);
				if (traceInstructionOption)
					printf("DIV \t%7ld, %7ld", (long)lh, (long)rh);
				push(stack, lh / rh);
				traceIntegerTopValue();
				break;
			}
			case I_NOT: {
				Aword val = pop(stack);
				if (traceInstructionOption)
					printf("NOT \t%s\t\t\t", booleanValue(val));
				push(stack, !val);
				tracebooleanTopValue();
				break;
			}
			case I_RND: {
				Aint from = pop(stack);
				Aint to = pop(stack);
				if (traceInstructionOption)
					printf("RANDOM \t%7ld, %7ld", (long)from, (long)to);
				push(stack, randomInteger(from, to));
				traceIntegerTopValue();
				break;
			}
			case I_BTW: {
				Aint high = pop(stack);
				Aint low = pop(stack);
				Aint val = pop(stack);
				if (traceInstructionOption)
					printf("BETWEEN \t%7ld, %7ld, %7ld", (long)val, (long)low, (long)high);
				push(stack, between(val, low, high));
				traceIntegerTopValue();
				break;
			}

			/*------------------------------------------------------------* \
			  String functions
			\*------------------------------------------------------------*/
			case I_CONCAT: {
				Aptr s2 = pop(stack);
				Aptr s1 = pop(stack);
				if (traceInstructionOption)
					printf("CONCAT \t%s, %s", pointerValue(s1), pointerValue(s2));
				push(stack, concat(s1, s2));
				traceStringTopValue();
				deallocate(fromAptr(s1));
				deallocate(fromAptr(s2));
				break;
			}

			case I_CONTAINS: {
				Aptr substring = pop(stack);
				Aptr string = pop(stack);
				if (traceInstructionOption)
					printf("CONTAINS \t%s, %s", pointerValue(string), pointerValue(substring));
				push(stack, contains(string, substring));
				traceIntegerTopValue();
				deallocate(fromAptr(string));
				deallocate(fromAptr(substring));
				break;
			}

			case I_STRIP: {
				Aint atr = pop(stack);
				Aid id = pop(stack);
				Aint words = pop(stack);
				Aint count = pop(stack);
				Aint first = pop(stack);
				if (traceInstructionOption)
					printf("STRIP \t%7ld, %7ld, %7ld, %7ld, %7ld", (long)first, (long)count, (long)words, (long)id, (long)atr);
				push(stack, strip(first, count, words, id, atr));
				traceStringTopValue();
				break;
			}

			/*------------------------------------------------------------
			  Aggregation
			  ------------------------------------------------------------*/
			case I_MIN:
			case I_SUM:
			case I_MAX: {
				Aint attribute = pop(stack);
				Aint loopIndex = pop(stack);
				Aint limit = pop(stack);
				Aint aggregate = pop(stack);
				switch (I_OP(i)) {
				case I_MAX:
					if (traceInstructionOption)
						printf("MAX \t%7ld\t\t\t", (long)attribute);
					if (aggregate < attribute)
						push(stack, attribute);
					else
						push(stack, aggregate);
					break;
				case I_MIN:
					if (traceInstructionOption)
						printf("MIN \t%7ld\t\t\t", (long)attribute);
					if (aggregate > attribute)
						push(stack, attribute);
					else
						push(stack, aggregate);
					break;
				case I_SUM:
					if (traceInstructionOption)
						printf("SUM \t%7ld\t\t\t", (long)attribute);
					push(stack, aggregate + attribute);
					break;
				default:
					break;
				}
				traceIntegerTopValue();
				push(stack, limit);
				push(stack, loopIndex);
				break;
			}
			case I_COUNT: {
				Aint loopIndex = pop(stack);
				Aint limit = pop(stack);
				if (traceInstructionOption)
					printf("COUNT\t\t\t");
				push(stack, pop(stack) + 1);
				traceIntegerTopValue();
				push(stack, limit);
				push(stack, loopIndex);
				break;
			}
			case I_TRANSCRIPT: {
				Aint on_or_off = pop(stack);
				if (traceInstructionOption)
					printf("TRANSCRIPT\t\t\t");
				if (on_or_off)
					startTranscript();
				else
					stopTranscript();
				break;
			}

			/*------------------------------------------------------------
			  Depending On
			  ------------------------------------------------------------*/
			case I_DEPEND:
				if (traceInstructionOption)
					printf("DEPEND\t\t\t\t\t\t");
				break;

			case I_DEPCASE:
				if (traceInstructionOption)
					printf("DEPCASE\t\t\t\t\t\t");
				depcase();
				break;

			case I_DEPEXEC: {
				Aword v = pop(stack);
				if (traceInstructionOption) {
					printf("DEPEXEC \t\t\t");
					if (v) printf(" TRUE");
					else printf("FALSE");
					printf("\t\t\t\t\t");
				}
				depexec(v);
				break;
			}

			case I_DEPELSE:
				if (traceInstructionOption)
					printf("DEPELSE\t\t\t\t\t\t");
				depcase();
				break;

			case I_ENDDEP:
				if (traceInstructionOption)
					printf("ENDDEP\t\t\t\t\t\t");
				pop(stack);
				break;

			case I_ISA: {
				Aid rh = pop(stack);
				Aid lh = pop(stack);
				if (traceInstructionOption)
					printf("ISA \t%7ld, %7ld\t", (long)lh, (long)rh);
				push(stack, isA(lh, rh));
				tracebooleanTopValue();
				break;
			}

			case I_FRAME: {
				Aint size = pop(stack);
				if (traceInstructionOption)
					printf("FRAME \t%7ld\t\t\t\t\t", (long)size);
				newFrame(stack, size);
				break;
			}

			case I_GETLOCAL: {
				Aint framesBelow = pop(stack);
				Aint variableNumber = pop(stack);
				if (traceInstructionOption)
					printf("GETLOCAL \t%7ld, %7ld\t", (long)framesBelow, (long)variableNumber);
				push(stack, getLocal(stack, framesBelow, variableNumber));
				traceIntegerTopValue();
				break;
			}

			case I_SETLOCAL: {
				Aint framesBelow = pop(stack);
				Aint variableNumber = pop(stack);
				Aint value = pop(stack);
				if (traceInstructionOption)
					printf("SETLOCAL \t%7ld, %7ld, %7ld\t\t", (long)framesBelow, (long)variableNumber, (long)value);
				setLocal(stack, framesBelow, variableNumber, value);
				break;
			}

			case I_ENDFRAME: {
				if (traceInstructionOption)
					printf("ENDFRAME\t\t\t\t\t\t");
				endFrame(stack);
				break;
			}

			case I_LOOP: {
				Aint index = pop(stack);
				Aint limit = pop(stack);
				if (traceInstructionOption)
					printf("LOOP \t\t\t\t\t\t");
				push(stack, limit);
				push(stack, index);
				if (index > limit)
					goToLOOPEND();
				break;
			}

			case I_LOOPNEXT: {
				if (traceInstructionOption)
					printf("LOOPNEXT\t\t\t\t\t\t");
				nextLoop();
				break;
			}

			case I_LOOPEND: {
				Aint index = pop(stack);
				Aint limit = pop(stack);
				if (traceInstructionOption)
					printf("LOOPEND\t\t\t\t\t\t");
				endLoop(index, limit);
				break;
			}

			case I_RETURN:
				if (traceInstructionOption)
					printf("RETURN\n--------------------------------------------------\n");
				pc = oldpc;
				goto exitInterpreter;

			default:
				syserr("Unknown STMOP instruction.");
				break;
			}
			if (fail) {
				pc = oldpc;
				goto exitInterpreter;
			}
			if (traceStackOption) {
				if (!skipStackDump)
					dumpStack(stack);
				skipStackDump = FALSE;
			}
			break;

		default:
			syserr("Unknown instruction class.");
			break;
		}
	}
exitInterpreter:
	recursionDepth--;

}

/*======================================================================*/
Aword evaluate(CONTEXT, Aaddr adr) {
	R0CALL1(interpret, adr)
	return pop(stack);
}

} // End of namespace Alan3
} // End of namespace Glk
