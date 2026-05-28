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

// NPC and prop sprite rendering for panoramic views.
//
// Data flow:
//   loadSpritePlaceList() — called after WAM load; parses the sprite list text
//       file (e.g. SPWATL1.TXT from SPRLIST\) into _spritePlaceList.
//   compositeNPCSprites() — called after loadCycloHNM(); for each entry
//       matching _currentPlaceId, reads the SPW file, decodes pixel data,
//       and blits pixels onto _warpSurface.
//
// SPW file layout (confirmed from atlantis.exe decompilation):
//
//   File header (20 bytes):
//     [0x00] uint32 file_size
//     [0x04] uint32 num_frames
//     [0x08] uint32 format  (9 = 16-bit palette; 8 = 15-bit palette)
//     [0x0C] uint32 unknown
//     [0x10] uint32 unknown
//   Frame offset table (4 bytes per frame, starting at [0x14]):
//     [0x14] uint32 frame_offsets[0]   (low 2 bits may hold flags; mask with ~3)
//     [0x18] uint32 frame_offsets[1]   (for multi-frame sprites)
//     ...
//
//   Each frame (at file + frame_offsets[n]):
//     frame[0x00..0x01] uint16  x1        (sprite bounding box left)
//     frame[0x02..0x03] uint16  (unknown)
//     frame[0x04..0x07] uint32  y_bot     (sprite bounding box bottom)
//     frame[0x08..0x0B] uint32  x2        (sprite bounding box right)
//     frame[0x0C..0x0D] uint16  y_top     (sprite bounding box top)
//     frame[0x0E..0x0F] uint16  (unknown)
//     frame[0x10..0x13] uint32  format    (same as file-level format)
//     frame[0x14..0x17] uint32  pixel_stream_off  (from frame start)
//     frame[0x18 .. 0x18+palette_bytes-1]  palette  (RGB565 entries, 2 bytes each)
//     frame[pixel_stream_off ..]           RLE pixel stream
//
//   Pixel stream opcodes (uint16 control words, little-endian):
//     ctrl == 0x0000          : no-op (skip)
//     0x0001 <= ctrl < 0x4000 : DRAW ctrl pixels; next ctrl BYTES = palette indices (1/pixel)
//     0x4000 <= ctrl < 0x8000 : BLEND (ctrl & 0x3FFF) pixels; next 2*(ctrl&0x3FFF) BYTES =
//                               pairs (lo=palette_index, hi=alpha_byte)
//     ctrl == 0x8000          : END of sprite
//     0x8001 <= ctrl < 0xC000 : ROW_SEP short — advance (ctrl & 0x7FFF) pixels in flat buffer
//     ctrl >= 0xC000          : ROW_SEP long — reads one extra uint16 word; advance =
//                               (ctrl & 0x3FFF) * 65536 + extra_word   (pixel count)
//
//   The pixel stream uses a flat-buffer model: the output pointer starts at
//   the beginning of the 2048-wide panoramic framebuffer (flat offset 0), and
//   ROW_SEP opcodes advance the pointer by an arbitrary number of pixels, which
//   may be either an intra-row skip (skipping transparent gaps within one
//   scanline) or an end-of-row jump (crossing the scanline boundary to reach
//   the sprite's x1 in the next row).  The first ROW_SEP in the stream is
//   always a large jump from flat=0 to the sprite's first pixel row.
//   The last ROW_SEP before END is a large jump to the very last pixel of
//   the framebuffer (sentinel that need not be drawn).
//
//   The bounding box fields (x1, y_top, x2, y_bot) are used for NPC mouse
//   hit-testing.  The x1/x2 values match the framebuffer column range exactly.
//   The y_top/y_bot values do NOT match framebuffer rows (they appear to be in
//   a different vertical coordinate system, offset by a sprite-dependent amount).
//
// SPWATL1.TXT line formats:
//   NPC:   placeId  0  0  -persoId  angle  spwFile
//   Torch: placeId  type  frames  spwFile
// The two are distinguished by field[1] (type): 0 → NPC entry.

