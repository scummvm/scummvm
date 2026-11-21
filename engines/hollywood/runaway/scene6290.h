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

#ifndef HOLLYWOOD_RUNAWAY_SCENE6290_H
#define HOLLYWOOD_RUNAWAY_SCENE6290_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene6290 : public RunawayRoomScene {
public:
	Scene6290(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

	void click(Common::Point mouse, RunawayVerb verb, byte item) override;
	byte hotspotAt(Common::Point mouse) const override;
	Common::U32String actionCaption(byte id, RunawayVerb verb, byte item) const override;
	bool canSave() const override;
	bool busy() const override;

private:
	const char *actionCaptionVariant(byte id, bool look, byte item) const override;
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	void applyCustomHotspotState() override;
	void prepareCustomGameplayLoop() override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) override;
	ActionResult dispatchCustomSceneAction(uint16 action) override;
	void handleCustomDialogueChoice(byte effect) override;
	void resumeAmbientAudio(bool entering) override;
	void findWalkPath(Common::Point target) override;
	float actorHeight() const override;
	Common::U32String hotspotName(byte id) const override;
	int8 customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const override;

	void startJoshuaConversation(bool introduction);
	void startKeyboard();
	void leaveKeyboard(bool review);
	void reviewMelody();
	void setJoshuaAnimation(uint chunk, uint frames);
	void handleAnimationFrameHook() override;
	void finishCustomFilm() override;
	bool shouldDrawActor() const override;
	bool shouldUpdateActor() const override;
	void finishCustomConversation() override;
	uint16 dialoguePlayerCue() const override;
	uint16 dialogueResponseCue() const override;

	RunawayAnimation _joshuaAnimation, _machineAnimation, _lightsAnimation;
	RunawayAnimation _keyboardBrian, _keyboardButtons, _keyboardPlatform;
	Common::Array<byte> _keyboardMask, _keyboardMaps;
	RunawayAnimation _machineAmbientAnimation, _machineOverlayAnimation, _machineIndicator;
	byte _joshuaFrame = 0;
	uint32 _joshuaTick = 0;
	bool _joshuaTalking = false, _joshuaAnimating = false, _joshuaDown = false;
	bool _keyboard = false, _reviewingMelody = false;
	bool _noteSoundPlayed = false;
	byte _notes[5] = {}, _noteCount = 0, _pressedNote = 0;
	uint _joshuaChunk = 10;
	uint32 _noteStarted = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE6290_H
