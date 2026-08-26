/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hollywood/scenes/playable/scene7050.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene7050ReturnToG04State = 0x1b81;
const int kScene7050EntryX = 0x0a1;
const int kScene7050EntryY = 0x158;
const byte kScene7050EntryFacing = 2;
const int kScene7050MinimumWalkX = 0x7a;
const int kScene7050MaximumWalkX = 0x2c0;
const uint16 kScene7050Chunk7DescriptorCount = 0x1b;
const uint16 kScene7050Chunk8DescriptorCount = 4;
const uint16 kScene7050Chunk11DescriptorCount = 0x0c;
const uint32 kScene7050FrameMillis = 75;
const byte kScene7050DialogueStageId = 0x62;
const byte kScene7050DialoguePrimaryRow = 99;
const byte kScene7050DialoguePrimaryRed = 6;
const byte kScene7050DialoguePrimaryGreen = 0x3f;
const byte kScene7050DialoguePrimaryBlue = 0x2d;
const uint16 kScene7050DialoguePrimaryCenterX = 0x1d6;
const uint16 kScene7050DialoguePrimaryTopY = 0x95;
const uint16 kScene7050DialoguePrimaryAltCenterX = 0x1cb;
const uint16 kScene7050DialoguePrimaryAltTopY = 0x96;
const byte kScene7050PrimarySpeechNormalGroup = 7;
const byte kScene7050PrimarySpeechAltGroup = 8;
const uint kScene7050DialogueChoiceRecordCount = 10 * 10 * 7;
const uint kScene7050ColorToItemMapOffset = 0x100;
const uint kScene7050ColorMapSize = 0x100;
const byte kScene7050CloakroomRagPickupHook = 1;
const byte kScene7050Chunk7FrameMap[] = {
	0, 0, 1, 2, 3, 25, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
	4, 18, 19, 20, 21, 22, 23, 24, 26, 20, 19, 18, 0
};
const byte kScene7050Chunk8ReturnFrameMap[] = { 0, 0, 1, 2, 3 };
const byte kScene7050Chunk11PickupItem10FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

static PlayableSceneConfig scene7050Config() {
	PlayableSceneConfig config(7050,
		SceneResourceLayout(12, 5, 11),
		SceneViewport(0x68),
		SceneActorPose(0x0a1, 0x158, 2));
	return config;
}

Scene7050::Scene7050(HollywoodEngine *vm) :
		PlayableScene(vm, scene7050Config()),
		_cloakroomAttendantRepeatCount(0),
		_cloakroomAttendantLayer() {
	_cloakroomAttendantAnimation.configure(kScene7050FrameMillis, 1, 5, 6, 0x0e, 0x0e, 0x31);
	_cloakroomAttendantAnimation.returnToIdleAfterLongSequence = false;
	_cloakroomAttendantLayer.configure(7, kScene7050Chunk7DescriptorCount,
		kScene7050Chunk7FrameMap, ARRAYSIZE(kScene7050Chunk7FrameMap));
	_cloakroomAttendantLayer.visible = true;
}

void Scene7050::initializeCustomPreviewState() {
	_actionOverlayPlayer.reset();
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_cloakroomAttendantRepeatCount = 0;
	_cloakroomAttendantAnimation.reset();
	_cloakroomAttendantLayer.reset(_cloakroomAttendantAnimation.channel.frameIndex);
	_cloakroomAttendantLayer.visible = true;
	setActiveActorPose(kScene7050EntryX, kScene7050EntryY, kScene7050EntryFacing);
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7050::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();

	_cloakroomAttendantLayer.setFrame(_cloakroomAttendantAnimation.channel.frameIndex);
	drawResourceSpriteLayer(_cloakroomAttendantLayer);

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	drawActionOverlayLayer();

	const uint blockChunk = activeWorldX < 0x1a4 ? 5 : 6;
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[blockChunk], _sceneFramebuffer);
}

void Scene7050::runCustomEntrySequence() {
	_soundBank0.playSample(4, 100);
	setActiveActorPose(kScene7050EntryX, kScene7050EntryY, kScene7050EntryFacing);
	_cloakroomAttendantAnimation.reset();
	drawPlayableComposite();
	presentFrame();
	waitSceneMillis(kScene7050FrameMillis);
}

bool Scene7050::prepareCustomGameplayLoop() {
	_cloakroomAttendantAnimation.channel.resetTimer();
	if (_cloakroomAttendantAnimation.channel.frameIndex == 0)
		_cloakroomAttendantAnimation.setFrame(1);
	return true;
}

