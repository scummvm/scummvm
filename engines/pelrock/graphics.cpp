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

#include "common/scummsys.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "graphics.h"
#include "pelrock/graphics.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

GraphicsManager::GraphicsManager() {
}

GraphicsManager::~GraphicsManager() {
}

Common::Point GraphicsManager::showOverlay(int height, Graphics::ManagedSurface &buf) {
	int overlayY = 400 - height;
	int overlayX = 0;
	for (int x = 0; x < 640; x++) {
		for (int y = overlayY; y < 400; y++) {
			byte pixel = (byte)buf.getPixel(x, y);
			buf.setPixel(x, y, g_engine->_room->_paletteRemaps[2][pixel]);
		}
	}
	return Common::Point(overlayX, overlayY);
}

void GraphicsManager::fadeToBlack(int stepSize) {
	byte palette[768];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	while (!g_engine->shouldQuit()) {
		g_engine->_events->pollEvent();
		g_engine->_chrono->updateChrono();
		if (g_engine->_chrono->_gameTick) {

			for (int i = 0; i < 768; i++) {
				if (palette[i] > 0) {
					palette[i] = MAX(palette[i] - stepSize, 0);
				}
			}
			g_system->getPaletteManager()->setPalette(palette, 0, 256);

			bool allBlack = true;
			for (int i = 0; i < 768; i++) {
				if (palette[i] != 0) {
					allBlack = false;
				}
			}

			if (allBlack) {
				break;
			}

			g_engine->_screen->markAllDirty();
			g_engine->_screen->update();
		}
		g_system->delayMillis(10);
	}
}

/**
 * Fades between two palettes by incrementally changing the current palette towards the target palette.
 */
void GraphicsManager::fadePaletteToTarget(byte *targetPalette, int stepSize) {
	byte currentPalette[768];
	memcpy(currentPalette, g_engine->_room->_roomPalette, 768);

	while (!g_engine->shouldQuit()) {
		g_engine->_events->pollEvent();

		bool didRender = g_engine->renderScene(OVERLAY_NONE);
		if (didRender) {
			bool changed = false;

			for (int i = 0; i < 768; i++) {
				if (currentPalette[i] < targetPalette[i]) {
					currentPalette[i] = MIN((int)currentPalette[i] + stepSize, (int)targetPalette[i]);
					changed = true;
				} else if (currentPalette[i] > targetPalette[i]) {
					currentPalette[i] = MAX((int)currentPalette[i] - stepSize, (int)targetPalette[i]);
					changed = true;
				}
			}

			if (!changed)
				break;

			g_system->getPaletteManager()->setPalette(currentPalette, 0, 256);
		}

		g_engine->_screen->update();
		g_system->delayMillis(10);
	}

	memcpy(g_engine->_room->_roomPalette, targetPalette, 768);
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
}

void GraphicsManager::clearScreen() {
	g_engine->_screen->clear(0);
}

void GraphicsManager::drawColoredText(Graphics::ManagedSurface *screen, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font) {
	int currentX = x;

	Common::String segment;
	for (uint i = 0; i < text.size(); i++) {
		if (text[i] == '@' && i + 1 < text.size()) {
			// Draw accumulated segment
			if (!segment.empty()) {
				font->drawString(screen, segment, currentX, y, w, defaultColor);
				currentX += font->getStringWidth(segment);
				segment.clear();
			}
			defaultColor = text[i + 1];
			i++; // skip color code
		} else {
			segment += text[i];
		}
	}

	// Draw remaining segment
	if (!segment.empty()) {
		font->drawString(screen, segment, currentX, y, w, defaultColor);
	}
}

void GraphicsManager::drawColoredText(Graphics::ManagedSurface &buf, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font) {

	Graphics::Surface tempSurface;
	Common::Rect r = font->getBoundingBox(text); // Ensure font metrics are loaded before creating surface

	tempSurface.create(r.width(), r.height(), Graphics::PixelFormat::createFormatCLUT8());

	int currentX = x;

	Common::String segment;
	for (uint i = 0; i < text.size(); i++) {
		if (text[i] == '@' && i + 1 < text.size()) {
			// Draw accumulated segment
			if (!segment.empty()) {
				font->drawString(&tempSurface, segment, currentX, y, w, defaultColor);
				currentX += font->getStringWidth(segment);
				segment.clear();
			}
			defaultColor = text[i + 1];
			i++; // skip color code
		} else {
			segment += text[i];
		}
	}

	// Draw remaining segment
	if (!segment.empty()) {
		font->drawString(&tempSurface, segment, currentX, y, w, defaultColor);
	}

	// Use transBlitFrom to blit non-zero pixels
	buf.transBlitFrom(tempSurface, Common::Point(x, y), 0);
	tempSurface.free();
}

