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

#ifndef BACKENDS_GRAPHICS_IOS_IOS_METAL_GRAPHICS_H
#define BACKENDS_GRAPHICS_IOS_IOS_METAL_GRAPHICS_H

#include "backends/graphics/metal/metal-graphics.h"
#include "backends/graphics/ios/ios-graphics.h"
#include "backends/graphics/ios/metal-renderbuffer.h"


class iOSMetalGraphicsManager : public Metal::MetalGraphicsManager, public iOSCommonGraphics {
public:
	iOSMetalGraphicsManager();
	virtual ~iOSMetalGraphicsManager();
	
	void initSurface() override;
	void deinitSurface() override;
	
	void notifyResize(const int width, const int height) override;
	
	iOSCommonGraphics::State getState() const override;
	bool setState(const iOSCommonGraphics::State &state) override;
	
	bool notifyMousePosition(Common::Point &mouse) override;
	Common::Point getMousePosition() override { return Common::Point(_cursorX, _cursorY); }
	
	float getHiDPIScreenFactor() const override;

protected:
	void setSystemMousePosition(const int x, const int y) override {}
	
	void showOverlay(bool inGUI) override;
	void hideOverlay() override;
	void refreshScreen() override;
	
private:
	 Metal::MetalRenderbufferTarget *_renderTarget;
};
#endif
