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

#include "mohawk/myst.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/resource.h"

#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"
#include "graphics/paletteman.h"
#include "graphics/scaler.h"
#include "image/pict.h"

namespace Mohawk {

MystGraphics::MystGraphics(MohawkEngine_Myst* vm) :
		GraphicsManager(),
		_vm(vm),
		_menuFont(nullptr) {
	_bmpDecoder = new MystBitmap();

	_viewport = Common::Rect(544, 332);

	if (_vm->isGameVariant(GF_ME)) {
		// High color
		initGraphics(_viewport.width(), _viewport.height(), nullptr);

		if (_vm->_system->getScreenFormat().bytesPerPixel == 1)
			error("Myst ME requires greater than 256 colors to run");
	} else {
		// Paletted
		initGraphics(_viewport.width(), _viewport.height());
		clearScreenPalette();
	}

	_pixelFormat = _vm->_system->getScreenFormat();

	// Initialize our buffer
	_backBuffer = new Graphics::Surface();
	_backBuffer->create(_vm->_system->getWidth(), _vm->_system->getHeight(), _pixelFormat);

	_mainMenuBackupScreen.reset(new Graphics::Surface());
	_mainMenuBackupScreenThumbnail.reset(new Graphics::Surface());
	_mainMenuBackupBackBuffer.reset(new Graphics::Surface());

	if (_vm->isGameVariant(GF_ME) && _vm->isGameVariant(GF_25TH)) {
		loadMenuFont();
	}
}

MystGraphics::~MystGraphics() {
	delete _bmpDecoder;

	_backBuffer->free();
	delete _backBuffer;
	delete _menuFont;
}

void MystGraphics::loadMenuFont() {
	delete _menuFont;
	_menuFont = nullptr;

	const char *menuFontName = "NotoSans-ExtraBold.ttf";
#ifdef USE_FREETYPE2
	int fontSize;
	if (_vm->getLanguage() == Common::PL_POL) {
		fontSize = 11; // The Polish diacritics need significantly more space, so we use a smaller font
	} else {
		fontSize = 16;
	}

	Common::SeekableReadStream *fontStream = SearchMan.createReadStreamForMember(menuFontName);
	if (fontStream) {
		_menuFont = Graphics::loadTTFFont(*fontStream, fontSize);
		delete fontStream;
	} else
#endif
	{
		warning("Unable to open the menu font file '%s'", menuFontName);
	}
}

MohawkSurface *MystGraphics::decodeImage(uint16 id) {
	// We need to grab the image from the current stack archive, however, we  don't know
	// if it's a PICT or WDIB resource. If it's Myst ME it's most likely a PICT, and if it's
	// original it's definitely a WDIB. However, Myst ME throws us another curve ball in
	// that PICT resources can contain WDIB's instead of PICT's.
	Common::SeekableReadStream *dataStream = nullptr;

	if (_vm->isGameVariant(GF_ME) && _vm->hasResource(ID_PICT, id)) {
		// The PICT resource exists. However, it could still contain a MystBitmap
		// instead of a PICT image...
		dataStream = _vm->getResource(ID_PICT, id);
	} else {
		// No PICT, so the WDIB must exist. Let's go grab it.
		dataStream = _vm->getResource(ID_WDIB, id);
	}

	bool isPict = false;

	if (_vm->isGameVariant(GF_ME) && dataStream->size() > 512 + 10 + 4) {
		// Here we detect whether it's really a PICT or a WDIB. Since a MystBitmap
		// would be compressed, there's no way to detect for the BM without a hack.
		// So, we search for the PICT version opcode for detection.
		dataStream->seek(512 + 10); // 512 byte pict header
		isPict = (dataStream->readUint32BE() == 0x001102FF);
		dataStream->seek(0);
	}

	MohawkSurface *mhkSurface = nullptr;

	if (isPict) {
		Image::PICTDecoder pict;

		if (!pict.loadStream(*dataStream))
			error("Could not decode Myst ME PICT %d", id);

		delete dataStream;

		mhkSurface = new MohawkSurface(pict.getSurface()->convertTo(_pixelFormat));
	} else {
		mhkSurface = _bmpDecoder->decodeImage(dataStream);

		if (_vm->isGameVariant(GF_ME)) {
			mhkSurface->convertToTrueColor();
		} else {
			remapSurfaceToSystemPalette(mhkSurface);
		}
	}

	assert(mhkSurface);
	applyImagePatches(id, mhkSurface);
	return mhkSurface;
}

void MystGraphics::applyImagePatches(uint16 id, const MohawkSurface *mhkSurface) const {

	// In the English ME version of the game, the instructions found on Stoneship
	// to open the vault are incorrect. They are:
	//     Turn every one of [these switches to the] "off" position.
	// They should be:
	//     Turn every one of [these switches to the] "on" position.
	//
	// Here we stomp over the "off" with an "on".
	// The fixed image was provided by dafioram in bug Trac#10115.
	if (id == 2019 && _vm->isGameVariant(GF_ME) && _vm->getLanguage() == Common::EN_ANY) {
		static const byte markerSwitchInstructionsFixPic[] = {
				0x1d, 0x1c, 0x19, 0x19, 0x19, 0x19, 0x1c, 0x19, 0x19, 0x17, 0x19, 0x19, 0x19, 0x19, 0x19,
				0x1e, 0x1e, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19,
				0x1c, 0x19, 0x19, 0x19, 0x1c, 0x19, 0x19, 0x19, 0x1c, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19,
				0x1d, 0x1e, 0x16, 0x0d, 0x0e, 0x12, 0x19, 0x19, 0x17, 0x10, 0x06, 0x05, 0x19, 0x19, 0x19,
				0x1e, 0x1e, 0x10, 0x13, 0x1c, 0x11, 0x0d, 0x19, 0x12, 0x09, 0x16, 0x04, 0x18, 0x18, 0x19,
				0x1e, 0x1a, 0x03, 0x1b, 0x1c, 0x17, 0x02, 0x15, 0x13, 0x00, 0x19, 0x06, 0x18, 0x19, 0x18,
				0x1e, 0x1e, 0x01, 0x1b, 0x1c, 0x1b, 0x02, 0x15, 0x13, 0x00, 0x19, 0x07, 0x0a, 0x19, 0x18,
				0x1e, 0x1c, 0x0c, 0x0e, 0x14, 0x0c, 0x0c, 0x19, 0x0b, 0x00, 0x19, 0x00, 0x08, 0x19, 0x19,
				0x1e, 0x1c, 0x19, 0x14, 0x0f, 0x0f, 0x14, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x17,
				0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19,
				0x1c, 0x1c, 0x1e, 0x19, 0x19, 0x19, 0x17, 0x19, 0x19, 0x17, 0x19, 0x19, 0x19, 0x19, 0x19
		};

		static const byte markerSwitchInstructionsFixPal[] = {
				0x00, 0x00, 0x00,
				0x10, 0x08, 0x08,
				0x18, 0x10, 0x10,
				0x28, 0x10, 0x08,
				0x20, 0x18, 0x18,
				0x28, 0x20, 0x20,
				0x38, 0x20, 0x10,
				0x30, 0x28, 0x20,
				0x38, 0x30, 0x28,
				0x40, 0x38, 0x28,
				0x48, 0x38, 0x28,
				0x48, 0x40, 0x30,
				0x50, 0x48, 0x38,
				0x50, 0x48, 0x40,
				0x60, 0x50, 0x38,
				0x68, 0x58, 0x40,
				0x68, 0x58, 0x48,
				0x70, 0x60, 0x50,
				0x78, 0x68, 0x50,
				0x80, 0x70, 0x50,
				0x80, 0x78, 0x60,
				0x88, 0x80, 0x60,
				0x98, 0x90, 0x70,
				0xb0, 0xa0, 0x78,
				0xb8, 0xa8, 0x8d,
				0xb8, 0xa8, 0x90,
				0xb8, 0xb0, 0x88,
				0xc0, 0xb8, 0x90,
				0xd8, 0xcc, 0x98,
				0xd0, 0xe0, 0xc8,
				0xf0, 0xe4, 0xc8
		};

		Graphics::Surface fixSurf;
		fixSurf.create(15, 11, Graphics::PixelFormat::createFormatCLUT8());
		fixSurf.copyRectToSurface(markerSwitchInstructionsFixPic, fixSurf.w, 0, 0, fixSurf.w, fixSurf.h);
		fixSurf.convertToInPlace(_pixelFormat, markerSwitchInstructionsFixPal, sizeof(markerSwitchInstructionsFixPal) / 3);

		mhkSurface->getSurface()->copyRectToSurface(fixSurf, 171, 208, Common::Rect(fixSurf.w, fixSurf.h));

		fixSurf.free();
	}

	// Fix map picture with inverted colors in Myst ME Polish version
	if (id == 9934 && _vm->isGameVariant(GF_ME) && _vm->getLanguage() == Common::PL_POL) {
		mhkSurface->getSurface()->convertToInPlace(Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 0, 24));
	}
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

