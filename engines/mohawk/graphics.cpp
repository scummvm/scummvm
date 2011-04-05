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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "mohawk/resource.h"
#include "mohawk/graphics.h"
#include "mohawk/livingbooks.h"

#include "common/substream.h"
#include "engines/util.h"
#include "graphics/primitives.h"
#include "gui/message.h"

#ifdef ENABLE_CSTIME
#include "mohawk/cstime.h"
#endif

#ifdef ENABLE_MYST
#include "mohawk/myst.h"
#include "graphics/jpeg.h"
#endif

#ifdef ENABLE_RIVEN
#include "mohawk/riven.h"
#endif

namespace Mohawk {

MohawkSurface::MohawkSurface() : _surface(0), _palette(0) {
	_offsetX = 0;
	_offsetY = 0;
}

MohawkSurface::MohawkSurface(Graphics::Surface *surface, byte *palette, int offsetX, int offsetY) : _palette(palette), _offsetX(offsetX), _offsetY(offsetY) {
	assert(surface);

	_surface = surface;
}

MohawkSurface::~MohawkSurface() {
	free(_palette);

	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

void MohawkSurface::convertToTrueColor() {
	assert(_surface);

	if (_surface->bytesPerPixel > 1)
		return;

	assert(_palette);

	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_surface->w, _surface->h, pixelFormat.bytesPerPixel);

	for (uint16 i = 0; i < _surface->h; i++) {
		for (uint16 j = 0; j < _surface->w; j++) {
			byte palIndex = *((byte *)_surface->pixels + i * _surface->pitch + j);
			byte r = _palette[palIndex * 3 + 0];
			byte g = _palette[palIndex * 3 + 1];
			byte b = _palette[palIndex * 3 + 2];
			if (pixelFormat.bytesPerPixel == 2)
				*((uint16 *)surface->getBasePtr(j, i)) = pixelFormat.RGBToColor(r, g, b);
			else
				*((uint32 *)surface->getBasePtr(j, i)) = pixelFormat.RGBToColor(r, g, b);
		}
	}

	// Free everything and set the new surface as the converted surface
	_surface->free();
	delete _surface;
	free(_palette);
	_palette = 0;
	_surface = surface;
}

GraphicsManager::GraphicsManager() {
}

GraphicsManager::~GraphicsManager() {
	clearCache();
}

void GraphicsManager::clearCache() {
	for (Common::HashMap<uint16, MohawkSurface*>::iterator it = _cache.begin(); it != _cache.end(); it++)
		delete it->_value;
	for (Common::HashMap<uint16, Common::Array<MohawkSurface*> >::iterator it = _subImageCache.begin(); it != _subImageCache.end(); it++) {
		Common::Array<MohawkSurface *> &array = it->_value;
		for (uint i = 0; i < array.size(); i++)
			delete array[i];
	}

	_cache.clear();
	_subImageCache.clear();
}

MohawkSurface *GraphicsManager::findImage(uint16 id) {
	if (!_cache.contains(id))
		_cache[id] = decodeImage(id);

	// TODO: Probably would be nice to limit the size of the cache
	// Currently, this can't get large because it is freed on every
	// card/stack change in Myst/Riven so I'm not worried about it.
	// Doesn't mean this shouldn't be done in the future.

	return _cache[id];
}

Common::Array<MohawkSurface *> GraphicsManager::decodeImages(uint16 id) {
	error("decodeImages not implemented for this game");
}

void GraphicsManager::preloadImage(uint16 image) {
	findImage(image);
}

void GraphicsManager::setPalette(uint16 id) {
	Common::SeekableReadStream *tpalStream = getVM()->getResource(ID_TPAL, id);

	uint16 colorStart = tpalStream->readUint16BE();
	uint16 colorCount = tpalStream->readUint16BE();
	byte *palette = new byte[colorCount * 3];

	for (uint16 i = 0; i < colorCount; i++) {
		palette[i * 3 + 0] = tpalStream->readByte();
		palette[i * 3 + 1] = tpalStream->readByte();
		palette[i * 3 + 2] = tpalStream->readByte();
		tpalStream->readByte();
	}

	delete tpalStream;

	getVM()->_system->getPaletteManager()->setPalette(palette, colorStart, colorCount);
	delete[] palette;
}

void GraphicsManager::copyAnimImageToScreen(uint16 image, int left, int top) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	Common::Rect srcRect(0, 0, surface->w, surface->h);
	Common::Rect dstRect(left, top, left + surface->w, top + surface->h);
	copyAnimImageSectionToScreen(image, srcRect, dstRect);
}

void GraphicsManager::copyAnimImageSectionToScreen(uint16 image, Common::Rect srcRect, Common::Rect dstRect) {
	copyAnimImageSectionToScreen(findImage(image), srcRect, dstRect);
}

