/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_SHADOW_VOLUME_H
#define WINTERMUTE_SHADOW_VOLUME_H

#include "engines/wintermute//base/base.h"
#include "engines/wintermute/coll_templ.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class ShadowVolume : public BaseClass {
public:
	ShadowVolume(BaseGame *inGame);
	virtual ~ShadowVolume();

	// we need to pass mesh information in some way
	bool addMesh(uint32 *adjacency, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth);
	bool reset();

	bool renderToStencilBuffer();
	bool renderToScene();

	bool setColor(uint32 color);

private:
	bool render();
	inline void addVertex(Math::Vector3d &vertex);
	uint32 _color;
	bool initMask();
	BaseArray<Math::Vector3d> _vertices; // Vertex data for rendering shadow volume
};

} // namespace Wintermute

#endif
