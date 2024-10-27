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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/3dlight.h"
#include "engines/wintermute/base/gfx/3dloader_3ds.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/wintypes.h"

#include "graphics/opengl/system_headers.h"

#include "math/glmath.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Light3D::Light3D(BaseGame *inGame) : BaseScriptable(inGame, false, false) {
	_diffuseColor = BYTETORGBA(255, 255, 255, 255);
	_pos = DXVector3(0, 0, 0);
	_target = DXVector3(0, 0, 0);
	_isSpotlight = false;
	_falloff = 0;
	_active = true;

	_distance = 0.0f;
	_isAvailable = false;
}

//////////////////////////////////////////////////////////////////////////
Light3D::~Light3D() {
}

//////////////////////////////////////////////////////////////////////////
bool Light3D::setLight(int index) {
	DXVector4 diffuse;
	diffuse._x = RGBCOLGetR(_diffuseColor) / 256.0f;
	diffuse._y = RGBCOLGetG(_diffuseColor) / 256.0f;
	diffuse._z = RGBCOLGetB(_diffuseColor) / 256.0f;
	diffuse._w = 1.0f;

	_gameRef->_renderer3D->setLightParameters(index, _pos, _target - _pos, diffuse, _isSpotlight);

	if (_active) {
		_gameRef->_renderer3D->lightEnable(index, true);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Light3D::getViewMatrix(DXMatrix *viewMatrix) {
	DXVector3 up = DXVector3(0.0f, 1.0f, 0.0f);
	DXMatrixLookAtLH(viewMatrix, &_pos, &_target, &up);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Light3D::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferBool(TMEMBER(_active));
	persistMgr->transferUint32(TMEMBER(_diffuseColor));
	return true;
}

} // namespace Wintermute
