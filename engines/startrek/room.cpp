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
#include "startrek/resource.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

#include "rooms/function_map.h"

#define ADD_ROOM(ROOM) {\
		if (name.equalsIgnoreCase(#ROOM)) {\
			_roomActionList = ROOM##ActionList;\
		}\
	}

namespace StarTrek {

Room::Room(StarTrekEngine *vm, const Common::String &name) : _vm(vm), _awayMission(&vm->_awayMission) {
	Common::MemoryReadStreamEndian *rdfFile = _vm->_resource->loadFile(name + ".RDF");

	int size = rdfFile->size();
	_rdfData = new byte[size];
	_rdfSize = size;
	rdfFile->read(_rdfData, size);
	delete rdfFile;

	_roomActionList = nullptr;

	ADD_ROOM(demon0);
	ADD_ROOM(demon1);
	ADD_ROOM(demon2);
	ADD_ROOM(demon3);
	ADD_ROOM(demon4);
	ADD_ROOM(demon5);
	ADD_ROOM(demon6);
	ADD_ROOM(tug0);
	ADD_ROOM(tug1);
	ADD_ROOM(tug2);
	ADD_ROOM(tug3);
	ADD_ROOM(love0);
	ADD_ROOM(love1);
	ADD_ROOM(love2);
	ADD_ROOM(love3);
	ADD_ROOM(love4);
	ADD_ROOM(love5);
	ADD_ROOM(mudd0);
	ADD_ROOM(mudd1);
	ADD_ROOM(mudd2);
	ADD_ROOM(mudd3);
	ADD_ROOM(mudd4);
	ADD_ROOM(mudd5);
	ADD_ROOM(feather0);
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
	}

	bool isDemo = _vm->getFeatures() & GF_DEMO;
	bool isFloppy = !(_vm->getFeatures() & GF_CDROM);
	if (!isDemo && !isFloppy) {
		loadRoomMessages();
		loadOtherRoomMessages();
	}
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
	uint16 offset = messagesOffset;
	const char *text = (const char *)_rdfData + messagesOffset;
	const char roomIndexChar = '0' + _vm->_roomIndex;

	do {
		while ((text[0] != '#' || (text[1] != _vm->_missionName[0] && text[4] != roomIndexChar)) && offset < _rdfSize) {
			text++;
			offset++;
		}

		if (text[5] == '\\')
			loadRoomMessage(text);

		while (*text != '\0' && offset < _rdfSize) {
			text++;
			offset++;
		}

		// Peek the next byte, in case there's a filler text
		if (Common::isAlpha(*(text + 1))) {
			while (*text != '\0' && offset < _rdfSize) {
				text++;
				offset++;
			}
		}
	} while (*(text + 1) == '#' && offset < _rdfSize);
}

void Room::loadRoomMessage(const char *text) {
	int messageNum;
	bool isTalkMessage;
	bool isLookMessage;
	bool isLookWithTalkerMessage;

	Common::String patchedText = patchRoomMessage(text);
	text = patchedText.c_str();

	char textType = text[10];	// _, U and S: talk message, N: look message, L: look with talker message
	char numberType = text[11];	// Sxx: Scotty

	if (text[5] != '\\')
		error("loadRoomMessage: Invalid message");

	isTalkMessage = (textType == '_' || textType == 'U' || numberType == 'S' || numberType == 'F');	// U = Uhura, S = Scotty, F = followup
	isLookMessage = (textType == 'N');
	isLookWithTalkerMessage = (textType == 'L');

	sscanf((const char *)(text + 11), "%3d", &messageNum);
	if (text[14] != '#')
		error("loadRoomMessage: Invalid message");

	if (memcmp(text + 1, _vm->_missionName.c_str(), 3) || text[4] != _vm->_roomIndex + '0') {
		// String with a prefix of another mission, separate it
		messageNum += COMMON_MESSAGE_OFFSET;
	} else if (numberType == 'S') {
		// For some reason, Uhura's messages (Uxx) follow the same numbering as the
		// rest, but Scott's don't, and start from one.
		messageNum += SCOTTY_MESSAGE_OFFSET;
	} else if (numberType == 'F') {
		messageNum += FOLLOWUP_MESSAGE_OFFSET;
	}

	if (isTalkMessage)
		_talkMessages[messageNum] = text;
	else if (isLookMessage)
		_lookMessages[messageNum] = text;
	else if (isLookWithTalkerMessage)
		_lookWithTalkerMessages[messageNum] = text;
}

struct TypoFix {
	Common::String prefix;
	Common::String origText;
	Common::String newText;
};

