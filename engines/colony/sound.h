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

#ifndef COLONY_SOUND_H
#define COLONY_SOUND_H

#include "audio/softsynth/pcspk.h"
#include "common/ptr.h"

namespace Colony {

class ColonyEngine;

class Sound {
public:
	Sound(ColonyEngine *vm);
	~Sound();

	void play(int soundID);
	void stop();

	enum {
		kKlaxon,
		kAirlock,
		kOuch,
		kChime,
		kBang,
		kShoot,
		kEat,
		kBonk,
		kBzzz,
		kExplode,
		kElevator,
		kPShot,
		kTest,
		kDit,
		kSink,
		kClatter,
		kStop,
		kTeleport,
		kSlug,
		kTunnel2,
		kLift,
		kDrop,
		kGlass,
		kDoor,
		kStars1,
		kStars2,
		kStars3,
		kStars4,
		kToilet,
		kBath
	};

private:
	ColonyEngine *_vm;
	Audio::PCSpeaker *_speaker;

	void playPCSpeaker(int soundID);
};

} // End of namespace Colony

#endif // COLONY_SOUND_H
