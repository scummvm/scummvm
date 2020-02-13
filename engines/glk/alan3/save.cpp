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

static void saveGameInfo(Common::WriteStream *saveFile) {
	saveFile->writeUint32BE(MKTAG('A', 'S', 'A', 'V'));
	saveFile->write(header->version, 4);
	saveFile->writeUint32LE(header->uid);
}

static void verifySaveFile(CONTEXT, Common::SeekableReadStream *saveFile) {
	if (saveFile->readUint32BE() != MKTAG('A', 'S', 'A', 'V'))
		error(context, M_NOTASAVEFILE);
}

static void verifyCompilerVersion(CONTEXT, Common::SeekableReadStream *saveFile) {
	char savedVersion[4];

	saveFile->read(&savedVersion, 4);
	if (!ignoreErrorOption && memcmp(savedVersion, header->version, 4))
		error(context, M_SAVEVERS);
}

static void verifyGameId(CONTEXT, Common::SeekableReadStream *saveFile) {
	Aword savedUid = saveFile->readUint32LE();
	if (!ignoreErrorOption && savedUid != header->uid)
		error(context, M_SAVEVERS);
}

void syncGame(Common::Serializer &s) {
	// Current values
	current.synchronize(s);

	// Attributes area
	for (Aint i = 0; i < header->attributesAreaSize / 3; ++i)
		attributes[i].synchronize(s);

	// Admin data
	for (uint i = 1; i <= header->instanceMax; i++)
		admin[i].synchronize(s);

	// Event queue
	s.syncAsSint32LE(eventQueueTop);
	for (int i = 0; i < eventQueueTop; ++i)
		eventQueue[i].synchronize(s);

	// Scores
	for (Aint i = 0; i < header->scoreCount; ++i)
		s.syncAsUint32LE(scores[i]);

	// Strings
	if (header->stringInitTable != 0)
		for (StringInitEntry *initEntry = (StringInitEntry *)pointerTo(header->stringInitTable);
				!isEndOfArray(initEntry); initEntry++) {

		if (s.isSaving()) {
			char *attr = (char *)getInstanceStringAttribute(initEntry->instanceCode, initEntry->attributeCode);
			Aint length = strlen(attr) + 1;
			s.syncAsUint32LE(length);
			s.syncBytes((byte *)attr, length);
		} else {
			Aint length = 0;
			s.syncAsUint32LE(length);
			char *string = (char *)allocate(length + 1);
			s.syncBytes((byte *)string, length);
			setInstanceAttribute(initEntry->instanceCode, initEntry->attributeCode, toAptr(string));
		}
	}

	// Sets
	if (header->setInitTable != 0) {
		for (SetInitEntry *initEntry = (SetInitEntry *)pointerTo(header->setInitTable);
				!isEndOfArray(initEntry); initEntry++) {

			if (s.isSaving()) {
				Set *attr = (Set *)getInstanceSetAttribute(initEntry->instanceCode, initEntry->attributeCode);
				s.syncAsUint32LE(attr->size);
				for (int i = 0; i < attr->size; ++i)
					s.syncAsUint32LE(attr->members[i]);

			} else {
				Aword setSize = 0, member = 0;
				s.syncAsUint32BE(setSize);
				Set *set = newSet(setSize);
				for (uint i = 0; i < setSize; ++i) {
					s.syncAsUint32LE(member);
					addToSet(set, member);
				}

				setInstanceAttribute(initEntry->instanceCode, initEntry->attributeCode, toAptr(set));
			}
		}
	}
}

void saveGame(Common::WriteStream *saveFile) {
	// Save tag, version of interpreter, and unique id of game
	saveGameInfo(saveFile);

	// Save game data
	Common::Serializer s(nullptr, saveFile);
	syncGame(s);
}

bool restoreGame(Common::SeekableReadStream *saveFile) {
	Context ctx;
	verifySaveFile(ctx, saveFile);
	if (ctx._break) return false;

	// Verify version of compiler/interpreter of saved game with us
	verifyCompilerVersion(ctx, saveFile);
	if (ctx._break) return false;

	// Verify unique id of game
	verifyGameId(ctx, saveFile);
	if (ctx._break) return false;

	// Restore game data
	Common::Serializer s(saveFile, nullptr);
	syncGame(s);

	return true;
}

} // End of namespace Alan3
} // End of namespace Glk
