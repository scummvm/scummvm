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

#ifndef WINTERMUTE_MESH_X_H
#define WINTERMUTE_MESH_X_H

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/coll_templ.h"
#include "graphics/opengl/system_headers.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class BaseSprite;
class FrameNode;
class ModelX;
class ShadowVolume;
class VideoTheoraPlayer;
class XFileLexer;

class MeshX : public BaseNamedObject {
public:
	MeshX(BaseGame *inGame);
	virtual ~MeshX();

	virtual bool loadFromX(const Common::String &filename, XFileLexer &lexer) = 0;
	virtual bool findBones(FrameNode *rootFrame) = 0;
	virtual bool update(FrameNode *parentFrame) = 0;
	virtual bool render(ModelX *model) = 0;
	virtual bool updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth) = 0;

	virtual bool pickPoly(Math::Vector3d *pickRayOrig, Math::Vector3d *pickRayDir) = 0;

	Math::Vector3d _BBoxStart;
	Math::Vector3d _BBoxEnd;

	virtual bool setMaterialSprite(const Common::String &matName, BaseSprite *sprite) = 0;
	virtual bool setMaterialTheora(const Common::String &matName, VideoTheoraPlayer *theora) = 0;

	virtual bool invalidateDeviceObjects() = 0;
	virtual bool restoreDeviceObjects() = 0;
};

} // namespace Wintermute

#endif
