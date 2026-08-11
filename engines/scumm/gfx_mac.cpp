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
#include "common/system.h"

#include "graphics/macega.h"

#include "scumm/charset.h"
#include "scumm/macgui/macgui.h"

namespace Scumm {

void ScummEngine::mac_markScreenAsDirty(int x, int y, int w, int h) {
	// Mark the virtual screen as dirty. The top and left coordinates are
	// rounded down, while the bottom and right ones are rounded up.

	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	int vsTop = y / 2 - vs->topline;
	int vsBottom = (y + h) / 2 - vs->topline;
	int vsLeft = x / 2;
	int vsRight = (x + w) / 2;

	if ((y + h) & 1)
		vsBottom++;

	if ((x + w) & 1)
		vsRight++;

	markRectAsDirty(kMainVirtScreen, vsLeft, vsRight, vsTop, vsBottom);
}

void ScummEngine::mac_drawStripToScreen(VirtScreen *vs, int top, int x, int y, int width, int height) {
	// The verb screen is completely replaced with a custom GUI. While
	// it is active, all other drawing to that area is suspended.
	if (_macGui && vs->number == kVerbVirtScreen && _macGui->isVerbGuiActive())
		return;

	const byte *pixels = vs->getPixels(x, top);
	const byte *ts = (byte *)_textSurface.getBasePtr(x * 2, y * 2);
	byte *mac = (byte *)_macScreen->getBasePtr(x * 2, y * 2 + _macScreenDrawOffset * 2);

	int pixelsPitch = vs->pitch;
	int tsPitch = _textSurface.pitch;
	int macPitch = _macScreen->pitch;

	// In b/w Mac rendering mode, the shadow palette is implemented here,
	// and not as a palette manipulation. See special cases in o5_roomOps()
	// and updatePalette().
	//
	// This is used at the very least for the lightning flashes at Castle
	// Brunwald in Indy 3, as well as the scene where the dragon finds
	// Rusty in Loom.
	//
	// Interestingly, the original Mac interpreter does not seem to do
	// this, and instead just renders the scene as if the palette was
	// unmodified. At least, that's what Mini vMac did when I tried it.

	if (_renderMode == Common::kRenderMacintoshBW) {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				int color = enhancementEnabled(kEnhVisualChanges) ? _shadowPalette[pixels[w]] : pixels[w];
				if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w] = Graphics::macEGADither[color][0];
				if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + 1] = Graphics::macEGADither[color][1];
				if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch] = Graphics::macEGADither[color][2];
				if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch + 1] = Graphics::macEGADither[color][3];
			}

			pixels += pixelsPitch;
			ts += tsPitch * 2;
			mac += macPitch * 2;
		}
	} else {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w] = pixels[w];
				if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + 1] = pixels[w];
				if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch] = pixels[w];
				if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch + 1] = pixels[w];
			}

			pixels += pixelsPitch;
			ts += tsPitch * 2;
			mac += macPitch * 2;
		}
	}

	_system->copyRectToScreen(_macScreen->getBasePtr(x * 2, y * 2 + _macScreenDrawOffset * 2), _macScreen->pitch, x * 2, y * 2 + _macScreenDrawOffset * 2, width * 2, height * 2);
}

void ScummEngine::mac_drawIndy3TextBox() {
	Graphics::Surface *s = _macGui->textArea();

	// The first two rows of the text box are padding for font rendering.
	// They are not drawn to the screen.

	int x = 96;
	int y = 32;
	int w = s->w;
	int h = s->h - 2;

	// The text box is drawn to the Mac screen and text surface, as if it
	// had been one giant glyph. Note that it will be drawn on the main
	// virtual screen, but we still pretend it's on the text one.

	byte *ptr = (byte *)s->getBasePtr(0, 2);
	int pitch = s->pitch;

	_macScreen->copyRectToSurface(ptr, pitch, x, y + 2 * _macScreenDrawOffset, w, h);
	_textSurface.fillRect(Common::Rect(x, y, x + w, y + h), 0);

	mac_markScreenAsDirty(x, y, w, h);
}

