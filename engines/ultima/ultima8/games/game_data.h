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
	const Shape *getMouse() const {
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

	Std::string translate(const Std::string &text);
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

	RawArchive *_fixed;
	MainShapeArchive *_mainShapes;
	Usecode *_mainUsecode;
	Std::vector<MapGlob *> _globs;
	FontShapeArchive *_fonts;
	GumpShapeArchive *_gumps;
	Shape *_mouse;
	MusicFlex *_music;
	WpnOvlayDat *_weaponOverlay;
	Std::vector<NPCDat *> _npcTable;
	Std::vector<CombatDat *> _combatData;
	Std::vector<WeaselDat *> _weaselData;

	SoundFlex *_soundFlex;
	Std::vector<SpeechFlex **> _speech;
	GameInfo *_gameInfo;

	static GameData *_gameData;
};

#define _TL_(x) (GameData::get_instance()->translate(x))
#define _TL_SHP_(x) (GameData::get_instance()->translate(x))

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
