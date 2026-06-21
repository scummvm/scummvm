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
#include "graphics/managed_surface.h"

#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/rebel/codec_ra2.h"
#include "scumm/smush/rebel/font_rebel2.h"
#include "scumm/smush/rebel/smush_player_ra2.h"

#include "scumm/insane/rebel2/rebel.h"

namespace Scumm {

// External codec functions from codec1.cpp
extern void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
extern void smushDecodeUncompressed(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

int getRebel2IndicatorScale(int width, int height) {
	// RA2's 424x260 low-res gameplay buffer is still displayed through a 320x200
	return (width >= 640 || height >= 400) ? 2 : 1;
}

static bool isValidEmbeddedFrame(const InsaneRebel2::EmbeddedSanFrame &frame) {
	return frame.valid && frame.pixels && frame.width > 0 && frame.height > 0;
}

static int countEmbeddedFramePixels(const InsaneRebel2::EmbeddedSanFrame &frame) {
	if (!isValidEmbeddedFrame(frame))
		return 0;

	int count = 0;
	for (int i = 0; i < frame.width * frame.height; i++) {
		if (frame.pixels[i] != 0)
			count++;
	}

	return count;
}

bool parseRebel2TextOverlayFormat(const char *&str, NutRenderer *&curFont, int &curColor,
		NutRenderer **fonts, int numFonts, NutRenderer *defaultFont) {
	int fontId = InsaneRebel2::parseFormatCode(str, curColor);
	if (fontId >= 0) {
		curFont = (fontId < numFonts && fonts[fontId]) ? fonts[fontId] : defaultFont;
		return true;
	}

	return fontId == -2;
}

const char *getHandler8PovOverlayString(int id) {
	switch (id) {
	case 200:
		return "^f03^c248 MODE  ^c005 INFARED";
	case 201:
		return "^f03^c248 SCANNING";
	case 202:
		return "^f03^c005 VHL EMM %lx";
	case 203:
		return "^f03^c005 SCOOBYNESS FACTOR %lx";
	case 204:
		return "^f03^c005 USELESS COORDINATES %lx %lx";
	case 205:
		return "^f03^c248 Buy Afterlife";
	case 206:
		return "^f03^c248 No Homework.. You must fight the bear";
	case 207:
		return "^f03^c248 What is that watermelon doing there?";
	case 208:
		return "^f03^c248 I just like to say taboo";
	case 209:
		return "^f03^c248 How about them Bears";
	case 210:
		return "^f03^c248 There is cause to be optimistic -HAL";
	case 211:
		return "^f03^c248 Little Darth is between his legs -LISA";
	default:
		return nullptr;
	}
}

int getHandler8PovOverlayRandom(ScummEngine_v7 *vm, int max) {
	if (max == 0)
		return 0;
	if (max < 0)
		return -vm->_rnd.getRandomNumber(-max - 1);
	return vm->_rnd.getRandomNumber(max - 1);
}

void drawHandler8PovOverlayText(const Rebel2FontSet &fontSet, byte *renderBitmap,
		int pitch, int width, int height, const char *str, int x, int y, int16 color,
		TextStyleFlags flags) {
	if (!str)
		return;

	Common::Rect clipRect(0, 0, width, height);
	drawRebel2String(fontSet, str, strlen(str), renderBitmap, clipRect, x, y, pitch, color, flags);
}

static void blitEmbeddedFrameRegion(byte *renderBitmap, int pitch, int clipWidth, int clipHeight,
		const InsaneRebel2::EmbeddedSanFrame &frame, int destX, int destY,
		int srcX, int srcY, int drawWidth, int drawHeight) {
	if (!renderBitmap || !isValidEmbeddedFrame(frame) || drawWidth <= 0 || drawHeight <= 0)
		return;
	if (srcX < 0 || srcY < 0)
		return;

	drawWidth = MIN(drawWidth, frame.width - srcX);
	drawHeight = MIN(drawHeight, frame.height - srcY);
	if (drawWidth <= 0 || drawHeight <= 0)
		return;

	for (int y = 0; y < drawHeight; y++) {
		int dy = destY + y;
		if (dy < 0 || dy >= clipHeight)
			continue;

		const byte *srcRow = frame.pixels + (srcY + y) * frame.width + srcX;
		byte *dstRow = renderBitmap + dy * pitch;
		for (int x = 0; x < drawWidth; x++) {
			int dx = destX + x;
			if (dx < 0 || dx >= clipWidth)
				continue;

			byte pixel = srcRow[x];
			if (pixel != 0 && pixel != 231)
				dstRow[dx] = pixel;
		}
	}
}

static bool readEmbeddedSanChunkHeader(Common::SeekableReadStream &stream, int64 containerEnd, const char *context,
		uint32 &tag, uint32 &chunkSize, int64 &dataEnd, int64 &nextChunkPos) {
	const int64 headerPos = stream.pos();
	if (headerPos < 0 || headerPos + 8 > containerEnd)
		return false;

	tag = stream.readUint32BE();
	chunkSize = stream.readUint32BE();

	const int64 dataStart = stream.pos();
	if ((int64)chunkSize > containerEnd - dataStart) {
		debugC(DEBUG_INSANE, "Truncated embedded SAN %s chunk 0x%08X at %lld: size=%u, remaining=%lld",
			context, tag, headerPos, chunkSize, containerEnd - dataStart);
		return false;
	}

	dataEnd = dataStart + chunkSize;
	nextChunkPos = dataEnd + (chunkSize & 1);
	if (nextChunkPos > containerEnd)
		nextChunkPos = dataEnd;

	return true;
}

// renderEmbeddedFrame -- Blit a decoded embedded frame to the video buffer.
void InsaneRebel2::renderEmbeddedFrame(byte *renderBitmap, const EmbeddedSanFrame &frame, int userId) {
	// Render the decoded embedded frame to the video buffer
	// Skip immediate draw for handlers that render HUD during post-processing:
	// Exception: Handler 25 (0x19) background overlays (par4/userId=4, 6, 7) should draw immediately.
	// These complete the visual scene and are NOT positioned by mouse/crosshair.
	bool skipImmediateDraw = (_rebelHandler == 7 || _rebelHandler == 8 ||
	                          _rebelHandler == 0x26);

	// Handler 25 overlays:
	// - userId 4 (corridor overlay): draw immediately at the current view offset.
	// - userId 6, 7 (static overlays): Draw immediately (they don't move)
	if (_rebelHandler == 0x19 && userId == 4) {
		drawHandler25CorridorOverlay(renderBitmap);
		return;
	}
	if (_rebelHandler == 0x19 && (userId == 6 || userId == 7)) {
		skipImmediateDraw = false;
		debugC(DEBUG_INSANE, "Handler 25 static overlay userId=%d - forcing immediate draw", userId);
	}

	if (!frame.valid || !renderBitmap || skipImmediateDraw) {
		if (skipImmediateDraw && frame.valid) {
			debugC(DEBUG_INSANE, "Skipped immediate draw for Handler %d HUD %d (will render during post-processing)",
				_rebelHandler, userId);
		}
		return;
	}

	int pitch = (_player && _player->_width > 0) ? _player->_width : 320;
	int bufHeight = (_player && _player->_height > 0) ? _player->_height : 200;

	blitEmbeddedFrameRegion(renderBitmap, pitch, pitch, bufHeight, frame,
		frame.renderX, frame.renderY, 0, 0, frame.width, frame.height);
	debugC(DEBUG_INSANE, "Rendered embedded HUD %d at (%d,%d)", userId, frame.renderX, frame.renderY);
}

void InsaneRebel2::drawHandler25CorridorOverlay(byte *renderBitmap) {
	if (!renderBitmap)
		return;

	EmbeddedSanFrame &corridorOverlay = _rebelEmbeddedHud[4];
	if (!isValidEmbeddedFrame(corridorOverlay))
		return;

	int pitch = (_player && _player->_width > 0) ? _player->_width : 320;
	int bufHeight = (_player && _player->_height > 0) ? _player->_height : 200;

	int srcOffsetX = 0;
	int srcOffsetY = 0;
	int destX = _rebelViewOffsetX;
	int destY = _rebelViewOffsetY;
	int drawWidth = corridorOverlay.width;
	int drawHeight = corridorOverlay.height;

	if (destX < 0) {
		srcOffsetX = -destX;
		drawWidth -= srcOffsetX;
		destX = 0;
	}
	if (destY < 0) {
		srcOffsetY = -destY;
		drawHeight -= srcOffsetY;
		destY = 0;
	}
	if (destX + drawWidth > pitch)
		drawWidth = pitch - destX;
	if (destY + drawHeight > bufHeight)
		drawHeight = bufHeight - destY;
	if (drawWidth > corridorOverlay.width - srcOffsetX)
		drawWidth = corridorOverlay.width - srcOffsetX;
	if (drawHeight > corridorOverlay.height - srcOffsetY)
		drawHeight = corridorOverlay.height - srcOffsetY;

	if (drawWidth <= 0 || drawHeight <= 0)
		return;

	for (int y = 0; y < drawHeight; y++) {
		memcpy(renderBitmap + (destY + y) * pitch + destX,
			   corridorOverlay.pixels + (srcOffsetY + y) * corridorOverlay.width + srcOffsetX,
			   drawWidth);
	}

	debugC(DEBUG_INSANE, "Handler25: Corridor overlay drawn at (%d,%d) size(%d,%d)",
		_rebelViewOffsetX, _rebelViewOffsetY, corridorOverlay.width, corridorOverlay.height);
}

// loadEmbeddedSan -- Decode an embedded SAN (ANIM/FOBJ) from IACT opcode 8 data.
// Parses ANIM container, extracts FOBJ codec data, decodes using codec 21/23/45,
// and stores the result in _rebelEmbeddedHud[userId] for later rendering.
void InsaneRebel2::loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) {
	// Validate userId - Level 3 uses slots 0-11, allow up to 15 for safety
	if (userId < 0 || userId > 15 || !animData || size < 8) {
		debugC(DEBUG_INSANE, "Invalid embedded SAN: userId=%d, size=%d", userId, size);
		return;
	}

	Common::MemoryReadStream stream(animData, size);
	const int64 streamEnd = stream.size();

	// Read ANIM header
	uint32 animTag = stream.readUint32BE();
	if (animTag != MKTAG('A','N','I','M')) {
		debugC(DEBUG_INSANE, "Embedded SAN missing ANIM tag, got 0x%08X", animTag);
		return;
	}
	uint32 animSize = stream.readUint32BE();
	int64 animEnd = streamEnd;
	if ((int64)animSize <= streamEnd - 8) {
		animEnd = 8 + (int64)animSize;
	} else {
		debugC(DEBUG_INSANE, "Embedded ANIM truncated: reported size=%u, actual=%lld", animSize, streamEnd - 8);
	}
	debugC(DEBUG_INSANE, "Parsing embedded ANIM: userId=%d, reported size=%u, actual=%lld", userId, animSize, streamEnd - 8);

	// Iterate through chunks to find FRME -> FOBJ
	while (!stream.eos() && stream.pos() + 8 <= animEnd) {
		uint32 tag;
		uint32 chunkSize;
		int64 chunkDataEnd;
		int64 nextChunkPos;
		if (!readEmbeddedSanChunkHeader(stream, animEnd, "top-level", tag, chunkSize, chunkDataEnd, nextChunkPos))
			break;

		if (tag == MKTAG('F','R','M','E')) {
			// Iterate sub-chunks in FRME
			while (stream.pos() + 8 <= chunkDataEnd && !stream.eos()) {
				uint32 subTag;
				uint32 subSize;
				int64 subDataEnd;
				int64 nextSubPos;
				if (!readEmbeddedSanChunkHeader(stream, chunkDataEnd, "FRME", subTag, subSize, subDataEnd, nextSubPos))
					break;

				if (subTag == MKTAG('F','O','B','J')) {
					if (subSize < 14) {
						debugC(DEBUG_INSANE, "Embedded FOBJ too small: userId=%d, size=%u", userId, subSize);
						stream.seek(nextSubPos);
						continue;
					}

					// Read FOBJ header
					int codec = stream.readUint16LE();
					int left = stream.readSint16LE();
					int top = stream.readSint16LE();
					int width = stream.readUint16LE();
					int height = stream.readUint16LE();
					stream.readUint16LE();  // unknown
					stream.readUint16LE();  // unknown

					debugC(DEBUG_INSANE, "Embedded HUD frame: userId=%d, %dx%d at (%d,%d), codec=%d",
						userId, width, height, left, top, codec);

					// High-resolution HUD frames are used when the RA2 high-res option
					// selects a 640x400 virtual screen. Keep skipping them in low-res mode.
					if (!isHiRes() && (width > 400 || height > 250)) {
						debugC(DEBUG_INSANE, "SKIPPING high-res embedded frame: userId=%d, %dx%d (exceeds 400x250)",
							userId, width, height);
						stream.seek(nextSubPos);
						continue;
					}

					// Allocate storage for the decoded frame
					EmbeddedSanFrame &frame = _rebelEmbeddedHud[userId];
					frame.valid = false;

					if (width > 0 && height > 0 && width <= 800 && height <= 480) {
						if (frame.width != width || frame.height != height || !frame.pixels) {
							free(frame.pixels);
							frame.pixels = (byte *)malloc(width * height);
							if (!frame.pixels) {
								warning("Rebel2: Failed to allocate embedded HUD frame: %dx%d", width, height);
								return;
							}
							frame.width = width;
							frame.height = height;
						}
						// Clear buffer before decode (important for delta codecs)
						memset(frame.pixels, 0, width * height);

						// Update render position from FOBJ header
						frame.renderX = left;
						frame.renderY = top;

						// Read the raw FOBJ data
						int32 dataSize = (int32)(subDataEnd - stream.pos());
						if (dataSize > 0) {
							byte *fobjData = (byte *)malloc(dataSize);
							if (!fobjData) {
								warning("Rebel2: Failed to allocate embedded FOBJ data: %d bytes", dataSize);
								return;
							}
							uint32 bytesRead = stream.read(fobjData, dataSize);
							if (bytesRead != (uint32)dataSize) {
								debugC(DEBUG_INSANE, "Short embedded FOBJ read: got %u of %d bytes", bytesRead, dataSize);
								free(fobjData);
								return;
							}

							if (codec == 1 || codec == 3) {
								smushDecodeRLE(frame.pixels, fobjData, 0, 0, width, height, width);
								frame.valid = true;
								debugC(DEBUG_INSANE, "Decoded embedded HUD (codec %d/RLE): %dx%d", codec, width, height);
							} else if (codec == 20) {
								smushDecodeUncompressed(frame.pixels, fobjData, 0, 0, width, height, width);
								frame.valid = true;
								debugC(DEBUG_INSANE, "Decoded embedded HUD (codec 20/raw): %dx%d", width, height);
							} else if (codec == 21 || codec == 44) {
								smushDecodeLineUpdate(frame.pixels, fobjData, 0, 0, width, height, width, dataSize);
								frame.valid = true;
								debugC(DEBUG_INSANE, "Decoded embedded HUD (codec %d/line update): %dx%d", codec, width, height);
							} else if (codec == 45) {
								smushDecodeRA2Blur(frame.pixels, fobjData, 0, 0, width, height, width, dataSize,
									_rebelEmbeddedCodec45Palette, _rebelEmbeddedCodec45Lookup);
								frame.valid = true;
							} else if (codec == 23) {
								smushDecodeSkipRLE(frame.pixels, fobjData, 0, 0, width, height, width, dataSize);
								frame.valid = true;
								debugC(DEBUG_INSANE, "Decoded embedded HUD (codec 23/skip-RLE): %dx%d", width, height);
							} else {
								debugC(DEBUG_INSANE, "Unsupported embedded HUD codec %d", codec);
								frame.valid = false;
							}

							// Count non-zero pixels to verify frame has content
							if (frame.valid) {
								int nonZeroPixels = countEmbeddedFramePixels(frame);
								debugC(DEBUG_INSANE, "Frame userId=%d has %d non-zero pixels (%d%%)",
									userId, nonZeroPixels, (nonZeroPixels * 100) / (width * height));
							}

							// Render the decoded frame to the video buffer
							renderEmbeddedFrame(renderBitmap, frame, userId);

							free(fobjData);
						}
					}

					// Done with FOBJ - assume only one relevant frame per embedded SAN
					return;
				} else {
					// Skip other sub-chunks (AHDR inside FRME?) or padding
					stream.seek(nextSubPos);
				}
			}
			stream.seek(nextChunkPos);
		} else {
			// Skip non-FRME chunks (AHDR, etc at top level)
			stream.seek(nextChunkPos);
		}
	}

	debugC(DEBUG_INSANE, "No FOBJ found in embedded SAN userId=%d", userId);
}

// Spawn explosion into the shared 5-slot system.
// spawnExplosion -- Allocate an explosion slot at the given position.
// free slot (counter==0), set counter=10, scale=objectHalfWidth, position=center.
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

// Used both as initial shot counter AND maxFrames for beam rendering.
int16 InsaneRebel2::getShotMaxDuration() {
	LevelDifficultyParams params = getDifficultyParams();
	// Clamp to reasonable range to avoid division by zero or extreme beams
	int16 duration = params.laserDelay;
	if (duration <= 0)
		duration = 4;  // Fallback for -1 entries (disabled levels)
	return duration;
}

// spawnShot -- Dispatch to per-handler shot spawn.
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
		break;
	}
}

void InsaneRebel2::spawnTurretShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter == 0) {
			// levelType 5: BLAST.SAD (slot 0), otherwise: TBLAST.SAD (slot 7)
			playSfx((_rebelLevelType == 5) ? 0 : 7, 127, 0);

			_turretShots[i].counter = getShotMaxDuration();
			_turretShots[i].seqNum = _turretShotSeqCounter;
			_turretShotSeqCounter++;
			_turretShots[i].targetX = x + _viewX;
			_turretShots[i].targetY = y + _viewY;
			break;
		}
	}
}

void InsaneRebel2::spawnVehicleShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_vehicleShots[i].counter == 0) {
			playSfx(6, 127, 0);
			_vehicleShots[i].counter = getShotMaxDuration();
			_vehicleShots[i].targetX = x;
			_vehicleShots[i].targetY = y;
			break;
		}
	}
}

