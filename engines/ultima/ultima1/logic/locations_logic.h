
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

#ifndef ULTIMA2_LOGIC_LOCATIONS_LOGIC_H
#define ULTIMA2_LOGIC_LOCATIONS_LOGIC_H

#include "ultima/ultima1/logic/logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

/**
 * Base class for gameplay handling logic used by the Commands class to respond to player actions
 * or other miscellaneous keypresses.
 */
class LocationsLogic : public Logic {
public:
	~LocationsLogic() override {
	}

	void action(int action) override;
	void keypress(Common::KeyCode keycode) override;
};

class CityLogic : public LocationsLogic {
public:
	~CityLogic() override {
	}

	void enter() override;
};

class CastleLogic : public LocationsLogic {
public:
	~CastleLogic() override {
	}

	void enter() override;
};

class PillarLogic : public LocationsLogic {
public:
	~PillarLogic() override {
	}

	void enter() override;
};

class DungeonLogic : public LocationsLogic {
public:
	~DungeonLogic() override {
	}

	void enter() override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