	_vm->_system->copyRectToScreen(surface->getBasePtr(src.left, top), surface->pitch, dest.left, dest.top, width, height);
}

void MystGraphics::copyImageSectionToBackBuffer(uint16 image, Common::Rect src, Common::Rect dest) {
	MohawkSurface *mhkSurface = findImage(image);
	Graphics::Surface *surface = mhkSurface->getSurface();

	if (image == 2258 && _vm->isGameVariant(GF_ME)) {
		// In Myst ME, the image for the open red page brother door
		// when the special lights are on does not have the correct width.
		// We work around this issue by tweaking the destination rectangle
		// so it renders at the correct position.
		// The original executable does the same hack.
		dest.left += 49;
	}

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
		memcpy(_backBuffer->getBasePtr(dest.left, i + dest.top), surface->getBasePtr(src.left, top + i), width * surface->format.bytesPerPixel);

	if (!_vm->isGameVariant(GF_ME)) {
		// Make sure the palette is set
		assert(mhkSurface->getPalette());
		memcpy(_palette, mhkSurface->getPalette(), 256 * 3);
		setPaletteToScreen();
	}
}

void MystGraphics::copyImageToScreen(uint16 image, Common::Rect dest) {
	copyImageSectionToScreen(image, Common::Rect(544, 333), dest);
}

