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

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"

#include "video/flic_decoder.h"

#include "eem/animation.h"
#include "eem/coords.h"
#include "eem/eem.h"
#include "eem/music.h"
#include "eem/site.h"

namespace EEM {

struct LondonApproachData {
	uint16 videoId = 0;
	uint16 videoX = 0;
	uint16 videoY = 0;
	Common::Rect textRect;
	Common::Array<Common::String> pages;
};

struct LondonApproachButton {
	int x1;
	int y1;
	int x2;
	int y2;
	int x;
	int y;
	uint16 picId;

	Common::Rect rect() const { return Common::Rect(x1, y1, x2, y2); }
};

const LondonApproachButton kLondonApproachButtons[4] = {
	{  10, 139,  35, 157,  11, 139, 0x361 }, // Done
	{  10, 172,  35, 190,  11, 172, 0x362 }, // Play
	{ 287, 172, 307, 190, 287, 172, 0x35f }, // Next
	{ 287, 139, 307, 157, 287, 139, 0x360 }, // Previous
};

const uint16 kMacLondonApproachBackgroundBasePic = 0x38c;
const int kMacLondonApproachMovieX = 114;
const int kMacLondonApproachMovieY = 23;
const int kMacLondonApproachPlayLeft = 9;
const int kMacLondonApproachPlayTop = 277;
const int kMacLondonApproachPlayRight = 60;
const int kMacLondonApproachPlayBottom = 332;
const int kMacLondonApproachDoneLeft = 452;
const int kMacLondonApproachDoneTop = 277;
const int kMacLondonApproachDoneRight = 504;
const int kMacLondonApproachDoneBottom = 332;
// Mac FUN_0000849e maps overview clicks to detail scroll with
// mouse * 2 - center, clamped to zero before subtracting.
const int kMacBigMapScrollCenterX = 0xbf;
const int kMacBigMapScrollCenterY = 0xab;

byte mapVisitedMarkerColor(byte color) {
	switch (color) {
	case 0xf7:
	case 0xfb:
	case 0xfd:
		return 0x1b;
	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfc:
	case 0xfe:
		return 0x19;
	default:
		return color;
	}
}

void blitBigMapMarker(Graphics::ManagedSurface &dstSurface, const Picture &marker,
					  int x, int y, bool useVisitedColors) {
	const byte transp = (byte)(marker.flags >> 8);
	for (int row = 0; row < marker.surface.h; row++) {
		const int dstY = y + row;
		if (dstY < 0 || dstY >= dstSurface.h)
			continue;
		const byte *src = (const byte *)marker.surface.getBasePtr(0, row);
		byte *dst = (byte *)dstSurface.getBasePtr(0, dstY);
		for (int col = 0; col < marker.surface.w; col++) {
			const int dstX = x + col;
			if (dstX < 0 || dstX >= dstSurface.w)
				continue;
			if (src[col] != transp)
				dst[dstX] = useVisitedColors ? mapVisitedMarkerColor(src[col])
											  : src[col];
		}
	}
}

void blitMacBigMapPartnerFrame(Graphics::ManagedSurface &dstSurface,
							   const Picture &frame, int anchorX,
							   int anchorY) {
	const byte transp = (byte)(frame.flags >> 8);
	const int x = anchorX - (int)(int16)frame.miscflags;
	const int y = anchorY - (int)(int16)frame.rowoff;
	for (int row = 0; row < frame.surface.h; row++) {
		const int dstY = y + row;
		if (dstY < 0 || dstY >= dstSurface.h)
			continue;
		const byte *src = (const byte *)frame.surface.getBasePtr(0, row);
		byte *dst = (byte *)dstSurface.getBasePtr(0, dstY);
		for (int col = 0; col < frame.surface.w; col++) {
			const int dstX = x + col;
			if (dstX < 0 || dstX >= dstSurface.w)
				continue;
			const byte color = src[col];
			if (color != transp) {
				// The map partner frames are authored against the overview
				// ColorTable: 0 is white and 0xff is black. The detail-map
				// ColorTable swaps those endpoints.
				if (color == 0x00)
					dst[dstX] = 0xff;
				else if (color == 0xff)
					dst[dstX] = 0x00;
				else
					dst[dstX] = color;
			}
		}
	}
}

struct BigMapEntryInfo {
	uint16 overviewX = 0;
	uint16 overviewY = 0;
	uint16 detailX = 0;
	uint16 detailY = 0;
	uint16 buttonId = 0;
	uint16 crime = 0;
};

bool readBigMapEntryInfo(const byte *entry, bool floppy, bool macintosh,
						 BigMapEntryInfo &out) {
	if (!entry)
		return false;

	if (macintosh) {
		out.detailX   = READ_LE_UINT16(entry + 0x0);
		out.detailY   = READ_LE_UINT16(entry + 0x2);
		out.buttonId  = entry[0x5];
		out.overviewX = READ_LE_UINT16(entry + 0x6);
		out.overviewY = READ_LE_UINT16(entry + 0x8);
		out.crime     = READ_LE_UINT16(entry + 0xa);
		return true;
	}

	if (floppy) {
		out.detailX   = READ_LE_UINT16(entry + 0x0);
		out.detailY   = READ_LE_UINT16(entry + 0x2);
		out.buttonId  = entry[0x4];
		out.overviewX = READ_LE_UINT16(entry + 0x6);
		out.overviewY = READ_LE_UINT16(entry + 0x8);
		out.crime     = entry[0xa];
		return true;
	}

	out.buttonId  = READ_LE_UINT16(entry + 0x0);
	out.overviewX = READ_LE_UINT16(entry + 0x4);
	out.overviewY = READ_LE_UINT16(entry + 0x6);
	out.detailX   = READ_LE_UINT16(entry + 0x8);
	out.detailY   = READ_LE_UINT16(entry + 0xa);
	out.crime     = READ_LE_UINT16(entry + 0xc);
	return true;
}

const uint32 kMacMapColorCycleDelayMs = 25 * 1000 / 60;

bool loadMacBigMapPixels(Common::Array<byte> &mapPixels,
						 uint16 &mapW, uint16 &mapH) {
	DBDArchive bigMapArchive;
	if (!bigMapArchive.open(Common::Path("BIGMAP.DBD"),
							Common::Path("BIGMAP.DBX"), true)) {
		warning("doBigMap: BIGMAP archive missing");
		return false;
	}

	Picture mapPic;
	if (!bigMapArchive.loadEntry(0, mapPic) || mapPic.surface.empty()) {
		warning("doBigMap: BIGMAP.DBD entry 0 failed to load");
		return false;
	}
	if (mapPic.surface.w <= 0 || mapPic.surface.h <= 0)
		return false;

	mapW = (uint16)mapPic.surface.w;
	mapH = (uint16)mapPic.surface.h;
	mapPixels.resize((uint32)mapW * mapH);
	for (uint y = 0; y < mapH; y++) {
		const byte *src = (const byte *)mapPic.surface.getBasePtr(0, y);
		byte *dst = mapPixels.data() + y * mapW;
		// BIGMAP.DBD uses 0xff for dark coast/detail pixels, but the Mac
		// detail-map palette keeps 0xff white for the UI frame/buttons.
		for (uint x = 0; x < mapW; x++)
			dst[x] = src[x] == 0xff ? 0x00 : src[x];
	}
	return true;
}

static bool openNumberedScriptFile(Common::File &f, Common::String &name,
								   uint num, const char *const *patterns,
								   uint patternCount) {
	for (uint i = 0; i < patternCount; i++) {
		name = Common::String::format(patterns[i], num);
		if (f.open(Common::Path(name)))
			return true;
	}
	name.clear();
	return false;
}

Common::String cleanLondonApproachPage(const byte *start, uint32 len) {
	Common::String page((const char *)start, len);
	while (!page.empty() &&
		   (page.lastChar() == '\r' || page.lastChar() == '\n' ||
			(byte)page.lastChar() == 0xff))
		page.deleteLastChar();
	return page;
}

bool loadLondonApproachData(uint16 approachId, LondonApproachData &out,
							 bool macintosh) {
	static const char *const kDosPatterns[] = {
		"A%u.BIN"
	};
	static const char *const kMacPatterns[] = {
		"Approaches/a%u.bin",
		"Approaches/A%u.BIN",
		"a%u.bin",
		"A%u.BIN"
	};

	Common::File f;
	Common::String name;
	const bool opened = macintosh
		? openNumberedScriptFile(f, name, approachId, kMacPatterns,
								 ARRAYSIZE(kMacPatterns))
		: openNumberedScriptFile(f, name, approachId, kDosPatterns,
								 ARRAYSIZE(kDosPatterns));
	if (!opened) {
		warning("London approach: cannot open approach %u", approachId);
		return false;
	}

	const uint32 size = f.size();
	if (size < 16) {
		warning("London approach: %s is too short", name.c_str());
		return false;
	}

	Common::Array<byte> data;
	data.resize(size);
	if (f.read(data.data(), size) != size) {
		warning("London approach: short read on %s", name.c_str());
		return false;
	}

	out.videoId = readScriptU16(data.data() + 0, macintosh);
	out.videoX = readScriptU16(data.data() + 2, macintosh);
	out.videoY = readScriptU16(data.data() + 4, macintosh);
	out.textRect = macintosh ? readMacQuickDrawRectBE(data.data() + 6)
							 : readDosRectLE(data.data() + 6);

	const uint16 pageCount = readScriptU16(data.data() + 14, macintosh);
	out.pages.clear();
	uint32 pos = 16;
	// `_DoApproach @ 1717:009b` reads the pages with `_fgets` into 255-byte
	// slots, so each page is one NEWLINE-terminated record, NOT NUL-separated.
	for (uint16 i = 0; i < pageCount && pos < size; i++) {
		const uint32 start = pos;
		while (pos < size && data[pos] != '\n')
			pos++;
		out.pages.push_back(cleanLondonApproachPage(data.data() + start,
													pos - start));
		if (pos < size)
			pos++;  // skip the '\n' separator
	}
	return out.videoId != 0 && !out.pages.empty();
}

bool openLondonApproachFlic(uint16 videoId, Video::FlicDecoder &flic,
							Common::String &name) {
	static const char *const kPatterns[] = {
		"VIDEO%02u.FLC",
		"video%02u.FLC"
	};

	for (uint i = 0; i < ARRAYSIZE(kPatterns); i++) {
		name = Common::String::format(kPatterns[i], videoId);
		if (flic.loadFile(Common::Path(name)))
			return true;
	}
	name.clear();
	return false;
}

bool decodeLondonApproachFirstFrame(uint16 videoId,
									Graphics::ManagedSurface &base,
									byte *palette, bool macintosh) {
	if (macintosh) {
		Video::FlicDecoder flic;
		Common::String name;
		if (!openLondonApproachFlic(videoId, flic, name)) {
			warning("London approach: cannot open FLC video %u", videoId);
			return false;
		}

		flic.start();
		const Graphics::Surface *frame = flic.decodeNextFrame();
		if (!frame) {
			warning("London approach: %s has no first frame", name.c_str());
			return false;
		}

		const byte *fpal = flic.getPalette();
		if (fpal)
			memcpy(palette, fpal, 768);
		base.clear();
		const int w = MIN<int>(frame->w, base.w);
		const int h = MIN<int>(frame->h, base.h);
		base.copyRectToSurface(frame->getPixels(), frame->pitch, 0, 0, w, h);
		return true;
	}

	const Common::String name = Common::String::format("VIDEO%02u.A", videoId);
	ANMDecoder anm;
	if (!anm.open(Common::Path(name))) {
		warning("London approach: cannot open %s", name.c_str());
		return false;
	}

	anm.getPalette8(palette);
	const byte *frame = anm.nextFrame();
	if (!frame) {
		warning("London approach: %s has no first frame", name.c_str());
		return false;
	}

	base.clear();
	const int w = MIN<int>(anm.width(), base.w);
	const int h = MIN<int>(anm.height(), base.h);
	base.copyRectToSurface(frame, anm.width(), 0, 0, w, h);
	return true;
}

byte closestPaletteIndex(const byte *palette, byte r, byte g, byte b,
						 byte fallback) {
	if (!palette)
		return fallback;

	uint bestDist = 0xffffffff;
	byte best = fallback;
	for (uint i = 0; i < 256; i++) {
		const int dr = (int)palette[i * 3 + 0] - r;
		const int dg = (int)palette[i * 3 + 1] - g;
		const int db = (int)palette[i * 3 + 2] - b;
		const uint dist = (uint)(dr * dr + dg * dg + db * db);
		if (dist < bestDist) {
			bestDist = dist;
			best = (byte)i;
		}
	}
	return best;
}

void remapSurfaceColor(Graphics::ManagedSurface &surface, byte from, byte to) {
	if (surface.empty() || from == to)
		return;

	for (int y = 0; y < surface.h; y++) {
		byte *row = (byte *)surface.getBasePtr(0, y);
		for (int x = 0; x < surface.w; x++) {
			if (row[x] == from)
				row[x] = to;
		}
	}
}

// `_DoBigMap @ 20fe:09e7` two stage:
//   Stage 1 (Overview): PIC 0x42 + site icons at MapData[+4/+6]
//     (`_DrawBigMapButtons @ 20fe:0877`). Click in BigMapWindow
//     returns scroll (mouseX*2 - 0x74, mouseY*2 - 0x55).
//   Stage 2 (Detail): PIC 0x43 frame + 0xe9×0xab BIGMAP.PIC viewport
//     at (2,2). Icons stamped at MapData[+8/+0xa] (`_StampButtons @
//     20fe:0d2f`). Click icon = travel.
// MapData entry (14 bytes): +0..3 ???, +4 BigMapX, +6 BigMapY,
//   +8 SmallMapX, +0xa SmallMapY, +0xc crime-flag.
void EEMEngine::bigMapCycleOverviewPalette(bool mac) {
	if (isLondon()) {
		if (mac) {
			// Mac `_UpdateBigMap` rotates ColorTable entries 0xef..0xf2
			// and 0xfc..0xff. Redraw happens after the cycle so Mac endpoint
			// art maps its black pixels to the current black slot.
			cyclePaletteRangeReverse(0xef, 0xf2);
			cyclePaletteRangeReverse(0xfc, 0xff);
		} else {
			cyclePaletteRangeReverse(0xf4, 0xf9);
			cyclePaletteRangeReverse(0xfa, 0xff);
		}
	} else {
		cyclePaletteRangeReverse(0xf7, 0xfa);
		cyclePaletteRangeReverse(0xfb, 0xfe);
	}
}

void EEMEngine::bigMapCycleDetailPalette(bool mac) {
	if (!isLondon())
		return;

	if (mac) {
		cyclePaletteRangeReverse(0xe9, 0xeb);
		cyclePaletteRangeReverse(0xec, 0xef);
		cyclePaletteRangeReverse(0xf0, 0xf2);
	} else {
		cyclePaletteRangeReverse(0xee, 0xf2);
		cyclePaletteRangeReverse(0xea, 0xed);
	}
}

bool EEMEngine::bigMapRunOverview(BigMapOverviewState &state) {
	setInteractiveMouseCursor(false);
	setSitePalette(isLondon() ? 0x3b : 0x24);

	const bool mac = isMacintosh();
	const Common::Rect setupBtnBase = isFloppy()
		? Common::Rect(251, 3, 315, 42)
		: (isLondon() ? Common::Rect(252, 1, 315, 42)
					  : Common::Rect(252, 4, 315, 42));
	const Common::Rect setupBtnRect = scaleDosRectIfMac(*this, setupBtnBase);
	state.window = scaleDosRectIfMac(*this, Common::Rect(0, 0, 247, 192));
	state.zoomX = 0;
	state.zoomY = 0;

	const uint32 mapStartTick = g_system->getMillis();
	drawBigMapOverview(0);
	uint32 mapLastTick = mapStartTick;
	uint32 mapLastCycleTick = mapStartTick;

	while (!shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return false;
			if (ev.type == Common::EVENT_KEYDOWN &&
				ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
				openMainMenuDialog();
				continue;
			}
			if (ev.type != Common::EVENT_LBUTTONDOWN)
				continue;

			if (setupBtnRect.contains(ev.mouse.x, ev.mouse.y)) {
				_nextScreen = kScreenSetup;
				return false;
			}

			if (state.window.contains(ev.mouse.x, ev.mouse.y)) {
				if (mac) {
					state.zoomX = ev.mouse.x - state.window.left;
					state.zoomY = ev.mouse.y - state.window.top;
				} else {
					int sx = ev.mouse.x * 2;
					int sy = ev.mouse.y * 2;
					sx = (sx < 0x75) ? 0 : sx - 0x74;
					sy = (sy < 0x56) ? 0 : sy - 0x55;
					state.zoomX = sx;
					state.zoomY = sy;
				}
				return true;
			}
		}

		const uint32 now = g_system->getMillis();
		if (mac && isLondon() &&
			now - mapLastCycleTick >= kMacMapColorCycleDelayMs) {
			mapLastCycleTick = now;
			bigMapCycleOverviewPalette(mac);
			drawBigMapOverview(now - mapStartTick);
			mapLastTick = now;
		} else if (now - mapLastTick >= 100) {
			mapLastTick = now;
			drawBigMapOverview(now - mapStartTick);
			if (!mac || !isLondon())
				bigMapCycleOverviewPalette(mac);
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	return false;
}

bool EEMEngine::bigMapLoadDetailPixels(Common::Array<byte> &mapPixels,
									   uint16 &mapW, uint16 &mapH) {
	mapW = 0;
	mapH = 0;
	mapPixels.clear();

	if (isMacintosh())
		return loadMacBigMapPixels(mapPixels, mapW, mapH);

	Common::File f;
	if (!f.open(Common::Path("BIGMAP.PIC"))) {
		warning("doBigMap: BIGMAP.PIC missing for detail view");
		return false;
	}
	mapH = f.readUint16LE();
	mapW = f.readUint16LE();
	if (mapW == 0 || mapH == 0)
		return false;
	mapPixels.resize((uint32)mapW * mapH);
	if (f.read(mapPixels.data(), mapPixels.size()) != mapPixels.size()) {
		warning("doBigMap: short read on BIGMAP.PIC for detail view");
		return false;
	}
	return true;
}

void EEMEngine::bigMapInitDetailState(BigMapDetailState &state,
									  const Common::Array<byte> &mapPixels,
									  uint16 mapW, uint16 mapH,
									  const BigMapOverviewState &overview) {
	const bool mac = isMacintosh();

	state.mapPixels = &mapPixels;
	state.mapW = mapW;
	state.mapH = mapH;
	state.mapWinW = mac ? scaleX(0xe9) : 0xe9; // 233
	state.mapWinH = mac ? scaleY(0xab) : 0xab; // 171
	state.mapWinX = mac ? scaleX(2) : 2;
	state.mapWinY = mac ? scaleY(2) : 2;
	state.maxScrollX = MAX<int>(0, (int)mapW - state.mapWinW);
	state.maxScrollY = MAX<int>(0, (int)mapH - state.mapWinH);

	if (mac) {
		state.scrollX = overview.zoomX * 2;
		state.scrollY = overview.zoomY * 2;
		state.scrollX = state.scrollX < kMacBigMapScrollCenterX + 1
			? 0 : state.scrollX - kMacBigMapScrollCenterX;
		state.scrollY = state.scrollY < kMacBigMapScrollCenterY + 1
			? 0 : state.scrollY - kMacBigMapScrollCenterY;
	} else {
		state.scrollX = overview.zoomX;
		state.scrollY = overview.zoomY;
	}
	state.scrollX = MAX<int>(0, MIN<int>(state.maxScrollX, state.scrollX));
	state.scrollY = MAX<int>(0, MIN<int>(state.maxScrollY, state.scrollY));

	state.returnRect = scaleDosRectIfMac(*this,
		Common::Rect(252, 43, kScreenWidth, kScreenHeight));
	state.arrowYUp = scaleDosRectIfMac(*this, Common::Rect(237, 2, 247, 11));
	state.arrowYDown = scaleDosRectIfMac(*this, Common::Rect(237, 163, 247, 172));
	state.arrowXLeft = scaleDosRectIfMac(*this, Common::Rect(2, 175, 12, 185));
	state.arrowXRight = scaleDosRectIfMac(*this, Common::Rect(224, 175, 234, 185));
	state.xSlider = scaleDosRectIfMac(*this, isLondon()
		? Common::Rect(15, 176, 220, 184)
		: Common::Rect(15, 175, 221, 185));
	state.ySlider = scaleDosRectIfMac(*this, isLondon()
		? Common::Rect(238, 16, 246, 158)
		: Common::Rect(237, 14, 247, 160));
	state.setupRect = scaleDosRectIfMac(*this, isFloppy()
		? Common::Rect(251, 3, 315, 42)
		: (isLondon() ? Common::Rect(251, 3, 315, 42)
					  : Common::Rect(252, 4, 315, 42)));

	const int baseArrowStep = isLondon() ? 8 : 16;
	state.arrowStepX = mac ? scaleX(baseArrowStep) : baseArrowStep;
	state.arrowStepY = mac ? scaleY(baseArrowStep) : baseArrowStep;
}

bool EEMEngine::bigMapTrySelectDetailSite(int mouseX, int mouseY,
										  const BigMapDetailState &state) {
	if (mouseX < state.mapWinX || mouseX >= state.mapWinX + state.mapWinW ||
		mouseY < state.mapWinY || mouseY >= state.mapWinY + state.mapWinH)
		return false;

	// Per-site bbox from `_StampButtons` (SmallMap +8/+0xa).
	struct DetailMapHit {
		uint site;
		Common::Rect rect;
	};
	Common::Array<DetailMapHit> hits;
	const bool floppyMap = _mystery.isLoaded() && isFloppy();
	const bool macMap = isMacintosh() && _mystery.usesCompactMacData();
	for (uint i = 0; i < _mystery.numSites(); i++) {
		// On-map flag alone, matching `_SearchMapButtons`.
		if (!_mystery._onSites[i])
			continue;
		const byte *entry = _mystery.mapEntry(i);
		if (!entry)
			continue;
		BigMapEntryInfo info;
		if (!readBigMapEntryInfo(entry, floppyMap, macMap, info))
			continue;

		Picture button;
		int bw = 16;
		int bh = 16;
		if (_buttonArchive.loadEntry(info.buttonId, button)) {
			bw = button.surface.w;
			bh = button.surface.h;
		}
		const int sx = (int)info.detailX - state.scrollX + state.mapWinX;
		const int sy = (int)info.detailY - state.scrollY + state.mapWinY;
		const Common::Rect r(sx, sy, sx + bw, sy + bh);
		if (r.intersects(Common::Rect(state.mapWinX, state.mapWinY,
				state.mapWinX + state.mapWinW,
				state.mapWinY + state.mapWinH))) {
			DetailMapHit hit = { i, r };
			hits.push_back(hit);
		}
	}
	Common::sort(hits.begin(), hits.end(),
		[](const DetailMapHit &a, const DetailMapHit &b) {
			if (a.rect.top != b.rect.top)
				return a.rect.top < b.rect.top;
			return a.rect.left < b.rect.left;
		});
	for (uint i = 0; i < hits.size(); i++) {
		if (hits[i].rect.contains(mouseX, mouseY)) {
			_mystery._lastSite = _mystery._siteNumber;
			_mystery._siteNumber = (uint16)hits[i].site;
			return true;
		}
	}
	return false;
}

void EEMEngine::bigMapHandleDetailKey(const Common::Event &ev,
									  BigMapDetailState &state,
									  bool &dirty) {
	switch (ev.kbd.keycode) {
	case Common::KEYCODE_ESCAPE:
		openMainMenuDialog();
		dirty = true;
		break;
	case Common::KEYCODE_LEFT:
		state.scrollX = MAX<int>(0, state.scrollX - state.arrowStepX);
		dirty = true;
		break;
	case Common::KEYCODE_RIGHT:
		state.scrollX = MIN<int>(state.maxScrollX,
								 state.scrollX + state.arrowStepX);
		dirty = true;
		break;
	case Common::KEYCODE_UP:
		state.scrollY = MAX<int>(0, state.scrollY - state.arrowStepY);
		dirty = true;
		break;
	case Common::KEYCODE_DOWN:
		state.scrollY = MIN<int>(state.maxScrollY,
								 state.scrollY + state.arrowStepY);
		dirty = true;
		break;
	default:
		break;
	}
}

bool EEMEngine::bigMapHandleDetailMouseDown(const Common::Event &ev,
											BigMapDetailState &state,
											bool &returnToOverview,
											bool &dirty) {
	setInteractiveMouseCursor(
		state.returnRect.contains(ev.mouse.x, ev.mouse.y) ||
		state.setupRect.contains(ev.mouse.x, ev.mouse.y));

	if (state.setupRect.contains(ev.mouse.x, ev.mouse.y)) {
		_nextScreen = kScreenSetup;
		setInteractiveMouseCursor(false);
		return false;
	}
	if (state.returnRect.contains(ev.mouse.x, ev.mouse.y)) {
		returnToOverview = true;
		return true;
	}
	if (state.arrowYUp.contains(ev.mouse.x, ev.mouse.y)) {
		state.scrollY = MAX<int>(0, state.scrollY - state.arrowStepY);
		dirty = true;
		return true;
	}
	if (state.arrowYDown.contains(ev.mouse.x, ev.mouse.y)) {
		state.scrollY = MIN<int>(state.maxScrollY,
								 state.scrollY + state.arrowStepY);
		dirty = true;
		return true;
	}
	if (state.arrowXLeft.contains(ev.mouse.x, ev.mouse.y)) {
		state.scrollX = MAX<int>(0, state.scrollX - state.arrowStepX);
		dirty = true;
		return true;
	}
	if (state.arrowXRight.contains(ev.mouse.x, ev.mouse.y)) {
		state.scrollX = MIN<int>(state.maxScrollX,
								 state.scrollX + state.arrowStepX);
		dirty = true;
		return true;
	}
	if (state.xSlider.contains(ev.mouse.x, ev.mouse.y)) {
		if (state.maxScrollX > 0) {
			const int t = ev.mouse.x - state.xSlider.left;
			const int tw = state.xSlider.width();
			state.scrollX = MAX<int>(0, MIN<int>(state.maxScrollX,
				t * state.maxScrollX / MAX<int>(1, tw)));
			dirty = true;
		}
		return true;
	}
	if (state.ySlider.contains(ev.mouse.x, ev.mouse.y)) {
		if (state.maxScrollY > 0) {
			const int t = ev.mouse.y - state.ySlider.top;
			const int th = state.ySlider.height();
			state.scrollY = MAX<int>(0, MIN<int>(state.maxScrollY,
				t * state.maxScrollY / MAX<int>(1, th)));
			dirty = true;
		}
		return true;
	}
	if (bigMapTrySelectDetailSite(ev.mouse.x, ev.mouse.y, state)) {
		setInteractiveMouseCursor(false);
		return false;
	}

	return true;
}

bool EEMEngine::bigMapHandleDetailEvent(const Common::Event &ev,
										BigMapDetailState &state,
										bool &returnToOverview,
										bool &dirty) {
	if (ev.type == Common::EVENT_QUIT ||
		ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
		setInteractiveMouseCursor(false);
		return false;
	}

	if (ev.type == Common::EVENT_KEYDOWN) {
		bigMapHandleDetailKey(ev, state, dirty);
		return true;
	}

	if (ev.type == Common::EVENT_MOUSEMOVE) {
		setInteractiveMouseCursor(
			state.returnRect.contains(ev.mouse.x, ev.mouse.y) ||
			state.setupRect.contains(ev.mouse.x, ev.mouse.y));
		return true;
	}

	if (ev.type == Common::EVENT_LBUTTONDOWN)
		return bigMapHandleDetailMouseDown(ev, state, returnToOverview,
										   dirty);

	return true;
}

bool EEMEngine::bigMapRunDetail(BigMapDetailState &state) {
	if (!state.mapPixels)
		return false;

	const bool mac = isMacintosh();
	setSitePalette(isLondon() ? 0x3a : 0x23);

	state.startTick = g_system->getMillis();
	state.lastDrawTick = state.startTick;
	state.lastCycleTick = state.startTick;
	drawBigMapDetail(state.scrollX, state.scrollY, *state.mapPixels,
					 state.mapW, state.mapH, 0);

	const Common::Point detailMouse =
		g_system->getEventManager()->getMousePos();
	setInteractiveMouseCursor(
		state.returnRect.contains(detailMouse.x, detailMouse.y) ||
		state.setupRect.contains(detailMouse.x, detailMouse.y));

	bool returnToOverview = false;
	while (!shouldQuit() && !returnToOverview) {
		Common::Event ev;
		bool dirty = false;
		bool cycleTick = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (!bigMapHandleDetailEvent(ev, state, returnToOverview, dirty))
				return false;
			if (returnToOverview)
				break;
		}
		if (returnToOverview)
			break;

		const uint32 now = g_system->getMillis();
		if (now - state.lastDrawTick >= 100) {
			state.lastDrawTick = now;
			dirty = true;
		}
		if (isLondon()) {
			const uint32 cycleDelay = mac ? kMacMapColorCycleDelayMs : 100;
			if (now - state.lastCycleTick >= cycleDelay) {
				state.lastCycleTick = now;
				cycleTick = true;
				dirty = true;
			}
		}
		if (cycleTick)
			bigMapCycleDetailPalette(mac);
		if (dirty)
			drawBigMapDetail(state.scrollX, state.scrollY, *state.mapPixels,
							 state.mapW, state.mapH,
							 now - state.startTick);
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	return returnToOverview;
}

void EEMEngine::doBigMap() {
	if (!_mystery.isLoaded())
		return;

	if (isLondon()) {
		_mystery._pendingSiteJump = 0;
		_mystery._siteReturnDepth = 0;
		memset(_mystery._siteReturnStack, 0, sizeof(_mystery._siteReturnStack));
	}

	CursorMan.showMouse(true);

	while (!shouldQuit()) {
		BigMapOverviewState overview;
		if (!bigMapRunOverview(overview))
			return;

		Common::Array<byte> mapPixels;
		uint16 mapW = 0;
		uint16 mapH = 0;
		if (!bigMapLoadDetailPixels(mapPixels, mapW, mapH))
			return;

		BigMapDetailState detail;
		bigMapInitDetailState(detail, mapPixels, mapW, mapH, overview);
		if (!bigMapRunDetail(detail))
			return;
	}
}

bool EEMEngine::doLondonApproach(uint16 approachId) {
	if (!isLondon())
		return false;

	LondonApproachData data;
	const bool mac = isMacintosh();
	if (!loadLondonApproachData(approachId, data, mac))
		return false;

	const int sw = screenWidth();
	const int sh = screenHeight();

	Graphics::ManagedSurface base(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	byte palette[768] = {};
	const bool haveVideo =
		decodeLondonApproachFirstFrame(data.videoId, base, palette, mac);
	if (!haveVideo)
		base.clear();
	if (mac) {
		Picture background;
		const uint16 backgroundPic =
			kMacLondonApproachBackgroundBasePic + data.videoId;
		if (_picsArchive.getPicture(backgroundPic, background) &&
			!background.surface.empty()) {
			if (haveVideo) {
				const byte black =
					closestPaletteIndex(palette, 0x00, 0x00, 0x00, 0xfe);
				remapSurfaceColor(background.surface, 0xff, black);
			}
			base.clear();
			base.simpleBlitFrom(background.surface);
		}
	}

	Picture buttonPics[ARRAYSIZE(kLondonApproachButtons)];
	bool haveButtons[ARRAYSIZE(kLondonApproachButtons)] = {};
	for (uint i = 0; i < ARRAYSIZE(kLondonApproachButtons); i++) {
		// DOS `_DoApproach` loads PICS 0x361/0x362/0x35f/0x360 as the
		// four control sprites. In the Mac London data those IDs are
		// full-screen scrapbook pages, not buttons.
		haveButtons[i] = !mac && _picsArchive.getPicture(
			kLondonApproachButtons[i].picId, buttonPics[i]);
	}

	auto buttonRect = [&](uint idx) {
		return kLondonApproachButtons[idx].rect();
	};
	auto buttonPoint = [&](uint idx) {
		const LondonApproachButton &b = kLondonApproachButtons[idx];
		return Common::Point(b.x, b.y);
	};
	const int movieX = mac ? kMacLondonApproachMovieX : (int)data.videoX;
	const int movieY = mac ? kMacLondonApproachMovieY : (int)data.videoY;
	auto approachTextRect = [&]() {
		Common::Rect textRect = data.textRect;
		if (mac)
			textRect = scaleRect(textRect);
		return textRect.findIntersectingRect(Common::Rect(sw, sh));
	};
	auto macApproachButton = [&](const Common::Point &mouse) {
		if (mouse.x >= kMacLondonApproachDoneLeft &&
			mouse.x < kMacLondonApproachDoneRight &&
			mouse.y >= kMacLondonApproachDoneTop &&
			mouse.y < kMacLondonApproachDoneBottom)
			return 0;
		if (mouse.x >= kMacLondonApproachPlayLeft &&
			mouse.x < kMacLondonApproachPlayRight &&
			mouse.y >= kMacLondonApproachPlayTop &&
			mouse.y < kMacLondonApproachPlayBottom)
			return 1;
		return -1;
	};
	const byte approachTextColor =
		closestPaletteIndex(haveVideo ? palette : nullptr, 0, 0, 0,
							mac ? (byte)0xfe : (byte)1);

	auto drawApproachOverlay = [&](Graphics::ManagedSurface &scratch,
								   uint page) {
		const Common::Rect textRect = approachTextRect();
		if (!textRect.isEmpty()) {
			if (page < data.pages.size()) {
				Common::Array<Common::String> wrapped;
				_font.wordWrapText(data.pages[page], MAX<int>(8, textRect.width()),
								   wrapped);
				const int lineH = _font.getFontHeight();
				const int maxLines = MAX<int>(1, textRect.height() / lineH);
				for (uint i = 0; i < wrapped.size() && (int)i < maxLines; i++) {
					_font.drawString(&scratch, wrapped[i], textRect.left,
									 textRect.top + (int)i * lineH,
									 textRect.width(),
									 approachTextColor);
				}
			}
		}

		for (uint i = 0; i < ARRAYSIZE(kLondonApproachButtons); i++) {
			if (haveButtons[i]) {
				const Common::Point p = buttonPoint(i);
				scratch.transBlitFrom(buttonPics[i].surface, p,
									  (uint32)(byte)(buttonPics[i].flags >> 8));
			}
		}
	};

	auto drawScreen = [&](uint page) {
		Graphics::ManagedSurface scratch(sw, sh,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		scratch.simpleBlitFrom(base);
		drawApproachOverlay(scratch, page);

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, sw, sh);
		g_system->updateScreen();
	};

	auto playVideo = [&](uint page) {
		if (mac) {
			Video::FlicDecoder flic;
			Common::String name;
			if (!openLondonApproachFlic(data.videoId, flic, name)) {
				warning("London approach: cannot open FLC video %u",
						data.videoId);
				return;
			}

			flic.start();
			while (!shouldQuit() && !flic.endOfVideo()) {
				const Graphics::Surface *frame = flic.decodeNextFrame();
				if (!frame)
					break;
				Graphics::ManagedSurface scratch(sw, sh,
					Graphics::PixelFormat::createFormatCLUT8());
				scratch.clear();
				scratch.simpleBlitFrom(base);
				const int copyW = MIN<int>(frame->w, sw - movieX);
				const int copyH = MIN<int>(frame->h, sh - movieY);
				if (copyW <= 0 || copyH <= 0)
					break;
				scratch.copyRectToSurface(frame->getPixels(), frame->pitch,
										  movieX, movieY,
										  copyW, copyH);
				drawApproachOverlay(scratch, page);
				g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
										   0, 0, sw, sh);
				if (flic.hasDirtyPalette()) {
					const byte *fpal = flic.getPalette();
					if (fpal)
						g_system->getPaletteManager()->setPalette(fpal, 0, 256);
				}
				g_system->updateScreen();

				const uint32 start = g_system->getMillis();
				const uint32 delay = MAX<uint32>(10, flic.getTimeToNextFrame());
				bool skip = false;
				while (g_system->getMillis() - start < delay && !skip) {
					Common::Event ev;
					while (g_system->getEventManager()->pollEvent(ev)) {
						if (ev.type == Common::EVENT_QUIT ||
							ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
							return;
						if (ev.type == Common::EVENT_LBUTTONDOWN ||
							ev.type == Common::EVENT_KEYDOWN) {
							skip = true;
							break;
						}
					}
					g_system->delayMillis(5);
				}
				if (skip)
					break;
			}
			return;
		}

		const Common::String name =
			Common::String::format("VIDEO%02u.A", data.videoId);
		ANMDecoder anm;
		if (!anm.open(Common::Path(name))) {
			warning("London approach: cannot open %s", name.c_str());
			return;
		}

		// Frame 0 is already the static background. Replay frames 1..end
		// with the approach text/buttons composited above it.
		(void)anm.nextFrame();
		const int copyW = MIN<int>(anm.width(), sw - (int)data.videoX);
		const int copyH = MIN<int>(anm.height(), sh - (int)data.videoY);
		if (copyW <= 0 || copyH <= 0)
			return;

		while (!shouldQuit()) {
			const byte *frame = anm.nextFrame();
			if (!frame)
				break;
			Graphics::ManagedSurface scratch(sw, sh,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.clear();
			scratch.copyRectToSurface(frame, anm.width(),
									  data.videoX, data.videoY,
									  copyW, copyH);
			drawApproachOverlay(scratch, page);
			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, sw, sh);
			g_system->updateScreen();

			const uint32 start = g_system->getMillis();
			bool skip = false;
			while (g_system->getMillis() - start < 120 && !skip) {
				Common::Event ev;
				while (g_system->getEventManager()->pollEvent(ev)) {
					if (ev.type == Common::EVENT_QUIT ||
						ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
						return;
					}
					if (ev.type == Common::EVENT_LBUTTONDOWN ||
						ev.type == Common::EVENT_KEYDOWN) {
						skip = true;
						break;
					}
				}
				g_system->delayMillis(5);
			}
			if (skip)
				break;
		}
	};

	fadeCurrentPaletteToBlack();
	CursorMan.showMouse(true);
	setSiteHotspotCursorId(6);
	if (_music && _voiceOn)
		_music->playMus(0x27, /* loop= */ true);

	uint page = 0;
	drawScreen(page);
	if (haveVideo)
		fadePaletteFromBlack(palette);
	else
		setSitePalette(0x3b);
	if (haveVideo) {
		playVideo(page);
		drawScreen(page);
	}
	bool done = false;
	uint32 lastShimmer = g_system->getMillis();
	while (!shouldQuit() && !done) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				done = true;
				break;
			}
			if (ev.type == Common::EVENT_MOUSEMOVE) {
				bool overButton = false;
				if (mac) {
					overButton = macApproachButton(
						Common::Point(ev.mouse.x, ev.mouse.y)) >= 0;
				} else {
					for (uint i = 0; i < ARRAYSIZE(kLondonApproachButtons); i++) {
						if (buttonRect(i).contains(ev.mouse.x, ev.mouse.y)) {
							overButton = true;
							break;
						}
					}
				}
				setInteractiveMouseCursor(overButton);
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE ||
					ev.kbd.keycode == Common::KEYCODE_RETURN ||
					ev.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					done = true;
					break;
				}
				if (ev.kbd.keycode == Common::KEYCODE_SPACE) {
					playVideo(page);
					drawScreen(page);
				} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT ||
						   ev.kbd.keycode == Common::KEYCODE_DOWN) {
					if (page + 1 < data.pages.size()) {
						page++;
						drawScreen(page);
					}
				} else if (ev.kbd.keycode == Common::KEYCODE_LEFT ||
						   ev.kbd.keycode == Common::KEYCODE_UP) {
					if (page > 0) {
						page--;
						drawScreen(page);
					}
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				if (mac) {
					const int button = macApproachButton(
						Common::Point(ev.mouse.x, ev.mouse.y));
					if (button == 0) {
						done = true;
						break;
					}
					if (button == 1) {
						playVideo(page);
						drawScreen(page);
					}
					continue;
				}
				if (buttonRect(0).contains(ev.mouse.x, ev.mouse.y)) {
					done = true;
					break;
				}
				if (buttonRect(1).contains(ev.mouse.x, ev.mouse.y)) {
					playVideo(page);
					drawScreen(page);
				} else if (buttonRect(2).contains(ev.mouse.x, ev.mouse.y)) {
					if (page + 1 < data.pages.size()) {
						page++;
						drawScreen(page);
					}
				} else if (buttonRect(3).contains(ev.mouse.x, ev.mouse.y)) {
					if (page > 0) {
						page--;
						drawScreen(page);
					}
				}
			}
		}
		if (haveVideo && !mac) {
			const uint32 now = g_system->getMillis();
			if (now - lastShimmer >= 70) {
				lastShimmer = now;
				cyclePaletteRange(0xF3, 0xF7);
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	setInteractiveMouseCursor(false);
	setSiteHotspotCursorId(0);
	stopMusic();
	return true;
}

void EEMEngine::drawBigMapOverview(uint32 elapsedMs) {
	const bool mac = isMacintosh();
	const int sw = screenWidth();
	const int sh = screenHeight();
	Graphics::ManagedSurface scratch(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	Picture frame;
	if (_picsArchive.getPicture(0x42, frame)) {
		if (mac)
			remapMacSurfaceEndpoints(frame.surface, getMacSpritePaletteMap());
		scratch.simpleBlitFrom(frame.surface);
	}

	// Marker PICs from `_main`:
	//   EEM1 CD @ 1a35:0f59: Done=0x20d, Site=0xc5, Crime=0xc6.
	//   EEM2 CD @ 1abf:11a6: Done=0x006, Site=0xc5, Crime=0xc6.
	Picture done;
	Picture normal;
	Picture crimeM;
	const bool haveDone = _picsArchive.getPicture(isLondon() ? 0x006 : 0x20d, done) &&
		done.surface.w < 64 && done.surface.h < 64;
	const bool haveNormal = _picsArchive.getPicture(0xc5, normal) &&
		normal.surface.w < 64 && normal.surface.h < 64;
	const bool haveCrime = _picsArchive.getPicture(0xc6, crimeM) &&
		crimeM.surface.w < 64 && crimeM.surface.h < 64;

	for (uint i = 0; i < _mystery.numSites(); i++) {
		// `_DrawBigMapButtons` gates markers on the on-map flag alone, never the
		// current site: a sublocation (in-site jump, never flagged) must not draw.
		if (!_mystery._onSites[i])
			continue;
		const byte *entry = _mystery.mapEntry(i);
		if (!entry)
			continue;

		const bool floppy  = _mystery.isLoaded() && isFloppy();
		BigMapEntryInfo info;
		if (!readBigMapEntryInfo(entry, floppy,
								 mac && _mystery.usesCompactMacData(), info))
			continue;
		const bool isDone = (i < Mystery::kVisitedSiteCap)
							 && _mystery._visitedSite[i];

		const Picture *m = nullptr;
		bool useVisitedColors = false;
		if (isDone && haveDone) {
			m = &done;
		} else if (isDone && haveNormal) {
			m = &normal;
			useVisitedColors = true;
		} else if (info.crime != 0 && haveCrime) {
			m = &crimeM;
		} else if (haveNormal) {
			m = &normal;
		}

		if (m) {
			blitBigMapMarker(scratch, *m, (int)info.overviewX,
							  (int)info.overviewY,
							  useVisitedColors);
		} else {
			const int mx = (int)info.overviewX;
			const int my = (int)info.overviewY;
			const Common::Rect mark(mx - 3, my - 3, mx + 4, my + 4);
			scratch.fillRect(mark, 0x0F);
		}
	}

	const uint kMapAniId = (_partner == kPartnerJake) ? 0x14 : 0x12;
	Animation mapAnim;
	if (_aniArchive.loadAnimation(kMapAniId, mapAnim) && !mapAnim.empty()) {
		const uint frameIdx = bigMapPartnerFrameAtTick((uint)mapAnim.size(),
													   elapsedMs, isLondon());

		const int anchorX = mac ? scaleX(0xfd) : 0xfd;
		const int anchorY = mac ? scaleY(0x50) : 0x50;
		if (mac)
			blitMacAnimFrameAnchored(scratch.surfacePtr(), mapAnim[frameIdx],
									 anchorX, anchorY);
		else
			blitAnimFrameAnchored(scratch.surfacePtr(), mapAnim[frameIdx],
								  anchorX, anchorY);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, sw, sh);
	g_system->updateScreen();
}

void EEMEngine::drawBigMapDetail(int scrollX, int scrollY,
								 const Common::Array<byte> &mapPixels,
								 uint16 mapW, uint16 mapH,
								 uint32 elapsedMs) {

	const bool mac = isMacintosh();
	const int sw = screenWidth();
	const int sh = screenHeight();
	const int kMapWinW = mac ? scaleX(0xe9) : 0xe9;
	const int kMapWinH = mac ? scaleY(0xab) : 0xab;
	const int kMapWinX = mac ? scaleX(2) : 2;
	const int kMapWinY = mac ? scaleY(2) : 2;

	Graphics::ManagedSurface scratch(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	Picture frame;
	if (_picsArchive.getPicture(0x43, frame)) {
		// The frame and its buttons use the normal 0x00=white / 0xFF=black
		// convention, but the detail-map ColorTable (0x23) swaps those endpoints,
		// so normalize them or the SETUP button's black border renders white.
		if (mac)
			remapMacSurfaceEndpoints(frame.surface, getMacSpritePaletteMap());
		scratch.simpleBlitFrom(frame.surface);
	}

	const int copyW = MIN<int>(mapW - scrollX, kMapWinW);
	const int copyH = MIN<int>(mapH - scrollY, kMapWinH);
	scratch.copyRectToSurface(mapPixels.data() + scrollY * mapW + scrollX,
							  mapW, kMapWinX, kMapWinY, copyW, copyH);

	const bool floppyMap = _mystery.isLoaded() && isFloppy();
	for (uint i = 0; i < _mystery.numSites(); i++) {
		// `_DrawBigMapButtons` gates markers on the on-map flag alone, never the
		// current site: a sublocation (in-site jump, never flagged) must not draw.
		if (!_mystery._onSites[i])
			continue;
		const byte *entry = _mystery.mapEntry(i);
		if (!entry)
			continue;
		BigMapEntryInfo info;
		if (!readBigMapEntryInfo(entry, floppyMap,
								 mac && _mystery.usesCompactMacData(), info))
			continue;
		Picture button;
		if (!_buttonArchive.loadEntry(info.buttonId, button))
			continue;
		const int sx = (int)info.detailX - scrollX + kMapWinX;
		const int sy = (int)info.detailY - scrollY + kMapWinY;
		// Crop the button blit against the viewport.
		const int x0 = MAX<int>(sx, kMapWinX);
		const int y0 = MAX<int>(sy, kMapWinY);
		const int x1 = MIN<int>(sx + button.surface.w, kMapWinX + kMapWinW);
		const int y1 = MIN<int>(sy + button.surface.h, kMapWinY + kMapWinH);
		if (x1 > x0 && y1 > y0) {
			const Common::Rect srcRect(x0 - sx, y0 - sy, x1 - sx, y1 - sy);
			scratch.transBlitFrom(button.surface, srcRect,
								  Common::Point(x0, y0),
								  (uint32)(byte)(button.flags >> 8));
		}
	}

	const uint kDetailAniId = (_partner == kPartnerJake) ? 0x13 : 0x11;
	Animation detailAnim;
	if (_aniArchive.loadAnimation(kDetailAniId, detailAnim) &&
		!detailAnim.empty()) {
		const uint frameIdx = bigMapDetailPartnerFrameAtTick(
				(uint)detailAnim.size(), elapsedMs);
		const int anchorX = mac ? scaleX(0x101) : 0x101;
		const int anchorY = mac ? scaleY(0x50) : 0x50;
		if (mac)
			blitMacBigMapPartnerFrame(scratch, detailAnim[frameIdx],
									  anchorX, anchorY);
		else
			blitAnimFrameAnchored(scratch.surfacePtr(),
								  detailAnim[frameIdx],
								  anchorX, anchorY);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, sw, sh);
	g_system->updateScreen();
}


} // End of namespace EEM
