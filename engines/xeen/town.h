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

#ifndef XEEN_TOWN_H
#define XEEN_TOWN_H

#include "common/scummsys.h"
#include "common/str-array.h"
#include "xeen/dialogs.h"

namespace Xeen {

class XeenEngine;

class Town: public ButtonContainer {
private:
	XeenEngine *_vm;
	SpriteResource _icons1, _icons2;
	Common::StringArray _textStrings;
	Common::Array<SpriteResource> _townSprites;
	int _townMaxId;
	int _townActionId;
	int _townCurrent;
	int _v1;
	int _v2;
	Common::Point _townPos;
	int _arr1[6];

	void loadStrings(const Common::String &name);

	void pyramidEvent();

	void arenaEvent();

	void reaperEvent();

	void golemEvent();

	void sphinxEvent();

	void dwarfEvent();

	Common::String createTownText();

	void townWait();

	void doTownOptions();
public:
	Town(XeenEngine *vm);

	int townAction(int actionId);
};

} // End of namespace Xeen

#endif /* XEEN_SPELLS_H */
