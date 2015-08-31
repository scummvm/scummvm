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

#include "math/vector3d.h"

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

		kRumbleScene = 19,
		kFadeScene = 20,

		kItem3DPlaceOn = 81,
		kItem3DWalkTo = 82,

		kItemLookAt = 84,

		kItemEnable = 87,
		kItemSetActivity = 88,
		kItemSelectInInventory = 89,

		kUseAnimHierarchy = 92,
		kPlayAnimation = 93,
		kScriptEnable = 94,
		kShowPlay = 95,
		kSetBoolean = 96,

		kSetInteger = 100,

		kEnableFloorField = 103,
		kPlayAnimScriptItem = 104,

		kKnowledgeAssignBool = 107,

		kSoundPlay = 112,

		kItemLookDirection = 118,

		kStopPlayingSound = 119,
		kGoLayer = 120,

		kScrollSet = 122,
		kPlayFullMotionVideo = 123,

	    kKnowledgeAssignNegatedBool = 126,
		kEnableDiaryEntry = 127,
		kPATChangeTooltip = 128,

		kChangeSound = 129,

		kItem3DRunTo = 132,
		kItemPlaceDirection = 133,

		kActivateTexture = 135,
		kActivateMesh = 136,
		kSetTarget = 137,

		kSpeakWithoutTalking = 139,

		kIsOnFloorField = 162,
		kIsItemEnabled = 163,

		kIsScriptEnabled = 165,
		kIsSet = 166,

		kIsIntegerInRange = 170,
		kIsIntegerAbove = 171,
		kIsIntegerEqual = 172,
		kIsIntegerLower = 173,
		kIsScriptActive = 174,
		kIsRandom = 175,

		kIsOnPlace = 177,

		kIsAnimPlaying = 179,

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

	Math::Vector3d getObjectPosition(const ResourceReference &targetRef, int32 *floorFace = nullptr);

	Command *opScriptBegin();
	Command *opScriptCall(Script *script, const ResourceReference &scriptRef, int32 synchronous);
	Command *opDialogCall(Script *script, const ResourceReference &dialogRef, int32 suspend);
	Command *opSetInteractiveMode(bool enabled);
	Command *opLocationGoTo(const Common::String &level, const Common::String &location, const ResourceReference &bookmarkRef, int32 direction);
	Command *opWalkTo(Script *script, const ResourceReference &objectRef, int32 suspend);
	Command *opGameLoop(int32 unknown);
	Command *opScriptPause(Script *script, const ResourceReference &durationRef);
	Command *opPauseRandom(Script *script, const ResourceReference &itemRef);
	Command *opExit2DLocation(Script *script);
	Command *opGoto2DLocation(const Common::String &level, const Common::String &location);
	Command *opRumbleScene(int32 unknown1, int32 unknown2);
	Command *opFadeScene(int32 unknown1, int32 unknown2, int32 unknown3);
	Command *opItem3DPlaceOn(const ResourceReference &itemRef, const ResourceReference &targetRef);
	Command *opItem3DWalkTo(Script *script, const ResourceReference &itemRef, const ResourceReference &targetRef, bool suspend);
	Command *opItemLookAt(Script *script, const ResourceReference &itemRef, const ResourceReference &objRef, bool suspend, int32 unknown);
	Command *opItemEnable(const ResourceReference &itemRef, int32 enable);
	Command *opItemSetActivity(const ResourceReference &itemRef, int32 unknown1, int32 unknown2);
	Command *opItemSelectInInventory(const ResourceReference &itemRef);
	Command *opUseAnimHierachy(const ResourceReference &animHierRef);
	Command *opPlayAnimation(Script *script, const ResourceReference &animRef, bool suspend);
	Command *opScriptEnable(const ResourceReference &scriptRef, int32 enable);
	Command *opShowPlay(Script *script, const ResourceReference &ref, int32 suspend);
	Command *opSetBoolean(const ResourceReference &knowledgeRef, int32 enable);
	Command *opSetInteger(const ResourceReference &knowledgeRef, int32 value);
	Command *opEnableFloorField(const ResourceReference &floorFieldRef, int32 value);
	Command *opPlayAnimScriptItem(Script *script, const ResourceReference &animScriptItemRef, int32 pause);
	Command *opKnowledgeAssignBool(const ResourceReference &knowledgeRef1, const ResourceReference &knowledgeRef2);
	Command *opKnowledgeAssignNegatedBool(const ResourceReference &knowledgeRef1, const ResourceReference &knowledgeRef2);
	Command *opSoundPlay(Script *script, const ResourceReference &soundRef, int32 suspend);
	Command *opItemLookDirection(Script *script, const ResourceReference &itemRef, int32 direction, bool suspend);
	Command *opStopPlayingSound(const ResourceReference &soundRef);
	Command *opGoLayer(const ResourceReference &layerRef);
	Command *opScrollSet(const ResourceReference &scrollRef);
	Command *opPlayFullMotionVideo(Script *script, const ResourceReference &movieRef, int32 unknown);
	Command *opEnableDiaryEntry(const ResourceReference &knowledgeRef);
    Command *opPATChangeTooltip(const ResourceReference &patRef, const ResourceReference &stringRef);
	Command *opChangeSound(const ResourceReference &soundRef, int32 unknown1, int32 unknown2, int32 unknown3, int32 unknown4);
	Command *opItem3DRunTo(const ResourceReference &itemRef, const ResourceReference &bookmarkRef, int32 unknown);
	Command *opItemPlaceDirection(const ResourceReference &itemRef, int32 direction);
	Command *opActivateTexture(const ResourceReference &textureRef);
	Command *opActivateMesh(const ResourceReference &meshRef);
	Command *opSetTarget(const ResourceReference &itemRef1, const ResourceReference &itemRef2);
	Command *opSpeakWithoutTalking(Script *script, const ResourceReference &speechRef, int32 unknown);
	Command *opIsOnFloorField(const ResourceReference &itemRef, const ResourceReference &floorFieldRef);
	Command *opIsItemEnabled(const ResourceReference &itemRef);
	Command *opIsScriptEnabled(const ResourceReference &scriptRef);
	Command *opIsSet(const ResourceReference &knowledgeRef);
	Command *opIsIntegerInRange(const ResourceReference &knowledgeRef, int32 min, int32 max);
	Command *opIsIntegerAbove(const ResourceReference &knowledgeRef, int32 value);
	Command *opIsIntegerEqual(const ResourceReference &knowledgeRef, int32 value);
	Command *opIsIntegerLower(const ResourceReference &knowledgeRef, int32 value);
	Command *opIsScriptActive(const ResourceReference &scriptRef);
	Command *opIsRandom(int32 chance);
	Command *opIsOnNearPlace(const ResourceReference &itemRef, const ResourceReference &positionRef, int32 testDistance);
	Command *opIsOnPlace(const ResourceReference &itemRef, const ResourceReference &positionRef);
	Command *opIsAnimPlaying(int branch1, int branch2, const ResourceReference &animRef);
	Command *opIsAnimAtTime(int branch1, int branch2, const ResourceReference &animRef, int32 time);
	
	Common::Array<Argument> _arguments;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_COMMAND_H