void GraphicsManager::drawColoredTexts(Graphics::ManagedSurface *surface, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font) {
	int currentX = x;
	byte currentColor = 255;

	for (uint i = 0; i < text.size(); i++) {
		drawColoredText(surface, text[i], currentX, y + i * (font->getFontHeight() + yPadding), w, currentColor, font);
	}
}

void GraphicsManager::drawColoredTexts(Graphics::ManagedSurface &buf, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font) {
	int currentX = x;
	byte currentColor = 255;

	for (uint i = 0; i < text.size(); i++) {
		drawColoredText(buf, text[i], currentX, y + i * (font->getFontHeight() + yPadding), w, currentColor, font);
	}
}

void GraphicsManager::copyBackgroundToBuffer() {
	g_engine->_compositeBuffer.blitFrom(g_engine->_currentBackground);
}

void GraphicsManager::presentFrame() {
	g_engine->_screen->blitFrom(g_engine->_compositeBuffer);
	// g_engine->paintDebugLayer();
	g_engine->_screen->markAllDirty();
}

void GraphicsManager::updatePaletteAnimations() {
	if (g_engine->_room->_currentPaletteAnim != nullptr) {
		if (g_engine->_room->_currentPaletteAnim->paletteMode == 1) {
			animateFadePalette(g_engine->_room->_currentPaletteAnim);
		} else {
			animateRotatePalette(g_engine->_room->_currentPaletteAnim);
		}
	}
}

void GraphicsManager::animateFadePalette(PaletteAnim *anim) {
	// FADE palette animation - cycles a single palette entry between min/max RGB values
	// Data layout (after loading from EXE):
	//   data[0] = current R
	//   data[1] = current G
	//   data[2] = current B
	//   data[3] = min R
	//   data[4] = min G
	//   data[5] = min B
	//   data[6] = max R
	//   data[7] = max G
	//   data[8] = max B
	//   data[9] = flags byte:
	//             bits 0-1: R increment
	//             bits 2-3: G increment
	//             bits 4-5: B increment
	//             bit 6: direction (0=decreasing toward min, 1=increasing toward max)

	byte flags = anim->data[9];
	// Increments are scaled by 4 (<<2) to match the shifted RGB values
	byte rInc = (flags & 0x03) << 2;
	byte gInc = ((flags >> 2) & 0x03) << 2;
	byte bInc = ((flags >> 4) & 0x03) << 2;
	bool increasing = (flags & 0x40) != 0;

	if (increasing) {
		// Increasing toward max values
		anim->data[0] += rInc;
		anim->data[1] += gInc;
		anim->data[2] += bInc;
		// Check if R reached max, then reverse direction
		if (anim->data[0] >= anim->data[6]) {
			anim->data[9] &= ~0x40; // Clear direction bit
		}
	} else {
		// Decreasing toward min values
		anim->data[0] -= rInc;
		anim->data[1] -= gInc;
		anim->data[2] -= bInc;
		// Check if R reached min, then reverse direction
		if (anim->data[0] <= anim->data[3]) {
			anim->data[9] |= 0x40; // Set direction bit
		}
	}

	g_engine->_room->_roomPalette[anim->startIndex * 3] = anim->data[0];
	g_engine->_room->_roomPalette[anim->startIndex * 3 + 1] = anim->data[1];
	g_engine->_room->_roomPalette[anim->startIndex * 3 + 2] = anim->data[2];
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
}

void GraphicsManager::animateRotatePalette(PaletteAnim *anim) {
	if (anim->curFrame >= anim->data[1]) {
		anim->curFrame = 0;
		int colors = anim->paletteMode;
		byte *paletteValues = new byte[colors * 3];
		for (int i = 0; i < colors; i++) {
			paletteValues[i * 3] = g_engine->_room->_roomPalette[(anim->startIndex + i) * 3];
			paletteValues[i * 3 + 1] = g_engine->_room->_roomPalette[(anim->startIndex + i) * 3 + 1];
			paletteValues[i * 3 + 2] = g_engine->_room->_roomPalette[(anim->startIndex + i) * 3 + 2];
		}
		for (int i = 0; i < colors; i++) {
			int srcIndex = (i + 1) % colors;
			g_engine->_room->_roomPalette[(anim->startIndex + i) * 3] = paletteValues[srcIndex * 3];
			g_engine->_room->_roomPalette[(anim->startIndex + i) * 3 + 1] = paletteValues[srcIndex * 3 + 1];
			g_engine->_room->_roomPalette[(anim->startIndex + i) * 3 + 2] = paletteValues[srcIndex * 3 + 2];
		}
		delete[] paletteValues;

		g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);

	} else {
		anim->curFrame++;
	}
}

