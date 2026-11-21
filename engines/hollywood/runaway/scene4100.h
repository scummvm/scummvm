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

#ifndef HOLLYWOOD_RUNAWAY_SCENE4100_H
#define HOLLYWOOD_RUNAWAY_SCENE4100_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene4100 : public RunawayRoomScene {
public:
	Scene4100(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void finishCustomAnimation(uint16 action) override;
	void handleAnimationFrameHook() override;
	void finishCustomFilm() override;
	void handleCustomDialogueChoice(byte effect) override;
	void finishCustomConversation() override;
	void runExitSideEffectsAfterLoop() override;
	float actorHeight() const override;
	Common::U32String hotspotName(byte id) const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	int8 customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const override;
	Common::Point customizeSceneActionPosition(byte id, byte item, Common::Point position) const override;
	byte customizeSceneActionFacing(byte id, byte facing) const override;

	void closeChamber();
	void startComputerConversation();

	RunawayAnimation _ambientAnimation, _overlayAnimation;
	RunawayAnimation _leftMachineAnimation, _rightMachineAnimation, _cliveAnimation;
	RunawayAnimation _computerAnimation, _rejectionLight;
	uint32 _machineTick = 0;
	uint16 _machinePosition[2] = {0, 0};
	byte _computerFrame = 48;
	bool _cliveVisible = false, _computerReject = false;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE4100_H
