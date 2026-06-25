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
#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"

#include "video/flic_decoder.h"

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/music.h"
#include "eem/site.h"

namespace EEM {

struct GallerySlot { int x; int y; };
const GallerySlot kGallerySlots[5] = {
	{  83,  14 }, // 0
	{ 155,  14 }, // 1
	{ 227,  14 }, // 2
	{ 119,  90 }, // 3
	{ 191,  90 }  // 4
};

const GallerySlot kMacGallerySlots[5] = {
	{ 133,  27 }, // 0
	{ 248,  27 }, // 1
	{ 363,  27 }, // 2
	{ 190, 173 }, // 3
	{ 306, 173 }  // 4
};

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

static uint16 readScriptU16(const byte *p, bool bigEndian) {
	return bigEndian ? READ_BE_UINT16(p) : READ_LE_UINT16(p);
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
	const int16 x1 = (int16)readScriptU16(data.data() + 6, macintosh);
	const int16 y1 = (int16)readScriptU16(data.data() + 8, macintosh);
	const int16 x2 = (int16)readScriptU16(data.data() + 10, macintosh);
	const int16 y2 = (int16)readScriptU16(data.data() + 12, macintosh);
	out.textRect = Common::Rect(x1, y1, x2, y2);

	const uint16 pageCount = readScriptU16(data.data() + 14, macintosh);
	out.pages.clear();
	uint32 pos = 16;
	// `_DoApproach @ 1717:009b` reads the pages with `_fgets` into 255-byte
	// slots, so each page is one NEWLINE-terminated record, NOT NUL-separated.
	for (uint16 i = 0; i < pageCount && pos < size; i++) {
		const uint32 start = pos;
		while (pos < size && data[pos] != '\n')
			pos++;
		out.pages.push_back(Common::String((const char *)data.data() + start,
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

// Setup-screen highlight rects
constexpr Common::Rect kSetupKid1Rect    (Common::Point( 99,  44),  49,  8);
constexpr Common::Rect kSetupKid2Rect    (Common::Point( 99,  54),  49,  8);
constexpr Common::Rect kSetupSoundOnRect (Common::Point(106,  86),  19,  8);
constexpr Common::Rect kSetupSoundOffRect(Common::Point(106,  96),  19,  8);

// EEM2/London setup highlights
constexpr Common::Rect kLonSetJake    (Common::Point( 99,  44), 49, 8); // 0x13ec
constexpr Common::Rect kLonSetJenny   (Common::Point( 99,  54), 49, 8); // 0x13f4
constexpr Common::Rect kLonSetVoiceOn (Common::Point(106,  68), 20, 8); // 0x13fc
constexpr Common::Rect kLonSetVoiceOff(Common::Point(106,  68), 40, 8); // 0x1404
constexpr Common::Rect kLonSetMusicOn (Common::Point(106,  84), 20, 8); // 0x141c
constexpr Common::Rect kLonSetMusicOff(Common::Point(128,  85), 18, 7); // 0x1424
constexpr Common::Rect kLonSetHiOn    (Common::Point(106, 110), 29, 8); // 0x1414
constexpr Common::Rect kLonSetHiOff   (Common::Point(106, 100), 29, 8); // 0x140c

void swapColors(Graphics::ManagedSurface &dst,
					   const Common::Rect &r, byte from, byte to) {
	const int x1 = MAX<int>(0, r.left);
	const int y1 = MAX<int>(0, r.top);
	const int x2 = MIN<int>(dst.w, r.right);
	const int y2 = MIN<int>(dst.h, r.bottom);
	for (int y = y1; y < y2; y++) {
		byte *row = (byte *)dst.getBasePtr(0, y);
		for (int x = x1; x < x2; x++) {
			if (row[x] == from)
				row[x] = to;
		}
	}
}

// TODO: change Pda naming to TRAVIS
struct PdaPartnerSpec {
	uint16 scriptId;
	uint16 animJake;
	uint16 animJenny;
	int    anchorX;
	int    anchorY;
};
constexpr PdaPartnerSpec kPdaGalleryPartner { 0x02, 0x02, 0x10, 5, 0x50 };
constexpr PdaPartnerSpec kPdaNotebookPartner{ 0x01, 0x01, 0x0b, 5, 0x50 };

// Lookup current frame for spec; returns nullptr if anim load fails. `outAni`
// owns the loaded cells; the returned pointer is valid until `outAni` goes
// out of scope.
const Picture *partnerFrameFor(DBDArchive &aniArchive, uint8 partner,
							   const PdaPartnerSpec &spec, uint32 tickMs,
							   Animation &outAni) {
	const uint animId = (partner == 0) ? spec.animJake : spec.animJenny;
	if (!aniArchive.loadAnimation(animId, outAni) || outAni.empty())
		return nullptr;
	const uint frameIdx = partnerFrameAtTick(spec.scriptId,
											  (uint)outAni.size(), tickMs);
	return &outAni[frameIdx];
}

int scalePdaAnchor(int value, int target, int source) {
	const bool negative = value < 0;
	const int magnitude = negative ? -value : value;
	const int scaled = (magnitude * target + source / 2) / source;
	return negative ? -scaled : scaled;
}

void blitPdaPartner(Graphics::ManagedSurface &dst, DBDArchive &aniArchive,
					uint8 partner, const PdaPartnerSpec &spec,
					uint32 tickMs, bool mac = false) {
	Animation ani;
	if (const Picture *fr = partnerFrameFor(aniArchive, partner, spec,
											tickMs, ani)) {
		if (mac) {
			const int anchorX =
				scalePdaAnchor(spec.anchorX, kMacScreenWidth, kScreenWidth);
			const int anchorY =
				scalePdaAnchor(spec.anchorY, kMacScreenHeight, kScreenHeight);
			blitMacAnimFrameAnchored(dst.surfacePtr(), *fr, anchorX,
									 anchorY);
		} else {
			blitAnimFrameAnchored(dst.surfacePtr(), *fr, spec.anchorX,
								  spec.anchorY);
		}
	}
}

void blitPdaPartner(Graphics::Surface *screen, DBDArchive &aniArchive,
					uint8 partner, const PdaPartnerSpec &spec,
					uint32 tickMs, bool mac = false) {
	Animation ani;
	if (const Picture *fr = partnerFrameFor(aniArchive, partner, spec,
											tickMs, ani)) {
		if (mac) {
			const int anchorX =
				scalePdaAnchor(spec.anchorX, kMacScreenWidth, kScreenWidth);
			const int anchorY =
				scalePdaAnchor(spec.anchorY, kMacScreenHeight, kScreenHeight);
			blitMacAnimFrameAnchored(screen, *fr, anchorX, anchorY);
		} else {
			blitAnimFrameAnchored(screen, *fr, spec.anchorX, spec.anchorY);
		}
	}
}

bool playMacScrapbookPartnerAnimation(EEMEngine *vm,
									  Graphics::ManagedSurface &base) {
	if (!vm || !vm->isMacintosh())
		return false;

	g_system->copyRectToScreen(base.getPixels(), base.pitch, 0, 0,
							   base.w, base.h);
	g_system->updateScreen();

	const uint animId =
		(vm->getPartnerIndex() == kPartnerJake) ? 0x17 : 0x3b;
	const int anchorX =
		(vm->getPartnerIndex() == kPartnerJake) ? 0x144 : 0x146;
	const int anchorY =
		(vm->getPartnerIndex() == kPartnerJake) ? 0x0d8 : 0x0d6;
	Animation anim;
	if (!vm->getAni().loadAnimation(animId, anim) || anim.empty())
		return false;

	const uint kFrameDelayMs = 140;
	bool skip = false;
	for (uint frame = 0; frame < anim.size() && !vm->shouldQuit() && !skip;
		 frame++) {
		Graphics::ManagedSurface scratch(base.w, base.h,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(*base.surfacePtr());

		// The Mac executable drives these endgame partner cells with the
		// same script used by the case-intro desk animation.
		const uint cell = partnerFrameAtTick(0x17, (uint)anim.size(),
											 frame * kFrameDelayMs);
		blitMacAnimFrameAnchored(scratch.surfacePtr(), anim[cell],
								  anchorX, anchorY);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, scratch.w, scratch.h);
		g_system->updateScreen();

		const uint32 wakeup = g_system->getMillis() + kFrameDelayMs;
		while (g_system->getMillis() < wakeup && !vm->shouldQuit() &&
			   !skip) {
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
					return true;
				if (ev.type == Common::EVENT_KEYDOWN ||
					ev.type == Common::EVENT_LBUTTONDOWN) {
					skip = true;
					break;
				}
			}
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}

	return true;
}

constexpr Common::Rect kEndingPrevPageRect(Common::Point(0, 0), 28, kScreenHeight);
constexpr Common::Rect kEndingNextPageRect(Common::Point(292, 0), 28, kScreenHeight);
constexpr uint16 kFloppyEndingBackgroundPic = 0x8b;
constexpr uint16 kFirstTryBadgePic = 0x205;
constexpr Common::Point kFirstTryBadgePos(0x1e, 9);
constexpr uint kMacMysteryDataTableOffset = 0x08cd;
constexpr uint kMacMysteryDataMysteryCount = 55;
constexpr uint kMacMysteryDataEndingCount = 55;

constexpr Common::Rect kPdaHelpRect(Common::Point(93, 174), 22, 16);
constexpr Common::Rect kPdaNotebookRect(Common::Point(134, 174), 21, 16);
constexpr Common::Rect kPdaGalleryRect(Common::Point(157, 174), 21, 16);
constexpr Common::Rect kPdaAccuseRect(Common::Point(180, 174), 21, 16);
constexpr Common::Rect kPdaPageNextRect(Common::Point(204, 174), 20, 16);
constexpr Common::Rect kPdaPagePrevRect(Common::Point(226, 174), 21, 16);
constexpr Common::Rect kPdaHelp2Rect(Common::Point(267, 174), 21, 16);
constexpr Common::Rect kPdaLondonCloseRect(Common::Point(0, 0), 66, 79);

// Mac _NotebookRect and _NoteButtons table at 0x177e. The coordinates are
// native QuickDraw rects; keep them exact instead of deriving them from the
// rounded DOS scaler.
constexpr Common::Rect kMacNotebookTextRect(Common::Point(125, 23), 336, 269);
constexpr Common::Rect kMacPdaNotebookRect(Common::Point(214, 334), 34, 30);
constexpr Common::Rect kMacPdaHelpRect(Common::Point(149, 334), 35, 30);
constexpr Common::Rect kMacPdaGalleryRect(Common::Point(251, 334), 34, 30);
constexpr Common::Rect kMacPdaPartnerHeadHintRect(Common::Point(13, 154), 57, 57);
constexpr Common::Rect kMacPdaAccuseRect(Common::Point(288, 334), 33, 30);
constexpr Common::Rect kMacPdaPageNextRect(Common::Point(325, 334), 33, 30);
constexpr Common::Rect kMacPdaPagePrevRect(Common::Point(362, 334), 33, 30);
constexpr Common::Rect kMacPdaPartnerFootMapRect(Common::Point(11, 340), 80, 44);
constexpr Common::Rect kMacPdaSiteRect(Common::Point(56, 213), 34, 48);
constexpr Common::Rect kMacPdaHelp2Rect(Common::Point(427, 334), 33, 30);

constexpr uint16 kProfilePickerRevealPic = 0x105;
constexpr int kProfilePickerRevealX = 0x3e;
constexpr int kProfilePickerRevealY = 0xb3;
constexpr int kProfileListX = 61;
constexpr int kProfileListY = 35;
constexpr int kProfileListW = 220;
constexpr int kProfileLineH = 10;
constexpr int kProfileVisibleRows = 12;
constexpr Common::Rect kChooserOkRect(Common::Point(12, 63), 29, 24);
constexpr Common::Rect kChooserHelpRect(Common::Point(12, 100), 29, 24);
constexpr Common::Rect kChooserExitRect(Common::Point(12, 137), 29, 24);
constexpr Common::Rect kChooserUpArrowRect(Common::Point(240, 31), 10, 12);
constexpr Common::Rect kChooserDnArrowRect(Common::Point(240, 148), 10, 11);
constexpr Common::Rect kChooserListRect(Common::Point(58, 35), 180, 123);
constexpr Common::Rect kChooserNewPlayerRect(Common::Point(61, 176), 185, 15);

constexpr uint16 kNameEntryPeekPic = 0x107;
constexpr int kNameEntryPeekX = 0x3e;
constexpr int kNameEntryPeekY = 0xb3;

constexpr uint16 kCaseSelectionRevealPic = 0x53;
constexpr int kCaseSelectionRevealX = 0x3e;
constexpr int kCaseSelectionRevealY = 0xb2;
constexpr uint16 kActionScreenBackgroundPic = 0x104;
constexpr uint16 kActionScreenDecorPic = 0x0009;
constexpr int kActionScreenDecorX = 10;
constexpr int kActionScreenDecorY = 0x87;
constexpr byte kChooserCycleStart = 0x6f;
constexpr byte kChooserCycleEnd = 0x73;
constexpr uint32 kChooserCycleMillis = 100;
const char kScrapbookExtraFilename[] = "SCRAPBK_EXTRA.ANI";
const byte kScrapbookExtraMagic[] = {
	'E', 'E', 'M', 'S', 'B', 'X', '0', '2'
};
constexpr uint16 kScrapbookExtraVersion = 2;
constexpr uint16 kScrapbookExtraCaseCount = 55;
constexpr uint16 kScrapbookExtraNoVoice = 0xFFFF;
constexpr uint16 kScrapbookExtraMaxRecords = 16;
constexpr uint kRestoredContentFirstMystery = 1;
constexpr uint kRestoredContentLastMystery = 0x18;

Common::Rect pdaControlRect(const EEMEngine *vm, const Common::Rect &rect) {
	if (!vm || !vm->isMacintosh())
		return rect;
	if (rect == kPdaNotebookRect)
		return kMacPdaNotebookRect;
	if (rect == kPdaHelpRect)
		return kMacPdaHelpRect;
	if (rect == kPdaGalleryRect)
		return kMacPdaGalleryRect;
	if (rect == kPdaPartnerHeadHintRect)
		return kMacPdaPartnerHeadHintRect;
	if (rect == kPdaAccuseRect)
		return kMacPdaAccuseRect;
	if (rect == kPdaPageNextRect)
		return kMacPdaPageNextRect;
	if (rect == kPdaPagePrevRect)
		return kMacPdaPagePrevRect;
	if (rect == kPdaPartnerFootMapRect)
		return kMacPdaPartnerFootMapRect;
	if (rect == kPdaSiteRect)
		return kMacPdaSiteRect;
	if (rect == kPdaHelp2Rect)
		return kMacPdaHelp2Rect;
	return vm->scaleRect(rect);
}

bool notebookButtonAt(const EEMEngine *vm, int x, int y) {
	return pdaControlRect(vm, kPdaHelpRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaGalleryRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaPartnerHeadHintRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaAccuseRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaPageNextRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaPagePrevRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaHelp2Rect).contains(x, y) ||
		   pdaControlRect(vm, kPdaPartnerFootMapRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaSiteRect).contains(x, y);
}

bool notebookButtonAt(int x, int y) {
	return notebookButtonAt(nullptr, x, y);
}

bool galleryButtonAt(int x, int y) {
	return kPdaSiteRect.contains(x, y) ||
		   kPdaPartnerFootMapRect.contains(x, y) ||
		   kPdaAccuseRect.contains(x, y) ||
		   kPdaNotebookRect.contains(x, y) ||
		   kPdaHelpRect.contains(x, y) ||
		   kPdaPartnerHeadHintRect.contains(x, y);
}

bool galleryButtonAt(const EEMEngine *vm, int x, int y) {
	return pdaControlRect(vm, kPdaSiteRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaPartnerFootMapRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaAccuseRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaNotebookRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaHelpRect).contains(x, y) ||
		   pdaControlRect(vm, kPdaPartnerHeadHintRect).contains(x, y);
}

bool rectListContains(const Common::Array<Common::Rect> &rects, int x, int y) {
	for (uint i = 0; i < rects.size(); i++) {
		if (rects[i].contains(x, y))
			return true;
	}
	return false;
}

bool readScrapbookExtraText(Common::File &file, uint16 size,
							Common::String &out) {
	out.clear();
	if (size == 0)
		return true;

	Common::Array<char> buf;
	buf.resize(size);
	if (file.read(buf.data(), size) != size)
		return false;

	out = Common::String(buf.data(), size);
	return true;
}

bool restoredContentVoiceAppliesTo(uint mysteryNum) {
	return mysteryNum >= kRestoredContentFirstMystery &&
		   mysteryNum <= kRestoredContentLastMystery;
}

bool gallerySlotAt(const Common::Array<Common::Rect> &rects,
				   const Common::Array<int> &suspects, int x, int y) {
	for (uint i = 0; i < rects.size() && i < suspects.size(); i++) {
		if (suspects[i] >= 0 && rects[i].contains(x, y))
			return true;
	}
	return false;
}

const byte *advanceFloppyDialogRecords(const byte *rec, uint count,
									   const byte *end, bool mac = false) {
	if (!rec || (!end && count != 0))
		return nullptr;
	const uint headerLen = mac ? 14 : 11;
	const uint textCountOffset = mac ? 13 : 10;
	for (uint i = 0; i < count; i++) {
		if (rec + headerLen > end)
			return nullptr;
		const uint len = headerLen + rec[textCountOffset];
		if (rec + len > end)
			return nullptr;
		rec += len;
	}
	return rec;
}

// Floppy gallery slot positions @ 2608:0x16c.
const GallerySlot kFloppyGallerySlots[5] = {
	{ 0x53, 0x0e }, // 0
	{ 0x9b, 0x0e }, // 1
	{ 0xe3, 0x0e }, // 2
	{ 0x77, 0x5a }, // 3
	{ 0xbf, 0x5a }  // 4
};

// `_GetKDTextBalloon @ 1df2:0105` digit-balloon table @ 29be:1064
const uint16 kDigitBalloons[10] = {
	0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x20, 0x21, 0x22, 0x1e
};

// Next non-empty slot in `slotRects` from `from`, stepping by `dir` with
// wraparound. Mirrors `_PutMouseInRect` skipping eliminated suspects.
int nextLiveSlot(const Common::Array<Common::Rect> &slotRects,
				 int from, int dir) {
	const int n = (int)slotRects.size();
	if (n <= 0)
		return 0;
	for (int step = 1; step <= n; step++) {
		int idx = (from + dir * step) % n;
		if (idx < 0)
			idx += n;
		if (!slotRects[idx].isEmpty())
			return idx;
	}
	return from;
}

void copyToScreen(Graphics::ManagedSurface &scratch) {
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, scratch.w, scratch.h);
	g_system->updateScreen();
}

bool loadMacEndingBlob(uint num, Common::Array<byte> &out, bool &looseScript) {
	static const char *const kLoosePatterns[] = {
		"Endings/e%u.bin",
		"Endings/E%u.BIN",
		"e%u.bin",
		"E%u.BIN"
	};

	looseScript = false;
	Common::File loose;
	Common::String name;
	if (openNumberedScriptFile(loose, name, num, kLoosePatterns,
							   ARRAYSIZE(kLoosePatterns))) {
		const uint32 size = loose.size();
		if (size < 12) {
			warning("doShowEnding: %s too small (%u bytes)",
					name.c_str(), size);
			return false;
		}
		out.resize(size);
		if (loose.read(out.data(), size) != size) {
			warning("doShowEnding: %s short read", name.c_str());
			out.clear();
			return false;
		}
		looseScript = true;
		return true;
	}

	if (num >= kMacMysteryDataEndingCount)
		return false;

	Common::File f;
	if (!f.open(Common::Path("MysteryData"))) {
		warning("doShowEnding: cannot open MysteryData");
		return false;
	}

	const uint entry = kMacMysteryDataMysteryCount + num;
	const uint32 tableOff = kMacMysteryDataTableOffset + entry * 8;
	if (tableOff + 8 > (uint32)f.size()) {
		warning("doShowEnding: MysteryData ending index %u out of range",
				num);
		return false;
	}

	f.seek(tableOff);
	const uint32 offset = f.readUint32BE();
	const uint32 size = f.readUint32BE();
	if (size < 16 || offset > (uint32)f.size() ||
		size > (uint32)f.size() - offset) {
		warning("doShowEnding: MysteryData ending %u is invalid "
				"(off=0x%08x size=0x%08x)", num, offset, size);
		return false;
	}

	out.resize(size);
	f.seek(offset);
	if (f.read(out.data(), size) != size) {
		warning("doShowEnding: short read on MysteryData ending %u", num);
		out.clear();
		return false;
	}

	return true;
}

void cycleChooserPalette() {
	cyclePaletteRange(kChooserCycleStart, kChooserCycleEnd);
}

void blitMaskedPicSlice(Graphics::ManagedSurface &dst, const Picture &pic,
						int srcX, int srcY, int w, int h,
						int dstX, int dstY) {
	if (pic.surface.empty() || w <= 0 || h <= 0)
		return;
	const Common::Rect srcRect(srcX, srcY, srcX + w, srcY + h);
	dst.transBlitFrom(pic.surface, srcRect, Common::Point(dstX, dstY),
					  (uint32)(byte)(pic.flags >> 8));
}

void blitMaskedPic(Graphics::ManagedSurface &dst, const Picture &pic,
				   int x, int y) {
	blitMaskedPicSlice(dst, pic, 0, 0, pic.surface.w, pic.surface.h, x, y);
}

void blitMaskedPicRightReveal(Graphics::ManagedSurface &dst,
							  const Picture &pic, int x, int y,
							  int visibleW) {
	const int w = CLIP<int>(visibleW, 0, pic.surface.w);
	if (w == 0)
		return;
	blitMaskedPicSlice(dst, pic, 0, 0, w, pic.surface.h,
					   x + pic.surface.w - w, y);
}

void blitMaskedPicBottomReveal(Graphics::ManagedSurface &dst,
							   const Picture &pic, int x, int y,
							   int visibleH) {
	const int h = CLIP<int>(visibleH, 0, pic.surface.h);
	if (h == 0)
		return;
	blitMaskedPicSlice(dst, pic, 0, 0, pic.surface.w, h,
					   x, y + pic.surface.h - h);
}

bool pumpQuitEvents(EEMEngine *vm) {
	Common::Event ev;
	while (g_system->getEventManager()->pollEvent(ev)) {
		if (ev.type == Common::EVENT_QUIT ||
			ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
			return true;
	}
	return vm && vm->shouldQuit();
}

void drawCaseBookTitle(Graphics::ManagedSurface &scratch, const EEMEngine *vm,
					   uint book) {
	if (!vm || !vm->getFont().isLoaded())
		return;

	const bool spanish = vm->isSpanish();
	const Common::String title = (book == 3)
		? Common::String(spanish ? "Libro de Retos" : "Challenge Book")
		: Common::String::format(spanish ? "Lib. %u" : "Book %u", book);
	const int titleW = vm->getFont().getStringWidth(title);
	const int titleX = (0xba - titleW) / 2 + 0x3c;
	vm->getFont().drawString(&scratch, title, vm->scaleX(titleX),
							 vm->scaleY(12), vm->scaleX(kScreenWidth), 0xF);
}

void drawNameEntryFrame(EEMEngine *vm, const Picture *bg, bool haveBG,
						const Picture *peek, const Common::String &name,
						const char *prompt) {
	Graphics::ManagedSurface scratch(vm->screenWidth(), vm->screenHeight(),
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (haveBG)
		scratch.simpleBlitFrom(bg->surface);
	if (peek)
		blitMaskedPic(scratch, *peek, vm->scaleX(kNameEntryPeekX),
					   vm->scaleY(kNameEntryPeekY));
	vm->getFont().drawString(&scratch, prompt, vm->scaleX(80),
							 vm->scaleY(40), vm->scaleX(240), 0xF);
	vm->getFont().drawString(&scratch, name + "_", vm->scaleX(80),
							 vm->scaleY(80), vm->scaleX(240), 0xF);
	copyToScreen(scratch);
}

bool animateNameEntryPeek(EEMEngine *vm, const Picture *bg, bool haveBG,
						  const Picture *peek) {
	if (!peek || peek->surface.empty())
		return false;

	for (int w = 1; w <= peek->surface.w; w++) {
		if (pumpQuitEvents(vm))
			return true;
		Graphics::ManagedSurface scratch(vm->screenWidth(), vm->screenHeight(),
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveBG)
			scratch.simpleBlitFrom(bg->surface);
		blitMaskedPicRightReveal(scratch, *peek,
								  vm->scaleX(kNameEntryPeekX),
								  vm->scaleY(kNameEntryPeekY), w);
		copyToScreen(scratch);
		g_system->delayMillis(10);
	}
	return false;
}

bool animateProfilePickerReveal(EEMEngine *vm, const Picture *bg,
								bool haveBG, const Picture *reveal) {
	if (!reveal || reveal->surface.empty())
		return false;

	for (int h = 1; h <= reveal->surface.h; h++) {
		if (pumpQuitEvents(vm))
			return true;
		Graphics::ManagedSurface scratch(vm->screenWidth(), vm->screenHeight(),
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveBG)
			scratch.simpleBlitFrom(bg->surface);
		blitMaskedPicBottomReveal(scratch, *reveal,
								   vm->scaleX(kProfilePickerRevealX),
								   vm->scaleY(kProfilePickerRevealY), h);
		copyToScreen(scratch);
		g_system->delayMillis(10);
	}
	return false;
}

struct ProfilePickerEntry {
	Common::String label;
	int slot;
};

struct ProfilePickerView {
	EEMEngine *vm;
	const Picture *bg;
	bool haveBG;
	const Picture *reveal;
	bool haveReveal;
	const Common::Array<ProfilePickerEntry> *entries;
	int selected;
	int start;
};

void clampProfileScroll(int &selected, int &start, int count) {
	if (count <= 0) {
		selected = 0;
		start = 0;
		return;
	}
	selected = CLIP<int>(selected, 0, count - 1);
	const int maxStart = MAX<int>(0, count - kProfileVisibleRows);
	start = CLIP<int>(start, 0, maxStart);
	if (selected < start)
		start = selected;
	if (selected >= start + kProfileVisibleRows)
		start = selected - kProfileVisibleRows + 1;
	start = CLIP<int>(start, 0, maxStart);
}

void drawProfilePickerFrame(const ProfilePickerView &v) {
	Graphics::ManagedSurface scratch(v.vm->screenWidth(), v.vm->screenHeight(),
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (v.haveBG)
		scratch.simpleBlitFrom(v.bg->surface);
	if (v.haveReveal)
		blitMaskedPic(scratch, *v.reveal,
					   v.vm->scaleX(kProfilePickerRevealX),
					   v.vm->scaleY(kProfilePickerRevealY));

	const int count = v.entries ? (int)v.entries->size() : 0;
	for (int row = 0; row < kProfileVisibleRows; row++) {
		const int idx = v.start + row;
		if (idx >= count)
			break;
		const byte color = idx == v.selected ? 0xF : 0x8;
		v.vm->getFont().drawString(&scratch, (*v.entries)[idx].label,
									v.vm->scaleX(kProfileListX),
									v.vm->scaleY(kProfileListY + row * kProfileLineH),
									v.vm->scaleX(kProfileListW), color);
	}
	copyToScreen(scratch);
}

struct ActionMenuView {
	EEMEngine *vm;
	const Picture *bg;
	bool haveBg;
	const Picture *decor;
	bool haveDecor;
	const char *separator;
	const char *const *pickLabel;
	const bool *pickEnabled;
	uint pick;
	uint numPicks;
};

Common::StringArray loadMacBookNames(uint book) {
	// The Mac release embeds the chooser strings in the application resource
	// data instead of shipping BOOK*.NME files.
	static const char *const kMacBook1Names[] = {
		"Case of the Stolen Skateboard",
		"Case of the Roundabout Robber",
		"Case of the Runaway Reptile",
		"Case of the Mysterious Monster",
		"Case of the Rock Ripoff",
		"Case of the Pilfered Pop",
		"Case of the Creepy Cinema",
		"Case of the Angry Arsonist",
		"Case of the Crazy Compass",
		"Case of the Midnight Masquerade",
		"Case of the Missing Mink",
		"Case of the Basketball Blooper",
		"Case of the Reappearing Recipe",
		"Case of the Attacking Aliens",
		"Case of the Dangling Diamond",
		"Case of the Buried Booty",
		"Case of the Questionable Quiz",
		"Case of the Cryptic Cavern",
		"Case of the International Idol",
		"Case of the Counterfeit Card",
		"Case of the Puzzling Pooches",
		"Case of the Baffling Bones",
		"Case of the Vanishing Violin",
		"Case of the Antique Autograph",
	};
	static const char *const kMacBook2Names[] = {
		"Case of the Stolen Skateboard",
		"Case of the Angry Arsonist",
		"Case of the Roundabout Robber",
		"Case of the Cryptic Cavern",
		"Case of the Missing Mink",
		"Case of the International Idol",
		"Case of the Runaway Reptile",
		"Case of the Buried Booty",
		"Case of the Midnight Masquerade",
		"Case of the Pilfered Pop",
		"Case of the Rock Ripoff",
		"Case of the Basketball Blooper",
		"Case of the Questionable Quiz",
		"Case of the Baffling Bones",
		"Case of the Vanishing Violin",
		"Case of the Dangling Diamond",
		"Case of the Puzzling Pooches",
		"Case of the Mysterious Monster",
		"Case of the Attacking Aliens",
		"Case of the Antique Autograph",
		"Case of the Reappearing Recipe",
		"Case of the Creepy Cinema",
		"Case of the Crazy Compass",
		"Case of the Counterfeit Card",
	};
	static const char *const kMacBook3Names[] = {
		"Case of the Midnight Masquerade",
		"Case of the Puzzled Pooches",
		"Case of the Buried Booty",
		"Case of the International Idol",
		"Case of the Antique Autograph",
		"Case of the Attacking Aliens",
	};

	const char *const *src = nullptr;
	uint count = 0;
	switch (book) {
	case 1:
		src = kMacBook1Names;
		count = ARRAYSIZE(kMacBook1Names);
		break;
	case 2:
		src = kMacBook2Names;
		count = ARRAYSIZE(kMacBook2Names);
		break;
	case 3:
		src = kMacBook3Names;
		count = ARRAYSIZE(kMacBook3Names);
		break;
	default:
		break;
	}

	Common::StringArray names;
	for (uint i = 0; i < count; i++)
		names.push_back(src[i]);
	return names;
}

// `_DoChooseMystery @ 1a35:02b7`
Common::StringArray loadBookNames(uint book, bool macintosh) {
	if (macintosh)
		return loadMacBookNames(book);

	Common::StringArray names;
	const Common::String fname = Common::String::format("BOOK%u.NME", book);
	Common::File f;
	if (!f.open(Common::Path(fname))) {
		warning("loadBookNames: %s missing", fname.c_str());
		return names;
	}
	while (!f.eos()) {
		Common::String line = f.readLine();
		if (f.eos() && line.empty())
			break;
		// Trim trailing whitespace so the sentinel line doesn't render.
		while (!line.empty() &&
			   (line.lastChar() == ' ' || line.lastChar() == '\t' ||
				line.lastChar() == '\r'))
			line.deleteLastChar();
		if (line.empty())
			continue;
		names.push_back(line);
	}
	return names;
}

void clampCaseTopRow(uint &topRow, uint listLen, uint visibleRows) {
	if (listLen <= visibleRows) {
		topRow = 0;
		return;
	}
	const uint maxTop = listLen - visibleRows;
	if (topRow > maxTop)
		topRow = maxTop;
}

struct CaseSubmenuView {
	EEMEngine *vm;
	const Picture *caseBg;
	bool haveCaseBg;
	const Picture *revealPic;
	bool haveRevealPic;
	const Animation *kdAnim;
	bool haveKdAnim;
	int kdAnimX;
	int kdAnimY;
	const Common::StringArray *names;
	const Common::Array<bool> *solvedFlags;
	uint topRow;
	uint selRow;
	uint book;
};

void drawCaseGreeter(Graphics::ManagedSurface &scratch,
					 const Animation *kdAnim, bool haveKdAnim,
					 int kdAnimX, int kdAnimY) {
	if (!haveKdAnim || !kdAnim || kdAnim->empty())
		return;

	const uint32 now = g_system->getMillis();
	const uint frameIdx = partnerFrameAtTick(0x15, (uint)kdAnim->size(), now);
	blitAnimFrameAnchored(scratch.surfacePtr(), (*kdAnim)[frameIdx],
						  kdAnimX, kdAnimY);
}

void drawCaseBase(Graphics::ManagedSurface &scratch, EEMEngine *vm,
				  const Picture *caseBg, bool haveCaseBg,
				  const Picture *revealPic, bool haveRevealPic,
				  const Animation *kdAnim, bool haveKdAnim,
				  int kdAnimX, int kdAnimY, uint book) {
	scratch.clear();
	if (haveCaseBg && caseBg)
		scratch.simpleBlitFrom(caseBg->surface);
	if (haveRevealPic && revealPic)
		blitMaskedPic(scratch, *revealPic,
					   vm->scaleX(kCaseSelectionRevealX),
					   vm->scaleY(kCaseSelectionRevealY));
	drawCaseBookTitle(scratch, vm, book);
	drawCaseGreeter(scratch, kdAnim, haveKdAnim,
					vm->scaleX(kdAnimX), vm->scaleY(kdAnimY));
}

bool animateCaseSelectionReveal(EEMEngine *vm, const Picture *caseBg,
								bool haveCaseBg, const Picture *revealPic,
								bool haveRevealPic, const Animation *kdAnim,
								bool haveKdAnim, int kdAnimX, int kdAnimY,
								uint book) {
	if (!haveRevealPic || !revealPic || revealPic->surface.empty())
		return false;

	const int steps = vm && vm->isFloppy()
		? revealPic->surface.w : revealPic->surface.h;
	for (int i = 1; i <= steps; i++) {
		if (pumpQuitEvents(vm))
			return true;
		Graphics::ManagedSurface scratch(vm->screenWidth(), vm->screenHeight(),
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveCaseBg && caseBg)
			scratch.simpleBlitFrom(caseBg->surface);
		if (vm && vm->isFloppy()) {
			blitMaskedPicRightReveal(scratch, *revealPic,
									  vm->scaleX(kCaseSelectionRevealX),
									  vm->scaleY(kCaseSelectionRevealY), i);
		} else {
			blitMaskedPicBottomReveal(scratch, *revealPic,
									   vm->scaleX(kCaseSelectionRevealX),
									   vm->scaleY(kCaseSelectionRevealY), i);
		}
		drawCaseBookTitle(scratch, vm, book);
		drawCaseGreeter(scratch, kdAnim, haveKdAnim,
						vm->scaleX(kdAnimX), vm->scaleY(kdAnimY));
		copyToScreen(scratch);
		g_system->delayMillis(8);
	}
	return false;
}

// `DrawList @ 1c33:040d`
void drawCaseSubmenu(const CaseSubmenuView &v) {
	Graphics::ManagedSurface scratch(v.vm->screenWidth(), v.vm->screenHeight(),
		Graphics::PixelFormat::createFormatCLUT8());
	drawCaseBase(scratch, v.vm, v.caseBg, v.haveCaseBg,
				 v.revealPic, v.haveRevealPic,
				 v.kdAnim, v.haveKdAnim, v.kdAnimX, v.kdAnimY, v.book);
	if (!v.vm->getFont().isLoaded() || !v.names)
		return;

	const int kListX  = 61;
	const int kListW  = 238 - kListX;
	const int kListY0 = 35;
	const int kLineH  = 10;
	const int kVisible = 12;
	const uint count = (uint)v.names->size();

	for (int r = 0; r < kVisible; r++) {
		const uint idx = v.topRow + (uint)r;
		if (idx >= count)
			break;
		const Common::String &name = (*v.names)[idx];
		byte color = 0xF;  // default
		if (idx == v.selRow) {
			color = 0xF;   // highlighted
		} else if (v.solvedFlags && idx < v.solvedFlags->size() &&
				   (*v.solvedFlags)[idx]) {
			color = 0x8;   // greyed (already solved)
		} else {
			color = 0x7;   // normal
		}
		v.vm->getFont().drawString(&scratch, name,
			v.vm->scaleX(kListX), v.vm->scaleY(kListY0 + r * kLineH),
			v.vm->scaleX(kListW), color);
	}

	// Selection arrow.
	if (v.selRow >= v.topRow && v.selRow < v.topRow + (uint)kVisible) {
		const int r = (int)(v.selRow - v.topRow);
		v.vm->getFont().drawString(&scratch, ">",
			v.vm->scaleX(kListX - 6), v.vm->scaleY(kListY0 + r * kLineH),
			v.vm->scaleX(6), 0xF);
	}

	// Scrollbar thumb at (240, 45..146), proportional to scroll position.
	if (count > (uint)kVisible) {
		const int trackY0 = 45;
		const int trackH  = 146 - 45;
		const int thumbH  = MAX<int>(8, (trackH * kVisible) / (int)count);
		const int travel  = trackH - thumbH;
		const int pos = (int)v.topRow * travel /
						MAX<int>(1, (int)count - kVisible);
		const Common::Rect thumb(240, trackY0 + pos,
								  250, trackY0 + pos + thumbH);
		scratch.fillRect(v.vm->scaleRect(thumb), 0x8);
		scratch.frameRect(v.vm->scaleRect(thumb), 0xF);
	}

	copyToScreen(scratch);
}

void drawActionMenuFrame(const ActionMenuView &v) {
	Graphics::ManagedSurface scratch(v.vm->screenWidth(), v.vm->screenHeight(),
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (v.haveBg && v.bg)
		scratch.simpleBlitFrom(v.bg->surface);
	if (v.haveDecor && v.decor)
		blitMaskedPic(scratch, *v.decor,
					   v.vm->scaleX(kActionScreenDecorX),
					   v.vm->scaleY(kActionScreenDecorY));

	if (v.vm->getFont().isLoaded()) {
		// `DrawList @ 1c33:040d`. _TextBox @ 29be:0d00 = {58, 35, 238, 158}.
		const int kListX  = 58 + 3;
		const int kListW  = 238 - kListX;
		const int kListY0 = 35;
		const int kLineH  = 10;

		// Separator/item pairs (0=sep, 1=Choose A Mystery, ..., trailing sep):
		// 11 rows for EEM1's five picks, 9 for London's four (no ScrapBook 3).
		const int kRows = (int)(2 * v.numPicks + 1);
		for (int r = 0; r < kRows; r++) {
			const int y = kListY0 + r * kLineH;
			if ((r & 1) == 0) {
				v.vm->getFont().drawString(&scratch, v.separator,
										   v.vm->scaleX(kListX), v.vm->scaleY(y),
										   v.vm->scaleX(kListW), 0x7);
				continue;
			}
			const uint mp = (uint)(r >> 1);
			const bool isSel  = (mp == v.pick);
			const byte color  = isSel             ? 0xF :
								v.pickEnabled[mp] ? 0x7 : 0x8;
			v.vm->getFont().drawString(&scratch, v.pickLabel[mp],
									   v.vm->scaleX(kListX), v.vm->scaleY(y),
									   v.vm->scaleX(kListW), color);
		}
	}
	copyToScreen(scratch);
}

void EEMEngine::doProfilePicker() {
	_profileCreatedThisSession = false;

	setSitePalette(0);

	const SaveStateList saves = listProfiles();
	if (saves.empty()) {
		if (isLondon())
			showLondonCharSelect();
		else
			doNewPlayer();
		return;
	}

	if (!_font.isLoaded()) {
		if (isLondon())
			showLondonCharSelect();
		else
			doNewPlayer();
		return;
	}

	Common::Array<ProfilePickerEntry> entries;
	for (const SaveStateDescriptor &s : saves) {
		ProfilePickerEntry e;
		e.label = s.getDescription();
		e.slot  = s.getSaveSlot();
		entries.push_back(e);
	}

	int sel = 0;
	int start = 0;
	bool done = false;
	bool createNew = false;
	Picture bg;
	const bool haveBG = _picsArchive.getPicture(0x104, bg);
	Picture reveal;
	const bool haveReveal =
		_picsArchive.getPicture(kProfilePickerRevealPic, reveal);

	ProfilePickerView view;
	view.vm = this;
	view.bg = &bg;
	view.haveBG = haveBG;
	view.reveal = &reveal;
	view.haveReveal = haveReveal;
	view.entries = &entries;
	view.selected = sel;
	view.start = start;
	if (animateProfilePickerReveal(this, &bg, haveBG,
								   haveReveal ? &reveal : nullptr))
		return;
	drawProfilePickerFrame(view);
	uint32 chooserLastTick = g_system->getMillis();

	while (!done && !shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		bool committed = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_playerName = "Detective";
				return;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_UP:
					sel = (sel + (int)entries.size() - 1) % (int)entries.size();
					clampProfileScroll(sel, start, (int)entries.size());
					dirty = true;
					break;
				case Common::KEYCODE_DOWN:
					sel = (sel + 1) % (int)entries.size();
					clampProfileScroll(sel, start, (int)entries.size());
					dirty = true;
					break;
				case Common::KEYCODE_PAGEUP:
					start -= kProfileVisibleRows;
					sel = start;
					clampProfileScroll(sel, start, (int)entries.size());
					dirty = true;
					break;
				case Common::KEYCODE_PAGEDOWN:
					start += kProfileVisibleRows;
					sel = start;
					clampProfileScroll(sel, start, (int)entries.size());
					dirty = true;
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					committed = true;
					break;
				case Common::KEYCODE_ESCAPE:
					createNew = true;
					committed = true;
					break;
				default:
					break;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				const Common::Point mouse(unscaleX(ev.mouse.x),
										  unscaleY(ev.mouse.y));
				if (kChooserOkRect.contains(mouse.x, mouse.y)) {
					committed = true;
					break;
				}
				if (kChooserExitRect.contains(mouse.x, mouse.y) ||
					kChooserNewPlayerRect.contains(mouse.x, mouse.y)) {
					createNew = true;
					committed = true;
					break;
				}
				if (kChooserUpArrowRect.contains(mouse.x, mouse.y)) {
					if (start > 0) {
						start--;
						if (sel >= start + kProfileVisibleRows)
							sel = start + kProfileVisibleRows - 1;
						clampProfileScroll(sel, start, (int)entries.size());
						dirty = true;
					}
					break;
				}
				if (kChooserDnArrowRect.contains(mouse.x, mouse.y)) {
					const int maxStart = MAX<int>(0,
						(int)entries.size() - kProfileVisibleRows);
					if (start < maxStart) {
						start++;
						if (sel < start)
							sel = start;
						clampProfileScroll(sel, start, (int)entries.size());
						dirty = true;
					}
					break;
				}
				if (kChooserHelpRect.contains(mouse.x, mouse.y)) {
					break;
				}
				if (kChooserListRect.contains(mouse.x, mouse.y)) {
					const int hit = (mouse.y - kProfileListY) /
									kProfileLineH;
					const int idx = start + hit;
					if (hit >= 0 && hit < kProfileVisibleRows &&
						idx >= 0 && idx < (int)entries.size()) {
						if (idx == sel) {
							committed = true;
							break;
						}
						sel = idx;
						dirty = true;
					}
				}
			}
			if (committed)
				break;
		}
		if (committed) {
			done = true;
			break;
		}
		if (dirty) {
			view.selected = sel;
			view.start = start;
			drawProfilePickerFrame(view);
		}
		const uint32 now = g_system->getMillis();
		if (now - chooserLastTick >= kChooserCycleMillis) {
			chooserLastTick = now;
			cycleChooserPalette();
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	if (createNew) {
		if (isLondon())
			showLondonCharSelect();
		else
			doNewPlayer();
		return;
	}

	const ProfilePickerEntry &e = entries[sel];
	if (loadProfile(e.label)) {
		_profileCreatedThisSession = false;
	} else {
		warning("doProfilePicker: failed to load profile '%s' at slot %d",
				e.label.c_str(), e.slot);
		if (isLondon())
			showLondonCharSelect();
		else
			doNewPlayer();
	}
}
// `_NewPlayer @ 1c33:0dda`
void EEMEngine::doNewPlayer() {
	_profileCreatedThisSession = false;
	if (!_font.isLoaded()) {
		_playerName = "Detective";
		return;
	}

	Common::String name;
	const int maxChars = 12;

	Picture bg;
	const bool haveBG = _picsArchive.getPicture(0x104, bg);
	Picture peek;
	const bool havePeek = _picsArchive.getPicture(kNameEntryPeekPic, peek);

	// Spanish from EEM.EXE ("Teclea tu nombre"); colon added.
	const char *prompt = isSpanish()
		? "Teclea tu nombre:" : "Please type your name:";

	if (animateNameEntryPeek(this, &bg, haveBG, havePeek ? &peek : nullptr))
		return;
	// Prompt (y=40, x=80), input (y=80, x=80).
	drawNameEntryFrame(this, &bg, haveBG, havePeek ? &peek : nullptr,
					   name, prompt);

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	bool done = false;
	while (!done && !shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				done = true;
				break;
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_RETURN) {
				if (name.empty())
					name = "Detective";
				if (loadProfile(name)) {
					_profileCreatedThisSession = false;
				} else {
					_playerName = name;
					memset(_mysteriesSolved, 0, sizeof(_mysteriesSolved));
					_mystery.clear();
					_partner = kPartnerJake;
					_chainStage = 1;
					saveProfile(name);
					_profileCreatedThisSession = true;
				}
				done = true;
				break;
			}
			if (k == Common::KEYCODE_ESCAPE) {
				_playerName = "Detective";
				done = true;
				break;
			}
			if (k == Common::KEYCODE_BACKSPACE) {
				if (!name.empty()) {
					name.deleteLastChar();
					dirty = true;
				}
				continue;
			}
			if (ev.kbd.ascii >= ' ' && ev.kbd.ascii < 127 &&
				(int)name.size() < maxChars) {
				name += (char)ev.kbd.ascii;
				dirty = true;
			}
		}
		if (dirty) {
			drawNameEntryFrame(this, &bg, haveBG, havePeek ? &peek : nullptr,
							   name, prompt);
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}
// `_DisplayEnding @ 1df2:0548` + `_DisplayEndingPage @ 1df2:044c`
int EEMEngine::doShowEnding(uint num, bool firstPage) {
	Common::Array<byte> buf;
	uint32 size = 0;
	const bool macEnding = isMacintosh();
	bool macLooseEnding = false;

	if (macEnding) {
		if (!loadMacEndingBlob(num, buf, macLooseEnding))
			return 0;
		size = (uint32)buf.size();
	} else {
		const Common::String fname = Common::String::format("E%u.BIN", num);
		Common::File f;
		if (!f.open(Common::Path(fname))) {
			warning("doShowEnding: %s missing", fname.c_str());
			return 0;
		}
		size = f.size();
		if (size < 2) {
			warning("doShowEnding: %s too small (%u bytes)",
					fname.c_str(), size);
			return 0;
		}
		buf.resize(size);
		if (f.read(buf.data(), size) != size) {
			warning("doShowEnding: %s short read", fname.c_str());
			return 0;
		}
	}

	EEMFont tinyFont;
	const bool haveTinyFont =
		!macEnding && tinyFont.load(Common::Path("TINY.FNT"));
	if (!macEnding && !haveTinyFont)
		warning("doShowEnding: TINY.FNT failed to load — falling back");

	setSitePalette(0);
	CursorMan.showMouse(true);

	const bool floppyEnding = isFloppy();
	const int sw = screenWidth();
	const int sh = screenHeight();
	const Common::Rect endingPrevRect =
		macEnding ? scaleRect(kEndingPrevPageRect) : kEndingPrevPageRect;
	const Common::Rect endingNextRect =
		macEnding ? scaleRect(kEndingNextPageRect) : kEndingNextPageRect;
	uint pageOffsets[8];
	const uint pageOffsetCap =
		(uint)(sizeof(pageOffsets) / sizeof(pageOffsets[0]));
	uint validPages = 0;
	const bool compactEnding = floppyEnding || (macEnding && !macLooseEnding);
	const bool cdEnding = !compactEnding;

	if (compactEnding) {
		// Floppy `E<num>.BIN` and Mac EEM1 `MysteryData[55 + num]` start with:
		//   u8 type, 3 bytes of title metadata, char title[], u8 pageCount
		// followed by pages. Mac stores overlay pic/x words big-endian and
		// native-coordinate text rects in little-endian order.
		uint titleEnd = 4;
		while (titleEnd < size && buf[titleEnd] != 0)
			titleEnd++;
		if (titleEnd + 2 >= size)
			return 0;
		const uint pageCount = buf[titleEnd + 1];
		uint cursor = titleEnd + 2;
		const uint maxPages = MIN<uint>(pageCount, pageOffsetCap);
		for (uint p = 0; p < maxPages; p++) {
			if (cursor >= size)
				break;
			const uint pageStart = cursor;
			const uint overlayCount = buf[cursor++];
			const uint overlaysSize = overlayCount * 5;
			if (cursor + overlaysSize + 8 >= size)
				break;
			cursor += overlaysSize + 8;
			while (cursor < size && buf[cursor] != 0)
				cursor++;
			if (cursor >= size)
				break;
			pageOffsets[validPages++] = pageStart;
			cursor++;  // past the null
		}
	} else {
		const uint16 pageCount = readScriptU16(buf.data(), macLooseEnding);
		if (pageCount == 0)
			return 0;
		const uint maxPages = MIN<uint>(pageCount, pageOffsetCap);
		uint cursor = 2;
		for (uint p = 0; p < maxPages; p++) {
			if (cursor + 10 >= size)
				break;
			pageOffsets[validPages++] = cursor;
			// Skip the 10-byte header and find the null terminator.
			cursor += 10;
			while (cursor < size && buf[cursor] != 0)
				cursor++;
			cursor++;  // past the null
		}
	}
	if (validPages == 0)
		return 0;

	const bool showFirstTryBadge =
		num < sizeof(_mysteriesSolved) && _mysteriesSolved[num] == 2 &&
		(floppyEnding || ConfMan.getBool("restored_content"));
	Picture firstTryBadge;
	const bool haveFirstTryBadge =
		showFirstTryBadge &&
		_picsArchive.getPicture(kFirstTryBadgePic, firstTryBadge);

	uint pageIdx = firstPage ? 0 : (validPages - 1);
	int direction = 0;
	bool exitLoop = false;
	bool dirty = true;
	const Common::Point mousePos = g_system->getEventManager()->getMousePos();
	setInteractiveMouseCursor(endingPrevRect.contains(mousePos.x,
													  mousePos.y) ||
							  endingNextRect.contains(mousePos.x,
													  mousePos.y));
	while (!shouldQuit() && !exitLoop) {
		if (dirty) {
			const uint off = pageOffsets[pageIdx];
			uint16 x1 = 0;
			uint16 y1 = 0;
			uint16 x2 = 0;
			const char *raw = nullptr;
			Graphics::ManagedSurface scratch(sw, sh,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.clear();

			if (compactEnding) {
				Picture bg;
				if (floppyEnding &&
					_picsArchive.getPicture(kFloppyEndingBackgroundPic, bg))
					scratch.simpleBlitFrom(bg.surface);

				uint cursor = off;
				if (cursor >= size)
					break;
				const uint overlayCount = buf[cursor++];
				for (uint i = 0; i < overlayCount; i++) {
					if (cursor + 5 > size)
						break;
					const uint16 picNum = macEnding
						? READ_BE_UINT16(buf.data() + cursor)
						: READ_LE_UINT16(buf.data() + cursor);
					const uint16 px = macEnding
						? READ_BE_UINT16(buf.data() + cursor + 2)
						: READ_LE_UINT16(buf.data() + cursor + 2);
					const byte py = buf[cursor + 4];
					Picture overlay;
					if (_picsArchive.getPicture(picNum, overlay)) {
						if (macEnding)
							blitMacMaskedSurface(scratch.surfacePtr(),
												 overlay, px, py);
						else
							scratch.transBlitFrom(overlay.surface,
								Common::Point(px, py),
								(uint32)(byte)(overlay.flags >> 8));
					}
					cursor += 5;
				}
				if (cursor + 8 >= size)
					break;
				x1 = READ_LE_UINT16(buf.data() + cursor);
				y1 = READ_LE_UINT16(buf.data() + cursor + 2);
				x2 = READ_LE_UINT16(buf.data() + cursor + 4);
				(void)READ_LE_UINT16(buf.data() + cursor + 6);
				raw = (const char *)buf.data() + cursor + 8;
				if (macEnding && (byte)*raw == 0xd9)
					raw++;
			} else if (cdEnding) {
				if (off + 10 >= size)
					break;
				const uint16 picNum =
					readScriptU16(buf.data() + off, macLooseEnding);
				x1 = readScriptU16(buf.data() + off + 2, macLooseEnding);
				y1 = readScriptU16(buf.data() + off + 4, macLooseEnding);
				x2 = readScriptU16(buf.data() + off + 6, macLooseEnding);
				(void)readScriptU16(buf.data() + off + 8, macLooseEnding);  // y2 (unused — WordWrap2 takes width only)

				Picture bg;
				if (_picsArchive.getPicture(picNum, bg))
					scratch.simpleBlitFrom(bg.surface);
				raw = (const char *)buf.data() + off + 10;
			}

			if (pageIdx == 0 && haveFirstTryBadge) {
				const byte transp = (byte)(firstTryBadge.flags >> 8);
				scratch.transBlitFrom(firstTryBadge.surface,
									  kFirstTryBadgePos, transp);
			}

			const Common::String text = parseString(raw, _playerName, _partner);

			const EEMFont &renderFont = haveTinyFont ? tinyFont : _font;
			if (renderFont.isLoaded() && x2 > x1) {
				const int textW = MIN<int>((int)x2 - (int)x1,
										   sw - (int)x1);
				MacSpritePaletteMap macPaletteMap = {0x00, 0xFF};
				if (macEnding)
					macPaletteMap = getMacSpritePaletteMap();
				renderFont.drawWordWrapped(&scratch, (int)x1, (int)y1,
										   textW, text,
										   macEnding ? macPaletteMap.black : 0);
			}

			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, sw, sh);
			g_system->updateScreen();
			dirty = false;
		}

		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				direction = 0;
				exitLoop = true;
				break;
			}
			if (ev.type == Common::EVENT_MOUSEMOVE)
				setInteractiveMouseCursor(
					endingPrevRect.contains(ev.mouse.x, ev.mouse.y) ||
					endingNextRect.contains(ev.mouse.x, ev.mouse.y));
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					direction = 0;
					exitLoop = true;
					break;
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_PAGEUP:
					if (pageIdx > 0) {
						pageIdx--;
						dirty = true;
					} else {
						direction = -1;
						exitLoop = true;
					}
					break;
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_PAGEDOWN:
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
				case Common::KEYCODE_SPACE:
				case Common::KEYCODE_TAB:
					if (pageIdx + 1 < validPages) {
						pageIdx++;
						dirty = true;
					} else {
						direction = 1;
						exitLoop = true;
					}
					break;
				default:
					break;
				}
				if (exitLoop)
					break;
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				setInteractiveMouseCursor(
					endingPrevRect.contains(ev.mouse.x, ev.mouse.y) ||
					endingNextRect.contains(ev.mouse.x, ev.mouse.y));
				if (endingPrevRect.contains(ev.mouse.x, ev.mouse.y)) {
					if (pageIdx > 0) {
						pageIdx--;
						dirty = true;
					} else {
						direction = -1;
						exitLoop = true;
					}
				} else if (endingNextRect.contains(ev.mouse.x, ev.mouse.y)) {
					if (pageIdx + 1 < validPages) {
						pageIdx++;
						dirty = true;
					} else {
						direction = 1;
						exitLoop = true;
					}
				}
				if (exitLoop)
					break;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
	setInteractiveMouseCursor(false);
	return direction;
}
// EEM1 `_ShowScrapbook(stage, 0) @ 1f78:0642`; EEM2/London scrapbook `2046::09dd`.
void EEMEngine::doShowScrapbook(uint stage) {

	uint tierLo = 0, tierHi = 0;
	if (stage < 1 || !mysteryTierRange(stage, tierLo, tierHi))
		return;
	const int solvedCount =
		(int)(sizeof(_mysteriesSolved) / sizeof(_mysteriesSolved[0]));
	const int lo = (int)tierLo;
	const int hi = MIN<int>((int)tierHi + 1, solvedCount);
	if (lo >= hi)
		return;
	const bool currentTier = (stage == _chainStage);

	if (isLondon() && _music && _voiceOn)
		_music->playMus(0x5d, /* loop= */ true);

	int mystery = lo;
	if (currentTier) {
		while (mystery < hi && _mysteriesSolved[mystery] == 0)
			mystery++;
	}

	bool firstPage = true;
	while (!shouldQuit() && mystery >= lo && mystery < hi) {
		const int direction = doShowEnding((uint)mystery, firstPage);
		if (direction < 0) {
			int prevMystery = mystery - 1;
			if (currentTier) {
				while (prevMystery >= lo && _mysteriesSolved[prevMystery] == 0)
					prevMystery--;
			}
			if (prevMystery < lo) {
				firstPage = true;
				continue;
			}
			mystery = prevMystery;
			firstPage = false;
		} else if (direction > 0) {
			int nextMystery = mystery + 1;
			if (currentTier) {
				while (nextMystery < hi && _mysteriesSolved[nextMystery] == 0)
					nextMystery++;
			}
			if (nextMystery >= hi) {
				firstPage = false;
				continue;
			}
			mystery = nextMystery;
			firstPage = true;
		} else {
			break;
		}
	}
	if (isLondon() && _music)
		stopMusic();
}
// `_DoSetup @ 1f78:044e` (CD) / `_DoSetup_Floppy @ 1ee2:0387`.
void EEMEngine::doSetup() {
	if (!_font.isLoaded()) {
		_nextScreen = (ScreenId)_lastScreen;
		return;
	}

	const Common::Rect kPartnerBtn   ( 20,  44,  39,  61); // [0]
	const Common::Rect kVoiceBtn     ( 20,  87,  39, 104); // [1]
	const Common::Rect kProfileBtn   ( 20, 127,  39, 144); // [2]
	const Common::Rect kScrap1Btn    (281,  43, 299,  60); // [3]
	const Common::Rect kScrap2Btn    (281,  62, 299,  79); // [4]
	const Common::Rect kScrap3Btn    (281,  81, 299,  98); // [5]
	const Common::Rect kSaveBtn      (281, 108, 299, 125); // [6]
	const Common::Rect kNewCaseBtn   (281, 127, 299, 144); // [7]
	const Common::Rect kDoneBtn      ( 53, 153, 108, 183); // [8]
	const Common::Rect kHelpBtn      (145, 163, 174, 187); // [9]
	const Common::Rect kQuitBtn      (212, 153, 266, 184); // [10]
	const Common::Rect kCreditsBtn   ( 81,  25, 238,  37); // [11]

	const Common::Rect &kKid1Rect     = kSetupKid1Rect;
	const Common::Rect &kKid2Rect     = kSetupKid2Rect;
	const Common::Rect &kSoundOnRect  = kSetupSoundOnRect;
	const Common::Rect &kSoundOffRect = kSetupSoundOffRect;

	setupDrawScreen();

	_nextScreen = kScreenSetup;
	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_nextScreen = kScreenInvalid;
				return;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE ||
					ev.kbd.keycode == Common::KEYCODE_RETURN) {
					setupLeave();
					return;
				}
			}
			if (ev.type != Common::EVENT_LBUTTONDOWN)
				continue;
			const Common::Point mouse(unscaleX(ev.mouse.x),
									  unscaleY(ev.mouse.y));
			const int mx = mouse.x;
			const int my = mouse.y;

			if (kPartnerBtn.contains(mx, my)) {
				_partner = _partner == kPartnerJake ? kPartnerJenny : kPartnerJake;
				dirty = true;
				continue;
			}
			if (kKid1Rect.contains(mx, my)) {
				if (_partner != kPartnerJake) {
					_partner = kPartnerJake;
					dirty = true;
				}
				continue;
			}
			if (kKid2Rect.contains(mx, my)) {
				if (_partner != kPartnerJenny) {
					_partner = kPartnerJenny;
					dirty = true;
				}
				continue;
			}

			if (kVoiceBtn.contains(mx, my)) {
				_voiceOn = !_voiceOn;
				if (_audio)
					_audio->setVoiceEnabled(_voiceOn);
				dirty = true;
				continue;
			}
			if (kSoundOnRect.contains(mx, my)) {
				if (!_voiceOn) {
					_voiceOn = true;
					if (_audio)
						_audio->setVoiceEnabled(_voiceOn);
					dirty = true;
				}
				continue;
			}
			if (kSoundOffRect.contains(mx, my)) {
				if (_voiceOn) {
					_voiceOn = false;
					if (_audio)
						_audio->setVoiceEnabled(_voiceOn);
					dirty = true;
				}
				continue;
			}

			if (kNewCaseBtn.contains(mx, my)) {
				saveProfile(_playerName);
				if (isDemo()) {
					if (_mystery.load(0, &_rng, isMacintosh())) {
						resetSiteArrivalState();
						_nextScreen = kScreenInitClues;
					} else {
						warning("doSetup: failed to restart demo practice mystery");
						_mystery.clear();
						_nextScreen = kScreenAction;
					}
				} else {
					_nextScreen = kScreenChooseMystery;
				}
				return;
			}

			if (kSaveBtn.contains(mx, my)) {
				saveProfile(_playerName);
				continue;
			}

			if (kDoneBtn.contains(mx, my)) {
				setupLeave();
				return;
			}

			if (kQuitBtn.contains(mx, my)) {
				if (areYouSure()) {
					_nextScreen = kScreenInvalid;
					return;
				}
				dirty = true;
				continue;
			}

			if (kHelpBtn.contains(mx, my)) {
				static const uint16 kHelp1Pics[] = { 0x0192, 0x01B1 };
				CursorMan.showMouse(false);
				for (uint i = 0; i < ARRAYSIZE(kHelp1Pics); i++) {
					setupDrawScreen();
					const Common::KeyCode k =
						setupShowFullscreenPic(kHelp1Pics[i], /* transparent= */ true);
					if (k == Common::KEYCODE_ESCAPE)
						break;
				}
				CursorMan.showMouse(true);
				dirty = true;
				continue;
			}

			if (kCreditsBtn.contains(mx, my)) {
				CursorMan.showMouse(false);
				setupShowFullscreenPic(0x208, /* transparent= */ false);
				CursorMan.showMouse(true);
				// PIC 0x208 has its own baked palette; restore site 0.
				setSitePalette(0);
				dirty = true;
				continue;
			}

			if (kProfileBtn.contains(mx, my)) {
				saveProfile(_playerName);
				_nextScreen = kScreenProfile;
				return;
			}

			if (kScrap1Btn.contains(mx, my)) {
				doShowScrapbook(1);
				setSitePalette(0);
				dirty = true;
				continue;
			}
			if (kScrap2Btn.contains(mx, my) && _chainStage >= 2) {
				doShowScrapbook(2);
				setSitePalette(0);
				dirty = true;
				continue;
			}

			if (kScrap3Btn.contains(mx, my) && !isLondon() &&
				_chainStage >= 3) {
				doShowScrapbook(3);
				setSitePalette(0);
				dirty = true;
				continue;
			}
		}
		if (dirty)
			setupDrawScreen();
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
}

void EEMEngine::setupDrawScreen() {
	// The setup screen shares the map overview's ColorTable. Set it explicitly
	// so the screen is correct no matter where it was entered from: the zoomed
	// detail map leaves its own palette (0x23) active, which would otherwise
	// bleed through here.
	setSitePalette(0x24);

	Graphics::ManagedSurface scratch(screenWidth(), screenHeight(),
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	Picture bg;
	if (_picsArchive.getPicture(0x40, bg))
		scratch.simpleBlitFrom(bg.surface);

	const byte kKey    = 0xFE;
	const byte kBright = 0x15;
	const byte kDim    = 0x00;
	swapColors(scratch, scaleRect(kSetupKid1Rect), kKey,
			   _partner == kPartnerJake ? kBright : kDim);
	swapColors(scratch, scaleRect(kSetupKid2Rect), kKey,
			   _partner == kPartnerJenny ? kBright : kDim);
	swapColors(scratch, scaleRect(kSetupSoundOnRect),  kKey,
			   _voiceOn ? kBright : kDim);
	swapColors(scratch, scaleRect(kSetupSoundOffRect), kKey,
			   _voiceOn ? kDim : kBright);

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, scratch.w, scratch.h);
	g_system->updateScreen();
}

Common::KeyCode EEMEngine::setupShowFullscreenPic(uint16 picId, bool transparent) {
	Picture pic;
	if (!_picsArchive.getPicture(picId, pic)) {
		warning("doSetup: PIC %u missing", (uint)picId);
		return Common::KEYCODE_INVALID;
	}
	Graphics::ManagedSurface scratch(screenWidth(), screenHeight(),
		Graphics::PixelFormat::createFormatCLUT8());
	if (transparent) {
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			scratch.simpleBlitFrom(*cur);
			g_system->unlockScreen();
		}
		const byte transp = (byte)(pic.flags >> 8);
		scratch.transBlitFrom(pic.surface, Common::Point(0, 0),
							  (uint32)transp);
	} else {
		scratch.clear();
		scratch.simpleBlitFrom(pic.surface);
	}
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, scratch.w, scratch.h);
	g_system->updateScreen();
	while (!shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return Common::KEYCODE_ESCAPE;
			if (ev.type == Common::EVENT_KEYDOWN)
				return ev.kbd.keycode;
			if (ev.type == Common::EVENT_LBUTTONDOWN)
				return Common::KEYCODE_INVALID;
		}
		g_system->delayMillis(15);
	}
	return Common::KEYCODE_ESCAPE;
}

void EEMEngine::setupLeave() {
	if (_nextScreen == kScreenSetup) {
		_nextScreen = (ScreenId)_lastScreen;
		if (_nextScreen == kScreenSetup ||
			_nextScreen == kScreenInvalid)
			_nextScreen = kScreenMap;
	}
	saveProfile(_playerName);
}
// `_SetupSettings @ 2046:0008`
void EEMEngine::setupDrawScreenLondon() {
	Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	Picture bg;
	if (_picsArchive.getPicture(0x40, bg))
		scratch.simpleBlitFrom(bg.surface);

	const byte kKey = 0xFE, kBright = 0x15, kDim = 0x00;

	swapColors(scratch, kLonSetJake,  kKey, _partner == kPartnerJake  ? kBright : kDim);
	swapColors(scratch, kLonSetJenny, kKey, _partner == kPartnerJenny ? kBright : kDim);

	swapColors(scratch, kLonSetVoiceOn,  kKey, _voiceOn ? kBright : kDim);
	swapColors(scratch, kLonSetVoiceOff, kKey, _voiceOn ? kDim : kBright);

	swapColors(scratch, kLonSetMusicOn,  kKey, _musicOn ? kBright : kDim);
	swapColors(scratch, kLonSetMusicOff, kKey, _musicOn ? kDim : kBright);

	const bool hiOn = !ConfMan.getBool("hide_highlight_boxes");
	swapColors(scratch, kLonSetHiOn,  kKey, hiOn ? kBright : kDim);
	swapColors(scratch, kLonSetHiOff, kKey, hiOn ? kDim : kBright);

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();
}

void EEMEngine::setupShowSavedConfirm() {
	Picture pic;
	if (!_picsArchive.getPicture(0x203, pic) || pic.surface.empty())
		return;
	Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	Graphics::Surface *cur = g_system->lockScreen();
	if (cur) {
		scratch.simpleBlitFrom(*cur);
		g_system->unlockScreen();
	}
	const int sx = MAX<int>(0, (kScreenWidth  - pic.surface.w) / 2);
	const int sy = MAX<int>(0, (kScreenHeight - pic.surface.h) / 2);
	scratch.transBlitFrom(pic.surface, Common::Point(sx, sy),
						  (uint32)(byte)(pic.flags >> 8));
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();
	while (!shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER ||
				ev.type == Common::EVENT_KEYDOWN ||
				ev.type == Common::EVENT_LBUTTONDOWN)
				return;
		}
		g_system->delayMillis(15);
	}
}

