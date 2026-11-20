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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8240_H
#define HOLLYWOOD_RUNAWAY_SCENE8240_H

#include "hollywood/runaway/scene6240.h"

namespace Hollywood {
namespace Runaway {

// The return visit keeps the greenhouse's walking geometry and plant layers.
class Scene8240 : public Scene6240 {
public:
	Scene8240(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void runExitSideEffectsAfterLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool updateCustomSequence() override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	Common::U32String hotspotName(byte id) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleAnimationFrameHook() override;
	void finishCustomFilm() override;

	enum RutgerState { kDrumming, kStartScratching, kScratching, kStopScratching };
	enum Film { kNoFilm, kBranchCloseup, kLeaveAfterBrew };
	RunawayAnimation _rutger;
	RutgerState _rutgerState = kDrumming;
	Film _film = kNoFilm;
	uint _rutgerStep = 0;
	uint32 _rutgerTick = 0;
	bool _brew = false, _rutgerApproaching = false;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8240_H
