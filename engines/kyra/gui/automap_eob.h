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

#ifndef KYRA_GUI_AUTOMAP_EOB_H
#define KYRA_GUI_AUTOMAP_EOB_H

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "common/scummsys.h"
#include "common/hashmap.h" // TODO: REMOVE
#include "common/str.h" // TODO: REMOVE

namespace Common {
class OutSaveFile;
class SeekableReadStreamEndianWrapper;
} // End of namespace Common

namespace Graphics {
struct Surface;
} // End of namespace Graphics

class OSystem;

namespace Kyra {

struct LevelBlockProperty;
class EoBCoreEngine; // TODO: REMOVE

class Automap_EoB {
public:
	Automap_EoB(OSystem *system, LevelBlockProperty **blockData, const uint8 *wllFlags, int gameID, int lang, bool featureEnabled);
	~Automap_EoB();

	void markVisited(uint16 block);
	bool isVisited(uint16 block) const;
	void markSeen(uint16 block, int8 dir);
	bool isSeen(uint16 block) const;

	void mainLoopProcess(EoBCoreEngine *vm, int inputFlag); // TODO: REMOVE
	bool isVisible() const { return _visible; } // TODO: REMOVE
	void draw(EoBCoreEngine *vm);

private:
	// Geometry shared by drawing and click hit-testing.
	struct AutomapLayout {
		int cell;
		int offX, offY;                 // top-left of the 32x32 grid (mouse hit-test)
		int frame;                      // stone frame thickness
		int mapX, mapY, mapW, mapH;     // parchment region (map + footer strip)
		int sideX, sideY, sideW, sideH; // stone side panel (plaque, legend, keys)
		int plX, plY, plW, plH;         // level/coords plaque inside the side panel
		int footY;                      // top of the selected-note footer strip
	};

	struct TranslateableStrings {
		const char *const legendStrings[9];
		const char *const levelNames[2][16];
	};

	AutomapLayout createLayout() const;

	uint32 noteKey(uint lvl, uint16 block) const { return (lvl << 16) | block; }

	void moveSelection(int dx, int dy);

	void drawIcon(Graphics::Surface *surf, int sx, int sy, int cell, uint8 icon, uint32 color) const;

	uint16 calcNewBlockPosition(uint16 block, int8 dir) const;

	bool _visible;

	LevelBlockProperty *&_blockData;
	const uint8 *const _wllWallFlags;
	const bool _enabled;

	enum AutomapIcon {
		kAmNone = 0,
		kAmStairsDown = 1,
		kAmStairsUp = 2,
		kAmTeleport = 3
	};


	uint16 _selectedBlock; // TODO: remove
	Common::HashMap<uint32, uint8> _automapDoorBits; // TODO: remove

	static const TranslateableStrings _stringTable[];
	const char * const *_legendStrings;
	const char * const *_levelNames;
	const int _numLevelNames;

	OSystem *_system;
	Graphics::Surface *_automapBg;
	Graphics::Surface *_automapFrame;
};

} // End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL

#endif // KYRA_GUI_AUTOMAP_EOB_H
