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

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"

namespace LastExpress {

GraphicsManager::GraphicsManager(LastExpressEngine *engine) {
	_engine = engine;

	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_screenSurface.create(640, 480, format);
	_mainSurfaceIsInit = true;

	memset(_subtitlesBackBuffer, 0, sizeof(_subtitlesBackBuffer));
	memset(_mouseBackBuffer, 0, sizeof(_mouseBackBuffer));

	_renderBox1.width = 640;
	_renderBox1.height = 480;

	_renderBox2.width = 640;
	_renderBox2.height = 480;
}

GraphicsManager::~GraphicsManager() {
	_screenSurface.free();
}

bool GraphicsManager::acquireSurface() {
	// This function is technically useless, but I'm keeping it
	// as a marker for when the engine wants to acquire the gfx context
	// for when it has to draw something.
	//
	// It has proven to be useful during debugging,
	// maybe it'll be useful in the future as well...

	return true;
}

void GraphicsManager::unlockSurface() {
	// As before, this function is technically useless, but I'm keeping it
	// as a marker for when the engine wants to release the gfx context
	// for when it finishes drawing something.
	//
	// It has proven to be useful during debugging,
	// maybe it'll be useful in the future as well...
}

void GraphicsManager::burstAll() {
	bool subtitlesDrawn = false;
	bool mouseDrawn = false;

	if ((_engine->getSubtitleManager()->_flags & 1) != 0) {
		_engine->getSubtitleManager()->vSubOn();
		subtitlesDrawn = true;
	}

	if (canDrawMouse()) {
		drawMouse();
		mouseDrawn = true;
	}

	g_system->copyRectToScreen(_screenSurface.getPixels(), 640 * 2, 0, 0, 640, 480);
	g_system->updateScreen();

	if (mouseDrawn)
		restoreMouse();

	if (subtitlesDrawn)
		_engine->getSubtitleManager()->vSubOff();
}

void GraphicsManager::stepBG(int sceneIndex) {
	_disableCharacterDrawing = false;

	if (_stepBGRecursionFlag) {
		_renderBox1.x = 80;
		_renderBox1.y = 0;
		_renderBox1.width = 480;
		_renderBox1.height = 480;

		goStepBG(sceneIndex);

		_disableCharacterDrawing = true;
	} else {
		_stepBGRecursionFlag = true;

		goStepBG(sceneIndex);

		_stepBGRecursionFlag = false;
	}
}

void GraphicsManager::goStepBG(int sceneIndex) {
	TBM *chosenTbm;
	char bgName[12];

	_engine->getLogicManager()->doPreFunction(&sceneIndex);
	Common::strcpy_s(bgName, _engine->getLogicManager()->_trainData[sceneIndex].sceneFilename);
	int loadResult = _engine->getArchiveManager()->loadBG(bgName);

	if (loadResult == -1) {
		return;
	} else if (loadResult == 0) {
		chosenTbm = &_renderBox1;
	} else { // loadResult == 1
		chosenTbm = &_renderBox2;
	}

	_engine->getLogicManager()->_activeNode = sceneIndex;

	if (_engine->getLogicManager()->_closeUp) {
		getCharacter(kCharacterCath).characterPosition = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_nodeReturn].nodePosition;
	} else {
		getCharacter(kCharacterCath).characterPosition = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].nodePosition;
	}

	_engine->getSoundManager()->_scanAnySoundLoopingSection = true;

	if (_engine->_navigationEngineIsRunning) {
		_engine->getLogicManager()->sendAll(kCharacterCath, 17, 0);
		_engine->getMessageManager()->flush();

		if (_disableCharacterDrawing)
			return;

		_engine->getOtisManager()->adjustOtisTrueTime();
		_engine->getOtisManager()->refreshSequences();
		_engine->getOtisManager()->updateAll();
	}

	_engine->getSpriteManager()->drawCycleSimple(_frontBuffer);
	stepDissolve(chosenTbm);
	_engine->getLogicManager()->doPostFunction();
}

void GraphicsManager::stepDissolve(TBM *tbm) {
	for (int i = 0; i < _dissolveSteps; ++i) {
		int32 pos = sizeof(PixMap) * (tbm->x + 640 * tbm->y);

		int32 curFrameCount = _engine->getSoundFrameCounter();
		_engine->getSoundManager()->soundThread();

		dissolve(pos + sizeof(PixMap) * (i & 1), tbm->width, tbm->height, _frontBuffer);

		burstBox(tbm->x, tbm->y, tbm->width, tbm->height);
		int32 frameTimeDiff = _engine->getSoundFrameCounter() - curFrameCount;
		int32 timeFactor = 20;

		if (timeFactor / _dissolveSteps <= frameTimeDiff) {
			if (_dissolveSteps != 1 && timeFactor / (_dissolveSteps + 1) <= frameTimeDiff)
				_dissolveSteps--;
		} else {
			if (_dissolveSteps != 4 && timeFactor / (_dissolveSteps + 1) > frameTimeDiff)
				_dissolveSteps++;

			while (_engine->getSoundFrameCounter() - curFrameCount < timeFactor / _dissolveSteps) {
				// Not present in the original, since the input callbacks are on a different
				// thread than the game engine. We need to do this on our end though.
				_engine->handleEvents();
			}
		}

		// Not present in the original, like above.
		_engine->handleEvents();
	}

	if (acquireSurface()) {
		copy(_frontBuffer, (PixMap *)_screenSurface.getPixels(), tbm->x, tbm->y, tbm->width, tbm->height);
		unlockSurface();
	}

	burstBox(tbm->x, tbm->y, tbm->width, tbm->height);
}

void GraphicsManager::clear(Graphics::Surface &surface, int32 x, int32 y, int32 width, int32 height) {
	surface.fillRect(Common::Rect(x, y, x + width, y + height), 0);
}

void GraphicsManager::clear(PixMap *pixels, int32 x, int32 y, int32 width, int32 height) {
	// Shortcut for the whole screen...
	if (x == 0 && y == 0 && width == 640 && height == 480) {
		memset(pixels, 0, 640 * 480 * sizeof(PixMap));
		return;
	}

	// Clear line by line...
	PixMap *pixBuf = &pixels[640 * y + x];

	for (int32 row = 0; row < height; row++) {
		memset(pixBuf, 0, width * sizeof(PixMap));
		pixBuf += 640;
	}
}

