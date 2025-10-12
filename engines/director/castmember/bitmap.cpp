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

#include "common/config-manager.h"
#include "common/stream.h"
#include "common/macresman.h"
#include "graphics/surface.h"
#include "graphics/macgui/macwidget.h"
#include "image/bmp.h"
#include "image/jpeg.h"
#include "image/pict.h"
#include "image/png.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/images.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/score.h"
#include "director/types.h"
#include "director/window.h"
#include "director/castmember/bitmap.h"
#include "director/lingo/lingo-the.h"

namespace Director {

BitmapCastMember::BitmapCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint32 castTag, uint16 version, uint8 flags1)
		: CastMember(cast, castId, stream) {
	_type = kCastBitmap;
	_picture = new Picture();
	_ditheredImg = nullptr;
	_matte = nullptr;
	_noMatte = false;
	_bytes = 0;
	_pitch = 0;
	_flags2 = 0;
	_regX = _regY = 0;
	_clut = CastMemberID(0, 0);
	_ditheredTargetClut = CastMemberID(0, 0);
	_bitsPerPixel = 0;
	_external = false;
	_editVersion = 0;
	_updateFlags = 0;
	_version = version;

	if (debugChannelSet(5, kDebugLoading)) {
		stream.hexdump(stream.size());
	}

	if (version < kFileVer400) {
		_flags1 = flags1;	// region: 0 - auto, 1 - matte, 2 - disabled

		_updateFlags |= (_flags1 & kFlagCenterRegPointD4) ? kFlagCenterRegPoint : 0;
		_updateFlags |= (_flags1 & kFlagMatteD4) ? kFlagMatte : 0;

		_bytes = stream.readUint16();
		// A little context about how bitmap bounding boxes are stored.
		// In the Director editor, images can be edited on a big scrolling canvas with
		// the image in the middle. _initialRect describes the location on that virtual
		// canvas, with the top-left being the start position of the image.
		// _regX and _regY is the registration offset, in canvas space.
		// This means if a bitmap cast member is placed at (64, 64) on the score, the
		// registration offset of the image is placed at (64, 64).
		// By default the registration offset is the dead centre of the image.
		// _boundingRect I think is used internally by the editor and not elsewhere.
		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_regY = stream.readSint16();
		_regX = stream.readSint16();

		if (_bytes & 0x8000) {
			_bitsPerPixel = stream.readUint16();
			int clutId = stream.readSint16();

			if (clutId <= 0) // builtin palette
				_clut = CastMemberID(clutId - 1, -1);
			else
				_clut = CastMemberID(clutId, DEFAULT_CAST_LIB);
		} else {
			_bitsPerPixel = 1;
			_clut = CastMemberID(kClutSystemMac, -1);
		}

		_pitch = _initialRect.width();
		if (_pitch % 16)
			_pitch += 16 - (_initialRect.width() % 16);

		_pitch *= _bitsPerPixel;
		_pitch >>= 3;

	} else if (version >= kFileVer400 && version < kFileVer600) {
		_flags1 = flags1;

		_updateFlags |= (_flags1 & kFlagCenterRegPointD4) ? kFlagCenterRegPoint : 0;
		_updateFlags |= (_flags1 & kFlagMatteD4) ? kFlagMatte : 0;

		_pitch = stream.readUint16();
		_pitch &= 0x0fff;

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		_bitsPerPixel = 0;

		if (stream.pos() < stream.size()) {
			// castSize is > 22 bytes
			stream.readByte();
			_bitsPerPixel = stream.readByte();
			int clutCastLib = -1;
			if (version >= kFileVer500) {
				clutCastLib = stream.readSint16();
			}
			int clutId = stream.readSint16();

			if (clutId <= 0) // builtin palette
				_clut = CastMemberID(clutId - 1, -1);
			else if (clutId > 0) {
				if (clutCastLib == -1) {
					clutCastLib = _cast->_castLibID;
				}
				_clut = CastMemberID(clutId, clutCastLib);
			}
			if (stream.pos() < stream.size()) {
				// castSize > 26 bytes on D4, > 28 bytes on D5
				stream.readUint16();
				/* uint16 unk1 = */ stream.readUint16();
				stream.readUint16();

				stream.readUint32();
				stream.readUint32();

				_flags2 = stream.readUint16();
			}
		}

		if (_bitsPerPixel == 0)
			_bitsPerPixel = 1;

		int tail = stream.size() - stream.pos();
		if (tail > 0) {
			warning("BUILDBOT: BitmapCastMember: %d bytes left", tail);
			if (debugChannelSet(2, kDebugLoading)) {
				byte buf[256];
				tail = MIN(256, tail);
				stream.read(buf, tail);
				debug("BitmapCastMember: tail");
				Common::hexdump(buf, tail);
			}
		}
	} else if (version >= kFileVer600 && version < kFileVer1100) {
		_flags1 = flags1;
		_pitch = stream.readUint16();

		_initialRect = Movie::readRect(stream);

		if (version >= kFileVer700) {
			_alphaThreshold = stream.readByte();
			stream.readByte(); // padding
		} else {
			stream.readUint16(); // padding
		}

		_editVersion = stream.readUint16();

		_scrollPoint.y = stream.readSint16();
		_scrollPoint.x = stream.readSint16();

		_regY = stream.readUint16();
		_regX = stream.readUint16();

		_updateFlags = stream.readByte();

		// 22 bytes
		// This is color image flag
		if (_pitch & 0x8000) {
			_pitch &= 0x0fff;

			_bitsPerPixel = stream.readByte();

			int clutCastLib = -1;
			if (version >= kFileVer500) {
				clutCastLib = stream.readSint16();
			}
			int clutId = stream.readSint16();

			if (clutId <= 0) // builtin palette
				_clut = CastMemberID(clutId - 1, -1);
			else if (clutId > 0) {
				if (clutCastLib == -1) {
					clutCastLib = _cast->_castLibID;
				}
				_clut = CastMemberID(clutId, clutCastLib);
			}
		} else {
			_bitsPerPixel = 1;
		}

		int tail = stream.size() - stream.pos();
		if (tail > 0) {
			warning("BUILDBOT: BitmapCastMember: %d bytes left", tail);
			if (debugChannelSet(2, kDebugLoading)) {
				byte buf[256];
				tail = MIN(256, tail);
				stream.read(buf, tail);
				debug("BitmapCastMember: tail");
				Common::hexdump(buf, tail);
			}
		}
	} else {
		warning("STUB: BitmapCastMember::BitmapCastMember(): Bitmaps not yet supported for version v%d (%d)", humanVersion(version), version);
	}

	debugC(3, kDebugLoading, "  BitmapCastMember: %s", formatInfo().c_str());

	_tag = castTag;
}

