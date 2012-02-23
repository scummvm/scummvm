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
 */

#ifndef BACKENDS_PLATFORM_IPHONE_IPHONE_COMMON_H
#define BACKENDS_PLATFORM_IPHONE_IPHONE_COMMON_H

enum InputEvent {
	kInputMouseDown,
	kInputMouseUp,
	kInputMouseDragged,
	kInputMouseSecondDragged,
	kInputMouseSecondDown,
	kInputMouseSecondUp,
	kInputOrientationChanged,
	kInputKeyPressed,
	kInputApplicationSuspended,
	kInputApplicationResumed,
	kInputSwipe
};

enum ScreenOrientation {
	kScreenOrientationPortrait,
	kScreenOrientationLandscape,
	kScreenOrientationFlippedLandscape
};

enum UIViewSwipeDirection {
	kUIViewSwipeUp = 1,
	kUIViewSwipeDown = 2,
	kUIViewSwipeLeft = 4,
	kUIViewSwipeRight = 8
};

enum GraphicsModes {
	kGraphicsModeLinear = 0,
	kGraphicsModeNone = 1
};

struct VideoContext {
	// Game screen state
	uint screenWidth, screenHeight;

	// Overlay state
	bool overlayVisible;
	uint overlayWidth, overlayHeight;

	// Mouse cursor state
	uint mouseX, mouseY;
	int mouseHotspotX, mouseHotspotY;
	uint mouseWidth, mouseHeight;
	bool mouseIsVisible;

	// Misc state
	GraphicsModes graphicsMode;
	int shakeOffsetY;
};

// On the ObjC side
void iPhone_updateScreen();
void iPhone_updateScreenRect(unsigned short *screen, int x1, int y1, int x2, int y2, int width);
void iPhone_updateOverlayRect(unsigned short *screen, int x1, int y1, int x2, int y2, int width);
bool iPhone_fetchEvent(int *outEvent, int *outX, int *outY);
const char *iPhone_getDocumentsDir();
bool iPhone_isHighResDevice();
void iPhone_setMouseCursor(unsigned short *buffer);

uint getSizeNextPOT(uint size);

#endif