void GraphicsManager::copyAnimSubImageToScreen(uint16 image, uint16 subimage, int left, int top) {
	if (!_subImageCache.contains(image))
		_subImageCache[image] = decodeImages(image);
	Common::Array<MohawkSurface *> &images = _subImageCache[image];

	Graphics::Surface *surface = images[subimage]->getSurface();

	Common::Rect srcRect(0, 0, surface->w, surface->h);
	Common::Rect dstRect(left, top, left + surface->w, top + surface->h);
	copyAnimImageSectionToScreen(images[subimage], srcRect, dstRect);
}

void GraphicsManager::getSubImageSize(uint16 image, uint16 subimage, uint16 &width, uint16 &height) {
	if (!_subImageCache.contains(image))
		_subImageCache[image] = decodeImages(image);
	Common::Array<MohawkSurface *> &images = _subImageCache[image];

	Graphics::Surface *surface = images[subimage]->getSurface();
	width = surface->w;
	height = surface->h;
}

void GraphicsManager::copyAnimImageSectionToScreen(MohawkSurface *image, Common::Rect srcRect, Common::Rect dstRect) {
	uint16 startX = 0;
	uint16 startY = 0;

	assert(srcRect.isValidRect() && dstRect.isValidRect());
	assert(srcRect.left >= 0 && srcRect.top >= 0);

	// TODO: clip rect
	if (dstRect.left < 0) {
		startX -= dstRect.left;
		dstRect.left = 0;
	}

	if (dstRect.top < 0) {
		startY -= dstRect.top;
		dstRect.top = 0;
	}

	if (dstRect.left >= getVM()->_system->getWidth())
		return;
	if (dstRect.top >= getVM()->_system->getHeight())
		return;

	Graphics::Surface *surface = image->getSurface();
	if (startX >= surface->w)
		return;
	if (startY >= surface->h)
		return;

	if (srcRect.left > surface->w)
		return;
	if (srcRect.top > surface->h)
		return;
	if (srcRect.right > surface->w)
		srcRect.right = surface->w;
	if (srcRect.bottom > surface->h)
		srcRect.bottom = surface->h;

	uint16 width = MIN<int>(srcRect.right - srcRect.left - startX, getVM()->_system->getWidth() - dstRect.left);
	uint16 height = MIN<int>(srcRect.bottom - srcRect.top - startY, getVM()->_system->getHeight() - dstRect.top);

	byte *surf = (byte *)surface->getBasePtr(0, srcRect.top + startY);
	Graphics::Surface *screen = getVM()->_system->lockScreen();

	// image and screen should always be 8bpp
	for (uint16 y = 0; y < height; y++) {
		byte *dest = (byte *)screen->getBasePtr(dstRect.left, dstRect.top + y);
		byte *src = surf + srcRect.left + startX;
		// blit, with 0 being transparent
		for (uint16 x = 0; x < width; x++) {
			if (*src)
				*dest = *src;
			src++;
			dest++;
		}
		surf += surface->pitch;
	}

	getVM()->_system->unlockScreen();
}

void GraphicsManager::addImageToCache(uint16 id, MohawkSurface *surface) {
	if (_cache.contains(id))
		error("Image %d already in cache", id);

	_cache[id] = surface;
}

#ifdef ENABLE_MYST

MystGraphics::MystGraphics(MohawkEngine_Myst* vm) : GraphicsManager(), _vm(vm) {
	_bmpDecoder = new MystBitmap();

	_viewport = Common::Rect(544, 332);

	// The original version of Myst could run in 8bpp color too.
	// However, it dithered videos to 8bpp and they looked considerably
	// worse (than they already did :P). So we're not even going to
	// support 8bpp mode in Myst (Myst ME required >8bpp anyway).
	initGraphics(_viewport.width(), _viewport.height(), true, NULL); // What an odd screen size!

	_pixelFormat = _vm->_system->getScreenFormat();

	if (_pixelFormat.bytesPerPixel == 1)
		error("Myst requires greater than 256 colors to run");

	if (_vm->getFeatures() & GF_ME) {
		_jpegDecoder = new Graphics::JPEG();
		_pictDecoder = new Graphics::PictDecoder(_pixelFormat);
	} else {
		_jpegDecoder = NULL;
		_pictDecoder = NULL;
	}

	_pictureFile.entries = NULL;

	// Initialize our buffer
	_backBuffer = new Graphics::Surface();
	_backBuffer->create(_vm->_system->getWidth(), _vm->_system->getHeight(), _pixelFormat.bytesPerPixel);
}

MystGraphics::~MystGraphics() {
	delete _bmpDecoder;
	delete _jpegDecoder;
	delete _pictDecoder;
	delete[] _pictureFile.entries;

	_backBuffer->free();
	delete _backBuffer;
}

static const char* picFileNames[] = {
	"CHpics",
	"",
	"DUpics",
	"INpics",
	"MEpics",
	"MYpics",
	"SEpics",
	"STpics",
	""
};

