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

	byte rSrc, gSrc, bSrc;
	byte rDst, gDst, bDst;
	int numColors = _vm->getPaletteColorCount();

	Common::Rect t = Common::Rect(spriteSurface.w, spriteSurface.h);
	t.moveTo(drawRect.left, drawRect.top);

	Common::Point maskOrigin(MAX(0, -drawRect.left), MAX(0, -drawRect.top));
	drawRect.clip(Common::Rect(_maskSurface->w, _maskSurface->h));

	Graphics::ManagedSurface *castMask = nullptr;
	if (ink == kInkTypeMask) {
		Cast *member = g_director->getCastMember(_channels[spriteId]->_sprite->_castId + 1);

		if (!member->_widget)
			member->createWidget();

		if (member->_initialRect == _channels[spriteId]->_sprite->_cast->_initialRect)
			castMask = member->_widget->getSurface();
	}

	// HACK: A custom blitter is needed for the logical AND necessary here;
	// surface class doesn't provide it.
	for (int ii = 0; ii < drawRect.height(); ii++) {
		const byte *msk = castMask ? (const byte *)castMask->getBasePtr(maskOrigin.x, maskOrigin.y + ii) : nullptr;
		const byte *src = (const byte *)maskSurface->getBasePtr(maskOrigin.x, maskOrigin.y + ii);
		byte *dst = (byte *)_maskSurface->getBasePtr(t.left + maskOrigin.x, t.top + maskOrigin.y + ii);

		for (int j = 0; j < drawRect.width(); j++, src++, dst++) {
			*dst &= *src;

			if (msk)
				*dst = (*(msk++) ? 0 : *dst);
		}
	}

	// TODO: Merge these two into the switch logic that is below
	if (ink == kInkTypeMatte) {
		Common::Rect spriteRect(spriteSurface.w, spriteSurface.h);
		drawMatteSprite(spriteSurface, t);
		return;
	} else if (ink == kInkTypeReverse) {
		drawReverseSprite(spriteSurface, t, spriteId);
		return;
	}

	for (int ii = 0; ii < drawRect.height(); ii++) {
		const byte *msk = (const byte *)_maskSurface->getBasePtr(t.left + maskOrigin.x, t.top + maskOrigin.y + ii);
		const byte *src = (const byte *)spriteSurface.getBasePtr(maskOrigin.x, ii + maskOrigin.y);
		byte *dst = (byte *)_surface->getBasePtr(t.left + maskOrigin.x, t.top + maskOrigin.y + ii);

		for (int j = 0; j < drawRect.width(); j++, msk++, src++, dst++) {
			if (*msk) {
				_vm->_wm->decomposeColor(*src, rSrc, gSrc, bSrc);
				_vm->_wm->decomposeColor(*dst, rDst, gDst, bDst);

				switch (ink) {
				case kInkTypeBackgndTrans:
					if (*src == _channels[spriteId]->_sprite->_backColor)
						break;
					// fall through
				case kInkTypeCopy:
				case kInkTypeMask:
					*dst = *src;
					break;
				case kInkTypeTransparent:
					// FIXME: Is colour to ignore always white (last entry in pallette)?
					if (*src != numColors - 1)
						*dst &= *src;
					break;
				case kInkTypeReverse:
					if (*src != numColors - 1)
						*dst ^= *src;
					break;
				case kInkTypeGhost:
					if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0) && *src != numColors - 1)
						*dst = *dst | ~(*src);
					break;
				case kInkTypeNotCopy:
					if (*src != numColors - 1)
						*dst = ~(*src);
					break;
				case kInkTypeNotTrans:
					if (*src != numColors - 1)
						*dst = *dst & ~(*src);
					break;
				case kInkTypeNotReverse:
					if (*src != numColors - 1)
						*dst = *dst ^ *src;
					break;
				case kInkTypeNotGhost:
					if (*src != numColors - 1)
						*dst = *dst | *src;
					break;
				case kInkTypeMatte:
					break;
					// Arithmetic ink types
				case kInkTypeBlend:
					if (*src != numColors - 1)
						*dst = _vm->_wm->findBestColor((rSrc + rDst) / 2, (gSrc + gDst) / 2, (bSrc + bDst) / 2);
					break;
				case kInkTypeAddPin:
					if (*src != numColors - 1)
						*dst = _vm->_wm->findBestColor(MIN((rSrc + rDst), numColors - 1), MIN((gSrc + gDst), numColors - 1), MIN((bSrc + bDst), numColors - 1));
					break;
				case kInkTypeAdd:
					if (*src != numColors - 1)
						*dst = _vm->_wm->findBestColor(abs(rSrc + rDst) % numColors, abs(gSrc + gDst) % numColors, abs(bSrc + bDst) % numColors);
					break;
				case kInkTypeSubPin:
					if (*src != numColors - 1)
						*dst = _vm->_wm->findBestColor(MAX(rSrc - rDst, 0), MAX(gSrc - gDst, 0), MAX(bSrc - bDst, 0));
					break;
				case kInkTypeLight:
					if (*src != numColors - 1)
						*dst = _vm->_wm->findBestColor(MAX(rSrc, rDst), MAX(gSrc, gDst), MAX(bSrc, bDst));
					break;
				case kInkTypeSub:
					if (*src != numColors - 1)
						*dst = _vm->_wm->findBestColor(abs(rSrc - rDst) % numColors, abs(gSrc - gDst) % numColors, abs(bSrc - bDst) % numColors);
					break;
				case kInkTypeDark:
					if (*src != numColors - 1)
						*dst = _vm->_wm->findBestColor(MIN(rSrc, rDst), MIN(gSrc, gDst), MIN(bSrc, bDst));
					break;
				}
			}
		}
	}
}

