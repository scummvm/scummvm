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
 */

#ifndef PLAYGROUND3D_GFX_H
#define PLAYGROUND3D_GFX_H

#include "common/rect.h"
#include "common/system.h"

#include "math/frustum.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Playground3d {

class Renderer {
public:
	Renderer(OSystem *system);
	virtual ~Renderer();

	virtual void init() = 0;
	virtual void clear() = 0;

	/**
	 *  Swap the buffers, making the drawn screen visible
	 */
	virtual void flipBuffer() { }

	Common::Rect viewport() const;

	void setupCameraPerspective(float pitch, float heading, float fov);

	static const int kOriginalWidth = 640;
	static const int kOriginalHeight = 480;

	void computeScreenViewport();

	virtual void drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) = 0;

protected:
	OSystem *_system;

	Common::Rect _screenViewport;

	Math::Matrix4 _projectionMatrix;
	Math::Matrix4 _modelViewMatrix;
	Math::Matrix4 _mvpMatrix;

	static const float cubeVertices[11 * 6 * 4];

	Math::Matrix4 makeProjectionMatrix(float fov, float nearClip, float farClip) const;
};

Renderer *CreateGfxOpenGL(OSystem *system);
Renderer *CreateGfxOpenGLShader(OSystem *system);
Renderer *CreateGfxTinyGL(OSystem *system);
Renderer *createRenderer(OSystem *system);

} // End of namespace Playground3d

#endif // PLAYGROUND3D_GFX_H
