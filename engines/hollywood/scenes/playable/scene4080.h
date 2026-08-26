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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4080_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4080_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene4080 : public PlayableScene {
public:
	Scene4080(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;

	void resetAnimationLayers();
	void configurePalettePatchLayerForState();
	void advancePalettePatchLayer(uint32 delta);
	void advanceForegroundFlickerLayer(uint32 delta);
	void drawSceneLayers(int activeWorldY);
	void rememberOriginalColorMap();
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void copySmallTextRow(byte destinationRow, byte sourceRow);
	void beginGwendolynSpeechLine(uint16 rowIndex, byte frameIndex);
	void runCorridorExit();
	void runSidePatchSequence();
	void runBottlePickupSequence();
	void runGominolaPickupSequence();
	void runSteakPickupSequence();
	void runUseMabusePillsOnFoodBags();
	void runUseStakeOnGwendolyn();
	void runGwendolynScriptedReply(uint16 secondaryRow);
	void runGwendolynDialogue();
	void initializeGwendolynDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;

	ResourceSpriteLayer _palettePatchLayer;
	ResourceSpriteLayer _foregroundFlickerLayer;
	ResourceSpriteLayer _scriptLayer;
	TimedAnimationChannel _palettePatchChannel;
	TimedAnimationChannel _foregroundFlickerChannel;
	Common::Array<byte> _originalColorToItemMap;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4080_H