BitmapCastMember::BitmapCastMember(Cast *cast, uint16 castId, Image::ImageDecoder *img, uint8 flags1)
	: CastMember(cast, castId) {
	_type = kCastBitmap;
	_matte = nullptr;
	_noMatte = false;
	_bytes = 0;
	if (img != nullptr) {
		_picture = new Picture(*img);
	}
	_ditheredImg = nullptr;
	_clut = CastMemberID(0, 0);
	_ditheredTargetClut = CastMemberID(0, 0);
	_initialRect = Common::Rect(0, 0, img->getSurface()->w, img->getSurface()->h);
	_pitch = img->getSurface()->pitch;
	_bitsPerPixel = img->getSurface()->format.bytesPerPixel * 8;
	_regY = img->getSurface()->h / 2;
	_regX = img->getSurface()->w / 2;
	_flags1 = flags1;
	_flags2 = 0;
	_tag = 0;
	_external = false;
	_editVersion = 0;
	_updateFlags = 0;

	_version = g_director->getVersion();
}

BitmapCastMember::BitmapCastMember(Cast *cast, uint16 castId, BitmapCastMember &source)
	: CastMember(cast, castId) {
	_type = kCastBitmap;
	// force a load so we can copy the cast resource information
	source.load();
	_loaded = true;

	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	if (cast == source._cast)
		_children = source._children;

	_picture = source._picture ? new Picture(*source._picture) : nullptr;
	_ditheredImg = nullptr;
	_matte = nullptr;

	_pitch = source._pitch;
	_regX = source._regX;
	_regY = source._regY;
	_flags2 = source._regY;
	_bytes = source._bytes;
	_clut = source._clut;
	_ditheredTargetClut = source._ditheredTargetClut;
	_editVersion = source._editVersion;
	_updateFlags = source._updateFlags;
	_scrollPoint = source._scrollPoint;

	_bitsPerPixel = source._bitsPerPixel;

	_tag = source._tag;
	_noMatte = source._noMatte;
	_external = source._external;

	_version = source._version;

	warning("BitmapCastMember(): Duplicating source %d to target %d! This is unlikely to work properly, as the resource loader is based on the cast ID", source._castId, castId);
}

BitmapCastMember::~BitmapCastMember() {
	delete _picture;

	if (_ditheredImg) {
		_ditheredImg->free();
		delete _ditheredImg;
		_ditheredImg = nullptr;
	}

	if (_matte) {
		_matte->free();
		delete _matte;
		_matte = nullptr;
	}
}

