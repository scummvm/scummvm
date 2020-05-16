/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_MAP_SHRINE_H
#define ULTIMA4_MAP_SHRINE_H

#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/filesys/savegame.h"

namespace Ultima {
namespace Ultima4 {

#define SHRINE_MEDITATION_INTERVAL  100
#define MEDITATION_MANTRAS_PER_CYCLE 16

class Shrine : public Map {
public:
	Shrine();
	~Shrine() override {}

	// Methods
	Common::String  getName() override;
	Virtue getVirtue() const;
	Common::String getMantra() const;

	void setVirtue(Virtue v);
	void setMantra(Common::String mantra);

	/**
	 * Enter the shrine
	 */
	void enter();
	void enhancedSequence();
	void meditationCycle();
	void askMantra();
	void eject();
	void showVision(bool elevated);

	// Properties
private:
	Common::String _name;
	Virtue _virtue;
	Common::String _mantra;
};

class Shrines {
public:
	int _cycles, _completedCycles;
	Std::vector<Common::String> _advice;
public:
	/**
	 * Constructor
	 */
	Shrines();

	/**
	 * Destructor
	 */
	~Shrines();

	/**
	 * Returns true if advice data has been loaded
	 */
	bool isAdviceLoaded() const {
		return !_advice.empty();
	}

	/**
	 * Load advice
	 */
	void loadAdvice();
};

extern Shrines *g_shrines;

/**
 * Returns true if the player can use the portal to the shrine
 */
bool shrineCanEnter(const Portal *p);

/**
 * Returns true if 'map' points to a Shrine map
 */
bool isShrine(Map *punknown);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