void MystGraphics::copyImageToBackBuffer(uint16 image, Common::Rect dest) {
	copyImageSectionToBackBuffer(image, Common::Rect(544, 333), dest);
}

void MystGraphics::copyBackBufferToScreen(Common::Rect r) {
	r.clip(_viewport);

	_vm->_system->copyRectToScreen(_backBuffer->getBasePtr(r.left, r.top), _backBuffer->pitch, r.left, r.top, r.width(), r.height());
}

void MystGraphics::runTransition(TransitionType type, Common::Rect rect, uint16 steps, uint16 delay) {

	switch (type) {
	case kTransitionLeftToRight:	{
			debugC(kDebugView, "Left to Right");

			uint16 step = (rect.right - rect.left) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.left = rect.left + step * i;
				area.right = area.left + step;

				copyBackBufferToScreen(area);
				_vm->wait(delay);
			}
			if (area.right < rect.right) {
				area.left = area.right;
				area.right = rect.right;

				copyBackBufferToScreen(area);
			}
		}
		break;
	case kTransitionRightToLeft:	{
			debugC(kDebugView, "Right to Left");

			uint16 step = (rect.right - rect.left) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.right = rect.right - step * i;
				area.left = area.right - step;

				copyBackBufferToScreen(area);
				_vm->wait(delay);
			}
			if (area.left > rect.left) {
				area.right = area.left;
				area.left = rect.left;

				copyBackBufferToScreen(area);
			}
		}
		break;
	case kTransitionSlideToLeft:
		debugC(kDebugView, "Slide to left");
		transitionSlideToLeft(rect, steps, delay);
		break;
	case kTransitionSlideToRight:
		debugC(kDebugView, "Slide to right");
		transitionSlideToRight(rect, steps, delay);
		break;
	case kTransitionDissolve: {
			debugC(kDebugView, "Dissolve");

			for (int16 step = 0; step < 8; step++) {
				transitionDissolve(rect, step);
				_vm->doFrame();
			}
		}
		break;
	case kTransitionTopToBottom:	{
			debugC(kDebugView, "Top to Bottom");

			uint16 step = (rect.bottom - rect.top) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.top = rect.top + step * i;
				area.bottom = area.top + step;

				copyBackBufferToScreen(area);
				_vm->wait(delay);
			}
			if (area.bottom < rect.bottom) {
				area.top = area.bottom;
				area.bottom = rect.bottom;

				copyBackBufferToScreen(area);
			}
		}
		break;
	case kTransitionBottomToTop:	{
			debugC(kDebugView, "Bottom to Top");

			uint16 step = (rect.bottom - rect.top) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.bottom = rect.bottom - step * i;
				area.top = area.bottom - step;

				copyBackBufferToScreen(area);
				_vm->wait(delay);
			}
			if (area.top > rect.top) {
				area.bottom = area.top;
				area.top = rect.top;

				copyBackBufferToScreen(area);
			}
		}
		break;
	case kTransitionSlideToTop:
		debugC(kDebugView, "Slide to top");
		transitionSlideToTop(rect, steps, delay);
		break;
	case kTransitionSlideToBottom:
		debugC(kDebugView, "Slide to bottom");
		transitionSlideToBottom(rect, steps, delay);
		break;
	case kTransitionPartToRight: {
			debugC(kDebugView, "Partial left to right");

			transitionPartialToRight(rect, 75, 3);
		}
		break;
	case kTransitionPartToLeft: {
			debugC(kDebugView, "Partial right to left");

			transitionPartialToLeft(rect, 75, 3);
		}
		break;
	case kTransitionCopy:
		copyBackBufferToScreen(rect);
		break;
	default:
		error("Unknown transition %d", type);
	}
}