void GraphicsManager::copy(PixMap *src, PixMap *dst, int32 x, int32 y, int32 width, int32 height) {
	// Shortcut for the whole screen...
	if (x == 0 && y == 0 && width == 640 && height == 480) {
		memcpy(dst, src, 640 * 480 * sizeof(PixMap));
		return;
	}

	// Copy line by line...
	int32 offset = x + 640 * y;
	PixMap *srcPtr = &src[offset];
	PixMap *dstPtr = &dst[offset];

	for (int32 row = 0; row < height; row++) {
		memcpy(dstPtr, srcPtr, width * sizeof(PixMap));
		srcPtr += 640;
		dstPtr += 640;
	}
}

#define DISSOLVE_MASK  _brightnessData[1]

void GraphicsManager::goDissolve(int32 location, int32 width, int32 height, PixMap *buffer) {
	int32 widthCount = width;
	int32 heightCount = height;
	PixMap *bufPtr = (PixMap *)((byte *)buffer + location);
	PixMap *surfPtr = (PixMap *)((byte *)_screenSurface.getPixels() + location);

	while (heightCount > 0) {
		while (widthCount > 0) {
			*surfPtr &= DISSOLVE_MASK;
			*surfPtr >>= 1;
			*surfPtr += (*bufPtr & DISSOLVE_MASK) >> 1;

			surfPtr += 2;
			bufPtr += 2;
			widthCount -= 2;
		}

		widthCount = width;

		surfPtr = (PixMap *)(((uintptr)(&surfPtr[640 - width])) ^ 2);
		bufPtr = (PixMap *)(((uintptr)(&bufPtr[640 - width])) ^ 2);

		heightCount--;
	}
}

#undef DISSOLVE_MASK

void GraphicsManager::bitBltSprite255(Sprite *sprite, PixMap *pixels) {
	PixMap *destPtr;
	uint16 *palette;
	byte *compressedData;
	PixMap *destEndPtr;
	uint16 cmd;
	uint16 color;
	int count;
	PixMap *sourcePtr;
	uint16 offset;

	destPtr = pixels;
	palette = sprite->colorPalette;
	compressedData = sprite->compData;
	destEndPtr = pixels + (640 * 480);

	while (destPtr < destEndPtr) {
		cmd = READ_LE_UINT16((uint16 *)compressedData);

		// Direct color lookup
		if ((cmd & 0xFF) < 0x80) {
			*destPtr = palette[(cmd & 0xFF)];
			destPtr++;
			compressedData++;
			continue;
		}

		// Handle compression commands
		if ((cmd & 0xFF) >= 0xF0) {
			// Skip pixels command
			offset = ((cmd & 0xFF) << 8) | ((cmd >> 8) & 0xFF);
			offset &= 0xFFF;
			destPtr += offset;
			compressedData += 2;
		} else if ((cmd & 0xFF) >= 0xE0) {
			// Run-length encoding (repeat color)
			color = palette[(cmd >> 8) & 0xFF];
			count = (cmd & 0x0F) + 1;

			while (count--) {
				*destPtr = color;
				destPtr++;
			}

			compressedData += 2;
		} else {
			// Copy pixels from previous data
			uint16 cmdHiByte = (cmd >> 8) & 0xFF;
			uint16 cmdLoByte = cmd & 0xFF;

			offset = cmdHiByte | ((cmdLoByte & 0x7) << 8); 

			sourcePtr = &destPtr[offset - 0x1000 / sizeof(PixMap)];

			count = ((cmdLoByte & 0x78) >> 3) + 3;

			while (count--) {
				*destPtr = *sourcePtr;

				destPtr++;
				sourcePtr++;
			}

			compressedData += 2;
		}
	}
}

void GraphicsManager::bitBltSprite128(Sprite *sprite, PixMap *pixels) {
	uint16 *palette;
	PixMap *destPtr;
	byte *compressedData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap color;
	int count;
	int offset;

	palette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compressedData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	while (destPtr < destEndPtr) {
		cmd = *compressedData++;

		// Direct color lookup
		if (!(cmd & 0x80)) {
			*destPtr = palette[cmd];
			destPtr++;
			continue;
		}

		// Handle compression commands
		if (cmd & 0x40) {
			// Pattern command
			if (cmd & 0x20) {
				// Extract pattern length
				count = cmd & 0x1F;
				offset = skipValue + count;

				// Write pattern: color, skip, color
				color = palette[1];
				*destPtr = color;
				destPtr += offset + 1;
				*destPtr = color;
				destPtr++;
			} else {
				// Complex pattern command
				count = cmd & 0x1F;

				if ((count & 0x10)) {
					count = (((count & 0x0F) << 8) | *compressedData);
					compressedData++;

					if ((count & 0x800) != 0) {
						// Skip pixels
						count &= 0x7FF;
						destPtr += count;
					} else {
						// Write pattern: color, skip, color
						color = palette[1];
						*destPtr = color;
						destPtr += count + 1;
						*destPtr = color;
						destPtr++;
					}
				} else {
					// Write pattern: color, skip, color
					color = palette[1];
					*destPtr = color;
					destPtr += count + 1;
					*destPtr = color;
					destPtr++;
				}
			}
		} else {
			// Run-length encoding (repeat color)
			count = cmd & 0x3F;
			cmd = *compressedData++;
			color = palette[cmd];

			while (count--) {
				*destPtr = color;
				destPtr++;
			}
		}
	}
}

void GraphicsManager::bitBltSprite32(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destEndPtr;
	PixMap *destPtr;
	byte *compData;
	byte cmd;
	uint16 value;
	byte count;
	PixMap color;
	PixMap *tempDestPtr;

	colorPalette = sprite->colorPalette;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		// Handle run-length encoding
		if ((cmd & 0x1F) != 0) {
			count = cmd >> 5;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0x1F];

			while (count--) {
				*destPtr++ = color;
			}
		} else {
			// Handle other compression commands
			value = cmd << 3;

			byte secondByte = *compData++;
			value = (value & 0xFF00) | secondByte;

			if ((value & 0x400) != 0) {
				// Skip pixels command
				value &= 0x3FF;
				destPtr += value;
			} else {
				// Pattern command: color, skip, color
				tempDestPtr = destPtr;
				color = colorPalette[1];

				*destPtr = color;

				tempDestPtr = &destPtr[value + 1];
				*tempDestPtr = color;

				destPtr = tempDestPtr + 1;
			}
		}
	}
}

