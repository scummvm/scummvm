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

#ifndef HOLLYWOOD_RUNAWAY_SCENE6300_H
#define HOLLYWOOD_RUNAWAY_SCENE6300_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene6300 : public RunawayRoomScene {
public:
	Scene6300(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

protected:
	bool loadWorkshopForeground();
	void drawWorkshopForeground(Graphics::ManagedSurface &viewport);

private:
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void resumeAmbientAudio(bool entering) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool drawCustomComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	bool roomAnimationInForeground() const override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleCustomDialogueChoice(byte effect) override;
	void handleAnimationFrameHook() override;
	void finishCustomConversation() override;
	float actorHeight() const override;
	void setupActorDepth(RunawayActor::RenderParams &params) const override;
	void findWalkPath(Common::Point target) override;
	bool adjustCustomWalkTargetToFloorMask(Common::Point &target) const override;
	Common::U32String hotspotName(byte id) const override;
	Common::Point sceneActionLookPosition(byte id) const override;
	int8 customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const override;

	void startKevinConversation(bool introduction);
	void updateKevinTopics();
	void setKevinPose(uint chunk, uint frames, byte frame, byte talkFrame);
	void animateKevin(uint16 action, uint chunk, uint frames, uint first, uint last);
	void advanceKevinIdle();
	void drawKevin(Graphics::ManagedSurface &viewport);
	void drawCatapult(Graphics::ManagedSurface &viewport);
	void showCatapultImpact();

	RunawayAnimation _kevinAnimation, _catapultAnimation, _statueAnimation, _torchAnimation, _sculptureAnimation;
	RunawayAnimation _wrenchAnimation;
	Common::Array<byte> _foregroundPatches[2];
	Graphics::ManagedSurface _impactBackground;
	uint32 _kevinTick = 0;
	uint _kevinChunk = 0;
	byte _kevinFrame = 0, _kevinTalkFrame = 106;
	byte _kevinIdle = 0;
	bool _kevinTalking = false, _kevinAnimating = false, _catapultImpact = false;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE6300_H
