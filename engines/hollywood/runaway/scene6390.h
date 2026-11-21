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

#ifndef HOLLYWOOD_RUNAWAY_SCENE6390_H
#define HOLLYWOOD_RUNAWAY_SCENE6390_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene6390 : public RunawayRoomScene {
public:
	Scene6390(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

	void click(Common::Point mouse, RunawayVerb verb, byte item) override;
	Common::U32String actionCaption(byte id, RunawayVerb verb, byte item) const override;
	bool canSave() const override { return false; }
	bool busy() const override;

private:
	Common::U32String hotspotName(byte id) const override;
	bool loadCustomResources() override;
	bool applyCustomBackgroundPatches() override;
	uint sceneTextNumber() const override { return 625; }
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool drawCustomComposite(Graphics::ManagedSurface &viewport) override;
	bool shouldUpdateActor() const override { return false; }
	bool isValidHotspot(byte id) const override { return id <= 4; }
	void turnDial(byte direction);
	void updateSlider(Common::Point mouse);
	void leaveDial();

	RunawayAnimation _dialAnimation, _handAnimation, _leverAnimation, _pointerAnimation;
	byte _hand = 0, _heldDirection = 0;
	byte _pointer = 120;
	int _dialTarget = -1;
	bool _sliderOpen = false, _autoTurn = false;
	uint32 _repeatAt = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE6390_H
