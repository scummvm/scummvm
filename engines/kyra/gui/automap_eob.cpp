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
#include "kyra/engine/eobcommon.h"
#include "kyra/gui/automap_eob.h"


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
			"Wall of Force",
			"Switch",
			"Interactive Object",
			"Niche",
			"Stone Portal"
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

Automap_EoB::Automap_EoB(OSystem *system, LevelBlockProperty **blockData, const uint8 *wllFlags, const uint8 *specialWallTypes, int gameID, int lang, bool featureEnabled) : _system(system),
	_blockData(*blockData), _wllWallFlags(wllFlags), _specialWallTypes(specialWallTypes), _enabled(featureEnabled), _visible(false), _automapBg(nullptr), _automapFrame(nullptr), _specialBlockIDs(nullptr), _levelNames(nullptr),
		_colors(nullptr), _legendStrings(nullptr), _controlStrings(nullptr), _numLevelNames(gameID == GI_EOB1 ? 12 : (gameID == GI_EOB2 ? 16 : 0)),	_gameSupportsBreakables(gameID == GI_EOB2), _wallOfForceID(gameID == GI_EOB1 ? 0xFF : 74),
			_portalParamsLen(0), _portalParams(nullptr){
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
	static const uint8 eob1PortalParams[] = { 2, 4, 46, 5, 43, 6, 45, 7, 40, 7, 41, 7, 43, 7, 44, 7, 46, 9, 43, 10, 39, 11, 37, 11, 36, 12, 37 };
	static const uint8 eob2PortalParams[] = { 7, 3, 54, 6, 54 };

	uint8 *specialBlockIDs = new uint8[ARRAYSIZE(types)]();
	memcpy(specialBlockIDs, types, sizeof(types));
	_specialBlockIDs = specialBlockIDs;
	_numSpecialBlockIDs = ARRAYSIZE(types);
	_portalParams = gameID == GI_EOB1 ? eob1PortalParams : eob2PortalParams;
	_portalParamsLen = (gameID == GI_EOB1 ? ARRAYSIZE(eob1PortalParams) : ARRAYSIZE(eob2PortalParams));

	createColors();
}

Automap_EoB::~Automap_EoB() {
	delete _automapBg;
	delete _automapFrame;
	delete[] _specialBlockIDs;
	delete[] _colors;
}

