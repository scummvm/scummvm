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

#ifndef BACKENDS_GRAPHICS_IOS_IOS_GRAPHICS_H
#define BACKENDS_GRAPHICS_IOS_IOS_GRAPHICS_H

#include "common/scummsys.h"
#include <OpenGLES/ES2/gl.h>

#include "backends/graphics/opengl/opengl-graphics.h"

class iOSGraphicsManager :
  public OpenGL::OpenGLGraphicsManager {
public:
	iOSGraphicsManager();
	virtual ~iOSGraphicsManager();

	void initSurface();
	void deinitSurface();

	void notifyResize(const int width, const int height);

	bool notifyMousePosition(Common::Point &mouse);
	Common::Point getMousePosition() { return Common::Point(_cursorX, _cursorY); }

	float getHiDPIScreenFactor() const override;

	void setSafeAreaInsets(int l, int r, int t, int b) { _insets.left = l; _insets.top = t; _insets.right = r; _insets.bottom = b; }
	WindowedGraphicsManager::Insets getSafeAreaInsets() const override { return _insets; }

protected:
	void setSystemMousePosition(const int x, const int y) override {}

	bool loadVideoMode(uint requestedWidth, uint requestedHeight, bool resizable, int antialiasing) override;
	void showOverlay(bool inGUI) override;
	void hideOverlay() override;

	void refreshScreen() override;

	int _old_touch_mode;
	WindowedGraphicsManager::Insets _insets;
};

#endif
