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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_3D_MESH_H
#define WINTERMUTE_3D_MESH_H

#include "common/memstream.h"

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/wintermute/base/gfx/xbuffer.h"
#include "engines/wintermute/base/gfx/3dface.h"
#include "engines/wintermute/base/gfx/3dvertex.h"

namespace Wintermute {

#if defined(SCUMMVM_USE_PRAGMA_PACK)
#pragma pack(4)
#endif

struct Mesh3DSVertex {
	float _x, _y, _z;
	float _nx, _ny, _nz;
	float _r, _g, _b, _a;
};

#if defined(SCUMMVM_USE_PRAGMA_PACK)
#pragma pack()
#endif

class Mesh3DS : public BaseNamedObject {
public:
	bool persist(BasePersistenceManager *persistMgr);
	bool createVertexBuffer();
	void computeNormals();
	void cleanup();
	Mesh3DS(BaseGame *inGame);
	virtual ~Mesh3DS();
	virtual void fillVertexBuffer() = 0;
	bool fillVertexBuffer(uint32 color);
	virtual void render() = 0;

	Face3D *_faces;
	uint16 _numFaces;
	uint16 _numVertices;
	Vertex3D *_vertices;
	DXBuffer _vb;
	bool _visible;
};

} // namespace Wintermute

#endif
