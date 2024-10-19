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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/3dshadow_volume.h"
#include "engines/wintermute/dcgf.h"
#include "graphics/opengl/system_headers.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
ShadowVolume::ShadowVolume(BaseGame *inGame) : BaseClass(inGame), _color(0x7f000000) {
}

//////////////////////////////////////////////////////////////////////////
ShadowVolume::~ShadowVolume() {
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::reset() {
	_vertices.clear();
	return true;
}

//////////////////////////////////////////////////////////////////////////^M
bool ShadowVolume::addMesh(DXMesh *mesh, uint32 *adjacency, DXMatrix *modelMat, DXVector3 *light, float extrusionDepth) {
	// TODO

	return true;
}
//////////////////////////////////////////////////////////////////////////
void ShadowVolume::addVertex(DXVector3 &vertex) {
	_vertices.add(vertex);
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::setColor(uint32 color) {
	if (color != _color) {
		_color = color;
		return initMask();
	} else {
		return true;
	}
}

} // namespace Wintermute
