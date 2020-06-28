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
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/gfx/opengl/material.h"
#include "engines/wintermute/base/gfx/x/loader_x.h"
#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/video/video_theora_player.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
Material::Material(BaseGame *inGame) : BaseNamedObject(inGame),
                                       _surface(nullptr), _ownedSurface(false),
                                       _sprite(nullptr), _theora(nullptr) {
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

bool Material::loadFromX(XFileLexer &lexer, const Common::String &filename) {
	lexer.advanceToNextToken(); // skip optional name
	lexer.advanceOnOpenBraces();

	_diffuse.r() = readFloat(lexer);
	_diffuse.g() = readFloat(lexer);
	_diffuse.b() = readFloat(lexer);
	_diffuse.a() = readFloat(lexer);
	lexer.advanceToNextToken(); // skip semicolon

	_shininess = readFloat(lexer);

	_specular.r() = readFloat(lexer);
	_specular.g() = readFloat(lexer);
	_specular.b() = readFloat(lexer);
	lexer.advanceToNextToken(); // skip semicolon

	_emissive.r() = readFloat(lexer);
	_emissive.g() = readFloat(lexer);
	_emissive.b() = readFloat(lexer);
	lexer.advanceToNextToken();

	while (!lexer.eof()) {
		// according to assimp sources, we got both possibilities
		// wine also seems to support this
		// MSDN only names the first option
		if (lexer.tokenIsIdentifier("TextureFilename") || lexer.tokenIsIdentifier("TextureFileName")) {
			lexer.advanceToNextToken(); // skip optional name
			lexer.advanceOnOpenBraces();

			Common::String textureFilename = readString(lexer);
			PathUtil::getDirectoryName(filename);
			setTexture(PathUtil::getDirectoryName(filename) + textureFilename);
			lexer.advanceToNextToken(); // skip semicolon
		} else if (lexer.tokenIsIdentifier()) {
			warning("Material::loadFromX unexpected token %i", lexer.getTypeOfToken());
			return false;
		} else {
			break;
		}
	}

	lexer.advanceToNextToken(); // skip semicolon
	return true;
}

} // namespace Wintermute
