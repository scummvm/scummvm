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

#include "common/macresman.h"
#include "graphics/surface.h"
#include "graphics/macgui/macwidget.h"
#include "image/bmp.h"
#include "image/image_decoder.h"
#include "image/jpeg.h"
#include "image/pict.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/images.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/score.h"
#include "director/window.h"
#include "director/castmember/bitmap.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-the.h"

namespace Director {

BitmapCastMember::BitmapCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint32 castTag, uint16 version, uint8 flags1)
		: CastMember(cast, castId, stream) {
	_type = kCastBitmap;
	_picture = nullptr;
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

	if (version < kFileVer400) {
		_flags1 = flags1;	// region: 0 - auto, 1 - matte, 2 - disabled

		_bytes = stream.readUint16();
		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

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
		_pitch = stream.readUint16();
		_pitch &= 0x0fff;

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		_bitsPerPixel = stream.readUint16();

		if (stream.eos()) {
			_bitsPerPixel = 0;
		} else {
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
			stream.readUint16();
			/* uint16 unk1 = */ stream.readUint16();
			stream.readUint16();

			stream.readUint32();
			stream.readUint32();

			_flags2 = stream.readUint16();
		}

		if (_bitsPerPixel == 0)
			_bitsPerPixel = 1;

		int tail = 0;
		byte buf[256];

		while (!stream.eos()) {
			byte c = stream.readByte();
			if (tail < 256)
				buf[tail] = c;
			tail++;
		}

		if (tail)
			warning("BUILDBOT: BitmapCastMember: %d bytes left", tail);

		if (tail && debugChannelSet(2, kDebugLoading)) {
			debug("BitmapCastMember: tail");
			Common::hexdump(buf, tail);
		}
	} else {
		warning("STUB: BitmapCastMember::BitmapCastMember(): Bitmaps not yet supported for version %d", version);
	}

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
}