void GraphicsManager::placeStickersFirstPass() {
	// also place temporary stickers
	for (uint i = 0; i < g_engine->_room->_roomStickers.size(); i++) {
		Sticker sticker = g_engine->_room->_roomStickers[i];
		placeSticker(sticker);
	}
}

void GraphicsManager::placeStickersSecondPass() {
	// Some stickers need to be placed AFTER sprites, hardcoded in the original
	if (g_engine->_room->_currentRoomNumber == 3) {
		for (uint i = 0; i < g_engine->_state->stickersPerRoom[3].size(); i++) {
			if (g_engine->_state->stickersPerRoom[3][i].stickerIndex == 14) {
				placeSticker(g_engine->_state->stickersPerRoom[3][i]);
				break;
			}
		}
	}
}

void GraphicsManager::placeSticker(Sticker sticker) {
	// Wrap sticker data as a surface and blit (no transparency - all pixels copied)
	Graphics::Surface stickerSurf;
	stickerSurf.init(sticker.w, sticker.h, sticker.w, sticker.stickerData, Graphics::PixelFormat::createFormatCLUT8());
	// Clip to screen bounds
	Common::Rect destRect(sticker.x, sticker.y, sticker.x + sticker.w, sticker.y + sticker.h);
	Common::Rect screenRect(0, 0, 640, 400);
	destRect.clip(screenRect);
	if (destRect.isEmpty())
		return;
	Common::Rect srcRect(destRect.left - sticker.x, destRect.top - sticker.y,
						 destRect.right - sticker.x, destRect.bottom - sticker.y);
	g_engine->_compositeBuffer.blitFrom(stickerSurf, srcRect, Common::Point(destRect.left, destRect.top));
}

void GraphicsManager::reflectionEffect(byte *buf, int x, int y, int width, int height) {
	// Water reflection - draws mirrored sprite on water pixels
	// Only sprite pixels 0-15 are reflected (checked via pixel & 0xF0 == 0)
	for (int row = 0; row < height; row++) {
		int reflectY = y + row;

		if (reflectY >= 400)
			break; // Screen boundary

		for (int col = 0; col < width; col++) {
			byte pixel = buf[(height - 1 - row) * width + col]; // Read from bottom up for mirror
			// Only reflect pixels 0-15 (high nibble must be 0)
			if (pixel != 255 && (pixel & 0xF0) == 0) {
				int px = x + col;
				if (px >= 0 && px < 640) {
					byte bgPixel = (byte)g_engine->_compositeBuffer.getPixel(px, reflectY);
					if (bgPixel >= 223 && bgPixel < 228) { // Is water (0xDF-0xE3)
						g_engine->_compositeBuffer.setPixel(px, reflectY, g_engine->_room->_paletteRemaps[4][pixel]);
					}
				}
			}
		}
	}
}

void GraphicsManager::calculateScalingMasks() {
	for (int scaleFactor = 0; scaleFactor < kAlfredFrameWidth; scaleFactor++) {
		float step = kAlfredFrameWidth / (scaleFactor + 1.0f);
		Common::Array<int> row;
		float index = 0.0f;
		int sourcePixel = 0;

		while (index < kAlfredFrameWidth) {
			row.push_back(sourcePixel);
			index += step;
			sourcePixel += 1;
			if (sourcePixel >= kAlfredFrameWidth) {
				sourcePixel = kAlfredFrameWidth - 1;
			}
		}

		// Pad to exactly kAlfredFrameWidth entries
		while (row.size() < kAlfredFrameWidth) {
			row.push_back(row.empty() ? 0 : row[row.size() - 1]);
		}

		_widthScalingTable.push_back(row);
	}

	for (int scaleFactor = 0; scaleFactor < kAlfredFrameHeight; scaleFactor++) {
		float step = kAlfredFrameHeight / (scaleFactor + 1.0f);
		Common::Array<int> row;
		row.resize(kAlfredFrameHeight, 0);
		float position = step;
		int counter = 1;
		while (position < kAlfredFrameHeight) {
			int idx = static_cast<int>(round(position));
			if (idx < kAlfredFrameHeight) {
				row[idx] = counter;
				counter++;
			}
			position += step;
		}
		_heightScalingTable.push_back(row);
	}
}

