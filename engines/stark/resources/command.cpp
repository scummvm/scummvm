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

#include "engines/stark/movement/turn.h"
#include "engines/stark/movement/walk.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/animhierarchy.h"
#include "engines/stark/resources/animscript.h"
#include "engines/stark/resources/bookmark.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/camera.h"
#include "engines/stark/resources/dialog.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/floorfield.h"
#include "engines/stark/resources/fmv.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/knowledge.h"
#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/sound.h"
#include "engines/stark/resources/speech.h"
#include "engines/stark/resources/string.h"
#include "engines/stark/resources/textureset.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/userinterface.h"

#include "common/random.h"

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
	case kScriptCall:
		return opScriptCall(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kDialogCall:
		return opDialogCall(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kSetInteractiveMode:
		return opSetInteractiveMode(_arguments[1].intValue);
	case kLocationGoTo:
		return opLocationGoTo(_arguments[0].stringValue, _arguments[1].stringValue, _arguments[2].referenceValue, _arguments[3].intValue);
	case kScriptPause:
		return opScriptPause(script, _arguments[1].referenceValue);
	case kWalkTo:
		return opWalkTo(script, _arguments[2].referenceValue, _arguments[3].intValue);
	case kGameLoop:
		return opScriptPauseGameLoop(script, _arguments[1].intValue);
	case kScriptPauseRandom:
		return opScriptPauseRandom(script, _arguments[1].referenceValue);
	case kExit2DLocation:
		return opExit2DLocation(script);
	case kGoto2DLocation:
		return opGoto2DLocation(_arguments[0].stringValue, _arguments[1].stringValue);
	case kRumbleScene:
		return opRumbleScene(_arguments[1].intValue, _arguments[2].intValue);
	case kFadeScene:
		return opFadeScene(_arguments[1].intValue, _arguments[2].intValue, _arguments[3].intValue);
	case kItem3DPlaceOn:
		return opItem3DPlaceOn(_arguments[1].referenceValue, _arguments[2].referenceValue);
	case kItem3DWalkTo:
		return opItem3DWalkTo(script, _arguments[1].referenceValue, _arguments[2].referenceValue, _arguments[3].intValue);
	case kItemLookAt:
		return opItemLookAt(script, _arguments[1].referenceValue, _arguments[2].referenceValue, _arguments[3].intValue,	_arguments[4].intValue);
	case kItemEnable:
		return opItemEnable(_arguments[1].referenceValue, _arguments[2].intValue);
	case kItemSelectInInventory:
		return opItemSelectInInventory(_arguments[1].referenceValue);
	case kItemSetActivity:
		return opItemSetActivity(_arguments[1].referenceValue, _arguments[2].intValue, _arguments[3].intValue);
	case kUseAnimHierarchy:
		return opUseAnimHierachy(_arguments[1].referenceValue);
	case kPlayAnimation:
		return opPlayAnimation(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kShowPlay:
		return opShowPlay(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kScriptEnable:
		return opScriptEnable(_arguments[1].referenceValue, _arguments[2].intValue);
	case kKnowledgeSetBoolean:
		return opKnowledgeSetBoolean(_arguments[1].referenceValue, _arguments[2].intValue);
	case kKnowledgeSetInteger:
		return opKnowledgeSetInteger(_arguments[1].referenceValue, _arguments[2].intValue);
	case kKnowledgeAddInteger:
		return opKnowledgeAddInteger(_arguments[1].referenceValue, _arguments[2].intValue);
	case kEnableFloorField:
		return opEnableFloorField(_arguments[1].referenceValue, _arguments[2].intValue);
	case kPlayAnimScriptItem:
		return opPlayAnimScriptItem(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kKnowledgeAssignBool:
		return opKnowledgeAssignBool(_arguments[1].referenceValue, _arguments[2].referenceValue);
	case kKnowledgeAssignNegatedBool:
		return opKnowledgeAssignNegatedBool(_arguments[1].referenceValue, _arguments[2].referenceValue);
	case kSoundPlay:
		return opSoundPlay(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kItemLookDirection:
		return opItemLookDirection(script, _arguments[1].referenceValue, _arguments[2].intValue, _arguments[3].intValue);
	case kStopPlayingSound:
		return opStopPlayingSound(_arguments[1].referenceValue);
	case kGoLayer:
		return opGoLayer(_arguments[1].referenceValue);
	case kScrollSet:
		return opScrollSet(_arguments[1].referenceValue);
	case kPlayFullMotionVideo:
		return opPlayFullMotionVideo(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kEnableDiaryEntry:
		return opEnableDiaryEntry(_arguments[1].referenceValue);
	case kPATChangeTooltip:
		return opPATChangeTooltip(_arguments[1].referenceValue, _arguments[2].referenceValue);
	case kChangeSound:
		return opChangeSound(_arguments[1].referenceValue, _arguments[2].intValue, _arguments[3].intValue, _arguments[4].intValue, _arguments[5].intValue);
	case kItem3DRunTo:
		return opItem3DRunTo(_arguments[1].referenceValue, _arguments[2].referenceValue, _arguments[3].intValue);
	case kItemPlaceDirection:
		return opItemPlaceDirection(_arguments[1].referenceValue, _arguments[2].intValue);
	case kActivateTexture:
		return opActivateTexture(_arguments[1].referenceValue);
	case kActivateMesh:
		return opActivateMesh(_arguments[1].referenceValue);
	case kSetTarget:
		return opSetTarget(_arguments[1].referenceValue, _arguments[2].referenceValue);
	case kSpeakWithoutTalking:
		return opSpeakWithoutTalking(script, _arguments[1].referenceValue, _arguments[2].intValue);
	case kIsOnFloorField:
		return opIsOnFloorField(_arguments[2].referenceValue, _arguments[3].referenceValue);
	case kIsItemEnabled:
		return opIsItemEnabled(_arguments[2].referenceValue);
	case kIsScriptEnabled:
		return opIsScriptEnabled(_arguments[2].referenceValue);
	case kIsSet:
		return opIsSet(_arguments[2].referenceValue);
	case kIsIntegerInRange:
		return opIsIntegerInRange(_arguments[2].referenceValue, _arguments[3].intValue, _arguments[4].intValue);
	case kIsIntegerAbove:
		return opIsIntegerAbove(_arguments[2].referenceValue, _arguments[3].intValue);
	case kIsIntegerEqual:
		return opIsIntegerEqual(_arguments[2].referenceValue, _arguments[3].intValue);
	case kIsIntegerLower:
		return opIsIntegerLower(_arguments[2].referenceValue, _arguments[3].intValue);
	case kIsScriptActive:
		return opIsScriptActive(_arguments[2].referenceValue);
	case kIsRandom:
		return opIsRandom(_arguments[2].intValue);
	case kIsOnPlace:
		return opIsOnPlace(_arguments[2].referenceValue, _arguments[3].referenceValue);
	case kIsOnNearPlace:
		return opIsOnNearPlace(_arguments[2].referenceValue, _arguments[3].referenceValue, _arguments[4].intValue);
	case kIsAnimPlaying:
		return opIsAnimPlaying(_arguments[2].referenceValue);
	case kIsItemActivity:
		return opIsItemActivity(_arguments[2].referenceValue, _arguments[3].intValue);
	case kIsAnimAtTime:
		return opIsAnimAtTime(_arguments[2].referenceValue, _arguments[3].intValue);
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

Command *Command::opScriptCall(Script *script, const ResourceReference &scriptRef, int32 synchronous) {
	Script *calleeScript = scriptRef.resolve<Script>();

	if (synchronous) {
		// Store the current command for use when the callee's execution ends
		script->addReturnObject(this);

		// Run the callee with the current script's execution context
		return calleeScript->getBeginCommand();
	} else {
		// Kickstart the callee script by skipping its Begin command, overriding all checks
		// Both scripts will continue in parallel
		calleeScript->goToNextCommand();
		return nextCommand();
	}
}

Command *Command::opDialogCall(Script *script, const ResourceReference &dialogRef, int32 suspend) {
	Dialog *dialog = dialogRef.resolve<Dialog>();
	StarkDialogPlayer->run(dialog);

	if (suspend) {
		script->suspend(dialog);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opSetInteractiveMode(bool enabled) {
	StarkUserInterface->setInteractive(enabled);

	return nextCommand();
}

Command *Command::opLocationGoTo(const Common::String &level, const Common::String &location, const ResourceReference &bookmarkRef, int32 direction) {
	uint levelIndex = strtol(level.c_str(), nullptr, 16);
	uint locationIndex = strtol(location.c_str(), nullptr, 16);
	StarkResourceProvider->requestLocationChange(levelIndex, locationIndex);
	StarkResourceProvider->setNextLocationPosition(bookmarkRef, direction);

	return nullptr;
}


Command *Command::opScriptPause(Script *script, const ResourceReference &durationRef) {
	Knowledge *duration = durationRef.resolve<Knowledge>();
	script->pause(duration->getIntegerValue());

	return this; // Stay on this command while the script is suspended
}

Command *Command::opWalkTo(Script *script, const ResourceReference &objectRef, int32 suspend) {
	Resources::ModelItem *april = StarkGlobal->getCurrent()->getInteractive();

	Math::Vector3d destinationPosition = getObjectPosition(objectRef);
	Math::Vector3d currentPosition = april->getPosition3D();

	if (destinationPosition == currentPosition) {
		return nextCommand();
	}

	Walk *walk = new Walk(april);
	walk->setDestination(destinationPosition);
	walk->start();

	april->setMovement(walk);

	if (suspend) {
		script->suspend(april);
		april->setMovementSuspendedScript(script);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opScriptPauseGameLoop(Script *script, int32 count) {
	uint gameloopDuration = StarkGlobal->getMillisecondsPerGameloop();

	script->pause(gameloopDuration * count);

	return this; // Stay on this command while the script is suspended
}

Command *Command::opScriptPauseRandom(Script *script, const ResourceReference &ref) {
	float randomFactor = StarkRandomSource->getRandomNumber(10000) / 10000.0;

	Knowledge *duration = ref.resolve<Knowledge>();
	script->pause(randomFactor * duration->getIntegerValue());

	return this; // Stay on this command while the script is suspended
}

Command *Command::opExit2DLocation(Script *script) {
	warning("(TODO: Implement) Exit 2D Location");

	StarkResourceProvider->returnToPushedLocation();

	return nullptr;
}

Command *Command::opGoto2DLocation(const Common::String &level, const Common::String &location) {
	warning("TODO: Implement) opGoto2DLocation");
	// TODO: This needs to be handled differently, to allow exiting.

	uint levelIndex = strtol(level.c_str(), nullptr, 16);
	uint locationIndex = strtol(location.c_str(), nullptr, 16);
	StarkResourceProvider->pushAndChangeLocation(levelIndex, locationIndex);

	return nullptr;
}

Command *Command::opRumbleScene(int32 unknown1, int32 unknown2) {
	warning("(TODO: Implement) opRumble(%d, %d)", unknown1, unknown2);

	return nextCommand();
}

Command *Command::opFadeScene(int32 unknown1, int32 unknown2, int32 unknown3) {
	warning("(TODO: Implement) opFadeScene(%d, %d, %d) : %s", unknown1, unknown2, unknown3, getName().c_str());

	return nextCommand();
}

Math::Vector3d Command::getObjectPosition(const ResourceReference &targetRef, int32 *floorFace) {
	Object *target = targetRef.resolve<Object>();
	Floor *floor = StarkGlobal->getCurrent()->getFloor();

	Math::Vector3d position;
	switch (target->getType().get()) {
		case Type::kBookmark: {
			Bookmark *bookmark = Object::cast<Bookmark>(target);
			position = bookmark->getPosition();

			if (floorFace) {
				*floorFace = floor->findFaceContainingPoint(position);
			}

	        break;
		}
		case Type::kItem: {
			FloorPositionedItem *item = Object::cast<FloorPositionedItem>(target);
			position = item->getPosition3D();

			if (floorFace) {
				*floorFace = item->getFloorFaceIndex();
			}

			break;
		}
		default:
			warning("Unimplemented getObjectPosition target type %s", target->getType().getName());
	}

	return position;
}

Command *Command::opItem3DPlaceOn(const ResourceReference &itemRef, const ResourceReference &targetRef) {
	FloorPositionedItem *item = itemRef.resolve<FloorPositionedItem>();

	int32 targetFace = -1;
	Math::Vector3d targetPosition = getObjectPosition(targetRef, &targetFace);

	item->setPosition3D(targetPosition);
	item->setFloorFaceIndex(targetFace);

	return nextCommand();
}

Command *Command::opItem3DWalkTo(Script *script, const ResourceReference &itemRef, const ResourceReference &targetRef, bool suspend) {
	FloorPositionedItem *item = itemRef.resolve<FloorPositionedItem>();
	Math::Vector3d targetPosition = getObjectPosition(targetRef);

	Walk *walk = new Walk(item);
	walk->setDestination(targetPosition);
	walk->start();

	item->setMovement(walk);

	if (suspend) {
		script->suspend(item);
		item->setMovementSuspendedScript(script);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opItemLookAt(Script *script, const ResourceReference &itemRef, const ResourceReference &objRef, bool suspend, int32 unknown) {
	FloorPositionedItem *item = itemRef.resolve<FloorPositionedItem>();
	Math::Vector3d currentPosition = item->getPosition3D();

	Math::Vector3d targetPosition = getObjectPosition(objRef);
	Math::Vector3d targetDirection = targetPosition - currentPosition;

	Turn *movement = new Turn(item);
	movement->setTargetDirection(targetDirection);
	movement->start();

	item->setMovement(movement);

	if (suspend) {
		script->suspend(item);
		item->setMovementSuspendedScript(script);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
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
	default:
		warning("Unhandled item enable command %d", enable);
		break;
	}

	return nextCommand();
}

Command *Command::opItemSetActivity(const ResourceReference &itemRef, int32 unknown1, int32 unknown2) {
	assert(_arguments.size() == 4);
	Object *itemObj = itemRef.resolve<Object>();
	warning("(TODO: Implement) opSetActivity(%s, %d, %d) : %s", itemObj->getName().c_str(), unknown1, unknown2, itemRef.describe().c_str());

	return nextCommand();
}

Command *Command::opItemSelectInInventory(const ResourceReference &itemRef) {
	InventoryItem *item = itemRef.resolve<InventoryItem>();
	StarkUserInterface->selectInventoryItem(item->getIndex());

	return nextCommand();
}

Command *Command::opUseAnimHierachy(const ResourceReference &animHierRef) {
	AnimHierarchy *animHier = animHierRef.resolve<AnimHierarchy>();
	Item *item = animHier->findParent<Item>();

	item->setAnimHierarchy(animHier);

	// TODO: Only set the anim usage if the next opcode is not going to do it
	ItemVisual *sceneItem = item->getSceneInstance();
	sceneItem->setAnimKind(Anim::kActorUsageIdle);

	return nextCommand();
}

Command *Command::opPlayAnimation(Script *script, const ResourceReference &animRef, bool suspend) {
	Anim *anim = animRef.resolve<Anim>();
	Item *item = anim->findParent<Item>();
	ItemVisual *sceneItem = item->getSceneInstance();

	sceneItem->setMovement(nullptr);
	sceneItem->playActionAnim(anim);

	// TODO: Check if the anim should reset the anim hirarchy upon completion

	if (suspend) {
		float animDuration = anim->getDuration();
		script->pause(animDuration);
		item->setMovementSuspendedScript(script);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opScriptEnable(const ResourceReference &scriptRef, int32 enable) {
	Script *script = scriptRef.resolve<Script>();

	bool previousState = script->isEnabled();

	switch (enable) {
		case 0:
			script->enable(false);
	        break;
		case 1:
			script->enable(true);
	        break;
		case 2:
			script->enable(!previousState);
	        break;
		default:
			warning("Unhandled script enable command %d", enable);
	        break;
	}

	return nextCommand();
}

Command *Command::opShowPlay(Script *script, const ResourceReference &ref, int32 suspend) {
	assert(_arguments.size() == 3);
	Speech *speechObj = ref.resolve<Speech>();
	assert(speechObj->getType().get() == Type::kSpeech);
	warning("(TODO: Implement) opShowPlay(%s %d) %s : %s", speechObj->getName().c_str(), suspend, speechObj->getPhrase().c_str(), ref.describe().c_str());

	StarkDialogPlayer->playSingle(speechObj);

	if (suspend) {
		script->suspend(speechObj);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opKnowledgeSetBoolean(const ResourceReference &knowledgeRef, int32 enable) {
	Knowledge *boolean = knowledgeRef.resolve<Knowledge>();

	bool previousState = boolean->getBooleanValue();

	switch (enable) {
		case 0:
			boolean->setBooleanValue(false);
	        break;
		case 1:
			boolean->setBooleanValue(true);
	        break;
		case 2:
			boolean->setBooleanValue(!previousState);
	        break;
		default:
			warning("Unhandled set boolean value command %d", enable);
	        break;
	}

	return nextCommand();
}

Command *Command::opKnowledgeSetInteger(const ResourceReference &knowledgeRef, int32 value) {
	Knowledge *knowledge = knowledgeRef.resolve<Knowledge>();

	knowledge->setIntegerValue(value);

	return nextCommand();
}

Command *Command::opKnowledgeAddInteger(const ResourceReference &knowledgeRef, int32 increment) {
	Knowledge *knowledge = knowledgeRef.resolve<Knowledge>();

	int32 oldValue = knowledge->getIntegerValue();
	knowledge->setIntegerValue(oldValue + increment);

	return nextCommand();
}

Command *Command::opEnableFloorField(const ResourceReference &floorFieldRef, int32 value) {
	assert(_arguments.size() == 3);
	Object *floorFieldObj = floorFieldRef.resolve<Object>();
	warning("(TODO: Implement) opEnableFloorField(%s, %d) : %s", floorFieldObj->getName().c_str(), value, floorFieldRef.describe().c_str());

	return nextCommand();
}

Command *Command::opPlayAnimScriptItem(Script *script, const ResourceReference &animScriptItemRef, int32 suspend) {
	AnimScriptItem *animScriptItem = animScriptItemRef.resolve<AnimScriptItem>();
	AnimScript *animScript = animScriptItem->findParent<AnimScript>();
	Anim *anim = animScriptItem->findParent<Anim>();
	Item *item = animScriptItem->findParent<Item>();
	ItemVisual *sceneItem = item->getSceneInstance();

	sceneItem->playActionAnim(anim);
	animScript->goToScriptItem(animScriptItem);

	if (suspend) {
		uint32 duration = animScript->getDurationStartingWithItem(animScriptItem);
		script->pause(duration);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opKnowledgeAssignBool(const ResourceReference &knowledgeRef1, const ResourceReference &knowledgeRef2) {
	Knowledge *src = knowledgeRef1.resolve<Knowledge>();
	Knowledge *dst = knowledgeRef2.resolve<Knowledge>();

	dst->setBooleanValue(src->getBooleanValue());

	return nextCommand();
}

Command *Command::opKnowledgeAssignNegatedBool(const ResourceReference &knowledgeRef1, const ResourceReference &knowledgeRef2) {
	Knowledge *src = knowledgeRef1.resolve<Knowledge>();
	Knowledge *dst = knowledgeRef2.resolve<Knowledge>();

	dst->setBooleanValue(!src->getBooleanValue());

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

Command *Command::opItemLookDirection(Script *script, const ResourceReference &itemRef, int32 direction, bool suspend) {
	FloorPositionedItem *item = itemRef.resolve<FloorPositionedItem>();

	Current *current = StarkGlobal->getCurrent();
	Camera *camera = current->getCamera();
	Math::Angle cameraAngle = camera->getHorizontalAngle();
	Math::Angle targetAngle = direction + cameraAngle;

	Math::Matrix3 rot;
	rot.buildAroundZ(-targetAngle);

	Math::Vector3d directionVector(1.0, 0.0, 0.0);
	rot.transformVector(&directionVector);

	Turn *movement = new Turn(item);
	movement->setTargetDirection(directionVector);
	movement->start();

	item->setMovement(movement);

	if (suspend) {
		script->suspend(item);
		item->setMovementSuspendedScript(script);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opStopPlayingSound(const ResourceReference &soundRef) {
	Sound *soundObj = soundRef.resolve<Sound>();

	soundObj->stop();

	return nextCommand();
}

Command *Command::opGoLayer(const ResourceReference &layerRef) {
	Object *layerObj = layerRef.resolve<Object>();
	warning("(TODO: Implement) opGoLayer(%s) : %s", layerObj->getName().c_str(), layerRef.describe().c_str());

	return nextCommand();
}

Command *Command::opScrollSet(const ResourceReference &scrollRef) {
	assert(_arguments.size() == 2);
	Object *scroll =  scrollRef.resolve<Object>();
	warning("(TODO: Implement) opScrollSet(%s) : %s", scroll->getName().c_str(), scrollRef.describe().c_str());

	return nextCommand();
}

Command *Command::opPlayFullMotionVideo(Script *script, const ResourceReference &movieRef, int32 unknown) {
	FMV *movie =  movieRef.resolve<FMV>();
	warning("(TODO: Implement) opPlayFullMotionVideo(%s) : %s - %d", movie->getName().c_str(), movieRef.describe().c_str(), unknown);
	StarkUserInterface->requestFMVPlayback(movie->getFilename());

	// Unconditional suspension
	script->suspend(movie);

	return this; // Stay on the same command while suspended
}

Command *Command::opEnableDiaryEntry(const ResourceReference &knowledgeRef) {
	assert(_arguments.size() == 2);
	Knowledge *entry = knowledgeRef.resolve<Knowledge>();
	warning("(TODO: Implement) opEnableDiaryEntry(%s) : %s", entry->getName().c_str(), knowledgeRef.describe().c_str());

	return nextCommand();
}

Command *Command::opPATChangeTooltip(const ResourceReference &patRef, const ResourceReference &stringRef) {
	PATTable *entry = patRef.resolve<PATTable>();
	String *string = stringRef.resolve<String>();

	entry->setTooltip(string);

	return nextCommand();
}

Command *Command::opChangeSound(const ResourceReference &soundRef, int32 unknown1, int32 unknown2, int32 unknown3, int32 unknown4) {
	assert(_arguments.size() == 6);
	Object *sound = soundRef.resolve<Object>();
	warning("(TODO: Implement) opChangeSound(%s, %d, %d, %d, %d) : %s", sound->getName().c_str(), unknown1, unknown2, unknown3, unknown4, soundRef.describe().c_str());
	return nextCommand();
}

Command *Command::opItem3DRunTo(const ResourceReference &itemRef, const ResourceReference &bookmarkRef, int32 unknown) {
	assert(_arguments.size() == 4);
	Object *itemObj = itemRef.resolve<Object>();
	Object *bookmarkObj = bookmarkRef.resolve<Object>();

	warning("(TODO: Implement) opItem3DRunTo(%s, %s, %d) %s : %s", itemObj->getName().c_str(), bookmarkObj->getName().c_str(),unknown, itemRef.describe().c_str(), bookmarkRef.describe().c_str());

	return nextCommand();
}

Command *Command::opItemPlaceDirection(const ResourceReference &itemRef, int32 direction) {
	FloorPositionedItem *item = itemRef.resolve<FloorPositionedItem>();

	Current *current = StarkGlobal->getCurrent();
	Camera *camera = current->getCamera();
	Math::Angle cameraAngle = camera->getHorizontalAngle();

	item->setDirection(direction + cameraAngle);

	return nextCommand();
}

Command *Command::opActivateTexture(const ResourceReference &textureRef) {
	TextureSet *texture = textureRef.resolve<TextureSet>();
	Item *item = texture->findParent<Item>();

	if (!item || (item->getSubType() != Item::kItemGlobalTemplate && item->getSubType() != Item::kItemLevelTemplate && item->getSubType() != Item::kItemModel)) {
		return nextCommand();
	}

	if (item->getSubType() == Item::kItemModel) {
		ModelItem *modelItem = Object::cast<ModelItem>(item);
		modelItem->setTexture(texture->getIndex(), texture->getSubType());
	} else {
		ItemTemplate *templateItem = Object::cast<ItemTemplate>(item);
		templateItem->setTexture(texture->getIndex(), texture->getSubType());
	}

	return nextCommand();
}

Command *Command::opActivateMesh(const ResourceReference &meshRef) {
	BonesMesh *mesh = meshRef.resolve<BonesMesh>();
	Item *item = mesh->findParent<Item>();

	if (!item || (item->getSubType() != Item::kItemGlobalTemplate && item->getSubType() != Item::kItemLevelTemplate && item->getSubType() != Item::kItemModel)) {
		return nextCommand();
	}

	if (item->getSubType() == Item::kItemModel) {
		ModelItem *modelItem = Object::cast<ModelItem>(item);
		modelItem->setBonesMesh(mesh->getIndex());
	} else {
		ItemTemplate *templateItem = Object::cast<ItemTemplate>(item);
		templateItem->setBonesMesh(mesh->getIndex());
	}

	return nextCommand();
}

Command *Command::opSetTarget(const ResourceReference &itemRef1, const ResourceReference &itemRef2) {
	Object *itemObj1 = itemRef1.resolve<Object>();
	Object *itemObj2 = itemRef2.resolve<Object>();
	warning("(TODO: Implement) opSetTarget(%s, %s) %s : %s", itemObj1->getName().c_str(), itemObj2->getName().c_str(), itemRef1.describe().c_str(), itemRef2.describe().c_str());

	return nextCommand();
}

Command *Command::opSpeakWithoutTalking(Script *script, const ResourceReference &speechRef, int32 suspend) {
	assert(_arguments.size() == 3);
	Speech *speech = speechRef.resolve<Speech>();
	warning("(TODO: Implement) opSpeakWithoutTalking(%s, %d) : %s", speech->getName().c_str(), suspend, speechRef.describe().c_str());

	// TODO: Complete
	StarkDialogPlayer->playSingle(speech);

	if (suspend) {
		script->suspend(speech);
		return this; // Stay on the same command while suspended
	} else {
		return nextCommand();
	}
}

Command *Command::opIsOnFloorField(const ResourceReference &itemRef, const ResourceReference &floorFieldRef) {
	FloorPositionedItem *item = itemRef.resolve<FloorPositionedItem>();
	FloorField *floorField = floorFieldRef.resolve<FloorField>();

	int32 itemFaceIndex = item->getFloorFaceIndex();
	bool itemOnFloorField = floorField->hasFace(itemFaceIndex);

	return nextCommandIf(itemOnFloorField);
}

Command *Command::opIsItemEnabled(const ResourceReference &itemRef) {
	Item *itemObj = itemRef.resolve<Item>();

	return nextCommandIf(itemObj->isEnabled());
}

Command *Command::opIsScriptEnabled(const ResourceReference &scriptRef) {
	Script *script = scriptRef.resolve<Script>();

	return nextCommandIf(script->isEnabled());
}

Command *Command::opIsSet(const ResourceReference &knowledgeRef) {
	Knowledge *value = knowledgeRef.resolve<Knowledge>();

	return nextCommandIf(value->getBooleanValue());
}

Command *Command::opIsIntegerInRange(const ResourceReference &knowledgeRef, int32 min, int32 max) {
	Knowledge *knowledgeValue = knowledgeRef.resolve<Knowledge>();
	int value = knowledgeValue->getIntegerValue();

	return nextCommandIf(value >= min && value <= max);
}

Command *Command::opIsIntegerAbove(const ResourceReference &knowledgeRef, int32 value) {
	Knowledge *knowledgeValue = knowledgeRef.resolve<Knowledge>();

	return nextCommandIf(knowledgeValue->getIntegerValue() > value);
}

Command *Command::opIsIntegerEqual(const ResourceReference &knowledgeRef, int32 value) {
	Knowledge *knowledgeValue = knowledgeRef.resolve<Knowledge>();

	return nextCommandIf(knowledgeValue->getIntegerValue() == value);
}

Command *Command::opIsIntegerLower(const ResourceReference &knowledgeRef, int32 value) {
	Knowledge *knowledgeValue = knowledgeRef.resolve<Knowledge>();

	return nextCommandIf(knowledgeValue->getIntegerValue() < value);
}

Command *Command::opIsScriptActive(const ResourceReference &scriptRef) {
	Script *script = scriptRef.resolve<Script>();

	return nextCommandIf(!script->isOnBegin());
}

Command *Command::opIsRandom(int32 chance) {
	int32 value = StarkRandomSource->getRandomNumber(100);

	return nextCommandIf(value < chance);
}

Command *Command::opIsOnPlace(const ResourceReference &itemRef, const ResourceReference &positionRef) {
	FloorPositionedItem *item = itemRef.resolve<FloorPositionedItem>();

	Math::Vector3d itemPostion = item->getPosition3D();
	Math::Vector3d testPosition = getObjectPosition(positionRef);

	return nextCommandIf(itemPostion == testPosition);
}

Command *Command::opIsOnNearPlace(const ResourceReference &itemRef, const ResourceReference &positionRef, int32 testDistance) {
	FloorPositionedItem *item = itemRef.resolve<FloorPositionedItem>();

	Math::Vector3d itemPostion = item->getPosition3D();
	Math::Vector3d testPosition = getObjectPosition(positionRef);

	return nextCommandIf(itemPostion.getDistanceTo(testPosition) < testDistance);
}

Command *Command::opIsAnimPlaying(const ResourceReference &animRef) {
	Anim *anim = animRef.resolve<Anim>();

	return nextCommandIf(anim->isInUse());
}

Command *Command::opIsItemActivity(const ResourceReference &itemRef, int32 value) {
	Item *item = itemRef.resolve<Item>();
	ItemVisual *sceneItem = item->getSceneInstance();

	return nextCommandIf(sceneItem->getAnimKind() == value);
}

Command *Command::opIsAnimAtTime(const ResourceReference &animRef, int32 time) {
	Anim *anim = animRef.resolve<Anim>();

	bool condition = anim->isInUse() && anim->isAtTime(time);

	return nextCommandIf(condition);
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
		case 0: // WORKAROUND: This case fixes what seems like a bug in the game data
			warning("Command argument type 0 found");
			argument.type = Argument::kTypeInteger1;
			argument.intValue = 0;
			break;
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
