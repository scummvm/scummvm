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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_3D_SHADOW_VOLUME_H
#define WINTERMUTE_3D_SHADOW_VOLUME_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/base/gfx/xbuffer.h"
#include "engines/wintermute/base/gfx/xmesh.h"
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/wintermute/coll_templ.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

struct ShadowVertex {
	float x;
	float y;
	float z;
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
};

class ShadowVolume : public BaseClass {
public:
	ShadowVolume(BaseGame *inGame);
	virtual ~ShadowVolume();

	bool addMesh(DXMesh *mesh, uint32 *adjacency, DXMatrix *modelMat, DXVector3 *light, float extrusionDepth);
	void addVertex(DXVector3 &vertex);
	bool reset();

	virtual bool renderToStencilBuffer() = 0;
	virtual bool renderToScene() = 0;

	bool setColor(uint32 color);

protected:
	BaseArray<DXVector3> _vertices; // Vertex data for rendering shadow volume
	uint32 _color;

private:
	virtual bool initMask() = 0;
};

} // namespace Wintermute

#endif