Graphics::MacWidget *BitmapCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!_picture) {
		warning("BitmapCastMember::createWidget: No picture");
		return nullptr;
	}

	// skip creating widget when the bbox is not available, maybe we should create it using initialRect
	if (!bbox.width() || !bbox.height())
		return nullptr;

	// Check if we need to dither the image
	int dstBpp = g_director->_wm->_pixelformat.bytesPerPixel;
	int srcBpp = _picture->_surface.format.bytesPerPixel;

	const byte *pal = _picture->_palette;
	bool previouslyDithered = _ditheredImg != nullptr;

	// _ditheredImg should contain a cached copy of the bitmap after any expensive
	// colourspace transformations (e.g. palette remapping or dithering).
	// We also want to make sure that
	if (isModified() || (((srcBpp == 1) || (srcBpp > 1 && dstBpp == 1)) && !previouslyDithered)) {
		if (_ditheredImg) {
			_ditheredImg->free();
			delete _ditheredImg;
			_ditheredImg = nullptr;
			_ditheredTargetClut = CastMemberID(0, 0);
		}

		if (dstBpp == 1) {
			// ScummVM using 8-bit video

			if (srcBpp > 1
			// At least early directors were not remapping 8bpp images. But in case it is
			// needed, here is the code
#if 0
			|| (srcBpp == 1 &&
				memcmp(g_director->_wm->getPalette(), _img->_palette, _img->_paletteSize))
#endif
				) {

				_ditheredImg = _picture->_surface.convertTo(g_director->_wm->_pixelformat, nullptr, 0, g_director->_wm->getPalette(), g_director->_wm->getPaletteSize());

				pal = g_director->_wm->getPalette();
			} else if (srcBpp == 1) {
				_ditheredImg = getDitherImg();
			}
		} else {
			// ScummVM using 32-bit video
			//if (srcBpp > 1 && srcBpp != 4) {
				// non-indexed surface, convert to 32-bit
			//	_ditheredImg = _picture->_surface.convertTo(g_director->_wm->_pixelformat, nullptr, 0, g_director->_wm->getPalette(), g_director->_wm->getPaletteSize());

			//} else
			if (srcBpp == 1) {
				_ditheredImg = getDitherImg();
			}
		}

		Movie *movie = g_director->getCurrentMovie();
		Score *score = movie->getScore();

		if (_ditheredImg) {
			debugC(4, kDebugImages, "BitmapCastMember::createWidget(): Dithering cast %d from source palette %s to target palette %s", _castId, _clut.asString().c_str(), score->getCurrentPalette().asString().c_str());
		} else if (previouslyDithered) {
			debugC(4, kDebugImages, "BitmapCastMember::createWidget(): Removed dithered image for cast %d, score palette %s matches cast member", _castId, score->getCurrentPalette().asString().c_str());

		}
	}

	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow()->getMacWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);

	Graphics::Surface *srcSurface = _ditheredImg ? _ditheredImg : &_picture->_surface;
	if ((srcSurface->w <= 0) || (srcSurface->h <= 0)) {
		// We're copying from a zero-sized surface; fill widget with white so transparent ink works
		Common::Rect dims = widget->getDimensions();
		widget->getSurface()->fillRect(Common::Rect(dims.width(), dims.height()), g_director->_wm->_colorWhite);
	} else {
		// scale for drawing a different size sprite
		copyStretchImg(
			srcSurface,
			widget->getSurface()->surfacePtr(),
			_initialRect,
			bbox,
			pal
		);
	}

	return widget;
}

Graphics::Surface *BitmapCastMember::getDitherImg() {
	Graphics::Surface *dither = nullptr;

	// Convert indexed image to indexed palette
	Movie *movie = g_director->getCurrentMovie();
	Score *score = movie->getScore();
	int targetBpp = g_director->_wm->_pixelformat.bytesPerPixel;

	// Get the current score palette. Note that this is the ID of the palette in the list, not the cast member!
	CastMemberID currentPaletteId = score->getCurrentPalette();
	if (currentPaletteId.isNull())
		currentPaletteId = movie->_defaultPalette;
	PaletteV4 *currentPalette = g_director->getPalette(currentPaletteId);
	if (!currentPalette) {
		currentPaletteId = CastMemberID(kClutSystemMac, -1);
		currentPalette = g_director->getPalette(currentPaletteId);
	}
	CastMemberID castPaletteId = _clut;
	// It is possible for Director to have saved an invalid ID in _clut;
	// if this is the case, do no dithering.
	if (castPaletteId.isNull())
		castPaletteId = currentPaletteId;

	// Check if the palette is in the middle of a color fade event
	bool isColorCycling = score->isPaletteColorCycling();

	const byte *dstPalette = targetBpp == 1 ? currentPalette->palette : nullptr;
	int dstPaletteCount = targetBpp == 1 ? currentPalette->length : 0;

	// First, check if the palettes are different
	switch (_bitsPerPixel) {
	// 1bpp - this is preconverted to 0x00 and 0xff, change nothing.
	case 1:
		break;
	// 2bpp - convert to nearest using the standard 2-bit palette.
	case 2:
		{
			const PaletteV4 &srcPal = g_director->getLoaded4Palette();
			dither = _picture->_surface.convertTo(g_director->_wm->_pixelformat, srcPal.palette, srcPal.length, dstPalette, dstPaletteCount, Graphics::kDitherNaive);
		}
		break;
	// 4bpp - if using a builtin palette, use one of the corresponding 4-bit ones.
	case 4:
		{
			const auto pals = g_director->getLoaded16Palettes();
			// in D4 you aren't allowed to use custom palettes for 4-bit images, so uh...
			// I guess default to the mac palette?
			CastMemberID palIndex = pals.contains(castPaletteId) ? castPaletteId : CastMemberID(kClutSystemMac, -1);
			const PaletteV4 &srcPal = pals.getVal(palIndex);
			dither = _picture->_surface.convertTo(g_director->_wm->_pixelformat, srcPal.palette, srcPal.length, dstPalette, dstPaletteCount, Graphics::kDitherNaive);
		}
		break;
	// 8bpp - if using a different palette, and we're not doing a color cycling operation, convert using nearest colour matching
	case 8:
		// "break" means falling back to the default of rendering the image with
		// the current 8-bit palette. The below is only about -redithering colours-;
		// i.e. redrawing the picture to use the current palette.
		// Only redither 8-bit images in 8-bit mode if we have the remap palette flag set, or it is external
		if (targetBpp == 1 && !movie->_remapPalettesWhenNeeded && !_external)
			break;
		// If we're in 32-bit mode, and not in puppet palette mode, then "redither" as well.
		if (targetBpp == 4 && score->_puppetPalette && !_external)
			break;
		if (_external || (targetBpp == 4) || (castPaletteId != currentPaletteId && !isColorCycling)) {
			const auto pals = g_director->getLoadedPalettes();
			CastMemberID palIndex = pals.contains(castPaletteId) ? castPaletteId : CastMemberID(kClutSystemMac, -1);
			const PaletteV4 &srcPal = pals.getVal(palIndex);

			// If it is an external image, use the included palette.
			// For BMP images especially, they'll often have the right colors
			// but in the wrong palette order.
			const byte *palPtr = _external ? _picture->_palette : srcPal.palette;
			int palCount = _external ? _picture->getPaletteCount() : srcPal.length;
			dither = _picture->_surface.convertTo(g_director->_wm->_pixelformat, palPtr, palCount, dstPalette, dstPaletteCount, Graphics::kDitherNaive);
		}
		break;
	default:
		break;
	}

	if (dither) {
		// Save the palette ID so we can check if a redraw is required
		_ditheredTargetClut = currentPaletteId;

		if (!_external && targetBpp == 1) {
			// Finally, the first and last colours in the palette are special. No matter what the palette remap
			// does, we need to scrub those to be the same.
			const Graphics::Surface *src = &_picture->_surface;
			for (int y = 0; y < src->h; y++) {
				for (int x = 0; x < src->w; x++) {
					const int test = *(const byte *)src->getBasePtr(x, y);
					if (test == 0 || test == (1 << _bitsPerPixel) - 1) {
						*(byte *)dither->getBasePtr(x, y) = test == 0 ? 0x00 : 0xff;
					}
				}
			}
		}
	}
	return dither;

}

