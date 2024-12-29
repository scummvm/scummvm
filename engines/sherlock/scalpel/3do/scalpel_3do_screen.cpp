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

#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/3do/scalpel_3do_screen.h"

namespace Sherlock {

namespace Scalpel {

Scalpel3DOScreen::Scalpel3DOScreen(SherlockEngine *vm): ScalpelScreen(vm) {
}

void Scalpel3DOScreen::SHblitFrom(const Graphics::Surface &src) {
	SHblitFrom(src, Common::Point(0, 0));
}

void Scalpel3DOScreen::SHblitFrom(const Graphics::Surface &src, const Common::Point &destPos) {
	SHblitFrom(src, destPos, Common::Rect(0, 0, src.w, src.h));
}

void Scalpel3DOScreen::SHblitFrom(const Graphics::Surface &src, const Common::Point &pt, const Common::Rect &srcBounds) {
	if (!_vm->_isScreenDoubled) {
		ScalpelScreen::SHblitFrom(src, pt, srcBounds);
		return;
	}

	Common::Rect srcRect = srcBounds;
	srcRect.clip(320, 200);
	Common::Rect destRect(pt.x, pt.y, pt.x + srcRect.width(), pt.y + srcRect.height());

	if (!srcRect.isValidRect() || !clip(srcRect, destRect))
		return;

	// Add dirty area remapped to the 640x200 surface
	addDirtyRect(Common::Rect(destRect.left * 2, destRect.top * 2, destRect.right * 2, destRect.bottom * 2));

	// Transfer the area, doubling each pixel
	for (int yp = 0; yp < srcRect.height(); ++yp) {
		const uint16 *srcP = (const uint16 *)src.getBasePtr(srcRect.left, srcRect.top + yp);
		uint16 *destP = (uint16 *)getBasePtr(destRect.left * 2, (destRect.top + yp) * 2);

		for (int xp = srcRect.left; xp < srcRect.right; ++xp, ++srcP, destP += 2) {
			*destP = *srcP;
			*(destP + 1) = *srcP;
			*(destP + 640) = *srcP;
			*(destP + 640 + 1) = *srcP;
		}
	}
}

void Scalpel3DOScreen::transBlitFromUnscaled(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped, int overrideColor) {
	error("TODO: Refactor");
#if 0
	if (!_vm->_isScreenDoubled) {
		ScalpelScreen::transBlitFromUnscaled(src, pt, flipped, overrideColor);
		return;
	}

	Common::Rect drawRect(0, 0, src.w, src.h);
	Common::Rect destRect(pt.x, pt.y, pt.x + src.w, pt.y + src.h);

	// Clip the display area to on-screen
	if (!clip(drawRect, destRect))
		// It's completely off-screen
		return;

	if (flipped)
		drawRect = Common::Rect(src.w - drawRect.right, src.h - drawRect.bottom,
		src.w - drawRect.left, src.h - drawRect.top);

	Common::Point destPt(destRect.left, destRect.top);
	addDirtyRect(Common::Rect(destPt.x * 2, destPt.y * 2, (destPt.x + drawRect.width()) * 2,
		(destPt.y + drawRect.height()) * 2));

	assert(src.format.bytesPerPixel == 2 && _surface.format.bytesPerPixel == 2);

	for (int yp = 0; yp < drawRect.height(); ++yp) {
		const uint16 *srcP = (const uint16 *)src.getBasePtr(
			flipped ? drawRect.right - 1 : drawRect.left, drawRect.top + yp);
		uint16 *destP = (uint16 *)getBasePtr(destPt.x * 2, (destPt.y + yp) * 2);

		for (int xp = 0; xp < drawRect.width(); ++xp, destP += 2) {
			// RGB 0, 0, 0 -> transparent on 3DO
			if (*srcP) {
				*destP = *srcP;
				*(destP + 1) = *srcP;
				*(destP + 640) = *srcP;
				*(destP + 640 + 1) = *srcP;
			}

			srcP = flipped ? srcP - 1 : srcP + 1;
		}
	}
#endif
}

void Scalpel3DOScreen::SHfillRect(const Common::Rect &r, uint color) {
	if (_vm->_isScreenDoubled)
		ScalpelScreen::fillRect(Common::Rect(r.left * 2, r.top * 2, r.right * 2, r.bottom * 2), color);
	else
		ScalpelScreen::fillRect(r, color);
}

void Scalpel3DOScreen::SHtransBlitFrom(const ImageFrame &src, const Common::Point &pt,
		bool flipped, int scaleVal) {
	ScalpelScreen::SHtransBlitFrom(src, pt, flipped,
		_vm->_isScreenDoubled ? scaleVal / 2 : scaleVal);
}

void Scalpel3DOScreen::SHtransBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped, int scaleVal) {
	ScalpelScreen::SHtransBlitFrom(src, pt, flipped,
		_vm->_isScreenDoubled ? scaleVal / 2 : scaleVal);
}


void Scalpel3DOScreen::fadeIntoScreen3DO(int speed) {
	Events &events = *_vm->_events;
	uint16 *currentScreenBasePtr = (uint16 *)getPixels();
	uint16 *targetScreenBasePtr = (uint16 *)_backBuffer.getPixels();
	uint16  currentScreenPixel = 0;
	uint16  targetScreenPixel = 0;

	uint16  currentScreenPixelRed = 0;
	uint16  currentScreenPixelGreen = 0;
	uint16  currentScreenPixelBlue = 0;

	uint16  targetScreenPixelRed = 0;
	uint16  targetScreenPixelGreen = 0;
	uint16  targetScreenPixelBlue = 0;

	uint16  screenWidth = SHERLOCK_SCREEN_WIDTH;
	uint16  screenHeight = SHERLOCK_SCREEN_HEIGHT;
	uint16  screenX = 0;
	uint16  screenY = 0;
	uint16  pixelsChanged = 0;

	clearDirtyRects();

	do {
		pixelsChanged = 0;
		uint16 *currentScreenPtr = currentScreenBasePtr;
		uint16 *targetScreenPtr = targetScreenBasePtr;

		for (screenY = 0; screenY < screenHeight; screenY++) {
			for (screenX = 0; screenX < screenWidth; screenX++) {
				currentScreenPixel = *currentScreenPtr;
				targetScreenPixel = *targetScreenPtr;

				if (currentScreenPixel != targetScreenPixel) {
					// pixel doesn't match, adjust accordingly
					currentScreenPixelRed = currentScreenPixel & 0xF800;
					currentScreenPixelGreen = currentScreenPixel & 0x07E0;
					currentScreenPixelBlue = currentScreenPixel & 0x001F;
					targetScreenPixelRed = targetScreenPixel & 0xF800;
					targetScreenPixelGreen = targetScreenPixel & 0x07E0;
					targetScreenPixelBlue = targetScreenPixel & 0x001F;

					if (currentScreenPixelRed != targetScreenPixelRed) {
						if (currentScreenPixelRed < targetScreenPixelRed) {
							currentScreenPixelRed += 0x0800;
						} else {
							currentScreenPixelRed -= 0x0800;
						}
					}
					if (currentScreenPixelGreen != targetScreenPixelGreen) {
						// Adjust +2/-2 because we are running RGB555 at RGB565
						if (currentScreenPixelGreen < targetScreenPixelGreen) {
							currentScreenPixelGreen += 0x0040;
						} else {
							currentScreenPixelGreen -= 0x0040;
						}
					}
					if (currentScreenPixelBlue != targetScreenPixelBlue) {
						if (currentScreenPixelBlue < targetScreenPixelBlue) {
							currentScreenPixelBlue += 0x0001;
						} else {
							currentScreenPixelBlue -= 0x0001;
						}
					}

					uint16 v = currentScreenPixelRed | currentScreenPixelGreen | currentScreenPixelBlue;
					*currentScreenPtr = v;
					if (_vm->_isScreenDoubled) {
						*(currentScreenPtr + 1) = v;
						*(currentScreenPtr + 640) = v;
						*(currentScreenPtr + 640 + 1) = v;
					}

					pixelsChanged++;
				}

				currentScreenPtr += _vm->_isScreenDoubled ? 2 : 1;
				targetScreenPtr++;
			}

			if (_vm->_isScreenDoubled)
				currentScreenPtr += 640;
		}

		// Too much considered dirty at the moment
		if (_vm->_isScreenDoubled)
			addDirtyRect(Common::Rect(0, 0, screenWidth * 2, screenHeight * 2));
		else
			addDirtyRect(Common::Rect(0, 0, screenWidth, screenHeight));

		events.pollEvents();
		events.delay(10 * speed);
	} while ((pixelsChanged) && (!_vm->shouldQuit()));
}

void Scalpel3DOScreen::blitFrom3DOcolorLimit(uint16 limitColor) {
	uint16 *currentScreenPtr = (uint16 *)getPixels();
	uint16 *targetScreenPtr = (uint16 *)_backBuffer.getPixels();
	uint16  currentScreenPixel = 0;

	uint16  screenWidth = SHERLOCK_SCREEN_WIDTH;
	uint16  screenHeight = SHERLOCK_SCREEN_HEIGHT;
	uint16  screenX = 0;
	uint16  screenY = 0;

	uint16  currentScreenPixelRed = 0;
	uint16  currentScreenPixelGreen = 0;
	uint16  currentScreenPixelBlue = 0;

	uint16  limitPixelRed = limitColor & 0xF800;
	uint16  limitPixelGreen = limitColor & 0x07E0;
	uint16  limitPixelBlue = limitColor & 0x001F;

	for (screenY = 0; screenY < screenHeight; screenY++) {
		for (screenX = 0; screenX < screenWidth; screenX++) {
			currentScreenPixel = *targetScreenPtr;

			currentScreenPixelRed = currentScreenPixel & 0xF800;
			currentScreenPixelGreen = currentScreenPixel & 0x07E0;
			currentScreenPixelBlue = currentScreenPixel & 0x001F;

			if (currentScreenPixelRed < limitPixelRed)
				currentScreenPixelRed = limitPixelRed;
			if (currentScreenPixelGreen < limitPixelGreen)
				currentScreenPixelGreen = limitPixelGreen;
			if (currentScreenPixelBlue < limitPixelBlue)
				currentScreenPixelBlue = limitPixelBlue;

			uint16 v = currentScreenPixelRed | currentScreenPixelGreen | currentScreenPixelBlue;
			*currentScreenPtr = v;
			if (_vm->_isScreenDoubled) {
				*(currentScreenPtr + 1) = v;
				*(currentScreenPtr + 640) = v;
				*(currentScreenPtr + 640 + 1) = v;
			}

			currentScreenPtr += _vm->_isScreenDoubled ? 2 : 1;
			targetScreenPtr++;
		}

		if (_vm->_isScreenDoubled)
			currentScreenPtr += 640;
	}

	// Too much considered dirty at the moment
	if (_vm->_isScreenDoubled)
		addDirtyRect(Common::Rect(0, 0, screenWidth * 2, screenHeight * 2));
	else
		addDirtyRect(Common::Rect(0, 0, screenWidth, screenHeight));
}

uint16 Scalpel3DOScreen::width() const {
	return _vm->_isScreenDoubled ? this->w / 2 : this->w;
}

uint16 Scalpel3DOScreen::height() const {
	return _vm->_isScreenDoubled ? this->h / 2 : this->h;
}

void Scalpel3DOScreen::rawBlitFrom(const Graphics::Surface &src, const Common::Point &pt) {
	Common::Rect srcRect(0, 0, src.w, src.h);
	Common::Rect destRect(pt.x, pt.y, pt.x + src.w, pt.y + src.h);

	addDirtyRect(destRect);
	copyRectToSurface(src, destRect.left, destRect.top, srcRect);
}

} // End of namespace Scalpel

} // End of namespace Sherlock