// `_DoSetup @ 2046:067b`
void EEMEngine::doSetupLondon() {
	if (!_font.isLoaded()) {
		_nextScreen = (ScreenId)_lastScreen;
		return;
	}
	const Common::Rect kPartnerBtn ( 20,  44,  39,  61); // [0]
	const Common::Rect kVoiceBtn   ( 20,  63,  39,  80); // [1]
	const Common::Rect kHiBtn      ( 20, 101,  39, 118); // [2]
	const Common::Rect kProfileBtn ( 20, 127,  39, 144); // [3]
	const Common::Rect kScrapNext  (281,  43, 299,  60); // [4]
	const Common::Rect kScrapPrev  (281,  62, 299,  79); // [5]
	const Common::Rect kSaveBtn    (281, 108, 299, 125); // [6]
	const Common::Rect kNewCaseBtn (281, 127, 299, 144); // [7]
	const Common::Rect kDoneBtn    ( 53, 153, 108, 183); // [8]
	const Common::Rect kHelpBtn    (145, 163, 174, 187); // [9]
	const Common::Rect kQuitBtn    (212, 153, 266, 184); // [10]
	const Common::Rect kCreditsBtn ( 81,  25, 238,  37); // [11]
	const Common::Rect kMusicBtn   ( 20,  82,  38,  99); // [12]

	setupDrawScreenLondon();
	_nextScreen = kScreenSetup;
	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_nextScreen = kScreenInvalid;
				return;
			}
			if (ev.type == Common::EVENT_KEYDOWN &&
				(ev.kbd.keycode == Common::KEYCODE_ESCAPE ||
				 ev.kbd.keycode == Common::KEYCODE_RETURN)) {
				setupLeave();
				return;
			}
			if (ev.type != Common::EVENT_LBUTTONDOWN)
				continue;
			const int mx = ev.mouse.x, my = ev.mouse.y;

			if (kPartnerBtn.contains(mx, my)) {
				_partner = (_partner == kPartnerJake) ? kPartnerJenny
													  : kPartnerJake;
				dirty = true;
				continue;
			}
			if (kVoiceBtn.contains(mx, my)) {
				_voiceOn = !_voiceOn;
				if (_audio)
					_audio->setVoiceEnabled(_voiceOn);
				dirty = true;
				continue;
			}
			if (kMusicBtn.contains(mx, my)) {
				_musicOn = !_musicOn;
				if (!_musicOn)
					stopMusic();
				dirty = true;
				continue;
			}
			if (kHiBtn.contains(mx, my)) {
				ConfMan.setBool("hide_highlight_boxes",
								!ConfMan.getBool("hide_highlight_boxes"));
				dirty = true;
				continue;
			}

			if (kProfileBtn.contains(mx, my)) {
				saveProfile(_playerName);
				_nextScreen = kScreenProfile;
				return;
			}
			if (kNewCaseBtn.contains(mx, my)) {
				saveProfile(_playerName);
				_nextScreen = kScreenChooseMystery;
				return;
			}
			if (kDoneBtn.contains(mx, my)) {
				setupLeave();
				return;
			}
			if (kQuitBtn.contains(mx, my)) {
				if (areYouSure()) {
					_nextScreen = kScreenInvalid;
					return;
				}
				dirty = true;
				continue;
			}

			if (kSaveBtn.contains(mx, my)) {
				if (_mystery.isLoaded()) {
					saveProfile(_playerName);
					setupShowSavedConfirm();
					dirty = true;
				}
				continue;
			}
			if (kHelpBtn.contains(mx, my)) {
				static const uint16 kHelpPics[] = { 0x0192, 0x01B1 };
				CursorMan.showMouse(false);
				for (uint i = 0; i < ARRAYSIZE(kHelpPics); i++) {
					setupDrawScreenLondon();
					if (setupShowFullscreenPic(kHelpPics[i], true) ==
						Common::KEYCODE_ESCAPE)
						break;
				}
				CursorMan.showMouse(true);
				dirty = true;
				continue;
			}
			if (kCreditsBtn.contains(mx, my)) {
				CursorMan.showMouse(false);
				setupShowFullscreenPic(0x208, /* transparent= */ false);
				CursorMan.showMouse(true);
				setSitePalette(0);
				dirty = true;
				continue;
			}

			if (kScrapNext.contains(mx, my)) {
				doShowScrapbook(1);
				setSitePalette(0);
				dirty = true;
				continue;
			}
			if (kScrapPrev.contains(mx, my) && _chainStage >= 2) {
				doShowScrapbook(2);
				setSitePalette(0);
				dirty = true;
				continue;
			}
		}
		if (dirty)
			setupDrawScreenLondon();
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
}
// `_ActionScreen @ 1c33:195b`
void EEMEngine::doActionScreen() {
	enum MenuPick {
		kPickChoose = 0,
		kPickPractice,
		kPickScrap1,
		kPickScrap2,
		kPickScrap3,
		kNumPicks
	};
	// Spanish from EEM.EXE floppy-es.
	const char *kPickLabelEN[kNumPicks] = {
		"         Choose A Mystery",
		"         Practice Mystery",
		"         See ScrapBook 1",
		"         See ScrapBook 2",
		"         See ScrapBook 3"
	};
	const char *kPickLabelES[kNumPicks] = {
		"         Elegir Misterio ",
		"         Caso de Practica",
		"         Ver Recortes  1",
		"         Ver Recortes  2",
		"         Ver Recortes  3"
	};
	const char * const *kPickLabel = isSpanish() ? kPickLabelES : kPickLabelEN;
	// London has no BOOK3.NME, and the demo ships only the practice case.
	const uint numPicks = isDemo() ? (uint)kPickScrap1
						: isLondon() ? (uint)kPickScrap3 : (uint)kNumPicks;

	uint loT = 0, hiT = 0;
	const bool anySolved1 = mysteryTierRange(1, loT, hiT) &&
							anyMysterySolved(loT, hiT);
	const bool anySolved2 = mysteryTierRange(2, loT, hiT) &&
							anyMysterySolved(loT, hiT);
	const bool haveTier3 = mysteryTierRange(3, loT, hiT);
	const bool anySolved3 = haveTier3 && anyMysterySolved(loT, hiT);

	const bool chooseOn   = !isDemo() && _chainStage < 4;
	const bool practiceOn = isDemo() || _chainStage <= 1;
	const bool scrap1On =
		_chainStage >= 2 || (_chainStage == 1 && anySolved1);
	const bool scrap2On =
		_chainStage >= 3 || (_chainStage == 2 && anySolved2);
	const bool scrap3On =
		haveTier3 && (_chainStage >= 4 || (_chainStage == 3 && anySolved3));
	const bool kPickEnabled[kNumPicks] = {
		chooseOn, practiceOn, scrap1On, scrap2On, scrap3On
	};

	uint pick = 0;
	for (uint i = 0; i < numPicks; i++) {
		if (kPickEnabled[i]) {
			pick = i;
			break;
		}
	}

	const char *kSeparator = "----------------------------------";

	const Common::Rect kOkRect      ( 12,  63,  41,  87); // 29be:0cd8 confirm
	const Common::Rect kHelpRect    ( 12, 100,  41, 124); // 29be:0ce0 help
	const Common::Rect kExitRect    ( 12, 137,  41, 161); // 29be:0ce8 cancel
	const Common::Rect kListRect    ( 58,  35, 238, 158); // 29be:0d00 list panel

	CursorMan.showMouse(true);

	setSitePalette(0);

	Picture bg;
	const bool haveBg = _picsArchive.getPicture(kActionScreenBackgroundPic, bg);
	Picture decor;
	const bool haveDecor = _picsArchive.getPicture(kActionScreenDecorPic, decor);

	ActionMenuView v;
	v.vm = this;
	v.bg = &bg;
	v.haveBg = haveBg;
	v.decor = &decor;
	v.haveDecor = haveDecor;
	v.separator = kSeparator;
	v.pickLabel = kPickLabel;
	v.pickEnabled = kPickEnabled;
	v.pick = pick;
	v.numPicks = numPicks;

	drawActionMenuFrame(v);
	uint32 chooserLastTick = g_system->getMillis();

	bool exitChosen = false;
	while (!shouldQuit()) {
		Common::Event ev;
		bool confirmed = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				const Common::Point mouse(unscaleX(ev.mouse.x),
										  unscaleY(ev.mouse.y));
				if (kOkRect.contains(mouse.x, mouse.y)) {
					if (kPickEnabled[pick])
						confirmed = true;
					break;
				}
				if (kExitRect.contains(mouse.x, mouse.y)) {
					exitChosen = true;
					confirmed = true;
					break;
				}
				if (kHelpRect.contains(mouse.x, mouse.y)) {
					continue;
				}
				if (kListRect.contains(mouse.x, mouse.y)) {
					const int kLineH = 10;
					const int row = (mouse.y - kListRect.top) / kLineH;
					if ((row & 1) == 1) {
						const uint mp = (uint)(row >> 1);
						if (mp < numPicks && kPickEnabled[mp]) {
							pick = mp;
							v.pick = pick;
							drawActionMenuFrame(v);
							continue;
						}
					}
				}
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_ESCAPE) {
				openMainMenuDialog();
				continue;
			}
			if (k == Common::KEYCODE_RETURN ||
				k == Common::KEYCODE_KP_ENTER) {
				if (kPickEnabled[pick])
					confirmed = true;
				break;
			}
			if (k == Common::KEYCODE_UP || k == Common::KEYCODE_LEFT) {
				for (int i = 0; i < (int)numPicks; i++) {
					pick = (pick == 0) ? (numPicks - 1) : pick - 1;
					if (kPickEnabled[pick])
						break;
				}
				v.pick = pick;
				drawActionMenuFrame(v);
				continue;
			}
			if (k == Common::KEYCODE_DOWN || k == Common::KEYCODE_RIGHT ||
				k == Common::KEYCODE_TAB) {
				for (int i = 0; i < (int)numPicks; i++) {
					pick = (pick + 1) % numPicks;
					if (kPickEnabled[pick])
						break;
				}
				v.pick = pick;
				drawActionMenuFrame(v);
				continue;
			}
		}
		if (confirmed) {
			v.pick = pick;
			drawActionMenuFrame(v);
			break;
		}
		const uint32 now = g_system->getMillis();
		if (now - chooserLastTick >= kChooserCycleMillis) {
			chooserLastTick = now;
			cycleChooserPalette();
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	if (shouldQuit())
		return;

	if (exitChosen) {
		if (areYouSure()) {
			_mystery.clear();
			_nextScreen = kScreenInvalid;
		} else {
			drawActionMenuFrame(v);
			_nextScreen = kScreenAction;
		}
		return;
	}

	if (pick == kPickPractice) {
		if (!_mystery.load(0, &_rng, isMacintosh())) {
			warning("doActionScreen: failed to load practice mystery");
			_mystery.clear();
			resetSiteArrivalState();
		} else {
			resetSiteArrivalState();
			if (_audio && !isFloppy())
				_audio->initMysterySounds(0);
		}
		return;
	}

	if (pick == kPickScrap1 || pick == kPickScrap2 || pick == kPickScrap3) {
		const uint stage = (pick == kPickScrap1) ? 1
						 : (pick == kPickScrap2) ? 2 : 3;
		doShowScrapbook(stage);
		setSitePalette(0);
		_mystery.clear();
		_nextScreen = kScreenAction;
		return;
	}

	_nextScreen = kScreenChooseMystery;
}
// EEM1: `_DoChooseMystery @ 1a35:02b7` + `_CaseSelection @ 1c33:0a87`
// EEM2:  1abf:022a + 1cd3:0a9d)
void EEMEngine::doCaseSelection() {
	const uint kMaxMystery = isLondon() ? 50 : 54;

	CursorMan.showMouse(true);
	setSitePalette(0);
	_mystery.clear();
	resetSiteArrivalState();

	Picture caseBg;
	const bool haveCaseBg = _picsArchive.getPicture(0x41, caseBg);
	Picture revealPic;
	const bool haveRevealPic =
		_picsArchive.getPicture(kCaseSelectionRevealPic, revealPic);

	const uint kKdAniId = (_partner == kPartnerJake) ? 0x15 : 0x16;
	Animation kdAnim;
	const bool haveKdAnim = _aniArchive.loadAnimation(kKdAniId, kdAnim)
							 && !kdAnim.empty();
	const int kKdAnimX = 0x112;
	const int kKdAnimY = 0x50;

	uint book;
	switch (_chainStage) {
	case 2:  book = 2; break;
	case 3:  book = isLondon() ? 2 : 3; break;
	default: book = 1; break;
	}
	uint stageLo = 0, stageHi = 0;
	mysteryTierRange(book, stageLo, stageHi);
	if (stageHi > kMaxMystery)
		stageHi = kMaxMystery;

	const Common::StringArray names = loadBookNames(book, isMacintosh());
	if (names.empty()) {
		warning("doCaseSelection: BOOK%u.NME failed to load", book);
		return;
	}

	if (isLondon() && _music && _voiceOn)
		_music->playMus(2, /* loop= */ true);

	const uint listLen = MIN<uint>((uint)names.size(), stageHi - stageLo + 1);

	Common::Array<bool> solvedFlags;
	solvedFlags.resize(listLen);
	for (uint i = 0; i < listLen; i++) {
		const uint mn = stageLo + i;
		solvedFlags[i] =
			mn < sizeof(_mysteriesSolved) && _mysteriesSolved[mn] != 0;
	}

	uint selRow = 0;
	while (selRow < listLen && solvedFlags[selRow])
		selRow++;
	if (selRow >= listLen)
		selRow = 0;

	uint topRow = 0;
	const uint kVisible = 12;
	if (selRow >= kVisible) {
		topRow = selRow - kVisible / 2;
		clampCaseTopRow(topRow, listLen, kVisible);
	}

	CaseSubmenuView sv;
	sv.vm = this;
	sv.caseBg = &caseBg;
	sv.haveCaseBg = haveCaseBg;
	sv.revealPic = &revealPic;
	sv.haveRevealPic = haveRevealPic;
	sv.kdAnim = &kdAnim;
	sv.haveKdAnim = haveKdAnim;
	sv.kdAnimX = kKdAnimX;
	sv.kdAnimY = kKdAnimY;
	sv.names = &names;
	sv.solvedFlags = &solvedFlags;
	sv.topRow = topRow;
	sv.selRow = selRow;
	sv.book = book;

	if (animateCaseSelectionReveal(this, &caseBg, haveCaseBg,
								   &revealPic, haveRevealPic,
								   &kdAnim, haveKdAnim,
								   kKdAnimX, kKdAnimY, book))
		return;
	drawCaseSubmenu(sv);
	uint32 submenuLastTick = g_system->getMillis();
	bool confirmed = false;
	while (!confirmed && !shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				const Common::Point mouse(unscaleX(ev.mouse.x),
										  unscaleY(ev.mouse.y));
				if (kChooserOkRect.contains(mouse.x, mouse.y)) {
					if (selRow < listLen && !solvedFlags[selRow])
						confirmed = true;
					break;
				}
				if (kChooserExitRect.contains(mouse.x, mouse.y)) {
					_mystery.clear();
					return;
				}
				if (kChooserHelpRect.contains(mouse.x, mouse.y)) {
					saveProfile(_playerName);
					_mystery.clear();
					_nextScreen = kScreenProfile;
					return;
				}
				if (kChooserUpArrowRect.contains(mouse.x, mouse.y)) {
					if (topRow > 0) {
						topRow--;
						dirty = true;
					}
					continue;
				}
				if (kChooserDnArrowRect.contains(mouse.x, mouse.y)) {
					topRow++;
					clampCaseTopRow(topRow, listLen, kVisible);
					dirty = true;
					continue;
				}
				if (kChooserListRect.contains(mouse.x, mouse.y)) {
					const int kLineH = 10;
					const int row = (mouse.y - kChooserListRect.top) / kLineH;
					if (row < 0 || row >= (int)kVisible)
						continue;
					const uint idx = topRow + (uint)row;
					if (idx >= listLen || solvedFlags[idx])
						continue;
					// Second click on selected row = OK (QoL fix).
					if (idx == selRow) {
						confirmed = true;
						break;
					}
					selRow = idx;
					dirty = true;
					continue;
				}
				continue;
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_ESCAPE) {
				openMainMenuDialog();
				continue;
			}
			if (k == Common::KEYCODE_RETURN ||
				k == Common::KEYCODE_KP_ENTER) {
				if (selRow < listLen && !solvedFlags[selRow])
					confirmed = true;
				break;
			}
			if (k == Common::KEYCODE_DOWN || k == Common::KEYCODE_TAB) {
				if (selRow + 1 < listLen) {
					selRow++;
					if (selRow >= topRow + kVisible) {
						topRow = selRow - kVisible + 1;
						clampCaseTopRow(topRow, listLen, kVisible);
					}
					dirty = true;
				}
				continue;
			}
			if (k == Common::KEYCODE_UP) {
				if (selRow > 0) {
					selRow--;
					if (selRow < topRow)
						topRow = selRow;
					dirty = true;
				}
				continue;
			}
			if (k == Common::KEYCODE_PAGEDOWN) {
				selRow = MIN<uint>(selRow + kVisible, listLen - 1);
				if (selRow >= topRow + kVisible) {
					topRow = selRow - kVisible + 1;
					clampCaseTopRow(topRow, listLen, kVisible);
				}
				dirty = true;
				continue;
			}
			if (k == Common::KEYCODE_PAGEUP) {
				selRow = (selRow >= kVisible) ? selRow - kVisible : 0;
				if (selRow < topRow)
					topRow = selRow;
				dirty = true;
				continue;
			}
			if (k == Common::KEYCODE_HOME) {
				selRow = 0;
				topRow = 0;
				dirty = true;
				continue;
			}
			if (k == Common::KEYCODE_END) {
				selRow = listLen - 1;
				topRow = listLen > kVisible ? listLen - kVisible : 0;
				dirty = true;
				continue;
			}
		}
		const uint32 now = g_system->getMillis();
		const bool chooserTick = now - submenuLastTick >= kChooserCycleMillis;
		if (chooserTick) {
			submenuLastTick = now;
			cycleChooserPalette();
		}
		if (dirty || (chooserTick && haveKdAnim)) {
			sv.topRow = topRow;
			sv.selRow = selRow;
			drawCaseSubmenu(sv);
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	if (shouldQuit())
		return;

	const uint mn = stageLo + selRow;
	if (!_mystery.load(mn, &_rng, isMacintosh())) {
		warning("doCaseSelection: failed to load mystery %u", mn);
		_mystery.clear();
		return;
	}
	resetSiteArrivalState();
	if (_audio && !isFloppy())
		_audio->initMysterySounds(mn);
	debugC(1, kDebugMystery, "Mystery %u loaded; %u sites, %u suspects",
		   mn, _mystery.numSites(), _mystery.numSuspects());
}
// `_DoNotebook @ 161e:0500` + `_DrawNotes @ 161e:01d0` +
// `_HandleNoteButton @ 161e:03cb`. _NotebookRect = (78, 12, 288, 152).
// _NoteButtons @ 29be:0147 — 11 rects × 8 bytes. Jumptable @ 161e:04ec
// dispatches handler[i-1] (rect 0's i-1 underflows = decorative slot):
//   [0] (134,174,155,190)  decorative — no handler
//   [1] ( 93,174,115,190)  HELP → `_InterfaceHelp(0)`           (0x3f9)
//   [2] (157,174,178,190)  GALLERY → `_NextScreen = 5`          (0x477)
//   [3] (  5, 80, 44,110)  host hint → `_KDHelp`                (0x403)
//   [4] (180,174,201,190)  SOLVE → `_SolvedCheck` → NextScreen=7 (0x436)
//   [5] (204,174,224,190)  PAGE NEXT → `_EraseNotes` + redraw   (0x489)
//   [6] (226,174,247,190)  PAGE PREV → CurrentPage-- + redraw   (0x4ab)
//   [7] (  7,177, 57,200)  MAP → `_NextScreen = 2`              (0x480)
//   [8] ( 35,111, 56,136)  SITE → `_NextScreen = 3`             (0x3ed)
//   [9] (  0,  0,  0,  0)  same exit as [8]
//   [10] (267,174,288,190) → `_InterfaceHelp(0)` again          (0x3f9)
//
// EEM2/London (`_DoNotebook @ 16a0:0517`, `_HandleNoteButton @ 16a0:03dd`,
// jumptable @ 16a0:0503) reuses the SAME button rect table (2bca:0151) and
// handler set, but reassigns two slots and revives slot 9 (verified by
// disassembly — `[0x9292]` is the next-screen code):
//   [1] ( 93,174,115,190) MAP  → screen 2  (EEM1: a 2nd InterfaceHelp).
//                         London's dedicated map button.
//   [7] (  7,177, 57,200) DOS EEM2 → SITE, but we keep it as MAP (→ 2) in
//                         both variants — the EEM1 partner-foot map shortcut
//                         (a player convenience alongside button [1]).
//   [9] (  0,  0, 66, 79) SITE → screen 3  (EEM1: dead 0-rect)
// Everything else (gallery, accuse, host hint, page next/prev, help [10],
// site [8]) is identical. The note rendering, pagination, partner ANI
// (same 1/0xb), and gizmo colour-cycle are shared as-is; only button [1]
// (London → map) and the extra close area [9] are gated on `isLondon()`.