bool BitmapCastMember::isModified() {
	if (CastMember::isModified()) {
		// Let's us use "setChanged" when changing the picture through Lingo
		return true;
	}
	// Check for palette changes.
	// If a bitmap has a custom palette assigned to it, createWidget()
	// will dither the image so that it fits within the current palette.
	// When the score palette changes, we need to flag that the widget needs
	// to be recreated.
	if (!_clut.isNull()) {
		Movie *movie = g_director->getCurrentMovie();
		Score *score = movie->getScore();
		CastMemberID currentPaletteId = score->getCurrentPalette();
		if (currentPaletteId.isNull())
			currentPaletteId = movie->_defaultPalette;
		PaletteV4 *currentPalette = g_director->getPalette(currentPaletteId);
		if (!currentPalette) {
			currentPaletteId = CastMemberID(kClutSystemMac, -1);
			currentPalette = g_director->getPalette(currentPaletteId);
		}
		CastMemberID castPaletteId = _clut;
		if (castPaletteId.isNull())
			castPaletteId = movie->_defaultPalette;

		return !_ditheredTargetClut.isNull() && _ditheredTargetClut != currentPaletteId;
	}
	return false;
}

void BitmapCastMember::createMatte(const Common::Rect &bbox) {
	// Like background trans, but all white pixels NOT ENCLOSED by coloured pixels
	// are transparent
	Graphics::Surface tmp;
	tmp.create(bbox.width(), bbox.height(), g_director->_pixelformat);

	copyStretchImg(
		_ditheredImg ? _ditheredImg : &_picture->_surface,
		&tmp,
		_initialRect,
		bbox
	);

	_noMatte = true;

	// Searching white color in the corners
	uint32 whiteColor = 0;
	bool colorFound = false;

	if (g_director->_pixelformat.bytesPerPixel == 1) {
		for (int y = 0; y < tmp.h; y++) {
			for (int x = 0; x < tmp.w; x++) {
				byte color = *(byte *)tmp.getBasePtr(x, y);

				if (g_director->getPalette()[color * 3 + 0] == 0xff &&
						g_director->getPalette()[color * 3 + 1] == 0xff &&
						g_director->getPalette()[color * 3 + 2] == 0xff) {
					whiteColor = color;
					colorFound = true;
					break;
				}
			}
		}
	} else {
		whiteColor = g_director->_wm->_colorWhite;
		colorFound = true;
	}

	if (!colorFound) {
		debugC(1, kDebugImages, "BitmapCastMember::createMatte(): No white color for matte image");
	} else {
		if (_matte) {
			_matte->free();
			delete _matte;
			_matte = nullptr;
		}

		Graphics::FloodFill matteFill(&tmp, whiteColor, 0, true);

		for (int yy = 0; yy < tmp.h; yy++) {
			matteFill.addSeed(0, yy);
			matteFill.addSeed(tmp.w - 1, yy);
		}

		for (int xx = 0; xx < tmp.w; xx++) {
			matteFill.addSeed(xx, 0);
			matteFill.addSeed(xx, tmp.h - 1);
		}

		matteFill.fillMask();
		Graphics::Surface *matteSurf = matteFill.getMask();
		// convert the mask to the same surface format used for 1bpp bitmaps.
		// this uses the director palette scheme, so white is 0x00 and black is 0xff.
		_matte = new Graphics::Surface();
		_matte->create(matteSurf->w, matteSurf->h, Graphics::PixelFormat::createFormatCLUT8());
		for (int y = 0; y < matteSurf->h; y++) {
			for (int x = 0; x < matteSurf->w; x++) {
				_matte->setPixel(x, y, matteSurf->getPixel(x, y) ? 0x00 : 0xff);
			}
		}
		_noMatte = false;
	}

	tmp.free();
}

Graphics::Surface *BitmapCastMember::getMatte(const Common::Rect &bbox) {
	// Lazy loading of mattes
	if (!_matte && !_noMatte) {
		createMatte(bbox);
	}

	// check for the scale matte
	if (_matte && (_matte->w != bbox.width() || _matte->h != bbox.height())) {
		createMatte(bbox);
	}

	return _matte;
}

