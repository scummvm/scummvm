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
		kScriptPauseSkippable = 11,

		kScriptAbort = 13,

		kExit2DLocation = 16,
		kGoto2DLocation = 17,

		kRumbleScene = 19,
		kFadeScene = 20,
		kSwayScene = 21,

		kLocationGoToNewCD = 22,
		kGameEnd = 23,
		kInventoryOpen = 24,
		kFloatScene = 25,
		kBookOfSecretsOpen = 26,

		kDoNothing = 80,
		kItem3DPlaceOn = 81,
		kItem3DWalkTo = 82,
		kItem3DFollowPath = 83,
		kItemLookAt = 84,

		kItem2DFollowPath = 86,
		kItemEnable = 87,
		kItemSetActivity = 88,
		kItemSelectInInventory = 89,

		kUseAnimHierarchy = 92,
		kPlayAnimation = 93,
		kScriptEnable = 94,
		kShowPlay = 95,
		kKnowledgeSetBoolean = 96,

		kKnowledgeSetInteger = 100,
		kKnowledgeAddInteger = 101,

		kEnableFloorField = 103,
		kPlayAnimScriptItem = 104,
		kItemAnimFollowPath = 105,

		kKnowledgeAssignBool = 107,

		kKnowledgeAssignInteger = 110,
		kLocationScrollTo = 111,
		kSoundPlay = 112,

		kKnowledgeSetIntRandom = 115,

		kKnowledgeSubValue = 117,
		kItemLookDirection = 118,

		kStopPlayingSound = 119,
		kLayerGoTo = 120,
		kLayerEnable = 121,
		kLocationScrollSet = 122,
		kFullMotionVideoPlay = 123,

		kAnimSetFrame = 125,
		kKnowledgeAssignNegatedBool = 126,
		kDiaryEnableEntry = 127,
		kPATChangeTooltip = 128,
		kSoundChange = 129,
		kLightSetColor = 130,
		kLightFollowPath = 131,
		kItem3DRunTo = 132,
		kItemPlaceDirection = 133,
		kItemRotateDirection = 134,
		kActivateTexture = 135,
		kActivateMesh = 136,
		kItem3DSetWalkTarget = 137,

		kSpeakWithoutTalking = 139,

		kIsOnFloorField = 162,
		kIsItemEnabled = 163,

		kIsScriptEnabled = 165,
		kIsKnowledgeBooleanSet = 166,

		kIsKnowledgeIntegerInRange = 170,
		kIsKnowledgeIntegerAbove = 171,
		kIsKnowledgeIntegerEqual = 172,
		kIsKnowledgeIntegerLower = 173,
		kIsScriptActive = 174,
		kIsRandom = 175,
		kIsAnimScriptItemReached = 176,
		kIsItemOnPlace = 177,

		kIsAnimPlaying = 179,
		kIsItemActivity = 180,

		kIsItemNearPlace = 183,

		kIsAnimAtTime = 185,
		kIsLocation2D = 186,
		kIsInventoryOpen = 187
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

	/** Get the command's list of arguments */
	Common::Array<Argument> getArguments() const;

	/** Resume the opcode ItemSetActivity after it has stopped waiting for the action anim to complete */
	void resumeItemSetActivity();