void GraphicsManager::bitBltSprite16(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destPtr;
	byte *compData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap value;
	PixMap color;
	PixMap *tempDestPtr;
	byte count;

	colorPalette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		if (!(cmd & 0x80)) {
			// Handle run-length encoding
			count = cmd >> 4;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0xF];

			while (count > 0) {
				*destPtr++ = color;
				count--;
			}
		} else {
			// Handle other compression commands
			if (cmd & 0x40) {
				// Pattern command
				value = cmd & 0x3F;

				// Pattern: color, skip by a certain amount, color
				color = colorPalette[1];
				*destPtr = color;

				tempDestPtr = &destPtr[skipValue + value + 1];
				*tempDestPtr = color;

				destPtr = tempDestPtr + 1;
			} else {
				value = cmd & 0x3F;

				if ((value & 0x20) && ((value = ((value & 0x1F) << 8) | *compData++, (value & 0x1000)))) {
					// Skip pixels command
					value &= 0xFFF;
					destPtr += value;
				} else {
					// Pattern: color, skip, color
					color = colorPalette[1];
					*destPtr = color;

					tempDestPtr = &destPtr[value + 1];
					*tempDestPtr = color;

					destPtr = tempDestPtr + 1;
				}
			}
		}
	}
}

void GraphicsManager::bitBltSprite8(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destPtr;
	byte *compData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap value;
	PixMap color;
	PixMap *tempDestPtr;
	byte count;

	colorPalette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		// Handle run-length encoding
		if (!(cmd & 0x80)) {
			count = cmd >> 3;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0x7];

			while (count > 0) {
				*destPtr++ = color;
				count--;
			}
		} else {
			// Handle pattern commands
			if (cmd & 0x40) {
				value = cmd & 0x3F;

				// Pattern: color, skip by a certain amount, color
				color = colorPalette[1];
				*destPtr = color;

				tempDestPtr = &destPtr[skipValue + value + 1];
				*tempDestPtr = color;

				destPtr = tempDestPtr + 1;
			} else {
				value = cmd & 0x3F;

				if ((value & 0x20) && ((value = ((value & 0x1F) << 8) | *compData++, (value & 0x1000)))) {
					// Skip pixels command
					value &= 0xFFF;
					destPtr += value;
				} else {
					// Pattern: color, skip, color
					color = colorPalette[1];
					*destPtr = color;

					tempDestPtr = &destPtr[value + 1];
					*tempDestPtr = color;

					destPtr = tempDestPtr + 1;
				}
			}
		}
	}
}

void GraphicsManager::bitBltWane128(Sprite *sprite, PixMap *pixels) {
	uint16 *palette;
	PixMap *destPtr;
	byte *compressedData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap color;
	int count;
	int page;

	palette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compressedData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	// Calculate initial page for checkerboard pattern
	page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

	while (destPtr < destEndPtr) {
		cmd = *compressedData++;

		// Direct color lookup
		if (!(cmd & 0x80)) {
			color = palette[cmd];

			// Write pixel if allowed by the current checkerboard page
			if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) == 0))) {
				*destPtr = color;
			}

			destPtr++;

			// Update checkerboard page
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
			continue;
		}

		// Handle compression commands
		if (cmd & 0x40) {
			// Pattern command
			if (cmd & 0x20) {
				// Extract pattern length
				count = cmd & 0x1F;
				count += skipValue;

				// Write pattern: color, skip, color
				color = palette[1];

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) != 0))) {
					*destPtr = color;
				}

				destPtr++;
				destPtr += count;

				// Update checkerboard page
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) != 0))) {
					*destPtr = color;
				}

				destPtr++;
			} else {
				// Complex pattern command
				count = cmd & 0x1F;

				if (count & 0x10) {
					count = (((count & 0x0F) << 8) | *compressedData);
					compressedData++;

					// Skip pixels
					if ((count & 0x800) != 0) {
						count &= 0x7FF;
						destPtr += count;

						// Update checkerboard page
						page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
					} else {
						// Write pattern: color, skip, color
						color = palette[1];

						// Write pixel if allowed by the current checkerboard page
						if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
							(page == 1 && (((uintptr)destPtr & 2) != 0))) {
							*destPtr = color;
						}

						destPtr++;
						destPtr += count;

						// Update checkerboard page
						page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

						// Write pixel if allowed by the current checkerboard page
						if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
							(page == 1 && (((uintptr)destPtr & 2) != 0))) {
							*destPtr = color;
						}

						destPtr++;
					}
				} else {
					// Write pattern: color, skip, color
					color = palette[1];

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) != 0))) {
						*destPtr = color;
					}

					destPtr++;
					destPtr += count;

					// Update checkerboard page
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) != 0))) {
						*destPtr = color;
					}

					destPtr++;
				}
			}
		} else {
			// Run-length encoding (repeat color)
			count = cmd & 0x3F;
			cmd = *compressedData++;
			color = palette[cmd];

			// Write pixel if allowed by the current checkerboard page
			if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) != 0))) {
				destPtr++;

				count--;
				if (count == 0)
					continue;
			}

			// First, determine how many full pairs (colored pixel + empty pixel) we have...
			int halfCount = count >> 1;
			bool hasRemainder = (count & 1) != 0;

			// Process full pairs...
			if (halfCount > 0) {
				do {
					*destPtr = color;
					destPtr += 2; // Move two positions to maintain checkerboard...
					halfCount--;
				} while (halfCount > 0);
			}

			// Handle remaining pixel if count was odd...
			if (hasRemainder) {
				*destPtr = color;
				destPtr++;
			}

			// Update page number
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
		}
	}
}

void GraphicsManager::bitBltWane32(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destEndPtr;
	PixMap *destPtr;
	byte *compData;
	byte cmd;
	uint16 value;
	byte count;
	PixMap color;
	int page;

	colorPalette = sprite->colorPalette;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;

	// Calculate initial page for checkerboard pattern
	page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		// Handle run-length encoding
		if ((cmd & 0x1F) != 0) {
			count = cmd >> 5;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0x1F];

			// Write pixel if allowed by the current checkerboard page
			if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) != 0))) {
				destPtr++;
				count--;

				if (count == 0)
					continue;
			}

			// First, determine how many full pairs (colored pixel + empty pixel) we have...
			int halfCount = count >> 1;
			bool hasRemainder = (count & 1) != 0;

			if (halfCount > 0) {
				do {
					*destPtr = color;
					destPtr += 2; // Move two positions to maintain checkerboard...
					halfCount--;
				} while (halfCount > 0);
			}

			// Handle remaining pixel if count was odd...
			if (hasRemainder) {
				*destPtr = color;
				destPtr++;
			}

			// Update page number
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
		} else {
			// Handle other compression commands
			value = cmd << 3;

			byte secondByte = *compData++;
			value = (value & 0xFF00) | secondByte;

			if (value & 0x400) {
				// Skip pixels command
				value &= 0x3FF;
				destPtr += value;

				// Update page number
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
			} else {
				// Pattern command: color, skip, color
				color = colorPalette[1];

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) != 0))) {
					*destPtr = color;
				}

				destPtr++;
				destPtr += value;

				// Update page number
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) != 0))) {
					*destPtr = color;
				}

				destPtr++;
			}
		}
	}
}

