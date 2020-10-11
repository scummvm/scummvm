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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/ios7/ios7_osys_main.h"
#include "backends/platform/ios7/ios7_video.h"

#include "graphics/conversion.h"
#include "backends/platform/ios7/ios7_app_delegate.h"

@interface iOS7AlertHandler : NSObject<UIAlertViewDelegate>
@end

@implementation iOS7AlertHandler

- (void)alertView:(UIAlertView *)alertView willDismissWithButtonIndex:(NSInteger)buttonIndex {
	OSystem_iOS7::sharedInstance()->quit();
	abort();
}

@end

static void displayAlert(void *ctx) {
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Fatal Error"
	                                                message:[NSString stringWithCString:(const char *)ctx encoding:NSUTF8StringEncoding]
	                                               delegate:[[iOS7AlertHandler alloc] init]
	                                      cancelButtonTitle:@"OK"
	                                      otherButtonTitles:nil];
	[alert show];
	[alert autorelease];
}

void OSystem_iOS7::fatalError() {
	if (_lastErrorMessage.size()) {
		dispatch_async_f(dispatch_get_main_queue(), (void *)_lastErrorMessage.c_str(), displayAlert);
		for(;;);
	}
	else {
		OSystem::fatalError();
	}
}

void OSystem_iOS7::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	if (type == LogMessageType::kError) {
		_lastErrorMessage = message;
		NSString *messageString = [NSString stringWithUTF8String:message];
		NSLog(@"%@", messageString);
	}

	fputs(message, output);
	fflush(output);
}

void OSystem_iOS7::engineInit() {
	EventsBaseBackend::engineInit();
	// Prevent the device going to sleep during game play (and in particular cut scenes)
	dispatch_async(dispatch_get_main_queue(), ^{
		[[UIApplication sharedApplication] setIdleTimerDisabled:YES];
	});
}

void OSystem_iOS7::engineDone() {
	EventsBaseBackend::engineDone();
	// Allow the device going to sleep if idle while in the Launcher
	dispatch_async(dispatch_get_main_queue(), ^{
		[[UIApplication sharedApplication] setIdleTimerDisabled:NO];
	});
}

void OSystem_iOS7::initVideoContext() {
	_videoContext = [[iOS7AppDelegate iPhoneView] getVideoContext];
}

const OSystem::GraphicsMode *OSystem_iOS7::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OSystem_iOS7::getDefaultGraphicsMode() const {
	return kGraphicsModeNone;
}

bool OSystem_iOS7::setGraphicsMode(int mode, uint /*flags*/) {
	switch (mode) {
	case kGraphicsModeNone:
	case kGraphicsMode2xSaI:
	case kGraphicsModeSuper2xSaI:
	case kGraphicsModeSuperEagle:
	case kGraphicsModeAdvMame2x:
	case kGraphicsModeAdvMame3x:
	case kGraphicsModeHQ2x:
	case kGraphicsModeHQ3x:
	case kGraphicsModeTV2x:
	case kGraphicsModeDotMatrix:
		_videoContext->graphicsMode = (GraphicsModes)mode;
		return true;

	default:
		return false;
	}
}

int OSystem_iOS7::getGraphicsMode() const {
	return _videoContext->graphicsMode;
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> OSystem_iOS7::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> list;
	// RGB565
	list.push_back(Graphics::createPixelFormat<565>());
	// CLUT8
	list.push_back(Graphics::PixelFormat::createFormatCLUT8());
	return list;
}
#endif

static inline void execute_on_main_thread(void (^block)(void)) {
	if ([NSThread currentThread] == [NSThread mainThread]) {
		block();
	}
	else {
		dispatch_sync(dispatch_get_main_queue(), block);
	}
}

