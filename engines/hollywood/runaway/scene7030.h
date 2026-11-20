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

#ifndef HOLLYWOOD_RUNAWAY_SCENE7030_H
#define HOLLYWOOD_RUNAWAY_SCENE7030_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene7030 : public RunawayRoomScene {
public:
	Scene7030(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	bool loadCustomResources() override;
	void resumeAmbientAudio(bool entering) override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	void updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	bool shouldPlayGameplayClickPath() const override { return false; }
	byte initialActorFacing() const override { return 5; }
	float actorHeight() const override { return 12; }
	float actorWalkSpeed() const override { return RunawayRoomScene::actorWalkSpeed() * 5; }

	void advanceRoute();

	Common::Array<byte> _layers[3];
	RunawayAnimation _water[4];
	const Common::Point *_route = nullptr;
	uint _routeCount = 0, _routeStep = 0;
	byte _crossing = 0;
	uint _panElapsed = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE7030_H
