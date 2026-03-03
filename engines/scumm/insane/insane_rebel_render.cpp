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

#include "common/system.h"
#include "common/memstream.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/smush_font.h"

#include "scumm/insane/insane_rebel.h"

namespace Scumm {

// External codec functions from codec1.cpp
extern void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
extern void smushDecodeUncompressed(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);


// ======================= Embedded Frame Codec Decoders =======================
// These implement the retail codec functions FUN_0042BD60, FUN_0042BBF0, FUN_0042B5F0

void InsaneRebel2::decodeCodec21(byte *dst, const byte *src, int width, int height) {
	// Codec 21/44: Line Update codec (FUN_0042BD60)
	// Format: each line has 2-byte size header, then pairs of (skip, count+1, literal_bytes)
	for (int row = 0; row < height; row++) {
		int lineDataSize = READ_LE_UINT16(src);
		src += 2;
		const byte *lineEnd = src + lineDataSize;
		byte *lineDst = dst + row * width;
		int x = 0;

		while (src < lineEnd && x < width) {
			int skip = READ_LE_UINT16(src);
			src += 2;
			x += skip;
			if (src >= lineEnd)
				break;

			int count = READ_LE_UINT16(src) + 1;
			src += 2;
			while (count-- > 0 && x < width && src < lineEnd) {
				lineDst[x++] = *src++;
			}
		}
		src = lineEnd;
	}
}

void InsaneRebel2::decodeCodec23(byte *dst, const byte *src, int width, int height, int dataSize) {
	// Codec 23: Skip/Copy with embedded RLE (FUN_0042BBF0)
	// Format: each line has 2-byte size, then pairs of (skip, runSize, RLE_data)
	const byte *dataEnd = src + dataSize;

	for (int row = 0; row < height && src < dataEnd; row++) {
		int lineDataSize = READ_LE_UINT16(src);
		src += 2;
		const byte *lineEnd = src + lineDataSize;
		byte *lineDst = dst + row * width;
		int x = 0;

		while (src < lineEnd && x < width) {
			int skip = READ_LE_UINT16(src);
			src += 2;
			x += skip;
			if (src >= lineEnd || x >= width)
				break;

			int runSize = READ_LE_UINT16(src);
			src += 2;

			// Decode RLE within this run
			const byte *runEnd = src + runSize;
			while (src < runEnd && x < width) {
				byte code = *src++;
				int num = (code >> 1) + 1;
				if (num > width - x)
					num = width - x;

				if (code & 1) {
					// RLE run
					byte color = (src < runEnd) ? *src++ : 0;
					for (int i = 0; i < num && x < width; i++) {
						lineDst[x++] = color;
					}
				} else {
					// Literal run
					for (int i = 0; i < num && x < width && src < runEnd; i++) {
						lineDst[x++] = *src++;
					}
				}
			}
			src = runEnd;
		}
		src = lineEnd;
	}
}

void InsaneRebel2::decodeCodec45(byte *dst, const byte *src, int width, int height, int dataSize) {
	// Codec 45: RA2-specific BOMP RLE with variable header (FUN_0042B5F0)
	// May have a 6-byte sub-header starting with "01 FE"

	debug("Rebel2: Codec 45 first 20 bytes: %02X %02X %02X %02X %02X %02X | %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7],
		src[8], src[9], src[10], src[11], src[12], src[13], src[14], src[15],
		src[16], src[17], src[18], src[19]);

	// Probe for header offset
	int headerSkip = 0;
	bool foundValidOffset = false;

	// Check for known 6-byte header pattern: 01 FE XX XX XX XX
	if (dataSize > 6 && src[0] == 0x01 && src[1] == 0xFE) {
		headerSkip = 6;
		debug("Rebel2: Codec 45 found 01 FE header, skipping 6 bytes");
		foundValidOffset = true;
	}

	// If no known header found, probe offsets 0, 2, 4, 6 to find valid RLE start
	if (!foundValidOffset) {
		for (int testOffset = 0; testOffset <= 6 && testOffset + 2 <= dataSize; testOffset += 2) {
			int testLineSize = READ_LE_UINT16(src + testOffset);
			// A valid first line size should be: > 0, <= width*2
			if (testLineSize > 0 && testLineSize <= width * 2 && testLineSize < dataSize - testOffset) {
				// Validate by summing line sizes
				int sumTest = 0;
				int linesTest = 0;
				const byte *testPtr = src + testOffset;
				bool validSum = true;

				while (linesTest < height && testPtr + 2 <= src + dataSize) {
					int ls = READ_LE_UINT16(testPtr);
					if (ls <= 0 || ls > width * 2) {
						validSum = false;
						break;
					}
					sumTest += ls + 2;
					testPtr += ls + 2;
					linesTest++;
				}

				// Accept if we got close to expected number of lines
				if (validSum && linesTest >= height - 1) {
					headerSkip = testOffset;
					foundValidOffset = true;
					debug("Rebel2: Codec 45 found valid RLE at offset %d (tested %d lines)", testOffset, linesTest);
					break;
				}
			}
		}
	}

	if (!foundValidOffset) {
		debug("Rebel2: Codec 45 couldn't find valid RLE offset, using offset 0");
	}

	const byte *srcPtr = src + headerSkip;
	const byte *dataEnd = src + dataSize;

	// Check if this is per-line RLE or continuous RLE
	int firstVal = READ_LE_UINT16(srcPtr);
	bool perLineMode = (firstVal > 0 && firstVal <= width * 2);

	if (perLineMode) {
		debug("Rebel2: Codec 45 using per-line RLE (firstLineSize=%d)", firstVal);
		for (int row = 0; row < height && srcPtr < dataEnd; row++) {
			int lineSize = READ_LE_UINT16(srcPtr);
			srcPtr += 2;
			if (lineSize <= 0 || lineSize > (int)(dataEnd - srcPtr))
				break;

			const byte *lineEnd = srcPtr + lineSize;
			byte *rowDst = dst + row * width;
			int x = 0;

			while (srcPtr < lineEnd && x < width) {
				byte ctrl = *srcPtr++;
				int count = (ctrl >> 1) + 1;
				if (ctrl & 1) {
					byte color = (srcPtr < lineEnd) ? *srcPtr++ : 0;
					for (int i = 0; i < count && x < width; i++)
						rowDst[x++] = color;
				} else {
					for (int i = 0; i < count && x < width && srcPtr < lineEnd; i++)
						rowDst[x++] = *srcPtr++;
				}
			}
			srcPtr = lineEnd;
		}
	} else {
		// Continuous BOMP RLE (no per-line headers)
		debug("Rebel2: Codec 45 using continuous BOMP RLE");
		for (int row = 0; row < height && srcPtr < dataEnd; row++) {
			byte *rowDst = dst + row * width;
			int x = 0;

			while (x < width && srcPtr < dataEnd) {
				byte ctrl = *srcPtr++;
				int count = (ctrl >> 1) + 1;

				if (ctrl & 1) {
					// RLE fill
					byte color = (srcPtr < dataEnd) ? *srcPtr++ : 0;
					for (int i = 0; i < count && x < width; i++) {
						rowDst[x++] = color;
					}
				} else {
					// Literal copy
					for (int i = 0; i < count && x < width && srcPtr < dataEnd; i++) {
						rowDst[x++] = *srcPtr++;
					}
				}
			}
		}
	}

	// Count non-zero pixels for debug
	int nonZero = 0;
	for (int i = 0; i < width * height; i++) {
		if (dst[i] != 0)
			nonZero++;
	}
	debug("Rebel2: Decoded codec 45: %dx%d, %d non-zero (%d%%)",
		width, height, nonZero, (nonZero * 100) / (width * height));
}

void InsaneRebel2::renderEmbeddedFrame(byte *renderBitmap, const EmbeddedSanFrame &frame, int userId) {
	// Render the decoded embedded frame to the video buffer
	// Skip immediate draw for handlers that render HUD during post-processing:
	// - Handler 7/8: Ship direction sprites selected based on direction
	// - Handler 0x26/0x19: Cockpit HUD positioned based on mouse/crosshair
	//
	// Exception: Handler 25 (0x19) background overlays (par4/userId=4, 6, 7) should draw immediately.
	// These complete the visual scene and are NOT positioned by mouse/crosshair.
	bool skipImmediateDraw = (_rebelHandler == 7 || _rebelHandler == 8 ||
	                          _rebelHandler == 0x26 || _rebelHandler == 0x19);

	// Handler 25 overlays:
	// - userId 4 (corridor overlay): Draw during procPostRendering at view offset, NOT immediately
	// - userId 6, 7 (static overlays): Draw immediately (they don't move)
	if (_rebelHandler == 0x19 && (userId == 6 || userId == 7)) {
		skipImmediateDraw = false;
		debug("Rebel2: Handler 25 static overlay userId=%d - forcing immediate draw", userId);
	}
	// userId 4 should NOT draw immediately - it will be drawn at view offset each frame

	if (!frame.valid || !renderBitmap || skipImmediateDraw) {
		if (skipImmediateDraw && frame.valid) {
			debug("Rebel2: Skipped immediate draw for Handler %d HUD %d (will render during post-processing)",
				_rebelHandler, userId);
		}
		return;
	}

	int pitch = (_player && _player->_width > 0) ? _player->_width : 320;
	int bufHeight = (_player && _player->_height > 0) ? _player->_height : 200;

	for (int y = 0; y < frame.height && (frame.renderY + y) < bufHeight; y++) {
		for (int x = 0; x < frame.width && (frame.renderX + x) < pitch; x++) {
			byte pixel = frame.pixels[y * frame.width + x];
			if (pixel != 0 && pixel != 231) {  // 0 and 231 = transparent
				int destX = frame.renderX + x;
				int destY = frame.renderY + y;
				if (destX >= 0 && destY >= 0) {
					renderBitmap[destY * pitch + destX] = pixel;
				}
			}
		}
	}
	debug("Rebel2: Rendered embedded HUD %d at (%d,%d)", userId, frame.renderX, frame.renderY);
}

void InsaneRebel2::loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) {
	// Validate userId - Level 3 uses slots 0-11, allow up to 15 for safety
	if (userId < 0 || userId > 15 || !animData || size < 32) {
		debug("Rebel2: Invalid embedded SAN: userId=%d, size=%d", userId, size);
		return;
	}
	
	Common::MemoryReadStream stream(animData, size);
	
	// Read ANIM header
	uint32 animTag = stream.readUint32BE();
	if (animTag != MKTAG('A','N','I','M')) {
		debug("Rebel2: Embedded SAN missing ANIM tag, got 0x%08X", animTag);
		return;
	}
	uint32 animSize = stream.readUint32BE();
	debug("Rebel2: Parsing embedded ANIM: userId=%d, reported size=%u, actual=%d", userId, animSize, size - 8);
	
	// Iterate through chunks to find FRME -> FOBJ
	while (!stream.eos() && stream.pos() < size) {
		uint32 tag = stream.readUint32BE();
		uint32 chunkSize = stream.readUint32BE();
		int32 nextChunkPos = stream.pos() + chunkSize;

		if (tag == MKTAG('F','R','M','E')) {
			// Iterate sub-chunks in FRME
			while (stream.pos() < nextChunkPos && !stream.eos()) {
				uint32 subTag = stream.readUint32BE();
				uint32 subSize = stream.readUint32BE();
				int32 nextSubPos = stream.pos() + subSize;

				if (subTag == MKTAG('F','O','B','J')) {
					// Found FOBJ - Embedded HUD Frame
					// Dump raw FOBJ bytes for analysis
					int32 fobjStart = stream.pos();
					byte rawHeader[20];
					int headerBytesToRead = MIN((int)subSize, 20);
					stream.read(rawHeader, headerBytesToRead);
					stream.seek(fobjStart);  // Reset to read normally

					debug("Rebel2: Raw FOBJ header (%d bytes): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
						headerBytesToRead,
						rawHeader[0], rawHeader[1], rawHeader[2], rawHeader[3],
						rawHeader[4], rawHeader[5], rawHeader[6], rawHeader[7],
						rawHeader[8], rawHeader[9], rawHeader[10], rawHeader[11],
						rawHeader[12], rawHeader[13], rawHeader[14], rawHeader[15],
						rawHeader[16], rawHeader[17], rawHeader[18], rawHeader[19]);

					// Read FOBJ header
					int codec = stream.readUint16LE();
					int left = stream.readUint16LE();
					int top = stream.readUint16LE();
					int width = stream.readUint16LE();
					int height = stream.readUint16LE();
					stream.readUint16LE();  // unknown
					stream.readUint16LE();  // unknown

					debug("Rebel2: Embedded HUD frame: userId=%d, %dx%d at (%d,%d), codec=%d",
						userId, width, height, left, top, codec);

					// Skip high-resolution frames - ScummVM runs at 320x200
					// If frame dimensions exceed low-res screen size, it's high-res data
					if (width > 400 || height > 250) {
						debug("Rebel2: SKIPPING high-res embedded frame: userId=%d, %dx%d (exceeds 400x250)",
							userId, width, height);
						stream.seek(nextSubPos);
						continue;
					}

					// Allocate storage for the decoded frame
					EmbeddedSanFrame &frame = _rebelEmbeddedHud[userId];

					if (width > 0 && height > 0 && width <= 800 && height <= 480) {
						if (frame.width != width || frame.height != height || !frame.pixels) {
							free(frame.pixels);
							frame.pixels = (byte *)malloc(width * height);
							frame.width = width;
							frame.height = height;
						}
						// Clear buffer before decode (important for delta codecs)
						memset(frame.pixels, 0, width * height);

						// Update render position from FOBJ header
						frame.renderX = left;
						frame.renderY = top;
						
						// Read the raw FOBJ data
						int32 dataSize = subSize - 14;
						if (dataSize > 0) {
							byte *fobjData = (byte *)malloc(dataSize);
							stream.read(fobjData, dataSize);

							// Decode based on codec - use extracted helper functions (FUN_0042BD60, etc.)
							if (codec == 1 || codec == 3) {
								// Codec 1/3: RLE - use existing decoder (FUN_0042C590)
								smushDecodeRLE(frame.pixels, fobjData, 0, 0, width, height, width);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec %d/RLE): %dx%d", codec, width, height);
							} else if (codec == 20) {
								// Codec 20: Uncompressed (FUN_0042C400)
								smushDecodeUncompressed(frame.pixels, fobjData, 0, 0, width, height, width);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec 20/raw): %dx%d", width, height);
							} else if (codec == 21 || codec == 44) {
								// Codec 21/44: Line update (FUN_0042BD60)
								decodeCodec21(frame.pixels, fobjData, width, height);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec %d/line update): %dx%d", codec, width, height);
							} else if (codec == 45) {
								// Codec 45: RA2-specific BOMP RLE (FUN_0042B5F0)
								decodeCodec45(frame.pixels, fobjData, width, height, dataSize);
								frame.valid = true;
							} else if (codec == 23) {
								// Codec 23: Skip/copy with embedded RLE (FUN_0042BBF0)
								decodeCodec23(frame.pixels, fobjData, width, height, dataSize);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec 23/skip-RLE): %dx%d", width, height);
							} else {
								debug("Rebel2: TODO: Decode codec %d for embedded HUD", codec);
								frame.valid = false;
							}

							// Count non-zero pixels to verify frame has content
							if (frame.valid) {
								int nonZeroPixels = 0;
								for (int i = 0; i < width * height; i++) {
									if (frame.pixels[i] != 0)
										nonZeroPixels++;
								}
								debug("Rebel2: Frame userId=%d has %d non-zero pixels (%d%%)",
									userId, nonZeroPixels, (nonZeroPixels * 100) / (width * height));
							}

							// Render the decoded frame to the video buffer
							renderEmbeddedFrame(renderBitmap, frame, userId);

							free(fobjData);
						}
					}
					
					// Done with FOBJ - assume only one relevant frame per embedded SAN
					stream.seek(nextChunkPos);
					goto end_parsing;
				} else {
					// Skip other sub-chunks (AHDR inside FRME?) or padding
					stream.seek(nextSubPos);
					if (subSize & 1)
						stream.skip(1);
				}
			}
		} else {
			// Skip non-FRME chunks (AHDR, etc at top level)
			stream.seek(nextChunkPos);
			if (chunkSize & 1)
				stream.skip(1);
		}
	}
	
	debug("Rebel2: No FOBJ found in embedded SAN userId=%d", userId);

end_parsing:;
}

// Spawn explosion into the shared 5-slot system.
// In the original, each handler has its own spawn logic inside its enemy processing function:
//   Handler 0x26: FUN_40A2E0 (0x40A2E0) — spawns in slot arrays DAT_0044368e[]
//   Handler 8:    FUN_4028C5 (0x4028C5) — spawns in slot arrays DAT_0043f854[]
//   Handler 7:    FUN_40F628 (0x40F628) — spawns in slot arrays DAT_00443770[]
//   Handler 25:   FUN_41E7C2 (0x41E7C2) — spawns in slot arrays DAT_0045792c[]
// All share the same logic: find first free slot (counter==0), set counter=10,
// scale=objectHalfWidth, position=enemy center, velocity=0.
void InsaneRebel2::spawnExplosion(int x, int y, int objectHalfWidth) {
	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active || _explosions[i].counter <= 0) {
			_explosions[i].active = true;
			_explosions[i].counter = 10;
			_explosions[i].x = x;
			_explosions[i].y = y;
			_explosions[i].scale = objectHalfWidth;
			break;
		}
	}
}

// Get max shot duration from level table (DAT_0047e0f0 indexed by DAT_0047a7fa/DAT_0047a7f8)
// The original reads from a per-level per-difficulty table. The field at offset +0x00
// (DAT_0047e0f0) is the first field of each record — our laserDelay field.
// This value is used both as the initial shot counter AND as maxFrames for beam rendering.
int16 InsaneRebel2::getShotMaxDuration() {
	LevelDifficultyParams params = getDifficultyParams();
	// laserDelay = DAT_0047e0f0 field: shot duration in frames
	// Clamp to reasonable range to avoid division by zero or extreme beams
	int16 duration = params.laserDelay;
	if (duration <= 0)
		duration = 4;  // Fallback for -1 entries (disabled levels)
	return duration;
}

// Dispatcher - calls appropriate spawn function based on current handler
void InsaneRebel2::spawnShot(int x, int y) {
	switch (_rebelHandler) {
	case 0x26:  // Turret
		spawnTurretShot(x, y);
		break;
	case 8:     // Vehicle
		spawnVehicleShot(x, y);
		break;
	case 7:     // Space combat
		spawnSpaceShot(x, y);
		break;
	case 25:    // Speeder bike - uses turret shot array with different gun position
		spawnHandler25Shot(x, y);
		break;
	default:
		// Legacy fallback
		for (int i = 0; i < 2; i++) {
			if (!_shots[i].active) {
				_shots[i].active = true;
				_shots[i].counter = getShotMaxDuration();
				_shots[i].x = x + _viewX;
				_shots[i].y = y + _viewY;
				break;
			}
		}
		break;
	}
}

// Handler 0x26 Turret shot spawn (based on FUN_4089AB lines 127-140)
void InsaneRebel2::spawnTurretShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter == 0) {
			// FUN_0041189e(-(ushort)(DAT_004436de == 5) & 7, i + 1, 0x7f, 0, 0)
			// levelType 5: BLAST.SAD (slot 0), otherwise: TBLAST.SAD (slot 7)
			playSfx((_rebelLevelType == 5) ? 0 : 7, 127, 0);

			_turretShots[i].counter = getShotMaxDuration();
			_turretShots[i].seqNum = _turretShotSeqCounter;
			_turretShotSeqCounter++;
			_turretShots[i].targetX = x + _viewX;  // DAT_0044366e in original
			_turretShots[i].targetY = y + _viewY;  // DAT_00443670 in original
			break;
		}
	}
}

