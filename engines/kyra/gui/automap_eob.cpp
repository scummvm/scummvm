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

	const int bpp = dst.format.bytesPerPixel;
	if (bpp != 2 && bpp != 4) {
		font->drawString(&dst, text, x, y, w, color, align);
		return;
	}

	Graphics::Surface tmp;
	tmp.create(tw, th, dst.format);
	tmp.fillRect(Common::Rect(0, 0, tw, th), 0);
	font->drawString(&tmp, text, 0, 0, tw, color, Graphics::kTextAlignLeft);

	const int dw = tw * scale;
	int dx = x + (w - dw) / 2;
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
			if (c == 0)
				continue;
			const int px = dx + sx * scale;
			const int py = y + sy * scale;
			dst.fillRect(Common::Rect(px, py, px + scale, py + scale), c);
		}
	}
	tmp.free();
}

static void automapFillTri(Graphics::Surface &s, int ax, int ay, int bx, int by, int cx, int cy, uint32 color) {
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

// Sparse two-tone speckle to fake stone/paper grain
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
			"Pit",
			"Plate",
			"Door",
			"Illusionary Wall",
			"Switch",
			"Interactive Object",
			"Niche"
		},

		{
			"- KEYS -",
			"Select Level",
			"Exit"
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
			"", "", ""
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

Automap_EoB::Automap_EoB(OSystem *system, LevelBlockProperty **blockData, const uint8 *wllFlags, const uint8 *specialWallTypes, int gameID, int lang, bool featureEnabled) : _system(system), _blockData(*blockData),
	_wllWallFlags(wllFlags), _specialWallTypes(specialWallTypes), _enabled(featureEnabled), _visible(false), _automapBg(nullptr), _automapFrame(nullptr), _specialBlockIDs(nullptr), _levelNames(nullptr),
		_legendStrings(nullptr), _controlStrings(nullptr), _numLevelNames(gameID == GI_EOB1 ? 12 : (gameID == GI_EOB2 ? 16 : 0)),	_gameSupportsBreakables(gameID == GI_EOB2) {
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
	_controlStrings = _stringTable[langIndex].controlStrings;
	_levelNames = _stringTable[langIndex].levelNames[gameIndex];

	const uint8 teleporter = gameID == GI_EOB1 ? 52 : 44;
	const uint8 illusion1 = gameID == GI_EOB1 ? 67 : 46;
	const uint8 illusion2 = gameID == GI_EOB1 ? 64 : 46;
	const uint8 plate1 = gameID == GI_EOB1 ? 28 : 35;
	const uint8 plate2 = gameID == GI_EOB1 ? 28 : 36;
	const uint8 pit = gameID == GI_EOB1 ? 27 : 38;
	const uint8 stairsUp = 23;
	const uint8 stairsDown = 24;
	const uint8 types[] = { teleporter, illusion1, illusion2, stairsUp, stairsDown, pit, plate1, plate2 };
	uint8 *specialBlockIDs = new uint8[ARRAYSIZE(types)]();
	memcpy(specialBlockIDs, types, sizeof(types));
	_specialBlockIDs = specialBlockIDs;
	_numSpecialBlockIDs = ARRAYSIZE(types);
}

Automap_EoB::~Automap_EoB() {
	delete _automapBg;
	delete _automapFrame;
	delete[] _specialBlockIDs;
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

	// Mark the cells in the line of sight ahead up to the first wall, door or enemy (but include the block with the enemy).
	// Also 1 block diagonal to the left/right of the line of sight, to have a bit more realistic view (and not make
	// players walk around pointlessly just to complete the map). 
	// First value is the relative direction to move, next is whether to reveal the block or not. We don't reveal the
	// blocks directly to the left/right of the party (behind line of sight), even if our route passes through them.
	// We also don't reveal behind doors or destructable block objects.
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
			uint8 wn = _blockData[nb].walls[d ^ 2];

			bool blockDestructable = false;
			if (_gameSupportsBreakables && (_blockData[nb].flags & 0x800)) {
				// Check if it is destructable from multiple sides to determine if we should treat it as
				// a wall object (like the breakable windows on the temple ground floor in EOBII) or a
				// block object (like the barrels in the catacombs).
				int cn = 0;
				for (int iii = 0; iii < 4; ++iii) {
					uint8 s = _specialWallTypes[_blockData[nb].walls[iii]];
					if (s == 8 || s == 9)
						++cn;
				}
				blockDestructable = cn >= 2;
			}
			bool blockHasDestructable = (_gameSupportsBreakables && (_specialWallTypes[wn] == 8 || _specialWallTypes[wn] == 9) );


			if ((!(_wllWallFlags[wn] & 9) && !blockDestructable) || _blockData[b].flags & 7)
				break;
			b = nb;
			if (reveal)
				_blockData[b].direction |= 1;
			if (_wllWallFlags[wn] & 8 || blockHasDestructable)
				break;
		}
	}
}

