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
 * $URL$
 * $Id$
 *
 */

#ifndef INTERFACE_MANAGER_H
#define INTERFACE_MANAGER_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "common/system.h"

#include "graphics/VectorRenderer.h"

namespace GUI {

class InterfaceManager {

public:
	enum Graphics_Mode {
		GFX_Disabled = 0,
		GFX_Standard_16bit,
		GFX_Antialias_16bit
	};

	InterfaceManager(OSystem *system, Graphics_Mode mode) : _vectorRenderer(NULL), 
		_system(system), _graphicsMode(GFX_Disabled), _screen(NULL), _bytesPerPixel(0) {

		setGraphicsMode(mode);
	}

	~InterfaceManager() {
		freeRenderer();
		freeScreen();
	}

	void setGraphicsMode(Graphics_Mode mode);
	int runGUI();

protected:
	template<typename PixelType> void screenInit();

	void freeRenderer() {
		if (_vectorRenderer != NULL)
			delete _vectorRenderer;
	}

	void freeScreen() {
		if (_screen != NULL) {
			_screen->free();
			delete _screen;
		}
	}

	OSystem *_system;
	Graphics::VectorRenderer *_vectorRenderer;
	Graphics::Surface *_screen;

	int _bytesPerPixel;
	Graphics_Mode _graphicsMode;
};

} // end of namespace GUI.

#endif
