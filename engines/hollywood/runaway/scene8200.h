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

#ifndef HOLLYWOOD_RUNAWAY_SCENE8200_H
#define HOLLYWOOD_RUNAWAY_SCENE8200_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene8200 : public RunawayRoomScene {
public:
	Scene8200(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	const Common::U32String &subtitle() const override;
	void sayInventory(const RunawayInventory::Reply &reply) override;

private:
	enum GangsterPose {
		kSeated, kRising, kSittingDown, kStanding, kApproaching,
		kAtBars, kRetreating, kTurningAside, kAside, kReturning
	};
	enum GangsterMovement { kIdle, kComeToBars, kLeaveBars };
	struct Gangster {
		GangsterPose pose = kSeated;
		uint16 step = 0;
		byte lastTaunt = 0xff;
		bool replyPending = false;
	};

	bool usesAlternateActor() const override;
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void stopAmbientSpeech() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool updateCustomSequence() override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	float actorHeight() const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	void findWalkPath(Common::Point target) override;
	Common::Point customizeSceneActionPosition(byte id, byte item, Common::Point position) const override;
	byte customizeSceneActionFacing(byte id, byte facing) const override;
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleAnimationFrameHook() override;

	void setGangsterPose(uint index, GangsterPose pose);
	void updateGangster(uint index);
	void moveGangsters(GangsterMovement movement);
	void startTaunt(uint index, byte variation);
	void showTauntLine();

	RunawayAnimation _cellDoorAnimation, _gangsterAnimation[2];
	Gangster _gangsters[2];
	GangsterMovement _gangsterMovement = kIdle;
	bool _gangstersInitialized = false;
	uint _gangsterElapsed = 0;
	RunawaySpeech _tauntSpeech;
	SceneSpeechCue _tauntCue;
	byte _tauntLine = 0, _tauntSpeaker = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE8200_H
