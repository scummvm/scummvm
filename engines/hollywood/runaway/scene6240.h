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

#ifndef HOLLYWOOD_RUNAWAY_SCENE6240_H
#define HOLLYWOOD_RUNAWAY_SCENE6240_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene6240 : public RunawayRoomScene {
public:
	Scene6240(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

protected:
	bool loadCustomResources() override;
	void handleAnimationFrameHook() override;
	RunawayAnimation _plantAnimation;

private:
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	bool usesAlternateActor() const override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void runCustomEntrySequence() override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	float actorHeight() const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	Common::U32String hotspotName(byte id) const override;
	int8 customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const override;
	void findWalkPath(Common::Point target) override;
	bool adjustCustomWalkTargetToFloorMask(Common::Point &target) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;

	Common::Array<byte> _plantPatches[3];
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE6240_H
