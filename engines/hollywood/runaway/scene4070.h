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

#ifndef HOLLYWOOD_RUNAWAY_SCENE4070_H
#define HOLLYWOOD_RUNAWAY_SCENE4070_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene4070 : public RunawayRoomScene {
public:
	Scene4070(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	bool loadCustomResources() override;
	void runCustomEntrySequence() override;
	void updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) override;
	void drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) override;
	void handleAnimationFrameHook() override;
	void finishCustomAnimation(uint16 action) override;
	bool handleCustomDialogueReply() override;
	void handleCustomDialogueChoice(byte effect) override;
	void runCustomDialogueIntro() override;
	void finishCustomConversation() override;
	uint sceneTextNumber() const override;

	void startWillyConversation();

	RunawayAnimation _willyAnimation;
	byte _willyPose = 54;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE4070_H