void OSystem_iOS7::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	//printf("initSize(%u, %u, %p)\n", width, height, (const void *)format);

	_videoContext->screenWidth = width;
	_videoContext->screenHeight = height;
	_videoContext->shakeXOffset = 0;
	_videoContext->shakeYOffset = 0;

	// In case we use the screen texture as frame buffer we reset the pixels
	// pointer here to avoid freeing the screen texture.
	if (_framebuffer.getPixels() == _videoContext->screenTexture.getPixels())
		_framebuffer.setPixels(0);

	// Create the screen texture right here. We need to do this here, since
	// when a game requests hi-color mode, we actually set the framebuffer
	// to the texture buffer to avoid an additional copy step.
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] createScreenTexture];
	});

	// In case the client code tries to set up a non supported mode, we will
	// fall back to CLUT8 and set the transaction error accordingly.
	if (format && format->bytesPerPixel != 1 && *format != _videoContext->screenTexture.format) {
		format = 0;
		_gfxTransactionError = kTransactionFormatNotSupported;
	}

	if (!format || format->bytesPerPixel == 1) {
		_framebuffer.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	} else {
#if 0
		printf("bytesPerPixel: %u RGBAlosses: %u,%u,%u,%u RGBAshifts: %u,%u,%u,%u\n", format->bytesPerPixel,
		       format->rLoss, format->gLoss, format->bLoss, format->aLoss,
		       format->rShift, format->gShift, format->bShift, format->aShift);
#endif
		// We directly draw on the screen texture in hi-color mode. Thus
		// we copy over its settings here and just replace the width and
		// height to avoid any problems.
		_framebuffer = _videoContext->screenTexture;
		_framebuffer.w = width;
		_framebuffer.h = height;
	}

	_fullScreenIsDirty = false;
	dirtyFullScreen();
	_mouseCursorPaletteEnabled = false;
}

void OSystem_iOS7::beginGFXTransaction() {
	_gfxTransactionError = kTransactionSuccess;
}

OSystem::TransactionError OSystem_iOS7::endGFXTransaction() {
	_screenChangeCount++;
	updateOutputSurface();
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] setGraphicsMode];
	});

	return _gfxTransactionError;
}

void OSystem_iOS7::updateOutputSurface() {
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] initSurface];
	});
}

int16 OSystem_iOS7::getHeight() {
	return _videoContext->screenHeight;
}

int16 OSystem_iOS7::getWidth() {
	return _videoContext->screenWidth;
}

void OSystem_iOS7::setPalette(const byte *colors, uint start, uint num) {
	//printf("setPalette(%p, %u, %u)\n", colors, start, num);
	assert(start + num <= 256);
	const byte *b = colors;

	for (uint i = start; i < start + num; ++i) {
		_gamePalette[i] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(b[0], b[1], b[2]);
		_gamePaletteRGBA5551[i] = Graphics::RGBToColor<Graphics::ColorMasks<5551> >(b[0], b[1], b[2]);
		b += 3;
	}

	dirtyFullScreen();

	// Automatically update the mouse texture when the palette changes while the
	// cursor palette is disabled.
	if (!_mouseCursorPaletteEnabled && _mouseBuffer.format.bytesPerPixel == 1)
		_mouseDirty = _mouseNeedTextureUpdate = true;
}

void OSystem_iOS7::grabPalette(byte *colors, uint start, uint num) const {
	//printf("grabPalette(%p, %u, %u)\n", colors, start, num);
	assert(start + num <= 256);
	byte *b = colors;

	for (uint i = start; i < start + num; ++i) {
		Graphics::colorToRGB<Graphics::ColorMasks<565> >(_gamePalette[i], b[0], b[1], b[2]);
		b += 3;
	}
}

void OSystem_iOS7::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	//printf("copyRectToScreen(%p, %d, %i, %i, %i, %i)\n", buf, pitch, x, y, w, h);
	//Clip the coordinates
	const byte *src = (const byte *)buf;
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > (int)_framebuffer.w - x) {
		w = _framebuffer.w - x;
	}

	if (h > (int)_framebuffer.h - y) {
		h = _framebuffer.h - y;
	}

	if (w <= 0 || h <= 0)
		return;

	if (!_fullScreenIsDirty) {
		_dirtyRects.push_back(Common::Rect(x, y, x + w, y + h));
	}

	byte *dst = (byte *)_framebuffer.getBasePtr(x, y);
	if (_framebuffer.pitch == pitch && _framebuffer.w == w) {
		memcpy(dst, src, h * pitch);
	} else {
		do {
			memcpy(dst, src, w * _framebuffer.format.bytesPerPixel);
			src += pitch;
			dst += _framebuffer.pitch;
		} while (--h);
	}
}