// Handler 8 Vehicle shot spawn (based on FUN_401CCF lines 65-69)
void InsaneRebel2::spawnVehicleShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_vehicleShots[i].counter == 0) {
			// FUN_0041189e(6, local_c + 1, 0x7f, 0, 0) — HBLAST.SAD
			playSfx(6, 127, 0);
			_vehicleShots[i].counter = getShotMaxDuration();
			_vehicleShots[i].targetX = x + _viewX;
			_vehicleShots[i].targetY = y + _viewY;
			break;
		}
	}
}

// Handler 25 on-foot shot spawn (based on FUN_0041db5e lines 170-190)
// Gun position computed from GRD002 character sprite.
// Original stores: DAT_0045791c[i] = gunOffsetTable[spriteIdx] + DAT_00457910 - DAT_0045790c
//                  DAT_00457920[i] = gunYTable[spriteIdx] + DAT_00457912 - DAT_0045790e
// Render adds view offset back, so screen gun = table[idx] + spriteOffset.
void InsaneRebel2::spawnHandler25Shot(int x, int y) {
	// Handler 25 can only shoot when uncovered (damage == 0)
	if (_rebelDamageLevel != 0) {
		return;  // Can't shoot while taking cover
	}

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter == 0) {
			// FUN_0041189e(6, local_1c + 1, 0x7f, 0, 0) — HBLAST.SAD
			playSfx(6, 127, 0);

			_turretShots[i].counter = getShotMaxDuration();
			_turretShots[i].seqNum = _turretShotSeqCounter;
			_turretShotSeqCounter++;

			// Target position is where player clicked (screen coords)
			_turretShots[i].targetX = x;
			_turretShots[i].targetY = y;

			// Compute gun position from GRD002 character sprite.
			// Original uses per-direction lookup tables DAT_004578a6/DAT_004578c6.
			// We approximate from the NUT sprite center + directional offset.
			if (_grd002Sprite && _grd002Sprite->getNumChars() > 0) {
				// Compute current sprite index (same logic as renderHandler25Ship)
				int spriteIdx;
				if (_rebelDamageLevel == 0) {
					// Uncovered: compute from crosshair position zones
					int16 areaLeft = (_corridorLeftX > 0) ? _corridorLeftX : 0;
					int16 areaRight = (_corridorRightX > 0) ? _corridorRightX : 320;
					int16 areaTop = (_corridorTopY > 0) ? _corridorTopY : 0;
					int16 areaBottom = (_corridorBottomY > 0) ? _corridorBottomY : 180;
					int areaWidth = areaRight - areaLeft;
					int areaHeight = areaBottom - areaTop;
					int zoneWidth = (areaWidth > 0) ? (areaWidth + 3) / 4 : 80;
					int zoneHeight = (areaHeight > 0) ? areaHeight / 2 : 90;
					int xZone = (zoneWidth > 0) ? ((zoneWidth / 2) + (x - areaLeft)) / zoneWidth : 2;
					int yZone = (zoneHeight > 0) ? ((zoneHeight / 2) + (y - areaTop)) / zoneHeight : 0;
					if (xZone < 0)
						xZone = 0;
					if (xZone > 4)
						xZone = 4;
					if (yZone < 0)
						yZone = 0;
					if (yZone > 1)
						yZone = 1;
					if (_rebelFlightDir == (yZone & 1)) {
						xZone = 4 - xZone;
					}
					spriteIdx = yZone * 5 + xZone + 5;
				} else {
					spriteIdx = (_rebelFlightDir == 0) ? (5 - _rebelDamageLevel) : (25 - _rebelDamageLevel);
				}
				int numSprites = _grd002Sprite->getNumChars();
				if (spriteIdx < 0)
					spriteIdx = 0;
				if (spriteIdx >= numSprites)
					spriteIdx = numSprites - 1;

				// Get sprite rendering position (same as in renderHandler25Ship)
				int16 spriteXOffset = _grd002Sprite->getCharXOffset(spriteIdx);
				int16 spriteYOffset = _grd002Sprite->getCharYOffset(spriteIdx);
				int spriteW = _grd002Sprite->getCharWidth(spriteIdx);
				int spriteH = _grd002Sprite->getCharHeight(spriteIdx);
				bool shouldMirror = (_rebelFlightDir != 0 && _rebelDamageLevel == 0);

				int drawX;
				if (shouldMirror) {
					drawX = _rebelViewOffset2X + (320 - spriteW - spriteXOffset);
				} else {
					drawX = _rebelViewOffset2X + spriteXOffset;
				}
				int drawY = spriteYOffset + _rebelViewOffset2Y;

				// Gun barrel is approximately at the character's hand level:
				// X: center of sprite ± directional offset toward the target
				// Y: about 60% down the sprite height (hand/arm level)
				_turretShots[i].gunX = drawX + spriteW / 2;
				_turretShots[i].gunY = drawY + (spriteH * 3) / 5;
			} else {
				// Fallback: approximate center-bottom of character area
				_turretShots[i].gunX = _rebelViewOffset2X + 160;
				_turretShots[i].gunY = _rebelViewOffset2Y + 140;
			}

			debug("Rebel2 Handler25: Spawned shot %d target (%d,%d) gun (%d,%d)",
				i, _turretShots[i].targetX, _turretShots[i].targetY,
				_turretShots[i].gunX, _turretShots[i].gunY);
			break;
		}
	}
}

// Handler 7 Space combat shot spawn (based on FUN_40D836 lines 146-166)
void InsaneRebel2::spawnSpaceShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_spaceShots[i].counter == 0) {
			// FUN_0041189e(6, local_2c + 1, 0x7f, 0, 0) — HBLAST.SAD
			playSfx(6, 127, 0);

			_spaceShots[i].counter = getShotMaxDuration();
			_spaceShots[i].targetX = x;  // Screen coords
			_spaceShots[i].targetY = y;

			// Calculate gun positions from direction-based lookup tables
			// In the original, these come from tables indexed by _shipDirectionIndex
			// DAT_004437c2/DAT_00443808 for left gun, DAT_0044384e/DAT_00443894 for right gun
			// For now, use simplified positions relative to ship
			int shipScreenX = 160 + ((_shipPosX - 160) >> 3);
			int shipScreenY = 105 + ((_shipPosY - 40) >> 2);

			// Gun offsets (approximate from disassembly)
			_spaceShots[i].leftGunX = shipScreenX - 28;
			_spaceShots[i].leftGunY = shipScreenY + 10;
			_spaceShots[i].rightGunX = shipScreenX + 28;
			_spaceShots[i].rightGunY = shipScreenY + 10;
			_spaceShots[i].variant = _spaceShotDirection;
			break;
		}
	}
}

void InsaneRebel2::drawTexturedLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, NutRenderer *nut, int spriteIdx, int v, bool mask231) {
	if (!nut || spriteIdx >= nut->getNumChars())
		return;

	const byte *srcData = nut->getCharData(spriteIdx);
	int texW = nut->getCharWidth(spriteIdx);
	int texH = nut->getCharHeight(spriteIdx);
	
	if (!srcData || texW <= 0 || texH <= 0)
		return;
	if (v < 0)
		v = 0;
	if (v >= texH)
		v = texH - 1;

	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;
	
	// Total length approximation for UV mapping
	int totalDist = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
	if (totalDist == 0)
		totalDist = 1;
	
	int currentDist = 0;

	for (;;) {
		if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
			// Map currentDist/totalDist to 0..texW (Run along texture width)
			int u = (currentDist * texW) / totalDist;
			if (u >= texW)
				u = texW - 1;
			
			byte color = srcData[v * texW + u];
			
			// Check for transparency (0 and optionally 231)
			if (color != 0 && (!mask231 || color != 231)) { 
				dst[y0 * pitch + x0] = color;
			}
		}
		
		if (x0 == x1 && y0 == y1)
			break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
		
		currentDist++;
	}
}

// Helper: draw a textured segment between two points using the game's original routine (FUN_00429360 port)
void drawTexturedSegment(byte *dst, int pitch, int width, int height, int param_3, int param_4, int param_5, int param_6, int param_7, const byte *param_8) {
	// Ported from FUN_00429360 (decompiled). Only 0 in texture is transparent.
	int sVar4 = 0;                // left
	int sVar1 = 0;                // top
	int sVar7 = width - 1;        // right
	int sVar10 = height - 1;      // bottom

	int px0 = param_3;
	int py0 = param_4;
	int px1 = param_5;
	int py1 = param_6;

	// Clip against screen bounds (translation of original clipping logic)
	if (px0 == px1) {
		if (px0 < sVar4 || px0 > sVar7)
			return;
	} else {
		if (px0 < sVar4) {
			if (px1 < sVar4)
				return;
			py0 = py1 + ((py0 - py1) * (sVar4 - px1)) / (px0 - px1);
			px0 = sVar4;
		} else if (px0 > sVar7) {
			if (px1 > sVar7)
				return;
			py0 = py1 + ((py0 - py1) * (sVar7 - px1)) / (px0 - px1);
			px0 = sVar7;
		}
		if (px1 < sVar4) {
			py1 = py0 + ((py1 - py0) * (sVar4 - px0)) / (px1 - px0);
			px1 = sVar4;
		} else if (px1 > sVar7) {
			py1 = py0 + ((py1 - py0) * (sVar7 - px0)) / (px1 - px0);
			px1 = sVar7;
		}
	}

	if (py0 == py1) {
		if (py0 < sVar1 || py0 > sVar10)
			return;
	} else {
		if (py0 < sVar1) {
			if (py1 < sVar1)
				return;
			px0 = px1 + ((px0 - px1) * (sVar1 - py1)) / (py0 - py1);
			py0 = sVar1;
		} else if (py0 > sVar10) {
			if (py1 > sVar10)
				return;
			px0 = px1 + ((px0 - px1) * (sVar10 - py1)) / (py0 - py1);
			py0 = sVar10;
		}
		if (py1 < sVar1) {
			px1 = px0 + ((px1 - px0) * (sVar1 - py0)) / (py1 - py0);
			py1 = sVar1;
		} else if (py1 > sVar10) {
			px1 = px0 + ((px1 - px0) * (sVar10 - py0)) / (py1 - py0);
			py1 = sVar10;
		}
	}

	int dx = px1 - px0;
	int dy = py1 - py0;
	int absdx = dx < 0 ? -dx : dx;
	int absdy = dy < 0 ? -dy : dy;

	// pointer into destination and texture
	byte *baseDst = dst;
	const byte *texPtr = param_8;

	if (absdx == 0) {
		if (absdy == 0) {
			if (*texPtr != 0)
				baseDst[py0 * pitch + px0] = *texPtr;
			return;
		}
		// vertical-ish
		int step = absdy + 1;
		int curY = py0;
		int signY = dy > 0 ? 1 : -1;
		int iVar9 = step; // adv counter
		for (int i = 0; i < step; i++) {
			if (*texPtr != 0)
				baseDst[curY * pitch + px0] = *texPtr;
			curY += signY;
			iVar9 -= param_7;
			while (iVar9 < 0) { texPtr++; iVar9 += step; }
		}
		return;
	}

	if (absdy == 0) {
		// horizontal-ish
		int step = absdx + 1;
		int curX = px0;
		int signX = dx > 0 ? 1 : -1;
		int iVar11 = step;
		for (int i = 0; i < step; i++) {
			if (*texPtr != 0)
				baseDst[py0 * pitch + curX] = *texPtr;
			curX += signX;
			iVar11 -= param_7;
			while (iVar11 < 0) { texPtr++; iVar11 += step; }
		}
		return;
	}

	// general case
	int steps = (absdx > absdy) ? absdx + 1 : absdy + 1;
	int x = px0, y = py0;
	int sx = dx > 0 ? 1 : -1;
	int sy = dy > 0 ? 1 : -1;
	int err = absdx - absdy;
	int iVar12 = steps - 1; // Original starts at majorAxis, not majorAxis+1

	for (int i = 0; i < steps; i++) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			if (*texPtr != 0)
				baseDst[y * pitch + x] = *texPtr;
		}
		int e2 = 2 * err;
		if (e2 > -absdy) { err -= absdy; x += sx; }
		if (e2 < absdx) { err += absdx; y += sy; }
		iVar12 -= param_7;
		while (iVar12 < 0) { texPtr++; iVar12 += steps; }
	}
}


// Initialize laser texture buffer from NUT sprite (FUN_0040BAB0)
// This pre-renders a sprite into a buffer that drawLaserBeam uses
void InsaneRebel2::initLaserTexture(NutRenderer *nut, int spriteIdx) {
	if (!nut || spriteIdx >= nut->getNumChars())
		return;

	// Get sprite dimensions (FUN_0040BAB0 lines 13-14)
	int16 texWidth = nut->getCharWidth(spriteIdx);
	int16 texHeight = nut->getCharHeight(spriteIdx);

	// Clamp height to max 15 pixels (FUN_0040BAB0 lines 15-17)
	if (texHeight > 15) {
		texHeight = 15;
	}

	// Free existing texture if any (FUN_0040BAB0 lines 18-20)
	freeLaserTexture();

	// Allocate new buffer (FUN_0040BAB0 line 21)
	_laserTexture.width = texWidth;
	_laserTexture.height = texHeight;
	_laserTexture.pixels = (byte *)calloc(texWidth * texHeight, 1);

	if (!_laserTexture.pixels)
		return;

	// Render sprite into buffer (FUN_0040BAB0 lines 23-24)
	// We copy the sprite data directly since it's already in the right format
	const byte *srcData = nut->getCharData(spriteIdx);
	if (srcData) {
		int srcHeight = nut->getCharHeight(spriteIdx);
		int copyHeight = MIN(texHeight, (int16)srcHeight);
		memcpy(_laserTexture.pixels, srcData, texWidth * copyHeight);
	}

	debug("Rebel2: Initialized laser texture %dx%d from sprite %d", texWidth, texHeight, spriteIdx);

	// Diagnostic: dump texture pixel stats to verify data is loaded correctly
	if (_laserTexture.pixels && texWidth > 0 && texHeight > 0) {
		int third = texWidth / 3;
		int band1 = 0, band2 = 0, band3 = 0;
		for (int row = 0; row < texHeight; row++) {
			for (int col = 0; col < texWidth; col++) {
				if (_laserTexture.pixels[row * texWidth + col] != 0) {
					if (col < third) band1++;
					else if (col < third * 2) band2++;
					else band3++;
				}
			}
		}
		debug("Rebel2: Texture non-zero pixels by band: [0-%d]=%d  [%d-%d]=%d  [%d-%d]=%d",
			third - 1, band1, third, third * 2 - 1, band2, third * 2, texWidth - 1, band3);

		// Dump first row hex (first 64 bytes)
		Common::String hexRow;
		int dumpLen = MIN(texWidth, (int16)64);
		for (int col = 0; col < dumpLen; col++) {
			hexRow += Common::String::format("%02x ", _laserTexture.pixels[col]);
		}
		debug("Rebel2: Texture row 0 (first %d): %s", dumpLen, hexRow.c_str());
	}
}

// Free laser texture buffer (FUN_0040BBD1)
void InsaneRebel2::freeLaserTexture() {
	free(_laserTexture.pixels);
	_laserTexture.pixels = nullptr;
	_laserTexture.width = 0;
	_laserTexture.height = 0;
}

// Initialize edge blend tables (FUN_410510)
// When data is nullptr, fills with default tables:
//   _edgeTable[a*256+b] = min(a,b) (symmetric identity blend)
//   _edgeTableAlt[a*256+b] = special blend for hi-res mode
// When data is non-null, loads the primary table from data+8 (upper triangle, symmetric).
void InsaneRebel2::initEdgeTable(const byte *data) {
	if (data == nullptr) {
		// Default table initialization (FUN_410510 param_1==NULL path, lines 12-36)
		for (int a = 0; a < 256; a++) {
			for (int b = a; b < 256; b++) {
				// Primary table: table[a][b] = a (i.e. min(a,b) since b >= a)
				_edgeTable[a + b * 256] = (byte)a;
				_edgeTable[b + a * 256] = (byte)a;

				// Secondary table: special blend rules (FUN_410510 lines 17-31)
				if (a < 0x10 || b > 0x4f) {
					// Outside blend range: use b if b==0, or (0xf < b && b < 0x50), or b==4
					if (b == 0 || (b > 0xf && b < 0x50) || b == 4) {
						_edgeTableAlt[a + b * 256] = (byte)b;
					} else {
						_edgeTableAlt[a + b * 256] = (byte)a;
					}
				} else {
					// Blend range [0x10..0x4f]: average of a and b
					_edgeTableAlt[a + b * 256] = (byte)((a + b) / 2);
				}
				_edgeTableAlt[b + a * 256] = _edgeTableAlt[a + b * 256];
			}
		}
		// Special entries (FUN_410510 lines 33-36)
		_edgeTable[0x42 * 256 + 0xf1] = 0x42;   // DAT_00447ff1
		_edgeTable[0x42 + 0xf0 * 256] = 0x42;   // DAT_004480f0 (symmetric)
		_edgeTable[0x41 * 256 + 0xb0] = 0x41;   // DAT_00447fb0
		_edgeTableAlt[0x41 * 256 + 0xf0] = 0x41; // DAT_00443ff0
	} else {
		// Load table from IACT data (FUN_410510 non-NULL path, lines 39-47)
		// Data format: 8-byte header + upper triangle of 256x256 symmetric table
		const byte *src = data + 8;
		for (int a = 0; a < 256; a++) {
			for (int b = a; b < 256; b++) {
				_edgeTable[a + b * 256] = *src;
				_edgeTable[b + a * 256] = *src;
				src++;
			}
		}
	}
}

