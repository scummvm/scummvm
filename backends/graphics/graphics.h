/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef BACKENDS_GRAPHICS_ABSTRACT_H
#define BACKENDS_GRAPHICS_ABSTRACT_H

#include "common/system.h"
#include "common/noncopyable.h"
#include "common/keyboard.h"

//#include "graphics/palette.h"

/**
 * Abstract class for graphics manager. Subclasses
 * implement the real functionality.
 */
class GraphicsManager : Common::NonCopyable  {
public:
	virtual ~GraphicsManager() {}

	virtual bool hasFeature(OSystem::Feature f) = 0;
	virtual void setFeatureState(OSystem::Feature f, bool enable) = 0;
	virtual bool getFeatureState(OSystem::Feature f) = 0;

	virtual void launcherInitSize(uint w, uint h) = 0;
	virtual Graphics::PixelBuffer setupScreen(int screenW, int screenH, bool fullscreen, bool accel3d) = 0;
	virtual int getScreenChangeID() const = 0;
	virtual int16 getHeight() = 0;
	virtual int16 getWidth() = 0;
	virtual void updateScreen() = 0;

	virtual void showOverlay() = 0;
	virtual void hideOverlay() = 0;
	virtual Graphics::PixelFormat getOverlayFormat() const = 0;
	virtual void clearOverlay() = 0;
	virtual void grabOverlay(OverlayColor *buf, int pitch) = 0;
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h)= 0;
	virtual int16 getOverlayHeight() = 0;
	virtual int16 getOverlayWidth() = 0;

	virtual bool showMouse(bool visible) = 0;
	virtual void warpMouse(int x, int y) = 0;
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale = 1, const Graphics::PixelFormat *format = NULL) = 0;

	// ResidualVM specific method
	virtual bool lockMouse(bool lock) = 0;

	virtual void displayMessageOnOSD(const char *msg) {}
};

#endif