Common::String Room::patchRoomMessage(const char *text) {
	Common::String txt = text;
	int i = 0;

	TypoFix typoFixes[] = {
		{ "#LOV2\\LOV2_012#", "#LOV2\\LOV2_012#", "#LOV1\\LOV1_010#" },	// Audio file missing
		{ "#LOV3\\LOV3_#",    "#LOV3\\LOV3_#", "#LOV3\\LOV3_000#"},	// Message index missing
		{ "#LOVA\\LOVA_F08#", "spock", "Spock" },
		{ "#LOVA\\LOVA_F55#", "sysnthesize", "synthesize" },
		{ "#FEA3\\FEA3_030#", "#FEA3\\FEA3_030#", "#LOVA\\LOVA_100#" },	// Wrong voice actor
		{ "#MUD0\\MUD0_023#", "gullability", "gullibility" },
		{ "#MUD2\\MUD2_002#", "Well, now! I think", "Well, now I think" },
		{ "#MUD2\\MUD2_014#", "I don't understand enough of the alien's thinking", "I don't understand enough of how the aliens thought," },
		{ "#MUD3\\MUD3_011#", "to think after all the stunts that Harry has pulled", "to think that after all the stunts that Harry has pulled," },
		{ "#MUD3\\MUD3_022#", "and they were certain", "and they are certain" },
		{ "#MUD3\\MUD4_008#", "DId you know", "Did you know" },
		{ "#FEA1\\FEA1_035#", "before it retreats Captain", "before it retreats, Captain" },
		{ "#FEA1\\FEA1_041#", "it must have a nasty bite", "it may have a nasty bite" },
		{ "#FEA3\\FEA3_012#", "he'll be up in about an hour", "he'll be up in about a half hour" },
		{ "#FEA3\\FEA3_030#", "sHe's dead, Jim!", "He's dead, Jim!" },
		{ "#FEA5\\FEA5_009#", "those thorns.You might", "those thorns. You might" },
		{ "#FEA5\\FEA5_018#", "with our phaser not working", "with our phasers not working" },
		{ "#FEA5\\FEA5_020#", "in a previous life", "in your previous life" },
		{ "#FEA6\\FEA6_017#", "isn't that just great", "isn't this just great" },
		{ "#FEA6\\FEA6_019#", "that action, Captain It may", "that action, Captain. It may" },
		{ "#FEA6\\FEA6N016#", "that attack you", "that attacked you" },
		{ "#SIN2\\SIN2_012#", "I'm a surgeon not a alien", "I'm a surgeon, not an alien" },
		{ "#SIN4\\SIN4_023#", "to bypass it's lock system", "to bypass its lock system" },
		{ "#SIN5\\SIN5N012#", "Sparks explode and", "Sparks fly and" },
		{ "#TRI0\\TRI0_036#", "the Enterprise!We've", "the Enterprise! We've" },
		{ "#TRI1\\TRI1_025#", "Male Human-Vulcan", "One male Human-Vulcan" },
		{ "#TRI1\\TRI1_048#", "with a phaser", "with your phaser" },
		{ "#TRI2\\TRI2_015#", "Male Human,", "He's a male Human," },
		{ "#TRI2\\TRI2_017#", "Male Human-Vulcan", "One male Human-Vulcan" },
		{ "#TRI3\\TRI3_013#", "He's a Male Human", "One male Human" },
		{ "#TRI3\\TRI3_014#", "Male Human,", "He's a male Human," },
		{ "#TRI3\\TRI3_016#", "Male Human-Vulcan", "One male Human-Vulcan" },
		{ "#TRI3\\TRI3U084#", "Captain, come in please!", "Captain, please come in!" },
		{ "#TRI4\\TRI4_003#", "I didn't want it", "I don't want it" },
		{ "#TRI4\\TRI4_024#", "a fair trail", "a fair trial" },
		{ "#TRI4\\TRI4_039#", "what an enemy does not expect", "what the enemy does not expect" },
		{ "#TRI4\\TRI4_057#", "will believe you", "to believe you" },
		{ "#TRI5\\TRI5_045#", "at which to transport you", "to which to transport you" },
		{ "#TRI5\\TRI5N002#", "a beam light", "a beam of light" },
		{ "#TRI5\\TRI5N016#", "saphire", "sapphire" },
		{ "#TRI5\\TRI5N017#", "saphire", "sapphire" },
		{ "#TRI5\\TRI5N018#", "saphire", "sapphire" },
		{ "#TRI5\\TRI5N019#", "a emerald", "an emerald" },
		{ "#TRI5\\TRI5N020#", "a emerald", "an emerald" },
		{ "#TRI5\\TRI5N021#", "a emerald", "an emerald" },
		{ "#VEN2\\VEN2_050#", "torpedo is loaded", "torpedoes are loaded" },
		{ "#VEN6\\VEN6_005#", "><upon", "upon" },
		{ "#VEN8\\VEN8_037#", "Its not", "It's not" },
		{ "", "", "" }
	};

	// Fix typos where some messages contain a hyphen instead of an underscore
	// (e.g in LOV2)
	if (txt[10] == '-')
		txt.replace(10, 1, "_");

	// Fix typos where some messages contain double spacing (e.g. in FEA3_020)
	int32 spacePos = txt.find("  ");
	if (spacePos > 0)
		txt.deleteChar(spacePos);

	// Fix typos
	do {
		const Common::String origText = typoFixes[i].origText;
		const Common::String newText = typoFixes[i].newText;

		int32 pos = txt.find(origText);
		if (pos > 0)
			txt.replace(pos, origText.size(), newText, pos, newText.size());

		i++;
	} while (typoFixes[i].prefix != "");

	return txt;
}

