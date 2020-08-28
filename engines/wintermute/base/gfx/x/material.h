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

#ifndef WINTERMUTE_MATERIAL_H
#define WINTERMUTE_MATERIAL_H

#include "engines/wintermute/base/base_named_object.h"

namespace Wintermute {

class BaseSprite;
class BaseSurface;
class VideoTheoraPlayer;
class XFileLexer;

struct ColorValue {
	float &r() {
		return data[0];
	}

	float r() const {
		return data[0];
	}

	float &g() {
		return data[1];
	}

	float g() const {
		return data[1];
	}

	float &b() {
		return data[2];
	}

	float b() const {
		return data[2];
	}

	float &a() {
		return data[3];
	}

	float a() const {
		return data[3];
	}

	float data[4];
};

class Material : public BaseNamedObject {
public:
	Material(BaseGame *inGame);
	virtual ~Material();

	ColorValue _diffuse;
	ColorValue _ambient;
	ColorValue _specular;
	ColorValue _emissive;
	float _shininess;

	bool setTexture(const Common::String &filename, bool adoptName = false);
	bool setSprite(BaseSprite *sprite, bool adoptName = false);
	bool setTheora(VideoTheoraPlayer *theora, bool adoptName = false);
	BaseSurface *getSurface();

	bool loadFromX(XFileLexer &lexer, const Common::String &filename);

	bool invalidateDeviceObjects();
	bool restoreDeviceObjects();

private:
	Common::String _textureFilename;
	BaseSurface *_surface;
	bool _ownedSurface;
	BaseSprite *_sprite;
	VideoTheoraPlayer *_theora;
};

} // namespace Wintermute

#endif
