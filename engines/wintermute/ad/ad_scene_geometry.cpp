// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "ad_scene_geometry.h"
#include "../base/gfx/opengl/camera3d.h"
#include "../base/gfx/opengl/light3d.h"
#include "../base/base_game.h"
#include "ad_game.h"
#include "ad_scene.h"
#include "../base/base_sprite.h"
#include "../base/file/base_file.h"
#include "../system/sys_class_registry.h"
#include "ad_path_point3d.h"
#include "ad_path3d.h"
#include "ad_walkplane.h"
#include "ad_generic.h"
#include "ad_block.h"
#include "ad_waypoint_group3d.h"
#include "ad_geom_ext.h"
#include "ad_geom_ext_node.h"
#include "../base/base_file_manager.h"
#include "common/math.h"
#include "../wintermute.h"
#include "../base/gfx/opengl/mesh.h"
#include "math/glmath.h"
#include "../base/gfx/opengl/loader3ds.h"
#include "../base/gfx/opengl/base_render_opengl3d.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdSceneGeometry, false);

//////////////////////////////////////////////////////////////////////////
AdSceneGeometry::AdSceneGeometry(BaseGame* gameRef): BaseObject(gameRef)
{
	m_ActiveCamera = m_ActiveLight = -1;
	m_ViewMatrix.setToIdentity();
	//m_WaypointHeight = 5.0f;
	//m_WaypointHeight = 1.0f;
	m_WaypointHeight = 10.0f;
	m_WptMarker = NULL;

	_PFReady = true;
	_PFTargetPath = NULL;
	_PFMaxTime = 15;
	_PFRerun = false;

	_PFSource = _PFTarget = _PFAlternateTarget = Math::Vector3d(0, 0, 0);
	_PFAlternateDist = FLT_MAX;


	m_DrawingViewport.setRect(0, 0, 0, 0);

	m_LastWorldMat.setToIdentity();
	m_LastViewMat.setToIdentity();
	m_LastProjMat.setToIdentity();

	m_LastOffsetX = m_LastOffsetY = 0;
	m_LastScrollX = m_LastScrollY = 0;

	m_LastValuesInitialized = false;
	m_MaxLightsWarning = false;
}


//////////////////////////////////////////////////////////////////////////
AdSceneGeometry::~AdSceneGeometry()
{	
	Cleanup();

	if (m_WptMarker)
	{
		delete  m_WptMarker;
	}
}


//////////////////////////////////////////////////////////////////////////
void AdSceneGeometry::Cleanup()
{
	unsigned i;

	for(i=0; i <_planes.size(); i++) delete _planes[i];
	_planes.clear();

	for(i=0; i <_blocks.size(); i++) delete _blocks[i];
	_blocks.clear();

	for(i=0; i <_generics.size(); i++) delete _generics[i];
	_generics.clear();

	for(i=0; i <_waypointGroups.size(); i++) delete _waypointGroups[i];
	_waypointGroups.clear();

	for(i=0; i <_cameras.size(); i++)
	{
//		CBRenderD3D* _renderer = (CBRenderD3D*)_gameRef->_renderer;
//		if(m_Renderer->m_Camera == _cameras[i]) m_Renderer->m_Camera = NULL;

		delete _cameras[i];
	}
	_cameras.clear();

	for(i=0; i <_lights.size(); i++) delete _lights[i];
	_lights.clear();

	m_ActiveCamera = m_ActiveLight = -1;
	m_ViewMatrix.setToIdentity();

	for(i=0; i <_PFPath.size(); i++)
		delete _PFPath[i];
	_PFPath.clear();

	_PFTargetPath = NULL;
}