BitmapCastMember::~BitmapCastMember() {
	delete _picture;

	if (_ditheredImg) {
		_ditheredImg->free();
		delete _ditheredImg;
	}

	if (_matte)
		delete _matte;
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
	if (_ditheredImg) {
		_ditheredImg->free();
		delete _ditheredImg;
		_ditheredImg = nullptr;
		_ditheredTargetClut = CastMemberID(0, 0);
	}

	if (dstBpp == 1) {
		if (srcBpp > 1
		// At least early directors were not remapping 8bpp images. But in case it is
		// needed, here is the code
#if 0
		|| (srcBpp == 1 &&
			memcmp(g_director->_wm->getPalette(), _img->_palette, _img->_paletteSize))
#endif
			) {

			_ditheredImg = _picture->_surface.convertTo(g_director->_wm->_pixelformat, _picture->_palette, _picture->_paletteColors, g_director->_wm->getPalette(), g_director->_wm->getPaletteSize());

			pal = g_director->_wm->getPalette();
		} else {
			// Convert indexed image to indexed palette
			Movie *movie = g_director->getCurrentMovie();
			Cast *cast = movie->getCast();
			Score *score = movie->getScore();
			// Get the current score palette. Note that this is the ID of the palette in the list, not the cast member!
			CastMemberID currentPaletteId = score->getCurrentPalette();
			if (currentPaletteId.isNull())
				currentPaletteId = cast->_defaultPalette;
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

			// First, check if the palettes are different
			switch (_bitsPerPixel) {
			// 1bpp - this is preconverted to 0x00 and 0xff, change nothing.
			case 1:
				break;
			// 2bpp - convert to nearest using the standard 2-bit palette.
			case 2:
				{
					const PaletteV4 &srcPal = g_director->getLoaded4Palette();
					_ditheredImg = _picture->_surface.convertTo(g_director->_wm->_pixelformat, srcPal.palette, srcPal.length, currentPalette->palette, currentPalette->length, Graphics::kDitherNaive);
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
					_ditheredImg = _picture->_surface.convertTo(g_director->_wm->_pixelformat, srcPal.palette, srcPal.length, currentPalette->palette, currentPalette->length, Graphics::kDitherNaive);
				}
				break;
			// 8bpp - if using a different palette, and we're not doing a color cycling operation, convert using nearest colour matching
			case 8:
				// Only redither 8-bit images if we have the flag set, or it is external
				if (!movie->_remapPalettesWhenNeeded && !_external)
					break;
				if (_external || (castPaletteId != currentPaletteId && !isColorCycling)) {
					const auto pals = g_director->getLoadedPalettes();
					CastMemberID palIndex = pals.contains(castPaletteId) ? castPaletteId : CastMemberID(kClutSystemMac, -1);
					const PaletteV4 &srcPal = pals.getVal(palIndex);

					// If it is an external image, use the included palette.
					// For BMP images especially, they'll often have the right colors
					// but in the wrong palette order.
					const byte *palPtr = _external ? pal : srcPal.palette;
					int palLength = _external ? _picture->getPaletteSize() : srcPal.length;
					_ditheredImg = _picture->_surface.convertTo(g_director->_wm->_pixelformat, palPtr, palLength, currentPalette->palette, currentPalette->length, Graphics::kDitherNaive);
				}
				break;
			default:
				break;
			}

			if (_ditheredImg) {
				debugC(4, kDebugImages, "BitmapCastMember::createWidget(): Dithering image from source palette %s to target palette %s", _clut.asString().c_str(), score->getCurrentPalette().asString().c_str());
				// Save the palette ID so we can check if a redraw is required
				_ditheredTargetClut = currentPaletteId;

				if (!_external) {
					// Finally, the first and last colours in the palette are special. No matter what the palette remap
					// does, we need to scrub those to be the same.
					const Graphics::Surface *src = &_picture->_surface;
					for (int y = 0; y < src->h; y++) {
						for (int x = 0; x < src->w; x++) {
							const int test = *(const byte *)src->getBasePtr(x, y);
							if (test == 0 || test == (1 << _bitsPerPixel) - 1) {
								*(byte *)_ditheredImg->getBasePtr(x, y) = test == 0 ? 0x00 : 0xff;
							}
						}
					}
				}
			} else if (previouslyDithered) {
				debugC(4, kDebugImages, "BitmapCastMember::createWidget(): Removed dithered image, score palette %s matches cast member", score->getCurrentPalette().asString().c_str());
			}

		}
	}

	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);

	// scale for drawing a different size sprite
	copyStretchImg(widget->getSurface()->surfacePtr(), bbox, pal);

	return widget;
}