// Gun position from GRD002 offset tables:
void InsaneRebel2::spawnHandler25Shot(int x, int y) {
	// Handler 25 can only shoot when uncovered (damage == 0)
	if (_rebelDamageLevel != 0) {
		return;  // Can't shoot while taking cover
	}

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter == 0) {
			playSfx(6, 127, 0);

			_turretShots[i].counter = getShotMaxDuration();
			_turretShots[i].seqNum = _turretShotSeqCounter;
			_turretShotSeqCounter++;

			// Target position is where player clicked, in buffer coords
			_turretShots[i].targetX = x + _viewX;
			_turretShots[i].targetY = y + _viewY;

			// where gunXTable/gunYTable are loaded by opcode 8, par4=8.
			if (_grdShotOriginTableLoaded) {
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
				if (spriteIdx < 0)
					spriteIdx = 0;
				if (spriteIdx >= ARRAYSIZE(_grdShotOriginX))
					spriteIdx = ARRAYSIZE(_grdShotOriginX) - 1;

				int16 gunXTable = _grdShotOriginX[spriteIdx];
				int16 gunYTable = _grdShotOriginY[spriteIdx];

				if (_rebelFlightDir != 0) {
					gunXTable = 320 - gunXTable;
				}

				_turretShots[i].gunX = gunXTable + _rebelViewOffset2X - _rebelViewOffsetX + _viewX;
				_turretShots[i].gunY = gunYTable + _rebelViewOffset2Y - _rebelViewOffsetY + _viewY;
			} else {
				// Fallback when table payload (opcode 8/par4=8) was not loaded.
				_turretShots[i].gunX = _rebelViewOffset2X + 160 + _viewX;
				_turretShots[i].gunY = _rebelViewOffset2Y + 140 + _viewY;
			}

			debugC(DEBUG_INSANE, "Handler25: Spawned shot %d target (%d,%d) gun (%d,%d)",
				i, _turretShots[i].targetX, _turretShots[i].targetY,
				_turretShots[i].gunX, _turretShots[i].gunY);
			break;
		}
	}
}

Common::Point InsaneRebel2::getHandler7ShipDrawPoint() {
	Common::Point projected = getHandler7ProjectedPoint();

	if (!_flyShipSprite || _flyShipSprite->getNumChars() <= 0)
		return Common::Point(projected.x - 0xd4, projected.y - 0x82);

	int spriteIndex = CLIP<int>(_shipDirectionIndex, 0, _flyShipSprite->getNumChars() - 1);
	return Common::Point(projected.x - 0xd4 + _flyShipSprite->getCharXOffset(spriteIndex),
	                     projected.y - 0x82 + _flyShipSprite->getCharYOffset(spriteIndex));
}

Common::Point InsaneRebel2::getHandler7ProjectedPointFor(int16 x, int16 y) {
	int viewTilt = (_viewShift * 5) / 128;
	int xSkew = (viewTilt * 9) / 4;
	int ySkew = viewTilt * 5;
	int relX = x - (_perspectiveX + 0xd4);
	int relY = y - (_perspectiveY + 0x82);
	int projectedX = (xSkew * relY) / 0x55 + relX + 0xa0 + _viewX;
	int projectedY = relY + 0x55 - (ySkew * relX) / 0xa0 + _viewY;

	return Common::Point(projectedX, projectedY);
}

Common::Point InsaneRebel2::getHandler7ProjectedPoint() {
	return getHandler7ProjectedPointFor(_flyShipScreenX, _flyShipScreenY);
}

Common::Point InsaneRebel2::getHandler7ShotTargetPoint() {
	// Handler 7 targets the projected ship/crosshair point computed in
	Common::Point projected = getHandler7ProjectedPoint();

	return Common::Point(projected.x + _smoothedVelocity / 2,
	                     projected.y + ABS(_smoothedVelocity) / 4 - _verticalInput / 2 - 0x28);
}

Common::Point InsaneRebel2::getHandler8ShotTargetPoint() {
	// Handler 8 stores and draws the shot target from the damped ship
	return Common::Point(((_shipPosX - 0xa0) >> 3) + 0xa0,
	                     ((_shipPosY - 0x28) >> 2) + 0x69);
}

void InsaneRebel2::spawnSpaceShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_spaceShots[i].counter == 0) {
			playSfx(6, 127, 0);

			_spaceShots[i].counter = getShotMaxDuration();
			Common::Point projected = getHandler7ProjectedPoint();
			Common::Point target = getHandler7ShotTargetPoint();
			int tableIndex = CLIP<int>(_shipDirectionIndex, 0, 34);

			_spaceShots[i].targetX = target.x;
			_spaceShots[i].targetY = target.y;

			// 8 par4=12/13. Values are centered FLY coordinates, adjusted by
			// the projected ship point before drawing the line.
			if (_flyLeftGunTableLoaded) {
				_spaceShots[i].leftGunX = projected.x + _flyLeftGunX[tableIndex] - 0xd4;
				_spaceShots[i].leftGunY = projected.y + _flyLeftGunY[tableIndex] - 0x82;
			} else {
				_spaceShots[i].leftGunX = projected.x - 28;
				_spaceShots[i].leftGunY = projected.y + 10;
			}
			if (_flyRightGunTableLoaded) {
				_spaceShots[i].rightGunX = projected.x + _flyRightGunX[tableIndex] - 0xd4;
				_spaceShots[i].rightGunY = projected.y + _flyRightGunY[tableIndex] - 0x82;
			} else {
				_spaceShots[i].rightGunX = projected.x + 28;
				_spaceShots[i].rightGunY = projected.y + 10;
			}
			_spaceShots[i].variant = _spaceShotDirection;
			break;
		}
	}
}

// drawTexturedLine -- Draw a line segment textured from a NUT sprite row.
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

void drawTexturedSegment(byte *dst, int pitch, int width, int height,
                         int param_3, int param_4, int param_5, int param_6, int param_7, const byte *param_8,
                         int clipLeft, int clipTop, int clipRight, int clipBottom) {
	// Only color 0 is transparent.
	int sVar4 = clipLeft;
	int sVar1 = clipTop;
	int sVar7 = clipRight;
	int sVar10 = clipBottom;
	int p3 = param_3;
	int p4 = param_4;
	int p5 = param_5;
	int p6 = param_6;

	if (p5 == p3) {
		if (p5 < sVar4 || sVar7 < p5)
			return;
	} else {
		if (p3 < sVar4) {
			if (p5 < sVar4)
				return;
			p4 = p6 + (((p4 - p6) * (sVar4 - p5)) / (p3 - p5));
			p3 = sVar4;
		} else if (sVar7 < p3) {
			if (sVar7 < p5)
				return;
			p4 = p6 + (((p4 - p6) * (sVar7 - p5)) / (p3 - p5));
			p3 = sVar7;
		}
		if (p5 < sVar4) {
			p6 = p4 + (((p6 - p4) * (sVar4 - p3)) / (p5 - p3));
			p5 = sVar4;
		} else if (sVar7 < p5) {
			p6 = p4 + (((p6 - p4) * (sVar7 - p3)) / (p5 - p3));
			p5 = sVar7;
		}
	}

	if (p6 == p4) {
		if (p6 < sVar1 || sVar10 < p6)
			return;
	} else {
		if (p4 < sVar1) {
			if (p6 < sVar1)
				return;
			p3 = p5 + (((p3 - p5) * (sVar1 - p6)) / (p4 - p6));
			p4 = sVar1;
		} else if (sVar10 < p4) {
			if (sVar10 < p6)
				return;
			p3 = p5 + (((p3 - p5) * (sVar10 - p6)) / (p4 - p6));
			p4 = sVar10;
		}
		if (p6 < sVar1) {
			p5 = (((p5 - p3) * (sVar1 - p4)) / (p6 - p4)) + p3;
			p6 = sVar1;
		} else if (sVar10 < p6) {
			p5 = (((p5 - p3) * (sVar10 - p4)) / (p6 - p4)) + p3;
			p6 = sVar10;
		}
	}

	int iVar5 = pitch;
	int sD = p5 - p3;
	int sE = p6 - p4;
	byte *pcVar6 = dst + p4 * iVar5 + p3;
	const byte *texPtr = param_8;

	if (sD == 0) {
		if (sE == 0) {
			if (*texPtr != 0)
				*pcVar6 = *texPtr;
			return;
		} else if (sE < 1) {
			int iVar9 = -(sE - 1);
			if (iVar9 <= 0)
				return;
			int iVar11 = iVar9;
			do {
				iVar11--;
				if (*texPtr != 0)
					*pcVar6 = *texPtr;
				pcVar6 -= iVar5;
				for (iVar9 = iVar9 - param_7; iVar9 < 0; iVar9 = iVar9 - (sE - 1))
					texPtr++;
			} while (0 < iVar11);
			return;
		} else {
			int iVar9 = sE + 1;
			if (iVar9 <= 0)
				return;
			int iVar8 = iVar9;
			int iVar11 = iVar9;
			do {
				iVar11--;
				if (*texPtr != 0)
					*pcVar6 = *texPtr;
				pcVar6 += iVar5;
				for (iVar8 = iVar8 - param_7; iVar8 < 0; iVar8 += iVar9)
					texPtr++;
			} while (0 < iVar11);
			return;
		}
	} else if (sE == 0) {
		if (sD < 1) {
			int iVar5h = -(sD - 1);
			if (iVar5h <= 0)
				return;
			int iVar9 = iVar5h;
			do {
				iVar9--;
				if (*texPtr != 0)
					*pcVar6 = *texPtr;
				pcVar6 -= 1;
				for (iVar5h = iVar5h - param_7; iVar5h < 0; iVar5h = iVar5h - (sD - 1))
					texPtr++;
			} while (0 < iVar9);
			return;
		} else {
			int iVar5h = sD + 1;
			if (iVar5h <= 0)
				return;
			int iVar11 = iVar5h;
			int iVar9 = iVar5h;
			do {
				iVar9--;
				if (*texPtr != 0)
					*pcVar6 = *texPtr;
				pcVar6 += 1;
				for (iVar11 = iVar11 - param_7; iVar11 < 0; iVar11 += iVar5h)
					texPtr++;
			} while (0 < iVar9);
			return;
		}
	}

	if (sD < 0) {
		if (-1 < sE) {
			int iVar11 = sD;
			int iVar8 = sE;
			int iVar9 = -iVar11;
			if (-iVar8 == iVar11 || iVar9 < iVar8) {
				iVar9 = sE >> 1;
				if (iVar8 + 1 < 1)
					return;
				int iVar3 = iVar8;
				int iVar12 = iVar8;
				do {
					if (*texPtr != 0)
						*pcVar6 = *texPtr;
					pcVar6 += iVar5;
					iVar9 += iVar11;
					if (iVar9 < 0) {
						iVar9 += iVar8;
						pcVar6 -= 1;
					}
					for (iVar12 = iVar12 - param_7; iVar12 < 0; iVar12 = iVar8 + 1 + iVar12)
						texPtr++;
					bool bVar2 = 0 < iVar3;
					iVar3--;
					if (!bVar2)
						break;
				} while (true);
				return;
			}
			int iVar12 = iVar9 >> 1;
			if (-iVar11 + 1 < 1)
				return;
			int iVar3 = -iVar11;
			do {
				if (*texPtr != 0)
					*pcVar6 = *texPtr;
				pcVar6 -= 1;
				iVar12 -= iVar8;
				if (iVar12 < 0) {
					iVar12 -= iVar11;
					pcVar6 += iVar5;
				}
				for (iVar9 = iVar9 - param_7; iVar9 < 0; iVar9 = (iVar9 - iVar11) + 1)
					texPtr++;
				bool bVar2 = 0 < iVar3;
				iVar3--;
				if (!bVar2)
					break;
			} while (true);
			return;
		}
	} else if (-1 < sE) {
		if (sD <= sE) {
			int iVar11 = sE;
			int iVar9 = sE >> 1;
			if (iVar11 + 1 < 1)
				return;
			int iVar8 = iVar11;
			int iVar12 = iVar11;
			do {
				if (*texPtr != 0)
					*pcVar6 = *texPtr;
				pcVar6 += iVar5;
				iVar9 -= sD;
				if (iVar9 < 0) {
					iVar9 += iVar11;
					pcVar6 += 1;
				}
				for (iVar8 = iVar8 - param_7; iVar8 < 0; iVar8 = iVar11 + 1 + iVar8)
					texPtr++;
				bool bVar2 = 0 < iVar12;
				iVar12--;
				if (!bVar2)
					break;
			} while (true);
			return;
		}
		int iVar11 = sD;
		int iVar9 = sD >> 1;
		if (iVar11 + 1 < 1)
			return;
		int iVar8 = iVar11;
		int iVar12 = iVar11;
		do {
			if (*texPtr != 0)
				*pcVar6 = *texPtr;
			pcVar6 += 1;
			iVar9 -= sE;
			if (iVar9 < 0) {
				iVar9 += iVar11;
				pcVar6 += iVar5;
			}
			for (iVar8 = iVar8 - param_7; iVar8 < 0; iVar8 = iVar11 + 1 + iVar8)
				texPtr++;
			bool bVar2 = 0 < iVar12;
			iVar12--;
			if (!bVar2)
				break;
		} while (true);
		return;
	}

	if (sD < 0) {
		if (sD < sE) {
			int iVar11 = sD;
			int iVar9 = -iVar11;
			int iVar8 = iVar9 >> 1;
			if (0 < -iVar11 + 1) {
				int iVar12 = -iVar11;
				do {
					if (*texPtr != 0)
						*pcVar6 = *texPtr;
					pcVar6 -= 1;
					iVar8 += sE;
					if (iVar8 < 0) {
						iVar8 -= iVar11;
						pcVar6 -= iVar5;
					}
					for (iVar9 = iVar9 - param_7; iVar9 < 0; iVar9 = (iVar9 - iVar11) + 1)
						texPtr++;
					bool bVar2 = 0 < iVar12;
					iVar12--;
					if (!bVar2)
						break;
				} while (true);
				return;
			}
		} else {
			int iVar11 = sE;
			int iVar9 = -iVar11;
			int iVar8 = iVar9 >> 1;
			if (0 < -iVar11 + 1) {
				int iVar12 = -iVar11;
				do {
					if (*texPtr != 0)
						*pcVar6 = *texPtr;
					pcVar6 -= iVar5;
					iVar8 += sD;
					if (iVar8 < 0) {
						iVar8 -= iVar11;
						pcVar6 -= 1;
					}
					for (iVar9 = iVar9 - param_7; iVar9 < 0; iVar9 = (iVar9 - iVar11) + 1)
						texPtr++;
					bool bVar2 = 0 < iVar12;
					iVar12--;
					if (!bVar2)
						break;
				} while (true);
				return;
			}
		}
	} else {
		int iVar11 = sE;
		int iVar8 = sD;
		int iVar9 = -iVar11;
		if (iVar9 < iVar8) {
			iVar9 = sD >> 1;
			if (0 < iVar8 + 1) {
				int iVar3 = iVar8;
				int iVar12 = iVar8;
				do {
					if (*texPtr != 0)
						*pcVar6 = *texPtr;
					pcVar6 += 1;
					iVar9 += iVar11;
					if (iVar9 < 0) {
						iVar9 += iVar8;
						pcVar6 -= iVar5;
					}
					for (iVar12 = iVar12 - param_7; iVar12 < 0; iVar12 = iVar8 + 1 + iVar12)
						texPtr++;
					bool bVar2 = 0 < iVar3;
					iVar3--;
					if (!bVar2)
						break;
				} while (true);
				return;
			}
		} else {
			int iVar12 = iVar9 >> 1;
			if (0 < -iVar11 + 1) {
				int iVar3 = -iVar11;
				do {
					if (*texPtr != 0)
						*pcVar6 = *texPtr;
					pcVar6 -= iVar5;
					iVar12 -= iVar8;
					if (iVar12 < 0) {
						iVar12 -= iVar11;
						pcVar6 += 1;
					}
					for (iVar9 = iVar9 - param_7; iVar9 < 0; iVar9 = (iVar9 - iVar11) + 1)
						texPtr++;
					bool bVar2 = 0 < iVar3;
					iVar3--;
					if (!bVar2)
						break;
				} while (true);
				return;
			}
		}
	}
}


void InsaneRebel2::initLaserTexture(NutRenderer *nut, int spriteIdx) {
	if (!nut || spriteIdx >= nut->getNumChars())
		return;

	int16 texWidth = nut->getCharWidth(spriteIdx);
	int16 texHeight = nut->getCharHeight(spriteIdx);

	if (texHeight > 15) {
		texHeight = 15;
	}

	freeLaserTexture();

	_laserTexture.width = texWidth;
	_laserTexture.height = texHeight;
	_laserTexture.pixels = (byte *)calloc(texWidth * texHeight, 1);

	if (!_laserTexture.pixels)
		return;

	// so we must honor x/y offsets and transparency, not just memcpy glyph rows.
	const byte *srcData = nut->getCharData(spriteIdx);
	const int srcWidth = nut->getCharWidth(spriteIdx);
	const int srcHeight = nut->getCharHeight(spriteIdx);
	const int srcXOff = nut->getCharXOffset(spriteIdx);
	const int srcYOff = nut->getCharYOffset(spriteIdx);
	if (srcData && srcWidth > 0 && srcHeight > 0) {
		for (int sy = 0; sy < srcHeight; sy++) {
			int dy = srcYOff + sy;
			if (dy < 0 || dy >= texHeight)
				continue;

			const byte *srcRow = srcData + sy * srcWidth;
			byte *dstRow = _laserTexture.pixels + dy * texWidth;
			for (int sx = 0; sx < srcWidth; sx++) {
				int dx = srcXOff + sx;
				if (dx < 0 || dx >= texWidth)
					continue;

				byte px = srcRow[sx];
				// Keep 231 pixels from the source texture to avoid dropping beam sub-segments.
				if (px != 0) {
					dstRow[dx] = px;
				}
			}
		}
	}

	debugC(DEBUG_INSANE, "Initialized laser texture %dx%d from sprite %d (xoff=%d yoff=%d src=%dx%d)",
	      texWidth, texHeight, spriteIdx, srcXOff, srcYOff, srcWidth, srcHeight);
}