void MystGraphics::transitionDissolve(Common::Rect rect, uint step) {
	static const bool pattern[][4][4] = {
		{
			{ true,  false, false, false },
			{ false, false, false, false },
			{ false, false, true,  false },
			{ false, false, false, false }
		},
		{
			{ false, false, true,  false },
			{ false, false, false, false },
			{ true,  false, false, false },
			{ false, false, false, false }
		},
		{
			{ false, false, false, false },
			{ false, true,  false, false },
			{ false, false, false, false },
			{ false, false, false, true  }
		},
		{
			{ false, false, false, false },
			{ false, false, false, true  },
			{ false, false, false, false },
			{ false, true,  false, false }
		},
		{
			{ false, false, false, false },
			{ false, false, true,  false },
			{ false, true,  false, false },
			{ false, false, false, false }
		},
		{
			{ false, true,  false, false },
			{ false, false, false, false },
			{ false, false, false, false },
			{ false, false, true,  false }
		},
		{
			{ false, false, false, false },
			{ true,  false, false, false },
			{ false, false, false, true  },
			{ false, false, false, false }
		},
		{
			{ false, false, false, true  },
			{ false, false, false, false },
			{ false, false, false, false },
			{ true,  false, false, false }
		}
	};

	rect.clip(_viewport);

	Graphics::Surface *screen = _vm->_system->lockScreen();

	for (uint16 y = rect.top; y < rect.bottom; y++) {
		const bool *linePattern = pattern[step][y % 4];

		if (!linePattern[0] && !linePattern[1] && !linePattern[2] && !linePattern[3])
			continue;

		for (uint16 x = rect.left; x < rect.right; x++) {
			if (linePattern[x % 4]) {
				switch (_pixelFormat.bytesPerPixel) {
				case 1:
					*((byte *)screen->getBasePtr(x, y)) = *((const byte *)_backBuffer->getBasePtr(x, y));
					break;
				case 2:
					*((uint16 *)screen->getBasePtr(x, y)) = *((const uint16 *)_backBuffer->getBasePtr(x, y));
					break;
				case 4:
					*((uint32 *)screen->getBasePtr(x, y)) = *((const uint32 *)_backBuffer->getBasePtr(x, y));
					break;
				default:
					break;
				}
			}
		}
	}

	_vm->_system->unlockScreen();
}

