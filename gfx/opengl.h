/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef STARK_GFX_OPENGL_H
#define STARK_GFX_OPENGL_H

#ifdef USE_OPENGL

#include "engines/stark/gfx/driver.h"

namespace Stark {

class OpenGLGfxDriver : public GfxDriver {
public:
	OpenGLGfxDriver();
	~OpenGLGfxDriver();

	const char *getVideoDeviceName();

	void setupScreen(int screenW, int screenH, bool fullscreen);

	void clearScreen();
	void flipBuffer();

	void drawSurface(Graphics::Surface *surface);

/*
	bool isHardwareAccelerated();

	void set3DMode();

	void setupCamera(float fov, float nclip, float fclip, float roll);
	void positionCamera(Graphics::Vector3d pos, Graphics::Vector3d interest);

	void translateViewpointStart(Graphics::Vector3d pos, float pitch, float yaw, float roll);
	void translateViewpointFinish();
*/
};

} // End of namespace Stark

#endif // USE_OPENGL

#endif // STARK_GFX_OPENGL_H