void EEMEngine::doNotebook() {
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	CursorMan.showMouse(true);

	int page = 0;
	int hoveredNoteSlot = -1;
	(void)hoveredNoteSlot;

	const bool notebookFromSite = isLondon() && _lastScreen == kScreenSite;
	if (_music && _voiceOn && notebookFromSite)
		_music->playMus(30, /* loop= */ false);

	if (isMacintosh())
		setSitePalette(0);
	drawNotebookFrame(page);
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	setInteractiveMouseCursor(notebookButtonAt(this, mouse.x, mouse.y));

	if (isLondon() && _music && _voiceOn) {
		while (notebookFromSite && _music->isPlaying() && !shouldQuit()) {
			Common::Event drain;
			while (g_system->getEventManager()->pollEvent(drain)) {}
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
		_music->playMus(5, /* loop= */ true);
	}

	uint32 lastDraw = g_system->getMillis();
	uint32 gizmoLastTick = lastDraw;

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		bool exitFlag = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_nextScreen = kScreenInvalid;
				exitFlag = true;
				break;
			}
			if (ev.type == Common::EVENT_MOUSEMOVE) {
				setInteractiveMouseCursor(notebookButtonAt(this, ev.mouse.x,
														   ev.mouse.y));
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					openMainMenuDialog();
					continue;
				}
				if (ev.kbd.keycode == Common::KEYCODE_LEFT ||
					ev.kbd.keycode == Common::KEYCODE_PAGEUP) {
					if (page > 0)
						page--;
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT ||
						   ev.kbd.keycode == Common::KEYCODE_PAGEDOWN ||
						   ev.kbd.keycode == Common::KEYCODE_TAB) {
					page++;
					dirty = true;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				if (pdaControlRect(this, kPdaSiteRect).contains(ev.mouse.x,
																ev.mouse.y) ||
					(isLondon() &&
					 kPdaLondonCloseRect.contains(ev.mouse.x, ev.mouse.y))) {
					_nextScreen = kScreenSite;
					exitFlag = true;
					break;  // back to site
				}
				if (pdaControlRect(this, kPdaPartnerFootMapRect)
						.contains(ev.mouse.x, ev.mouse.y)) {
					_nextScreen = kScreenMapAlt;
					exitFlag = true;
					break;
				}
				if (pdaControlRect(this, kPdaPartnerHeadHintRect)
						.contains(ev.mouse.x, ev.mouse.y)) {
					setInteractiveMouseCursor(false);
					doHelp();
					if (isMacintosh())
						setSitePalette(0);
					dirty = true;
					continue;
				}
				if (pdaControlRect(this, kPdaAccuseRect).contains(ev.mouse.x,
																  ev.mouse.y)) {
					_nextScreen = kScreenAccuse;
					exitFlag = true;
					break;
				}
				if (pdaControlRect(this, kPdaGalleryRect).contains(ev.mouse.x,
																   ev.mouse.y)) {
					_nextScreen = kScreenGallery;
					exitFlag = true;
					break;
				}
				if (pdaControlRect(this, kPdaHelpRect).contains(ev.mouse.x,
																ev.mouse.y)) {
					if (isLondon()) {
						_nextScreen = kScreenMapAlt;
						exitFlag = true;
						break;
					}
					setInteractiveMouseCursor(false);
					doInterfaceHelp(0);
					if (isMacintosh())
						setSitePalette(0);
					dirty = true;
					continue;
				}
				if (pdaControlRect(this, kPdaPagePrevRect)
						.contains(ev.mouse.x, ev.mouse.y)) {
					if (page > 0)
						page--;
					dirty = true;
					continue;
				}
				if (pdaControlRect(this, kPdaPageNextRect)
						.contains(ev.mouse.x, ev.mouse.y)) {
					page++;
					dirty = true;
					continue;
				}
				if (pdaControlRect(this, kPdaHelp2Rect).contains(ev.mouse.x,
																 ev.mouse.y)) {
					setInteractiveMouseCursor(false);
					doInterfaceHelp(0);
					if (isMacintosh())
						setSitePalette(0);
					dirty = true;
					continue;
				}
			}
		}
		if (exitFlag)
			break;

		const uint32 now = g_system->getMillis();
		if (dirty || now - lastDraw >= 100) {
			drawNotebookFrame(page);
			lastDraw = now;
			mouse = g_system->getEventManager()->getMousePos();
			setInteractiveMouseCursor(notebookButtonAt(this, mouse.x,
													   mouse.y));
		}
		if (now - gizmoLastTick >= kChooserCycleMillis) {
			gizmoLastTick = now;
			cycleChooserPalette();
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
	setInteractiveMouseCursor(false);
}

