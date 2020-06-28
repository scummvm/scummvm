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

#include "common/math.h"
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
#include "engines/wintermute/base/gfx/opengl/camera3d.h"
#include "engines/wintermute/base/gfx/opengl/light3d.h"
#include "engines/wintermute/base/gfx/opengl/loader3ds.h"
#include "engines/wintermute/base/gfx/opengl/mesh3ds.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/system/sys_class_registry.h"
#include "engines/wintermute/wintermute.h"
#include "math/glmath.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdSceneGeometry, false)

//////////////////////////////////////////////////////////////////////////
AdSceneGeometry::AdSceneGeometry(BaseGame *gameRef) : BaseObject(gameRef) {
	_activeCamera = _activeLight = -1;
	_viewMatrix.setToIdentity();
	//m_WaypointHeight = 5.0f;
	//m_WaypointHeight = 1.0f;
	_waypointHeight = 10.0f;
	_wptMarker = NULL;

	_PFReady = true;
	_PFTargetPath = NULL;
	_PFMaxTime = 15;
	_PFRerun = false;

	_PFSource = _PFTarget = _PFAlternateTarget = Math::Vector3d(0, 0, 0);
	_PFAlternateDist = FLT_MAX;

	_drawingViewport.setRect(0, 0, 0, 0);

	_lastWorldMat.setToIdentity();
	_lastViewMat.setToIdentity();
	_lastProjMat.setToIdentity();

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
		//		CBRenderD3D* _renderer = (CBRenderD3D*)_gameRef->_renderer;
		//		if(m_Renderer->m_Camera == _cameras[i]) m_Renderer->m_Camera = NULL;

		delete _cameras[i];
	}
	_cameras.clear();

	for (i = 0; i < _lights.size(); i++) {
		delete _lights[i];
	}
	_lights.clear();

	_activeCamera = _activeLight = -1;
	_viewMatrix.setToIdentity();

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

	if (!filenameTmp.hasSuffix(".3ds")) {
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

	if (!load3DSFile(filename, meshes, meshNames, _lights, _cameras)) {
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
			// TODO: These constants are endianness dependent
			plane->_mesh->fillVertexBuffer(0xFFFF0000);
			plane->_receiveShadows = ExtNode->_receiveShadows;
			_planes.add(plane);
			} break;

		case GEOM_BLOCKED: {
			AdBlock *block = new AdBlock(_gameRef);
			block->setName(meshNames[i].c_str());
			block->_mesh = meshes[i];
			block->_mesh->computeNormals();
			block->_mesh->fillVertexBuffer(0xFF0000FF);
			block->_receiveShadows = ExtNode->_receiveShadows;
			_blocks.add(block);
			} break;

		case GEOM_WAYPOINT: {
			Mesh3DS *mesh = meshes[i];
			// TODO: groups
			if (_waypointGroups.size() == 0) {
				_waypointGroups.add(new AdWaypointGroup3D(_gameRef));
			}
			_waypointGroups[0]->addFromMesh(mesh);
			delete mesh;
			} break;

		case GEOM_GENERIC: {
			AdGeneric *generic = new AdGeneric(_gameRef);
			generic->setName(meshNames[i].c_str());
			generic->_mesh = meshes[i];
			generic->_mesh->computeNormals();
			generic->_mesh->fillVertexBuffer(0xFF00FF00);
			generic->_receiveShadows = ExtNode->_receiveShadows;
			_generics.add(generic);
			} break;
		}
	}

	SystemClassRegistry::getInstance()->_disabled = false;

	if (_cameras.size() > 0) {
		setActiveCamera(0, -1.0f, -1.0f, -1.0f);
	}
	createLights();

	if (_lights.size() > 0) {
		uint activeLight = 0;
		setActiveLight(activeLight);
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
			Math::Vector3d *point = _waypointGroups[i]->_points[j];
			point->y() = getHeightAt(*point) + _waypointHeight;
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
			_cameras[camera]->_fov = _cameras[camera]->_originalFOV;
		}

		_cameras[camera]->_nearClipPlane = nearClipPlane;
		_cameras[camera]->_farClipPlane = farClipPlane;

		_cameras[camera]->getViewMatrix(&_viewMatrix);
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::setActiveCamera(char *camera, float fov, float nearClipPlane, float farClipPlane) {
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
Math::Matrix4 *AdSceneGeometry::getViewMatrix() {
	return &_viewMatrix;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::storeDrawingParams() {
	// implement this later
	//	CBRenderD3D* m_Renderer = (CBRenderD3D*)_gameRef->m_Renderer;

	//  // store values
	//	m_Renderer->m_Device->GetViewport(&m_DrawingViewport);

	//	m_Renderer->m_Device->GetTransform(D3DTS_WORLD, &m_LastWorldMat);
	//	m_Renderer->m_Device->GetTransform(D3DTS_VIEW, &m_LastViewMat);
	//	m_Renderer->m_Device->GetTransform(D3DTS_PROJECTION, &m_LastProjMat);

	warning("AdSceneGeometry::storeDrawingParams not yet implemented");

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
//	float width = (float)rc.right - (float)rc.left;
//	float height = (float)rc.bottom - (float)rc.top;

	// margins
	//	int mleft = rc.left;
	//	int mright = m_Renderer->m_Width - Width - rc.left;
	//	int mtop = rc.top;
	//	int mbottom = m_Renderer->m_Height - Height - rc.top;

	//	m_LastOffsetX = _gameRef->_offsetX + (mleft - mright)/2;
	//	m_LastOffsetY = _gameRef->_offsetY + (mtop - mbottom)/2;

	_lastValuesInitialized = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::render(bool render) {
	//	store values
	//	StoreDrawingParams();
	if (!render) {
		return true;
	}

	_gameRef->_renderer3D->resetModelViewTransform();
	_gameRef->_renderer3D->setup3D(getActiveCamera(), true);

	// factor this out later

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//	m_Renderer->m_Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	//	m_Renderer->m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	//	m_Renderer->m_Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	//	m_Renderer->m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	//	m_Renderer->m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	//	m_Renderer->m_Device->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
	//	m_Renderer->m_Device->SetTexture(0, NULL);

	uint i;

	// render walk planes
	for (i = 0; i < _planes.size(); i++) {
		if (!_planes[i]->_active) {
			//			continue;
		}

		_planes[i]->_mesh->render();

		//m_Renderer->m_NumPolygons += _planes[i]->m_Mesh->m_NumFaces;
	}

	// render blocks
	for (i = 0; i < _blocks.size(); i++) {
		if (!_blocks[i]->_active) {
			//			continue;
		}

		_blocks[i]->_mesh->render();

		//		m_Renderer->m_NumPolygons += _blocks[i]->m_Mesh->m_NumFaces;
	}

	// render generic objects
	for (i = 0; i < _generics.size(); i++) {
		if (!_generics[i]->_active) {
			//			continue;
		}

		_generics[i]->_mesh->render();

		//		m_Renderer->m_NumPolygons += _generics[i]->m_Mesh->m_NumFaces;
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//	m_Renderer->m_Device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	//	// render waypoints
	//	if(m_WptMarker)
	//	{
	//		Math::Matrix4 viewMat, projMat, worldMat;
	//		Math::Vector3d vec2d(0,0,0);
	//		m_Renderer->m_Device->GetTransform(D3DTS_VIEW, &viewMat);
	//		m_Renderer->m_Device->GetTransform(D3DTS_PROJECTION, &projMat);
	//		Math::Matrix4Identity(&worldMat);
	//		D3DVIEWPORT vport;
	//		m_Renderer->m_Device->GetViewport(&vport);

	//		m_Renderer->Setup2D();

	//		CAdScene* Scene = ((CAd_gameRef*)_gameRef)->m_Scene;

	//		for(i=0; i<_waypointGroups.size(); i++)
	//		{
	//			for(int j=0; j<_waypointGroups[i]->m_Points.size(); j++)
	//			{
	//				Math::Vector3d *vect = _waypointGroups[i]->m_Points[j];
	//				D3DXVec3Project(&vec2d, _waypointGroups[i]->m_Points[j], &vport, &projMat, &viewMat, &worldMat);
	//				m_WptMarker->Display(vec2d.x + Scene->GetOffsetLeft() - m_Renderer->m_DrawOffsetX, vec2d.y + Scene->GetOffsetTop() - m_Renderer->m_DrawOffsetY);
	//			}
	//		}
	//	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::renderShadowGeometry() {
	storeDrawingParams();

	warning("AdSceneGeometry::renderShadowGeometry not yet implemented");

	// implement this later

	//	CBRenderD3D* m_Renderer = (CBRenderD3D*)_gameRef->m_Renderer;

	//	// render the geometry
	//	Math::Matrix4 matIdentity;
	//	matIdentity.setToIdentity();

	//	if(m_ActiveCamera>=0 && m_ActiveCamera<_cameras.size())
	//		m_Renderer->Setup3D(_cameras[m_ActiveCamera]);

	//	m_Renderer->m_Device->SetTransform(D3DTS_WORLD, &matIdentity);

	//	// disable color write
	//	m_Renderer->SetSpriteBlendMode(BLEND_UNKNOWN);
	//	m_Renderer->m_Device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ZERO);
	//	m_Renderer->m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//	m_Renderer->m_Device->SetTransform(D3DTS_WORLD, &matIdentity);
	//	C3DUtils::SetFixedVertexShader(m_Renderer->m_Device, D3DFVF_MODELVERTEXCOLOR);

	//	// no texture
	//	m_Renderer->m_LastTexture = NULL;
	//	m_Renderer->m_Device->SetTexture(0, NULL);

	//	m_Renderer->m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//	// render blocks
	//	for(int i=0; i<_blocks.size(); i++)
	//	{
	//		if(!_blocks[i]->m_Active) continue;
	//		if(!_blocks[i]->m_Mesh->m_VB) continue;
	//		if(!_blocks[i]->m_ReceiveShadows) continue;
	//		C3DUtils::SetStreamSource(m_Renderer->m_Device, 0, _blocks[i]->m_Mesh->m_VB, sizeof(MODELVERTEXCOLOR));
	//		m_Renderer->m_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, _blocks[i]->m_Mesh->m_NumFaces);

	//		m_Renderer->m_NumPolygons += _blocks[i]->m_Mesh->m_NumFaces;
	//	}

	//	// render walkplanes
	//	for(int i=0; i<_planes.size(); i++)
	//	{
	//		if(!_planes[i]->m_Active) continue;
	//		if(!_planes[i]->m_Mesh->m_VB) continue;
	//		if(!_planes[i]->m_ReceiveShadows) continue;
	//		C3DUtils::SetStreamSource(m_Renderer->m_Device, 0, _planes[i]->m_Mesh->m_VB, sizeof(MODELVERTEXCOLOR));
	//		m_Renderer->m_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, _planes[i]->m_Mesh->m_NumFaces);

	//		m_Renderer->m_NumPolygons += _planes[i]->m_Mesh->m_NumFaces;
	//	}

	//	// render generic meshes
	//	for(int i=0; i<_generics.size(); i++)
	//	{
	//		if(!_generics[i]->m_Active) continue;
	//		if(!_generics[i]->m_Mesh->m_VB) continue;
	//		if(!_generics[i]->m_ReceiveShadows) continue;
	//		C3DUtils::SetStreamSource(m_Renderer->m_Device, 0, _generics[i]->m_Mesh->m_VB, sizeof(MODELVERTEXCOLOR));
	//		m_Renderer->m_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, _generics[i]->m_Mesh->m_NumFaces);

	//		m_Renderer->m_NumPolygons += _generics[i]->m_Mesh->m_NumFaces;
	//	}

	//	m_Renderer->SetSpriteBlendMode(BLEND_NORMAL);

	return true;
}

//////////////////////////////////////////////////////////////////////////
float AdSceneGeometry::getHeightAt(Math::Vector3d pos, float tolerance, bool *intFound) {
	float ret = pos.y();
	Math::Vector3d intersection;
	Math::Vector3d dir = Math::Vector3d(0, -1, 0);

	pos.y() += tolerance;

	bool intFoundTmp = false;

	for (uint32 i = 0; i < _planes.size(); i++) {
		for (int j = 0; j < _planes[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _planes[i]->_mesh->getFace(j);
			float *v0 = _planes[i]->_mesh->getVertexPosition(triangle[0]);
			float *v1 = _planes[i]->_mesh->getVertexPosition(triangle[1]);
			float *v2 = _planes[i]->_mesh->getVertexPosition(triangle[2]);

			if (lineIntersectsTriangle(pos, dir,
			                           Math::Vector3d(v0[0], v0[1], v0[2]),
			                           Math::Vector3d(v1[0], v1[1], v1[2]),
			                           Math::Vector3d(v2[0], v2[1], v2[2]),
			                           intersection.x(), intersection.y(), intersection.z())) {
				if (intersection.y() > pos.y() + tolerance) {
					continue; // only fall down
				}

				if (!intFoundTmp || ABS(ret - pos.y()) > ABS(intersection.y() - pos.y())) {
					ret = intersection.y();
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
bool AdSceneGeometry::directPathExists(Math::Vector3d *p1, Math::Vector3d *p2) {
	// pretty sure this stuff can be somewhat simplified by factoring it out
	// into some functions but let's leave this to a later point

	// test walkplanes
	for (uint i = 0; i < _planes.size(); i++) {
		for (int j = 0; j < _planes[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _planes[i]->_mesh->getFace(j);
			float *v0 = _planes[i]->_mesh->getVertexPosition(triangle[0]);
			float *v1 = _planes[i]->_mesh->getVertexPosition(triangle[1]);
			float *v2 = _planes[i]->_mesh->getVertexPosition(triangle[2]);
			Math::Vector3d intersection;
			float dist;

			if (lineSegmentIntersectsTriangle(*p1, *p2, Math::Vector3d(v0[0], v0[1], v0[2]),
			                                  Math::Vector3d(v1[0], v1[1], v1[2]),
			                                  Math::Vector3d(v2[0], v2[1], v2[2]),
			                                  intersection, dist)) {
				if (lineIntersectsTriangle(*p1, *p1 - *p2, v0, v1, v2,
				                           intersection.x(), intersection.y(), intersection.z())) {
					return false;
				}

				if (lineIntersectsTriangle(*p2, *p2 - *p1, v0, v1, v2,
				                           intersection.x(), intersection.y(), intersection.z())) {
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
			float *v0 = _blocks[i]->_mesh->getVertexPosition(triangle[0]);
			float *v1 = _blocks[i]->_mesh->getVertexPosition(triangle[1]);
			float *v2 = _blocks[i]->_mesh->getVertexPosition(triangle[2]);
			Math::Vector3d intersection;
			float dist;

			if (lineSegmentIntersectsTriangle(*p1, *p2, Math::Vector3d(v0[0], v0[1], v0[2]),
			                                  Math::Vector3d(v1[0], v1[1], v1[2]),
			                                  Math::Vector3d(v2[0], v2[1], v2[2]),
			                                  intersection, dist)) {
				if (lineIntersectsTriangle(*p1, *p1 - *p2, v0, v1, v2,
				                           intersection.x(), intersection.y(), intersection.z())) {
					return false;
				}

				if (lineIntersectsTriangle(*p2, *p2 - *p1, v0, v1, v2,
				                           intersection.x(), intersection.y(), intersection.z())) {
					return false;
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
Math::Vector3d AdSceneGeometry::getBlockIntersection(Math::Vector3d *p1, Math::Vector3d *p2) {
	// test blocks
	for (uint i = 0; i < _blocks.size(); i++) {
		if (!_blocks[i]->_active) {
			continue;
		}

		for (int j = 0; j < _blocks[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _blocks[i]->_mesh->getFace(j);
			float *v0 = _blocks[i]->_mesh->getVertexPosition(triangle[0]);
			float *v1 = _blocks[i]->_mesh->getVertexPosition(triangle[1]);
			float *v2 = _blocks[i]->_mesh->getVertexPosition(triangle[2]);

			Math::Vector3d intersection;
			float dist;

			if (lineSegmentIntersectsTriangle(*p1, *p2, v0, v1, v2, intersection, dist)) {
				if (lineIntersectsTriangle(*p1, *p1 - *p2, v0, v1, v2, intersection.x(), intersection.y(), intersection.z())) {
					return intersection;
				}

				if (lineIntersectsTriangle(*p2, *p2 - *p1, v0, v1, v2, intersection.x(), intersection.y(), intersection.z())) {
					return intersection;
				}
			}
		}
	}

	return Math::Vector3d(0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::convert2Dto3DTolerant(int x, int y, Math::Vector3d *pos) {
	bool Ret = convert2Dto3D(x, y, pos);
	if (Ret) {
		return Ret;
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
bool AdSceneGeometry::convert2Dto3D(int x, int y, Math::Vector3d *pos) {
	bool intFound = false;
	float minDist = FLT_MAX;

	Math::Ray ray = _gameRef->_renderer3D->rayIntoScene(x, y);

	for (uint32 i = 0; i < _planes.size(); i++) {
		for (int j = 0; j < _planes[i]->_mesh->faceCount(); j++) {
			uint16 *triangle = _planes[i]->_mesh->getFace(j);
			float *v0 = _planes[i]->_mesh->getVertexPosition(triangle[0]);
			float *v1 = _planes[i]->_mesh->getVertexPosition(triangle[1]);
			float *v2 = _planes[i]->_mesh->getVertexPosition(triangle[2]);
			Math::Vector3d intersection;

			if (lineIntersectsTriangle(ray.getOrigin(), ray.getDirection(),
			                           Math::Vector3d(v0[0], v0[1], v0[2]),
			                           Math::Vector3d(v1[0], v1[1], v1[2]),
			                           Math::Vector3d(v2[0], v2[1], v2[2]),
			                           intersection.x(), intersection.y(), intersection.z())) {
				Math::Vector3d lineSegement = intersection - getActiveCamera()->_position;
				float dist = lineSegement.getMagnitude();

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
bool AdSceneGeometry::getPath(Math::Vector3d source, Math::Vector3d target, AdPath3D *path, bool rerun) {
	if (!_PFReady) {
		return false;
	} else {
		source.y() = getHeightAt(source, _waypointHeight) + _waypointHeight;
		target.y() = getHeightAt(target, _waypointHeight) + _waypointHeight;

		_PFReady = false;
		_PFSource = source;
		_PFTarget = target;
		_PFTargetPath = path;
		_PFAlternateTarget = Math::Vector3d(0, 0, 0);
		_PFAlternateDist = FLT_MAX;

		_PFTargetPath->reset();
		_PFTargetPath->setReady(false);
		_PFRerun = rerun;

		// prepare working path
		uint i;
		uint j;

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
			if (_PFAlternateTarget != Math::Vector3d(0, 0, 0)) {
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
			_PFTargetPath->_points.insert_at(0, new Math::Vector3d(lowest_pt->_pos));
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
					Math::Vector3d Line = _PFPath[i]->_pos - lowest_pt->_pos;
					float Len = Line.getMagnitude();

					if (Len < _PFAlternateDist) {
						_PFAlternateDist = Len;
						_PFAlternateTarget = getBlockIntersection(&lowest_pt->_pos, &_PFPath[i]->_pos);

						Math::Vector3d Dir = _PFAlternateTarget - lowest_pt->_pos;
						Dir.normalize();
						_PFAlternateTarget -= Dir * 30;
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
float AdSceneGeometry::getPointsDist(Math::Vector3d p1, Math::Vector3d p2) {
	if (!directPathExists(&p1, &p2)) {
		return -1;
	}

	Math::Vector3d vect = p2 - p1;
	return vect.getMagnitude();
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
	warning("AdSceneGeometry::createLights");

	//	int i;

	//	// disable all lights
	//	CBRenderD3D* m_Renderer = (CBRenderD3D*)_gameRef->m_Renderer;
	//	int MaxLights = m_Renderer->GetMaxActiveLights();

	//	for(i=0; i<100; i++)
	//		m_Renderer->m_Device->LightEnable(i, FALSE);

	//	for(i=0; i<_lights.size(); i++)
	//	{
	//		if(i >= 100) break;
	//		_lights[i]->SetLight(i);
	//	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::enableLights(Math::Vector3d point, BaseArray<char *> &ignoreLights) {
	warning("AdScene::enableLights not yet implemented");

	//	CBRenderD3D* m_Renderer = (CBRenderD3D*)_gameRef->m_Renderer;
	//	int MaxLights = m_Renderer->GetMaxActiveLights();

	//	int NumActiveLights = 0;
	//	for(int i=0; i<_lights.size(); i++)
	//	{
	//		_lights[i]->m_IsAvailable = false;
	//		if(_lights[i]->m_Active) NumActiveLights++;
	//	}
	//	if(NumActiveLights <= MaxLights)
	//	{
	//		for(int i=0; i<_lights.size(); i++)
	//		{
	//			_lights[i]->m_IsAvailable = true;
	//		}
	//	}
	//	else
	//	{
	//		if(!m_MaxLightsWarning)
	//		{
	//			_gameRef->LOG(0, "Warning: Using more lights than the hardware supports (%d)", MaxLights);
	//			m_MaxLightsWarning = true;
	//		}

	//		BaseArray<Light3D*> ActiveLights;

	//		// compute distance to point
	//		for(int i=0; i<_lights.size(); i++)
	//		{
	//			if(!_lights[i]->m_Active) continue;

	//			Math::Vector3d Dif;
	//			if(_lights[i]->m_IsSpotlight)
	//			{
	//				//Dif = _lights[i]->m_Target - Point;
	//				Math::Vector3d Dir = _lights[i]->m_Target - _lights[i]->m_Pos;
	//				Dif = (_lights[i]->m_Pos + Dir * 0.75f) - Point;
	//			}
	//			else
	//				Dif = _lights[i]->m_Pos - Point;

	//			_lights[i]->m_Distance = fabs(D3DXVec3Length(&Dif));

	//			ActiveLights.add(_lights[i]);
	//		}

	//		// sort by distance
	//		if(ActiveLights.size() > 0)
	//		{
	//			qsort(ActiveLights.GetData(), ActiveLights.size(), sizeof(Light3D*), AdSceneGeometry::CompareLights);

	//			for(int i=0; i<ActiveLights.size(); i++)
	//			{
	//				ActiveLights[i]->m_IsAvailable = i < MaxLights;
	//			}
	//		}
	//	}

	//	// light all available lights
	//	for(int i=0; i<100; i++)
	//	{
	//		m_Renderer->m_Device->LightEnable(i, FALSE);
	//	}

	//	NumActiveLights = 0;
	//	for(int i=0; i<_lights.size(); i++)
	//	{
	//		if(NumActiveLights >= MaxLights) break;

	//		if(IgnoreLights.size())
	//		{
	//			bool Ignore = false;
	//			for(int j=0; j<IgnoreLights.size(); j++)
	//			{
	//				char* c1 = _lights[i]->m_Name;
	//				char* c2 = IgnoreLights[j];
	//				if(stricmp(_lights[i]->m_Name, IgnoreLights[j])==0)
	//				{
	//					Ignore = true;
	//					break;
	//				}
	//			}
	//			if(Ignore) continue; // ship this light
	//		}

	//		if(_lights[i]->m_IsAvailable)
	//		{
	//			m_Renderer->m_Device->LightEnable(i, _lights[i]->m_Active);
	//			if(_lights[i]->m_Active) NumActiveLights++;
	//		}
	//	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// keep this commented out for later, although we are going to
// use a different siganture anyways since we want to use ScummVM's sort
//int AdSceneGeometry::compareLights(const void *obj1, const void *obj2) {
//	Light3D *Light1 = *(Light3D **)obj1;
//	Light3D *Light2 = *(Light3D **)obj2;

//	if (Light1->_distance < Light2->_distance) {
//		return -1;
//	} else if (Light1->_distance > Light2->_distance) {
//		return 1;
//	} else {
//		return 0;
//	}
//}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::correctTargetPoint(const Math::Vector3d &source, Math::Vector3d *target) {
	// the source parameter is not even used in wme3d
	int i;
	int MaxLen = 1000;
	int Step = 10;
	Math::Vector3d newTarget;

	int lenLeft = 0;
	int lenRight = 0;
	int lenUp = 0;
	int lenDown = 0;

	// left
	newTarget = *target;
	for (i = 1; i <= MaxLen; i += Step) {
		newTarget.x() -= i;
		if (!directPathExists(target, &newTarget)) {
			lenLeft = i;
			break;
		}
	}

	// right
	newTarget = *target;
	for (i = 1; i <= MaxLen; i += Step) {
		newTarget.x() += i;
		if (!directPathExists(target, &newTarget)) {
			lenRight = i;
			break;
		}
	}

	// up
	newTarget = *target;
	for (i = 1; i <= MaxLen; i += Step) {
		newTarget.z() -= i;
		if (!directPathExists(target, &newTarget)) {
			lenUp = i;
			break;
		}
	}

	// down
	newTarget = *target;
	for (i = 1; i <= MaxLen; i += Step) {
		newTarget.z() += i;
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
		target->x() += offsetX;
	} else {
		target->z() += offsetZ;
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
bool AdSceneGeometry::enableLight(char *lightName, bool enable) {
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
bool AdSceneGeometry::isLightEnabled(char *lightName) {
	for (uint i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(lightName, _lights[i]->getName()) == 0) {
			return _lights[i]->_active;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::setLightColor(char *lightName, uint32 color) {
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
uint32 AdSceneGeometry::getLightColor(char *lightName) {
	for (uint i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(lightName, _lights[i]->getName()) == 0) {
			return _lights[i]->_diffuseColor;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
Math::Vector3d AdSceneGeometry::getLightPos(char *lightName) {
	for (uint i = 0; i < _lights.size(); i++) {
		if (scumm_stricmp(lightName, _lights[i]->getName()) == 0) {
			return _lights[i]->_position;
		}
	}
	return Math::Vector3d(0, 0, 0);
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
	int numLights = _lights.size();
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
			}
		}
	}
	createLights();

	//////////////////////////////////////////////////////////////////////////
	int numBlocks = _blocks.size();
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
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int numPlanes = _planes.size();
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
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int numGenerics = _generics.size();
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
			}
		}
	}

	if (!persistMgr->getIsSaving()) {
		_maxLightsWarning = false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::convert3Dto2D(Math::Vector3d *pos, int *x, int *y) {
	Math::Matrix4 worldMat;
	worldMat.setToIdentity();

	Math::Vector3d vect2D;
	int viewportTmp[4];
	// TODO: gluMathProject expects an OpenGL viewport,
	// hence the first coordinates specify the lower left corner
	// wme works with a Direct3D viewport, though
	// so check if this does work
	viewportTmp[0] = _drawingViewport.left;
	viewportTmp[1] = _drawingViewport.bottom;
	viewportTmp[2] = _drawingViewport.width();
	viewportTmp[3] = _drawingViewport.height();
	Math::Matrix4 modelViewMatrix = _lastViewMat * worldMat;
	Math::gluMathProject(*pos, modelViewMatrix.getData(), _lastProjMat.getData(), viewportTmp, vect2D);
	*x = vect2D.x() + _lastScrollX;
	*y = vect2D.y() + _lastScrollY;

	return true;
}

} // namespace Wintermute