void MystGraphics::loadExternalPictureFile(uint16 stack) {
	if (_vm->getPlatform() != Common::kPlatformMacintosh)
		return;

	if (_pictureFile.picFile.isOpen())
		_pictureFile.picFile.close();
	delete[] _pictureFile.entries;

	if (!scumm_stricmp(picFileNames[stack], ""))
		return;

	if (!_pictureFile.picFile.open(picFileNames[stack]))
		error ("Could not open external picture file \'%s\'", picFileNames[stack]);

	_pictureFile.pictureCount = _pictureFile.picFile.readUint32BE();
	_pictureFile.entries = new PictureFile::PictureEntry[_pictureFile.pictureCount];

	for (uint32 i = 0; i < _pictureFile.pictureCount; i++) {
		_pictureFile.entries[i].offset = _pictureFile.picFile.readUint32BE();
		_pictureFile.entries[i].size = _pictureFile.picFile.readUint32BE();
		_pictureFile.entries[i].id = _pictureFile.picFile.readUint16BE();
		_pictureFile.entries[i].type = _pictureFile.picFile.readUint16BE();
		_pictureFile.entries[i].width = _pictureFile.picFile.readUint16BE();
		_pictureFile.entries[i].height = _pictureFile.picFile.readUint16BE();
	}
}

MohawkSurface *MystGraphics::decodeImage(uint16 id) {
	MohawkSurface *mhkSurface = 0;

	// Myst ME uses JPEG/PICT images instead of compressed Windows Bitmaps for room images,
	// though there are a few weird ones that use that format. For further nonsense with images,
	// the Macintosh version stores images in external "picture files." We check them before
	// going to check for a PICT resource.
	if (_vm->getFeatures() & GF_ME && _vm->getPlatform() == Common::kPlatformMacintosh && _pictureFile.picFile.isOpen()) {
		for (uint32 i = 0; i < _pictureFile.pictureCount; i++)
			if (_pictureFile.entries[i].id == id) {
				if (_pictureFile.entries[i].type == 0) {
					Common::SeekableReadStream *stream = new Common::SeekableSubReadStream(&_pictureFile.picFile, _pictureFile.entries[i].offset, _pictureFile.entries[i].offset + _pictureFile.entries[i].size);

					if (!_jpegDecoder->read(stream))
						error("Could not decode Myst ME Mac JPEG");

					mhkSurface = new MohawkSurface(_jpegDecoder->getSurface(_pixelFormat));
					delete stream;
				} else if (_pictureFile.entries[i].type == 1) {
					mhkSurface = new MohawkSurface(_pictDecoder->decodeImage(new Common::SeekableSubReadStream(&_pictureFile.picFile, _pictureFile.entries[i].offset, _pictureFile.entries[i].offset + _pictureFile.entries[i].size)));
				} else
					error ("Unknown Picture File type %d", _pictureFile.entries[i].type);
				break;
			}
	}

	// We're not using the external Mac files, so it's time to delve into the main Mohawk
	// archives. However, we still don't know if it's a PICT or WDIB resource. If it's Myst
	// ME it's most likely a PICT, and if it's original it's definitely a WDIB. However,
	// Myst ME throws us another curve ball in that PICT resources can contain WDIB's instead
	// of PICT's.
	if (!mhkSurface) {
		bool isPict = false;
		Common::SeekableReadStream *dataStream = NULL;

		if (_vm->getFeatures() & GF_ME && _vm->hasResource(ID_PICT, id)) {
			// The PICT resource exists. However, it could still contain a MystBitmap
			// instead of a PICT image...
			dataStream = _vm->getResource(ID_PICT, id);
		} else // No PICT, so the WDIB must exist. Let's go grab it.
			dataStream = _vm->getResource(ID_WDIB, id);

		if (_vm->getFeatures() & GF_ME) {
			// Here we detect whether it's really a PICT or a WDIB. Since a MystBitmap
			// would be compressed, there's no way to detect for the BM without a hack.
			// So, we search for the PICT version opcode for detection.
			dataStream->seek(512 + 10); // 512 byte pict header
			isPict = (dataStream->readUint32BE() == 0x001102FF);
			dataStream->seek(0);
		}

		if (isPict)
			mhkSurface = new MohawkSurface(_pictDecoder->decodeImage(dataStream));
		else {
			mhkSurface = _bmpDecoder->decodeImage(dataStream);
			mhkSurface->convertToTrueColor();
		}
	}

	assert(mhkSurface);
	return mhkSurface;
}

