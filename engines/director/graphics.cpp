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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/memstream.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/images.h"

namespace Director {

#include "director/graphics-data.h"

/**
 * The sprites colors are in reverse order with respect to the ids in director.
 * The palette is in reverse order, this eases the code for loading files.
 * All other color ids can be converted with: 255 - colorId.
 **/
uint32 DirectorEngine::transformColor(uint32 color) {
	if (_pixelformat.bytesPerPixel == 1)
		return 255 - color;

	color = 255 - color;

	return _wm->findBestColor(_currentPalette[color * 3], _currentPalette[color * 3 + 1], _currentPalette[color * 3 + 2]);
}

void DirectorEngine::loadPatterns() {
	for (int i = 0; i < ARRAYSIZE(director3Patterns); i++)
		_director3Patterns.push_back(director3Patterns[i]);

	for (int i = 0; i < ARRAYSIZE(director3QuickDrawPatterns); i++)
		_director3QuickDrawPatterns.push_back(director3QuickDrawPatterns[i]);

	_pixelformat = Graphics::PixelFormat::createFormatCLUT8();

	for (int i = 0; i < ARRAYSIZE(builtinTiles); i++) {
		Common::MemoryReadStream stream(builtinTiles[i].ptr, builtinTiles[i].size);

		_builtinTiles[i] = new BITDDecoder(builtinTiles[i].w, builtinTiles[i].h, 8, builtinTiles[i].w, macPalette, kFileVer300);
		_builtinTiles[i]->loadStream(stream);
	}
}

Graphics::MacPatterns &DirectorEngine::getPatterns() {
	// TOOD: implement switch and other version patterns. (use getVersion());
	return _director3QuickDrawPatterns;
}

void DirectorEngine::loadDefaultPalettes() {
	_loadedPalettes[kClutSystemMac] = PaletteV4(kClutSystemMac, macPalette, 256);
	_loadedPalettes[kClutRainbow] = PaletteV4(kClutRainbow, rainbowPalette, 256);
	_loadedPalettes[kClutGrayscale] = PaletteV4(kClutGrayscale, grayscalePalette, 256);
	_loadedPalettes[kClutPastels] = PaletteV4(kClutPastels, pastelsPalette, 256);
	_loadedPalettes[kClutVivid] = PaletteV4(kClutVivid, vividPalette, 256);
	_loadedPalettes[kClutNTSC] = PaletteV4(kClutNTSC, ntscPalette, 256);
	_loadedPalettes[kClutMetallic] = PaletteV4(kClutMetallic, metallicPalette, 256);
	_loadedPalettes[kClutSystemWin] = PaletteV4(kClutSystemWin, winPalette, 256);
}

PaletteV4 *DirectorEngine::getPalette(int id) {
	if (!_loadedPalettes.contains(id)) {
		warning("DirectorEngine::addPalette(): Palette %d not found", id);
		return nullptr;
	}

	return &_loadedPalettes[id];
}

void DirectorEngine::addPalette(int id, byte *palette, int length) {
	if (id < 0) {
		warning("DirectorEngine::addPalette(): Negative palette ids reserved for default palettes");
		return;
	} else if (_loadedPalettes.contains(id)) {
		delete[] _loadedPalettes[id].palette;
	}

	_loadedPalettes[id] = PaletteV4(id, palette, length);
}

bool DirectorEngine::setPalette(int id) {
	if (id == 0) {
		// Palette id of 0 is unused
		return false;
	} else if (!_loadedPalettes.contains(id)) {
		warning("setPalette(): no palette with matching id %d", id);
		return false;
	}

	PaletteV4 pal = _loadedPalettes[id];
	setPalette(pal.palette, pal.length);

	return true;
}

void DirectorEngine::setPalette(byte *palette, uint16 count) {
	// Pass the palette to OSystem only for 8bpp mode
	if (_pixelformat.bytesPerPixel == 1)
		_system->getPaletteManager()->setPalette(palette, 0, count);

	_currentPalette = palette;
	_currentPaletteLength = count;

	_wm->passPalette(palette, count);
}

void DirectorEngine::clearPalettes() {
	for (Common::HashMap<int, PaletteV4>::iterator it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
		if (it->_value.id > 0)
			delete[] it->_value.palette;
	}
}

void DirectorEngine::setCursor(DirectorCursor type) {
	switch (type) {
	case kCursorMouseDown:
		_wm->replaceCustomCursor(mouseDown, 16, 16, 0, 0, 3);
		break;
	case kCursorMouseUp:
		_wm->replaceCustomCursor(mouseUp, 16, 16, 0, 0, 3);
		break;
	}
}

void DirectorEngine::draw() {
	_wm->renderZoomBox(true);
	_wm->draw();
	g_system->updateScreen();
}

template <typename T>
void inkDrawPixel(int x, int y, int src, void *data) {
	DirectorPlotData *p = (DirectorPlotData *)data;
	Graphics::MacWindowManager *wm = p->d->_wm;

	if (!p->destRect.contains(x, y))
		return;

	T dst;
	uint32 tmpDst;

	dst = (T)p->dst->getBasePtr(x, y);

	if (p->ms) {
		// Get the pixel that macDrawPixel will give us, but store it to apply the
		// ink later
		tmpDst = *dst;
		(wm->getDrawPixel())(x, y, src, p->ms->pd);
		src = *dst;

		*dst = tmpDst;
	} else if (p->alpha) {
		// Sprite blend does not respect colourization; defaults to matte ink
		byte rSrc, gSrc, bSrc;
		byte rDst, gDst, bDst;

		wm->decomposeColor(src, rSrc, gSrc, bSrc);
		wm->decomposeColor(*dst, rDst, gDst, bDst);

		double alpha = (double)p->alpha / 100.0;
		rDst = static_cast<byte>((rSrc * alpha) + (rDst * (1.0 - alpha)));
		gDst = static_cast<byte>((gSrc * alpha) + (gDst * (1.0 - alpha)));
		bDst = static_cast<byte>((bSrc * alpha) + (bDst * (1.0 - alpha)));

		*dst = wm->findBestColor(rDst, gDst, bDst);
		return;
	}

 	switch (p->ink) {
	case kInkTypeBackgndTrans:
		if ((uint32)src == p->backColor)
			break;
		// fall through
	case kInkTypeMatte:
	case kInkTypeMask:
		// Only unmasked pixels make it here, so copy them straight
	case kInkTypeCopy: {
		if (p->applyColor) {
			// TODO: Improve the efficiency of this composition
			byte rSrc, gSrc, bSrc;
			byte rDst, gDst, bDst;
			byte rFor, gFor, bFor;
			byte rBak, gBak, bBak;

			wm->decomposeColor(src, rSrc, gSrc, bSrc);
			wm->decomposeColor(*dst, rDst, gDst, bDst);
			wm->decomposeColor(p->foreColor, rFor, gFor, bFor);
			wm->decomposeColor(p->backColor, rBak, gBak, bBak);

			*dst = wm->findBestColor((rSrc | rFor) & (~rSrc | rBak),
									 (gSrc | gFor) & (~gSrc | gBak),
									 (bSrc | bFor) & (~bSrc | bBak));
		} else {
			*dst = src;
		}
		break;
	}
	case kInkTypeNotCopy:
		if (p->applyColor) {
			// TODO: Improve the efficiency of this composition
			byte rSrc, gSrc, bSrc;
			byte rDst, gDst, bDst;
			byte rFor, gFor, bFor;
			byte rBak, gBak, bBak;

			wm->decomposeColor(src, rSrc, gSrc, bSrc);
			wm->decomposeColor(*dst, rDst, gDst, bDst);
			wm->decomposeColor(p->foreColor, rFor, gFor, bFor);
			wm->decomposeColor(p->backColor, rBak, gBak, bBak);

			*dst = wm->findBestColor((~rSrc | rFor) & (rSrc | rBak),
									 (~gSrc | gFor) & (gSrc | gBak),
									 (~bSrc | bFor) & (bSrc | bBak));
		} else {
			*dst = src;
		}
		break;
	case kInkTypeTransparent:
		*dst = p->applyColor ? (~src & p->foreColor) | (*dst & src) : (*dst & src);
		break;
	case kInkTypeNotTrans:
		*dst = p->applyColor ? (src & p->foreColor) | (*dst & ~src) : (*dst & ~src);
		break;
	case kInkTypeReverse:
		*dst ^= ~(src);
		break;
	case kInkTypeNotReverse:
		*dst ^= src;
		break;
	case kInkTypeGhost:
		*dst = p->applyColor ? (src | p->backColor) & (*dst | ~src) : (*dst | ~src);
		break;
	case kInkTypeNotGhost:
		*dst = p->applyColor ? (~src | p->backColor) & (*dst | src) : *dst | src;
		break;
		// Arithmetic ink types
	default: {
		byte rSrc, gSrc, bSrc;
		byte rDst, gDst, bDst;

		wm->decomposeColor(src, rSrc, gSrc, bSrc);
		wm->decomposeColor(*dst, rDst, gDst, bDst);

		switch (p->ink) {
		case kInkTypeBlend:
				*dst = wm->findBestColor((rSrc + rDst) / 2, (gSrc + gDst) / 2, (bSrc + bDst) / 2);
			break;
		case kInkTypeAddPin:
				*dst = wm->findBestColor(MIN((rSrc + rDst), 0xff), MIN((gSrc + gDst), 0xff), MIN((bSrc + bDst), 0xff));
			break;
		case kInkTypeAdd:
			// in basilisk, D3.1 is exactly using this method, adding color directly without preventing the overflow.
			// but i think min(src + dst, 255) will give us a better visual effect
				*dst = wm->findBestColor(rSrc + rDst, gSrc + gDst, bSrc + bDst);
			break;
		case kInkTypeSubPin:
				*dst = wm->findBestColor(MAX(rSrc - rDst, 0), MAX(gSrc - gDst, 0), MAX(bSrc - bDst, 0));
			break;
		case kInkTypeLight:
				*dst = wm->findBestColor(MAX(rSrc, rDst), MAX(gSrc, gDst), MAX(bSrc, bDst));
			break;
		case kInkTypeSub:
				*dst = wm->findBestColor(abs(rSrc - rDst) % 0xff + 1, abs(gSrc - gDst) % 0xff + 1, abs(bSrc - bDst) % 0xff + 1);
			break;
		case kInkTypeDark:
				*dst = wm->findBestColor(MIN(rSrc, rDst), MIN(gSrc, gDst), MIN(bSrc, bDst));
			break;
		default:
			break;
		}
	}
	}
}

Graphics::MacDrawPixPtr DirectorEngine::getInkDrawPixel() {
	if (_pixelformat.bytesPerPixel == 1)
		return &inkDrawPixel<byte *>;
	else
		return &inkDrawPixel<uint32 *>;
}

void DirectorPlotData::setApplyColor() {
	applyColor = false;

	if (foreColor != colorBlack) {
		if (ink != kInkTypeGhost && ink != kInkTypeNotGhost)
			applyColor = true;
	}

	if (backColor != colorWhite) {
		if (ink != kInkTypeTransparent && ink != kInkTypeNotTrans && ink != kInkTypeBackgndTrans)
			applyColor = true;
	}
}

uint32 DirectorPlotData::preprocessColor(uint32 src) {
	// HACK: Right now this method is just used for adjusting the colourization on text
	// sprites, as it would be costly to colourize the chunks on the fly each
	// time a section needs drawing. It's ugly but mostly works.
	if (sprite == kTextSprite) {
		switch(ink) {
		case kInkTypeMask:
			src = (src == backColor ? foreColor : 0xff);
			break;
		case kInkTypeReverse:
			src = (src == foreColor ? 0 : colorWhite);
			break;
		case kInkTypeNotReverse:
			src = (src == backColor ? colorWhite : 0);
			break;
			// looks like this part is wrong, maybe it's very same as reverse?
			// check warlock/DATA/WARLOCKSHIP/ENG/ABOUT to see more detail.
//		case kInkTypeGhost:
//			src = (src == foreColor ? backColor : colorWhite);
//			break;
		case kInkTypeNotGhost:
			src = (src == backColor ? colorWhite : backColor);
			break;
		case kInkTypeNotCopy:
			src = (src == foreColor ? backColor : foreColor);
			break;
		case kInkTypeNotTrans:
			src = (src == foreColor ? backColor : colorWhite);
			break;
		default:
			break;
		}
	}

	return src;
}

void DirectorPlotData::inkBlitShape(Common::Rect &srcRect) {
	if (!ms)
		return;

	// Preprocess shape colours
	switch (ink) {
	case kInkTypeNotTrans:
	case kInkTypeNotReverse:
	case kInkTypeNotGhost:
		return;
	case kInkTypeReverse:
		ms->foreColor = 0;
		ms->backColor = 0;
		break;
	default:
		break;
	}

	Common::Rect fillAreaRect((int)srcRect.width(), (int)srcRect.height());
	fillAreaRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotFill(dst, nullptr, &d->getPatterns(), ms->pattern, srcRect.left, srcRect.top, 1, ms->backColor);

	Common::Rect strokeRect(MAX((int)srcRect.width() - ms->lineSize, 0), MAX((int)srcRect.height() - ms->lineSize, 0));
	strokeRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotStroke(dst, nullptr, &d->getPatterns(), 1, strokeRect.left, strokeRect.top, ms->lineSize, ms->backColor);

	switch (ms->spriteType) {
	case kRectangleSprite:
		ms->pd = &plotFill;
		Graphics::drawFilledRect(fillAreaRect, ms->foreColor, d->getInkDrawPixel(), this);
		// fall through
	case kOutlinedRectangleSprite:
		// if we have lineSize <= 0, means we are not drawing anything. so we may return directly.
		if (ms->lineSize <= 0)
			break;
		ms->pd = &plotStroke;
		Graphics::drawRect(strokeRect, ms->foreColor, d->getInkDrawPixel(), this);
		break;
	case kRoundedRectangleSprite:
		ms->pd = &plotFill;
		Graphics::drawRoundRect(fillAreaRect, 12, ms->foreColor, true, d->getInkDrawPixel(), this);
		// fall through
	case kOutlinedRoundedRectangleSprite:
		if (ms->lineSize <= 0)
			break;
		ms->pd = &plotStroke;
		Graphics::drawRoundRect(strokeRect, 12, ms->foreColor, false, d->getInkDrawPixel(), this);
		break;
	case kOvalSprite:
		ms->pd = &plotFill;
		Graphics::drawEllipse(fillAreaRect.left, fillAreaRect.top, fillAreaRect.right, fillAreaRect.bottom, ms->foreColor, true, d->getInkDrawPixel(), this);
		// fall through
	case kOutlinedOvalSprite:
		if (ms->lineSize <= 0)
			break;
		ms->pd = &plotStroke;
		Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, false, d->getInkDrawPixel(), this);
		break;
	case kLineTopBottomSprite:
		ms->pd = &plotStroke;
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, d->getInkDrawPixel(), this);
		break;
	case kLineBottomTopSprite:
		ms->pd = &plotStroke;
		Graphics::drawLine(strokeRect.left, strokeRect.bottom, strokeRect.right, strokeRect.top, ms->foreColor, d->getInkDrawPixel(), this);
		break;
	default:
		warning("DirectorPlotData::inkBlitShape: Expected shape type but got type %d", ms->spriteType);
	}
}

