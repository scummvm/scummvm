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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6020_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6020_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene6020 : public PlayableScene {
public:
	Scene6020(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldAnimatePrimarySpeechLine() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetTaffyLayer();
	void advanceTaffyLayer(uint32 delta);
	void drawTaffyForegroundBlock();
	void drawForegroundBlocks(int activeWorldX, int activeWorldY);
	void setTaffyDeskMagnifierHidden(bool hidden);
	void applyTaffyDeskMagnifierPatch();
	void applyTaffyLeftSceneStatePatch();
	bool loadTaffyDepartureResource();
	void setTaffyDepartureFrame(uint32 baseOffset, uint16 descriptorCount, uint16 descriptorIndex);
	void clearTaffyDepartureFrame();
	void drawTaffyDepartureLayer();
	void rememberOriginalColorMap();
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void rebuildWorkingWalkableMask();
	void applyTaffyKnownSceneLabel();
	void runEntryFromScene6010();
	void runEntryFromScene6030();
	void runSmallObjectAnimation();
	void runPickupItem5A();
	void runPickupItem5B();
	void runPickupItem5E();
	void runUseItem39Overlay();
	void runUseItem64Overlay();
	void runTaffyRatHandoffAnimation();
	void runTaffyWalkOffAnimation();
	void runLateSceneObjectAnimation();
	void runFinalSceneObjectAnimation();
	void runDialogueAndMaybeEnterScene6030();
	void initializeTaffyDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setTaffyDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	void runTaffyLookUpTransition();
	void runTaffyFrameSequence(const byte *frames, uint frameCount);
	void runExitToScene6010();
	void runExitToScene6030();

	Common::Array<byte> _originalColorToItemMap;
	TimedAnimationChannel _taffyChannel;
	ResourceSpriteLayer _taffyLayer;
	byte _taffyAnimationState;
	byte _taffyHoldCounter;
	bool _taffyDeskMagnifierHidden;
	Common::Array<byte> _taffyDepartureResource;
	uint32 _taffyWalkOffResourceOffset;
	bool _taffyDepartureResourceLoaded;
	bool _taffyDepartureAnimationActive;
	bool _taffyDepartureFrameVisible;
	uint32 _taffyDepartureFrameBaseOffset;
	uint16 _taffyDepartureFrameDescriptorCount;
	uint16 _taffyDepartureFrameDescriptorIndex;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6020_H