void BitmapCastMember::copyStretchImg(Graphics::Surface *surface, const Common::Rect &bbox, const byte *pal) {
	const Graphics::Surface *srcSurf;

	if (_ditheredImg)
		srcSurf = _ditheredImg;
	else
		srcSurf = &_picture->_surface;

	if (bbox.width() != _initialRect.width() || bbox.height() != _initialRect.height()) {

		int scaleX = SCALE_THRESHOLD * _initialRect.width() / bbox.width();
		int scaleY = SCALE_THRESHOLD * _initialRect.height() / bbox.height();

		for (int y = 0, scaleYCtr = 0; y < bbox.height(); y++, scaleYCtr += scaleY) {
			if (g_director->_wm->_pixelformat.bytesPerPixel == 1) {
				for (int x = 0, scaleXCtr = 0; x < bbox.width(); x++, scaleXCtr += scaleX) {
					const byte *src = (const byte *)srcSurf->getBasePtr(scaleXCtr / SCALE_THRESHOLD, scaleYCtr / SCALE_THRESHOLD);
					*(byte *)surface->getBasePtr(x, y) = *src;
				}
			} else {
				for (int x = 0, scaleXCtr = 0; x < bbox.width(); x++, scaleXCtr += scaleX) {
					const void *ptr = srcSurf->getBasePtr(scaleXCtr / SCALE_THRESHOLD, scaleYCtr / SCALE_THRESHOLD);
					int32 color;

					switch (srcSurf->format.bytesPerPixel) {
					case 1:
						{
							color = *(const byte *)ptr * 3;
							color = surface->format.RGBToColor(pal[color], pal[color + 1], pal[color + 2]);
						}
						break;
					case 4:
						color = *(const int32 *)ptr;
						break;
					default:
						error("Unimplemented src bpp: %d", srcSurf->format.bytesPerPixel);
					}

					*(int32 *)surface->getBasePtr(x, y) = color;
				}
			}
		}
	} else {
		surface->copyFrom(*srcSurf);
	}

	if (g_director->_debugDraw & kDebugDrawCast) {
		surface->frameRect(Common::Rect(0, 0, surface->w, surface->h), g_director->_wm->_colorWhite);

		const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
		font->drawString(surface, Common::String::format("%d", _castId), 2, 2, 10, g_director->_wm->_colorWhite);
	}
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
		Cast *cast = movie->getCast();
		Score *score = movie->getScore();
		CastMemberID currentPaletteId = score->getCurrentPalette();
		if (currentPaletteId.isNull())
			currentPaletteId = cast->_defaultPalette;
		PaletteV4 *currentPalette = g_director->getPalette(currentPaletteId);
		if (!currentPalette) {
			currentPaletteId = CastMemberID(kClutSystemMac, -1);
			currentPalette = g_director->getPalette(currentPaletteId);
		}
		CastMemberID castPaletteId = _clut;
		if (castPaletteId.isNull())
			castPaletteId = cast->_defaultPalette;

		if (currentPaletteId == castPaletteId) {
			return !_ditheredTargetClut.isNull();
		} else {
			return _ditheredTargetClut != currentPaletteId;
		}
	}
	return false;
}

void BitmapCastMember::createMatte(Common::Rect &bbox) {
	// Like background trans, but all white pixels NOT ENCLOSED by coloured pixels
	// are transparent
	Graphics::Surface tmp;
	tmp.create(bbox.width(), bbox.height(), g_director->_pixelformat);

	copyStretchImg(&tmp, bbox);

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
		delete _matte;

		_matte = new Graphics::FloodFill(&tmp, whiteColor, 0, true);

		for (int yy = 0; yy < tmp.h; yy++) {
			_matte->addSeed(0, yy);
			_matte->addSeed(tmp.w - 1, yy);
		}

		for (int xx = 0; xx < tmp.w; xx++) {
			_matte->addSeed(xx, 0);
			_matte->addSeed(xx, tmp.h - 1);
		}

		_matte->fillMask();
		_noMatte = false;
	}

	tmp.free();
}

Graphics::Surface *BitmapCastMember::getMatte(Common::Rect &bbox) {
	// Lazy loading of mattes
	if (!_matte && !_noMatte) {
		createMatte(bbox);
	}

	// check for the scale matte
	Graphics::Surface *surface = _matte ? _matte->getMask() : nullptr;
	if (surface && (surface->w != bbox.width() || surface->h != bbox.height())) {
		createMatte(bbox);
	}

	return _matte ? _matte->getMask() : nullptr;
}

Common::String BitmapCastMember::formatInfo() {
	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, foreColor: %d, backColor: %d, regX: %d, regY: %d, pitch: %d, bitsPerPixel: %d, palette: %s",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		getForeColor(), getBackColor(),
		_regX, _regY, _pitch, _bitsPerPixel, _clut.asString().c_str()
	);
}

