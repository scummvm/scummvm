/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "director/director.h"
#include "director/cast.h"
#include "director/score.h"
#include "director/sprite.h"

namespace Director {

void Score::inkBasedBlit(Graphics::ManagedSurface *maskSurface, const Graphics::Surface &spriteSurface, InkType ink, Common::Rect drawRect, uint spriteId) {
	// drawRect could be bigger than the spriteSurface. Clip it
	Common::Rect t(spriteSurface.w, spriteSurface.h);
	t.moveTo(drawRect.left, drawRect.top);

	drawRect.clip(Common::Rect(_maskSurface->w, _maskSurface->h));
	maskSurface->transBlitFrom(*_maskSurface, drawRect, Common::Point(0, 0), _stageColor);

	switch (ink) {
	case kInkTypeCopy:
			_surface->transBlitFrom(spriteSurface, Common::Point(t.left, t.top), *maskSurface);
		break;
	case kInkTypeTransparent:
		// FIXME: is it always white (last entry in pallette)?
		_surface->transBlitFrom(spriteSurface, Common::Point(t.left, t.top), _vm->getPaletteColorCount() - 1);
		break;
	case kInkTypeBackgndTrans:
		drawBackgndTransSprite(spriteSurface, t, spriteId);
		break;
	case kInkTypeMatte:
		drawMatteSprite(spriteSurface, t);
		break;
	case kInkTypeGhost:
		drawGhostSprite(spriteSurface, t);
		break;
	case kInkTypeReverse:
		drawReverseSprite(spriteSurface, t, spriteId);
		break;
	default:
		warning("Score::inkBasedBlit(): Unhandled ink type %d", ink);
		_surface->blitFrom(spriteSurface, Common::Point(t.left, t.top));
		break;
	}

}

void Score::drawBackgndTransSprite(const Graphics::Surface &sprite, Common::Rect &drawRect, int spriteId) {
	byte skipColor = _sprites[spriteId]->_backColor;
	if (_sprites[spriteId]->_castType == kCastText && _sprites[spriteId]->_cast) {
		skipColor = ((TextCast *)_sprites[spriteId]->_cast)->_bgcolor;
	}

	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	for (int ii = 0; ii < drawRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < drawRect.width(); j++) {
			if (*src != skipColor)
				*dst = *src;

			src++;
			dst++;
		}
	}
}

void Score::drawGhostSprite(const Graphics::Surface &sprite, Common::Rect &drawRect) {
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < drawRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < drawRect.width(); j++) {
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0) && (*src != skipColor))
				*dst = (_vm->getPaletteColorCount() - 1) - *src; // Oposite color

			src++;
			dst++;
		}
	}
}

void Score::drawReverseSprite(const Graphics::Surface &sprite, Common::Rect &drawRect, uint16 spriteId) {
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < drawRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + ii);
		byte srcColor = *src;

		for (int j = 0; j < drawRect.width(); j++) {
			if (!_sprites[spriteId]->_cast || _sprites[spriteId]->_cast->_type == kCastShape)
				srcColor = 0x0;
			else
				srcColor = *src;
			uint16 targetSprite = getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii));
			if ((targetSprite != 0)) {
				// TODO: This entire reverse colour attempt needs a lot more testing on
				// a lot more colour depths.
				if (srcColor != skipColor) {
					if (!_sprites[targetSprite]->_cast || _sprites[targetSprite]->_cast->_type != kCastBitmap) {
						if (*dst == 0 || *dst == 255) {
							*dst = _vm->transformColor(*dst);
						} else if (srcColor == 255 || srcColor == 0) {
							*dst = _vm->transformColor(*dst - 40);
						} else {
							*dst = _vm->transformColor(*src - 40);
						}
					} else {
						if (*dst == 0 && _vm->getVersion() == 3 &&
							((BitmapCast*)_sprites[spriteId]->_cast)->_bitsPerPixel > 1) {
							*dst = _vm->transformColor(*src - 40);
						} else {
							*dst ^= _vm->transformColor(srcColor);
						}
					}
				}
			} else if (srcColor != skipColor) {
				*dst = _vm->transformColor(srcColor);
			}
			src++;
			dst++;
		}
	}
}

void Score::drawMatteSprite(const Graphics::Surface &sprite, Common::Rect &drawRect) {
	// Like background trans, but all white pixels NOT ENCLOSED by coloured pixels are transparent
	Graphics::Surface tmp;
	tmp.copyFrom(sprite);
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	// Searching white color in the corners
	int whiteColor = -1;

	for (int y = 0; y < tmp.h; y++) {
		for (int x = 0; x < tmp.w; x++) {
			byte color = *(byte *)tmp.getBasePtr(x, y);

			if (_vm->getPalette()[color * 3 + 0] == 0xff &&
				_vm->getPalette()[color * 3 + 1] == 0xff &&
				_vm->getPalette()[color * 3 + 2] == 0xff) {
				whiteColor = color;
				break;
			}
		}
	}

	if (whiteColor == -1) {
		debugC(1, kDebugImages, "Score::drawMatteSprite(): No white color for Matte image");

		for (int yy = 0; yy < drawRect.height(); yy++) {
			const byte *src = (const byte *)tmp.getBasePtr(srcRect.left, srcRect.top + yy);
			byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + yy);

			for (int xx = 0; xx < drawRect.width(); xx++, src++, dst++)
				*dst = *src;
		}
	} else {
		Graphics::FloodFill ff(&tmp, whiteColor, 0, true);

		for (int yy = 0; yy < tmp.h; yy++) {
			ff.addSeed(0, yy);
			ff.addSeed(tmp.w - 1, yy);
		}

		for (int xx = 0; xx < tmp.w; xx++) {
			ff.addSeed(xx, 0);
			ff.addSeed(xx, tmp.h - 1);
		}
		ff.fillMask();

		for (int yy = 0; yy < drawRect.height(); yy++) {
			const byte *src = (const byte *)tmp.getBasePtr(srcRect.left, srcRect.top + yy);
			const byte *mask = (const byte *)ff.getMask()->getBasePtr(srcRect.left, srcRect.top + yy);
			byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + yy);

			for (int xx = 0; xx < drawRect.width(); xx++, src++, dst++, mask++)
				if (*mask == 0)
					*dst = *src;
		}
	}

	tmp.free();
}

} // End of namespace Director
