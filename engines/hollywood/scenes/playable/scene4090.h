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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4090_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4090_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene4090 : public PlayableScene {
public:
	Scene4090(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldConvertSavedFramebufferFF() const override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;

	void resetAnimationLayers();
	void drawForegroundLayers(int activeWorldY, bool includeHighLayer);
	void rememberOriginalColorMap();
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void setSmallRowText(byte row, const char *text);
	void advanceAmbientSound(uint32 delta);
	void startOrganBodyAnimation(byte firstFrame, byte targetFrame, bool waitForSound);
	void advanceOrganBodyAnimation(uint32 delta);
	bool waitForOrganBodyAnimation();
	void setMultiSpriteLayersVisible(bool visible);
	void stopMultiSpriteAnimation();
	void runDoorExit();
	void runOrganRevealSequence();
	void runCoffinSwapSequence();
	bool runCoffinInsertSequence();
	bool playCoffinDeltaClip(uint chunkIndex);
	bool waitCoffinDeltaFrame(uint32 millis);
	bool advanceCoffinPaletteCycle(uint32 delta);
	void rotateCoffinPaletteCycle();
	bool runCurtainReveal(const Graphics::ManagedSurface &source,
		const Common::Array<byte> &palette);
	bool runCurtainClearToBlack();
	void applyCurtainBand(const Graphics::Surface *source, uint sweepOffset,
		byte bandWidth);
	void runFinalCutscene();

	SceneLayerStack _ambientLayers;
	ResourceSpriteLayer _scriptLayer;
	TimedAnimationChannel _chunk12Channel;
	TimedAnimationChannel _organBodyChannel;
	Common::Array<byte> _originalColorToItemMap;
	uint32 _ambientSoundTimerAccumulator;
	uint32 _coffinPaletteCycleAccumulator;
	byte _previousAmbientSoundCue;
	byte _organBodyTargetFrame;
	bool _organBodyAnimationActive;
	bool _organBodyWaitForSound;
	bool _randomAmbientAnimationActive;
	bool _multiSpriteCompositeActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4090_H
