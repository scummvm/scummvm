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

#ifndef WINTERMUTE_XFRAME_NODE_H
#define WINTERMUTE_XFRAME_NODE_H

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/base/gfx/xmesh.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/coll_templ.h"

#include "math/matrix4.h"
#include "math/quat.h"
#include "math/vector3d.h"

namespace Wintermute {

class XModel;
class XFileData;
class BaseSprite;
class Effect3D;
class Effect3DParams;

class FrameNode : public BaseNamedObject {
public:
	FrameNode(BaseGame *inGame);
	virtual ~FrameNode();

	bool updateMatrices(DXMatrix *parentMat);
	bool updateMeshes();
	bool resetMatrices();
	bool render(XModel *model);
	bool renderFlatShadowModel();
	bool updateShadowVol(ShadowVolume *shadow, DXMatrix *modelMat, DXVector3 *light, float extrusionDepth);

	bool loadFromXData(const Common::String &filename, XModel *model, XFileData *xobj);
	bool mergeFromXData(const Common::String &filename, XModel *model, XFileData *xobj);
	bool findBones(FrameNode *rootFrame);
	FrameNode *findFrame(const char *frameName);
	DXMatrix *getCombinedMatrix();
	DXMatrix *getOriginalMatrix();
	void setTransformationMatrix(DXMatrix *mat);
	bool pickPoly(DXVector3 *pickRayOrig, DXVector3 *pickRayDir);
	bool getBoundingBox(DXVector3 *boxStart, DXVector3 *boxEnd);

	void setTransformation(int slot, DXVector3 pos, DXVector3 scale, DXQuaternion rot, float lerpValue);

	bool hasChildren();

	bool setMaterialSprite(char *matName, BaseSprite *sprite);
	bool setMaterialTheora(char *matName, VideoTheoraPlayer *theora);
	bool setMaterialEffect(char *matName, Effect3D *effect, Effect3DParams *params);
	bool removeMaterialEffect(const char *matName);

	bool invalidateDeviceObjects();
	bool restoreDeviceObjects();

protected:
	BaseArray<FrameNode *> _frames;
	BaseArray<XMesh *> _meshes;

	DXMatrix _transformationMatrix;
	DXMatrix _originalMatrix;
	DXMatrix _combinedMatrix;

	DXVector3 _transPos[2];
	DXVector3 _transScale[2];
	DXQuaternion _transRot[2];
	bool _transUsed[2];
	float _lerpValue[2];
};

} // namespace Wintermute

#endif
