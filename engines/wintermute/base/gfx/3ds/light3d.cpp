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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/3ds/light3d.h"
#include "engines/wintermute/base/gfx/3ds/loader3ds.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/wintypes.h"
#include "graphics/opengl/system_headers.h"
#include "math/glmath.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Light3D::Light3D(BaseGame *inGame) : BaseScriptable(inGame, false, false),
									 _diffuseColor(BYTETORGBA(255, 255, 255, 255)),
									 _position(0, 0, 0), _target(0, 0, 0), _isSpotlight(false),
									 _active(true), _falloff(0), _distance(0.0f), _isAvailable(false) {
}

//////////////////////////////////////////////////////////////////////////
Light3D::~Light3D() {
}

//////////////////////////////////////////////////////////////////////////
bool Light3D::setLight(int index) {
	Math::Vector4d diffuse;
	diffuse.getData()[0] = RGBCOLGetR(_diffuseColor) / 256.0f;
	diffuse.getData()[1] = RGBCOLGetG(_diffuseColor) / 256.0f;
	diffuse.getData()[2] = RGBCOLGetB(_diffuseColor) / 256.0f;
	diffuse.getData()[3] = 1.0f;

	_gameRef->_renderer3D->setLightParameters(index, _position, _target - _position, diffuse, _isSpotlight);

	if (_active) {
		_gameRef->_renderer3D->enableLight(index);
	}

	return true;
}

bool Light3D::loadFrom3DS(Common::MemoryReadStream &fileStream) {
	uint32 wholeChunkSize = fileStream.readUint32LE();
	int32 end = fileStream.pos() + wholeChunkSize - 6;

	_position.x() = fileStream.readFloatLE();
	_position.z() = -fileStream.readFloatLE();
	_position.y() = fileStream.readFloatLE();

	while (fileStream.pos() < end) {
		uint16 chunkId = fileStream.readUint16LE();
		uint32 chunkSize = fileStream.readUint32LE();

		switch (chunkId) {
		case SPOTLIGHT:
			_target.x() = fileStream.readFloatLE();
			_target.z() = -fileStream.readFloatLE();
			_target.y() = fileStream.readFloatLE();

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

//////////////////////////////////////////////////////////////////////////
bool Light3D::getViewMatrix(Math::Matrix4 *viewMatrix) {
	Math::Vector3d up = Math::Vector3d(0.0f, 1.0f, 0.0f);
	*viewMatrix = Math::makeLookAtMatrix(_position, _target, up);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Light3D::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferBool("_active", &_active);
	persistMgr->transferUint32("_diffuseColor", &_diffuseColor);
	return true;
}

} // namespace Wintermute
