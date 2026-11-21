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

#ifndef HOLLYWOOD_RUNAWAY_SCENE7010_H
#define HOLLYWOOD_RUNAWAY_SCENE7010_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene7010 : public RunawayRoomScene {
public:
	Scene7010(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void runCustomEntrySequence() override;
	void finishCustomFilm() override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleAnimationFrameHook() override;
	bool usesAlternateActor() const override { return false; }
	byte initialActorFacing() const override { return 1; }
	float actorHeight() const override;
	void findWalkPath(Common::Point target) override;
	int sceneViewportYOffset() const override { return 0; }
	bool adjustCustomWalkTargetToFloorMask(Common::Point &target) const override;
	int8 customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const override;
	Common::Point customizeSceneActionPosition(byte id, byte item, Common::Point position) const override;
	byte customizeSceneActionFacing(byte id, byte facing) const override;
	Common::Point sceneActionLookPosition(byte id) const override;
	Common::U32String hotspotName(byte id) const override;

	RunawayAnimation _bats, _nail, _anchor, _lowerRope;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE7010_H
