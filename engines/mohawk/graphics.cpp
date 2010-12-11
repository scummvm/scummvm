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
#include "mohawk/myst.h"
#include "mohawk/riven.h"
#include "mohawk/livingbooks.h"

#include "common/substream.h"

#include "engines/util.h"

#include "graphics/primitives.h"
#include "gui/message.h"

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
			byte r = _palette[palIndex * 4];
			byte g = _palette[palIndex * 4 + 1];
			byte b = _palette[palIndex * 4 + 2];
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

	_cache.clear();
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

void GraphicsManager::preloadImage(uint16 image) {
	findImage(image);
}

void GraphicsManager::setPalette(uint16 id) {
	Common::SeekableReadStream *tpalStream = getVM()->getResource(ID_TPAL, id);

	uint16 colorStart = tpalStream->readUint16BE();
	uint16 colorCount = tpalStream->readUint16BE();
	byte *palette = new byte[colorCount * 4];

	for (uint16 i = 0; i < colorCount; i++) {
		palette[i * 4] = tpalStream->readByte();
		palette[i * 4 + 1] = tpalStream->readByte();
		palette[i * 4 + 2] = tpalStream->readByte();
		palette[i * 4 + 3] = tpalStream->readByte();
	}

	delete tpalStream;

	getVM()->_system->setPalette(palette, colorStart, colorCount);
	delete[] palette;
}

void GraphicsManager::copyAnimImageToScreen(uint16 image, int left, int top) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	Common::Rect srcRect(0, 0, surface->w, surface->h);
	Common::Rect dstRect(left, top, left + surface->w, top + surface->h);
	copyAnimImageSectionToScreen(image, srcRect, dstRect);
}

void GraphicsManager::copyAnimImageSectionToScreen(uint16 image, Common::Rect srcRect, Common::Rect dstRect) {
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

	Graphics::Surface *surface = findImage(image)->getSurface();
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

MystGraphics::MystGraphics(MohawkEngine_Myst* vm) : GraphicsManager(), _vm(vm) {
	_bmpDecoder = new MystBitmap();

	// The original version of Myst could run in 8bpp color too.
	// However, it dithered videos to 8bpp and they looked considerably
	// worse (than they already did :P). So we're not even going to
	// support 8bpp mode in Myst (Myst ME required >8bpp anyway).
	initGraphics(544, 333, true, NULL); // What an odd screen size!

	_pixelFormat = _vm->_system->getScreenFormat();

	if (_pixelFormat.bytesPerPixel == 1)
		error("Myst requires greater than 256 colors to run");

	if (_vm->getFeatures() & GF_ME) {
		_jpegDecoder = new Graphics::JPEGDecoder();
		_pictDecoder = new Graphics::PictDecoder(_pixelFormat);
	} else {
		_jpegDecoder = NULL;
		_pictDecoder = NULL;
	}

	_pictureFile.entries = NULL;

	// Initialize our buffer
	_mainScreen = new Graphics::Surface();
	_mainScreen->create(_vm->_system->getWidth(), _vm->_system->getHeight(), _pixelFormat.bytesPerPixel);
}

MystGraphics::~MystGraphics() {
	delete _bmpDecoder;
	delete _jpegDecoder;
	delete _pictDecoder;
	delete[] _pictureFile.entries;

	_mainScreen->free();
	delete _mainScreen;
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
					Graphics::Surface *surface = new Graphics::Surface();
					surface->copyFrom(*_jpegDecoder->decodeImage(new Common::SeekableSubReadStream(&_pictureFile.picFile, _pictureFile.entries[i].offset, _pictureFile.entries[i].offset + _pictureFile.entries[i].size)));
					mhkSurface = new MohawkSurface(surface);
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
	// Clip the destination rect to the screen
	if (dest.right > _vm->_system->getWidth() || dest.bottom > _vm->_system->getHeight())
		dest.debugPrint(4, "Clipping destination rect to the screen:");

	dest.right = CLIP<int>(dest.right, 0, _vm->_system->getWidth());
	dest.bottom = CLIP<int>(dest.bottom, 0, _vm->_system->getHeight());

	Graphics::Surface *surface = findImage(image)->getSurface();

	debug(3, "Image Blit:");
	debug(3, "src.x: %d", src.left);
	debug(3, "src.y: %d", src.top);
	debug(3, "dest.x: %d", dest.left);
	debug(3, "dest.y: %d", dest.top);
	debug(3, "width: %d", src.width());
	debug(3, "height: %d", src.height());

	uint16 width = MIN<int>(surface->w, dest.width());
	uint16 height = MIN<int>(surface->h, dest.height());

	// Convert from bitmap coordinates to surface coordinates
	uint16 top = surface->h - src.top - height;

	for (uint16 i = 0; i < height; i++)
		memcpy(_mainScreen->getBasePtr(dest.left, i + dest.top), surface->getBasePtr(src.left, top + i), width * surface->bytesPerPixel);

	// Add to the list of dirty rects
	_dirtyRects.push_back(dest);
}

void MystGraphics::copyImageToScreen(uint16 image, Common::Rect dest) {
	copyImageSectionToScreen(image, Common::Rect(0, 0, 544, 333), dest);
}

void MystGraphics::updateScreen() {
	// Only update the screen if there have been changes since last frame
	if (!_dirtyRects.empty()) {

		// Copy any modified area
		for (uint i = 0; i < _dirtyRects.size(); i++) {
			Common::Rect &r = _dirtyRects[i];
			_vm->_system->copyRectToScreen((byte *)_mainScreen->getBasePtr(r.left, r.top), _mainScreen->pitch, r.left, r.top, r.width(), r.height());
		}

		_vm->_system->updateScreen();
		_dirtyRects.clear();
	}
}

void MystGraphics::runTransition(uint16 type, Common::Rect rect, uint16 steps, uint16 delay) {
	// Bypass dirty rects for animated updates
	_dirtyRects.clear();

	switch (type) {
	case 0:	{
			debugC(kDebugScript, "Left to Right");

			uint16 step = (rect.right - rect.left) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.left = rect.left + step * i;
				area.right = area.left + step;

				_vm->_system->delayMillis(delay);

				_dirtyRects.push_back(area);
				updateScreen();
			}
			if (area.right < rect.right) {
				area.left = area.right;
				area.right = rect.right;

				_dirtyRects.push_back(area);
				updateScreen();
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

				_dirtyRects.push_back(area);
				updateScreen();
			}
			if (area.left > rect.left) {
				area.right = area.left;
				area.left = rect.left;

				_dirtyRects.push_back(area);
				updateScreen();
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

				_dirtyRects.push_back(area);
				updateScreen();
			}
			if (area.bottom < rect.bottom) {
				area.top = area.bottom;
				area.bottom = rect.bottom;

				_dirtyRects.push_back(area);
				updateScreen();
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

				_dirtyRects.push_back(area);
				updateScreen();
			}
			if (area.top > rect.top) {
				area.bottom = area.top;
				area.top = rect.top;

				_dirtyRects.push_back(area);
				updateScreen();
			}
		}
		break;
	default:
		warning("Unknown Update Direction");

		//TODO: Replace minimal implementation
		_dirtyRects.push_back(rect);
		updateScreen();
		break;
	}
}

