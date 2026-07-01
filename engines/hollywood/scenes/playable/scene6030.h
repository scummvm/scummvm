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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6030_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6030_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene6030 : public PlayableScene {
public:
	Scene6030(HollywoodEngine *vm);

private:
	bool hasCustomPreviewState() const override;
	void initializeCustomPreviewState() override;
	bool hasCustomComposite() const override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool hasCustomEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldAnimatePrimarySpeechLine() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void rebuildWorkingWalkableMask();
	void resetAnimationLayers();
	void advanceHannoverLayer(uint32 delta);
	void advanceTaffyEntranceLayer(uint32 delta);
	void drawForegroundBlocks(int activeWorldX, int activeWorldY);
	void runEntryConversation();
	void runHannoverDialogueMenu();
	void initializeHannoverDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setHannoverDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	void beginHannoverSpeechLine(byte frameIndex, byte animationGroup = 0);
	uint32 beginStaticHannoverSpeechLine(byte frameIndex, uint16 centerX, uint16 topY);
	void runHannoverFrameRange(byte firstFrame, byte lastFrame, byte finalFrame, uint32 frameMillis);
	void runHannoverFrameRangeSequence(bool alternatePose);
	void runHannoverDeskWalkSequence();
	void runTaffyEnteringAnimation();
	void returnToScene6020();

	TimedAnimationChannel _largeBackgroundChannel;
	TimedAnimationChannel _taffyEntranceChannel;
	ResourceSpriteLayer _largeBackgroundLayer;
	ResourceSpriteLayer _smallForegroundLayer;
	ResourceSpriteLayer _taffyEntranceLayer;
	bool _hannoverManualSequenceActive;
	bool _hannoverLayerSuppressed;
	bool _taffyEntranceSequenceActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6030_H