void MystGraphics::copyImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	// Make sure the image is bottom aligned in the dest rect
	dest.top = dest.bottom - MIN<int>(surface->h, dest.height());

	// Convert from bitmap coordinates to surface coordinates
	uint16 top = surface->h - (src.top + MIN<int>(surface->h, dest.height()));

	// Do not draw the top pixels if the image is too tall
	if (dest.height() > _viewport.height())
		top += dest.height() - _viewport.height();

	// Clip the destination rect to the screen
	if (dest.right > _vm->_system->getWidth() || dest.bottom > _vm->_system->getHeight())
		dest.debugPrint(4, "Clipping destination rect to the screen");
	dest.right = CLIP<int>(dest.right, 0, _vm->_system->getWidth());
	dest.bottom = CLIP<int>(dest.bottom, 0, _vm->_system->getHeight());

	uint16 width = MIN<int>(surface->w, dest.width());
	uint16 height = MIN<int>(surface->h, dest.height());

	// Clamp Width and Height to within src surface dimensions
	if (src.left + width > surface->w)
		width = surface->w - src.left;
	if (src.top + height > surface->h)
		height = surface->h - src.top;

	debug(3, "MystGraphics::copyImageSectionToScreen()");
	debug(3, "\tImage: %d", image);
	debug(3, "\tsrc.left: %d", src.left);
	debug(3, "\tsrc.top: %d", src.top);
	debug(3, "\tdest.left: %d", dest.left);
	debug(3, "\tdest.top: %d", dest.top);
	debug(3, "\twidth: %d", width);
	debug(3, "\theight: %d", height);

	_vm->_system->copyRectToScreen((byte *)surface->getBasePtr(src.left, top), surface->pitch, dest.left, dest.top, width, height);
}

void MystGraphics::copyImageSectionToBackBuffer(uint16 image, Common::Rect src, Common::Rect dest) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	// Make sure the image is bottom aligned in the dest rect
	dest.top = dest.bottom - MIN<int>(surface->h, dest.height());

	// Convert from bitmap coordinates to surface coordinates
	uint16 top = surface->h - (src.top + MIN<int>(surface->h, dest.height()));

	// Do not draw the top pixels if the image is too tall
	if (dest.height() > _viewport.height()) {
		top += dest.height() - _viewport.height();
	}

	// Clip the destination rect to the screen
	if (dest.right > _vm->_system->getWidth() || dest.bottom > _vm->_system->getHeight())
		dest.debugPrint(4, "Clipping destination rect to the screen");
	dest.right = CLIP<int>(dest.right, 0, _vm->_system->getWidth());
	dest.bottom = CLIP<int>(dest.bottom, 0, _vm->_system->getHeight());

	uint16 width = MIN<int>(surface->w, dest.width());
	uint16 height = MIN<int>(surface->h, dest.height());

	// Clamp Width and Height to within src surface dimensions
	if (src.left + width > surface->w)
		width = surface->w - src.left;
	if (src.top + height > surface->h)
		height = surface->h - src.top;

	debug(3, "MystGraphics::copyImageSectionToBackBuffer()");
	debug(3, "\tImage: %d", image);
	debug(3, "\tsrc.left: %d", src.left);
	debug(3, "\tsrc.top: %d", src.top);
	debug(3, "\tdest.left: %d", dest.left);
	debug(3, "\tdest.top: %d", dest.top);
	debug(3, "\twidth: %d", width);
	debug(3, "\theight: %d", height);

	for (uint16 i = 0; i < height; i++)
		memcpy(_backBuffer->getBasePtr(dest.left, i + dest.top), surface->getBasePtr(src.left, top + i), width * surface->bytesPerPixel);
}

void MystGraphics::copyImageToScreen(uint16 image, Common::Rect dest) {
	copyImageSectionToScreen(image, Common::Rect(544, 333), dest);
}

void MystGraphics::copyImageToBackBuffer(uint16 image, Common::Rect dest) {
	copyImageSectionToBackBuffer(image, Common::Rect(544, 333), dest);
}

void MystGraphics::copyBackBufferToScreen(Common::Rect r) {
	r.clip(_viewport);
	_vm->_system->copyRectToScreen((byte *)_backBuffer->getBasePtr(r.left, r.top), _backBuffer->pitch, r.left, r.top, r.width(), r.height());
}

void MystGraphics::runTransition(uint16 type, Common::Rect rect, uint16 steps, uint16 delay) {
	switch (type) {
	case 0:	{
			debugC(kDebugScript, "Left to Right");

			uint16 step = (rect.right - rect.left) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.left = rect.left + step * i;
				area.right = area.left + step;

				_vm->_system->delayMillis(delay);

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
			if (area.right < rect.right) {
				area.left = area.right;
				area.right = rect.right;

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
		}
		break;
	case 1:	{
			debugC(kDebugScript, "Right to Left");

			uint16 step = (rect.right - rect.left) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.right = rect.right - step * i;
				area.left = area.right - step;

				_vm->_system->delayMillis(delay);

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
			if (area.left > rect.left) {
				area.right = area.left;
				area.left = rect.left;

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
		}
		break;
	case 5:	{
			debugC(kDebugScript, "Top to Bottom");

			uint16 step = (rect.bottom - rect.top) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.top = rect.top + step * i;
				area.bottom = area.top + step;

				_vm->_system->delayMillis(delay);

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
			if (area.bottom < rect.bottom) {
				area.top = area.bottom;
				area.bottom = rect.bottom;

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
		}
		break;
	case 6:	{
			debugC(kDebugScript, "Bottom to Top");

			uint16 step = (rect.bottom - rect.top) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.bottom = rect.bottom - step * i;
				area.top = area.bottom - step;

				_vm->_system->delayMillis(delay);

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
			if (area.top > rect.top) {
				area.bottom = area.top;
				area.top = rect.top;

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
		}
		break;
	default:
		warning("Unknown Update Direction");

		//TODO: Replace minimal implementation
		copyBackBufferToScreen(rect);
		_vm->_system->updateScreen();
		break;
	}
}

void MystGraphics::drawRect(Common::Rect rect, RectState state) {
	rect.clip(_viewport);

	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	if (!rect.isValidRect() || rect.width() == 0 || rect.height() == 0)
		return;

	Graphics::Surface *screen = _vm->_system->lockScreen();

	if (state == kRectEnabled)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else if (state == kRectUnreachable)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 0, 255));
	else
		screen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));

	_vm->_system->unlockScreen();
}

