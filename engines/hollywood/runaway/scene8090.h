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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8090_H
#define HOLLYWOOD_RUNAWAY_SCENE8090_H

#include "hollywood/runaway/oscar.h"
#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene8090 : public RunawayRoomScene {
public:
	Scene8090(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	bool busy() const override { return _entryScene == 8092 || RunawayRoomScene::busy(); }
	bool playingVideo() const override { return _entryScene == 8092 || RunawayRoomScene::playingVideo(); }
	void skipVideo() override;

private:
	enum ArrivalPhase { kNoArrival, kArrivalWait, kArrivalSpeech, kArrivalCurtain, kArrivalFadeOut };

	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool updateCustomSequence() override;
	bool drawCustomComposite(Graphics::ManagedSurface &viewport) override;
	void handleAnimationFrameHook() override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	bool roomAnimationInForeground() const override;
	float actorHeight() const override;
	byte initialActorFacing() const override;
	void findWalkPath(Common::Point target) override;
	int8 customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const override;
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	uint16 dialogueCue(byte choice, bool response) const override;
	void handleCustomDialogueChoice(byte effect) override;

	void drawOscar(Graphics::ManagedSurface &viewport, bool foreground);
	void startOscarConversation();
	void enterHouse(bool visitGina);
	void returnFromHouse();
	void describeGinaVisit();
	void setOscarPose(uint chunk, uint frames, byte pose);

	OscarAnimation _oscar;
	RunawayAnimation _oscarAnimation, _oscarPoseAnimation, _carryingGina;
	ArrivalPhase _arrivalPhase = kNoArrival;
	uint32 _arrivalStarted = 0;
	byte _oscarPose = 0, _oscarMouth = 0;
	bool _animatesOscar = false, _curtainOpen = false, _visitingGina = false;
	uint16 _houseDestination = 8100;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8090_H