// Draw edge highlight line using the edge blend table (FUN_410962)
// For each pixel along the line, reads the two adjacent pixels perpendicular to
// the line direction and uses _edgeTable[above*256+below] as the output color.
// This creates a glow/blend effect at beam edges.
//
// For horizontal-dominant lines (dx > dy), reads pixels above and below.
// For vertical-dominant lines (dy > dx), reads pixels left and right.
//
// param_1 = dst buffer info (base pointer at [0], pitch at [1], width at word [1], height at byte offset 6)
// param_2 = clip rect (or NULL for full buffer)
// param_3..param_6 = x0, y0, x1, y1 line endpoints
void InsaneRebel2::drawEdgeHighlightLine(byte *dst, int pitch, int width, int height,
                                          int16 x0, int16 y0, int16 x1, int16 y1) {
	// Clip region (FUN_410962 lines 19-30, simplified for our buffer layout)
	int16 clipLeft = 1;
	int16 clipTop = 1;
	int16 clipRight = width - 2;
	int16 clipBottom = height - 2;

	// Clip X endpoints (FUN_410962 lines 35-69)
	if (x0 == x1) {
		if (x0 < clipLeft || x0 > clipRight)
			return;
	} else {
		if (x0 < clipLeft) {
			if (x1 < clipLeft) return;
			y0 = y1 + (int16)(((int)(y0 - y1) * (int)(clipLeft - x1)) / (int)(x0 - x1));
			x0 = clipLeft;
		} else if (x0 > clipRight) {
			if (x1 > clipRight) return;
			y0 = y1 + (int16)(((int)(y0 - y1) * (int)(clipRight - x1)) / (int)(x0 - x1));
			x0 = clipRight;
		}
		if (x1 < clipLeft) {
			y1 = y0 + (int16)(((int)(y1 - y0) * (int)(clipLeft - x0)) / (int)(x1 - x0));
			x1 = clipLeft;
		} else if (x1 > clipRight) {
			y1 = y0 + (int16)(((int)(y1 - y0) * (int)(clipRight - x0)) / (int)(x1 - x0));
			x1 = clipRight;
		}
	}

	// Clip Y endpoints (FUN_410962 lines 71-106)
	if (y0 == y1) {
		if (y0 < clipTop || y0 > clipBottom)
			return;
	} else {
		if (y0 < clipTop) {
			if (y1 < clipTop) return;
			x0 = x1 + (int16)(((int)(x0 - x1) * (int)(clipTop - y1)) / (int)(y0 - y1));
			y0 = clipTop;
		} else if (y0 > clipBottom) {
			if (y1 > clipBottom) return;
			x0 = x1 + (int16)(((int)(x0 - x1) * (int)(clipBottom - y1)) / (int)(y0 - y1));
			y0 = clipBottom;
		}
		if (y1 < clipTop) {
			x1 = x0 + (int16)(((int)(x1 - x0) * (int)(clipTop - y0)) / (int)(y1 - y0));
			y1 = clipTop;
		} else if (y1 > clipBottom) {
			x1 = x0 + (int16)(((int)(x1 - x0) * (int)(clipBottom - y0)) / (int)(y1 - y0));
			y1 = clipBottom;
		}
	}

	// Calculate starting pixel address and deltas (FUN_410962 lines 107-110)
	byte *pixel = dst + y0 * pitch + x0;
	int16 dx = x1 - x0;
	int16 dy = y1 - y0;

	// Bresenham line with perpendicular neighbor lookup (FUN_410962 lines 111-270)
	// The key insight: for each pixel, the blend reads neighbors PERPENDICULAR to the line.
	// - Horizontal lines: blend pixel_above * 256 + pixel_below
	// - Vertical lines: blend pixel_left * 256 + pixel_right (reversed: left=[-1], right=[+1])
	if (dx == 0) {
		if (dy == 0) {
			// Single pixel: blend from above/below
			*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
		} else if (dy < 0) {
			// Vertical line going up: read left/right neighbors
			while (dy < 1) {
				*pixel = _edgeTable[(uint)pixel[1] + (uint)pixel[-1] * 256];
				pixel -= pitch;
				dy++;
			}
		} else {
			// Vertical line going down: read left/right neighbors
			while (dy >= 0) {
				*pixel = _edgeTable[(uint)pixel[1] + (uint)pixel[-1] * 256];
				pixel += pitch;
				dy--;
			}
		}
	} else if (dy == 0) {
		if (dx < 0) {
			// Horizontal line going left: read above/below neighbors
			while (dx < 1) {
				*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
				pixel--;
				dx++;
			}
		} else {
			// Horizontal line going right: read above/below neighbors
			while (dx >= 0) {
				*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
				pixel++;
				dx--;
			}
		}
	} else if (dx < 0 || dy < 0) {
		// Mixed negative direction cases (FUN_410962 lines 149-240)
		if (dy < 0) {
			if (dx < 0) {
				// Both negative: going up-left
				if (dx < dy) {
					// X-major (|dx| > |dy|): read above/below
					int err = (-dx) >> 1;
					int steps = 1 - dx;
					while (steps > 0) {
						*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
						pixel--;
						err += dy;
						steps--;
						if (err < 0) {
							err -= dx;
							pixel -= pitch;
						}
					}
				} else {
					// Y-major (|dy| > |dx|): read left/right
					int err = (-dy) >> 1;
					int steps = 1 - dy;
					while (steps > 0) {
						*pixel = _edgeTable[(uint)pixel[1] + (uint)pixel[-1] * 256];
						pixel -= pitch;
						err += dx;
						steps--;
						if (err < 0) {
							err -= dy;
							pixel--;
						}
					}
				}
			} else {
				// dx > 0, dy < 0: going right-up
				if (-dy < dx) {
					// X-major: read above/below
					int err = dx >> 1;
					int steps = dx + 1;
					while (steps > 0) {
						*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
						pixel++;
						err += dy;
						steps--;
						if (err < 0) {
							err += dx;
							pixel -= pitch;
						}
					}
				} else {
					// Y-major: read left/right
					int err = (-dy) >> 1;
					int steps = 1 - dy;
					while (steps > 0) {
						*pixel = _edgeTable[(uint)pixel[1] + (uint)pixel[-1] * 256];
						pixel -= pitch;
						err -= dx;
						steps--;
						if (err < 0) {
							err -= dy;
							pixel++;
						}
					}
				}
			}
		} else {
			// dx < 0, dy > 0: going left-down
			if (-dx == dy || -dx < dy) {
				// Y-major: read left/right
				int err = dy >> 1;
				int steps = dy + 1;
				while (steps > 0) {
					*pixel = _edgeTable[(uint)pixel[1] + (uint)pixel[-1] * 256];
					pixel += pitch;
					err += dx;
					steps--;
					if (err < 0) {
						err += dy;
						pixel--;
					}
				}
			} else {
				// X-major: read above/below
				int err = (-dx) >> 1;
				int steps = 1 - dx;
				while (steps > 0) {
					*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
					pixel--;
					err -= dy;
					steps--;
					if (err < 0) {
						err -= dx;
						pixel += pitch;
					}
				}
			}
		}
	} else {
		// Both positive: going right-down (FUN_410962 lines 242-270)
		if (dy < dx) {
			// X-major: read above/below
			int err = dx >> 1;
			int steps = dx + 1;
			while (steps > 0) {
				*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
				pixel++;
				err -= dy;
				steps--;
				if (err < 0) {
					err += dx;
					pixel += pitch;
				}
			}
		} else {
			// Y-major: read left/right
			int err = dy >> 1;
			int steps = dy + 1;
			while (steps > 0) {
				*pixel = _edgeTable[(uint)pixel[1] + (uint)pixel[-1] * 256];
				pixel += pitch;
				err -= dx;
				steps--;
				if (err < 0) {
					err += dy;
					pixel++;
				}
			}
		}
	}
}

