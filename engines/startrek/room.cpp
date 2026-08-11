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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "startrek/bridge.h"
#include "startrek/iwfile.h"
#include "startrek/resource.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

#include "rooms/function_map.h"

#define ADD_ROOM(ROOM) {                     \
		if (name.equalsIgnoreCase(#ROOM)) {      \
			_roomActionList = ROOM##ActionList;  \
			_roomTextList = ROOM##TextOffsets;   \
			_roomStaticTextList = ROOM##Texts;   \
		}                                        \
	}

#define ADD_ROOM_COMMON(PREFIX) {                        \
		if (name.hasPrefixIgnoreCase(#PREFIX)) {         \
			_roomCommonTextList = PREFIX##TextOffsets;   \
			_commonTextRdf = #PREFIX;                    \
		}                                                \
	}

namespace StarTrek {

Room::Room(StarTrekEngine *vm, const Common::String &name) : _vm(vm), _awayMission(&vm->_awayMission) {
	_roomActionList = nullptr;
	_roomTextList = nullptr;
	_roomCommonTextList = nullptr;
	_roomStaticTextList = nullptr;
	_commonTextRdf = "";

	ADD_ROOM(demon0)
	ADD_ROOM(demon1)
	ADD_ROOM(demon2)
	ADD_ROOM(demon3)
	ADD_ROOM(demon4)
	ADD_ROOM(demon5)
	ADD_ROOM(demon6)
	ADD_ROOM(tug0)
	ADD_ROOM(tug1)
	ADD_ROOM(tug2)
	ADD_ROOM(tug3)
	ADD_ROOM(love0)
	ADD_ROOM(love1)
	ADD_ROOM(love2)
	ADD_ROOM(love3)
	ADD_ROOM(love4)
	ADD_ROOM(love5)
	ADD_ROOM_COMMON(love)
	ADD_ROOM(mudd0)
	ADD_ROOM(mudd1)
	ADD_ROOM(mudd2)
	ADD_ROOM(mudd3)
	ADD_ROOM(mudd4)
	ADD_ROOM(mudd5)
	ADD_ROOM_COMMON(mudd)
	ADD_ROOM(feather0)
	ADD_ROOM(feather1)
	ADD_ROOM(feather2)
	ADD_ROOM(feather3)
	ADD_ROOM(feather4)
	ADD_ROOM(feather5)
	ADD_ROOM(feather6)
	ADD_ROOM(feather7)
	ADD_ROOM(trial0)
	ADD_ROOM(trial1)
	ADD_ROOM(trial2)
	ADD_ROOM(trial3)
	ADD_ROOM(trial4)
	ADD_ROOM(trial5)
	ADD_ROOM(sins0)
	ADD_ROOM(sins1)
	ADD_ROOM(sins2)
	ADD_ROOM(sins3)
	ADD_ROOM(sins4)
	ADD_ROOM(sins5)
	ADD_ROOM(veng0)
	ADD_ROOM(veng1)
	ADD_ROOM(veng2)
	ADD_ROOM(veng3)
	ADD_ROOM(veng4)
	ADD_ROOM(veng5)
	ADD_ROOM(veng6)
	ADD_ROOM(veng7)
	ADD_ROOM(veng8)
	ADD_ROOM_COMMON(veng)

	if (_roomActionList == nullptr) {
		warning("Room \"%s\" unimplemented", name.c_str());
	}

	_rdfData = loadRoomRDF(name);
	_commonRdfData = !_commonTextRdf.empty() ? loadRoomRDF(_commonTextRdf + "0") : nullptr;

	memset(&_roomVar, 0, sizeof(_roomVar));
}

Room::~Room() {
	delete[] _rdfData;
	delete[] _commonRdfData;
}

byte *Room::loadRoomRDF(Common::String fileName) {
	Common::MemoryReadStreamEndian *rdfFile = _vm->_resource->loadFile(fileName + ".RDF");

	int size = rdfFile->size();
	byte *buffer = new byte[size];
	//_rdfSize = size;
	rdfFile->read(buffer, size);
	delete rdfFile;

	return buffer;
}

uint16 Room::readRdfWord(int offset) {
	return READ_LE_UINT16((_rdfData + offset));
}

bool Room::actionHasCode(const Action &action) {
	const RoomAction *roomActionPtr = _roomActionList;

	while (roomActionPtr->action.type != ACTION_LIST_END) {
		if (action == roomActionPtr->action)
			return true;
		roomActionPtr++;
	}
	return false;
}

bool Room::actionHasCode(int8 type, byte b1, byte b2, byte b3) {
	const Action a = {type, b1, b2, b3, 0};
	return actionHasCode(a);
}

bool Room::handleAction(const Action &action) {
	const RoomAction *roomActionPtr = _roomActionList;

	while (roomActionPtr->action.type != ACTION_LIST_END) {
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

bool Room::handleAction(int8 type, byte b1, byte b2, byte b3) {
	const Action a = {type, b1, b2, b3, 0};
	return handleAction(a);
}

bool Room::handleActionWithBitmask(const Action &action) {
	const RoomAction *roomActionPtr = _roomActionList;

	while (roomActionPtr->action.type != ACTION_LIST_END) {
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

bool Room::handleActionWithBitmask(int8 type, byte b1, byte b2, byte b3) {
	Action a = {type, b1, b2, b3, 0};
	return handleActionWithBitmask(a);
}

Common::Point Room::getBeamInPosition(int crewmanIndex) {
	bool isDemo = _vm->getFeatures() & GF_DEMO;
	if (!isDemo) {
		int base = RDF_BEAM_IN_POSITIONS + crewmanIndex * 4;
		return Common::Point(readRdfWord(base), readRdfWord(base + 2));
	} else {
		// TODO
		return Common::Point(86, 158);
	}
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
	const RoomAction *roomActionPtr = _roomActionList;

	while (roomActionPtr->action.type != ACTION_LIST_END) {
		if (roomActionPtr->action.type == action && roomActionPtr->funcPtr == funcPtr)
			return roomActionPtr->action.b1;
		roomActionPtr++;
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

const char *Room::getText(uint16 textId) {
	uint16 offset;
	bool isCD = _vm->getFeatures() & GF_CDROM;
	int index = 0;
	const RoomTextOffsets *textList = textId < 5000 ? _roomTextList : _roomCommonTextList;
	const RoomTextOffsets *offsets = nullptr;

	// Check for text indices that point inside the RDF file
	do {
		if (textList[index].id == textId) {
			offsets = &textList[index];
			break;
		}
		index++;
	} while (textList[index].id != -1);

	// Check if we got a hardcoded text
	index = 0;
	if (offsets == nullptr && _roomStaticTextList != nullptr) {
		do {
			if (_roomStaticTextList[index].id == textId) {
				// TODO: Add non-English languages
				return _roomStaticTextList[index].text;
			}
			index++;
		} while (_roomStaticTextList[index].id != -1);
	}

	if (offsets == nullptr)
		error("Missing text ID: %d", textId);

	switch (_vm->getLanguage()) {
	case Common::FR_FRA:
		offset = offsets->offsetFrenchCD;
		break;
	case Common::DE_DEU:
		offset = offsets->offsetGermanCD;
		break;
	default:
		offset = isCD ? offsets->offsetEnglishCD : offsets->offsetEnglishFloppy;
		break;
	}

	const char *text = textId < 5000 ? (const char *)_rdfData + offset : (const char *)_commonRdfData + offset;
	//return patchRoomMessage(text).c_str();	// TODO
	return text;
}

int Room::showMultipleTexts(const TextRef *textIDs) {
	int numIDs = 0;
	int retval;
	while (textIDs[numIDs] != TX_END)
		numIDs++;

	const char **text = (const char **)malloc(sizeof(const char *) * (numIDs + 1));

	for (int i = 0; i < numIDs; i++) {
		text[i] = textIDs[i] == TX_EMPTY ? "" : getText(textIDs[i]);
	}

	text[numIDs] = "";	// terminator

	retval = showRoomSpecificText(text);
	free(text);

	return retval;
}

int Room::showText(TextRef speaker, TextRef text) {
	TextRef textIDs[3];
	textIDs[0] = speaker;
	textIDs[1] = text;
	textIDs[2] = TX_END;
	return showMultipleTexts(textIDs);
}

int Room::showDescription(TextRef text) {
	return showText(TX_EMPTY, text);
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
	_vm->_mapFile = _vm->_resource->loadFile(name + ".map");

	delete _vm->_iwFile;
	_vm->_iwFile = new IWFile(_vm, name + ".iw");
}

Common::MemoryReadStreamEndian *Room::loadBitmapFile(Common::String baseName) {
	return _vm->_resource->loadBitmapFile(baseName);
}

Common::MemoryReadStreamEndian *Room::loadFileWithParams(Common::String filename, bool unk1, bool unk2, bool unk3) {
	return _vm->_resource->loadFileWithParams(filename, unk1, unk2, unk3);
}

void Room::showBitmapFor5Ticks(const Common::String &bmpName, int priority) {
	if (priority < 0 || priority > 15)
		priority = 5;

	Sprite sprite;
	_vm->_gfx->addSprite(&sprite);
	sprite.setXYAndPriority(0, 0, priority);
	sprite.setBitmap(_vm->_resource->loadBitmapFile(bmpName));

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

void Room::playSoundEffectIndex(SoundEffects soundEffect) {
	_vm->_sound->playSoundEffectIndex(soundEffect);
}

void Room::playMidiMusicTracks(MidiTracks startTrack, MidiLoopType loopType) {
	_vm->_sound->playMidiMusicTracks(startTrack, loopType);
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

	playSoundEffectIndex(kSfxTransporterDematerialize);

	while (_vm->_kirkActor->spriteDrawn)
		_vm->handleAwayMissionEvents();

	_vm->_awayMission.disableInput = false;
	_vm->_roomIndexToLoad = 0;
	_vm->_gameMode = GAMEMODE_BEAMUP;

	if (_vm->_missionName == "DEMON") {
		_vm->_bridgeSequenceToLoad = kSeqEndMissionDemon;
	} else if (_vm->_missionName == "TUG") {
		_vm->_bridgeSequenceToLoad = kSeqEndMissionTug;
	} else if (_vm->_missionName == "LOVE") {
		_vm->_bridgeSequenceToLoad = kSeqEndMissionLove;
	} else if (_vm->_missionName == "MUDD") {
		_vm->_bridgeSequenceToLoad = kSeqEndMissionMudd;
	} else if (_vm->_missionName == "FEATHER") {
		_vm->_bridgeSequenceToLoad = kSeqEndMissionFeather;
	} else if (_vm->_missionName == "TRIAL") {
		_vm->_bridgeSequenceToLoad = kSeqEndMissionTrial;
	} else if (_vm->_missionName == "SINS") {
		_vm->_bridgeSequenceToLoad = kSeqEndMissionSins;
	}
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

void Room::spockScan(int direction, TextRef speaker, TextRef text, bool changeDirection) {
	const char *dirs = "nsew";
	Common::String anim = "sscan_";
	anim.setChar(dirs[direction], 5);

	if (changeDirection) // Check whether he should turn back to original direction after scanning
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = direction;

	loadActorAnim2(OBJECT_SPOCK, anim, -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);

	if (text != -1)
		showText(speaker, text);
}

void Room::mccoyScan(int direction, TextRef speaker, TextRef text, bool changeDirection) {
	const char *dirs = "nsew";
	Common::String anim = "mscan_";
	anim.setChar(dirs[direction], 5);

	if (changeDirection)
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = direction;

	loadActorAnim2(OBJECT_MCCOY, anim, -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);

	if (text != -1)
		showText(speaker, text);
}

bool Room::isPointInPolygon(int offset, int16 x, int16 y) {
	int16 *data = (int16 *)(_rdfData + offset);
	int16 numVertices = data[1];
	int16 *vertData = &data[2];

	for (int i = 0; i < numVertices; i++) {
		Common::Point p1(vertData[0], vertData[1]);
		Common::Point p2;
		if (i == numVertices - 1) // Loop to 1st vertex
			p2 = Common::Point(data[2], data[3]);
		else
			p2 = Common::Point(vertData[2], vertData[3]);

		if ((p2.x - p1.x) * (y - p1.y) - (p2.y - p1.y) * (x - p1.x) < 0)
			return false;

		vertData += 2;
	}

	return true;
}

} // End of namespace StarTrek
