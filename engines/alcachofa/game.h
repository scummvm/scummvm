/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ALCACHOFA_GAME_H
#define ALCACHOFA_GAME_H

#include "common/textconsole.h"
#include "common/file.h"

namespace Alcachofa {

class ObjectBase;
class PointObject;
class Character;
class Door;
class Room;
class Process;
struct ScriptInstruction;

/**
 * @brief Provides functionality specific to a game title.
 * Also includes all exemptions to inconsistencies in the original games.
 *
 * If an error is truly unrecoverable or a warning never an engine bug, no method is necessary here
 */
class Game {
	typedef void (*Message)(const char *s, ...);
public:
	Game();
	virtual ~Game() {}

	virtual void onLoadedGameFiles();

	virtual bool doesRoomHaveBackground(const Room *room);
	virtual void unknownRoomObject(const Common::String &type);
	virtual void unknownRoomType(const Common::String &type);
	virtual void unknownDoorTargetRoom(const Common::String &name);
	virtual void unknownDoorTargetDoor(const Common::String &room, const Common::String &door);

	virtual void invalidDialogLine(uint index);
	virtual void tooManyDialogLines(uint lineCount, uint maxLineCount);
	virtual void tooManyDrawRequests(int order);

	virtual bool shouldCharacterTrigger(const Character *character, const char *action);
	virtual bool shouldTriggerDoor(const Door *door);
	virtual bool hasMortadeloVoice(const Character *character);
	virtual void onUserChangedCharacter();

	virtual void unknownCamSetInactiveAttribute(int attribute);
	virtual void unknownFadeType(int fadeType);
	virtual void unknownSerializedObject(const char *object, const char *owner, const char *room);
	virtual void unknownPickupItem(const char *name);
	virtual void unknownDropItem(const char *name);
	virtual void unknownVariable(const char *name);
	virtual void unknownInstruction(const ScriptInstruction &instruction);
	virtual void unknownAnimateObject(const char *name);
	virtual void unknownScriptCharacter(const char *action, const char *name);
	virtual PointObject *unknownGoPutTarget(const Process &process, const char *action, const char *name); ///< May return an alternative target to use
	virtual void unknownChangeCharacterRoom(const char *name);
	virtual void unknownAnimateCharacterObject(const char *name);
	virtual void unknownSayTextCharacter(const char *name, int32 dialogId);
	virtual void unknownAnimateTalkingObject(const char *name);
	virtual void unknownClearInventoryTarget(int characterKind);
	virtual void unknownCamLerpTarget(const char *action, const char *name);
	virtual void unknownKernelTask(int task);
	virtual void unknownScriptProcedure(const Common::String &procedure);

	virtual void missingAnimation(const Common::String &fileName);
	virtual void missingSound(const Common::String &fileName);
	virtual void invalidSNDFormat(uint format, uint channels, uint freq, uint bps);
	virtual void notEnoughRoomDataRead(const char *path, int64 filePos, int64 objectEnd);
	virtual void notEnoughObjectDataRead(const char *room, int64 filePos, int64 objectEnd);
	virtual bool isKnownBadVideo(int32 videoId);
	virtual void invalidVideo(int32 videoId, const char *context);

	static Game *createForMovieAdventure();

	const Message _message;
};

}

#endif // ALCACHOFA_GAME_H