// Draw laser beam using pre-initialized texture (FUN_0040BBF6)
// This is a direct port of the assembly function.
// Two-layer rendering:
//   Layer 1: Textured scanlines (beam body) via drawTexturedSegment()
//   Layer 2: Edge highlights (glow) via drawEdgeHighlightLine(), gated by _rebelDetailMode >= 0
//
// Parameters (matching FUN_0040bbf6):
//   dst, pitch, width, height: destination buffer info
//   gunX, gunY (param_3, param_4): gun/start position
//   targetX, targetY (param_5, param_6): target/end position
//   animFrame (param_7): current animation frame (shot counter)
//   maxFrames (param_8): max animation frames (shot duration)
//   widthScale (param_9): width scaling factor for perspective
//   heightScale (param_10): height/thickness multiplier
//   thickness (param_11): base line thickness
void InsaneRebel2::drawLaserBeam(byte *dst, int pitch, int width, int height,
                                  int16 gunX, int16 gunY, int16 targetX, int16 targetY,
                                  int16 animFrame, int16 maxFrames,
                                  int16 widthScale, int16 heightScale, int16 thickness) {
	// Check if laser texture is initialized
	if (!_laserTexture.pixels || _laserTexture.width <= 0 || _laserTexture.height <= 0)
		return;

	int16 texW = _laserTexture.width;
	int16 texH = _laserTexture.height;
	byte *texPixels = _laserTexture.pixels;

	// FUN_0040BBF6 line 23: sVar7 = (thickness * animFrame * 16) / maxFrames
	if (maxFrames == 0)
		maxFrames = 1;
	int16 sVar7 = (int16)(((int)thickness * (int)animFrame * 16) / (int)maxFrames);

	// FUN_0040BBF6 lines 24-25: Calculate delta with scaling
	int16 dx = targetX - gunX;
	int16 dy = targetY - gunY;
	int16 sVar6 = (int16)(((int)dx * (thickness + 1)) / (int)thickness);
	int16 sVar1 = (int16)(((int)dy * (thickness + 1)) / (int)thickness);

	// FUN_0040BBF6 lines 26-29: Calculate adjusted start and end points
	// Start point (closer to gun, adjusted by animation progress)
	int16 startX = (sVar6 + gunX) - (int16)(((int)sVar6 * 16) / (sVar7 + 16));
	int16 startY = (sVar1 + gunY) - (int16)(((int)sVar1 * 16) / (sVar7 + 16));
	// End point (closer to target)
	int16 endX = (sVar6 + gunX) - (int16)(((int)sVar6 * 16) / (widthScale + sVar7 + 16));
	int16 endY = (sVar1 + gunY) - (int16)(((int)sVar1 * 16) / (widthScale + sVar7 + 16));

	// FUN_0040BBF6 line 30: Get texture pixel pointer
	byte *local_28 = texPixels;

	debug(5, "Rebel2: drawLaserBeam gun(%d,%d) tgt(%d,%d) start(%d,%d) end(%d,%d) anim=%d/%d ws=%d hs=%d th=%d",
		gunX, gunY, targetX, targetY, startX, startY, endX, endY,
		animFrame, maxFrames, widthScale, heightScale, thickness);

	// FUN_0040BBF6 lines 31-32: Calculate abs differences (FUN_004356e4 = abs)
	int iVar2 = abs(startY - endY);  // |dy| of beam
	int iVar3 = abs(startX - endX);  // |dx| of beam

	// FUN_0040BBF6 line 33: Choose rendering path based on beam orientation
	if (iVar2 < iVar3) {
		// Mostly horizontal beam - draw vertical scanlines
		// FUN_0040BBF6 lines 34-37
		iVar2 = abs(startX - endX);
		int temp = iVar2 * texH * heightScale;
		int16 numLines = (int16)((temp >> 3) / texW) + 2;
		int16 local_24 = -numLines;
		int16 halfLines = numLines >> 1;

		// FUN_0040BBF6 lines 39-46: Draw parallel textured scanlines (beam body)
		for (int16 lineIdx = 0; lineIdx < numLines; lineIdx++) {
			drawTexturedSegment(dst, pitch, width, height,
			                    startX, (startY - halfLines) + lineIdx,
			                    endX, (endY - halfLines) + lineIdx,
			                    texW, local_28);

			// Advance texture pointer (step through texture rows)
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= numLines) {
				local_28 += texW;
			}
		}

		// FUN_0040BBF6 lines 47-51: Edge highlights along top and bottom beam edges
		if (_rebelDetailMode >= 0) {
			drawEdgeHighlightLine(dst, pitch, width, height,
			                      startX, startY - halfLines,
			                      endX, endY - halfLines);
			drawEdgeHighlightLine(dst, pitch, width, height,
			                      startX, (startY - halfLines) + numLines - 1,
			                      endX, (endY - halfLines) + numLines - 1);
		}
	} else {
		// Mostly vertical beam - draw horizontal scanlines
		// FUN_0040BBF6 lines 54-56
		iVar2 = abs(startY - endY);
		int16 numLines = (int16)((iVar2 * texH) / texW) + 2;
		int16 local_24 = -numLines;

		// FUN_0040BBF6 lines 58-60: Clamp to texture height
		if (texH < numLines) {
			numLines = texH;
		}

		int16 halfLines = numLines >> 1;

		// FUN_0040BBF6 lines 61-68: Draw parallel textured scanlines (beam body)
		for (int16 lineIdx = 0; lineIdx < numLines; lineIdx++) {
			drawTexturedSegment(dst, pitch, width, height,
			                    (startX - halfLines) + lineIdx, startY,
			                    (endX - halfLines) + lineIdx, endY,
			                    texW, local_28);

			// Advance texture pointer
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= numLines) {
				local_28 += texW;
			}
		}

		// FUN_0040BBF6 lines 69-73: Edge highlights along left and right beam edges
		if (_rebelDetailMode >= 0) {
			drawEdgeHighlightLine(dst, pitch, width, height,
			                      startX - halfLines, startY,
			                      endX - halfLines, endY);
			drawEdgeHighlightLine(dst, pitch, width, height,
			                      (startX - halfLines) + numLines - 1, startY,
			                      (endX - halfLines) + numLines - 1, endY);
		}
	}
}
void InsaneRebel2::drawLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, byte color) {
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;

	for (;;) {
		if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
			dst[y0 * pitch + x0] = color;
		}
		if (x0 == x1 && y0 == y1)
			break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

void InsaneRebel2::drawCornerBrackets(byte *dst, int pitch, int width, int height, int x, int y, int w, int h, byte color) {
	// Draw L-shaped brackets at corners of the rect (x,y,w,h)
	// Bracket size: approx 8 pixels
	int armLen = 2;
	if (armLen > w / 2)
		armLen = w / 2;
	if (armLen > h / 2)
		armLen = h / 2;

	int x2 = x + w - 1;
	int y2 = y + h - 1;

	// Top-Left Corner
	drawLine(dst, pitch, width, height, x, y, x + armLen, y, color);
	drawLine(dst, pitch, width, height, x, y, x, y + armLen, color);

	// Top-Right Corner
	drawLine(dst, pitch, width, height, x2 - armLen, y, x2, y, color);
	drawLine(dst, pitch, width, height, x2, y, x2, y + armLen, color);

	// Bottom-Left Corner
	drawLine(dst, pitch, width, height, x, y2, x + armLen, y2, color);
	drawLine(dst, pitch, width, height, x, y2 - armLen, x, y2, color);

	// Bottom-Right Corner
	drawLine(dst, pitch, width, height, x2 - armLen, y2, x2, y2, color);
	drawLine(dst, pitch, width, height, x2, y2 - armLen, x2, y2, color);
}

// ============================================================
// COLLISION ZONE SYSTEM (for Level 3 pilot ship obstacle avoidance)
// ============================================================
// Based on FUN_40E35E, FUN_40C3CC disassembly from info.md
// Zones are quadrilaterals registered via IACT opcode 5

void InsaneRebel2::registerCollisionZone(Common::SeekableReadStream &b, int16 subOpcode, int16 par4) {
	// IACT Opcode 5 data layout — corrected from FUN_4033CF / FUN_4092D9 analysis:
	//
	// Original game stores pointer to full IACT data (starting at opcode).
	// SmushPlayer reads the first 8 bytes as header (code/flags/unknown/userId),
	// so our stream starts at body[0] (IACT byte offset +8).
	//
	// FUN_4092D9 field mapping (byte offsets from stored pointer):
	//   +0x00: opcode (5) — already consumed by SmushPlayer
	//   +0x02: par2 (sub-opcode) — already consumed, passed as parameter
	//   +0x04: par3 — already consumed by SmushPlayer
	//   +0x06: par4 (userId) — filter value for < 1000 test, passed as parameter
	//   +0x08: body[0] (sVar1) — control field 1 (frame check: field2-1 == field1)
	//   +0x0A: body[1] (sVar2) — control field 2
	//   +0x0C: body[2] — vertex 1 X
	//   +0x0E: body[3] — vertex 1 Y
	//   +0x10: body[4] — vertex 2 X
	//   +0x12: body[5] — vertex 2 Y
	//   +0x14: body[6] — vertex 3 X
	//   +0x16: body[7] — vertex 3 Y
	//   +0x18: body[8] — vertex 4 X
	//   +0x1A: body[9] — vertex 4 Y

	int16 field1 = b.readSint16LE();     // body[0] — control field 1
	int16 field2 = b.readSint16LE();     // body[1] — control field 2
	int16 x1 = b.readSint16LE();         // body[2] — vertex 1 X
	int16 y1 = b.readSint16LE();         // body[3] — vertex 1 Y
	int16 x2 = b.readSint16LE();         // body[4] — vertex 2 X
	int16 y2 = b.readSint16LE();         // body[5] — vertex 2 Y
	int16 x3 = b.readSint16LE();         // body[6] — vertex 3 X
	int16 y3 = b.readSint16LE();         // body[7] — vertex 3 Y
	int16 x4 = b.readSint16LE();         // body[8] — vertex 4 X
	int16 y4 = b.readSint16LE();         // body[9] — vertex 4 Y

	CollisionZone zone;
	zone.x1 = x1;
	zone.y1 = y1;
	zone.x2 = x2;
	zone.y2 = y2;
	zone.x3 = x3;
	zone.y3 = y3;
	zone.x4 = x4;
	zone.y4 = y4;
	zone.field1 = field1;
	zone.field2 = field2;
	zone.filterValue = par4;
	zone.subOpcode = subOpcode;
	zone.active = true;

	// Register zone into appropriate table based on sub-opcode
	if (subOpcode == 0x0D && _primaryZoneCount < kMaxCollisionZones) {
		_primaryZones[_primaryZoneCount++] = zone;
		debug("Rebel2: Registered PRIMARY zone %d: filter=%d fields=[%d,%d] quad=(%d,%d)-(%d,%d)-(%d,%d)-(%d,%d)",
			_primaryZoneCount - 1, par4, field1, field2,
			x1, y1, x2, y2, x3, y3, x4, y4);
	} else if (subOpcode == 0x0E && _secondaryZoneCount < kMaxCollisionZones) {
		_secondaryZones[_secondaryZoneCount++] = zone;
		debug("Rebel2: Registered SECONDARY zone %d: filter=%d fields=[%d,%d] quad=(%d,%d)-(%d,%d)-(%d,%d)-(%d,%d)",
			_secondaryZoneCount - 1, par4, field1, field2,
			x1, y1, x2, y2, x3, y3, x4, y4);
	} else {
		debug("Rebel2: WARNING - Could not register zone (subOpcode=%d, primary=%d, secondary=%d)",
			subOpcode, _primaryZoneCount, _secondaryZoneCount);
	}
}

void InsaneRebel2::resetCollisionZones() {
	// Reset zone counters at end of frame (FUN_403240 equivalent)
	// This clears the zone tables so they can be rebuilt from the next frame's IACT chunks
	_primaryZoneCount = 0;
	_secondaryZoneCount = 0;
}

void InsaneRebel2::checkCollisionZones() {
	// Per-frame collision checking — FUN_4092D9 first loop (lines 39-202).
	// Tests aim/ship position against primary collision zone quadrilaterals.
	//
	// Original coordinate system:
	//   Zone vertices are in 424x260 buffer space, centered by subtracting (0xD4=212, 0x82=130).
	//   Aim position (DAT_00443668/DAT_0044366a) is in centered coords [-52..52, -45..45].
	//   In FUN_407FCB: DAT_00443668 is a smoothed mouse-derived position.
	//
	// For our implementation:
	//   Map mouse position to centered coords matching the original range.
	//   Mouse X 0..320 → centered X ≈ [-52..52] (with smoothing in original)
	//   Mouse Y 0..200 → centered Y ≈ [-45..45]

	if (_primaryZoneCount == 0)
		return;

	// Calculate aim position in centered coordinates.
	// Original: local_10 = mouseOffset + 0xa0, then smoothed and clamped to [-0x34..0x34]
	// Simplified mapping: mouse 0..320 → [-52..52], mouse 0..200 → [-45..45]
	int16 aimX = (int16)((_vm->_mouse.x - 160) * 52 / 160);
	int16 aimY = (int16)((100 - _vm->_mouse.y) * 45 / 100);

	// Clamp to original ranges (DAT_0047a7fc < 1 path)
	if (aimX > 0x34)
		aimX = 0x34;
	if (aimX < -0x34)
		aimX = -0x34;
	if (aimY > 0x2d)
		aimY = 0x2d;
	if (aimY < -0x2d)
		aimY = -0x2d;

	for (int i = 0; i < _primaryZoneCount; i++) {
		CollisionZone &zone = _primaryZones[i];
		if (!zone.active)
			continue;

		// Filter: only process zones with filterValue < 1000 (par4 from IACT header)
		// Original: *(short *)(*local_c + 6) < 1000
		if (zone.filterValue >= 1000)
			continue;

		// Frame check: field2 - 1 == field1
		// Original: sVar2 + -1 == (int)sVar1
		if (zone.field2 - 1 != zone.field1)
			continue;

		// Center zone vertices by subtracting buffer center (0xD4=212, 0x82=130)
		// Original: sVar4 = x1 - 0xD4, sVar8 = y1 - 0x82, etc.
		int cx1 = zone.x1 - 0xD4;
		int cy1 = zone.y1 - 0x82;
		int cx2 = zone.x2 - 0xD4;
		int cy2 = zone.y2 - 0x82;
		int cx3 = zone.x3 - 0xD4;
		int cy3 = zone.y3 - 0x82;
		int cx4 = zone.x4 - 0xD4;
		int cy4 = zone.y4 - 0x82;

		// Point-in-quadrilateral test — FUN_4092D9 lines 119-128
		// Tests if aim position is OUTSIDE the safe corridor (= collision with obstacle).
		// Original uses 4 edge interpolation tests connected by OR (any failure = collision).
		//
		// Edge 1: interpolate Y along top edge (v1→v2) at aim X position
		//   if aimY < interpolated Y → outside top edge → collision
		// Edge 2: interpolate Y along bottom edge (v4→v3) at aim X position
		//   if interpolated Y < aimY → outside bottom edge → collision
		// Edge 3: interpolate X along left edge (v1→v4) at aim Y position
		//   if aimX < interpolated X → outside left edge → collision
		// Edge 4: interpolate X along right edge (v2→v3) at aim Y position
		//   if interpolated X < aimX → outside right edge → collision
		bool collision = false;

		// Avoid division by zero for degenerate edges
		if (cx2 != cx1) {
			int interpY1 = ((aimX - cx1) * (cy2 - cy1)) / (cx2 - cx1) + cy1;
			if (aimY < interpY1)
				collision = true;
		}
		if (!collision && cx3 != cx4) {
			int interpY2 = ((aimX - cx4) * (cy3 - cy4)) / (cx3 - cx4) + cy4;
			if (interpY2 < aimY)
				collision = true;
		}
		if (!collision && cy4 != cy1) {
			int interpX1 = ((aimY - cy1) * (cx4 - cx1)) / (cy4 - cy1) + cx1;
			if (aimX < interpX1)
				collision = true;
		}
		if (!collision && cy3 != cy2) {
			int interpX2 = ((aimY - cy2) * (cx3 - cx2)) / (cy3 - cy2) + cx2;
			if (interpX2 < aimX)
				collision = true;
		}

		if (collision) {
			// Collision detected — apply damage from collision damage table
			// Original: DAT_0047a7ec += DAT_0047e0f6[chapter * 0x242 + level * 0x22]
			LevelDifficultyParams params = getDifficultyParams();
			int collisionDamage = (params.dodgeDamage >= 0) ? params.dodgeDamage : 0;

			if (!_rebelInvulnerable) {
				_playerDamage += collisionDamage;
				if (_playerDamage > 255)
					_playerDamage = 255;
				debug("Rebel2: COLLISION damage! zone=%d aim=(%d,%d) damage=%d total=%d",
					i, aimX, aimY, collisionDamage, _playerDamage);
			}
			// Visual effect — FUN_00420515 (palette flash)
			initDamageFlash();
			// TODO: FUN_0041189e sound based on collision direction
		} else {
			// Safely passed — award score bonus
			// Original: FUN_0041bf8d(DAT_0047e100[levelIdx])
			LevelDifficultyParams scoreParams = getDifficultyParams();
			if (scoreParams.dodgePoints > 0) {
				addScore(scoreParams.dodgePoints);
			}
		}
	}
}

void InsaneRebel2::checkHandler7CollisionZones() {
	// FUN_40E35E — Handler 7 per-frame collision system.
	// Uses ship position (_flyShipScreenX/_flyShipScreenY) in raw buffer coords.
	// Two modes depending on _flyControlMode:
	//   Mode 0/2: Obstacle collision using SECONDARY zones (inside quad = hit)
	//   Mode 1/3: Wall/boundary collision using PRIMARY zones (per-edge push-back)

	// Note: _hitCooldown is decremented in renderSpaceExplosions (FUN_40F1C5)
	// to match the original where the decrement happens during rendering.

	if (_flyControlMode == 0 || _flyControlMode == 2) {
		// ---- Mode 0/2: Obstacle collision using SECONDARY zones (FUN_403b5b) ----
		// Original lines 52-132: Point-in-quad test with 15px inward margin.
		// Inside the quad = collision with obstacle.
		const int margin = 15;  // local_14 = 0x0f, local_20 = 0x0f

		for (int i = 0; i < _secondaryZoneCount; i++) {
			CollisionZone &zone = _secondaryZones[i];
			if (!zone.active)
				continue;

			int x1 = zone.x1, y1 = zone.y1;
			int x2 = zone.x2, y2 = zone.y2;
			int x3 = zone.x3, y3 = zone.y3;
			int x4 = zone.x4, y4 = zone.y4;

			// Point-in-quad test (lines 75-89)
			// Start assuming inside, clear if outside any edge (with margin)
			bool inside = true;

			// Top edge: interpolate Y along v1→v2 at shipX, +15 margin
			if (x2 != x1) {
				int interpY = (_flyShipScreenX - x1) * (y2 - y1) / (x2 - x1) + margin + y1;
				if (_flyShipScreenY < interpY)
					inside = false;
			}
			// Bottom edge: interpolate Y along v4→v3 at shipX, -15 margin
			if (inside && x3 != x4) {
				int interpY = (_flyShipScreenX - x4) * (y3 - y4) / (x3 - x4) + y4 - margin;
				if (interpY < _flyShipScreenY)
					inside = false;
			}
			// Left edge: interpolate X along v1→v4 at shipY, +15 margin
			if (inside && y4 != y1) {
				int interpX = (_flyShipScreenY - y1) * (x4 - x1) / (y4 - y1) + margin + x1;
				if (_flyShipScreenX < interpX)
					inside = false;
			}
			// Right edge: interpolate X along v2→v3 at shipY, -15 margin
			if (inside && y3 != y2) {
				int interpX = (_flyShipScreenY - y2) * (x3 - x2) / (y3 - y2) + x2 - margin;
				if (interpX < _flyShipScreenX)
					inside = false;
			}

			// Frame match: field2 - 1 == field1 (line 90)
			if (zone.field2 - 1 == zone.field1) {
				if (inside) {
					// Collision with obstacle — apply damage and break
					_hitCooldown = 10;
					_spaceShotDirection = zone.filterValue + 2;

					LevelDifficultyParams params = getDifficultyParams();
					int collisionDamage = (params.dodgeDamage >= 0) ? params.dodgeDamage : 0;
					if (!_rebelInvulnerable) {
						_playerDamage += collisionDamage;
						if (_playerDamage > 255)
							_playerDamage = 255;
					}
					_rebelHitCounter++;
					initDamageFlash();
					// Pan based on ship X position relative to screen center
					playSfx(1, 127, CLIP((_flyShipScreenX - 212) * 127 / 160, -127, 127));
					debug("Rebel2: Handler7 Mode0/2 OBSTACLE HIT zone=%d ship=(%d,%d) damage=%d",
						i, _flyShipScreenX, _flyShipScreenY, collisionDamage);
					break;  // Only one collision per frame (original breaks)
				} else {
					// Safely avoided obstacle — award score
					// Original: FUN_0041bf8d(DAT_0047e100[levelIdx])
					LevelDifficultyParams scoreParams = getDifficultyParams();
					if (scoreParams.dodgePoints > 0) {
						addScore(scoreParams.dodgePoints);
					}
				}
			}
		}

		// Corridor boundary proximity (lines 127-131)
		// These flags are used for directional indicators (not critical for damage)

	} else {
		// ---- Mode 1/3: Wall/boundary collision using PRIMARY zones (FUN_403b34) ----
		// Original lines 133-235: Per-edge interpolation with push-back.
		// Ship position is clamped to wall boundaries when hitting.
		int16 hMargin = (_flyControlMode == 1) ? 0x28 : 0x0f;  // local_14
		const int16 vMargin = 0x0f;  // local_20
		LevelDifficultyParams wallParams = getDifficultyParams();
		int wallDamage = (wallParams.dodgeDamage >= 0) ? wallParams.dodgeDamage : 0;

		for (int i = 0; i < _primaryZoneCount; i++) {
			CollisionZone &zone = _primaryZones[i];
			if (!zone.active)
				continue;

			int x1 = zone.x1, y1 = zone.y1;
			int x2 = zone.x2, y2 = zone.y2;
			int x3 = zone.x3, y3 = zone.y3;
			int x4 = zone.x4, y4 = zone.y4;

			// Top edge: interpolate Y along v1→v2 at shipX (lines 152-166)
			if (x2 != x1) {
				int16 edgeY = (int16)((_flyShipScreenX - x1) * (y2 - y1) / (x2 - x1) + y1 + vMargin);
				if (_flyShipScreenY < edgeY) {
					// Ship above top wall — push down
					if (_hitCooldown < 5 && !_rebelInvulnerable) {
						int damage = wallDamage;
						_playerDamage += damage;
						if (_playerDamage > 255)
							_playerDamage = 255;
						_rebelHitCounter++;
						_hitCooldown = 10;
						playSfx(1, 127, 0);  // CRASH.SAD, top wall → center pan
						debug("Rebel2: Handler7 Mode1/3 TOP WALL ship=(%d,%d) edgeY=%d damage=%d",
							_flyShipScreenX, _flyShipScreenY, edgeY, damage);
					}
					_spaceShotDirection = 2;  // Direction: pushed down
					_flyShipScreenY = edgeY;  // Push-back
					initDamageFlash();
				}
			}

			// Bottom edge: interpolate Y along v4→v3 at shipX (lines 167-183)
			if (x3 != x4) {
				int16 edgeY = (int16)((_flyShipScreenX - x4) * (y3 - y4) / (x3 - x4) + y4 - vMargin);
				_corridorBottomY = vMargin + edgeY;  // DAT_00443b10 update
				if (edgeY < _flyShipScreenY) {
					// Ship below bottom wall — push up
					if (_hitCooldown < 5 && !_rebelInvulnerable) {
						int damage = wallDamage;
						_playerDamage += damage;
						if (_playerDamage > 255)
							_playerDamage = 255;
						_rebelHitCounter++;
						_hitCooldown = 10;
						playSfx(1, 127, 0);  // CRASH.SAD, bottom wall → center pan
						debug("Rebel2: Handler7 Mode1/3 BOTTOM WALL ship=(%d,%d) edgeY=%d damage=%d",
							_flyShipScreenX, _flyShipScreenY, edgeY, damage);
					}
					_spaceShotDirection = 3;  // Direction: pushed up
					_flyShipScreenY = edgeY;  // Push-back
					initDamageFlash();
				}
			}

			// Left edge: interpolate X along v1→v4 at shipY (lines 184-199)
			if (y4 != y1) {
				int16 edgeX = (int16)((_flyShipScreenY - y1) * (x4 - x1) / (y4 - y1) + x1 + hMargin);
				if (_flyShipScreenX < edgeX) {
					// Ship left of left wall — push right
					_flyShipScreenX = edgeX;  // Push-back
					if (_hitCooldown < 5 && !_rebelInvulnerable) {
						int damage = wallDamage;
						_playerDamage += damage;
						if (_playerDamage > 255)
							_playerDamage = 255;
						_rebelHitCounter++;
						_hitCooldown = 10;
						playSfx(1, 127, -100);  // CRASH.SAD, left wall → pan left
						debug("Rebel2: Handler7 Mode1/3 LEFT WALL ship=(%d,%d) edgeX=%d damage=%d",
							_flyShipScreenX, _flyShipScreenY, edgeX, damage);
					}
					_spaceShotDirection = 0;  // Direction: pushed right
					initDamageFlash();
				}
			}

			// Right edge: interpolate X along v2→v3 at shipY (lines 200-215)
			if (y3 != y2) {
				int16 edgeX = (int16)((_flyShipScreenY - y2) * (x3 - x2) / (y3 - y2) + x2 - hMargin);
				if (edgeX < _flyShipScreenX) {
					// Ship right of right wall — push left
					_flyShipScreenX = edgeX;  // Push-back
					if (_hitCooldown < 5 && !_rebelInvulnerable) {
						int damage = wallDamage;
						_playerDamage += damage;
						if (_playerDamage > 255)
							_playerDamage = 255;
						_rebelHitCounter++;
						_hitCooldown = 10;
						playSfx(1, 127, 100);  // CRASH.SAD, right wall → pan right
						debug("Rebel2: Handler7 Mode1/3 RIGHT WALL ship=(%d,%d) edgeX=%d damage=%d",
							_flyShipScreenX, _flyShipScreenY, edgeX, damage);
					}
					_spaceShotDirection = 1;  // Direction: pushed left
					initDamageFlash();
				}
			}
		}
	}
}

void InsaneRebel2::drawQuad(byte *dst, int pitch, int width, int height,
                            int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, byte color) {
	// Draw a quadrilateral by connecting its 4 vertices with lines
	// Vertex order: top-left (1), top-right (2), bottom-right (3), bottom-left (4)
	drawLine(dst, pitch, width, height, x1, y1, x2, y2, color);  // Top edge
	drawLine(dst, pitch, width, height, x2, y2, x3, y3, color);  // Right edge
	drawLine(dst, pitch, width, height, x3, y3, x4, y4, color);  // Bottom edge
	drawLine(dst, pitch, width, height, x4, y4, x1, y1, color);  // Left edge
}

void InsaneRebel2::drawCollisionZones(byte *dst, int pitch, int width, int height, byte color) {
	// Draw all active collision zones as wireframe quadrilaterals for debugging
	// Uses different colors for primary vs secondary zones

	const byte primaryColor = 44;    // Bright red for primary (obstacle) zones
	const byte secondaryColor = 47;  // Yellow for secondary (boundary) zones

	// Draw primary zones (sub-opcode 0x0D - obstacles)
	for (int i = 0; i < _primaryZoneCount; i++) {
		CollisionZone &zone = _primaryZones[i];
		if (!zone.active)
			continue;

		// Apply view offset to convert from video coords to screen coords
		int x1 = zone.x1 + _viewX;
		int y1 = zone.y1 + _viewY;
		int x2 = zone.x2 + _viewX;
		int y2 = zone.y2 + _viewY;
		int x3 = zone.x3 + _viewX;
		int y3 = zone.y3 + _viewY;
		int x4 = zone.x4 + _viewX;
		int y4 = zone.y4 + _viewY;

		drawQuad(dst, pitch, width, height, x1, y1, x2, y2, x3, y3, x4, y4, primaryColor);
	}

	// Draw secondary zones (sub-opcode 0x0E - boundaries)
	for (int i = 0; i < _secondaryZoneCount; i++) {
		CollisionZone &zone = _secondaryZones[i];
		if (!zone.active)
			continue;

		// Apply view offset
		int x1 = zone.x1 + _viewX;
		int y1 = zone.y1 + _viewY;
		int x2 = zone.x2 + _viewX;
		int y2 = zone.y2 + _viewY;
		int x3 = zone.x3 + _viewX;
		int y3 = zone.y3 + _viewY;
		int x4 = zone.x4 + _viewX;
		int y4 = zone.y4 + _viewY;

		drawQuad(dst, pitch, width, height, x1, y1, x2, y2, x3, y3, x4, y4, secondaryColor);
	}

	// Draw corridor boundaries as a rectangle (from IACT opcode 7)
	if (_corridorLeftX != 0 || _corridorRightX != 0x1A8) {
		const byte corridorColor = 45;  // Cyan for corridor boundaries
		// Draw vertical lines for left/right boundaries
		drawLine(dst, pitch, width, height,
			_corridorLeftX + _viewX, _corridorTopY + _viewY,
			_corridorLeftX + _viewX, _corridorBottomY + _viewY, corridorColor);
		drawLine(dst, pitch, width, height,
			_corridorRightX + _viewX, _corridorTopY + _viewY,
			_corridorRightX + _viewX, _corridorBottomY + _viewY, corridorColor);
		// Draw horizontal lines for top/bottom boundaries
		drawLine(dst, pitch, width, height,
			_corridorLeftX + _viewX, _corridorTopY + _viewY,
			_corridorRightX + _viewX, _corridorTopY + _viewY, corridorColor);
		drawLine(dst, pitch, width, height,
			_corridorLeftX + _viewX, _corridorBottomY + _viewY,
			_corridorRightX + _viewX, _corridorBottomY + _viewY, corridorColor);
	}
}

void InsaneRebel2::renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx) {
	renderNutSpriteMirrored(dst, pitch, width, height, x, y, nut, spriteIdx, false);
}

// Render NUT sprite with optional horizontal mirroring
// Based on FUN_004236e0 disassembly - flags=0x2001 triggers horizontal flip
void InsaneRebel2::renderNutSpriteMirrored(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx, bool mirror) {
	if (!nut || spriteIdx < 0 || spriteIdx >= nut->getNumChars())
		return;

	int w = nut->getCharWidth(spriteIdx);
	int h = nut->getCharHeight(spriteIdx);
	const byte *src = nut->getCharData(spriteIdx);

	// Clipping
	int drawX = x;
	int drawY = y;
	int drawW = w;
	int drawH = h;
	int srcOffsetX = 0;
	int srcOffsetY = 0;

	if (drawX < 0) {
		srcOffsetX = -drawX;
		drawW += drawX;
		drawX = 0;
	}
	if (drawY < 0) {
		srcOffsetY = -drawY;
		drawH += drawY;
		drawY = 0;
	}

	if (drawX + drawW > width) {
		drawW = width - drawX;
	}
	if (drawY + drawH > height) {
		drawH = height - drawY;
	}

	if (drawW <= 0 || drawH <= 0)
		return;

	// Draw loop - with optional horizontal mirroring
	for (int iy = 0; iy < drawH; iy++) {
		const byte *s = src + (srcOffsetY + iy) * w;
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			int srcX;
			if (mirror) {
				// When mirrored, read from the opposite side of the sprite
				srcX = (w - 1) - (srcOffsetX + ix);
			} else {
				srcX = srcOffsetX + ix;
			}
			byte px = s[srcX];
			if (px != 231 && px != 0) { // Check both 0 and 231 (0xE7) for transparency
				d[ix] = px;
			}
		}
	}
}

