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

extern void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
extern void smushDecodeUncompressed(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

int getRebel2IndicatorScale(int width, int height) {
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

static Common::String getRebel2VisibleTextPrefix(const char *text, int visibleChars) {
	Common::String out;
	if (!text || visibleChars <= 0)
		return out;

	int visible = 0;
	const char *s = text;
	while (*s && visible < visibleChars) {
		if (*s == '^' && (s[1] == 'f' || s[1] == 'c')) {
			out += *s++;
			out += *s++;
			while (*s >= '0' && *s <= '9')
				out += *s++;
			continue;
		}
		if (*s == '^' && s[1] == 'l') {
			out += *s++;
			out += *s++;
			continue;
		}

		out += *s;
		if (*s != '\n' && *s != '\r')
			visible++;
		s++;
	}

	return out;
}

static const char *getRebel2LevelEndFallbackString(int id) {
	switch (id) {
	case 190:
		return "^f02^c244Chapter Complete\n^f01^c244Password: %s";
	case 191:
		return "^f01^c001Target Accuracy %hd%%\nBonus %ld\n\nScore %ld\n^f00%s";
	case 192:
		return "^f01^c001Flight Errors %hd\nBonus %ld\n\nScore %ld\n^f00%s";
	case 193:
		return "^f01^c001Flight Errors %hd\n^f01^c001Target Accuracy %hd%%\nTotal Bonus %ld\n\nScore %ld\n^f00%s";
	case 194:
		return "^f01^c001Target Accuracy %hd%%\nSkill Bonus Awarded\nTotal Bonus %ld\n\nScore %ld\n^f00%s";
	case 195:
		return "^f01^c001Flight Errors %hd\nSkill Bonus Awarded\nTotal Bonus %ld\n\nScore %ld\n^f00%s";
	case 196:
		return "^f01^c001Flight Errors %hd\n^f01^c001Target Accuracy %hd%%\nSkill Bonus Awarded\nTotal Bonus %ld\n\nScore %ld\n^f00%s";
	default:
		return "";
	}
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

void InsaneRebel2::renderEmbeddedFrame(byte *renderBitmap, const EmbeddedSanFrame &frame, int userId) {
	// Some handler 25 overlays are static scene layers and draw immediately.
	bool skipImmediateDraw = (_rebelHandler == 7 || _rebelHandler == 8 ||
	                          _rebelHandler == 0x26);

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

void InsaneRebel2::loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) {
	if (userId < 0 || userId > 15 || !animData || size < 8) {
		debugC(DEBUG_INSANE, "Invalid embedded SAN: userId=%d, size=%d", userId, size);
		return;
	}

	Common::MemoryReadStream stream(animData, size);
	const int64 streamEnd = stream.size();

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

	while (!stream.eos() && stream.pos() + 8 <= animEnd) {
		uint32 tag;
		uint32 chunkSize;
		int64 chunkDataEnd;
		int64 nextChunkPos;
		if (!readEmbeddedSanChunkHeader(stream, animEnd, "top-level", tag, chunkSize, chunkDataEnd, nextChunkPos))
			break;

		if (tag == MKTAG('F','R','M','E')) {
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

					int codec = stream.readUint16LE();
					int left = stream.readSint16LE();
					int top = stream.readSint16LE();
					int width = stream.readUint16LE();
					int height = stream.readUint16LE();
					stream.readUint16LE();  // unknown
					stream.readUint16LE();  // unknown

					debugC(DEBUG_INSANE, "Embedded HUD frame: userId=%d, %dx%d at (%d,%d), codec=%d",
						userId, width, height, left, top, codec);

					if (!isHiRes() && (width > 400 || height > 250)) {
						debugC(DEBUG_INSANE, "SKIPPING high-res embedded frame: userId=%d, %dx%d (exceeds 400x250)",
							userId, width, height);
						stream.seek(nextSubPos);
						continue;
					}

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
						memset(frame.pixels, 0, width * height);

						frame.renderX = left;
						frame.renderY = top;

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

							if (frame.valid) {
								int nonZeroPixels = countEmbeddedFramePixels(frame);
								debugC(DEBUG_INSANE, "Frame userId=%d has %d non-zero pixels (%d%%)",
									userId, nonZeroPixels, (nonZeroPixels * 100) / (width * height));
							}

							renderEmbeddedFrame(renderBitmap, frame, userId);

							free(fobjData);
						}
					}

					return;
				} else {
					stream.seek(nextSubPos);
				}
			}
			stream.seek(nextChunkPos);
		} else {
			stream.seek(nextChunkPos);
		}
	}

	debugC(DEBUG_INSANE, "No FOBJ found in embedded SAN userId=%d", userId);
}

void InsaneRebel2::spawnExplosion(int x, int y, int objectHalfWidth, int dx, int dy) {
	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active || _explosions[i].counter <= 0) {
			_explosions[i].active = true;
			_explosions[i].counter = 10;
			_explosions[i].x = x;
			_explosions[i].y = y;
			_explosions[i].scale = objectHalfWidth;
			_explosions[i].dx = dx;
			_explosions[i].dy = dy;
			break;
		}
	}
}

int16 InsaneRebel2::getShotMaxDuration() {
	LevelDifficultyParams params = getDifficultyParams();
	int16 duration = params.laserDelay;
	if (duration <= 0)
		duration = 4;
	return duration;
}

void InsaneRebel2::spawnShot(int x, int y) {
	switch (_rebelHandler) {
	case 0x26:
		spawnTurretShot(x, y);
		break;
	case 8:
		spawnVehicleShot(x, y);
		break;
	case 7:
		spawnSpaceShot(x, y);
		break;
	case 25:
		spawnHandler25Shot(x, y);
		break;
	default:
		break;
	}
}

