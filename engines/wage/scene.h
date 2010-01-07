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

private:
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

public:
	int getSoundFrequency() { return _soundFrequency; }
	void setSoundFrequency(int soundFrequency) { _soundFrequency = soundFrequency; }

	Common::Rect *getTextBounds() {
		return _textBounds == NULL ? NULL : new Common::Rect(*_textBounds);
	}

	void setTextBounds(Common::Rect bounds) { _textBounds = new Common::Rect(bounds); }
	
	void setDirMessage(int dir, String message) { _messages[dir] = message; }
	String getDirMessage(int dir) { return _messages[dir]; }

	void setDirBlocked(int dir, bool blocked) { _blocked[dir] = blocked; }
	bool isDirBlocked(int dir) { return _blocked[dir]; }

	String getText() { return _text; }
	void setText(String text) { _text = text; }

	Script *getScript() { return _script; }
	void setScript(Script *script) { _script = script; }

	int getSoundType() { return _soundType; }
	void setSoundType(int soundType) { _soundType = soundType; }

	int getWorldX() { return _worldX; }
	void setWorldX(int worldX) { _worldX = worldX; }

	int getWorldY() { return _worldY; }
	void setWorldY(int worldY) { _worldY = worldY; }

	String getSoundName() { return _soundName; }
	void setSoundName(String soundName) { _soundName = soundName; }

	int getFontSize() { return _fontSize; }
	void setFontSize(int fontSize) { _fontSize = fontSize; }

	int getFontType() { return _fontType; }
 
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

	void setFontType(int fontType) { _fontType = fontType; }

	Common::List<Chr> getChrs() { return _chrs; }
	Common::List<Obj> getObjs() { return _objs; }
};

} // End of namespace Wage
 
#endif