void ScummEngine::mac_undrawIndy3TextBox() {
	Graphics::Surface *s = _macGui->textArea();

	int x = 96;
	int y = 32;
	int w = s->w;
	int h = s->h - 2;

	_macScreen->fillRect(Common::Rect(x, y + 2 * _macScreenDrawOffset, x + w, y + h + 2 * _macScreenDrawOffset), 0);
	_textSurface.fillRect(Common::Rect(x, y, x + w, y + h), CHARSET_MASK_TRANSPARENCY);

	mac_markScreenAsDirty(x, y, w, h);
}

void ScummEngine::mac_undrawIndy3CreditsText() {
	// Set _masMask to make the text clear, and _textScreenID to ensure
	// that it's the main area that's cleared.
	_charset->_hasMask = true;
	_charset->_textScreenID = kMainVirtScreen;
	restoreCharsetBg();
}

void ScummEngine::mac_drawBufferToScreen(const byte *buffer, int pitch, int x, int y, int width, int height, bool epxRectangleExpansion) {
	// Composite the dirty rectangle into _completeScreenBuffer
	mac_updateCompositeBuffer(buffer, pitch, x, y, width, height);

	if (_useMacGraphicsSmoothing) {
		// Apply the EPX scaling algorithm to produce a 640x480 image...
		mac_blitEPXImage(buffer, pitch, x, y, width, height, epxRectangleExpansion);
	} else {
		// ...otherwise just double the resolution.
		mac_blitDoubleResImage(buffer, pitch, x, y, width, height);
	}
}

void ScummEngine::mac_updateCompositeBuffer(const byte *buffer, int pitch, int x, int y, int width, int height) {
	const byte *pixels = buffer;

	assert(width <= _screenWidth && height <= _screenHeight);

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			// Calculate absolute coordinates
			int absX = x + w;
			int absY = y + h;

			// Update the complete screen buffer
			_completeScreenBuffer[absY * _screenWidth + absX] = pixels[w];
		}

		pixels += pitch;
	}
}

void ScummEngine::mac_blitDoubleResImage(const byte *buffer, int pitch, int x, int y, int width, int height) {
	byte *mac = (byte *)_macScreen->getBasePtr(x * 2, y * 2 + _macScreenDrawOffset * 2);

	mac_applyDoubleResToBuffer(buffer, mac, width, height, pitch, _macScreen->pitch);

	_system->copyRectToScreen(_macScreen->getBasePtr(x * 2, y * 2 + _macScreenDrawOffset * 2), _macScreen->pitch, x * 2, y * 2 + _macScreenDrawOffset * 2, width * 2, height * 2);
}

void ScummEngine::mac_applyDoubleResToBuffer(const byte *inputBuffer, byte *outputBuffer, int width, int height, int inputPitch, int outputPitch) {
	const byte *pixels = inputBuffer;
	byte *outPixels = outputBuffer;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			outPixels[2 * w] = pixels[w];
			outPixels[2 * w + 1] = pixels[w];
			outPixels[2 * w + outputPitch] = pixels[w];
			outPixels[2 * w + outputPitch + 1] = pixels[w];
		}

		pixels += inputPitch;
		outPixels += outputPitch * 2;
	}
}