void InsaneRebel2::spawnTurretShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter == 0) {
			// Ship type 5 (the turbolaser levels 13-15) fires TBLAST, the rest BLAST.
			playSfx((_rebelLevelType == 5) ? 7 : 0, 127, 0);

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

void InsaneRebel2::spawnHandler25Shot(int x, int y) {
	if (_rebelDamageLevel != 0) {
		return;
	}

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter == 0) {
			playSfx(6, 127, 0);

			_turretShots[i].counter = getShotMaxDuration();
			_turretShots[i].seqNum = _turretShotSeqCounter;
			_turretShotSeqCounter++;

			_turretShots[i].targetX = x + _viewX;
			_turretShots[i].targetY = y + _viewY;

			if (_grdShotOriginTableLoaded) {
				int spriteIdx;
				if (_rebelDamageLevel == 0) {
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
	Common::Point projected = getHandler7ProjectedPoint();

	return Common::Point(projected.x + _smoothedVelocity / 2,
	                     projected.y + ABS(_smoothedVelocity) / 4 - _verticalInput / 2 - 0x28);
}

Common::Point InsaneRebel2::getHandler8ShotTargetPoint() {
	return Common::Point(((_shipPosX - 0xa0) >> 3) + 0xa0,
	                     ((_shipPosY - 0x28) >> 2) + 0x69);
}

void InsaneRebel2::spawnSpaceShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_spaceShots[i].counter == 0) {
			playSfx(6, 127, 0);

			_spaceShots[i].counter = getShotMaxDuration();
			// Shots are stored in presentation-window space (the projection without
			// the scroll/crop offset), like the original; spawning happens before the
			// frame's presentation pass, so strip the offset the projection baked in
			// and let renderSpaceLaserShots apply the current frame's one.
			Common::Point projected = getHandler7ProjectedPoint();
			Common::Point target = getHandler7ShotTargetPoint();
			projected.x -= _viewX;
			projected.y -= _viewY;
			target.x -= _viewX;
			target.y -= _viewY;
			int tableIndex = CLIP<int>(_shipDirectionIndex, 0, 34);

			_spaceShots[i].targetX = target.x;
			_spaceShots[i].targetY = target.y;

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

	int totalDist = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
	if (totalDist == 0)
		totalDist = 1;

	int currentDist = 0;

	for (;;) {
		if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
			int u = (currentDist * texW) / totalDist;
			if (u >= texW)
				u = texW - 1;

			byte color = srcData[v * texW + u];

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

void InsaneRebel2::initEdgeTable(const byte *data) {
	if (data == nullptr) {
		for (int a = 0; a < 256; a++) {
			for (int b = a; b < 256; b++) {
				_edgeTable[a + b * 256] = (byte)a;
				_edgeTable[b + a * 256] = (byte)a;
			}
		}
		_edgeTable[0x42 * 256 + 0xf1] = 0x42;
		_edgeTable[0x42 + 0xf0 * 256] = 0x42;
		_edgeTable[0x41 * 256 + 0xb0] = 0x41;
	} else {
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

	if (dx == 0) {
		if (dy == 0) {
			*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
		} else if (dy < 0) {
			while (dy < 1) {
				*pixel = _edgeTable[(uint)pixel[1] + (uint)pixel[-1] * 256];
				pixel -= pitch;
				dy++;
			}
		} else {
			while (dy >= 0) {
				*pixel = _edgeTable[(uint)pixel[1] + (uint)pixel[-1] * 256];
				pixel += pitch;
				dy--;
			}
		}
	} else if (dy == 0) {
		if (dx < 0) {
			while (dx < 1) {
				*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
				pixel--;
				dx++;
			}
		} else {
			while (dx >= 0) {
				*pixel = _edgeTable[(uint)pixel[pitch] + (uint)pixel[-pitch] * 256];
				pixel++;
				dx--;
			}
		}
	} else if (dx < 0 || dy < 0) {
		if (dy < 0) {
			if (dx < 0) {
				if (dx < dy) {
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
				if (-dy < dx) {
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
			if (-dx == dy || -dx < dy) {
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

void InsaneRebel2::drawLaserBeam(byte *dst, int pitch, int width, int height,
                                  int16 gunX, int16 gunY, int16 targetX, int16 targetY,
                                  int16 animFrame, int16 maxFrames,
                                  int16 widthScale, int16 heightScale, int16 thickness) {
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

	int16 startX = (sVar6 + gunX) - (int16)(((int)sVar6 * 16) / (sVar7 + 16));
	int16 startY = (sVar1 + gunY) - (int16)(((int)sVar1 * 16) / (sVar7 + 16));
	int16 endX = (sVar6 + gunX) - (int16)(((int)sVar6 * 16) / (widthScale + sVar7 + 16));
	int16 endY = (sVar1 + gunY) - (int16)(((int)sVar1 * 16) / (widthScale + sVar7 + 16));

	byte *local_28 = texPixels;

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

// Level 3 obstacle zones are quadrilaterals from IACT opcode 5.

void InsaneRebel2::registerCollisionZone(Common::SeekableReadStream &b, int16 subOpcode, int16 par4) {
	int16 field1 = b.readSint16LE();
	int16 field2 = b.readSint16LE();
	int16 x1 = b.readSint16LE();
	int16 y1 = b.readSint16LE();
	int16 x2 = b.readSint16LE();
	int16 y2 = b.readSint16LE();
	int16 x3 = b.readSint16LE();
	int16 y3 = b.readSint16LE();
	int16 x4 = b.readSint16LE();
	int16 y4 = b.readSint16LE();

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
	_primaryZoneCount = 0;
	_secondaryZoneCount = 0;
}

void InsaneRebel2::checkCollisionZones(byte *renderBitmap, int pitch, int width, int height, int32 curFrame) {
	if (_primaryZoneCount == 0)
		return;

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

		if (zone.filterValue >= 1000)
			continue;

		int cx1 = zone.x1 - 0xD4;
		int cy1 = zone.y1 - 0x82;
		int cx2 = zone.x2 - 0xD4;
		int cy2 = zone.y2 - 0x82;
		int cx3 = zone.x3 - 0xD4;
		int cy3 = zone.y3 - 0x82;
		int cx4 = zone.x4 - 0xD4;
		int cy4 = zone.y4 - 0x82;

		if (zone.field2 - 1 == zone.field1) {
			bool collision = false;

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
				_rebelDeathCause = 2;
				int collisionDamage = (dparams.dodgeDamage >= 0) ? dparams.dodgeDamage : 0;

				if (applyPlayerDamage(collisionDamage)) {
					debugC(DEBUG_INSANE, "COLLISION damage! zone=%d aim=(%d,%d) damage=%d total=%d",
						i, aimX, aimY, collisionDamage, _playerDamage);
				}
				if (!_noDamage)
					initDamageFlash();
				const int hitAvgX = (cx1 + cx2 + cx3 + cx4) / 4;
				const int hitAvgY = (cy1 + cy2 + cy3 + cy4) / 4;
				playSfx(1, 127, CLIP(hitAvgX * -4, -127, 127));
				// The amplified impulse enters the shake ring and decays over the next 14 frames.
				_turretShakeRingX[14] = (int16)(CLIP(hitAvgX, -0x2b, 0x2b) * 8);
				_turretShakeRingY[14] = (int16)(CLIP(hitAvgY, -0x19, 0x19) * 8);
			} else {
				if (dparams.dodgePoints > 0) {
					addScore(dparams.dodgePoints);
				}
			}
		} else if (warningFrame && zone.field2 - 0x0c < zone.field1) {
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

bool InsaneRebel2::isHandler7ShipInsideObstacleZone(const InsaneRebel2::CollisionZone &zone, int margin) {
	int x1 = zone.x1, y1 = zone.y1;
	int x2 = zone.x2, y2 = zone.y2;
	int x3 = zone.x3, y3 = zone.y3;
	int x4 = zone.x4, y4 = zone.y4;

	bool inside = true;

	if (x2 != x1) {
		int interpY = (_flyShipScreenX - x1) * (y2 - y1) / (x2 - x1) + margin + y1;
		if (_flyShipScreenY < interpY)
			inside = false;
	}
	if (inside && x3 != x4) {
		int interpY = (_flyShipScreenX - x4) * (y3 - y4) / (x3 - x4) + y4 - margin;
		if (interpY < _flyShipScreenY)
			inside = false;
	}
	if (inside && y4 != y1) {
		int interpX = (_flyShipScreenY - y1) * (x4 - x1) / (y4 - y1) + margin + x1;
		if (_flyShipScreenX < interpX)
			inside = false;
	}
	if (inside && y3 != y2) {
		int interpX = (_flyShipScreenY - y2) * (x3 - x2) / (y3 - y2) + x2 - margin;
		if (interpX < _flyShipScreenX)
			inside = false;
	}

	return inside;
}

void InsaneRebel2::applyHandler7ObstacleHit(const InsaneRebel2::CollisionZone &zone, int zoneIndex) {
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
	LevelDifficultyParams scoreParams = getDifficultyParams();
	if (scoreParams.dodgePoints > 0) {
		addScore(scoreParams.dodgePoints);
	}
}

void InsaneRebel2::checkHandler7ObstacleZones(uint16 &warningMask) {
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
			const bool damageApplied = applyHandler7WallDamage(wallDamage);
			if (damageApplied) {
				debugC(DEBUG_INSANE, "Handler7 Mode1/3 TOP WALL ship=(%d,%d) edgeY=%d damage=%d",
					_flyShipScreenX, _flyShipScreenY, edgeY, wallDamage);
			}
			_spaceShotDirection = 2;
			_flyShipScreenY = edgeY;
			playSfx(1, 127, 0);
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
			const bool damageApplied = applyHandler7WallDamage(wallDamage);
			if (damageApplied) {
				debugC(DEBUG_INSANE, "Handler7 Mode1/3 BOTTOM WALL ship=(%d,%d) edgeY=%d damage=%d",
					_flyShipScreenX, _flyShipScreenY, edgeY, wallDamage);
			}
			_spaceShotDirection = 3;
			_flyShipScreenY = edgeY;
			playSfx(1, 127, 0);
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
			_flyShipScreenX = edgeX;

			resetHandler7HorizontalVelocity(127);

			const bool damageApplied = applyHandler7WallDamage(wallDamage);
			if (damageApplied) {
				debugC(DEBUG_INSANE, "Handler7 Mode1/3 LEFT WALL ship=(%d,%d) edgeX=%d damage=%d",
					_flyShipScreenX, _flyShipScreenY, edgeX, wallDamage);
			}
			_spaceShotDirection = 0;
			playSfx(1, 127, -100);
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
			_flyShipScreenX = edgeX;

			resetHandler7HorizontalVelocity(-127);

			const bool damageApplied = applyHandler7WallDamage(wallDamage);
			if (damageApplied) {
				debugC(DEBUG_INSANE, "Handler7 Mode1/3 RIGHT WALL ship=(%d,%d) edgeX=%d damage=%d",
					_flyShipScreenX, _flyShipScreenY, edgeX, wallDamage);
			}
			_spaceShotDirection = 1;
			playSfx(1, 127, 100);
			if (!_noDamage)
				initDamageFlash();
		}
	}
}

void InsaneRebel2::checkHandler7BoundaryZones(uint16 &warningMask) {
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
	uint16 warningMask = 0;

	if (_flyControlMode == 0 || _flyControlMode == 2) {
		checkHandler7ObstacleZones(warningMask);
	} else {
		checkHandler7BoundaryZones(warningMask);
	}

	renderHandler7WarningCues(renderBitmap, pitch, width, height, curFrame, warningMask);
}

void InsaneRebel2::renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx) {
	renderNutSpriteMirrored(dst, pitch, width, height, x, y, nut, spriteIdx, false);
}

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

	for (int iy = 0; iy < drawH; iy++) {
		const byte *s = src + (srcOffsetY + iy) * w;
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			int srcX;
			if (mirror) {
				srcX = (w - 1) - (srcOffsetX + ix);
			} else {
				srcX = srcOffsetX + ix;
			}
			byte px = s[srcX];
			if (px != 231 && px != 0) {
				d[ix] = px;
			}
		}
	}
}

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

	if (_rebelHandler == 0x26) {
		// Dodge-fail view shake: the hit impulse travels a 15-slot weighted ring
		// and nudges the scroll, with a +-5 jitter while the damage flash runs.
		static const int16 kShakeWeights[14] = { 1, 1, 2, 2, 3, 3, 4, 8, 14, 16, 16, 14, 12, 6 };
		int shakeX = 0, shakeY = 0, weightSum = 2;
		for (int i = 0; i < 14; i++) {
			_turretShakeRingX[i] = _turretShakeRingX[i + 1];
			_turretShakeRingY[i] = _turretShakeRingY[i + 1];
			shakeX += _turretShakeRingX[i] * kShakeWeights[i];
			shakeY += _turretShakeRingY[i] * kShakeWeights[i];
			weightSum += kShakeWeights[i];
		}
		_turretShakeRingX[14] = 0;
		_turretShakeRingY[14] = 0;
		shakeX /= weightSum;
		shakeY /= weightSum;
		if (_damageFlashCounter > 0) {
			shakeX += _vm->_rnd.getRandomNumber(9) - 5;
			shakeY += _vm->_rnd.getRandomNumber(9) - 5;
		}
		_viewX = CLIP<int>(_viewX + shakeX, 0, maxScrollX);
		_viewY = CLIP<int>(_viewY + shakeY, 0, maxScrollY);
	}

	_player->setScrollOffset(_viewX, _viewY);
}

void InsaneRebel2::updatePostRenderDeath() {
	if (_rebelHandler != 0) {
		_playerShield = 255 - _playerDamage;
		if (_playerShield <= 0) {
			_playerShield = 0;
			_vm->_smushVideoShouldFinish = true;
		}
	}
}

void InsaneRebel2::updateLevel7Fork(int32 curFrame) {
	// Level 7 (TIE-fighter flight) forks at frame 0x638: if the ship is on the
	// right half of the corridor (_flyShipScreenX past the 0xd4 center, matching
	// the original's DAT_0047ab8c test), stop 07PLAY here so the alternate
	// 07PLAYB segment can splice in. The left half plays 07PLAY through to its
	// end. Sampled exactly once (the active flag is its own one-shot guard).
	if (!_level7ForkActive || curFrame < 0x638)
		return;

	_level7ForkActive = false;
	if (_flyShipScreenX + _smoothedVelocity > 0xd4) {
		_level7TookRightFork = true;
		_vm->_smushVideoShouldFinish = true;
	}
}

void InsaneRebel2::updateLevel15TypeSwitch(int32 curFrame) {
	// From frame 0x21e level 15 uses the type-0x10 difficulty column; runLevel15 resets per attempt.
	// Tracked as a flag because the turret opcode-6 rewrites _rebelLevelType every frame.
	if (_selectedLevel == 15 && curFrame >= 0x21e)
		_level15SecondHalf = true;
}

void InsaneRebel2::updateLevel9WaveReset(int32 curFrame) {
	// 09PLAY re-arms its gauge groups per wave; the counters clear between waves.
	if (_selectedLevel == 9 && (curFrame == 0x19f || curFrame == 0x352))
		resetGaugeCounters();
}

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

bool InsaneRebel2::handlePostRenderMenuModes(byte *renderBitmap, int pitch, int width, int height, bool introPlaying) {
	bool menuMode = (introPlaying && _gameState == kStateMainMenu);
	bool pilotSelectMode = (introPlaying && (_gameState == kStatePilotSelect || _gameState == kStateDifficultySelect));
	bool chapterSelectMode = (introPlaying && _gameState == kStateChapterSelect);

	if (pilotSelectMode) {
		int selection = processLevelSelectInput();

		drawLevelSelectOverlay(renderBitmap, pitch, width, height);
		renderPostRenderMenuCursor(renderBitmap, pitch, width, height);

		if (selection >= 0) {
			debugC(DEBUG_INSANE, "Pilot selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

		return true;
	}

	if (chapterSelectMode) {
		int selection = processChapterSelectInput();

		drawChapterSelectOverlay(renderBitmap, pitch, width, height);
		renderPostRenderMenuCursor(renderBitmap, pitch, width, height);

		if (selection >= 0) {
			debugC(DEBUG_INSANE, "Chapter selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

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
		int selection = processMenuInput();

		_menuInactivityTimer++;

		if (_menuInactivityTimer > 300) {
			debugC(DEBUG_INSANE, "Menu inactivity timeout - resuming intro/demo loop");
			_menuInactivityTimer = 0;
			_menuInactivityTimedOut = true;
			_menuSelectionConfirmed = false;
			_vm->_smushVideoShouldFinish = true;
		}

		drawMenuOverlay(renderBitmap, pitch, width, height);
		renderPostRenderMenuCursor(renderBitmap, pitch, width, height);

		if (selection >= 0) {
			debugC(DEBUG_INSANE, "Menu selection confirmed: %d", selection);
			_menuSelectionConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
		}

		return true;
	}

	return false;
}

bool InsaneRebel2::handlePostRenderIntro(byte *renderBitmap, int pitch, int width, int height, int32 curFrame, int32 maxFrame) {
	if (_rebelHandler == 0) {
		CursorMan.showMouse(false);

		if (!_introCursorPushed) {
			_introCursorPushed = true;
			debugC(DEBUG_INSANE, "Intro/cinematic mode (handler=0, flags=0x%x, state=%d) - HUD disabled, mouse hidden",
				  _player->_curVideoFlags, _gameState);
		}

		if (_textOverlayActive)
			renderTextOverlay(renderBitmap, pitch, width, height, curFrame);
		if (_levelEndStats.active)
			renderLevelEndStatsOverlay(renderBitmap, pitch, width, height, curFrame, maxFrame);

		return true;
	}

	if (_introCursorPushed) {
		_introCursorPushed = false;
		debugC(DEBUG_INSANE, "Gameplay mode (handler=%d, flags=0x%x, state=%d) - HUD enabled",
			  _rebelHandler, _player->_curVideoFlags, _gameState);
	}

	return false;
}

void InsaneRebel2::updateGameplayDamageEffects(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler == 8) {
		// Suppressed during autopilot (mode 4) and cutscene (mode 5).
		if (_shipLevelMode != 4 && _shipLevelMode != 5) {
			updateDamageEffect(renderBitmap, pitch, width, height);
		}
	} else if (_rebelHandler == 0x19 || _rebelHandler == 0x26 || _rebelHandler == 7) {
		updateDamageFlashPalette();
	}
}

void InsaneRebel2::updateGameplayTimedTick(int32 curFrame) {
	// Every 16th frame the handlers score timePoints and recover one point of
	// damage; the cover handler (25) scores without recovering.
	if ((curFrame & 0xf) != 0)
		return;
	if (_rebelHandler != 7 && _rebelHandler != 8 && _rebelHandler != 25 && _rebelHandler != 0x26)
		return;

	const int16 timePoints = getDifficultyParams().timePoints;
	if (timePoints > 0)
		addScore(timePoints);

	if (_rebelHandler != 25 && _playerDamage > 0) {
		_playerDamage--;
		_playerShield = 255 - _playerDamage;
	}
}

void InsaneRebel2::checkGameplayPostRenderCollisions(byte *renderBitmap, int pitch, int width, int height, int32 curFrame) {
	if (_rebelHandler == 0x26) {
		checkCollisionZones(renderBitmap, pitch, width, height, curFrame);
	} else if (_rebelHandler == 7) {
		checkHandler7CollisionZones(renderBitmap, pitch, width, height, curFrame);
	}
}

void InsaneRebel2::prepareHandler7Viewport(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 7 || _rebelDetailMode < 1 || isHiRes() || !renderBitmap || pitch <= 0 || width <= 0 || height <= 0)
		return;

	copyRA2Handler7PerspectiveViewport(renderBitmap, pitch, width, height,
		renderBitmap, pitch, MIN(width, pitch), height, _perspectiveX, _perspectiveY, _viewShift);
	_viewX = 0;
	_viewY = 0;
	_player->setScrollOffset(0, 0);
}

void InsaneRebel2::renderGameplayPostFrame(byte *renderBitmap, int pitch, int width, int height,
										   int videoWidth, int videoHeight, int statusBarY, int32 curFrame) {
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
		bool promoted = false;
		if (ra2Player) {
			if (_rebelHandler == 7 && _rebelDetailMode >= 1) {
				promoted = ra2Player->ra2PromoteHandler7PerspectiveToHiRes(_perspectiveX, _perspectiveY, _viewShift);
				nativeViewX = 0;
				nativeViewY = 0;
			} else {
				promoted = ra2Player->ra2PromoteCurrentFrameToHiRes(_viewX, _viewY);
			}
		}
		if (promoted) {
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
	prepareHandler7Viewport(renderBitmap, pitch, width, height);

	renderStatusBarBackground(renderBitmap, pitch, width, height, videoWidth, videoHeight, gameplayStatusBarY);

	debugC(DEBUG_INSANE, "Ship Check: handler=%d shipSprite=%p flyShipSprite=%p shipLevelMode=%d numSprites=%d/%d",
		_rebelHandler, (void*)_shipSprite, (void*)_flyShipSprite, _shipLevelMode,
		_shipSprite ? _shipSprite->getNumChars() : 0,
		_flyShipSprite ? _flyShipSprite->getNumChars() : 0);

	renderVehicleShotImpacts(renderBitmap, pitch, width, height);
	renderHandler8Ship(renderBitmap, pitch, width, height);
	renderFallbackShip(renderBitmap, pitch, width, height);

	renderEnemyOverlays(renderBitmap, pitch, width, height, videoWidth);

	renderExplosions(renderBitmap, pitch, width, height);

	renderLaserShots(renderBitmap, pitch, width, height);

	renderHandler7Ship(renderBitmap, pitch, width, height);

	renderHandler25ShipPre(renderBitmap, pitch, width, height);
	renderHandler25Ship(renderBitmap, pitch, width, height);

	renderTurretHudOverlays(renderBitmap, pitch, width, height, curFrame);

	renderEmbeddedHudOverlays(renderBitmap, pitch, width, height);

	renderStatusBarSprites(renderBitmap, pitch, width, height, gameplayStatusBarY, curFrame);

	updateGameplayDamageEffects(renderBitmap, pitch, width, height);
	checkGameplayPostRenderCollisions(renderBitmap, pitch, width, height, curFrame);
	updateGameplayTimedTick(curFrame);

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

void InsaneRebel2::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) {

	int width = _player->_width;
	int height = _player->_height;
	if (width == 0)
		width = _vm->_screenWidth;
	if (height == 0)
		height = _vm->_screenHeight;
	int pitch = width;

	updatePostRenderScroll(width, height);
	updatePostRenderDeath();
	updateLevel7Fork(curFrame);
	updateLevel15TypeSwitch(curFrame);
	updateLevel9WaveReset(curFrame);

	// End the looping attack-run segment once the shield/reactor is destroyed.
	if (_rebelShieldGateActive) {
		// Level 13: the finale (continuation segment, flag 0x40) ends when the last armed
		// group (the reactor) is depleted; the approach segment plays fully.
		if (_rebelReactorMode && _rebelGaugeArmed && _rebelLastArmedSlot >= 0 &&
		    (_player->_curVideoFlags & 0x40) != 0) {
			const int slot = _rebelLastArmedSlot;
			const short remaining = _rebelValueCounters[slot];
			if (remaining <= 0)
				_rebelShieldDestroyed = true;
		}
		if (_rebelShieldDestroyed)
			_vm->_smushVideoShouldFinish = true;
	}

	const int hudScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);
	const int videoWidth = 320 * hudScale;
	const int videoHeight = 200 * hudScale;
	const int statusBarY = 180 * hudScale;    // 0xb4 low-res, 0x168 high-res

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

	if (handlePostRenderIntro(renderBitmap, pitch, width, height, curFrame, maxFrame))
		return;
	renderGameplayPostFrame(renderBitmap, pitch, width, height, videoWidth, videoHeight, statusBarY, curFrame);
}

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

	if (_damageFlashCounter == 0 && _player) {
		memcpy(_damageSavedPalette, _player->_pal, 0x300);
	}
	_damageFlashCounter = 5;
}

void InsaneRebel2::triggerDamageEffect() {
	initDamageFlash();
	_damageShakeCounter = 10;
}

void InsaneRebel2::updateDamageFlashPalette() {
	if (_playerDamage < 0xFF) {
		_damageHighFlashCounter = 0;
	} else {
		if (_damageHighFlashCounter == 0) {
			if (!_damageRestorePaletteValid) {
				memcpy(_damageRestorePalette, _player->_pal, 0x300);
				_damageRestorePaletteValid = true;
			}
			memcpy(_damageSavedPalette, _player->_pal, 0x300);
		}
		if (_damageHighFlashCounter < 0x10) {
			_damageHighFlashCounter++;
		}
	}

	if (_damageHighFlashCounter == 0 || (_damageHighFlashCounter & 1) != 0) {
		if (_damageFlashCounter != 0) {
			_damageFlashCounter--;
			if ((_damageFlashCounter & 1) == 0) {
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
		byte modPal[0x300];
		memcpy(modPal, _player->_pal, 0x300);
		int blend = 0x10 - _damageHighFlashCounter;
		for (int i = 0; i < 0x300; i += 3) {
			modPal[i] = 0xFF - (((0xFF - _damageSavedPalette[i]) * blend) >> 4);
		}
		_player->setPalette(modPal);
	}
}

void InsaneRebel2::updateDamageEffect(byte *renderBitmap, int pitch, int width, int height) {
	if (_damageShakeCounter != 0) {
		_damageShakeCounter--;
		int numLines = _damageShakeCounter * 5;

		byte tempLine[640];
		const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
		const int maxY = MIN(height, renderHiRes ? 360 : 180);

		for (int n = numLines; n > 0; n--) {
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
				memmove(linePtr, linePtr + offset, copyLen);
				break;
			case 1:
				memcpy(tempLine, linePtr, MIN(copyLen, (int)sizeof(tempLine)));
				memmove(linePtr + offset, tempLine, MIN(copyLen, (int)sizeof(tempLine)));
				break;
			case 2:
			case 4:
				memmove(linePtr + offset, linePtr, copyLen);
				break;
			}
		}
	}

	updateDamageFlashPalette();
}

void InsaneRebel2::drawLevelEndTextBlock(byte *renderBitmap, const char *text, int centerX, int y) {
	if (!renderBitmap || !text)
		return;

	const int lineStep = isHiRes() ? 20 : 10;
	Common::String line;
	const char *s = text;
	while (true) {
		if (*s == '\0' || *s == '\n' || *s == '\r') {
			if (!line.empty())
				drawMenuStringCentered(renderBitmap, line.c_str(), centerX, y);
			y += lineStep;
			line.clear();
			if (*s == '\0')
				break;
			if (*s == '\r' && s[1] == '\n')
				s++;
			s++;
			continue;
		}

		line += *s++;
	}
}

void InsaneRebel2::renderLevelEndStatsOverlay(byte *renderBitmap, int pitch, int width, int height,
		int32 curFrame, int32 maxFrame) {
	(void)pitch;
	(void)width;
	(void)height;

	if (!_levelEndStats.active)
		return;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer)
		return;

	int32 titleStart = 0;
	int32 titleEnd = 0x7fffffff;
	if (maxFrame > 0) {
		titleStart = maxFrame - _levelEndStats.titleStartBeforeEnd;
		titleEnd = maxFrame - _levelEndStats.titleEndBeforeEnd;
	}

	if (curFrame < titleStart || curFrame >= titleEnd)
		return;

	const int scale = isHiRes() ? 2 : 1;
	const int centerX = _levelEndStats.textX * scale;
	const int titleY = _levelEndStats.textY * scale;

	const char *titleFmt = splayer->getString(190);
	if (!titleFmt || !titleFmt[0])
		titleFmt = getRebel2LevelEndFallbackString(190);

	Common::String password = getChapterPassword(_levelEndStats.levelId, _difficulty);
	Common::String titleText = Common::String::format(titleFmt, password.c_str());
	int visibleChars = curFrame + 10 - titleStart;
	if (visibleChars > 0xbe)
		visibleChars = 0xbe;
	Common::String visibleTitle = getRebel2VisibleTextPrefix(titleText.c_str(), visibleChars);
	drawLevelEndTextBlock(renderBitmap, visibleTitle.c_str(), centerX, titleY);

	if (curFrame <= titleEnd - 0x32)
		return;

	int displayBonus = _levelEndStats.bonus;
	for (int32 frame = titleEnd - 0x23 + 1; frame <= curFrame && displayBonus > 0; frame++) {
		const int reduced = displayBonus - 0xaf;
		const int halved = displayBonus >> 1;
		displayBonus = (reduced <= halved) ? halved : reduced;
	}

	const int displayScore = _levelEndStats.finalScore - displayBonus;
	const int rank = (curFrame >= titleEnd - 0x19) ? _levelEndStats.newRating : _levelEndStats.oldRating;
	Common::String rankStr = getRankString(rank);

	int stringId;
	if (_levelEndStats.hasAccuracy && _levelEndStats.hasFlightErrors)
		stringId = _levelEndStats.skillBonus ? 196 : 193;
	else if (_levelEndStats.hasAccuracy)
		stringId = _levelEndStats.skillBonus ? 194 : 191;
	else if (_levelEndStats.hasFlightErrors)
		stringId = _levelEndStats.skillBonus ? 195 : 192;
	else
		return;

	const char *statsFmt = splayer->getString(stringId);
	if (!statsFmt || !statsFmt[0])
		statsFmt = getRebel2LevelEndFallbackString(stringId);

	Common::String statsText;
	if (_levelEndStats.hasAccuracy && _levelEndStats.hasFlightErrors) {
		statsText = Common::String::format(statsFmt,
			(short)_levelEndStats.flightErrors, (short)_levelEndStats.accuracy,
			(long)displayBonus, (long)displayScore, rankStr.c_str());
	} else if (_levelEndStats.hasAccuracy) {
		statsText = Common::String::format(statsFmt,
			(short)_levelEndStats.accuracy, (long)displayBonus,
			(long)displayScore, rankStr.c_str());
	} else {
		statsText = Common::String::format(statsFmt,
			(short)_levelEndStats.flightErrors, (long)displayBonus,
			(long)displayScore, rankStr.c_str());
	}

	drawLevelEndTextBlock(renderBitmap, statsText.c_str(), centerX, titleY + 0x21 * scale);
}

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

	int displayLen = curFrame + 10 - _textOverlayFadeIn;
	if (displayLen > 0xBE)
		displayLen = 0xBE;
	if (displayLen < 0)
		return;

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
	if (_rebelHandler != 0x26 || !_hudOverlayNut || _hudOverlayNut->getNumChars() <= 0)
		return;

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

	hudX += _viewX;
	hudY += _viewY;

	renderNutSprite(renderBitmap, pitch, width, height,
		hudX + baseSpriteXOff, hudY + baseSpriteYOff, _hudOverlayNut, 0);

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

void InsaneRebel2::renderEmbeddedHudOverlays(byte *renderBitmap, int pitch, int width, int height) {
	for (int hudSlot = 1; hudSlot < 16; hudSlot++) {
		EmbeddedSanFrame &frame = _rebelEmbeddedHud[hudSlot];
		if (!isValidEmbeddedFrame(frame))
			continue;

		// Handler 25 slot 4 is a corridor overlay that would cover enemies here.
		if (_rebelHandler == 25 && hudSlot == 4) {
			continue;
		}

		if (frame.renderX == 0 && frame.renderY == 0 && frame.width < 50 && frame.height < 60) {
			debugC(DEBUG_INSANE, "Skipping small embedded frame at (0,0): slot=%d size=%dx%d",
				hudSlot, frame.width, frame.height);
			continue;
		}

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

		int destX = frame.renderX;
		int destY = frame.renderY;

		const int hudScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);

		if (_rebelHandler == 0x26 && hudSlot >= 1 && hudSlot <= 4) {
			destX = 160 * hudScale - frame.width / 2 - frame.renderX;
			destY = 200 * hudScale - frame.height - frame.renderY;
		}

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
	if (!_smush_cockpitNut)
		return;

	int numSprites = _smush_cockpitNut->getNumChars();
	const int statusScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);

	// Critical-damage warning beep every 25 gameplay frames; volume depends on the difficulty row.
	if (_rebelHandler != 0 && _playerDamage > 170 && (curFrame % 25) == 0) {
		int alertVolume;
		switch (getDifficultyRow()) {
		case 1: case 11: case 12:
			alertVolume = 40;
			break;
		case 2: case 3: case 5: case 6: case 7: case 9: case 10: case 13:
			alertVolume = 63;
			break;
		default:
			alertVolume = 127;
			break;
		}
		playSfx(3, alertVolume, 0);
	}

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

void InsaneRebel2::renderHandler7FlySprite(byte *renderBitmap, int pitch, int width, int height,
		bool renderHiRes, int renderScale, int nativeViewX, int nativeViewY,
		int nativeX, int nativeY, NutRenderer *nut, int spriteIndex) {
	int dstX = renderHiRes ? (nativeX - nativeViewX) * renderScale : nativeX;
	int dstY = renderHiRes ? (nativeY - nativeViewY) * renderScale : nativeY;

	if (renderHiRes) {
		const int clipBottom = MIN(height, _gameplayPresentationClipBottom + 1);
		if (drawRebel2Codec23Sprite(nut, renderBitmap, pitch, width, height,
				Common::Rect(0, 0, width, clipBottom), dstX, dstY, spriteIndex, renderScale))
			return;

		if (drawRebel2Codec45Sprite(nut, renderBitmap, pitch, width, height,
				Common::Rect(0, 0, width, clipBottom), dstX, dstY, spriteIndex, renderScale))
			return;

		renderNutSpriteScaledClipped(renderBitmap, pitch, width, height,
			0, 0, width, clipBottom, dstX, dstY, nut, spriteIndex, false, renderScale, true);
	} else {
		Common::Rect clipRect(0, 0, width, height);
		if (_rebelHandler == 7) {
			clipRect = Common::Rect(_viewX, _viewY,
				MIN(width, _viewX + 320),
				MIN(height, _viewY + 170));
		}
		if (drawRebel2Codec23Sprite(nut, renderBitmap, pitch, width, height,
				clipRect, dstX, dstY, spriteIndex, renderScale))
			return;

		if (drawRebel2Codec45Sprite(nut, renderBitmap, pitch, width, height,
				clipRect, dstX, dstY, spriteIndex, renderScale))
			return;

		if (_rebelHandler == 7) {
			renderNutSpriteClipped(renderBitmap, pitch, height,
				clipRect.left, clipRect.top, clipRect.right, clipRect.bottom,
				dstX, dstY, nut, spriteIndex);
		} else {
			renderNutSprite(renderBitmap, pitch, width, height, dstX, dstY, nut, spriteIndex);
		}
	}
}

static Common::Point getHandler7SpriteDrawPoint(const Common::Point &base, NutRenderer *nut, int spriteIndex) {
	Common::Point point = base;
	if (nut && spriteIndex >= 0 && spriteIndex < nut->getNumChars()) {
		point.x += nut->getCharXOffset(spriteIndex);
		point.y += nut->getCharYOffset(spriteIndex);
	}
	return point;
}

void InsaneRebel2::renderHandler7Ship(byte *renderBitmap, int pitch, int width, int height) {
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
	if (renderHiRes) {
		projected.x += nativeViewX;
		projected.y += nativeViewY;
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

	Common::Point shipBase(projected.x - 0xd4, projected.y - 0x82);
	Common::Point shipDraw = getHandler7SpriteDrawPoint(shipBase, _flyShipSprite, spriteIndex);

	int drawX = renderHiRes ? (shipDraw.x - nativeViewX) * renderScale : shipDraw.x;
	int drawY = renderHiRes ? (shipDraw.y - nativeViewY) * renderScale : shipDraw.y;

	renderHandler7FlySprite(renderBitmap, pitch, width, height,
		renderHiRes, renderScale, nativeViewX, nativeViewY,
		shipDraw.x, shipDraw.y, _flyShipSprite, spriteIndex);

	if (_flyLaserSprite && _flyOverlayRepeatCount > 0) {
		int overlayIndex = spriteIndex + 0x14;
		if (overlayIndex >= 0 && overlayIndex < _flyLaserSprite->getNumChars()) {
			Common::Point overlayDraw = getHandler7SpriteDrawPoint(shipBase, _flyLaserSprite, overlayIndex);
			for (int i = 0; i < _flyOverlayRepeatCount; i++) {
				renderHandler7FlySprite(renderBitmap, pitch, width, height,
					renderHiRes, renderScale, nativeViewX, nativeViewY,
					overlayDraw.x, overlayDraw.y, _flyLaserSprite, overlayIndex);
			}
		}
	}

	if (_flyTargetSprite && _rebelDetailMode >= 0 &&
		spriteIndex >= 0 && spriteIndex < _flyTargetSprite->getNumChars()) {
		Common::Point targetDraw = getHandler7SpriteDrawPoint(shipBase, _flyTargetSprite, spriteIndex);
		renderHandler7FlySprite(renderBitmap, pitch, width, height,
			renderHiRes, renderScale, nativeViewX, nativeViewY,
			targetDraw.x, targetDraw.y, _flyTargetSprite, spriteIndex);
	}

	debugC(DEBUG_INSANE, "Handler7Ship: draw=(%d,%d) sprite=%d/%d shipPos=(%d,%d) view=(%d,%d) persp=(%d,%d) smoothVel=%d vertIn=%d fxCtr=%d fxRep=%d",
		drawX, drawY, spriteIndex, numSprites, _flyShipScreenX, _flyShipScreenY,
		renderHiRes ? nativeViewX : _viewX, renderHiRes ? nativeViewY : _viewY,
		_perspectiveX, _perspectiveY, _smoothedVelocity, _verticalInput, _flyEffectAnimCounter, _flyOverlayRepeatCount);
}

void InsaneRebel2::renderHandler8Ship(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 8 || !_shipSprite || _shipLevelMode == 5)
		return;

	int16 displayOffsetX = (_shipPosX - 0xa0) >> 3;
	int16 displayOffsetY = (_shipPosY - 0x28) >> 2;

	int numSprites = _shipSprite->getNumChars();
	int spriteIndex = _shipFiring ? 1 : 0;
	if (spriteIndex >= numSprites)
		spriteIndex = 0;

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

void InsaneRebel2::renderHandler25ShipPre(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 25)
		return;

	if (!_grd001Sprite || _grd001Sprite->getNumChars() <= 0)
		return;

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
		useHalfWidth = (_rebelDamageLevel == 0);
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

		int nativeDrawX = _rebelViewOffset2X + spriteXOffset + nativeBufferViewX;
		int nativeDrawY = _rebelViewOffset2Y + spriteYOffset + nativeBufferViewY;
		int drawX = renderHiRes ? (nativeDrawX - nativeViewX) * renderScale : nativeDrawX;
		int drawY = renderHiRes ? (nativeDrawY - nativeViewY) * renderScale : nativeDrawY;

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
	if (_rebelHandler != 25)
		return;

	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int renderScale = renderHiRes ? 2 : 1;
	const int nativeViewX = renderHiRes ? _hiResPresentationViewX : 0;
	const int nativeViewY = renderHiRes ? _hiResPresentationViewY : 0;
	const int nativeBufferViewX = renderHiRes ? nativeViewX : _viewX;
	const int nativeBufferViewY = renderHiRes ? nativeViewY : _viewY;
	int renderHeight = renderHiRes ? MIN(height, 180 * renderScale) : MIN(height, 180 + _viewY);

	if (_grd002Sprite && _grd002Sprite->getNumChars() > 0) {
		int spriteIdx;
		int numSprites = _grd002Sprite->getNumChars();

		bool shouldMirror = (_rebelFlightDir != 0) && (_rebelDamageLevel == 0);

		if (_rebelDamageLevel == 0) {
			int16 areaLeft = (_corridorLeftX > 0) ? _corridorLeftX : 0;
			int16 areaRight = (_corridorRightX > 0) ? _corridorRightX : 320;
			int16 areaTop = (_corridorTopY > 0) ? _corridorTopY : 0;
			int16 areaBottom = (_corridorBottomY > 0) ? _corridorBottomY : 180;

			Common::Point aimPos = getGameplayAimPoint();
			int16 crosshairX = aimPos.x;
			int16 crosshairY = aimPos.y;

			int areaWidth = areaRight - areaLeft;
			int areaHeight = areaBottom - areaTop;
			int zoneWidth = (areaWidth > 0) ? (areaWidth + 3) / 4 : 80;
			int zoneHeight = (areaHeight > 0) ? areaHeight / 2 : 90;

			int xZone = (zoneWidth > 0) ? ((zoneWidth / 2) + (crosshairX - areaLeft)) / zoneWidth : 2;
			int yZone = (zoneHeight > 0) ? ((zoneHeight / 2) + (crosshairY - areaTop)) / zoneHeight : 0;

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
			if (_rebelFlightDir == 0) {
				spriteIdx = 5 - _rebelDamageLevel;
			} else {
				spriteIdx = 25 - _rebelDamageLevel;
			}
		}

		if (spriteIdx < 0)
			spriteIdx = 0;
		if (spriteIdx >= numSprites)
			spriteIdx = numSprites - 1;

		int spriteW = _grd002Sprite->getCharWidth(spriteIdx);
		int spriteH = _grd002Sprite->getCharHeight(spriteIdx);

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

void InsaneRebel2::renderFallbackShip(byte *renderBitmap, int pitch, int width, int height) {
	if ((_rebelHandler != 7 && _rebelHandler != 8) || _shipLevelMode == 5)
		return;

	if (_rebelHandler == 7 && _flyShipSprite)
		return;
	if (_rebelHandler == 8 && _shipSprite)
		return;

	EmbeddedSanFrame &shipFrame = _rebelEmbeddedHud[11];
	if (!isValidEmbeddedFrame(shipFrame))
		return;

	int16 displayOffsetX = (_shipPosX - 0xa0) >> 3;
	int16 displayOffsetY = (_shipPosY - 0x28) >> 2;
	int shipScreenX = 0xa0 + displayOffsetX;
	int shipScreenY = 0x69 + displayOffsetY;

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
	if (_rebelHandler != 0x26 || !_smush_iconsNut)
		return;

	LevelDifficultyParams dparams = getDifficultyParams();
	if ((dparams.flags & 4) != 0)
		return;

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

void InsaneRebel2::renderExplosions(byte *renderBitmap, int pitch, int width, int height) {
	LevelDifficultyParams dparams = getDifficultyParams();
	bool suppressExplosionSprites = (dparams.flags & 1) != 0;

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

		int screenX = _explosions[i].x;
		int screenY = _explosions[i].y;
		renderExplosionFrame(renderBitmap, pitch, width, height, _explosions[i],
			screenX, screenY, kExplosionAdvanceAfterDraw, false);
	}
}

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

	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		// Explosions live in raw flight-buffer coordinates: keep them moving
		// with the victim's last motion and project into the presented window
		// with the current perspective every frame.
		_explosions[i].x += _explosions[i].dx;
		_explosions[i].y += _explosions[i].dy;
		Common::Point p = getHandler7ProjectedPointFor(_explosions[i].x, _explosions[i].y);
		if (renderHiRes) {
			p.x += _hiResPresentationViewX;
			p.y += _hiResPresentationViewY;
		}
		renderExplosionFrame(renderBitmap, pitch, width, height, _explosions[i],
			p.x, p.y, kExplosionAdvanceBeforeDraw, true);
	}

	if (_hitCooldown != 0) {
		_hitCooldown--;

		int numChars = _smush_iconsNut->getNumChars();
		int spriteIndex = 0x15 - _hitCooldown;
		const int nativeViewX = renderHiRes ? _hiResPresentationViewX : _viewX;
		const int nativeViewY = renderHiRes ? _hiResPresentationViewY : _viewY;

		if (spriteIndex >= 0 && spriteIndex < numChars) {
			Common::Point shipProjected = getHandler7ProjectedPoint();
			if (renderHiRes) {
				shipProjected.x += nativeViewX;
				shipProjected.y += nativeViewY;
			}

			int offsetX = 0, offsetY = 0;
			switch (_spaceShotDirection) {
			case 0:
				offsetX = -35;
				break;
			case 1:
				offsetX = 35;
				break;
			case 2:
				offsetY = 20;
				break;
			case 3:
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
		break;
	}
}

void InsaneRebel2::renderTurretLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	int16 maxDuration = getShotMaxDuration();
	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int nativeViewX = renderHiRes ? _hiResPresentationViewX : _viewX;
	const int nativeViewY = renderHiRes ? _hiResPresentationViewY : _viewY;

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter <= 0)
			continue;

		int16 pan = ((2 - _turretShots[i].counter) * (_turretShots[i].targetX - nativeViewX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);

		int16 targetX = _turretShots[i].targetX;
		int16 targetY = _turretShots[i].targetY;
		int16 progress = maxDuration - _turretShots[i].counter;

		switch (_rebelLevelType) {
		case 1:
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
			drawLaserBeam(renderBitmap, pitch, width, height,
				110 + nativeViewX, 230 + nativeViewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				210 + nativeViewX, 230 + nativeViewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);
			break;

		case 6:
			drawLaserBeam(renderBitmap, pitch, width, height,
				-100 + nativeViewX, 0 + nativeViewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				0 + nativeViewX, 0 + nativeViewY, targetX, targetY,
				progress, maxDuration, 25, 8, 12);
			break;

		default:
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
	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_vehicleShots[i].counter <= 0)
			continue;

		int16 pan = ((2 - _vehicleShots[i].counter) * (_vehicleShots[i].targetX - _viewX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);

		int16 shipScreenX = ((_shipPosX - 160) >> 3) + 160;
		int16 shipScreenY = ((_shipPosY - 40) >> 2) + 105;
		int16 gunX = shipScreenX + 7;
		int16 gunY = shipScreenY + 25;

		int16 progress = maxDuration - _vehicleShots[i].counter;

		drawLaserBeam(renderBitmap, pitch, width, height,
			gunX, gunY,
			shipScreenX, shipScreenY,
			progress, maxDuration, 20, 8, 4);

		_vehicleShots[i].counter--;

		// Level 2 mask pixels select the POV impact sprite.
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
	int16 maxDuration = getShotMaxDuration();

	// Shots are stored in presentation-window space; convert with the offset the
	// current frame's presentation actually consumed (drawLaserBeam subtracts it
	// back in high-res, the low-res paths draw into the scrolled/warped buffer).
	const bool renderHiRes = isHiRes() && width >= 640 && height >= 400;
	const int16 nativeViewX = renderHiRes ? _hiResPresentationViewX : _viewX;
	const int16 nativeViewY = renderHiRes ? _hiResPresentationViewY : _viewY;

	for (int i = 0; i < 2; i++) {
		if (_spaceShots[i].counter <= 0)
			continue;

		int16 pan = ((_spaceShots[i].targetX - 160) * (2 - _spaceShots[i].counter)) / 2;
		pan = CLIP<int16>(pan, -127, 127);

		int16 targetX = _spaceShots[i].targetX + nativeViewX;
		int16 targetY = _spaceShots[i].targetY + nativeViewY;
		int16 leftGunX = _spaceShots[i].leftGunX + nativeViewX;
		int16 leftGunY = _spaceShots[i].leftGunY + nativeViewY;
		int16 rightGunX = _spaceShots[i].rightGunX + nativeViewX;
		int16 rightGunY = _spaceShots[i].rightGunY + nativeViewY;
		int16 progress = maxDuration - _spaceShots[i].counter;

		drawLaserBeam(renderBitmap, pitch, width, height,
			leftGunX, leftGunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		drawLaserBeam(renderBitmap, pitch, width, height,
			rightGunX, rightGunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		_spaceShots[i].counter--;
	}
}

void InsaneRebel2::renderHandler25LaserShots(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelDamageLevel != 0) {
		return;
	}

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter <= 0)
			continue;

		int16 pan = ((2 - _turretShots[i].counter) * (_turretShots[i].targetX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);

		int16 targetX = _turretShots[i].targetX + _rebelViewOffsetX;
		int16 targetY = _turretShots[i].targetY + _rebelViewOffsetY;

		int16 gunX = _turretShots[i].gunX + _rebelViewOffsetX;
		int16 gunY = _turretShots[i].gunY + _rebelViewOffsetY;

		int16 progress = maxDuration - _turretShots[i].counter;

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

void InsaneRebel2::renderCrosshair(byte *renderBitmap, int pitch, int width, int height) {
	if (!isShootingAllowed()) {
		return;
	}

	if (_rebelHandler == 25 && _rebelDamageLevel != 0) {
		return;
	}

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
		// A fresh lock (timer fully decayed) pings LOCKON, panned by crosshair X.
		if (_rebelHandler == 0x26 && _targetLockTimer == 0 && (getDifficultyParams().flags & 2) == 0)
			playSfx(4, 127, CLIP(aimPos.x - 160, -127, 127));
		_targetLockTimer = 7;
	} else if (_targetLockTimer > 0) {
		_targetLockTimer--;
	}

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

		// Enlarge the tiny third-person/cover reticles; the turret crosshair is already large.
		const int spriteScale = (_rebelHandler == 0x26) ? 1 : 2;

		const int reticleScale = isHiRes() ? 2 : getRebel2IndicatorScale(width, height);
		int crosshairX = aimPos.x * reticleScale;
		int crosshairY = aimPos.y * reticleScale;
		if (_rebelHandler != 7) {
			crosshairX += _viewX;
			crosshairY += _viewY;
		}

		if (_rebelHandler == 25) {
			crosshairX += _rebelViewOffsetX * reticleScale;
			crosshairY += _rebelViewOffsetY * reticleScale;
		}

		crosshairX += (_smush_iconsNut->getCharXOffset(reticleIndex) - cw / 2) * spriteScale;
		crosshairY += (_smush_iconsNut->getCharYOffset(reticleIndex) - ch / 2) * spriteScale;

		// reticle glyphs key only on color 0, so transparent231 is false.
		renderNutSpriteScaledClipped(renderBitmap, pitch, width, height,
			0, 0, width, height,
			crosshairX, crosshairY,
			_smush_iconsNut, reticleIndex, false, spriteScale, false);
	}
}

void InsaneRebel2::frameEndCleanup() {
	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (!it->destroyed) {
			it->active = false;
		}
	}

	resetCollisionZones();
}

} // End of namespace Scumm
