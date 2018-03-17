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
#include "objects/module.h"
#include "objects/page.h"

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
        {"GamePage", kGamePage},
        {"ModuleProxy", kModuleProxy}
};

static Object* createObject(int objectId){
    switch (objectId){
        case kGamePage:
            return new GamePage();
        case kModuleProxy:
            return new ModuleProxy();
        default:
            return nullptr;
    }
}


Archive::Archive(Common::File &file)
    : _file(file)
{
    debug("Archive created");
    _objectMap.push_back(0);
    _objectIdMap.push_back(kNullObject);
}

Archive::~Archive()
{
    debug("Archive destroyed");
}

void Archive::mapObject(Object *obj) {
    _objectMap.push_back(obj); // Basically a hack, but behavior is all correct
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

    if (res && !isCopyReturned)
        res->deserialize(*this);

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
        _objectMap.push_back(res);
        _objectIdMap.push_back(objectId);

        //_objectMap.push_back(res); // Basically a hack, but behavior is all correct
        //_objectIdMap.push_back(objectId);

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
        error("Class %s is not implemented", name);

    return found->id;
}

Common::String Archive::readString() {
    char buffer[kMaxStringLength]; // test and lower then
    byte len = _file.readByte();
    _file.read(buffer, len);
    return Common::String(buffer, len);
}

} // End of namespace Pink