void MystGraphics::drawLine(const Common::Point &p1, const Common::Point &p2, uint32 color) {
	_backBuffer->drawLine(p1.x, p1.y, p2.x, p2.y, color);
}

#endif // ENABLE_MYST

#ifdef ENABLE_RIVEN

RivenGraphics::RivenGraphics(MohawkEngine_Riven* vm) : GraphicsManager(), _vm(vm) {
	_bitmapDecoder = new MohawkBitmap();

	// Give me the best you've got!
	initGraphics(608, 436, true, NULL);
	_pixelFormat = _vm->_system->getScreenFormat();

	if (_pixelFormat.bytesPerPixel == 1)
		error("Riven requires greater than 256 colors to run");

	// The actual game graphics only take up the first 392 rows. The inventory
	// occupies the rest of the screen and we don't use the buffer to hold that.
	_mainScreen = new Graphics::Surface();
	_mainScreen->create(608, 392, _pixelFormat.bytesPerPixel);

	_updatesEnabled = true;
	_scheduledTransition = -1;	// no transition
	_dirtyScreen = false;
	_inventoryDrawn = false;

	_creditsImage = 302;
	_creditsPos = 0;
}

RivenGraphics::~RivenGraphics() {
	_mainScreen->free();
	delete _mainScreen;
	delete _bitmapDecoder;
}

MohawkSurface *RivenGraphics::decodeImage(uint16 id) {
	MohawkSurface *surface = _bitmapDecoder->decodeImage(_vm->getResource(ID_TBMP, id));
	surface->convertToTrueColor();
	return surface;
}

void RivenGraphics::copyImageToScreen(uint16 image, uint32 left, uint32 top, uint32 right, uint32 bottom) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	// Clip the width to fit on the screen. Fixes some images.
	if (left + surface->w > 608)
		surface->w = 608 - left;

	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(left, i + top), surface->getBasePtr(0, i), surface->w * surface->bytesPerPixel);

	_dirtyScreen = true;
}

void RivenGraphics::drawPLST(uint16 x) {
	Common::SeekableReadStream* plst = _vm->getResource(ID_PLST, _vm->getCurCard());
	uint16 recordCount = plst->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		uint16 index = plst->readUint16BE();
		uint16 id = plst->readUint16BE();
		uint16 left = plst->readUint16BE();
		uint16 top = plst->readUint16BE();
		uint16 right = plst->readUint16BE();
		uint16 bottom = plst->readUint16BE();

		// We are also checking here to make sure we haven't drawn the image yet on screen.
		// This fixes problems with drawing PLST 1 twice and some other images twice. PLST
		// 1 is sometimes not called by the scripts, so some cards don't appear if we don't
		// draw PLST 1 each time. This "hack" is here to catch any PLST attempting to draw
		// twice. There should never be a problem with doing it this way.
		if (index == x && !(Common::find(_activatedPLSTs.begin(), _activatedPLSTs.end(), x) != _activatedPLSTs.end())) {
			debug(0, "Drawing image %d", id);
			copyImageToScreen(id, left, top, right, bottom);
			_activatedPLSTs.push_back(x);
			break;
		}
	}

	delete plst;
}

void RivenGraphics::updateScreen(Common::Rect updateRect) {
	if (_updatesEnabled) {
		_vm->runUpdateScreenScript();

		if (_dirtyScreen) {
			_activatedPLSTs.clear();

			// Copy to screen if there's no transition. Otherwise transition. ;)
			if (_scheduledTransition < 0)
				_vm->_system->copyRectToScreen((byte *)_mainScreen->getBasePtr(updateRect.left, updateRect.top), _mainScreen->pitch, updateRect.left, updateRect.top, updateRect.width(), updateRect.height());
			else
				runScheduledTransition();

			// Finally, update the screen.
			_vm->_system->updateScreen();
			_dirtyScreen = false;
		}
	}
}

