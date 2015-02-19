/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/command.h"

#include "engines/stark/debug.h"
#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/bookmark.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/dialog.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/knowledge.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/sound.h"
#include "engines/stark/resources/speech.h"
#include "engines/stark/resources/textureset.h"
#include "engines/stark/resourcereference.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/resourceprovider.h"

namespace Stark {
namespace Resources {

Command::~Command() {
}

Command::Command(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Object(parent, subType, index, name) {
	_type = TYPE;
}

Command *Command::execute(uint32 callMode, Script *script) {
	switch (_subType) {
	case kCommandBegin:
		return opScriptBegin();
	case kDialogCall:
		return opDialogCall(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kSetInteractiveMode:
		return opSetInteractiveMode(_arguments[1].intValue);
	case kLocationGoTo:
		return opLocationGoTo(_arguments[0].stringValue, _arguments[1].stringValue, _arguments[2].referenceValue, _arguments[3].intValue);
	case kScriptPause:
		return opScriptPause(script, _arguments[1].referenceValue);
	case kScriptPauseRandom:
		return opPauseRandom(script, _arguments[1].referenceValue);
	case kItem3DPlaceOn:
		return opItem3DPlaceOn(_arguments[1].referenceValue, _arguments[2].referenceValue);
	case kItemEnable:
		return opItemEnable(_arguments[1].referenceValue, _arguments[2].intValue);
	case kScriptEnable:
		return opScriptEnable(_arguments[1].referenceValue, _arguments[2].intValue);
	case kSetBoolean:
		return opSetBoolean(_arguments[1].referenceValue, _arguments[2].intValue);
	case kSoundPlay:
		return opSoundPlay(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kPlayFullMotionVideo:
		return opPlayFullMotionVideo(_arguments[1].referenceValue, _arguments[2].intValue);
	case kEnableDiaryEntry:
		return opEnableDiaryEntry(_arguments[1].referenceValue);
	case kItemPlaceDirection:
		return opItemPlaceDirection(_arguments[1].referenceValue, _arguments[2].intValue);
	case kActivateTexture:
		return opActivateTexture(_arguments[1].referenceValue);
	case kActivateMesh:
		return opActivateMesh(_arguments[1].referenceValue);
	case kSpeakWithoutTalking:
		return opSpeakWithoutTalking(_arguments[1].referenceValue, _arguments[2].intValue);
	case kIsOnFloorField:
		return opIsOnFloorField(_arguments[0].intValue, _arguments[1].intValue, _arguments[2].referenceValue, _arguments[3].referenceValue);
	case kIsSet:
		return opIsSet(_arguments[0].intValue, _arguments[1].intValue, _arguments[2].referenceValue);
	case kIsRandom:
		return opIsRandom(_arguments[0].intValue, _arguments[1].intValue, _arguments[2].intValue);
	default:
		warning("Unimplemented command %d - %s", _subType, _name.c_str());
		printData();
		break;
	}

	return nextCommand();
}

Command *Command::opScriptBegin() {
	return nextCommand();
}

Command *Command::opDialogCall(Script *script, const ResourceReference &dialogRef, int32 suspend) {
	DialogPlayer *dialogPlayer = StarkServices::instance().dialogPlayer;

	Dialog *dialog = dialogRef.resolve<Dialog>();
	dialogPlayer->run(dialog);

	if (suspend) {
		script->suspend(dialog);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opSetInteractiveMode(bool enabled) {
	assert(_arguments.size() == 2);
	warning("(TODO: Implement) opSetInteractiveMode(%d)", enabled);
	return nextCommand();
}

Command *Command::opLocationGoTo(const Common::String &level, const Common::String &location, const ResourceReference &bookmarkRef, int32 direction) {
	ResourceProvider *resourceProvider = StarkServices::instance().resourceProvider;

	uint levelIndex = strtol(level.c_str(), nullptr, 16);
	uint locationIndex = strtol(location.c_str(), nullptr, 16);
	resourceProvider->requestLocationChange(levelIndex, locationIndex);
	resourceProvider->setNextLocationPosition(bookmarkRef, direction);

	return nullptr;
}


Command *Command::opScriptPause(Script *script, const ResourceReference &durationRef) {
	Knowledge *duration = durationRef.resolve<Knowledge>();
	script->pause(duration->getIntegerValue());

	return this; // Stay on this command while the script is suspended
}

Command *Command::opPauseRandom(Script *script, const ResourceReference &ref) {
	Knowledge *duration = ref.resolve<Knowledge>();
	warning("(TODO: Implement) Pause random: %d, %s", duration->getIntegerValue(), ref.describe().c_str());
	// TODO: Actually get a random value here.
	script->pause(1000);
	return this; // Stay on this command while the script is suspended
}

Command *Command::opItem3DPlaceOn(const ResourceReference &itemRef, const ResourceReference &targetRef) {
	ItemSub5610 *item = itemRef.resolve<ItemSub5610>();
	Object *target = targetRef.resolve<Object>();

	switch (target->getType().get()) {
	case Type::kBookmark:
		item->placeOnBookmark(Object::cast<Bookmark>(target));
		break;
	default:
		warning("Unimplemented op3DPlaceOn target type %s", target->getType().getName());
	}

	return nextCommand();
}

Command *Command::opItemEnable(const ResourceReference &itemRef, int32 enable) {
	Item *item = itemRef.resolve<Item>();

	bool previousState = item->isEnabled();

	switch (enable) {
	case 0:
		item->setEnabled(false);
		break;
	case 1:
		if (!previousState) {
			item->setEnabled(true);
		}
		break;
	case 2:
		item->setEnabled(!previousState);
		break;
	}

	return nextCommand();
}

Command *Command::opScriptEnable(const ResourceReference &scriptRef, int32 enable) {
	assert(_arguments.size() == 3);
	Script *script = scriptRef.resolve<Script>();
	warning("(TODO: Implement) opScriptEnable(%s, %d) : %s", script->getName().c_str(), enable, scriptRef.describe().c_str());

	script->enable(enable);

	return nextCommand();
}

Command *Command::opSetBoolean(const ResourceReference &knowledgeRef, int32 value) {
	assert(_arguments.size() == 3);
	Knowledge *boolean = knowledgeRef.resolve<Knowledge>();
	warning("(TODO: Implement) opSetBoolean(%s, %d) : %s", boolean->getName().c_str(), value, knowledgeRef.describe().c_str());

	return nextCommand();
}

Command *Command::opSoundPlay(Script *script, const ResourceReference &soundRef, int32 suspend) {
	Sound *sound = soundRef.resolve<Sound>();
	sound->play();

	if (suspend) {
		script->suspend(sound);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opPlayFullMotionVideo(const ResourceReference &movieRef, int32 unknown) {
	Object *movie =  movieRef.resolve<Object>();
	warning("(TODO: Implement) opPlayFullMotionVideo(%s) : %s - %d", movie->getName().c_str(), movieRef.describe().c_str(), unknown);
	return nextCommand();
}

Command *Command::opEnableDiaryEntry(const ResourceReference &knowledgeRef) {
	assert(_arguments.size() == 2);
	Knowledge *entry = knowledgeRef.resolve<Knowledge>();
	warning("(TODO: Implement) opEnableDiaryEntry(%s) : %s", entry->getName().c_str(), knowledgeRef.describe().c_str());

	return nextCommand();
}

Command *Command::opItemPlaceDirection(const ResourceReference &itemRef, int32 direction) {
	ItemSub5610 *item = itemRef.resolve<ItemSub5610>();

	item->setDirection(abs(direction) % 360);

	return nextCommand();
}

Command *Command::opActivateTexture(const ResourceReference &textureRef) {
	TextureSet *texture = textureRef.resolve<TextureSet>();
	warning("(TODO: Implement) opActivateTexture(%s) : %s", texture->getName().c_str(), textureRef.describe().c_str());

	return nextCommand();
}

Command *Command::opActivateMesh(const ResourceReference &meshRef) {
	BonesMesh *mesh = meshRef.resolve<BonesMesh>();
	warning("(TODO: Implement) opActivateMesh(%s) : %s", mesh->getName().c_str(), meshRef.describe().c_str());

	return nextCommand();
}

Command *Command::opSpeakWithoutTalking(const ResourceReference &speechRef, int32 unknown) {
	assert(_arguments.size() == 3);
	Speech *speechObj = speechRef.resolve<Speech>();
	warning("(TODO: Implement) opSpeakWithoutTalking(%s, %d) : %s", speechObj->getName().c_str(), unknown, speechRef.describe().c_str());

	// TODO: Block further calls untill it'd done
	speechObj->playSound();
	return nextCommand();
}

Command *Command::opIsOnFloorField(int branch1, int branch2, const ResourceReference &itemRef, const ResourceReference &floorFieldRef) {
	Object *itemObj = itemRef.resolve<Object>();
	Object *floorFieldObj = floorFieldRef.resolve<Object>();
	warning("(TODO: Implement) opIsOnFloorField(%d, %d, %s, %s) : %s , %s", branch1, branch2, itemObj->getName().c_str(), floorFieldObj->getName().c_str(), itemRef.describe().c_str(), floorFieldRef.describe().c_str());
	// TODO: Actually implement the logic
	return nextCommandIf(false);
}

Command *Command::opIsSet(int branch1, int branch2, const ResourceReference &knowledgeRef) {
	assert(_arguments.size() == 3);
	Knowledge *value = knowledgeRef.resolve<Knowledge>();
	warning("(TODO: Implement) opIsSet(%d, %d, %s) %d : %s", branch1, branch2, value->getName().c_str(), value->getIntegerValue(), knowledgeRef.describe().c_str());
	// TODO: Verify how this logic actually should be handled
	return nextCommandIf(value->getIntegerValue());
}

Command *Command::opIsRandom(int branch1, int branch2, int32 unknown) {
	assert(_arguments.size() == 3);
	warning("(TODO: Implement) opIsRandom(%d, %d, %d)", branch1, branch2, unknown);
	// TODO: Verify how this logic actually should be handled
	return nextCommandIf(true);
}

Command *Command::nextCommand() {
	assert(!_arguments.empty());
	assert(_arguments[0].type == Argument::kTypeInteger1);

	return resolveArgumentSiblingReference(_arguments[0]);
}

Command *Command::nextCommandIf(bool predicate) {
	assert(_arguments.size() >= 2);
	assert(_arguments[0].type == Argument::kTypeInteger1);
	assert(_arguments[1].type == Argument::kTypeInteger1);

	if (predicate) {
		return resolveArgumentSiblingReference(_arguments[1]);
	} else {
		return resolveArgumentSiblingReference(_arguments[0]);
	}
}

Command *Command::resolveArgumentSiblingReference(const Argument &argument) {
	return _parent->findChildWithIndex<Command>(argument.intValue);
}

void Command::readData(Formats::XRCReadStream *stream) {
	uint32 count = stream->readUint32LE();
	for (uint i = 0; i < count; i++) {
		Argument argument;
		argument.type = stream->readUint32LE();

		switch (argument.type) {
		case Argument::kTypeInteger1:
		case Argument::kTypeInteger2:
			argument.intValue = stream->readUint32LE();
			break;

		case Argument::kTypeResourceReference:
			argument.referenceValue = stream->readResourceReference();
			break;
		case Argument::kTypeString:
			argument.stringValue = stream->readString();
			break;
		default:
			error("Unknown argument type %d", argument.type);
		}

		_arguments.push_back(argument);
	}
}

void Command::printData() {
	for (uint i = 0; i < _arguments.size(); i++) {
		switch (_arguments[i].type) {
		case Argument::kTypeInteger1:
		case Argument::kTypeInteger2:
			debug("%d: %d", i, _arguments[i].intValue);
			break;

		case Argument::kTypeResourceReference: {
			debug("%d: %s", i, _arguments[i].referenceValue.describe().c_str());
		}
			break;
		case Argument::kTypeString:
			debug("%d: %s", i, _arguments[i].stringValue.c_str());
			break;
		default:
			error("Unknown argument type %d", _arguments[i].type);
		}
	}
}

} // End of namespace Resources
} // End of namespace Stark
