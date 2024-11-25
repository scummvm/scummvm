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
#include "graphics/paletteman.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/images.h"
#include "director/picture.h"
#include "director/window.h"
#include "director/castmember/bitmap.h"

namespace Director {

#include "director/graphics-data.h"

/**
 * Used to upgrade to 32-bit color if required.
 **/
uint32 DirectorEngine::transformColor(uint32 color) {
	if (_pixelformat.bytesPerPixel == 1)
		return color;

	return _wm->findBestColor(_currentPalette[color * 3], _currentPalette[color * 3 + 1], _currentPalette[color * 3 + 2]);
}

void DirectorEngine::loadPatterns() {
	for (int i = 0; i < ARRAYSIZE(director3Patterns); i++)
		_director3Patterns.push_back(director3Patterns[i]);

	for (int i = 0; i < ARRAYSIZE(director3QuickDrawPatterns); i++)
		_director3QuickDrawPatterns.push_back(director3QuickDrawPatterns[i]);

	// We must set it here for correct work of BITDDecoder.
	// It is set later in Director properly
	_pixelformat = Graphics::PixelFormat::createFormatCLUT8();

	for (int i = 0; i < ARRAYSIZE(builtinTiles); i++) {
		Common::MemoryReadStream stream(builtinTiles[i].ptr, builtinTiles[i].size);

		auto decoder = new BITDDecoder(builtinTiles[i].w, builtinTiles[i].h, 8, builtinTiles[i].w, macPalette, kFileVer300);
		decoder->loadStream(stream);
		_builtinTiles[i].img = new Picture(*decoder);
		delete decoder;

		_builtinTiles[i].rect = Common::Rect(0, 0, builtinTiles[i].w, builtinTiles[i].h);
	}
}

Graphics::MacPatterns &DirectorEngine::getPatterns() {
	// TOOD: implement switch and other version patterns. (use getVersion());
	return _director3QuickDrawPatterns;
}

Picture *DirectorEngine::getTile(int num) {
	TilePatternEntry *tile = &getCurrentMovie()->getCast()->_tiles[num];

	if (tile->bitmapId.isNull())
		return _builtinTiles[num].img;

	CastMember *member = getCurrentMovie()->getCastMember(tile->bitmapId);

	if (!member) {
		warning("BUILDBOT: DirectorEngine::getTile(%d) VWTL refers to non-existing cast %s", num,
				tile->bitmapId.asString().c_str());

		return _builtinTiles[num].img;
	}

	if (member->_type != kCastBitmap) {
		warning("BUILDBOT: DirectorEngine::getTile(%d) VWTL refers to incorrect cast %s type %s", num,
				tile->bitmapId.asString().c_str(), castType2str(member->_type));

		return _builtinTiles[num].img;
	}

	return ((BitmapCastMember *)member)->_picture;
}

const Common::Rect &DirectorEngine::getTileRect(int num) {
	TilePatternEntry *tile = &getCurrentMovie()->getCast()->_tiles[num];

	if (tile->bitmapId.isNull())
		return _builtinTiles[num].rect;

	return tile->rect;
}

void DirectorEngine::loadDefaultPalettes() {
	_loadedPalettes[CastMemberID(kClutSystemMac, -1)] = PaletteV4(CastMemberID(kClutSystemMac, -1), macPalette, 256);
	_loadedPalettes[CastMemberID(kClutRainbow, -1)] = PaletteV4(CastMemberID(kClutRainbow, -1), rainbowPalette, 256);
	_loadedPalettes[CastMemberID(kClutGrayscale, -1)] = PaletteV4(CastMemberID(kClutGrayscale, -1), grayscalePalette, 256);
	_loadedPalettes[CastMemberID(kClutPastels, -1)] = PaletteV4(CastMemberID(kClutPastels, -1), pastelsPalette, 256);
	_loadedPalettes[CastMemberID(kClutVivid, -1)] = PaletteV4(CastMemberID(kClutVivid, -1), vividPalette, 256);
	_loadedPalettes[CastMemberID(kClutNTSC, -1)] = PaletteV4(CastMemberID(kClutNTSC, -1), ntscPalette, 256);
	_loadedPalettes[CastMemberID(kClutMetallic, -1)] = PaletteV4(CastMemberID(kClutMetallic, -1), metallicPalette, 256);
	_loadedPalettes[CastMemberID(kClutSystemWin, -1)] = PaletteV4(CastMemberID(kClutSystemWin, -1), winPalette, 256);
	_loadedPalettes[CastMemberID(kClutSystemWinD5, -1)] = PaletteV4(CastMemberID(kClutSystemWinD5, -1), winD5Palette, 256);

	_loaded16Palettes[CastMemberID(kClutSystemMac, -1)] = PaletteV4(CastMemberID(kClutSystemMac, -1), mac16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutRainbow, -1)] = PaletteV4(CastMemberID(kClutRainbow, -1), rainbow16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutGrayscale, -1)] = PaletteV4(CastMemberID(kClutGrayscale, -1), grayscale16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutPastels, -1)] = PaletteV4(CastMemberID(kClutPastels, -1), pastels16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutVivid, -1)] = PaletteV4(CastMemberID(kClutVivid, -1), vivid16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutNTSC, -1)] = PaletteV4(CastMemberID(kClutNTSC, -1), ntsc16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutMetallic, -1)] = PaletteV4(CastMemberID(kClutMetallic, -1), metallic16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutSystemWin, -1)] = PaletteV4(CastMemberID(kClutSystemWin, -1), win16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutSystemWinD5, -1)] = PaletteV4(CastMemberID(kClutSystemWinD5, -1), winD516Palette, 16);

	_loaded4Palette = PaletteV4(CastMemberID(kClutGrayscale, -1), grayscale4Palette, 4);
}

