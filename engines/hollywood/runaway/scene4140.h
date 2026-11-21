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

#ifndef HOLLYWOOD_RUNAWAY_SCENE4140_H
#define HOLLYWOOD_RUNAWAY_SCENE4140_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene4140 : public RunawayRoomScene {
public:
	Scene4140(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

	void click(Common::Point mouse, RunawayVerb verb, byte item) override;
	bool hasVerbControls() const override { return false; }
	Common::U32String actionCaption(byte id, RunawayVerb verb, byte item) const override;
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	bool canSave() const override;

private:
	Common::U32String hotspotName(byte id) const override;
	bool loadCustomResources() override;
	void runCustomEntrySequence() override;
	bool drawCustomComposite(Graphics::ManagedSurface &viewport) override;
	void finishCustomAnimation(uint16 action) override;
	bool shouldUpdateActor() const override;
	bool isValidHotspot(byte id) const override;

	Common::Array<byte> _keyPatches[25];
	byte _pressedKey = 0, _keyResult = 0;
	uint32 _keyUntil = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE4140_H