void InsaneRebel2::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) {

	// Determine correct pitch for the video buffer (usually 320 for Rebel2)
	int width = _player->_width;
	int height = _player->_height;
	if (width == 0)
		width = _vm->_screenWidth;
	if (height == 0)
		height = _vm->_screenHeight;
	int pitch = width;

	// Calculate View/Scroll Offsets
	// Rebel Assault 2 uses a buffer larger (424x260) than screen (320x200)
	// Map mouse X (0-320) to Scroll X (0-104)
	// Map mouse Y (0-200) to Scroll Y (0-60)
	int maxScrollX = width - _vm->_screenWidth;
	int maxScrollY = height - _vm->_screenHeight;
	
	if (maxScrollX < 0)
		maxScrollX = 0;
	if (maxScrollY < 0)
		maxScrollY = 0;
	
	// Simple linear mapping: Center of screen corresponds to center of buffer
	_viewX = (_vm->_mouse.x * maxScrollX) / _vm->_screenWidth;
	_viewY = (_vm->_mouse.y * maxScrollY) / _vm->_screenHeight;
	
	_player->setScrollOffset(_viewX, _viewY);

	// --- HUD Drawing Order (from FUN_004089ab assembly analysis) ---
	// Based on FUN_004089ab:
	// 1. Line 156: FUN_004288c0 fills status bar background at Y=0xb4 (180)
	// 2. Lines 171-226: Draw turret overlays, targeting reticle, crosshair
	// 3. Line 243: FUN_0041c012 draws status bar sprites LAST (on top)
	//
	// In FUN_0041c012:
	// - Sprites are drawn to buffer DAT_00482204 at position (0,0)
	// - Buffer is composited at Y=0xb4 (180) via FUN_0042f780
	// - DISPFONT.NUT (DAT_00482200) sprites 1-7 contain the status bar elements
	//
	// We draw directly to screen at Y=180
	
	// Use video content coordinates, NOT buffer coordinates
	const int videoWidth = 320;    // Native video width
	const int videoHeight = 200;   // Native video height  
	const int statusBarY = 180;    // 0xb4 - status bar starts at Y=180 in video coords

	// Hide HUD/status bar during intro videos (marked by SmushPlayer video flag 0x20)
	// The 0x20 flag indicates a non-interactive cutscene/intro sequence OR menu
	bool introPlaying = ((_player->_curVideoFlags & 0x20) != 0);

	// Check if we're in menu mode (menu state + intro flag)
	bool menuMode = (introPlaying && _gameState == kStateMainMenu);
	bool pilotSelectMode = (introPlaying && (_gameState == kStatePilotSelect || _gameState == kStateDifficultySelect));
	bool chapterSelectMode = (introPlaying && _gameState == kStateChapterSelect);

	// Handle pilot selection input and rendering (FUN_00414A41)
	// This is the pilot/save slot selection screen with centered menu
	if (pilotSelectMode) {
		// Show the standard Windows arrow cursor
		Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
		CursorMan.replaceCursor(cursor);
		delete cursor;
		CursorMan.showMouse(true);

		// Process pilot selection input - emulates FUN_00414A41 input handling
		int selection = processLevelSelectInput();

		// Draw pilot selection overlay - centered menu like main menu
		drawLevelSelectOverlay(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop
		if (selection >= 0) {
			debug("Rebel2: Pilot selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in pilot select mode
		return;
	}

	// Handle chapter selection input and rendering (FUN_00415CF8)
	// This is the actual level/chapter selection screen with preview and password
	if (chapterSelectMode) {
		// Show the standard Windows arrow cursor (same as menu)
		Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
		CursorMan.replaceCursor(cursor);
		delete cursor;
		CursorMan.showMouse(true);

		// O_LEVEL.SAN provides the background with chapter preview thumbnails.
		// The FOBJ offset system (set in procPreRendering) scrolls the correct preview
		// into the preview box area. No black fill needed — video frame shows through.

		// Process chapter selection input - emulates FUN_00415CF8 input handling
		int selection = processChapterSelectInput();

		// Draw chapter selection overlay - emulates FUN_00415CF8 rendering
		drawChapterSelectOverlay(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop
		if (selection >= 0) {
			debug("Rebel2: Chapter selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in chapter select mode
		return;
	}

	// Handle menu input and rendering if in menu mode
	if (menuMode) {
		// The original game uses the standard Windows arrow cursor (IDC_ARROW)
		// loaded via LoadCursorA(NULL, 0x7f00) in FUN_420C70.decompiled.txt
		// MSTOVER.NUT is a background overlay, NOT a cursor
		Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
		CursorMan.replaceCursor(cursor);
		delete cursor;
		CursorMan.showMouse(true);

		// Process menu input during each frame
		int selection = processMenuInput();

		// Update inactivity timer (only increments when no input is received)
		// Input resets timer in processMenuInput()
		_menuInactivityTimer++;

		// Check for inactivity timeout
		// From FUN_004147b2: 300 frames of inactivity returns 0 (exit to intro/attract mode)
		// At 12fps video rate, 300 frames = ~25 seconds of inactivity
		// The original checks: if (local_8 > 299) return 0;
		if (_menuInactivityTimer > 300) {
			debug("Rebel2: Menu inactivity timeout - ending video to loop");
			// Signal video to end so menu loop plays new video
			// This emulates the attract mode behavior where a new random
			// menu video is selected after inactivity
			_menuInactivityTimer = 0;
			// Don't set _smushVideoShouldFinish here - let video end naturally
			// This will cause runMainMenu to loop and play a new random video
		}

		// Draw menu selection overlay
		drawMenuOverlay(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop
		if (selection >= 0) {
			debug("Rebel2: Menu selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in menu mode
		return;
	}

	// During intro/cinematic sequences:
	// - Hide the mouse cursor (original: ShowCursor(0) at startup in FUN_00420c70)
	// - Skip all HUD/status bar/crosshair rendering
	// - Skip mouse input processing (no shooting during intros)
	//
	// Original behavior from FUN_00403240:
	// - if (DAT_0047a814 == 0) { switch(DAT_0047ee84) { ... } }
	// - DAT_0047ee84 (handler) is only set by IACT opcode 6 during gameplay videos
	// - Cinematics/intros don't have opcode 6, so handler stays 0
	// - We use _rebelHandler == 0 as the primary indicator for intro/cinematic mode
	if (_rebelHandler == 0) {
		// Hide mouse cursor during intro - no crosshair, no clicking
		CursorMan.showMouse(false);

		// Track state transition for debugging
		if (!_introCursorPushed) {
			_introCursorPushed = true;
			debug("Rebel2: Intro/cinematic mode (handler=0, flags=0x%x, state=%d) - HUD disabled, mouse hidden",
				  _player->_curVideoFlags, _gameState);
		}

		// Chapter title text overlay (FUN_004171c5)
		if (_textOverlayActive)
			renderTextOverlay(renderBitmap, pitch, width, height, curFrame);

		// Skip all HUD rendering during intro - subtitles are rendered via opcode 9
		return;
	} else {
		// Gameplay mode - handler was set by IACT opcode 6
		if (_introCursorPushed) {
			_introCursorPushed = false;
			debug("Rebel2: Gameplay mode (handler=%d, flags=0x%x, state=%d) - HUD enabled",
				  _rebelHandler, _player->_curVideoFlags, _gameState);
		}
	}

	// From here on, we're in gameplay mode (_rebelHandler != 0)
	// Process mouse input for shooting
	// Original: FUN_00403240 only runs handlers when DAT_0047a814 == 0
	processMouse();

	// NOTE: Level 2 background is drawn ONCE during IACT opcode 8 par4=5 processing
	// (in procIACT when the background ANIM is first loaded). The 0x08 video flag
	// (preserve background) prevents the frame buffer from being cleared, so the
	// background persists. FOBJ sprites (enemies) are then decoded on top by SMUSH.
	// We do NOT redraw the background here as that would overwrite FOBJ content.

	// --- HUD Drawing Order (from FUN_004089ab / FUN_40D836 assembly analysis) ---
	// Original assembly render order for handler 0x26:
	//   1. FUN_004288c0: Fill status bar background
	//   2. FUN_004092d9: Collision/hit processing
	//   3. FUN_00409fbc: Explosion rendering
	//   4. FUN_0040ad63: LASER SHOTS (drawn BEFORE cockpit overlays)
	//   5. FUN_004236e0: Crosshair + cockpit NUT overlays (drawn ON TOP of lasers)
	//   6. FUN_0041c012: Status bar text/numbers
	// The cockpit frame covers laser beam edges, giving the appearance
	// that beams emerge from behind the cockpit.

	// STEP 0: Fill status bar background (FUN_004288c0)
	renderStatusBarBackground(renderBitmap, pitch, width, height, videoWidth, videoHeight, statusBarY);

	// Ship rendering (FUN_00401ccf for Handler 8, FUN_0040d836 for Handler 7)
	debug("Rebel2 Ship Check: handler=%d shipSprite=%p flyShipSprite=%p shipLevelMode=%d numSprites=%d/%d",
		_rebelHandler, (void*)_shipSprite, (void*)_flyShipSprite, _shipLevelMode,
		_shipSprite ? _shipSprite->getNumChars() : 0,
		_flyShipSprite ? _flyShipSprite->getNumChars() : 0);

	renderHandler7Ship(renderBitmap, pitch, width, height);
	renderHandler8Ship(renderBitmap, pitch, width, height);
	// GRD001 (wall/cockpit) drawn AFTER FOBJs per original FUN_0041DB5E lines 202-210
	renderHandler25ShipPre(renderBitmap, pitch, width, height);
	renderHandler25Ship(renderBitmap, pitch, width, height);
	renderFallbackShip(renderBitmap, pitch, width, height);

	// Enemy indicators and destroyed enemy area erase
	renderEnemyOverlays(renderBitmap, pitch, width, height, videoWidth);

	// Explosion animations (FUN_409FBC) — drawn before lasers in original
	renderExplosions(renderBitmap, pitch, width, height);

	// Laser shot beams — drawn BEFORE cockpit/HUD overlays so cockpit covers beam edges
	renderLaserShots(renderBitmap, pitch, width, height);

	// STEP 1A: Draw NUT-based HUD overlays for Handler 0x26/0x19 (FUN_004089ab lines 195-226)
	// These are cockpit frame, crosshair, and reticle — drawn ON TOP of laser beams
	renderTurretHudOverlays(renderBitmap, pitch, width, height, curFrame);

	// STEP 1B: Draw embedded SAN HUD overlays (from IACT chunks)
	renderEmbeddedHudOverlays(renderBitmap, pitch, width, height);

	// STEP 2: Draw DISPFONT.NUT status bar sprites (FUN_0041c012)
	renderStatusBarSprites(renderBitmap, pitch, width, height, statusBarY, curFrame);

	// Damage visual effects — handler-specific per original architecture:
	//   Handler 8:    FUN_401CCF line 119 → FUN_00420754 (palette flash + screen shake)
	//   Handler 0x19: FUN_41DB5E line 192 → FUN_00420562 (palette flash only, every frame)
	//   Handler 0x26: FUN_4092D9 lines 135/225/237 → FUN_00420515 trigger + palette flash
	//   Handler 7:    FUN_40E35E → FUN_00420515 trigger + palette flash
	if (_rebelHandler == 8) {
		// Full damage effect: palette flash + screen shake
		// Suppressed during autopilot (mode 4) and cutscene (mode 5)
		if (_shipLevelMode != 4 && _shipLevelMode != 5) {
			updateDamageEffect(renderBitmap, pitch, width, height);
		}
	} else if (_rebelHandler == 0x19 || _rebelHandler == 0x26 || _rebelHandler == 7) {
		// Palette flash only — no screen shake for turret/FPS/ship handlers
		updateDamageFlashPalette();
	}

	// Per-frame collision checking against registered zones.
	//
	// Handler 0x26 (turret): FUN_4092D9 — aim position vs primary zones (centered coords)
	//   Zones with filterValue < 1000 tested via point-in-quad against mouse/aim position.
	//
	// Handler 7 (ship): FUN_40E35E — ship position vs zones per control mode:
	//   Mode 0/2: SECONDARY zones (0x0E) — obstacle collision (inside quad = hit)
	//   Mode 1/3: PRIMARY zones (0x0D) — wall/boundary per-edge with push-back
	//   Uses ship position in raw buffer coords, hit cooldown, directional damage.
	if (_rebelHandler == 0x26) {
		checkCollisionZones();
	} else if (_rebelHandler == 7) {
		checkHandler7CollisionZones();
	}

	// Collision zone visualization (debug - for Handler 7/8 pilot modes)
	if (_rebelHandler == 7 || _rebelHandler == 8) {
		drawCollisionZones(renderBitmap, pitch, width, height, 0);
	}

	// Crosshair/reticle (FUN_004089ab, FUN_0040d836)
	renderCrosshair(renderBitmap, pitch, width, height);

	// HUD score/lives rendering (FUN_0041c012)
	renderScoreHUD(renderBitmap, pitch, width, height, 0);

	// Reset FOBJ position offsets (FUN_00424510(0,0) in original FUN_0041DB5E line 271)
	if (_player) {
		_player->_fobjOffsetX = 0;
		_player->_fobjOffsetY = 0;
	}

	// Frame end cleanup: reset enemy active flags and collision zones (FUN_403240)
	frameEndCleanup();
}

// ======================= Damage Visual Effect Functions =======================
// Palette flash + screen shake when the player takes damage.
// Original retail functions: FUN_420515, FUN_420562, FUN_420754, FUN_42073B, FUN_420501

// FUN_00420501 - Reset palette flash counter.
// Called at level start / scene transitions to clear any in-progress flash.
void InsaneRebel2::resetDamageFlash() {
	_damageFlashCounter = 0;
}

// FUN_00420515 - Save current palette and initiate a 5-frame flash.
// If a flash is already in progress, just resets the counter to 5
// (the palette was already saved on the first hit).
void InsaneRebel2::initDamageFlash() {
	if (_damageFlashCounter == 0) {
		// Save current SMUSH palette before modifying it
		memcpy(_damageSavedPalette, _player->_pal, 0x300);
	}
	_damageFlashCounter = 5;
}

// FUN_0042073B - Trigger both palette flash and screen shake.
// Called from the damage hit handler when the player takes damage.
void InsaneRebel2::triggerDamageEffect() {
	initDamageFlash();
	_damageShakeCounter = 10;
}

// FUN_00420562 - Per-frame palette modification.
//
// Two modes determined by _damageHighFlashCounter:
//
//   Normal hit flash (_damageHighFlashCounter == 0 or odd):
//     Decrements _damageFlashCounter. On even counter values, all 768 palette
//     bytes (RGB) are blended from inverted toward the saved original:
//       output[i] = 0xFF - ((0xFF - saved[i]) * (0x10 - counter)) >> 4
//     Counter 5→4(apply)→3(skip)→2(apply)→1(skip)→0(apply=original). The
//     alternating apply/skip creates a strobe-like flash effect.
//
//   High-damage red pulse (_playerDamage >= 0xFF, even counter):
//     Only the R channel (every 3rd byte) is modified using the same formula
//     with _damageHighFlashCounter. Creates a pulsing red tint overlay.
void InsaneRebel2::updateDamageFlashPalette() {
	// High-damage mode: persistent red pulsing when damage is maxed out
	if (_playerDamage < 0xFF) {
		_damageHighFlashCounter = 0;
	} else {
		if (_damageHighFlashCounter == 0) {
			// Save palette on first frame of high-damage mode
			memcpy(_damageSavedPalette, _player->_pal, 0x300);
		}
		if (_damageHighFlashCounter < 0x10) {
			_damageHighFlashCounter++;
		}
	}

	if (_damageHighFlashCounter == 0 || (_damageHighFlashCounter & 1) != 0) {
		// Normal hit flash path: decrement counter, apply on even values.
		// Original C: if ((counter != 0) && (counter--, (counter & 1) == 0))
		if (_damageFlashCounter != 0) {
			_damageFlashCounter--;
			if ((_damageFlashCounter & 1) == 0) {
				// Apply palette inversion on ALL RGB channels
				byte modPal[0x300];
				int blend = 0x10 - _damageFlashCounter;
				for (int i = 0; i < 0x300; i++) {
					modPal[i] = 0xFF - (((0xFF - _damageSavedPalette[i]) * blend) >> 4);
				}
				_player->setPalette(modPal);
			}
		}
	} else {
		// High-damage red-only flash (even _damageHighFlashCounter):
		// Modify only R channel (stride 3), G and B stay unchanged.
		byte modPal[0x300];
		memcpy(modPal, _player->_pal, 0x300);
		int blend = 0x10 - _damageHighFlashCounter;
		for (int i = 0; i < 0x300; i += 3) {
			modPal[i] = 0xFF - (((0xFF - _damageSavedPalette[i]) * blend) >> 4);
		}
		_player->setPalette(modPal);
	}
}

// FUN_00420754 - Per-frame screen shake + palette flash.
//
// Screen shake randomly shifts scanlines left or right for visual distortion.
// The number of affected scanlines decreases each frame (counter * 5),
// creating a diminishing shake effect over 10 frames.
//
// Called every frame from procPostRendering when not in cutscene modes
// (shipLevelMode != 4 and != 5, matching original: DAT_0043e000 != 4 && != 5).
void InsaneRebel2::updateDamageEffect(byte *renderBitmap, int pitch, int width, int height) {
	if (_damageShakeCounter != 0) {
		_damageShakeCounter--;
		int numLines = _damageShakeCounter * 5;

		// Temporary buffer for scanline rotation (case 1 in original)
		byte tempLine[640];

		for (int n = numLines; n > 0; n--) {
			// Pick a random scanline within the gameplay area (0..179, not status bar)
			int maxY = MIN(height, 180);
			int scanline = _vm->_rnd.getRandomNumber(maxY - 1);

			byte *linePtr = renderBitmap + pitch * scanline;
			int offset = _vm->_rnd.getRandomNumber(4) + 1;  // 1..5 pixel shift
			int direction = _vm->_rnd.getRandomNumber(4);    // 0..4

			int copyLen = pitch - offset;
			if (copyLen <= 0)
				continue;

			switch (direction) {
			case 0:
			case 3:
				// Shift left: copy line[offset..] -> line[0..]
				memmove(linePtr, linePtr + offset, copyLen);
				break;
			case 1:
				// Shift right with wrap: save, then copy
				memcpy(tempLine, linePtr, MIN(copyLen, (int)sizeof(tempLine)));
				memmove(linePtr + offset, tempLine, MIN(copyLen, (int)sizeof(tempLine)));
				break;
			case 2:
			case 4:
				// Shift right: copy line[0..] -> line[offset..]
				memmove(linePtr + offset, linePtr, copyLen);
				break;
			}
		}
	}

	// Palette flash runs every frame (even without shake)
	updateDamageFlashPalette();
}

// ======================= Rendering Helper Functions =======================
// These are extracted from procPostRendering for better readability

void InsaneRebel2::renderTextOverlay(byte *renderBitmap, int pitch, int width, int height, int curFrame) {
	// Emulates FUN_004171c5 text overlay: progressive chapter title during [fadeIn, fadeOut)
	if (curFrame < _textOverlayFadeIn || curFrame >= _textOverlayFadeOut)
		return;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	const char *text = splayer->getString(_textOverlayID);
	debug(5, "Rebel2: Text overlay frame %d/%d-%d textID=0x%x text='%s'",
	      curFrame, _textOverlayFadeIn, _textOverlayFadeOut, _textOverlayID,
	      text ? text : "(null)");
	if (!text)
		return;

	// Progressive reveal: displayLen = currentFrame + 10 - fadeInFrame, capped at 0xBE (190)
	int displayLen = curFrame + 10 - _textOverlayFadeIn;
	if (displayLen > 0xBE)
		displayLen = 0xBE;
	if (displayLen < 0)
		return;

	// Font system — ^fNN = font switch, ^cNNN = color code
	NutRenderer *fonts[3] = { _smush_talkfontNut, _smush_smalfontNut, _smush_titlefontNut };
	NutRenderer *defaultFont = fonts[0] ? fonts[0] : _smush_smalfontNut;
	if (!defaultFont)
		return;

	Common::Rect clipRect(0, 0, width, height);

	// Format code parser (same as drawMenuItems / FUN_00434d10)
	auto parseFormat = [&](const char *&s, NutRenderer *&curFont, int &curColor) {
		if (*s != '^')
			return false;
		const char *p = s + 1;
		if (*p == '^') { s = p; return false; }
		if (*p == 'f') {
			p++;
			int idx = 0;
			while (*p >= '0' && *p <= '9') { idx = idx * 10 + (*p - '0'); p++; }
			s = p;
			curFont = (idx >= 0 && idx < 3 && fonts[idx]) ? fonts[idx] : defaultFont;
			return true;
		}
		if (*p == 'c') {
			p++;
			int col = 0;
			while (*p >= '0' && *p <= '9') { col = col * 10 + (*p - '0'); p++; }
			s = p;
			curColor = col;
			return true;
		}
		return false;
	};

	// The TRS parser joins multi-line strings with spaces (stripping \n//),
	// so " ^f" marks where a line break was in the original TRS file.
	// Split into lines, then render each centered at textX (FUN_004341a0).
	Common::Array<Common::String> lines;
	{
		Common::String cur;
		const char *s = text;
		while (*s) {
			if (*s == ' ' && s[1] == '^' && s[2] == 'f') {
				lines.push_back(cur);
				cur.clear();
				s++; // skip the space, keep ^f for the next line
				continue;
			}
			cur += *s++;
		}
		if (!cur.empty())
			lines.push_back(cur);
	}

	int drawY = _textOverlayY;
	int visCount = 0;

	for (uint lineIdx = 0; lineIdx < lines.size() && visCount < displayLen; lineIdx++) {
		const char *lineStr = lines[lineIdx].c_str();
		const char *lineEnd = lineStr + lines[lineIdx].size();

		// Measure visible chars up to displayLen
		int lineWidth = 0;
		int lineVisCount = 0;
		NutRenderer *lineFont = defaultFont;
		{
			const char *s = lineStr;
			NutRenderer *mFont = defaultFont;
			int mColor = 1;
			while (s < lineEnd && (visCount + lineVisCount) < displayLen) {
				if (parseFormat(s, mFont, mColor))
					continue;
				lineFont = mFont;
				byte c = (byte)*s++;
				if (c >= 'a' && c <= 'z')
					c = c - 'a' + 'A';
				if (mFont && c < mFont->getNumChars())
					lineWidth += mFont->getCharWidth(c);
				lineVisCount++;
			}
		}

		// Draw line centered at textX
		int drawX = _textOverlayX - lineWidth / 2;
		int lineCharsDrawn = 0;
		{
			const char *s = lineStr;
			NutRenderer *curFont = defaultFont;
			int curColor = 1;
			while (s < lineEnd && (visCount + lineCharsDrawn) < displayLen) {
				if (parseFormat(s, curFont, curColor))
					continue;
				byte c = (byte)*s++;
				if (c >= 'a' && c <= 'z')
					c = c - 'a' + 'A';
				if (!curFont || c >= curFont->getNumChars()) {
					lineCharsDrawn++;
					continue;
				}
				int charW = curFont->getCharWidth(c);
				if (drawX >= 0 && drawY >= 0 && charW > 0) {
					curFont->drawCharV7(renderBitmap, clipRect, drawX, drawY,
					                    pitch, curColor, kStyleAlignLeft, c, false, false);
				}
				drawX += charW;
				lineCharsDrawn++;
			}
		}
		visCount += lineCharsDrawn;

		int lineHeight = lineFont->getCharHeight('A') + 2;
		drawY += lineHeight;
	}
}

void InsaneRebel2::renderStatusBarBackground(byte *renderBitmap, int pitch, int width, int height,
											 int videoWidth, int videoHeight, int statusBarY) {
	// Fill status bar background (FUN_004288c0 equivalent)
	// Original assembly: FUN_004288c0(local_8, 0, 0, 0xb4, 0x140, 0x14, 4)
	// This fills width=320, height=20 starting at Y=180 with color index 4
	const byte statusBarBgColor = 4;

	for (int y = statusBarY; y < videoHeight; y++) {
		int destY = y + _viewY;
		if (destY >= height)
			continue;
		for (int x = 0; x < videoWidth; x++) {
			int destX = x + _viewX;
			if (destX >= pitch)
				continue;
			renderBitmap[destY * pitch + destX] = statusBarBgColor;
		}
	}
}

void InsaneRebel2::renderTurretHudOverlays(byte *renderBitmap, int pitch, int width, int height, int32 curFrame) {
	// Draw NUT-based HUD overlays for Handler 0x26/0x19 (turret modes)
	// From FUN_004089ab disassembly (lines 195-226):
	// - DAT_0047fe78 (_hudOverlayNut): Primary HUD overlay with 6 animation frames
	// - Position formula (low-res):
	//   X = 160 + (mouseOffsetX >> 4) - (width / 2) - spriteOffsetX
	//   Y = 182 - (mouseOffsetY >> 4) - height - spriteOffsetY
	// - Animation: spriteIndex = (frameCounter / 2) % 6

	if ((_rebelHandler != 0x26 && _rebelHandler != 0x19) || !_hudOverlayNut || _hudOverlayNut->getNumChars() <= 0)
		return;

	// Calculate mouse offset (clamped to -127..127)
	int mouseOffsetX = (_vm->_mouse.x - 160);
	int mouseOffsetY = (_vm->_mouse.y - 100);
	if (mouseOffsetX > 127)
		mouseOffsetX = 127;
	if (mouseOffsetX < -127)
		mouseOffsetX = -127;
	if (mouseOffsetY > 127)
		mouseOffsetY = 127;
	if (mouseOffsetY < -127)
		mouseOffsetY = -127;

	// Animation frame cycling: (frameCounter / 2) % 6
	int numSprites = _hudOverlayNut->getNumChars();
	int animFrameCount = MIN(numSprites, 6);
	int animFrame = 0;
	if (animFrameCount > 0) {
		animFrame = (curFrame / 2) % animFrameCount;
	}

	// Get sprite dimensions
	int spriteW = _hudOverlayNut->getCharWidth(animFrame);
	int spriteH = _hudOverlayNut->getCharHeight(animFrame);

	// Position calculation from assembly (low-res mode)
	int spriteOffsetX = 0;
	int spriteOffsetY = 0;
	int hudX = 160 + (mouseOffsetX >> 4) - (spriteW / 2) - spriteOffsetX;
	int hudY = 182 - (mouseOffsetY >> 4) - spriteH - spriteOffsetY;

	// Apply view offset for scrolling background
	hudX += _viewX;
	hudY += _viewY;

	// Draw base cockpit (sprite 0 always drawn first)
	renderNutSprite(renderBitmap, pitch, width, height, hudX, hudY, _hudOverlayNut, 0);

	// Draw animation overlay frame if not frame 0
	if (animFrame != 0 && animFrame < numSprites) {
		renderNutSprite(renderBitmap, pitch, width, height, hudX, hudY, _hudOverlayNut, animFrame);
	}

	debug(5, "Rebel2 HUD: Drawing NUT overlay frame %d/%d at (%d,%d) mouseOffset=(%d,%d)",
		  animFrame, numSprites, hudX, hudY, mouseOffsetX, mouseOffsetY);

	// Draw secondary HUD overlay if present (DAT_0047fe80)
	if (_hudOverlay2Nut && _hudOverlay2Nut->getNumChars() > 0) {
		int spr2W = _hudOverlay2Nut->getCharWidth(0);
		int spr2H = _hudOverlay2Nut->getCharHeight(0);
		int hud2X = 160 + (mouseOffsetX >> 4) - (spr2W / 2) + _viewX;
		int hud2Y = 182 - (mouseOffsetY >> 4) - spr2H + _viewY;
		renderNutSprite(renderBitmap, pitch, width, height, hud2X, hud2Y, _hudOverlay2Nut, 0);
	}
}

void InsaneRebel2::renderEmbeddedHudOverlays(byte *renderBitmap, int pitch, int width, int height) {
	// Draw embedded SAN HUD overlays (from IACT chunks)
	// For Handler 7 (Level 3): HUD elements are scattered across the screen
	// For turret handlers: slots 1-2 form a two-part cockpit overlay

	for (int hudSlot = 1; hudSlot < 16; hudSlot++) {
		EmbeddedSanFrame &frame = _rebelEmbeddedHud[hudSlot];
		if (!frame.valid || !frame.pixels || frame.width <= 0 || frame.height <= 0)
			continue;

		// Handler 25: Skip slot 4 (corridor overlay) in post-rendering.
		// The corridor is a full background image (no color 0 transparent center).
		// Drawing it here would cover enemies. It's already drawn in procPreRendering
		// with transparency to preserve frame persistence for codec 23 delta.
		if (_rebelHandler == 25 && hudSlot == 4) {
			continue;
		}

		// Skip small frames at (0,0) - likely animation patches
		if (frame.renderX == 0 && frame.renderY == 0 && frame.width < 50 && frame.height < 60) {
			debug(3, "Rebel2: Skipping small embedded frame at (0,0): slot=%d size=%dx%d",
				hudSlot, frame.width, frame.height);
			continue;
		}

		// For Handler 7: handle direction-based frame selection
		if (_rebelHandler == 7) {
			int groupMembers[16];
			int groupCount = 0;

			for (int id = 1; id < 16; id++) {
				EmbeddedSanFrame &g = _rebelEmbeddedHud[id];
				if (g.valid && g.renderX == frame.renderX && g.renderY == frame.renderY &&
					g.width == frame.width && g.height == frame.height) {
					groupMembers[groupCount++] = id;
				}
			}

			if (groupCount > 1) {
				int selectedOffset = _shipDirectionIndex % groupCount;
				int selectedId = groupMembers[selectedOffset];

				// Verify selected frame has pixels
				EmbeddedSanFrame &selectedFrame = _rebelEmbeddedHud[selectedId];
				int nonZero = 0;
				for (int i = 0; i < selectedFrame.width * selectedFrame.height; i++) {
					if (selectedFrame.pixels[i] != 0)
						nonZero++;
				}

				if (nonZero == 0) {
					for (int i = 0; i < groupCount; i++) {
						EmbeddedSanFrame &altFrame = _rebelEmbeddedHud[groupMembers[i]];
						int altNonZero = 0;
						for (int j = 0; j < altFrame.width * altFrame.height; j++) {
							if (altFrame.pixels[j] != 0)
								altNonZero++;
						}
						if (altNonZero > 0) {
							selectedId = groupMembers[i];
							break;
						}
					}
				}

				if (hudSlot != selectedId)
					continue;
			}
		}

		// Calculate destination position
		int destX = frame.renderX;
		int destY = frame.renderY;

		// Handler 0x26/0x19 turret positioning
		if ((_rebelHandler == 0x26 || _rebelHandler == 0x19) && (hudSlot == 1 || hudSlot == 2)) {
			destX = 160 - frame.width / 2 - frame.renderX;
			destY = 200 - frame.height - frame.renderY;
		}

		// Handler 7 large cockpit frame positioning
		if (_rebelHandler == 7 && (hudSlot == 1 || hudSlot == 2) && frame.width > 100) {
			destX = 160 - frame.width / 2 - frame.renderX;
			destY = 170 - frame.height - frame.renderY;
		} else if (_rebelHandler == 7 && destX > 100 && destY > 50) {
			int16 offsetX = (_shipPosX - 160) / 8;
			int16 offsetY = (_shipPosY - 100) / 8;
			destX += offsetX;
			destY += offsetY;
		}

		destX += _viewX;
		destY += _viewY;

		debug(3, "Rebel2: Rendering embedded HUD slot=%d size=%dx%d at (%d,%d)",
			hudSlot, frame.width, frame.height, destX, destY);

		// Draw frame with transparency (pixel 0 and 231 = transparent)
		for (int y = 0; y < frame.height && (destY + y) < height; y++) {
			for (int x = 0; x < frame.width && (destX + x) < pitch; x++) {
				byte pixel = frame.pixels[y * frame.width + x];
				if (pixel != 0 && pixel != 231) {
					int fx = destX + x;
					int fy = destY + y;
					if (fx >= 0 && fy >= 0) {
						renderBitmap[fy * pitch + fx] = pixel;
					}
				}
			}
		}
	}
}

void InsaneRebel2::renderStatusBarSprites(byte *renderBitmap, int pitch, int width, int height,
										  int statusBarY, int32 curFrame) {
	// FUN_0041c012 equivalent — renders DISPFONT.NUT status bar sprites.
	// DISPFONT.NUT sprite layout:
	//   Sprite 1:   Status bar background
	//   Sprites 2-5: Difficulty variants (full status bar with 1-4 stars)
	//   Sprite 6:   Bar fill element (reused for both damage and lives bars)
	//   Sprite 7:   Damage alert overlay (flashing when critical)

	if (!_smush_cockpitNut)
		return;

	int numSprites = _smush_cockpitNut->getNumChars();

	// --- Sprite 1: Status bar background (always drawn first as base layer) ---
	if (numSprites > 1) {
		renderNutSprite(renderBitmap, pitch, width, height,
			_viewX, statusBarY + _viewY, _smush_cockpitNut, 1);
	}

	// --- Difficulty sprite (2-5) overlaid on top ---
	// FUN_0041c012 lines 33-43: sprite index = min(difficulty, 4) + 1
	int difficulty = _difficulty;
	if (difficulty > 3)
		difficulty = 3;
	int difficultySprite = difficulty + 2;
	if (numSprites > difficultySprite) {
		renderNutSprite(renderBitmap, pitch, width, height,
			_viewX, statusBarY + _viewY, _smush_cockpitNut, difficultySprite);
	}

	// --- Damage/shield bar (sprite 6 within damage clip rect) ---
	// FUN_0041c012 lines 44-76:
	//   Clip rect (low-res): {X=0x3f, Y=9, W=0x40, H=6} = {63, 9, 64, 6}
	//   Bar width = shield_value >> 2 (divide by 4, range 0-63)
	//   If shield > 0xAA (170): alert blink with sprite 7
	if (numSprites > 6) {
		// Bar width from assembly: param_1 >> 2 (low-res)
		int damageBarWidth = _playerDamage >> 2;

		const byte *src = _smush_cockpitNut->getCharData(6);
		int sw = _smush_cockpitNut->getCharWidth(6);
		int sh = _smush_cockpitNut->getCharHeight(6);

		const int dmgClipX = 63, dmgClipY = 9, dmgClipW = 64, dmgClipH = 6;

		if (src && sw > 0 && sh > 0 && damageBarWidth > 0) {
			int drawW = MIN(damageBarWidth, MIN(dmgClipW, sw - dmgClipX));
			int drawH = MIN(dmgClipH, sh - dmgClipY);
			if (drawW > 0 && drawH > 0) {
				for (int y = 0; y < drawH; y++) {
					int destY = statusBarY + dmgClipY + y + _viewY;
					if (destY < 0 || destY >= height)
						continue;
					for (int x = 0; x < drawW; x++) {
						int destX = dmgClipX + x + _viewX;
						if (destX < 0 || destX >= pitch)
							continue;
						byte pixel = src[(dmgClipY + y) * sw + (dmgClipX + x)];
						if (pixel != 0) {
							renderBitmap[destY * pitch + destX] = pixel;
						}
					}
				}
			}
		}

		// Damage alert overlay (sprite 7) — FUN_0041c012 lines 68-76:
		// When damage > 0xAA (170) and frame counter bit 3 is clear, draw sprite 7
		// at full clip rect width (64 pixels) to show flashing alert
		if (numSprites > 7 && _playerDamage > 170 && ((curFrame & 8) == 0)) {
			if (src && sw > 0 && sh > 0) {
				int alertW = MIN(dmgClipW, sw - dmgClipX);
				int alertH = MIN(dmgClipH, sh - dmgClipY);
				if (alertW > 0 && alertH > 0) {
					const byte *alertSrc = _smush_cockpitNut->getCharData(7);
					int alertSW = _smush_cockpitNut->getCharWidth(7);
					int alertSH = _smush_cockpitNut->getCharHeight(7);
					if (alertSrc && alertSW > 0 && alertSH > 0) {
						int aW = MIN(alertW, alertSW - dmgClipX);
						int aH = MIN(alertH, alertSH - dmgClipY);
						for (int y = 0; y < aH; y++) {
							int destY = statusBarY + dmgClipY + y + _viewY;
							if (destY < 0 || destY >= height)
								continue;
							for (int x = 0; x < aW; x++) {
								int destX = dmgClipX + x + _viewX;
								if (destX < 0 || destX >= pitch)
									continue;
								byte pixel = alertSrc[(dmgClipY + y) * alertSW + (dmgClipX + x)];
								if (pixel != 0) {
									renderBitmap[destY * pitch + destX] = pixel;
								}
							}
						}
					}
				}
			}
		}
	}

	// --- Lives bar (sprite 6 within lives clip rect) ---
	// FUN_0041c012 lines 99-131:
	//   Clip rect (low-res): {X=0xa8, Y=7, W=0x32, H=9} = {168, 7, 50, 9}
	//   Bar width = min((lives * 5 - 5) * 2, 50) — only drawn when lives > 1
	if (numSprites > 6 && _playerLives > 1) {
		int livesBarWidth = (_playerLives * 5 - 5) * 2;
		if (livesBarWidth > 50)
			livesBarWidth = 50;

		const byte *src = _smush_cockpitNut->getCharData(6);
		int sw = _smush_cockpitNut->getCharWidth(6);
		int sh = _smush_cockpitNut->getCharHeight(6);

		const int livClipX = 168, livClipY = 7, livClipW = 50, livClipH = 9;

		if (src && sw > 0 && sh > 0 && livesBarWidth > 0) {
			int drawW = MIN(livesBarWidth, MIN(livClipW, sw - livClipX));
			int drawH = MIN(livClipH, sh - livClipY);
			if (drawW > 0 && drawH > 0) {
				for (int y = 0; y < drawH; y++) {
					int destY = statusBarY + livClipY + y + _viewY;
					if (destY < 0 || destY >= height)
						continue;
					for (int x = 0; x < drawW; x++) {
						int destX = livClipX + x + _viewX;
						if (destX < 0 || destX >= pitch)
							continue;
						byte pixel = src[(livClipY + y) * sw + (livClipX + x)];
						if (pixel != 0) {
							renderBitmap[destY * pitch + destX] = pixel;
						}
					}
				}
			}
		}
	}
}

void InsaneRebel2::renderHandler7Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 7 Ship Rendering (Third-Person Ship - FLY sprites)
	// Based on FUN_0040d836 lines 173-185:
	//   FUN_004236e0(buf, frameInfo, screenX - 0xd4, screenY - 0x82, 0, sprite, frameIdx, 1, 0)
	// The ship sprite is drawn at the perspective-transformed position offset from center.
	// FUN_0041c720 transforms game coords (shipX, shipY) using perspective offsets.

	if (_rebelHandler != 7 || !_flyShipSprite || _shipLevelMode == 5)
		return;

	int numSprites = _flyShipSprite->getNumChars();
	int spriteIndex = _shipDirectionIndex;
	if (spriteIndex < 0)
		spriteIndex = 0;
	if (spriteIndex >= numSprites)
		spriteIndex = numSprites - 1;

	// Transform game coordinates to screen coordinates (FUN_0041c720 equivalent)
	// The perspective transform shifts the ship position based on perspective offsets.
	// Close view: FOBJ offset = (-52 - perspX, -45 - perspY), ship at screen center.
	// For now, use a simplified perspective: ship position = center + offset from center
	// scaled by perspective. In the original, FUN_00424510 shifts all FOBJ sprites.
	//
	// Screen position for sprite drawing (FUN_0040d836 line 174):
	//   drawX = transformedX - 0xd4, drawY = transformedY - 0x82
	// Where transformedX/Y come from FUN_0041c720(shipX, shipY, perspX, perspY, viewShift)
	//
	// Simplified: screenX = 160 + (shipX - 212) * perspFactor
	// With the perspective formula, objects near center barely move, objects at edges move more.
	int drawX = (_flyShipScreenX - 0xd4) + _perspectiveX;
	int drawY = (_flyShipScreenY - 0x82) + _perspectiveY;

	// Convert from game-center-relative to screen coordinates
	// The sprite system expects coordinates relative to the 320x200 frame
	// Center of frame = (160, 100), so offset = game position - game center
	drawX += 160 + _viewX;
	drawY += 100 + _viewY;

	// Center the sprite on the position
	int spriteW = _flyShipSprite->getCharWidth(spriteIndex);
	int spriteH = _flyShipSprite->getCharHeight(spriteIndex);
	drawX -= spriteW / 2;
	drawY -= spriteH / 2;

	renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _flyShipSprite, spriteIndex);

	// Laser overlay if firing (same position as ship)
	if (_shipFiring && _flyLaserSprite && _flyLaserSprite->getNumChars() > 0) {
		int laserIndex = spriteIndex % _flyLaserSprite->getNumChars();
		renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _flyLaserSprite, laserIndex);
	}

	debug("Rebel2 Handler7Ship: draw=(%d,%d) sprite=%d/%d shipPos=(%d,%d) persp=(%d,%d) smoothVel=%d vertIn=%d",
		drawX, drawY, spriteIndex, numSprites, _flyShipScreenX, _flyShipScreenY,
		_perspectiveX, _perspectiveY, _smoothedVelocity, _verticalInput);
}

void InsaneRebel2::renderHandler8Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 8 Ship Rendering (Third-Person On Foot - POV sprites)
	// Uses _shipSprite (POV001) with position-based offset

	if (_rebelHandler != 8 || !_shipSprite || _shipLevelMode == 5)
		return;

	// Calculate display offset from raw ship position (FUN_00401ccf lines 88-89)
	int16 displayOffsetX = (_shipPosX - 0xa0) >> 3;
	int16 displayOffsetY = (_shipPosY - 0x28) >> 2;

	// Base screen position (low-res: X=160, Y=105)
	int shipScreenX = 0xa0 + displayOffsetX;
	int shipScreenY = 0x69 + displayOffsetY;

	int numSprites = _shipSprite->getNumChars();
	int spriteIndex = 0;

	// Select sprite based on direction and sprite count
	if (numSprites >= 35) {
		spriteIndex = _shipDirectionH * 7 + _shipDirectionV;
		if (spriteIndex >= numSprites)
			spriteIndex = numSprites - 1;
	} else if (numSprites >= 25) {
		int vDir5 = (_shipDirectionV * 5) / 7;
		spriteIndex = _shipDirectionH * 5 + vDir5;
		if (spriteIndex >= numSprites)
			spriteIndex = numSprites - 1;
	} else if (numSprites >= 5) {
		spriteIndex = _shipDirectionH;
		if (spriteIndex >= numSprites)
			spriteIndex = numSprites - 1;
	} else if (numSprites == 2) {
		spriteIndex = _shipFiring ? 1 : 0;
	}

	// Center sprite at position
	int spriteW = _shipSprite->getCharWidth(spriteIndex);
	int spriteH = _shipSprite->getCharHeight(spriteIndex);
	int drawX = shipScreenX - spriteW / 2 + _viewX;
	int drawY = shipScreenY - spriteH / 2 + _viewY;

	renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _shipSprite, spriteIndex);

	// Shadow sprite (POV004 / DAT_0047e028): drawn at same position as primary ship.
	// Original FUN_401CCF lines 91-92 uses param_5 & 1 (firing flag) as sprite index
	// for both primary and shadow, NOT the direction-based spriteIndex.
	if (_shipSprite2) {
		int shadowIndex = _shipFiring ? 1 : 0;
		if (shadowIndex < _shipSprite2->getNumChars()) {
			renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _shipSprite2, shadowIndex);
		}
	}

	debug("Rebel2 Handler8: Ship at (%d,%d) raw(%d,%d) offset(%d,%d) sprite=%d/%d dir=(%d,%d)",
		drawX, drawY, _shipPosX, _shipPosY, displayOffsetX, displayOffsetY,
		spriteIndex, numSprites, _shipDirectionH, _shipDirectionV);
}

// Handler 25: Draw GRD001 (wall/cockpit overlay) in procPostRendering.
// Per original FUN_0041DB5E, GRD sprites are drawn AFTER FOBJ enemies, before GRD002.
//
// From FUN_0041db5e disassembly (lines 202-221):
// - Mode 1 with damage==0: Width halved (left half only, pixels 0-159)
// - Mode 4 with damage==0: Width halved AND buffer offset (right half only, pixels 160-319)
// - All other cases: Full width (320 pixels)
void InsaneRebel2::renderHandler25ShipPre(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 25)
		return;

	if (!_grd001Sprite || _grd001Sprite->getNumChars() <= 0)
		return;

	// CRITICAL: Clip height to 180 (0xb4) to avoid drawing over status bar
	const int clipHeight = 180;
	int renderHeight = MIN(height, clipHeight);

	// Draw _grd001Sprite based on _grdSpriteMode (DAT_00457900)
	// Each mode has specific conditions from FUN_0041db5e:
	bool shouldDraw = false;
	bool useHalfWidth = false;
	bool useRightHalf = false;

	// Mode 1 (lines 202-210): Draw with width halving when uncovered
	if (_grdSpriteMode == 1) {
		shouldDraw = true;
		useHalfWidth = (_rebelDamageLevel == 0);  // Half width when uncovered
	}
	// Mode 2 (lines 222-224): Only draw when damaged (covered)
	else if (_grdSpriteMode == 2 && _rebelDamageLevel != 0) {
		shouldDraw = true;
	}
	// Mode 3 (lines 225-228): Always draw full width
	else if (_grdSpriteMode == 3) {
		shouldDraw = true;
	}
	// Mode 4 (lines 211-221): Draw to right half when uncovered
	else if (_grdSpriteMode == 4) {
		shouldDraw = true;
		useHalfWidth = (_rebelDamageLevel == 0);
		useRightHalf = (_rebelDamageLevel == 0);
	}

	if (shouldDraw) {
		int spriteW = _grd001Sprite->getCharWidth(0);
		int spriteH = _grd001Sprite->getCharHeight(0);
		int16 spriteXOffset = _grd001Sprite->getCharXOffset(0);
		int16 spriteYOffset = _grd001Sprite->getCharYOffset(0);

		int drawX = _rebelViewOffset2X + spriteXOffset;
		int drawY = _rebelViewOffset2Y + spriteYOffset;

		// Apply width-halving logic from original assembly:
		// When damage==0 (uncovered), the original halves DAT_00482234 (buffer width)
		// This clips the sprite to only half the screen.
		int renderWidth = width;
		byte *dstBitmap = renderBitmap;

		if (useHalfWidth) {
			renderWidth = width / 2;  // Clip to half width (160 pixels)

			if (useRightHalf) {
				// Mode 4: Draw to right half by offsetting the destination buffer
				// Original: DAT_00482230 += DAT_00482234 (adds 160 to buffer start)
				// This makes drawing appear on the right half (pixels 160-319)
				dstBitmap = renderBitmap + (width / 2);
			}
		}

		renderNutSprite(dstBitmap, pitch, renderWidth, renderHeight, drawX, drawY, _grd001Sprite, 0);

		debug("Rebel2 Handler25 PRE: GRD001 at (%d,%d) nutOff(%d,%d) viewOff(%d,%d) size(%d,%d) mode=%d dmg=%d halfW=%d rightHalf=%d renderW=%d",
			drawX, drawY, spriteXOffset, spriteYOffset, _rebelViewOffset2X, _rebelViewOffset2Y,
			spriteW, spriteH, _grdSpriteMode, _rebelDamageLevel, useHalfWidth ? 1 : 0, useRightHalf ? 1 : 0, renderWidth);
	}
}

void InsaneRebel2::renderHandler25Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 25 POST-rendering: Draw GRD002 (character sprite) on top of enemies.
	// GRD001 (wall/cockpit) is drawn before this via renderHandler25ShipPre().
	//
	// From FUN_0041db5e disassembly (lines 230-248):
	// GRD002 is drawn LAST (after enemies) so the character appears in front.

	if (_rebelHandler != 25)
		return;

	// CRITICAL: Clip height to 180 (0xb4) to avoid drawing over status bar
	const int clipHeight = 180;
	int renderHeight = MIN(height, clipHeight);

	// _grd002Sprite (GRD002) is always drawn if it exists (from FUN_41DB5E line 230)
	// The sprite index is calculated based on damage level and aiming position
	// From FUN_0041db5e lines 160-168:
	//   If damage == 0: index = yZone * 5 + xZone + 5 (aiming-based, 5-14)
	//   If damage != 0:
	//     If direction == 0: index = 5 - damage (0-5, covered left)
	//     If direction != 0: index = 25 - damage (20-25, covered right)
	if (_grd002Sprite && _grd002Sprite->getNumChars() > 0) {
		// Calculate sprite index based on damage level and direction
		int spriteIdx;
		int numSprites = _grd002Sprite->getNumChars();

		// Determine if we should mirror the sprite (from FUN_41DB5E lines 231-235)
		// Mirror when: direction != 0 AND damage == 0 (fully uncovered, facing right)
		bool shouldMirror = (_rebelFlightDir != 0) && (_rebelDamageLevel == 0);

		if (_rebelDamageLevel == 0) {
			// Uncovered state: use aiming-based sprite selection (5-14)
			// Calculate zones from crosshair position relative to playable area
			// From FUN_41DB5E lines 155-164
			//
			// The playable area bounds are defined by corridor boundaries.
			// xZone = 0-4 (left to right), yZone = 0-1 (top to bottom)
			// Default to center if bounds not set
			int16 areaLeft = (_corridorLeftX > 0) ? _corridorLeftX : 0;
			int16 areaRight = (_corridorRightX > 0) ? _corridorRightX : 320;
			int16 areaTop = (_corridorTopY > 0) ? _corridorTopY : 0;
			int16 areaBottom = (_corridorBottomY > 0) ? _corridorBottomY : 180;

			// Get crosshair position (using mouse position scaled to game coords)
			int16 crosshairX = _vm->_mouse.x;
			int16 crosshairY = _vm->_mouse.y;
			if (_player && _player->_width > 320) {
				crosshairX = (crosshairX * 320) / _player->_width;
				crosshairY = (crosshairY * 200) / _player->_height;
			}

			// Calculate zone widths
			int areaWidth = areaRight - areaLeft;
			int areaHeight = areaBottom - areaTop;
			int zoneWidth = (areaWidth > 0) ? (areaWidth + 3) / 4 : 80;  // Divide into ~4 zones
			int zoneHeight = (areaHeight > 0) ? areaHeight / 2 : 90;     // Divide into 2 zones

			// Calculate xZone (0-4) and yZone (0-1) from crosshair position
			int xZone = (zoneWidth > 0) ? ((zoneWidth / 2) + (crosshairX - areaLeft)) / zoneWidth : 2;
			int yZone = (zoneHeight > 0) ? ((zoneHeight / 2) + (crosshairY - areaTop)) / zoneHeight : 0;

			// Clamp to valid ranges
			if (xZone < 0)
				xZone = 0;
			if (xZone > 4)
				xZone = 4;
			if (yZone < 0)
				yZone = 0;
			if (yZone > 1)
				yZone = 1;

			// Direction-based sprite flip logic (line 161-162 in decompiled)
			// if (DAT_00457902 == (uVar7 & 1)) { local_58 = 4 - local_58; }
			if (_rebelFlightDir == (yZone & 1)) {
				xZone = 4 - xZone;
			}

			spriteIdx = yZone * 5 + xZone + 5;
		} else {
			// Transitioning/covered state: use direction-based sprite
			// From FUN_41DB5E lines 166-168:
			// sVar8 = ((-(ushort)(DAT_00457902 == 0) & 0xffec) + 0x19) - DAT_0045790a
			// direction == 0: 5 - damage
			// direction != 0: 25 - damage
			if (_rebelFlightDir == 0) {
				// Direction 0: sprites 0-5 (transition left)
				spriteIdx = 5 - _rebelDamageLevel;
			} else {
				// Direction 1: sprites 20-25 (transition right)
				spriteIdx = 25 - _rebelDamageLevel;
			}
		}

		// Clamp to valid range
		if (spriteIdx < 0)
			spriteIdx = 0;
		if (spriteIdx >= numSprites)
			spriteIdx = numSprites - 1;

		int spriteW = _grd002Sprite->getCharWidth(spriteIdx);
		int spriteH = _grd002Sprite->getCharHeight(spriteIdx);

		// Position calculation from FUN_41DB5E lines 237-247:
		// GRD002 explicitly adds sprite internal offsets from NUT header:
		//
		// Normal case (direction==0 OR damage!=0):
		//   local_60 = sprite_internal_x_offset (from NUT header +0x12)
		//   X = DAT_00457910 + local_60
		//   Y = sprite_internal_y_offset (from NUT header +0x14) + DAT_00457912
		//
		// Mirrored case (direction!=0 AND damage==0):
		//   local_60 = 320 - sprite_width - sprite_internal_x_offset
		//   X = DAT_00457910 + local_60
		//   Y = sprite_internal_y_offset + DAT_00457912
		//
		// Now using actual NUT sprite offsets from NutRenderer!
		int16 spriteXOffset = _grd002Sprite->getCharXOffset(spriteIdx);
		int16 spriteYOffset = _grd002Sprite->getCharYOffset(spriteIdx);

		int drawX, drawY;

		if (shouldMirror) {
			// Mirrored position: X = DAT_00457910 + (320 - sprite_width - sprite_x_offset)
			// From assembly lines 240-243
			drawX = _rebelViewOffset2X + (320 - spriteW - spriteXOffset);
		} else {
			// Normal position: X = DAT_00457910 + sprite_internal_x_offset
			// From assembly line 238
			drawX = _rebelViewOffset2X + spriteXOffset;
		}

		// Y = sprite_internal_y_offset + DAT_00457912
		// From assembly line 246
		drawY = spriteYOffset + _rebelViewOffset2Y;

		renderNutSpriteMirrored(renderBitmap, pitch, width, renderHeight, drawX, drawY, _grd002Sprite, spriteIdx, shouldMirror);

		debug("Rebel2 Handler25: GRD002 at (%d,%d) nutOffset(%d,%d) viewOffset(%d,%d) size(%d,%d) spriteIdx=%d damage=%d dir=%d mirror=%d",
			drawX, drawY, spriteXOffset, spriteYOffset, _rebelViewOffset2X, _rebelViewOffset2Y, spriteW, spriteH, spriteIdx, _rebelDamageLevel, _rebelFlightDir, shouldMirror ? 1 : 0);
	}
}

void InsaneRebel2::renderFallbackShip(byte *renderBitmap, int pitch, int width, int height) {
	// Fallback: Use embedded HUD frame as ship sprite (Level 3 style)
	// userId=11 contains the ship sprite strip

	if ((_rebelHandler != 7 && _rebelHandler != 8) || _shipLevelMode == 5)
		return;

	// Skip if we have proper sprites
	if (_rebelHandler == 7 && _flyShipSprite)
		return;
	if (_rebelHandler == 8 && _shipSprite)
		return;

	EmbeddedSanFrame &shipFrame = _rebelEmbeddedHud[11];
	if (!shipFrame.valid || !shipFrame.pixels || shipFrame.width <= 0 || shipFrame.height <= 0)
		return;

	// Calculate display offset
	int16 displayOffsetX = (_shipPosX - 0xa0) >> 3;
	int16 displayOffsetY = (_shipPosY - 0x28) >> 2;
	int shipScreenX = 0xa0 + displayOffsetX;
	int shipScreenY = 0x69 + displayOffsetY;

	// Detect sprite strip layout
	int spriteW = shipFrame.width;
	int spriteH = shipFrame.height;
	int srcX = 0, srcY = 0;
	int numHorizontal = 1, numVertical = 1;

	if (spriteW >= 200 && spriteW % 5 == 0) {
		numHorizontal = 5;
		spriteW = shipFrame.width / 5;
	}
	if (spriteH >= 350 && spriteH % 7 == 0) {
		numVertical = 7;
		spriteH = shipFrame.height / 7;
	}

	int hDir = MIN((int)_shipDirectionH, numHorizontal - 1);
	int vDir = MIN((int)_shipDirectionV, numVertical - 1);
	srcX = hDir * spriteW;
	srcY = vDir * spriteH;

	int drawX = shipScreenX - spriteW / 2 + _viewX;
	int drawY = shipScreenY - spriteH / 2 + _viewY;

	// Blit from embedded HUD
	for (int y = 0; y < spriteH && (drawY + y) < height; y++) {
		if (drawY + y < 0)
			continue;
		for (int x = 0; x < spriteW && (drawX + x) < width; x++) {
			if (drawX + x < 0)
				continue;
			int srcIdx = (srcY + y) * shipFrame.width + (srcX + x);
			byte pixel = shipFrame.pixels[srcIdx];
			if (pixel != 0 && pixel != 231) {
				int dstIdx = (drawY + y) * pitch + (drawX + x);
				renderBitmap[dstIdx] = pixel;
			}
		}
	}

	debug("Rebel2: Ship (fallback) at (%d,%d) strip=(%d,%d) of (%dx%d) dir=(%d,%d)",
		drawX, drawY, srcX, srcY, numHorizontal, numVertical, _shipDirectionH, _shipDirectionV);
}

void InsaneRebel2::renderEnemyOverlays(byte *renderBitmap, int pitch, int width, int height, int videoWidth) {
	// Draw enemy indicator brackets for active enemies
	//
	// NOTE: Do NOT fill destroyed enemy areas with black. The original game does not do this.
	// When an enemy is destroyed:
	// 1. setBit(enemy_id) disables the enemy in the bit table
	// 2. clearBit(dependency_id) enables dependent objects (explosion animations)
	// 3. SKIP chunks in the video cause enemy FOBJ sprites to be skipped (via procSKIP)
	// 4. renderExplosions() draws the explosion animation from the 5-slot system
	// 5. The background video shows through where the enemy was

	// Draw green brackets for active enemies (Easy/Medium difficulty only)
	if (_difficulty >= 2)
		return;

	// FOBJ sprites are rendered with _fobjOffsetX/Y applied (set from _rebelViewOffsetX/Y
	// for Handler 25). Brackets must use the same offset so they align with the sprites.
	int fobjOffX = _player ? _player->_fobjOffsetX : 0;
	int fobjOffY = _player ? _player->_fobjOffsetY : 0;

	Common::Rect viewRect(_viewX, _viewY, _viewX + videoWidth, _viewY + 200);

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->destroyed || !it->active || isBitSet(it->id))
			continue;

		Common::Rect r(it->rect.left + fobjOffX, it->rect.top + fobjOffY,
		               it->rect.right + fobjOffX, it->rect.bottom + fobjOffY);
		if (r.right <= viewRect.left || r.left >= viewRect.right ||
		    r.bottom <= viewRect.top || r.top >= viewRect.bottom)
			continue;

		const byte color = 5;  // Green
		drawCornerBrackets(renderBitmap, pitch, width, height, r.left, r.top, r.width(), r.height(), color);
	}
}