void RivenGraphics::scheduleWaterEffect(uint16 sfxeID) {
	Common::SeekableReadStream *sfxeStream = _vm->getResource(ID_SFXE, sfxeID);

	if (sfxeStream->readUint16BE() != 'SL')
		error ("Unknown sfxe tag");

	// Read in header info
	SFXERecord sfxeRecord;
	sfxeRecord.frameCount = sfxeStream->readUint16BE();
	uint32 offsetTablePosition = sfxeStream->readUint32BE();
	sfxeRecord.rect.left = sfxeStream->readUint16BE();
	sfxeRecord.rect.top = sfxeStream->readUint16BE();
	sfxeRecord.rect.right = sfxeStream->readUint16BE();
	sfxeRecord.rect.bottom = sfxeStream->readUint16BE();
	sfxeRecord.speed = sfxeStream->readUint16BE();
	// Skip the rest of the fields...

	// Read in offsets
	sfxeStream->seek(offsetTablePosition);
	uint32 *frameOffsets = new uint32[sfxeRecord.frameCount];
	for (uint16 i = 0; i < sfxeRecord.frameCount; i++)
		frameOffsets[i] = sfxeStream->readUint32BE();
	sfxeStream->seek(frameOffsets[0]);

	// Read in the scripts
	for (uint16 i = 0; i < sfxeRecord.frameCount; i++)
		sfxeRecord.frameScripts.push_back(sfxeStream->readStream((i == sfxeRecord.frameCount - 1) ? sfxeStream->size() - frameOffsets[i] : frameOffsets[i + 1] - frameOffsets[i]));

	// Set it to the first frame
	sfxeRecord.curFrame = 0;
	sfxeRecord.lastFrameTime = 0;

	delete[] frameOffsets;
	delete sfxeStream;
	_waterEffects.push_back(sfxeRecord);
}

void RivenGraphics::clearWaterEffects() {
	_waterEffects.clear();
}

bool RivenGraphics::runScheduledWaterEffects() {
	// Don't run the effect if it's disabled
	if (_vm->_vars["waterenabled"] == 0)
		return false;

	Graphics::Surface *screen = NULL;

	for (uint16 i = 0; i < _waterEffects.size(); i++) {
		if (_vm->_system->getMillis() > _waterEffects[i].lastFrameTime + 1000 / _waterEffects[i].speed) {
			// Lock the screen!
			if (!screen)
				screen = _vm->_system->lockScreen();

			// Make sure the script is at the starting point
			Common::SeekableReadStream *script = _waterEffects[i].frameScripts[_waterEffects[i].curFrame];
			if (script->pos() != 0)
				script->seek(0);

			// Run script
			uint16 curRow = 0;
			for (uint16 op = script->readUint16BE(); op != 4; op = script->readUint16BE()) {
				if (op == 1) {        // Increment Row
					curRow++;
				} else if (op == 3) { // Copy Pixels
					uint16 dstLeft = script->readUint16BE();
					uint16 srcLeft = script->readUint16BE();
					uint16 srcTop = script->readUint16BE();
					uint16 rowWidth = script->readUint16BE();
					memcpy ((byte *)screen->getBasePtr(dstLeft, curRow + _waterEffects[i].rect.top), (byte *)_mainScreen->getBasePtr(srcLeft, srcTop), rowWidth * _pixelFormat.bytesPerPixel);
				} else if (op != 4) { // End of Script
					error ("Unknown SFXE opcode %d", op);
				}
			}

			// Increment frame
			_waterEffects[i].curFrame++;
			if (_waterEffects[i].curFrame == _waterEffects[i].frameCount)
				_waterEffects[i].curFrame = 0;

			// Set the new time
			_waterEffects[i].lastFrameTime = _vm->_system->getMillis();
		}
	}

	// Unlock the screen if it has been locked and return true to update the screen
	if (screen) {
		_vm->_system->unlockScreen();
		return true;
	}

	return false;
}

void RivenGraphics::scheduleTransition(uint16 id, Common::Rect rect) {
	_scheduledTransition = id;
	_transitionRect = rect;
}

