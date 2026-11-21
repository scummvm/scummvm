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

#ifndef HOLLYWOOD_RUNAWAY_SCENE7050_H
#define HOLLYWOOD_RUNAWAY_SCENE7050_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene7050 : public RunawayRoomScene {
public:
	Scene7050(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void resumeAmbientAudio(bool entering) override;
	bool drawCustomComposite(Graphics::ManagedSurface &viewport) override;
	void updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	bool updateCustomSequence() override;
	bool usesAlternateActor() const override { return false; }
	byte initialActorFacing() const override { return 5; }
	float actorHeight() const override;
	void findWalkPath(Common::Point target) override;
	bool adjustCustomWalkTargetToFloorMask(Common::Point &target) const override;
	int8 customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const override;
	Common::U32String hotspotName(byte id) const override;

	Graphics::ManagedSurface _statue;
	bool _showStatue = false, _pickingUp = false;
	uint32 _pickupStarted = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE7050_H