#include "common/array.h"
#include "common/debug.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

#include "cryomni3d/atlantis/engine.h"

namespace CryOmni3D {
namespace Atlantis {

// ---------------------------------------------------------------------------
// loadSpritePlaceList
// ---------------------------------------------------------------------------

void CryOmni3DEngine_Atlantis::loadSpritePlaceList() {
	_spritePlaceList.clear();

	const Common::String &listFile = _wam.spwListFile;
	if (listFile.empty()) {
		debugC(1, kDebugVideo, "loadSpritePlaceList: WAM has no SPW list");
		return;
	}

	Common::SeekableReadStream *s = openBigFileStream(kFileTypeSprite, listFile);
	if (!s) {
		warning("loadSpritePlaceList: cannot open '%s'", listFile.c_str());
		return;
	}

	uint32 fileSize = (uint32)s->size();
	byte *buf = new byte[fileSize + 1];
	s->read(buf, fileSize);
	buf[fileSize] = '\0';
	delete s;

	const char *p = (const char *)buf;
	int lineNum = 0;

	while (*p) {
		// Skip blank / whitespace-only prefixes between lines.
		while (*p == '\r' || *p == '\n' || *p == ' ' || *p == '\t')
			p++;
		if (!*p)
			break;

		// Find end of this line.
		const char *eol = p;
		while (*eol && *eol != '\r' && *eol != '\n')
			eol++;
		lineNum++;

		Common::String line(p, (uint32)(eol - p));
		p = eol;

		// Skip comments.
		if (line.empty() || line[0] == '#' || line[0] == ';')
			continue;

		// Tokenise on whitespace.
		Common::Array<Common::String> tok;
		const char *tp = line.c_str();
		while (*tp) {
			while (*tp == ' ' || *tp == '\t') tp++;
			if (!*tp) break;
			const char *te = tp;
			while (*te && *te != ' ' && *te != '\t') te++;
			tok.push_back(Common::String(tp, (uint32)(te - tp)));
			tp = te;
		}

		if (tok.size() < 4) {
			debugC(3, kDebugVideo, "loadSpritePlaceList: line %d skipped (only %u tokens): '%s'",
			      lineNum, tok.size(), line.c_str());
			continue;
		}

		SpritePlaceEntry entry;
		entry.placeId = (uint16)atoi(tok[0].c_str());
		entry.type    = atoi(tok[1].c_str());
		entry.frames  = atoi(tok[2].c_str());
		entry.angle   = 0.f;
		entry.persoId = 0;
		entry.interactive = false;
		// Trailing '-' on the placeId token marks the sprite as starting
		// INACTIVE (atlantis.exe sprite-record field +0x04 = 0).  Such
		// entries only render after /set(ShowPerso=N) or /set(NewWsprAnim=N)
		// flips them active.  Atomic example: CHAPI012 vue 180 has
		//   180  1 1 atorc180.spw            (no dash -> torch, always on)
		//   180- 0 0 -617 00 a183h180.spw    (dash    -> guard, requires command)
		entry.startActive = !tok[0].empty() && tok[0][tok[0].size() - 1] != '-';
		entry.active = entry.startActive;
		entry.frame  = 0;

		// The SPW list has two row formats, distinguished by token COUNT
		// (not by type) — every SPW*.TXT in the data uses exactly 4 or 6
		// tokens, never 5 or 7:
		//   4 tokens: placeId type frames spwFile
		//   6 tokens: placeId type frames -persoId angle spwFile
		// Both formats can carry any type value.  type=1 torches are
		// usually 4-token; type=0 NPCs and type=2/3 wspranim alt-positions
		// are usually 6-token; but palais2 has a handful of 4-token type=0
		// puzzle decorations (e.g. "196 0 0 aoeil196.spw") with no perso
		// reference.  The previous type-based branching mis-parsed every
		// 6-token type=2 entry (the moving-guard alt-positions) as a
		// 4-token row — the spwFile field came out as "-617" instead of
		// the real "a183g180.spw" filename, so the moving guard never
		// rendered.
		if (tok.size() >= 6) {
			int signedId  = atoi(tok[3].c_str());
			entry.persoId = (signedId < 0) ? -signedId : signedId;
			entry.angle   = (float)atof(tok[4].c_str());
			// Interaction gate: the SPW list encodes "this is the NPC's home
			// place — clickable here, look-only elsewhere" as a leading `-`
			// on the angle token (e.g. `-00` vs. plain `00`).  atof drops
			// the sign on a zero magnitude, so check the raw token.
			entry.interactive = (!tok[4].empty() && tok[4][0] == '-');
			entry.spwFile = tok[5];
		} else {
			// 4-token row: no perso/angle, just placeId/type/frames/file.
			entry.spwFile = tok[3];
		}

		debugC(2, kDebugVideo, "  sprite: place=%u type=%d frames=%d perso=%d angle=%.2f %sfile=%s",
		      entry.placeId, entry.type, entry.frames, entry.persoId,
		      entry.angle, entry.interactive ? "[talk] " : "",
		      entry.spwFile.c_str());
		_spritePlaceList.push_back(entry);
	}

	delete[] buf;
	debugC(1, kDebugVideo, "loadSpritePlaceList: %u entries from '%s'",
	      _spritePlaceList.size(), listFile.c_str());
}

// ---------------------------------------------------------------------------
// loadSpfList
// ---------------------------------------------------------------------------
//
// SPF list file (e.g. SPFATL1.TXT) lists, for each place-to-place
// transition, which NPCs to show animated during the camera move:
//
//   fromPlace toPlace persoId spfFile
//   001 002 600 a1001002.spf
//   002 013 600 a1002013.spf
//   ...
//   #014 016 612 a8014016.spf   <- '#' starts a comment / disabled entry
//   FIN
//
// Stored verbatim into _spriteTransList; playTransitionVideo filters by
// the active (fromPlace,toPlace) pair when a transition starts.

void CryOmni3DEngine_Atlantis::loadSpfList() {
	_spriteTransList.clear();

	const Common::String &listFile = _wam.spfListFile;
	if (listFile.empty()) {
		debugC(1, kDebugVideo, "loadSpfList: WAM has no SPF list");
		return;
	}

	Common::SeekableReadStream *s = openBigFileStream(kFileTypeSprite, listFile);
	if (!s) {
		warning("loadSpfList: cannot open '%s'", listFile.c_str());
		return;
	}

	uint32 fileSize = (uint32)s->size();
	byte *buf = new byte[fileSize + 1];
	s->read(buf, fileSize);
	buf[fileSize] = '\0';
	delete s;

	const char *p = (const char *)buf;
	while (*p) {
		while (*p == '\r' || *p == '\n' || *p == ' ' || *p == '\t') p++;
		if (!*p) break;

		// Skip comment / disabled lines (leading '#').
		if (*p == '#') {
			while (*p && *p != '\r' && *p != '\n') p++;
			continue;
		}

		// Find end of line.
		const char *eol = p;
		while (*eol && *eol != '\r' && *eol != '\n') eol++;
		Common::String line(p, (uint32)(eol - p));
		p = eol;

		if (line.equalsIgnoreCase("FIN")) break;

		Common::Array<Common::String> tok;
		const char *tp = line.c_str();
		while (*tp) {
			while (*tp == ' ' || *tp == '\t') tp++;
			if (!*tp) break;
			const char *te = tp;
			while (*te && *te != ' ' && *te != '\t') te++;
			tok.push_back(Common::String(tp, (uint32)(te - tp)));
			tp = te;
		}

		if (tok.size() < 4) continue;
		SpriteTransEntry entry;
		entry.fromPlace = (uint16)atoi(tok[0].c_str());
		entry.toPlace   = (uint16)atoi(tok[1].c_str());
		entry.persoId   = atoi(tok[2].c_str());
		entry.spfFile   = tok[3];
		_spriteTransList.push_back(entry);
		debugC(3, kDebugVideo, "  spf: %u->%u perso=%d file=%s",
		      entry.fromPlace, entry.toPlace, entry.persoId, entry.spfFile.c_str());
	}

	delete[] buf;
	debugC(1, kDebugVideo, "loadSpfList: %u entries from '%s'",
	      _spriteTransList.size(), listFile.c_str());
}

// ---------------------------------------------------------------------------
// compositeNPCSprites
// ---------------------------------------------------------------------------

// Read frame_offsets[frameIdx] from the file header (frame table starts at file[0x14]).
// Returns the frame offset with the low 2 flag bits masked off.
static uint32 readSPWFrameOffset(Common::SeekableReadStream *s, uint frameIdx = 0) {
	if (!s->seek(0x14 + frameIdx * 4))
		return 0;
	return s->readUint32LE() & 0xFFFFFFFCU;
}


// Read bounding box from SPW frame header.
// frame[0x00]: x1 (uint16), frame[0x04]: y_bot (uint32),
// frame[0x08]: x2 (uint32), frame[0x0C]: y_top (uint16).
// Returns false on read error.
static bool readSPWBounds(Common::SeekableReadStream *s,
                          int16 &x1, int16 &y1, int16 &x2, int16 &y2,
                          uint frameIdx = 0) {
	uint32 frameOff = readSPWFrameOffset(s, frameIdx);
	if (!frameOff || s->err())
		return false;

	if (!s->seek(frameOff))
		return false;

	x1 = (int16)s->readUint16LE();  // frame[0x00]: x_left
	s->readUint16LE();               // frame[0x02]: skip
	y2 = (int16)s->readUint32LE();  // frame[0x04]: y_bottom
	x2 = (int16)s->readUint32LE();  // frame[0x08]: x_right
	y1 = (int16)s->readUint16LE();  // frame[0x0C]: y_top

	return !s->err();
}

static inline uint32 rgb565ToSurfaceColor(uint16 v, const Graphics::PixelFormat &fmt) {
	uint8 r = ((v >> 11) & 0x1F) * 255 / 31;
	uint8 g = ((v >>  5) & 0x3F) * 255 / 63;
	uint8 b = (v & 0x1F) * 255 / 31;
	return fmt.RGBToColor(r, g, b);
}

static inline void putSurfacePixel(Graphics::Surface &surf, int x, int y, uint32 color) {
	byte *dst = (byte *)surf.getBasePtr(x, y);
	switch (surf.format.bytesPerPixel) {
	case 2: *(uint16 *)dst = (uint16)color; break;
	case 4: *(uint32 *)dst = color;         break;
	default: break;
	}
}

// Decode SPW sprite and composite onto surf.
//
// The pixel stream uses a flat-buffer model matching the original engine:
// the output pointer starts at flat offset 0 of the panoramic framebuffer,
// and ROW_SEP opcodes advance it by an arbitrary pixel count that may cross
// row boundaries.  Pixel coordinates are derived by dividing the flat offset
// by the surface width.
static void decodeSPWPixels(Common::SeekableReadStream *spw,
                             Graphics::Surface &surf, uint frameIdx = 0) {
	// Locate frame.
	uint32 frameOff = readSPWFrameOffset(spw, frameIdx);
	if (!frameOff)
		return;

	// Read pixel-stream offset (relative to frame start).
	if (!spw->seek(frameOff + 0x14))
		return;
	uint32 pixOff = spw->readUint32LE();
	if (spw->err() || pixOff < 0x18)
		return;

	// Read palette: frame[0x18 .. frame+pixOff-1], 2 bytes per RGB565 entry.
	uint32 palBytes = pixOff - 0x18;
	uint32 palCount = palBytes / 2;
	if (palCount > 256)
		palCount = 256;

	uint16 palette[256] = {};
	if (!spw->seek(frameOff + 0x18))
		return;
	for (uint32 i = 0; i < palCount; ++i)
		palette[i] = spw->readUint16LE();
	if (spw->err())
		return;

	// Seek to pixel stream.
	if (!spw->seek(frameOff + pixOff))
		return;

	const Graphics::PixelFormat &fmt = surf.format;
	const int32 surfW = surf.w;
	const int32 surfH = surf.h;
	const int64 surfArea = (int64)surfW * surfH;

	// Flat pixel offset into the framebuffer, starting at 0.
	// The first ROW_SEP in the stream is a large jump from 0 to the sprite's
	// first actual pixel position (matching the original engine where param_4
	// is initialized to the buffer base address).
	int64 curFlat = 0;

	while (!spw->eos()) {
		uint16 ctrl = spw->readUint16LE();

		if (ctrl == 0x8000)
			break;  // END

		if (ctrl >= 0xC000) {
			// ROW_SEP long form: advance = (ctrl & 0x3FFF) * 65536 + extra_word.
			// Derived from atlantis.exe FUN_00452bde:
			//   param_4 += CONCAT22((short)(((uVar6 & 0x7ffe) << 0xf) >> 0x10), *puVar8)
			// where uVar6 = (ctrl * 2) & 0xFFFF, yielding high = ctrl & 0x3FFF.
			uint16 extra = spw->readUint16LE();
			int64 advance = (int64)(ctrl & 0x3FFF) * 65536 + extra;
			curFlat += advance;
			if (curFlat >= surfArea)
				break;
		} else if (ctrl >= 0x8000) {
			// ROW_SEP short form: advance = ctrl & 0x7FFF pixels.
			// Derived from: param_4 = (ushort*)((int)param_4 + uVar6)
			// where uVar6 = (ctrl * 2) & 0xFFFF bytes, i.e. (ctrl & 0x7FFF) pixels.
			curFlat += (ctrl & 0x7FFF);
			if (curFlat >= surfArea)
				break;
		} else if (ctrl >= 0x4000) {
			// BLEND: (ctrl & 0x3FFF) pixels; 2 bytes each (palette index, factor).
			// atlantis.exe FUN_00452bde/FUN_00452d39: the palette colour is
			// premultiplied; the framebuffer contributes (32-factor)/32 of
			// itself via the SPW blend tables (see blendSpwPixel565).
			int count = ctrl & 0x3FFF;
			for (int i = 0; i < count && !spw->eos(); ++i) {
				uint8 idx    = spw->readByte();
				uint8 factor = spw->readByte();
				if (curFlat >= 0 && curFlat < surfArea) {
					int cx = (int)(curFlat % surfW);
					int cy = (int)(curFlat / surfW);
					uint16 src = (idx < palCount) ? palette[idx] : 0;
					// The panorama framebuffer is RGB565; blend in that space.
					uint16 dst = ((const uint16 *)surf.getBasePtr(cx, cy))[0];
					uint16 out = blendSpwPixel565(src, dst, factor);
					putSurfacePixel(surf, cx, cy, rgb565ToSurfaceColor(out, fmt));
				}
				++curFlat;
			}
		} else if (ctrl > 0) {
			// DRAW: ctrl pixels; 1 byte each (palette index).
			int count = ctrl;
			for (int i = 0; i < count && !spw->eos(); ++i) {
				uint8 idx = spw->readByte();
				if (curFlat >= 0 && curFlat < surfArea) {
					int cx = (int)(curFlat % surfW);
					int cy = (int)(curFlat / surfW);
					uint16 c = (idx < palCount) ? palette[idx] : 0;
					putSurfacePixel(surf, cx, cy, rgb565ToSurfaceColor(c, fmt));
				}
				++curFlat;
			}
		}
		// ctrl == 0: no-op, continue
	}
}

void CryOmni3DEngine_Atlantis::rebuildNpcPlaceBounds() {
	// Always clear, even if !_warpLoaded — leaving stale bounds from a
	// previous place is what would let npcPanoHitTest false-positive after
	// a transition where loadCycloHNM hasn't completed yet.
	_npcPlaceBounds.clear();
	if (!_warpLoaded)
		return;
	for (const SpritePlaceEntry &entry : _spritePlaceList) {
		if (entry.placeId != (uint16)_currentPlaceId)
			continue;
		// Click target: a real perso AND marked interactive.  Skip the
		// non-interactive "distance" sprites (NPC drawn but too far to talk
		// -- recompositeSpriteLayer still draws those; only the click list is
		// gated here) and any perso currently hidden.
		//
		// Type filter: type-0 is the normal NPC click target; type-3 are
		// script-managed sprites (rope at the Cockerel inn vue 158, palace
		// traps at vue 194/215, Echelle character at vue 114) that the data
		// flags as interactive via the `-00` angle prefix.  Those need to be
		// clickable so /con(ClicPerso=N) sections like CHAPI015 section 203
		// (`/con*(ClicPerso=277)&(ObjetEnMain=262)` -- cut the rope with the
		// knife) can fire.  Atlantis.exe's hit-test loop iterates every
		// sprite regardless of type; only the pickup branch (FUN_004278a0)
		// then gates on `sprite.type != 3`, leaving the click action
		// available for the generic /con scan to consume.
		if (entry.persoId <= 0 || !entry.interactive)
			continue;
		if (entry.type != 0 && entry.type != 3)
			continue;
		// Invisible sprites must not be clickable.  Atlantis.exe's perso
		// hit-test (FUN_004261a0) only considers sprites whose active flag
		// (record +0x04) is set -- the same flag recompositeSpriteLayer
		// gates rendering on.  Sprites that start inactive via the leading
		// dash on the placeId token (e.g. CHAPI015's `151- 0 0 -579 -00
		// a151p151.spw`) are extras the chapter never shows, but they were
		// landing in the click list and stealing clicks from real NPCs
		// behind them -- CHAPI015's Lascoyt menu wouldn't open because
		// perso 579 at vue 151 was eating the click and producing "no
		// options found" (no /con(ClicPerso=579) exists anywhere).
		if (!entry.active)
			continue;

		bool hidden = false;
		for (int h : _hiddenPersos) if (h == entry.persoId) { hidden = true; break; }
		if (hidden) continue;

		Common::SeekableReadStream *spw =
		    openBigFileStream(kFileTypeSpriteCyclo, entry.spwFile);
		if (!spw) continue;

		// Frame 0 bounds are representative.
		int16 x1, y1, x2, y2;
		if (readSPWBounds(spw, x1, y1, x2, y2) && y2 >= y1) {
			if (x2 >= x1) {
				NPCBound nb;
				nb.persoId    = entry.persoId;
				nb.panoBounds = Common::Rect(x1, y1, x2 + 1, y2 + 1);
				_npcPlaceBounds.push_back(nb);
			} else {
				// Sprite wraps the panorama's 360-degree boundary --
				// e.g. CHAPI013 vue 126's `apotf126.spw` reports
				// x1=1931 x2=25 (the priest sprite straddles the
				// 0/2048 seam).  Split into two NPCBound entries so
				// hit-test still works on both sides.
				const int16 panoW = 2048;
				NPCBound nbR, nbL;
				nbR.persoId    = entry.persoId;
				nbR.panoBounds = Common::Rect(x1, y1, panoW, y2 + 1);
				_npcPlaceBounds.push_back(nbR);
				nbL.persoId    = entry.persoId;
				nbL.panoBounds = Common::Rect(0, y1, x2 + 1, y2 + 1);
				_npcPlaceBounds.push_back(nbL);
			}
		}
		delete spw;
	}
}

void CryOmni3DEngine_Atlantis::compositeNPCSprites() {
	if (!_warpLoaded)
		return;

	_propAnimFrames = 0;
	_propAnimFrame  = 0;
	_wsprAnimActiveIdx = -1;

	// Reset each entry at the new place to its parsed startActive baseline
	// (no-dash entries on; dashed entries off), so wspranim activations from
	// a previous place don't leak.  Persistent hide/show state from
	// _hiddenPersos is reapplied below by leaving entries with persoId in
	// the hidden list deactivated.
	for (SpritePlaceEntry &e : _spritePlaceList) {
		if (e.placeId != (uint16)_currentPlaceId) continue;
		e.active = e.startActive;
		e.frame  = 0;
		if (e.persoId > 0) {
			for (int h : _hiddenPersos)
				if (h == e.persoId) { e.active = false; break; }
			// Companion NPCs added via /set(addguest=N) override the
			// hidden list -- a guest is always visible at every place
			// they have a sprite, regardless of any prior HidePerso.
			// This is what makes Servage / Lascoyt etc. follow Seth
			// across the city/palace after the script attaches them.
			for (int g : _guests)
				if (g == e.persoId) { e.active = true; break; }
		}
	}

	// Detect the animated prop (if any) at the current place.
	//
	// The TXT `frames` field is NOT the animation cycle length — every
	// type-1 SPW in the data has 8 actual frame offsets, while the TXT
	// frames field ranges 1..6 across chapters (palais2's torches even
	// say "1", which would mean static).  The cycle length comes from the
	// SPW file header (uint32 at +0x04, equal to the count of valid frame
	// offsets at +0x14..).  Without this, palais2 torches/fountains
	// rendered as a single frozen frame; reading the actual count makes
	// them flicker again.
	//
	// Selection rule: prefer the LONGEST animation that carries a
	// persoId (the "main" animated character at the place) over the
	// nearest perso-less prop (torches etc.).  CHAPI013 vue 126 has
	// both atorc126.spw (torch, perso=0, 8 frames) and a086m126.spw
	// (priest aide, perso=529, 126 frames).  The script's WSprFrame
	// gates need the 126-frame animation -- /con sections like line 92
	// (`/con(vue=126)&(ObjetEnMain=261)&...&(Wsprframe>7)&(Wsprframe<34)`)
	// only fire on the priest's idle frame range, not the torch's
	// 8-frame loop.  Falls back to any type-1 sprite if no perso-bearing
	// candidate exists.
	int   bestNFrames = 0;
	const SpritePlaceEntry *bestEntry = nullptr;
	bool  bestHasPerso = false;
	for (const SpritePlaceEntry &entry : _spritePlaceList) {
		if (entry.placeId != (uint16)_currentPlaceId)
			continue;
		if (entry.type != 1)
			continue;
		Common::SeekableReadStream *spw =
		    openBigFileStream(kFileTypeSpriteCyclo, entry.spwFile);
		if (!spw)
			continue;
		spw->seek(0x04);
		uint32 nFrames = spw->readUint32LE();
		delete spw;
		if (nFrames <= 1 || nFrames > 256)
			continue;
		bool hasPerso = (entry.persoId > 0);
		// Perso-bearing wins over perso-less; among same group pick longer.
		if ((hasPerso && !bestHasPerso) ||
		    (hasPerso == bestHasPerso && (int)nFrames > bestNFrames)) {
			bestNFrames = (int)nFrames;
			bestEntry   = &entry;
			bestHasPerso = hasPerso;
		}
	}
	if (bestEntry) {
		_propAnimFrames = bestNFrames;
		_propAnimFrame  = 0;
		_propAnimNextMs = g_system->getMillis() + kPropAnimIntervalMs;
		_gameVars["wsprframe"] = 0;
	}

	rebuildNpcPlaceBounds();
	recompositeSpriteLayer();
}

void CryOmni3DEngine_Atlantis::recompositeSpriteLayer() {
	if (!_warpLoaded)
		return;

	// Start from the clean cyclo to avoid stacking animation frames.
	_warpSurface.copyFrom(_cycloSurface);

	// Render every sprite at the current place whose `active` flag is set.
	// This mirrors atlantis.exe's render loop (FUN at ~0x420060) which
	// gates rendering on sprite-record field +0x04 (`!= 0`).  Active state
	// starts from the parsed trailing-dash flag (no dash = active, dash =
	// inactive) and is toggled by /set(ShowPerso=N), /set(HidePerso=N),
	// /set(NewWsprAnim=K), /set(StopWsprAnim=K).
	for (const SpritePlaceEntry &entry : _spritePlaceList) {
		if (entry.placeId != (uint16)_currentPlaceId)
			continue;
		if (!entry.active)
			continue;

		Common::SeekableReadStream *spw =
		    openBigFileStream(kFileTypeSpriteCyclo, entry.spwFile);
		if (!spw) {
			warning("recompositeSpriteLayer: cannot open '%s'", entry.spwFile.c_str());
			continue;
		}

		// Frame index per sprite type:
		//   type=0 NPC: frame 0 (static).
		//   type=1 ambient prop: cycles through _propAnimFrame, then
		//                        wraps modulo this sprite's own frame
		//                        count (different type-1 sprites at the
		//                        same place can have different lengths --
		//                        CHAPI013 vue 126 has atorc126 with 8
		//                        frames AND a086m126 with 126).
		//   type>=2 wspranim: entry's own per-sprite frame counter
		//                     (incremented by the timer tick, exposed as
		//                     the script's WSPRFRAME variable each tick).
		uint frameIdx;
		if (entry.type == 1 && _propAnimFrames > 0) {
			spw->seek(0x04);
			uint32 nF = spw->readUint32LE();
			frameIdx = (nF > 0) ? _propAnimFrame % nF : 0;
		} else if (entry.type >= 2) {
			spw->seek(0x04);
			uint32 nFrames = spw->readUint32LE();
			int wf = entry.frame;
			if (wf < 0) wf = 0;
			frameIdx = (nFrames > 0) ? (uint)wf % nFrames : 0;
		} else {
			frameIdx = 0;
		}

		int16 x1, y1, x2, y2;
		if (!readSPWBounds(spw, x1, y1, x2, y2, frameIdx)) {
			warning("recompositeSpriteLayer: bad header in '%s' frame %u",
			        entry.spwFile.c_str(), frameIdx);
			delete spw;
			continue;
		}

		debugC(5, kDebugVideo, "recompositeSpriteLayer: place=%u %s frame=%u bounds [%d,%d]-[%d,%d]",
		      entry.placeId, entry.spwFile.c_str(), frameIdx, x1, y1, x2, y2);

		decodeSPWPixels(spw, _warpSurface, frameIdx);
		delete spw;
	}
}

// ---------------------------------------------------------------------------
// compositeSpfFrame — used by playTransitionVideo
// ---------------------------------------------------------------------------
//
// SPF files share the SPW on-disk format (file_size / num_frames / format
// header followed by per-frame palette + RLE pixel stream), so we drive
// the same decoder against the transition video's 640×480 frame surface.
// Out-of-range frameIdx (transition video longer than the SPF animation)
// degrades to a no-op: each SPF is at most ~39 frames; if the HNM has
// more, the last frame stays on screen instead of glitching.

void CryOmni3DEngine_Atlantis::compositeSpfFrame(Common::SeekableReadStream *spf,
                                                  uint frameIdx,
                                                  Graphics::Surface &dst) {
	if (!spf) return;

	// Bounds check against header[0x04] = num_frames.
	if (!spf->seek(4)) return;
	uint32 numFrames = spf->readUint32LE();
	if (frameIdx >= numFrames) frameIdx = numFrames - 1;

	decodeSPWPixels(spf, dst, frameIdx);
}

} // namespace Atlantis
} // namespace CryOmni3D
