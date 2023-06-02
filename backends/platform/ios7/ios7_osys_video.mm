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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/ios7/ios7_osys_main.h"
#include "backends/platform/ios7/ios7_video.h"

#include "graphics/blit.h"
#include "backends/platform/ios7/ios7_app_delegate.h"

#define UIViewParentController(__view) ({ \
	UIResponder *__responder = __view; \
	while ([__responder isKindOfClass:[UIView class]]) \
		__responder = [__responder nextResponder]; \
	(UIViewController *)__responder; \
})

static void displayAlert(void *ctx) {
	UIAlertController* alert = [UIAlertController alertControllerWithTitle:@"Fatal Error"
								message:[NSString stringWithCString:(const char *)ctx 	encoding:NSUTF8StringEncoding]
								preferredStyle:UIAlertControllerStyleAlert];

	UIAlertAction* defaultAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
	   handler:^(UIAlertAction * action) {
		OSystem_iOS7::sharedInstance()->quit();
		abort();
	}];

	[alert addAction:defaultAction];
	[UIViewParentController([iOS7AppDelegate iPhoneView]) presentViewController:alert animated:YES completion:nil];
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
	[[iOS7AppDelegate iPhoneView] setIsInGame:YES];
}

void OSystem_iOS7::engineDone() {
	EventsBaseBackend::engineDone();
	// Allow the device going to sleep if idle while in the Launcher
	dispatch_async(dispatch_get_main_queue(), ^{
		[[UIApplication sharedApplication] setIdleTimerDisabled:NO];
	});
	[[iOS7AppDelegate iPhoneView] setIsInGame:NO];
}

void OSystem_iOS7::initVideoContext() {
	_videoContext = [[iOS7AppDelegate iPhoneView] getVideoContext];
}

static inline void execute_on_main_thread(void (^block)(void)) {
	if ([NSThread currentThread] == [NSThread mainThread]) {
		block();
	}
	else {
		dispatch_sync(dispatch_get_main_queue(), block);
	}
}

void OSystem_iOS7::updateOutputSurface() {
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] initSurface];
	});
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

void OSystem_iOS7::virtualController(bool connect) {
	execute_on_main_thread(^ {
		[[iOS7AppDelegate iPhoneView] virtualController:connect];
	});
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

void OSystem_iOS7::updateMouseTexture() {
	int texWidth = getSizeNextPOT(_videoContext->mouseWidth);
	int texHeight = getSizeNextPOT(_videoContext->mouseHeight);

	Graphics::Surface &mouseTexture = _videoContext->mouseTexture;
	if (mouseTexture.w != texWidth || mouseTexture.h != texHeight)
		mouseTexture.create(texWidth, texHeight, Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));

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
			// Apply color keying
			const uint8 * src = (const uint8 *)_mouseBuffer.getPixels();
			int srcBpp = _mouseBuffer.format.bytesPerPixel;

			uint8 *dstRaw = (uint8 *)mouseTexture.getPixels();

			for (int y = 0; y < _mouseBuffer.h; ++y, dstRaw += mouseTexture.pitch) {
				uint16 *dst = (uint16 *)dstRaw;
				for (int x = 0; x < _mouseBuffer.w; ++x, ++dst, src += srcBpp) {
					if (
						(srcBpp == 2 && *((const uint16*)src) == _mouseKeyColor) ||
						(srcBpp == 4 && *((const uint32*)src) == _mouseKeyColor)
					)
						*dst &= ~1;
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
#if TARGET_OS_IOS
		execute_on_main_thread(^ {
			[[iOS7AppDelegate iPhoneView] showKeyboard];
		});
#elif TARGET_OS_TV
		// Delay the showing of keyboard 1 second so the user
		// is able to see the message
		dispatch_time_t delay = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC));
		dispatch_after(delay, dispatch_get_main_queue(), ^(void){
			[[iOS7AppDelegate iPhoneView] showKeyboard];
		});
#endif
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
	__block bool isShown = false;
	execute_on_main_thread(^{
		isShown = [[iOS7AppDelegate iPhoneView] isKeyboardShown];
	});
	return isShown;
}

uint OSystem_iOS7::createOpenGLContext() {
	return [[iOS7AppDelegate iPhoneView] createOpenGLContext];
}

void OSystem_iOS7::destroyOpenGLContext() {
	[[iOS7AppDelegate iPhoneView] destroyOpenGLContext];
}

void OSystem_iOS7::refreshScreen() const {
	[[iOS7AppDelegate iPhoneView] refreshScreen];
}

int OSystem_iOS7::getScreenWidth() const {
	return [[iOS7AppDelegate iPhoneView] getScreenWidth];
}

int OSystem_iOS7::getScreenHeight() const {
	return [[iOS7AppDelegate iPhoneView] getScreenHeight];
}

float OSystem_iOS7::getSystemHiDPIScreenFactor() const {
	return [[UIScreen mainScreen] scale];
}