void GraphicsManager::bitBltWane16(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destPtr;
	byte *compData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap value;
	PixMap color;
	byte count;
	int page;

	colorPalette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	// Calculate initial page for checkerboard pattern
	page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		if (!(cmd & 0x80)) {
			// Handle run-length encoding
			count = cmd >> 4;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0xF];

			// Write pixel if allowed by the current checkerboard page
			if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) != 0))) {
				destPtr++;
				count--;

				if (count == 0)
					continue;
			}

			// First, determine how many full pairs (colored pixel + empty pixel) we have...
			int halfCount = count >> 1;
			bool hasRemainder = (count & 1) != 0;

			if (halfCount > 0) {
				do {
					*destPtr = color;
					destPtr += 2; // Move two positions to maintain checkerboard...
					halfCount--;
				} while (halfCount > 0);
			}

			// Handle remaining pixel if count was odd...
			if (hasRemainder) {
				*destPtr = color;
				destPtr++;
			}

			// Update page number
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
		} else {
			// Handle other compression commands
			if (cmd & 0x40) {
				// Pattern command
				value = cmd & 0x3F;
				value += skipValue;

				// Pattern: color, skip by a certain amount, color
				color = colorPalette[1];

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) != 0))) {
					*destPtr = color;
				}

				destPtr++;
				destPtr += value;

				// Update page number
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) != 0))) {
					*destPtr = color;
				}

				destPtr++;
			} else {
				value = cmd & 0x3F;

				if ((value & 0x20) && ((value = ((value & 0x1F) << 8) | *compData++, (value & 0x1000)))) {
					// Skip pixels command
					value &= 0xFFF;
					destPtr += value;

					// Update page number
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
				} else {
					// Pattern: color, skip, color
					color = colorPalette[1];

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) != 0))) {
						*destPtr = color;
					}

					destPtr++;
					destPtr += value;

					// Update page number
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) != 0))) {
						*destPtr = color;
					}

					destPtr++;
				}
			}
		}
	}
}

void GraphicsManager::bitBltWane8(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destPtr;
	byte *compData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap value;
	PixMap color;
	byte count;
	int page;

	colorPalette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	// Calculate initial page for checkerboard pattern
	page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		// Handle run-length encoding
		if (!(cmd & 0x80)) {
			count = cmd >> 3;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0x7];

			// First, determine how many full pairs (colored pixel + empty pixel) we have...
			if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) != 0))) {
				destPtr++;
				count--;

				if (count == 0)
					continue;
			}

			// Process pixel runs with checkerboard pattern
			int halfCount = count >> 1;
			bool hasRemainder = (count & 1) != 0;

			if (halfCount > 0) {
				do {
					*destPtr = color;
					destPtr += 2; // Move two positions to maintain checkerboard...
					halfCount--;
				} while (halfCount > 0);
			}

			// Handle remaining pixel if count was odd...
			if (hasRemainder) {
				*destPtr = color;
				destPtr++;
			}

			// Update page number
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
		} else {
			// Handle pattern commands
			if (cmd & 0x40) {
				value = cmd & 0x3F;
				value += skipValue;

				// Pattern: color, skip by a certain amount, color
				color = colorPalette[1];

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) != 0))) {
					*destPtr = color;
				}

				destPtr++;
				destPtr += value;

				// Update page number
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) != 0))) {
					*destPtr = color;
				}

				destPtr++;
			} else {
				value = cmd & 0x3F;

				if ((value & 0x20) && ((value = ((value & 0x1F) << 8) | *compData++, (value & 0x1000)))) {
					// Skip pixels command
					value &= 0xFFF;
					destPtr += value;

					// Update page number
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
				} else {
					// Pattern: color, skip, color
					color = colorPalette[1];

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) != 0))) {
						*destPtr = color;
					}

					destPtr++;
					destPtr += value;

					// Update page number
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) != 0))) {
						*destPtr = color;
					}

					destPtr++;
				}
			}
		}
	}
}

void GraphicsManager::bitBltWax128(Sprite *sprite, PixMap *pixels) {
	uint16 *palette;
	PixMap *destPtr;
	byte *compressedData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap color;
	int count;
	int page;

	palette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compressedData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	// Calculate initial page for checkerboard pattern
	page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

	while (destPtr < destEndPtr) {
		cmd = *compressedData++;

		// Direct color lookup
		if (!(cmd & 0x80)) {
			color = palette[cmd];

			// Write pixel if allowed by the current checkerboard page
			if ((page == 0 && (((uintptr)destPtr & 2) == 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) != 0))) {
				*destPtr = color;
			}

			destPtr++;

			// Update checkerboard page
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
			continue;
		}

		// Handle compression commands
		if (cmd & 0x40) {
			// Pattern command
			if (cmd & 0x20) {
				// Extract pattern length
				count = cmd & 0x1F;
				count += skipValue;

				// Write pattern: color, skip, color
				color = palette[1];

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) == 0))) {
					*destPtr = color;
				}

				destPtr++;
				destPtr += count;

				// Update checkerboard page
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) == 0))) {
					*destPtr = color;
				}

				destPtr++;
			} else {
				// Complex pattern command
				count = cmd & 0x1F;

				if (count & 0x10) {
					count = (((count & 0x0F) << 8) | *compressedData);
					compressedData++;

					// Skip pixels
					if ((count & 0x800) != 0) {
						count &= 0x7FF;
						destPtr += count;

						// Update checkerboard page
						page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
					} else {
						// Write pattern: color, skip, color
						color = palette[1];

						// Write pixel if allowed by the current checkerboard page
						if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
							(page == 1 && (((uintptr)destPtr & 2) == 0))) {
							*destPtr = color;
						}

						destPtr++;
						destPtr += count;

						// Update checkerboard page
						page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

						// Write pixel if allowed by the current checkerboard page
						if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
							(page == 1 && (((uintptr)destPtr & 2) == 0))) {
							*destPtr = color;
						}

						destPtr++;
					}
				} else {
					// Write pattern: color, skip, color
					color = palette[1];

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) == 0))) {
						*destPtr = color;
					}

					destPtr++;
					destPtr += count;

					// Update checkerboard page
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) == 0))) {
						*destPtr = color;
					}

					destPtr++;
				}
			}
		} else {
			// Run-length encoding (repeat color)
			count = cmd & 0x3F;
			cmd = *compressedData++;
			color = palette[cmd];

			// Write pixel if allowed by the current checkerboard page
			if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) == 0))) {
				destPtr++;

				count--;
				if (count == 0)
					continue;
			}

			// First, determine how many full pairs (empty pixel + colored pixel) we have...
			int halfCount = count >> 1;
			bool hasRemainder = (count & 1) != 0;

			// Process full pairs...
			if (halfCount > 0) {
				do {
					*destPtr = color;
					destPtr += 2; // Move two positions to maintain checkerboard...
					halfCount--;
				} while (halfCount > 0);
			}

			// Handle remaining pixel if count was even...
			if (hasRemainder) {
				*destPtr = color;
				destPtr++;
			}

			// Update page number
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
		}
	}
}