Common::String EEMEngine::notebookNoteText(uint clueId, const byte *ni,
										   uint16 niCount, bool floppyNb,
										   const byte *bufBase,
										   uint32 mysSz) const {
	if (!ni || clueId >= niCount)
		return Common::String();
	if (floppyNb && bufBase) {
		const uint16 textOff = READ_LE_UINT16(ni + clueId * 7);
		if (textOff == 0 || textOff >= mysSz)
			return Common::String();
		const char *p = (const char *)(bufBase + textOff);
		uint32 len = 0;
		while (textOff + len < mysSz && p[len] != 0)
			len++;
		return parseString(Common::String(p, len),
						   _playerName, _partner);
	}
	if (isMacintosh() && !isLondon() && bufBase) {
		const uint16 textOff = READ_LE_UINT16(ni + clueId * 8);
		if (textOff == 0 || textOff >= mysSz)
			return Common::String();
		const char *p = (const char *)(bufBase + textOff);
		uint32 len = 0;
		while (textOff + len < mysSz && p[len] != 0)
			len++;
		return parseString(Common::String(p, len),
						   _playerName, _partner);
	}
	const uint stride = isLondon() ? 2 : 4;
	const uint16 textOff = READ_LE_UINT16(ni + clueId * stride);
	return parseString(_mystery.textAt(textOff),
					   _playerName, _partner);
}

