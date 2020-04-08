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

#ifndef ULTIMA4_SHRINE_H
#define ULTIMA4_SHRINE_H

#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/filesys/savegame.h"

namespace Ultima {
namespace Ultima4 {

#define SHRINE_MEDITATION_INTERVAL  100
#define MEDITATION_MANTRAS_PER_CYCLE 16

class Shrine : public Map {
public:
	Shrine();

	// Methods
	virtual Common::String  getName();
	Virtue          getVirtue() const;
	Common::String          getMantra() const;

	void            setVirtue(Virtue v);
	void            setMantra(Common::String mantra);

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

bool shrineCanEnter(const struct _Portal *p);
bool isShrine(Map *punknown);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
