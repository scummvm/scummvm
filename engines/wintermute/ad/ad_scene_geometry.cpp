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

#include "common/util.h"

#include "engines/wintermute/ad/ad_block.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_generic.h"
#include "engines/wintermute/ad/ad_geom_ext.h"
#include "engines/wintermute/ad/ad_geom_ext_node.h"
#include "engines/wintermute/ad/ad_path3d.h"
#include "engines/wintermute/ad/ad_path_point3d.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_geometry.h"
#include "engines/wintermute/ad/ad_walkplane.h"
#include "engines/wintermute/ad/ad_waypoint_group3d.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/file/base_file.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/3dcamera.h"
#include "engines/wintermute/base/gfx/3dlight.h"
#include "engines/wintermute/base/gfx/3dloader_3ds.h"
#include "engines/wintermute/base/gfx/3dmesh.h"
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/system/sys_class_registry.h"
#include "engines/wintermute/wintermute.h"

#include "math/glmath.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdSceneGeometry, false)

//////////////////////////////////////////////////////////////////////////
AdSceneGeometry::AdSceneGeometry(BaseGame *gameRef) : BaseObject(gameRef) {
	_activeCamera = _activeLight = -1;
	DXMatrixIdentity(&_viewMatrix);
	_waypointHeight = 10.0f;
	_wptMarker = NULL;

	_PFReady = true;
	_PFTargetPath = NULL;
	_PFMaxTime = 15;
	_PFRerun = false;

	_PFSource = _PFTarget = _PFAlternateTarget = DXVector3(0, 0, 0);
	_PFAlternateDist = FLT_MAX;

	_drawingViewport.setRect(0, 0, 0, 0);

	DXMatrixIdentity(&_lastWorldMat);
	DXMatrixIdentity(&_lastViewMat);
	DXMatrixIdentity(&_lastProjMat);

	_lastOffsetX = _lastOffsetY = 0;
	_lastScrollX = _lastScrollY = 0;

	_lastValuesInitialized = false;
	_maxLightsWarning = false;
}

//////////////////////////////////////////////////////////////////////////
AdSceneGeometry::~AdSceneGeometry() {
	cleanup();
	delete _wptMarker;
}

//////////////////////////////////////////////////////////////////////////
void AdSceneGeometry::cleanup() {
	uint i;

	for (i = 0; i < _planes.size(); i++) {
		delete _planes[i];
	}
	_planes.clear();

	for (i = 0; i < _blocks.size(); i++) {
		delete _blocks[i];
	}
	_blocks.clear();

	for (i = 0; i < _generics.size(); i++) {
		delete _generics[i];
	}
	_generics.clear();

	for (i = 0; i < _waypointGroups.size(); i++) {
		delete _waypointGroups[i];
	}
	_waypointGroups.clear();

	for (i = 0; i < _cameras.size(); i++) {
		if (_gameRef->_renderer3D->_camera == _cameras[i])
			_gameRef->_renderer3D->_camera = nullptr;
		delete _cameras[i];
	}
	_cameras.clear();

	for (i = 0; i < _lights.size(); i++) {
		delete _lights[i];
	}
	_lights.clear();

	_activeCamera = _activeLight = -1;
	DXMatrixIdentity(&_viewMatrix);

	for (i = 0; i < _PFPath.size(); i++) {
		delete _PFPath[i];
	}
	_PFPath.clear();

	_PFTargetPath = NULL;
}