void Room::loadOtherRoomMessages() {
	uint16 startOffset = readRdfWord(14);
	// Some RDF files, lile MUDD0, contain text beyond the end offset,
	// so we read up to the end of the file
	uint16 endOffset = _rdfSize;	// readRdfWord(16);
	uint16 offset = startOffset;
	const char *validPrefixes[] = {
		"BRI", "COM", "DEM", "FEA", "GEN", "LOV", "MUD", "SIN", "TRI", "TUG", "VEN"
	};

	while (offset < endOffset) {
		uint16 nextOffset = readRdfWord(offset + 4);
		if (nextOffset >= endOffset || offset >= nextOffset)
			break;
		
		while (offset < nextOffset) {
			const char *text = (const char *)_rdfData + offset;

			if (text[0] == '#' && text[5] == '\\') {
				for (uint i = 0; i < ARRAYSIZE(validPrefixes); i++) {
					if (!memcmp(text + 1, validPrefixes[i], 3)) {
						loadRoomMessage(text);
						break;
					}
				}
			}

			offset++;
		}
	}
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
	const Action a = {type, b1, b2, b3};
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
	const Action a = {type, b1, b2, b3};
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
	Action a = {type, b1, b2, b3};
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
				text[i] = _lookMessages.contains(textIDs[i]) ? _lookMessages[textIDs[i]].c_str() : _lookMessages[textIDs[i] - COMMON_MESSAGE_OFFSET].c_str();
			else if (lookWithTalker)
				text[i] = _lookWithTalkerMessages.contains(textIDs[i]) ? _lookWithTalkerMessages[textIDs[i]].c_str() : _lookWithTalkerMessages[textIDs[i] - COMMON_MESSAGE_OFFSET].c_str();
			else
				text[i] = _talkMessages.contains(textIDs[i]) ? _talkMessages[textIDs[i]].c_str() : _talkMessages[textIDs[i] - COMMON_MESSAGE_OFFSET].c_str();
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

void Room::playSoundEffectIndex(int soundEffect) {
	_vm->_sound->playSoundEffectIndex(soundEffect);
}

void Room::playMidiMusicTracks(int startTrack, int loopTrack) {
	_vm->_sound->playMidiMusicTracks(startTrack, loopTrack);
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

	if (_vm->_missionName == "DEMON") {
		_vm->_gameMode = GAMEMODE_BEAMUP;
		_vm->_roomIndexToLoad = 0;
		_vm->_bridgeSequenceToLoad = 4; // kSeqEndMissionDemon
	} else if (_vm->_missionName == "TUG") {
		_vm->_gameMode = GAMEMODE_BEAMUP;
		_vm->_roomIndexToLoad = 0;
		_vm->_bridgeSequenceToLoad = 9; // kSeqEndMissionTug
	} else if (_vm->_missionName == "LOVE") {
		_vm->_gameMode = GAMEMODE_BEAMUP;
		_vm->_roomIndexToLoad = 0;
		_vm->_bridgeSequenceToLoad = 15; // kSeqEndMissionLove
	} else if (_vm->_missionName == "MUDD") {
		_vm->_gameMode = GAMEMODE_BEAMUP;
		_vm->_roomIndexToLoad = 0;
		_vm->_bridgeSequenceToLoad = 18; // kSeqEndMissionMudd
	} else {
		// TODO: This is a stopgap measure (loading the next away mission immediately).
		// Replace this with the proper code later.
		_vm->_gameMode = GAMEMODE_BEAMDOWN;
		_vm->_roomIndexToLoad = 0;

		const char *missionNames[] = {
		    //"DEMON",
		    //"TUG",
		    //"LOVE",
		    //"MUDD",
		    "FEATHER",
		    "TRIAL",
		    "SINS",
		    "VENG"};

		for (int i = 0; i < ARRAYSIZE(missionNames) - 1; i++) {
			if (_vm->_missionName == missionNames[i]) {
				_vm->_missionToLoad = missionNames[i + 1];
				break;
			}
		}
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

void Room::spockScan(int direction, TextRef text, bool changeDirection, bool fromRDF) {
	const char *dirs = "nsew";
	Common::String anim = "sscan_";
	anim.setChar(dirs[direction], 5);

	if (changeDirection) // Check whether he should turn back to original direction after scanning
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = direction;

	loadActorAnim2(OBJECT_SPOCK, anim, -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);

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
	playSoundEffectIndex(kSfxTricorder);

	if (text != -1)
		showText(TX_SPEAKER_MCCOY, text, fromRDF);
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
