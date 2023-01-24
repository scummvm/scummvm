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

#ifndef BACKENDS_PLATFORM_IOS7_IOS7_COMMON_H
#define BACKENDS_PLATFORM_IOS7_IOS7_COMMON_H

#include "graphics/surface.h"


enum InputEvent {
	kInputTouchFirstDown,
	kInputTouchFirstUp,
	kInputTouchFirstDragged,
	kInputTouchSecondDragged,
	kInputTouchSecondDown,
	kInputTouchSecondUp,
	kInputMouseLeftButtonDown,
	kInputMouseLeftButtonUp,
	kInputMouseRightButtonDown,
	kInputMouseRightButtonUp,
	kInputMouseDelta,
	kInputOrientationChanged,
	kInputKeyPressed,
	kInputApplicationSuspended,
	kInputApplicationResumed,
	kInputApplicationSaveState,
	kInputApplicationClearState,
	kInputApplicationRestoreState,
	kInputSwipe,
	kInputTap,
	kInputMainMenu,
	kInputJoystickAxisMotion,
	kInputJoystickButtonDown,
	kInputJoystickButtonUp,
	kInputChanged
};

enum ScreenOrientation {
	kScreenOrientationPortrait,
	kScreenOrientationFlippedPortrait,
	kScreenOrientationLandscape,
	kScreenOrientationFlippedLandscape
};

enum UIViewSwipeDirection {
	kUIViewSwipeUp = 1,
	kUIViewSwipeDown = 2,
	kUIViewSwipeLeft = 4,
	kUIViewSwipeRight = 8
};

enum UIViewTapDescription {
	kUIViewTapSingle = 1,
	kUIViewTapDouble = 2
};

struct VideoContext {
	VideoContext() : asprectRatioCorrection(), screenWidth(), screenHeight(), overlayVisible(false),
	                 overlayInGUI(false), overlayWidth(), overlayHeight(), mouseX(), mouseY(),
	                 mouseHotspotX(), mouseHotspotY(), mouseWidth(), mouseHeight(),
	                 mouseIsVisible(), filtering(false), shakeXOffset(), shakeYOffset() {
	}

	// Game screen state
	bool asprectRatioCorrection;
	uint screenWidth, screenHeight;
	Graphics::Surface screenTexture;

	// Overlay state
	bool overlayVisible;
	bool overlayInGUI;
	uint overlayWidth, overlayHeight;
	Graphics::Surface overlayTexture;

	// Mouse cursor state
	uint mouseX, mouseY;
	int mouseHotspotX, mouseHotspotY;
	uint mouseWidth, mouseHeight;
	bool mouseIsVisible;
	Graphics::Surface mouseTexture;

	// Misc state
	bool filtering;
	int shakeXOffset;
	int shakeYOffset;
};

struct InternalEvent {
	InternalEvent() : type(), value1(), value2() {}
	InternalEvent(InputEvent t, int v1, int v2) : type(t), value1(v1), value2(v2) {}

	InputEvent type;
	int value1, value2;
};

// On the ObjC side

extern int iOS7_argc;
extern char **iOS7_argv;

void iOS7_updateScreen();
bool iOS7_fetchEvent(InternalEvent *event);
bool iOS7_isBigDevice();

void iOS7_buildSharedOSystemInstance();
void iOS7_main(int argc, char **argv);
Common::String iOS7_getDocumentsDir();
Common::String iOS7_getAppBundleDir();
bool iOS7_touchpadModeEnabled();

uint getSizeNextPOT(uint size);

#endif
