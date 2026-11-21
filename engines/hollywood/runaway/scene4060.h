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

#ifndef HOLLYWOOD_RUNAWAY_SCENE4060_H
#define HOLLYWOOD_RUNAWAY_SCENE4060_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene4060 : public RunawayRoomScene {
public:
	Scene4060(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	byte actionCursor(byte id) const override;

private:
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	bool loadCustomResources() override;
	void applyCustomHotspotState() override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void finishCustomAnimation(uint16 action) override;
	void handleAnimationFrameHook() override;
	void finishCustomFilm() override;
	float actorHeight() const override;
	Common::U32String hotspotName(byte id) const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	bool usesAlternateActor() const override;
	int sceneViewportYOffset() const override;

	RunawayAnimation _willyAnimation;
	byte _willyFrame = 0;
	int _entranceCameraX = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE4060_H