// Dispatcher — calls per-handler explosion render function.
// Original code has separate functions per handler, each with its own
// position transformation, scale thresholds, and secondary NUT rendering.
// Each handler's render function checks DAT_0047e108 flags & 1:
// when bit 0 is set, explosion NUT sprites are suppressed (counter still ticks).
void InsaneRebel2::renderExplosions(byte *renderBitmap, int pitch, int width, int height) {
	// Check flags bit 0: suppress explosion sprite rendering
	LevelDifficultyParams dparams = getDifficultyParams();
	bool suppressExplosionSprites = (dparams.flags & 1) != 0;

	// Even when suppressed, still tick down explosion counters
	if (suppressExplosionSprites) {
		for (int i = 0; i < 5; i++) {
			if (_explosions[i].active && _explosions[i].counter > 0) {
				_explosions[i].counter--;
				if (_explosions[i].counter <= 0)
					_explosions[i].active = false;
			}
		}
		return;
	}

	switch (_rebelHandler) {
	case 0x26:
		renderTurretExplosions(renderBitmap, pitch, width, height);
		break;
	case 8:
		renderVehicleExplosions(renderBitmap, pitch, width, height);
		break;
	case 7:
		renderSpaceExplosions(renderBitmap, pitch, width, height);
		break;
	case 25:
		renderHandler25Explosions(renderBitmap, pitch, width, height);
		break;
	default:
		break;
	}
}

