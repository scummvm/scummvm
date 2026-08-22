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

#ifdef ENABLE_EOB

#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "kyra/engine/kyra_rpg.h"
#include "kyra/gui/automap_eob.h"

#include "common/hashmap.h"	// TODO: REMOVE
#include "common/savefile.h" // TODO: REMOVE
#include "backends/keymapper/keymapper.h"         // TODO: REMOVE
#include "kyra/engine/eobcommon.h" // TODO: REMOVE

namespace Kyra {

static void automapDrawBigString(Graphics::Surface &dst, const Graphics::Font *font,
								 const Common::String &text, int x, int y, int w, uint32 color, int scale,
								 Graphics::TextAlign align = Graphics::kTextAlignCenter) {
	if (!font || text.empty() || scale < 1)
		return;
	const int tw = font->getStringWidth(text);
	const int th = font->getFontHeight();
	if (tw <= 0 || th <= 0)
		return;

	if (scale == 1) {
		font->drawString(&dst, text, x, y, w, color, align);
		return;
	}

	// The overlay may be 16- or 32-bit; index pixels by byte width.
	const int bpp = dst.format.bytesPerPixel;
	if (bpp != 2 && bpp != 4) {
		font->drawString(&dst, text, x, y, w, color, align);
		return;
	}

	// Render onto a zeroed scratch surface so inked pixels are distinguishable.
	Graphics::Surface tmp;
	tmp.create(tw, th, dst.format);
	tmp.fillRect(Common::Rect(0, 0, tw, th), 0);
	font->drawString(&tmp, text, 0, 0, tw, color, Graphics::kTextAlignLeft);

	const int dw = tw * scale;
	int dx = x + (w - dw) / 2; // center (default)
	if (align == Graphics::kTextAlignLeft)
		dx = x;
	else if (align == Graphics::kTextAlignRight)
		dx = x + w - dw;
	if (dx < x)
		dx = x;
	for (int sy = 0; sy < th; ++sy) {
		const byte *src = (const byte *)tmp.getBasePtr(0, sy);
		for (int sx = 0; sx < tw; ++sx) {
			const uint32 c = (bpp == 2) ? ((const uint16 *)src)[sx] : ((const uint32 *)src)[sx];
			if (c == 0) // transparent background pixel
				continue;
			const int px = dx + sx * scale;
			const int py = y + sy * scale;
			dst.fillRect(Common::Rect(px, py, px + scale, py + scale), c);
		}
	}
	tmp.free();
}

// Solid triangle by scanlines (party arrow).
static void automapFillTri(Graphics::Surface &s, int ax, int ay, int bx, int by, int cx, int cy, uint32 color) {
	// Sort vertices by y.
	if (ay > by) {
		SWAP(ax, bx);
		SWAP(ay, by);
	}
	if (ay > cy) {
		SWAP(ax, cx);
		SWAP(ay, cy);
	}
	if (by > cy) {
		SWAP(bx, cx);
		SWAP(by, cy);
	}
	if (cy == ay)
		return;

	for (int y = ay; y <= cy; ++y) {
		int xLong = ax + (cx - ax) * (y - ay) / (cy - ay);
		int xShort = (y < by)
						 ? (by == ay ? ax : ax + (bx - ax) * (y - ay) / (by - ay))
						 : (cy == by ? bx : bx + (cx - bx) * (y - by) / (cy - by));
		int x0 = MIN(xLong, xShort);
		int x1 = MAX(xLong, xShort);
		s.hLine(x0, y, x1, color);
	}
}

// Sparse two-tone speckle to fake stone/paper grain (overwrites pixels, no blend).
static void automapNoise(Graphics::Surface &s, int x0, int y0, int w, int h, uint32 cA, uint32 cB, uint32 seed) {
	if (w <= 0 || h <= 0 || x0 < 0 || y0 < 0 || x0 + w > s.w || y0 + h > s.h)
		return;
	const int bpp = s.format.bytesPerPixel;
	if (bpp != 2 && bpp != 4)
		return;
	uint32 st = seed | 1u;
	for (int y = 0; y < h; ++y) {
		byte *row = (byte *)s.getBasePtr(x0, y0 + y);
		for (int x = 0; x < w; ++x) {
			st = st * 1664525u + 1013904223u;
			const uint32 v = st >> 24; // 0..255
			if (v < 26) {
				const uint32 c = (v & 1u) ? cA : cB;
				if (bpp == 2)
					((uint16 *)row)[x] = (uint16)c;
				else
					((uint32 *)row)[x] = c;
			}
		}
	}
}

static void automapBevel(Graphics::Surface &s, const Common::Rect &r, uint32 hi, uint32 lo) {
	s.hLine(r.left, r.top, r.right - 1, hi);
	s.vLine(r.left, r.top, r.bottom - 1, hi);
	s.hLine(r.left, r.bottom - 1, r.right - 1, lo);
	s.vLine(r.right - 1, r.top, r.bottom - 1, lo);
}

static void automapRivet(Graphics::Surface &s, int x, int y, int sz, uint32 dark, uint32 mid, uint32 hi) {
	s.fillRect(Common::Rect(x - sz, y - sz, x + sz + 1, y + sz + 1), dark);
	s.fillRect(Common::Rect(x - sz + 1, y - sz + 1, x + sz, y + sz), mid);
	const int q = MAX(1, sz / 2);
	s.fillRect(Common::Rect(x - sz + 1, y - sz + 1, x - sz + 1 + q, y - sz + 1 + q), hi);
}

static int automapFit(const Graphics::Font *f, const Common::String &str, int maxW, int maxSc) {
	int sc = MAX(1, maxSc);
	while (sc > 1 && f && f->getStringWidth(str) * sc > maxW)
		--sc;
	return sc;
}

// Automap strings that should be translated to the game's language. Currently, only English is available.
// TODO: Probably move to KYRA.DAT eventually.
const Automap_EoB::TranslateableStrings Automap_EoB::_stringTable[] = {
	{
		{
			"- LEGEND -",
			"Party",
			"Stairs up",
			"Stairs down",
			"Teleporter",
			"Door",
			"Lever",
			"Niche",
			"Note"
		},

		// Area names for the plaque. The games have no level names in their data and the file
		// names of the graphics and sound data files provide only vague hints. So we hardcode
		// the level names here. The Sega CD version of EOBI has an automap, but it doesn't use
		// level names. The maps are just called "L%d" or "Floor %d" (%d being the level number).
		// The FM-Towns version of EOBII has level names for its autogenerated savegame names
		// which are similar to what we do (names based on the graphics/sound files).
		{
			{
				"Upper Sewers",
				"Middle Sewers",
				"Lower Sewers",
				"Dwarven Ruins I",
				"Dwarven Camp",
				"Dwarven Ruins III",
				"Upper Drow",
				"Drow Outcasts",
				"Lower Drow",
				"Mantis Hive",
				"Xanathar Sanctum",
				"Xanathar's Lair",
				"",
				"",
				"",
				""
			},
			{
				"Catacombs I",
				"Catacombs II",
				"Catacombs III",
				"Forest",
				"Darkmoon Temple",
				"Temple Mezzanine",
				"Silver Tower I",
				"Silver Tower II",
				"Silver Tower III",
				"Azure Tower I",
				"Azure Tower II",
				"Azure Tower III",
				"Azure Tower IV",
				"Azure Tower V",
				"Crimson Tower I",
				"Crimson Tower II"
			}
		}
	},
	{
		{
			"", "", "", "", "", "", "", "", ""
		},
		{
			{
				"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
			},
			{
				"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
			}
		}
	}
};

Automap_EoB::Automap_EoB(OSystem *system, LevelBlockProperty **blockData, const uint8 *wllFlags, int gameID, int lang, bool featureEnabled) :
	_system(system), _blockData(*blockData), _wllWallFlags(wllFlags), _enabled(featureEnabled), _visible(false), _selectedBlock(0xFFFF), _automapBg(nullptr),
	_automapFrame(nullptr), _levelNames(nullptr), _legendStrings(nullptr), _numLevelNames(gameID == GI_EOB1 ? 12 : (gameID == GI_EOB2 ? 16 : 0)) {
	_automapBg = new Graphics::Surface();
	_automapFrame = new Graphics::Surface();

	uint langIndex = 0;
	uint gameIndex = gameID - GI_EOB1;

	switch (lang) {
	// TODO: Add remaining languages here.
	case Common::EN_ANY:
	default:
		break;
	}

	assert(langIndex < ARRAYSIZE(_stringTable));
	assert(gameIndex < ARRAYSIZE(_stringTable[0].levelNames));

	_legendStrings = _stringTable[langIndex].legendStrings;
	_levelNames = _stringTable[langIndex].levelNames[gameIndex];
}

Automap_EoB::~Automap_EoB() {
	delete _automapBg;
	delete _automapFrame;
}

void Automap_EoB::markVisited(uint16 block) {
	assert(block < 1024);
	_blockData[block].direction |= 2;
}

bool Automap_EoB::isVisited(uint16 block) const {
	assert(block < 1024);
	return (_blockData[block].direction & 2) != 0;
}

void Automap_EoB::markSeen(uint16 block, int8 dir) {
	assert(block < 1024);

	// Mark the cells in the line of sight ahead up to the first wall or enemy (but include the block with the enemy).
	// Also 1 block diagonal to the left/right of the line of sight, to have a bit more realistic view (and not make
	// players walk around pointlessly just to complete the map). 
	// First value is the relative direction to move, next is whether to reveal the block or not. We don't reveal the
	// blocks directly to the left/right of the party (behind line of sight), even if our route passes through them.
	static const int8 traceRoutes[5][4] = {
		{ -1,  0,  0,  1 },
		{  0,  0, -1,  1 },
		{  0,  1,  0,  1 },
		{  0,  0,  1,  1 },
		{  1,  0,  0,  1 }
	};	

	for (int i = 0; i < ARRAYSIZE(traceRoutes); ++i) {
		uint16 b = block;
		const int8 *r = traceRoutes[i];
		for (int ii = 0; ii < 2; ++ii) {
			int8 d = (dir + *r++) & 3;
			const uint16 nb = calcNewBlockPosition(b, d);
			bool reveal = *r++;
			if (!(_wllWallFlags[_blockData[nb].walls[d ^ 2]] & 1) || _blockData[b].flags & 7)
				break;
			b = nb;
			if (reveal)
				_blockData[b].direction |= 1;
		}
	}
}

bool Automap_EoB::isSeen(uint16 block) const {
	assert(block < 1024);
	return (_blockData[block].direction & 1) != 0;
}

/*
void Automap_EoB::tagTransition(int fromLevel, uint16 fromBlock, int toLevel) {
	// On a script-driven level change, tag the cell we left with a stairs/teleport
	// glyph and a "to level N" note (deeper level = down).
	if (fromLevel < 0 || fromLevel >= 20 || fromBlock >= 1024)
		return;
	const uint32 k = ((uint32)fromLevel << 16) | fromBlock;
	uint8 icon;
	Common::String info;
	if (toLevel > fromLevel) {
		icon = kAmStairsDown;
		info = Common::String::format("Down to level %d", toLevel);
	} else if (toLevel < fromLevel) {
		icon = kAmStairsUp;
		info = Common::String::format("Up to level %d", toLevel);
	} else {
		icon = kAmTeleport;
		info = Common::String::format("Teleport to level %d", toLevel);
	}
	//_automapIcons[k] = icon;
	//_automapAutoInfo[k] = info;
}

void Automap_EoB::collectCellInfo(int level, uint16 block, int teleporterWallId) {
	if (level >= 20 || block >= 1024)
		return;
	const uint32 k = noteKey(level, block);
	if (_automapIcons.contains(k)) // already classified (e.g. as stairs)
		return;
	for (int d = 0; d < 4; ++d) {
		if (_blockData[block].walls[d] == teleporterWallId) {
			_automapIcons[k] = kAmTeleport;
			if (!_automapAutoInfo.contains(k))
				_automapAutoInfo[k] = "Teleporter";
			break;
		}
	}
}
*/
void Automap_EoB::mainLoopProcess(EoBCoreEngine *vm, int inputFlag) {
	if (!_enabled)
		return;

	if (inputFlag == vm->_keyMap[Common::KEYCODE_TAB] || (_visible && inputFlag == vm->_keyMap[Common::KEYCODE_ESCAPE])) {
		_selectedBlock = vm->_currentBlock;
		_visible = !_visible;
		if (_visible) {
			_system->showOverlay(true);
		} else {
			_system->hideOverlay();
			vm->_sceneUpdateRequired = true;
		}
	} /* else if (_visible && inputFlag == 199) {
		handleClick(vm);
	} else if (_visible && inputFlag && inputFlag == vm->_keyMap[Common::KEYCODE_n]) {
		_selectedBlock = vm->_currentBlock;
		editNote(vm);
	} */else if (_visible && inputFlag &&
		(inputFlag == (vm->_keyMap[Common::KEYCODE_UP] | 0x100) || inputFlag == (vm->_keyMap[Common::KEYCODE_DOWN] | 0x100) ||
			inputFlag == (vm->_keyMap[Common::KEYCODE_LEFT] | 0x100) || inputFlag == (vm->_keyMap[Common::KEYCODE_RIGHT] | 0x100))) {

		if (inputFlag == (vm->_keyMap[Common::KEYCODE_UP] | 0x100))
			moveSelection(0, -1);
		else if (inputFlag == (vm->_keyMap[Common::KEYCODE_DOWN] | 0x100))
			moveSelection(0, 1);
		else if (inputFlag == (vm->_keyMap[Common::KEYCODE_LEFT] | 0x100))
			moveSelection(-1, 0);
		else
			moveSelection(1, 0);
	} else if (_visible && inputFlag &&
			   (inputFlag == vm->_keyMap[Common::KEYCODE_UP] || inputFlag == vm->_keyMap[Common::KEYCODE_DOWN] ||
				inputFlag == vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == vm->_keyMap[Common::KEYCODE_RIGHT] ||
				inputFlag == vm->_keyMap[Common::KEYCODE_HOME] || inputFlag == vm->_keyMap[Common::KEYCODE_PAGEUP])) {
		// Movement is normally button-driven, but those are suppressed now, so
		// dispatch the keys here. The handlers only read button->index, so a dummy is fine.
		Button dummy;
		if (inputFlag == vm->_keyMap[Common::KEYCODE_UP])
			vm->clickedUpArrow(&dummy);
		else if (inputFlag == vm->_keyMap[Common::KEYCODE_DOWN])
			vm->clickedDownArrow(&dummy);
		else if (inputFlag == vm->_keyMap[Common::KEYCODE_LEFT])
			vm->clickedLeftArrow(&dummy);
		else if (inputFlag == vm->_keyMap[Common::KEYCODE_RIGHT])
			vm->clickedRightArrow(&dummy);
		else if (inputFlag == vm->_keyMap[Common::KEYCODE_HOME])
			vm->clickedTurnLeftArrow(&dummy);
		else
			vm->clickedTurnRightArrow(&dummy);
	}
}

void Automap_EoB::draw(EoBCoreEngine *vm) {
	markVisited(vm->_currentBlock);

	const int ow = _system->getOverlayWidth();
	const int oh = _system->getOverlayHeight();
	const Graphics::PixelFormat fmt = _system->getOverlayFormat();
	const AutomapLayout L = createLayout();

	// "Stone tablet" palette: opaque grey stone + aged parchment
	const uint32 cStone = fmt.RGBToColor(0x54, 0x56, 0x5e);
	const uint32 cStoneDark = fmt.RGBToColor(0x3c, 0x3e, 0x45);
	const uint32 cStoneEdge = fmt.RGBToColor(0x23, 0x25, 0x2a);
	const uint32 cStoneHi = fmt.RGBToColor(0x6b, 0x6e, 0x78);
	const uint32 cRivet = fmt.RGBToColor(0xc2, 0xc5, 0xcd);
	const uint32 cPaper = fmt.RGBToColor(0xd6, 0xbf, 0x94);
	const uint32 cPaperHi = fmt.RGBToColor(0xe2, 0xcd, 0xa4);
	const uint32 cPaperLo = fmt.RGBToColor(0xc4, 0xab, 0x78);
	const uint32 cPaperEdge = fmt.RGBToColor(0xa9, 0x8b, 0x56);
	const uint32 cInk = fmt.RGBToColor(0x3a, 0x2a, 0x18);
	const uint32 cInkSoft = fmt.RGBToColor(0x7a, 0x60, 0x38);
	const uint32 cFloor = fmt.RGBToColor(0xbb, 0x9c, 0x5e); // a touch darker than paper
	const uint32 cFloorSeen = fmt.RGBToColor(0xc7, 0xb1, 0x80);
	const uint32 cGrid = fmt.RGBToColor(0x8a, 0x70, 0x38);
	const uint32 cWall = fmt.RGBToColor(0x2c, 0x1e, 0x10);
	const uint32 cWallSeen = fmt.RGBToColor(0x8a, 0x73, 0x4a);
	const uint32 cDoor = fmt.RGBToColor(0x7a, 0x4a, 0x1c);
	const uint32 cStair = fmt.RGBToColor(0x3f, 0x7a, 0x3a);
	const uint32 cTele = fmt.RGBToColor(0x5a, 0x3f, 0x8a);
	const uint32 cLever = fmt.RGBToColor(0x9a, 0x2d, 0x2d);
	const uint32 cNiche = fmt.RGBToColor(0x8a, 0x5a, 0x1c);
	const uint32 cNote = fmt.RGBToColor(0x9a, 0x6a, 0x14);
	const uint32 cParty = fmt.RGBToColor(0xa8, 0x28, 0x1c);
	const uint32 cPartyEdge = fmt.RGBToColor(0x5a, 0x14, 0x0e);
	const uint32 cSel = fmt.RGBToColor(0x7a, 0x2a, 0x18);
	const uint32 cPlaqueBg = fmt.RGBToColor(0x1c, 0x1a, 0x16);
	const uint32 cPlaqueEd = fmt.RGBToColor(0x0d, 0x0c, 0x0a);
	const uint32 cGold = fmt.RGBToColor(0xf0, 0xc8, 0x50);
	const uint32 cGoldDim = fmt.RGBToColor(0xb8, 0x92, 0x3a);
	const uint32 cPanelTxt = fmt.RGBToColor(0xd8, 0xc8, 0xa8);

	const Graphics::Font *bigFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	const int fh = bigFont ? bigFont->getFontHeight() : 8;
	const int sc = CLIP<int>(oh / 320, 1, 3); // base side-panel text scale
	const int mpad = MAX(8, L.mapW / 40);

	// (Re)build the cached map background on an overlay resize.
	if (_automapBg->w != ow || _automapBg->h != oh) {
		_automapBg->free();
		_automapBg->create(ow, oh, fmt);
		_automapFrame->free();
		_automapFrame->create(ow, oh, fmt);
		Graphics::Surface &bg = *_automapBg;

		bg.fillRect(Common::Rect(0, 0, ow, oh), cStone);
		automapNoise(bg, 0, 0, ow, oh, cStoneHi, cStoneEdge, 0x9e3779b9u);
		automapBevel(bg, Common::Rect(0, 0, ow, oh), cStoneHi, cStoneEdge);
		automapBevel(bg, Common::Rect(2, 2, ow - 2, oh - 2), cStone, cStoneEdge);
		const int rv = MAX(2, L.frame / 4);
		const int ri = L.frame / 2;
		automapRivet(bg, ri, ri, rv, cStoneEdge, cStoneHi, cRivet);
		automapRivet(bg, ow - ri, ri, rv, cStoneEdge, cStoneHi, cRivet);
		automapRivet(bg, ri, oh - ri, rv, cStoneEdge, cStoneHi, cRivet);
		automapRivet(bg, ow - ri, oh - ri, rv, cStoneEdge, cStoneHi, cRivet);

		const Common::Rect mr(L.mapX, L.mapY, L.mapX + L.mapW, L.mapY + L.mapH);
		bg.fillRect(mr, cPaperEdge);
		bg.fillRect(Common::Rect(mr.left + 2, mr.top + 2, mr.right - 2, mr.bottom - 2), cPaper);
		automapNoise(bg, mr.left + 2, mr.top + 2, L.mapW - 4, L.mapH - 4, cPaperHi, cPaperLo, 0x85ebca6bu);
		bg.frameRect(Common::Rect(mr.left + 2, mr.top + 2, mr.right - 2, mr.bottom - 2), cInkSoft);
		bg.frameRect(Common::Rect(mr.left + 5, mr.top + 5, mr.right - 5, mr.bottom - 5), cInk);
		bg.hLine(L.mapX + mpad, L.footY, L.mapX + L.mapW - mpad, cInkSoft);

		const Common::Rect sr(L.sideX, L.sideY, L.sideX + L.sideW, L.sideY + L.sideH);
		bg.fillRect(sr, cStoneDark);
		automapNoise(bg, sr.left, sr.top, L.sideW, L.sideH, cStoneHi, cStoneEdge, 0xc2b2ae35u);
		automapBevel(bg, sr, cStoneEdge, cStoneHi); // sunken: dark top/left

		// Plaque face
		bg.fillRect(Common::Rect(L.plX, L.plY, L.plX + L.plW, L.plY + L.plH), cPlaqueEd);
		bg.fillRect(Common::Rect(L.plX + 2, L.plY + 2, L.plX + L.plW - 2, L.plY + L.plH - 2), cPlaqueBg);

		const int lx = L.plX, colW = L.plW;
		const int rowH = fh * sc + MAX(4, sc * 4);
		int cyy = L.plY + L.plH + MAX(8, sc * 6);

		automapDrawBigString(bg, bigFont, _legendStrings[0], lx, cyy, colW, cGoldDim, sc);
		cyy += fh * sc + MAX(4, sc * 3);
		bg.hLine(lx, cyy, lx + colW - 1, cStoneHi);
		bg.hLine(lx, cyy + 1, lx + colW - 1, cStoneEdge);
		cyy += MAX(6, sc * 4);

		const int isz = MAX(6, fh * sc);
		for (int i = 0; i < 8; ++i) {
			const int ix = lx, iy = cyy;
			switch (i) {
			case 0:
				automapFillTri(bg, ix + isz / 2, iy, ix + isz, iy + isz, ix, iy + isz, cParty);
				break; // party (up)
			case 1:
				automapFillTri(bg, ix, iy + isz, ix + isz, iy + isz, ix + isz / 2, iy, cStair);
				break; // stairs up
			case 2:
				automapFillTri(bg, ix, iy, ix + isz, iy, ix + isz / 2, iy + isz, cStair);
				break; // stairs down
			case 3:
				bg.frameRect(Common::Rect(ix, iy, ix + isz, iy + isz), cTele);
				bg.frameRect(Common::Rect(ix + 1, iy + 1, ix + isz - 1, iy + isz - 1), cTele);
				break; // teleporter
			case 4:
				bg.fillRect(Common::Rect(ix, iy + isz / 3, ix + isz, iy + isz - isz / 3), cDoor);
				break; // door
			case 5:
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), cLever);
				break; // lever
			case 6:
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), cNiche);
				break; // niche
			default:
				bg.fillRect(Common::Rect(ix, iy, ix + isz, iy + isz), cNote);
				break; // note
			}
			const int tx = lx + isz + MAX(4, sc * 2);
			const int tw = colW - isz - MAX(4, sc * 2);
			const int lsc = automapFit(bigFont, _legendStrings[i + 1], tw, sc);
			automapDrawBigString(bg, bigFont, _legendStrings[i + 1], tx, iy + (isz - fh * lsc) / 2, tw, cPanelTxt, lsc, Graphics::kTextAlignLeft);
			cyy += rowH;
		}

		cyy += MAX(6, sc * 5);
		automapDrawBigString(bg, bigFont, "- KEYS -", lx, cyy, colW, cGoldDim, sc);
		cyy += fh * sc + MAX(4, sc * 3);
		bg.hLine(lx, cyy, lx + colW - 1, cStoneHi);
		bg.hLine(lx, cyy + 1, lx + colW - 1, cStoneEdge);
		cyy += MAX(6, sc * 4);
		const int chipPad = MAX(2, sc * 2);
		const int chipW = (bigFont ? bigFont->getStringWidth("N") * sc : 6 * sc) + chipPad * 2;
		const int chipH = fh * sc + chipPad;
		bg.fillRect(Common::Rect(lx, cyy, lx + chipW, cyy + chipH), cPlaqueEd);
		bg.fillRect(Common::Rect(lx + 1, cyy + 1, lx + chipW - 1, cyy + chipH - 1), cPlaqueBg);
		automapDrawBigString(bg, bigFont, "N", lx, cyy + chipPad / 2, chipW, cGold, sc);
		automapDrawBigString(bg, bigFont, "Add note", lx + chipW + MAX(4, sc * 3), cyy + (chipH - fh * sc) / 2,
							 colW - chipW - MAX(4, sc * 3), cPanelTxt, sc, Graphics::kTextAlignLeft);
	}

	// Copy the chrome into the reused surface, then draw dynamics on top.
	Graphics::Surface &surf = *_automapFrame;
	surf.copyRectToSurface(*_automapBg, 0, 0, Common::Rect(0, 0, ow, oh));

	const int cell = L.cell;
	const int offX = L.offX;
	const int offY = L.offY;
	const int wt = MAX(2, cell / 5); // wall bar thickness

	for (int by = 0; by < 32; ++by) {
		for (int bx = 0; bx < 32; ++bx) {
			const uint16 block = (by << 5) | bx;
			const bool visited = isVisited(block);
			const bool seen = !visited && isSeen(block);
			if (!visited && !seen)
				continue;

			const int sx = offX + bx * cell;
			const int sy = offY + by * cell;

			bool wall[4];
			for (int d = 0; d < 4; ++d) {
				const uint16 nb = calcNewBlockPosition(block, d);
				wall[d] = !(_wllWallFlags[_blockData[nb].walls[d ^ 2]] & 1);
			}

			// Cache the door bits: an open door loses its wall flag mid-animation but
			// should still draw as a door.
			const LevelBlockProperty *bp = &_blockData[block];
			const uint32 bkey = noteKey(vm->_currentLevel, block);
			int8 dbits = _automapDoorBits.contains(bkey) ? _automapDoorBits[bkey] : 0;
			if ((_wllWallFlags[bp->walls[0]] & 8) || (_wllWallFlags[bp->walls[2]] & 8))
				dbits |= 1;
			if ((_wllWallFlags[bp->walls[1]] & 8) || (_wllWallFlags[bp->walls[3]] & 8))
				dbits |= 2;
			if (dbits)
				_automapDoorBits[bkey] = dbits;
			const bool doorNS = (dbits & 1) != 0;
			const bool doorEW = (dbits & 2) != 0;
			if (doorNS)
				wall[0] = wall[2] = false;
			if (doorEW)
				wall[1] = wall[3] = false;

			surf.fillRect(Common::Rect(sx, sy, sx + cell, sy + cell), visited ? cFloor : cFloorSeen);
			if (visited) {
				surf.hLine(sx, sy, sx + cell - 1, cGrid);
				surf.vLine(sx, sy, sy + cell - 1, cGrid);
			}
			const uint32 wc = visited ? cWall : cWallSeen;
			if (wall[0])
				surf.fillRect(Common::Rect(sx, sy, sx + cell, sy + wt), wc);
			if (wall[1])
				surf.fillRect(Common::Rect(sx + cell - wt, sy, sx + cell, sy + cell), wc);
			if (wall[2])
				surf.fillRect(Common::Rect(sx, sy + cell - wt, sx + cell, sy + cell), wc);
			if (wall[3])
				surf.fillRect(Common::Rect(sx, sy, sx + wt, sy + cell), wc);

			if (doorNS || doorEW) {
				const int dt = MAX(2, wt + 1);
				const int dm = MAX(1, cell / 5);
				const int dcx = sx + cell / 2, dcy = sy + cell / 2;
				if (doorNS)
					surf.fillRect(Common::Rect(sx + dm, dcy - dt / 2, sx + cell - dm, dcy - dt / 2 + dt), cDoor);
				if (doorEW)
					surf.fillRect(Common::Rect(dcx - dt / 2, sy + dm, dcx - dt / 2 + dt, sy + cell - dm), cDoor);
			}

			// Special-wall pip on the carrying side (skip door types 5/6; 10 = niche).
			for (int d = 0; d < 4; ++d) {
				const uint8 st = vm->_specialWallTypes[bp->walls[d]];
				if (st == 0 || st == 5 || st == 6)
					continue;
				const uint32 pcol = (st == 10) ? cNiche : cLever;
				const int ps = MAX(2, cell / 4);
				int px, py;
				switch (d) {
				case 0:
					px = sx + cell / 2 - ps / 2;
					py = sy;
					break;
				case 1:
					px = sx + cell - ps;
					py = sy + cell / 2 - ps / 2;
					break;
				case 2:
					px = sx + cell / 2 - ps / 2;
					py = sy + cell - ps;
					break;
				default:
					px = sx;
					py = sy + cell / 2 - ps / 2;
					break;
				}
				surf.fillRect(Common::Rect(px, py, px + ps, py + ps), pcol);
			}

			/* if (_automapIcons.contains(bkey)) {
				const uint8 ic = _automapIcons[bkey];
				uint32 col = (ic == kAmTeleport) ? cTele : cStair;
				if (!visited)
					col = cWallSeen;
				drawIcon(&surf, sx, sy, cell, ic, col);
			}*/

			/* if (_automapNotes.contains(bkey)) {
				const int ds = MAX(2, cell / 3);
				surf.fillRect(Common::Rect(sx + cell - ds, sy, sx + cell, sy + ds), cNote);
			}*/
		}
	}

	if (_selectedBlock != 0xFFFF) {
		const int sx = offX + (_selectedBlock & 0x1F) * cell;
		const int sy = offY + (_selectedBlock >> 5) * cell;
		surf.frameRect(Common::Rect(sx, sy, sx + cell, sy + cell), cSel);
		surf.frameRect(Common::Rect(sx + 1, sy + 1, sx + cell - 1, sy + cell - 1), cSel);
	}

	const int mx = offX + (vm->_currentBlock & 0x1F) * cell;
	const int my = offY + (vm->_currentBlock >> 5) * cell;
	const int cx = mx + cell / 2;
	const int cyp = my + cell / 2;
	const int r = MAX(2, cell / 2 - MAX(1, cell / 6));
	const int b = MAX(2, r * 3 / 4);
	int tipX, tipY, l1X, l1Y, l2X, l2Y;
	switch (vm->_currentDirection) {
	case 1:
		tipX = cx + r;
		tipY = cyp;
		l1X = cx - b;
		l1Y = cyp - b;
		l2X = cx - b;
		l2Y = cyp + b;
		break;
	case 2:
		tipX = cx;
		tipY = cyp + r;
		l1X = cx - b;
		l1Y = cyp - b;
		l2X = cx + b;
		l2Y = cyp - b;
		break;
	case 3:
		tipX = cx - r;
		tipY = cyp;
		l1X = cx + b;
		l1Y = cyp - b;
		l2X = cx + b;
		l2Y = cyp + b;
		break;
	default:
		tipX = cx;
		tipY = cyp - r;
		l1X = cx - b;
		l1Y = cyp + b;
		l2X = cx + b;
		l2Y = cyp + b;
		break;
	}
	automapFillTri(surf, tipX, tipY, l1X, l1Y, l2X, l2Y, cPartyEdge);
	const int gx = (tipX + l1X + l2X) / 3, gy = (tipY + l1Y + l2Y) / 3;
	automapFillTri(surf,
				   tipX + (gx - tipX) / 4, tipY + (gy - tipY) / 4,
				   l1X + (gx - l1X) / 4, l1Y + (gy - l1Y) / 4,
				   l2X + (gx - l2X) / 4, l2Y + (gy - l2Y) / 4, cParty);


	if (bigFont) {
		const uint16 cb = (_selectedBlock != 0xFFFF) ? _selectedBlock : vm->_currentBlock;
		const Common::String lvl = _levelNames[vm->_currentLevel - 1];
		const Common::String crd = Common::String::format("X %d   Y %d", cb & 0x1F, cb >> 5);
		const int pm = MAX(3, sc * 2);
		const int innerW = L.plW - 2 * pm;
		const int innerH = L.plH - 2 * pm;
		const int gap = MAX(2, innerH / 12);
		const int lvlBand = (innerH - gap) * 6 / 10; // level gets ~60%, coords ~40%
		const int crdBand = (innerH - gap) - lvlBand;
		const int lvlSc = automapFit(bigFont, lvl, innerW, MAX(1, lvlBand / fh));
		const int crdSc = automapFit(bigFont, crd, innerW, MAX(1, crdBand / fh));
		const int lvlY = L.plY + pm + (lvlBand - fh * lvlSc) / 2;
		const int crdY = L.plY + pm + lvlBand + gap + (crdBand - fh * crdSc) / 2;
		automapDrawBigString(surf, bigFont, lvl, L.plX, lvlY, L.plW, cGold, lvlSc);
		automapDrawBigString(surf, bigFont, crd, L.plX, crdY, L.plW, cPanelTxt, crdSc);
	}

	// Footer: the selected cell's note / auto-info (the KEYS panel documents N).
	if (bigFont) {
		Common::String foot;
		/* if (_editing) {
			const char *caret = ((_system->getMillis() / 400) & 1) ? "_" : " ";
			foot = Common::String("Note: ") + _editBuffer + caret;
		} else if (_selectedBlock != 0xFFFF) {
			const uint32 selKey = noteKey(vm->_currentLevel, _selectedBlock);
			Common::String body;
			if (_automapNotes.contains(selKey)) {
				body = _automapNotes[selKey];
			} else {
				if (_automapAutoInfo.contains(selKey))
					body = _automapAutoInfo[selKey];
				const Common::String items = listItems(vm, _selectedBlock);
				if (!items.empty())
					body = body.empty() ? items : body + " - " + items;
			}
			if (!body.empty())
				foot = Common::String("Selected: ") + body;
		}


		*/
		if (!foot.empty()) {
			const int footH = (L.mapY + L.mapH) - L.footY;
			const int fpad = MAX(2, footH / 6);
			const int maxFsc = MAX(1, (footH - 2 * fpad) / fh);
			const int fw = L.mapW - 2 * mpad;
			const int fsc = MIN(automapFit(bigFont, foot, fw, sc), maxFsc);
			const int fy = L.footY + (footH - fh * fsc) / 2;
			automapDrawBigString(surf, bigFont, foot, L.mapX + mpad, fy, fw, cInk, fsc, Graphics::kTextAlignLeft);
		}
	}

	_system->copyRectToOverlay(surf.getPixels(), surf.pitch, 0, 0, ow, oh);
	_system->updateScreen();
}

