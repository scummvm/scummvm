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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8100_H
#define HOLLYWOOD_RUNAWAY_SCENE8100_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene8100 : public RunawayRoomScene {
public:
	Scene8100(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	void draw() override;

private:
	enum Fade { kNoFade, kMediumTestFade, kSeanceFade };

	bool loadCustomResources() override;
	bool usesAlternateActor() const override;
	byte initialActorFacing() const override;
	float actorHeight() const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	void findWalkPath(Common::Point target) override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool updateCustomSequence() override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	bool drawCustomComposite(Graphics::ManagedSurface &viewport) override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleAnimationFrameHook() override;
	void handleCustomDialogueChoice(byte effect) override;
	void finishCustomFilm() override;

	void moveDorita(uint first, uint last, uint idle);
	void startDoritaConversation();
	void startMediumTest(byte test);
	void startPrayers();
	void leaveHouse();
	void startSeance();

	RunawayAnimation _dorita, _prayerAnimation;
	Graphics::ManagedSurface _altar;
	Fade _fade = kNoFade;
	uint _doritaStep = 20, _doritaLast = 20, _doritaIdle = 20;
	uint32 _doritaTick = 0, _ambientTick = 0, _fadeStarted = 0;
	byte _blink = 0, _mediumTest = 0, _prayer = 0, _lastAmbientEffect = 0;
	bool _movingDorita = false, _showAltar = false, _visitingGina = false, _seanceFilm = false;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8100_H