// FUN_409FBC — Handler 0x26 (Turret/Cockpit) explosion rendering.
// Position: Uses FUN_0041c720 for 3D→2D projection. At low-res, world coords ≈ screen coords.
// Scale thresholds: Fixed (<11, <21).
// Secondary NUT: DAT_0047fe80 (rendered if DAT_0047a7fc >= 0).
// Hi-res: Coordinates doubled when DAT_0047a808 >= 2.
void InsaneRebel2::renderTurretExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// FUN_409FBC: Fixed thresholds (0x0b=11, 0x15=21)
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;   // Small (sprites 11-20)
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;  // Medium (sprites 21-30)
		} else {
			baseIndex = 29;  // Large (sprites 31-40)
		}

		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		// Position: world coords passed through FUN_0041c720 (3D→2D projection).
		// At 320x200 low-res turret view, projection is effectively identity.
		int screenX = _explosions[i].x;
		int screenY = _explosions[i].y;

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				screenX - ew / 2, screenY - eh / 2, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}
}

// FUN_402696 — Handler 8 (Third-Person On-Foot) explosion rendering.
// Position: World coords minus camera offset (DAT_0043e006/DAT_0043e008 = _viewX/_viewY).
// Scale thresholds: Fixed (<11, <21) — same as handler 0x26.
// Secondary NUT: None (only DAT_0047a828).
void InsaneRebel2::renderVehicleExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// FUN_402696: Fixed thresholds (0x0b=11, 0x15=21)
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;
		} else {
			baseIndex = 29;
		}

		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		// FUN_402696 line 22-23: screenX = worldX - DAT_0043e006, screenY = worldY - DAT_0043e008
		int screenX = _explosions[i].x - _viewX;
		int screenY = _explosions[i].y - _viewY;

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				screenX - ew / 2, screenY - eh / 2, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}
}