Automap_EoB::AutomapLayout Automap_EoB::createLayout() const {
	const int ow = _system->getOverlayWidth();
	const int oh = _system->getOverlayHeight();
	AutomapLayout l;

	// Stone frame around the overlay, a side panel on the right, and a parchment map
	// inset filling the rest; the 32x32 grid is centred with a footer strip below.
	l.frame = CLIP<int>(oh / 36, 8, 28);
	const int gap = MAX(4, l.frame / 2);
	l.sideW = MAX(150, ow / 4);

	const int inX = l.frame, inY = l.frame;
	const int inW = ow - 2 * l.frame, inH = oh - 2 * l.frame;

	l.mapX = inX;
	l.mapY = inY;
	l.mapW = inW - l.sideW - gap;
	l.mapH = inH;
	l.sideX = inX + l.mapW + gap;
	l.sideY = inY;
	l.sideH = inH;

	const int mpad = MAX(8, l.mapW / 40);
	const int footStrip = MAX(18, oh / 22);
	const int availW = l.mapW - 2 * mpad;
	const int availH = l.mapH - 2 * mpad - footStrip;
	l.cell = MAX(2, MIN(availW / 32, availH / 32));
	const int grid = l.cell * 32;
	l.offX = l.mapX + (l.mapW - grid) / 2;
	l.offY = l.mapY + mpad + (availH - grid) / 2;
	l.footY = l.mapY + l.mapH - footStrip;

	const int spad = MAX(6, l.sideW / 12);
	l.plX = l.sideX + spad;
	l.plY = l.sideY + spad;
	l.plW = l.sideW - 2 * spad;
	l.plH = MAX(30, oh / 8);
	return l;
}