//////////////////////////////////////////////////////////////////////////
AdGeomExt *AdSceneGeometry::getGeometryExtension(char *filename) {
	AdGeomExt *ret = new AdGeomExt(_gameRef);

	bool loadOK = false;
	if (BaseFileManager::getEngineInstance()->openFile(filename) != nullptr) {
		loadOK = ret->loadFile(filename);
	}

	// no ext file found, just use the defaults
	if (!loadOK) {
		ret->addStandardNodes();
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::loadFile(const char *filename) {
	cleanup();

	// load waypoint graphics from resources
	if (!_wptMarker) {
		_wptMarker = new BaseSprite(_gameRef);
		if (_wptMarker) {
			if (!_wptMarker->loadFile("wpt.sprite")) {
				delete _wptMarker;
				_wptMarker = NULL;
			}
		}
	}

	Common::String filenameTmp(filename);

	if (!filenameTmp.hasSuffix(".3ds") && !filenameTmp.hasSuffix(".3DS")) {
		_gameRef->LOG(0, "Error: no suitable loader found for file '%s'", filename);
		return false;
	}

	filenameTmp.replace(filenameTmp.size() - 3, 3, "geometry", 0, 8);
	AdGeomExt *geomExt = getGeometryExtension(filenameTmp.begin());

	// Light3D, AdBlock, AdGeneric and AdWalkplane all inherit from BaseScriptable
	// the latter one is overriding the new operator such that instances are registered
	// in the system class registry
	// for the most part, the subclasses of BaseScriptable override the new operator themselves,
	// but here this is not the case. So these instances are not supposed to be registered
	// and doing so would create faulty savegames. The persistence of them will be handled by AdSceneGeometry
	SystemClassRegistry::getInstance()->_disabled = true;

	BaseArray<Mesh3DS *> meshes;
	BaseArray<Common::String> meshNames;

	if (!load3DSFile(filename, meshes, meshNames, _lights, _cameras, _gameRef)) {
		delete geomExt;
		return false;
	}

	uint i;

	// load meshes
	for (i = 0; i < meshes.size(); i++) {
		AdGeomExtNode *ExtNode = geomExt->matchName(meshNames[i].c_str());

		if (!ExtNode) {
			continue;
		}

		switch (ExtNode->_type) {
		case GEOM_WALKPLANE: {
				AdWalkplane *plane = new AdWalkplane(_gameRef);
				plane->setName(meshNames[i].c_str());
				plane->_mesh = meshes[i];
				plane->_mesh->computeNormals();
				plane->_mesh->fillVertexBuffer(0xFF0000FF); // original 0x700000FF
				plane->_receiveShadows = ExtNode->_receiveShadows;
				_planes.add(plane);
			}
			break;

		case GEOM_BLOCKED: {
				AdBlock *block = new AdBlock(_gameRef);
				block->setName(meshNames[i].c_str());
				block->_mesh = meshes[i];
				block->_mesh->computeNormals();
				block->_mesh->fillVertexBuffer(0xFFFF0000); // original 0x70FF0000
				block->_receiveShadows = ExtNode->_receiveShadows;
				_blocks.add(block);
			}
			break;

		case GEOM_WAYPOINT: {
				Mesh3DS *mesh = meshes[i];
				if (_waypointGroups.size() == 0) {
					_waypointGroups.add(new AdWaypointGroup3D(_gameRef));
				}
				_waypointGroups[0]->addFromMesh(mesh);
				delete mesh;
			}
			break;

		case GEOM_GENERIC: {
				AdGeneric *generic = new AdGeneric(_gameRef);
				generic->setName(meshNames[i].c_str());
				generic->_mesh = meshes[i];
				generic->_mesh->computeNormals();
				generic->_mesh->fillVertexBuffer(0xFF00FF00); // original 0x7000FF00
				generic->_receiveShadows = ExtNode->_receiveShadows;
				_generics.add(generic);
			}
			break;
		}
	}

	SystemClassRegistry::getInstance()->_disabled = false;

	if (_cameras.size() > 0) {
		setActiveCamera(0, -1.0f, -1.0f, -1.0f);
	}
	createLights();

	if (_lights.size() > 0) {
		setActiveLight(0);
	}

	delete geomExt;

	// drop waypoints to the ground
	dropWaypoints();

	if (getFilename() != filename) {
		setFilename(filename);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::dropWaypoints() {
	for (uint i = 0; i < _waypointGroups.size(); i++) {
		for (uint j = 0; j < _waypointGroups[i]->_points.size(); j++) {
			DXVector3 *point = _waypointGroups[i]->_points[j];
			point->_y = getHeightAt(*point) + _waypointHeight;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::setActiveCamera(int camera, float fov, float nearClipPlane, float farClipPlane) {
	if (camera < 0 || static_cast<uint>(camera) >= _cameras.size()) {
		_gameRef->LOG(0, "Warning: Camera %d is out of bounds.", camera);
		return false;
	} else {
		_activeCamera = camera;

		if (fov >= 0.0f) {
			_cameras[camera]->_fov = fov;
		} else {
			_cameras[camera]->_fov = _cameras[camera]->_origFov;
		}

		_cameras[camera]->_nearClipPlane = nearClipPlane;
		_cameras[camera]->_farClipPlane = farClipPlane;

		_cameras[camera]->getViewMatrix(&_viewMatrix);
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::setActiveCamera(const char *camera, float fov, float nearClipPlane, float farClipPlane) {
	for (uint i = 0; i < _cameras.size(); i++) {
		if (scumm_stricmp(_cameras[i]->getName(), camera) == 0)
			return setActiveCamera(i, fov, nearClipPlane, farClipPlane);
	}

	_gameRef->LOG(0, "Warning: Camera '%s' not found.", camera);
	return false;
}

//////////////////////////////////////////////////////////////////////////
Camera3D *AdSceneGeometry::getActiveCamera() {
	if (_activeCamera >= 0 && static_cast<uint>(_activeCamera) < _cameras.size()) {
		return _cameras[_activeCamera];
	} else {
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::setActiveLight(int light) {
	if (light < 0 || static_cast<uint>(light) >= _lights.size()) {
		_gameRef->LOG(0, "Warning: Light %d is out of bounds.", light);
		return false;
	} else {
		_activeLight = light;
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::setActiveLight(char *light) {
	for (uint i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(_lights[i]->getName(), light) == 0) {
			return setActiveLight(i);
		}
	}

	_gameRef->LOG(0, "Warning: Light '%s' not found.", light);
	return false;
}

//////////////////////////////////////////////////////////////////////////
DXMatrix *AdSceneGeometry::getViewMatrix() {
	return &_viewMatrix;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::storeDrawingParams() {
	// store values
	_drawingViewport = _gameRef->_renderer3D->getViewPort();

	_gameRef->_renderer3D->getWorldTransform(&_lastWorldMat);
	_gameRef->_renderer3D->getViewTransform(&_lastViewMat);
	_gameRef->_renderer3D->getProjectionTransform(&_lastProjMat);


	AdScene *scene = ((AdGame *)_gameRef)->_scene;
	if (scene) {
		_lastScrollX = scene->getOffsetLeft();
		_lastScrollY = scene->getOffsetTop();
	} else {
		_lastScrollX = 0;
		_lastScrollY = 0;
	}

	Rect32 rc;
	_gameRef->getCurrentViewportRect(&rc);
	float width = (float)rc.right - (float)rc.left;
	float height = (float)rc.bottom - (float)rc.top;

	// margins
	int mleft = rc.left;
	int mright = _gameRef->_renderer3D->getWidth() - width - rc.left;
	int mtop = rc.top;
	int mbottom = _gameRef->_renderer3D->getHeight() - height - rc.top;

	_lastOffsetX = _gameRef->_offsetX + (mleft - mright) / 2;
	_lastOffsetY = _gameRef->_offsetY + (mtop - mbottom) / 2;

	_lastValuesInitialized = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::render(bool render) {
	// store values
	storeDrawingParams();
	if (render) {
		_gameRef->_renderer3D->renderSceneGeometry(_planes, _blocks, _generics, _lights, getActiveCamera());
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::renderShadowGeometry() {
	storeDrawingParams();

	_gameRef->_renderer3D->renderShadowGeometry(_planes, _blocks, _generics, getActiveCamera());
	return true;
}

//////////////////////////////////////////////////////////////////////////
float AdSceneGeometry::getHeightAt(DXVector3 pos, float tolerance, bool *intFound) {
	float ret = pos._y;
	DXVector3 intersection;
	DXVector3 dir = DXVector3(0, -1, 0);

	pos._y += tolerance;

	bool intFoundTmp = false;

	for (uint32 i = 0; i < _planes.size(); i++) {
		for (int j = 0; j < _planes[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _planes[i]->_mesh->getFace(j);
			float *vp0 = _planes[i]->_mesh->getVertexPosition(triangle[0]);
			float *vp1 = _planes[i]->_mesh->getVertexPosition(triangle[1]);
			float *vp2 = _planes[i]->_mesh->getVertexPosition(triangle[2]);
			DXVector3 v0(vp0[0], vp0[1], vp0[2]);
			DXVector3 v1(vp1[0], vp1[1], vp1[2]);
			DXVector3 v2(vp2[0], vp2[1], vp2[2]);

			if (intersectTriangle(pos, dir, v0, v1, v2, &intersection._x, &intersection._y, &intersection._z)) {
				if (intersection._y > pos._y + tolerance) {
					continue; // only fall down
				}

				if (!intFoundTmp || fabs(ret - pos._y) > fabs(intersection._y - pos._y)) {
					ret = intersection._y;
				}

				intFoundTmp = true;
			}
		}
	}

	if (intFound) {
		*intFound = intFoundTmp;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::directPathExists(DXVector3 *p1, DXVector3 *p2) {
	// test walkplanes
	for (uint i = 0; i < _planes.size(); i++) {
		for (int j = 0; j < _planes[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _planes[i]->_mesh->getFace(j);
			float *vp0 = _planes[i]->_mesh->getVertexPosition(triangle[0]);
			float *vp1 = _planes[i]->_mesh->getVertexPosition(triangle[1]);
			float *vp2 = _planes[i]->_mesh->getVertexPosition(triangle[2]);
			DXVector3 v0(vp0[0], vp0[1], vp0[2]);
			DXVector3 v1(vp1[0], vp1[1], vp1[2]);
			DXVector3 v2(vp2[0], vp2[1], vp2[2]);
			DXVector3 intersection;
			float dist;

			if (pickGetIntersect(*p1, *p2, v0, v1, v2, &intersection, &dist)) {
				if (intersectTriangle(*p1, *p1 - *p2, v0, v1, v2, &intersection._x, &intersection._y, &intersection._z)) {
					return false;
				}

				if (intersectTriangle(*p2, *p2 - *p1, v0, v1, v2, &intersection._x, &intersection._y, &intersection._z)) {
					return false;
				}
			}
		}
	}

	// test blocks
	for (uint i = 0; i < _blocks.size(); i++) {
		if (!_blocks[i]->_active) {
			continue;
		}

		for (int j = 0; j < _blocks[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _blocks[i]->_mesh->getFace(j);
			float *vp0 = _blocks[i]->_mesh->getVertexPosition(triangle[0]);
			float *vp1 = _blocks[i]->_mesh->getVertexPosition(triangle[1]);
			float *vp2 = _blocks[i]->_mesh->getVertexPosition(triangle[2]);
			DXVector3 v0(vp0[0], vp0[1], vp0[2]);
			DXVector3 v1(vp1[0], vp1[1], vp1[2]);
			DXVector3 v2(vp2[0], vp2[1], vp2[2]);
			DXVector3 intersection;
			float dist;

			if (pickGetIntersect(*p1, *p2, v0, v1, v2, &intersection, &dist)) {
				if (intersectTriangle(*p1, *p1 - *p2, v0, v1, v2,
				                           &intersection._x, &intersection._y, &intersection._z)) {
					return false;
				}

				if (intersectTriangle(*p2, *p2 - *p1, v0, v1, v2,
				                           &intersection._x, &intersection._y, &intersection._z)) {
					return false;
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
DXVector3 AdSceneGeometry::getBlockIntersection(DXVector3 *p1, DXVector3 *p2) {
	// test blocks
	for (uint i = 0; i < _blocks.size(); i++) {
		if (!_blocks[i]->_active) {
			continue;
		}

		for (int j = 0; j < _blocks[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _blocks[i]->_mesh->getFace(j);
			float *vp0 = _blocks[i]->_mesh->getVertexPosition(triangle[0]);
			float *vp1 = _blocks[i]->_mesh->getVertexPosition(triangle[1]);
			float *vp2 = _blocks[i]->_mesh->getVertexPosition(triangle[2]);
			DXVector3 v0(vp0[0], vp0[1], vp0[2]);
			DXVector3 v1(vp1[0], vp1[1], vp1[2]);
			DXVector3 v2(vp2[0], vp2[1], vp2[2]);
			DXVector3 intersection;
			float dist;

			if (pickGetIntersect(*p1, *p2, v0, v1, v2, &intersection, &dist)) {
				if (intersectTriangle(*p1, *p1 - *p2, v0, v1, v2, &intersection._x, &intersection._y, &intersection._z)) {
					return intersection;
				}

				if (intersectTriangle(*p2, *p2 - *p1, v0, v1, v2, &intersection._x, &intersection._y, &intersection._z)) {
					return intersection;
				}
			}
		}
	}

	return DXVector3(0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::convert2Dto3DTolerant(int x, int y, DXVector3 *pos) {
	bool ret = convert2Dto3D(x, y, pos);
	if (ret) {
		return ret;
	}

	int lenLeft = 0;
	int lenRight = 0;
	int lenDown = 0;
	int lenUp = 0;

	int i;

	// left
	for (i = 0; i < 1000; i += 10) {
		if (convert2Dto3D(x - i, y, pos)) {
			lenLeft = i;
			break;
		}
	}

	// right
	for (i = 0; i < 1000; i += 10) {
		if (convert2Dto3D(x + i, y, pos)) {
			lenRight = i;
			break;
		}
	}

	// up
	for (i = 0; i < 1000; i += 10) {
		if (convert2Dto3D(x, y - i, pos)) {
			lenUp = i;
			break;
		}
	}

	// down
	for (i = 0; i < 1000; i += 10) {
		if (convert2Dto3D(x, y + i, pos)) {
			lenDown = i;
			break;
		}
	}

	if (!lenLeft && !lenRight && !lenUp && !lenDown) {
		return false;
	}

	int offsetX = INT_MAX_VALUE;
	int offsetY = INT_MAX_VALUE;

	if (lenLeft || lenRight) {
		if (lenRight) {
			if (lenLeft && lenLeft < lenRight) {
				offsetX = -lenLeft;
			} else {
				offsetX = lenRight;
			}
		} else {
			offsetX = -lenLeft;
		}
	}

	if (lenUp || lenDown) {
		if (lenDown) {
			if (lenUp && lenUp < lenDown)
				offsetY = -lenUp;
			else
				offsetY = lenDown;
		} else
			offsetY = -lenUp;
	}

	if (abs(offsetX) < abs(offsetY)) {
		x += offsetX;
	} else {
		y += offsetY;
	}

	return convert2Dto3D(x, y, pos);
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::convert2Dto3D(int x, int y, DXVector3 *pos) {
	if (!_lastValuesInitialized) {
		_drawingViewport = _gameRef->_renderer3D->getViewPort();
		_gameRef->_renderer3D->getProjectionTransform(&_lastProjMat);
	}

	float resWidth, resHeight;
	float layerWidth, layerHeight;
	float modWidth, modHeight;
	bool customViewport;
	_gameRef->_renderer3D->getProjectionParams(&resWidth, &resHeight, &layerWidth, &layerHeight, &modWidth, &modHeight, &customViewport);

	// modify coordinates according to viewport settings
	int mleft = _drawingViewport.left;
	int mright = resWidth - _drawingViewport.width() - _drawingViewport.left;
	int mtop = _drawingViewport.top;
	int mbottom = resHeight - _drawingViewport.height() - _drawingViewport.top;

	x -= (mleft + mright) / 2 + modWidth;
	y -= (mtop + mbottom) / 2 + modHeight;


	DXVector3 vPickRayDir;
	DXVector3 vPickRayOrig;

	// Compute the vector of the pick ray in screen space
	DXVector3 vec;
	vec._x =  (((2.0f * x) / _drawingViewport.width()) - 1) / _lastProjMat.matrix._11;
	vec._y = -(((2.0f * y) / _drawingViewport.height()) - 1) / _lastProjMat.matrix._22;
	vec._z =  -1.0f;

	// Get the inverse view matrix
	DXMatrix m;
	DXMatrixInverse(&m, nullptr, &_lastViewMat);

	// Transform the screen space pick ray into 3D space
	vPickRayDir._x  = vec._x * m.matrix._11 + vec._y * m.matrix._21 + vec._z * m.matrix._31;
	vPickRayDir._y  = vec._x * m.matrix._12 + vec._y * m.matrix._22 + vec._z * m.matrix._32;
	vPickRayDir._z  = vec._x * m.matrix._13 + vec._y * m.matrix._23 + vec._z * m.matrix._33;
	vPickRayOrig._x = m.matrix._41;
	vPickRayOrig._y = m.matrix._42;
	vPickRayOrig._z = m.matrix._43;


	bool intFound = false;
	float minDist = FLT_MAX;
	DXVector3 intersection, ray;
	for (uint32 i = 0; i < _planes.size(); i++) {
		for (int j = 0; j < _planes[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _planes[i]->_mesh->getFace(j);
			float *vp0 = _planes[i]->_mesh->getVertexPosition(triangle[0]);
			float *vp1 = _planes[i]->_mesh->getVertexPosition(triangle[1]);
			float *vp2 = _planes[i]->_mesh->getVertexPosition(triangle[2]);
			DXVector3 v0(vp0[0], vp0[1], vp0[2]);
			DXVector3 v1(vp1[0], vp1[1], vp1[2]);
			DXVector3 v2(vp2[0], vp2[1], vp2[2]);

			if (intersectTriangle(vPickRayOrig, vPickRayDir, v0, v1, v2, &intersection._x, &intersection._y, &intersection._z)) {
				ray = intersection - vPickRayOrig;
				float dist = DXVec3Length(&ray);

				if (dist < minDist) {
					*pos = intersection;
					minDist = dist;
				}

				intFound = true;
			}
		}
	}

	return intFound;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::getPath(DXVector3 source, DXVector3 target, AdPath3D *path, bool rerun) {
	if (!_PFReady) {
		return false;
	} else {
		source._y = getHeightAt(source, _waypointHeight) + _waypointHeight;
		target._y = getHeightAt(target, _waypointHeight) + _waypointHeight;

		_PFReady = false;
		_PFSource = source;
		_PFTarget = target;
		_PFTargetPath = path;
		_PFAlternateTarget = DXVector3(0, 0, 0);
		_PFAlternateDist = FLT_MAX;

		_PFTargetPath->reset();
		_PFTargetPath->setReady(false);
		_PFRerun = rerun;

		// prepare working path
		uint i, j;

		for (i = 0; i < _PFPath.size(); i++) {
			delete _PFPath[i];
		}

		_PFPath.clear();

		// first point
		_PFPath.add(new AdPathPoint3D(source, 0));

		// last point
		_PFPath.add(new AdPathPoint3D(target, FLT_MAX));

		// add all active waypoints
		for (i = 0; i < _waypointGroups.size(); i++) {
			if (_waypointGroups[i]->_active) {
				for (j = 0; j < _waypointGroups[i]->_points.size(); j++) {
					_PFPath.add(new AdPathPoint3D(*_waypointGroups[i]->_points[j], FLT_MAX));
				}
			}
		}

		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
void AdSceneGeometry::pathFinderStep() {
	uint i;

	// get lowest unmarked
	float lowest_dist = FLT_MAX;
	AdPathPoint3D *lowest_pt = NULL;

	for (i = 0; i < _PFPath.size(); i++) {
		if (!_PFPath[i]->_marked && _PFPath[i]->_distance < lowest_dist) {
			lowest_dist = _PFPath[i]->_distance;
			lowest_pt = _PFPath[i];
		}
	}

	if (lowest_pt == NULL) { // no path -> terminate PathFinder
		_PFReady = true;

		if (!_PFRerun) {
			if (_PFAlternateTarget != DXVector3(0, 0, 0)) {
				getPath(_PFSource, _PFAlternateTarget, _PFTargetPath, true);
			} else {
				_PFTargetPath->setReady(true);
			}
		} else {
			_PFTargetPath->setReady(true);
		}

		return;
	}

	lowest_pt->_marked = true;

	// target point marked, generate path and terminate
	if (lowest_pt->_pos == _PFTarget) {
		while (lowest_pt != NULL) {
			_PFTargetPath->_points.insert_at(0, new DXVector3(lowest_pt->_pos));
			lowest_pt = lowest_pt->_origin;
		}
		// remove current position
		if (_PFTargetPath->_points.size() > 0) {
			delete _PFTargetPath->_points[0];
			_PFTargetPath->_points.remove_at(0);
		}

		_PFReady = true;
		_PFTargetPath->setReady(true);
		return;
	}

	// otherwise keep on searching
	for (i = 0; i < _PFPath.size(); i++) {
		if (!_PFPath[i]->_marked) {
			float dist = getPointsDist(lowest_pt->_pos, _PFPath[i]->_pos);
			if (dist >= 0 && lowest_pt->_distance + dist < _PFPath[i]->_distance) {
				_PFPath[i]->_distance = lowest_pt->_distance + dist;
				_PFPath[i]->_origin = lowest_pt;
			} else {
				if (!_PFRerun && _PFPath[i]->_pos == _PFTarget) {
					DXVector3 line = _PFPath[i]->_pos - lowest_pt->_pos;
					float len = DXVec3Length(&line);

					if (len < _PFAlternateDist) {
						_PFAlternateDist = len;
						_PFAlternateTarget = getBlockIntersection(&lowest_pt->_pos, &_PFPath[i]->_pos);

						DXVector3 dir = _PFAlternateTarget - lowest_pt->_pos;
						DXVec3Normalize(&dir, &dir);
						_PFAlternateTarget -= dir * 30;
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
float AdSceneGeometry::getPointsDist(DXVector3 p1, DXVector3 p2) {
	if (!directPathExists(&p1, &p2)) {
		return -1;
	}

	DXVector3 vect = p2 - p1;
	return DXVec3Length(&vect);
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::initLoop() {
	uint32 start = _gameRef->_currentTime;
	while (!_PFReady && _gameRef->_currentTime - start <= _PFMaxTime) {
		pathFinderStep();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::createLights() {
	// disable all lights
	for (int i = 0; i < _gameRef->_renderer3D->getMaxActiveLights(); i++) {
		_gameRef->_renderer3D->disableLight(i);
	}

	int lightCount = MIN(static_cast<int>(_lights.size()), _gameRef->_renderer3D->getMaxActiveLights());

	for (int i = 0; i < lightCount; i++) {
		_lights[i]->setLight(i);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool compareLights(const Light3D *light1, const Light3D *light2) {
	return light1->_distance < light2->_distance;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::enableLights(DXVector3 point, BaseArray<char *> &ignoreLights) {
	const int maxLightCount = 100;

	int activeLightCount = 0;
	for (uint i = 0; i < _lights.size(); i++) {
		_lights[i]->_isAvailable = false;
		if (_lights[i]->_active) {
			activeLightCount++;
		}
	}

	if (activeLightCount <= _gameRef->_renderer3D->getMaxActiveLights()) {
		for (uint i = 0; i < _lights.size(); i++) {
			_lights[i]->_isAvailable = true;
		}
	} else {
		if (!_maxLightsWarning) {
			_gameRef->LOG(0, "Warning: Using more lights than the hardware supports (%d)", _gameRef->_renderer3D->getMaxActiveLights());
			_maxLightsWarning = true;
		}

		Common::Array<Light3D *> activeLights;

		// compute distance to point
		for (uint i = 0; i < _lights.size(); i++) {
			if (!_lights[i]->_active) {
				continue;
			}

			DXVector3 dif;

			if (_lights[i]->_isSpotlight) {
				DXVector3 dir = _lights[i]->_target - _lights[i]->_position;
				dif = (_lights[i]->_position + dir * 0.75f) - point;
			} else {
				dif = _lights[i]->_position - point;
			}

			_lights[i]->_distance = fabs(DXVec3Length(&dif));

			activeLights.push_back(_lights[i]);
		}

		// sort by distance
		if (activeLights.size() > 0) {
			Common::sort(activeLights.begin(), activeLights.end(), compareLights);

			for (uint i = 0; i < activeLights.size(); i++) {
				activeLights[i]->_isAvailable = static_cast<int>(i) < _gameRef->_renderer3D->getMaxActiveLights();
			}
		}
	}

	// light all available lights
	for (int i = 0; i < maxLightCount; i++) {
		_gameRef->_renderer3D->disableLight(i);
	}

	activeLightCount = 0;

	for (uint i = 0; i < _lights.size(); i++) {
		if (activeLightCount >= _gameRef->_renderer3D->getMaxActiveLights()) {
			break;
		}

		if (ignoreLights.size()) {
			bool ignore = false;

			for (uint j = 0; j < ignoreLights.size(); j++) {
				if (scumm_stricmp(_lights[i]->getName(), ignoreLights[j]) == 0) {
					ignore = true;
					break;
				}
			}

			if (ignore) {
				continue; // skip this light
			}
		}

		if (_lights[i]->_isAvailable) {
			if (_lights[i]->_active) {
				_gameRef->_renderer3D->enableLight(i);
				++activeLightCount;
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::correctTargetPoint(const DXVector3 &source, DXVector3 *target) {
	// the source parameter is not even used in wme3d
	int i;
	int MaxLen = 1000;
	int Step = 10;
	DXVector3 newTarget;

	int lenLeft = 0;
	int lenRight = 0;
	int lenUp = 0;
	int lenDown = 0;

	// left
	newTarget = *target;
	for (i = 1; i <= MaxLen; i += Step) {
		newTarget._x -= i;
		if (!directPathExists(target, &newTarget)) {
			lenLeft = i;
			break;
		}
	}

	// right
	newTarget = *target;
	for (i = 1; i <= MaxLen; i += Step) {
		newTarget._x += i;
		if (!directPathExists(target, &newTarget)) {
			lenRight = i;
			break;
		}
	}

	// up
	newTarget = *target;
	for (i = 1; i <= MaxLen; i += Step) {
		newTarget._z -= i;
		if (!directPathExists(target, &newTarget)) {
			lenUp = i;
			break;
		}
	}

	// down
	newTarget = *target;
	for (i = 1; i <= MaxLen; i += Step) {
		newTarget._z += i;
		if (!directPathExists(target, &newTarget)) {
			lenDown = i;
			break;
		}
	}

	if (!lenLeft && !lenRight && !lenUp && !lenDown) {
		return true;
	}

	int offsetX = INT_MAX_VALUE;
	int offsetZ = INT_MAX_VALUE;

	if (lenLeft || lenRight) {
		if (lenRight) {
			if (lenLeft && lenLeft < lenRight) {
				offsetX = -lenLeft;
			} else {
				offsetX = lenRight;
			}
		} else {
			offsetX = -lenLeft;
		}
	}

	if (lenUp || lenDown) {
		if (lenDown) {
			if (lenUp && lenUp < lenDown) {
				offsetZ = -lenUp;
			} else {
				offsetZ = lenDown;
			}
		} else {
			offsetZ = -lenUp;
		}
	}

	if (abs(offsetX) < abs(offsetZ)) {
		target->_x += offsetX;
	} else {
		target->_z += offsetZ;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::enableNode(const char *nodeName, bool enable) {
	bool ret = false;

	uint i;
	for (i = 0; i < _blocks.size(); i++) {
		if (scumm_stricmp(nodeName, _blocks[i]->getName()) == 0) {
			_blocks[i]->_active = enable;
			ret = true;
		}
	}

	for (i = 0; i < _planes.size(); i++) {
		if (scumm_stricmp(nodeName, _planes[i]->getName()) == 0) {
			_planes[i]->_active = enable;
			ret = true;
		}
	}

	for (i = 0; i < _generics.size(); i++) {
		if (scumm_stricmp(nodeName, _generics[i]->getName()) == 0) {
			_generics[i]->_active = enable;
			ret = true;
		}
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::isNodeEnabled(const char *nodeName) {
	for (uint i = 0; i < _blocks.size(); i++) {
		if (scumm_stricmp(nodeName, _blocks[i]->getName()) == 0) {
			return _blocks[i]->_active;
		}
	}
	for (uint i = 0; i < _planes.size(); i++) {
		if (scumm_stricmp(nodeName, _planes[i]->getName()) == 0) {
			return _planes[i]->_active;
		}
	}

	for (uint i = 0; i < _generics.size(); i++) {
		if (scumm_stricmp(nodeName, _generics[i]->getName()) == 0) {
			return _generics[i]->_active;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::enableLight(const char *lightName, bool enable) {
	bool ret = false;

	uint i;
	for (i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(lightName, _lights[i]->getName()) == 0) {
			_lights[i]->_active = enable;
			ret = true;
		}
	}
	createLights();

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::isLightEnabled(const char *lightName) {
	for (uint i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(lightName, _lights[i]->getName()) == 0) {
			return _lights[i]->_active;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::setLightColor(const char *lightName, uint32 color) {
	bool ret = false;

	uint i;
	for (i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(lightName, _lights[i]->getName()) == 0) {
			_lights[i]->_diffuseColor = color;
			ret = true;
		}
	}
	createLights();

	return ret;
}

//////////////////////////////////////////////////////////////////////////
uint32 AdSceneGeometry::getLightColor(const char *lightName) {
	for (uint i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(lightName, _lights[i]->getName()) == 0) {
			return _lights[i]->_diffuseColor;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
DXVector3 AdSceneGeometry::getLightPos(const char *lightName) {
	for (uint i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(lightName, _lights[i]->getName()) == 0) {
			return _lights[i]->_position;
		}
	}
	return DXVector3(0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transferFloat(TMEMBER(_waypointHeight));
	persistMgr->transferPtr(TMEMBER(_wptMarker));

	if (!persistMgr->getIsSaving()) {
		//m_WptMarker = NULL;
		loadFile(getFilename());
		_lastValuesInitialized = false;
	}

	persistMgr->transferSint32(TMEMBER(_activeCamera));
	persistMgr->transferSint32(TMEMBER(_activeLight));
	persistMgr->transferMatrix4(TMEMBER(_viewMatrix));

	_PFPath.persist(persistMgr);
	persistMgr->transferBool(TMEMBER(_PFReady));
	persistMgr->transferVector3d(TMEMBER(_PFSource));
	persistMgr->transferVector3d(TMEMBER(_PFTarget));
	persistMgr->transferVector3d(TMEMBER(_PFAlternateTarget));
	persistMgr->transferPtr(TMEMBER(_PFTargetPath));
	persistMgr->transferUint32(TMEMBER(_PFMaxTime));
	persistMgr->transferBool(TMEMBER(_PFRerun));

	// now save/load light/blocks/walkplanes/generic node states by name
	int i;

	//////////////////////////////////////////////////////////////////////////
	int32 numLights = _lights.size();
	persistMgr->transferSint32(TMEMBER(numLights));
	for (i = 0; i < numLights; i++) {
		if (persistMgr->getIsSaving()) {
			persistMgr->transferCharPtr(TMEMBER(_lights[i]->_name));
			_lights[i]->persist(persistMgr);
		} else {
			char *name = nullptr;
			persistMgr->transferCharPtr(TMEMBER(name));
			bool found = false;

			for (uint j = 0; j < _lights.size(); j++) {
				if (scumm_stricmp(name, _lights[j]->getName()) == 0) {
					_lights[j]->persist(persistMgr);
					found = true;
					break;
				}
			}

			if (!found) {
				Light3D *light = new Light3D(_gameRef);
				light->persist(persistMgr);
				delete light;
			}

			if (name) {
				delete[] name;
				name = nullptr;
			}
		}
	}
	createLights();

	//////////////////////////////////////////////////////////////////////////
	int32 numBlocks = _blocks.size();
	persistMgr->transferSint32(TMEMBER(numBlocks));
	for (i = 0; i < numBlocks; i++) {
		if (persistMgr->getIsSaving()) {
			persistMgr->transferCharPtr(TMEMBER(_blocks[i]->_name));
			_blocks[i]->persist(persistMgr);
		} else {
			char *name = nullptr;
			persistMgr->transferCharPtr(TMEMBER(name));
			bool found = false;
			for (uint j = 0; j < _blocks.size(); j++) {
				if (scumm_stricmp(name, _blocks[j]->getName()) == 0) {
					_blocks[j]->persist(persistMgr);
					found = true;
					break;
				}
			}
			if (!found) {
				AdBlock *block = new AdBlock(_gameRef);
				block->persist(persistMgr);
				delete block;
			}

			if (name) {
				delete[] name;
				name = nullptr;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int32 numPlanes = _planes.size();
	persistMgr->transferSint32(TMEMBER(numPlanes));
	for (i = 0; i < numPlanes; i++) {
		if (persistMgr->getIsSaving()) {
			persistMgr->transferCharPtr(TMEMBER(_planes[i]->_name));
			_planes[i]->persist(persistMgr);
		} else {
			char *name = nullptr;
			persistMgr->transferCharPtr(TMEMBER(name));
			bool found = false;
			for (uint j = 0; j < _planes.size(); j++) {
				if (scumm_stricmp(name, _planes[j]->getName()) == 0) {
					_planes[j]->persist(persistMgr);
					found = true;
					break;
				}
			}
			if (!found) {
				AdWalkplane *plane = new AdWalkplane(_gameRef);
				plane->persist(persistMgr);
				delete plane;
			}

			if (name) {
				delete[] name;
				name = nullptr;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int32 numGenerics = _generics.size();
	persistMgr->transferSint32(TMEMBER(numGenerics));
	for (i = 0; i < numGenerics; i++) {
		if (persistMgr->getIsSaving()) {
			persistMgr->transferCharPtr(TMEMBER(_generics[i]->_name));
			_generics[i]->persist(persistMgr);
		} else {
			char *name = nullptr;
			persistMgr->transferCharPtr(TMEMBER(name));
			bool found = false;
			for (uint j = 0; j < _generics.size(); j++) {
				if (scumm_stricmp(name, _generics[j]->getName()) == 0) {
					_generics[j]->persist(persistMgr);
					found = true;
					break;
				}
			}
			if (!found) {
				AdGeneric *generic = new AdGeneric(_gameRef);
				generic->persist(persistMgr);
				delete generic;
			}

			if (name) {
				delete[] name;
				name = nullptr;
			}
		}
	}

	if (!persistMgr->getIsSaving()) {
		_maxLightsWarning = false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::convert3Dto2D(DXVector3 *pos, int32 *x, int32 *y) {
	DXMatrix worldMat;
	DXMatrixIdentity(&worldMat);

	DXVector3 vect2D;
	DXVec3Project(&vect2D, pos, &_drawingViewport, &_lastProjMat, &_lastViewMat, &worldMat);
	*x = vect2D._x + _lastScrollX;
	*y = vect2D._y + _lastScrollY;

	return true;
}

} // namespace Wintermute