void InsaneRebel2::freeLaserTexture() {
	free(_laserTexture.pixels);
	_laserTexture.pixels = nullptr;
	_laserTexture.width = 0;
	_laserTexture.height = 0;
}

// When data is nullptr, fills with the default table:
//   _edgeTable[a*256+b] = min(a,b) (symmetric identity blend)
// When data is non-null, loads the primary table from data+8 (upper triangle, symmetric).
void InsaneRebel2::initEdgeTable(const byte *data) {
	if (data == nullptr) {
		for (int a = 0; a < 256; a++) {
			for (int b = a; b < 256; b++) {
				// Primary table: table[a][b] = a (i.e. min(a,b) since b >= a)
				_edgeTable[a + b * 256] = (byte)a;
				_edgeTable[b + a * 256] = (byte)a;
			}
		}
		_edgeTable[0x42 * 256 + 0xf1] = 0x42;
		_edgeTable[0x42 + 0xf0 * 256] = 0x42;
		_edgeTable[0x41 * 256 + 0xb0] = 0x41;
	} else {
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

// For each pixel along the line, reads the two adjacent pixels perpendicular to
// the line direction and uses _edgeTable[above*256+below] as the output color.
// This creates a glow/blend effect at beam edges.
// For horizontal-dominant lines (dx > dy), reads pixels above and below.
// For vertical-dominant lines (dy > dx), reads pixels left and right.
void InsaneRebel2::drawEdgeHighlightLine(byte *dst, int pitch, int width, int height,
                                          int16 x0, int16 y0, int16 x1, int16 y1,
                                          int16 clipLeftIn, int16 clipTopIn, int16 clipRightIn, int16 clipBottomIn) {
	int16 clipLeft = CLIP<int16>(clipLeftIn, 1, width - 2);
	int16 clipTop = CLIP<int16>(clipTopIn, 1, height - 2);
	int16 clipRight = CLIP<int16>(clipRightIn, 1, width - 2);
	int16 clipBottom = CLIP<int16>(clipBottomIn, 1, height - 2);
	if (clipLeft > clipRight || clipTop > clipBottom)
		return;

	if (x0 == x1) {
		if (x0 < clipLeft || x0 > clipRight)
			return;
	} else {
		if (x0 < clipLeft) {
			if (x1 < clipLeft)
				return;
			y0 = y1 + (int16)(((int)(y0 - y1) * (int)(clipLeft - x1)) / (int)(x0 - x1));
			x0 = clipLeft;
		} else if (x0 > clipRight) {
			if (x1 > clipRight)
				return;
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

	if (y0 == y1) {
		if (y0 < clipTop || y0 > clipBottom)
			return;
	} else {
		if (y0 < clipTop) {
			if (y1 < clipTop)
				return;
			x0 = x1 + (int16)(((int)(x0 - x1) * (int)(clipTop - y1)) / (int)(y0 - y1));
			y0 = clipTop;
		} else if (y0 > clipBottom) {
			if (y1 > clipBottom)
				return;
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

	byte *pixel = dst + y0 * pitch + x0;
	int16 dx = x1 - x0;
	int16 dy = y1 - y0;

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

// Two-layer rendering:
//   Layer 1: Textured scanlines (beam body) via drawTexturedSegment()
//   Layer 2: Edge highlights (glow) via drawEdgeHighlightLine(), gated by _rebelDetailMode >= 0
//   dst, pitch, width, height: destination buffer info
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

	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	if (renderHiRes) {
		gunX = (int16)((gunX - _hiResPresentationViewX) * 2);
		gunY = (int16)((gunY - _hiResPresentationViewY) * 2);
		targetX = (int16)((targetX - _hiResPresentationViewX) * 2);
		targetY = (int16)((targetY - _hiResPresentationViewY) * 2);
	}

	if (maxFrames == 0)
		maxFrames = 1;
	int16 sVar7 = (int16)(((int)thickness * (int)animFrame * 16) / (int)maxFrames);

	int16 dx = targetX - gunX;
	int16 dy = targetY - gunY;
	int16 sVar6 = (int16)(((int)dx * (thickness + 1)) / (int)thickness);
	int16 sVar1 = (int16)(((int)dy * (thickness + 1)) / (int)thickness);

	// Start point (closer to gun, adjusted by animation progress)
	int16 startX = (sVar6 + gunX) - (int16)(((int)sVar6 * 16) / (sVar7 + 16));
	int16 startY = (sVar1 + gunY) - (int16)(((int)sVar1 * 16) / (sVar7 + 16));
	// End point (closer to target)
	int16 endX = (sVar6 + gunX) - (int16)(((int)sVar6 * 16) / (widthScale + sVar7 + 16));
	int16 endY = (sVar1 + gunY) - (int16)(((int)sVar1 * 16) / (widthScale + sVar7 + 16));

	byte *local_28 = texPixels;

	// This preserves texture phase at the viewport edge and avoids visibly "chopped" beams.
	int clipLeft = renderHiRes ? 0 : CLIP<int>(_viewX, 0, width - 1);
	int clipTop = renderHiRes ? 0 : CLIP<int>(_viewY, 0, height - 1);
	int clipRight = renderHiRes ? MIN(width - 1, 639) : CLIP<int>(_viewX + 319, 0, width - 1);
	int clipBottom = renderHiRes ? MIN(height - 1, _gameplayPresentationClipBottom) : CLIP<int>(_viewY + 179, 0, height - 1);
	if (clipLeft > clipRight || clipTop > clipBottom)
		return;
	int edgeClipLeft = CLIP<int>(clipLeft + 1, 1, width - 2);
	int edgeClipTop = CLIP<int>(clipTop + 1, 1, height - 2);
	int edgeClipRight = CLIP<int>(clipRight - 1, 1, width - 2);
	int edgeClipBottom = CLIP<int>(clipBottom - 1, 1, height - 2);

	debugC(DEBUG_INSANE, "drawLaserBeam gun(%d,%d) tgt(%d,%d) start(%d,%d) end(%d,%d) anim=%d/%d ws=%d hs=%d th=%d",
		gunX, gunY, targetX, targetY, startX, startY, endX, endY,
		animFrame, maxFrames, widthScale, heightScale, thickness);

	int iVar2 = abs(startY - endY);  // |dy| of beam
	int iVar3 = abs(startX - endX);  // |dx| of beam

	if (iVar2 < iVar3) {
		// Mostly horizontal beam - draw vertical scanlines
		iVar2 = abs(startX - endX);
		int temp = iVar2 * texH * heightScale;
		int16 numLines = (int16)((temp >> 3) / texW) + 2;
		int16 local_24 = -numLines;
		int16 halfLines = numLines >> 1;

		for (int16 lineIdx = 0; lineIdx < numLines; lineIdx++) {
			drawTexturedSegment(dst, pitch, width, height,
			                    startX, (startY - halfLines) + lineIdx,
			                    endX, (endY - halfLines) + lineIdx,
			                    texW, local_28,
			                    clipLeft, clipTop, clipRight, clipBottom);

			// Advance texture pointer (step through texture rows)
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= numLines) {
				local_28 += texW;
			}
		}

		if (_rebelDetailMode >= 0 && edgeClipLeft <= edgeClipRight && edgeClipTop <= edgeClipBottom) {
			drawEdgeHighlightLine(dst, pitch, width, height,
			                      startX, startY - halfLines,
			                      endX, endY - halfLines,
			                      edgeClipLeft, edgeClipTop, edgeClipRight, edgeClipBottom);
			drawEdgeHighlightLine(dst, pitch, width, height,
			                      startX, (startY - halfLines) + numLines - 1,
			                      endX, (endY - halfLines) + numLines - 1,
			                      edgeClipLeft, edgeClipTop, edgeClipRight, edgeClipBottom);
		}
	} else {
		// Mostly vertical beam - draw horizontal scanlines
		iVar2 = abs(startY - endY);
		int16 numLines = (int16)((iVar2 * texH) / texW) + 2;
		int16 local_24 = -numLines;

		if (texH < numLines) {
			numLines = texH;
		}

		int16 halfLines = numLines >> 1;

		for (int16 lineIdx = 0; lineIdx < numLines; lineIdx++) {
			drawTexturedSegment(dst, pitch, width, height,
			                    (startX - halfLines) + lineIdx, startY,
			                    (endX - halfLines) + lineIdx, endY,
			                    texW, local_28,
			                    clipLeft, clipTop, clipRight, clipBottom);

			// Advance texture pointer
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= numLines) {
				local_28 += texW;
			}
		}

		if (_rebelDetailMode >= 0 && edgeClipLeft <= edgeClipRight && edgeClipTop <= edgeClipBottom) {
			drawEdgeHighlightLine(dst, pitch, width, height,
			                      startX - halfLines, startY,
			                      endX - halfLines, endY,
			                      edgeClipLeft, edgeClipTop, edgeClipRight, edgeClipBottom);
			drawEdgeHighlightLine(dst, pitch, width, height,
			                      (startX - halfLines) + numLines - 1, startY,
			                      (endX - halfLines) + numLines - 1, endY,
			                      edgeClipLeft, edgeClipTop, edgeClipRight, edgeClipBottom);
		}
	}

}
// Collision Zone System
// Level 3 pilot ship obstacle avoidance. Zones are quadrilaterals

void InsaneRebel2::registerCollisionZone(Common::SeekableReadStream &b, int16 subOpcode, int16 par4) {
	// SmushPlayer reads the first 8 bytes as header (code/flags/unknown/userId),
	//   +0x00: opcode (5) — already consumed by SmushPlayer
	//   +0x02: par2 (sub-opcode) — already consumed, passed as parameter
	//   +0x04: par3 — already consumed by SmushPlayer
	//   +0x06: par4 (userId) — filter value for < 1000 test, passed as parameter
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

	if (subOpcode == 0x0D && _primaryZoneCount < kMaxCollisionZones) {
		_primaryZones[_primaryZoneCount++] = zone;
		debugC(DEBUG_INSANE, "Registered PRIMARY zone %d: filter=%d fields=[%d,%d] quad=(%d,%d)-(%d,%d)-(%d,%d)-(%d,%d)",
			_primaryZoneCount - 1, par4, field1, field2,
			x1, y1, x2, y2, x3, y3, x4, y4);
	} else if (subOpcode == 0x0E && _secondaryZoneCount < kMaxCollisionZones) {
		_secondaryZones[_secondaryZoneCount++] = zone;
		debugC(DEBUG_INSANE, "Registered SECONDARY zone %d: filter=%d fields=[%d,%d] quad=(%d,%d)-(%d,%d)-(%d,%d)-(%d,%d)",
			_secondaryZoneCount - 1, par4, field1, field2,
			x1, y1, x2, y2, x3, y3, x4, y4);
	} else {
		debugC(DEBUG_INSANE, "WARNING - Could not register zone (subOpcode=%d, primary=%d, secondary=%d)",
			subOpcode, _primaryZoneCount, _secondaryZoneCount);
	}
}

void InsaneRebel2::resetCollisionZones() {
	// This clears the zone tables so they can be rebuilt from the next frame's IACT chunks
	_primaryZoneCount = 0;
	_secondaryZoneCount = 0;
}

void InsaneRebel2::checkCollisionZones(byte *renderBitmap, int pitch, int width, int height, int32 curFrame) {
	// Tests aim/ship position against primary collision zone quadrilaterals.
	//   Zone vertices are in 424x260 buffer space, centered by subtracting (0xD4=212, 0x82=130).
	// For our implementation:
	//   Mouse Y 0..200 → centered Y ≈ [-45..45]

	if (_primaryZoneCount == 0)
		return;

	// Calculate aim position in centered coordinates.
	const Common::Point aimPos = getGameplayAimPoint();
	const int rawX = aimPos.x - 160;
	const int rawY = aimPos.y - 100;
	int16 aimX = (int16)(rawX * 52 / 160);
	int16 aimY = (int16)((_optControlsFlipped ? -rawY : rawY) * 45 / 100);

	if (aimX > 0x34)
		aimX = 0x34;
	if (aimX < -0x34)
		aimX = -0x34;
	if (aimY > 0x2d)
		aimY = 0x2d;
	if (aimY < -0x2d)
		aimY = -0x2d;

	LevelDifficultyParams dparams = getDifficultyParams();
	const bool showDirectionalWarnings = (dparams.flags & 8) != 0;
	const bool showGenericWarnings = !showDirectionalWarnings && ((dparams.flags & 0x10) != 0);
	const bool warningFrame = ((curFrame & 2) != 0) && _smush_iconsNut && (showDirectionalWarnings || showGenericWarnings);
	const int indicatorScale = getRebel2IndicatorScale(width, height);

	for (int i = 0; i < _primaryZoneCount; i++) {
		CollisionZone &zone = _primaryZones[i];
		if (!zone.active)
			continue;

		// Filter: only process zones with filterValue < 1000 (par4 from IACT header)
		if (zone.filterValue >= 1000)
			continue;

		// Center zone vertices by subtracting buffer center (0xD4=212, 0x82=130)
		int cx1 = zone.x1 - 0xD4;
		int cy1 = zone.y1 - 0x82;
		int cx2 = zone.x2 - 0xD4;
		int cy2 = zone.y2 - 0x82;
		int cx3 = zone.x3 - 0xD4;
		int cy3 = zone.y3 - 0x82;
		int cx4 = zone.x4 - 0xD4;
		int cy4 = zone.y4 - 0x82;

		// Frame check: field2 - 1 == field1
		if (zone.field2 - 1 == zone.field1) {
			// Tests if aim position is OUTSIDE the safe corridor (= collision with obstacle).
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
				int collisionDamage = (dparams.dodgeDamage >= 0) ? dparams.dodgeDamage : 0;

				if (applyPlayerDamage(collisionDamage)) {
					debugC(DEBUG_INSANE, "COLLISION damage! zone=%d aim=(%d,%d) damage=%d total=%d",
						i, aimX, aimY, collisionDamage, _playerDamage);
				}
				if (!_noDamage)
					initDamageFlash();
			} else {
				// Safely passed — award score bonus
				if (dparams.dodgePoints > 0) {
					addScore(dparams.dodgePoints);
				}
			}
		} else if (warningFrame && zone.field2 - 0x0c < zone.field1) {
			// draws directional arrows from cockpit icon slots 0x2a..0x2d.
			// Novice mode (flags bit 0x10) draws a generic indicator from slot 0x36.
			const int iconChars = _smush_iconsNut->getNumChars();
			if (showDirectionalWarnings) {
				int avgX = (cx1 + cx2 + cx3 + cx4) / 4;
				int avgY = (cy1 + cy2 + cy3 + cy4) / 4;

				if (avgX >= 0x15 && iconChars > 0x2d) {
					renderNutSprite(renderBitmap, pitch, width, height,
						0xd7 * indicatorScale + _viewX, 0x55 * indicatorScale + _viewY, _smush_iconsNut, 0x2d);
				} else if (avgX < -0x14 && iconChars > 0x2c) {
					renderNutSprite(renderBitmap, pitch, width, height,
						0x69 * indicatorScale + _viewX, 0x55 * indicatorScale + _viewY, _smush_iconsNut, 0x2c);
				}

				if (avgY >= 0x15 && iconChars > 0x2b) {
					renderNutSprite(renderBitmap, pitch, width, height,
						0xa0 * indicatorScale + _viewX, 0x82 * indicatorScale + _viewY, _smush_iconsNut, 0x2b);
				} else if (avgY < -0x14 && iconChars > 0x2a) {
					renderNutSprite(renderBitmap, pitch, width, height,
						0xa0 * indicatorScale + _viewX, 0x28 * indicatorScale + _viewY, _smush_iconsNut, 0x2a);
				}
			} else if (showGenericWarnings && iconChars > 0x36) {
				renderNutSprite(renderBitmap, pitch, width, height,
					0xa0 * indicatorScale + _viewX, 0x1e * indicatorScale + _viewY, _smush_iconsNut, 0x36);
			}
		}
	}
}

// Two modes: obstacle collision (secondary zones) and wall/boundary
// collision (primary zones with per-edge push-back).
// The helpers in this block are split out of checkHandler7CollisionZones;
bool InsaneRebel2::isHandler7ShipInsideObstacleZone(const InsaneRebel2::CollisionZone &zone, int margin) {
	int x1 = zone.x1, y1 = zone.y1;
	int x2 = zone.x2, y2 = zone.y2;
	int x3 = zone.x3, y3 = zone.y3;
	int x4 = zone.x4, y4 = zone.y4;

	// Start assuming inside, clear if outside any edge (with margin).
	bool inside = true;

	// Top edge: interpolate Y along v1->v2 at shipX, +15 margin.
	if (x2 != x1) {
		int interpY = (_flyShipScreenX - x1) * (y2 - y1) / (x2 - x1) + margin + y1;
		if (_flyShipScreenY < interpY)
			inside = false;
	}
	// Bottom edge: interpolate Y along v4->v3 at shipX, -15 margin.
	if (inside && x3 != x4) {
		int interpY = (_flyShipScreenX - x4) * (y3 - y4) / (x3 - x4) + y4 - margin;
		if (interpY < _flyShipScreenY)
			inside = false;
	}
	// Left edge: interpolate X along v1->v4 at shipY, +15 margin.
	if (inside && y4 != y1) {
		int interpX = (_flyShipScreenY - y1) * (x4 - x1) / (y4 - y1) + margin + x1;
		if (_flyShipScreenX < interpX)
			inside = false;
	}
	// Right edge: interpolate X along v2->v3 at shipY, -15 margin.
	if (inside && y3 != y2) {
		int interpX = (_flyShipScreenY - y2) * (x3 - x2) / (y3 - y2) + x2 - margin;
		if (interpX < _flyShipScreenX)
			inside = false;
	}

	return inside;
}

void InsaneRebel2::applyHandler7ObstacleHit(const InsaneRebel2::CollisionZone &zone, int zoneIndex) {
	// Collision with obstacle - apply damage and break.
	_hitCooldown = 10;
	_spaceShotDirection = zone.filterValue + 2;

	LevelDifficultyParams params = getDifficultyParams();
	int collisionDamage = (params.dodgeDamage >= 0) ? params.dodgeDamage : 0;
	applyPlayerDamage(collisionDamage);
	_rebelHitCounter++;
	if (!_noDamage)
		initDamageFlash();
	playSfx(1, 127, CLIP((_flyShipScreenX - 212) * 127 / 160, -127, 127));
	debugC(DEBUG_INSANE, "Handler7 Mode0/2 OBSTACLE HIT zone=%d ship=(%d,%d) damage=%d",
		zoneIndex, _flyShipScreenX, _flyShipScreenY, collisionDamage);
}

void InsaneRebel2::awardHandler7DodgeScore() {
	// Safely avoided obstacle - award score.
	LevelDifficultyParams scoreParams = getDifficultyParams();
	if (scoreParams.dodgePoints > 0) {
		addScore(scoreParams.dodgePoints);
	}
}

void InsaneRebel2::checkHandler7ObstacleZones(uint16 &warningMask) {
	// Inside the quad = collision with obstacle.
	const int margin = 15;

	for (int i = 0; i < _secondaryZoneCount; i++) {
		CollisionZone &zone = _secondaryZones[i];
		if (!zone.active)
			continue;

		bool inside = isHandler7ShipInsideObstacleZone(zone, margin);

		if (zone.field2 - 1 == zone.field1) {
			if (inside) {
				applyHandler7ObstacleHit(zone, i);
				break;
			} else {
				awardHandler7DodgeScore();
			}
		}

		if (zone.field2 - 13 < zone.field1) {
			uint32 bit = 4u << ((byte)zone.filterValue & 0x1f);
			warningMask = (uint16)(warningMask | (uint16)bit);
		}
	}

	if (_flyShipScreenX < _corridorLeftX + 0x28)
		warningMask |= 1;
	if (_corridorRightX - 0x28 < _flyShipScreenX)
		warningMask |= 2;
}

bool InsaneRebel2::applyHandler7WallDamage(int wallDamage) {
	if (_hitCooldown < 5 && !_rebelAutoPlay) {
		const bool damageApplied = applyPlayerDamage(wallDamage);
		_rebelHitCounter++;
		_hitCooldown = 10;
		return damageApplied;
	}

	return false;
}

void InsaneRebel2::resetHandler7HorizontalVelocity(int16 velocity) {
	for (int j = 0; j < ARRAYSIZE(_velocityHistory); j++) {
		_velocityHistory[j] = velocity;
	}
}

void InsaneRebel2::checkHandler7TopBoundary(const InsaneRebel2::CollisionZone &zone, int16 vMargin, int wallDamage, uint16 &warningMask) {
	int x1 = zone.x1, y1 = zone.y1;
	int x2 = zone.x2, y2 = zone.y2;

	if (x2 != x1) {
		int16 edgeY = (int16)((_flyShipScreenX - x1) * (y2 - y1) / (x2 - x1) + y1 + vMargin);
		if (_flyShipScreenY < edgeY) {
			// Ship above top wall - push down.
			const bool damageApplied = applyHandler7WallDamage(wallDamage);
			if (damageApplied) {
				debugC(DEBUG_INSANE, "Handler7 Mode1/3 TOP WALL ship=(%d,%d) edgeY=%d damage=%d",
					_flyShipScreenX, _flyShipScreenY, edgeY, wallDamage);
			}
			_spaceShotDirection = 2;  // Direction: pushed down
			_flyShipScreenY = edgeY;  // Push-back
			playSfx(1, 127, 0);  // CRASH.SAD, top wall -> center pan (always)
			if (!_noDamage)
				initDamageFlash();
		} else if (_flyShipScreenY < edgeY + 0x28) {
			warningMask |= 4;
		}
	}
}

void InsaneRebel2::checkHandler7BottomBoundary(const InsaneRebel2::CollisionZone &zone, int16 vMargin, int wallDamage, uint16 &warningMask) {
	int x3 = zone.x3, y3 = zone.y3;
	int x4 = zone.x4, y4 = zone.y4;

	if (x3 != x4) {
		int16 edgeY = (int16)((_flyShipScreenX - x4) * (y3 - y4) / (x3 - x4) + y4 - vMargin);
		_corridorBottomY = vMargin + edgeY;
		if (edgeY < _flyShipScreenY) {
			// Ship below bottom wall - push up.
			const bool damageApplied = applyHandler7WallDamage(wallDamage);
			if (damageApplied) {
				debugC(DEBUG_INSANE, "Handler7 Mode1/3 BOTTOM WALL ship=(%d,%d) edgeY=%d damage=%d",
					_flyShipScreenX, _flyShipScreenY, edgeY, wallDamage);
			}
			_spaceShotDirection = 3;  // Direction: pushed up
			_flyShipScreenY = edgeY;  // Push-back
			playSfx(1, 127, 0);  // CRASH.SAD, bottom wall -> center pan (always)
			if (!_noDamage)
				initDamageFlash();
		} else if (edgeY - 0x28 < _flyShipScreenY) {
			warningMask |= 8;
		}
	}
}

void InsaneRebel2::checkHandler7LeftBoundary(const InsaneRebel2::CollisionZone &zone, int16 hMargin, int wallDamage) {
	int x1 = zone.x1, y1 = zone.y1;
	int x4 = zone.x4, y4 = zone.y4;

	if (y4 != y1) {
		int16 edgeX = (int16)((_flyShipScreenY - y1) * (x4 - x1) / (y4 - y1) + x1 + hMargin);
		if (_flyShipScreenX < edgeX) {
			// Ship left of left wall - push right.
			_flyShipScreenX = edgeX;  // Push-back

			resetHandler7HorizontalVelocity(127);

			const bool damageApplied = applyHandler7WallDamage(wallDamage);
			if (damageApplied) {
				debugC(DEBUG_INSANE, "Handler7 Mode1/3 LEFT WALL ship=(%d,%d) edgeX=%d damage=%d",
					_flyShipScreenX, _flyShipScreenY, edgeX, wallDamage);
			}
			_spaceShotDirection = 0;  // Direction: pushed right
			playSfx(1, 127, -100);  // CRASH.SAD, left wall -> pan left (always)
			if (!_noDamage)
				initDamageFlash();
		}
	}
}

void InsaneRebel2::checkHandler7RightBoundary(const InsaneRebel2::CollisionZone &zone, int16 hMargin, int wallDamage) {
	int x2 = zone.x2, y2 = zone.y2;
	int x3 = zone.x3, y3 = zone.y3;

	if (y3 != y2) {
		int16 edgeX = (int16)((_flyShipScreenY - y2) * (x3 - x2) / (y3 - y2) + x2 - hMargin);
		if (edgeX < _flyShipScreenX) {
			// Ship right of right wall - push left.
			_flyShipScreenX = edgeX;  // Push-back

			resetHandler7HorizontalVelocity(-127);

			const bool damageApplied = applyHandler7WallDamage(wallDamage);
			if (damageApplied) {
				debugC(DEBUG_INSANE, "Handler7 Mode1/3 RIGHT WALL ship=(%d,%d) edgeX=%d damage=%d",
					_flyShipScreenX, _flyShipScreenY, edgeX, wallDamage);
			}
			_spaceShotDirection = 1;  // Direction: pushed left
			playSfx(1, 127, 100);  // CRASH.SAD, right wall -> pan right (always)
			if (!_noDamage)
				initDamageFlash();
		}
	}
}

void InsaneRebel2::checkHandler7BoundaryZones(uint16 &warningMask) {
	// Ship position is clamped to wall boundaries when hitting.
	int16 hMargin = (_flyControlMode == 1) ? 0x28 : 0x0f;
	const int16 vMargin = 0x0f;
	LevelDifficultyParams wallParams = getDifficultyParams();
	int wallDamage = (wallParams.dodgeDamage >= 0) ? wallParams.dodgeDamage : 0;

	for (int i = 0; i < _primaryZoneCount; i++) {
		CollisionZone &zone = _primaryZones[i];
		if (!zone.active)
			continue;

		checkHandler7TopBoundary(zone, vMargin, wallDamage, warningMask);
		checkHandler7BottomBoundary(zone, vMargin, wallDamage, warningMask);
		checkHandler7LeftBoundary(zone, hMargin, wallDamage);
		checkHandler7RightBoundary(zone, hMargin, wallDamage);
	}
}

void InsaneRebel2::renderHandler7WarningCues(byte *renderBitmap, int pitch, int width, int height, int32 curFrame, uint16 warningMask) {
	// Note: These are cue sprites (often perceived as "shadows"), not the aiming reticle.
	LevelDifficultyParams dparams = getDifficultyParams();
	if ((curFrame & 2) != 0 && (dparams.flags & 8) != 0 && _smush_iconsNut) {
		int scale = getRebel2IndicatorScale(width, height);

		if ((warningMask & 1) != 0 && _smush_iconsNut->getNumChars() > 0x2d) {
			renderNutSprite(renderBitmap, pitch, width, height,
				0xd7 * scale + _viewX, 0x55 * scale + _viewY, _smush_iconsNut, 0x2d);
		}
		if ((warningMask & 2) != 0 && _smush_iconsNut->getNumChars() > 0x2c) {
			renderNutSprite(renderBitmap, pitch, width, height,
				0x69 * scale + _viewX, 0x55 * scale + _viewY, _smush_iconsNut, 0x2c);
		}
		if ((warningMask & 4) != 0 && _smush_iconsNut->getNumChars() > 0x2b) {
			renderNutSprite(renderBitmap, pitch, width, height,
				0xa0 * scale + _viewX, 0x82 * scale + _viewY, _smush_iconsNut, 0x2b);
		}
		if ((warningMask & 8) != 0 && _smush_iconsNut->getNumChars() > 0x2a) {
			renderNutSprite(renderBitmap, pitch, width, height,
				0xa0 * scale + _viewX, 0x28 * scale + _viewY, _smush_iconsNut, 0x2a);
		}
	}
}

void InsaneRebel2::checkHandler7CollisionZones(byte *renderBitmap, int pitch, int width, int height, int32 curFrame) {
	// Uses ship position (_flyShipScreenX/_flyShipScreenY) in raw buffer coords.
	// Two modes depending on _flyControlMode:
	//   Mode 0/2: Obstacle collision using SECONDARY zones (inside quad = hit)
	//   Mode 1/3: Wall/boundary collision using PRIMARY zones (per-edge push-back)

	// bit 0=left, bit 1=right, bit 2=top, bit 3=bottom
	uint16 warningMask = 0;

	if (_flyControlMode == 0 || _flyControlMode == 2) {
		checkHandler7ObstacleZones(warningMask);
	} else {
		checkHandler7BoundaryZones(warningMask);
	}

	renderHandler7WarningCues(renderBitmap, pitch, width, height, curFrame, warningMask);
}

// renderNutSprite -- Draw a NUT sprite with transparency.
void InsaneRebel2::renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx) {
	renderNutSpriteMirrored(dst, pitch, width, height, x, y, nut, spriteIdx, false);
}

// renderNutSpriteClipped -- Draw a NUT sprite with explicit clip rectangle.
void renderNutSpriteClipped(byte *dst, int pitch, int dstH,
		int clipLeft, int clipTop, int clipRight, int clipBottom,
		int x, int y, NutRenderer *nut, int spriteIdx) {
	if (!nut || spriteIdx < 0 || spriteIdx >= nut->getNumChars())
		return;

	if (clipLeft < 0)
		clipLeft = 0;
	if (clipTop < 0)
		clipTop = 0;
	if (clipRight > pitch)
		clipRight = pitch;
	if (clipBottom > dstH)
		clipBottom = dstH;
	if (clipLeft >= clipRight || clipTop >= clipBottom)
		return;

	int w = nut->getCharWidth(spriteIdx);
	int h = nut->getCharHeight(spriteIdx);
	const byte *src = nut->getCharData(spriteIdx);
	if (!src || w <= 0 || h <= 0)
		return;

	for (int iy = 0; iy < h; ++iy) {
		int dstY = y + iy;
		if (dstY < clipTop || dstY >= clipBottom)
			continue;

		const byte *s = src + iy * w;
		byte *d = dst + dstY * pitch;

		int dstStart = x;
		int dstEnd = x + w;
		int srcStart = 0;
		if (dstStart < clipLeft) {
			srcStart = clipLeft - dstStart;
			dstStart = clipLeft;
		}
		if (dstEnd > clipRight)
			dstEnd = clipRight;
		if (dstStart >= dstEnd)
			continue;

		int copyCount = dstEnd - dstStart;
		for (int ix = 0; ix < copyCount; ++ix) {
			byte px = s[srcStart + ix];
			if (px != 0)
				d[dstStart + ix] = px;
		}
	}
}

static void renderNutSpriteScaledClipped(byte *dst, int pitch, int width, int height,
		int clipLeft, int clipTop, int clipRight, int clipBottom,
		int x, int y, NutRenderer *nut, int spriteIdx, bool mirror, int scale, bool transparent231) {
	if (!nut || spriteIdx < 0 || spriteIdx >= nut->getNumChars() || !dst)
		return;

	if (scale < 1)
		scale = 1;

	const int dstW = MIN(width, pitch);
	if (dstW <= 0 || height <= 0)
		return;

	if (clipLeft < 0)
		clipLeft = 0;
	if (clipTop < 0)
		clipTop = 0;
	if (clipRight > dstW)
		clipRight = dstW;
	if (clipBottom > height)
		clipBottom = height;
	if (clipLeft >= clipRight || clipTop >= clipBottom)
		return;

	int srcW = nut->getCharWidth(spriteIdx);
	int srcH = nut->getCharHeight(spriteIdx);
	const byte *src = nut->getCharData(spriteIdx);
	if (!src || srcW <= 0 || srcH <= 0)
		return;

	const int scaledW = srcW * scale;
	const int scaledH = srcH * scale;
	int drawLeft = MAX(x, clipLeft);
	int drawTop = MAX(y, clipTop);
	int drawRight = MIN(x + scaledW, clipRight);
	int drawBottom = MIN(y + scaledH, clipBottom);
	if (drawLeft >= drawRight || drawTop >= drawBottom)
		return;

	if (!transparent231 && x >= clipLeft && y >= clipTop &&
			x + scaledW <= clipRight && y + scaledH <= clipBottom) {
		const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
		Graphics::Surface srcSurface;
		srcSurface.init(srcW, srcH, srcW, const_cast<byte *>(src), format);

		Graphics::ManagedSurface dstSurface;
		dstSurface.surfacePtr()->init(dstW, height, pitch, dst, format);
		dstSurface.transBlitFrom(srcSurface, Common::Rect(0, 0, srcW, srcH),
			Common::Rect(x, y, x + scaledW, y + scaledH), 0, mirror);
		return;
	}

	for (int dy = drawTop; dy < drawBottom; dy++) {
		int srcY = (dy - y) / scale;
		const byte *srcRow = src + srcY * srcW;
		byte *dstRow = dst + dy * pitch;

		for (int dx = drawLeft; dx < drawRight; dx++) {
			int srcX = (dx - x) / scale;
			if (mirror)
				srcX = srcW - 1 - srcX;

			byte px = srcRow[srcX];
			if (px != 0 && (!transparent231 || px != 231))
				dstRow[dx] = px;
		}
	}
}

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

// updatePostRenderScroll -- Set SmushPlayer scroll offsets for the current frame.
void InsaneRebel2::updatePostRenderScroll(int width, int height) {
	if (_rebelHandler == 0) {
		_viewX = 0;
		_viewY = 0;
		_player->setScrollOffset(0, 0);
		return;
	}

	if (_rebelHandler == 8) {
		_viewX = 0;
		_viewY = 0;
		_player->setScrollOffset(0, 0);
		return;
	}

	if (_rebelHandler == 25) {
		// Handler 25's L2 corridor layers are authored for the 320x200 viewport.
		// The backing buffer may be larger to decode unclipped FOBJ data, but
		// panning the final copy exposes unfilled columns/rows and breaks the
		// corridor perspective. High-res mode must scale that same locked native
		// viewport instead of selecting a different region of the 424x260 buffer.
		_viewX = 0;
		_viewY = 0;
		_player->setScrollOffset(0, 0);
		return;
	}

	// Rebel Assault 2 uses a native 320x200 viewport into buffers that may be
	// larger (424x260). High-res mode still scrolls in those logical units; the
	// selected viewport is promoted to 640x400 after native FOBJ decoding.
	const int viewportWidth = isHiRes() ? 320 : _vm->_screenWidth;
	const int viewportHeight = isHiRes() ? 200 : _vm->_screenHeight;
	int maxScrollX = width - viewportWidth;
	int maxScrollY = height - viewportHeight;

	if (maxScrollX < 0)
		maxScrollX = 0;
	if (maxScrollY < 0)
		maxScrollY = 0;

	if (_rebelHandler == 7) {
		// flight buffer is presented through a perspective-derived 320x170
		// Keep the final crop in that same space so rendered ship/cues and
		// raw collision zones describe the same tunnel position.
		const int handler7MaxScrollY = isHiRes() ? MAX<int>(0, height - 170) : maxScrollY;
		_viewX = CLIP<int>(0x34 + _perspectiveX, 0, maxScrollX);
		_viewY = CLIP<int>(0x2d + _perspectiveY, 0, handler7MaxScrollY);
		_player->setScrollOffset(_viewX, _viewY);
		return;
	}

	// Simple linear mapping: Center of screen corresponds to center of buffer.
	Common::Point aimPos = getGameplayAimPoint();
	_viewX = (aimPos.x * maxScrollX) / viewportWidth;
	_viewY = (aimPos.y * maxScrollY) / viewportHeight;

	_player->setScrollOffset(_viewX, _viewY);
}

// updatePostRenderDeath -- End gameplay playback when player damage reaches 255.
void InsaneRebel2::updatePostRenderDeath() {
	// Sync _playerShield from _playerDamage and break out of video on death.
	if (_rebelHandler != 0) {
		_playerShield = 255 - _playerDamage;
		if (_playerShield <= 0) {
			_playerShield = 0;
			_vm->_smushVideoShouldFinish = true;
		}
	}
}

// renderPostRenderMenuCursor -- Draw RA2's software cursor for menu videos.
void InsaneRebel2::renderPostRenderMenuCursor(byte *renderBitmap, int pitch, int width, int height) {
	static const byte kRa2MenuCursor[] = {
		 0,  0,  1,  1,  1,  1,  1,
		 0, 15,  0,  1,  1,  1,  1,
		 0, 15, 15,  0,  1,  1,  1,
		 0, 15, 15, 15,  0,  1,  1,
		 0, 15, 15, 15, 15,  0,  1,
		 0, 15, 15, 15, 15, 15,  0,
		 0, 15, 15, 15,  0,  0,  0,
		 0, 15,  0, 15, 15,  1,  1,
		 0,  0,  0,  0, 15,  0,  1,
		 1,  1,  1,  0,  0,  0,  1
	};
	const int cursorWidth = 7;
	const int cursorHeight = 10;

	CursorMan.showMouse(false);

	const int cursorX = _vm->_mouse.x;
	const int cursorY = _vm->_mouse.y;
	for (int y = 0; y < cursorHeight; y++) {
		int dstY = cursorY + y;
		if (dstY < 0 || dstY >= height)
			continue;

		for (int x = 0; x < cursorWidth; x++) {
			int dstX = cursorX + x;
			if (dstX < 0 || dstX >= width)
				continue;

			byte color = kRa2MenuCursor[y * cursorWidth + x];
			if (color != 1)
				renderBitmap[dstY * pitch + dstX] = color;
		}
	}
}

// handlePostRenderMenuModes -- Process menu-like videos drawn during post-rendering.
bool InsaneRebel2::handlePostRenderMenuModes(byte *renderBitmap, int pitch, int width, int height, bool introPlaying) {
	// Check if we're in menu mode (menu state + intro flag).
	bool menuMode = (introPlaying && _gameState == kStateMainMenu);
	bool pilotSelectMode = (introPlaying && (_gameState == kStatePilotSelect || _gameState == kStateDifficultySelect));
	bool chapterSelectMode = (introPlaying && _gameState == kStateChapterSelect);

	// This is the pilot/save slot selection screen with centered menu.
	if (pilotSelectMode) {
		int selection = processLevelSelectInput();

		// Draw pilot selection overlay - centered menu like main menu.
		drawLevelSelectOverlay(renderBitmap, pitch, width, height);
		renderPostRenderMenuCursor(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop.
		if (selection >= 0) {
			debugC(DEBUG_INSANE, "Pilot selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in pilot select mode.
		return true;
	}

	// This is the actual level/chapter selection screen with preview and password.
	if (chapterSelectMode) {
		// O_LEVEL.SAN provides the background with chapter preview thumbnails.
		// The FOBJ offset system (set in procPreRendering) scrolls the correct preview
		// into the preview box area. No black fill needed — video frame shows through.

		int selection = processChapterSelectInput();

		drawChapterSelectOverlay(renderBitmap, pitch, width, height);
		renderPostRenderMenuCursor(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop.
		if (selection >= 0) {
			debugC(DEBUG_INSANE, "Chapter selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in chapter select mode.
		return true;
	}

	if (introPlaying && _gameState == kStateTopPilots) {
		drawTopPilotsOverlay(renderBitmap, pitch, width, height);
		return true;
	}

	if (introPlaying && _gameState == kStateOptions) {
		processOptionsInput();
		drawOptionsOverlay(renderBitmap, pitch, width, height);
		renderPostRenderMenuCursor(renderBitmap, pitch, width, height);
		return true;
	}

	if (menuMode) {
		// Process menu input during each frame.
		int selection = processMenuInput();

		// Update inactivity timer (only increments when no input is received).
		// Input resets timer in processMenuInput().
		_menuInactivityTimer++;

		// Check for inactivity timeout.
		// At 12fps video rate, 300 frames = ~25 seconds of inactivity.
		if (_menuInactivityTimer > 300) {
			debugC(DEBUG_INSANE, "Menu inactivity timeout - resuming intro/demo loop");
			_menuInactivityTimer = 0;
			_menuInactivityTimedOut = true;
			_menuSelectionConfirmed = false;
			_vm->_smushVideoShouldFinish = true;
		}

		// Draw menu selection overlay.
		drawMenuOverlay(renderBitmap, pitch, width, height);
		renderPostRenderMenuCursor(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop.
		if (selection >= 0) {
			debugC(DEBUG_INSANE, "Menu selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in menu mode.
		return true;
	}

	return false;
}

// handlePostRenderIntro -- Hide gameplay HUD for intro/cinematic videos.
bool InsaneRebel2::handlePostRenderIntro(byte *renderBitmap, int pitch, int width, int height, int32 curFrame) {
	// During intro/cinematic sequences:
	// - Skip all HUD/status bar/crosshair rendering
	// - Skip mouse input processing (no shooting during intros)
	// - Cinematics/intros don't have opcode 6, so handler stays 0
	// - We use _rebelHandler == 0 as the primary indicator for intro/cinematic mode
	if (_rebelHandler == 0) {
		// Hide mouse cursor during intro - no crosshair, no clicking.
		CursorMan.showMouse(false);

		// Track state transition for debugging.
		if (!_introCursorPushed) {
			_introCursorPushed = true;
			debugC(DEBUG_INSANE, "Intro/cinematic mode (handler=0, flags=0x%x, state=%d) - HUD disabled, mouse hidden",
				  _player->_curVideoFlags, _gameState);
		}

		if (_textOverlayActive)
			renderTextOverlay(renderBitmap, pitch, width, height, curFrame);

		// Skip all HUD rendering during intro - subtitles are rendered via opcode 9.
		return true;
	}

	// Gameplay mode - handler was set by IACT opcode 6.
	if (_introCursorPushed) {
		_introCursorPushed = false;
		debugC(DEBUG_INSANE, "Gameplay mode (handler=%d, flags=0x%x, state=%d) - HUD enabled",
			  _rebelHandler, _player->_curVideoFlags, _gameState);
	}

	return false;
}

// updateGameplayDamageEffects -- Apply handler-specific damage visuals.
void InsaneRebel2::updateGameplayDamageEffects(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler == 8) {
		// Full damage effect: palette flash + screen shake.
		// Suppressed during autopilot (mode 4) and cutscene (mode 5).
		if (_shipLevelMode != 4 && _shipLevelMode != 5) {
			updateDamageEffect(renderBitmap, pitch, width, height);
		}
	} else if (_rebelHandler == 0x19 || _rebelHandler == 0x26 || _rebelHandler == 7) {
		// Palette flash only - no screen shake for turret/FPS/ship handlers.
		updateDamageFlashPalette();
	}
}

// updateGameplayDamageRecovery -- Apply RA2's damage auto-reduction.
void InsaneRebel2::updateGameplayDamageRecovery(int32 curFrame) {
	// timed score tick in the same slot and does not reduce damage.
	if ((_rebelHandler != 0x26 && _rebelHandler != 8 && _rebelHandler != 7) ||
			(curFrame & 0xf) != 0 || _playerDamage <= 0) {
		return;
	}

	_playerDamage--;
	_playerShield = 255 - _playerDamage;
}

// checkGameplayPostRenderCollisions -- Run handler-specific collision checks.
void InsaneRebel2::checkGameplayPostRenderCollisions(byte *renderBitmap, int pitch, int width, int height, int32 curFrame) {
	// Per-frame collision checking against registered zones.
	//   Zones with filterValue < 1000 tested via point-in-quad against mouse/aim position.
	//   Mode 0/2: SECONDARY zones (0x0E) - obstacle collision (inside quad = hit)
	//   Mode 1/3: PRIMARY zones (0x0D) - wall/boundary per-edge with push-back
	//   Uses ship position in raw buffer coords, hit cooldown, directional damage.
	if (_rebelHandler == 0x26) {
		checkCollisionZones(renderBitmap, pitch, width, height, curFrame);
	} else if (_rebelHandler == 7) {
		checkHandler7CollisionZones(renderBitmap, pitch, width, height, curFrame);
	}
}

void InsaneRebel2::renderGameplayPostFrame(byte *renderBitmap, int pitch, int width, int height,
										   int videoWidth, int videoHeight, int statusBarY, int32 curFrame) {
	// From here on, we're in gameplay mode (_rebelHandler != 0).
	// Process mouse input for shooting.
	processMouse();

	// Handler 7's high-detail flight view uses a 320x170 gameplay area after
	// the first frame; the remaining RA2 gameplay handlers keep the 180px split.
	const int statusScale = (statusBarY >= 360) ? 2 : 1;
	const int gameplayStatusBarY = (_rebelHandler == 7 && curFrame != 0) ? 170 * statusScale : statusBarY;
	_gameplayPresentationClipBottom = gameplayStatusBarY - 1;

	_hiResPresentationViewX = 0;
	_hiResPresentationViewY = 0;
	if (isHiRes()) {
		SmushPlayerRebel2 *ra2Player = static_cast<SmushPlayerRebel2 *>(_player);
		int nativeViewX = _viewX;
		int nativeViewY = _viewY;
		if (ra2Player && ra2Player->ra2PromoteCurrentFrameToHiRes(_viewX, _viewY)) {
			renderBitmap = _player->_dst;
			width = _player->_width;
			height = _player->_height;
			pitch = width;
			_hiResPresentationViewX = nativeViewX;
			_hiResPresentationViewY = nativeViewY;
			_viewX = 0;
			_viewY = 0;
		}
	}

	// NOTE: Level 2 handler 8's background is restored in procPreRendering before
	// SMUSH decodes the frame's FOBJ sprites. Handler 25 draws its corridor overlay
	// from IACT opcode 6 instead. Redrawing either here would overwrite enemies.

	// The cockpit frame covers laser beam edges, giving the appearance
	// that beams emerge from behind the cockpit.

	// 2. Lines 171-226: Draw turret overlays, targeting reticle, crosshair
	// We draw directly to screen at Y=180.

	renderStatusBarBackground(renderBitmap, pitch, width, height, videoWidth, videoHeight, gameplayStatusBarY);

	// Ship rendering. Handler 7 is drawn later, after its lasers, matching
	debugC(DEBUG_INSANE, "Ship Check: handler=%d shipSprite=%p flyShipSprite=%p shipLevelMode=%d numSprites=%d/%d",
		_rebelHandler, (void*)_shipSprite, (void*)_flyShipSprite, _shipLevelMode,
		_shipSprite ? _shipSprite->getNumChars() : 0,
		_flyShipSprite ? _flyShipSprite->getNumChars() : 0);

	renderVehicleShotImpacts(renderBitmap, pitch, width, height);
	renderHandler8Ship(renderBitmap, pitch, width, height);
	renderFallbackShip(renderBitmap, pitch, width, height);

	// Enemy target indicators (handler-specific; sprite-based in turret mode).
	renderEnemyOverlays(renderBitmap, pitch, width, height, videoWidth);

	renderExplosions(renderBitmap, pitch, width, height);

	// Laser shot beams - drawn BEFORE cockpit/HUD overlays so cockpit covers beam edges.
	renderLaserShots(renderBitmap, pitch, width, height);

	renderHandler7Ship(renderBitmap, pitch, width, height);

	//   Lines 202-229: GRD001 (wall, opaque, covers enemies behind wall)
	//   Lines 230-248: GRD002 (character, transparent, drawn last)
	renderHandler25ShipPre(renderBitmap, pitch, width, height);
	renderHandler25Ship(renderBitmap, pitch, width, height);

	// These are cockpit frame, crosshair, and reticle - drawn ON TOP of laser beams.
	renderTurretHudOverlays(renderBitmap, pitch, width, height, curFrame);

	// STEP 1B: Draw embedded SAN HUD overlays (from IACT chunks).
	renderEmbeddedHudOverlays(renderBitmap, pitch, width, height);

	renderStatusBarSprites(renderBitmap, pitch, width, height, gameplayStatusBarY, curFrame);

	updateGameplayDamageEffects(renderBitmap, pitch, width, height);
	checkGameplayPostRenderCollisions(renderBitmap, pitch, width, height, curFrame);
	updateGameplayDamageRecovery(curFrame);

	renderCrosshair(renderBitmap, pitch, width, height);

	renderHandler8MonitorEffect(renderBitmap, pitch, width, height);

	renderHandler8PovOverlay(renderBitmap, pitch, width, height);

	renderScoreHUD(renderBitmap, pitch, width, height, gameplayStatusBarY);

	if (_player) {
		_player->_fobjOffsetX = 0;
		_player->_fobjOffsetY = 0;
	}

	frameEndCleanup();
}

// procPostRendering -- Post-frame rendering: HUD, ships, enemies, effects, status bar.
// Called after FOBJ decoding. Dispatches to per-handler rendering functions
// for ship sprites, laser shots, explosions, crosshair, and damage effects.
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

	updatePostRenderScroll(width, height);
	updatePostRenderDeath();

	// End the looping attack-run segment once the shield/reactor is destroyed.
	if (_rebelShieldGateActive) {
		// Level 13: the finale (continuation segment, flag 0x40) ends when the last armed
		// group (the reactor) is depleted; the approach segment plays fully.
		if (_rebelReactorMode && _rebelGaugeArmed && _rebelLastArmedSlot >= 0 &&
		    (_player->_curVideoFlags & 0x40) != 0) {
			const int slot = _rebelLastArmedSlot;
			const short remaining = (slot < 10) ? _rebelValueCounters[slot]
			                                    : _rebelMaskCounters[slot - 10];
			if (remaining <= 0)
				_rebelShieldDestroyed = true;
		}
		if (_rebelShieldDestroyed)
			_vm->_smushVideoShouldFinish = true;
	}

	// Use video content coordinates, NOT oversized low-res gameplay-buffer coordinates.
	const int hudScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);
	const int videoWidth = 320 * hudScale;
	const int videoHeight = 200 * hudScale;
	const int statusBarY = 180 * hudScale;    // 0xb4 low-res, 0x168 high-res

	// Hide HUD/status bar during intro videos (marked by SmushPlayer video flag 0x20)
	// The 0x20 flag indicates a non-interactive cutscene/intro sequence OR menu
	bool introPlaying = ((_player->_curVideoFlags & 0x20) != 0);

	if (isHiRes() && _rebelHandler == 0) {
		SmushPlayerRebel2 *ra2Player = static_cast<SmushPlayerRebel2 *>(_player);
		if (ra2Player && ra2Player->ra2PromoteCurrentFrameToHiRes(0, 0)) {
			renderBitmap = _player->_dst;
			width = _player->_width;
			height = _player->_height;
			pitch = width;
		}
	}

	if (handlePostRenderMenuModes(renderBitmap, pitch, width, height, introPlaying))
		return;

	if (handlePostRenderIntro(renderBitmap, pitch, width, height, curFrame))
		return;
	renderGameplayPostFrame(renderBitmap, pitch, width, height, videoWidth, videoHeight, statusBarY, curFrame);
}

// Damage Visual Effect Functions
// Palette flash + screen shake when the player takes damage.

void InsaneRebel2::resetDamageFlash() {
	_damageFlashCounter = 0;
}

void InsaneRebel2::restoreDamageFlashPalette() {
	if (_player) {
		if (_damageRestorePaletteValid)
			_player->setPalette(_damageRestorePalette);
		else if (_damageFlashCounter != 0 || _damageHighFlashCounter != 0)
			_player->setPalette(_damageSavedPalette);
	}

	_damageFlashCounter = 0;
	_damageHighFlashCounter = 0;
	_damageShakeCounter = 0;
	_damageRestorePaletteValid = false;
}

void InsaneRebel2::initDamageFlash() {
	if (!_damageRestorePaletteValid && _player) {
		memcpy(_damageRestorePalette, _player->_pal, 0x300);
		_damageRestorePaletteValid = true;
	}

	if (_damageFlashCounter == 0) {
		// Save current SMUSH palette before modifying it
		memcpy(_damageSavedPalette, _player->_pal, 0x300);
	}
	_damageFlashCounter = 5;
}

void InsaneRebel2::triggerDamageEffect() {
	initDamageFlash();
	_damageShakeCounter = 10;
}

// Normal hit flash (_damageHighFlashCounter == 0 or odd):
//   Blend formula: output[i] = 0xFF - ((0xFF - saved[i]) * (0x10 - counter)) >> 4
// High-damage red pulse (_playerDamage >= 0xFF, even counter):
//   R channel only (every 3rd byte) using same formula with _damageHighFlashCounter.
void InsaneRebel2::updateDamageFlashPalette() {
	// High-damage mode: persistent red pulsing when damage is maxed out
	if (_playerDamage < 0xFF) {
		_damageHighFlashCounter = 0;
	} else {
		if (_damageHighFlashCounter == 0) {
			if (!_damageRestorePaletteValid) {
				memcpy(_damageRestorePalette, _player->_pal, 0x300);
				_damageRestorePaletteValid = true;
			}
			// Save palette on first frame of high-damage mode
			memcpy(_damageSavedPalette, _player->_pal, 0x300);
		}
		if (_damageHighFlashCounter < 0x10) {
			_damageHighFlashCounter++;
		}
	}

	if (_damageHighFlashCounter == 0 || (_damageHighFlashCounter & 1) != 0) {
		// Normal hit flash path: decrement counter, apply on even values.
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
				if (_damageFlashCounter == 0 && _damageHighFlashCounter == 0)
					_damageRestorePaletteValid = false;
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

// Shifts counter*5 random scanlines per frame, diminishing over 10 frames.
void InsaneRebel2::updateDamageEffect(byte *renderBitmap, int pitch, int width, int height) {
	if (_damageShakeCounter != 0) {
		_damageShakeCounter--;
		int numLines = _damageShakeCounter * 5;

		byte tempLine[640];
		const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
		const int maxY = MIN(height, renderHiRes ? 360 : 180);

		for (int n = numLines; n > 0; n--) {
			// Pick a random scanline within the gameplay area, not the status bar.
			if (maxY <= 0)
				continue;
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

// Rendering Helper Functions
// Extracted from procPostRendering for better readability.

void InsaneRebel2::renderTextOverlay(byte *renderBitmap, int pitch, int width, int height, int curFrame) {
	if (curFrame < _textOverlayFadeIn || curFrame >= _textOverlayFadeOut)
		return;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	const char *text = splayer->getString(_textOverlayID);
	debugC(DEBUG_INSANE, "Text overlay frame %d/%d-%d textID=0x%x text='%s'",
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

	// Older RA2 text loading joined multi-line strings with spaces, leaving
	// " ^f" as the separator; current TRES loading preserves real newlines.
	Common::Array<Common::String> lines;
	{
		Common::String cur;
		const char *s = text;
		while (*s) {
			if (*s == '\n' || *s == '\r') {
				if (!cur.empty())
					lines.push_back(cur);
				cur.clear();
				if (*s == '\r' && s[1] == '\n')
					s++;
				s++;
				continue;
			}
			if (*s == ' ' && s[1] == '^' && s[2] == 'f') {
				if (!cur.empty())
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

	const int textScale = isHiRes() ? 2 : 1;
	int drawY = _textOverlayY * textScale;
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
				if (parseRebel2TextOverlayFormat(s, mFont, mColor, fonts, ARRAYSIZE(fonts), defaultFont))
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
		int drawX = _textOverlayX * textScale - lineWidth / 2;
		int lineCharsDrawn = 0;
		{
			const char *s = lineStr;
			NutRenderer *curFont = defaultFont;
			int curColor = 1;
			while (s < lineEnd && (visCount + lineCharsDrawn) < displayLen) {
				if (parseRebel2TextOverlayFormat(s, curFont, curColor, fonts, ARRAYSIZE(fonts), defaultFont))
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
					drawRebel2Char(curFont, renderBitmap, clipRect, drawX, drawY, pitch, curColor, c);
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
	// Draw NUT-based HUD overlays for Handler 0x26 (turret modes)
	// - Position formula (low-res):
	//   X = 160 + (mouseOffsetX >> 4) - (width / 2) - spriteOffsetX
	//   Y = 182 - (mouseOffsetY >> 4) - height - spriteOffsetY
	// - Animation: spriteIndex = (frameCounter / 2) % 6

	if (_rebelHandler != 0x26 || !_hudOverlayNut || _hudOverlayNut->getNumChars() <= 0)
		return;

	// Calculate mouse offset (clamped to -127..127)
	Common::Point aimPos = getGameplayAimPoint();
	int mouseOffsetX = (aimPos.x - 160);
	int mouseOffsetY = (aimPos.y - 100);
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

	const int hudScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);

	const int baseSpriteW = _hudOverlayNut->getCharWidth(0);
	const int baseSpriteH = _hudOverlayNut->getCharHeight(0);
	const int baseSpriteXOff = _hudOverlayNut->getCharXOffset(0);
	const int baseSpriteYOff = _hudOverlayNut->getCharYOffset(0);
	const int horizontalTerm = (mouseOffsetX * hudScale) >> 4;
	const int verticalInput = isHiRes() ? mouseOffsetY * 2 - 0x100 : mouseOffsetY - 0x80;
	const int verticalTerm = verticalInput >> 4;
	int hudX = 160 * hudScale + horizontalTerm - baseSpriteW / 2 - baseSpriteXOff;
	int hudY = 182 * hudScale - verticalTerm - baseSpriteH - baseSpriteYOff;

	// Apply view offset for scrolling background
	hudX += _viewX;
	hudY += _viewY;

	// Draw base cockpit (sprite 0 always drawn first)
	renderNutSprite(renderBitmap, pitch, width, height,
		hudX + baseSpriteXOff, hudY + baseSpriteYOff, _hudOverlayNut, 0);

	// Draw animation overlay frame if not frame 0
	if (animFrame != 0 && animFrame < numSprites) {
		renderNutSprite(renderBitmap, pitch, width, height,
			hudX + _hudOverlayNut->getCharXOffset(animFrame),
			hudY + _hudOverlayNut->getCharYOffset(animFrame),
			_hudOverlayNut, animFrame);
	}

	debugC(DEBUG_INSANE, "HUD: Drawing NUT overlay frame %d/%d at (%d,%d) mouseOffset=(%d,%d)",
		  animFrame, numSprites, hudX, hudY, mouseOffsetX, mouseOffsetY);

	if (_hudOverlay2Nut && _hudOverlay2Nut->getNumChars() > 0) {
		int spr2W = _hudOverlay2Nut->getCharWidth(0);
		int spr2H = _hudOverlay2Nut->getCharHeight(0);
		int hud2X = 160 * hudScale + ((mouseOffsetX * hudScale) >> 4) - (spr2W / 2) + _viewX;
		int hud2Y = 182 * hudScale - ((mouseOffsetY * hudScale) >> 4) - spr2H + _viewY;
		renderNutSprite(renderBitmap, pitch, width, height, hud2X, hud2Y, _hudOverlay2Nut, 0);
	}
}

// renderEmbeddedHudOverlays -- Draw embedded SAN HUD overlays from IACT chunks.
void InsaneRebel2::renderEmbeddedHudOverlays(byte *renderBitmap, int pitch, int width, int height) {
	// Draw embedded SAN HUD overlays (from IACT chunks)
	// For Handler 7 (Level 3): HUD elements are scattered across the screen
	// For turret handlers: slots 1-2 form a two-part cockpit overlay

	for (int hudSlot = 1; hudSlot < 16; hudSlot++) {
		EmbeddedSanFrame &frame = _rebelEmbeddedHud[hudSlot];
		if (!isValidEmbeddedFrame(frame))
			continue;

		// Handler 25: skip slot 4 (corridor overlay) in post-rendering.
		// loading par4=4; drawing it here would cover enemies.
		if (_rebelHandler == 25 && hudSlot == 4) {
			continue;
		}

		// Skip small frames at (0,0) - likely animation patches
		if (frame.renderX == 0 && frame.renderY == 0 && frame.width < 50 && frame.height < 60) {
			debugC(DEBUG_INSANE, "Skipping small embedded frame at (0,0): slot=%d size=%dx%d",
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

				EmbeddedSanFrame &selectedFrame = _rebelEmbeddedHud[selectedId];
				if (countEmbeddedFramePixels(selectedFrame) == 0) {
					for (int i = 0; i < groupCount; i++) {
						EmbeddedSanFrame &altFrame = _rebelEmbeddedHud[groupMembers[i]];
						if (countEmbeddedFramePixels(altFrame) > 0) {
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

		const int hudScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);

		// Handler 0x26 turret positioning
		if (_rebelHandler == 0x26 && hudSlot >= 1 && hudSlot <= 4) {
			destX = 160 * hudScale - frame.width / 2 - frame.renderX;
			destY = 200 * hudScale - frame.height - frame.renderY;
		}

		// Handler 7 large cockpit frame positioning
		if (_rebelHandler == 7 && (hudSlot == 1 || hudSlot == 2) && frame.width > 100) {
			destX = 160 * hudScale - frame.width / 2 - frame.renderX;
			destY = 170 * hudScale - frame.height - frame.renderY;
		} else if (_rebelHandler == 7 && destX > 100 && destY > 50) {
			int16 offsetX = (_shipPosX - 160) / 8;
			int16 offsetY = (_shipPosY - 100) / 8;
			destX += offsetX;
			destY += offsetY;
		}

		destX += _viewX;
		destY += _viewY;

		debugC(DEBUG_INSANE, "Rendering embedded HUD slot=%d size=%dx%d at (%d,%d)",
			hudSlot, frame.width, frame.height, destX, destY);

		blitEmbeddedFrameRegion(renderBitmap, pitch, pitch, height, frame,
			destX, destY, 0, 0, frame.width, frame.height);
	}
}

void InsaneRebel2::renderStatusBarSprites(byte *renderBitmap, int pitch, int width, int height,
										  int statusBarY, int32 curFrame) {
	// DISPFONT.NUT sprite layout:
	//   Sprite 1:   Status bar background
	//   Sprites 2-5: Difficulty variants (full status bar with 1-4 stars)
	//   Sprite 6:   Bar fill element (reused for both damage and lives bars)
	//   Sprite 7:   Damage alert overlay (flashing when critical)

	if (!_smush_cockpitNut)
		return;

	int numSprites = _smush_cockpitNut->getNumChars();
	const int statusScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);

	if (numSprites > 1) {
		renderNutSprite(renderBitmap, pitch, width, height,
			_viewX, statusBarY + _viewY, _smush_cockpitNut, 1);
	}

	int difficulty = _difficulty;
	if (difficulty > 3)
		difficulty = 3;
	int difficultySprite = difficulty + 2;
	if (numSprites > difficultySprite) {
		renderNutSprite(renderBitmap, pitch, width, height,
			_viewX, statusBarY + _viewY, _smush_cockpitNut, difficultySprite);
	}

	//   Clip rect (low-res): {X=0x3f, Y=9, W=0x40, H=6} = {63, 9, 64, 6}
	//   Bar width = shield_value >> 2 (divide by 4, range 0-63)
	//   If shield > 0xAA (170): alert blink with sprite 7
	if (numSprites > 6) {
		int damageBarWidth = (_playerDamage * statusScale) >> 2;

		const byte *src = _smush_cockpitNut->getCharData(6);
		int sw = _smush_cockpitNut->getCharWidth(6);
		int sh = _smush_cockpitNut->getCharHeight(6);

		const int dmgClipX = 63 * statusScale;
		const int dmgClipY = 9 * statusScale;
		const int dmgClipW = 64 * statusScale;
		const int dmgClipH = 6 * statusScale;

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

	//   Clip rect (low-res): {X=0xa8, Y=7, W=0x32, H=9} = {168, 7, 50, 9}
	//   Bar width = min((lives * 5 - 5) * 2, 50) — only drawn when lives > 1
	if (numSprites > 6 && _playerLives > 1) {
		int livesBarWidth = (_playerLives * 5 - 5) * 2;
		if (livesBarWidth > 50)
			livesBarWidth = 50;
		livesBarWidth *= statusScale;

		const byte *src = _smush_cockpitNut->getCharData(6);
		int sw = _smush_cockpitNut->getCharWidth(6);
		int sh = _smush_cockpitNut->getCharHeight(6);

		const int livClipX = 168 * statusScale;
		const int livClipY = 7 * statusScale;
		const int livClipW = 50 * statusScale;
		const int livClipH = 9 * statusScale;

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

// renderHandler7FlySprite -- Draw a native Handler 7 FLY sprite into the current presentation target.
void InsaneRebel2::renderHandler7FlySprite(byte *renderBitmap, int pitch, int width, int height,
		bool renderHiRes, int renderScale, int nativeViewX, int nativeViewY,
		int nativeX, int nativeY, NutRenderer *nut, int spriteIndex) {
	int dstX = renderHiRes ? (nativeX - nativeViewX) * renderScale : nativeX;
	int dstY = renderHiRes ? (nativeY - nativeViewY) * renderScale : nativeY;

	if (renderHiRes) {
		const int clipBottom = MIN(height, _gameplayPresentationClipBottom + 1);
		renderNutSpriteScaledClipped(renderBitmap, pitch, width, height,
			0, 0, width, clipBottom, dstX, dstY, nut, spriteIndex, false, renderScale, true);
	} else {
		renderNutSprite(renderBitmap, pitch, width, height, dstX, dstY, nut, spriteIndex);
	}
}

void InsaneRebel2::renderHandler7Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 7 Ship Rendering (Third-Person Ship - FLY sprites)
	// The ship sprite is drawn at the perspective-transformed position offset from center.

	if (_rebelHandler != 7 || !_flyShipSprite || _shipLevelMode == 5)
		return;

	int numSprites = _flyShipSprite->getNumChars();
	int spriteIndex = _shipDirectionIndex;
	if (spriteIndex < 0)
		spriteIndex = 0;
	if (spriteIndex >= numSprites)
		spriteIndex = numSprites - 1;

	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int renderScale = renderHiRes ? 2 : 1;
	const int nativeViewX = renderHiRes ? _hiResPresentationViewX : 0;
	const int nativeViewY = renderHiRes ? _hiResPresentationViewY : 0;

	Common::Point projected = getHandler7ProjectedPoint();
	Common::Point shipDraw = getHandler7ShipDrawPoint();
	if (renderHiRes) {
		// Low-res draws into the native source buffer with _viewX/_viewY baked in,
		// then SmushPlayer copies the scrolled viewport. High-res promotion has
		// already consumed those offsets, so reconstruct the same native source
		// position before applying the 2x presentation transform.
		projected.x += nativeViewX;
		projected.y += nativeViewY;
		shipDraw.x += nativeViewX;
		shipDraw.y += nativeViewY;
	}
	int shipCenterX = projected.x;
	int shipCenterY = projected.y;

	if (_flyLaserSprite && _flyLaserSprite->getNumChars() > 0) {
		const int laserChars = _flyLaserSprite->getNumChars();
		_flyEffectAnimCounter++;

		if (_flyControlMode == 0) {
			int16 leftDist = _flyShipScreenX - _corridorLeftX;
			if (leftDist < 0x32) {
				int cueIndex = _flyEffectAnimCounter % 10;
				if (cueIndex >= 0 && cueIndex < laserChars) {
					int cueX = ((shipCenterX - 0x28) - leftDist) - leftDist / 2;
					renderHandler7FlySprite(renderBitmap, pitch, width, height,
						renderHiRes, renderScale, nativeViewX, nativeViewY,
						cueX, shipCenterY, _flyLaserSprite, cueIndex);
				}
			}

			int16 rightDist = _corridorRightX - _flyShipScreenX;
			if (rightDist < 0x32) {
				int cueIndex = (_flyEffectAnimCounter % 10) + 10;
				if (cueIndex >= 0 && cueIndex < laserChars) {
					int cueX = rightDist / 2 + rightDist + shipCenterX + 0x28;
					renderHandler7FlySprite(renderBitmap, pitch, width, height,
						renderHiRes, renderScale, nativeViewX, nativeViewY,
						cueX, shipCenterY, _flyLaserSprite, cueIndex);
				}
			}
		} else {
			int16 bottomDist = _corridorBottomY - _flyShipScreenY;
			Common::Point bottomProjected = getHandler7ProjectedPointFor(_flyShipScreenX, _corridorBottomY);
			if (renderHiRes) {
				bottomProjected.x += nativeViewX;
				bottomProjected.y += nativeViewY;
			}
			int bottomX = bottomProjected.x;
			int bottomY = bottomProjected.y;

			if (bottomDist < 0x19) {
				_flyEffectAnimCounter++;
				int cueIndex = _flyEffectAnimCounter % 10;
				if (cueIndex >= 0 && cueIndex < laserChars)
					renderHandler7FlySprite(renderBitmap, pitch, width, height,
						renderHiRes, renderScale, nativeViewX, nativeViewY,
						bottomX, bottomY, _flyLaserSprite, cueIndex);
			}
			if (bottomDist < 0x32) {
				_flyEffectAnimCounter++;
				int cueIndex = _flyEffectAnimCounter % 10;
				if (cueIndex >= 0 && cueIndex < laserChars)
					renderHandler7FlySprite(renderBitmap, pitch, width, height,
						renderHiRes, renderScale, nativeViewX, nativeViewY,
						bottomX, bottomY, _flyLaserSprite, cueIndex);
			}

			int cueIndex = _flyEffectAnimCounter % 10;
			if (cueIndex >= 0 && cueIndex < laserChars)
				renderHandler7FlySprite(renderBitmap, pitch, width, height,
					renderHiRes, renderScale, nativeViewX, nativeViewY,
					bottomX, bottomY, _flyLaserSprite, cueIndex);
		}
	}

	int drawX = renderHiRes ? (shipDraw.x - nativeViewX) * renderScale : shipDraw.x;
	int drawY = renderHiRes ? (shipDraw.y - nativeViewY) * renderScale : shipDraw.y;

	renderHandler7FlySprite(renderBitmap, pitch, width, height,
		renderHiRes, renderScale, nativeViewX, nativeViewY,
		shipDraw.x, shipDraw.y, _flyShipSprite, spriteIndex);

	if (_flyLaserSprite && _flyOverlayRepeatCount > 0) {
		int overlayIndex = spriteIndex + 0x14;
		if (overlayIndex >= 0 && overlayIndex < _flyLaserSprite->getNumChars()) {
			for (int i = 0; i < _flyOverlayRepeatCount; i++) {
				renderHandler7FlySprite(renderBitmap, pitch, width, height,
					renderHiRes, renderScale, nativeViewX, nativeViewY,
					shipDraw.x, shipDraw.y, _flyLaserSprite, overlayIndex);
			}
		}
	}

	if (_flyTargetSprite && _rebelDetailMode >= 0 &&
		spriteIndex >= 0 && spriteIndex < _flyTargetSprite->getNumChars()) {
		renderHandler7FlySprite(renderBitmap, pitch, width, height,
			renderHiRes, renderScale, nativeViewX, nativeViewY,
			shipDraw.x, shipDraw.y, _flyTargetSprite, spriteIndex);
	}

	debugC(DEBUG_INSANE, "Handler7Ship: draw=(%d,%d) sprite=%d/%d shipPos=(%d,%d) view=(%d,%d) persp=(%d,%d) smoothVel=%d vertIn=%d fxCtr=%d fxRep=%d",
		drawX, drawY, spriteIndex, numSprites, _flyShipScreenX, _flyShipScreenY,
		renderHiRes ? nativeViewX : _viewX, renderHiRes ? nativeViewY : _viewY,
		_perspectiveX, _perspectiveY, _smoothedVelocity, _verticalInput, _flyEffectAnimCounter, _flyOverlayRepeatCount);
}

void InsaneRebel2::renderHandler8Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 8 Ship Rendering (Third-Person On Foot - POV sprites)
	// Uses _shipSprite (POV001) with position-based offset
	//       (short)(shipPosX - 0xa0) >> 3,    // small X offset
	//       (short)(shipPosY - 0x28) >> 2,    // small Y offset
	// parameters. The sprite's built-in offsets encode where it should appear
	// on screen (e.g., center for Level 2/11, bottom for Level 12 FPS gun).

	if (_rebelHandler != 8 || !_shipSprite || _shipLevelMode == 5)
		return;

	int16 displayOffsetX = (_shipPosX - 0xa0) >> 3;
	int16 displayOffsetY = (_shipPosY - 0x28) >> 2;

	int numSprites = _shipSprite->getNumChars();
	int spriteIndex = _shipFiring ? 1 : 0;
	if (spriteIndex >= numSprites)
		spriteIndex = 0;

	// The internal offsets position the sprite correctly for each level type
	// (centered for Level 2/11 third-person, bottom for Level 12 FPS).
	int16 spriteXOffset = _shipSprite->getCharXOffset(spriteIndex);
	int16 spriteYOffset = _shipSprite->getCharYOffset(spriteIndex);
	int drawX = displayOffsetX + spriteXOffset;
	int drawY = displayOffsetY + spriteYOffset;

	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int renderScale = renderHiRes ? 2 : 1;
	if (renderHiRes) {
		renderNutSpriteScaledClipped(renderBitmap, pitch, width, height,
			0, 0, width, height,
			drawX * renderScale, drawY * renderScale, _shipSprite, spriteIndex,
			false, renderScale, true);
	} else {
		renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _shipSprite, spriteIndex);
	}

	if (_shipSprite2) {
		int shadowIndex = _shipFiring ? 1 : 0;
		if (shadowIndex < _shipSprite2->getNumChars()) {
			int16 shadowXOff = _shipSprite2->getCharXOffset(shadowIndex);
			int16 shadowYOff = _shipSprite2->getCharYOffset(shadowIndex);
			int shadowX = displayOffsetX + shadowXOff;
			int shadowY = displayOffsetY + shadowYOff;
			if (renderHiRes) {
				renderNutSpriteScaledClipped(renderBitmap, pitch, width, height,
					0, 0, width, height,
					shadowX * renderScale, shadowY * renderScale, _shipSprite2, shadowIndex,
					false, renderScale, true);
			} else {
				renderNutSprite(renderBitmap, pitch, width, height, shadowX, shadowY, _shipSprite2, shadowIndex);
			}
		}
	}

	int sprW = _shipSprite->getCharWidth(spriteIndex);
	int sprH = _shipSprite->getCharHeight(spriteIndex);
	debugC(DEBUG_INSANE, "Handler8: Ship at (%d,%d) raw(%d,%d) offset(%d,%d) nutOff(%d,%d) size(%d,%d) bottom=%d view(%d,%d) sprite=%d/%d scale=%d",
		drawX, drawY, _shipPosX, _shipPosY, displayOffsetX, displayOffsetY,
		spriteXOffset, spriteYOffset, sprW, sprH, drawY + sprH,
		_viewX, _viewY, spriteIndex, numSprites, renderScale);
}

void InsaneRebel2::renderVehicleShotImpacts(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 8)
		return;

	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;

	for (int i = 0; i < 7; i++) {
		VehicleShotImpact &impact = _vehicleShotImpacts[i];
		if (impact.counter <= 0)
			continue;

		int drawX = impact.x - _shipPosX;
		int drawY = impact.y - _shipPosY;
		if (renderHiRes) {
			drawX = (drawX - _hiResPresentationViewX) * 2;
			drawY = (drawY - _hiResPresentationViewY) * 2;
		}

		if (_shipOverlay1 && impact.spriteIndex >= 0 && impact.spriteIndex < _shipOverlay1->getNumChars()) {
			int spriteX = drawX + _shipOverlay1->getCharXOffset(impact.spriteIndex) -
				_shipOverlay1->getCharWidth(impact.spriteIndex) / 2;
			int spriteY = drawY + _shipOverlay1->getCharYOffset(impact.spriteIndex) -
				_shipOverlay1->getCharHeight(impact.spriteIndex) / 2;
			for (int repeat = 0; repeat <= (impact.counter >> 2); repeat++)
				renderNutSprite(renderBitmap, pitch, width, height, spriteX, spriteY, _shipOverlay1, impact.spriteIndex);
		}
		if (_shipOverlay2 && impact.spriteIndex >= 0 && impact.spriteIndex < _shipOverlay2->getNumChars()) {
			int spriteX = drawX + _shipOverlay2->getCharXOffset(impact.spriteIndex) -
				_shipOverlay2->getCharWidth(impact.spriteIndex) / 2;
			int spriteY = drawY + _shipOverlay2->getCharYOffset(impact.spriteIndex) -
				_shipOverlay2->getCharHeight(impact.spriteIndex) / 2;
			renderNutSprite(renderBitmap, pitch, width, height, spriteX, spriteY, _shipOverlay2, impact.spriteIndex);
		}

		impact.counter--;
	}
}

// Handler 25: Draw GRD001 (wall/cockpit overlay) in procPostRendering.
// GRD sprites drawn AFTER FOBJ enemies, before GRD002. Mode-based clipping:
//   Mode 1, damage==0: left half only (pixels 0-159)
//   Mode 4, damage==0: right half only (pixels 160-319)
//   All other cases: full width (320 pixels)
void InsaneRebel2::renderHandler25ShipPre(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 25)
		return;

	if (!_grd001Sprite || _grd001Sprite->getNumChars() <= 0)
		return;

	// CRITICAL: Clip height to 180 (0xb4) + viewport Y to avoid drawing over status bar.
	// In high-res presentation the low-res GRD sprite is scaled into the promoted
	// 640x400 frame, so the gameplay clip becomes 360 pixels tall.
	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int renderScale = renderHiRes ? 2 : 1;
	const int nativeViewX = renderHiRes ? _hiResPresentationViewX : 0;
	const int nativeViewY = renderHiRes ? _hiResPresentationViewY : 0;
	const int nativeBufferViewX = renderHiRes ? nativeViewX : _viewX;
	const int nativeBufferViewY = renderHiRes ? nativeViewY : _viewY;
	int renderHeight = renderHiRes ? MIN(height, 180 * renderScale) : MIN(height, 180 + _viewY);

	bool shouldDraw = false;
	bool useHalfWidth = false;
	bool useRightHalf = false;

	if (_grdSpriteMode == 1) {
		shouldDraw = true;
		useHalfWidth = (_rebelDamageLevel == 0);  // Half width when uncovered
	}
	else if (_grdSpriteMode == 2 && _rebelDamageLevel != 0) {
		shouldDraw = true;
	}
	else if (_grdSpriteMode == 3) {
		shouldDraw = true;
	}
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

		// Add viewport offset so sprite follows the visible area.
		// Handler 25 stays viewport-locked in low-res mode, so _viewX/_viewY
		// remain 0 even when the backing buffer is larger than 320x200.
		// Other oversized-buffer modes scroll and need this compensation.
		int nativeDrawX = _rebelViewOffset2X + spriteXOffset + nativeBufferViewX;
		int nativeDrawY = _rebelViewOffset2Y + spriteYOffset + nativeBufferViewY;
		int drawX = renderHiRes ? (nativeDrawX - nativeViewX) * renderScale : nativeDrawX;
		int drawY = renderHiRes ? (nativeDrawY - nativeViewY) * renderScale : nativeDrawY;

		// - mode1 uncovered: left half
		// - mode4 uncovered: right half
		int clipLeft = 0;
		int clipRight = renderHiRes ? 320 : width;
		if (useHalfWidth) {
			const int halfWidth = clipRight / 2;
			clipLeft = useRightHalf ? halfWidth : 0;
			clipRight = clipLeft + halfWidth;
		}
		int scaledClipLeft = clipLeft * renderScale;
		int scaledClipRight = clipRight * renderScale;

		renderNutSpriteScaledClipped(renderBitmap, pitch, width, renderHeight,
			scaledClipLeft, 0, scaledClipRight, renderHeight,
			drawX, drawY, _grd001Sprite, 0, false, renderScale, false);

		debugC(DEBUG_INSANE, "Handler25 PRE: GRD001 at (%d,%d) nutOff(%d,%d) viewOff(%d,%d) size(%d,%d) mode=%d dmg=%d halfW=%d rightHalf=%d clip=[%d,%d) scale=%d",
			drawX, drawY, spriteXOffset, spriteYOffset, _rebelViewOffset2X, _rebelViewOffset2Y,
			spriteW, spriteH, _grdSpriteMode, _rebelDamageLevel, useHalfWidth ? 1 : 0, useRightHalf ? 1 : 0, scaledClipLeft, scaledClipRight, renderScale);

		if (_grdSpriteMode == 3 && _grd005Sprite && _grd005Sprite->getNumChars() > 1) {
			const int overlayIdx = 1;
			int overlayW = _grd005Sprite->getCharWidth(overlayIdx);
			int overlayH = _grd005Sprite->getCharHeight(overlayIdx);
			int16 overlayXOffset = _grd005Sprite->getCharXOffset(overlayIdx);
			int16 overlayYOffset = _grd005Sprite->getCharYOffset(overlayIdx);

			int nativeOverlayX = _rebelViewOffset2X + overlayXOffset + nativeBufferViewX;
			int nativeOverlayY = _rebelViewOffset2Y + overlayYOffset + nativeBufferViewY;
			int overlayDrawX = renderHiRes ? (nativeOverlayX - nativeViewX) * renderScale : nativeOverlayX;
			int overlayDrawY = renderHiRes ? (nativeOverlayY - nativeViewY) * renderScale : nativeOverlayY;

			renderNutSpriteScaledClipped(renderBitmap, pitch, width, renderHeight,
				0, 0, width, renderHeight,
				overlayDrawX, overlayDrawY, _grd005Sprite, overlayIdx, false, renderScale, false);

			debugC(DEBUG_INSANE, "Handler25 PRE: GRD005 at (%d,%d) nutOff(%d,%d) viewOff(%d,%d) size(%d,%d) mode=%d scale=%d",
				overlayDrawX, overlayDrawY, overlayXOffset, overlayYOffset,
				_rebelViewOffset2X, _rebelViewOffset2Y, overlayW, overlayH, _grdSpriteMode, renderScale);
		}
	}
}

void InsaneRebel2::renderHandler25Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 25 POST-rendering: Draw GRD002 (character sprite) on top of enemies.
	// GRD001 (wall/cockpit) is drawn before this via renderHandler25ShipPre().
	// GRD002 is drawn LAST (after enemies) so the character appears in front.

	if (_rebelHandler != 25)
		return;

	// CRITICAL: Clip height to 180 (0xb4) + viewport Y to avoid drawing over status bar.
	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int renderScale = renderHiRes ? 2 : 1;
	const int nativeViewX = renderHiRes ? _hiResPresentationViewX : 0;
	const int nativeViewY = renderHiRes ? _hiResPresentationViewY : 0;
	const int nativeBufferViewX = renderHiRes ? nativeViewX : _viewX;
	const int nativeBufferViewY = renderHiRes ? nativeViewY : _viewY;
	int renderHeight = renderHiRes ? MIN(height, 180 * renderScale) : MIN(height, 180 + _viewY);

	//   If damage == 0: index = yZone * 5 + xZone + 5 (aiming-based, 5-14)
	//   If damage != 0:
	//     If direction == 0: index = 5 - damage (0-5, covered left)
	//     If direction != 0: index = 25 - damage (20-25, covered right)
	if (_grd002Sprite && _grd002Sprite->getNumChars() > 0) {
		int spriteIdx;
		int numSprites = _grd002Sprite->getNumChars();

		// Mirror when: direction != 0 AND damage == 0 (fully uncovered, facing right)
		bool shouldMirror = (_rebelFlightDir != 0) && (_rebelDamageLevel == 0);

		if (_rebelDamageLevel == 0) {
			// Uncovered state: use aiming-based sprite selection (5-14)
			// Calculate zones from crosshair position relative to playable area
			// The playable area bounds are defined by corridor boundaries.
			// xZone = 0-4 (left to right), yZone = 0-1 (top to bottom)
			// Default to center if bounds not set
			int16 areaLeft = (_corridorLeftX > 0) ? _corridorLeftX : 0;
			int16 areaRight = (_corridorRightX > 0) ? _corridorRightX : 320;
			int16 areaTop = (_corridorTopY > 0) ? _corridorTopY : 0;
			int16 areaBottom = (_corridorBottomY > 0) ? _corridorBottomY : 180;

			// Get crosshair position (using mouse position scaled to game coords)
			Common::Point aimPos = getGameplayAimPoint();
			int16 crosshairX = aimPos.x;
			int16 crosshairY = aimPos.y;
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

			if (_rebelFlightDir == (yZone & 1)) {
				xZone = 4 - xZone;
			}

			spriteIdx = yZone * 5 + xZone + 5;
		} else {
			// Transitioning/covered state: use direction-based sprite
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

		// GRD002 explicitly adds sprite internal offsets from NUT header:
		// Normal case (direction==0 OR damage!=0):

		// Mirrored case (direction!=0 AND damage==0):

		// Now using actual NUT sprite offsets from NutRenderer!
		int16 spriteXOffset = _grd002Sprite->getCharXOffset(spriteIdx);
		int16 spriteYOffset = _grd002Sprite->getCharYOffset(spriteIdx);

		int nativeDrawX, nativeDrawY;

		if (shouldMirror) {
			nativeDrawX = _rebelViewOffset2X + (320 - spriteW - spriteXOffset) + nativeBufferViewX;
		} else {
			nativeDrawX = _rebelViewOffset2X + spriteXOffset + nativeBufferViewX;
		}

		nativeDrawY = spriteYOffset + _rebelViewOffset2Y + nativeBufferViewY;
		int drawX = renderHiRes ? (nativeDrawX - nativeViewX) * renderScale : nativeDrawX;
		int drawY = renderHiRes ? (nativeDrawY - nativeViewY) * renderScale : nativeDrawY;

		renderNutSpriteScaledClipped(renderBitmap, pitch, width, renderHeight,
			0, 0, width, renderHeight,
			drawX, drawY, _grd002Sprite, spriteIdx, shouldMirror, renderScale, true);

		debugC(DEBUG_INSANE, "Handler25: GRD002 at (%d,%d) nutOffset(%d,%d) viewOffset(%d,%d) size(%d,%d) spriteIdx=%d damage=%d dir=%d mirror=%d scale=%d",
			drawX, drawY, spriteXOffset, spriteYOffset, _rebelViewOffset2X, _rebelViewOffset2Y, spriteW, spriteH, spriteIdx, _rebelDamageLevel, _rebelFlightDir, shouldMirror ? 1 : 0, renderScale);
	}
}

// renderFallbackShip -- Fallback ship rendering using embedded HUD frame.
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
	if (!isValidEmbeddedFrame(shipFrame))
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

	blitEmbeddedFrameRegion(renderBitmap, pitch, width, height, shipFrame,
		drawX, drawY, srcX, srcY, spriteW, spriteH);

	debugC(DEBUG_INSANE, "Ship (fallback) at (%d,%d) strip=(%d,%d) of (%dx%d) dir=(%d,%d)",
		drawX, drawY, srcX, srcY, numHorizontal, numVertical, _shipDirectionH, _shipDirectionV);
}

void InsaneRebel2::renderEnemyOverlays(byte *renderBitmap, int pitch, int width, int height, int videoWidth) {
	// - Draws cockpit icon sprites 6..10 at enemy centers.
	// - Enabled when level flags bit 2 (0x04) is clear.
	// - Sprite index depends on object half-width bucket.
	// It is not a generic all-handler bracket overlay.
	if (_rebelHandler != 0x26 || !_smush_iconsNut)
		return;

	LevelDifficultyParams dparams = getDifficultyParams();
	if ((dparams.flags & 4) != 0)
		return;

	// FOBJ sprites are rendered with _fobjOffsetX/Y applied. Use the same offsets
	// so indicators stay aligned with decoded enemy sprites.
	int fobjOffX = _player ? _player->_fobjOffsetX : 0;
	int fobjOffY = _player ? _player->_fobjOffsetY : 0;
	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int indicatorScale = renderHiRes ? 2 : getRebel2IndicatorScale(width, height);
	const int nativeViewX = renderHiRes ? _hiResPresentationViewX : _viewX;
	const int nativeViewY = renderHiRes ? _hiResPresentationViewY : _viewY;
	const int nativeVideoWidth = renderHiRes ? videoWidth / indicatorScale : 320;

	Common::Rect viewRect(nativeViewX, nativeViewY, nativeViewX + nativeVideoWidth, nativeViewY + 200);
	const int sizeClamp = dparams.specialDamage;

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->destroyed || !it->active || isBitSet(it->id))
			continue;

		Common::Rect r(it->rect.left + fobjOffX, it->rect.top + fobjOffY,
		               it->rect.right + fobjOffX, it->rect.bottom + fobjOffY);
		if (r.right <= viewRect.left || r.left >= viewRect.right ||
		    r.bottom <= viewRect.top || r.top >= viewRect.bottom)
			continue;

		int halfW = r.width() / 2;
		int halfH = r.height() / 2;
		if (halfW <= 0 || halfH <= 0)
			continue;

		// class 0..4 -> sprite 6..10.
		int indicatorHalfW = halfW;
		if (sizeClamp > 0)
			indicatorHalfW = MIN(indicatorHalfW, sizeClamp / 2);

		int sizeClass;
		if (indicatorHalfW < 3) {
			sizeClass = 0;
		} else if (indicatorHalfW < 6) {
			sizeClass = 1;
		} else if (indicatorHalfW < 9) {
			sizeClass = 2;
		} else if (indicatorHalfW < 12) {
			sizeClass = 3;
		} else {
			sizeClass = 4;
		}

		int spriteIndex = sizeClass + 6;
		if (spriteIndex < 0 || spriteIndex >= _smush_iconsNut->getNumChars())
			continue;

		int centerX = r.left + halfW;
		int centerY = r.top + halfH;
		int iw = _smush_iconsNut->getCharWidth(spriteIndex);
		int ih = _smush_iconsNut->getCharHeight(spriteIndex);
		int drawX = centerX * indicatorScale - iw / 2;
		int drawY = centerY * indicatorScale - ih / 2;
		if (renderHiRes) {
			drawX = (centerX - nativeViewX) * indicatorScale - iw / 2;
			drawY = (centerY - nativeViewY) * indicatorScale - ih / 2;
		}
		renderNutSprite(renderBitmap, pitch, width, height,
			drawX, drawY, _smush_iconsNut, spriteIndex);
	}
}

// renderExplosions -- Dispatch to per-handler explosion renderer.
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

// renderExplosionFrame -- Shared explosion sprite path.
// 320x200 path used here, they share centered NUT drawing; callers keep their
// coordinate transforms, frame timing, and scale bucket rules explicit.
void InsaneRebel2::renderExplosionFrame(byte *renderBitmap, int pitch, int width, int height,
		InsaneRebel2::Explosion &explosion, int screenX, int screenY, ExplosionFrameAdvance advance,
		bool resolutionDependentScale) {
	if (!explosion.active)
		return;

	if (explosion.counter <= 0) {
		explosion.active = false;
		return;
	}

	if (advance == kExplosionAdvanceBeforeDraw)
		explosion.counter--;

	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int mediumThreshold = (resolutionDependentScale && renderHiRes) ? 20 : 10;
	const int largeThreshold = (resolutionDependentScale && renderHiRes) ? 40 : 20;
	int baseIndex;
	if (explosion.scale > largeThreshold) {
		baseIndex = 29;
	} else if (explosion.scale > mediumThreshold) {
		baseIndex = 19;
	} else {
		baseIndex = 9;
	}

	int spriteIndex = baseIndex + (12 - explosion.counter);

	if (_smush_iconsNut->getNumChars() > spriteIndex) {
		int ew = _smush_iconsNut->getCharWidth(spriteIndex);
		int eh = _smush_iconsNut->getCharHeight(spriteIndex);
		int drawX = screenX;
		int drawY = screenY;
		if (renderHiRes) {
			drawX = (screenX - _hiResPresentationViewX) * 2;
			drawY = (screenY - _hiResPresentationViewY) * 2;
		}
		if (renderHiRes) {
			renderNutSpriteClipped(renderBitmap, pitch, height,
				0, 0, MIN(width, pitch), _gameplayPresentationClipBottom + 1,
				drawX - ew / 2, drawY - eh / 2, _smush_iconsNut, spriteIndex);
		} else {
			renderNutSprite(renderBitmap, pitch, width, height,
				drawX - ew / 2, drawY - eh / 2, _smush_iconsNut, spriteIndex);
		}
	}

	if (advance == kExplosionAdvanceAfterDraw)
		explosion.counter--;
}

void InsaneRebel2::renderTurretExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		// At 320x200 low-res turret view, projection is effectively identity.
		int screenX = _explosions[i].x;
		int screenY = _explosions[i].y;
		renderExplosionFrame(renderBitmap, pitch, width, height, _explosions[i],
			screenX, screenY, kExplosionAdvanceAfterDraw, false);
	}
}

// Scale thresholds: <11, <21. No secondary NUT.
void InsaneRebel2::renderVehicleExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		int screenX = _explosions[i].x - _shipPosX;
		int screenY = _explosions[i].y - _shipPosY;
		renderExplosionFrame(renderBitmap, pitch, width, height, _explosions[i],
			screenX, screenY, kExplosionAdvanceAfterDraw, false);
	}
}

void InsaneRebel2::renderSpaceExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		// At low-res, this is close to identity for the ship view.
		int screenX = _explosions[i].x;
		int screenY = _explosions[i].y;
		renderExplosionFrame(renderBitmap, pitch, width, height, _explosions[i],
			screenX, screenY, kExplosionAdvanceAfterDraw, true);
	}

	//   0 = left side (hit left boundary), 1 = right side (hit right boundary)
	//   2 = bottom (zone push down), 3 = top (zone push up)
	// Sprite frames: 0x15 - cooldown = 21 - cooldown (frames 12→21 as cooldown 9→0)
	if (_hitCooldown != 0) {
		_hitCooldown--;

		int numChars = _smush_iconsNut->getNumChars();
		int spriteIndex = 0x15 - _hitCooldown;  // 21 - remaining cooldown
		const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
		const int nativeViewX = renderHiRes ? _hiResPresentationViewX : _viewX;
		const int nativeViewY = renderHiRes ? _hiResPresentationViewY : _viewY;

		if (spriteIndex >= 0 && spriteIndex < numChars) {
			Common::Point shipProjected = getHandler7ProjectedPoint();
			if (renderHiRes) {
				shipProjected.x += nativeViewX;
				shipProjected.y += nativeViewY;
			}

			// Per-direction offset from ship center.
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

			int drawX = shipProjected.x + offsetX;
			int drawY = shipProjected.y + offsetY;
			if (renderHiRes) {
				drawX = (drawX - nativeViewX) * 2;
				drawY = (drawY - nativeViewY) * 2;
			}

			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			if (renderHiRes) {
				renderNutSpriteClipped(renderBitmap, pitch, height,
					0, 0, MIN(width, pitch), _gameplayPresentationClipBottom + 1,
					drawX - ew / 2, drawY - eh / 2, _smush_iconsNut, spriteIndex);
			} else {
				renderNutSprite(renderBitmap, pitch, width, height,
					drawX - ew / 2, drawY - eh / 2, _smush_iconsNut, spriteIndex);
			}

			debugC(DEBUG_INSANE, "H7 corridor explosion: dir=%d frame=%d pos=(%d,%d) cooldown=%d",
				_spaceShotDirection, spriteIndex, drawX, drawY, _hitCooldown);
		}
	}
}

void InsaneRebel2::renderHandler25Explosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		int screenX = _explosions[i].x + _rebelViewOffsetX;
		int screenY = _explosions[i].y + _rebelViewOffsetY;
		renderExplosionFrame(renderBitmap, pitch, width, height, _explosions[i],
			screenX, screenY, kExplosionAdvanceBeforeDraw, true);
	}
}

// renderLaserShots -- Dispatch to per-handler laser renderer.
void InsaneRebel2::renderLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	switch (_rebelHandler) {
	case 0x26:
		renderTurretLaserShots(renderBitmap, pitch, width, height);
		break;
	case 8:
		renderVehicleLaserShots(renderBitmap, pitch, width, height);
		break;
	case 7:
		renderSpaceLaserShots(renderBitmap, pitch, width, height);
		break;
	case 25:
		renderHandler25LaserShots(renderBitmap, pitch, width, height);
		break;
	default:
		// No laser rendering for other handlers
		break;
	}
}

void InsaneRebel2::renderTurretLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// Uses pre-initialized _laserTexture from sprite 5 of CPITIMAG.NUT

	int16 maxDuration = getShotMaxDuration();
	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int nativeViewX = renderHiRes ? _hiResPresentationViewX : _viewX;
	const int nativeViewY = renderHiRes ? _hiResPresentationViewY : _viewY;

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter <= 0)
			continue;

		int16 pan = ((2 - _turretShots[i].counter) * (_turretShots[i].targetX - nativeViewX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning to sound channel i+1

		int16 targetX = _turretShots[i].targetX;
		int16 targetY = _turretShots[i].targetY;
		int16 progress = maxDuration - _turretShots[i].counter;

		switch (_rebelLevelType) {
		case 1:
			// Type 1: 3 guns (triple cannon configuration)
			// Gun 1: (0x136, 0xaa) = (310, 170) - right
			// Gun 2: (0xa0, 0x17c) = (160, 380) - center bottom (off-screen, clipped)
			// Gun 3: (0x0a, 0xaa) = (10, 170) - left
			drawLaserBeam(renderBitmap, pitch, width, height,
				310 + nativeViewX, 170 + nativeViewY, targetX, targetY,
				progress, maxDuration, 12, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				160 + nativeViewX, 380 + nativeViewY, targetX, targetY,
				progress, maxDuration, 8, 5, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				10 + nativeViewX, 170 + nativeViewY, targetX, targetY,
				progress, maxDuration, 12, 8, 12);
			break;

		case 2:
		case 5:
			// Type 2/5: 2 guns (wing cannons)
			// Left: (0x6e, 0xe6) = (110, 230)
			// Right: (0xd2, 0xe6) = (210, 230)
			// Assembly: widthScale=0x19(25), heightScale=8, thickness=0xC(12)
			drawLaserBeam(renderBitmap, pitch, width, height,
				110 + nativeViewX, 230 + nativeViewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				210 + nativeViewX, 230 + nativeViewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);
			break;

		case 6:
			// Type 6: 2 guns (offscreen - cinematic effect)
			// Gun 1: (-100, 0)
			// Gun 2: (0, 0)
			// Assembly: widthScale=0x19(25), heightScale=8, thickness=0xC(12)
			drawLaserBeam(renderBitmap, pitch, width, height,
				-100 + nativeViewX, 0 + nativeViewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				0 + nativeViewX, 0 + nativeViewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);
			break;

		default:
			// When seqNum & 1 == 0: Left (10, 50), Right (310, 130)
			// When seqNum & 1 == 1: Left (310, 50), Right (10, 130)
			// Assembly: widthScale=0x19(25), heightScale=8, thickness=0xC(12)
			if ((_turretShots[i].seqNum & 1) == 0) {
				drawLaserBeam(renderBitmap, pitch, width, height,
					10 + nativeViewX, 50 + nativeViewY, targetX, targetY,
					progress, maxDuration, 25, 8, 12);

				drawLaserBeam(renderBitmap, pitch, width, height,
					310 + nativeViewX, 130 + nativeViewY, targetX, targetY,
					progress, maxDuration, 25, 8, 12);
			} else {
				drawLaserBeam(renderBitmap, pitch, width, height,
					310 + nativeViewX, 50 + nativeViewY, targetX, targetY,
					progress, maxDuration, 25, 8, 12);

				drawLaserBeam(renderBitmap, pitch, width, height,
					10 + nativeViewX, 130 + nativeViewY, targetX, targetY,
					progress, maxDuration, 25, 8, 12);
			}
			break;
		}

		_turretShots[i].counter--;
	}
}

void InsaneRebel2::renderVehicleLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// No NUT check needed - uses pre-initialized _laserTexture

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_vehicleShots[i].counter <= 0)
			continue;

		// pan = ((2 - counter) * (targetX - 160)) / 2, clamped to [-127, 127]
		int16 pan = ((2 - _vehicleShots[i].counter) * (_vehicleShots[i].targetX - _viewX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning

		// creating a short muzzle flash effect (7 pixels horizontal, 25 pixels vertical).
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
		// Parameters: gunX, gunY -> shipScreenX, shipScreenY (NOT the stored target!)
		drawLaserBeam(renderBitmap, pitch, width, height,
			gunX, gunY,
			shipScreenX, shipScreenY,
			progress, maxDuration, 20, 8, 4);

		_vehicleShots[i].counter--;

		// mask pixels select the POV002/POV003 impact sprite index rendered by
		if (_shipLevelMode != 2 && _level2BackgroundLoaded && _level2Background) {
			int impactX = ((_shipPosX - 160) >> 3) + _shipPosX + 160;
			int impactY = ((_shipPosY - 40) >> 2) + _shipPosY + 105;
			int maskX = impactX - 160;
			int maskY = impactY - 30;

			if (impactX > 160 && impactX < 480 &&
					maskX >= 0 && maskX < 320 && maskY >= 0 && maskY < 200) {
				byte spriteIndex = _level2Background[maskY * 320 + maskX];
				if (spriteIndex != 0) {
					VehicleShotImpact &impact = _vehicleShotImpacts[_vehicleShotImpactIndex];
					_vehicleShotImpactIndex++;
					if (_vehicleShotImpactIndex >= 7)
						_vehicleShotImpactIndex = 0;

					impact.counter = 12;
					impact.x = impactX;
					impact.y = impactY;
					impact.spriteIndex = spriteIndex;
				}
			}
		}
	}
}

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

void InsaneRebel2::renderHandler25LaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// Only render when player is uncovered (damage == 0)
	if (_rebelDamageLevel != 0) {
		return;  // Can't shoot while taking cover
	}

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter <= 0)
			continue;

		int16 pan = ((2 - _turretShots[i].counter) * (_turretShots[i].targetX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning to sound channel i+1

		int16 targetX = _turretShots[i].targetX + _rebelViewOffsetX;
		int16 targetY = _turretShots[i].targetY + _rebelViewOffsetY;

		int16 gunX = _turretShots[i].gunX + _rebelViewOffsetX;
		int16 gunY = _turretShots[i].gunY + _rebelViewOffsetY;

		int16 progress = maxDuration - _turretShots[i].counter;

		// widthScale=0xC(12), heightScale=4, thickness=6
		drawLaserBeam(renderBitmap, pitch, width, height,
			gunX, gunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		_turretShots[i].counter--;

		debugC(DEBUG_INSANE, "Handler25: Laser shot %d from (%d,%d) to (%d,%d) progress=%d/%d",
			i, gunX, gunY, targetX, targetY, progress, maxDuration);
	}
}

// renderHandler8MonitorEffect -- Level 12 POV monitor scanline effect.
void InsaneRebel2::renderHandler8MonitorEffect(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 8 || !renderBitmap)
		return;

	const bool highRes = (width >= 640 || height >= 400);
	const int effectWidth = MIN<int>(MIN<int>(width, pitch), highRes ? 640 : 320);
	const int effectHeight = MIN<int>(height, highRes ? 360 : 180);
	if (effectWidth <= 0 || effectHeight <= 1)
		return;

	if (highRes) {
		for (int y = 1; y < effectHeight; y += 2) {
			byte *row = renderBitmap + y * pitch;
			memset(row, 4, effectWidth);
		}
		return;
	}

	if (_rebelDetailMode <= 0)
		return;

	const byte *monitorTable = _edgeTable + 0x400;
	for (int y = 1; y < effectHeight; y += 2) {
		byte *row = renderBitmap + y * pitch;
		for (int x = 0; x < effectWidth; x++)
			row[x] = monitorTable[row[x]];
	}
}

void InsaneRebel2::renderHandler8PovOverlay(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 8 || !renderBitmap || !_smush_talkfontNut || !_smush_povfontNut)
		return;

	Rebel2FontSet fontSet;
	fontSet.numFonts = 4;
	fontSet.defaultFont = 0;
	fontSet.fonts[0] = _smush_talkfontNut;
	fontSet.fonts[1] = _smush_smalfontNut ? _smush_smalfontNut : _smush_talkfontNut;
	fontSet.fonts[2] = _smush_titlefontNut ? _smush_titlefontNut : _smush_talkfontNut;
	fontSet.fonts[3] = _smush_povfontNut;

	ScummEngine_v7 *vm = (ScummEngine_v7 *)_vm;
	const int povScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);

	if (getHandler8PovOverlayRandom(vm, 20) == 0)
		_handler8HudGlyph = (char)(getHandler8PovOverlayRandom(vm, 5) + 0x23);

	drawHandler8PovOverlayText(fontSet, renderBitmap, pitch, width, height,
		getHandler8PovOverlayString(200), 10 * povScale, 5 * povScale, 1, kStyleAlignLeft);

	char buffer[128];
	Common::sprintf_s(buffer, "^f03&#$%c", _handler8HudGlyph);
	drawHandler8PovOverlayText(fontSet, renderBitmap, pitch, width, height,
		buffer, 10 * povScale, 150 * povScale, 248, kStyleAlignLeft);

	const char *text = getHandler8PovOverlayString(203);
	if (text) {
		Common::sprintf_s(buffer, text,
			(unsigned long)(uint32)(int32)getHandler8PovOverlayRandom(vm, 20000));
		drawHandler8PovOverlayText(fontSet, renderBitmap, pitch, width, height,
			buffer, 10 * povScale, 170 * povScale, 1, kStyleAlignLeft);
	}

	text = getHandler8PovOverlayString(202);
	if (text) {
		Common::sprintf_s(buffer, text,
			(unsigned long)(uint32)(int32)getHandler8PovOverlayRandom(vm, -0xd50));
		drawHandler8PovOverlayText(fontSet, renderBitmap, pitch, width, height,
			buffer, 220 * povScale, 160 * povScale, 1, kStyleAlignLeft);
	}

	text = getHandler8PovOverlayString(204);
	if (text) {
		Common::sprintf_s(buffer, text, (unsigned long)(uint32)(int32)(int16)_shipPosX,
			(unsigned long)(uint32)(int32)(int16)_shipPosY);
		drawHandler8PovOverlayText(fontSet, renderBitmap, pitch, width, height,
			buffer, 220 * povScale, 170 * povScale, 1, kStyleAlignLeft);
	}

	if (_handler8HudMessageTimer == 0) {
		_handler8HudMessageIndex = getHandler8PovOverlayRandom(vm, 100);
		if (_handler8HudMessageIndex < 7)
			_handler8HudMessageTimer = 0x32;
	} else {
		_handler8HudMessageTimer--;
		drawHandler8PovOverlayText(fontSet, renderBitmap, pitch, width, height,
			getHandler8PovOverlayString(_handler8HudMessageIndex + 0xcd),
			200 * povScale, 5 * povScale, 1, kStyleAlignCenter);
	}
}

// renderCrosshair -- Draw crosshair/reticle at the current handler's aim point.
void InsaneRebel2::renderCrosshair(byte *renderBitmap, int pitch, int width, int height) {
	// Don't draw crosshair when shooting is disabled (flight-only segments)
	if (!isShootingAllowed()) {
		return;
	}

	if (_rebelHandler == 25 && _rebelDamageLevel != 0) {
		return;
	}

	// Update target lock state and draw crosshair/reticle

	Common::Point aimPos;
	if (_rebelHandler == 7) {
		aimPos = getHandler7ShotTargetPoint();
	} else if (_rebelHandler == 8) {
		aimPos = getHandler8ShotTargetPoint();
	} else {
		aimPos = getGameplayAimPoint();
	}
	Common::Point worldMousePos = aimPos;
	if (_rebelHandler == 8) {
		worldMousePos.x += _shipPosX;
		worldMousePos.y += _shipPosY;
	} else if (_rebelHandler == 7 && isHiRes() && width >= 640 && height >= 400) {
		worldMousePos.x += _hiResPresentationViewX;
		worldMousePos.y += _hiResPresentationViewY;
	} else if (_rebelHandler != 7) {
		worldMousePos.x += _viewX;
		worldMousePos.y += _viewY;
	}
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
		int animOffset = (_targetLockTimer == 0) ? 0 : 3 - ((_vm->_system->getMillis() / 33) & 3);

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
		const int reticleScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);
		int crosshairX = aimPos.x * reticleScale;
		int crosshairY = aimPos.y * reticleScale;
		if (_rebelHandler != 7) {
			crosshairX += _viewX;
			crosshairY += _viewY;
		}

		// Handler 25 (0x19): Add view offset to crosshair position
		if (_rebelHandler == 25) {
			crosshairX += _rebelViewOffsetX * reticleScale;
			crosshairY += _rebelViewOffsetY * reticleScale;
		}

		crosshairX += _smush_iconsNut->getCharXOffset(reticleIndex) - cw / 2;
		crosshairY += _smush_iconsNut->getCharYOffset(reticleIndex) - ch / 2;

		renderNutSprite(renderBitmap, pitch, width, height,
			crosshairX, crosshairY,
			_smush_iconsNut, reticleIndex);
	}
}

void InsaneRebel2::frameEndCleanup() {
	// Reset enemy active flags and collision zones at frame end

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (!it->destroyed) {
			it->active = false;
		}
	}

	resetCollisionZones();
}

} // End of namespace Scumm
