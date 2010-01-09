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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef WAGE_SCENE_H
#define WAGE_SCENE_H

#include "common/list.h"
#include "common/rect.h"

#include "wage/designed.h"

namespace Wage {

class Script;
class Chr;
class Obj;

class Scene : public Designed {
public:
	enum Directions {
		NORTH = 0,
		SOUTH = 1,
		EAST = 2,
		WEST = 3
	};
	
	enum SceneTypes {
		PERIODIC = 0,
		RANDOM = 1
	};

	Script *_script;
	String _text;
	Common::Rect *_textBounds;
	int _fontSize;
	int _fontType; // 3 => Geneva, 22 => Courier, param to TextFont() function
	bool _blocked[4];
	String _messages[4];
	int _soundFrequency; // times a minute, max 3600
	int _soundType;
	String _soundName;
	int _worldX;
	int _worldY;
	
	Common::List<Obj> _objs;
	Common::List<Chr> _chrs;

	Scene() {}
	Scene(String name, byte *data);

	Common::Rect *getTextBounds() {
		return _textBounds == NULL ? NULL : new Common::Rect(*_textBounds);
	}

#if 0
	String getFontName() { 
		String[] fonts = {
			"Chicago",	// system font
			"Geneva",	// application font
			"New York",
			"Geneva",

			"Monaco",
			"Venice",
			"London",
			"Athens",
	
			"San Francisco",
			"Toronto",
			"Cairo",
			"Los Angeles", // 12

			null, null, null, null, null, null, null, // not in Inside Macintosh

			"Times", // 20
			"Helvetica",
			"Courier",
			"Symbol",
			"Taliesin" // mobile?
		};
		/*
mappings found on some forums:
systemFont(0):System(Swiss)
times(20):Times New Roman(Roman)
helvetica(21):Arial(Modern)
courier(22):Courier New(Modern)
symbol(23):Symbol(Decorative)
applFont(1):Arial(Swiss)
newYork(2):Times New Roman(Roman)
geneva(3):Arial(Swiss)
monaco(4):Courier New(Modern)
venice(5):Times New Roman(Roman)
london(6):Times New Roman(Roman)
athens(7):Times New Roman(Roman)
sanFran(8):Times New Roman(Roman)
toronto(9):Times New Roman(Roman)
cairo(11):Wingdings(Decorative)
losAngeles(12):Times New Roman(Roman)
taliesin(24):Wingdings(Decorative)
		 */
		if (fontType >= 0 && fontType < fonts.length && fonts[fontType] != null) {
			return _fonts[fontType];
		}
		return _"Unknown";
	}
#endif
};

} // End of namespace Wage
 
#endif