// FUN_40F1C5 — Handler 7 (Third-Person Ship) explosion rendering.
// Position: Uses FUN_0041c720 for 3D→2D projection.
// Scale thresholds: Resolution-dependent (low-res: <11/<21, high-res: <21/<41).
// Secondary NUT: DAT_0047ff00 (FLY004, rendered if DAT_0047a7fc >= 0).
void InsaneRebel2::renderSpaceExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	// --- Part 1: Space shot explosions (FUN_40F1C5 lines 19-60) ---
	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// FUN_40F1C5 lines 41-51: Resolution-dependent thresholds.
		// Low-res (DAT_0047a808 < 2): thresholds 20, 10
		// High-res: thresholds 40, 20
		// We run at low-res (320x200), so use 10/20 (same as fixed handlers).
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;
		} else {
			baseIndex = 29;
		}

		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		// Position: world coords through FUN_0041c720 (3D→2D projection).
		// At low-res, this is close to identity for the ship view.
		int screenX = _explosions[i].x;
		int screenY = _explosions[i].y;

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				screenX - ew / 2, screenY - eh / 2, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}

	// --- Part 2: Corridor/zone hit explosion (FUN_40F1C5 lines 61-85) ---
	// Rendered when _hitCooldown > 0 (DAT_0044374c). Decrement happens HERE
	// (matching original where FUN_40F1C5 decrements DAT_0044374c during render).
	// _spaceShotDirection (DAT_0044374e) determines explosion side:
	//   0 = left side (hit left boundary), 1 = right side (hit right boundary)
	//   2 = bottom (zone push down), 3 = top (zone push up)
	// Sprite frames: 0x15 - cooldown = 21 - cooldown (frames 12→21 as cooldown 9→0)
	if (_hitCooldown != 0) {
		_hitCooldown--;

		int numChars = _smush_iconsNut->getNumChars();
		int spriteIndex = 0x15 - _hitCooldown;  // 21 - remaining cooldown

		if (spriteIndex >= 0 && spriteIndex < numChars) {
			// Compute ship screen position (simplified FUN_0041c720 transform)
			int shipDrawX = (_flyShipScreenX - 0xd4) + _perspectiveX + 160 + _viewX;
			int shipDrawY = (_flyShipScreenY - 0x82) + _perspectiveY + 100 + _viewY;

			// Per-direction offset from ship center.
			// Original uses lookup tables (DAT_004438da etc.) indexed by
			// _shipDirectionIndex (35 entries per direction). We approximate
			// with fixed offsets since we don't have the table data.
			int offsetX = 0, offsetY = 0;
			switch (_spaceShotDirection) {
			case 0:  // Left wall hit → explosion on left side of ship
				offsetX = -35;
				break;
			case 1:  // Right wall hit → explosion on right side of ship
				offsetX = 35;
				break;
			case 2:  // Zone push down → explosion on bottom
				offsetY = 20;
				break;
			case 3:  // Zone push up → explosion on top
				offsetY = -20;
				break;
			default:
				break;
			}

			int drawX = shipDrawX + offsetX;
			int drawY = shipDrawY + offsetY;

			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				drawX - ew / 2, drawY - eh / 2, _smush_iconsNut, spriteIndex);

			debug("Rebel2 H7 corridor explosion: dir=%d frame=%d pos=(%d,%d) cooldown=%d",
				_spaceShotDirection, spriteIndex, drawX, drawY, _hitCooldown);
		}
	}
}

// FUN_41F29A — Handler 25 (FPS/Mixed) explosion rendering.
// Position: World coords + view offset (DAT_0045790c/DAT_0045790e = _rebelViewOffsetX/_rebelViewOffsetY).
// Scale thresholds: Resolution-dependent (same formula as Handler 7).
// Secondary NUT: DAT_00482260 (hi-res HUD alternative, rendered if DAT_0047a7fc >= 0).
// Note: No per-frame sound panning update (unlike handlers 0x26, 8, 7).
void InsaneRebel2::renderHandler25Explosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// FUN_41F29A lines 27-37: Resolution-dependent thresholds (same as Handler 7).
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;
		} else {
			baseIndex = 29;
		}

		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		// FUN_41F29A line 22-23: screenX = worldX + DAT_0045790c, screenY = worldY + DAT_0045790e
		int screenX = _explosions[i].x + _rebelViewOffsetX;
		int screenY = _explosions[i].y + _rebelViewOffsetY;

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				screenX - ew / 2, screenY - eh / 2, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}
}

// Dispatcher - calls appropriate render function based on current handler
void InsaneRebel2::renderLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	switch (_rebelHandler) {
	case 0x26:  // Turret - FUN_40AD63
		renderTurretLaserShots(renderBitmap, pitch, width, height);
		break;
	case 8:     // Vehicle - FUN_402ED0
		renderVehicleLaserShots(renderBitmap, pitch, width, height);
		break;
	case 7:     // Space combat - FUN_40FADF
		renderSpaceLaserShots(renderBitmap, pitch, width, height);
		break;
	case 25:    // Speeder bike - FUN_0041f004
		renderHandler25LaserShots(renderBitmap, pitch, width, height);
		break;
	default:
		// No laser rendering for other handlers
		break;
	}
}

// Handler 0x26 Turret laser rendering (FUN_40AD63)
// Gun positions depend on _rebelLevelType (DAT_004436de)
void InsaneRebel2::renderTurretLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// Uses pre-initialized _laserTexture from sprite 5 of CPITIMAG.NUT

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter <= 0)
			continue;

		// Calculate sound panning from target X position (FUN_004262f0 call)
		// sVar1 = ((2 - counter) * (targetX - 160)) / 2, clamped to [-127, 127]
		int16 pan = ((2 - _turretShots[i].counter) * (_turretShots[i].targetX - _viewX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning to sound channel i+1

		int16 targetX = _turretShots[i].targetX;
		int16 targetY = _turretShots[i].targetY;
		int16 progress = maxDuration - _turretShots[i].counter;

		// Gun positions based on level type (from FUN_40AD63 switch statement)
		// Parameters from assembly: widthScale=0xC(12), heightScale=8, thickness=0xC(12)
		switch (_rebelLevelType) {
		case 1:
			// Type 1: 3 guns (triple cannon configuration)
			// Gun 1: (0x136, 0xaa) = (310, 170) - right
			// Gun 2: (0xa0, 0x17c) = (160, 380) - center bottom (off-screen, clipped)
			// Gun 3: (0x0a, 0xaa) = (10, 170) - left
			drawLaserBeam(renderBitmap, pitch, width, height,
				310 + _viewX, 170 + _viewY, targetX, targetY,
				progress, maxDuration, 12, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				160 + _viewX, 380 + _viewY, targetX, targetY,
				progress, maxDuration, 8, 5, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				10 + _viewX, 170 + _viewY, targetX, targetY,
				progress, maxDuration, 12, 8, 12);
			break;

		case 2:
		case 5:
			// Type 2/5: 2 guns (wing cannons)
			// Left: (0x6e, 0xe6) = (110, 230)
			// Right: (0xd2, 0xe6) = (210, 230)
			// Assembly: widthScale=0x19(25), heightScale=8, thickness=0xC(12)
			drawLaserBeam(renderBitmap, pitch, width, height,
				110 + _viewX, 230 + _viewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				210 + _viewX, 230 + _viewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);
			break;

		case 6:
			// Type 6: 2 guns (offscreen - cinematic effect)
			// Gun 1: (-100, 0)
			// Gun 2: (0, 0)
			// Assembly: widthScale=0x19(25), heightScale=8, thickness=0xC(12)
			drawLaserBeam(renderBitmap, pitch, width, height,
				-100 + _viewX, 0 + _viewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				0 + _viewX, 0 + _viewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);
			break;

		default:
			// Default: 2 guns with alternating pattern based on shot sequence
			// When seqNum & 1 == 0: Left (10, 50), Right (310, 130)
			// When seqNum & 1 == 1: Left (310, 50), Right (10, 130)
			// Assembly: widthScale=0x19(25), heightScale=8, thickness=0xC(12)
			if ((_turretShots[i].seqNum & 1) == 0) {
				drawLaserBeam(renderBitmap, pitch, width, height,
					10 + _viewX, 50 + _viewY, targetX, targetY,
					progress, maxDuration, 25, 8, 12);

				drawLaserBeam(renderBitmap, pitch, width, height,
					310 + _viewX, 130 + _viewY, targetX, targetY,
					progress, maxDuration, 25, 8, 12);
			} else {
				drawLaserBeam(renderBitmap, pitch, width, height,
					310 + _viewX, 50 + _viewY, targetX, targetY,
					progress, maxDuration, 25, 8, 12);

				drawLaserBeam(renderBitmap, pitch, width, height,
					10 + _viewX, 130 + _viewY, targetX, targetY,
					progress, maxDuration, 25, 8, 12);
			}
			break;
		}

		_turretShots[i].counter--;
	}
}

// Handler 8 Vehicle laser rendering (FUN_402ED0)
// In the original, the laser is a short muzzle flash from gun barrel toward ship center,
// NOT a projectile traveling across the screen. The "hit" effect is handled separately.
void InsaneRebel2::renderVehicleLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// No NUT check needed - uses pre-initialized _laserTexture

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_vehicleShots[i].counter <= 0)
			continue;

		// Calculate sound panning from STORED target position (FUN_402ED0 lines 24-51)
		// pan = ((2 - counter) * (targetX - 160)) / 2, clamped to [-127, 127]
		int16 pan = ((2 - _vehicleShots[i].counter) * (_vehicleShots[i].targetX - _viewX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning

		// Calculate positions from CURRENT ship position (FUN_402ED0 lines 53-122)
		// The original game draws the laser from gun position toward ship center,
		// creating a short muzzle flash effect (7 pixels horizontal, 25 pixels vertical).
		//
		// Low-res formula (DAT_0047a808 < 2):
		// shipScreenY = ((shipPosY - 0x28) >> 2) + 0x69 = ((shipPosY - 40) >> 2) + 105
		// shipScreenX = ((shipPosX - 0xa0) >> 3) + 0xa0 = ((shipPosX - 160) >> 3) + 160
		// gunY = ((shipPosY - 0x28) >> 2) + 0x82 = shipScreenY + 25
		// gunX = ((shipPosX - 0xa0) >> 3) + 0xa7 = shipScreenX + 7
		int16 shipScreenX = ((_shipPosX - 160) >> 3) + 160;
		int16 shipScreenY = ((_shipPosY - 40) >> 2) + 105;
		int16 gunX = shipScreenX + 7;
		int16 gunY = shipScreenY + 25;

		int16 progress = maxDuration - _vehicleShots[i].counter;

		// Draw beam from gun toward ship center (muzzle flash effect)
		// From FUN_402ED0: widthScale=0x14(20), heightScale=8, thickness=4
		// Parameters: gunX, gunY -> shipScreenX, shipScreenY (NOT the stored target!)
		drawLaserBeam(renderBitmap, pitch, width, height,
			gunX + _viewX, gunY + _viewY,
			shipScreenX + _viewX, shipScreenY + _viewY,
			progress, maxDuration, 20, 8, 4);

		_vehicleShots[i].counter--;
	}
}

// Handler 7 Space combat laser rendering (FUN_40FADF)
// Dual beams from left and right gun positions
void InsaneRebel2::renderSpaceLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// No NUT check needed - uses pre-initialized _laserTexture

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_spaceShots[i].counter <= 0)
			continue;

		// Calculate sound panning
		int16 pan = ((_spaceShots[i].targetX - 160) * (2 - _spaceShots[i].counter)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning

		int16 targetX = _spaceShots[i].targetX;
		int16 targetY = _spaceShots[i].targetY;
		int16 leftGunX = _spaceShots[i].leftGunX;
		int16 leftGunY = _spaceShots[i].leftGunY;
		int16 rightGunX = _spaceShots[i].rightGunX;
		int16 rightGunY = _spaceShots[i].rightGunY;
		int16 progress = maxDuration - _spaceShots[i].counter;

		// Draw dual beams
		// From FUN_40FADF: widthScale=0xC(12), heightScale=4, thickness=6
		// Left gun beam
		drawLaserBeam(renderBitmap, pitch, width, height,
			leftGunX, leftGunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		// Right gun beam
		drawLaserBeam(renderBitmap, pitch, width, height,
			rightGunX, rightGunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		_spaceShots[i].counter--;
	}
}

// Handler 25 laser rendering (FUN_0041f004)
// Speeder bike laser shots - draws beam from gun position to target
void InsaneRebel2::renderHandler25LaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// FUN_0041f004 uses turret-style shot slots with view offset adjustment
	// Only render when player is uncovered (damage == 0)
	if (_rebelDamageLevel != 0) {
		return;  // Can't shoot while taking cover
	}

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter <= 0)
			continue;

		// Calculate sound panning from target X position (FUN_004262f0)
		// sVar1 = ((2 - counter) * (targetX - 160)) / 2, clamped to [-127, 127]
		int16 pan = ((2 - _turretShots[i].counter) * (_turretShots[i].targetX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning to sound channel i+1

		// Target position (where player clicked)
		int16 targetX = _turretShots[i].targetX;
		int16 targetY = _turretShots[i].targetY;

		// Gun position computed at spawn time from GRD002 sprite data
		// Original: DAT_0045791c[i] + DAT_0045790c, DAT_00457920[i] + DAT_0045790e
		int16 gunX = _turretShots[i].gunX;
		int16 gunY = _turretShots[i].gunY;

		int16 progress = maxDuration - _turretShots[i].counter;

		// From FUN_0041f004 parameters for FUN_0040bbf6:
		// widthScale=0xC(12), heightScale=4, thickness=6
		drawLaserBeam(renderBitmap, pitch, width, height,
			gunX, gunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		_turretShots[i].counter--;

		debug("Rebel2 Handler25: Laser shot %d from (%d,%d) to (%d,%d) progress=%d/%d",
			i, gunX, gunY, targetX, targetY, progress, maxDuration);
	}
}

void InsaneRebel2::renderCrosshair(byte *renderBitmap, int pitch, int width, int height) {
	// From FUN_0040d836 (Handler 7) line 167-168: crosshair only drawn when DAT_004437c0 == 2
	// Don't draw crosshair when shooting is disabled (flight-only segments)
	if (!isShootingAllowed()) {
		return;
	}

	// Handler 25 (0x19): From FUN_41DB5E lines 195-197, crosshair only drawn when
	// DAT_0045790a == 0 (fully uncovered). Hide crosshair during cover transition.
	if (_rebelHandler == 25 && _rebelDamageLevel != 0) {
		return;
	}

	// Update target lock state and draw crosshair/reticle

	// Target lock detection (DAT_00443676 equivalent)
	Common::Point worldMousePos(_vm->_mouse.x + _viewX, _vm->_mouse.y + _viewY);
	bool targetLocked = false;

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->active && !it->destroyed && it->rect.contains(worldMousePos)) {
			targetLocked = true;
			break;
		}
	}

	if (targetLocked) {
		_targetLockTimer = 7;
	} else if (_targetLockTimer > 0) {
		_targetLockTimer--;
	}

	// Draw crosshair
	if (!_smush_iconsNut)
		return;

	int reticleIndex;
	switch (_rebelHandler) {
	case 7:    // Third-Person Ship
	case 0x19: // FPS/Mixed (Handler 25)
		reticleIndex = 47;  // 0x2F
		break;
	case 0x26: { // Turret/Cockpit - animated crosshair
		static int turretAnimCounter = 0;
		turretAnimCounter++;

		int animOffset = (_targetLockTimer == 0) ? 0 : 3 - (turretAnimCounter & 3);

		if (_rebelLevelType == 5) {
			reticleIndex = 0x30 + animOffset;
		} else {
			reticleIndex = animOffset;
		}
		break;
	}
	case 8:    // Third-Person On Foot
	default:
		reticleIndex = 46;
		break;
	}

	if (_smush_iconsNut->getNumChars() > reticleIndex) {
		int cw = _smush_iconsNut->getCharWidth(reticleIndex);
		int ch = _smush_iconsNut->getCharHeight(reticleIndex);

		// Calculate crosshair position
		int crosshairX = _vm->_mouse.x - cw / 2 + _viewX;
		int crosshairY = _vm->_mouse.y - ch / 2 + _viewY;

		// Handler 25 (0x19): Add view offset to crosshair position
		// From FUN_41DB5E lines 198-199: X = DAT_00457914 + DAT_0045790c, Y = DAT_00457916 + DAT_0045790e
		if (_rebelHandler == 25) {
			crosshairX += _rebelViewOffsetX;
			crosshairY += _rebelViewOffsetY;
		}

		renderNutSprite(renderBitmap, pitch, width, height,
			crosshairX, crosshairY,
			_smush_iconsNut, reticleIndex);
	}
}

void InsaneRebel2::frameEndCleanup() {
	// Reset enemy active flags and collision zones at frame end
	// The original game rebuilds lists from scratch each frame

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (!it->destroyed) {
			it->active = false;
		}
	}

	resetCollisionZones();
}

} // End of namespace Scumm
