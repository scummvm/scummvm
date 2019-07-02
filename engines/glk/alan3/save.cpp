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

#include "glk/alan3/save.h"
#include "glk/alan3/acode.h"
#include "glk/alan3/args.h"
#include "glk/alan3/current.h"
#include "glk/alan3/event.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/options.h"
#include "glk/alan3/score.h"
#include "glk/alan3/types.h"

namespace Glk {
namespace Alan3 {

/*----------------------------------------------------------------------*/
static void saveStrings(Common::WriteStream *saveFile) {
	StringInitEntry *initEntry;

	if (header->stringInitTable != 0)
		for (initEntry = (StringInitEntry *)pointerTo(header->stringInitTable);
		        !isEndOfArray(initEntry); initEntry++) {
			char *attr = (char *)getInstanceStringAttribute(initEntry->instanceCode, initEntry->attributeCode);
			Aint length = strlen(attr) + 1;
			saveFile->writeUint32LE(length);
			saveFile->write(attr, length);
		}
}


/*----------------------------------------------------------------------*/
static void saveSets(Common::WriteStream *saveFile) {
	SetInitEntry *initEntry;

	if (header->setInitTable != 0)
		for (initEntry = (SetInitEntry *)pointerTo(header->setInitTable);
		        !isEndOfArray(initEntry); initEntry++) {
			Set *attr = (Set *)getInstanceSetAttribute(initEntry->instanceCode, initEntry->attributeCode);
			saveFile->writeUint32LE(attr->size);
			saveFile->write(attr->members, attr->size);
		}
}


/*----------------------------------------------------------------------*/
static void saveGameInfo(Common::WriteStream *saveFile) {
	saveFile->writeUint32BE(MKTAG('A', 'S', 'A', 'V'));
	saveFile->write(header->version, 4);
	saveFile->write(adventureName, strlen(adventureName) + 1);
	saveFile->writeUint32LE(header->uid);
}


/*----------------------------------------------------------------------*/
static void saveAdmin(Common::WriteStream *saveFile) {
	Common::Serializer s(nullptr, saveFile);
	for (uint i = 1; i <= header->instanceMax; i++)
		admin[i].synchronize(s);
}


/*----------------------------------------------------------------------*/
static void saveAttributeArea(Common::WriteStream *saveFile) {
	Common::Serializer s(nullptr, saveFile);
	for (Aint i = 0; i < header->attributesAreaSize; ++i)
		attributes[i].synchronize(s);
}


/*----------------------------------------------------------------------*/
static void saveEventQueue(Common::WriteStream *saveFile) {
	Common::Serializer s(nullptr, saveFile);

	s.syncAsSint32LE(eventQueueTop);
	for (int i = 0; i < eventQueueTop; ++i)
		eventQueue[i].synchronize(s);
}


/*----------------------------------------------------------------------*/
static void saveCurrentValues(Common::WriteStream *saveFile) {
	Common::Serializer s(nullptr, saveFile);
	current.synchronize(s);
}


/*----------------------------------------------------------------------*/
static void saveScores(Common::WriteStream *saveFile) {
	for (Aint i = 0; i < header->scoreCount; ++i)
		saveFile->writeUint32LE(scores[i]);
}


/*----------------------------------------------------------------------*/
void saveGame(Common::WriteStream *saveFile) {
	/* Save tag, version of interpreter, name and uid of game */
	saveGameInfo(saveFile);

	/* Save current values */
	saveCurrentValues(saveFile);

	saveAttributeArea(saveFile);
	saveAdmin(saveFile);

	saveEventQueue(saveFile);

	saveScores(saveFile);

	saveStrings(saveFile);
	saveSets(saveFile);
}


/*----------------------------------------------------------------------*/
static void restoreStrings(Common::SeekableReadStream *saveFile) {
	StringInitEntry *initEntry;

	if (header->stringInitTable != 0)
		for (initEntry = (StringInitEntry *)pointerTo(header->stringInitTable);
		        !isEndOfArray(initEntry); initEntry++) {
			Aint length = saveFile->readUint32LE();
			char *string = (char *)allocate(length + 1);

			saveFile->read(string, length);
			setInstanceAttribute(initEntry->instanceCode, initEntry->attributeCode, toAptr(string));
		}
}


/*----------------------------------------------------------------------*/
static void restoreSets(Common::SeekableReadStream *saveFile) {
	SetInitEntry *initEntry;

	if (header->setInitTable != 0)
		for (initEntry = (SetInitEntry *)pointerTo(header->setInitTable);
		        !isEndOfArray(initEntry); initEntry++) {
			Aint setSize = saveFile->readUint32LE();
			Set *set = newSet(setSize);

			for (int i = 0; i < setSize; i++) {
				Aword member = saveFile->readUint32LE();
				addToSet(set, member);
			}
			setInstanceAttribute(initEntry->instanceCode, initEntry->attributeCode, toAptr(set));
		}
}


/*----------------------------------------------------------------------*/
static void restoreScores(Common::SeekableReadStream *saveFile) {
	for (Aint i = 0; i < header->scoreCount; ++i)
		scores[i] = saveFile->readUint32LE();
}


/*----------------------------------------------------------------------*/
static void restoreEventQueue(Common::SeekableReadStream *saveFile) {
	Common::Serializer s(saveFile, nullptr);

	s.syncAsSint32LE(eventQueueTop);
	for (int i = 0; i < eventQueueTop; ++i)
		eventQueue[i].synchronize(s);
}


/*----------------------------------------------------------------------*/
static void restoreAdmin(Common::SeekableReadStream *saveFile) {
	// Restore admin for instances, remember to reset attribute area pointer
	Common::Serializer s(saveFile, nullptr);
	for (uint i = 1; i <= header->instanceMax; i++) {
		AttributeEntry *currentAttributesArea = admin[i].attributes;
		admin[i].synchronize(s);
		admin[i].attributes = currentAttributesArea;
	}
}


/*----------------------------------------------------------------------*/
static void restoreAttributeArea(Common::SeekableReadStream *saveFile) {
	Common::Serializer s(saveFile, nullptr);
	for (Aint i = 0; i < header->attributesAreaSize; ++i)
		attributes[i].synchronize(s);
}


/*----------------------------------------------------------------------*/
static void restoreCurrentValues(Common::SeekableReadStream *saveFile) {
	Common::Serializer s(saveFile, nullptr);
	current.synchronize(s);
}


/*----------------------------------------------------------------------*/
static void verifyGameId(CONTEXT, Common::SeekableReadStream *saveFile) {
	Aword savedUid = saveFile->readUint32LE();
	if (!ignoreErrorOption && savedUid != header->uid)
		error(context, M_SAVEVERS);
}


/*----------------------------------------------------------------------*/
static void verifyGameName(CONTEXT, Common::SeekableReadStream *saveFile) {
	char savedName[256];
	int i = 0;

	while ((savedName[i++] = saveFile->readByte()) != '\0');
	if (strcmp(savedName, adventureName) != 0)
		error(context, M_SAVENAME);
}


/*----------------------------------------------------------------------*/
static void verifyCompilerVersion(CONTEXT, Common::SeekableReadStream *saveFile) {
	char savedVersion[4];

	saveFile->read(&savedVersion, 4);
	if (!ignoreErrorOption && memcmp(savedVersion, header->version, 4))
		error(context, M_SAVEVERS);
}


/*----------------------------------------------------------------------*/
static void verifySaveFile(CONTEXT, Common::SeekableReadStream *saveFile) {
	char string[5];
	saveFile->read(string, 4);
	string[4] = '\0';

	if (strcmp(string, "ASAV") != 0)
		error(context, M_NOTASAVEFILE);
}


/*----------------------------------------------------------------------*/
bool restoreGame(Common::SeekableReadStream *saveFile) {
	Context ctx;
	verifySaveFile(ctx, saveFile);
	if (ctx._break) return false;

	// Verify version of compiler/interpreter of saved game with us
	verifyCompilerVersion(ctx, saveFile);
	if (ctx._break) return false;

	// Verify name of game 
	verifyGameName(ctx, saveFile);
	if (ctx._break) return false;

	// Verify unique id of game
	verifyGameId(ctx, saveFile);
	if (ctx._break) return false;

	restoreCurrentValues(saveFile);
	restoreAttributeArea(saveFile);
	restoreAdmin(saveFile);
	restoreEventQueue(saveFile);
	restoreScores(saveFile);
	restoreStrings(saveFile);
	restoreSets(saveFile);

	return true;
}

} // End of namespace Alan3
} // End of namespace Glk