void DirectorPlotData::inkBlitSurface(Common::Rect &srcRect, const Graphics::Surface *mask) {
	if (!srf)
		return;

	// TODO: Determine why colourization causes problems in Warlock
	if (sprite == kTextSprite)
		applyColor = false;

	srcPoint.y = abs(srcRect.top - destRect.top);
	for (int i = 0; i < destRect.height(); i++, srcPoint.y++) {
		if (d->_wm->_pixelformat.bytesPerPixel == 1) {
			srcPoint.x = abs(srcRect.left - destRect.left);
			const byte *msk = mask ? (const byte *)mask->getBasePtr(srcPoint.x, srcPoint.y) : nullptr;

			for (int j = 0; j < destRect.width(); j++, srcPoint.x++) {
				if (!mask || (msk && !(*msk++))) {
					(d->getInkDrawPixel())(destRect.left + j, destRect.top + i,
											preprocessColor(*((byte *)srf->getBasePtr(srcPoint.x, srcPoint.y))), this);
				}
			}
		} else {
			srcPoint.x = abs(srcRect.left - destRect.left);
			const uint32 *msk = mask ? (const uint32 *)mask->getBasePtr(srcPoint.x, srcPoint.y) : nullptr;

			for (int j = 0; j < destRect.width(); j++, srcPoint.x++) {
				if (!mask || (msk && !(*msk++))) {
					(d->getInkDrawPixel())(destRect.left + j, destRect.top + i,
											preprocessColor(*((uint32 *)srf->getBasePtr(srcPoint.x, srcPoint.y))), this);
				}
			}
		}
	}
}

