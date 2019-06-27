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

#include "glk/alan3/state.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/current.h"
#include "glk/alan3/word.h"
#include "glk/alan3/state_stack.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/attribute.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/score.h"
#include "glk/alan3/event.h"
#include "glk/alan3/set.h"

namespace Glk {
namespace Alan3 {

/* PRIVATE TYPES */

/* Implementation of the abstract type typedef struct game_state GameState */
struct game_state {
	/* Event queue */
	EventQueueEntry *eventQueue;
	int eventQueueTop;          /* Event queue top pointer */

	/* Scores */
	int score;
	Aword *scores;              /* Score table pointer */

	/* Instance data */
	AdminEntry *admin;          /* Administrative data about instances */
	AttributeEntry *attributes; /* Attributes data area */
	/* Sets and strings are dynamically allocated areas for which the
	   attribute is just a pointer to. So they are not catched by the
	   saving of attributes, instead they require special storage */
	Set **sets;                 /* Array of set pointers */
	char **strings;             /* Array of string pointers */
};

/* PRIVATE DATA */
static GameState gameState;     /* TODO: Make pointer, then we don't have to copy to stack, we can just use the pointer */
static StateStackP stateStack = NULL;

static char *playerCommand;


/*----------------------------------------------------------------------*/
static int countStrings(void) {
	StringInitEntry *entry;
	int count = 0;

	if (header->stringInitTable != 0)
		for (entry = (StringInitEntry *)pointerTo(header->stringInitTable); * (Aword *)entry != EOD; entry++)
			count++;
	return (count);
}


/*----------------------------------------------------------------------*/
static void deallocateStrings(GameState *gState) {
	int count = countStrings();
	int i;

	for (i = 0; i < count; i++)
		deallocate(gState->strings[i]);
	deallocate(gState->strings);
}

/*----------------------------------------------------------------------*/
static int countSets(void) {
	SetInitEntry *entry;
	int count = 0;

	if (header->setInitTable != 0)
		for (entry = (SetInitEntry *)pointerTo(header->setInitTable); * (Aword *)entry != EOD; entry++)
			count++;
	return (count);
}


/*----------------------------------------------------------------------*/
static void deallocateSets(GameState *gState) {
	int count = countSets();
	int i;

	for (i = 0; i < count; i++)
		freeSet(gState->sets[i]);
	deallocate(gState->sets);
}

/*======================================================================*/
void deallocateGameState(GameState *gState) {

	deallocate(gState->admin);
	deallocate(gState->attributes);

	if (gState->eventQueueTop > 0) {
		deallocate(gState->eventQueue);
		gState->eventQueue = NULL;
	}
	if (gState->scores)
		deallocate(gState->scores);

	deallocateStrings(gState);
	deallocateSets(gState);

	memset(gState, 0, sizeof(GameState));
}


/*======================================================================*/
void forgetGameState(void) {
	char *playerCmd;
	popGameState(stateStack, &gameState, &playerCmd);
	deallocateGameState(&gameState);
	if (playerCmd != NULL)
		deallocate(playerCmd);
}


/*======================================================================*/
void initStateStack(void) {
	if (stateStack != NULL)
		deleteStateStack(stateStack);
	stateStack = createStateStack(sizeof(GameState));
}


/*======================================================================*/
void terminateStateStack(void) {
	deleteStateStack(stateStack);
	stateStack = NULL;
}


/*======================================================================*/
bool anySavedState(void) {
	return !stateStackIsEmpty(stateStack);
}


/*----------------------------------------------------------------------*/
static Set **collectSets(void) {
	SetInitEntry *entry;
	int count = countSets();
	Set **sets;
	int i;

	if (count == 0) return NULL;

	sets = (Set **)allocate(count * sizeof(Set));

	entry = (SetInitEntry *)pointerTo(header->setInitTable);
	for (i = 0; i < count; i++)
		sets[i] = getInstanceSetAttribute(entry[i].instanceCode, entry[i].attributeCode);

	return sets;
}


/*----------------------------------------------------------------------*/
static char **collectStrings(void) {
	StringInitEntry *entry;
	int count = countStrings();
	char **strings;
	int i;

	if (count == 0) return NULL;

	strings = (char **)allocate(count * sizeof(char *));

	entry = (StringInitEntry *)pointerTo(header->stringInitTable);
	for (i = 0; i < count; i++)
		strings[i] = getInstanceStringAttribute(entry[i].instanceCode, entry[i].attributeCode);

	return strings;
}


/*======================================================================*/
void rememberCommands(void) {
	char *command = playerWordsAsCommandString();
	attachPlayerCommandsToLastState(stateStack, command);
	deallocate(command);
}


/*----------------------------------------------------------------------*/
static void collectEvents(void) {
	gameState.eventQueueTop = eventQueueTop;
	if (eventQueueTop > 0)
		gameState.eventQueue = (EventQueueEntry *)duplicate(eventQueue, eventQueueTop * sizeof(EventQueueEntry));
}


/*----------------------------------------------------------------------*/
static void collectInstanceData(void) {
	gameState.admin = (AdminEntry *)duplicate(admin, (header->instanceMax + 1) * sizeof(AdminEntry));
	gameState.attributes = (AttributeEntry *)duplicate(attributes, header->attributesAreaSize * sizeof(Aword));
	gameState.sets = collectSets();
	gameState.strings = collectStrings();
}


/*----------------------------------------------------------------------*/
static void collectScores(void) {
	gameState.score = current.score;
	if (scores == NULL)
		gameState.scores = NULL;
	else
		gameState.scores = (Aword *)duplicate(scores, header->scoreCount * sizeof(Aword));
}


/*======================================================================*/
void rememberGameState(void) {
	collectEvents();
	collectInstanceData();
	collectScores();

	if (stateStack == NULL)
		initStateStack();

	pushGameState(stateStack, &gameState);
	gameStateChanged = FALSE;
}


/*----------------------------------------------------------------------*/
static void freeCurrentSetAttributes(void) {
	SetInitEntry *entry;

	if (header->setInitTable == 0) return;
	for (entry = (SetInitEntry *)pointerTo(header->setInitTable); * (Aword *)entry != EOD; entry++) {
		Aptr attributeValue = getAttribute(admin[entry->instanceCode].attributes, entry->attributeCode);
		freeSet((Set *)fromAptr(attributeValue));
	}
}


/*----------------------------------------------------------------------*/
static void recallSets(Set **sets) {
	SetInitEntry *entry;
	int count = countSets();
	int i;

	if (header->setInitTable == 0) return;

	entry = (SetInitEntry *)pointerTo(header->setInitTable);
	for (i = 0; i < count; i++) {
		setAttribute(admin[entry[i].instanceCode].attributes, entry[i].attributeCode, toAptr(sets[i]));
		sets[i] = NULL; /* Since we reuse the saved set, we need to clear the pointer */
	}
}


/*----------------------------------------------------------------------*/
static void freeCurrentStringAttributes(void) {
	StringInitEntry *entry;

	if (header->stringInitTable == 0) return;
	for (entry = (StringInitEntry *)pointerTo(header->stringInitTable); * (Aword *)entry != EOD; entry++) {
		Aptr attributeValue = getAttribute(admin[entry->instanceCode].attributes, entry->attributeCode);
		deallocate(fromAptr(attributeValue));
	}
}


/*----------------------------------------------------------------------*/
static void recallStrings(char **strings) {
	StringInitEntry *entry;
	int count = countStrings();
	int i;

	if (header->stringInitTable == 0) return;

	entry = (StringInitEntry *)pointerTo(header->stringInitTable);
	for (i = 0; i < count; i++) {
		setAttribute(admin[entry[i].instanceCode].attributes, entry[i].attributeCode, toAptr(strings[i]));
		strings[i] = NULL;      /* Since we reuse the saved, we need to clear the state */
	}
}


/*----------------------------------------------------------------------*/
static void recallEvents(void) {
	eventQueueTop = gameState.eventQueueTop;
	if (eventQueueTop > 0) {
		memcpy(eventQueue, gameState.eventQueue,
		       (eventQueueTop + 1)*sizeof(EventQueueEntry));
	}
}


/*----------------------------------------------------------------------*/
static void recallInstances(void) {

	if (admin == NULL)
		syserr("admin[] == NULL in recallInstances()");

	memcpy(admin, gameState.admin,
	       (header->instanceMax + 1)*sizeof(AdminEntry));

	freeCurrentSetAttributes();     /* Need to free previous set values */
	freeCurrentStringAttributes();  /* Need to free previous string values */

	memcpy(attributes, gameState.attributes,
	       header->attributesAreaSize * sizeof(Aword));

	recallSets(gameState.sets);
	recallStrings(gameState.strings);
}


/*----------------------------------------------------------------------*/
static void recallScores(void) {
	current.score = gameState.score;
	memcpy(scores, gameState.scores, header->scoreCount * sizeof(Aword));
}


/*======================================================================*/
void recallGameState(void) {
	popGameState(stateStack, &gameState, &playerCommand);
	recallEvents();
	recallInstances();
	recallScores();
	deallocateGameState(&gameState);
}


/*======================================================================*/
char *recreatePlayerCommand(void) {
	return playerCommand;
}

} // End of namespace Alan3
} // End of namespace Glk
