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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8230_H
#define HOLLYWOOD_RUNAWAY_SCENE8230_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene8230 : public RunawayRoomScene {
public:
	Scene8230(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	bool canSave() const override { return false; }

private:
	uint sceneTextNumber() const override { return 624; }
	bool loadCustomResources() override;
	bool usesAlternateActor() const override { return false; }
	byte initialActorFacing() const override { return 7; }
	float actorHeight() const override;
	void findWalkPath(Common::Point target) override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool updateCustomSequence() override;
	bool shouldDrawActor() const override;
	bool drawCustomComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleCustomDialogueChoice(byte effect) override;
	void finishCustomConversation() override;
	void handleAnimationFrameHook() override;

	void moveRutger(uint first, uint last, uint idle);
	void startRutgerConversation();
	void startRutgerPuff();
	void startBrianPuff();

	RunawayAnimation _plants, _rutger, _pipeRutger, _pipeBrian, _smoke[2];
	uint _rutgerStep = 0, _rutgerLast = 0, _rutgerIdle = 40;
	uint32 _rutgerTick = 0;
	bool _movingRutger = false, _pipe = false, _brianPipePose = false;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8230_H