void MystGraphics::transitionSlideToLeft(Common::Rect rect, uint16 steps, uint16 delay) {
	rect.clip(_viewport);

	uint32 stepWidth = (rect.right - rect.left) / steps;
	Common::Rect srcRect = Common::Rect(rect.right, rect.top, rect.right, rect.bottom);
	Common::Rect dstRect = Common::Rect(rect.left, rect.top, rect.left, rect.bottom);

	for (uint step = 1; step <= steps; step++) {
		dstRect.right = dstRect.left + step * stepWidth;
		srcRect.left = srcRect.right - step * stepWidth;

		_vm->_system->copyRectToScreen(_backBuffer->getBasePtr(dstRect.left, dstRect.top),
				_backBuffer->pitch, srcRect.left, srcRect.top, srcRect.width(), srcRect.height());
		_vm->wait(delay);
	}

	if (dstRect.right != rect.right) {
		copyBackBufferToScreen(rect);
	}
}

void MystGraphics::transitionSlideToRight(Common::Rect rect, uint16 steps, uint16 delay) {
	rect.clip(_viewport);

	uint32 stepWidth = (rect.right - rect.left) / steps;
	Common::Rect srcRect = Common::Rect(rect.left, rect.top, rect.left, rect.bottom);
	Common::Rect dstRect = Common::Rect(rect.right, rect.top, rect.right, rect.bottom);

	for (uint step = 1; step <= steps; step++) {
		dstRect.left = dstRect.right - step * stepWidth;
		srcRect.right = srcRect.left + step * stepWidth;

		_vm->_system->copyRectToScreen(_backBuffer->getBasePtr(dstRect.left, dstRect.top),
				_backBuffer->pitch, srcRect.left, srcRect.top, srcRect.width(), srcRect.height());
		_vm->wait(delay);
	}

	if (dstRect.left != rect.left) {
		copyBackBufferToScreen(rect);
	}
}

void MystGraphics::transitionSlideToTop(Common::Rect rect, uint16 steps, uint16 delay) {
	rect.clip(_viewport);

	uint32 stepWidth = (rect.bottom - rect.top) / steps;
	Common::Rect srcRect = Common::Rect(rect.left, rect.bottom, rect.right, rect.bottom);
	Common::Rect dstRect = Common::Rect(rect.left, rect.top, rect.right, rect.top);

	for (uint step = 1; step <= steps; step++) {
		dstRect.bottom = dstRect.top + step * stepWidth;
		srcRect.top = srcRect.bottom - step * stepWidth;

		_vm->_system->copyRectToScreen(_backBuffer->getBasePtr(dstRect.left, dstRect.top),
				_backBuffer->pitch, srcRect.left, srcRect.top, srcRect.width(), srcRect.height());
		_vm->wait(delay);
	}


	if (dstRect.bottom < rect.bottom) {
		copyBackBufferToScreen(rect);
	}
}

