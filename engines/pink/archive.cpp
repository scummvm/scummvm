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

#include <common/debug.h>
#include <common/file.h>
#include <engines/pink/objects/object.h>
#include <engines/pink/objects/module.h>
#include <engines/pink/objects/pages/game_page.h>
#include <engines/pink/objects/actors/lead_actor.h>
#include <engines/pink/objects/condition.h>
#include <engines/pink/objects/side_effect.h>
#include <engines/pink/objects/sequences/sequence_item.h>
#include <engines/pink/objects/sequences/sequence.h>
#include <engines/pink/objects/handlers/handler.h>
#include <engines/pink/objects/actions/action_play.h>
#include <engines/pink/objects/actions/action_sound.h>
#include <engines/pink/objects/actions/action_hide.h>
#include <engines/pink/objects/walk/walk_location.h>

namespace Pink {

enum {
    kMaxClassLength = 32,
    kMaxStringLength = 64, // adjust
    kNullObject = 0
};

enum {
    kActionHide,
    kActionLoop,
    kActionPlay,
    kActionPlayWithSfx,
    kActionSfx,
    kActionSound,
    kActionStill,
    kActionTalk,
    kActionText,
    kActor,
    kAudioInfoPDAButton,
    kConditionGameVariable,
    kConditionInventoryItemOwner,
    kConditionModuleVariable,
    kConditionNotInventoryItemOwner,
    kConditionNotModuleVariable,
    kConditionNotPageVariable,
    kConditionPageVariable,
    kCursorActor,
    kGamePage,
    kHandlerLeftClick,
    kHandlerStartPage,
    kHandlerTimer,
    kHandlerUseClick,
    kInventoryActor,
    kInventoryItem,
    kLeadActor,
    kModuleProxy,
    kPDAButtonActor,
    kParlSqPink,
    kPubPink,
    kSeqTimer,
    kSequence,
    kSequenceAudio,
    kSequenceItem,
    kSequenceItemDefaultAction,
    kSequenceItemLeader,
    kSequenceItemLeaderAudio,
    kSideEffectExit,
    kSideEffectGameVariable,
    kSideEffectInventoryItemOwner,
    kSideEffectLocation,
    kSideEffectModuleVariable,
    kSideEffectPageVariable,
    kSideEffectRandomPageVariable,
    kSupportingActor,
    kWalkAction,
    kWalkLocation
};

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

static Object* createObject(int objectId){
    switch (objectId){
        case kActionHide:
            return new ActionHide;
        case kActionPlay:
            return new ActionPlay;
        case kActionSound:
            return new ActionSound;
        case kActionStill:
            return new ActionStill;
        case kActor:
            return new Actor;
        case kGamePage:
            return new GamePage;
        case kHandlerStartPage:
            return new HandlerStartPage;
        case kInventoryItem:
            return new InventoryItem;
        case kLeadActor:
            return new LeadActor;
        case kModuleProxy:
            return new ModuleProxy;
        case kSequence:
            return new Sequence;
        case kSequenceItem:
            return new SequenceItem;
        case kSequenceItemDefaultAction:
            return new SequenceItemDefaultAction;
        case kSequenceItemLeader:
            return new SequenceItemLeader;
        case kSideEffectExit:
            return new SideEffectExit;
        case kSideEffectModuleVariable:
            return new SideEffectModuleVariable;
        case kWalkLocation:
            return new WalkLocation;
        default:
            return nullptr;
    }
}

Archive::Archive(Common::File &file)
    : _file(file)
{
    _objectMap.push_back(0);
    _objectIdMap.push_back(kNullObject);
}

Archive::~Archive()
{}

void Archive::mapObject(Object *obj) {
    _objectMap.push_back(obj);
    _objectIdMap.push_back(0);
}

int Archive::readCount() {
    int count = _file.readUint16LE();

    if (count == 0xffff)
        count = _file.readUint32LE();

    return count;
}

Object *Archive::readObject() {
    bool isCopyReturned;
    Object *res = parseObject(isCopyReturned);

    if (res && !isCopyReturned) {
        res->deserialize(*this);
    }

    return res;
}

Object *Archive::parseObject(bool &isCopyReturned) {
    char className[kMaxClassLength];
    int objectId = 0;
    Object *res = 0;

    uint obTag = _file.readUint16LE();

    if (obTag == 0x0000) {
        return nullptr;
    } else if (obTag == 0xffff) {
        int schema = _file.readUint16LE();

        int size = _file.readUint16LE();
        _file.read(className, size);
        className[size] = '\0';

        objectId = findObjectId(className + 1);

        res = createObject(objectId);
        if (!res) error("Class %s is not implemented", className);

        _objectMap.push_back(res);
        _objectIdMap.push_back(objectId);

        _objectMap.push_back(res); // Basically a hack, but behavior is all correct
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

uint Archive::findObjectId(const char *name) {
    RuntimeClass * found = static_cast<RuntimeClass*>
    (bsearch(name, classMap, sizeof(classMap) / sizeof(RuntimeClass) , sizeof(RuntimeClass), [] (const void *a, const void *b) {
                return strcmp((const char *) a, *(const char **) b);
    }));

    if (!found)
        error("Class %s is not in class Map", name);

    return found->id;
}

Common::String Archive::readString() {
    char buffer[kMaxStringLength]; // test and lower then
    byte len = _file.readByte();
    _file.read(buffer, len);
    return Common::String(buffer, len);
}

uint32 Archive::readDWORD() {
    return _file.readUint32LE();
}

uint16 Archive::readWORD() {
    return _file.readUint16LE();
}

} // End of namespace Pink