void OSystem_iOS7::updateScreen() {
	if (_dirtyRects.size() == 0 && _dirtyOverlayRects.size() == 0 && !_mouseDirty)
		return;

	//printf("updateScreen(): %i dirty rects.\n", _dirtyRects.size());

	internUpdateScreen();
	_mouseDirty = false;
	_fullScreenIsDirty = false;
	_fullScreenOverlayIsDirty = false;

	iOS7_updateScreen();
}

void OSystem_iOS7::internUpdateScreen() {
	if (_mouseNeedTextureUpdate) {
		updateMouseTexture();
		_mouseNeedTextureUpdate = false;
	}

	while (_dirtyRects.size()) {
		Common::Rect dirtyRect = _dirtyRects.remove_at(_dirtyRects.size() - 1);

		//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);
		drawDirtyRect(dirtyRect);
		// TODO: Implement dirty rect code
		//updateHardwareSurfaceForRect(dirtyRect);
	}

	if (_videoContext->overlayVisible) {
		// TODO: Implement dirty rect code
		_dirtyOverlayRects.clear();
		/*while (_dirtyOverlayRects.size()) {
			Common::Rect dirtyRect = _dirtyOverlayRects.remove_at(_dirtyOverlayRects.size() - 1);

			//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);
			drawDirtyOverlayRect(dirtyRect);
		}*/
	}
}

void OSystem_iOS7::drawDirtyRect(const Common::Rect &dirtyRect) {
	// We only need to do a color look up for CLUT8
	if (_framebuffer.format.bytesPerPixel != 1)
		return;

	int h = dirtyRect.bottom - dirtyRect.top;
	int w = dirtyRect.right - dirtyRect.left;

	const byte *src = (const byte *)_framebuffer.getBasePtr(dirtyRect.left, dirtyRect.top);
	byte *dstRaw = (byte *)_videoContext->screenTexture.getBasePtr(dirtyRect.left, dirtyRect.top);

	// When we use CLUT8 do a color look up
	for (int y = h; y > 0; y--) {
		uint16 *dst = (uint16 *)dstRaw;
		for (int x = w; x > 0; x--)
			*dst++ = _gamePalette[*src++];

		dstRaw += _videoContext->screenTexture.pitch;
		src += _framebuffer.pitch - w;
	}
}

Graphics::Surface *OSystem_iOS7::lockScreen() {
	//printf("lockScreen()\n");
	return &_framebuffer;
}

void OSystem_iOS7::unlockScreen() {
	//printf("unlockScreen()\n");
	dirtyFullScreen();
}

void OSystem_iOS7::setShakePos(int shakeXOffset, int shakeYOffset) {
	//printf("setShakePos(%i, %i)\n", shakeXOffset, shakeYOffset);
	_videoContext->shakeXOffset = shakeXOffset;
	_videoContext->shakeYOffset = shakeYOffset;
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] setViewTransformation];
	});
	// HACK: We use this to force a redraw.
	_mouseDirty = true;
}

void OSystem_iOS7::showOverlay() {
	//printf("showOverlay()\n");
	_videoContext->overlayVisible = true;
	dirtyFullOverlayScreen();
	updateScreen();
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] updateMouseCursorScaling];
		[[iOS7AppDelegate iPhoneView] clearColorBuffer];
	});
}

void OSystem_iOS7::hideOverlay() {
	//printf("hideOverlay()\n");
	_videoContext->overlayVisible = false;
	_dirtyOverlayRects.clear();
	dirtyFullScreen();
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] updateMouseCursorScaling];
		[[iOS7AppDelegate iPhoneView] clearColorBuffer];
	});
}

void OSystem_iOS7::clearOverlay() {
	//printf("clearOverlay()\n");
	memset(_videoContext->overlayTexture.getPixels(), 0, _videoContext->overlayTexture.h * _videoContext->overlayTexture.pitch);
	dirtyFullOverlayScreen();
}

