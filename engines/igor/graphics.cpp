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

#include "igor/igor.h"

namespace Igor {

/**
 * Copies a w x h block between two pixel buffers (each with its own pitch).
 * Transparent copies skip source pixels equal to 0.
 */
void IgorEngine::copyArea(uint8 *dst, int dstOffset, int dstPitch, const uint8 *src, int srcPitch, int w, int h, bool transparent) {
	uint8 *p = dst + dstOffset;
	for (int y = 0; y < h; ++y) {
		if (transparent) {
			for (int x = 0; x < w; ++x) {
				if (src[x] != 0) {
					p[x] = src[x];
				}
			}
		} else {
			memcpy(p, src, w);
		}
		p += dstPitch;
		src += srcPitch;
	}
}

/**
 * Draws a block of w x h pixels stored row after row (pitch == w) in the  animation frames buffer onto the screen at the given screen offset.
 * With alsoBackground the raw block is also written to the room background (_screenLayer1), so it persists when the screen is redrawn.
 *
 * "blend" selects how the block is combined with what is already there:
 *   kBlendCopy               plain copy
 *   kBlendLitSprite          sprite pixels with colors 0xC0-0xCF (Igors colors) are replaced by the background pixel or darkened according to the lighting area they land in;
 *                            use it for sprites that move through lit and shaded parts of a room
 *   kBlendBehindIgor         the block is drawn behind Igor: screen pixels with colors 0xC0-0xCF are kept
 *   kBlendBehindIgorAndText  same, and screen pixels with the dialogue text colors 0xF0-0xF1 are kept too
 */
void IgorEngine::drawAnimRect(int dstOffset, int animOffset, int w, int h, bool alsoBackground, AnimBlend blend) {
	const uint8 *src = _animFramesBuffer + animOffset;
	if (blend == kBlendCopy) {
		copyArea(_screenVGA, dstOffset, 320, src, w, w, h);
	} else {
		assert(w <= 100);
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				const int dst = dstOffset + y * 320 + x;
				uint8 color = src[y * w + x];
				if (blend == kBlendLitSprite) {
					if (color >= 0xC0 && color <= 0xCF) {
						const RoomObjectArea &area = _roomObjectAreasTable[_screenLayer2[dst]];
						if (area.y1Lum > 0)
							color = _screenLayer1[dst];
						else if (area.y2Lum > 0)
							color -= area.deltaLum;
					}
				} else {
					const uint8 onScreen = _screenVGA[dst];
					if ((onScreen >= 0xC0 && onScreen <= 0xCF) ||
							(blend == kBlendBehindIgorAndText && onScreen >= 0xF0 && onScreen <= 0xF1))
						color = onScreen;
				}
				_screenTempLayer[y * 100 + x] = color;
			}
		}
		copyArea(_screenVGA, dstOffset, 320, _screenTempLayer, 100, w, h);
	}
	if (alsoBackground)
		copyArea(_screenLayer1, dstOffset, 320, src, w, w, h);
}

/**
 * Plays the frames from firstFrame to lastFrame (either direction) of a sprite strip with drawAnimRect(kBlendLitSprite)
 * (frames of frameSize bytes starting at srcOffset, each width x height), waiting "delay" ticks between frames. When soundFrame is drawn,
 * "sound" is played.
 */
void IgorEngine::animateLitAnimFrames(int srcOffset, int firstFrame, int lastFrame,
		int frameSize, int width, int height, int dstOffset, int delay,
		int soundFrame, int sound) {
	const int step = firstFrame <= lastFrame ? 1 : -1;
	for (int frame = firstFrame;; frame += step) {
		drawAnimRect(dstOffset, srcOffset + frame * frameSize, width, height, false, kBlendLitSprite);
		if (frame == soundFrame)
			playSound(sound, 1);
		if (frame != lastFrame)
			waitForTimer(delay);
		if (frame == lastFrame)
			break;
	}
}

const uint8 *IgorEngine::getAnimFrame(int baseOffset, int tableOffset, int frame) {
	const uint8 *src = _animFramesBuffer + baseOffset;
	assert(frame >= 1);
	int frameOffset = READ_LE_UINT16(src + tableOffset + (frame - 1) * 2);
	return src + frameOffset - 1;
}

