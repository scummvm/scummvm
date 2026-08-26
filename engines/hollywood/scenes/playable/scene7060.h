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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7060_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7060_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene7060 : public PlayableScene {
public:
	Scene7060(HollywoodEngine *vm);

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
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void initializeChunk6FrameMap();
	void applyChunk6FrameMapForInventoryState();
	void applyChunk6KeyTakenFrameMap();
	void rebuildWalkableMask();
	void setColorMapItem8Promoted(bool promoted);
	void advanceChunk6IdleAndMachineFrame(uint32 delta);
	void waitForMachineIdleBeforeDialogue();
	void initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void runDialogueMenuRow98();
	void beginPrimaryDialogueSpeech(byte frameIndex);
	void beginPrimaryBrunoSpeechLine(uint16 rowIndex, byte frameIndex);
	void runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int soundFrame = -1, byte soundId = 0);
	void handleSpeechRow04Variant();
	void handleSpeechRow04Or06();
	void handleChunk9Or10MachineAction();
	void handleShortExitToState7071();
	void handleChunk7PickupItem11();
	void handleChunk9ExitToG07();
	void handleChunk10SpeechAction();
	void handleUseItem0DOnMachine();

	Common::Array<byte> _chunk6FrameMap;
	byte _chunk8FrameIndex;
	bool _chunk6RandomIdlePaused;
	bool _colorMapItem8Promoted;
	RandomIdleAnimation _chunk6Animation;
};

} // End of namespace Hollywood

#endif
