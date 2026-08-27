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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2110_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2110_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2110 : public PlayableScene {
public:
	Scene2110(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	byte primarySpeechAnimationFrameCount(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void advanceAmbientLayer(uint32 delta);
	void advanceEntryIdle(uint32 delta);
	void startScriptedActorPath();
	void advanceScriptedActorPath(uint32 delta);
	void finishScriptedActorPath();
	void runEntryFromScene2100();
	void runScriptedReturnToScene2100();
	bool runScriptedEntryOpening();
	void runExitToScene2100();
	void runTreasureGrantAction();
	void runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel);
	void runEntrySecondarySpeechLine(byte frameIndex);
	void runEntryPrimarySpeechLine(byte frameIndex, byte animationGroup);
	void runTreasurePrimarySpeechLine(uint16 rowIndex, byte frameIndex);

	TimedAnimationChannel _ambientChannel;
	TimedAnimationChannel _entryIdleChannel;
	TimedAnimationChannel _scriptedActorPathChannel;
	ResourceSpriteLayer _entryLayer;
	ResourceSpriteLayer _ambientLayer;
	ResourceSpriteLayer _treasureLayer;
	bool _entryIdleActive;
	bool _scriptedActorPathActive;
	uint _scriptedActorPathFrameIndex;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2110_H
