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

#ifndef ULTIMA8_GAMES_GAMEDATA_H
#define ULTIMA8_GAMES_GAMEDATA_H

#include "common/array.h"
#include "common/str.h"
#include "ultima/ultima8/gfx/frame_id.h"

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
class NPCDat;
class CombatDat;
class FireType;
class ShapeFrame;
class WeaselDat;
class SoundFlex;
class SpeechFlex;
struct GameInfo;

class GameData {
public:
	GameData(GameInfo *gameinfo);
	~GameData();

	static GameData *get_instance() {
		return _gameData;
	}

	void loadU8Data();
	void loadRemorseData();
	void setupFontOverrides();

	Usecode *getMainUsecode() const {
		return _mainUsecode;
	}
	MainShapeArchive *getMainShapes() const {
		return _mainShapes;
	}
	RawArchive *getFixed() const {
		return _fixed;
	}
	MapGlob *getGlob(uint32 glob) const;
	FontShapeArchive *getFonts() const {
		return _fonts;
	}
	GumpShapeArchive *getGumps() const {
		return _gumps;
	}
	Shape *getMouse() const {
		return _mouse;
	}
	MusicFlex *getMusic() const {
		return _music;
	}
	WpnOvlayDat *getWeaponOverlay() const {
		return _weaponOverlay;
	}
	SoundFlex *getSoundFlex() const {
		return _soundFlex;
	}
	SpeechFlex *getSpeechFlex(uint32 shapenum);

	ShapeArchive *getShapeFlex(uint16 flexId) const;
	Shape *getShape(FrameID frameid) const;
	const ShapeFrame *getFrame(FrameID frameid) const;

	const NPCDat *getNPCData(uint16 entry) const;
	const NPCDat *getNPCDataForShape(uint16 shapeno) const;

	const CombatDat *getCombatDat(uint16 entry) const;

	const FireType *getFireType(uint16 type) const;

	const WeaselDat *getWeaselDat(uint16 level) const;

	Common::String translate(const Common::String &text);
	FrameID translate(FrameID frame);

	enum ShapeFlexId {
		OTHER       = 0,
		MAINSHAPES  = 1,
		GUMPS       = 2
	};
private:
	void loadTranslation();
	void setupTTFOverrides(const char *category, bool SJIS);
	void setupJPOverrides();

	RawArchive *_fixed;
	MainShapeArchive *_mainShapes;
	Usecode *_mainUsecode;
	Common::Array<MapGlob *> _globs;
	FontShapeArchive *_fonts;
	GumpShapeArchive *_gumps;
	Shape *_mouse;
	MusicFlex *_music;
	WpnOvlayDat *_weaponOverlay;
	Common::Array<NPCDat *> _npcTable;
	Common::Array<CombatDat *> _combatData;
	Common::Array<WeaselDat *> _weaselData;

	SoundFlex *_soundFlex;
	Common::Array<SpeechFlex **> _speech;
	GameInfo *_gameInfo;

	static GameData *_gameData;
};

#define _TL_(x) (GameData::get_instance()->translate(x))
#define _TL_SHP_(x) (GameData::get_instance()->translate(x))

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