Common::String BitmapCastMember::formatInfo() {
	if (_version < kFileVer600) {
		return Common::String::format(
			"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, foreColor: %d, backColor: %d, regX: %d, regY: %d, pitch: %d, bitsPerPixel: %d, palette: %s",
			_initialRect.width(), _initialRect.height(),
			_initialRect.left, _initialRect.top,
			_boundingRect.width(), _boundingRect.height(),
			_boundingRect.left, _boundingRect.top,
			getForeColor(), getBackColor(),
			_regX, _regY, _pitch, _bitsPerPixel, _clut.asString().c_str()
		);
	} else {
		return Common::String::format(
			"initialRect: %dx%d@%d,%d, scrollPoint: %d,%d, alphaThreshold: %d, foreColor: %d, backColor: %d, regX: %d, regY: %d, pitch: %d, bitsPerPixel: %d, palette: %s, editVersion: %d, updateFlags: 0x%02x",
			_initialRect.width(), _initialRect.height(),
			_initialRect.left, _initialRect.top,
			_scrollPoint.x, _scrollPoint.y,
			_alphaThreshold,
			getForeColor(), getBackColor(),
			_regX, _regY, _pitch, _bitsPerPixel, _clut.asString().c_str(),
			_editVersion, _updateFlags
		);
	}
}

void BitmapCastMember::load() {
	if (_loaded && !_needsReload)
		return;

	if (_ditheredImg) {
		_ditheredImg->free();
		delete _ditheredImg;
		_ditheredImg = nullptr;
	}

	_needsReload = false;

	uint32 tag = _tag;
	uint16 imgId = _castId;
	uint16 realId = 0;

	Image::ImageDecoder *img = nullptr;
	Common::SeekableReadStream *pic = nullptr;

	if (_cast->_version >= kFileVer400) {
		for (auto &it : _children) {
			if (it.tag == MKTAG('B', 'I', 'T', 'D')) {
				imgId = it.index;
				tag = it.tag;

				pic = _cast->getResource(tag, imgId);
				break;
			}
		}

		Common::String imageFilename = _cast->getLinkedPath(_castId);

		if ((pic == nullptr || pic->size() == 0)
				&& !imageFilename.empty()) {
			// image file is linked, load from the filesystem
			Common::Path location = findPath(imageFilename);
			Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(location);
			if (file) {
				debugC(2, kDebugLoading, "****** Loading file '%s', cast id: %d", imageFilename.c_str(), imgId);
				// Detect the filetype. Director will ignore file extensions, as do we.
				Image::ImageDecoder *decoder = nullptr;
				uint32 fileType = file->readUint32BE();
				file->seek(0);

				if ((fileType >> 16) == MKTAG16('B', 'M')) {
					// Windows Bitmap file
					decoder = new Image::BitmapDecoder();
				} else if ((fileType == 0xffd8ffe0) || (fileType == 0xffd8ffe1) || (fileType == 0xffd8ffe2)) {
					// JPEG file
					decoder = new Image::JPEGDecoder();
				} else {
					// Well... Director allowed someone to add it, so it must be a PICT. No further questions!
					decoder = new Image::PICTDecoder();
				}

				bool res = decoder->loadStream(*file);
				delete file;

				if (res) {
					setPicture(*decoder, decoder->hasPalette());
					_external = true;

					const Graphics::Surface *surf = decoder->getSurface();
					if (decoder->hasPalette()) {
						// For BMPs this sometimes gets set to 16 in the cast record,
						// we should go with what the target image has.
						_bitsPerPixel = 8;
					}

					debugC(5, kDebugImages, "BitmapCastMember::load(): Bitmap: id: %d, w: %d, h: %d, flags1: %x, flags2: %x bytes: %x, bpp: %d clut: %s", imgId, surf->w, surf->h, _flags1, _flags2, _bytes, _bitsPerPixel, _clut.asString().c_str());

					if (ConfMan.getBool("dump_scripts")) {

						Common::String prepend = _cast->getMacName();
						Common::String filename = Common::String::format("./dumps/%s-%s-%d.png", encodePathForDump(prepend).c_str(), tag2str(tag), imgId);
						Common::DumpFile bitmapFile;

						bitmapFile.open(Common::Path(filename), true);
						Image::writePNG(bitmapFile, *decoder->getSurface(), decoder->getPalette().data());

						bitmapFile.close();
					}

					delete pic;
					delete decoder;
					_loaded = true;
					return;
				} else {
					delete decoder;
					warning("BUILDBOT: BitmapCastMember::load(): wrong format for external picture '%s'", location.toString(Common::Path::kNativeSeparator).c_str());
				}
			} else {
				warning("BitmapCastMember::load(): cannot open external picture '%s'", location.toString(Common::Path::kNativeSeparator).c_str());
			}
		} else if ((!pic || (pic->size() == 0)) && (_initialRect.width() == 0) && (_initialRect.height() == 0)) {
			// If an image is 0x0, it doesn't matter if we don't have any data.
			_picture->_surface.create(0, 0, g_director->_wm->_pixelformat);
			delete pic;
			_loaded = true;
			return;
		}
	} else {
		realId = imgId + _cast->_castIDoffset;
		pic = _cast->getResource(tag, realId);
	}

	if (pic == nullptr) {
		warning("BitmapCastMember::load(): Bitmap image %d not found", imgId);
		return;
	}

	int w = _initialRect.width();
	int h = _initialRect.height();

	switch (tag) {
	case MKTAG('D', 'I', 'B', ' '):
		debugC(2, kDebugLoading, "****** Loading 'DIB ' id: %d (%d), %d bytes", imgId, realId, (int)pic->size());
		img = new DIBDecoder();
		break;

	case MKTAG('B', 'I', 'T', 'D'):
		debugC(2, kDebugLoading, "****** Loading 'BITD' id: %d (%d), %d bytes", imgId, realId, (int)pic->size());

		if (w > 0 && h > 0) {
			img = new BITDDecoder(w, h, _bitsPerPixel, _pitch, g_director->getPalette(), _cast->_version);
		} else if (pic->size() == 0) {
			// zero-length bitmap
		} else {
			warning("BitmapCastMember::load(): Bitmap image %d has invalid size", imgId);
		}

		break;

	default:
		warning("BitmapCastMember::load(): Unknown Bitmap CastMember Tag: [%d] %s", tag, tag2str(tag));
		break;
	}

	if (debugChannelSet(7, kDebugLoading)) {
		debug("BitmapCastMember::load(): Bitmap data:");
		pic->hexdump(MIN((int)pic->size(), 512));
	}

	if (!img || !img->loadStream(*pic)) {
		warning("BitmapCastMember::load(): Unable to load id: %d", imgId);
		delete pic;
		delete img;
		return;
	}

	// dumpFile("LoadedBitmap", _castId, MKTAG('B', 'I', 'T', 'D'), (byte *)img->getSurface()->getPixels(), img->getSurface()->h * img->getSurface()->w);

	setPicture(*img, true);

	if (ConfMan.getBool("dump_scripts")) {

		Common::String prepend = _cast->getMacName();
		Common::String filename = Common::String::format("./dumps/%s-%s-%d.png", encodePathForDump(prepend).c_str(), tag2str(tag), imgId);
		Common::DumpFile bitmapFile;

		bitmapFile.open(Common::Path(filename), true);
		Image::writePNG(bitmapFile, *img->getSurface(), img->getPalette());

		bitmapFile.close();
	}

	delete img;
	delete pic;

	debugC(5, kDebugImages, "BitmapCastMember::load(): Bitmap: id: %d, w: %d, h: %d, flags1: %x, flags2: %x bytes: %x, bpp: %d clut: %s", imgId, w, h, _flags1, _flags2, _bytes, _bitsPerPixel, _clut.asString().c_str());

	_loaded = true;
}

