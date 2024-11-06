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

#ifndef WINTERMUTE_XMESH_H
#define WINTERMUTE_XMESH_H

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/coll_templ.h"

#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class BaseSprite;
class FrameNode;
class Material;
class ShadowVolume;
class VideoTheoraPlayer;
class SkinMeshHelper;
class DXMesh;
class Effect3D;
class Effect3DParams;
struct XMeshObject;

class XMesh : public BaseNamedObject {
	friend class XSkinMeshLoader;
public:
	XMesh(BaseGame *inGame);
	virtual ~XMesh();

	virtual bool loadFromXData(const Common::String &filename, XFileData *xobj);
	bool findBones(FrameNode *rootFrame);
	virtual bool update(FrameNode *parentFrame);
	virtual bool render(XModel *model) = 0;
	virtual bool renderFlatShadowModel() = 0;
	bool updateShadowVol(ShadowVolume *shadow, DXMatrix *modelMat, DXVector3 *light, float extrusionDepth);

	bool pickPoly(DXVector3 *pickRayOrig, DXVector3 *pickRayDir);

	DXVector3 _BBoxStart;
	DXVector3 _BBoxEnd;

	bool setMaterialSprite(const Common::String &matName, BaseSprite *sprite);
	bool setMaterialTheora(const Common::String &matName, VideoTheoraPlayer *theora);
	bool setMaterialEffect(const Common::String &matName, Effect3D *effect, Effect3DParams *params);
	bool removeMaterialEffect(const Common::String &matName);

	bool invalidateDeviceObjects();
	bool restoreDeviceObjects();

protected:
	bool generateMesh();

	SkinMeshHelper *_skinMesh;
	DXMesh *_blendedMesh;
	DXMesh *_staticMesh;

	DXMatrix **_boneMatrices;

	uint32 *_adjacency;

	BaseArray<Material *> _materials;
};

} // namespace Wintermute

#endif
