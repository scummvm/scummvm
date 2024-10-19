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
	_position = DXVector3(0, 0, 0);
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

	_gameRef->_renderer3D->setLightParameters(index, _position, _target - _position, diffuse, _isSpotlight);

	if (_active) {
		_gameRef->_renderer3D->enableLight(index);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Light3D::getViewMatrix(DXMatrix *viewMatrix) {
	DXVector3 up = DXVector3(0.0f, 1.0f, 0.0f);
	DXMatrixLookAtRH(viewMatrix, &_position, &_target, &up);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Light3D::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferBool("_active", &_active);
	persistMgr->transferUint32("_diffuseColor", &_diffuseColor);
	return true;
}

bool Light3D::loadFrom3DS(Common::MemoryReadStream &fileStream) {
	uint32 wholeChunkSize = fileStream.readUint32LE();
	int32 end = fileStream.pos() + wholeChunkSize - 6;

	_position._x = fileStream.readFloatLE();
	_position._z = -fileStream.readFloatLE();
	_position._y = fileStream.readFloatLE();

	while (fileStream.pos() < end) {
		uint16 chunkId = fileStream.readUint16LE();
		uint32 chunkSize = fileStream.readUint32LE();

		switch (chunkId) {
		case SPOTLIGHT:
			_target._x = fileStream.readFloatLE();
			_target._z = -fileStream.readFloatLE();
			_target._y = fileStream.readFloatLE();

			// this is appearently not used
			fileStream.readFloatLE();

			_falloff = fileStream.readFloatLE();
			_isSpotlight = true;
			break;

		case LIGHT_IS_OFF:
			_active = false;
			break;

		case RGB_BYTE: {
			byte r = fileStream.readByte();
			byte g = fileStream.readByte();
			byte b = fileStream.readByte();

			_diffuseColor = r << 16;
			_diffuseColor |= g << 8;
			_diffuseColor |= b;
			_diffuseColor |= 255 << 24;
			break;
		}

		case RGB_FLOAT: {
			float r = fileStream.readFloatLE();
			float g = fileStream.readFloatLE();
			float b = fileStream.readFloatLE();

			_diffuseColor = static_cast<int32>(r * 255) << 16;
			_diffuseColor |= static_cast<int32>(g * 255) << 8;
			_diffuseColor |= static_cast<int32>(b * 255);
			_diffuseColor |= 255 << 24;
			break;
		}

		case RANGE_END:
		case 0x4659:
		case MULTIPLIER:
		case ROLL:
		case SPOT_SHADOW_MAP:
		case SPOT_RAY_TRACE_BIAS:
		case SPOT_RAY_TRACE:
		default:
			fileStream.seek(chunkSize - 6, SEEK_CUR);
			break;
		}
	}

	return true;
}

} // namespace Wintermute
