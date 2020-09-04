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
*/

#include "common/system.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"

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

void DirectorEngine::setCursor(int type) {
	switch (type) {
	case kCursorDefault:
		_wm->popCursor();
		break;

	case kCursorMouseDown:
		_wm->pushCustomCursor(mouseDown, 16, 16, 0, 0, 3);
		break;

	case kCursorMouseUp:
		_wm->pushCustomCursor(mouseUp, 16, 16, 0, 0, 3);
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

	if (!p->destRect.contains(x, y))
		return;

	T dst;
	uint32 tmpDst;

	dst = (T)p->dst->getBasePtr(x, y);

	if (p->ms) {
		// Get the pixel that macDrawPixel will give us, but store it to apply the
		// ink later.
		tmpDst = *dst;
		(p->_wm->getDrawPixel())(x, y, src, p->ms->pd);
		src = *dst;

		*dst = tmpDst;
	} else if (p->alpha) {
		// Sprite blend does not respect colourization; defaults to matte ink
		byte rSrc, gSrc, bSrc;
		byte rDst, gDst, bDst;

		g_director->_wm->decomposeColor(src, rSrc, gSrc, bSrc);
		g_director->_wm->decomposeColor(*dst, rDst, gDst, bDst);

		double alpha = (double)p->alpha / 100.0;
		rDst = static_cast<byte>((rSrc * alpha) + (rDst * (1.0 - alpha)));
		gDst = static_cast<byte>((gSrc * alpha) + (gDst * (1.0 - alpha)));
		bDst = static_cast<byte>((bSrc * alpha) + (bDst * (1.0 - alpha)));

		*dst = p->_wm->findBestColor(rDst, gDst, bDst);
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

			g_director->_wm->decomposeColor(src, rSrc, gSrc, bSrc);
			g_director->_wm->decomposeColor(*dst, rDst, gDst, bDst);
			g_director->_wm->decomposeColor(p->foreColor, rFor, gFor, bFor);
			g_director->_wm->decomposeColor(p->backColor, rBak, gBak, bBak);

			*dst = p->_wm->findBestColor((rSrc | rFor) & (~rSrc | rBak),
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

			g_director->_wm->decomposeColor(src, rSrc, gSrc, bSrc);
			g_director->_wm->decomposeColor(*dst, rDst, gDst, bDst);
			g_director->_wm->decomposeColor(p->foreColor, rFor, gFor, bFor);
			g_director->_wm->decomposeColor(p->backColor, rBak, gBak, bBak);

			*dst = p->_wm->findBestColor((~rSrc | rFor) & (rSrc | rBak),
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
		if ((uint32)src == p->colorWhite)
			break;

		byte rSrc, gSrc, bSrc;
		byte rDst, gDst, bDst;

		g_director->_wm->decomposeColor(src, rSrc, gSrc, bSrc);
		g_director->_wm->decomposeColor(*dst, rDst, gDst, bDst);

		switch (p->ink) {
		case kInkTypeBlend:
				*dst = p->_wm->findBestColor((rSrc + rDst) / 2, (gSrc + gDst) / 2, (bSrc + bDst) / 2);
			break;
		case kInkTypeAddPin:
				*dst = p->_wm->findBestColor(MIN((rSrc + rDst), 0xff), MIN((gSrc + gDst), 0xff), MIN((bSrc + bDst), 0xff));
			break;
		case kInkTypeAdd:
				*dst = p->_wm->findBestColor(abs(rSrc + rDst) % 0xff + 1, abs(gSrc + gDst) % 0xff + 1, abs(bSrc + bDst) % 0xff + 1);
			break;
		case kInkTypeSubPin:
				*dst = p->_wm->findBestColor(MAX(rSrc - rDst, 0), MAX(gSrc - gDst, 0), MAX(bSrc - bDst, 0));
			break;
		case kInkTypeLight:
				*dst = p->_wm->findBestColor(MAX(rSrc, rDst), MAX(gSrc, gDst), MAX(bSrc, bDst));
			break;
		case kInkTypeSub:
				*dst = p->_wm->findBestColor(abs(rSrc - rDst) % 0xff + 1, abs(gSrc - gDst) % 0xff + 1, abs(bSrc - bDst) % 0xff + 1);
			break;
		case kInkTypeDark:
				*dst = p->_wm->findBestColor(MIN(rSrc, rDst), MIN(gSrc, gDst), MIN(bSrc, bDst));
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

	if (foreColor == colorBlack && backColor == colorWhite)
		applyColor = false;

	switch (ink) {
	case kInkTypeReverse:
	case kInkTypeNotReverse:
	case kInkTypeAddPin:
	case kInkTypeAdd:
 	case kInkTypeSubPin:
	case kInkTypeLight:
	case kInkTypeSub:
	case kInkTypeDark:
	case kInkTypeBackgndTrans:
		applyColor = false;
	default:
		break;
	}

	if (foreColor != colorBlack) {
		if (ink != kInkTypeGhost && ink != kInkTypeNotGhost)
			applyColor = true;
	}

	if (backColor != colorWhite) {
		if (ink != kInkTypeTransparent &&
				ink != kInkTypeNotTrans)
			applyColor = true;
	}
}

} // End of namespace Director