PaletteV4 *DirectorEngine::getPalette(const CastMemberID &id) {
	if (id.isNull())
		return nullptr;

	if (!_loadedPalettes.contains(id)) {
		warning("DirectorEngine::getPalette(): Palette %s not found, hash %x", id.asString().c_str(), id.hash());
		return nullptr;
	}

	return &_loadedPalettes[id];
}

bool DirectorEngine::hasPalette(const CastMemberID &id) {
	return _loadedPalettes.contains(id);
}

void DirectorEngine::addPalette(CastMemberID &id, const byte *palette, int length) {
	if (id.castLib < 0) {
		warning("DirectorEngine::addPalette(): Negative cast library ids reserved for default palettes");
		return;
	} else if (_loadedPalettes.contains(id)) {
		delete[] _loadedPalettes[id].palette;
	}

	debugC(3, kDebugLoading, "DirectorEngine::addPalette(): Registered palette %s of size %d, hash: %x", id.asString().c_str(), length, id.hash());
	byte *palCopy = new byte[length * 3]; // freed by clearPalettes()
	memcpy(palCopy, palette, length * 3);

	_loadedPalettes[id] = PaletteV4(id, palCopy, length);
}

bool DirectorEngine::setPalette(const CastMemberID &id) {
	if (id.isNull()) {
		// Palette id of 0 is unused
		return false;
	}

	PaletteV4 *pal = getPalette(id);
	if (!pal)
		return false;
	debugC(5, kDebugImages, "DirectorEngine::setPalettes(): setting palette %d, %d", id.member, id.castLib);
	setPalette(pal->palette, pal->length);
	return true;
}

void DirectorEngine::setPalette(const byte *palette, uint16 count) {

	memset(_currentPalette, 0, 768);
	memmove(_currentPalette, palette, count * 3);
	_currentPaletteLength = count;
	if (debugChannelSet(8, kDebugImages)) {
		Common::String palData;
		for (size_t i = 0; i < (size_t)_currentPaletteLength; i++) {
			palData += Common::String::format("%02X%02X%02X", _currentPalette[3 * i], _currentPalette[3 * i + 1], _currentPalette[3 * i + 2]);
		}
		debugC(8, kDebugImages, "DirectorEngine::setPalette(): Setting current palette: %s", palData.c_str());
	}

	// Pass the palette to OSystem only for 8bpp mode
	if (_pixelformat.bytesPerPixel == 1)
		_system->getPaletteManager()->setPalette(_currentPalette, 0, _currentPaletteLength);

	_wm->passPalette(_currentPalette, _currentPaletteLength);
}