void OSystem_iOS7::grabOverlay(void *buf, int pitch) {
	//printf("grabOverlay()\n");
	int h = _videoContext->overlayHeight;

	byte *dst = (byte *)buf;
	const byte *src = (const byte *)_videoContext->overlayTexture.getPixels();
	do {
		memcpy(dst, src, _videoContext->overlayWidth * sizeof(uint16));
		src += _videoContext->overlayTexture.pitch;
		dst += pitch;
	} while (--h);
}

void OSystem_iOS7::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	//printf("copyRectToOverlay(%p, pitch=%i, x=%i, y=%i, w=%i, h=%i)\n", (const void *)buf, pitch, x, y, w, h);
	const byte *src = (const byte *)buf;

	//Clip the coordinates
	if (x < 0) {
		w += x;
		src -= x * sizeof(uint16);
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > (int)_videoContext->overlayWidth - x)
		w = _videoContext->overlayWidth - x;

	if (h > (int)_videoContext->overlayHeight - y)
		h = _videoContext->overlayHeight - y;

	if (w <= 0 || h <= 0)
		return;

	if (!_fullScreenOverlayIsDirty) {
		_dirtyOverlayRects.push_back(Common::Rect(x, y, x + w, y + h));
	}

	byte *dst = (byte *)_videoContext->overlayTexture.getBasePtr(x, y);
	do {
		memcpy(dst, src, w * sizeof(uint16));
		src += pitch;
		dst += _videoContext->overlayTexture.pitch;
	} while (--h);
}

int16 OSystem_iOS7::getOverlayHeight() {
	return _videoContext->overlayHeight;
}

int16 OSystem_iOS7::getOverlayWidth() {
	return _videoContext->overlayWidth;
}

bool OSystem_iOS7::showMouse(bool visible) {
	//printf("showMouse(%d)\n", visible);
	bool last = _videoContext->mouseIsVisible;
	_videoContext->mouseIsVisible = visible;
	_mouseDirty = true;

	return last;
}

void OSystem_iOS7::warpMouse(int x, int y) {
	//printf("warpMouse(%d, %d)\n", x, y);
	_videoContext->mouseX = x;
	_videoContext->mouseY = y;
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] notifyMouseMove];
	});
	_mouseDirty = true;
}

void OSystem_iOS7::dirtyFullScreen() {
	if (!_fullScreenIsDirty) {
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(0, 0, _videoContext->screenWidth, _videoContext->screenHeight));
		_fullScreenIsDirty = true;
	}
}

void OSystem_iOS7::dirtyFullOverlayScreen() {
	if (!_fullScreenOverlayIsDirty) {
		_dirtyOverlayRects.clear();
		_dirtyOverlayRects.push_back(Common::Rect(0, 0, _videoContext->overlayWidth, _videoContext->overlayHeight));
		_fullScreenOverlayIsDirty = true;
	}
}

void OSystem_iOS7::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	//printf("setMouseCursor(%p, %u, %u, %i, %i, %u, %d, %p)\n", (const void *)buf, w, h, hotspotX, hotspotY, keycolor, dontScale, (const void *)format);

	const Graphics::PixelFormat pixelFormat = format ? *format : Graphics::PixelFormat::createFormatCLUT8();
#if 0
	printf("bytesPerPixel: %u RGBAlosses: %u,%u,%u,%u RGBAshifts: %u,%u,%u,%u\n", pixelFormat.bytesPerPixel,
	       pixelFormat.rLoss, pixelFormat.gLoss, pixelFormat.bLoss, pixelFormat.aLoss,
	       pixelFormat.rShift, pixelFormat.gShift, pixelFormat.bShift, pixelFormat.aShift);
#endif
	assert(pixelFormat.bytesPerPixel == 1 || pixelFormat.bytesPerPixel == 2);

	if (_mouseBuffer.w != w || _mouseBuffer.h != h || _mouseBuffer.format != pixelFormat || !_mouseBuffer.getPixels())
		_mouseBuffer.create(w, h, pixelFormat);

	_videoContext->mouseWidth = w;
	_videoContext->mouseHeight = h;

	_videoContext->mouseHotspotX = hotspotX;
	_videoContext->mouseHotspotY = hotspotY;

	_mouseKeyColor = keycolor;

	memcpy(_mouseBuffer.getPixels(), buf, h * _mouseBuffer.pitch);

	_mouseDirty = true;
	_mouseNeedTextureUpdate = true;
}