void EEMEngine::drawNotebookFrame(int &page) {
	const Common::Rect kNotebookRect(78, 12, 288, 152);
	const Common::Rect notebookRect =
		isMacintosh() ? kMacNotebookTextRect : kNotebookRect;
	const int sw = screenWidth();
	const int sh = screenHeight();

	Graphics::ManagedSurface scratch(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	Picture frame;
	if (_picsArchive.getPicture(0x3f, frame))
		scratch.simpleBlitFrom(frame.surface);

	blitPdaPartner(scratch, _aniArchive, _partner, kPdaNotebookPartner,
				   g_system->getMillis(), isMacintosh());

	// `_DrawNotes` walks `_NoteIndex` for current page; word-wraps each
	// found clue in `_NotebookRect`. Selected = color 0x3c.
	// EEM2/London NoteIndex entries are 2 bytes (no points field), so its real
	// clue count is the section size / 2; `noteIndexCount()` assumes the EEM1
	// 4-byte stride and undercounts by half. The notebook, gallery, and accuse
	// note lists all use the London count so no found clue is dropped.
	const uint16 londonCount = isLondon()
		? (uint16)(_mystery.noteSectionSize() / 2) : 0;
	Common::Array<uint> found;
	for (uint i = 0; i < Mystery::kCluesFoundCap; i++) {
		const bool hasText = isLondon()
			? (i < londonCount)
			: _mystery.noteHasNotebookText(i);
		if (_mystery._cluesFound[i] && hasText)
			found.push_back(i);
	}
	const byte *ni = _mystery.noteIndex();
	const uint16 niCount = isLondon()
		? londonCount : _mystery.noteIndexCount();

	const int kRectX = notebookRect.left;
	const int kRectY = notebookRect.top;
	const int kRectW = notebookRect.width();
	const int kRectH = notebookRect.height();

	int clueCursor = 0;
	Common::Array<int> pageStarts;
	pageStarts.push_back(0);

	const bool floppyNb = isFloppy();
	const byte *bufBase = _mystery.blobAt(0);
	const uint32 mysSz  = _mystery.dataSize();
	{
		const int lineH = _font.getFontHeight();
		int y = kRectY;
		while (clueCursor < (int)found.size()) {
			const uint clueId = found[clueCursor];
			Common::String txt = notebookNoteText(clueId, ni, niCount,
												  floppyNb, bufBase, mysSz);
			// Measure height by wrapping the text without drawing.
			Common::Array<Common::String> wrapped;
			_font.wordWrapText(txt, kRectW, wrapped);
			const int h = (int)wrapped.size() * lineH;
			if (y + h + 7 > kRectY + kRectH) {
				// Page break before this clue.
				y = kRectY;
				pageStarts.push_back(clueCursor);
			}
			y += h + 7;
			clueCursor++;
		}
		if (page >= (int)pageStarts.size())
			page = (int)pageStarts.size() - 1;
		if (page < 0)
			page = 0;
	}

	const int startClue = (page < (int)pageStarts.size())
							? pageStarts[page] : 0;
	const int endClue   = (page + 1 < (int)pageStarts.size())
							? pageStarts[page + 1] : (int)found.size();

	int y = kRectY;
	for (int i = startClue; i < endClue; i++) {
		const uint clueId = found[i];
		Common::String txt = notebookNoteText(clueId, ni, niCount,
											  floppyNb, bufBase, mysSz);
		if (txt.empty())
			txt = Common::String::format(
				isSpanish() ? "nota %u" : "clue %u", clueId);
		Common::Array<Common::String> wrapped;
		_font.wordWrapText(txt, kRectW, wrapped);
		const int lineH = _font.getFontHeight();
		const int h = (int)wrapped.size() * lineH;
		const byte color = _mystery._noteSelected[clueId] ? 0x3C : 0x5C;
		for (uint li = 0; li < wrapped.size(); li++) {
			_font.drawString(&scratch, wrapped[li], kRectX,
							 y + (int)li * lineH, kRectW, color);
		}
		y += h + 7;
	}

	const bool isLastPage = (page + 1 >= (int)pageStarts.size());
	if (isLastPage) {
		const char *kEndMarker = isSpanish()
			? "-- Fin de las notas --"
			: "-- End of notes --";
		_font.drawString(&scratch, kEndMarker, kRectX, y, kRectW, 0x5C);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, sw, sh);
	g_system->updateScreen();
}

void EEMEngine::doGallery() {
	if (!_mystery.isLoaded())
		return;

	const byte *gd = _mystery.galleryData();
	if (!gd) {
		warning("doGallery: no GalleryData in mystery %u", _mystery.number());
		return;
	}

	CursorMan.showMouse(true);
	if (isMacintosh())
		setSitePalette(0);

	Picture galBg;
	const bool haveBg = _picsArchive.getPicture(0x3f, galBg);

	if (isLondon() && _music && _voiceOn)
		_music->playMus(5, /* loop= */ true);

	const uint8 num = _mystery.numSuspects();

	Common::Array<Common::Rect> slotRects;
	Common::Array<int> slotSuspect;
	slotRects.resize(num);
	slotSuspect.resize(num);
	for (uint i = 0; i < num; i++) {
		slotSuspect[i] = -1;
	}

	drawGalleryFrame(gd, num, slotRects, slotSuspect);
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	setInteractiveMouseCursor(galleryButtonAt(this, mouse.x, mouse.y) ||
							  gallerySlotAt(slotRects, slotSuspect,
											mouse.x, mouse.y));
	uint32 lastDraw = g_system->getMillis();
	uint32 gizmoLastTick = lastDraw;

	while (!shouldQuit()) {
		Common::Event ev;
		bool exitFlag = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_nextScreen = kScreenInvalid;
				setInteractiveMouseCursor(false);
				return;
			}
			if (ev.type == Common::EVENT_MOUSEMOVE) {
				setInteractiveMouseCursor(galleryButtonAt(this, ev.mouse.x,
														  ev.mouse.y) ||
										  gallerySlotAt(slotRects,
														slotSuspect,
														ev.mouse.x,
														ev.mouse.y));
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					openMainMenuDialog();
					continue;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// Shares _NoteButtons @ 29be:0147 with doNotebook;
				// _HandleGalleryButton @ 158f:05c0 jumptable @ 158f:0645:
				//   [0] (134,155) → NOTEBOOK = NextScreen=4   (0x5ef)
				//   [1] ( 93,115) → HELP = _InterfaceHelp(0)  (0x625)
				//   [2] (157,178) → generic exit (no-op)      (0x638)
				//   [3] (  5, 80) → _KDHelp (host hint)       (0x61e)
				//   [4] (180,201) → _SolvedCheck → SOLVE      (0x5ff)
				//   [5] (204,224) → generic exit              (0x638)
				//   [6] (226,247) → generic exit              (0x638)
				//   [7] (  7,177) → MAP = NextScreen=2        (0x5f7)
				//   [8] ( 35,111) → SITE = NextScreen=3       (0x5e4)
				if (pdaControlRect(this, kPdaSiteRect).contains(ev.mouse.x,
																ev.mouse.y)) {
					_nextScreen = kScreenSite;
					exitFlag = true;
					break;
				}
				if (pdaControlRect(this, kPdaPartnerFootMapRect)
						.contains(ev.mouse.x, ev.mouse.y)) {
					_nextScreen = kScreenMapAlt;
					exitFlag = true;
					break;
				}
				if (pdaControlRect(this, kPdaAccuseRect).contains(ev.mouse.x,
																  ev.mouse.y)) {
					_nextScreen = kScreenAccuse;
					exitFlag = true;
					break;
				}
				if (pdaControlRect(this, kPdaNotebookRect).contains(ev.mouse.x,
																	ev.mouse.y)) {
					_nextScreen = kScreenNotebook;
					exitFlag = true;
					break;
				}
				if (pdaControlRect(this, kPdaHelpRect).contains(ev.mouse.x,
																ev.mouse.y)) {
					if (isLondon()) {
						_nextScreen = kScreenMapAlt;
						exitFlag = true;
						break;
					}
					setInteractiveMouseCursor(false);
					doInterfaceHelp(0);
					if (isMacintosh())
						setSitePalette(0);
					lastDraw = 0;
					continue;
				}
				if (pdaControlRect(this, kPdaPartnerHeadHintRect)
						.contains(ev.mouse.x, ev.mouse.y)) {
					setInteractiveMouseCursor(false);
					doHelp();
					if (isMacintosh())
						setSitePalette(0);
					lastDraw = 0;
					continue;
				}
				bool clicked = false;
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotSuspect[i] < 0)
						continue;
					if (slotRects[i].contains(ev.mouse.x, ev.mouse.y)) {
						if (moreInfo(gd, (uint)slotSuspect[i],
									 galBg, haveBg))
							exitFlag = true;
						drawGalleryFrame(gd, num, slotRects, slotSuspect);
						lastDraw = g_system->getMillis();
						mouse = g_system->getEventManager()->getMousePos();
						setInteractiveMouseCursor(galleryButtonAt(this, mouse.x,
																  mouse.y) ||
												  gallerySlotAt(slotRects,
																slotSuspect,
																mouse.x,
																mouse.y));
						clicked = true;
						break;
					}
				}
				(void)clicked;
			}
		}
		if (exitFlag)
			break;

		const uint32 now = g_system->getMillis();
		if (now - lastDraw >= 100) {
			drawGalleryFrame(gd, num, slotRects, slotSuspect);
			lastDraw = now;
			mouse = g_system->getEventManager()->getMousePos();
			setInteractiveMouseCursor(galleryButtonAt(this, mouse.x, mouse.y) ||
									  gallerySlotAt(slotRects, slotSuspect,
													mouse.x, mouse.y));
		}
		if (now - gizmoLastTick >= kChooserCycleMillis) {
			gizmoLastTick = now;
			cycleChooserPalette();
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
	setInteractiveMouseCursor(false);
}
// `MoreInfo @ 158f:0419`
bool EEMEngine::moreInfo(const byte *gd, uint suspectIdx,
						  const Picture &galBg, bool haveBg) {
	const bool floppyMI = isFloppy();
	const bool mac = isMacintosh();
	const bool compactMI = floppyMI || _mystery.usesCompactMacData();
	const byte *suspect = compactMI
							  ? _mystery.floppySuspectEntry(suspectIdx)
							  : gd + suspectIdx * 0x46;
	if (!suspect)
		return false;
	const uint16 detailPic = READ_LE_UINT16(suspect + 0);
	const uint clueCount = compactMI
							   ? (uint)suspect[4]
							   : READ_LE_UINT16(suspect + 8);

	setInteractiveMouseCursor(false);

	const Common::Rect noteRectBase(78, 93, 288, 152);
	const Common::Rect noteRect = mac ? scaleRect(noteRectBase) : noteRectBase;
	const int rx = noteRect.left;
	const int ry = noteRect.top;
	const int rw = noteRect.width();
	const int rh = noteRect.height();
	const int sw = screenWidth();
	const int sh = screenHeight();
	const int lineH = _font.getFontHeight();
	const uint clueMax = compactMI ? clueCount : 30u;
	const byte *ni = _mystery.noteIndex();
	const uint16 niCount = isLondon()
		? (uint16)(_mystery.noteSectionSize() / 2)
		: _mystery.noteIndexCount();

	uint pageStart = 0;
	Common::Array<uint> pageStack;
	bool back = false;
	bool exitGallery = false;
	bool isFirstShow = true;

	while (!back && !shouldQuit()) {
		Graphics::ManagedSurface ms(sw, sh,
			Graphics::PixelFormat::createFormatCLUT8());
		ms.clear();
		if (haveBg)
			ms.simpleBlitFrom(galBg.surface);

		blitPdaPartner(ms, _aniArchive, _partner, kPdaGalleryPartner,
					   g_system->getMillis(), mac);
		Picture detail;
		if (_picsArchive.getPicture(detailPic, detail)) {
			const Common::Point detailPos =
				mac ? scalePoint(0x94, 0x0f) : Common::Point(0x94, 0x0f);
			if (mac)
				blitMacMaskedSurface(ms.surfacePtr(), detail,
									 detailPos.x, detailPos.y);
			else
				ms.transBlitFrom(detail.surface, detailPos,
								 (uint32)(byte)(detail.flags >> 8));
		}

		// Walk clues from pageStart; defer overflow to next page unless
		// first clue is too tall to ever fit.
		int yPos = ry;
		bool drewAny = false;
		uint k = pageStart;
		bool reachedEnd = false;
		for (; k < clueCount && k < clueMax; k++) {
			const uint16 clueId = compactMI
				? (uint16)suspect[5 + k]
				: READ_LE_UINT16(suspect + 0xa + k * 2);
			if (!compactMI && clueId == 0xFFFF) {
				reachedEnd = true;
				break;
			}
			if (clueId >= Mystery::kCluesFoundCap ||
				!_mystery._cluesFound[clueId])
				continue;
			if (!ni || clueId >= niCount)
				continue;
			// Shared notebook lookup: honours London's 2-byte NoteIndex stride
			// (this path previously hardcoded the EEM1 4-byte stride).
			Common::String txt = notebookNoteText(clueId, ni, niCount, floppyMI,
												  _mystery.blobAt(0),
												  _mystery.dataSize());
			if (txt.empty())
				continue;

			Common::Array<Common::String> wrapped;
			_font.wordWrapText(txt, MAX<int>(8, rw), wrapped);
			const int hClue = (int)wrapped.size() * lineH;
			if (yPos + hClue > ry + rh && drewAny) {
				// Defer to next page.
				break;
			}
			const byte color = _mystery._noteSelected[clueId]
								   ? 0x3C : 0x5C;
			for (uint l = 0; l < wrapped.size(); l++) {
				_font.drawString(&ms, wrapped[l], rx,
					yPos + (int)l * lineH, MAX<int>(8, rw), color);
			}
			yPos += hClue + 7;
			drewAny = true;
		}
		if (k >= clueCount || k >= clueMax)
			reachedEnd = true;
		const uint pageEnd = k;
		const bool hasMore = !reachedEnd;
		const bool hasPrev = !pageStack.empty();

		if (pageStart == 0 && !drewAny && _font.isLoaded()) {
			_font.drawString(&ms,
				isSpanish()
					? "Aun no hay pistas para este sospechoso."
					: "No clues yet for this suspect.",
				rx, ry, MAX<int>(8, rw), 0x5C);
		}
		// Header / footer text.
		if (_font.isLoaded()) {
			_font.drawString(&ms,
				isSpanish() ? "EXPEDIENTE" : "SUSPECT FILE",
				rx, ry - 11, MAX<int>(8, rw), 0x3C);
			_font.drawString(&ms,
				isSpanish() ? "(ESC: volver)" : "(ESC: back)",
				rx, ry + rh + 2, MAX<int>(8, rw), 0x3C);
		}
		g_system->copyRectToScreen(ms.getPixels(), ms.pitch,
			0, 0, sw, sh);
		g_system->updateScreen();

		// Drain the LBUTTONDOWN that opened MoreInfo (first page only).
		if (isFirstShow) {
			isFirstShow = false;
			g_system->delayMillis(150);
			Common::Event drain;
			while (g_system->getEventManager()->pollEvent(drain)) {
				if (drain.type == Common::EVENT_QUIT ||
					drain.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					_nextScreen = kScreenInvalid;
					return true;
				}
			}
		}

		bool advance = false;
		bool prev = false;
		bool redraw = false;
		uint32 gizmoLastTick = g_system->getMillis();
		while (!back && !advance && !prev && !redraw && !shouldQuit()) {
			Common::Event e2;
			while (g_system->getEventManager()->pollEvent(e2)) {
				if (e2.type == Common::EVENT_QUIT ||
					e2.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					_nextScreen = kScreenInvalid;
					return true;
				}
				if (e2.type == Common::EVENT_KEYDOWN &&
					e2.kbd.keycode == Common::KEYCODE_ESCAPE) {
					openMainMenuDialog();
					redraw = true;
					break;
				}
				if (e2.type == Common::EVENT_LBUTTONDOWN) {
					const int mx = e2.mouse.x;
					const int my = e2.mouse.y;
					debugC(2, kDebugGfx,
						"MoreInfo click (%d,%d) hasMore=%d hasPrev=%d",
						mx, my, (int)hasMore, (int)hasPrev);
					if (pdaControlRect(this, kPdaNotebookRect).contains(mx, my)) {
						_nextScreen = kScreenNotebook;
						exitGallery = true;
						back = true;
						break;
					}
					if (pdaControlRect(this, kPdaAccuseRect).contains(mx, my)) {
						_nextScreen = kScreenAccuse;
						exitGallery = true;
						back = true;
						break;
					}
					if (pdaControlRect(this, kPdaPartnerFootMapRect).contains(mx, my)) {
						_nextScreen = kScreenMapAlt;
						exitGallery = true;
						back = true;
						break;
					}
					if (isLondon() &&
						pdaControlRect(this, kPdaHelpRect).contains(mx, my)) {
						_nextScreen = kScreenMapAlt;
						exitGallery = true;
						back = true;
						break;
					}
					if (pdaControlRect(this, kPdaHelpRect).contains(mx, my) ||
						pdaControlRect(this, kPdaHelp2Rect).contains(mx, my)) {
						setInteractiveMouseCursor(false);
						doInterfaceHelp(0);
						if (mac)
							setSitePalette(0);
						redraw = true;
						break;
					}
					if (pdaControlRect(this, kPdaGalleryRect).contains(mx, my)) {
						// Case 2: close MoreInfo.
						back = true;
						break;
					}
					if (pdaControlRect(this, kPdaPageNextRect).contains(mx, my)) {
						if (hasMore)
							advance = true;
						break;
					}
					if (pdaControlRect(this, kPdaPagePrevRect).contains(mx, my)) {
						if (hasPrev)
							prev = true;
						break;
					}
					if (pdaControlRect(this, kPdaPartnerHeadHintRect).contains(mx, my)) {
						// Case 3: _KDHelp.
						setInteractiveMouseCursor(false);
						doHelp();
						if (mac)
							setSitePalette(0);
						redraw = true;
						break;
					}
					// Case 8 SITE / non-button = no-op.
					break;
				}
				if (e2.type == Common::EVENT_KEYDOWN && hasMore &&
					(e2.kbd.keycode == Common::KEYCODE_RETURN ||
					 e2.kbd.keycode == Common::KEYCODE_KP_ENTER ||
					 e2.kbd.keycode == Common::KEYCODE_SPACE ||
					 e2.kbd.keycode == Common::KEYCODE_PAGEDOWN ||
					 e2.kbd.keycode == Common::KEYCODE_RIGHT)) {
					advance = true;
					break;
				}
				if (e2.type == Common::EVENT_KEYDOWN && hasPrev &&
					(e2.kbd.keycode == Common::KEYCODE_BACKSPACE ||
					 e2.kbd.keycode == Common::KEYCODE_LEFT ||
					 e2.kbd.keycode == Common::KEYCODE_PAGEUP)) {
					prev = true;
					break;
				}
			}

			const uint32 now = g_system->getMillis();
			if (now - gizmoLastTick >= kChooserCycleMillis) {
				gizmoLastTick = now;
				cycleChooserPalette();
			}
			g_system->updateScreen();
			g_system->delayMillis(20);
		}

		if (advance) {
			pageStack.push_back(pageStart);
			pageStart = pageEnd;
		} else if (prev && !pageStack.empty()) {
			pageStart = pageStack.back();
			pageStack.pop_back();
		}
	}

	return exitGallery;
}