void MystGraphics::drawRect(Common::Rect rect, RectState state) {
	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	if (rect.left < 0 || rect.top < 0 || rect.right > 544 || rect.bottom > 333 || !rect.isValidRect() || rect.width() == 0 || rect.height() == 0)
		return;

	if (state == kRectEnabled)
		_mainScreen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else if (state == kRectUnreachable)
		_mainScreen->frameRect(rect, _pixelFormat.RGBToColor(0, 0, 255));
	else
		_mainScreen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));
}

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
	uint16 index, id, left, top, right, bottom;
	uint16 recordCount = plst->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		index = plst->readUint16BE();
		id = plst->readUint16BE();
		left = plst->readUint16BE();
		top = plst->readUint16BE();
		right = plst->readUint16BE();
		bottom = plst->readUint16BE();

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

void RivenGraphics::updateScreen() {
	if (_updatesEnabled) {
		_vm->runUpdateScreenScript();

		if (_dirtyScreen) {
			_activatedPLSTs.clear();

			// Copy to screen if there's no transition. Otherwise transition. ;)
			if (_scheduledTransition < 0)
				_vm->_system->copyRectToScreen((byte *)_mainScreen->pixels, _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
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
	if (*_vm->getVar("waterenabled") == 0)
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

		bool hasCathBook = *_vm->getVar("acathbook") != 0;
		bool hasTrapBook = *_vm->getVar("atrapbook") != 0;

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

LBGraphics::LBGraphics(MohawkEngine_LivingBooks *vm, uint16 width, uint16 height) : GraphicsManager(), _vm(vm) {
	_bmpDecoder = (_vm->getGameType() == GType_LIVINGBOOKSV1) ? new OldMohawkBitmap() : new MohawkBitmap();

	initGraphics(width, height, true);
}

LBGraphics::~LBGraphics() {
	delete _bmpDecoder;
}

MohawkSurface *LBGraphics::decodeImage(uint16 id) {
	if (_vm->getGameType() == GType_LIVINGBOOKSV1)
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
	if (_vm->getGameType() == GType_LIVINGBOOKSV1) {
		Common::SeekableSubReadStreamEndian *ctblStream = _vm->wrapStreamEndian(ID_CTBL, id);
		uint16 colorCount = ctblStream->readUint16();
		byte *palette = new byte[colorCount * 4];

		for (uint16 i = 0; i < colorCount; i++) {
			palette[i * 4] = ctblStream->readByte();
			palette[i * 4 + 1] = ctblStream->readByte();
			palette[i * 4 + 2] = ctblStream->readByte();
			palette[i * 4 + 3] = ctblStream->readByte();
		}

		delete ctblStream;

		_vm->_system->setPalette(palette, 0, colorCount);
		delete[] palette;
	} else {
		GraphicsManager::setPalette(id);
	}
}

} // End of namespace Mohawk