/**
 * extracts from src a frame and draws it into dst, optionally preserving text
 */
void IgorEngine::decodeAnimFrame(const uint8 *src, uint8 *dst, bool preserveText) {
	int y = READ_LE_UINT16(src) * 320; src += 2;
	int h = READ_LE_UINT16(src); src += 2;
	while (h--) {
		int w = *src++;
		int pos = y;
		while (w--) {
			pos += *src++;
			int len = *src++;
			if (len & 0x80) {
				uint8 color = *src++;
				len = 256 - len;
				if (preserveText) {
					for (int i = pos; i < pos + len; ++i) {
						if (dst[i] != _talkColorIndex && dst[i] != _talkShadowIndex) {
							dst[i] = color;
						}
					}
				} else {
					memset(dst + pos, color, len);
				}
				pos += len;
			} else {
				if (preserveText) {
					for (int i = pos; i < pos + len; ++i) {
						if (dst[i] != _talkColorIndex && dst[i] != _talkShadowIndex) {
							dst[i] = src[i - pos];
						}
					}
				} else {
					memcpy(dst + pos, src, len);
				}
				src += len;
				pos += len;
			}
		}
		y += 320;
	}
}

void IgorEngine::drawVerbsPanel() {
	memcpy(_screenVGA + 320 * 156, _verbsPanelBuffer, 320 * 12);
}

void IgorEngine::redrawVerb(uint8 verb, bool highlight) {
	uint8 verbBitmap[44 * 12];
	if (verb >= 2 && verb <= 8) {
		verb -= 2;
		for (int i = 0; i <= 11; ++i) {
			for (int j = 0; j <= 43; ++j) {
				uint8 color = _verbsPanelBuffer[i * 320 + verb * 46 + j];
				if (highlight && color != 0) {
					color += 8;
				}
				verbBitmap[i * 44 + j] = color;
			}
		}
		copyArea(_screenVGA, 320 * 156 + verb * 46, 320, verbBitmap, 44, 44, 12);
	}
}

void IgorEngine::scrollInventory() {
	if (_scrollInventoryStartY == _scrollInventoryEndY) {
		memcpy(_screenVGA + 54400, _inventoryPanelBuffer + (_scrollInventoryStartY - 1) * 320, 9600);
		_scrollInventory = false;
	} else {
		int offset = 54420;
		for (int y = _scrollInventoryStartY; y < _scrollInventoryStartY + 29; ++y) {
			memcpy(_screenVGA + offset, _inventoryPanelBuffer + 320 * y - 300, 280);
			offset += 320;
		}
		_scrollInventoryStartY += _scrollInventoryDy;
	}
}

