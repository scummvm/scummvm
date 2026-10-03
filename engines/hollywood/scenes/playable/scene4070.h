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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4070_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4070_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene4070 : public PlayableScene {
public:
	Scene4070(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void prepareCustomComposite(bool drawActors, byte activeFacing, int activeWorldX,
		int activeWorldY, byte actorDrawOrderMode) override;
	void drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	void prepareCustomGameplayLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	void realtimeSpeechEnded(byte speechId, bool completed) override;

	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	byte primarySpeechAnimationFrameCount(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	byte primarySpeechVolumePercent(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;

	void resetAnimationLayers();
	bool isDraculaVisible() const;
	void setRightSidePatchActive(bool active, bool playSound);
	void advanceDraculaIdle(uint32 delta);
	void updateSidePatchForActorPosition();
	void rememberOriginalColorMap();
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void applyDraculaHotspotState();
	void beginDraculaSpeechLine(uint16 rowIndex, byte frameIndex);
	void beginDraculaIdleSpeechLine(byte frameIndex, bool alternatePose);
	void stopDraculaIdleSpeech();
	void beginTrophySpeechLine(uint16 rowIndex, byte frameIndex);
	void runCorridorExit();
	void runTrophyBaseOpenAction();
	void runFrankiePartGrantSequence();
	void runSlimmingTreatmentSequence();
	void runFlyerOnDracula();
	void runDraculaDialogue();
	void runLaterDraculaConversation();
	void initializeDraculaDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;

	TimedAnimationChannel _draculaIdleChannel;
	uint _ambientTrack;
	uint _randomAmbientTrack;
	uint32 _draculaIdleSpeechTimerAccumulator;
	bool _rightSidePatchActive;
	byte _draculaIdleState;
	bool _draculaThrowAnimationActive;
	bool _draculaDialogueMenuActive;
	SoundBank0Player _loopingSoundBank0;
	Common::Array<byte> _originalColorToItemMap;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4070_H
