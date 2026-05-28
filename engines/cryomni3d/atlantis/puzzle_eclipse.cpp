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

#include "cryomni3d/atlantis/puzzle_eclipse.h"

#include "cryomni3d/atlantis/engine.h"
#include "cryomni3d/atlantis/sprite_blend.h"

#include "audio/audiostream.h"
#include "audio/decoders/apc.h"
#include "audio/mixer.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace CryOmni3D {
namespace Atlantis {

// ===========================================================================
// Constants reverse-engineered from atlantis.exe.
// ===========================================================================

static const int kScreenW = 640;
static const int kScreenH = 480;

static const uint16 kPuzzleSprTransp = 0x0001;

// Click hotspots -- FUN_00441cd8 lines 441daa-441ddb / 441e59-441e8d.
static const int kLever1X0 = 0x78 + 1;   // 121
static const int kLever1X1 = 0xf0;       // 240
static const int kLever2X0 = 0x1ae + 1;  // 431
static const int kLever2X1 = 0x226;      // 550
static const int kLeverY0  = 0x172 + 1;  // 371
static const int kLeverY1  = 0x1e0;      // 480
static const int kLeverYUp = 0x1a9 + 1;  // 426 -- y < this = step up

// Top-corner exit zones -- FUN_00441cd8 lines 441cf0-441d2a / 441d2f-441d6c.
static const int kExitLeftX0  = 0;
static const int kExitLeftX1  = 0xa0;    // 160
static const int kExitRightX0 = 0x21d;   // 541
static const int kExitRightX1 = 0x280;   // 640
static const int kExitY1      = 0xdc;    // 220

// Return codes -- .rdata at 0x496d0c (abort) and 0x496d14 (win).
static const int kFinPuzzleWin   = 255;
static const int kFinPuzzleAbort = 1;

// Layer-swap thresholds for the moon (record 5) vs earth (record 6)
// rendering order -- FUN_00441828 lines 441956-44196b.
static const int kLayerSwapLo = 0x60;    // 96
static const int kLayerSwapHi = 0x83;    // 131

// Special PZ1L sprite frames used by the win highlight overlay --
// FUN_00441828 lines 441a8c / 441ab8.
static const int kHighlightFrameSun  = 0xd5;  // 213
static const int kHighlightFrameMoon = 0xd6;  // 214

// Win-animation length -- FUN_00441724 lines 441813-441818 (`< 0x50` iterations).
static const int kWinAnimFrames = 0x50;     // 80

// Initial record table -- baked into .rdata at 0x49745E (16-bit shorts).
struct RecordInit {
	int dir, x, y, min, max;
};
static const RecordInit kRecordInit[7] = {
	{ 0, 215, 308,   0,   7 },   // 0  PZ2L 0..7   -- idle decoration
	{ 1, 298, 308,   8,  15 },   // 1  PZ2L 8..15  -- auto-cycling (DIR=1)
	{ 0, 227, 384,  16,  86 },   // 2  PZ2L 16..86 -- lever 1 mechanism
	{ 0, 443, 373,  87, 157 },   // 3  PZ2L 87..157 -- lever 2 mechanism
	{ 0, 319, 204,   0,  70 },   // 4  PZ1L 0..70  -- SUN sphere    (lever 2)
	{ 0, 236, 222,  71, 141 },   // 5  PZ1L 71..141 -- MOON sphere   (lever 1)
	{ 0, 271, 198, 142, 212 },   // 6  PZ1L 142..212 -- EARTH sphere (lever 1)
};
static const bool kRecordUsesPz1l[7] = { false, false, false, false, true, true, true };
static const int kLever1Records[] = { 2, 5, 6 };
static const int kLever2Records[] = { 3, 4 };

// ===========================================================================
// SPR loader.
// ===========================================================================
static bool loadPuzzleSpr(CryOmni3DEngine_Atlantis *engine, const char *fname,
                          Common::Array<PuzzleSpr> &out) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
	    engine->openBigFileStream(kFileTypePuzzles, fname));
	if (!stream) {
		warning("EclipsePuzzle: cannot open PUZZLES\\%s", fname);
		return false;
	}

	uint16 flags = stream->readUint16LE();
	uint16 colorTable[256];
	memset(colorTable, 0, sizeof(colorTable));
	if (flags & 0x4) {
		for (int i = 0; i < 256; i++) {
			uint16 rgb555 = stream->readUint16LE();
			uint16 r = (rgb555 >> 10) & 0x1f;
			uint16 g = (rgb555 >>  5) & 0x1f;
			uint16 b =  rgb555        & 0x1f;
			colorTable[i] = (r << 11) | (g << 6) | b;
		}
	}
	if (flags & 0x8)
		stream->skip(512);

	uint16 count = stream->readUint16LE();
	if (count == 0 || count > 1024) return false;

	const int64 sprDataBase = stream->pos();
	Common::Array<uint32> offsets(count);
	for (uint16 i = 0; i < count; i++)
		offsets[i] = stream->readUint32LE();

	out.resize(count);
	for (uint16 si = 0; si < count; si++) {
		if (!stream->seek(sprDataBase + offsets[si])) continue;
		PuzzleSpr &spr = out[si];
		spr.w    = stream->readUint16LE();
		spr.h    = stream->readUint16LE();
		spr.xoff = stream->readSint16LE();
		spr.yoff = stream->readSint16LE();
		if (spr.w == 0 || spr.h == 0 || spr.w > 640 || spr.h > 480) continue;

		Common::Array<uint32> rowOff(spr.h);
		for (uint16 r = 0; r < spr.h; r++)
			rowOff[r] = stream->readUint32LE();
		const int64 pixBase = stream->pos();

		spr.pixels.resize((uint)spr.w * spr.h, kPuzzleSprTransp);
		spr.blend.resize((uint)spr.w * spr.h, (uint8)kSprNoBlend);

		for (uint16 ry = 0; ry < spr.h; ry++) {
			stream->seek(pixBase + rowOff[ry]);
			int rx = 0;
			while (rx < (int)spr.w && !stream->eos()) {
				byte b = stream->readByte();
				int btype = (b >> 6) & 0x3;
				int cnt   = b & 0x3F;
				if (btype == 0) {
					rx += cnt;
				} else if (btype == 1) {
					for (int i = 0; i < cnt && rx < (int)spr.w; i++, rx++) {
						uint16 col = colorTable[stream->readByte()];
						if (col == kPuzzleSprTransp) col = 0x0000;
						spr.pixels[ry * spr.w + rx] = col;
					}
				} else {
					for (int i = 0; i < cnt && rx < (int)spr.w; i++, rx++) {
						byte factor = stream->readByte() & 0x1f;
						uint16 col  = colorTable[stream->readByte()];
						if (col == kPuzzleSprTransp) col = 0x0000;
						spr.pixels[ry * spr.w + rx] = col;
						spr.blend[ry * spr.w + rx]  = factor;
					}
				}
			}
		}
	}
	debugC(1, kDebugScript, "EclipsePuzzle: loaded %u frames from PUZZLES\\%s", count, fname);
	return true;
}

