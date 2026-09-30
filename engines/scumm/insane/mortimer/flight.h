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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_INSANE_MORTIMER_FLIGHT_H
#define SCUMM_INSANE_MORTIMER_FLIGHT_H

#include "scumm/insane/mortimer/movie.h"

namespace Scumm {

class MortimerFlight : public MortimerMovie {
public:
	enum { kWidth = 384, kHeight = 288 };
	enum Weapon { kRevitascope, kSlimeGun };
	enum ShotResult { kMiss, kAnimalRescued, kHornetHit, kShellRestored };
	struct Target {
		uint16 id, toggle, remapToggle;
		uint rescuedAt;
		Common::Rect rect;
		Weapon hitWeapon;
		bool active, hit;
	};

	MortimerFlight();
	bool load(Common::SeekableReadStream &stream, bool interactive = true);
	void reset() override;
	bool advance() override;
	ShotResult shoot(const Common::Point &point);
	bool hitsObstacle(const Common::Point &point) const;

	uint rescuedCount() const { return _rescuedCount; }
	uint rescuedSpecies() const { return _rescuedSpecies; }
	Weapon weapon() const { return _weapon; }
	const Common::String &nextFlight() const { return _nextFlight; }
	const Common::Array<Target> &targets() const { return _targets; }
	bool interactive() const { return _interactive; }

private:
	struct Obstacle {
		Common::Point corners[4];
	};

	void handleAction(const byte *data, uint32 size) override;
	bool skipObject(uint16 a, uint16 b) const override;
	int objectScale(byte codec) const override;
	Target *findTarget(uint16 id);

	Common::Array<Target> _targets;
	Common::Array<Obstacle> _obstacles;
	Common::String _nextFlight;
	bool _toggles[256];
	bool _initialInteractive, _interactive;
	Weapon _weapon;
	uint _rescuedCount, _rescuedSpecies;
};

} // End of namespace Scumm

#endif