void GraphicsManager::bitBltWax32(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destEndPtr;
	PixMap *destPtr;
	byte *compData;
	byte cmd;
	uint16 value;
	byte count;
	PixMap color;
	int page;

	colorPalette = sprite->colorPalette;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;

	// Calculate initial page for checkerboard pattern
	page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		// Handle run-length encoding
		if ((cmd & 0x1F) != 0) {
			count = cmd >> 5;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0x1F];

			// Write pixel if allowed by the current checkerboard page
			if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) == 0))) {
				destPtr++;
				count--;

				if (count == 0)
					continue;
			}

			// First, determine how many full pairs (empty pixel + colored pixel) we have...
			int halfCount = count >> 1;
			bool hasRemainder = (count & 1) != 0;

			if (halfCount > 0) {
				do {
					*destPtr = color;
					destPtr += 2; // Move two positions to maintain checkerboard...
					halfCount--;
				} while (halfCount > 0);
			}

			// Handle remaining pixel if count was even...
			if (hasRemainder) {
				*destPtr = color;
				destPtr++;
			}

			// Update page number
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
		} else {
			// Handle other compression commands
			value = cmd << 3;

			byte secondByte = *compData++;
			value = (value & 0xFF00) | secondByte;

			if (value & 0x400) {
				// Skip pixels command
				value &= 0x3FF;
				destPtr += value;

				// Update page number
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
			} else {
				// Pattern command: color, skip, color
				color = colorPalette[1];

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) == 0))) {
					*destPtr = color;
				}

				destPtr++;
				destPtr += value;

				// Update page number
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) == 0))) {
					*destPtr = color;
				}

				destPtr++;
			}
		}
	}
}

void GraphicsManager::bitBltWax16(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destPtr;
	byte *compData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap value;
	PixMap color;
	byte count;
	int page;

	colorPalette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	// Calculate initial page for checkerboard pattern
	page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		if (!(cmd & 0x80)) {
			// Handle run-length encoding
			count = cmd >> 4;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0xF];

			// Write pixel if allowed by the current checkerboard page
			if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) == 0))) {
				destPtr++;
				count--;

				if (count == 0)
					continue;
			}

			// First, determine how many full pairs (empty pixel + colored pixel) we have...
			int halfCount = count >> 1;
			bool hasRemainder = (count & 1) != 0;

			if (halfCount > 0) {
				do {
					*destPtr = color;
					destPtr += 2; // Move two positions to maintain checkerboard...
					halfCount--;
				} while (halfCount > 0);
			}

			// Handle remaining pixel if count was even...
			if (hasRemainder) {
				*destPtr = color;
				destPtr++;
			}

			// Update page number
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
		} else {
			// Handle other compression commands
			if (cmd & 0x40) {
				// Pattern command
				value = cmd & 0x3F;
				value += skipValue;

				// Pattern: color, skip by a certain amount, color
				color = colorPalette[1];

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) == 0))) {
					*destPtr = color;
				}

				destPtr++;
				destPtr += value;

				// Update page number
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) == 0))) {
					*destPtr = color;
				}

				destPtr++;
			} else {
				value = cmd & 0x3F;

				if ((value & 0x20) && ((value = ((value & 0x1F) << 8) | *compData++, (value & 0x1000)))) {
					// Skip pixels command
					value &= 0xFFF;
					destPtr += value;

					// Update page number
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
				} else {
					// Pattern: color, skip, color
					color = colorPalette[1];

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) == 0))) {
						*destPtr = color;
					}

					destPtr++;
					destPtr += value;

					// Update page after moving
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) == 0))) {
						*destPtr = color;
					}

					destPtr++;
				}
			}
		}
	}
}

void GraphicsManager::bitBltWax8(Sprite *sprite, PixMap *pixels) {
	uint16 *colorPalette;
	PixMap *destPtr;
	byte *compData;
	PixMap *destEndPtr;
	int skipValue;
	byte cmd;
	PixMap value;
	PixMap color;
	byte count;
	int page;

	colorPalette = sprite->colorPalette;
	destPtr = pixels + sprite->rect.width / sizeof(PixMap);
	compData = sprite->compData;
	destEndPtr = pixels + sprite->rect.height / sizeof(PixMap);
	skipValue = 639 - (sprite->rect.right - sprite->rect.left);

	// Calculate initial page for checkerboard pattern
	page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

	while (destPtr < destEndPtr) {
		cmd = *compData++;

		// Handle run-length encoding
		if (!(cmd & 0x80)) {
			count = cmd >> 3;
			if (count == 0) {
				count = *compData++;
			}

			color = colorPalette[cmd & 0x7];

			// First, determine how many full pairs (colored pixel + empty pixel) we have...
			if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
				(page == 1 && (((uintptr)destPtr & 2) == 0))) {
				destPtr++;
				count--;

				if (count == 0)
					continue;
			}

			// Process pixel runs with checkerboard pattern
			int halfCount = count >> 1;
			bool hasRemainder = (count & 1) != 0;

			if (halfCount > 0) {
				do {
					*destPtr = color;
					destPtr += 2; // Move two positions to maintain checkerboard...
					halfCount--;
				} while (halfCount > 0);
			}

			// Handle remaining pixel if count was even...
			if (hasRemainder) {
				*destPtr = color;
				destPtr++;
			}

			// Update page number
			page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
		} else {
			// Handle pattern commands
			if (cmd & 0x40) {
				value = cmd & 0x3F;
				value += skipValue;

				// Pattern: color, skip by a certain amount, color
				color = colorPalette[1];

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) == 0))) {
					*destPtr = color;
				}

				destPtr++;
				destPtr += value;

				// Update page number
				page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

				// Write pixel if allowed by the current checkerboard page
				if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
					(page == 1 && (((uintptr)destPtr & 2) == 0))) {
					*destPtr = color;
				}

				destPtr++;
			} else {
				value = cmd & 0x3F;

				if ((value & 0x20) && ((value = ((value & 0x1F) << 8) | *compData++, (value & 0x1000)))) {
					// Skip pixels command
					value &= 0xFFF;
					destPtr += value;

					// Update page number
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;
				} else {
					// Pattern: color, skip, color
					color = colorPalette[1];

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) == 0))) {
						*destPtr = color;
					}

					destPtr++;
					destPtr += value;

					// Update page number
					page = ((destPtr - pixels) / (0x500 / sizeof(PixMap))) & 1;

					// Write pixel if allowed by the current checkerboard page
					if ((page == 0 && (((uintptr)destPtr & 2) != 0)) ||
						(page == 1 && (((uintptr)destPtr & 2) == 0))) {
						*destPtr = color;
					}

					destPtr++;
				}
			}
		}
	}
}

