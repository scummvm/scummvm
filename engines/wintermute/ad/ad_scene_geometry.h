// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_SCENE_GEOMETRY_H
#define WINTERMUTE_AD_SCENE_GEOMETRY_H

#include "../base/base_object.h"
#include "math/matrix4.h"
#include "math/vector3d.h"
#include "../math/rect32.h"

namespace Wintermute {

class BaseSprite;
class Camera3D;
class Light3D;
class Block3D;
class AdGeneric;
class AdBlock;
class AdWalkplane;
class AdPath3D;
class AdWaypointGroup3D;
class AdGeomExt;
class AdPathPoint3D;

class AdSceneGeometry : public BaseObject
{
public:
	bool m_MaxLightsWarning;
	bool DropWaypoints();
	bool SetLightColor(char* LightName, uint32 Color);
	uint32 GetLightColor(char* LightName);
	Math::Vector3d GetLightPos(char* LightName);
	bool EnableNode(char* NodeName, bool Enable=true);
	bool IsNodeEnabled(char* NodeName);
	bool EnableLight(char* LightName, bool Enable=true);
	bool IsLightEnabled(char* LightName);
	DECLARE_PERSISTENT(AdSceneGeometry, BaseObject);
	bool CorrectTargetPoint(Math::Vector3d Source, Math::Vector3d* Target);

	bool m_LastValuesInitialized;
	Math::Matrix4 m_LastWorldMat;
	Math::Matrix4 m_LastViewMat;
	Math::Matrix4 m_LastProjMat;
	int m_LastOffsetX;
	int m_LastOffsetY;
	Rect32 m_DrawingViewport;
	int m_LastScrollX;
	int m_LastScrollY;


	bool CreateLights();
	bool EnableLights(Math::Vector3d Point, BaseArray<char*>& IgnoreLights);
	static int CompareLights(const void* Obj1, const void* Obj2);
	
	bool InitLoop();
	float GetPointsDist(Math::Vector3d p1, Math::Vector3d p2);
	void PathFinderStep();
	bool GetPath(Math::Vector3d source, Math::Vector3d target, AdPath3D* path, bool Rerun=false);
	bool Convert2Dto3D(int X, int Y, Math::Vector3d* Pos);
	bool Convert2Dto3DTolerant(int X, int Y, Math::Vector3d *Pos);
	bool Convert3Dto2D(Math::Vector3d* Pos, int* X, int* Y);
	BaseSprite* m_WptMarker;
	float m_WaypointHeight;
	bool DirectPathExists(Math::Vector3d* p1, Math::Vector3d* p2);
	float GetHeightAt(Math::Vector3d Pos, float Tolerance=0.0f, bool* IntFound=NULL);

	bool StoreDrawingParams();
	bool Render(bool Render);
	bool RenderShadowGeometry();

	Math::Matrix4* GetViewMatrix();
	Math::Matrix4 m_ViewMatrix;
	bool SetActiveCamera(char* Camera, float FOV, float NearClipPlane, float FarClipPlane);
	bool SetActiveCamera(int Camera, float FOV, float NearClipPlane, float FarClipPlane);
	//bool SetActiveCameraTwin(char* Camera);
	//bool SetActiveCameraTwin(int Camera);
	Camera3D* GetActiveCamera();
	int m_ActiveCamera;

	bool SetActiveLight(char* Light);
	bool SetActiveLight(int Light);
	int m_ActiveLight;

	void Cleanup();
	AdSceneGeometry(BaseGame* inGame);
	virtual ~AdSceneGeometry();
	bool LoadFile(const char* Filename);
	BaseArray<AdWalkplane*> _planes;
	BaseArray<AdBlock*> _blocks;
	BaseArray<AdGeneric*> _generics;
	BaseArray<Camera3D*> _cameras;
	BaseArray<Light3D*> _lights;
	BaseArray<AdWaypointGroup3D*> _waypointGroups;
	uint32 _PFMaxTime;

private:
	AdGeomExt* GetGeometryExtension(char* Filename);
	Math::Vector3d GetBlockIntersection(Math::Vector3d* p1, Math::Vector3d* p2);
	bool _PFReady;
	Math::Vector3d _PFSource;
	Math::Vector3d _PFTarget;
	AdPath3D* _PFTargetPath;
	Math::Vector3d _PFAlternateTarget;
	float _PFAlternateDist;
	bool _PFRerun;
	BaseArray<AdPathPoint3D*> _PFPath;

};

}

#endif