void MystGraphics::transitionSlideToBottom(Common::Rect rect, uint16 steps, uint16 delay) {
	rect.clip(_viewport);

	uint32 stepWidth = (rect.bottom - rect.top) / steps;
	Common::Rect srcRect = Common::Rect(rect.left, rect.top, rect.right, rect.top);
	Common::Rect dstRect = Common::Rect(rect.left, rect.bottom, rect.right, rect.bottom);

	for (uint step = 1; step <= steps; step++) {
		dstRect.top = dstRect.bottom - step * stepWidth;
		srcRect.bottom = srcRect.top + step * stepWidth;

		_vm->_system->copyRectToScreen(_backBuffer->getBasePtr(dstRect.left, dstRect.top),
				_backBuffer->pitch, srcRect.left, srcRect.top, srcRect.width(), srcRect.height());
		_vm->wait(delay);
	}


	if (dstRect.top > rect.top) {
		copyBackBufferToScreen(rect);
	}
}

void MystGraphics::transitionPartialToRight(Common::Rect rect, uint32 width, uint32 steps) {
	rect.clip(_viewport);

	uint32 stepWidth = width / steps;
	Common::Rect srcRect = Common::Rect(rect.right, rect.top, rect.right, rect.bottom);
	Common::Rect dstRect = Common::Rect(rect.left, rect.top, rect.left, rect.bottom);

	for (uint step = 1; step <= steps; step++) {
		dstRect.right = dstRect.left + step * stepWidth;
		srcRect.left = srcRect.right - step * stepWidth;

		_vm->_system->copyRectToScreen(_backBuffer->getBasePtr(dstRect.left, dstRect.top),
				_backBuffer->pitch, srcRect.left, srcRect.top, srcRect.width(), srcRect.height());
		_vm->doFrame();
	}

	copyBackBufferToScreen(rect);
}

void MystGraphics::transitionPartialToLeft(Common::Rect rect, uint32 width, uint32 steps) {
	rect.clip(_viewport);

	uint32 stepWidth = width / steps;
	Common::Rect srcRect = Common::Rect(rect.left, rect.top, rect.left, rect.bottom);
	Common::Rect dstRect = Common::Rect(rect.right, rect.top, rect.right, rect.bottom);

	for (uint step = 1; step <= steps; step++) {
		dstRect.left = dstRect.right - step * stepWidth;
		srcRect.right = srcRect.left + step * stepWidth;

		_vm->_system->copyRectToScreen(_backBuffer->getBasePtr(dstRect.left, dstRect.top),
				_backBuffer->pitch, srcRect.left, srcRect.top, srcRect.width(), srcRect.height());
		_vm->doFrame();
	}

	copyBackBufferToScreen(rect);
}

void MystGraphics::drawRect(Common::Rect rect, RectState state) {
	rect.clip(_viewport);

	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	if (!rect.isValidRect() || rect.width() == 0 || rect.height() == 0)
		return;

	Graphics::Surface *screen = _vm->_system->lockScreen();

	if (state == kRectEnabled)
		screen->frameRect(rect, _vm->isGameVariant(GF_ME) ? _pixelFormat.RGBToColor(0, 255, 0) : 250);
	else if (state == kRectUnreachable)
		screen->frameRect(rect, _vm->isGameVariant(GF_ME) ? _pixelFormat.RGBToColor(0, 0, 255) : 252);
	else
		screen->frameRect(rect, _vm->isGameVariant(GF_ME) ? _pixelFormat.RGBToColor(255, 0, 0) : 249);

	_vm->_system->unlockScreen();
}

void MystGraphics::drawLine(const Common::Point &p1, const Common::Point &p2, uint32 color) {
	_backBuffer->drawLine(p1.x, p1.y, p2.x, p2.y, color);
}

void MystGraphics::fadeToBlack() {
	// This is only for the demo
	assert(!_vm->isGameVariant(GF_ME));

	// Linear fade in 64 steps or less
	uint32 startTime = _vm->_system->getMillis();
	uint32 endTime = startTime + 640;
	uint32 time, i;

	do {
		byte palette[256 * 3];
		byte *src = _palette;
		byte *dst = palette;

		time = _vm->_system->getMillis();
		i = (endTime - time) / 10;

		if (time < endTime) {
			for (uint j = 0; j < sizeof(palette); j++)
				*dst++ = *src++ * i / 64;
		} else {
			memset(palette, 0, sizeof(palette));
		}

		_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
		_vm->doFrame();
	} while (time < endTime);
}