static void blitFrame(Graphics::ManagedSurface &out, const PuzzleSpr &spr,
                      int cx, int cy) {
	if (spr.pixels.empty() || out.format.bytesPerPixel != 2) return;
	int dx0 = cx - spr.xoff;
	int dy0 = cy - spr.yoff;
	for (int y = 0; y < spr.h; y++) {
		int dy = dy0 + y;
		if (dy < 0 || dy >= out.h) continue;
		for (int x = 0; x < spr.w; x++) {
			int dx = dx0 + x;
			if (dx < 0 || dx >= out.w) continue;
			uint16 src = spr.pixels[y * spr.w + x];
			if (src == kPuzzleSprTransp) continue;
			uint8 factor = spr.blend[y * spr.w + x];
			uint16 *p = (uint16 *)out.getBasePtr(dx, dy);
			*p = (factor == (uint8)kSprNoBlend) ? src
			                                     : blendSprPixel565(src, *p, factor);
		}
	}
}

static bool loadPuzzleTGA(CryOmni3DEngine_Atlantis *engine, const char *fname,
                          Common::Array<uint16> &outPixels) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
	    engine->openBigFileStream(kFileTypePuzzles, fname));
	if (!stream) return false;
	byte idLen = stream->readByte();
	/* cmType = */ stream->readByte();
	byte imgType = stream->readByte();
	stream->skip(9);
	uint16 w = stream->readUint16LE();
	uint16 h = stream->readUint16LE();
	byte depth = stream->readByte();
	stream->readByte();
	stream->skip(idLen);
	if (imgType != 2 || depth != 15 || w != kScreenW || h != kScreenH) return false;
	const Graphics::PixelFormat fmt = g_system->getScreenFormat();
	outPixels.resize((uint)w * h);
	for (uint16 y = 0; y < h; y++) {
		uint16 *row = outPixels.begin() + (uint)(h - 1 - y) * w;
		for (uint16 x = 0; x < w; x++) {
			uint16 px = stream->readUint16LE();
			uint8 r = (uint8)(((px >> 10) & 0x1f) << 3);
			uint8 g = (uint8)(((px >>  5) & 0x1f) << 3);
			uint8 b = (uint8)(( px        & 0x1f) << 3);
			row[x] = (uint16)fmt.RGBToColor(r, g, b);
		}
	}
	return true;
}