void BitmapCastMember::load() {
	if (_loaded)
		return;

	uint32 tag = _tag;
	uint16 imgId = _castId;
	uint16 realId = 0;

	Image::ImageDecoder *img = nullptr;
	Common::SeekableReadStream *pic = nullptr;

	if (_cast->_version >= kFileVer400) {
		if (_children.size() > 0) {
			imgId = _children[0].index;
			tag = _children[0].tag;

			pic = _cast->getResource(tag, imgId);
		}

		CastMemberInfo *ci = _cast->getCastMemberInfo(_castId);

		if ((pic == nullptr || pic->size() == 0)
				&& ci && !ci->fileName.empty()) {
			// image file is linked, load from the filesystem
			Common::String filename = ci->fileName;
			Common::String directory = ci->directory;

			Common::String imageFilename = directory + g_director->_dirSeparator + filename;

			Common::Path path = Common::Path(pathMakeRelative(imageFilename), g_director->_dirSeparator);

			Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(path);
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
					delete pic;
					delete decoder;
					_loaded = true;
					return;
				} else {
					delete decoder;
					warning("BUILDBOT: BitmapCastMember::load(): wrong format for external picture '%s'", path.toString().c_str());
				}
			} else {
				warning("BitmapCastMember::load(): cannot open external picture '%s'", path.toString().c_str());
			}
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
			if (_cast->_version < kFileVer600) {
				img = new BITDDecoder(w, h, _bitsPerPixel, _pitch, g_director->getPalette(), _cast->_version);
			} else {
				img = new Image::BitmapDecoder();
			}
		} else {
			warning("BitmapCastMember::load(): Bitmap image %d not found", imgId);
		}

		break;

	default:
		warning("BitmapCastMember::load(): Unknown Bitmap CastMember Tag: [%d] %s", tag, tag2str(tag));
		break;
	}

	if (!img || !img->loadStream(*pic)) {
		warning("BitmapCastMember::load(): Unable to load id: %d", imgId);
		delete pic;
		delete img;
		return;
	}

	setPicture(*img, true);

	delete img;
	delete pic;

	debugC(5, kDebugImages, "BitmapCastMember::load(): Bitmap: id: %d, w: %d, h: %d, flags1: %x, flags2: %x bytes: %x, bpp: %d clut: %s", imgId, w, h, _flags1, _flags2, _bytes, _bitsPerPixel, _clut.asString().c_str());

	_loaded = true;
}

void BitmapCastMember::unload() {
	if (!_loaded)
		return;

	delete _picture;
	_picture = nullptr;

	delete _ditheredImg;
	_ditheredImg = nullptr;

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
	delete _ditheredImg;
	_ditheredImg = nullptr;

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

bool BitmapCastMember::hasField(int field) {
	switch (field) {
	case kTheDepth:
	case kTheRegPoint:
	case kThePalette:
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
		d = _clut;
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

bool BitmapCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheDepth:
		warning("BitmapCastMember::setField(): Attempt to set read-only field %s of cast %d", g_lingo->field2str(field), _castId);
		return false;
	case kTheRegPoint:
		if (d.type == POINT || (d.type == ARRAY && d.u.farr->arr.size() >= 2)) {
			Score *score = g_director->getCurrentMovie()->getScore();
			score->invalidateRectsForMember(this);
			_regX = d.u.farr->arr[0].asInt();
			_regY = d.u.farr->arr[1].asInt();
			_modified = true;
		} else {
			warning("BitmapCastMember::setField(): Wrong Datum type %d for kTheRegPoint", d.type);
			return false;
		}
		return true;
	case kThePalette:
		{
			// FIXME: not multicast safe
			int id = d.asInt();
			if (id > 0) {
				_clut = CastMemberID(d.asInt(), DEFAULT_CAST_LIB);
			} else if (id < 0) {
				_clut = CastMemberID(d.asInt(), -1);
			} else {
				_clut = CastMemberID(0, 0);
			}
			return true;
		}
	case kThePicture:
		if (d.type == PICTUREREF && d.u.picture != nullptr) {
			setPicture(*d.u.picture);
			return true;
		} else {
			warning("BitmapCastMember::setField(): Wrong Datum type %d for kThePicture (or nullptr)", d.type);
		}
		return false;
	default:
		break;
	}

	return CastMember::setField(field, d);
}

} // End of namespace Director
