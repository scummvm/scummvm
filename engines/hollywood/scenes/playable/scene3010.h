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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3010_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3010_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene3010 : public PlayableScene {
public:
	Scene3010(HollywoodEngine *vm);

private:
	bool shouldLoadArenaChunk(uint index) const override;
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void advanceWindmillLayer(uint32 delta);
	void advanceForestIdleLayer(uint32 delta);
	void drawForegroundBlocks(int activeWorldY);
	void runEntryFromChapterStart();
	void runEntryFromScene3020();
	void runEntryFromPath();
	void runExitToScene3050();
	void runExitToScene3020();
	void runExitToScene3080();
	void runUmbrellaClimb();
	void runDepartureTransition(bool includeSecondClip);
	bool playDepartureClip(const Common::Array<byte> &clipData, uint tableEntryCount,
		Graphics::ManagedSurface &transitionBackground, bool skipWaits, uint32 firstFrameHoldMillis);
	bool waitDepartureFrame(uint32 millis, const Common::Array<byte> &clipData,
		uint tableEntryCount, byte frameIndex, Graphics::ManagedSurface &transitionBackground);
	void drawDepartureFrame(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::ManagedSurface &transitionBackground, bool applyFrame);
	void runWindmillClimbOverlay();

	TimedAnimationChannel _windmillChannel;
	TimedAnimationChannel _forestIdleChannel;
	ResourceSpriteLayer _windmillLayer;
	ResourceSpriteLayer _forestIdleLayer;
	byte _forestIdleState;
	bool _climbOverlayActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3010_H