void Automap_EoB::markVisited(uint16 block) {
	assert(block < 1024);
	_blockData[block].direction |= 2;
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

void Automap_EoB::draw(int level, uint16 partyBlock, int8 partyDirection) {
	const int ow = _system->getOverlayWidth();
	const int oh = _system->getOverlayHeight();
	const AutomapLayout l = createLayout();

	// Redraw the map background on an overlay resize.
	if (_automapBg->w != ow || _automapBg->h != oh)
		redrawBackground(l, ow, oh);

	Graphics::Surface &surf = *_automapFrame;
	surf.copyRectToSurface(*_automapBg, 0, 0, Common::Rect(0, 0, ow, oh));

	const int cell = l.cell;
	const int offX = l.offX;
	const int offY = l.offY;
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
				// Check for doors or breakable block objects (like the barrels in the EOB II catacombs) which shouldn't be drawn as walls.
				wall[d] = (!(_wllWallFlags[w] & 9) && w != _wallOfForceID && !isBreakableBlockObject(nb));
			}

			const LevelBlockProperty *bp = &_blockData[block];
			const bool doorNS = ((_wllWallFlags[bp->walls[0]] | _wllWallFlags[bp->walls[2]]) & 8);
			const bool doorEW = ((_wllWallFlags[bp->walls[1]] | _wllWallFlags[bp->walls[3]]) & 8);
			const bool wof = (bp->walls[0] == _wallOfForceID || bp->walls[1] == _wallOfForceID || bp->walls[2] == _wallOfForceID || bp->walls[3] == _wallOfForceID);

			if (doorNS)
				wall[0] = wall[2] = false;
			if (doorEW)
				wall[1] = wall[3] = false;

			surf.fillRect(Common::Rect(sx, sy, sx + cell, sy + cell), _colors[visited ? kColorFloor : kColorFloorSeen]);
			if (visited) {
				surf.hLine(sx, sy, sx + cell - 1, _colors[kColorGrid]);
				surf.vLine(sx, sy, sy + cell - 1, _colors[kColorGrid]);
			}
			const uint32 wc = _colors[visited ? kColorWall : kColorWallSeen];
			if (wall[0])
				surf.fillRect(Common::Rect(sx, sy, sx + cell, sy + wt), wc);
			if (wall[1])
				surf.fillRect(Common::Rect(sx + cell - wt, sy, sx + cell, sy + cell), wc);
			if (wall[2])
				surf.fillRect(Common::Rect(sx, sy + cell - wt, sx + cell, sy + cell), wc);
			if (wall[3])
				surf.fillRect(Common::Rect(sx, sy, sx + wt, sy + cell), wc);

			auto drawButton = [&surf, cell](int x, int y, int alignment, int len, int wdth, uint32 color) {
				switch (alignment) {
				case 0:
					x = x + cell / 2 - len / 2;
					break;
				case 1:
					x = x + cell - wdth;
					y = y + cell / 2 - len / 2;
					SWAP(len, wdth);
					break;
				case 2:
					x = x + cell / 2 - len / 2;
					y = y + cell - wdth;
					break;
				default:
					y = y + cell / 2 - len / 2;
					SWAP(len, wdth);
					break;
				}
				surf.fillRect(Common::Rect(x, y, x + len, y + wdth), color);
			};


			if (doorNS || doorEW) {
				// FIXME?: Doors on the map always look "closed", regardless of whether open oder close.
				const int dt = MAX(2, wt + 1);
				const int dm = MAX(1, cell / 5);
				const int dcx = sx + cell / 2, dcy = sy + cell / 2;
				// Draw the door and also the door button if the door has one.
				if (doorNS) {
					surf.fillRect(Common::Rect(sx + dm, dcy - dt / 2, sx + cell - dm, dcy - dt / 2 + dt), _colors[kColorDoor]);
					if (_specialWallTypes[bp->walls[0]] == 1)
						drawButton(sx - dm, sy + dt, 0, dm, dm, _colors[kColorLever]);	
					if (_specialWallTypes[bp->walls[2]] == 1)
						drawButton(sx + dm, sy - dt, 2, dm, dm, _colors[kColorLever]);
					
				}
				if (doorEW) {
					surf.fillRect(Common::Rect(dcx - dt / 2, sy + dm, dcx - dt / 2 + dt, sy + cell - dm), _colors[kColorDoor]);
					if (_specialWallTypes[bp->walls[1]] == 1)
						drawButton(sx - dt, sy - dm, 1, dm, dm, _colors[kColorLever]);	
					if (_specialWallTypes[bp->walls[3]] == 1)
						drawButton(sx + dt, sy + dm, 3, dm, dm, _colors[kColorLever]);
					
				}
			}

			// Interactive/clickable walls (lever, niche, banner, etc.)
			for (int d = 0; d < 4; ++d) {
				uint16 nb = calcNewBlockPosition(block, d);
				uint8 wn = _blockData[nb].walls[d ^ 2];
				uint8 st = _specialWallTypes[wn];
				if (st == 0 || st == 5 || st == 6)
					continue;

				// At least for EOBI, there are cases where a wall has a clickable type, but there is no
				// script function assigned to it. We don't want to draw these "fake" triggers.
				if (st == 2 && _blockData[nb].assignedObjects == 0)
					continue;

				// If it is a door button on the door or on the door frame it has already been drawn
				// together with the door. Skip it here...
				if (st == 1 && (_wllWallFlags[wn] & 8))
					continue;

				// Breakable objects are also "interactive" (you can click them and you get a text message),
				// but it makes more sense to not to draw them here, but in the middle of the block if they're
				// block objects (like the barrels in the EOB II catacombs). But if it is a wall object (like
				// the breakable windows on the temple ground floor in EOBII) we should draw it here.
				if (isBreakableBlockObject(nb))
					continue;

				// Check if it is a stone portal.
				if (st == _portalParams[0]) {
					for (int i = 1; i < _portalParamsLen && st != 20; i += 2) {
						if (level == _portalParams[i] && wn == _portalParams[i + 1])
							st = 20;
					}
				}

				uint32 pcol = _colors[(st == 10) ? kColorNiche : (st == 1 || st == 3 || st == 4) ? kColorLever : (st == 20 ? kColorTele : kColorInteractive)];
				int plen = MAX(2, cell * 2 / (st < 10 ? 8 : st == 10 ? 6 : 3));
				int pwdth = MAX(2, cell / 4);

				drawButton(sx, sy, d, plen, pwdth, pcol);				
			}

			// TODO: The icons could all be pregenerated instead of individually rendering them each time.
			auto drawIcon = [&surf, cell](int ix, int iy, uint8 icon, const uint32 *colTable) {
				const int cx = ix + cell / 2;
				const int cy = iy + cell / 2;
				const int r = MAX(2, cell / 3);
				const int margin = r * 4 / 5;
				const int step = (cell << 8) / 5;
				const int step2 = ((cell - r) << 8) / 5;
				switch (icon) {
				case 0:
					for (int i = 0; i < margin; ++i) {
						surf.drawLine(cx - margin + i, cy - i, cx + margin - i, cy - i, colTable[kColorTele]);
						surf.drawLine(cx - margin + i, cy + i, cx + margin - i , cy + i, colTable[kColorTele]);
					}
					break;
				case 1:
				case 2:
					for (int pos = 0, g = 0; g < 5; ++g, pos += step) {
						surf.drawLine(ix + (pos >> 8), iy, ix + (pos >> 8), iy + cell, colTable[kColorWall]);
						surf.drawLine(ix, iy + (pos >> 8), ix + cell, iy + (pos >> 8), colTable[kColorWall]);
					}
					break;
				case 3:
					automapFillTri(surf, cx - r, cy + r, cx + r, cy + r, cx, cy - r, colTable[kColorStair]);
					break;
				case 4:
					automapFillTri(surf, cx - r, cy - r, cx + r, cy - r, cx, cy + r, colTable[kColorStair]);
					break;
				case 5:
					surf.drawEllipse(cx - margin, cy - margin, cx + margin, cy + margin, colTable[kColorPit], true);
					break;
				case 6:
				case 7:
					surf.drawEllipse(cx - margin, cy - margin, cx + margin, cy + margin, colTable[kColorPlate], true);
					break;
				case 9:
					surf.fillRect(Common::Rect(cx - margin, cy - margin, cx + margin, cy + margin), colTable[kColorInteractive]);
					break;
				case 10:
					for (int posX = 0, g = 0; g < 3; ++g, posX += (step2 * 2)) {
						for (int posY = 0, gg = 0; gg < 3; ++gg, posY += (step2 * 2)) {
							surf.fillRect(Common::Rect(cx - r + (posX >> 8), cy - r + (posY >> 8), cx - r + ((posX + step2) >> 8), cy - r + ((posY + step2) >> 8)), colTable[kColorWoF]);
							if (g < 2 && gg < 2)
								surf.fillRect(Common::Rect(cx - r + ((posX + step2) >> 8), cy - r + ((posY + step2) >> 8), cx - r + ((posX + step2 * 2) >> 8), cy - r + ((posY + step2 * 2) >> 8)), colTable[kColorWoF]);
						}
					}
					break;
				default:
					break;
				}
			};

			if (wof) {
				drawIcon(sx, sy, 10, _colors);
			} else if (isBreakableBlockObject(block)) {
				drawIcon(sx, sy, 9, _colors);
			} else {
				// Special blocks: teleporter, illusionary wall, pressure plate, pit, stairs up/down
				for (int i = 0; i < _numSpecialBlockIDs; ++i) {
					uint8 s = _specialBlockIDs[i]; // order: teleporter, illusion1, illusion2, stairsUp, stairsDown, pit, plate1, plate2
					if (bp->walls[0] == s || bp->walls[1] == s || bp->walls[2] == s || bp->walls[3] == s)
						drawIcon(sx, sy, i, _colors);
				}
			}
		}
	}

	const int mx = offX + (partyBlock & 0x1F) * cell;
	const int my = offY + (partyBlock >> 5) * cell;
	const int cx = mx + cell / 2;
	const int cyp = my + cell / 2;
	const int r = MAX(2, cell / 2 - MAX(1, cell / 6));
	const int b = MAX(2, r * 3 / 4);
	int tipX, tipY, l1X, l1Y, l2X, l2Y;
	switch (partyDirection) {
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
	automapFillTri(surf, tipX, tipY, l1X, l1Y, l2X, l2Y, _colors[kColorPartyEdge]);
	const int gx = (tipX + l1X + l2X) / 3, gy = (tipY + l1Y + l2Y) / 3;
	automapFillTri(surf,
				   tipX + (gx - tipX) / 4, tipY + (gy - tipY) / 4,
				   l1X + (gx - l1X) / 4, l1Y + (gy - l1Y) / 4,
				   l2X + (gx - l2X) / 4, l2Y + (gy - l2Y) / 4, _colors[kColorParty]);


	const Graphics::Font *bigFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	const int fh = bigFont ? bigFont->getFontHeight() : 8;
	const int sc = CLIP<int>(ow / 320, 1, 3);
	const int mpad = MAX(8, l.mapW / 40);

	if (bigFont) {
		const uint16 cb = (partyBlock != 0xFFFF) ? partyBlock : 0;
		const Common::String lvl = _levelNames[level - 1];
		const Common::String crd = Common::String::format("X %d   Y %d", cb & 0x1F, cb >> 5);
		const int pm = MAX(3, sc * 2);
		const int innerW = l.plW - 2 * pm;
		const int innerH = l.plH - 2 * pm;
		const int gap = MAX(2, innerH / 12);
		const int lvlBand = (innerH - gap) * 6 / 10; 
		const int crdBand = (innerH - gap) - lvlBand;
		const int lvlSc = automapFit(bigFont, lvl, innerW, MAX(1, lvlBand / fh));
		const int crdSc = automapFit(bigFont, crd, innerW, MAX(1, crdBand / fh));
		const int lvlY = l.plY + pm + (lvlBand - fh * lvlSc) / 2;
		const int crdY = l.plY + pm + lvlBand + gap + (crdBand - fh * crdSc) / 2;
		automapDrawBigString(surf, bigFont, lvl, l.plX, lvlY, l.plW, _colors[kColorGold], lvlSc);
		automapDrawBigString(surf, bigFont, crd, l.plX, crdY, l.plW, _colors[kColorPanelTxt], crdSc);
	
		// Footer: TODO? We could write something? Or just leave it blank?
		Common::String foot;
		if (!foot.empty()) {
			const int footH = (l.mapY + l.mapH) - l.footY;
			const int fpad = MAX(2, footH / 6);
			const int maxFsc = MAX(1, (footH - 2 * fpad) / fh);
			const int fw = l.mapW - 2 * mpad;
			const int fsc = MIN(automapFit(bigFont, foot, fw, sc), maxFsc);
			const int fy = l.footY + (footH - fh * fsc) / 2;
			automapDrawBigString(surf, bigFont, foot, l.mapX + mpad, fy, fw, _colors[kColorInk], fsc, Graphics::kTextAlignLeft);
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

void Automap_EoB::createColors() {
	static const uint8 rgbTable[kNumColors][3] = {
		{ 0x54, 0x56, 0x5e },  // stone
		{ 0x3c, 0x3e, 0x45 },  // stone dark
		{ 0x23, 0x25, 0x2a },  // stone edge
		{ 0x6b, 0x6e, 0x78 },  // stone hi
		{ 0xc2, 0xc5, 0xcd },  // rivet
		{ 0xd6, 0xbf, 0x94 },  // paper
		{ 0xe2, 0xcd, 0xa4 },  // paper hi
		{ 0xc4, 0xab, 0x78 },  // paper lo
		{ 0xa9, 0x8b, 0x56 },  // paper edge
		{ 0x3a, 0x2a, 0x18 },  // ink
		{ 0x7a, 0x60, 0x38 },  // ink soft
		{ 0xbb, 0x9c, 0x5e },  // floor
		{ 0xc7, 0xb1, 0x80 },  // floor seen
		{ 0x8a, 0x70, 0x38 },  // grid
		{ 0x2c, 0x1e, 0x10 },  // wall
		{ 0x8a, 0x73, 0x4a },  // wall seen
		{ 0x3f, 0x7a, 0x3a },  // wall of force
		{ 0x7a, 0x4a, 0x1c },  // door
		{ 0x3f, 0x7a, 0x3a },  // stair
		{ 0x5f, 0x5f, 0xca },  // tele
		{ 0x9b, 0x6c, 0x2e },  // plate
		{ 0x6c, 0x6e, 0x80 },  // pit
		{ 0x9a, 0x2d, 0x2d },  // lever
		{ 0x9a, 0x4d, 0xad },  // interactive
		{ 0x95, 0x65, 0x26 },  // niche
		{ 0xa8, 0x28, 0x1c },  // party
		{ 0x5a, 0x14, 0x0e },  // party edge
		{ 0x1c, 0x1a, 0x16 },  // plaque bg
		{ 0x0d, 0x0c, 0x0a },  // plaque ed
		{ 0xf0, 0xc8, 0x50 },  // gold
		{ 0xb8, 0x92, 0x3a },  // gold dim
		{ 0xd8, 0xc8, 0xa8 }   // panel text
	};

	Graphics::PixelFormat fmt = _system->getOverlayFormat();

	uint32 *colors = new uint32[kNumColors]();
	for (int i = 0; i < kNumColors; ++i)
		colors[i] = fmt.RGBToColor(rgbTable[i][0], rgbTable[i][1], rgbTable[i][2]);
	_colors = colors;
}

void Automap_EoB::redrawBackground(const AutomapLayout &l, int width, int height) {
	const Graphics::PixelFormat fmt = _system->getOverlayFormat();
	const Graphics::Font *bigFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	const int fh = bigFont ? bigFont->getFontHeight() : 8;
	const int sc = CLIP<int>(width / 320, 1, 3);
	const int mpad = MAX(8, l.mapW / 40);

	_automapBg->free();
	_automapBg->create(width, height, fmt);
	_automapFrame->free();
	_automapFrame->create(width, height, fmt);
	Graphics::Surface &bg = *_automapBg;

	bg.fillRect(Common::Rect(0, 0, width, height), _colors[kColorStone]);
	automapNoise(bg, 0, 0, width, height, _colors[kColorStoneHi], _colors[kColorStoneEdge], 0x9e3779b9u);
		automapBevel(bg, Common::Rect(0, 0, width, height), _colors[kColorStoneHi], _colors[kColorStoneEdge]);
		automapBevel(bg, Common::Rect(2, 2, width - 2, height - 2), _colors[kColorStone], _colors[kColorStoneEdge]);
		const int rv = MAX(2, l.frame / 4);
		const int ri = l.frame / 2;
		automapRivet(bg, ri, ri, rv, _colors[kColorStoneEdge], _colors[kColorStoneHi], _colors[kColorRivet]);
		automapRivet(bg, width - ri, ri, rv, _colors[kColorStoneEdge], _colors[kColorStoneHi], _colors[kColorRivet]);
		automapRivet(bg, ri, height - ri, rv, _colors[kColorStoneEdge], _colors[kColorStoneHi], _colors[kColorRivet]);
		automapRivet(bg, width - ri, height - ri, rv, _colors[kColorStoneEdge], _colors[kColorStoneHi], _colors[kColorRivet]);

		const Common::Rect mr(l.mapX, l.mapY, l.mapX + l.mapW, l.mapY + l.mapH);
		bg.fillRect(mr, _colors[kColorPaperEdge]);
		bg.fillRect(Common::Rect(mr.left + 2, mr.top + 2, mr.right - 2, mr.bottom - 2), _colors[kColorPaper]);
		automapNoise(bg, mr.left + 2, mr.top + 2, l.mapW - 4, l.mapH - 4, _colors[kColorPaperHi], _colors[kColorPaperLo], 0x85ebca6bu);
		bg.frameRect(Common::Rect(mr.left + 2, mr.top + 2, mr.right - 2, mr.bottom - 2), _colors[kColorInkSoft]);
		bg.frameRect(Common::Rect(mr.left + 5, mr.top + 5, mr.right - 5, mr.bottom - 5), _colors[kColorInk]);
		bg.hLine(l.mapX + mpad, l.footY, l.mapX + l.mapW - mpad, _colors[kColorInkSoft]);

		const Common::Rect sr(l.sideX, l.sideY, l.sideX + l.sideW, l.sideY + l.sideH);
		bg.fillRect(sr, _colors[kColorStoneDark]);
		automapNoise(bg, sr.left, sr.top, l.sideW, l.sideH, _colors[kColorStoneHi], _colors[kColorStoneEdge], 0xc2b2ae35u);
		automapBevel(bg, sr, _colors[kColorStoneEdge], _colors[kColorStoneHi]);

		// Plaque face
		bg.fillRect(Common::Rect(l.plX, l.plY, l.plX + l.plW, l.plY + l.plH), _colors[kColorPlaqueEd]);
		bg.fillRect(Common::Rect(l.plX + 2, l.plY + 2, l.plX + l.plW - 2, l.plY + l.plH - 2), _colors[kColorPlaqueBg]);

		const int lx = l.plX, colW = l.plW;
		const int rowH = fh * sc + MAX(4, sc * 4);
		int cyy = l.plY + l.plH + MAX(8, sc * 6);

		automapDrawBigString(bg, bigFont, _legendStrings[0], lx, cyy, colW, _colors[kColorGoldDim], sc);
		cyy += fh * sc + MAX(4, sc * 3);
		bg.hLine(lx, cyy, lx + colW - 1, _colors[kColorStoneHi]);
		bg.hLine(lx, cyy + 1, lx + colW - 1, _colors[kColorStoneEdge]);
		cyy += MAX(6, sc * 4);

		// TODO: Move legend out of here and redraw it with only the icons that are actually
		// needed for the current level map (to make it less crowded, the same way it is done for LOL).
		const int isz = MAX(6, fh * sc);
		for (int i = 0; i < 13; ++i) {
			const int ix = lx, iy = cyy;
			switch (i) {
			case 0: // party (up)
				automapFillTri(bg, ix + isz / 2, iy, ix + isz, iy + isz, ix, iy + isz, _colors[kColorParty]);
				break;
			case 1: // stairs up
				automapFillTri(bg, ix, iy + isz, ix + isz, iy + isz, ix + isz / 2, iy, _colors[kColorStair]);
				break;
			case 2: // stairs down
				automapFillTri(bg, ix, iy, ix + isz, iy, ix + isz / 2, iy + isz, _colors[kColorStair]);
				break;
			case 3: // teleporter
				for (int ii = 0; ii < isz / 2 - sc; ++ii) {
					bg.drawLine(ix + sc + ii, iy + isz / 2 - ii, ix + isz - sc - ii, iy + isz / 2 - ii, _colors[kColorTele]);
					bg.drawLine(ix + sc + ii, iy + isz / 2 + ii, ix + isz - sc - ii, iy + isz / 2 + ii, _colors[kColorTele]);
				}
				break;
			case 4: // pit
				bg.drawEllipse(ix + sc, iy + sc, ix + isz - sc, iy + isz - sc, _colors[kColorPit], true);
				break;
			case 5: // plate
				bg.drawEllipse(ix + sc, iy + sc, ix + isz - sc, iy + isz - sc, _colors[kColorPlate], true);
				break;
			case 6: // door
				bg.fillRect(Common::Rect(ix, iy + isz / 3, ix + isz, iy + isz - isz / 3), _colors[kColorDoor]);
				break;
			case 7: { // illusionary wall
				bg.fillRect(Common::Rect(ix + sc, iy + sc, ix + isz - sc, iy + isz - sc), _colors[kColorFloor]);
				int step = ((isz - 2 * sc) << 8) / 5;
				for (int g = 0; g < 5; ++g) {
					bg.drawLine(ix + sc + (step * g >> 8), iy + sc, ix + sc + (step * g >> 8), iy + isz - sc, _colors[kColorWall]);
					bg.drawLine(ix + sc, iy + sc + (step * g >> 8), ix + isz - sc, iy + sc + (step * g >> 8), _colors[kColorWall]);
				}
			} break;
			case 8: // wall of force
				bg.fillRect(Common::Rect(ix + sc, iy + sc, ix + isz - sc, iy + isz - sc), _colors[kColorWoF]);
				break;
			case 9:
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), _colors[kColorLever]);
				break;
			case 10:
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), _colors[kColorInteractive]);
				break;
			case 11: // niche
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), _colors[kColorNiche]);
				break;
			case 12: // stone portal
				bg.fillRect(Common::Rect(ix + isz / 3, iy, ix + isz - isz / 3, iy + isz), _colors[kColorTele]);
			default:
				break;
			}
			const int tx = lx + isz + MAX(4, sc * 2);
			const int tw = colW - isz - MAX(4, sc * 2);
			const int lsc = automapFit(bigFont, _legendStrings[i + 1], tw, sc);
			automapDrawBigString(bg, bigFont, _legendStrings[i + 1], tx, iy + (isz - fh * lsc) / 2, tw, _colors[kColorPanelTxt], lsc, Graphics::kTextAlignLeft);
			cyy += rowH;
		}

		cyy += MAX(6, sc * 5);
		automapDrawBigString(bg, bigFont, _controlStrings[0], lx, cyy, colW, _colors[kColorGoldDim], sc);
		cyy += fh * sc + MAX(4, sc * 3);
		bg.hLine(lx, cyy, lx + colW - 1, _colors[kColorStoneHi]);
		bg.hLine(lx, cyy + 1, lx + colW - 1, _colors[kColorStoneEdge]);
		cyy += MAX(6, sc * 4);
		const int chipPad = MAX(2, sc * 2);
		const int chipW = (bigFont ? bigFont->getStringWidth("Up/Down") * sc : 6 * sc) + chipPad * 2; // TODO: Use actual up/down arrow symbols. Our font doesn't have these.
		const int chipH = fh * sc + chipPad;
		bg.fillRect(Common::Rect(lx, cyy, lx + chipW, cyy + chipH), _colors[kColorPlaqueEd]);
		bg.fillRect(Common::Rect(lx + 1, cyy + 1, lx + chipW - 1, cyy + chipH - 1), _colors[kColorPlaqueBg]);
		automapDrawBigString(bg, bigFont, "Up/Down", lx, cyy + chipPad / 2, chipW, _colors[kColorGold], sc); // TODO: see above
		automapDrawBigString(bg, bigFont, _controlStrings[1], lx + chipW + MAX(4, sc * 3), cyy + (chipH - fh * sc) / 2, colW - chipW - MAX(4, sc * 3), _colors[kColorPanelTxt], sc, Graphics::kTextAlignLeft);
}

uint16 Automap_EoB::calcNewBlockPosition(uint16 block, int8 dir) const {
	static const int16 blockPosTable[] = {-32, 1, 32, -1};
	return (block + blockPosTable[dir & 3]) & 0x3FF;
}

bool Automap_EoB::isVisited(uint16 block) const {
	assert(block < 1024);
	return (_blockData[block].direction & 2) != 0;
}

bool Automap_EoB::isSeen(uint16 block) const {
	assert(block < 1024);
	return (_blockData[block].direction & 1) != 0;
}

bool Automap_EoB::isBreakableBlockObject(uint16 block) const {
	LevelBlockProperty &bp = _blockData[block];
	if (!_gameSupportsBreakables || !(bp.flags & 0x800))
		return false;
	// Breakable objects (like the barrels or the crumbling wall in the EOBII catacombs)
	// Check if it is accessible/interactive from multiple sides.
	int cn = 0;
	for (int iii = 0; iii < 4; ++iii) {
		uint8 s = _specialWallTypes[bp.walls[iii]];
		if (s == 8 || s == 9)
			++cn;
	}
	return (cn >= 2);
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
			_automap->draw(_currentLevel, _currentBlock, _currentDirection);
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
		gui_drawAllCharPortraitsWithStats(false);
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
