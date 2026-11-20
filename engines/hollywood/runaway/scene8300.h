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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8300_H
#define HOLLYWOOD_RUNAWAY_SCENE8300_H

#include "hollywood/runaway/scene6300.h"

namespace Hollywood {
namespace Runaway {

// The workshop retains its walking geometry, depth layers and parallax scenery.
class Scene8300 : public Scene6300 {
public:
	Scene8300(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool updateCustomSequence() override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleCustomDialogueChoice(byte effect) override;
	void finishCustomConversation() override;
	uint16 dialoguePlayerCue() const override { return 81; }
	uint16 dialogueResponseCue() const override { return 91; }
	Common::U32String hotspotName(byte id) const override;

	void advanceSaturn();
	void drawSaturn(Graphics::ManagedSurface &viewport);
	void startSaturnConversation();

	enum SaturnState { kWorking, kStartGesture, kGesture, kFinishGesture, kChiseling, kTurning, kTalking, kReturning };
	RunawayAnimation _saturn;
	SaturnState _saturnState = kWorking;
	uint32 _saturnTick = 0;
	byte _saturnStep = 0, _sculptureStage = 0;
	bool _waitingForSaturn = false;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8300_H