void MystGraphics::fadeFromBlack() {
	// This is only for the demo
	assert(!_vm->isGameVariant(GF_ME));

	copyBackBufferToScreen(_viewport);

	// Linear fade in 64 steps or less
	uint32 startTime = _vm->_system->getMillis();
	uint32 endTime = startTime + 640;
	uint32 time, i;

	do {
		byte palette[256 * 3];
		byte *src = _palette;
		byte *dst = palette;
		time = _vm->_system->getMillis();
		i = (time - startTime) / 10;

		if (time < endTime) {
			for (uint j = 0; j < sizeof(palette); j++)
				*dst++ = *src++ * i / 64;
		} else {
			memcpy(palette, _palette, sizeof(palette));
		}

		_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
		_vm->doFrame();
	} while (time < endTime);

	// Set the full palette
	_vm->_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void MystGraphics::clearScreenPalette() {
	// Set the palette to all black
	byte palette[256 * 3];
	memset(palette, 0, sizeof(palette));
	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void MystGraphics::remapSurfaceToSystemPalette(MohawkSurface *mhkSurface) {
	// Entries [0, 9] of the palette
	static const byte lowPalette[] = {
		0x00, 0x00, 0x00,
		0x80, 0x00, 0x00,
		0x00, 0x80, 0x00,
		0x80, 0x80, 0x00,
		0x00, 0x00, 0x80,
		0x80, 0x00, 0x80,
		0x00, 0x80, 0x80,
		0xC0, 0xC0, 0xC0,
		0xC0, 0xDC, 0xC0,
		0xA6, 0xCA, 0xF0
	};

	// Entries [246, 255] of the palette
	static const byte highPalette[] = {
		0xFF, 0xFB, 0xF0,
		0xA0, 0xA0, 0xA4,
		0x80, 0x80, 0x80,
		0xFF, 0x00, 0x00,
		0x00, 0xFF, 0x00,
		0xFF, 0xFF, 0x00,
		0x00, 0x00, 0xFF,
		0xFF, 0x00, 0xFF,
		0x00, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF
	};

	byte *originalPalette = mhkSurface->getPalette();

	// The target palette is made of the Windows reserved palette, and colors 10 to 245
	// of the bitmap palette. Entries 0 to 9 and 246 to 255 of the bitmap palette are
	// discarded.
	byte targetPalette[256 * 3];
	memcpy(targetPalette, lowPalette, sizeof(lowPalette));
	memcpy(targetPalette + sizeof(lowPalette), originalPalette + sizeof(lowPalette), sizeof(_palette) - sizeof(lowPalette) - sizeof(highPalette));
	memcpy(targetPalette + sizeof(_palette) - sizeof(highPalette), highPalette, sizeof(highPalette));

	// Remap the discarded entries from the bitmap palette using the target palette.
	byte lowColorMap[ARRAYSIZE(lowPalette) / 3];
	byte highColorMap[ARRAYSIZE(highPalette) / 3];

	for (uint i = 0; i < ARRAYSIZE(lowColorMap); i++) {
		uint colorIndex = 3 * i;
		byte red = originalPalette[colorIndex + 0];
		byte green = originalPalette[colorIndex + 1];
		byte blue = originalPalette[colorIndex + 2];

		lowColorMap[i] = getColorIndex(targetPalette, red, green, blue);
	}

	for (uint i = 0; i < ARRAYSIZE(highColorMap); i++) {
		uint colorIndex = 3 * (i + 246);
		byte red = originalPalette[colorIndex + 0];
		byte green = originalPalette[colorIndex + 1];
		byte blue = originalPalette[colorIndex + 2];

		highColorMap[i] = getColorIndex(targetPalette, red, green, blue);
	}

	// Replace the original palette with the target palette
	memcpy(originalPalette, targetPalette, sizeof(targetPalette));

	// Remap the pixel data to the target palette
	Graphics::Surface *surface = mhkSurface->getSurface();
	byte *pixels = (byte *) surface->getPixels();

	for (int i = 0; i < surface->w * surface->h; i++) {
		if (pixels[i] < ARRAYSIZE(lowColorMap)) {
			pixels[i] = lowColorMap[pixels[i]];
		} else if (pixels[i] >= 246) {
			pixels[i] = highColorMap[pixels[i] - 246];
		}
	}
}

byte MystGraphics::getColorIndex(const byte *palette, byte red, byte green, byte blue) {
	for (uint i = 0; i < 256; i++) {
		if (palette[(3 * i) + 0] == red && palette[(3 * i) + 1] == green && palette[(3 * i) + 2] == blue) {
			return i;
		}
	}

	// GDI actually chooses the nearest color if no exact match is found,
	// but this should not happen in Myst
	debug(1, "Color (%d, %d, %d) not in target palette", red, green, blue);
	return 0;
}

void MystGraphics::setPaletteToScreen() {
	_vm->_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void MystGraphics::saveStateForMainMenu() {
	Graphics::Surface *screen = _vm->_system->lockScreen();
	_mainMenuBackupScreen->copyFrom(*screen);
	_vm->_system->unlockScreen();

	// Create a thumbnail of the screen that will be used when saving from the main menu
	createThumbnailFromScreen(_mainMenuBackupScreenThumbnail.get());

	_mainMenuBackupBackBuffer->copyFrom(*_backBuffer);
}

void MystGraphics::restoreStateForMainMenu() {
	_vm->_system->copyRectToScreen(_mainMenuBackupScreen->getPixels(), _mainMenuBackupScreen->pitch,
	                               0, 0, _mainMenuBackupScreen->w, _mainMenuBackupScreen->h);

	_backBuffer->copyFrom(*_mainMenuBackupBackBuffer);

	_mainMenuBackupScreen->free();
	_mainMenuBackupScreenThumbnail->free();
	_mainMenuBackupBackBuffer->free();
}

Graphics::Surface *MystGraphics::getThumbnailForMainMenu() const {
	return _mainMenuBackupScreenThumbnail.get();
}

void MystGraphics::drawText(uint16 image, const Common::U32String &text, const Common::Rect &dest, uint8 r, uint8 g, uint8 b, Graphics::TextAlign align, int16 deltaY) {
	MohawkSurface *mhkSurface = findImage(image);
	Graphics::Surface *surface = mhkSurface->getSurface();

	const Graphics::Font *font = getMenuFont();
	font->drawString(surface, text, dest.left, dest.top + deltaY, dest.width(), surface->format.RGBToColor(r, g, b), align);
}

Common::Rect MystGraphics::getTextBoundingBox(const Common::U32String &text, const Common::Rect &dest, Graphics::TextAlign align) {
	const Graphics::Font *font = getMenuFont();
	return font->getBoundingBox(text, dest.left, dest.top, dest.width(), align);
}

const Graphics::Font *MystGraphics::getMenuFont() const {
	const Graphics::Font *font;
	if (_menuFont) {
		font = _menuFont;
	} else {
		font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	}
	return font;
}

void MystGraphics::replaceImageWithRect(uint16 destImage, uint16 sourceImage, const Common::Rect &sourceRect) {
	MohawkSurface *sourceSurface = findImage(sourceImage);
	const Graphics::Surface sourceArea = sourceSurface->getSurface()->getSubArea(sourceRect);

	Graphics::Surface *replacementSurface = new Graphics::Surface();
	replacementSurface->copyFrom(sourceArea);

	MohawkSurface *destSurface = new MohawkSurface(replacementSurface, nullptr, 0, 0);
	addImageToCache(destImage, destSurface);
}

void MystGraphics::clearScreen() {
	_vm->_system->fillScreen(_pixelFormat.RGBToColor(0, 0, 0));
}

} // End of namespace Mohawk