void Score::drawReverseSprite(const Graphics::Surface &sprite, Common::Rect &drawRect, uint16 spriteId) {
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < drawRect.height(); ii++) {
		const byte *msk = (const byte *)_maskSurface->getBasePtr(drawRect.left, drawRect.top + ii);
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + ii);
		byte srcColor = *src;

		for (int j = 0; j < drawRect.width(); j++, msk++, src++, dst++) {
			if (*msk != 0) {
				if (!_channels[spriteId]->_sprite->_cast || _channels[spriteId]->_sprite->_cast->_type == kCastShape)
					srcColor = 0x0;
				else
					srcColor = *src;
				uint16 targetSprite = getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii));
				if ((targetSprite != 0)) {
					// TODO: This entire reverse colour attempt needs a lot more testing on
					// a lot more colour depths.
					if (srcColor != skipColor) {
						if (!_channels[targetSprite]->_sprite->_cast || _channels[targetSprite]->_sprite->_cast->_type != kCastBitmap) {
							if (*dst == 0 || *dst == 255) {
								*dst = _vm->transformColor(*dst);
							} else if (srcColor == 255 || srcColor == 0) {
								*dst = _vm->transformColor(*dst - 40);
							} else {
								*dst = _vm->transformColor(*src - 40);
							}
						} else {
							if (*dst == 0 && _vm->getVersion() == 3 &&
									_channels[spriteId]->_sprite->_cast->_type == kCastBitmap &&
									((BitmapCast*)_channels[spriteId]->_sprite->_cast)->_bitsPerPixel > 1) {
								*dst = _vm->transformColor(*src - 40);
							} else {
								*dst ^= _vm->transformColor(srcColor);
							}
						}
					}
				} else if (srcColor != skipColor) {
					*dst = _vm->transformColor(srcColor);
				}
			}
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
			const byte *msk = (const byte *)_maskSurface->getBasePtr(drawRect.left, drawRect.top + yy);
			const byte *src = (const byte *)tmp.getBasePtr(srcRect.left, srcRect.top + yy);
			byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + yy);

			for (int xx = 0; xx < drawRect.width(); xx++, src++, dst++, msk++)
				if (*msk != 0)
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
