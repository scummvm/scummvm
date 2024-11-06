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

#ifndef WINTERMUTE_XMATERIAL_H
#define WINTERMUTE_XMATERIAL_H

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/base/gfx/xskinmesh.h"

namespace Wintermute {

class Effect3D;
class Effect3DParams;
class BaseSprite;
class BaseSurface;
class VideoTheoraPlayer;
class XFileData;

class Material : public BaseNamedObject {
public:
	Material(BaseGame *inGame);
	virtual ~Material();

	DXMaterial _material;

	bool setTexture(const Common::String &filename, bool adoptName = false);
	bool setSprite(BaseSprite *sprite, bool adoptName = false);
	bool setTheora(VideoTheoraPlayer *theora, bool adoptName = false);
	bool setEffect(Effect3D *effect, Effect3DParams *params, bool adoptName = false);

	BaseSurface *getSurface();
	Effect3D *getEffect() { return _effect; }
	Effect3DParams *getEffectParams() { return _params; }

	bool invalidateDeviceObjects();
	bool restoreDeviceObjects();

private:
	Common::String _textureFilename;
	BaseSurface *_surface;
	bool _ownedSurface;
	BaseSprite *_sprite;
	VideoTheoraPlayer *_theora;
	Effect3D *_effect;
	Effect3DParams *_params;
};

} // namespace Wintermute

#endif