void BitmapCastMember::unload() {
	if (!_loaded)
		return;

	delete _picture;
	_picture = new Picture();

	if (_ditheredImg) {
		_ditheredImg->free();
		delete _ditheredImg;
		_ditheredImg = nullptr;
	}

	_loaded = false;
}

PictureReference *BitmapCastMember::getPicture() const {
	auto picture = new PictureReference;

	// Not sure if we can make the assumption that the owning
	// BitmapCastMember will live as long as any reference,
	// so we'll make a copy of the Picture.
	picture->_picture = new Picture(*_picture);

	return picture;
}

void BitmapCastMember::setPicture(PictureReference &picture) {
	delete _picture;
	_picture = new Picture(*picture._picture);

	// Force redither
	if (_ditheredImg) {
		_ditheredImg->free();
		delete _ditheredImg;
		_ditheredImg = nullptr;
	}

	// Make sure we get redrawn
	setModified(true);
	// TODO: Should size be adjusted?
}

void BitmapCastMember::setPicture(Image::ImageDecoder &image, bool adjustSize) {
	delete _picture;
	_picture = new Picture(image);
	if (adjustSize) {
		auto surf = image.getSurface();
		_size = surf->pitch * surf->h + _picture->getPaletteSize();
	}
	// Make sure we get redrawn
	setModified(true);
}

Common::Point BitmapCastMember::getRegistrationOffset() {
	return Common::Point(_regX - _initialRect.left, _regY - _initialRect.top);
}

Common::Point BitmapCastMember::getRegistrationOffset(int16 width, int16 height) {
	Common::Point offset = getRegistrationOffset();
	return Common::Point(offset.x * width / MAX((int16)1, _initialRect.width()), offset.y * height / MAX((int16)1, _initialRect.height()));
}


CollisionTest BitmapCastMember::isWithin(const Common::Rect &bbox, const Common::Point &pos, InkType ink) {
	if (!bbox.contains(pos))
		return kCollisionNo;

	if (ink == kInkTypeMatte) {
		Graphics::Surface *matte = getMatte(bbox);
		return (matte ? *(byte *)(matte->getBasePtr(pos.x - bbox.left, pos.y - bbox.top)) : true) ? kCollisionYes : kCollisionNo;
	}
	return kCollisionYes;
}