void DirectorEngine::shiftPalette(int startIndex, int endIndex, bool reverse) {
	if (startIndex == endIndex)
		return;

	if (startIndex > endIndex)
		return;

	byte temp[3] = { 0, 0, 0 };
	int span = endIndex - startIndex + 1;
	if (reverse) {
		memcpy(temp, _currentPalette + 3 * startIndex, 3);
		memmove(_currentPalette + 3 * startIndex,
			_currentPalette + 3 * startIndex + 3,
			(span - 1) * 3);
		memcpy(_currentPalette + 3 * endIndex, temp, 3);
	} else {
		memcpy(temp, _currentPalette + 3 * endIndex, 3);
		memmove(_currentPalette + 3 * startIndex + 3,
			_currentPalette + 3 * startIndex,
			(span - 1) * 3);
		memcpy(_currentPalette + 3 * startIndex, temp, 3);
	}

	if (debugChannelSet(8, kDebugImages)) {
		Common::String palData;
		for (size_t i = 0; i < (size_t)_currentPaletteLength; i++) {
			palData += Common::String::format("%02X%02X%02X", _currentPalette[3 * i], _currentPalette[3 * i + 1], _currentPalette[3 * i + 2]);
		}
		debugC(8, kDebugImages, "DirectorEngine::shiftPalette(): Rotating current palette (start: %d, end: %d, reverse: %d): %s", startIndex, endIndex, reverse, palData.c_str());
	}

	// Pass the palette to OSystem only for 8bpp mode
	if (_pixelformat.bytesPerPixel == 1)
		_system->getPaletteManager()->setPalette(_currentPalette, 0, _currentPaletteLength);

	_wm->passPalette(_currentPalette, _currentPaletteLength);
}

