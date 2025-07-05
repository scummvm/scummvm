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

#ifndef BACKENDS_GRAPHICS_ANDROID_ANDROID_GRAPHICS_H
#define BACKENDS_GRAPHICS_ANDROID_ANDROID_GRAPHICS_H

#include "common/scummsys.h"
#include "backends/graphics/opengl/opengl-graphics.h"

#include "backends/platform/android/touchcontrols.h"

class AndroidGraphicsManager :
	public OpenGL::OpenGLGraphicsManager, public TouchControlsDrawer {
public:
	AndroidGraphicsManager();
	virtual ~AndroidGraphicsManager();

	void initSurface();
	void deinitSurface();
	void resizeSurface();

	WindowedGraphicsManager::Insets getSafeAreaInsets() const override;

	void updateScreen() override;

	void displayMessageOnOSD(const Common::U32String &msg) override;

	bool notifyMousePosition(Common::Point &mouse);
	Common::Point getMousePosition() { return Common::Point(_cursorX, _cursorY); }

	float getHiDPIScreenFactor() const override;

	void touchControlInitSurface(const Graphics::ManagedSurface &surf) override;
	void touchControlNotifyChanged() override;
	void touchControlDraw(uint8 alpha, int16 x, int16 y, int16 w, int16 h, const Common::Rect &clip) override;

	void syncVirtkeyboardState(bool virtkeybd_on);
	void applyTouchSettings() const;

protected:
	void recalculateDisplayAreas() override;
	void setSystemMousePosition(const int x, const int y) override {}

	void showOverlay(bool inGUI) override;
	void hideOverlay() override;


	bool loadVideoMode(uint requestedWidth, uint requestedHeight, bool resizable, int antialiasing) override;

	void refreshScreen() override;

private:
	OpenGL::Surface *_touchcontrols;
	int _old_touch_mode;
	bool _rendering3d;
};

#endif
