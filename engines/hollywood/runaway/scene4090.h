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

#ifndef HOLLYWOOD_RUNAWAY_SCENE4090_H
#define HOLLYWOOD_RUNAWAY_SCENE4090_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene4090 : public RunawayRoomScene {
public:
	Scene4090(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	void setCameraX(int x) override;

private:
	bool applyCustomBackgroundPatches() override;
	void runCustomEntrySequence() override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	float actorHeight() const override;
	Common::U32String hotspotName(byte id) const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE4090_H
