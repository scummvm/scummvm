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

#ifndef ULTIMA8_GAMES_GAMEDATA_H
#define ULTIMA8_GAMES_GAMEDATA_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/graphics/frame_id.h"

namespace Ultima {
namespace Ultima8 {

class RawArchive;
class MainShapeArchive;
class FontShapeArchive;
class GumpShapeArchive;
class ShapeArchive;
class Usecode;
class MapGlob;
class Shape;
class MusicFlex;
class WpnOvlayDat;
class ShapeFrame;
class SoundFlex;
class SpeechFlex;
struct GameInfo;

class GameData {
public:
	GameData(GameInfo *gameinfo);
	~GameData();

	static GameData *get_instance() {
		return gamedata;
	}

	void loadU8Data();
	void loadRemorseData();
	void setupFontOverrides();

	Usecode *getMainUsecode() const {
		return mainusecode;
	}
	MainShapeArchive *getMainShapes() const {
		return mainshapes;
	}
	RawArchive *getFixed() const {
		return fixed;
	}
	MapGlob *getGlob(uint32 glob) const;
	FontShapeArchive *getFonts() const {
		return fonts;
	}
	GumpShapeArchive *getGumps() const {
		return gumps;
	}
	Shape *getMouse() const {
		return mouse;
	}
	MusicFlex *getMusic() const {
		return music;
	}
	WpnOvlayDat *getWeaponOverlay() const {
		return weaponoverlay;
	}
	SoundFlex *getSoundFlex() const {
		return soundflex;
	}
	SpeechFlex *getSpeechFlex(uint32 shapenum);

	ShapeArchive *getShapeFlex(uint16 flexId) const;
	Shape *getShape(FrameID frameid) const;
	ShapeFrame *getFrame(FrameID frameid) const;

	std::string translate(std::string text);
	FrameID translate(FrameID frame);

	enum ShapeFlexId {
		OTHER       = 0,
		MAINSHAPES  = 1,
		GUMPS       = 2
	};
private:
	void loadTranslation();
	void setupTTFOverrides(const char *configkey, bool SJIS);
	void setupJPOverrides();

	RawArchive *fixed;
	MainShapeArchive *mainshapes;
	Usecode *mainusecode;
	std::vector<MapGlob *> globs;
	FontShapeArchive *fonts;
	GumpShapeArchive *gumps;
	Shape *mouse;
	MusicFlex *music;
	WpnOvlayDat *weaponoverlay;

	SoundFlex *soundflex;
	std::vector<SpeechFlex **> speech;
	GameInfo *gameinfo;

	static GameData *gamedata;
};

#define _TL_(x) (GameData::get_instance()->translate(x))
#define _TL_SHP_(x) (GameData::get_instance()->translate(x))

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