bool BitmapCastMember::hasField(int field) {
	switch (field) {
	case kTheDepth:
	case kTheRegPoint:
	case kThePalette:
	case kThePaletteRef:
	case kThePicture:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum BitmapCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheDepth:
		d = _bitsPerPixel;
		break;
	case kTheRegPoint:
		d.type = POINT;
		d.u.farr = new FArray;
		d.u.farr->arr.push_back(_regX);
		d.u.farr->arr.push_back(_regY);
		break;
	case kThePalette:
		// D5 and below return an integer for this field
		if (_clut.castLib > 0) {
			d = Datum(_clut.toMultiplex());
		} else {
			d = Datum(_clut.member);
		}
		break;
	case kThePaletteRef:
		if (_clut.castLib > 0) {
			d = _clut;
		} else if (_clut.castLib == -1) {
			switch (_clut.member) {
			case kClutSystemMac:
				d = Datum("systemMac");
				d.type = SYMBOL;
				break;
			case kClutSystemWin:
				d = Datum("systemWinDir4");
				d.type = SYMBOL;
				break;
			case kClutSystemWinD5:
				d = Datum("systemWin");
				d.type = SYMBOL;
				break;
			case kClutGrayscale:
				d = Datum("grayscale");
				d.type = SYMBOL;
				break;
			case kClutMetallic:
				d = Datum("metallic");
				d.type = SYMBOL;
				break;
			case kClutNTSC:
				d = Datum("NTSC");
				d.type = SYMBOL;
				break;
			case kClutPastels:
				d = Datum("pastels");
				d.type = SYMBOL;
				break;
			case kClutRainbow:
				d = Datum("rainbow");
				d.type = SYMBOL;
				break;
			case kClutVivid:
				d = Datum("vivid");
				d.type = SYMBOL;
				break;
			default:
				break;
			}
		}
		break;
	case kThePicture:
		d.type = PICTUREREF;
		d.u.picture = getPicture();
		break;
	default:
		d = CastMember::getField(field);
	}

	return d;
}

void BitmapCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheDepth:
		warning("BitmapCastMember::setField(): Attempt to set read-only field %s of cast %d", g_lingo->field2str(field), _castId);
		return;
	case kTheRegPoint:
		if (d.type == POINT || (d.type == ARRAY && d.u.farr->arr.size() >= 2)) {
			Score *score = g_director->getCurrentMovie()->getScore();
			score->invalidateRectsForMember(this);
			_regX = d.u.farr->arr[0].asInt();
			_regY = d.u.farr->arr[1].asInt();
			_modified = true;
		} else {
			warning("BitmapCastMember::setField(): Wrong Datum type %d for kTheRegPoint", d.type);
		}
		return;
	case kThePalette:
		{
			CastMemberID newClut;
			if (d.isCastRef()) {
				newClut = *d.u.cast;
			} else {
				int id = d.asInt();
				if (id > 0) {
					// For palette IDs, D5 and above use multiples of 0x20000 to denote
					// the castLib in the integer representation
					newClut = CastMemberID().fromMultiplex(id);
				} else if (id < 0) {
					// Negative integer refers to one of the builtin palettes
					newClut = CastMemberID(id, -1);
				} else {
					// 0 indicates a fallback to the default palette settings
					newClut = CastMemberID(0, 0);
				}
			}
			if (newClut != _clut) {
				_clut = newClut;
				_modified = true;
			}
			return;
		}
	case kThePaletteRef:
		{
			CastMemberID newClut = _clut;
			if (d.isCastRef()) {
				newClut = *d.u.cast;
			} else if (d.type == SYMBOL) {
				Common::String name = *d.u.s;
				if (name.equalsIgnoreCase("systemMac")) {
					newClut = CastMemberID(kClutSystemMac, -1);
				} else if (name.equalsIgnoreCase("systemWinDir4")) {
					newClut = CastMemberID(kClutSystemWin, -1);
				} else if (name.equalsIgnoreCase("systemWin")) {
					newClut = CastMemberID(kClutSystemWinD5, -1);
				} else if (name.equalsIgnoreCase("grayscale")) {
					newClut = CastMemberID(kClutGrayscale, -1);
				} else if (name.equalsIgnoreCase("metallic")) {
					newClut = CastMemberID(kClutMetallic, -1);
				} else if (name.equalsIgnoreCase("NTSC")) {
					newClut = CastMemberID(kClutNTSC, -1);
				} else if (name.equalsIgnoreCase("pastels")) {
					newClut = CastMemberID(kClutPastels, -1);
				} else if (name.equalsIgnoreCase("rainbow")) {
					newClut = CastMemberID(kClutRainbow, -1);
				} else if (name.equalsIgnoreCase("vivid")) {
					newClut = CastMemberID(kClutVivid, -1);
				}
			}
			if (newClut != _clut) {
				_clut = newClut;
				_modified = true;
			}
		}
		return;
	case kThePicture:
		if (d.type == PICTUREREF && d.u.picture != nullptr) {
			setPicture(*d.u.picture);
			// This is a random PICT from somewhere,
			// set the external flag so we remap the palette.
			_external = true;
			// Remove the canvas-space transformation
			_regX -= _initialRect.left;
			_regY -= _initialRect.top;
			_initialRect = Common::Rect(_picture->_surface.w, _picture->_surface.h);
		} else {
			warning("BitmapCastMember::setField(): Wrong Datum type %d for kThePicture (or nullptr)", d.type);
		}
		return;
	default:
		break;
	}

	CastMember::setField(field, d);
}

uint32 BitmapCastMember::getCastDataSize() {
	// _pitch : 2 bytes
	// _initialRect : 8 bytes
	// _boundingRect : 8 bytes
	// _regY : 2 bytes
	// _regX : 2 bytes
	// Total: 22 bytes
	// For Director 4 : 2 byte extra for casttype and flags (See Cast::loadCastData())
	uint32 dataSize = 22 + 2;

	if (_bitsPerPixel != 0) {
		dataSize += 4;
		// if (_cast->_version >= kFileVer500) {
		// 	dataSize += 2;		// Added two bytes for _clut.member
		// 	dataSize -= 2;		// Removed two bytes for _castType and _flags (See Cast::loadCastData())
		// }

		if (_flags2 != 0) {
			dataSize += 16;
		}
	}
	return dataSize;
}

