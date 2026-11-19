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

#ifndef HOLLYWOOD_RUNAWAY_SCENE5140_H
#define HOLLYWOOD_RUNAWAY_SCENE5140_H

#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene5140 : public RunawayRoomScene {
public:
	Scene5140(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);

private:
	bool loadCustomResources() override;
	void runCustomEntrySequence() override;
	void updateIntroduction();
	bool updateCustomSequence() override;
	void finishCustomFilm() override;
	bool drawCustomComposite(Graphics::ManagedSurface &viewport) override;
	bool shouldUpdateActor() const override;
	bool loadWalkMap() override;

	RunawayAnimation _brianAnimation, _carlaAnimation, _lulaAnimation, _mariolaAnimation;
	struct BusIntroduction {
		enum Phase { kArrival, kConversation, kFilm, kReturn, kFarewell };
		Phase phase = kArrival;
		uint32 tick = 0;
		byte nextLine = 0, frame = 0, lastFrame = 51;
		byte facing = 2, speaker = 6;
		bool blink = false;
	} _busIntroduction;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE5140_H