bool Scene7050::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceSecondaryActorAnimation(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene7050::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, kScene7050MinimumWalkX, kScene7050MaximumWalkX);
	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _fullPaletteRegionMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		++targetY;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _fullPaletteRegionMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		--targetY;
	}
	return true;
}

bool Scene7050::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	GameplayState &state = _vm->gameState();
	bool textRowsChanged = false;

	if ((selector == 0 || selector == 0xff) && state.spokenToCloakroomAttendant &&
			_stage003SmallRows.size() >= 0xcd + kStage003SmallRowSize &&
			_stage003SmallRows.size() >= 0x52 + kStage003SmallRowSize) {
		const byte *source = _stage003SmallRows.data() + 0xcd;
		byte *destination = _stage003SmallRows.data() + 0x52;
		uint length = 0;
		while (length < kStage003SmallRowSize && source[length] != 0)
			++length;
		if (length < kStage003SmallRowSize)
			++length;
		memcpy(destination, source, length);
		textRowsChanged = true;
	}

	if (selector == 1 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		restoreBaseFramebufferFromOriginal();

		if (state.cloakroomRagVisible != 0) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
		} else {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
			if (_paletteMaskOriginal.size() >= kScene7050ColorToItemMapOffset + kScene7050ColorMapSize &&
					_paletteMask.size() >= kScene7050ColorToItemMapOffset + kScene7050ColorMapSize) {
				for (uint i = 0; i < kScene7050ColorMapSize; ++i) {
					if (_paletteMaskOriginal[kScene7050ColorToItemMapOffset + i] == 3)
						_paletteMask[kScene7050ColorToItemMapOffset + i] = 4;
				}
			}
		}

		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		textRowsChanged = false;
	}

	if (textRowsChanged)
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	return true;
}

byte Scene7050::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene7050PrimarySpeechAltGroup)
		return 0x24;
	return 1;
}

void Scene7050::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_cloakroomAttendantAnimation.setFrame(frameIndex);
}

bool Scene7050::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 306: // Unused G05 no-op slot.
	case 307: // Unused G05 no-op slot.
		return true;
	case 301: // Mirar puerta (look at door)
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar/Abrir puerta (use/open door)
		handleActionSlot01ReturnToG04();
		return true;
	case 303: // Hablar con empleado del guardarropa (talk to cloakroom attendant)
		runDialogueMenuRow98();
		applySceneStateToHotspotsAndPatches(0);
		return true;
	case 304: // Mirar empleado del guardarropa (look at cloakroom attendant)
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Mirar trapo (look at rag)
		beginSecondarySpeechLine(3, 0);
		return true;
	case 308: // Mirar caja (look at box): party horns / movement behind it.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 311: // Coger trapo (take rag)
		handleActionSlot10PickupItem10();
		return true;
	default:
		return false;
	}
}

void Scene7050::runDialogueMenuRow98() {
	Common::Array<DialogueChoiceRecord> records;
	initializeDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	GameplayState &state = _vm->gameState();
	if (!state.spokenToCloakroomAttendant) {
		beginSecondarySpeechLine(kScene7050DialogueStageId, 0);
		beginCloakroomAttendantSpeechLine(0, false);
		state.spokenToCloakroomAttendant = true;
	} else {
		beginSecondarySpeechLine(kScene7050DialogueStageId, 1);
		beginCloakroomAttendantSpeechLine(1, false);
	}

	while (!finished && !Engine::shouldQuit()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene7050DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene7050DialogueStageId, 4);
			beginCloakroomAttendantSpeechLine(4, false);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene7050DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			const bool alternatePose = record.transitionMode != 0;
			if (alternatePose)
				runSecondaryActorPoseIn();
			beginCloakroomAttendantSpeechLine(record.responseFrameIndex, alternatePose);
			if (alternatePose)
				runSecondaryActorPoseOut();
		}

		if (record.disableAfterUse == 1)
			record.enabled = 0;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			finished = true;
			break;
		case 1:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 1;
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 2;
			break;
		default:
			break;
		}
	}
}