void GraphicsManager::eraseSprite(byte *data) {
	if (acquireSurface()) {
		doErase(data);
		unlockSurface();
	}
}

void GraphicsManager::dissolve(int32 location, int32 width, int32 height, PixMap *buf) {
	if (acquireSurface()) {
		goDissolve(location, width, height, buf);
		unlockSurface();
	}
}

void GraphicsManager::doErase(byte *data) {
	byte *screenSurface = (byte *)_screenSurface.getPixels();
	byte *endOfSurface = screenSurface + (640 * 480 * sizeof(PixMap));
	byte *previousScreenBuffer = (byte *)_backBuffer;

	uint16 *eraseMask = (uint16 *)data;

	// Apply the old screen buffer on the erase mask
	do {
		if (READ_LE_UINT16(eraseMask)) {
			memcpy(screenSurface, previousScreenBuffer, 4 * READ_LE_UINT16(eraseMask));
			previousScreenBuffer += 4 * READ_LE_UINT16(eraseMask);
			screenSurface += 4 * READ_LE_UINT16(eraseMask);
		}
	
		int skipSize = READ_LE_UINT16(eraseMask + 1) << 2;
		screenSurface += skipSize;
		previousScreenBuffer += skipSize;
		eraseMask += 2;
	} while (screenSurface < endOfSurface);
}

void GraphicsManager::modifyPalette(uint16 *data, uint32 size) {
	if (_luminosityIsInit) {
		// Apply gamma correction unless gamma level is 3 (i.e. no gamma correction)...
		if (_gammaLevel != 3) {
			uint16 *dataPtr = data;

			for (uint32 i = 0; i < size; i++) {
				uint16 pixel = *dataPtr;

				uint16 blue = (pixel & 0x1F);
				uint16 green = (pixel & 0x3E0) >> 5;
				uint16 red = (pixel & 0x7C00) >> 10;

				// Edit the color components...
				blue = _luminosityData[32 * _gammaLevel + blue];
				green = _luminosityData[32 * _gammaLevel + green];
				red = _luminosityData[32 * _gammaLevel + red];

				// ...and recombine them
				*dataPtr = blue + (green << 5) + (red << 10);

				dataPtr++;
			}
		}

		// Adjust brightness...
		for (uint32 i = 0; i < size; i++) {
			uint16 pixel = data[i];

			uint16 maskedPart = _brightnessMaskedBits & pixel;
			uint16 unmaskedPart = _brightnessUnmaskedBits & pixel;

			// Apply brightness (double the masked part)...
			maskedPart *= 2;

			// ...and recombine
			data[i] = unmaskedPart | maskedPart;
		}
	}
}

int GraphicsManager::getGammaLevel() {
	return _gammaLevel;
}

void GraphicsManager::setGammaLevel(int level) {
	if (level < 0)
		level = 0;

	if (level > 6)
		level = 6;

	_gammaLevel = level;

	_engine->getSubtitleManager()->initSubtitles();
	_engine->getArchiveManager()->loadMice();
}

void GraphicsManager::initLuminosityValues(int16 rMask, int16 gMask, int16 bMask) {
	if (bMask != 0x1F) {
		_luminosityIsInit = true;
		_brightnessUnmaskedBits = 0;
		_brightnessData[1] = 0xF7BC;
		_brightnessData[2] = 0xE738;
		_brightnessData[3] = 0xC630;
		_brightnessData[4] = 0x8420;
		_brightnessMaskedBits = rMask | gMask | bMask;
		_engine->getSubtitleManager()->initSubtitles();
		_engine->getArchiveManager()->loadMice();
		return;
	}

	if (gMask != 0x3E0) {
		_luminosityIsInit = true;
		_brightnessData[1] = 0xF79E;
		_brightnessData[2] = 0xE71C;
		_brightnessData[3] = 0xC618;
		_brightnessData[4] = 0x8410;
		_brightnessMaskedBits = rMask | gMask;
		_brightnessUnmaskedBits = 0x1F;
		_engine->getSubtitleManager()->initSubtitles();
		_engine->getArchiveManager()->loadMice();
		return;
	}

	if (rMask != 0x7C00) {
		_luminosityIsInit = true;
		_brightnessData[1] = 0xF3DE;
		_brightnessData[2] = 0xE39C;
		_brightnessData[3] = 0xC398;
		_brightnessData[4] = 0x8210;
		_brightnessMaskedBits = rMask;
		_brightnessUnmaskedBits = 0x3FF;
		_engine->getSubtitleManager()->initSubtitles();
		_engine->getArchiveManager()->loadMice();
		return;
	}
}

void GraphicsManager::initDecomp(PixMap *data, TBM *tbm) {
	_bgDecompTargetRect = tbm;
	_bgDecompOutBuffer = (byte *)data;
	_bgDecompFlags = 0;
}

void GraphicsManager::decompR(byte *data, int32 size) {
	int remainingSize = size;

	while (remainingSize > 0) {
		// Determine the next command byte
		byte commandByte;
		if (_bgDecompFlags & 0x10) {
			// Use saved byte from previous incomplete decompression
			_bgDecompFlags &= ~0x10;
			commandByte = _bgLastCompItem;
		} else {
			// Read next byte from input
			commandByte = *data;

			data++;
			remainingSize--;
		}

		// Check if this is a copy command
		if ((commandByte & 0x80) != 0) {
			// Check if we have enough data to process
			if (remainingSize == 0) {
				// Save state for next call
				_bgDecompFlags |= 0x10;
				_bgLastCompItem = commandByte;
				return;
			}

			// Read offset byte and decrement remaining size
			int offsetByte = *data;

			data++;
			remainingSize--;

			// Calculate parameters for copy command
			int count = ((commandByte & 0x70) >> 4) + 3;
			byte *sourcePtr = &_bgDecompOutBufferTemp[2 * (((commandByte & 0xF) << 8) + offsetByte) - 0x2000];

			// Copy pixels from previous output
			for (int i = 0; i < count; i++) {
				*_bgDecompOutBufferTemp = *sourcePtr;
				_bgDecompOutBufferTemp += 2;
				sourcePtr += 2;
			}
		} else {
			// This is a run command (repeating value)
			int count = (commandByte >> 5) + 1;
			byte value = (commandByte & 0x1F) << 2;

			// Output repeated value
			for (int i = 0; i < count; i++) {
				*_bgDecompOutBufferTemp = value;
				_bgDecompOutBufferTemp += 2;
			}
		}
	}
}

