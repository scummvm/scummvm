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

#include "common/pack-start.h"

struct ShadowVertex {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
	float x;
	float y;
	float z;
} PACKED_STRUCT;

#include "common/pack-end.h"

class ShadowVolume : public BaseClass {
public:
	ShadowVolume(BaseGame *inGame);
	virtual ~ShadowVolume();

	void addVertex(const Math::Vector3d &vertex);
	bool reset();

	virtual bool renderToStencilBuffer() = 0;
	virtual bool renderToScene() = 0;

	bool setColor(uint32 color);

protected:
	BaseArray<Math::Vector3d> _vertices; // Vertex data for rendering shadow volume
	uint32 _color;

private:
	virtual bool initMask() = 0;
};

} // namespace Wintermute

#endif