void ScummEngine::mac_blitEPXImage(const byte *buffer, int pitch, int x, int y, int width, int height, bool epxRectangleExpansion) {
	// This is a piecewise version of EPX/Scale2x.
	//
	// Just like the original, it applies EPX not on the entire screen but just on the
	// interested "dirty" rectangle areas. This is easy: with each new rectangle we iteratively
	// piece together a representation of the entire 320x200 screen buffer (_completeScreenBuffer),
	// and for each pixel to scale, we look for its neighbors in that buffer, so that neighbors
	// outside the rectangle area will be accounted for correctly.
	//
	// So to summarize, the algorithm is applied iteratively on the rectangle areas in the queue,
	// in isolation. Unfortunately this can cause an interesting edge case:
	// consider a black screen, and two big adjacent white rectangles with the same dimensions
	// being drawn, the first one on the left and the second one on its right:
	//
	// 1. The first one gets drawn, and because of the EPX filter it exhibits rounded corners
	//    (e.g. the lower right corner);
	//
	// 2. The second one gets drawn next to the other but has no rounded corners on its left side,
	//    because the cumulative 320x200 screen buffer being pieced together already has the
	//    previously drawn rectangle in it;
	//
	// We end up with two white rectangles and a line of a couple black pixels e.g. down in the middle.
	//
	// How do we solve that? We expand the considered area one pixel outwards on every rectangle dimension,
	// so that the algorithm can update previously drawn edges, preventing the issue explained above.
	//
	// I have later found out that this is the same strategy employed on Aaron Giles' 2002 re-releases,
	// which supposedly use most of the same EPX code from the Macintosh interpreters.

	// Rectangle expansion
	int x1, y1, x2, y2;
	if (epxRectangleExpansion) {
		x1 = (x > 0) ? x - 1 : 0;
		y1 = (y > 0) ? y - 1 : 0;
		x2 = (x + width < _screenWidth) ? x + width + 1 : _screenWidth;
		y2 = (y + height < _screenHeight) ? y + height + 1 : _screenHeight;
	} else {
		x1 = (x > 0) ? x : 0;
		y1 = (y > 0) ? y : 0;
		x2 = (x + width < _screenWidth) ? x + width : _screenWidth;
		y2 = (y + height < _screenHeight) ? y + height : _screenHeight;
	}

	// Adjust output buffer accordingly
	byte *targetScreenBuf = (byte *)_macScreen->getBasePtr(x1 * 2, y1 * 2 + _macScreenDrawOffset * 2);

	// Apply the EPX/Scale2x algorithm
	mac_applyEPXToBuffer(_completeScreenBuffer, targetScreenBuf, x2 - x1, y2 - y1, _screenWidth, _macScreen->pitch, x1, y1, _screenWidth, _screenHeight);

	_system->copyRectToScreen(_macScreen->getBasePtr(x1 * 2, y1 * 2 + _macScreenDrawOffset * 2), _macScreen->pitch, x1 * 2, y1 * 2 + _macScreenDrawOffset * 2, (x2 - x1) * 2, (y2 - y1) * 2);
}

