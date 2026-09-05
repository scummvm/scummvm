
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

#ifndef ULTIMA2_LOGIC_CITY_CASTLE_LOGIC_H
#define ULTIMA2_LOGIC_CITY_CASTLE_LOGIC_H

#include "ultima/ultima1/logic/logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

class CityCastleLogic : public Logic {
protected:
	/**
	 * Load the NPCs for the given location
	 */
	void loadEntities();

	/**
	 * The Drop command is only usable within cities and castles
	 */
	void drop() override;

public:
	~CityCastleLogic() override {
	}

	void entering() override;
};

class CityLogic : public CityCastleLogic {
protected:
	/**
	 * Handle movement
	 */
	void move(Data::Direction dir) override;

	/**
	 * Checks if the player can move to a given location
	 */
	int checkAt(int x, int y) const;

public:
	CityLogic();
	~CityLogic() override {
	}

	void entering() override;
};

class CastleLogic : public CityCastleLogic {
protected:
	/**
	 * Handle movement
	 */
	void move(Data::Direction dir) override {
	}

public:
	CastleLogic();
	~CastleLogic() override {
	}

	void entering() override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