void RivenGraphics::runScheduledTransition() {
	if (_scheduledTransition < 0) // No transition is scheduled
		return;

	// TODO: There's a lot to be done here...

	// Note: Transitions 0-11 are actual transitions, but none are used in-game.
	// There's no point in implementing them if they're not used. These extra
	// transitions were found by hacking scripts.

	switch (_scheduledTransition) {
	case 0:  // Swipe Left
	case 1:  // Swipe Right
	case 2:  // Swipe Up
	case 3:  // Swipe Down
	case 12: // Pan Left
	case 13: // Pan Right
	case 14: // Pan Up
	case 15: // Pan Down
	case 16: // Dissolve
	case 17: // Dissolve (tspit CARD 155)
		break;
	default:
		if (_scheduledTransition >= 4 && _scheduledTransition <= 11)
			error("Found unused transition %d", _scheduledTransition);
		else
			error("Found unknown transition %d", _scheduledTransition);
	}

	// For now, just copy the image to screen without doing any transition.
	_vm->_system->copyRectToScreen((byte *)_mainScreen->pixels, _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
	_vm->_system->updateScreen();

	_scheduledTransition = -1; // Clear scheduled transition
}

void RivenGraphics::clearMainScreen() {
	_mainScreen->fillRect(Common::Rect(0, 0, 608, 392), _pixelFormat.RGBToColor(0, 0, 0));
}

void RivenGraphics::fadeToBlack() {
	// Self-explanatory
	scheduleTransition(16);
	clearMainScreen();
	runScheduledTransition();
}

void RivenGraphics::showInventory() {
	// Don't redraw the inventory
	if (_inventoryDrawn)
		return;

	// Clear the inventory area
	clearInventoryArea();

	// Draw the demo's exit button
	if (_vm->getFeatures() & GF_DEMO) {
		// extras.mhk tBMP 101 contains "EXIT" instead of Atrus' journal in the demo!
		// The demo's extras.mhk contains all the other inventory/marble/credits image
		// but has hacked tBMP 101 with "EXIT". *sigh*
		drawInventoryImage(101, g_demoExitRect);
	} else {
		// We don't want to show the inventory on setup screens or in other journals.
		if (_vm->getCurStack() == aspit)
			return;

		// There are three books and three vars. We have three different
		// combinations. At the start you have just Atrus' journal. Later,
		// you get Catherine's journal and the trap book. Near the end,
		// you lose the trap book and have just the two journals.

		bool hasCathBook = _vm->_vars["acathbook"] != 0;
		bool hasTrapBook = _vm->_vars["atrapbook"] != 0;

		if (!hasCathBook) {
			drawInventoryImage(101, g_atrusJournalRect1);
		} else if (!hasTrapBook) {
			drawInventoryImage(101, g_atrusJournalRect2);
			drawInventoryImage(102, g_cathJournalRect2);
		} else {
			drawInventoryImage(101, g_atrusJournalRect3);
			drawInventoryImage(102, g_cathJournalRect3);
			drawInventoryImage(100, g_trapBookRect3);
		}
	}

	_vm->_system->updateScreen();
	_inventoryDrawn = true;
}

void RivenGraphics::hideInventory() {
	// Don't hide the inventory twice
	if (!_inventoryDrawn)
		return;

	// Clear the area
	clearInventoryArea();

	_inventoryDrawn = false;
}

void RivenGraphics::clearInventoryArea() {
	// Clear the inventory area
	static const Common::Rect inventoryRect = Common::Rect(0, 392, 608, 436);

	// Lock the screen
	Graphics::Surface *screen = _vm->_system->lockScreen();

	// Fill the inventory area with black
	screen->fillRect(inventoryRect, _pixelFormat.RGBToColor(0, 0, 0));

	_vm->_system->unlockScreen();
}

void RivenGraphics::drawInventoryImage(uint16 id, const Common::Rect *rect) {
	MohawkSurface *mhkSurface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	mhkSurface->convertToTrueColor();
	Graphics::Surface *surface = mhkSurface->getSurface();

	_vm->_system->copyRectToScreen((byte *)surface->pixels, surface->pitch, rect->left, rect->top, surface->w, surface->h);

	delete mhkSurface;
}

void RivenGraphics::drawRect(Common::Rect rect, bool active) {
	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	Graphics::Surface *screen = _vm->_system->lockScreen();

	if (active)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else
		screen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));

	_vm->_system->unlockScreen();
}

void RivenGraphics::drawImageRect(uint16 id, Common::Rect srcRect, Common::Rect dstRect) {
	// Draw tBMP id from srcRect to dstRect
	Graphics::Surface *surface = findImage(id)->getSurface();

	assert(srcRect.width() == dstRect.width() && srcRect.height() == dstRect.height());

	for (uint16 i = 0; i < srcRect.height(); i++)
		memcpy(_mainScreen->getBasePtr(dstRect.left, i + dstRect.top), surface->getBasePtr(srcRect.left, i + srcRect.top), srcRect.width() * surface->bytesPerPixel);

	_dirtyScreen = true;
}

void RivenGraphics::drawExtrasImage(uint16 id, Common::Rect dstRect) {
	MohawkSurface *mhkSurface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	mhkSurface->convertToTrueColor();
	Graphics::Surface *surface = mhkSurface->getSurface();

	assert(dstRect.width() == surface->w);

	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(dstRect.left, i + dstRect.top), surface->getBasePtr(0, i), surface->pitch);

	delete mhkSurface;
	_dirtyScreen = true;
}

void RivenGraphics::beginCredits() {
	// Clear the old cache
	clearCache();

	// Now cache all the credits images
	for (uint16 i = 302; i <= 320; i++) {
		MohawkSurface *surface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, i));
		surface->convertToTrueColor();
		addImageToCache(i, surface);
	}

	// And clear our screen too
	clearMainScreen();
}