void DirectorPlotData::inkBlitStretchSurface(Common::Rect &srcRect, const Graphics::Surface *mask) {
	if (!srf)
		return;

	// TODO: Determine why colourization causes problems in Warlock
	if (sprite == kTextSprite)
		applyColor = false;

	int scaleX = SCALE_THRESHOLD * srcRect.width() / destRect.width();
	int scaleY = SCALE_THRESHOLD * srcRect.height() / destRect.height();

	srcPoint.y = abs(srcRect.top - destRect.top);

	for (int i = 0, scaleYCtr = 0; i < destRect.height(); i++, scaleYCtr += scaleY, srcPoint.y++) {
		if (d->_wm->_pixelformat.bytesPerPixel == 1) {
			srcPoint.x = abs(srcRect.left - destRect.left);
			const byte *msk = mask ? (const byte *)mask->getBasePtr(srcPoint.x, srcPoint.y) : nullptr;

			for (int xCtr = 0, scaleXCtr = 0; xCtr < destRect.width(); xCtr++, scaleXCtr += scaleX, srcPoint.x++) {
				if (!mask || !(*msk++)) {
				(d->getInkDrawPixel())(destRect.left + xCtr, destRect.top + i,
										preprocessColor(*((byte *)srf->getBasePtr(scaleXCtr / SCALE_THRESHOLD, scaleYCtr / SCALE_THRESHOLD))), this);
				}
			}
		} else {
			srcPoint.x = abs(srcRect.left - destRect.left);
			const uint32 *msk = mask ? (const uint32 *)mask->getBasePtr(srcPoint.x, srcPoint.y) : nullptr;

			for (int xCtr = 0, scaleXCtr = 0; xCtr < destRect.width(); xCtr++, scaleXCtr += scaleX, srcPoint.x++) {
				if (!mask || !(*msk++)) {
				(d->getInkDrawPixel())(destRect.left + xCtr, destRect.top + i,
										preprocessColor(*((int *)srf->getBasePtr(scaleXCtr / SCALE_THRESHOLD, scaleYCtr / SCALE_THRESHOLD))), this);
				}
			}
		}
	}
}

} // End of namespace Director
