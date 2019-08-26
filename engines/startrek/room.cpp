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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "startrek/iwfile.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

#include "rooms/function_map.h"

// TODO: Delete this macro, replacing it with the next one.
// New "[roomName]NumActions" variables need to be made before that.
#define ADD_ROOM_OLD(ROOM) {\
		if (name.equalsIgnoreCase(#ROOM)) {\
			_roomActionList = ROOM##ActionList;\
			_numRoomActions = ARRAYSIZE(ROOM##ActionList);\
		}\
	}

#define ADD_ROOM(ROOM) {\
		if (name.equalsIgnoreCase(#ROOM)) {\
			_roomActionList = ROOM##ActionList;\
			_numRoomActions = ROOM##NumActions;\
		}\
	}

namespace StarTrek {

Room::Room(StarTrekEngine *vm, const Common::String &name) : _vm(vm), _awayMission(&vm->_awayMission) {
	Common::MemoryReadStreamEndian *rdfFile = _vm->loadFile(name + ".RDF");

	int size = rdfFile->size();
	_rdfData = new byte[size];
	rdfFile->read(_rdfData, size);
	delete rdfFile;

	_roomActionList = nullptr;

	ADD_ROOM_OLD(demon0);
	ADD_ROOM_OLD(demon1);
	ADD_ROOM_OLD(demon2);
	ADD_ROOM_OLD(demon3);
	ADD_ROOM_OLD(demon4);
	ADD_ROOM_OLD(demon5);
	ADD_ROOM_OLD(demon6);
	ADD_ROOM_OLD(tug0);
	ADD_ROOM_OLD(tug1);
	ADD_ROOM_OLD(tug2);
	ADD_ROOM_OLD(tug3);
	ADD_ROOM_OLD(love0);
	ADD_ROOM_OLD(love1);
	ADD_ROOM_OLD(love2);
	ADD_ROOM_OLD(love3);
	ADD_ROOM_OLD(love4);
	ADD_ROOM_OLD(love5);
	ADD_ROOM_OLD(mudd0);
	ADD_ROOM_OLD(mudd1);
	ADD_ROOM_OLD(mudd2);
	ADD_ROOM_OLD(mudd3);
	ADD_ROOM_OLD(mudd4);
	ADD_ROOM_OLD(mudd5);
	ADD_ROOM_OLD(feather0);
	ADD_ROOM(feather1);
	ADD_ROOM(feather2);
	ADD_ROOM(feather3);
	ADD_ROOM(feather4);
	ADD_ROOM(feather5);
	ADD_ROOM(feather6);
	ADD_ROOM(feather7);
	ADD_ROOM(trial0);
	ADD_ROOM(trial1);
	ADD_ROOM(trial2);
	ADD_ROOM(trial3);
	ADD_ROOM(trial4);
	ADD_ROOM(trial5);
	ADD_ROOM(sins0);
	ADD_ROOM(sins1);
	ADD_ROOM(sins2);
	ADD_ROOM(sins3);
	ADD_ROOM(sins4);
	ADD_ROOM(sins5);
	ADD_ROOM(veng0);
	ADD_ROOM(veng1);
	ADD_ROOM(veng2);
	ADD_ROOM(veng3);
	ADD_ROOM(veng4);
	ADD_ROOM(veng5);
	ADD_ROOM(veng6);
	ADD_ROOM(veng7);
	ADD_ROOM(veng8);

	if (_roomActionList == nullptr) {
		warning("Room \"%s\" unimplemented", name.c_str());
		_numRoomActions = 0;
	}

	loadRoomMessages();
	loadOtherRoomMessages();
	memset(&_roomVar, 0, sizeof(_roomVar));
}

Room::~Room() {
	_lookMessages.clear();
	_lookWithTalkerMessages.clear();
	_talkMessages.clear();
	delete[] _rdfData;
}

void Room::loadRoomMessages() {
	// TODO: There are some more messages which are not stored in that offset
	uint16 messagesOffset = readRdfWord(32);
	const char *text = (const char *)_rdfData + messagesOffset;
	const char roomIndexChar = '0' + _vm->_roomIndex;

	do {
		while (text[0] != '#' || (text[1] != _vm->_missionName[0] && text[4] != roomIndexChar))
			text++;

		if (text[5] == '\\')
			loadRoomMessage(text);

		while (*text != '\0')
			text++;

		// Peek the next byte, in case there's a filler text
		if (Common::isAlpha(*(text + 1))) {
			while (*text != '\0')
				text++;
		}
	} while (*(text + 1) == '#');
}

void Room::loadRoomMessage(const char *text) {
	int messageNum;
	bool isTalkMessage;
	bool isLookWithTalkerMessage;
	char textType = text[10];	// _ and U: talk message, N: look message, L: look with talker message

	if (text[5] != '\\')
		error("loadRoomMessage: Invalid message");

	isTalkMessage = (textType == '_' || textType == 'U');	// U = Uhura
	isLookWithTalkerMessage = (textType == 'L');

	sscanf((const char *)(text + 11), "%3d", &messageNum);
	if (text[14] != '#')
		error("loadRoomMessage: Invalid message");

	if (isTalkMessage)
		_talkMessages[messageNum] = Common::String((const char *)text);
	else if (isLookWithTalkerMessage)
		_lookWithTalkerMessages[messageNum] = Common::String((const char *)text);
	else
		_lookMessages[messageNum] = Common::String((const char *)text);

}

void Room::loadOtherRoomMessages() {
	uint16 startOffset = readRdfWord(14);
	uint16 endOffset = readRdfWord(16);
	uint16 offset = startOffset;

	while (offset < endOffset) {
		uint16 nextOffset = readRdfWord(offset + 4);
		if (nextOffset >= endOffset)
			break;

		while (offset < nextOffset) {
			const char *text = (const char *)_rdfData + offset;

			if (text[0] == '#' && text[1] == _vm->_missionName[0] && text[5] == '\\')
				loadRoomMessage(text);

			offset++;
		}
	}
}

uint16 Room::readRdfWord(int offset) {
	return READ_LE_UINT16((_rdfData + offset));
}

bool Room::actionHasCode(const Action &action) {
	const RoomAction *roomActionPtr = _roomActionList;
	int n = _numRoomActions;

	while (n-- > 0) {
		if (action == roomActionPtr->action)
			return true;
		roomActionPtr++;
	}
	return false;
}

bool Room::actionHasCode(byte type, byte b1, byte b2, byte b3) {
	const Action a = {type, b1, b2, b3};
	return actionHasCode(a);
}

bool Room::handleAction(const Action &action) {
	const RoomAction *roomActionPtr = _roomActionList;
	int n = _numRoomActions;

	while (n-- > 0) {
		if (action == roomActionPtr->action) {
			_vm->_awayMission.rdfStillDoDefaultAction = false;
			(this->*(roomActionPtr->funcPtr))();
			if (!_vm->_awayMission.rdfStillDoDefaultAction)
				return true;
		}
		roomActionPtr++;
	}
	return false;
}

bool Room::handleAction(byte type, byte b1, byte b2, byte b3) {
	const Action a = {type, b1, b2, b3};
	return handleAction(a);
}

bool Room::handleActionWithBitmask(const Action &action) {
	const RoomAction *roomActionPtr = _roomActionList;
	int n = _numRoomActions;

	while (n-- > 0) {
		uint32 bitmask = roomActionPtr->action.getBitmask();
		if ((action.toUint32() & bitmask) == (roomActionPtr->action.toUint32() & bitmask)) {
			_vm->_awayMission.rdfStillDoDefaultAction = false;
			(this->*(roomActionPtr->funcPtr))();
			if (!_vm->_awayMission.rdfStillDoDefaultAction)
				return true;
		}
		roomActionPtr++;
	}
	return false;
}

bool Room::handleActionWithBitmask(byte type, byte b1, byte b2, byte b3) {
	Action a = {type, b1, b2, b3};
	return handleActionWithBitmask(a);
}

Common::Point Room::getBeamInPosition(int crewmanIndex) {
	int base = RDF_BEAM_IN_POSITIONS + crewmanIndex * 4;
	return Common::Point(readRdfWord(base), readRdfWord(base + 2));
}

Common::Point Room::getSpawnPosition(int crewmanIndex) {
	int base = RDF_SPAWN_POSITIONS + crewmanIndex * 4;
	return Common::Point(readRdfWord(base), readRdfWord(base + 2));
}

// For actions of type ACTION_FINISHED_ANIMATION or ACTION_FINISHED_WALKING, this takes
// a function pointer and returns the index corresponding to that callback.
// Creates a fatal error on failure.
int Room::findFunctionPointer(int action, void (Room::*funcPtr)()) {
	assert(action == ACTION_FINISHED_ANIMATION || action == ACTION_FINISHED_WALKING);

	for (int i = 0; i < _numRoomActions; i++) {
		if (_roomActionList[i].action.type == action && _roomActionList[i].funcPtr == funcPtr)
			return _roomActionList[i].action.b1;
	}

	if (action == ACTION_FINISHED_ANIMATION)
		error("Couldn't find FINISHED_ANIMATION function pointer");
	else
		error("Couldn't find FINISHED_WALKING function pointer");
}

// Interface for room-specific code

void Room::loadActorAnim(int actorIndex, Common::String anim, int16 x, int16 y, uint16 finishedAnimActionParam) {
	Actor *actor = &_vm->_actorList[actorIndex];

	if (x == -1 || y == -1) {
		x = actor->sprite.pos.x;
		y = actor->sprite.pos.y;
	}

	if (actorIndex >= 0 && actorIndex < SCALED_ACTORS_END)
		_vm->loadActorAnimWithRoomScaling(actorIndex, anim, x, y);
	else
		_vm->loadActorAnim(actorIndex, anim, x, y, 1.0);

	if (finishedAnimActionParam != 0) {
		actor->triggerActionWhenAnimFinished = true;
		actor->finishedAnimActionParam = finishedAnimActionParam;
	}
}

// Same as above, but accepts a callback for when the animation finished (instead of an
// integer for an action)
void Room::loadActorAnimC(int actorIndex, Common::String anim, int16 x, int16 y, void (Room::*funcPtr)()) {
	Actor *actor = &_vm->_actorList[actorIndex];

	if (x == -1 || y == -1) {
		x = actor->sprite.pos.x;
		y = actor->sprite.pos.y;
	}

	if (actorIndex >= 0 && actorIndex < SCALED_ACTORS_END)
		_vm->loadActorAnimWithRoomScaling(actorIndex, anim, x, y);
	else
		_vm->loadActorAnim(actorIndex, anim, x, y, 1.0);

	if (funcPtr != nullptr) {
		actor->triggerActionWhenAnimFinished = true;
		actor->finishedAnimActionParam = findFunctionPointer(ACTION_FINISHED_ANIMATION, funcPtr);
	}
}

void Room::loadActorStandAnim(int actorIndex) {
	if (_vm->_awayMission.redshirtDead && actorIndex == OBJECT_REDSHIRT)
		_vm->removeActorFromScreen(actorIndex);
	else {
		Actor *actor = &_vm->_actorList[actorIndex];
		if (actor->animationString.empty())
			_vm->removeActorFromScreen(actorIndex);
		else
			_vm->initStandAnim(actorIndex);
	}
}

void Room::loadActorAnim2(int actorIndex, Common::String anim, int16 x, int16 y, uint16 finishedAnimActionParam) {
	loadActorAnim(actorIndex, anim, x, y, finishedAnimActionParam);
}

int Room::showRoomSpecificText(const char **array) {
	Common::String speaker;
	byte textColor;

	if (array[0] != nullptr && array[0][0] != '\0') {
		speaker = Common::String(array[0]);
		if (speaker.equalsIgnoreCase("Capt. Kirk"))
			textColor = TEXTCOLOR_YELLOW;
		else if (speaker.equalsIgnoreCase("Mr. Spock"))
			textColor = TEXTCOLOR_BLUE;
		else if (speaker.equalsIgnoreCase("Dr. McCoy"))
			textColor = TEXTCOLOR_BLUE;
		else if (speaker.equalsIgnoreCase("Mr. Chekov"))
			textColor = TEXTCOLOR_YELLOW;
		else if (speaker.equalsIgnoreCase("Mr. Scott"))
			textColor = TEXTCOLOR_RED;
		else if (speaker.hasPrefixIgnoreCase("Lt"))
			textColor = TEXTCOLOR_RED;
		else if (speaker.hasPrefixIgnoreCase("Ensign"))
			textColor = TEXTCOLOR_RED;
		else
			textColor = TEXTCOLOR_GREY;
	} else
		textColor = TEXTCOLOR_YELLOW;

	return _vm->showText(&StarTrekEngine::readTextFromArrayWithChoices, (uintptr)array, 20, 20, textColor, true, false, false);
}

int Room::showMultipleTexts(const TextRef *textIDs, bool fromRDF, bool lookWithTalker) {
	int numIDs = 0;
	int retval;
	while (textIDs[numIDs] != TX_BLANK)
		numIDs++;

	const char **text = (const char **)malloc(sizeof(const char *) * (numIDs + 1));

	for (int i = 0; i <= numIDs; i++) {
		// TODO: This isn't nice, but it's temporary till we migrate to reading text from RDF files
		if (i > 0 && fromRDF) {
			if (textIDs[0] == TX_NULL)
				text[i] = _lookMessages[textIDs[i]].c_str();
			else if (lookWithTalker)
				text[i] = _lookWithTalkerMessages[textIDs[i]].c_str();
			else
				text[i] = _talkMessages[textIDs[i]].c_str();
		} else
			text[i] = g_gameStrings[textIDs[i]];
	}

	retval = showRoomSpecificText(text);
	free(text);

	return retval;
}

int Room::showText(TextRef speaker, TextRef text, bool fromRDF, bool lookWithTalker) {
	TextRef textIDs[3];
	textIDs[0] = speaker;
	textIDs[1] = text;
	textIDs[2] = TX_BLANK;
	return showMultipleTexts(textIDs, fromRDF, lookWithTalker);
}

int Room::showDescription(TextRef text, bool fromRDF, bool lookWithTalker) {
	return showText(TX_NULL, text, fromRDF, lookWithTalker);
}

void Room::giveItem(int item) {
	assert(item >= ITEMS_START && item < ITEMS_END);
	_vm->_itemList[item - ITEMS_START].have = true;
}

void Room::loadRoomIndex(int roomIndex, int spawnIndex) {
	if (_vm->_awayMission.crewDownBitset != 0)
		return;

	_vm->_missionToLoad = _vm->_missionName;
	_vm->_roomIndexToLoad = roomIndex;
	_vm->_spawnIndexToLoad = spawnIndex;

	// WORKAROUND: original game manipulates the stack to return directly to the start of
	// "runAwayMission". Instead, we set some variables and the room will be changed
	// later. (We wouldn't want to delete the room we're currently in...)
}

void Room::loseItem(int item) {
	assert(item >= ITEMS_START && item < ITEMS_END);
	_vm->_itemList[item - ITEMS_START].have = false;

	if (_vm->_awayMission.activeAction == ACTION_USE && _vm->_awayMission.activeObject == item) {
		_vm->_awayMission.activeAction = ACTION_WALK;
		_vm->chooseMouseBitmapForAction(ACTION_WALK, false);
		_vm->hideInventoryIcons();
	}
}

void Room::walkCrewman(int actorIndex, int16 destX, int16 destY, uint16 finishedAnimActionParam) {
	if (!(actorIndex >= OBJECT_KIRK && actorIndex <= OBJECT_REDSHIRT))
		error("Tried to walk a non PC");

	Actor *actor = &_vm->_actorList[actorIndex];
	Common::String anim = _vm->getCrewmanAnimFilename(actorIndex, "walk");
	bool success = _vm->actorWalkToPosition(actorIndex, anim, actor->pos.x, actor->pos.y, destX, destY);

	if (success && finishedAnimActionParam != 0) {
		actor->triggerActionWhenAnimFinished = true;
		actor->finishedAnimActionParam = finishedAnimActionParam;
	}
}

// Same as above, but with a function callback instead of an integer value to generate an
// action
void Room::walkCrewmanC(int actorIndex, int16 destX, int16 destY, void (Room::*funcPtr)()) {
	if (!(actorIndex >= OBJECT_KIRK && actorIndex <= OBJECT_REDSHIRT))
		error("Tried to walk a non PC");

	Actor *actor = &_vm->_actorList[actorIndex];
	Common::String anim = _vm->getCrewmanAnimFilename(actorIndex, "walk");
	bool success = _vm->actorWalkToPosition(actorIndex, anim, actor->pos.x, actor->pos.y, destX, destY);

	if (success && funcPtr != nullptr) {
		actor->triggerActionWhenAnimFinished = true;
		actor->finishedAnimActionParam = 0;
		actor->finishedAnimActionParam = findFunctionPointer(ACTION_FINISHED_WALKING, funcPtr);
	}
}

void Room::loadMapFile(const Common::String &name) {
	delete _vm->_mapFile;
	_vm->_mapFile = _vm->loadFile(name + ".map");

	_vm->_iwFile.reset();
	_vm->_iwFile = SharedPtr<IWFile>(new IWFile(_vm, name + ".iw"));
}

void Room::showBitmapFor5Ticks(const Common::String &bmpName, int priority) {
	if (priority < 0 || priority > 15)
		priority = 5;

	Sprite sprite;
	_vm->_gfx->addSprite(&sprite);
	sprite.setXYAndPriority(0, 0, priority);
	sprite.setBitmap(_vm->_gfx->loadBitmap(bmpName));

	_vm->_gfx->drawAllSprites();

	TrekEvent event;
	int ticks = 0;

	while (ticks < 5) {
		while (!_vm->popNextEvent(&event));

		if (event.type == TREKEVENT_TICK)
			ticks++;
	}

	sprite.dontDrawNextFrame();
	_vm->_gfx->drawAllSprites();
	_vm->_gfx->delSprite(&sprite);
}

bool Room::haveItem(int item) {
	return _vm->_itemList[item - 0x40].have;
}

Common::Point Room::getActorPos(int actorIndex) {
	return _vm->_actorList[actorIndex].pos;
}

int16 Room::getRandomWordInRange(int start, int end) {
	return _vm->getRandomWord() % (end - start + 1) + start;
}

void Room::playSoundEffectIndex(int soundEffect) {
	_vm->playSoundEffectIndex(soundEffect);
}

void Room::playMidiMusicTracks(int startTrack, int loopTrack) {
	_vm->playMidiMusicTracks(startTrack, loopTrack);
}

void Room::endMission(int16 score, int16 arg1, int16 arg2) {
	_vm->_awayMission.disableInput = true;

	for (int i = 0; i < (_vm->_awayMission.redshirtDead ? 3 : 4); i++) {
		Actor *actor = &_vm->_actorList[i];
		Common::String anim = _vm->getCrewmanAnimFilename(i, "teled");
		_vm->loadActorAnimWithRoomScaling(i, anim, actor->sprite.pos.x, actor->sprite.pos.y);
	}

	_vm->_kirkActor->animationString.clear();
	_vm->_spockActor->animationString.clear();
	_vm->_mccoyActor->animationString.clear();
	_vm->_redshirtActor->animationString.clear();

	playSoundEffectIndex(8);

	while (_vm->_kirkActor->spriteDrawn)
		_vm->handleAwayMissionEvents();

	_vm->_awayMission.disableInput = false;

	// TODO: This is a stopgap measure (loading the next away mission immediately).
	// Replace this with the proper code later.
	_vm->_gameMode = GAMEMODE_BEAMDOWN;

	const char *missionNames[] = {
		"DEMON",
		"TUG",
		"LOVE",
		"MUDD",
		"FEATHER",
		"TRIAL",
		"SINS",
		"VENG"
	};

	for (int i = 0; i < ARRAYSIZE(missionNames)-1; i++) {
		if (_vm->_missionName == missionNames[i]) {
			_vm->_missionToLoad = missionNames[i + 1];
			break;
		}
	}

	_vm->_roomIndexToLoad = 0;
}

void Room::showGameOverMenu() { // TODO: takes an optional parameter?
	_vm->showGameOverMenu();
	// TODO: finish. Shouldn't do this within a room due to deletion of current room?
}

int Room::showCodeInputBox(const char * const *codes) {
	Common::String inputString = _vm->showCodeInputBox();

	// ENHANCEMENT: Extra condition for "nothing entered"
	if (inputString.empty())
		return -1;

	int retval = 0;
	int code = 0;

	while (codes[code] != nullptr) {
		if (strcmp(codes[code], inputString.c_str()) == 0)
			retval = code + 1;
		code++;
	}

	return retval;
}

void Room::showRepublicMap(int16 arg0, int16 arg2) {
	_vm->showRepublicMap(arg0, arg2);
}

void Room::playVoc(Common::String filename) {
	_vm->_sound->playVoc(filename);
}

void Room::stopAllVocSounds() {
	_vm->_sound->stopAllVocSounds();
}

Common::String Room::getCrewmanAnimFilename(int object, const Common::String &str) {
	return _vm->getCrewmanAnimFilename(object, str);
}

void Room::spockScan(int direction, TextRef text, bool changeDirection, bool fromRDF) {
	const char *dirs = "nsew";
	Common::String anim = "sscan_";
	anim.setChar(dirs[direction], 5);

	if (changeDirection) // Check whether he should turn back to original direction after scanning
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = direction;

	loadActorAnim2(OBJECT_SPOCK, anim, -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);

	if (text != -1)
		showText(TX_SPEAKER_SPOCK, text, fromRDF);
}

void Room::mccoyScan(int direction, TextRef text, bool changeDirection, bool fromRDF) {
	const char *dirs = "nsew";
	Common::String anim = "mscan_";
	anim.setChar(dirs[direction], 5);

	if (changeDirection)
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = direction;

	loadActorAnim2(OBJECT_MCCOY, anim, -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);

	if (text != -1)
		showText(TX_SPEAKER_MCCOY, text, fromRDF);
}

} // End of namespace StarTrek
