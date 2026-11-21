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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8400_H
#define HOLLYWOOD_RUNAWAY_SCENE8400_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene8400 : public RunawayRoomScene {
public:
	Scene8400(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	bool loadCustomResources() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	float actorHeight() const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	void findWalkPath(Common::Point target) override;
	bool adjustCustomWalkTargetToFloorMask(Common::Point &target) const override;
	Common::U32String hotspotName(byte id) const override;
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	bool shouldDrawActor() const override { return !_cinematic; }
	bool shouldUpdateActor() const override { return !_cinematic; }
	bool updateCustomSequence() override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleAnimationFrameHook() override;

	void panTo(int x, uint step, uint16 action);
	void withdraw(uint16 action);
	void enterTrailer();

	RunawayAnimation _door, _scenery, _parkedCar, _arrivingCar, _brian, _gustav, _feodor;
	uint _sceneryFrame = 0, _doorFrame = 0;
	bool _sceneryBackwards = false, _carParked = false;
	uint32 _sceneryTick = 0, _panTick = 0, _castTick = 0;
	int _cameraTarget = -1;
	uint _panStep = 0;
	bool _cinematic = false, _carMoving = false, _gangstersEntering = false, _gangstersVisible = false;
	uint _carFrame = 30, _gustavFrame = 0, _feodorFrame = 0;
	int _brianPose = -1;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8400_H
