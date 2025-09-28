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

#include "alcachofa/alcachofa.h"
#include "alcachofa/game.h"
#include "alcachofa/script.h"

using namespace Common;

namespace Alcachofa {

Game::Game()
#ifdef ALCACHOFA_DEBUG // During development let's check out these errors more carefully
	: _message(error)
#else // For release builds the game might still work or the user might still be able to save and restart
	: _message(warning)
#endif
{}

void Game::onLoadedGameFiles() {}

void Game::drawScreenStates() {}

bool Game::doesRoomHaveBackground(const Room *room) {
	return true;
}

void Game::unknownRoomObject(const String &type) {
	_message("Unknown type for room object: %s", type.c_str());
}

void Game::unknownRoomType(const String &type) {
	_message("Unknown type for room: %s", type.c_str());
}

void Game::unknownDoorTargetRoom(const String &name) {
	_message("Unknown door target room: %s", name.c_str());
}

void Game::unknownDoorTargetDoor(const String &room, const String &door) {
	_message("Unknown door target door: %s in %s", door.c_str(), room.c_str());
}

void Game::invalidDialogLine(uint index) {
	_message("Invalid dialog line %u");
}

void Game::tooManyDialogLines(uint lineCount, uint maxLineCount) {
	// we set max line count as constant, if some game uses more we just have to adapt the constant
	// the bug will be not all dialog lines being rendered
	_message("Text to be rendered has too many lines (%u), check text validity and max line count (%u)", lineCount, maxLineCount);
}

void Game::tooManyDrawRequests(int order) {
	// similar, the bug will be some objects not being rendered
	_message("Too many draw requests in order %d", order);
}

bool Game::shouldCharacterTrigger(const Character *character, const char *action) {
	return true;
}

bool Game::shouldTriggerDoor(const Door *door) {
	return true;
}

void Game::onUserChangedCharacter() {}

bool Game::hasMortadeloVoice(const Character *character) {
	return character == &g_engine->world().mortadelo();
}

void Game::unknownCamSetInactiveAttribute(int attribute) {
	// this will be a bug by us, but gameplay should not be affected, so don't error in release builds
	// it could still happen if an attribute was added/removed in updates so we still want users to report this
	_message("Unknown CamSetInactiveAttribute attribute: %d", attribute);
}

void Game::unknownFadeType(int fadeType) {
	_message("Unknown fade type %d", fadeType);
}

void Game::unknownSerializedObject(const char *object, const char *owner, const char *room) {
	// potentially game-breaking for _currentlyUsingObject but might otherwise be just a graphical bug
	_message("Invalid object name \"%s\" saved for \"%s\" in \"%s\"", object, owner, room);
}

void Game::unknownPickupItem(const char *name) {
	_message("Tried to pickup unknown item: %s", name);
}

void Game::unknownDropItem(const char *name) {
	_message("Tried to drop unknown item: %s", name);
}

void Game::unknownVariable(const char *name) {
	_message("Unknown script variable: %s", name);
}

void Game::unknownInstruction(const ScriptInstruction &instruction) {
	const char *type;
	if (instruction._op < 0 || (uint32)instruction._op >= getScriptOpMap().size())
		type = "out-of-bounds";
	else if (getScriptOpMap()[instruction._op] == ScriptOp::Crash)
		type = "crash"; // these are defined in the game, but implemented as write to null-pointer
	else
		type = "unimplemented"; // we forgot to implement them
	_message("Script reached %s instruction: %d %d", type, (int)instruction._op, instruction._arg);
}

void Game::unknownAnimateObject(const char *name) {
	_message("Script tried to animated invalid graphic object: %s", name);
}

void Game::unknownScriptCharacter(const char *action, const char *name) {
	_message("Script tried to %s using invalid character: %s", action, name);
}

PointObject *Game::unknownGoPutTarget(const Process &process, const char *action, const char *name) {
	_message("Script tried to make character %s to invalid object %s", action, name);
	return nullptr;
}

void Game::missingAnimation(const String &fileName) {
	_message("Could not open animation %s", fileName.c_str());
}

void Game::unknownSayTextCharacter(const char *name, int32) {
	unknownScriptCharacter("say text", name);
}

void Game::unknownChangeCharacterRoom(const char *name) {
	_message("Invalid change character room name: %s", name);
}

void Game::unknownAnimateCharacterObject(const char *name) {
	_message("Invalid animate character object: %s", name);
}

void Game::unknownAnimateTalkingObject(const char *name) {
	_message("Invalid talk object name: %s", name);
}

void Game::unknownClearInventoryTarget(int characterKind) {
	_message("Invalid clear inventory character kind: %d", characterKind);
}

void Game::unknownCamLerpTarget(const char *action, const char *name) {
	_message("Invalid target object for %s: %s", action, name);
}

void Game::unknownKernelTask(int task) {
	_message("Invalid kernel task: %d", task);
}

void Game::unknownScriptProcedure(const String &procedure) {
	_message("Unknown required procedure: %s", procedure.c_str());
}

void Game::missingSound(const String &fileName) {
	_message("Missing sound file: %s", fileName.c_str());
}

void Game::invalidSNDFormat(uint format, uint channels, uint freq, uint bps) {
	_message("Invalid SND file, format: %u, channels: %u, freq: %u, bps: %u", format, channels, freq, bps);
}

void Game::notEnoughRoomDataRead(const char *path, int64 filePos, int64 roomEnd) {
	_message("Did not read enough data (%dll < %dll) for a room in %s", filePos, roomEnd, path);
}

void Game::notEnoughObjectDataRead(const char *room, int64 filePos, int64 objectEnd) {
	_message("Did not read enough data (%dll < %dll) for an object in room %s", filePos, objectEnd, room);
}

bool Game::isKnownBadVideo(int32 videoId) {
	return false;
}

void Game::invalidVideo(int32 videoId, const char *context) {
	_message("Could not play video %d (%s)", videoId, context);
}

}
