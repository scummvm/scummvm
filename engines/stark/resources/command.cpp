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
#include "engines/stark/resources/dialog.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/knowledge.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/sound.h"
#include "engines/stark/resourcereference.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/resourceprovider.h"

namespace Stark {

Command::~Command() {
}

Command::Command(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name) {
	_type = TYPE;
}

Command *Command::execute(uint32 callMode, Script *script) {
	switch (_subType) {
	case kDialogCall:
		return opDialogCall(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kLocationGoTo:
		return opLocationGoTo(_arguments[0].stringValue, _arguments[1].stringValue, _arguments[2].referenceValue, _arguments[3].intValue);
	case kScriptPause:
		return opScriptPause(script, _arguments[1].referenceValue);
	case kItem3DPlaceOn:
		return opItem3DPlaceOn(_arguments[1].referenceValue, _arguments[2].referenceValue);
	case kItemEnable:
		return opItemEnable(_arguments[1].referenceValue, _arguments[2].intValue);
	case kSoundPlay:
		return opSoundPlay(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kItemPlaceDirection:
		return opItemPlaceDirection(_arguments[1].referenceValue, _arguments[2].intValue);
	default:
		warning("Unimplemented command %d - %s", _subType, _name.c_str());
		printData();
		break;
	}

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

Command *Command::opItem3DPlaceOn(const ResourceReference &itemRef, const ResourceReference &targetRef) {
	ItemSub5610 *item = itemRef.resolve<ItemSub5610>();
	Resource *target = targetRef.resolve<Resource>();

	switch (target->getType().get()) {
	case ResourceType::kBookmark:
		item->placeOnBookmark(Resource::cast<Bookmark>(target));
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

Command *Command::opItemPlaceDirection(const ResourceReference &itemRef, int32 direction) {
	ItemSub5610 *item = itemRef.resolve<ItemSub5610>();

	item->setDirection(abs(direction) % 360);

	return nextCommand();
}

Command *Command::nextCommand() {
	assert(!_arguments.empty());
	assert(_arguments[0].type == Argument::kTypeInteger1);

	return resolveArgumentSiblingReference(_arguments[0]);
}

Command *Command::nextCommandIf(bool predicate) {
	assert(!_arguments.size() >= 2);

	if (predicate) {
		return resolveArgumentSiblingReference(_arguments[1]);
	} else {
		return resolveArgumentSiblingReference(_arguments[0]);
	}
}

Command *Command::resolveArgumentSiblingReference(const Argument &argument) {
	return _parent->findChildWithIndex<Command>(argument.intValue);
}

void Command::readData(XRCReadStream *stream) {
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

} // End of namespace Stark