void GraphicsManager::decompG(byte *data, int32 size) {
	int remainingSize = size;
	byte *outBuffer = _bgDecompOutBufferTemp;

	while (remainingSize > 0) {
		// Determine the next command byte
		byte commandByte;
		if (_bgDecompFlags & 0x10) {
			// Use saved byte from previous incomplete decompression
			_bgDecompFlags &= ~0x10;
			commandByte = _bgLastCompItem;
		} else {
			// Read next byte from input
			commandByte = *data;
			data++;
			remainingSize--;
		}

		// Check if this is a copy command
		if ((commandByte & 0x80) != 0) {
			// Check if we have enough data to process
			if (remainingSize == 0) {
				// Save state for next call
				_bgLastCompItem = commandByte;
				_bgDecompOutBufferTemp = outBuffer;
				_bgDecompFlags |= 0x10;
				return;
			}

			// Read offset byte and decrement remaining size
			int offsetByte = *data;

			data++;
			remainingSize--;

			// Calculate parameters for copy command
			int count = ((commandByte & 0x70) >> 4) + 3;
			int16 *sourcePtr = (int16 *)&outBuffer[2 * (((commandByte & 0xF) << 8) + offsetByte) - 0x2000];

			// Copy green component (bits 5-9) from previous output
			for (int i = 0; i < count; i++) {
				*((int16 *)outBuffer) |= (*sourcePtr & 0x3E0);
				outBuffer += 2;
				sourcePtr++;
			}
		} else {
			// This is a run command (repeating value)
			int count = (commandByte >> 5) + 1;
			int16 greenValue = 32 * (commandByte & 0x1F); // Shift to green position (bits 5-9)

			// Output repeated value
			for (int i = 0; i < count; i++) {
				*((int16 *)outBuffer) |= greenValue;
				outBuffer += 2;
			}
		}
	}

	_bgDecompOutBufferTemp = outBuffer;
}

void GraphicsManager::decompB(byte *data, int32 size) {
	int remainingSize = size;

	while (remainingSize > 0) {
		// Determine the next command byte
		byte commandByte;
		if (_bgDecompFlags & 0x10) {
			// Use saved byte from previous incomplete decompression
			_bgDecompFlags &= ~0x10;
			commandByte = _bgLastCompItem;
		} else {
			// Read next byte from input
			commandByte = *data;

			data++;
			remainingSize--;
		}

		// Check if this is a copy command
		if ((commandByte & 0x80) != 0) {
			// Check if we have enough data to process
			if (remainingSize == 0) {
				// Save state for next call
				_bgDecompFlags |= 0x10;
				_bgLastCompItem = commandByte;
				return;
			}

			// Read offset byte and decrement remaining size
			int offsetByte = *data;
			data++;
			remainingSize--;

			// Calculate parameters for copy command
			int count = ((commandByte & 0x70) >> 4) + 3;
			byte *sourcePtr = &_bgDecompOutBufferTemp[2 * (((commandByte & 0xF) << 8) + offsetByte) - 0x2000];

			// Copy blue component from previous output
			for (int i = 0; i < count; i++) {
				*_bgDecompOutBufferTemp = *sourcePtr;

				_bgDecompOutBufferTemp += 2;
				sourcePtr += 2;
			}
		} else {
			// This is a run command (repeating value)
			int count = (commandByte >> 5) + 1;
			byte blueValue = commandByte & 0x1F; // Blue component (bits 0-4)

			// Output repeated value
			for (int i = 0; i < count; i++) {
				*_bgDecompOutBufferTemp = blueValue;
				_bgDecompOutBufferTemp += 2;
			}
		}
	}
}

bool GraphicsManager::decomp16(byte *data, int32 size) {
	byte *srcBufData = data;
	int32 effSize = size;

	if (_bgDecompFlags == 0) {
		_bgDecompFlags = 0x1000;
#ifdef SCUMM_LITTLE_ENDIAN
		_bgDecompOutBufferTemp = (_bgDecompOutBuffer + 1); // High byte for red
#else
		_bgDecompOutBufferTemp = _bgDecompOutBuffer;       // Low byte for red on BE
#endif
		_bgDecompTargetRect->x = READ_LE_UINT32(data);
		_bgDecompTargetRect->y = READ_LE_UINT32(data + 1 * sizeof(uint32));
		_bgDecompTargetRect->width = READ_LE_UINT32(data + 2 * sizeof(uint32));
		_bgDecompTargetRect->height = READ_LE_UINT32(data + 3 * sizeof(uint32));
		_bgOutChannelDataSizes[0] = READ_LE_UINT32(data + 4 * sizeof(uint32));
		_bgOutChannelDataSizes[1] = READ_LE_UINT32(data + 5 * sizeof(uint32));
		_bgOutChannelDataSizes[2] = READ_LE_UINT32(data + 6 * sizeof(uint32));

		srcBufData = data + 28;
		effSize = size - 28;
	}

	while (effSize > 0) {
		int32 channel = _bgDecompFlags & 3;

		int32 remainingDataSize = _bgOutChannelDataSizes[channel];
		if (effSize < remainingDataSize) {
			remainingDataSize = effSize;
		} else {
			_bgDecompFlags |= 0x100;
		}

		_bgOutChannelDataSizes[channel] -= remainingDataSize;

		if (channel == 0) {
			decompR(srcBufData, remainingDataSize);
		} else if (channel == 1) {
			decompB(srcBufData, remainingDataSize);
		} else if (channel == 2) {
			decompG(srcBufData, remainingDataSize);
		}

		srcBufData += remainingDataSize;
		effSize -= remainingDataSize;

		if ((_bgDecompFlags & 0x100) != 0) {
			_bgDecompFlags &= ~0x100;

			_bgDecompOutBufferTemp = _bgDecompOutBuffer;

#ifndef SCUMM_LITTLE_ENDIAN
			if ((_bgDecompFlags & 3) == 0)
				_bgDecompOutBufferTemp++;
#endif

			_bgDecompFlags++; // Go to the next channel

			if ((_bgDecompFlags & 3) == 3)
				return false;
		}
	}

	return true;
}