// ===========================================================================
EclipsePuzzle::EclipsePuzzle(CryOmni3DEngine_Atlantis *engine)
    : _engine(engine), _frameCounter(0), _bgLoaded(false) {
	loadPuzzleSpr(engine, "PZ1L.SPR", _pz1lFrames);
	loadPuzzleSpr(engine, "PZ2L.SPR", _pz2lFrames);
	_bgLoaded = loadPuzzleTGA(engine, "PZLION.TGA", _bgPixels);

	for (int i = 0; i < kNumRecords; i++) {
		_records[i].dir  = kRecordInit[i].dir;
		_records[i].x    = kRecordInit[i].x;
		_records[i].y    = kRecordInit[i].y;
		_records[i].min  = kRecordInit[i].min;
		_records[i].max  = kRecordInit[i].max;
		_records[i].curr = (i >= 2) ? (_records[i].min + 0x25)
		                            : _records[i].min;
	}
}

EclipsePuzzle::~EclipsePuzzle() {
}

// Mirrors FUN_00441f50.
void EclipsePuzzle::tick(int recIdx) {
	Record &r = _records[recIdx];
	if (r.dir == 0) return;
	if (r.dir == 1) {
		r.curr++;
		if (r.curr > r.max) r.curr = r.min;
	} else if (r.dir == 2) {
		r.curr--;
		if (r.curr < r.min) r.curr = r.max;
	}
}

// Mirrors FUN_00441cd8 lines 441f0b-441f32.
bool EclipsePuzzle::isWin() const {
	return _records[4].curr == _records[4].min + 1 &&
	       _records[5].curr == _records[5].min;
}

// Mirrors FUN_00441828 in full.
void EclipsePuzzle::renderFrame(Graphics::ManagedSurface &out) {
	const Graphics::PixelFormat fmt = out.format;

	// 1. Background.
	if (_bgLoaded && fmt.bytesPerPixel == 2 &&
	    _bgPixels.size() == (uint)kScreenW * kScreenH) {
		for (int y = 0; y < kScreenH; y++)
			memcpy(out.getBasePtr(0, y),
			       _bgPixels.begin() + (uint)y * kScreenW, kScreenW * 2);
	} else {
		out.fillRect(Common::Rect(0, 0, kScreenW, kScreenH),
		             fmt.RGBToColor(8, 12, 32));
	}

	// 2. Records 0..3: tick + blit PZ2L (FUN_00441828 lines 38212-38216).
	for (int i = 0; i < 4; i++) {
		tick(i);
		if (_records[i].curr >= 0 && (uint)_records[i].curr < _pz2lFrames.size())
			blitFrame(out, _pz2lFrames[_records[i].curr],
			          _records[i].x, _records[i].y);
	}

	// 3. Records 4..6: tick first (FUN_00441828 lines 38217-38219).
	tick(4);
	tick(5);
	tick(6);

	// 4. Layer-swap: default render order is (record6 back, record4
	// middle, record5 front).  When record5.CURR is in (96, 131) the
	// exe swaps records 5 and 6 -- mirrors lines 441948-441974.
	int backIdx  = 6;
	int frontIdx = 5;
	if (_records[5].curr > kLayerSwapLo && _records[5].curr < kLayerSwapHi) {
		backIdx  = 5;
		frontIdx = 6;
	}

	auto blitPz1l = [&](int recIdx, int frameOverride) {
		int frame = (frameOverride >= 0) ? frameOverride : _records[recIdx].curr;
		if (frame >= 0 && (uint)frame < _pz1lFrames.size())
			blitFrame(out, _pz1lFrames[frame],
			          _records[recIdx].x, _records[recIdx].y);
	};

	blitPz1l(backIdx, -1);
	blitPz1l(4, -1);              // SUN
	blitPz1l(frontIdx, -1);

	// 5. Win highlight (FUN_00441828 lines 441a26-441ae1): when the win
	// condition holds AND (_frameCounter & 2) != 0, draw three extra
	// PZ1L blits to pulse the eclipse alignment:
	//   - record6 at its current frame (redrawn on top for z-order);
	//   - PZ1L frame 213 at record 4's position (SUN highlight sprite);
	//   - PZ1L frame 214 at record 5's position (MOON highlight sprite).
	if (isWin() && (_frameCounter & 2)) {
		blitPz1l(6, -1);
		blitPz1l(4, kHighlightFrameSun);
		blitPz1l(5, kHighlightFrameMoon);
	}

	_frameCounter++;
}

