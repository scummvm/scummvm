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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8380_H
#define HOLLYWOOD_RUNAWAY_SCENE8380_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene8380 : public RunawayRoomScene {
public:
	Scene8380(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	enum SushiState {
		kSitDown, kTyping, kLeanBack, kResting, kTurnToBrian, kTalking,
		kReturnToComputer, kComputerGesture, kUseMicrophone, kMicrophoneSpeech, kLeaveMicrophone
	};

	bool usesAlternateActor() const override { return false; }
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	float actorHeight() const override;
	void findWalkPath(Common::Point target) override;
	bool adjustCustomWalkTargetToFloorMask(Common::Point &target) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleCustomDialogueChoice(byte effect) override;
	void finishCustomConversation() override;
	bool updateCustomSequence() override;

	void turnSushi(uint16 action, bool microphone = false);
	void returnToComputer();
	void advanceSushi();
	void exchange(uint16 cue, byte first, byte last);
	void startSushiConversation();
	void receiveRecordings();
	void deliverItem(uint16 action);
	void beginPhoneRange(uint first, uint last, uint16 nextAction);

	RunawayAnimation _sushi, _antenna, _monitor, _equipment[16];
	SushiState _sushiState = kTyping, _sushiGoal = kTalking;
	uint _sushiStep = 10;
	uint32 _sushiTick = 0;
	bool _waitingForSushi = false;
	bool _recorderOnDesk = false;
	RunawayVideo _phone;
	RunawayFilmPortrait _phonePortraits;
	bool _phoneActive = false, _phoneMoving = false, _gog = false;
	uint _phoneFirst = 0, _phoneLast = 0, _phonePortrait = 0;
	uint32 _phoneStarted = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8380_H