bool Automap_EoB::isSeen(uint16 block) const {
	assert(block < 1024);
	return (_blockData[block].direction & 1) != 0;
}

void Automap_EoB::draw(EoBCoreEngine *vm) {
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
	const uint32 cTele = fmt.RGBToColor(0x5f, 0x5f, 0xca);
	const uint32 cPlate = fmt.RGBToColor(0x9b, 0x6c, 0x2e);
	const uint32 cPit = fmt.RGBToColor(0x6c, 0x6e, 0x80);
	const uint32 cLever = fmt.RGBToColor(0x9a, 0x2d, 0x2d);
	const uint32 cInteractive = fmt.RGBToColor(0x9a, 0x4d, 0xad);
	const uint32 cNiche = fmt.RGBToColor(0x8a, 0x5a, 0x1c);
	const uint32 cParty = fmt.RGBToColor(0xa8, 0x28, 0x1c);
	const uint32 cPartyEdge = fmt.RGBToColor(0x5a, 0x14, 0x0e);
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
		automapBevel(bg, sr, cStoneEdge, cStoneHi);

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

		// TODO: Move legend out of these braces and redraw it with only the icons that are actually
		// needed for the current level map (to make it less crowded, same way it is done for LOL).
		const int isz = MAX(6, fh * sc);
		for (int i = 0; i < 11; ++i) {
			const int ix = lx, iy = cyy;
			switch (i) {
			case 0: // party (up)
				automapFillTri(bg, ix + isz / 2, iy, ix + isz, iy + isz, ix, iy + isz, cParty);
				break; 
			case 1: // stairs up
				automapFillTri(bg, ix, iy + isz, ix + isz, iy + isz, ix + isz / 2, iy, cStair);
				break;
			case 2: // stairs down
				automapFillTri(bg, ix, iy, ix + isz, iy, ix + isz / 2, iy + isz, cStair);
				break;
			case 3: // teleporter
				for (int ii = 0; ii < isz / 2 - sc; ++ii) {
					bg.drawLine(ix + sc + ii, iy + isz / 2 - ii, ix + isz - sc - ii, iy + isz / 2 - ii, cTele);
					bg.drawLine(ix + sc + ii, iy + isz / 2 + ii, ix + isz - sc - ii, iy + isz / 2 + ii, cTele);
				}
				break; 
			case 4: // pit
				bg.drawEllipse(ix + sc, iy + sc, ix + isz - sc, iy + isz - sc, cPit, true);
				break;
			case 5: // plate
				bg.drawEllipse(ix + sc, iy + sc, ix + isz - sc, iy + isz - sc, cPlate, true);
				break; 
			case 6: // door
				bg.fillRect(Common::Rect(ix, iy + isz / 3, ix + isz, iy + isz - isz / 3), cDoor);
				break;
			case 7: { // illusionary wall
				bg.fillRect(Common::Rect(ix + sc, iy + sc, ix + isz - sc, iy + isz - sc), cFloor);
				int step = ((isz - 2 * sc) << 8) / 5;
				for (int g = 0; g < 5; ++g) {
					bg.drawLine(ix + sc + (step * g >> 8), iy + sc, ix + sc + (step * g >> 8), iy + isz - sc, cWall);
					bg.drawLine(ix + sc, iy + sc + (step * g >> 8), ix + isz - sc, iy + sc + (step * g >> 8), cWall);
				}
			}	break;
			case 8:
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), cLever);
				break;
			case 9:
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), cInteractive);
				break;
			case 10:
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), cNiche);
				break; // niche
			default:
				break;
			}
			const int tx = lx + isz + MAX(4, sc * 2);
			const int tw = colW - isz - MAX(4, sc * 2);
			const int lsc = automapFit(bigFont, _legendStrings[i + 1], tw, sc);
			automapDrawBigString(bg, bigFont, _legendStrings[i + 1], tx, iy + (isz - fh * lsc) / 2, tw, cPanelTxt, lsc, Graphics::kTextAlignLeft);
			cyy += rowH;
		}

		cyy += MAX(6, sc * 5);
		automapDrawBigString(bg, bigFont, _controlStrings[0], lx, cyy, colW, cGoldDim, sc);
		cyy += fh * sc + MAX(4, sc * 3);
		bg.hLine(lx, cyy, lx + colW - 1, cStoneHi);
		bg.hLine(lx, cyy + 1, lx + colW - 1, cStoneEdge);
		cyy += MAX(6, sc * 4);
		const int chipPad = MAX(2, sc * 2);
		const int chipW = (bigFont ? bigFont->getStringWidth("Up/Down") * sc : 6 * sc) + chipPad * 2;	// TODO: Use actual up/down arrow symbols. Our font doesn't have these.
		const int chipH = fh * sc + chipPad;
		bg.fillRect(Common::Rect(lx, cyy, lx + chipW, cyy + chipH), cPlaqueEd);
		bg.fillRect(Common::Rect(lx + 1, cyy + 1, lx + chipW - 1, cyy + chipH - 1), cPlaqueBg);
		automapDrawBigString(bg, bigFont, "Up/Down", lx, cyy + chipPad / 2, chipW, cGold, sc);			// TODO: see above
		automapDrawBigString(bg, bigFont, _controlStrings[1], lx + chipW + MAX(4, sc * 3), cyy + (chipH - fh * sc) / 2,
							 colW - chipW - MAX(4, sc * 3), cPanelTxt, sc, Graphics::kTextAlignLeft);
	}

	Graphics::Surface &surf = *_automapFrame;
	surf.copyRectToSurface(*_automapBg, 0, 0, Common::Rect(0, 0, ow, oh));

	const int cell = L.cell;
	const int offX = L.offX;
	const int offY = L.offY;
	const int wt = MAX(2, cell / 5);

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
				uint8 w = _blockData[nb].walls[d ^ 2];
				wall[d] = !(_wllWallFlags[w] & 9);

				// Now we check for breakable block objects (like the barrels in the EOB II catacombs) which shouldn't be drawn as walls.
				if (wall[d] && _gameSupportsBreakables && (_blockData[nb].flags & 0x800)) {
					// Check if it is accessible/interactive from multiple sides to determine if we should treat it as a wall object (like the
					// breakable windows on the temple ground floor in EOBII) or a block object (like the barrels in the catacombs).
					// If it is a wall object we should draw the wall.
					int cn = 0;
					for (int iii = 0; iii < 4; ++iii) {
						uint8 s = _specialWallTypes[_blockData[nb].walls[iii]];
						if (s == 8 || s == 9)
							++cn;
					}
					wall[d] = cn < 2;
				}
			}

			const LevelBlockProperty *bp = &_blockData[block];
			const bool doorNS = ((_wllWallFlags[bp->walls[0]] | _wllWallFlags[bp->walls[2]]) & 8);
			const bool doorEW = ((_wllWallFlags[bp->walls[1]] | _wllWallFlags[bp->walls[3]]) & 8);
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
				// FIXME?: Doors on the map always look "closed", regardless of whether open oder close.
				const int dt = MAX(2, wt + 1);
				const int dm = MAX(1, cell / 5);
				const int dcx = sx + cell / 2, dcy = sy + cell / 2;
				if (doorNS)
					surf.fillRect(Common::Rect(sx + dm, dcy - dt / 2, sx + cell - dm, dcy - dt / 2 + dt), cDoor);
				if (doorEW)
					surf.fillRect(Common::Rect(dcx - dt / 2, sy + dm, dcx - dt / 2 + dt, sy + cell - dm), cDoor);
			}

			// Interactive/clickable walls (lever, niche, banner, etc.)
			for (int d = 0; d < 4; ++d) {
				uint16 nb = calcNewBlockPosition(block, d);
				const uint8 st = _specialWallTypes[_blockData[nb].walls[d ^ 2]];
				if (st == 0 || st == 5 || st == 6)
					continue;

				// At least for EOBI, there are cases where a wall has a clickable type, but there is no
				// script function assigned to it. We don't want to draw these "fake" triggers.
				if (st == 2 && _blockData[nb].assignedObjects == 0)
					continue;

				// Breakable objects are also "interactive" (you can click them and you get a text message),
				// but it makes more sense to not to draw them here, but in the middle of the block if they're
				// block objects (like the barrels in the EOB II catacombs). If it's a wall object (like the
				// breakable windows on the temple ground floor in EOBII) we should draw it here.
				if (_gameSupportsBreakables && (_blockData[nb].flags & 0x800)) {
					int cn = 0;
					// Check if it is accessible/interactive from multiple sides.
					for (int iii = 0; iii < 4; ++iii) {
						uint8 s = _specialWallTypes[_blockData[nb].walls[iii]];
						if (s == 8 || s == 9)
							++cn;
					}
					if (cn >= 2)
						continue;
				}

				const uint32 pcol = (st == 10) ? cNiche : (st == 1 || st == 3 || st == 4) ? cLever : cInteractive;
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

			// TODO: The icons could all be pregenerated instead of individually rendering them each time.
			auto drawIcon = [cInteractive, cFloor, wc, cPlate, cPit, cStair, cTele](Graphics::Surface *sr, int ix, int iy, int cel, uint8 icon) {
				const int cx = ix + cel / 2;
				const int cy = iy + cel / 2;
				const int r = MAX(2, cel / 3);
				const int margin = r * 4 / 5;
				const int step = (cel << 8) / 5;
				switch (icon) {
				case 0:
					for (int i = 0; i < margin; ++i) {
						sr->drawLine(cx - margin + i, cy - i, cx + margin - i, cy - i , cTele);
						sr->drawLine(cx - margin + i, cy + i, cx + margin - i , cy + i, cTele);
					}
					break;
				case 1:
				case 2:
					for (int pos = 0, g = 0; g < 5; ++g, pos += step) {
						sr->drawLine(ix + (pos >> 8), iy, ix + (pos >> 8), iy + cel, wc);
						sr->drawLine(ix, iy + (pos >> 8), ix + cel, iy + (pos >> 8), wc);
					}
					break;
				case 3:
					automapFillTri(*sr, cx - r, cy + r, cx + r, cy + r, cx, cy - r, cStair);
					break;
				case 4:
					automapFillTri(*sr, cx - r, cy - r, cx + r, cy - r, cx, cy + r, cStair);
					break;
				case 5:
					sr->drawEllipse(cx - margin, cy - margin, cx + margin, cy + margin, cPit, true);
					break;
				case 6:
				case 7:
					sr->drawEllipse(cx - margin, cy - margin, cx + margin, cy + margin, cPlate, true);
					break;
				case 9:
					sr->fillRect(Common::Rect(cx - margin, cy - margin, cx + margin, cy + margin), cInteractive);
					break;
				default:
					break;
				}
			};

			 if (_gameSupportsBreakables && (bp->flags & 0x800)) {
				// Breakable objects (like the barrels or the crumbling wall in the EOBII catacombs)
				// Check if it is accessible/interactive from multiple sides.
				int cn = 0;
				for (int iii = 0; iii < 4; ++iii) {
					uint8 s = _specialWallTypes[bp->walls[iii]];
					if (s == 8 || s == 9)
						++cn;
				}
				if (cn >= 2)
					drawIcon(&surf, sx, sy, cell, 9);
			} else {
				// Special blocks: teleporter, illusionary wall, pressure plate, pit, stairs up/down
				for (int i = 0; i < _numSpecialBlockIDs; ++i) {
					uint8 s = _specialBlockIDs[i]; // order: teleporter, illusion1, illusion2, stairsUp, stairsDown, pit, plate1, plate2
					if (bp->walls[0] == s || bp->walls[1] == s || bp->walls[2] == s || bp->walls[3] == s)
						drawIcon(&surf, sx, sy, cell, i);
				}
			}
		}
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
		const uint16 cb = (vm->_currentBlock != 0xFFFF) ? vm->_currentBlock : 0;
		const Common::String lvl = _levelNames[vm->_currentLevel - 1];
		const Common::String crd = Common::String::format("X %d   Y %d", cb & 0x1F, cb >> 5);
		const int pm = MAX(3, sc * 2);
		const int innerW = L.plW - 2 * pm;
		const int innerH = L.plH - 2 * pm;
		const int gap = MAX(2, innerH / 12);
		const int lvlBand = (innerH - gap) * 6 / 10; 
		const int crdBand = (innerH - gap) - lvlBand;
		const int lvlSc = automapFit(bigFont, lvl, innerW, MAX(1, lvlBand / fh));
		const int crdSc = automapFit(bigFont, crd, innerW, MAX(1, crdBand / fh));
		const int lvlY = L.plY + pm + (lvlBand - fh * lvlSc) / 2;
		const int crdY = L.plY + pm + lvlBand + gap + (crdBand - fh * crdSc) / 2;
		automapDrawBigString(surf, bigFont, lvl, L.plX, lvlY, L.plW, cGold, lvlSc);
		automapDrawBigString(surf, bigFont, crd, L.plX, crdY, L.plW, cPanelTxt, crdSc);
	}

	if (bigFont) {
		// Footer: TODO? We could write something? Or just leave it blank?
		Common::String foot;
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

	// Stone frame around the overlay, a side panel on the right,
	// and a parchment map inset filling the rest
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

uint16 Automap_EoB::calcNewBlockPosition(uint16 block, int8 dir) const {
	static const int16 blockPosTable[] = {-32, 1, 32, -1};
	return (block + blockPosTable[dir & 3]) & 0x3FF;
}

int EoBCoreEngine::clickedAutomap(Button *button) {
	gui_updateControls();
	removeInputTop();
	disableSysTimer(2);

	txt()->removePageBreakFlag();
	_screen->copyRegion(0, 120, 0, 0, 176, 48, 0, Screen_EoB::kCampMenuBackupPage, Screen::CR_NO_P_CHECK);

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		_characters[i].damageTaken = 0;
		_characters[i].slotStatus[0] = _characters[i].slotStatus[1] = 0;
		gui_drawCharPortraitWithStats(i);
	}

	bool update = true;
	bool firstPage = true;
	int currentLevel = _currentLevel;
	int currentSub = _currentSub;
	int currentBlock = _currentBlock;
	int currentDirection = _currentDirection;
	int numLevels = (_flags.gameID == GI_EOB2) ? 16 : 12;

	int ow = _system->getOverlayWidth();
	int oh = _system->getOverlayHeight();

	_automap->markSeen(_currentBlock, _currentDirection);
	_automap->markVisited(_currentBlock);

	_system->showOverlay();

	for (bool runLoop = true; runLoop && !shouldQuit();) {
		uint32 frameEnd = _system->getMillis() + 8;

		int inputFlag = checkInput(nullptr, false, 0) & 0x80FF;	// TODO? We could allow GMM loading on the map screen, but it would require some post-processing
		removeInputTop();

		// restore overlay if the GMM or the debugger has been used on the map screen
		if (!_system->isOverlayVisible()) {
			_system->showOverlay();
			update = true;
		}

		// redraw map if the window has been resized
		if (ow != _system->getOverlayWidth() || oh != _system->getOverlayHeight()) {
			ow = _system->getOverlayWidth();
			oh = _system->getOverlayHeight();
			update = true;
		}

		int lvl = _currentLevel;

		if (inputFlag == _keyMap[Common::KEYCODE_ESCAPE] || inputFlag == _keyMap[Common::KEYCODE_TAB]) {
			runLoop = false;
		} else if (inputFlag == _keyMap[Common::KEYCODE_UP]) {
			for (int i = lvl; lvl == _currentLevel && i <= numLevels; ++i) {
				if (_hasTempDataFlags & (1 << (i - 1)))
					lvl = i;
			}
		} else if (inputFlag == _keyMap[Common::KEYCODE_DOWN]) {
			for (int i = lvl; lvl == _currentLevel && i > 0; --i) {
				if (_hasTempDataFlags & (1 << (i - 1)))
					lvl = i;
			}
		}

		if (lvl != _currentLevel) {
			if (firstPage) {
				completeDoorOperations();
				generateTempData();
				firstPage = false;
			}

			loadLevel(lvl, 0);

			_currentBlock = (currentLevel == lvl) ? currentBlock : 0xFFFF;
			update = true;
		}

		if (update) {
			_automap->draw(this);
			update = false;
		}

		delayUntil(frameEnd, false, false, false); // TODO? allow GMM loading on the map screen, seen comment above
	}

	if (currentLevel != _currentLevel || currentSub != _currentSub)
		loadLevel(currentLevel, currentSub);

	_currentBlock = currentBlock;
	_currentDirection = currentDirection;

	_system->hideOverlay();

	if (_flags.platform == Common::kPlatformSegaCD) {
		setLevelPalettes(_currentLevel);
		_screen->sega_selectPalette(-1, 2, true);
		gui_setupPlayFieldHelperPages(true);
		gui_drawAllCharPortraitsWithStats();
	}

	drawScene(0);

	_screen->setCurPage(0);
	const ScreenDim *dm = _screen->getScreenDim(10);
	_screen->copyRegion(0, 0, 0, 120, 176, 48, Screen_EoB::kCampMenuBackupPage, 2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, 2, 0, Screen::CR_NO_P_CHECK);

	_screen->updateScreen();

	enableSysTimer(2);
	advanceTimers(_restPartyElapsedTime);
	_restPartyElapsedTime = 0;

	checkPartyStatus(true);

	return button->arg;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
