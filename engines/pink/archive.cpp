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

#include "common/debug.h"
#include "common/file.h"

#include "pink/objects/condition.h"
#include "pink/objects/module.h"
#include "pink/objects/side_effect.h"
#include "pink/objects/actions/action_hide.h"
#include "pink/objects/actions/action_play_with_sfx.h"
#include "pink/objects/actions/action_sound.h"
#include "pink/objects/actions/action_talk.h"
#include "pink/objects/actions/action_text.h"
#include "pink/objects/actions/walk_action.h"
#include "pink/objects/actors/audio_info_pda_button.h"
#include "pink/objects/actors/cursor_actor.h"
#include "pink/objects/actors/inventory_actor.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/actors/pda_button_actor.h"
#include "pink/objects/actors/supporting_actor.h"
#include "pink/objects/handlers/handler.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/seq_timer.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/walk/walk_location.h"

namespace Pink {

static const struct RuntimeClass {
	const char *name;
	int id;
} classMap[] = {
	{"ActionHide", kActionHide},
	{"ActionLoop", kActionLoop},
	{"ActionPlay", kActionPlay},
	{"ActionPlayWithSfx", kActionPlayWithSfx},
	{"ActionSfx", kActionSfx},
	{"ActionSound", kActionSound},
	{"ActionStill", kActionStill},
	{"ActionTalk", kActionTalk},
	{"ActionText", kActionText},
	{"Actor", kActor},
	{"AudioInfoPDAButton", kAudioInfoPDAButton},
	{"ConditionGameVariable", kConditionGameVariable},
	{"ConditionInventoryItemOwner", kConditionInventoryItemOwner},
	{"ConditionModuleVariable", kConditionModuleVariable},
	{"ConditionNotInventoryItemOwner", kConditionNotInventoryItemOwner},
	{"ConditionNotModuleVariable", kConditionNotModuleVariable},
	{"ConditionNotPageVariable", kConditionNotPageVariable},
	{"ConditionPageVariable", kConditionPageVariable},
	{"CursorActor", kCursorActor},
	{"GamePage", kGamePage},
	{"HandlerLeftClick", kHandlerLeftClick},
	{"HandlerStartPage", kHandlerStartPage},
	{"HandlerTimer", kHandlerTimer},
	{"HandlerTimerActions", kHandlerTimerActions},
	{"HandlerTimerSequences", kHandlerTimerSequences},
	{"HandlerUseClick", kHandlerUseClick},
	{"InventoryActor", kInventoryActor},
	{"InventoryItem", kInventoryItem},
	{"LeadActor", kLeadActor},
	{"ModuleProxy", kModuleProxy},
	{"PDAButtonActor", kPDAButtonActor},
	{"ParlSqPink", kParlSqPink},
	{"PubPink", kPubPink},
	{"SeqTimer", kSeqTimer},
	{"Sequence", kSequence},
	{"SequenceAudio", kSequenceAudio},
	{"SequenceItem", kSequenceItem},
	{"SequenceItemDefaultAction", kSequenceItemDefaultAction},
	{"SequenceItemLeader", kSequenceItemLeader},
	{"SequenceItemLeaderAudio", kSequenceItemLeaderAudio},
	{"SideEffectExit", kSideEffectExit},
	{"SideEffectGameVariable", kSideEffectGameVariable},
	{"SideEffectInventoryItemOwner", kSideEffectInventoryItemOwner},
	{"SideEffectLocation", kSideEffectLocation},
	{"SideEffectModuleVariable", kSideEffectModuleVariable},
	{"SideEffectPageVariable", kSideEffectPageVariable},
	{"SideEffectRandomPageVariable", kSideEffectRandomPageVariable},
	{"SupportingActor", kSupportingActor},
	{"WalkAction", kWalkAction},
	{"WalkLocation", kWalkLocation}
};

static Object *createObject(int objectId) {
	switch (objectId) {
	case kActionHide:
		return new ActionHide;
	case kActionLoop:
		return new ActionLoop;
	case kActionPlay:
		return new ActionPlay;
	case kActionPlayWithSfx:
		return new ActionPlayWithSfx;
	case kActionSfx:
		return new ActionSfx;
	case kActionSound:
		return new ActionSound;
	case kActionStill:
		return new ActionStill;
	case kActionTalk:
		return new ActionTalk;
	case kActionText:
		return new ActionText;
	case kActor:
		return new Actor;
	case kAudioInfoPDAButton:
		return new AudioInfoPDAButton;
	case kConditionGameVariable:
		return new ConditionGameVariable;
	case kConditionInventoryItemOwner:
		return new ConditionInventoryItemOwner;
	case kConditionModuleVariable:
		return new ConditionModuleVariable;
	case kConditionNotInventoryItemOwner:
		return new ConditionNotInventoryItemOwner;
	case kConditionNotModuleVariable:
		return new ConditionNotModuleVariable;
	case kConditionNotPageVariable:
		return new ConditionNotPageVariable;
	case kConditionPageVariable:
		return new ConditionPageVariable;
	case kCursorActor:
		return new CursorActor;
	case kGamePage:
		return new GamePage;
	case kHandlerLeftClick:
		return new HandlerLeftClick;
	case kHandlerStartPage:
		return new HandlerStartPage;
	case kHandlerTimer:
	case kHandlerTimerActions:
		return new HandlerTimerActions; // hack for Peril, but behavior is correct
	case kHandlerTimerSequences:
		return new HandlerTimerSequences;
	case kHandlerUseClick:
		return new HandlerUseClick;
	case kInventoryActor:
		return new InventoryActor;
	case kInventoryItem:
		return new InventoryItem;
	case kLeadActor:
		return new LeadActor;
	case kModuleProxy:
		return new ModuleProxy;
	case kPDAButtonActor:
		return new PDAButtonActor;
	case kParlSqPink:
		return new ParlSqPink;
	case kPubPink:
		return new PubPink;
	case kSeqTimer:
		return new SeqTimer;
	case kSequence:
		return new Sequence;
	case kSequenceAudio:
		return new SequenceAudio;
	case kSequenceItem:
		return new SequenceItem;
	case kSequenceItemDefaultAction:
		return new SequenceItemDefaultAction;
	case kSequenceItemLeader:
		return new SequenceItemLeader;
	case kSequenceItemLeaderAudio:
		return new SequenceItemLeaderAudio;
	case kSideEffectExit:
		return new SideEffectExit;
	case kSideEffectGameVariable:
		return new SideEffectGameVariable;
	case kSideEffectInventoryItemOwner:
		return new SideEffectInventoryItemOwner;
	case kSideEffectLocation:
		return new SideEffectLocation;
	case kSideEffectModuleVariable:
		return new SideEffectModuleVariable;
	case kSideEffectPageVariable:
		return new SideEffectPageVariable;
	case kSideEffectRandomPageVariable:
		return new SideEffectRandomPageVariable;
	case kSupportingActor:
		return new SupportingActor;
	case kWalkAction:
		return new WalkAction;
	case kWalkLocation:
		return new WalkLocation;
	default:
		error("Unknown object id");
		return nullptr;
	}
}

Archive::Archive(Common::SeekableReadStream  *stream)
	: _readStream(stream), _writeStream(nullptr) {
	_objectMap.push_back(0);
	_objectIdMap.push_back(kNullObject);
}

Archive::Archive(Common::WriteStream *stream)
	: _writeStream(stream), _readStream(nullptr) {
	_objectMap.push_back(0);
	_objectIdMap.push_back(kNullObject);
}

void Archive::mapObject(Object *obj) {
	_objectMap.push_back(obj);
	_objectIdMap.push_back(0);
}

Object *Archive::readObject() {
	bool isCopyReturned;
	Object *res = parseObject(isCopyReturned);

	if (res && !isCopyReturned)
		res->deserialize(*this);

	return res;
}

Object *Archive::parseObject(bool &isCopyReturned) {
	char className[kMaxClassLength];
	int objectId = 0;
	Object *res = nullptr;

	uint obTag = _readStream->readUint16LE();

	if (obTag == 0x0000) {
		return nullptr;
	} else if (obTag == 0xffff) {
		/* int schema = */_readStream->readUint16LE();

		int size = _readStream->readUint16LE();
		_readStream->read(className, size);
		className[size] = '\0';

		objectId = findObjectId(className + 1);

		res = createObject(objectId);
		if (!res) error("Class %s is not implemented", className);

		_objectMap.push_back(res);
		_objectIdMap.push_back(objectId);

		_objectMap.push_back(res); // Basically a hack, but behavior is all correct. MFC uses one array for pointers and ids
		_objectIdMap.push_back(objectId);

		isCopyReturned = false;
	} else if ((obTag & 0x8000) == 0) {
		res = _objectMap[obTag];

		isCopyReturned = true;
	} else {
		obTag &= ~0x8000;

		objectId = _objectIdMap[obTag];

		res = createObject(objectId);
		_objectMap.push_back(res);
		_objectIdMap.push_back(objectId);

		isCopyReturned = false;
	}

	return res;
}

static int runtimeClassCmp(const void *key, const void *elem) {
	return strcmp((const char *)key, *(const char * const *)elem);
}

uint Archive::findObjectId(const char *name) {
	RuntimeClass *found = (RuntimeClass *)bsearch(name, classMap, sizeof(classMap) / sizeof(RuntimeClass), sizeof(RuntimeClass), runtimeClassCmp);

	if (!found)
		error("Class %s is not in class Map", name);

	return found->id;
}

Common::String Archive::readString() {
	char buffer[kMaxStringLength];
	byte len = _readStream->readByte();
	assert(len <= kMaxStringLength);
	_readStream->read(buffer, len);
	return Common::String(buffer, len);
}

void Archive::skipString() {
	byte len = _readStream->readByte();
	_readStream->skip(len);
}

void Archive::writeString(const Common::String &string) {
	_writeStream->writeByte(string.size());
	_writeStream->write(string.c_str(), string.size());
}

} // End of namespace Pink