void EEMEngine::drawGalleryFrame(const byte *gd, uint8 numSuspects,
								  Common::Array<Common::Rect> &slotRects,
								  Common::Array<int> &slotSuspect) {
	Picture galBg;
	const bool haveBg = _picsArchive.getPicture(0x3f, galBg);
	const bool mac = isMacintosh();
	const int sw = screenWidth();
	const int sh = screenHeight();

	Graphics::ManagedSurface scratch(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	if (haveBg)
		scratch.simpleBlitFrom(galBg.surface);

	blitPdaPartner(scratch, _aniArchive, _partner, kPdaGalleryPartner,
				   g_system->getMillis(), mac);

	const bool floppy = isFloppy();
	const bool compactGallery = floppy || _mystery.usesCompactMacData();
	const GallerySlot * const slots =
		mac ? kMacGallerySlots : (floppy ? kFloppyGallerySlots : kGallerySlots);
	for (uint i = 0; i < numSuspects && i < Mystery::kGalleryCap; i++) {
		slotRects[i] = Common::Rect();
		slotSuspect[i] = -1;

		const uint8 phys = _mystery._newOrder[i];
		if (phys >= 5)
			continue;
		const GallerySlot &s = slots[phys];

		const bool discovered = _mystery._inGallery[phys] != 0;
		if (discovered) {
			const byte *entry = compactGallery
				? _mystery.floppySuspectEntry(i)
				: gd + i * 0x46;
			if (!entry)
				continue;
			const uint16 picId = READ_LE_UINT16(entry);
			Picture portrait;
			if (picId == 0 ||
				!_picsArchive.getPicture(picId, portrait))
				continue;

			const int placeX = s.x;
			const int placeY = mac ? s.y : s.y + (0x48 - portrait.surface.h);
			const int w = MIN<int>(portrait.surface.w, sw - placeX);
			const int h = MIN<int>(portrait.surface.h, sh - placeY);
			if (w <= 0 || h <= 0)
				continue;
			if (mac)
				blitMacMaskedSurface(scratch.surfacePtr(), portrait,
									 placeX, placeY);
			else
				scratch.transBlitFrom(portrait.surface,
									  Common::Point(placeX, placeY),
									  (uint32)(byte)(portrait.flags >> 8));
			slotRects[i] = Common::Rect(placeX, placeY,
										 placeX + w, placeY + h);
			slotSuspect[i] = (int)i;
		} else {
			// Undiscovered placeholder — small framed "?" box.
			const int phW = mac ? 0x72 : 0x40;
			const int phH = mac ? 0x90 : 0x48;
			const int phX = s.x;
			const int phY = s.y;
			if (phX + phW <= sw && phY + phH <= sh) {
				scratch.fillRect(Common::Rect(phX, phY,
					phX + phW, phY + phH), 0x20);
				scratch.frameRect(Common::Rect(phX, phY,
					phX + phW, phY + phH), 0x5C);
				if (_font.isLoaded()) {
					_font.drawString(&scratch, "?",
						phX + phW / 2 - 3,
						phY + phH / 2 - 4, phW, 0x5C);
				}
			}
		}
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, sw, sh);
	g_system->updateScreen();
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
		setInteractiveMouseCursor(false);
		setSitePalette(isLondon() ? 0x3b : 0x24);

		const uint32 mapStartTick = g_system->getMillis();
		drawBigMapOverview(0);
		uint32 mapLastTick = mapStartTick;
		uint32 mapLastCycleTick = mapStartTick;

		const bool mac = isMacintosh();
		const Common::Rect bigMapWindowBase(0, 0, 247, 192);
		const Common::Rect kBigMapWindow =
			mac ? scaleRect(bigMapWindowBase) : bigMapWindowBase;
		const Common::Rect setupBtnBase = isFloppy()
			? Common::Rect(251, 3, 315, 42)
			: (isLondon() ? Common::Rect(252, 1, 315, 42)
						  : Common::Rect(252, 4, 315, 42));
		const Common::Rect kSetupBtnRect =
			mac ? scaleRect(setupBtnBase) : setupBtnBase;

		bool wantZoom = false;
		int zoomX = 0;
		int zoomY = 0;
		while (!shouldQuit()) {
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
					return;
				if (ev.type == Common::EVENT_KEYDOWN &&
					ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					openMainMenuDialog();
					continue;
				}
				if (ev.type == Common::EVENT_LBUTTONDOWN) {
					if (kSetupBtnRect.contains(ev.mouse.x, ev.mouse.y)) {
						_nextScreen = kScreenSetup;
						return;
					}

					if (kBigMapWindow.contains(ev.mouse.x, ev.mouse.y)) {
						if (mac) {
							zoomX = ev.mouse.x - kBigMapWindow.left;
							zoomY = ev.mouse.y - kBigMapWindow.top;
						} else {
							int sx = ev.mouse.x * 2;
							int sy = ev.mouse.y * 2;
							sx = (sx < 0x75) ? 0 : sx - 0x74;
							sy = (sy < 0x56) ? 0 : sy - 0x55;
							zoomX = sx;
							zoomY = sy;
						}
						wantZoom = true;
						break;
					}
				}
			}
			if (wantZoom)
				break;

			const uint32 now = g_system->getMillis();
			if (mac && isLondon() &&
				now - mapLastCycleTick >= kMacMapColorCycleDelayMs) {
				mapLastCycleTick = now;
				// Mac `_UpdateBigMap` rotates ColorTable entries 0xef..0xf2
				// and 0xfc..0xff. Redraw immediately so Mac endpoint art maps
				// its black pixels to the current black slot after the cycle.
				cyclePaletteRangeReverse(0xef, 0xf2);
				cyclePaletteRangeReverse(0xfc, 0xff);
				drawBigMapOverview(now - mapStartTick);
				mapLastTick = now;
			} else if (now - mapLastTick >= 100) {
				mapLastTick = now;
				drawBigMapOverview(now - mapStartTick);
				if (isLondon()) {
					if (!mac) {
						cyclePaletteRangeReverse(0xf4, 0xf9);
						cyclePaletteRangeReverse(0xfa, 0xff);
					}
				} else {
					cyclePaletteRangeReverse(0xf7, 0xfa);
					cyclePaletteRangeReverse(0xfb, 0xfe);
				}
			}
			g_system->updateScreen();
			g_system->delayMillis(10);
		}

		if (!wantZoom)
			return;

		uint16 mapW = 0;
		uint16 mapH = 0;
		Common::Array<byte> mapPixels;
		if (mac) {
			if (!loadMacBigMapPixels(mapPixels, mapW, mapH))
				return;
		} else {
			Common::File f;
			if (!f.open(Common::Path("BIGMAP.PIC"))) {
				warning("doBigMap: BIGMAP.PIC missing for detail view");
				return;
			}
			mapH = f.readUint16LE();
			mapW = f.readUint16LE();
			if (mapW == 0 || mapH == 0)
				return;
			mapPixels.resize((uint32)mapW * mapH);
			if (f.read(mapPixels.data(), mapPixels.size()) != mapPixels.size()) {
				warning("doBigMap: short read on BIGMAP.PIC for detail view");
				return;
			}
		}

		const int kMapWinW = mac ? scaleX(0xe9) : 0xe9; // 233
		const int kMapWinH = mac ? scaleY(0xab) : 0xab; // 171
		const int kMapWinX = mac ? scaleX(2) : 2;
		const int kMapWinY = mac ? scaleY(2) : 2;

		const int maxScrollX = MAX<int>(0, (int)mapW - kMapWinW);
		const int maxScrollY = MAX<int>(0, (int)mapH - kMapWinH);
		int scrollX;
		int scrollY;
		if (mac) {
			scrollX = zoomX * (int)mapW /
				MAX<int>(1, kBigMapWindow.width()) - kMapWinW / 2;
			scrollY = zoomY * (int)mapH /
				MAX<int>(1, kBigMapWindow.height()) - kMapWinH / 2;
		} else {
			scrollX = zoomX;
			scrollY = zoomY;
		}
		scrollX = MAX<int>(0, MIN<int>(maxScrollX, scrollX));
		scrollY = MAX<int>(0, MIN<int>(maxScrollY, scrollY));

		setSitePalette(isLondon() ? 0x3a : 0x23);

		const uint32 detailStartTick = g_system->getMillis();
		drawBigMapDetail(scrollX, scrollY, mapPixels, mapW, mapH, 0);
		uint32 detailLastTick = detailStartTick;
		uint32 detailLastCycleTick = detailStartTick;
		bool returnToOverview = false;

		const Common::Rect returnBase(252, 43, kScreenWidth, kScreenHeight);
		const Common::Rect kBigMapReturnRect =
			mac ? scaleRect(returnBase) : returnBase;
		const Common::Rect kArrowYUp =
			mac ? scaleRect(Common::Rect(237, 2, 247, 11))
				: Common::Rect(237, 2, 247, 11);
		const Common::Rect kArrowYDown =
			mac ? scaleRect(Common::Rect(237, 163, 247, 172))
				: Common::Rect(237, 163, 247, 172);
		const Common::Rect kArrowXLeft =
			mac ? scaleRect(Common::Rect(2, 175, 12, 185))
				: Common::Rect(2, 175, 12, 185);
		const Common::Rect kArrowXRight =
			mac ? scaleRect(Common::Rect(224, 175, 234, 185))
				: Common::Rect(224, 175, 234, 185);
		const Common::Rect xSliderBase = isLondon()
			? Common::Rect(15, 176, 220, 184)
			: Common::Rect(15, 175, 221, 185);
		const Common::Rect ySliderBase = isLondon()
			? Common::Rect(238, 16, 246, 158)
			: Common::Rect(237, 14, 247, 160);
		const Common::Rect kXSlider =
			mac ? scaleRect(xSliderBase) : xSliderBase;
		const Common::Rect kYSlider =
			mac ? scaleRect(ySliderBase) : ySliderBase;
		const Common::Rect detailSetupBase = isFloppy()
			? Common::Rect(251, 3, 315, 42)
			: (isLondon() ? Common::Rect(251, 3, 315, 42)
						  : Common::Rect(252, 4, 315, 42));
		const Common::Rect kDetailSetupBtn =
			mac ? scaleRect(detailSetupBase) : detailSetupBase;
		const int baseArrowStep = isLondon() ? 8 : 16;
		const int kArrowStepX = mac ? scaleX(baseArrowStep) : baseArrowStep;
		const int kArrowStepY = mac ? scaleY(baseArrowStep) : baseArrowStep;
		const int kSliderRange = maxScrollX;
		const int kSliderRangeY = maxScrollY;
		const Common::Point detailMouse =
			g_system->getEventManager()->getMousePos();
		setInteractiveMouseCursor(
			kBigMapReturnRect.contains(detailMouse.x, detailMouse.y) ||
			kDetailSetupBtn.contains(detailMouse.x, detailMouse.y));

		while (!shouldQuit() && !returnToOverview) {
			Common::Event ev;
			bool dirty = false;
			bool cycleTick = false;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					setInteractiveMouseCursor(false);
					return;
				}
				if (ev.type == Common::EVENT_KEYDOWN) {
					if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
						openMainMenuDialog();
						dirty = true;
						continue;
					}
					if (ev.kbd.keycode == Common::KEYCODE_LEFT) {
						scrollX = MAX<int>(0, scrollX - kArrowStepX);
						dirty = true;
					} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT) {
						scrollX = MIN<int>(maxScrollX, scrollX + kArrowStepX);
						dirty = true;
					} else if (ev.kbd.keycode == Common::KEYCODE_UP) {
						scrollY = MAX<int>(0, scrollY - kArrowStepY);
						dirty = true;
					} else if (ev.kbd.keycode == Common::KEYCODE_DOWN) {
						scrollY = MIN<int>(maxScrollY, scrollY + kArrowStepY);
						dirty = true;
					}
				}
				if (ev.type == Common::EVENT_MOUSEMOVE)
					setInteractiveMouseCursor(
						kBigMapReturnRect.contains(ev.mouse.x, ev.mouse.y) ||
						kDetailSetupBtn.contains(ev.mouse.x, ev.mouse.y));
				if (ev.type == Common::EVENT_LBUTTONDOWN) {
					setInteractiveMouseCursor(
						kBigMapReturnRect.contains(ev.mouse.x, ev.mouse.y) ||
						kDetailSetupBtn.contains(ev.mouse.x, ev.mouse.y));
					if (kDetailSetupBtn.contains(ev.mouse.x, ev.mouse.y)) {
						_nextScreen = kScreenSetup;
						setInteractiveMouseCursor(false);
						return;
					}
					if (kBigMapReturnRect.contains(ev.mouse.x, ev.mouse.y)) {
						returnToOverview = true;
						break;
					} else if (kArrowYUp.contains(ev.mouse.x, ev.mouse.y)) {
						scrollY = MAX<int>(0, scrollY - kArrowStepY);
						dirty = true;
					} else if (kArrowYDown.contains(ev.mouse.x, ev.mouse.y)) {
						scrollY = MIN<int>(MAX<int>(0, kSliderRangeY),
							scrollY + kArrowStepY);
						dirty = true;
					} else if (kArrowXLeft.contains(ev.mouse.x, ev.mouse.y)) {
						scrollX = MAX<int>(0, scrollX - kArrowStepX);
						dirty = true;
					} else if (kArrowXRight.contains(ev.mouse.x, ev.mouse.y)) {
						scrollX = MIN<int>(MAX<int>(0, kSliderRange),
							scrollX + kArrowStepX);
						dirty = true;
					} else if (kXSlider.contains(ev.mouse.x, ev.mouse.y)) {
						if (kSliderRange > 0) {
							const int t = ev.mouse.x - kXSlider.left;
							const int tw = kXSlider.width();
							scrollX = MAX<int>(0, MIN<int>(kSliderRange,
								t * kSliderRange / MAX<int>(1, tw)));
							dirty = true;
						}
					} else if (kYSlider.contains(ev.mouse.x, ev.mouse.y)) {
						if (kSliderRangeY > 0) {
							const int t = ev.mouse.y - kYSlider.top;
							const int th = kYSlider.height();
							scrollY = MAX<int>(0, MIN<int>(kSliderRangeY,
								t * kSliderRangeY / MAX<int>(1, th)));
							dirty = true;
						}
					} else if (ev.mouse.x >= kMapWinX &&
							   ev.mouse.x < kMapWinX + kMapWinW &&
							   ev.mouse.y >= kMapWinY &&
							   ev.mouse.y < kMapWinY + kMapWinH) {
						// Per-site bbox from `_StampButtons` (SmallMap +8/+0xa).
						const bool fmap = _mystery.isLoaded() && isFloppy();
						struct DetailMapHit {
							uint site;
							Common::Rect rect;
						};
						Common::Array<DetailMapHit> hits;
						for (uint i = 0; i < _mystery.numSites(); i++) {
							// On-map flag alone, matching `_SearchMapButtons`.
							if (!_mystery._onSites[i])
								continue;
							const byte *entry = _mystery.mapEntry(i);
							if (!entry)
								continue;
							BigMapEntryInfo info;
							if (!readBigMapEntryInfo(entry, fmap,
													  mac && _mystery.usesCompactMacData(),
													  info))
								continue;
							Picture button;
							int bw = 16;
							int bh = 16;
							if (_buttonArchive.loadEntry(info.buttonId, button)) {
								bw = button.surface.w;
								bh = button.surface.h;
							}
							const int sx = (int)info.detailX - scrollX + kMapWinX;
							const int sy = (int)info.detailY - scrollY + kMapWinY;
							const Common::Rect r(sx, sy, sx + bw, sy + bh);
							if (r.intersects(Common::Rect(kMapWinX, kMapWinY,
									kMapWinX + kMapWinW, kMapWinY + kMapWinH))) {
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
							if (hits[i].rect.contains(ev.mouse.x, ev.mouse.y)) {
								_mystery._lastSite = _mystery._siteNumber;
								_mystery._siteNumber = (uint16)hits[i].site;
								setInteractiveMouseCursor(false);
								return;
							}
						}
					}
				}
			}
			if (returnToOverview)
				break;

			const uint32 now = g_system->getMillis();
			if (now - detailLastTick >= 100) {
				detailLastTick = now;
				dirty = true;
			}
			if (isLondon()) {
				const uint32 cycleDelay = mac ? kMacMapColorCycleDelayMs : 100;
				if (now - detailLastCycleTick >= cycleDelay) {
					detailLastCycleTick = now;
					cycleTick = true;
					dirty = true;
				}
			}
			if (cycleTick && isLondon()) {
				if (mac) {
					cyclePaletteRangeReverse(0xe9, 0xeb);
					cyclePaletteRangeReverse(0xec, 0xef);
					cyclePaletteRangeReverse(0xf0, 0xf2);
				} else {
					cyclePaletteRangeReverse(0xee, 0xf2);
					cyclePaletteRangeReverse(0xea, 0xed);
				}
			}
			if (dirty)
				drawBigMapDetail(scrollX, scrollY, mapPixels, mapW, mapH,
					now - detailStartTick);
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
		if (!returnToOverview)
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
	MacSpritePaletteMap macPaletteMap = {0x00, 0xFF};
	if (mac)
		macPaletteMap = getMacSpritePaletteMap();

	Graphics::ManagedSurface base(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	byte palette[768] = {};
	const bool haveVideo =
		decodeLondonApproachFirstFrame(data.videoId, base, palette, mac);
	if (!haveVideo)
		base.clear();

	Picture buttonPics[ARRAYSIZE(kLondonApproachButtons)];
	bool haveButtons[ARRAYSIZE(kLondonApproachButtons)] = {};
	for (uint i = 0; i < ARRAYSIZE(kLondonApproachButtons); i++)
		haveButtons[i] = _picsArchive.getPicture(
			kLondonApproachButtons[i].picId, buttonPics[i]);

	auto buttonRect = [&](uint idx) {
		const Common::Rect r = kLondonApproachButtons[idx].rect();
		return mac ? scaleRect(r) : r;
	};
	auto buttonPoint = [&](uint idx) {
		const LondonApproachButton &b = kLondonApproachButtons[idx];
		return Common::Point(mac ? scaleX(b.x) : b.x,
							 mac ? scaleY(b.y) : b.y);
	};

	auto drawButtonFallback = [&](Graphics::ManagedSurface &dst, uint idx) {
		static const char *const kLabels[4] = { "OK", "PLAY", ">", "<" };
		const Common::Rect r = buttonRect(idx);
		dst.fillRect(r, 0);
		_font.drawString(&dst, kLabels[idx], r.left + 1, r.top + 5,
						 r.width(), 0x0f);
	};

	auto drawScreen = [&](uint page) {
		Graphics::ManagedSurface scratch(sw, sh,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(base);

		const Common::Rect textRect =
			data.textRect.findIntersectingRect(Common::Rect(sw, sh));
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
									 mac ? macPaletteMap.black : 1);
				}
			}
		}

		for (uint i = 0; i < ARRAYSIZE(kLondonApproachButtons); i++) {
			if (haveButtons[i]) {
				const Common::Point p = buttonPoint(i);
				if (mac)
					blitMacMaskedSurface(scratch.surfacePtr(),
										 buttonPics[i], p.x, p.y, false,
										 macPaletteMap);
				else
					scratch.transBlitFrom(buttonPics[i].surface, p,
										  (uint32)(byte)(buttonPics[i].flags >> 8));
			} else {
				drawButtonFallback(scratch, i);
			}
		}

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, sw, sh);
		g_system->updateScreen();
	};

	auto playVideo = [&]() {
		if (mac) {
			Video::FlicDecoder flic;
			Common::String name;
			if (!openLondonApproachFlic(data.videoId, flic, name)) {
				warning("London approach: cannot open FLC video %u",
						data.videoId);
				return;
			}

			flic.start();
			(void)flic.decodeNextFrame(); // first frame is already the background
			const int videoBandH = scaleY(0x82);
			while (!shouldQuit() && !flic.endOfVideo()) {
				const Graphics::Surface *frame = flic.decodeNextFrame();
				if (!frame)
					break;
				const int copyW = MIN<int>(frame->w - (int)data.videoX,
										   sw - (int)data.videoX);
				const int copyH = MIN<int>(videoBandH,
					MIN<int>(frame->h - (int)data.videoY,
							 sh - (int)data.videoY));
				if (copyW <= 0 || copyH <= 0)
					break;
				g_system->copyRectToScreen(
					(const byte *)frame->getBasePtr(data.videoX, data.videoY),
					frame->pitch, data.videoX, data.videoY, copyW, copyH);
				if (flic.hasDirtyPalette()) {
					const byte *fpal = flic.getPalette();
					if (fpal)
						g_system->getPaletteManager()->setPalette(fpal, 0, 256);
				}
				g_system->updateScreen();

				const uint32 start = g_system->getMillis();
				bool skip = false;
				while (g_system->getMillis() - start < 120 && !skip) {
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
		// into the upper video area, leaving text/buttons untouched.
		(void)anm.nextFrame();
		const int copyW = MIN<int>(anm.width(), sw - (int)data.videoX);
		const int copyH = MIN<int>(0x82, MIN<int>(anm.height(),
			sh - (int)data.videoY));
		if (copyW <= 0 || copyH <= 0)
			return;

		while (!shouldQuit()) {
			const byte *frame = anm.nextFrame();
			if (!frame)
				break;
			g_system->copyRectToScreen(
				frame + data.videoY * anm.width() + data.videoX,
				anm.width(), data.videoX, data.videoY, copyW, copyH);
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
				for (uint i = 0; i < ARRAYSIZE(kLondonApproachButtons); i++) {
					if (buttonRect(i).contains(ev.mouse.x, ev.mouse.y)) {
						overButton = true;
						break;
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
					playVideo();
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
				if (buttonRect(0).contains(ev.mouse.x, ev.mouse.y)) {
					done = true;
					break;
				}
				if (buttonRect(1).contains(ev.mouse.x, ev.mouse.y)) {
					playVideo();
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
		if (haveVideo) {
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

// `_GetKDTextBalloon @ 1df2:0105`
uint16 EEMEngine::getKDTextBalloon(byte firstChar) const {
	if (firstChar < '0' || firstChar > '9')
		return 0x17;
	return kDigitBalloons[firstChar - '0'];
}

Common::String EEMEngine::accuseNoteText(uint clueId,
										 const AccuseNotesCtx &ctx) const {
	if (ctx.floppyNote) {
		const uint16 textOff = READ_LE_UINT16(ctx.ni + clueId * 7);
		if (textOff == 0 || textOff >= _mystery.dataSize() ||
			!ctx.bufBaseNotes)
			return Common::String();
		return parseString(
			(const char *)(ctx.bufBaseNotes + textOff),
			_playerName, _partner);
	}
	if (isMacintosh() && ctx.bufBaseNotes) {
		const uint16 textOff = READ_LE_UINT16(ctx.ni + clueId * 8);
		if (textOff == 0 || textOff >= _mystery.dataSize())
			return Common::String();
		return parseString(
			(const char *)(ctx.bufBaseNotes + textOff),
			_playerName, _partner);
	}
	const uint stride = isLondon() ? 2 : 4;
	const uint16 textOff = READ_LE_UINT16(ctx.ni + clueId * stride);
	return parseString(_mystery.textAt(textOff),
					   _playerName, _partner);
}

void EEMEngine::accuseRebuildPagination(const AccuseNotesCtx &ctx) {
	*ctx.numPages = 1;
	ctx.pageBreaks[0] = 0;
	const int lineH = _font.getFontHeight();
	int y = ctx.rectY;
	const Common::Array<uint> &found = *ctx.found;
	for (uint i = 0; i < found.size(); i++) {
		const uint clueId = found[i];
		Common::String txt;
		if (clueId < ctx.niCount)
			txt = accuseNoteText(clueId, ctx);
		Common::Array<Common::String> wrapped;
		_font.wordWrapText(txt, ctx.rectW, wrapped);
		const int h = (int)wrapped.size() * lineH;
		if (y + h + 7 > ctx.rectY + ctx.rectH) {
			if (*ctx.numPages < ctx.pageBreaksCap) {
				ctx.pageBreaks[(*ctx.numPages)++] = (int)i;
				y = ctx.rectY;
			}
		}
		y += h + 7;
	}
	if (*ctx.page >= *ctx.numPages)
		*ctx.page = *ctx.numPages - 1;
	if (*ctx.page < 0)
		*ctx.page = 0;
}

void EEMEngine::accuseDrawScreen(const AccuseNotesCtx &ctx) {
	const bool mac = isMacintosh();
	const int sw = screenWidth();
	const int sh = screenHeight();

	Graphics::ManagedSurface scratch(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (ctx.haveBg)
		scratch.simpleBlitFrom(ctx.accuseBg->surface);

	blitPdaPartner(scratch, _aniArchive, _partner, kPdaGalleryPartner,
				   g_system->getMillis(), mac);

	Common::Array<Common::Rect> &slotRects = *ctx.slotRects;
	Common::Array<uint> &slotClues = *ctx.slotClues;
	const Common::Array<uint> &found = *ctx.found;
	slotRects.clear();
	slotClues.clear();
	const int lineH = _font.getFontHeight();
	const int startIdx = ctx.pageBreaks[*ctx.page];
	const int endIdx   = (*ctx.page + 1 < *ctx.numPages)
		? ctx.pageBreaks[*ctx.page + 1]
		: (int)found.size();
	int y = ctx.rectY;
	uint selectedCount = 0;
	for (uint i = 0; i < found.size(); i++) {
		if (_mystery._noteSelected[found[i]])
			selectedCount++;
	}
	for (int i = startIdx; i < endIdx; i++) {
		const uint clueId = found[i];
		Common::String txt;
		if (clueId < ctx.niCount)
			txt = accuseNoteText(clueId, ctx);
		if (txt.empty())
			txt = Common::String::format(
				isSpanish() ? "nota %u" : "clue %u", clueId);
		Common::Array<Common::String> wrapped;
		_font.wordWrapText(txt, ctx.rectW, wrapped);
		const int h = (int)wrapped.size() * lineH;
		const byte color = _mystery._noteSelected[clueId] ? 0x3c : 0x01;
		for (uint li = 0; li < wrapped.size(); li++) {
			_font.drawString(&scratch, wrapped[li], ctx.rectX,
							 y + (int)li * lineH, ctx.rectW, color);
		}
		slotRects.push_back(Common::Rect(ctx.rectX, y,
										  ctx.rectX + ctx.rectW, y + h));
		slotClues.push_back(clueId);
		y += h + 7;
	}

	const uint remaining = (selectedCount < ctx.expected)
		? ctx.expected - selectedCount
		: 0;
	// Spanish floppy uses "nota/notas".
	const char *clueWord = isSpanish()
		? (remaining == 1 ? "nota" : "notas")
		: (remaining == 1 ? "clue" : "clues");
	const Common::String counter =
		Common::String::format("%u %s", remaining, clueWord);
	_font.drawString(&scratch, counter, scaleX(209), scaleY(11),
					 scaleX(100), 0x0F);

	if (*ctx.numPages > 1) {
		_font.drawString(&scratch,
			Common::String::format("p%d/%d", *ctx.page + 1, *ctx.numPages),
			ctx.rectX, scaleY(11), scaleX(60), 0x0F);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, sw, sh);
	g_system->updateScreen();
}

bool EEMEngine::doAccuseNotes() {
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return false;
	const byte *ni = _mystery.noteIndex();
	// London's NoteIndex is 2-byte entries, so its real clue count is
	// section size / 2; noteIndexCount() assumes EEM1's 4-byte stride and
	// undercounts by half, which would drop the upper clues (e.g. answer
	// clues 13/16 in the training case) from the selectable accuse list.
	const uint16 niCount = isLondon()
		? (uint16)(_mystery.noteSectionSize() / 2)
		: _mystery.noteIndexCount();
	if (!ni)
		return false;

	Picture accuseBg;
	const bool haveBg = _picsArchive.getPicture(0x1a7, accuseBg);

	memset(_mystery._noteSelected, 0, sizeof(_mystery._noteSelected));

	const uint expected = isLondon()
		? 5
		: ((_chainStage >= 1 && _chainStage <= 3) ? (uint)(6 - _chainStage)
												  : 5);

	Common::Array<uint> found;
	for (uint i = 0; i < niCount && i < Mystery::kCluesFoundCap; i++) {
		const bool hasText = isLondon()
			? (i < niCount)
			: _mystery.noteHasNotebookText(i);
		if (_mystery._cluesFound[i] && hasText)
			found.push_back(i);
	}

	const Common::Rect noteRectBase(79, 27, 304, 159);
	const Common::Rect noteRect =
		isMacintosh() ? scaleRect(noteRectBase) : noteRectBase;
	const int rectX = noteRect.left;
	const int rectY = noteRect.top;
	const int rectW = noteRect.width();
	const int rectH = noteRect.height();

	const Common::Rect btnSolve =
		pdaControlRect(this, kPdaAccuseRect); // [4]
	const Common::Rect btnPageNext =
		pdaControlRect(this, kPdaPageNextRect); // [5]
	const Common::Rect btnPagePrev =
		pdaControlRect(this, kPdaPagePrevRect); // [6]
	const Common::Rect btnPartner =
		pdaControlRect(this, kPdaPartnerHeadHintRect); // [3]

	// Per-page slot rects + their clue IDs (for click hit-testing).
	Common::Array<Common::Rect> slotRects;
	Common::Array<uint> slotClues;

	int page = 0;
	int pageBreaks[16];
	int numPages = 1;
	pageBreaks[0] = 0;

	const bool floppyNote = isFloppy();
	const byte *bufBaseNotes = _mystery.blobAt(0);

	AccuseNotesCtx ctx;
	ctx.ni            = ni;
	ctx.niCount       = niCount;
	ctx.floppyNote    = floppyNote;
	ctx.bufBaseNotes  = bufBaseNotes;
	ctx.found         = &found;
	ctx.rectX         = rectX;
	ctx.rectY         = rectY;
	ctx.rectW         = rectW;
	ctx.rectH         = rectH;
	ctx.expected      = expected;
	ctx.haveBg        = haveBg;
	ctx.accuseBg      = &accuseBg;
	ctx.slotRects     = &slotRects;
	ctx.slotClues     = &slotClues;
	ctx.pageBreaks    = pageBreaks;
	ctx.pageBreaksCap = (int)ARRAYSIZE(pageBreaks);
	ctx.numPages      = &numPages;
	ctx.page          = &page;

	accuseRebuildPagination(ctx);
	accuseDrawScreen(ctx);
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	setInteractiveMouseCursor(notebookButtonAt(this, mouse.x, mouse.y) ||
							  pdaControlRect(this, kPdaNotebookRect)
								  .contains(mouse.x, mouse.y) ||
							  rectListContains(slotRects, mouse.x, mouse.y));

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_nextScreen = kScreenInvalid;
				return false;
			}
			if (ev.type == Common::EVENT_MOUSEMOVE) {
				setInteractiveMouseCursor(
					notebookButtonAt(this, ev.mouse.x, ev.mouse.y) ||
					pdaControlRect(this, kPdaNotebookRect)
						.contains(ev.mouse.x, ev.mouse.y) ||
					rectListContains(slotRects, ev.mouse.x, ev.mouse.y));
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					openMainMenuDialog();
					dirty = true;
					continue;
				}
				if (ev.kbd.keycode == Common::KEYCODE_LEFT &&
					page > 0) {
					page--;
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT &&
						   page + 1 < numPages) {
					page++;
					dirty = true;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				const int mx = ev.mouse.x;
				const int my = ev.mouse.y;
				if (pdaControlRect(this, kPdaSiteRect).contains(mx, my)) {
					_nextScreen = kScreenSite;
					return false;
				}
				if (pdaControlRect(this, kPdaPartnerFootMapRect)
						.contains(mx, my)) {
					_nextScreen = kScreenMapAlt;
					return false;
				}
				if (pdaControlRect(this, kPdaNotebookRect).contains(mx, my)) {
					_nextScreen = kScreenNotebook;
					return false;
				}
				if (pdaControlRect(this, kPdaGalleryRect).contains(mx, my)) {
					_nextScreen = kScreenGallery;
					return false;
				}
				if (isLondon() &&
					pdaControlRect(this, kPdaHelpRect).contains(mx, my)) {
					_nextScreen = kScreenMapAlt;
					return false;
				}
				if (pdaControlRect(this, kPdaHelpRect).contains(mx, my) ||
					pdaControlRect(this, kPdaHelp2Rect).contains(mx, my)) {
					setInteractiveMouseCursor(false);
					doInterfaceHelp(0);
					if (isMacintosh())
						setSitePalette(0);
					dirty = true;
					continue;
				}
				if (btnPageNext.contains(mx, my)) {
					if (page + 1 < numPages) {
						page++;
						dirty = true;
					}
					continue;
				}
				if (btnPagePrev.contains(mx, my)) {
					if (page > 0) {
						page--;
						dirty = true;
					}
					continue;
				}
				if (btnPartner.contains(mx, my)) {
					doHelp();
					if (isMacintosh())
						setSitePalette(0);
					dirty = true;
					continue;
				}
				if (btnSolve.contains(mx, my)) {
					uint selected = 0;
					for (uint i = 0; i < found.size(); i++) {
						if (_mystery._noteSelected[found[i]])
							selected++;
					}
					if (selected == expected) {
						return true;
					}
					continue;
				}
				// Toggle clue under cursor.
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotRects[i].contains(mx, my)) {
						const uint clueId = slotClues[i];
						if (!_mystery._noteSelected[clueId]) {
							uint selected = 0;
							for (uint j = 0; j < found.size(); j++) {
								if (_mystery._noteSelected[found[j]])
									selected++;
							}
							if (selected >= expected)
								break;
						}
						_mystery._noteSelected[clueId] =
							_mystery._noteSelected[clueId] ? 0 : 1;
						dirty = true;
						break;
					}
				}
			}
		}
		if (dirty)
			accuseDrawScreen(ctx);
		static uint32 sLastTick = 0;
		const uint32 now = g_system->getMillis();
		if (now - sLastTick >= 100) {
			sLastTick = now;
			accuseDrawScreen(ctx);
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
	return false;
}

void EEMEngine::drawKDBalloonOverCurrentScreen(Common::String text) {
	const bool mac = isMacintosh();
	const int sw = screenWidth();
	const int sh = screenHeight();

	Graphics::ManagedSurface ms(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	ms.clear();
	Graphics::Surface *cur = g_system->lockScreen();
	if (cur) {
		ms.simpleBlitFrom(*cur);
		g_system->unlockScreen();
	}

	const byte firstChar = text.empty() ? (byte)0 : (byte)text[0];
	uint16 bubNum = getKDTextBalloon(firstChar);
	if (firstChar >= '0' && firstChar <= '9')
		text.deleteChar(0);
	bubNum = fitBalloonToText(bubNum, text);

	Picture balloon;
	const bool haveBalloon =
		_balloonArchive.size() > (bubNum & 0x7F) &&
		_balloonArchive.loadEntry(bubNum & 0x7F, balloon);

	const int balloonX = mac ? scaleX(0x21) : 0x21;
	const int topBandH = mac ? scaleY(0x50) : 0x50;
	const int centeredMaxH = mac ? scaleY(0x4e) : 0x4e;
	int balloonY = mac ? scaleY(1) : 1;
	if (haveBalloon && balloon.surface.h < centeredMaxH)
		balloonY = (topBandH - balloon.surface.h) / 2;

	MacSpritePaletteMap macPaletteMap = {0x00, 0xFF};
	if (mac)
		macPaletteMap = getMacSpritePaletteMap();

	if (haveBalloon) {
		if (mac) {
			blitMacMaskedSurface(ms.surfacePtr(), balloon, balloonX,
								 balloonY, false, macPaletteMap);
		} else {
			ms.transBlitFrom(balloon.surface,
							 Common::Point(balloonX, balloonY),
							 (uint32)(byte)(balloon.flags >> 8));
		}
	}

	uint16 tx = 5, ty = 4, tw = 155;
	getBalloonInsets(bubNum, tx, ty, tw);
	const EEMFont &dialogFont =
		(mac && _dialogFont.isLoaded()) ? _dialogFont : _font;
	if (dialogFont.isLoaded()) {
		const byte textColor =
			mac ? macPaletteMap.black : (haveBalloon ? 0 : 0xF);
		dialogFont.drawWordWrapped(&ms, balloonX + tx, balloonY + ty,
								   tw, text, textColor);
	}

	copyToScreen(ms);
}

void EEMEngine::doAccuse() {
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	if (isFloppy()) {
		doAccuseFloppy();
		return;
	}

	const byte *entryKdIdx = _mystery.kdTextIndex();
	if (!entryKdIdx)
		return;
	// Readiness tier (0 nothing yet .. 3 ready-to-accuse). EEM1 `_AccuseEntry
	// @ 1df2:0ff8` tiers on the top-5 found-clue score; EEM2 `_AccuseEntry @
	// 1ea1:115c` tiers on minCluesRemaining() (jumptable @ 1ea1:1333: 0 ->
	// ready, 1 -> almost, 2..4 -> keep looking, 5 -> nothing yet — only the 0
	// case sets the can-accuse flag). Both variants then pick the same four KD
	// lines, so map onto one shared tier.
	int readyTier;
	if (isLondon()) {
		const int minRem = _mystery.minCluesRemaining();
		readyTier = (minRem >= (int)Mystery::kChainLen) ? 0
				  : (minRem >= 2)                       ? 1
				  : (minRem == 1)                       ? 2
														: 3;
	} else {
		const int foundPoints = _mystery.foundPoints();
		readyTier = (foundPoints == 0)    ? 0
				  : (foundPoints < 0x32)  ? 1
				  : (foundPoints < 0x65)  ? 2
											: 3;
	}

	uint entryKDSpeak = 0;
	uint16 entryTextOff = 0xFFFF;
	uint16 entryVoiceOverride = 0xFFFF;
	bool canAccuse = false;
	Common::String entryText;
	switch (readyTier) {
	case 0:
		entryKDSpeak = 9;
		entryText = "3We're not ready to solve this mystery yet.  "
					"Let's keep investigating until we have some more solid "
					"evidence to make our case!";
		// Practice mystery M0 ships the matching ZeroText takes as the
		// final two SDB entries, but its KD digital table points kdspeak 9
		// at earlier hint clips. Use the otherwise unreferenced pair. EEM1
		// SDB indices, so EEM1 only.
		if (!isLondon() && _mystery.number() == 0)
			entryVoiceOverride = (_partner == kPartnerJake) ? 105 : 104;
		break;
	case 1:
		entryKDSpeak = 0;
		entryTextOff = READ_LE_UINT16(entryKdIdx + 0);
		break;
	case 2:
		entryKDSpeak = 1;
		entryTextOff = READ_LE_UINT16(entryKdIdx + 2);
		break;
	default:
		entryKDSpeak = 2;
		entryTextOff = READ_LE_UINT16(entryKdIdx + 4);
		canAccuse = true;
		break;
	}
	if (entryText.empty() && entryTextOff != 0xFFFF) {
		entryText = parseString(_mystery.textAt(entryTextOff),
								_playerName, _partner);
	}
	if (!entryText.empty()) {
		drawKDBalloonOverCurrentScreen(entryText);
		if (_audio) {
			if (entryVoiceOverride != 0xFFFF)
				_audio->spoolSound(entryVoiceOverride);
			else
				_audio->sayKDDigital(entryKdIdx, entryKDSpeak, _partner);
		}
		waitForInput(60000);
	}
	if (!canAccuse) {
		_nextScreen = _lastScreen != kScreenInvalid
						? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	// `_DoAccuse @ 1df2:0bdd` + `_DoAccuseGallery @ 1df2:0a31`:
	//   1. Accuse-notes (PIC 0x1A7) — pick `6 - chainStage` clues (London: 5).
	//      Pass `_SolvedCheck` → gallery; fail → hint + return.
	//   2. KD intro balloon (`KDTextIndex[+8]` + `_SayKDDigital(4)`).
	//   3. PIC 0x3f + `_DrawGallery` portraits at 5 slots (29be:0x116).
	//   4. Click portrait → `_WITCH(picked)` → guilty/alibi.
	const uint8 num = _mystery.numSuspects();
	if (num == 0)
		return;

	const byte *gd = _mystery.galleryData();

	if (isLondon() && _music && _voiceOn)
		_music->playMus(4, /* loop= */ true);

	// `_DoAccuse @ 1df2:0c11` outer loop; ESC → NextScreen=3.
	if (!doAccuseNotes()) {
		if (_nextScreen == kScreenAccuse) {
			_nextScreen = _lastScreen != kScreenInvalid
							? (ScreenId)_lastScreen : kScreenSite;
		}
		return;
	}

	// `_DoAccuse @ 1df2:0c75` (EEM1) / `1ea1:0c75` (EEM2) — `_SolvedCheck` gate.
	// London matches the selected notes against the answer sets; EEM1/floppy
	// sum clue points.
	const bool accuseSolved =
		isLondon() ? _mystery.londonSolved() : _mystery.solvedCheck();
	if (!accuseSolved) {
		const byte *kdIdx = _mystery.kdTextIndex();
		const int16 hintOff = kdIdx
			? (int16)READ_LE_UINT16(kdIdx + 6)
			: -1;
		Common::String hint;
		if (hintOff != -1)
			hint = parseString(_mystery.textAt((uint16)hintOff),
							   _playerName, _partner);
		if (hint.empty()) {
			// Fallback if `KDTextIndex[+6]` missing. Spanish floppy
			// has only one "Necesitamos buscar pistas" string.
			if (isSpanish()) {
				hint = "Necesitamos buscar pistas antes de resolver "
					   "el misterio. Investiguemos un poco mas!";
			} else {
				// "nothing yet" vs "almost": London has no points, so judge by
				// whether any answer clue has been found (minCluesRemaining).
				const bool nothingYet = isLondon()
					? _mystery.minCluesRemaining() >= (int)Mystery::kChainLen
					: _mystery.selectedPoints() == 0;
				hint = nothingYet
					? "We're not ready to solve this mystery yet. "
					  "Let's keep investigating until we have some "
					  "more solid evidence."
					: "We don't have quite enough evidence yet. "
					  "Let's review our notes and find a few more "
					  "clues before we accuse anyone.";
			}
		}

		drawKDBalloonOverCurrentScreen(hint);

		if (_audio && kdIdx)
			_audio->sayKDDigital(kdIdx, 3, _partner);

		waitForInput(20000);
		_nextScreen = _lastScreen != kScreenInvalid
						? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	Common::Array<Common::Rect> slotRects;
	Common::Array<int> slotSuspect;
	slotRects.resize(num);
	slotSuspect.resize(num);
	for (uint i = 0; i < num; i++)
		slotSuspect[i] = -1;

	int highlighted = 0;

	const byte *kdIdx = _mystery.kdTextIndex();
	if (kdIdx) {
		const int16 textOff = (int16)READ_LE_UINT16(kdIdx + 8);
		if (textOff != -1) {
			const char *raw = _mystery.textAt((uint16)textOff);
			Common::String hint =
				parseString(raw ? raw : "", _playerName, _partner);
			if (!hint.empty()) {
				// Render gallery first so the snapshot includes partner.
				drawAccuseGallery(num, gd, /* highlighted= */ -1,
								  slotRects, slotSuspect);
				drawKDBalloonOverCurrentScreen(hint);
				if (_audio)
					_audio->sayKDDigital(kdIdx, 4, _partner);
				waitForInput(8000);
			}
		}
	}

	if (isLondon() && _music && _voiceOn)
		_music->playMus(33, /* loop= */ true);

	// Wrap past empty slots (matches original DI advance).
	if (slotRects[highlighted].isEmpty())
		highlighted = nextLiveSlot(slotRects, highlighted, +1);

	drawAccuseGallery(num, gd, highlighted, slotRects, slotSuspect);

	int picked = -1;
	uint32 lastTick = g_system->getMillis();
	bool dirty = false;
	while (picked < 0 && !shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					openMainMenuDialog();
					dirty = true;
					continue;
				case Common::KEYCODE_TAB:
				case Common::KEYCODE_RIGHT:
					highlighted = nextLiveSlot(slotRects, highlighted, +1);
					dirty = true;
					break;
				case Common::KEYCODE_LEFT:
					highlighted = nextLiveSlot(slotRects, highlighted, -1);
					dirty = true;
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					if (highlighted >= 0 &&
						highlighted < (int)slotRects.size() &&
						!slotRects[highlighted].isEmpty()) {
						picked = highlighted;
					}
					break;
				default: {
					const int k = (int)ev.kbd.keycode;
					if (k >= Common::KEYCODE_1 && k <= Common::KEYCODE_9) {
						const int idx = k - Common::KEYCODE_1;
						if (idx < num &&
							!slotRects[idx].isEmpty())
							picked = idx;
					}
					break;
				}
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotSuspect[i] < 0)
						continue;
					if (slotRects[i].contains(ev.mouse.x, ev.mouse.y)) {
						picked = (int)i;
						break;
					}
				}
			}
		}
		const uint32 now = g_system->getMillis();
		if (dirty || now - lastTick >= 100) {
			drawAccuseGallery(num, gd, highlighted, slotRects, slotSuspect);
			lastTick = now;
			dirty = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	if (picked < 0)
		return;

	const int points          = _mystery.selectedPoints();
	const bool pickedGuilty   = _mystery.isGuilty((uint)picked);
	const bool guessedRight   = pickedGuilty;
	debugC(1, kDebugScript,
		   "doAccuse: picked=%d selectedPts=%d guilty=%s -> %s",
		   picked, points,
		   pickedGuilty ? "yes" : "no",
		   guessedRight ? "correct" : "wrong");

	// `_DisplayAlibi @ 1df2:0145`:
	//   1. MIDI 6 loser sting (gallery still visible).
	//   2. PIC 0x3e + suspect balloon + portrait at (0x82, py).
	//      bindx = digit prefix (else 2). bindx<8 centres balloon,
	//      bindx>=8 pins at x=0x21.
	//   3. `_SpoolSound(talk-1)` where talk = partner==0 ? gd[+6] : gd[+0].
	//   4. Partner reaction balloon @ KDTextIndex[+10], `_SayKDDigital(5)`.
	//   5. _FirstTry = 0; NextScreen = LastScreen (1df2:043f).
	if (!guessedRight) {
		static const uint16 kAlibiBubbles[16] = {
			0x002B, 0x002C, 0x002D, 0x002E,
			0x00AB, 0x00AC, 0x00AD, 0x00AE,
			0x001D, 0x001E, 0x0015, 0x0016,
			0x0017, 0x0018, 0x0019, 0x001A,
		};

		const uint16 alibiOff = _mystery.alibiTextOffset((uint)picked);
		Common::String alibi;
		if (gd && alibiOff != 0xFFFF) {
			const char *raw = _mystery.textAt(alibiOff);
			if (raw)
				alibi = parseString(raw, _playerName, _partner);
		}
		uint bindx = 2;
		const byte firstChar = alibi.empty() ? (byte)0 : (byte)alibi[0];
		if (firstChar >= '0' && firstChar <= '9') {
			bindx = (uint)(firstChar - '0');
			alibi.deleteChar(0);
		}
		if (bindx >= 16)
			bindx = 2;
		uint16 bubNum = kAlibiBubbles[bindx];
		bubNum = fitBalloonToText(bubNum, alibi);

		Picture alibiBg;
		const bool haveAlibiBg = _picsArchive.getPicture(0x3e, alibiBg);
		Picture suspect;
		const byte *pickedSuspect = _mystery.usesCompactMacData()
			? _mystery.floppySuspectEntry((uint)picked)
			: (gd ? gd + (uint)picked * 0x46 : nullptr);
		const uint16 picId = pickedSuspect ? READ_LE_UINT16(pickedSuspect) : 0;
		const bool haveSuspect = picId != 0 &&
			_picsArchive.getPicture(picId, suspect);
		Picture balloon;
		const bool haveBalloon =
			_balloonArchive.size() > (bubNum & 0x7F) &&
			_balloonArchive.loadEntry(bubNum & 0x7F, balloon);

		int balloonX = 0x21;
		int balloonY = 1;
		int py = 0x5a;
		if (bindx < 8) {
			const int bw = haveBalloon ? balloon.surface.w : 0;
			const int bh = haveBalloon ? balloon.surface.h : 0;
			balloonX = (kScreenWidth - bw) / 2;
			if (bh < 0x5a) {
				balloonY = (0x5a - bh) / 2;
			} else {
				balloonY = 1;
				py = bh;
			}
		} else {
			const int bh = haveBalloon ? balloon.surface.h : 0;
			balloonX = 0x21;
			balloonY = (bh < 0x4f) ? (0x50 - bh) / 2 : 1;
		}

		Graphics::ManagedSurface base(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		base.clear();
		if (haveAlibiBg)
			base.simpleBlitFrom(alibiBg.surface);
		if (haveSuspect) {
			const byte transp = (byte)(suspect.flags >> 8);
			base.transBlitFrom(suspect.surface,
							   Common::Point(0x82, py),
							   (uint32)transp);
		}

		Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(base);
		if (haveBalloon) {
			const byte transp = (byte)(balloon.flags >> 8);
			scratch.transBlitFrom(balloon.surface,
								  Common::Point(balloonX, balloonY),
								  (uint32)transp);
		}

		uint16 tx = 5, ty = 4, tw = 155;
		getBalloonInsets(bubNum, tx, ty, tw);
		if (_font.isLoaded() && !alibi.empty()) {
			_font.drawWordWrapped(&scratch, balloonX + tx,
								  balloonY + ty, tw, alibi,
								  haveBalloon ? 0 : 0xF);
		}
		blitPdaPartner(scratch, _aniArchive, _partner, kPdaGalleryPartner,
					   g_system->getMillis());

		if (_music && _voiceOn) {
			_music->playMus(6, /* loop= */ false);
			const uint32 musStart = g_system->getMillis();
			bool aborted = false;
			while (_music->isPlaying() && !shouldQuit() && !aborted) {
				Common::Event ev;
				while (g_system->getEventManager()->pollEvent(ev)) {
					if (ev.type == Common::EVENT_QUIT ||
						ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
						_music->stop();
						return;
					}
					if (ev.type == Common::EVENT_KEYDOWN ||
						ev.type == Common::EVENT_LBUTTONDOWN) {
						aborted = true;
						break;
					}
				}
				if (g_system->getMillis() - musStart > 10000)
					break;
				g_system->updateScreen();
				g_system->delayMillis(20);
			}
			_music->stop();
		}

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, kScreenWidth, kScreenHeight);
		g_system->updateScreen();
		if (_audio && gd && !isMacintosh()) {
			const uint16 alibiVoice =
				READ_LE_UINT16(gd + (uint)picked * 0x46 + 0x00);
			const uint16 jakeVoice =
				READ_LE_UINT16(gd + (uint)picked * 0x46 + 0x06);
			const uint16 talk =
				(_partner == kPartnerJake) ? jakeVoice : alibiVoice;
			if (talk != 0)
				_audio->spoolSound((uint)(talk - 1));
		}
		waitForInput(60000);

		const byte *reactIdx = _mystery.kdTextIndex();
		if (reactIdx) {
			const int16 reactOff = (int16)READ_LE_UINT16(reactIdx + 10);
			Common::String react;
			if (reactOff != -1) {
				const char *raw = _mystery.textAt((uint16)reactOff);
				if (raw)
					react = parseString(raw, _playerName, _partner);
			}
			if (!react.empty()) {
				const byte rChar = (byte)react[0];
				uint16 rBub = getKDTextBalloon(rChar);
				if (rChar >= '0' && rChar <= '9')
					react.deleteChar(0);
				rBub = fitBalloonToText(rBub, react);
				Picture rBalloon;
				const bool haveR =
					_balloonArchive.size() > (rBub & 0x7F) &&
					_balloonArchive.loadEntry(rBub & 0x7F, rBalloon);
				const int rX = 0x21;
				int rY = 1;
				if (haveR && rBalloon.surface.h < 0x4e)
					rY = (0x50 - rBalloon.surface.h) / 2;

				scratch.simpleBlitFrom(base);
				if (haveR) {
					const byte transp = (byte)(rBalloon.flags >> 8);
					scratch.transBlitFrom(rBalloon.surface,
										   Common::Point(rX, rY),
										   (uint32)transp);
				}
				uint16 rtx = 5, rty = 4, rtw = 155;
				getBalloonInsets(rBub, rtx, rty, rtw);
				if (_font.isLoaded()) {
					_font.drawWordWrapped(&scratch, rX + rtx,
										  rY + rty, rtw, react,
										  haveR ? 0 : 0xF);
				}
				blitPdaPartner(scratch, _aniArchive, _partner,
							   kPdaGalleryPartner, g_system->getMillis());
				g_system->copyRectToScreen(scratch.getPixels(),
					scratch.pitch, 0, 0, kScreenWidth, kScreenHeight);
				g_system->updateScreen();
				if (_audio)
					_audio->sayKDDigital(reactIdx, 5, _partner);
			}
		}
		waitForInput(60000);

		_mystery._firstTry = false;
		_nextScreen = _lastScreen != kScreenInvalid
						? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	{
		const uint mn = _mystery.number();
		if (mn < sizeof(_mysteriesSolved)) {
			_mysteriesSolved[mn] = _mystery._firstTry ? 2 : 1;
		}

		advanceChainStageAfterSolve(mn);

		// `_DisplayCorrect` win background = `_BuildBackground(scene, 0x42, 0x14)`
		// (frame PIC 0x3d + scene at 0x42,0x14, palette scene+1). EEM1 CD uses
		// scene 5; EEM2/London uses scene 0x1b.
		const uint winScene = isLondon() ? 0x1b : 5;
		setSitePalette(winScene + 1);
		const int screenW = screenWidth();
		const int screenH = screenHeight();
		Graphics::ManagedSurface scratch(screenW, screenH,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		Picture frame, scene;
		if (_picsArchive.loadEntry(0x3d, frame))
			scratch.simpleBlitFrom(frame.surface);
		if (winScene < _sitesArchive.size() &&
			_sitesArchive.loadEntry(winScene, scene)) {
			const int sx = scaleX(0x42);
			const int sy = scaleY(0x14);
			const int sw = MIN<int>(scene.surface.w, screenW - sx);
			const int sh = MIN<int>(scene.surface.h, screenH - sy);
			if (sw > 0 && sh > 0)
				scratch.copyRectToSurface(scene.surface.getPixels(),
										  scene.surface.pitch, sx, sy,
										  sw, sh);
		}
		blitPdaPartner(scratch, _aniArchive, _partner,
					   kPdaGalleryPartner, g_system->getMillis(),
					   isMacintosh());
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, screenW, screenH);
		g_system->updateScreen();

		if (_music && _voiceOn)
			_music->playMus(5, /* loop= */ false);

		const byte *solved = _mystery.solvedClueBlock();
		if (isMacintosh() && _mystery.usesCompactMacData() && solved) {
			const byte *bufBase = _mystery.blobAt(0);
			const byte *end = bufBase ? bufBase + _mystery.dataSize()
									  : nullptr;
			const uint count = solved[0];
			const byte *records = solved + 1;
			const bool validChain =
				advanceFloppyDialogRecords(records, count, end,
										   /* mac= */ true) != nullptr;
			if (!validChain) {
				warning("doAccuse: malformed Mac solved chain");
			} else if (count > 3) {
				const uint beforeScrapbook = count - 3;
				const byte *tail =
					advanceFloppyDialogRecords(records, beforeScrapbook,
											   end, /* mac= */ true);
				if (tail) {
					displayFloppyDialogRecords(records, beforeScrapbook, 1);
					playMacScrapbookPartnerAnimation(this, scratch);
					displayFloppyDialogRecords(tail, 3, 1);
				}
			} else {
				displayFloppyDialogRecords(records, count, 1);
			}
		} else if (solved) {
			displayClue(solved);
		}
		if (_music && _voiceOn)
			_music->stop();

		if (!isMacintosh()) {
			playAnm(Common::Path(isLondon() ? "SCRAP.ANM" : "SCRAPBK.ANI"),
					120, /* holdLastFrame= */ false);
			displayScrapbookExtra(mn);
		}

		doShowEnding(mn);

		_mystery.clear();
		const Common::Error err = saveProfile(_playerName);
		if (err.getCode() != Common::kNoError)
			warning("saveProfile after solve failed: %s",
					err.getDesc().c_str());

		_nextScreen = kScreenAction;
	}
}

void EEMEngine::floppyKDHint(uint kdSlot, const byte *kdIdx,
							 const byte *bufBase, uint32 mysSize) {
	if ((uint)(kdSlot * 2) + 2 > (uint)(mysSize - (kdIdx - bufBase)))
		return;
	const uint16 textOff = READ_LE_UINT16(kdIdx + kdSlot * 2);
	if (textOff == 0 || textOff >= mysSize)
		return;
	const char *p = (const char *)(bufBase + textOff);
	uint32 lineLen = 0;
	while (textOff + lineLen < mysSize && p[lineLen] != 0)
		lineLen++;
	if (lineLen == 0)
		return;
	Common::String raw(p, lineLen);
	static const uint8 kDigitToBalloon[10] = {
		0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x1e, 0x0a
	};
	uint balloonIdx = 0x17;
	const char *txt = raw.c_str();
	if (*txt >= '0' && *txt <= '9') {
		balloonIdx = kDigitToBalloon[(int)(*txt - '0')];
		txt++;
	}
	Common::String text =
		parseString(Common::String(txt), _playerName, _partner);
	balloonIdx = fitBalloonToText((uint16)balloonIdx, text) & 0x7F;
	Graphics::ManagedSurface ms(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	Graphics::Surface *cur = g_system->lockScreen();
	if (cur) {
		ms.simpleBlitFrom(*cur);
		g_system->unlockScreen();
	}
	Picture balloon;
	const bool haveBalloon = _balloonArchive.size() > balloonIdx &&
		_balloonArchive.loadEntry(balloonIdx, balloon);
	uint16 balloonY = 1;
	if (haveBalloon) {
		const uint h = (uint)balloon.surface.h;
		if (h < 0x4e)
			balloonY = (uint16)((0x50 - h) >> 1);
		const byte transp = (byte)(balloon.flags >> 8);
		ms.transBlitFrom(balloon.surface,
						 Common::Point(0x21, balloonY), transp);
	}
	uint16 bx = 5;
	uint16 by = 4;
	uint16 bw = 142;
	getBalloonInsets(balloonIdx, bx, by, bw);
	_font.drawWordWrapped(&ms, 0x21 + bx, balloonY + by,
						  MAX<int>(8, (int)bw), text, 0);
	g_system->copyRectToScreen(ms.getPixels(), ms.pitch, 0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();

	while (!shouldQuit()) {
		Common::Event ev;
		bool advance = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER ||
				ev.type == Common::EVENT_LBUTTONDOWN ||
				ev.type == Common::EVENT_KEYDOWN) {
				advance = true;
				break;
			}
		}
		if (advance)
			break;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void EEMEngine::displayScrapbookExtra(uint mysteryNum) {
	if (isFloppy() || !ConfMan.getBool("restored_content") ||
		!_restoredContentDataLoaded ||
		mysteryNum >= kScrapbookExtraCaseCount || !_font.isLoaded())
		return;

	Common::File file;
	if (!file.open(Common::Path(kScrapbookExtraFilename))) {
		warning("EEM restored content unavailable: %s missing",
				kScrapbookExtraFilename);
		return;
	}

	byte magic[sizeof(kScrapbookExtraMagic)];
	if (file.read(magic, sizeof(magic)) != sizeof(magic) ||
		memcmp(magic, kScrapbookExtraMagic, sizeof(magic)) != 0)
		return;

	const uint16 version = file.readUint16LE();
	const uint16 caseCount = file.readUint16LE();
	if (version != kScrapbookExtraVersion ||
		caseCount > kScrapbookExtraCaseCount ||
		mysteryNum >= caseCount)
		return;

	const uint32 tableOffset = (uint32)sizeof(kScrapbookExtraMagic) + 4 +
		mysteryNum * 8;
	if (!file.seek(tableOffset))
		return;

	const uint32 recordsOffset = file.readUint32LE();
	const uint16 recordCount = file.readUint16LE();
	file.skip(2);                  // reserved
	if (recordsOffset == 0 || recordCount == 0 ||
		recordCount > kScrapbookExtraMaxRecords ||
		!file.seek(recordsOffset))
		return;

	Graphics::ManagedSurface bg(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	bg.clear();
	if (Graphics::Surface *screen = g_system->lockScreen()) {
		bg.simpleBlitFrom(*screen);
		g_system->unlockScreen();
	}

	for (uint recordIdx = 0; recordIdx < recordCount && !shouldQuit();
		 recordIdx++) {
		const uint16 picId = file.readUint16LE();
		const uint16 picX = file.readUint16LE();
		const uint8 picY = file.readByte();
		const uint8 balloonRaw = file.readByte();
		const uint16 balloonX = file.readUint16LE();
		const uint8 balloonY = file.readByte();
		file.skip(1);              // reserved
		const uint16 voiceJake = file.readUint16LE();
		const uint16 voiceJenny = file.readUint16LE();
		const uint16 voiceNancy = file.readUint16LE();
		const uint16 jakeTextSize = file.readUint16LE();
		const uint16 jennyTextSize = file.readUint16LE();

		if (file.eos() || file.err())
			return;

		Common::String rawJake;
		Common::String rawJenny;
		if (!readScrapbookExtraText(file, jakeTextSize, rawJake) ||
			!readScrapbookExtraText(file, jennyTextSize, rawJenny))
			return;

		Common::String text = parseString(
			_partner == kPartnerJake ? rawJake : rawJenny,
			_playerName, _partner);
		if (text.empty())
			continue;

		Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(*bg.surfacePtr());

		if (picId != 0 && picId != 0xFFFF) {
			Picture pic;
			if (_picsArchive.getPicture(picId, pic) &&
				picX < kScreenWidth && picY < kScreenHeight) {
				const byte transp = (byte)(pic.flags >> 8);
				scratch.transBlitFrom(pic.surface,
									  Common::Point(picX, picY),
									  (uint32)transp);
			}
		}

		const uint16 fittedBalloon = fitBalloonToText(balloonRaw, text);
		const uint16 balloonId = fittedBalloon & 0x7F;
		const bool flipBalloon = (fittedBalloon & 0x80) != 0;
		Picture balloon;
		const bool haveBalloon = balloonRaw != 0xFF &&
			_balloonArchive.size() > balloonId &&
			_balloonArchive.loadEntry(balloonId, balloon);

		uint16 textXInset = 5;
		uint16 textYInset = 4;
		uint16 textWidth = 155;
		if (haveBalloon) {
			const byte transp = (byte)(balloon.flags >> 8);
			scratch.transBlitFrom(balloon.surface,
								  Common::Point(balloonX, balloonY),
								  (uint32)transp, flipBalloon);
			getBalloonInsets(balloonId, textXInset, textYInset, textWidth);
		}

		_font.drawWordWrapped(&scratch, balloonX + textXInset,
							  balloonY + textYInset,
							  MAX<int>(8, (int)textWidth), text,
							  haveBalloon ? 0 : 0xF);

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, kScreenWidth, kScreenHeight);
		g_system->updateScreen();

		if (_audio && restoredContentVoiceAppliesTo(mysteryNum)) {
			uint16 voice = kScrapbookExtraNoVoice;
			if (recordIdx == 0 && voiceNancy != kScrapbookExtraNoVoice) {
				voice = voiceNancy;
			} else if (recordIdx == 1) {
				voice = (_partner == kPartnerJake) ? voiceJake : voiceJenny;
			} else {
				voice = (_partner == kPartnerJake) ? voiceJenny : voiceJake;
			}
			if (voice != kScrapbookExtraNoVoice)
				_audio->spoolSound(voice);
		}

		const bool skipRest = floppyDialogWaitForClick();
		if (_audio)
			_audio->stopSpool();
		if (skipRest)
			return;
	}
}

void EEMEngine::accuseDrawGallery(int highlighted,
								  Common::Array<Common::Rect> &rects,
								  Common::Array<int> &suspects, uint8 num,
								  bool haveAccuseBg,
								  const Picture &accuseBg) {
	Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (haveAccuseBg)
		scratch.simpleBlitFrom(accuseBg.surface);

	blitPdaPartner(scratch, _aniArchive, _partner, kPdaGalleryPartner,
				   g_system->getMillis());

	const byte *gd = _mystery.galleryData();
	rects.resize(num);
	suspects.resize(num);
	for (uint i = 0; i < num; i++) {
		rects[i] = Common::Rect();
		suspects[i] = -1;
		const uint8 phys = _mystery._newOrder[i];
		if (phys >= 5)
			continue;
		if (_mystery._inGallery[phys] == 0)
			continue;
		const byte *e = _mystery.usesCompactMacData()
			? _mystery.floppySuspectEntry(i)
			: (gd ? gd + i * 0x46 : nullptr);
		if (!e)
			continue;
		const uint16 picId = READ_LE_UINT16(e + 0);
		if (picId == 0)
			continue;
		Picture portrait;
		if (!_picsArchive.getPicture(picId, portrait))
			continue;
		const GallerySlot &s = kFloppyGallerySlots[phys];
		const int placeX = s.x;
		const int placeY = s.y + (0x48 - portrait.surface.h);
		const byte transp = (byte)(portrait.flags >> 8);
		scratch.transBlitFrom(portrait.surface,
							  Common::Point(placeX, placeY),
							  (uint32)transp);
		rects[i] = Common::Rect(placeX, placeY,
								 placeX + portrait.surface.w,
								 placeY + portrait.surface.h);
		suspects[i] = (int)i;
		if (highlighted == (int)i) {
			scratch.frameRect(rects[i], 0xFE);
		}
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();
}

void EEMEngine::doAccuseFloppy() {
	const byte *kdIdx     = _mystery.kdTextIndex();
	const byte *bufBase   = _mystery.blobAt(0);
	const uint32 mysSize  = _mystery.dataSize();
	if (!kdIdx || !bufBase)
		return;

	const int score = _mystery.selectedPoints();
	uint kdSlot;
	bool readyToSolve;
	if (score < 50) {
		kdSlot = 0;        // "we've barely started"
		readyToSolve = false;
	} else if (score < 100) {
		kdSlot = 1;        // "getting closer"
		readyToSolve = false;
	} else {
		kdSlot = 2;        // "ready to solve"
		readyToSolve = true;
	}
	floppyKDHint(kdSlot, kdIdx, bufBase, mysSize);
	if (!readyToSolve) {
		_nextScreen = _lastScreen != kScreenInvalid
			? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	if (!doAccuseNotes()) {
		if (_nextScreen == kScreenAccuse) {
			_nextScreen = _lastScreen != kScreenInvalid
				? (ScreenId)_lastScreen : kScreenSite;
		}
		return;
	}

	int userSelectedScore = 0;
	{
		const byte *ni2 = _mystery.noteIndex();
		const uint16 niCount2 = _mystery.noteIndexCount();
		if (ni2) {
			const uint maxIdx = MIN<uint>(niCount2,
										   Mystery::kCluesFoundCap);
			for (uint i = 0; i < maxIdx; i++) {
				if (_mystery._noteSelected[i])
					userSelectedScore += (int)ni2[i * 7 + 6];
			}
		}
	}
	if (userSelectedScore < 100) {
		floppyKDHint(3, kdIdx, bufBase, mysSize);
		_nextScreen = _lastScreen != kScreenInvalid
			? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	floppyKDHint(4, kdIdx, bufBase, mysSize);
	const uint8 num = _mystery.numSuspects();
	if (num == 0) {
		_nextScreen = _lastScreen != kScreenInvalid
			? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	Picture accuseBg;
	const bool haveAccuseBg = _picsArchive.getPicture(0x3f, accuseBg);

	Common::Array<Common::Rect> slotRects;
	Common::Array<int> slotSuspect;
	int highlighted = 0;
	int picked = -1;
	accuseDrawGallery(highlighted, slotRects, slotSuspect, num,
					  haveAccuseBg, accuseBg);

	uint32 lastTick = g_system->getMillis();
	while (picked < 0 && !shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					openMainMenuDialog();
					accuseDrawGallery(highlighted, slotRects, slotSuspect,
									  num, haveAccuseBg, accuseBg);
					continue;
				}
				if (ev.kbd.keycode == Common::KEYCODE_TAB ||
					ev.kbd.keycode == Common::KEYCODE_RIGHT) {
					highlighted = (highlighted + 1) % MAX<int>(1, (int)num);
					accuseDrawGallery(highlighted, slotRects, slotSuspect,
									  num, haveAccuseBg, accuseBg);
				} else if (ev.kbd.keycode == Common::KEYCODE_LEFT) {
					highlighted = (highlighted + (int)num - 1) %
								  MAX<int>(1, (int)num);
					accuseDrawGallery(highlighted, slotRects, slotSuspect,
									  num, haveAccuseBg, accuseBg);
				} else if ((ev.kbd.keycode == Common::KEYCODE_RETURN ||
							ev.kbd.keycode == Common::KEYCODE_KP_ENTER) &&
						   highlighted < (int)slotRects.size() &&
						   !slotRects[highlighted].isEmpty()) {
					picked = slotSuspect[highlighted];
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotSuspect[i] >= 0 &&
						slotRects[i].contains(ev.mouse.x, ev.mouse.y)) {
						picked = slotSuspect[i];
						break;
					}
				}
			}
		}
		const uint32 now = g_system->getMillis();
		if (now - lastTick >= 100) {
			accuseDrawGallery(highlighted, slotRects, slotSuspect, num,
							  haveAccuseBg, accuseBg);
			lastTick = now;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	if (picked < 0)
		return;

	const byte *gd = _mystery.galleryData();
	const bool guilty = _mystery.isGuilty((uint)picked);

	if (guilty) {
		const uint mn = _mystery.number();

		{
			Graphics::Surface *blk = g_system->lockScreen();
			if (blk) {
				memset(blk->getPixels(), 0, kScreenWidth * kScreenHeight);
				g_system->unlockScreen();
			}
			setSitePalette(6);
			Picture frame;
			if (_picsArchive.loadEntry(0x3d, frame)) {
				g_system->copyRectToScreen(frame.surface.getPixels(),
					frame.surface.pitch, 0, 0,
					frame.surface.w, frame.surface.h);
			}
			Picture scene;
			if (5 < _sitesArchive.size() &&
				_sitesArchive.loadEntry(5, scene)) {
				const int sx = 0x42, sy = 0x14;
				const int sw = MIN<int>(scene.surface.w, kScreenWidth - sx);
				const int sh = MIN<int>(scene.surface.h, kScreenHeight - sy);
				if (sw > 0 && sh > 0)
					g_system->copyRectToScreen(scene.surface.getPixels(),
						scene.surface.pitch, sx, sy, sw, sh);
			}

			if (Graphics::Surface *screen = g_system->lockScreen()) {
				blitPdaPartner(screen, _aniArchive, _partner,
							   kPdaGalleryPartner, g_system->getMillis());
				g_system->unlockScreen();
			}
			g_system->updateScreen();
		}

		if (_music && _voiceOn)
			_music->playFile(Common::Path("travel-2.xmi"), false);

		const byte *chain = _mystery.solvedClueBlock();
		if (chain) {
			const uint count = chain[0];
			const byte *records = chain + 1;
			const byte *end = bufBase + mysSize;
			if (count > 3) {
				const uint beforeScrapbook = count - 3;
				const byte *tail =
					advanceFloppyDialogRecords(records, beforeScrapbook,
											   end);
				if (tail) {
					displayFloppyDialogRecords(records, beforeScrapbook, 1);
					playAnm(Common::Path("SCRAPBK.ANI"), 120,
							/* holdLastFrame= */ false, /* fadeIn= */ true);
					displayFloppyDialogRecords(tail, 3, 1);
				} else {
					warning("doAccuseFloppy: malformed solved chain");
					displayFloppyDialogRecords(records, count, 1);
				}
			} else {
				displayFloppyDialogRecords(records, count, 1);
			}
		}
		if (_music && _voiceOn)
			_music->stop();

		if (mn < sizeof(_mysteriesSolved))
			_mysteriesSolved[mn] = _mystery._firstTry ? 2 : 1;

		advanceChainStageAfterSolve(mn);

		if (_audio && _voiceOn) {
			Common::File ending;
			const Common::String fname =
				Common::String::format("E%u.BIN", mn);
			if (ending.open(Common::Path(fname)) && ending.size() > 0) {
				static const uint8 kSolvedVoiceSlot[3] = {
					0x15, 0x16, 0x17
				};
				const byte type = ending.readByte();
				if (type < ARRAYSIZE(kSolvedVoiceSlot)) {
					_audio->playFloppyVoiceSlot(kSolvedVoiceSlot[type],
												_partner);
					_audio->waitForVoiceDone();
				}
			}
		}

		doShowEnding(mn);

		_mystery._solvedPuzzle = true;
		_mystery.clear();
		(void)saveProfile(_playerName);
		_nextScreen = kScreenAction;
		return;
	}

	const byte *susp = _mystery.usesCompactMacData()
		? _mystery.floppySuspectEntry((uint)picked)
		: (gd ? gd + (uint)picked * 0x46 : nullptr);
	uint16 picId = 0;
	uint16 alibiOff = 0xFFFF;
	if (susp) {
		picId    = READ_LE_UINT16(susp + 0);
		alibiOff = _mystery.alibiTextOffset((uint)picked);
	}

	if (_music && _voiceOn)
		_music->playFile(Common::Path("fanfare2.xmi"), false);

	Picture alibiBg;
	const bool haveAlibiBg = _picsArchive.getPicture(0x3e, alibiBg);
	Picture suspectPic;
	const bool haveSuspect = picId != 0 &&
		_picsArchive.getPicture(picId, suspectPic);

	Common::String alibi;
	if (alibiOff != 0xFFFF && alibiOff < mysSize) {
		const char *raw = (const char *)(bufBase + alibiOff);
		uint32 lineLen = 0;
		while (alibiOff + lineLen < mysSize && raw[lineLen] != 0)
			lineLen++;
		alibi = parseString(Common::String(raw, lineLen),
							_playerName, _partner);
	}

	static const uint8 kFloppyAlibiBalloonByDigit[10] = {
		0x2a, 0x2b, 0x2c, 0x2d, 0xaa, 0xab, 0xac, 0xad, 0x09, 0x0a
	};
	uint balloonRaw = 0x2c; // DAT_2608_0c3c
	if (!alibi.empty() && alibi[0] >= '0' && alibi[0] <= '9') {
		balloonRaw = kFloppyAlibiBalloonByDigit[(int)(alibi[0] - '0')];
		alibi.deleteChar(0);
	}
	balloonRaw = fitBalloonToText((uint16)balloonRaw, alibi);
	const uint balloonIdx = balloonRaw & 0x7F;
	const bool flipBalloon = (balloonRaw & 0x80) != 0;

	// Compose alibi screen.
	Graphics::ManagedSurface scene(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	scene.clear();
	if (haveAlibiBg)
		scene.simpleBlitFrom(alibiBg.surface);
	if (haveSuspect) {
		const byte transp = (byte)(suspectPic.flags >> 8);
		scene.transBlitFrom(suspectPic.surface,
							 Common::Point(0x82, 0x5a),
							 (uint32)transp);
	}
	Picture balloon;
	const bool haveBalloon = _balloonArchive.size() > balloonIdx &&
		_balloonArchive.loadEntry(balloonIdx, balloon);
	int balloonX = 0x21;
	int balloonY = 1;
	if (haveBalloon) {
		balloonX = (kScreenWidth - balloon.surface.w) / 2;
		balloonY = (0x5a - balloon.surface.h) / 2;
		if (balloonX < 0)
			balloonX = 0;
		if (balloonY < 0)
			balloonY = 0;
		const byte transp = (byte)(balloon.flags >> 8);
		// Mirror flag (high bit) flips balloon horizontally.
		scene.transBlitFrom(balloon.surface,
							Common::Point(balloonX, balloonY),
							transp, flipBalloon);
	}
	uint16 tx = 5, ty = 4, tw = 155;
	getBalloonInsets(balloonIdx, tx, ty, tw);
	if (!alibi.empty()) {
		_font.drawWordWrapped(&scene, balloonX + tx, balloonY + ty,
							  MAX<int>(8, (int)tw), alibi, 0);
	}
	// Stamp partner resting frame before KD reaction snapshots screen.
	blitPdaPartner(scene, _aniArchive, _partner, kPdaGalleryPartner,
				   g_system->getMillis());
	g_system->copyRectToScreen(scene.getPixels(), scene.pitch, 0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();

	while (!shouldQuit()) {
		Common::Event ev;
		bool advance = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER ||
				ev.type == Common::EVENT_LBUTTONDOWN ||
				ev.type == Common::EVENT_KEYDOWN) {
				advance = true;
				break;
			}
		}
		if (advance)
			break;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	floppyKDHint(5, kdIdx, bufBase, mysSize);
	if (_music && _voiceOn)
		_music->stop();

	_mystery._firstTry = false;
	_nextScreen = _lastScreen != kScreenInvalid
		? (ScreenId)_lastScreen : kScreenSite;
}
// _DoAccuseGallery @ 1df2:0a31.
void EEMEngine::drawAccuseGallery(uint8 numSuspects, const byte *gd,
								   int highlighted,
								   Common::Array<Common::Rect> &slotRects,
								   Common::Array<int> &slotSuspect) {
	Picture accuseBg;
	const bool haveAccuseBg = _picsArchive.getPicture(0x3f, accuseBg);
	const bool mac = isMacintosh();
	const int sw = screenWidth();
	const int sh = screenHeight();

	Graphics::ManagedSurface scratch(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (haveAccuseBg)
		scratch.simpleBlitFrom(accuseBg.surface);

	// Partner drawn first; defensive (no slot overlap).
	blitPdaPartner(scratch, _aniArchive, _partner, kPdaGalleryPartner,
				   g_system->getMillis(), mac);
	const GallerySlot * const slots = mac ? kMacGallerySlots : kGallerySlots;

	for (uint i = 0; i < numSuspects && i < Mystery::kGalleryCap; i++) {
		slotRects[i] = Common::Rect();
		slotSuspect[i] = -1;
		if (!gd)
			continue;
		const uint8 phys = _mystery._newOrder[i];
		if (phys >= 5)
			continue;
		if (_mystery._inGallery[phys] == 0)
			continue;
		const GallerySlot &s = slots[phys];

		const byte *entry = _mystery.usesCompactMacData()
			? _mystery.floppySuspectEntry(i)
			: gd + i * 0x46;
		if (!entry)
			continue;
		const uint16 picId = READ_LE_UINT16(entry);
		if (picId == 0)
			continue;
		Picture portrait;
		if (!_picsArchive.getPicture(picId, portrait))
			continue;

		const int placeX = s.x;
		const int placeY = mac ? s.y : s.y + (0x48 - portrait.surface.h);
		const int w = MIN<int>(portrait.surface.w, sw - placeX);
		const int h = MIN<int>(portrait.surface.h, sh - placeY);
		if (w <= 0 || h <= 0)
			continue;
		if (mac)
			blitMacMaskedSurface(scratch.surfacePtr(), portrait,
								 placeX, placeY);
		else
			scratch.transBlitFrom(portrait.surface,
								  Common::Point(placeX, placeY),
								  (uint32)(byte)(portrait.flags >> 8));
		slotRects[i] = Common::Rect(placeX, placeY,
									 placeX + w, placeY + h);
		slotSuspect[i] = (int)i;
	}

	// Highlight outline (original uses `_PutMouseInRect` @ 1df2:0b8e).
	if (highlighted >= 0 && highlighted < (int)slotRects.size() &&
		!slotRects[highlighted].isEmpty()) {
		Common::Rect r = slotRects[highlighted];
		r.grow(1);
		scratch.frameRect(r, 0xFE);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, sw, sh);
	g_system->updateScreen();
}

} // End of namespace EEM