void OSystem_iOS7::setCursorPalette(const byte *colors, uint start, uint num) {
	//printf("setCursorPalette(%p, %u, %u)\n", (const void *)colors, start, num);
	assert(start + num <= 256);

	for (uint i = start; i < start + num; ++i, colors += 3)
		_mouseCursorPalette[i] = Graphics::RGBToColor<Graphics::ColorMasks<5551> >(colors[0], colors[1], colors[2]);

	// FIXME: This is just stupid, our client code seems to assume that this
	// automatically enables the cursor palette.
	_mouseCursorPaletteEnabled = true;

	if (_mouseCursorPaletteEnabled)
		_mouseDirty = _mouseNeedTextureUpdate = true;
}

void OSystem_iOS7::updateMouseTexture() {
	uint texWidth = getSizeNextPOT(_videoContext->mouseWidth);
	uint texHeight = getSizeNextPOT(_videoContext->mouseHeight);

	Graphics::Surface &mouseTexture = _videoContext->mouseTexture;
	if (mouseTexture.w != texWidth || mouseTexture.h != texHeight)
		mouseTexture.create(texWidth, texHeight, Graphics::createPixelFormat<5551>());

	if (_mouseBuffer.format.bytesPerPixel == 1) {
		const uint16 *palette;
		if (_mouseCursorPaletteEnabled)
			palette = _mouseCursorPalette;
		else
			palette = _gamePaletteRGBA5551;

		uint16 *mouseBuf = (uint16 *)mouseTexture.getPixels();
		for (uint x = 0; x < _videoContext->mouseWidth; ++x) {
			for (uint y = 0; y < _videoContext->mouseHeight; ++y) {
				const byte color = *(const byte *)_mouseBuffer.getBasePtr(x, y);
				if (color != _mouseKeyColor)
					mouseBuf[y * texWidth + x] = palette[color] | 0x1;
				else
					mouseBuf[y * texWidth + x] = 0x0;
			}
		}
	} else {
		if (crossBlit((byte *)mouseTexture.getPixels(), (const byte *)_mouseBuffer.getPixels(), mouseTexture.pitch,
			          _mouseBuffer.pitch, _mouseBuffer.w, _mouseBuffer.h, mouseTexture.format, _mouseBuffer.format)) {
			// Apply color keying since the original cursor had no alpha channel.
			const uint16 *src = (const uint16 *)_mouseBuffer.getPixels();
			uint8 *dstRaw = (uint8 *)mouseTexture.getPixels();

			for (uint y = 0; y < _mouseBuffer.h; ++y, dstRaw += mouseTexture.pitch) {
				uint16 *dst = (uint16 *)dstRaw;
				for (uint x = 0; x < _mouseBuffer.w; ++x, ++dst) {
					if (*src++ == _mouseKeyColor)
						*dst &= ~1;
					else
						*dst |= 1;
				}
			}
		} else {
			// TODO: Log this!
			// Make the cursor all transparent... we really need a better fallback ;-).
			memset(mouseTexture.getPixels(), 0, mouseTexture.h * mouseTexture.pitch);
		}
	}

	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] updateMouseCursor];
	});
}

void OSystem_iOS7::setShowKeyboard(bool show) {
	if (show) {
		execute_on_main_thread(^ {
			[[iOS7AppDelegate iPhoneView] showKeyboard];
		});
	} else {
		// Do not hide the keyboard in portrait mode as it is shown automatically and not
		// just when asked with the kFeatureVirtualKeyboard.
		if (_screenOrientation == kScreenOrientationLandscape || _screenOrientation == kScreenOrientationFlippedLandscape) {
			execute_on_main_thread(^ {
				[[iOS7AppDelegate iPhoneView] hideKeyboard];
			});
		}
	}
}

bool OSystem_iOS7::isKeyboardShown() const {
	return [[iOS7AppDelegate iPhoneView] isKeyboardShown];
}
