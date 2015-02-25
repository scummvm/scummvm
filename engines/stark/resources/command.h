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
		kScriptCall = 2,
		kDialogCall = 3,
		kSetInteractiveMode = 4,
		kLocationGoTo = 5,

		kWalkTo = 7,
		kGameLoop = 8,
		kScriptPause = 9,
		kScriptPauseRandom = 10,

		kExit2DLocation = 16,
		kGoto2DLocation = 17,

		kItem3DPlaceOn = 81,
		kItem3DWalkTo = 82,

		kItemEnable = 87,

		kPlayAnimation = 93,
		kScriptEnable = 94,
		kShowPlay = 95,
		kSetBoolean = 96,

		kSetInteger = 100,

		kEnableFloorField = 103,

		kSoundPlay = 112,

		kLookDirection = 118,

		kGoLayer = 120,

		kScrollSet = 122,
		kPlayFullMotionVideo = 123,

		kEnableDiaryEntry = 127,

		kChangeSound = 129,

		kItemPlaceDirection = 133,

		kActivateTexture = 135,
		kActivateMesh = 136,
		kSetTarget = 137,

		kSpeakWithoutTalking = 139,

		kIsOnFloorField = 162,

		kIsSet = 166,

		kIsIntegerEqual = 172,
		kIsIntegerLower = 173,
		kIsRandom = 175,

		kIsOnPlace = 177,

		kIsOnNearPlace = 183,

		kIsAnimAtTime = 185
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
	Command *opScriptCall(Script *script, const ResourceReference &scriptRef, int32 unknown);
	Command *opDialogCall(Script *script, const ResourceReference &dialogRef, int32 suspend);
	Command *opSetInteractiveMode(bool enabled);
	Command *opLocationGoTo(const Common::String &level, const Common::String &location, const ResourceReference &bookmarkRef, int32 direction);
	Command *opWalkTo(int32 unknown, const ResourceReference &bookmarkRef, int32 unknown2);
	Command *opGameLoop(int32 unknown);
	Command *opScriptPause(Script *script, const ResourceReference &durationRef);
	Command *opPauseRandom(Script *script, const ResourceReference &itemRef);
	Command *opExit2DLocation(Script *script);
	Command *opGoto2DLocation(const Common::String &level, const Common::String &location);
	Command *opItem3DPlaceOn(const ResourceReference &itemRef, const ResourceReference &targetRef);
	Command *opItem3DWalkTo(const ResourceReference &itemRef, const ResourceReference &targetRef, int32 unknown);
	Command *opItemEnable(const ResourceReference &itemRef, int32 enable);
	Command *opPlayAnimation(const ResourceReference &animRef, int32 unknown);
	Command *opScriptEnable(const ResourceReference &scriptRef, int32 enable);
	Command *opShowPlay(Script *script, const ResourceReference &ref, int32 unknown);
	Command *opSetBoolean(const ResourceReference &knowledgeRef, int32 value);
	Command *opSetInteger(const ResourceReference &knowledgeRef, int32 value);
	Command *opEnableFloorField(const ResourceReference &floorFieldRef, int32 value);
	Command *opSoundPlay(Script *script, const ResourceReference &soundRef, int32 suspend);
	Command *opLookDirection(const ResourceReference &itemRef, int32 unknown1, int32 unknown2);
	Command *opGoLayer(const ResourceReference &layerRef);
	Command *opScrollSet(const ResourceReference &scrollRef);
	Command *opPlayFullMotionVideo(const ResourceReference &movieRef, int32 unknown);
	Command *opEnableDiaryEntry(const ResourceReference &knowledgeRef);
	Command *opChangeSound(const ResourceReference &soundRef, int32 unknown1, int32 unknown2, int32 unknown3, int32 unknown4);
	Command *opItemPlaceDirection(const ResourceReference &itemRef, int32 direction);
	Command *opActivateTexture(const ResourceReference &textureRef);
	Command *opActivateMesh(const ResourceReference &meshRef);
	Command *opSetTarget(const ResourceReference &itemRef1, const ResourceReference &itemRef2);
	Command *opSpeakWithoutTalking(Script *script, const ResourceReference &speechRef, int32 unknown);
	Command *opIsOnFloorField(int branch1, int branch2, const ResourceReference &itemRef, const ResourceReference &floorFieldRef);
	Command *opIsSet(int branch1, int branch2, const ResourceReference &knowledgeRef);
	Command *opIsIntegerEqual(int branch1, int branch2, const ResourceReference &knowledgeRef, int32 value);
	Command *opIsIntegerLower(int branch1, int branch2, const ResourceReference &knowledgeRef, int32 value);
	Command *opIsRandom(int branch1, int branch2, int32 unknown);
	Command *opIsOnNearPlace(int branch1, int branch2, const ResourceReference &itemRef, const ResourceReference &position, int32 unknown);
	Command *opIsOnPlace(int branch1, int branch2, const ResourceReference &itemRef, const ResourceReference &position);
	Command *opIsAnimAtTime(int branch1, int branch2, const ResourceReference &animRef, int32 time);
	
	Common::Array<Argument> _arguments;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_COMMAND_H