void BitmapCastMember::writeCastData(Common::SeekableWriteStream *writeStream) {
	writeStream->writeUint16BE(_pitch);

	Movie::writeRect(writeStream, _initialRect);
	Movie::writeRect(writeStream, _boundingRect);

	writeStream->writeUint16BE(_regY);
	writeStream->writeUint16BE(_regX);

	warning("BitmapCastMember::writeCastData(): TODO process D6+");

	if (_bitsPerPixel != 0) {
		writeStream->writeByte(0);		// Skip one byte (not stored)
		writeStream->writeByte(_bitsPerPixel);

		if (_cast->_version >= kFileVer500) {
			if (_clut.castLib == _cast->_castLibID) {
				writeStream->writeSint16BE(-1);
			} else {
				writeStream->writeSint16BE(_clut.castLib);
			}
		}

		if (_clut.member > 0) {
			writeStream->writeSint16BE(_clut.member);
		} else {	// builtin palette
			writeStream->writeSint16BE(_clut.member + 1);
		}

		if (_flags2 != 0) {
			// Skipping 14 bytes because they are not stored in ScummVM Director
			// May need to save in the future, see BitCastMember::BitCastMember constructor
			writeStream->writeUint64BE(0);
			writeStream->writeUint32BE(0);
			writeStream->writeUint16BE(0);
			writeStream->writeUint16BE(_flags2);
		}
	}
	// Ignoring the tail during loading as well as saving
}

uint32 BitmapCastMember::writeBITDResource(Common::SeekableWriteStream *writeStream, uint32 offset) {
	// Load it before writing
	if (!_loaded) {
		load();
	}

	writeStream->seek(offset);

	writeStream->writeUint32LE(MKTAG('B', 'I', 'T', 'D'));
	writeStream->writeUint32LE(getBITDResourceSize());

	if (_external) {
		warning("BitmapCastMember::writeBITDResource: the bitmap is external, ignoring for now");
		return 8;		// 8 for the tag and size
	}

	// No compression for now
	// pixels.size() == bytes needed
	Graphics::Surface pixels;
	Graphics::PixelFormat format;

	if (_bitsPerPixel >> 3) {
		format.bytesPerPixel = _bitsPerPixel >> 3;
		pixels.create(_picture->_surface.w, _picture->_surface.h, format);
	} else {
		format.bytesPerPixel = 1;
		pixels.create(_pitch, _picture->_surface.h, format);
	}

	offset = 0;

	if (_bitsPerPixel == 8 && _picture->_surface.w < (int)(_pitch * _picture->_surface.h / _picture->_surface.h)) {
		offset = (_pitch - _picture->_surface.w) % 2;
	}

	debugC(5, kDebugSaving, "BitmapCastMember::writeBITDResource: Saving 'BITD' Resource: bitsPerPixel: %d, castId: %d", _bitsPerPixel, _castId);
	for (int y = 0; y < _picture->_surface.h; y++) {
		byte *ptr = (byte *)pixels.getBasePtr(0, y);

		for (int x = 0; x < _picture->_surface.w;) {
			uint32 color = 0;

			switch (_bitsPerPixel) {
			case 1:
				for (int c = 0; c < 8 && x < _picture->_surface.w; c++, x++) {
					color += (*((byte *)_picture->_surface.getBasePtr(x, y))) & (1 << (7 - c));
				}
				*ptr = color; ptr++;
				break;

			case 2:
				for (int c = 0; c < 4 && x < _picture->_surface.w; c++, x++) {
					color += (*((byte *)_picture->_surface.getBasePtr(x, y)) & 0x3) << (2 * (3 - c));
				}
				*ptr = color; ptr++;
				break;

			case 4:
				for (int c = 0; c < 2 && x < _picture->_surface.w; c++, x++) {
					color += (*((byte *)_picture->_surface.getBasePtr(x, y)) & 0xF) << (4 * (1 - c));
				}
				*ptr = color; ptr++;
				break;

			case 8:
				*(ptr + (y * offset)) = *((byte *)_picture->_surface.getBasePtr(x, y));
				ptr++; x++;
				break;

			case 16:
				color = *((uint16 *)_picture->_surface.getBasePtr(x, y));
				*ptr = color >> 8; ptr++;
				*ptr = color & 0xFF; ptr++;
				x++;
				break;

			case 32:
				color = *((uint32 *)_picture->_surface.getBasePtr(x, y));
				ptr++;		// Ignore the Alpha value
				*ptr = (color >> 16) & 0xFF; ptr++;
				*ptr = (color >> 8) & 0xFF; ptr++;
				*ptr = color & 0xFF; ptr++;
				x++;
				break;

			default:
				x++;
				break;
			}
		}
	}

	writeStream->write(pixels.getPixels(), _picture->_surface.h * _pitch);

	if (debugChannelSet(7, kDebugSaving)) {
		dumpFile("BitmapData", _castId, MKTAG('B', 'I', 'T', 'D'), (byte *)pixels.getPixels(), _picture->_surface.h * _pitch);
	}
	return 0;
}

uint32 BitmapCastMember::getBITDResourceSize() {
	if (_external) {
		return 0;
	}

	// No compression for now
	return _pitch * _picture->_surface.h;
}

} // End of namespace Director