//////////////////////////////////////////////////////////////////////////
AdGeomExt* AdSceneGeometry::GetGeometryExtension(char* Filename)
{
	AdGeomExt* Ret = new AdGeomExt(_gameRef);

	bool LoadOK = false;
	if(BaseFileManager::getEngineInstance()->openFile(Filename) != nullptr)
	{
		LoadOK = Ret->LoadFile(Filename);
	}

	// no ext file found, just use the defaults
	if(!LoadOK) Ret->AddStandardNodes();

	return Ret;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::LoadFile(const char* Filename)
{
	Cleanup();

	// load waypoint graphics from resources
	if(!m_WptMarker)
	{
		m_WptMarker = new BaseSprite(_gameRef);
		if(m_WptMarker){
			if(!m_WptMarker->loadFile("wpt.sprite"))
			{
				delete m_WptMarker;
				m_WptMarker = NULL;
			}
		}
	}

	Common::String filenameTmp(Filename);

	if (!filenameTmp.hasSuffix(".3ds"))
	{
		_gameRef->LOG(0, "Error: no suitable loader found for file '%s'", Filename);
		return false;
	}

	filenameTmp.replace(filenameTmp.size() - 3, 3, "geometry", 0, 8);
	AdGeomExt* GeomExt = GetGeometryExtension(filenameTmp.begin());

	BaseArray<Mesh*> meshes;
	BaseArray<Common::String> meshNames;

	if(!load3DSFile(Filename, meshes, meshNames, _lights, _cameras))
	{
		delete GeomExt;
		return false;
	}

	unsigned i;

	SystemClassRegistry::getInstance()->_disabled = true;

	// load meshes
	for(i = 0; i < meshes.size(); i++)
	{
		AdGeomExtNode* ExtNode = GeomExt->MatchName((char*)meshNames[i].c_str());
		if(!ExtNode) continue;

		switch(ExtNode->m_Type)
		{
			case GEOM_WALKPLANE:
			{
				AdWalkplane* plane = new AdWalkplane(_gameRef);
				plane->setName((char*)meshNames[i].c_str());
				plane->m_Mesh = meshes[i];
				plane->m_Mesh->computeNormals();
				plane->m_Mesh->fillVertexBuffer(0xFFFF00FF);
				plane->m_ReceiveShadows = ExtNode->m_ReceiveShadows;
				_planes.add(plane);
			}
			break;

			case GEOM_BLOCKED:
			{
				AdBlock* block = new AdBlock(_gameRef);
				block->setName((char*)meshNames[i].c_str());
				block->m_Mesh = meshes[i];
				block->m_Mesh->computeNormals();
				block->m_Mesh->fillVertexBuffer(0xFFFF00FF);
				block->m_ReceiveShadows = ExtNode->m_ReceiveShadows;
				_blocks.add(block);
			}
			break;

			case GEOM_WAYPOINT:
			{
				Mesh* mesh = meshes[i];
				// TODO: groups
				if(_waypointGroups.size()==0) _waypointGroups.add(new AdWaypointGroup3D(_gameRef));
				_waypointGroups[0]->AddFromMesh(mesh);
				delete mesh;
			}
			break;

			case GEOM_GENERIC:
			{
				AdGeneric* generic = new AdGeneric(_gameRef);
				generic->setName((char*)meshNames[i].c_str());
				generic->m_Mesh = meshes[i];
				generic->m_Mesh->computeNormals();
				generic->m_Mesh->fillVertexBuffer(0xFFFF00FF);
				generic->m_ReceiveShadows = ExtNode->m_ReceiveShadows;
				_generics.add(generic);
			}
			break;

		}
	}

	SystemClassRegistry::getInstance()->_disabled = false;

	if(_cameras.size()>0) SetActiveCamera(0, -1.0f, -1.0f, -1.0f);
	CreateLights();

	if(_lights.size()>0) SetActiveLight(0);

	delete GeomExt;

	// drop waypoints to the ground
	DropWaypoints();

	if(getFilename()!=Filename) setFilename(Filename);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::DropWaypoints()
{
	for(unsigned i=0; i<_waypointGroups.size(); i++)
	{
		for(unsigned j=0; j<_waypointGroups[i]->_points.size(); j++)
		{
			Math::Vector3d* point = _waypointGroups[i]->_points[j];
			point->y() = GetHeightAt(*point) + m_WaypointHeight;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::SetActiveCamera(int Camera, float FOV, float NearClipPlane, float FarClipPlane)
{
	if(Camera < 0 || Camera >= _cameras.size())
	{
		_gameRef->LOG(0, "Warning: Camera %d is out of bounds.", Camera);
		return false;
	}
	else
	{
		m_ActiveCamera = Camera;
		
		if(FOV>=0.0f) _cameras[Camera]->m_FOV = FOV;
		else _cameras[Camera]->m_FOV = _cameras[Camera]->m_OrigFOV;
		
		_cameras[Camera]->m_NearClipPlane = NearClipPlane;
		_cameras[Camera]->m_FarClipPlane = FarClipPlane;
		
		_cameras[Camera]->GetViewMatrix(&m_ViewMatrix);
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::SetActiveCamera(char* Camera, float FOV, float NearClipPlane, float FarClipPlane)
{
	for(unsigned i=0; i<_cameras.size(); i++){
		if(scumm_stricmp(_cameras[i]->getName(), Camera)==0) return SetActiveCamera(i, FOV, NearClipPlane, FarClipPlane);
	}

	_gameRef->LOG(0, "Warning: Camera '%s' not found.", Camera);
	return false;
}

//////////////////////////////////////////////////////////////////////////
Camera3D* AdSceneGeometry::GetActiveCamera()
{
	if(m_ActiveCamera >= 0 && m_ActiveCamera < _cameras.size())
		return _cameras[m_ActiveCamera];
	else return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::SetActiveLight(int Light)
{
	if(Light < 0 || Light >= _lights.size()){
		_gameRef->LOG(0, "Warning: Light %d is out of bounds.", Light);
		return false;
	}
	else{
		m_ActiveLight = Light;
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::SetActiveLight(char* Light)
{
	for(unsigned i=0; i<_lights.size(); i++){
		if(scumm_stricmp(_lights[i]->getName(), Light)==0) return SetActiveLight(i);
	}

	_gameRef->LOG(0, "Warning: Light '%s' not found.", Light);
	return false;
}


//////////////////////////////////////////////////////////////////////////
Math::Matrix4* AdSceneGeometry::GetViewMatrix()
{
	return &m_ViewMatrix;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::StoreDrawingParams()
{
	// implement this later
//	CBRenderD3D* m_Renderer = (CBRenderD3D*)_gameRef->m_Renderer;

//	// store values
//	m_Renderer->m_Device->GetViewport(&m_DrawingViewport);

//	m_Renderer->m_Device->GetTransform(D3DTS_WORLD, &m_LastWorldMat);
//	m_Renderer->m_Device->GetTransform(D3DTS_VIEW, &m_LastViewMat);
//	m_Renderer->m_Device->GetTransform(D3DTS_PROJECTION, &m_LastProjMat);

	AdScene* Scene = ((AdGame*)_gameRef)->_scene;
	if(Scene)
	{
		m_LastScrollX = Scene->getOffsetLeft();
		m_LastScrollY = Scene->getOffsetTop();
	}
	else
	{
		m_LastScrollX = 0;
		m_LastScrollY = 0;
	}

	Rect32 rc;
	_gameRef->getCurrentViewportRect(&rc);
	float Width = (float)rc.right - (float)rc.left;
	float Height = (float)rc.bottom - (float)rc.top;

	// margins
//	int mleft = rc.left;
//	int mright = m_Renderer->m_Width - Width - rc.left;
//	int mtop = rc.top;
//	int mbottom = m_Renderer->m_Height - Height - rc.top;


//	m_LastOffsetX = _gameRef->_offsetX + (mleft - mright)/2;
//	m_LastOffsetY = _gameRef->_offsetY + (mtop - mbottom)/2;

	m_LastValuesInitialized = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::Render(bool Render)
{
	// we know that we have opengl available if this class is instantiated
	BaseRenderOpenGL3D* renderer = static_cast<BaseRenderOpenGL3D*>(_gameRef->_renderer);

//	// store values
//	StoreDrawingParams();
	if(!Render) return true;

	renderer->resetModelViewTransform();
	renderer->setup3D(GetActiveCamera());

	// factor this out later

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindTexture(GL_TEXTURE_2D, 0);

//	m_Renderer->m_Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
//	m_Renderer->m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
//	m_Renderer->m_Device->SetRenderState(D3DRS_ZENABLE, FALSE);
//	m_Renderer->m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
//	m_Renderer->m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
//	m_Renderer->m_Device->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
//	m_Renderer->m_Device->SetTexture(0, NULL);

	unsigned i;

	// render walk planes
	for(i=0; i<_planes.size(); i++)
	{
		if(!_planes[i]->m_Active) {
//			continue;
		}

		_planes[i]->m_Mesh->render();

		//m_Renderer->m_NumPolygons += _planes[i]->m_Mesh->m_NumFaces;
	}

	// render blocks
	for(i=0; i<_blocks.size(); i++)
	{
		if(!_blocks[i]->m_Active) {
//			continue;
		}

		_blocks[i]->m_Mesh->render();

//		m_Renderer->m_NumPolygons += _blocks[i]->m_Mesh->m_NumFaces;
	}

	// render generic objects
	for(i=0; i<_generics.size(); i++)
	{
		if(!_generics[i]->m_Active) {
//			continue;
		}

		_generics[i]->m_Mesh->render();

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
bool AdSceneGeometry::RenderShadowGeometry()
{
	StoreDrawingParams();

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
float AdSceneGeometry::GetHeightAt(Math::Vector3d Pos, float Tolerance, bool* IntFound)
{
	float ret = Pos.y();
	Math::Vector3d intersection;
	Math::Vector3d dir = Math::Vector3d(0, -1, 0);

//	Pos.y += Tolerance;

//	bool int_found = false;
//	for(int i=0; i<_planes.size(); i++){
//		for(int j=0; j<_planes[i]->m_Mesh->m_NumFaces; j++){
//			if(C3DUtils::IntersectTriangle(
//						Pos, dir,
//						_planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[0]].m_Pos,
//						_planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[1]].m_Pos,
//						_planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[2]].m_Pos,
//						&intersection.x, &intersection.y, &intersection.z)){
//				if(intersection.y > Pos.y+Tolerance) continue; // only fall down
//				if(!int_found || fabs(ret - Pos.y) > fabs(intersection.y - Pos.y)) ret = intersection.y;
//				int_found = true;
//			}

//		}
//	}

//	if(IntFound) *IntFound = int_found;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::DirectPathExists(Math::Vector3d* p1, Math::Vector3d* p2)
{
	Math::Vector3d v0, v1, v2;
	int i,j;

//	// test walkplanes
//	for(i=0; i<_planes.size(); i++){
//		for(j=0; j<_planes[i]->m_Mesh->m_NumFaces; j++){
//			v0 = _planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[0]].m_Pos;
//			v1 = _planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[1]].m_Pos;
//			v2 = _planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[2]].m_Pos;
			
//			D3DXPLANE plane;
//			Math::Vector3d intersection;
//			float dist;

//			if(C3DUtils::PickGetIntersect(*p1, *p2, v0, v1, v2, &intersection, &dist)){
//				if(C3DUtils::IntersectTriangle(*p1, *p1-*p2, v0, v1, v2, &intersection.x, &intersection.y, &intersection.z)) return false;
//				if(C3DUtils::IntersectTriangle(*p2, *p2-*p1, v0, v1, v2, &intersection.x, &intersection.y, &intersection.z)) return false;
//			}
//		}
//	}

//	// test blocks
//	for(i=0; i<_blocks.size(); i++)
//	{
//		if(!_blocks[i]->m_Active) continue;
//		for(j=0; j<_blocks[i]->m_Mesh->m_NumFaces; j++)
//		{
//			v0 = _blocks[i]->m_Mesh->m_Vertices[_blocks[i]->m_Mesh->m_Faces[j].m_Vertices[0]].m_Pos;
//			v1 = _blocks[i]->m_Mesh->m_Vertices[_blocks[i]->m_Mesh->m_Faces[j].m_Vertices[1]].m_Pos;
//			v2 = _blocks[i]->m_Mesh->m_Vertices[_blocks[i]->m_Mesh->m_Faces[j].m_Vertices[2]].m_Pos;
			
//			D3DXPLANE plane;
//			Math::Vector3d intersection;
//			float dist;

//			if(C3DUtils::PickGetIntersect(*p1, *p2, v0, v1, v2, &intersection, &dist))
//			{
//				if(C3DUtils::IntersectTriangle(*p1, *p1-*p2, v0, v1, v2, &intersection.x, &intersection.y, &intersection.z)) return false;
//				if(C3DUtils::IntersectTriangle(*p2, *p2-*p1, v0, v1, v2, &intersection.x, &intersection.y, &intersection.z)) return false;
//			}
//		}
//	}


	return true;
}

//////////////////////////////////////////////////////////////////////////
Math::Vector3d AdSceneGeometry::GetBlockIntersection(Math::Vector3d* p1, Math::Vector3d* p2)
{
	Math::Vector3d v0, v1, v2;

	// implement this later

//	// test blocks
//	for(int i=0; i<_blocks.size(); i++)
//	{
//		if(!_blocks[i]->m_Active) continue;
//		for(int j=0; j<_blocks[i]->m_Mesh->m_NumFaces; j++)
//		{
//			v0 = _blocks[i]->m_Mesh->m_Vertices[_blocks[i]->m_Mesh->m_Faces[j].m_Vertices[0]].m_Pos;
//			v1 = _blocks[i]->m_Mesh->m_Vertices[_blocks[i]->m_Mesh->m_Faces[j].m_Vertices[1]].m_Pos;
//			v2 = _blocks[i]->m_Mesh->m_Vertices[_blocks[i]->m_Mesh->m_Faces[j].m_Vertices[2]].m_Pos;

//			D3DXPLANE plane;
//			Math::Vector3d intersection;
//			float dist;

//			if(C3DUtils::PickGetIntersect(*p1, *p2, v0, v1, v2, &intersection, &dist))
//			{
//				if(C3DUtils::IntersectTriangle(*p1, *p1-*p2, v0, v1, v2, &intersection.x, &intersection.y, &intersection.z)) return intersection;
//				if(C3DUtils::IntersectTriangle(*p2, *p2-*p1, v0, v1, v2, &intersection.x, &intersection.y, &intersection.z)) return intersection;
//			}
//		}
//	}
	return Math::Vector3d(0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::Convert2Dto3DTolerant(int X, int Y, Math::Vector3d *Pos)
{
	bool Ret = Convert2Dto3D(X, Y, Pos);
	if(Ret) return Ret;


	int LenLeft = 0;
	int LenRight = 0;
	int LenDown = 0;
	int LenUp = 0;

	int i;

	// left
	for(i=0; i<1000; i+=10)
	{
		if(Convert2Dto3D(X-i, Y, Pos))
		{
			LenLeft = i;
			break;
		}
	}

	// right
	for(i=0; i<1000; i+=10)
	{
		if(Convert2Dto3D(X+i, Y, Pos))
		{
			LenRight = i;
			break;
		}
	}

	// up
	for(i=0; i<1000; i+=10)
	{
		if(Convert2Dto3D(X, Y-i, Pos))
		{
			LenUp = i;
			break;
		}
	}

	// down
	for(i=0; i<1000; i+=10)
	{
		if(Convert2Dto3D(X, Y+i, Pos))
		{
			LenDown = i;
			break;
		}
	}

	if(!LenLeft && !LenRight && !LenUp && !LenDown)	return false;


	int OffsetX = INT_MAX_VALUE;
	int OffsetY = INT_MAX_VALUE;

	if(LenLeft || LenRight)
	{
		if(LenRight)
		{
			if(LenLeft && LenLeft<LenRight) OffsetX = -LenLeft;
			else OffsetX = LenRight;
		}
		else OffsetX = -LenLeft;
	}

	if(LenUp || LenDown)
	{
		if(LenDown)
		{
			if(LenUp && LenUp<LenDown) OffsetY = -LenUp;
			else OffsetY = LenDown;
		}
		else OffsetY = -LenUp;
	}


	if(abs(OffsetX) < abs(OffsetY)) X += OffsetX;
	else Y += OffsetY;

	return Convert2Dto3D(X, Y, Pos);
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::Convert2Dto3D(int X, int Y, Math::Vector3d *Pos)
{
	// implement this later

//	CBRenderD3D* rend = (CBRenderD3D*)_gameRef->m_Renderer;

//	if(!m_LastValuesInitialized)
//	{
//		rend->m_Device->GetViewport(&m_DrawingViewport);
//		rend->m_Device->GetTransform(D3DTS_PROJECTION, &m_LastProjMat);
//	}


//	float ResWidth, ResHeight;
//	float LayerWidth, LayerHeight;
//	float ModWidth, ModHeight;
//	bool CustomViewport;
//	rend->GetProjectionParams(&ResWidth, &ResHeight, &LayerWidth, &LayerHeight, &ModWidth, &ModHeight, &CustomViewport);

//	// modify coordinates according to viewport settings
//	int mleft = m_DrawingViewport.X;
//	int mright = ResWidth - m_DrawingViewport.width() - m_DrawingViewport.X;
//	int mtop = m_DrawingViewport.Y;
//	int mbottom = ResHeight - m_DrawingViewport.height() - m_DrawingViewport.Y;

//	X-=(mleft + mright)/2 + ModWidth;
//	Y-=(mtop + mbottom)/2 + ModHeight;

	

//	Math::Vector3d vPickRayDir;
//	Math::Vector3d vPickRayOrig;

//	// Compute the vector of the pick ray in screen space
//	Math::Vector3d vec;
//	vec.x =  ((( 2.0f * X) / m_DrawingViewport.width() ) - 1) / m_LastProjMat._11;
//	vec.y = -((( 2.0f * Y) / m_DrawingViewport.height()) - 1) / m_LastProjMat._22;
//	vec.z() =  1.0f;
	
//	// Get the inverse view matrix
//	Math::Matrix4 m = m_ViewMatrix;
//	m.inverse();

//	// Transform the screen space pick ray into 3D space
//	vPickRayDir.x  = vec.x*m._11 + vec.y*m._21 + vec.z*m._31;
//	vPickRayDir.y  = vec.x*m._12 + vec.y*m._22 + vec.z*m._32;
//	vPickRayDir.z  = vec.x*m._13 + vec.y*m._23 + vec.z*m._33;
//	vPickRayOrig.x = m._41;
//	vPickRayOrig.y = m._42;
//	vPickRayOrig.z = m._43;

	bool int_found = false;
//	float min_dist = FLT_MAX;
//	Math::Vector3d intersection, ray;
//	for(int i=0; i<_planes.size(); i++){
//		for(int j=0; j<_planes[i]->m_Mesh->m_NumFaces; j++){
//			if(C3DUtils::IntersectTriangle(
//								vPickRayOrig, vPickRayDir,
//								_planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[0]].m_Pos,
//								_planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[1]].m_Pos,
//								_planes[i]->m_Mesh->m_Vertices[_planes[i]->m_Mesh->m_Faces[j].m_Vertices[2]].m_Pos,
//								&intersection.x, &intersection.y, &intersection.z))
//			{
//				ray = intersection - vPickRayOrig;
//				float dist = D3DXVec3Length(&ray);
//				if(dist < min_dist){
//					*Pos = intersection;
//					min_dist = dist;
//				}
//				int_found = true;
//			}

//		}
//	}
	
	return int_found;
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::GetPath(Math::Vector3d source, Math::Vector3d target, AdPath3D *path, bool Rerun)
{
	if(!_PFReady) return false;
	else{
		source.y() = GetHeightAt(source, m_WaypointHeight) + m_WaypointHeight;
		target.y() = GetHeightAt(target, m_WaypointHeight) + m_WaypointHeight;

		_PFReady = false;
		_PFSource = source;
		_PFTarget = target;
		_PFTargetPath = path;
		_PFAlternateTarget = Math::Vector3d(0, 0, 0);
		_PFAlternateDist = FLT_MAX;

		_PFTargetPath->Reset();
		_PFTargetPath->SetReady(false);
		_PFRerun = Rerun;

		// prepare working path
		unsigned i,j;
		for(i=0; i<_PFPath.size(); i++)
			delete _PFPath[i];
		_PFPath.clear();

		// first point
		_PFPath.add(new AdPathPoint3D(source, 0));

		// last point
		_PFPath.add(new AdPathPoint3D(target, FLT_MAX));

		// add all active waypoints
		for(i=0; i<_waypointGroups.size(); i++){
			if(_waypointGroups[i]->m_Active){
				for(j=0; j<_waypointGroups[i]->_points.size(); j++){
					_PFPath.add(new AdPathPoint3D(*_waypointGroups[i]->_points[j], FLT_MAX));
				}
			}
		}

		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
void AdSceneGeometry::PathFinderStep()
{
	unsigned i;

	// get lowest unmarked
	float lowest_dist = FLT_MAX;
	AdPathPoint3D* lowest_pt=NULL;

	for(i=0; i<_PFPath.size(); i++)
		if(!_PFPath[i]->m_Marked && _PFPath[i]->m_Distance < lowest_dist){
			lowest_dist = _PFPath[i]->m_Distance;
			lowest_pt = _PFPath[i];
		}

	if(lowest_pt == NULL){ // no path -> terminate PathFinder
		_PFReady = true;
		if(!_PFRerun)
		{
			if(_PFAlternateTarget!=Math::Vector3d(0, 0, 0))
			{
				GetPath(_PFSource, _PFAlternateTarget, _PFTargetPath, true);
			}
			else _PFTargetPath->SetReady(true);
		}
		else _PFTargetPath->SetReady(true);
		return;
	}

	lowest_pt->m_Marked = true;

	// target point marked, generate path and terminate
	if(lowest_pt->m_Pos == _PFTarget){
		while(lowest_pt!=NULL){
			_PFTargetPath->_points.insert_at(0, new Math::Vector3d(lowest_pt->m_Pos));
			lowest_pt = lowest_pt->m_Origin;
		}
		// remove current position
		if(_PFTargetPath->_points.size()>0)
		{
			delete _PFTargetPath->_points[0];
			_PFTargetPath->_points.remove_at(0);
		}
		

		_PFReady = true;
		_PFTargetPath->SetReady(true);
		return;
	}

	// otherwise keep on searching
	for(i=0; i<_PFPath.size(); i++)
		if(!_PFPath[i]->m_Marked){
			float dist = GetPointsDist(lowest_pt->m_Pos, _PFPath[i]->m_Pos);
			if(dist>=0 && lowest_pt->m_Distance + dist < _PFPath[i]->m_Distance){
				_PFPath[i]->m_Distance = lowest_pt->m_Distance + dist;
				_PFPath[i]->m_Origin = lowest_pt;
			}
			else
			{
				if(!_PFRerun && _PFPath[i]->m_Pos == _PFTarget)
				{
					Math::Vector3d Line = _PFPath[i]->m_Pos - lowest_pt->m_Pos;
					float Len = Line.getMagnitude();

					if(Len < _PFAlternateDist)
					{
						_PFAlternateDist = Len;
						_PFAlternateTarget = GetBlockIntersection(&lowest_pt->m_Pos, &_PFPath[i]->m_Pos);

						Math::Vector3d Dir = _PFAlternateTarget - lowest_pt->m_Pos;
						Dir.normalize();
						_PFAlternateTarget -= Dir * 30;
					}
				}
			}
		}
}


//////////////////////////////////////////////////////////////////////////
float AdSceneGeometry::GetPointsDist(Math::Vector3d p1, Math::Vector3d p2)
{
	if(!DirectPathExists(&p1, &p2)) return -1;

	Math::Vector3d vect = p2 - p1;
	return vect.getMagnitude();
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::InitLoop()
{
#ifdef _DEBUG
	int num_steps=0;
	DWORD start = _gameRef->m_CurrentTime;
	while(!_PFReady && _gameRef->m_CurrentTime - start <= _PFMaxTime){
		PathFinderStep();
		num_steps++;
	}
	//if(num_steps>0) _gameRef->LOG(0, "STAT: PathFinder3D iterations in one loop: %d (%s)  _PFMaxTime=%d", num_steps, _PFReady?"finished":"not yet done", _PFMaxTime);
#else
	uint32 start = _gameRef->_currentTime;
	while(!_PFReady && _gameRef->_currentTime - start <= _PFMaxTime) PathFinderStep();
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::CreateLights()
{
	// Implement this later

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
bool AdSceneGeometry::EnableLights(Math::Vector3d Point, BaseArray<char*>& IgnoreLights)
{
	// Implement this later

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
int AdSceneGeometry::CompareLights(const void* Obj1, const void* Obj2)
{
	Light3D* Light1 = *(Light3D**)Obj1;
	Light3D* Light2 = *(Light3D**)Obj2;

	if(Light1->m_Distance < Light2->m_Distance) return -1;
	else if(Light1->m_Distance > Light2->m_Distance) return 1;
	else return 0;
}



//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::CorrectTargetPoint(Math::Vector3d Source, Math::Vector3d* Target)
{
	int i;
	int MaxLen = 1000;
	int Step = 10;
	Math::Vector3d NewTarget;

	int LenLeft = 0;
	int LenRight = 0;
	int LenUp = 0;
	int LenDown = 0;


	// left
	NewTarget = *Target;
	for(i=1; i<=MaxLen; i+=Step)
	{
		NewTarget.x() -= i;
		if(!DirectPathExists(Target, &NewTarget))
		{
			LenLeft = i;
			break;
		}
	}

	// right
	NewTarget = *Target;
	for(i=1; i<=MaxLen; i+=Step)
	{
		NewTarget.x() += i;
		if(!DirectPathExists(Target, &NewTarget))
		{
			LenRight = i;
			break;
		}
	}

	// up
	NewTarget = *Target;
	for(i=1; i<=MaxLen; i+=Step)
	{
		NewTarget.z() -= i;
		if(!DirectPathExists(Target, &NewTarget))
		{
			LenUp = i;
			break;
		}
	}

	// down
	NewTarget = *Target;
	for(i=1; i<=MaxLen; i+=Step)
	{
		NewTarget.z() += i;
		if(!DirectPathExists(Target, &NewTarget))
		{
			LenDown = i;
			break;
		}
	}

	if(!LenLeft && !LenRight && !LenUp && !LenDown)	return true;


	int OffsetX = INT_MAX_VALUE;
	int OffsetZ = INT_MAX_VALUE;
	
	if(LenLeft || LenRight)
	{
		if(LenRight)
		{
			if(LenLeft && LenLeft<LenRight) OffsetX = -LenLeft;
			else OffsetX = LenRight;
		}
		else OffsetX = -LenLeft;
	}
	
	if(LenUp || LenDown)
	{
		if(LenDown)
		{
			if(LenUp && LenUp<LenDown) OffsetZ = -LenUp;
			else OffsetZ = LenDown;
		}
		else OffsetZ = -LenUp;
	}

	//_gameRef->QuickMessageForm("%f,%f,%f", Target->x, Target->y, Target->z);
	if(abs(OffsetX) < abs(OffsetZ)) Target->x() += OffsetX;
	else Target->z() += OffsetZ;

	//_gameRef->QuickMessageForm("%f,%f,%f", Target->x, Target->y, Target->z);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::EnableNode(char* NodeName, bool Enable)
{
	bool Ret = false;

	unsigned i;
	for(i=0; i<_blocks.size(); i++)
	{
		if(scumm_stricmp(NodeName, _blocks[i]->getName())==0)
		{
			_blocks[i]->m_Active = Enable;
			Ret = true;
		}
	}

	for(i=0; i<_planes.size(); i++)
	{
		if(scumm_stricmp(NodeName, _planes[i]->getName())==0)
		{
			_planes[i]->m_Active = Enable;
			Ret = true;
		}
	}

	for(i=0; i<_generics.size(); i++)
	{
		if(scumm_stricmp(NodeName, _generics[i]->getName())==0)
		{
			_generics[i]->m_Active = Enable;
			Ret = true;
		}
	}

	return Ret;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::IsNodeEnabled(char* NodeName)
{
	for(unsigned i=0; i<_blocks.size(); i++)
	{
		if(scumm_stricmp(NodeName, _blocks[i]->getName())==0)
		{
			return _blocks[i]->m_Active;
		}
	}
	for(unsigned i=0; i<_planes.size(); i++)
	{
		if(scumm_stricmp(NodeName, _planes[i]->getName())==0)
		{
			return _planes[i]->m_Active;
		}
	}

	for(unsigned i=0; i<_generics.size(); i++)
	{
		if(scumm_stricmp(NodeName, _generics[i]->getName())==0)
		{
			return _generics[i]->m_Active;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::EnableLight(char* LightName, bool Enable)
{
	bool Ret = false;

	unsigned i;
	for(i=0; i<_lights.size(); i++)
	{
		if(scumm_stricmp(LightName, _lights[i]->getName())==0)
		{
			_lights[i]->m_Active = Enable;
			Ret = true;
		}
	}
	CreateLights();

	return Ret;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::IsLightEnabled(char* LightName)
{
	for(unsigned i=0; i<_lights.size(); i++)
	{
		if(scumm_stricmp(LightName, _lights[i]->getName())==0)
		{
			return _lights[i]->m_Active;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::SetLightColor(char* LightName, uint32 Color)
{
	bool Ret = false;

	unsigned i;
	for(i=0; i<_lights.size(); i++)
	{
		if(scumm_stricmp(LightName, _lights[i]->getName())==0)
		{
			_lights[i]->m_DiffuseColor = Color;
			Ret = true;
		}
	}
	CreateLights();

	return Ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 AdSceneGeometry::GetLightColor(char* LightName)
{
	for(unsigned i=0; i<_lights.size(); i++)
	{
		if(scumm_stricmp(LightName, _lights[i]->getName())==0)
		{
			return _lights[i]->m_DiffuseColor;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
Math::Vector3d AdSceneGeometry::GetLightPos(char* LightName)
{
	for(unsigned i=0; i<_lights.size(); i++)
	{
		if(scumm_stricmp(LightName, _lights[i]->getName())==0)
		{
			return _lights[i]->m_Position;
		}
	}
	return Math::Vector3d(0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::persist(BasePersistenceManager* PersistMgr)
{
	BaseObject::persist(PersistMgr);

	PersistMgr->transferFloat(TMEMBER(m_WaypointHeight));
	PersistMgr->transferPtr(TMEMBER(m_WptMarker));

	if(!PersistMgr->getIsSaving())
	{
		//m_WptMarker = NULL;
		LoadFile(getFilename());
		m_LastValuesInitialized = false;
	}

	PersistMgr->transferSint32(TMEMBER(m_ActiveCamera));
	PersistMgr->transferSint32(TMEMBER(m_ActiveLight));
	PersistMgr->transferMatrix4(TMEMBER(m_ViewMatrix));

	_PFPath.persist(PersistMgr);
	PersistMgr->transferBool(TMEMBER(_PFReady));
	PersistMgr->transferVector3d(TMEMBER(_PFSource));
	PersistMgr->transferVector3d(TMEMBER(_PFTarget));
	PersistMgr->transferVector3d(TMEMBER(_PFAlternateTarget));
	PersistMgr->transferPtr(TMEMBER(_PFTargetPath));
	PersistMgr->transferUint32(TMEMBER(_PFMaxTime));
	PersistMgr->transferBool(TMEMBER(_PFRerun));

	// now save/load light/blocks/walkplanes/generic node states by name
	int i;

	//////////////////////////////////////////////////////////////////////////
	int NumLights = _lights.size();
	PersistMgr->transferSint32(TMEMBER(NumLights));
	for(i=0; i<NumLights; i++)
	{
		if(PersistMgr->getIsSaving())
		{
			PersistMgr->transferCharPtr(TMEMBER(_lights[i]->_name));
			_lights[i]->Persist(PersistMgr);
		}
		else
		{
			char* Name = NULL;
			PersistMgr->transferCharPtr(TMEMBER(Name));
			bool Found = false;
			for(unsigned j=0; j<_lights.size(); j++)
			{
				if(scumm_stricmp(Name, _lights[j]->getName())==0)
				{
					_lights[j]->Persist(PersistMgr);
					Found = true;
					break;
				}
			}
			if(!Found)
			{
				Light3D* Light = new Light3D(_gameRef);
				Light->Persist(PersistMgr);
				delete Light;
			}

			if (Name)
			{
				delete[] Name;
			}
		}
	}
	CreateLights();


	//////////////////////////////////////////////////////////////////////////
	int NumBlocks = _blocks.size();
	PersistMgr->transferSint32(TMEMBER(NumBlocks));
	for(i=0; i<NumBlocks; i++)
	{
		if(PersistMgr->getIsSaving())
		{
			PersistMgr->transferCharPtr(TMEMBER(_blocks[i]->_name));
			_blocks[i]->persist(PersistMgr);
		}
		else
		{
			char* Name = NULL;
			PersistMgr->transferCharPtr(TMEMBER(Name));
			bool Found = false;
			for(unsigned j=0; j<_blocks.size(); j++)
			{
				if(scumm_stricmp(Name, _blocks[j]->getName())==0)
				{
					_blocks[j]->persist(PersistMgr);
					Found = true;
					break;
				}
			}
			if(!Found)
			{
				AdBlock* Block = new AdBlock(_gameRef);
				Block->persist(PersistMgr);
				delete Block;
			}

			if (Name)
			{
				delete[] Name;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int NumPlanes = _planes.size();
	PersistMgr->transferSint32(TMEMBER(NumPlanes));
	for(i=0; i<NumPlanes; i++)
	{
		if(PersistMgr->getIsSaving())
		{
			PersistMgr->transferCharPtr(TMEMBER(_planes[i]->_name));
			_planes[i]->Persist(PersistMgr);
		}
		else
		{
			char* Name = NULL;
			PersistMgr->transferCharPtr(TMEMBER(Name));
			bool Found = false;
			for(unsigned j=0; j<_planes.size(); j++)
			{
				if(scumm_stricmp(Name, _planes[j]->getName())==0)
				{
					_planes[j]->persist(PersistMgr);
					Found = true;
					break;
				}
			}
			if(!Found)
			{
				AdWalkplane* Plane = new AdWalkplane(_gameRef);
				Plane->Persist(PersistMgr);
				delete Plane;
			}

			if (Name)
			{
				delete[] Name;
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	int NumGenerics = _generics.size();
	PersistMgr->transferSint32(TMEMBER(NumGenerics));
	for(i=0; i<NumGenerics; i++)
	{
		if(PersistMgr->getIsSaving())
		{
			PersistMgr->transferCharPtr(TMEMBER(_generics[i]->_name));
			_generics[i]->persist(PersistMgr);
		}
		else
		{
			char* Name = NULL;
			PersistMgr->transferCharPtr(TMEMBER(Name));
			bool Found = false;
			for(unsigned j=0; j<_generics.size(); j++)
			{
				if(scumm_stricmp(Name, _generics[j]->getName())==0)
				{
					_generics[j]->persist(PersistMgr);
					Found = true;
					break;
				}
			}
			if(!Found)
			{
				AdGeneric* Generic = new AdGeneric(_gameRef);
				Generic->persist(PersistMgr);
				delete Generic;
			}

			if (Name)
			{
				delete[] Name;
			}
		}
	}

	if(!PersistMgr->getIsSaving()) m_MaxLightsWarning = false;

	return true;
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneGeometry::Convert3Dto2D(Math::Vector3d* Pos, int* X, int* Y)
{
	Math::Matrix4 worldMat;
	worldMat.setToIdentity();

	Math::Vector3d Vect2D;
	int viewportTmp[4];
	// TODO: gluMathProject expects an OpenGL viewport,
	// hence the first coordinates specify the lower left corner
	// wme works with a Direct3D viewport, though
	// so check if this does work
	viewportTmp[0] = m_DrawingViewport.left;
	viewportTmp[1] = m_DrawingViewport.bottom;
	viewportTmp[2] = m_DrawingViewport.width();
	viewportTmp[3] = m_DrawingViewport.height();
	Math::Matrix4 modelViewMatrix = m_LastViewMat * worldMat;
	Math::gluMathProject(*Pos, modelViewMatrix.getData(), m_LastProjMat.getData(), viewportTmp, Vect2D);
	*X = Vect2D.x() + m_LastScrollX;
	*Y = Vect2D.y() + m_LastScrollY;

	return true;
}

}