void ScummEngine::mac_applyEPXToBuffer(const byte *inputBuffer, byte *outputBuffer, int width, int height, int inputPitch, int outputPitch, int xOffset, int yOffset, int bufferWidth, int bufferHeight) {
	// The EPX/Scale2x algorithm scales a single pixel to
	// four pixels based on the original pixel's neighbors.
	//
	//                +---+---+
	//    A           | 1 | 2 |
	//  C P B   -->   +---+---+
	//    D           | 3 | 4 |
	//                +---+---+
	//
	// Let P be the pixel, and A,B,C,D its neighbors,
	// then the new 1,2,3,4 pixels are defined as follows:
	//
	// 1=P; 2=P; 3=P; 4=P;
	//
	// IF C==A => 1=A
	// IF A==B => 2=B
	// IF D==C => 3=C
	// IF B==D => 4=D
	//
	// IF of A, B, C, D, three or more are identical: 1=2=3=4=P

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			// Calculate absolute screen coordinates of the current pixel
			int absX = w + xOffset;
			int absY = h + yOffset;

			if (absX < 0 || absX >= bufferWidth || absY < 0 || absY >= bufferHeight)
				continue; // Skip out-of-bounds pixels

			// Center pixel
			byte P = inputBuffer[absY * inputPitch + absX];

			// Top neighbor (A)
			byte A = (absY > 0) ? inputBuffer[(absY - 1) * inputPitch + absX] : P;

			// Right neighbor (B)
			byte B = (absX < bufferWidth - 1) ? inputBuffer[absY * inputPitch + (absX + 1)] : P;

			// Left neighbor (C)
			byte C = (absX > 0) ? inputBuffer[absY * inputPitch + (absX - 1)] : P;

			// Bottom neighbor (D)
			byte D = (absY < bufferHeight - 1) ? inputBuffer[(absY + 1) * inputPitch + absX] : P;

			int expW = w * 2;
			int expH = h * 2;

			// Actually scale the pixel
			if (expW >= 0 && expW + 1 < width * 2 && expH >= 0 && expH + 1 < height * 2) {
				outputBuffer[expH * outputPitch + expW]           = (C == A && C != D && A != B) ? A : P; // Top-left
				outputBuffer[expH * outputPitch + expW + 1]       = (A == B && A != C && B != D) ? B : P; // Top-right
				outputBuffer[(expH + 1) * outputPitch + expW]     = (D == C && D != B && C != A) ? C : P; // Bottom-left
				outputBuffer[(expH + 1) * outputPitch + expW + 1] = (B == D && B != A && D != C) ? D : P; // Bottom-right
			}
		}
	}
}

void ScummEngine::mac_scaleCursor(byte *&outCursor, int &outHotspotX, int &outHotspotY, int &outWidth, int &outHeight) {
	int newWidth = _cursor.width * 2;
	int newHeight = _cursor.height * 2;

	memset(_macGrabbedCursor, 0, ARRAYSIZE(_macGrabbedCursor));

	if (_useMacGraphicsSmoothing) {
		mac_applyEPXToBuffer(_grabbedCursor, _macGrabbedCursor, _cursor.width, _cursor.height, _cursor.width, newWidth, 0, 0, _cursor.width, _cursor.height);
	} else {
		mac_applyDoubleResToBuffer(_grabbedCursor, _macGrabbedCursor, _cursor.width, _cursor.height, _cursor.width, newWidth);
	}

	outWidth = newWidth;
	outHeight = newHeight;
	outCursor = _macGrabbedCursor;
	outHotspotX = _cursor.hotspotX * 2;
	outHotspotY = _cursor.hotspotY * 2;
}

void ScummEngine::mac_toggleSmoothing() {
	_useMacGraphicsSmoothing = !_useMacGraphicsSmoothing;

	ConfMan.setBool("mac_graphics_smoothing", _useMacGraphicsSmoothing);
	ConfMan.flushToDisk();

	// Allow the engine to update the graphics mode
	markRectAsDirty(kBannerVirtScreen, 0, 320, 0, 200);
	markRectAsDirty(kTextVirtScreen, 0, 320, 0, 200);
	markRectAsDirty(kVerbVirtScreen, 0, 320, 0, 200);
	markRectAsDirty(kMainVirtScreen, 0, 320, 0, 200);

	if (_game.version > 5)
		updateCursor();
}

Common::KeyState ScummEngine::mac_showOldStyleBannerAndPause(const char *msg, int32 waitTime) {
	char bannerMsg[512];

	_messageBannerActive = true;

	// Fetch the translated string for the message...
	convertMessageToString((const byte *)msg, (byte *)bannerMsg, sizeof(bannerMsg));

	Common::KeyState ks = Common::KEYCODE_INVALID;

	_macGui->drawBanner(bannerMsg);

	if (waitTime) {
		bool leftBtnPressed = false, rightBtnPressed = false;
		waitForBannerInput(waitTime, ks, leftBtnPressed, rightBtnPressed);
	}

	_macGui->undrawBanner();
	clearClickedStatus();
	_messageBannerActive = false;

	return ks;
}

} // End of namespace Scumm

