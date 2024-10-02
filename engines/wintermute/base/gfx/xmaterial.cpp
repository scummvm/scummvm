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
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/gfx/xmaterial.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"
#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/video/video_theora_player.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
Material::Material(BaseGame *inGame) : BaseNamedObject(inGame) {
	memset(&_material, 0, sizeof(_material));
	_surface = nullptr;
	_ownedSurface = false;
	_sprite = nullptr;
	_theora = nullptr;
}

//////////////////////////////////////////////////////////////////////////
Material::~Material() {
	if (_surface && _ownedSurface) {
		_gameRef->_surfaceStorage->removeSurface(_surface);
	}

	_sprite = nullptr; // ref only
	_theora = nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool Material::invalidateDeviceObjects() {
	// as long as we don't support D3DX effects, there is nothing to be done here
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Material::restoreDeviceObjects() {
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Material::setTexture(const Common::String &filename, bool adoptName) {
	if (adoptName) {
		setName(PathUtil::getFileName(filename).c_str());
	}

	_textureFilename = filename;

	if (_surface && _ownedSurface) {
		_gameRef->_surfaceStorage->removeSurface(_surface);
	}

	_surface = _gameRef->_surfaceStorage->addSurface(_textureFilename);
	_ownedSurface = true;
	_sprite = nullptr;

	return _surface != nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool Material::setSprite(BaseSprite *sprite, bool adoptName) {
	if (adoptName) {
		setName(PathUtil::getFileName(sprite->getFilename()).c_str());
	}

	_textureFilename = sprite->getFilename();
	if (_surface && _ownedSurface) {
		_gameRef->_surfaceStorage->removeSurface(_surface);
	}

	_surface = nullptr;

	_sprite = sprite;
	_ownedSurface = false;
	_theora = nullptr;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Material::setTheora(VideoTheoraPlayer *theora, bool adoptName) {
	if (adoptName) {
		setName(PathUtil::getFileName(theora->_filename).c_str());
	}
	_textureFilename = theora->_filename;

	if (_surface && _ownedSurface) {
		_gameRef->_surfaceStorage->removeSurface(_surface);
	}

	_surface = nullptr;

	_theora = theora;
	_ownedSurface = false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
BaseSurface *Material::getSurface() {
	if (_theora) {
		_theora->update();
		return _theora->getTexture();
	} else if (_sprite) {
		_sprite->getCurrentFrame();
		return _sprite->getSurface();
	} else {
		return _surface;
	}
}

} // namespace Wintermute
