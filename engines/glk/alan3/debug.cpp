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

#include "glk/alan3/debug.h"
#include "glk/alan3/alan3.h"
#include "glk/alan3/class.h"
#include "glk/alan3/sysdep.h"
#include "glk/alan3/alan_version.h"
#include "glk/alan3/compatibility.h"
#include "glk/alan3/current.h"
#include "glk/alan3/event.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/options.h"
#include "glk/alan3/output.h"
#include "glk/alan3/sysdep.h"
#include "glk/alan3/utils.h"
#include "glk/streams.h"

namespace Glk {
namespace Alan3 {

#define BREAKPOINTMAX 50


/* PUBLIC: */
int breakpointCount = 0;
Breakpoint breakpoint[BREAKPOINTMAX];

#define debugPrefix "adbg: "

/*----------------------------------------------------------------------*/
static void showAttributes(AttributeEntry *attrib) {
	AttributeEntry *at;
	int i;
	char str[80];

	if (attrib == 0)
		return;

	i = 1;
	for (at = attrib; !isEndOfArray(at); at++) {
		sprintf(str, "$i$t%s[%d] = %d", (char *) pointerTo(at->id), at->code, (int)at->value);

		output(str);
		i++;
	}
}


/*----------------------------------------------------------------------*/
static void showContents(CONTEXT, int cnt) {
	uint i;
	char str[80];
	Abool found = FALSE;

	output("$iContains:");
	for (i = 1; i <= header->instanceMax; i++) {
		if (isIn(i, cnt, DIRECT)) { /* Yes, it's directly in this container */
			if (!found)
				found = TRUE;
			output("$i$t");
			say(context, i);
			sprintf(str, "[%d] ", i);
			output(str);
		}
	}
	if (!found)
		output("nothing");
}


/*----------------------------------------------------------------------*/
static char *idOfInstance(CONTEXT, int instance) {
	int base = header->instanceTableAddress +
	           header->instanceMax * sizeof(InstanceEntry) / sizeof(Aword) + 1;
	return (char *)&memory[memory[base + instance - 1]];
}


/*----------------------------------------------------------------------*/
static void sayInstanceNumberAndName(CONTEXT, int ins) {
	char buf[1000];

	sprintf(buf, "[%d] %s (\"$$", ins, idOfInstance(context, ins));
	output(buf);
	say(context, ins);
	output("$$\")");
}


/*----------------------------------------------------------------------*/
static void sayLocationOfInstance(CONTEXT, int ins, const char *prefix) {
	if (admin[ins].location == 0)
		return;
	else {
		output(prefix);
		if (isALocation(admin[ins].location)) {
			output("at");
			CALL1(sayInstanceNumberAndName, admin[ins].location)
			CALL2(sayLocationOfInstance, admin[ins].location, prefix)
		} else if (isAContainer(admin[ins].location)) {
			if (isAObject(admin[ins].location))
				output("in");
			else if (isAActor(admin[ins].location))
				output("carried by");
			CALL1(sayInstanceNumberAndName, admin[ins].location)
			CALL2(sayLocationOfInstance, admin[ins].location, prefix)
		} else {
			output("Illegal location!");
		}
	}
}

/*----------------------------------------------------------------------*/
static void listInstance(CONTEXT, int ins) {
	output("$i");
	CALL1(sayInstanceNumberAndName, ins)
	if (instances[ins].container)
		output("(container)");
	CALL2(sayLocationOfInstance, ins, ", ")
}


/*----------------------------------------------------------------------*/
static void listInstances(CONTEXT, char *pattern) {
	uint ins;
	bool found = FALSE;

	for (ins = 1; ins <= header->instanceMax; ins++) {
		if (pattern == NULL || (pattern != NULL && match(pattern, idOfInstance(context, ins)))) {
			if (!found) {
				output("Instances:");
				found = TRUE;
			}
			CALL1(listInstance, ins)
		}
	}
	if (pattern != NULL && !found)
		output("No instances matched the pattern.");
}

/*----------------------------------------------------------------------*/
static void showInstance(CONTEXT, int ins) {
	char str[80];

	if (ins > (int)header->instanceMax || ins < 1) {
		sprintf(str, "Instance index %d is out of range.", ins);
		output(str);
		return;
	}

	output("The");
	CALL1(sayInstanceNumberAndName, ins)
	if (instances[ins].parent) {
		sprintf(str, "Isa %s[%d]", idOfClass(instances[ins].parent), instances[ins].parent);
		output(str);
	}

	if (!isA(ins, header->locationClassId) || (isA(ins, header->locationClassId) && admin[ins].location != 0)) {
		sprintf(str, "$iLocation:");
		output(str);
		needSpace = TRUE;
		CALL2(sayLocationOfInstance, ins, "")
	}

	output("$iAttributes:");
	showAttributes(admin[ins].attributes);

	if (instances[ins].container)
		CALL1(showContents, ins)

	if (isA(ins, header->actorClassId)) {
		if (admin[ins].script == 0)
			output("$iIs idle");
		else {
			sprintf(str, "$iExecuting script: %d, Step: %d", admin[ins].script, admin[ins].step);
			output(str);
		}
	}
}


/*----------------------------------------------------------------------*/
static void listObjects(CONTEXT) {
	uint obj;

	output("Objects:");
	for (obj = 1; obj <= header->instanceMax; obj++)
		if (isAObject(obj))
			CALL1(listInstance, obj)
}


/*----------------------------------------------------------------------*/
static void showObject(CONTEXT, int obj) {
	char str[80];


	if (!isAObject(obj)) {
		sprintf(str, "Instance %d is not an object", obj);
		output(str);
		return;
	}

	CALL1(showInstance, obj)
}

/*----------------------------------------------------------------------*/
static int sourceFileNumber(char *fileName) {
	SourceFileEntry *entries = (SourceFileEntry *)pointerTo(header->sourceFileTable);
	int n;

	for (n = 0; * (Aword *)&entries[n] != EOD; n++) {
		char *entryName;
		entryName = getStringFromFile(entries[n].fpos, entries[n].len);
		if (strcmp(entryName, fileName) == 0) return n;
		entryName = baseNameStart(entryName);
		if (strcmp(entryName, fileName) == 0) return n;
	}
	return -1;
}



/*----------------------------------------------------------------------*/
static void printClassName(int c) {
	output(idOfClass(c));
}


/*----------------------------------------------------------------------*/
static void showClassInheritance(int c) {
	char str[80];

	if (classes[c].parent != 0) {
		output(", Isa");
		printClassName(classes[c].parent);
		sprintf(str, "[%d]", classes[c].parent);
		output(str);
	}
}


/*----------------------------------------------------------------------*/
static void showClass(int cla) {
	char str[80];

	if (cla < 1) {
		sprintf(str, "Class index %d is out of range.", cla);
		output(str);
		return;
	}

	output("$t");
	printClassName(cla);
	sprintf(str, "[%d]", cla);
	output(str);
	showClassInheritance(cla);
}


/*----------------------------------------------------------------------*/
static void listClass(int c) {
	char str[80];

	sprintf(str, "%3d: ", c);
	output(str);
	printClassName(c);
	showClassInheritance(c);
}


/*----------------------------------------------------------------------*/
static void showClassHierarchy(int thisItem, int depth) {
	int i;
	uint child;

	output("$i");
	for (i = 0; i < depth; i++)
		output("$t");

	listClass(thisItem);
	for (child = 1; child <= header->classMax; child++) {
		if (classes[child].parent == thisItem) {
			showClassHierarchy(child, depth + 1);
		}
	}
}


/*----------------------------------------------------------------------*/
static void listLocations(CONTEXT) {
	uint loc;

	output("Locations:");
	for (loc = 1; loc <= header->instanceMax; loc++)
		if (isALocation(loc))
			listInstance(context, loc);
}


/*----------------------------------------------------------------------*/
static void showLocation(CONTEXT, int loc) {
	char str[80];


	if (!isALocation(loc)) {
		sprintf(str, "Instance %d is not a location.", loc);
		output(str);
		return;
	}

	output("The ");
	CALL1(say, loc)
	sprintf(str, "(%d) Isa location :", loc);
	output(str);

	output("$iAttributes =");
	showAttributes(admin[loc].attributes);
}


/*----------------------------------------------------------------------*/
static void listActors(CONTEXT) {
	uint act;

	output("Actors:");
	for (act = 1; act <= header->instanceMax; act++)
		if (isAActor(act))
			CALL1(listInstance, act)
}


/*----------------------------------------------------------------------*/
static void showActor(CONTEXT, int act) {
	char str[80];

	if (!isAActor(act)) {
		sprintf(str, "Instance %d is not an actor.", act);
		output(str);
		return;
	}

	CALL1(showInstance, act)
}


/*----------------------------------------------------------------------*/
static void showEvents(CONTEXT) {
	uint event;
	int i;
	char str[80];
	bool scheduled;

	output("Events:");
	for (event = 1; event <= header->eventMax; event++) {
		sprintf(str, "$i%d [%s]:", event, (char *)pointerTo(events[event].id));

		output(str);
		scheduled = FALSE;
		for (i = 0; i < eventQueueTop; i++)
			if ((scheduled = (eventQueue[i].event == (int)event)))
				break;
		if (scheduled) {
			sprintf(str, "Scheduled for +%d, at ", eventQueue[i].after);
			output(str);
			CALL1(say, eventQueue[i].where)
		} else
			output("Not scheduled.");
	}
}


/*======================================================================*/
char *sourceFileName(int fileNumber) {
	SourceFileEntry *entries = (SourceFileEntry *)pointerTo(header->sourceFileTable);

	return getStringFromFile(entries[fileNumber].fpos, entries[fileNumber].len);
}


/*======================================================================*/
bool readLine(Common::SeekableReadStream *rs, char *line, int maxLen) {
	if (rs->pos() < rs->size()) {
		line[maxLen - 1] = '\0';

		char c;
		do {
			c = rs->readByte();
			*line++ = c;
		} while (--maxLen > 1);
	}

	return rs->pos() < rs->size();
}


/*======================================================================*/
char *readSourceLine(int file, int line) {
	int count;
#define SOURCELINELENGTH 1000
	static char buffer[SOURCELINELENGTH];

	frefid_t sourceFileRef = g_vm->glk_fileref_create_by_name(fileusage_TextMode, sourceFileName(file), 0);
	strid_t sourceFile = g_vm->glk_stream_open_file(sourceFileRef, filemode_Read, 0);

	if (sourceFile != NULL) {
		for (count = 0; count < line; count++) {
			if (!readLine(*sourceFile, buffer, SOURCELINELENGTH))
				return NULL;

			// If not read the whole line, or no newline, try to read again
			while (strchr(buffer, '\n') == NULL) {
				if (!readLine(*sourceFile, buffer, SOURCELINELENGTH))
					break;
			}
		}

		delete sourceFile;
		return buffer;
	}

	return NULL;
}

/*======================================================================*/
void showSourceLine(int fileNumber, int line) {
	char *buffer = readSourceLine(fileNumber, line);
	if (buffer != NULL) {
		if (buffer[strlen(buffer) - 1] == '\n')
			buffer[strlen(buffer) - 1] = '\0';
		printf("<%05d>: %s", line, buffer);
	}
}


/*----------------------------------------------------------------------*/
static void listFiles() {
	SourceFileEntry *entry;
	int i = 0;
	for (entry = (SourceFileEntry *)pointerTo(header->sourceFileTable); * ((Aword *)entry) != EOD; entry++) {
		printf("  %2d : %s\n", i, sourceFileName(i));
		i++;
	}
}


/*----------------------------------------------------------------------*/
static int findSourceLineIndex(SourceLineEntry *entry, int file, int line) {
	/* Will return index to the closest line available */
	int i = 0;

	while (!isEndOfArray(&entry[i]) && entry[i].file != file)
		i++;
	while (!isEndOfArray(&entry[i]) && entry[i].file == file  && entry[i].line < line)
		i++;
	if (isEndOfArray(entry) || entry[i].file != file)
		return i - 1;
	else
		return i;
}


/*----------------------------------------------------------------------*/
static void listBreakpoints() {
	int i;
	bool found = FALSE;

	for (i = 0; i < BREAKPOINTMAX; i++)
		if (breakpoint[i].line != 0) {
			if (!found)
				printf("Breakpoints set:\n");
			found = TRUE;
			printf("    %s:%d\n", sourceFileName(breakpoint[i].file), breakpoint[i].line);
		}
	if (!found)
		printf("No breakpoints set\n");
}


/*======================================================================*/
int breakpointIndex(int file, int line) {
	int i;

	for (i = 0; i < BREAKPOINTMAX; i++)
		if (breakpoint[i].line == line && breakpoint[i].file == file)
			return i;
	return -1;
}


/*----------------------------------------------------------------------*/
static int availableBreakpointSlot() {
	int i;

	for (i = 0; i < BREAKPOINTMAX; i++)
		if (breakpoint[i].line == 0)
			return i;
	return -1;
}


/*----------------------------------------------------------------------*/
static void setBreakpoint(int file, int line) {
	int i = breakpointIndex(file, line);

	if (i != -1)
		printf("Breakpoint already set at %s:%d\n", sourceFileName(file), line);
	else {
		i = availableBreakpointSlot();
		if (i == -1)
			printf("No room for more breakpoints. Delete one first.\n");
		else {
			int lineIndex = findSourceLineIndex((SourceLineEntry *)pointerTo(header->sourceLineTable), file, line);
			SourceLineEntry *entry = (SourceLineEntry *)pointerTo(header->sourceLineTable);
			char leadingText[100] = "Breakpoint";
			if (entry[lineIndex].file == (Aint)EOD) {
				printf("Line %d not available\n", line);
			} else {
				if (entry[lineIndex].line != line)
					sprintf(leadingText, "Line %d not available, breakpoint instead", line);
				breakpoint[i].file = entry[lineIndex].file;
				breakpoint[i].line = entry[lineIndex].line;
				printf("%s set at %s:%d\n", leadingText, sourceFileName(entry[lineIndex].file), entry[lineIndex].line);
				showSourceLine(entry[lineIndex].file, entry[lineIndex].line);
				printf("\n");
			}
		}
	}
}


/*----------------------------------------------------------------------*/
static void deleteBreakpoint(int line, int file) {
	int i = breakpointIndex(file, line);

	if (i == -1)
		printf("No breakpoint set at %s:%d\n", sourceFileName(file), line);
	else {
		breakpoint[i].line = 0;
		printf("Breakpoint at %s:%d deleted\n", sourceFileName(file), line);
	}
}



static bool saved_traceSection, saved_traceInstruction, saved_capitilize, saved_tracePush, saved_traceStack, saved_traceSource;
static int loc;

/*======================================================================*/
void saveInfo(void) {
	/* Save some important things */
	saved_capitilize = capitalize;
	capitalize = FALSE;
	saved_traceSection = traceSectionOption;
	traceSectionOption = FALSE;
	saved_traceSource = traceSourceOption;
	traceSourceOption = FALSE;
	saved_traceInstruction = traceInstructionOption;
	traceInstructionOption = FALSE;
	saved_tracePush = tracePushOption;
	tracePushOption = FALSE;
	saved_traceStack = traceStackOption;
	traceStackOption = FALSE;
	loc = current.location;
	current.location = where(HERO, DIRECT);
}


/*======================================================================*/
void restoreInfo(void) {
	/* Restore! */
	capitalize = saved_capitilize;
	traceSectionOption = saved_traceSection;
	traceInstructionOption = saved_traceInstruction;
	traceSourceOption = saved_traceSource;
	tracePushOption = saved_tracePush;
	traceStackOption = saved_traceStack;
	current.location = loc;
}

#define HELP_COMMAND 'H'
#define QUIT_COMMAND 'Q'
#define EXIT_COMMAND 'X'
#define GO_COMMAND 'G'
#define FILES_COMMAND 'F'
#define INSTANCES_COMMAND 'I'
#define CLASSES_COMMAND 'C'
#define OBJECTS_COMMAND 'O'
#define ACTORS_COMMAND 'A'
#define LOCATIONS_COMMAND 'L'
#define EVENTS_COMMAND 'E'
#define BREAK_COMMAND 'B'
#define DELETE_COMMAND 'D'
#define TRACE_COMMAND 'R'
#define SECTION_TRACE_COMMAND 'T'
#define INSTRUCTION_TRACE_COMMAND 'S'
#define NEXT_COMMAND 'N'
#define UNKNOWN_COMMAND '?'
#define AMBIGUOUS_COMMAND '-'
#define TRACE_SOURCE_COMMAND 's'
#define TRACE_SECTION_COMMAND 'e'
#define TRACE_INSTRUCTION_COMMAND 'i'
#define TRACE_PUSH_COMMAND 'p'
#define TRACE_STACK_COMMAND 't'

typedef struct DebugParseEntry {
	const char *command;
	const char *parameter;
	char code;
	const char *helpText;
} DebugParseEntry;

static const DebugParseEntry commandEntries[] = {
	{"help", "", HELP_COMMAND, "this help"},
	{"?", "", HELP_COMMAND, "d:o"},
	{"break", "[[file:]n]", BREAK_COMMAND, "set breakpoint at source line [n] (optionally in [file])"},
	{"delete", "[[file:]n]", DELETE_COMMAND, "delete breakpoint at source line [n] (optionally in [file])"},
	{"files", "", FILES_COMMAND, "list source files"},
	{"events", "", EVENTS_COMMAND, "list events"},
	{"classes", "", CLASSES_COMMAND, "list class hierarchy"},
	{"instances", "[n]", INSTANCES_COMMAND, "list instance(s), all, wildcard, number or name"},
	{"objects", "[n]", OBJECTS_COMMAND, "list instance(s) that are objects"},
	{"actors", "[n]", ACTORS_COMMAND, "list instance(s) that are actors"},
	{"locations", "[n]", LOCATIONS_COMMAND, "list instances that are locations"},
	{"trace", "('source'|'section'|'instruction'|'push'|'stack')", TRACE_COMMAND, "toggle various traces"},
	{"next", "", NEXT_COMMAND, "run game and stop at next source line"},
	{"go", "", GO_COMMAND, "go another player turn"},
	{"exit", "", EXIT_COMMAND, "exit to game, enter 'debug' to get back"},
	{"x", "", EXIT_COMMAND, "d:o"},
	{"quit", "", QUIT_COMMAND, "quit game"},
	{NULL, NULL, '\0', NULL}
};

static const DebugParseEntry traceSubcommand[] = {
	{"source", "", TRACE_SOURCE_COMMAND, ""},
	{"section", "", TRACE_SECTION_COMMAND, ""},
	{"instructions", "", TRACE_INSTRUCTION_COMMAND, ""},
	{"pushs", "", TRACE_PUSH_COMMAND, ""},
	{"stacks", "", TRACE_STACK_COMMAND, ""},
	{NULL, NULL, '\0', NULL}
};


static char *spaces(int length) {
	static char buf[200];
	int i;

	for (i = 0; i < length; i++)
		buf[i] = ' ';
	buf[i] = '\0';
	return buf;
}


/*----------------------------------------------------------------------*/
static char *padding(const DebugParseEntry *entry, int maxLength) {
	return spaces(maxLength - strlen(entry->command) - strlen(entry->parameter));
}


/*----------------------------------------------------------------------*/
static void handleHelpCommand() {
	if (!regressionTestOption)
		output(alan.longHeader);

	const DebugParseEntry *entry = commandEntries;

	int maxLength = 0;
	for (entry = commandEntries; entry->command != NULL; entry++) {
		if (strlen(entry->command) + strlen(entry->parameter) > (uint)maxLength)
			maxLength = strlen(entry->command) + strlen(entry->parameter);
	}

	output("$nADBG Commands (can be abbreviated):");
	for (entry = commandEntries; entry->command != NULL; entry++) {
		char buf[200];
		sprintf(buf, "$i%s %s %s$n$t$t-- %s", entry->command, entry->parameter, padding(entry, maxLength), entry->helpText);
		output(buf);
	}
}


/*----------------------------------------------------------------------*/
static const DebugParseEntry *findEntry(char *command, const DebugParseEntry *entry) {
	while (entry->command != NULL) {
		if (scumm_strnicmp(command, entry->command, strlen(command)) == 0)
			return entry;
		entry++;
	}
	return NULL;
}


/*----------------------------------------------------------------------*/
static char parseDebugCommand(char *command) {
	const DebugParseEntry *entry = findEntry(command, commandEntries);
	if (entry != NULL) {
		if (strlen(command) < strlen(entry->command)) {
			/* See if there are any more partial matches */
			if (findEntry(command, entry + 1) != NULL)
				/* TODO: we should list the possible matches somehow */
				return AMBIGUOUS_COMMAND;
		}
		return entry->code;
	} else
		return UNKNOWN_COMMAND;
}


/*----------------------------------------------------------------------*/
static void readCommand(CONTEXT, char buf[], size_t maxLen) {
	char c;
	bool flag;

	capitalize = FALSE;
	if (anyOutput) newline();
	do {
		output("adbg> ");

		FUNC2(g_io->readLine, flag, buf, maxLen)
		if (!flag) {
			newline();
			CALL0(quitGame)
		}
		lin = 1;
		c = buf[0];
	} while (c == '\0');
}


/*----------------------------------------------------------------------*/
static void displaySourceLocation(int line, int fileNumber) {
	const char *cause;
	if (anyOutput) newline();
	if (breakpointIndex(fileNumber, line) != -1)
		cause = "Breakpoint hit at";
	else
		cause = "Stepping to";
	printf("%s %s %s:%d\n", debugPrefix, cause, sourceFileName(fileNumber), line);
	showSourceLine(fileNumber, line);
	printf("\n");
	anyOutput = FALSE;
}


/*----------------------------------------------------------------------*/
static void toggleSectionTrace() {
	if ((saved_traceSection = !saved_traceSection))
		printf("Section trace on.");
	else
		printf("Section trace off.");
}

/*----------------------------------------------------------------------*/
static void toggleInstructionTrace() {
	if ((saved_traceInstruction = !saved_traceInstruction))
		printf("Single instruction trace on.");
	else
		printf("Single instruction trace off.");
}

/*----------------------------------------------------------------------*/
static void toggleSourceTrace() {
	if ((saved_traceSource = !saved_traceSource))
		printf("Source code trace on.");
	else
		printf("Source code trace off.");
}


/*----------------------------------------------------------------------*/
static void togglePushTrace() {
	if ((saved_tracePush = !saved_tracePush))
		printf("Stack Push trace on.");
	else
		printf("Stack Push trace off.");
}


/*----------------------------------------------------------------------*/
static void toggleStackTrace() {
	if ((saved_traceStack = !saved_traceStack))
		printf("Full stack trace on.");
	else
		printf("Full stack trace off.");
}


/*----------------------------------------------------------------------*/
static int parseTraceCommand() {
	char *subcommand = strtok(NULL, "");
	const DebugParseEntry *entry;
	if (subcommand == 0)
		return UNKNOWN_COMMAND;
	else {
		entry = findEntry(subcommand, traceSubcommand);
		if (entry != NULL) {
			if (strlen(subcommand) < strlen(entry->command)) {
				if (findEntry(subcommand, entry + 1) != NULL)
					return AMBIGUOUS_COMMAND;
			}
			return entry->code;
		} else
			return UNKNOWN_COMMAND;
	}
}


/*----------------------------------------------------------------------*/
static const char *printTraceState(bool state) {
	if (state)
		return "on  - Traces";
	else
		return "off - Doesn't trace";
}

/*----------------------------------------------------------------------*/
static void printTrace(void) {
	printf("Trace section     : %s entry to every section (check, description, event, actor, ...)\n", printTraceState(saved_traceSection));
	printf("Trace source      : %s every source line executed\n", printTraceState(saved_traceSource));
	printf("Trace instruction : %s every Amachine instruction executed\n", printTraceState(saved_traceInstruction));
	printf("Trace push        : %s every push onto the Amachine stack\n", printTraceState(saved_tracePush));
	printf("Trace stack       : %s the complete stack every time\n", printTraceState(saved_traceStack));
}


/*----------------------------------------------------------------------*/
static void handleTraceCommand() {
	char subcommand = parseTraceCommand();

	switch (subcommand) {
	case TRACE_SECTION_COMMAND:
		toggleSectionTrace();
		break;
	case TRACE_SOURCE_COMMAND:
		toggleSourceTrace();
		break;
	case TRACE_INSTRUCTION_COMMAND:
		toggleInstructionTrace();
		break;
	case TRACE_PUSH_COMMAND:
		togglePushTrace();
		break;
	case TRACE_STACK_COMMAND:
		toggleStackTrace();
		break;
	case AMBIGUOUS_COMMAND:
		output("Ambiguous Trace subcommand abbreviation. ? for help.");
		break;
	default:
		printTrace();
	}
}


/*----------------------------------------------------------------------*/
static void handleBreakCommand(int fileNumber) {
	char *parameter = strtok(NULL, ":");
	if (parameter != NULL && Common::isAlpha((int)parameter[0])) {
		fileNumber = sourceFileNumber(parameter);
		if (fileNumber == -1) {
			printf("No such file: '%s'\n", parameter);
			return;
		}
		parameter = strtok(NULL, "");
	}
	if (parameter == NULL)
		listBreakpoints();
	else
		setBreakpoint(fileNumber, atoi(parameter));
}


/*----------------------------------------------------------------------*/
static void handleDeleteCommand(bool calledFromBreakpoint, int line, int fileNumber) {
	char *parameter = strtok(NULL, "");
	if (parameter == NULL) {
		if (calledFromBreakpoint)
			deleteBreakpoint(line, fileNumber);
		else
			printf("No current breakpoint to delete\n");
	} else
		deleteBreakpoint(atoi(parameter), fileNumber);
}


/*----------------------------------------------------------------------*/
static void handleNextCommand(bool calledFromBreakpoint) {
	stopAtNextLine = TRUE;
	debugOption = FALSE;
	if (!calledFromBreakpoint)
		current.sourceLine = 0;
	restoreInfo();
}


/*----------------------------------------------------------------------*/
static void handleLocationsCommand(CONTEXT) {
	char *parameter = strtok(NULL, "");
	if (parameter == 0)
		listLocations(context);
	else
		showLocation(context, atoi(parameter));
}


/*----------------------------------------------------------------------*/
static void handleActorsCommand(CONTEXT) {
	char *parameter = strtok(NULL, "");
	if (parameter == NULL)
		listActors(context);
	else
		showActor(context, atoi(parameter));
}


/*----------------------------------------------------------------------*/
static void handleClassesCommand(CONTEXT) {
	char *parameter = strtok(NULL, "");
	if (parameter == NULL || strchr(parameter, '*') != 0) {
		output("Classes:");
		showClassHierarchy(1, 0);
		listInstances(context, parameter);
	} else if (Common::isDigit((int)parameter[0]))
		showClass(atoi(parameter));
	else {
		printf("You have to give a class index to display. You can't use names (yet).");
	}
}


/*----------------------------------------------------------------------*/
static void handleObjectsCommand(CONTEXT) {
	char *parameter = strtok(NULL, "");
	if (parameter == NULL)
		listObjects(context);
	else
		showObject(context, atoi(parameter));
}


/*----------------------------------------------------------------------*/
static void handleInstancesCommand(CONTEXT) {
	char *parameter = strtok(NULL, "");
	uint i;

	if (parameter == NULL || strchr(parameter, '*') != 0)
		listInstances(context, parameter);
	else if (Common::isDigit((int)parameter[0]))
		showInstance(context, atoi(parameter));
	else {
		for (i = 1; i < header->instanceMax; i++)
			if (strcmp(parameter, idOfInstance(context, i)) == 0) {
				showInstance(context, i);
				return;
			}
		printf("No instance named '%s'.", parameter);
	}
}

/*----------------------------------------------------------------------*/
static bool exactSameVersion() {
	return header->version[3] == alan.version.version
	       && header->version[2] == alan.version.revision
	       && header->version[1] == alan.version.correction
	       && header->version[0] == alan.version.state[0];
}


/*======================================================================*/
void debug(CONTEXT, bool calledFromBreakpoint, int line, int fileNumber) {
	static bool warned = FALSE;

	saveInfo();
	g_vm->glk_set_style(style_Preformatted);

	if (calledFromBreakpoint)
		displaySourceLocation(line, fileNumber);
	else {
		if (!exactSameVersion() && !warned && !regressionTestOption) {
			printf("<WARNING: You are debugging a game which has version %s.>\n",
			       decodedGameVersion(header->version));
			printf("<That is not exactly the same as this interpreter (%s).>\n", alan.version.string);
			printf("<This might cause a lot of trouble. Cross your fingers...>\n");
			warned = TRUE;
		}
	}

	while (TRUE) {
		char commandLine[200];
		CALL2(readCommand, commandLine, 200)

		char *command = strtok(commandLine, " ");
		char commandCode = parseDebugCommand(command);

		switch (commandCode) {
		case AMBIGUOUS_COMMAND:
			output("Ambiguous ADBG command abbreviation. ? for help.");
			break;
		case ACTORS_COMMAND:
			handleActorsCommand(context);
			break;
		case BREAK_COMMAND:
			handleBreakCommand(fileNumber);
			break;
		case CLASSES_COMMAND:
			handleClassesCommand(context);
			break;
		case DELETE_COMMAND:
			handleDeleteCommand(calledFromBreakpoint, line, fileNumber);
			break;
		case EVENTS_COMMAND:
			showEvents(context);
			break;
		case EXIT_COMMAND:
			debugOption = FALSE;
			restoreInfo();
			goto exit_debug;
		case FILES_COMMAND:
			listFiles();
			break;
		case GO_COMMAND:
			restoreInfo();
			goto exit_debug;
		case HELP_COMMAND:
			handleHelpCommand();
			break;
		case INSTANCES_COMMAND:
			handleInstancesCommand(context);
			break;
		case TRACE_COMMAND:
			handleTraceCommand();
			break;
		case INSTRUCTION_TRACE_COMMAND:
			toggleInstructionTrace();
			break;
		case LOCATIONS_COMMAND:
			handleLocationsCommand(context);
			break;
		case NEXT_COMMAND:
			handleNextCommand(calledFromBreakpoint);
			goto exit_debug;
		case OBJECTS_COMMAND:
			handleObjectsCommand(context);
			break;
		case QUIT_COMMAND:
			CALL1(terminate, 0)
			break;
		case SECTION_TRACE_COMMAND:
			toggleSectionTrace();
			break;
		default:
			output("Unknown ADBG command. ? for help.");
			break;
		}
	}

exit_debug:
	g_vm->glk_set_style(style_Normal);
}


/*======================================================================*/
void traceSay(CONTEXT, int item) {
	/*
	  Say something, but make sure we don't disturb anything and that it is
	  shown to the player. Needed for tracing. During debugging things are
	  set up to avoid this problem.
	*/

	saveInfo();
	needSpace = FALSE;
	col = 1;
	if (item == 0) {
		printf("$null$");
	} else {
		CALL1(say, item)
	}

	needSpace = FALSE;
	col = 1;
	restoreInfo();
}

} // End of namespace Alan3
} // End of namespace Glk
