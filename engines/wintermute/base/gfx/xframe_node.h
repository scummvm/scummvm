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
class BaseSprite;
class XFileLexer;

class FrameNode : public BaseNamedObject {
public:
	FrameNode(BaseGame *inGame);
	virtual ~FrameNode();

	bool updateMatrices(Math::Matrix4 &parentMat);
	bool updateMeshes();
	bool resetMatrices();
	bool render(XModel *model);
	bool renderFlatShadowModel();
	bool updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth);

	bool loadFromX(const Common::String &filename, XFileLexer &lexer, XModel *model, Common::Array<MaterialReference> &materialReferences);
	bool loadFromXAsRoot(const Common::String &filename, XFileLexer &lexer, XModel *model, Common::Array<MaterialReference> &materialReferences);
	bool findBones(FrameNode *rootFrame);
	FrameNode *findFrame(const char *frameName);
	Math::Matrix4 *getCombinedMatrix();
	Math::Matrix4 *getOriginalMatrix();
	void setTransformationMatrix(Math::Matrix4 *mat);
	bool pickPoly(Math::Vector3d *pickRayOrig, Math::Vector3d *pickRayDir);
	bool getBoundingBox(Math::Vector3d *boxStart, Math::Vector3d *boxEnd);

	void setTransformation(int slot, Math::Vector3d pos, Math::Vector3d scale, Math::Quaternion rot, float lerpValue);

	bool hasChildren();

	bool setMaterialSprite(char *matName, BaseSprite *sprite);
	bool setMaterialTheora(char *matName, VideoTheoraPlayer *theora);

	bool invalidateDeviceObjects();
	bool restoreDeviceObjects();

protected:
	BaseArray<FrameNode *> _frames;
	BaseArray<XMesh *> _meshes;

	Math::Matrix4 _transformationMatrix;
	Math::Matrix4 _originalMatrix;
	Math::Matrix4 _combinedMatrix;

	Math::Vector3d _transPos[2];
	Math::Vector3d _transScale[2];
	Math::Quaternion _transRot[2];
	bool _transUsed[2];
	float _lerpValue[2];
};

} // namespace Wintermute

#endif