void GraphicsManager::drawItem(int cursor, int32 x, int32 y) {
	PixMap *cursorPtr = &_iconsBitmapData[1024 * cursor];

	if (acquireSurface()) {
		PixMap *screenPtr = (PixMap *)_screenSurface.getPixels() + (640 * y + x);

		// Draw the item row by row...
		for (int row = 0; row < 32; row++) {
			memcpy(screenPtr, cursorPtr, 32 * sizeof(PixMap));

			cursorPtr += 32;
			screenPtr += 640;
		}

		unlockSurface();
	}
}

void GraphicsManager::drawItemDim(int cursor, int32 x, int32 y, int brightness) {
	PixMap *cursorPtr = &_iconsBitmapData[1024 * cursor];

	if (acquireSurface()) {
		PixMap *screenPtr = (PixMap *)_screenSurface.getPixels() + (640 * y + x);

		// Draw the item row by row, with brightness adjustment...
		for (int row = 0; row < 32; row++) {
			for (int col = 0; col < 32; col++) {
				// Apply brightness mask and shift to dim the pixel...
				*screenPtr = (*cursorPtr & _brightnessData[brightness]) >> brightness;

				screenPtr++;
				cursorPtr++;
			}

			screenPtr += (640 - 32);
		}

		unlockSurface();
	}
}

void GraphicsManager::drawMouse() {
	_mouseScreenBufStart = _mouseAreaRect.x + 640 * _mouseAreaRect.y;
	storeMouse();

	if (acquireSurface()) {
		PixMap *cursorPtr = &_iconsBitmapData[1024 * _engine->_cursorType + 32 * _engine->_cursorYOffscreenOffset] + _engine->_cursorXOffscreenOffset;

		PixMap *screenPtr = (PixMap *)_screenSurface.getPixels() + _mouseScreenBufStart;

		// Draw cursor row by row...
		for (int row = 0; row < _mouseAreaRect.height; row++) {
			for (int col = 0; col < _mouseAreaRect.width; col++) {
				if (*cursorPtr) {
					*screenPtr = *cursorPtr;
				}

				cursorPtr++;
				screenPtr++;
			}

			cursorPtr += (32 - _mouseAreaRect.width);
			screenPtr += (640 - _mouseAreaRect.width);
		}

		unlockSurface();
	}
}

void GraphicsManager::storeMouse() {
	if (acquireSurface()) {
		PixMap *mousePtr = _mouseBackBuffer;
		PixMap *screenPtr = (PixMap *)_screenSurface.getPixels() + _mouseScreenBufStart;

		for (int row = 0; row < _mouseAreaRect.height; row++) {
			memcpy(mousePtr, screenPtr, 2 * _mouseAreaRect.width);
			mousePtr += 32;
			screenPtr += 640;
		}

		unlockSurface();
	}
}

void GraphicsManager::restoreMouse() {
	if (acquireSurface()) {
		PixMap *mousePtr = _mouseBackBuffer;
		PixMap *screenPtr = (PixMap *)_screenSurface.getPixels() + _mouseScreenBufStart;

		for (int row = 0; row < _mouseAreaRect.height; row++) {
			memcpy(screenPtr, mousePtr, 2 * _mouseAreaRect.width);
			mousePtr += 32;
			screenPtr += 640;
		}

		unlockSurface();
	}
}

void GraphicsManager::newMouseLoc() {
	if (_engine->_cursorX > 639)
		_engine->_cursorX = 639;

	if (_engine->_cursorY > 479)
		_engine->_cursorY = 479;

	CursorHeader *cursorHotspot = &_cursorsDataHeader[_engine->_cursorType];

	if (_engine->_cursorX - cursorHotspot->hotspotX >= 0) {
		_engine->_cursorXOffscreenOffset = 0;
		_mouseAreaRect.x = _engine->_cursorX - cursorHotspot->hotspotX;
	} else {
		_mouseAreaRect.x = 0;
		_engine->_cursorXOffscreenOffset = cursorHotspot->hotspotX - _engine->_cursorX;
	}
	if (_engine->_cursorY - cursorHotspot->hotspotY >= 0) {
		_engine->_cursorYOffscreenOffset = 0;
		_mouseAreaRect.y = _engine->_cursorY - cursorHotspot->hotspotY;
	} else {
		_mouseAreaRect.y = 0;
		_engine->_cursorYOffscreenOffset = cursorHotspot->hotspotY - _engine->_cursorY;
	}

	if (640 - _mouseAreaRect.x >= (32 - _engine->_cursorXOffscreenOffset)) {
		_mouseAreaRect.width = 32 - _engine->_cursorXOffscreenOffset;
	} else {
		_mouseAreaRect.width = 640 - _mouseAreaRect.x;
	}

	if (480 - _mouseAreaRect.y >= (32 - _engine->_cursorYOffscreenOffset)) {
		_mouseAreaRect.height = 32 - _engine->_cursorYOffscreenOffset;
	} else {
		_mouseAreaRect.height = 480 - _mouseAreaRect.y;
	}
}

void GraphicsManager::burstMouseArea(bool updateScreen) {
	burstBox(_mouseAreaRect.x, _mouseAreaRect.y, _mouseAreaRect.width, _mouseAreaRect.height, updateScreen);
}

void GraphicsManager::burstBox(int32 x, int32 y, int32 width, int32 height, bool updateScreen) {
	bool subtitlesDrawn = false;
	bool mouseDrawn = false;

	if (x < 640 && y < 480 && x + width <= 640 && y + height <= 480) {
		if ((_engine->getSubtitleManager()->_flags & 1) != 0 &&
			x + width >= 80 && x <= 560 && y + height >= 420 && y <= 458) {
			_engine->getSubtitleManager()->vSubOn();
			subtitlesDrawn = true;
		}

		if (canDrawMouse() &&
			_mouseAreaRect.x <= x + width &&
			_mouseAreaRect.x + _mouseAreaRect.width >= x &&
			_mouseAreaRect.y <= y + height &&
			_mouseAreaRect.y + _mouseAreaRect.height >= y) {
			drawMouse();
			mouseDrawn = true;
		}

		g_system->copyRectToScreen(_screenSurface.getBasePtr(x, y), 640 * 2, x, y, width, height);

		// The original always draws to screen. The check here is to prevent redrawing the mouse to screen
		// when the mouse is disabled, something which happens constantly in the original and causes flicker
		// on slower platforms.
		if (updateScreen) {
			g_system->updateScreen();
		}

		if (mouseDrawn)
			restoreMouse();

		if (subtitlesDrawn)
			_engine->getSubtitleManager()->vSubOff();
	}
}

bool GraphicsManager::canDrawMouse() {
	return _canDrawMouse;
}

void GraphicsManager::setMouseDrawable(bool drawable) {
	_canDrawMouse = drawable;
}

} // End of namespace LastExpress