void RivenGraphics::updateCredits() {
	if ((_creditsImage == 303 || _creditsImage == 304) && _creditsPos == 0)
		fadeToBlack();

	if (_creditsImage < 304) {
		// For the first two credit images, they are faded from black to the image and then out again
		scheduleTransition(16);

		Graphics::Surface *frame = findImage(_creditsImage++)->getSurface();

		for (int y = 0; y < frame->h; y++)
			memcpy(_mainScreen->getBasePtr(124, y), frame->getBasePtr(0, y), frame->pitch);

		runScheduledTransition();
	} else {
		// Otheriwse, we're scrolling
		// Move the screen up one row
		memmove(_mainScreen->pixels, _mainScreen->getBasePtr(0, 1), _mainScreen->pitch * (_mainScreen->h - 1));

		// Only update as long as we're not before the last frame
		// Otherwise, we're just moving up a row (which we already did)
		if (_creditsImage <= 320) {
			// Copy the next row to the bottom of the screen
			Graphics::Surface *frame = findImage(_creditsImage)->getSurface();
			memcpy(_mainScreen->getBasePtr(124, _mainScreen->h - 1), frame->getBasePtr(0, _creditsPos), frame->pitch);
			_creditsPos++;

			if (_creditsPos == _mainScreen->h) {
				_creditsImage++;
				_creditsPos = 0;
			}
		}

		// Now flush the new screen
		_vm->_system->copyRectToScreen((byte *)_mainScreen->pixels, _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
		_vm->_system->updateScreen();
	}
}

#endif // ENABLE_RIVEN

LBGraphics::LBGraphics(MohawkEngine_LivingBooks *vm, uint16 width, uint16 height) : GraphicsManager(), _vm(vm) {
	_bmpDecoder = _vm->isPreMohawk() ? new LivingBooksBitmap_v1() : new MohawkBitmap();

	initGraphics(width, height, true);
}

LBGraphics::~LBGraphics() {
	delete _bmpDecoder;
}

MohawkSurface *LBGraphics::decodeImage(uint16 id) {
	if (_vm->isPreMohawk())
		return _bmpDecoder->decodeImage(_vm->wrapStreamEndian(ID_BMAP, id));

	return _bmpDecoder->decodeImage(_vm->getResource(ID_TBMP, id));
}

void LBGraphics::copyOffsetAnimImageToScreen(uint16 image, int left, int top) {
	MohawkSurface *mhkSurface = findImage(image);

	left -= mhkSurface->getOffsetX();
	top -= mhkSurface->getOffsetY();

	GraphicsManager::copyAnimImageToScreen(image, left, top);
}

bool LBGraphics::imageIsTransparentAt(uint16 image, bool useOffsets, int x, int y) {
	MohawkSurface *mhkSurface = findImage(image);

	if (useOffsets) {
		x += mhkSurface->getOffsetX();
		y += mhkSurface->getOffsetY();
	}

	if (x < 0 || y < 0)
		return true;

	Graphics::Surface *surface = mhkSurface->getSurface();
	if (x >= surface->w || y >= surface->h)
		return true;

	return *(byte *)surface->getBasePtr(x, y) == 0;
}

void LBGraphics::setPalette(uint16 id) {
	// Old Living Books games use the old CTBL-style palette format while newer
	// games use the better tPAL format which can store partial palettes.
	if (_vm->isPreMohawk()) {
		Common::SeekableSubReadStreamEndian *ctblStream = _vm->wrapStreamEndian(ID_CTBL, id);
		uint16 colorCount = ctblStream->readUint16();
		byte *palette = new byte[colorCount * 3];

		for (uint16 i = 0; i < colorCount; i++) {
			palette[i * 3 + 0] = ctblStream->readByte();
			palette[i * 3 + 1] = ctblStream->readByte();
			palette[i * 3 + 2] = ctblStream->readByte();
			ctblStream->readByte();
		}

		delete ctblStream;

		_vm->_system->getPaletteManager()->setPalette(palette, 0, colorCount);
		delete[] palette;
	} else {
		GraphicsManager::setPalette(id);
	}
}

#ifdef ENABLE_CSTIME

CSTimeGraphics::CSTimeGraphics(MohawkEngine_CSTime *vm) : GraphicsManager(), _vm(vm) {
	_bmpDecoder = new MohawkBitmap();

	initGraphics(640, 480, true);
}

CSTimeGraphics::~CSTimeGraphics() {
	delete _bmpDecoder;
}

void CSTimeGraphics::drawRect(Common::Rect rect, byte color) {
	rect.clip(Common::Rect(640, 480));

	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	if (!rect.isValidRect() || rect.width() == 0 || rect.height() == 0)
		return;

	Graphics::Surface *screen = _vm->_system->lockScreen();

	screen->frameRect(rect, color);

	_vm->_system->unlockScreen();
}

MohawkSurface *CSTimeGraphics::decodeImage(uint16 id) {
	return _bmpDecoder->decodeImage(_vm->getResource(ID_TBMP, id));
}

Common::Array<MohawkSurface *> CSTimeGraphics::decodeImages(uint16 id) {
	return _bmpDecoder->decodeImages(_vm->getResource(ID_TBMH, id));
}

#endif

} // End of namespace Mohawk