void Scene7050::initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene7050DialogueChoiceRecordCount);

	records[0].enabled = 1;
	records[0].transitionMode = 3;
	records[0].playerTextRowId = 2;
	records[0].responseFrameIndex = 2;
	records[0].disableAfterUse = 1;
	records[0].reserved = 0xff;

	records[1].enabled = 1;
	records[1].transitionMode = 3;
	records[1].playerTextRowId = 3;
	records[1].responseFrameIndex = 3;
	records[1].disableAfterUse = 1;
	records[1].reserved = 0xff;

	records[2].enabled = 1;
	records[2].transitionMode = 0;
	records[2].playerTextRowId = 4;
	records[2].responseFrameIndex = 4;
	records[2].reserved = 0xff;
}

void Scene7050::runSecondaryActorPoseIn() {
	_cloakroomAttendantAnimation.setStateAndFrame(5, 0x20);
	playAnimationFrames(_cloakroomAttendantAnimation,
		AnimationFrameRange(0x21, 0x24, kScene7050FrameMillis));
	_cloakroomAttendantAnimation.setFrame(0x24);
}

void Scene7050::runSecondaryActorPoseOut() {
	_cloakroomAttendantAnimation.setFrame(0x28);
	playAnimationFrames(_cloakroomAttendantAnimation,
		AnimationFrameRange(0x29, 0x2c, kScene7050FrameMillis));
	_cloakroomAttendantAnimation.reset();
}

void Scene7050::beginCloakroomAttendantSpeechLine(byte frameIndex, bool alternatePose) {
	const byte group = alternatePose ? kScene7050PrimarySpeechAltGroup : kScene7050PrimarySpeechNormalGroup;
	beginPrimarySpeechLineWithAnimationGroup(kScene7050DialoguePrimaryRow, frameIndex,
		alternatePose ? kScene7050DialoguePrimaryAltCenterX : kScene7050DialoguePrimaryCenterX,
		alternatePose ? kScene7050DialoguePrimaryAltTopY : kScene7050DialoguePrimaryTopY,
		kScene7050DialoguePrimaryRed, kScene7050DialoguePrimaryGreen, kScene7050DialoguePrimaryBlue, group);
}

void Scene7050::handleActionSlot01ReturnToG04() {
	runActorReplacement(8, kScene7050Chunk8DescriptorCount, kScene7050Chunk8ReturnFrameMap,
		ARRAYSIZE(kScene7050Chunk8ReturnFrameMap), kScene7050FrameMillis);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene7050ReturnToG04State;
}

void Scene7050::handleActionSlot10PickupItem10() {
	dispatchGenericSceneAction(19);
	runActorReplacement(ActionOverlaySpec(11, kScene7050Chunk11DescriptorCount,
		kScene7050Chunk11PickupItem10FrameMap, ARRAYSIZE(kScene7050Chunk11PickupItem10FrameMap), kScene7050FrameMillis)
		.hookAt(4, kScene7050CloakroomRagPickupHook));
	addInventoryItem(0x10);
	_soundBank0.playSample(1, 100);
}

void Scene7050::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;

	if (hookId == kScene7050CloakroomRagPickupHook) {
		_vm->gameState().cloakroomRagVisible = 0;
		applySceneStateToHotspotsAndPatches(1);
	}
}

void Scene7050::advanceSecondaryActorAnimation(uint32 delta) {
	_cloakroomAttendantAnimation.channel.addDelta(delta);
	while (_cloakroomAttendantAnimation.channel.consumeFrame()) {
		switch (_cloakroomAttendantAnimation.state) {
		case 0:
		case 1:
		case 2:
			if (_cloakroomAttendantAnimation.advanceTick(_random) == RandomIdleAnimation::kLongFinished) {
				_cloakroomAttendantAnimation.setStateAndFrame(3, 0x0f);
				_cloakroomAttendantRepeatCount = (byte)(_random.getRandomNumber(3) + 2);
			}
			break;
		case 3:
			if (_cloakroomAttendantAnimation.channel.frameIndex == 0x17) {
				if (_cloakroomAttendantRepeatCount == 0) {
					_cloakroomAttendantAnimation.setStateAndFrame(4, 0x18);
				} else {
					_cloakroomAttendantAnimation.setFrame(0x0f);
					--_cloakroomAttendantRepeatCount;
				}
			} else {
				++_cloakroomAttendantAnimation.channel.frameIndex;
			}
			break;
		case 4:
			if (_cloakroomAttendantAnimation.channel.frameIndex == 0x20) {
				_cloakroomAttendantAnimation.reset();
			} else {
				++_cloakroomAttendantAnimation.channel.frameIndex;
			}
			break;
		default:
			break;
		}
	}
}

} // End of namespace Hollywood