void IgorEngine::drawInventory(int start, int mode) {
	loadData(IMG_InventoryPanel, _inventoryPanelBuffer);
	loadData(IMG_Objects, _inventoryImagesBuffer);
	int y, i;
	int end = start + 7; // seven slots
	int x = 1;
	// Paint inventory icons
	for (y = start; y != end; ++y) {
		if (_inventoryInfo[y - 1] == 0) {
			for (i = 1; i <= 30; ++i) {
				memset(_inventoryPanelBuffer + x * 40 - 20 + (i - 1) * 320, 0, 40);
			}
		} else {
			for (i = 1; i <= 30; ++i) {
				int img = _inventoryInfo[y - 1];
				assert(img >= 1);
				memcpy(_inventoryPanelBuffer + x * 40 - 20 + i * 320 - 321, _inventoryImagesBuffer + (i - 1) * 40 + (_inventoryImages[img - 1] - 1) * 1200, 40);
			}
		}
		++x;
	}
	// Hide arrows
	if (_inventoryInfo[72] == 1) {
		// 'hide' scroll up
		for (y = 5; y <= 11; ++y) {
			for (x = 4; x <= 12; ++x) {
				uint8 *p = _inventoryPanelBuffer + y * 320 + x - 321;
				if (*p == 0xF2) {
					*p = 0xF3;
					p = _inventoryPanelBuffer + y * 320 + x + 305 - 321;
					*p = 0xF3;
				}
			}
		}
	}
	if (_inventoryInfo[73] <= _inventoryInfo[72] + 6 || _inventoryInfo[72] >= _inventoryInfo[73] - 6) {
		// 'hide' scroll down
		for (y = 19; y <= 25; ++y) {
			for (x = 4; x <= 12; ++x) {
				uint8 *p = _inventoryPanelBuffer + y * 320 + x - 321;
				if (*p == 0xF2) {
					*p = 0xF3;
					p = _inventoryPanelBuffer + y * 320 + x + 305 - 321;
					*p = 0xF3;
				}
			}
		}
	}
	switch (mode) {
	case 0: // normal inventory rendering
		memcpy(_screenVGA + 54400, _inventoryPanelBuffer, 9600);
		_scrollInventory = false;
		break;
	case 1: // animation scrolling up
		for (y = 0; y <= 11; ++y) {
			for (x = 0; x <= 14; ++x) {
				uint8 *p = _screenVGA + x + y * 320 + 59520;
				if ((*p & 0x80) != 0) {
					*p += 8;
					p = _screenVGA + x + y * 320 + 59825;
					*p += 8;
				}
			}
		}
		memmove(_inventoryPanelBuffer + 9600, _inventoryPanelBuffer, 9600);
		memcpy(_inventoryPanelBuffer, _screenVGA + 54400, 9600);
		_scrollInventoryStartY = 7;
		_scrollInventoryEndY = 31;
		_scrollInventoryDy = 6;
		_scrollInventory = true;
		break;
	case 2: // animation scrolling down
		for (y = 0; y <= 11; ++y) {
			for (x = 0; x <= 14; ++x) {
				uint8 *p = _screenVGA + x + y * 320 + 55040;
				if ((*p & 0x80) != 0) {
					*p += 8;
					p = _screenVGA + x + y * 320 + 55345;
					*p += 8;
				}
			}
		}
		memmove(_inventoryPanelBuffer + 9600, _inventoryPanelBuffer, 9600);
		memcpy(_inventoryPanelBuffer + 9600, _screenVGA + 54400, 9600);
		_scrollInventoryStartY = 25;
		_scrollInventoryEndY = 1;
		_scrollInventoryDy = -6;
		_scrollInventory = true;
		break;
	}
}

/**
 * Idle animation of Igor (blinking): draws one or two single pixels (palette color 196, or 195 when fl is 1)
 * at fixed offsets from his head, depending on the way he faces. The pixels follow the room's shading
 * (they are skipped in shaded areas and darkened by the area's deltaLum when enableLight is 1).
 * Nothing is drawn unless Igor is at his default size (scaleHeight 50), or while dialogue text is running.
 */
void IgorEngine::updateIgorIdleAnimation(int fl) {
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	if (wd->scaleHeight != 50 || _gameState.dialogueTextRunning) {
		return;
	}
	int offset = 320 * (wd->y + 1 - wd->scaleWidth);
	int x = wd->x - _walkWidthScaleTable[wd->scaleHeight - 1] / 2;
	if (x <= 0) {
		return;
	}
	offset += x;
	RoomObjectArea *roa;
	int color = (fl == 0) ? 196 : 195;
	switch (wd->posNum) {
	case 2:
		roa = &_roomObjectAreasTable[_screenLayer2[offset + 1298]];
		if (wd->y > roa->y1Lum) {
			if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
				color -= roa->deltaLum;
			}
			_screenVGA[offset + 1298] = color;
		}
		break;
	case 3:
		roa = &_roomObjectAreasTable[_screenLayer2[offset + 1293]];
		if (wd->y > roa->y1Lum) {
			if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
				color -= roa->deltaLum;
			}
			_screenVGA[offset + 1293] = color;
		}
		color = (fl == 0) ? 196 : 195;
		roa = &_roomObjectAreasTable[_screenLayer2[offset + 1296]];
		if (wd->y > roa->y1Lum) {
			if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
				color -= roa->deltaLum;
			}
			_screenVGA[offset + 1296] = color;
		}
		break;
	case 4:
		roa = &_roomObjectAreasTable[_screenLayer2[offset + 1291]];
		if (wd->y > roa->y1Lum) {
			if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
				color -= roa->deltaLum;
			}
			_screenVGA[offset + 1291] = color;
		}
		break;
	}
}

} // End of namespace Igor
