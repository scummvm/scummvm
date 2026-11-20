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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8420_H
#define HOLLYWOOD_RUNAWAY_SCENE8420_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene8420 : public RunawayRoomScene {
public:
	Scene8420(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	bool canSave() const override { return false; }

private:
	enum Film { kNoFilm, kWakingUp, kConfession, kNightmare };

	bool loadCustomResources() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	bool shouldDrawActor() const override { return _showBrian; }
	bool shouldPlayGameplayClickPath() const override { return false; }
	bool updateCustomSequence() override;
	float actorHeight() const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	uint16 dialoguePlayerCue() const override { return 40; }
	uint16 dialogueResponseCue() const override { return 50; }
	void handleCustomDialogueChoice(byte effect) override;
	void finishCustomConversation() override;
	void finishCustomFilm() override;

	void exchange(uint16 cue, byte first, byte last);
	void startGinaConversation();
	void startConfession();
	void startNightmareMovement(byte first, byte last, uint16 nextAction);
	void updateNightmareAudio();

	RunawayAnimation _gina, _ambient[2], _dreamGina, _johnny;
	Film _film = kNoFilm;
	uint32 _ambientTick = 0, _ginaTick = 0;
	byte _ambientFrame[2] = {}, _ginaFrame = 0, _johnnyFrame = 0;
	byte _nightmareLast = 0, _lastScream = 0;
	bool _showBrian = true, _waitingForGina = false, _dreaming = false, _screaming = false;
	bool _nightmareMoving = false;
	uint16 _destination = 8091;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8420_H
