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

#ifndef STARK_RESOURCES_COMMAND_H
#define STARK_RESOURCES_COMMAND_H

#include "common/array.h"
#include "common/str.h"

#include "engines/stark/resources/object.h"
#include "engines/stark/resourcereference.h"

namespace Stark {

class ResourceReference;
namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Script;

/**
 * Command resources are script operations.
 *
 * The operation code is the resource subtype.
 *
 * The operation arguments can be integers, strings or resource references.
 */
class Command : public Object {
public:
	static const Type::ResourceType TYPE = Type::kCommand;

	Command(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Command();

	enum SubType {
		kCommandBegin = 0,
		kCommandEnd = 1,

		kDialogCall = 3,

		kLocationGoTo = 5,

		kScriptPause = 9,
		kScriptPauseRandom = 10,

		kItem3DPlaceOn = 81,

		kItemEnable = 87,

		kSoundPlay = 112,

		kPlayFullMotionVideo = 123,

		kItemPlaceDirection = 133,

		kActivateTexture = 135
	};

	struct Argument {
		enum Type {
			kTypeInteger1 = 1,
			kTypeInteger2 = 2,
			kTypeResourceReference = 3,
			kTypeString = 4
		};

		uint32 type;
		uint32 intValue;
		Common::String stringValue;
		ResourceReference referenceValue;
	};

	/** Execute the command */
	Command *execute(uint32 callMode, Script *script);

	/** Obtain the next command to be executed */
	Command *nextCommand();

	/** Obtain the next command to be executed, depending on a predicate */
	Command *nextCommandIf(bool predicate);

protected:
	void readData(Formats::XRCReadStream *stream) override;
	void printData() override;

	Command *resolveArgumentSiblingReference(const Argument &argument);

	Command *opScriptBegin();
	Command *opDialogCall(Script *script, const ResourceReference &dialogRef, int32 suspend);
	Command *opLocationGoTo(const Common::String &level, const Common::String &location, const ResourceReference &bookmarkRef, int32 direction);
	Command *opScriptPause(Script *script, const ResourceReference &durationRef);
	Command *opPauseRandom(Script *script, const ResourceReference &itemRef);
	Command *opItem3DPlaceOn(const ResourceReference &itemRef, const ResourceReference &targetRef);
	Command *opItemEnable(const ResourceReference &itemRef, int32 enable);
	Command *opSoundPlay(Script *script, const ResourceReference &soundRef, int32 suspend);
	Command *opPlayFullMotionVideo(const ResourceReference &movieRef, int32 unknown);
	Command *opItemPlaceDirection(const ResourceReference &itemRef, int32 direction);
	Command *opActivateTexture(const ResourceReference &textureRef);

	Common::Array<Argument> _arguments;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_COMMAND_H