// Open WAV/<name>.APC, queue and play.  When `wait`, blocks until the
// sound finishes (used for the win sound which leads straight to puzzle
// exit).  Click sounds fire-and-forget.
void EclipsePuzzle::playSound(const char *name, bool wait) {
	Common::String fname = Common::String::format("%s.APC", name);
	Common::SeekableReadStream *apcFile =
	    _engine->openBigFileStream(kFileTypeSound, fname);
	if (!apcFile) {
		warning("EclipsePuzzle: cannot open WAV\\%s", fname.c_str());
		return;
	}
	Audio::PacketizedAudioStream *apc = Audio::makeAPCStream(*apcFile);
	if (!apc) { delete apcFile; return; }
	int64 remaining = apcFile->size() - apcFile->pos();
	if (remaining > 0) {
		byte *buf = new byte[remaining];
		apcFile->read(buf, (uint32)remaining);
		apc->queuePacket(new Common::MemoryReadStream(buf, remaining,
		                                              DisposeAfterUse::YES));
	}
	apc->finish();
	delete apcFile;

	Audio::SoundHandle handle;
	_engine->_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, apc);
	if (wait) {
		while (!_engine->shouldAbort() &&
		       _engine->_mixer->isSoundHandleActive(handle)) {
			_engine->pollEvents();
			g_system->delayMillis(15);
		}
	}
}

int EclipsePuzzle::run() {
	const Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::ManagedSurface working(kScreenW, kScreenH, fmt);

	g_system->lockMouse(false);
	_engine->setMousePos(Common::Point(kScreenW / 2, kScreenH / 2));
	_engine->setArrowCursor();
	_engine->showMouse(true);

	bool exitLoop = false;
	int  result = kFinPuzzleAbort;
	bool prevButton = (_engine->getCurrentMouseButton() != 0);

	while (!exitLoop && !_engine->shouldAbort()) {
		_engine->pollEvents();

		bool nowButton = (_engine->getCurrentMouseButton() == 1);
		if (nowButton && !prevButton) {
			Common::Point m = _engine->getMousePos();

			bool inExitL = (m.x >= kExitLeftX0  && m.x < kExitLeftX1  &&
			                m.y >= 0            && m.y < kExitY1);
			bool inExitR = (m.x >= kExitRightX0 && m.x < kExitRightX1 &&
			                m.y >= 0            && m.y < kExitY1);
			if (inExitL || inExitR) {
				result = kFinPuzzleAbort;
				exitLoop = true;
			} else if (m.y >= kLeverY0 && m.y < kLeverY1) {
				int dir = (m.y < kLeverYUp) ? 1 : 2;
				bool clicked = false;
				if (m.x >= kLever1X0 && m.x < kLever1X1) {
					for (uint k = 0; k < ARRAYSIZE(kLever1Records); k++) {
						int idx = kLever1Records[k];
						_records[idx].dir = dir;
						tick(idx);
						_records[idx].dir = 0;
					}
					clicked = true;
				} else if (m.x >= kLever2X0 && m.x < kLever2X1) {
					for (uint k = 0; k < ARRAYSIZE(kLever2Records); k++) {
						int idx = kLever2Records[k];
						_records[idx].dir = dir;
						tick(idx);
						_records[idx].dir = 0;
					}
					clicked = true;
				}
				// Click sound -- mirrors FUN_00441cd8 lines 441e43-441e58
				// (lever 1) and 441edd-441eee (lever 2): both branches
				// of both levers play LION1.APC unconditionally.
				if (clicked)
					playSound("LION1", false);
			}

			if (!exitLoop && isWin()) {
				// Win path -- mirrors FUN_0044154c lines 38097-38108:
				// run FUN_00441724 (which plays LION3.APC and renders
				// 80 more frames) then return DAT_00496d14 = 255.
				playSound("LION3", false);
				for (int f = 0; f < kWinAnimFrames && !_engine->shouldAbort(); f++) {
					_engine->pollEvents();
					renderFrame(working);
					g_system->copyRectToScreen(working.getPixels(), working.pitch,
					                           0, 0, kScreenW, kScreenH);
					g_system->updateScreen();
					g_system->delayMillis(40);
				}
				result = kFinPuzzleWin;
				exitLoop = true;
			}
		}
		prevButton = nowButton;

		Common::KeyCode kc = _engine->getNextKey().keycode;
		if (kc == Common::KEYCODE_ESCAPE) {
			result = kFinPuzzleAbort;
			exitLoop = true;
		}

		renderFrame(working);
		g_system->copyRectToScreen(working.getPixels(), working.pitch,
		                           0, 0, kScreenW, kScreenH);
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	debugC(1, kDebugScript, "EclipsePuzzle::run: levers L1=(r2=%d,r5=%d,r6=%d) L2=(r3=%d,r4=%d) result=%d",
	      _records[2].curr, _records[5].curr, _records[6].curr,
	      _records[3].curr, _records[4].curr, result);
	return result;
}

} // namespace Atlantis
} // namespace CryOmni3D