ScaleCalculation GraphicsManager::calculateScaling(int yPos, ScalingParams scalingParams) {
	int scaleY = 0;
	int scaleX = 0;
	if (scalingParams.scaleMode == 0xFF) {
		// Maximum scaling - character is very small (used for bird's eye view maps)
		scaleY = 0x5e; // 94
		scaleX = 0x2f; // 47
	} else if (scalingParams.scaleMode == 0xFE) {
		// No scaling - full size character
		scaleY = 0;
		scaleX = 0;
	} else if (scalingParams.scaleMode == 0) {
		// Dynamic scaling based on Y position
		if (scalingParams.yThreshold < yPos) {
			// Below threshold - no scaling
			scaleY = 0;
			scaleX = 0;
		} else {
			if (scalingParams.scaleDivisor != 0) {
				scaleY = (scalingParams.yThreshold - yPos) / scalingParams.scaleDivisor;
				scaleX = scaleY / 2;
			} else {
				scaleY = 0;
				scaleX = 0;
			}
		}
	} else {
		scaleY = 0;
		scaleX = 0;
	}

	// Original game formula: actual dimensions = base - scale amount
	int finalHeight = kAlfredFrameHeight - scaleY;
	if (finalHeight < 1)
		finalHeight = 1;

	int finalWidth = kAlfredFrameWidth - scaleX;
	if (finalWidth < 1)
		finalWidth = 1;

	ScaleCalculation scaleCalc;
	scaleCalc.scaledHeight = finalHeight;
	scaleCalc.scaledWidth = finalWidth;
	scaleCalc.scaleY = scaleY;
	scaleCalc.scaleX = scaleX;
	return scaleCalc;
}

byte *GraphicsManager::scale(int scaleY, int finalWidth, int finalHeight, byte *buf) {
	// The scaling table is indexed by how many scanlines to skip (scaleY), not by final height
	int scaleIndex = scaleY;
	if (scaleIndex >= (int)_heightScalingTable.size()) {
		scaleIndex = _heightScalingTable.size() - 1;
	}
	if (scaleIndex < 0) {
		scaleIndex = 0;
	}
	int linesToSkip = kAlfredFrameHeight - finalHeight;

	byte *finalBuf = new byte[finalWidth * finalHeight];

	if (linesToSkip > 0) {
		int skipInterval = kAlfredFrameHeight / linesToSkip;
		Common::Array<float> idealSkipPositions;
		for (int i = 0; i < linesToSkip; i++) {
			float idealPos = (i + 0.5f) * skipInterval;
			idealSkipPositions.push_back(idealPos);
		}

		Common::Array<int> tableSkipPositions;
		for (int scanline = 0; scanline < kAlfredFrameHeight; scanline++) {
			if (_heightScalingTable[scaleIndex][scanline] != 0) {
				tableSkipPositions.push_back(scanline);
			}
		}

		Common::Array<int> skipTheseLines;
		for (size_t i = 0; i < idealSkipPositions.size(); i++) {
			float idealPos = idealSkipPositions[i];
			int closest = -1;
			int minDiff = INT32_MAX;
			for (size_t j = 0; j < tableSkipPositions.size(); j++) {
				int candidate = tableSkipPositions[j];
				int diff = static_cast<int>(abs(candidate - idealPos));
				if (diff < minDiff) {
					minDiff = diff;
					closest = candidate;
				}
			}
			if (closest != -1) {
				skipTheseLines.push_back(closest);
			}
			if (skipTheseLines.size() >= static_cast<size_t>(linesToSkip)) {
				break;
			}
		}

		int outY = 0;
		for (int srcY = 0; srcY < kAlfredFrameHeight; srcY++) {
			bool skipLine = false;
			for (size_t skipIdx = 0; skipIdx < skipTheseLines.size(); ++skipIdx) {
				if (skipTheseLines[skipIdx] == srcY) {
					skipLine = true;
					break;
				}
			}
			if (!skipLine) {
				for (int outX = 0; outX < finalWidth; outX++) {
					int srcX = static_cast<int>(outX * kAlfredFrameWidth / finalWidth);
					if (srcX >= kAlfredFrameWidth) {
						srcX = kAlfredFrameWidth - 1;
					}
					int srcIndex = srcY * kAlfredFrameWidth + srcX;
					int outIndex = outY * finalWidth + outX;
					if (outIndex >= finalWidth * finalHeight || srcIndex >= kAlfredFrameWidth * kAlfredFrameHeight) {
					} else
						finalBuf[outIndex] = buf[srcIndex];
				}
				outY++;
			}
		}
	} else {
		Common::copy(buf, buf + (kAlfredFrameWidth * kAlfredFrameHeight), finalBuf);
	}
	return finalBuf;
}

} // End of namespace Pelrock