void Automap_EoB::moveSelection(int dx, int dy) {
	const int bx = CLIP<int>((_selectedBlock & 0x1F) + dx, 0, 31);
	const int by = CLIP<int>((_selectedBlock >> 5) + dy, 0, 31);
	_selectedBlock = (by << 5) | bx;
}

uint16 Automap_EoB::calcNewBlockPosition(uint16 block, int8 dir) const {
	static const int16 blockPosTable[] = {-32, 1, 32, -1};
	return (block + blockPosTable[dir & 3]) & 0x3FF;
}

static void automapFillTri(Graphics::Surface &s, int ax, int ay, int bx, int by, int cx, int cy, uint32 color);

void Automap_EoB::drawIcon(Graphics::Surface *surf, int sx, int sy, int cell, uint8 icon, uint32 color) const {
	const int cx = sx + cell / 2;
	const int cy = sy + cell / 2;
	const int r = MAX(2, cell / 3);
	switch (icon) {
	case kAmStairsDown:
		automapFillTri(*surf, cx - r, cy - r, cx + r, cy - r, cx, cy + r, color);
		break;
	case kAmStairsUp:
		automapFillTri(*surf, cx - r, cy + r, cx + r, cy + r, cx, cy - r, color);
		break;
	case kAmTeleport:
		surf->frameRect(Common::Rect(cx - r, cy - r, cx + r, cy + r), color);
		surf->frameRect(Common::Rect(cx - r + 1, cy - r + 1, cx + r - 1, cy + r - 1), color);
		break;
	default:
		break;
	}
}

} // End of namespace Kyra

#endif // ENABLE_EOB