void DirectorEngine::clearPalettes() {
	// FIXME: Ideally we would run this every time we switch movie; but that would need to take
	// into account e.g. shared casts that haven't changed, multiple windows...
	for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
		if (it->_value.id.castLib > 0) {
			debugC(5, kDebugImages, "DirectorEngine::clearPalettes(): erasing palette %d, %d", it->_value.id.member, it->_value.id.castLib);
			delete[] it->_value.palette;
			_loadedPalettes.erase(it);
		}
	}
	_lastPalette = CastMemberID();
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

	T *dst;
	uint32 tmpDst;

	dst = (T *)p->dst->getBasePtr(x, y);

	if (p->ms) {
		if (p->ms->pd->thickness > 1) {
			int prevThickness = p->ms->pd->thickness;
			int x1 = x;
			int x2 = x1 + prevThickness;
			int y1 = y;
			int y2 = y1 + prevThickness;

			p->ms->pd->thickness = 1;	// We do not want recursive loops

			for (y = y1; y < y2; y++)
				for (x = x1; x < x2; x++)
					if (x >= 0 && x < p->ms->pd->surface->w && y >= 0 && y < p->ms->pd->surface->h) {
						inkDrawPixel<T>(x, y, src, data);
					}

			p->ms->pd->thickness = prevThickness;
			return;
		}

		if (p->ms->tile) {
			int x1 = p->ms->tileRect->left + (p->ms->pd->fillOriginX + x) % p->ms->tileRect->width();
			int y1 = p->ms->tileRect->top  + (p->ms->pd->fillOriginY + y) % p->ms->tileRect->height();

			src = p->ms->tile->_surface.getPixel(x1, y1);
		} else {
			// Get the pixel that macDrawPixel will give us, but store it to apply the
			// ink later
			tmpDst = *dst;
			(wm->getDrawPixel())(x, y, src, p->ms->pd);
			src = *dst;

			*dst = tmpDst;
		}
	} else if (p->alpha) {
		// Sprite blend does not respect colourization; defaults to matte ink
		byte rSrc, gSrc, bSrc;
		byte rDst, gDst, bDst;

		wm->decomposeColor<T>(src, rSrc, gSrc, bSrc);
		wm->decomposeColor<T>(*dst, rDst, gDst, bDst);

		rDst = lerpByte(rSrc, rDst, p->alpha, 255);
		gDst = lerpByte(gSrc, gDst, p->alpha, 255);
		bDst = lerpByte(bSrc, bDst, p->alpha, 255);
		*dst = wm->findBestColor(rDst, gDst, bDst);
		return;
	}

 	switch (p->ink) {
	case kInkTypeBackgndTrans:
		if (p->oneBitImage) {
			// One-bit images have a slightly different rendering algorithm for BackgndTrans.
			// Foreground colour is used, and background colour is ignored.
			*dst = (src == (int)p->colorBlack) ? p->foreColor : *dst;
		} else {
			*dst = (src == (int)p->backColor) ? *dst : src;
		}
		break;
	case kInkTypeMatte:
		// fall through
	case kInkTypeMask:
		// Only unmasked pixels make it here, so copy them straight
	case kInkTypeBlend:
		// If there's a blend factor set, it's dealt with in the alpha handling block.
		// Otherwise, treat it like a Matte image.
	case kInkTypeCopy: {
		if (p->applyColor) {
			if (sizeof(T) == 1) {
				*dst = src == 0xff ? p->foreColor : (src == 0x00 ? p->backColor : *dst);
			} else {
				// TODO: Improve the efficiency of this composition
				byte rSrc, gSrc, bSrc;
				byte rDst, gDst, bDst;
				byte rFor, gFor, bFor;
				byte rBak, gBak, bBak;

				wm->decomposeColor<T>(src, rSrc, gSrc, bSrc);
				wm->decomposeColor<T>(*dst, rDst, gDst, bDst);
				wm->decomposeColor<T>(p->foreColor, rFor, gFor, bFor);
				wm->decomposeColor<T>(p->backColor, rBak, gBak, bBak);

				*dst = wm->findBestColor((rSrc | rFor) & (~rSrc | rBak),
										(gSrc | gFor) & (~gSrc | gBak),
										(bSrc | bFor) & (~bSrc | bBak));
			}
		} else {
			*dst = src;
		}
		break;
	}
	case kInkTypeNotCopy:
		if (p->applyColor) {
			if (sizeof(T) == 1) {
				*dst = src == 0xff ? p->backColor : (src == 0x00 ? p->foreColor : src);
			} else {
				// TODO: Improve the efficiency of this composition
				byte rSrc, gSrc, bSrc;
				byte rDst, gDst, bDst;
				byte rFor, gFor, bFor;
				byte rBak, gBak, bBak;

				wm->decomposeColor<T>(src, rSrc, gSrc, bSrc);
				wm->decomposeColor<T>(*dst, rDst, gDst, bDst);
				wm->decomposeColor<T>(p->foreColor, rFor, gFor, bFor);
				wm->decomposeColor<T>(p->backColor, rBak, gBak, bBak);

				*dst = wm->findBestColor((~rSrc | rFor) & (rSrc | rBak),
										(~gSrc | gFor) & (gSrc | gBak),
										(~bSrc | bFor) & (bSrc | bBak));
			}
		} else {
			// Find the inverse of the colour and match it back to the palette if required
			byte rSrc, gSrc, bSrc;
			wm->decomposeColor<T>(src, rSrc, gSrc, bSrc);

			*dst = wm->findBestColor(~rSrc, ~gSrc, ~bSrc);
		}
		break;
	case kInkTypeTransparent:
		if (p->oneBitImage || p->applyColor) {
			*dst = src == (int)p->colorBlack ? p->foreColor : *dst;
		} else {
			// OR dst palette index with src.
			// Originally designed for 1-bit mode to make white pixels
			// transparent.
			*dst = *dst | src;
		}
		break;
	case kInkTypeNotTrans:
		if (p->oneBitImage || p->applyColor) {
			*dst = src == (int)p->colorWhite ? p->foreColor : *dst;
		} else {
			// OR dst palette index with the inverse of src.
			*dst = *dst | ~src;
		}
		break;
	case kInkTypeReverse:
		// XOR dst palette index with src.
		// Originally designed for 1-bit mode so that
		// black pixels would appear white on a black
		// background.
		*dst ^= src;
		break;
	case kInkTypeNotReverse:
		// XOR dst palette index with the inverse of src.
		*dst ^= ~(src);
		break;
	case kInkTypeGhost:
		if (p->oneBitImage || p->applyColor) {
			*dst = src == (int)p->colorBlack ? p->backColor : *dst;
		} else {
			// AND dst palette index with the inverse of src.
			// Originally designed for 1-bit mode so that
			// black pixels would be invisible until they were
			// over a black background, showing as white.
			*dst = *dst & ~src;
		}
		break;
	case kInkTypeNotGhost:
		if (p->oneBitImage || p->applyColor) {
			*dst = src == (int)p->colorWhite ? p->backColor : *dst;
		} else {
			// AND dst palette index with src.
			*dst = *dst & src;
		}
		break;
	default: {
		// Arithmetic ink types, based on real color values
		byte rSrc, gSrc, bSrc;
		byte rDst, gDst, bDst;

		wm->decomposeColor<T>(src, rSrc, gSrc, bSrc);
		wm->decomposeColor<T>(*dst, rDst, gDst, bDst);

		switch (p->ink) {
		case kInkTypeAddPin:
			// Add src to dst, but pinning each channel so it can't go above 0xff.
			*dst = wm->findBestColor(rDst + MIN(0xff - rDst, (int)rSrc), gDst + MIN(0xff - gDst, (int)gSrc), bDst + MIN(0xff - bDst, (int)bSrc));
			break;
		case kInkTypeAdd:
			// Add src to dst, allowing each channel to overflow and wrap around.
			*dst = wm->findBestColor(rDst + rSrc, gDst + gSrc, bDst + bSrc);
			break;
		case kInkTypeSubPin:
			// Subtract src from dst, but pinning each channel so it can't go below 0x00.
			*dst = wm->findBestColor(MAX(rDst - rSrc, 1) - 1, MAX(gDst - gSrc, 1) - 1, MAX(bDst - bSrc, 1) - 1);
			break;
		case kInkTypeLight:
			// Pick the higher of src and dst for each channel, lightening the image.
			*dst = wm->findBestColor(MAX(rSrc, rDst), MAX(gSrc, gDst), MAX(bSrc, bDst));
			break;
		case kInkTypeSub:
			// Subtract src from dst, allowing each channel to underflow and wrap around.
			*dst = wm->findBestColor(rDst - rSrc, gDst - gSrc, bDst - bSrc);
			break;
		case kInkTypeDark:
			// Pick the lower of src and dst for each channel, darkening the image.
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
		return &inkDrawPixel<byte>;
	else
		return &inkDrawPixel<uint32>;
}

uint32 DirectorEngine::getColorBlack() {
	if (_pixelformat.bytesPerPixel == 1)
		// needs to be the last entry in the palette.
		// we can't use findBestColor, as it might
		// pick a different entry that's also black.
		return 0xff;
	else
		// send RGB through findBestColor to avoid endian issues
		return _wm->findBestColor(0, 0, 0);
}

uint32 DirectorEngine::getColorWhite() {
	if (_pixelformat.bytesPerPixel == 1)
		// needs to be the first entry in the palette.
		// we can't use findBestColor, as it might
		// pick a different entry that's also white.
		return 0x00;
	else
		// send RGB through findBestColor to avoid endian issues
		return _wm->findBestColor(255, 255, 255);
}

void DirectorPlotData::setApplyColor() {
	// Director has two ways of rendering an ink setting.
	// The default is to incorporate the full range of colors in the image.
	// "applyColor" is used to denote the other option; reduce the image
	// to some combination of the currently set foreground and background color.
	applyColor = false;

	switch (ink) {
	case kInkTypeMatte:
	case kInkTypeMask:
	case kInkTypeCopy:
	case kInkTypeNotCopy:
		applyColor = (foreColor != colorBlack) || (backColor != colorWhite);
		break;
	case kInkTypeTransparent:
	case kInkTypeNotTrans:
	case kInkTypeBackgndTrans:
	case kInkTypeGhost:
	case kInkTypeNotGhost:
		applyColor = !((foreColor == colorBlack) && (backColor == colorWhite));
		break;
	default:
		break;
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
		ms->foreColor = 255;
		ms->backColor = 0;
		break;
	default:
		break;
	}

	Common::Point wpos;
	Movie *movie = g_director->getCurrentMovie();

	if (g_director->_wm->_mode & Graphics::kWMModeNoDesktop)
		wpos = Common::Point(movie->getCast()->_movieRect.left, movie->getCast()->_movieRect.top);
	else
		wpos = movie->getWindow()->getAbsolutePos();

	Common::Rect fillAreaRect((int)srcRect.width(), (int)srcRect.height());
	fillAreaRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotFill(dst, nullptr, &d->getPatterns(), ms->pattern, srcRect.left + wpos.x, srcRect.top + wpos.y, 1, ms->backColor);

	uint strokePattern = 1;

	bool outline = (ms->spriteType == kOutlinedRectangleSprite || ms->spriteType == kOutlinedRoundedRectangleSprite
			|| ms->spriteType == kOutlinedOvalSprite);

	if (outline)
		strokePattern = ms->pattern;

	Common::Rect strokeRect(MAX((int)srcRect.width() - ms->lineSize, 0), MAX((int)srcRect.height() - ms->lineSize, 0));
	strokeRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotStroke(dst, nullptr, &d->getPatterns(), strokePattern, strokeRect.left + wpos.x, strokeRect.top + wpos.y, ms->lineSize, ms->backColor);

	switch (ms->spriteType) {
	case kRectangleSprite:
		ms->pd = &plotFill;
		Graphics::drawFilledRect1(fillAreaRect, ms->foreColor, d->getInkDrawPixel(), this);
		// fall through
	case kOutlinedRectangleSprite:
		// if we have lineSize <= 0, means we are not drawing anything. so we may return directly.
		if (ms->lineSize <= 0)
			break;
		ms->pd = &plotStroke;

		if (!outline)
			ms->tile = nullptr;

		Graphics::drawRect1(strokeRect, ms->foreColor, d->getInkDrawPixel(), this);
		break;
	case kRoundedRectangleSprite:
		ms->pd = &plotFill;
		Graphics::drawRoundRect1(fillAreaRect, 12, ms->foreColor, true, d->getInkDrawPixel(), this);
		// fall through
	case kOutlinedRoundedRectangleSprite:
		if (ms->lineSize <= 0)
			break;
		ms->pd = &plotStroke;

		if (!outline)
			ms->tile = nullptr;

		Graphics::drawRoundRect1(strokeRect, 12, ms->foreColor, false, d->getInkDrawPixel(), this);
		break;
	case kOvalSprite:
		ms->pd = &plotFill;
		Graphics::drawEllipse(fillAreaRect.left, fillAreaRect.top, fillAreaRect.right, fillAreaRect.bottom, ms->foreColor, true, d->getInkDrawPixel(), this);
		// fall through
	case kOutlinedOvalSprite:
		if (ms->lineSize <= 0)
			break;
		ms->pd = &plotStroke;

		if (!outline)
			ms->tile = nullptr;

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

	Common::Rect srfClip = srf->getBounds();
	bool failedBoundsCheck = false;

	// FAST PATH: if we're not doing any per-pixel ops,
	// use the stock blitter. Your CPU will thank you.
	if (!applyColor && !alpha && !ms) {
		Common::Rect offsetRect(
			Common::Point(abs(srcRect.left - destRect.left), abs(srcRect.top - destRect.top)),
			destRect.width(),
			destRect.height()
		);
		offsetRect.clip(srfClip);
		switch (ink) {
		case kInkTypeCopy:
			dst->blitFrom(*srf, offsetRect, destRect);
			return;
			break;
		default:
			break;
		}
	}

	// For blit efficiency, surfaces passed here need to be the same
	// format as the window manager. Most of the time this is
	// the job of BitmapCastMember::createWidget.

	srcPoint.y = abs(srcRect.top - destRect.top);
	for (int i = 0; i < destRect.height(); i++, srcPoint.y++) {
		srcPoint.x = abs(srcRect.left - destRect.left);
		const byte *msk = mask ? (const byte *)mask->getBasePtr(srcPoint.x, srcPoint.y) : nullptr;

		for (int j = 0; j < destRect.width(); j++, srcPoint.x++) {
			if (!srfClip.contains(srcPoint)) {
				failedBoundsCheck = true;
				continue;
			}

			if (!mask || (msk && (*msk++))) {
				if (d->_wm->_pixelformat.bytesPerPixel == 1) {
					(d->getInkDrawPixel())(destRect.left + j, destRect.top + i,
										preprocessColor(*((byte *)srf->getBasePtr(srcPoint.x, srcPoint.y))), this);
				} else {
					(d->getInkDrawPixel())(destRect.left + j, destRect.top + i,
										preprocessColor(*((uint32 *)srf->getBasePtr(srcPoint.x, srcPoint.y))), this);
				}
			}
		}
	}

	if (failedBoundsCheck) {
		warning("DirectorPlotData::inkBlitSurface: Out of bounds - srfClip: %d,%d,%d,%d, srcRect: %d,%d,%d,%d, dstRect: %d,%d,%d,%d",
				srfClip.left, srfClip.top, srfClip.right, srfClip.bottom,
				srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
				destRect.left, destRect.top, destRect.right, destRect.bottom);
	}

}

} // End of namespace Director