protected:
	void readData(Formats::XRCReadStream *stream) override;

	Command *resolveArgumentSiblingReference(const Argument &argument);

	Math::Vector3d getObjectPosition(const ResourceReference &targetRef, int32 *floorFace = nullptr);

	Command *opScriptBegin();
	Command *opScriptCall(Script *script, const ResourceReference &scriptRef, int32 synchronous);
	Command *opDialogCall(Script *script, const ResourceReference &dialogRef, int32 suspend);
	Command *opSetInteractiveMode(bool enabled);
	Command *opLocationGoTo(const Common::String &level, const Common::String &location, const ResourceReference &bookmarkRef, int32 direction);
	Command *opWalkTo(Script *script, const ResourceReference &objectRef, int32 suspend);
	Command *opScriptPauseGameLoop(Script *script, int32 count);
	Command *opScriptPause(Script *script, const ResourceReference &durationRef);
	Command *opScriptPauseRandom(Script *script, const ResourceReference &itemRef);
	Command *opScriptPauseSkippable(Script *script, const ResourceReference &durationRef);
	Command *opScriptAbort(ResourceReference scriptRef, bool disable);
	Command *opExit2DLocation();
	Command *opGoto2DLocation(const Common::String &level, const Common::String &location);
	Command *opRumbleScene(Script *script, int32 rumbleDuration, int32 pause);
	Command *opFadeScene(Script *script, bool fadeOut, int32 fadeDuration, bool pause);
	Command *opSwayScene(int32 periodMs, int32 angleIn, int32 amplitudeIn, int32 offsetIn);
	Command *opGameEnd();
	Command *opInventoryOpen(bool open);
	Command *opFloatScene(int32 periodMs, int32 amplitudeIn, int32 offsetIn);
	Command *opBookOfSecretsOpen();
	Command *opDoNothing();
	Command *opItem3DPlaceOn(const ResourceReference &itemRef, const ResourceReference &targetRef);
	Command *opItem3DWalkTo(Script *script, const ResourceReference &itemRef, const ResourceReference &targetRef, bool suspend);
	Command *opItemFollowPath(Script *script, ResourceReference itemRef, ResourceReference pathRef, uint32 speed, uint32 suspend);
	Command *opItemLookAt(Script *script, const ResourceReference &itemRef, const ResourceReference &objRef, bool suspend, int32 unknown);
	Command *opItemEnable(const ResourceReference &itemRef, int32 enable);
	Command *opItemSetActivity(Script *script, const ResourceReference &itemRef, int32 animActivity, bool wait);
	Command *opItemSelectInInventory(const ResourceReference &itemRef);
	Command *opUseAnimHierachy(const ResourceReference &animHierRef);
	Command *opPlayAnimation(Script *script, const ResourceReference &animRef, bool suspend);
	Command *opScriptEnable(const ResourceReference &scriptRef, int32 enable);
	Command *opShowPlay(Script *script, const ResourceReference &ref, int32 suspend);
	Command *opKnowledgeSetBoolean(const ResourceReference &knowledgeRef, int32 enable);
	Command *opKnowledgeSetInteger(const ResourceReference &knowledgeRef, int32 value);
	Command *opKnowledgeSetIntRandom(const ResourceReference &knowledgeRef, uint32 min, uint32 max);
	Command *opKnowledgeAddInteger(const ResourceReference &knowledgeRef, int32 increment);
	Command *opKnowledgeSubValue(const ResourceReference &knowledgeRef, const ResourceReference &valueRef);
	Command *opEnableFloorField(const ResourceReference &floorFieldRef, bool enable);
	Command *opPlayAnimScriptItem(Script *script, const ResourceReference &animScriptItemRef, int32 suspend);
	Command *opItemAnimFollowPath(Script *script, const ResourceReference &animRef, const ResourceReference &pathRef, int32 speed, bool suspend);
	Command *opKnowledgeAssignBool(const ResourceReference &knowledgeRef1, const ResourceReference &knowledgeRef2);
	Command *opKnowledgeAssignNegatedBool(const ResourceReference &knowledgeRef1, const ResourceReference &knowledgeRef2);
	Command *opKnowledgeAssignInteger(const ResourceReference &knowledgeRef1, const ResourceReference &knowledgeRef2);
	Command *opLocationScrollTo(Script *script, const ResourceReference &scrollRef, bool suspend);
	Command *opSoundPlay(Script *script, const ResourceReference &soundRef, int32 suspend);
	Command *opItemLookDirection(Script *script, const ResourceReference &itemRef, int32 direction, bool suspend);
	Command *opStopPlayingSound(const ResourceReference &soundRef);
	Command *opLayerGoTo(const ResourceReference &layerRef);
	Command *opLayerEnable(const ResourceReference &layerRef, int32 enable);
	Command *opLocationScrollSet(const ResourceReference &scrollRef);
	Command *opFullMotionVideoPlay(Script *script, const ResourceReference &movieRef, int32 unknown);
	Command *opAnimSetFrame(const ResourceReference &animRef, const ResourceReference &knowledgeRef);
	Command *opDiaryEnableEntry(const ResourceReference &knowledgeRef);
	Command *opPATChangeTooltip(const ResourceReference &patRef, const ResourceReference &stringRef);
	Command *opSoundChange(Script *script, const ResourceReference &soundRef, int32 volume, int32 pan, int32 duration, bool pause);
	Command *opLightSetColor(const ResourceReference &lightRef, int32 red, int32 green, int32 blue);
	Command *opLightFollowPath(Script *script, const ResourceReference &itemRef, const ResourceReference &lightRef, const ResourceReference &pathRef, int32 speed, bool suspend);
	Command *opItem3DRunTo(Script *script, const ResourceReference &itemRef, const ResourceReference &targetRef, int32 suspend);
	Command *opItemPlaceDirection(const ResourceReference &itemRef, int32 direction);
	Command *opItemRotateDirection(Script *script, const ResourceReference &itemRef, int32 direction, int32 speed, bool suspend);
	Command *opActivateTexture(const ResourceReference &textureRef);
	Command *opActivateMesh(const ResourceReference &meshRef);
	Command *opItem3DSetWalkTarget(const ResourceReference &itemRef, const ResourceReference &targetRef);
	Command *opSpeakWithoutTalking(Script *script, const ResourceReference &speechRef, int32 unknown);
	Command *opIsOnFloorField(const ResourceReference &itemRef, const ResourceReference &floorFieldRef);
	Command *opIsItemEnabled(const ResourceReference &itemRef);
	Command *opIsScriptEnabled(const ResourceReference &scriptRef);
	Command *opIsKnowledgeBooleanSet(const ResourceReference &knowledgeRef);
	Command *opIsKnowledgeIntegerInRange(const ResourceReference &knowledgeRef, int32 min, int32 max);
	Command *opIsKnowledgeIntegerAbove(const ResourceReference &knowledgeRef, int32 value);
	Command *opIsKnowledgeIntegerEqual(const ResourceReference &knowledgeRef, int32 value);
	Command *opIsKnowledgeIntegerLower(const ResourceReference &knowledgeRef, int32 value);
	Command *opIsScriptActive(const ResourceReference &scriptRef);
	Command *opIsRandom(int32 chance);
	Command *opIsAnimScriptItemReached(const ResourceReference &animScriptItemRef);
	Command *opIsItemNearPlace(const ResourceReference &itemRef, const ResourceReference &positionRef, int32 testDistance);
	Command *opIsItemOnPlace(const ResourceReference &itemRef, const ResourceReference &positionRef);
	Command *opIsAnimPlaying(const ResourceReference &animRef);
	Command *opIsItemActivity(const ResourceReference &itemRef, int32 value);
	Command *opIsAnimAtTime(const ResourceReference &animRef, int32 time);
	Command *opIsLocation2D();
	Command *opIsInventoryOpen();
	
	Common::Array<Argument> _arguments;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_COMMAND_H
