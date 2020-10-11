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

#ifndef WINTERMUTE_AD_SCENE_GEOMETRY_H
#define WINTERMUTE_AD_SCENE_GEOMETRY_H

#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/math/rect32.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

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

class AdSceneGeometry : public BaseObject {
public:
	bool _maxLightsWarning;
	bool dropWaypoints();
	bool setLightColor(const char *lightName, uint32 color);
	uint32 getLightColor(const char *lightName);
	Math::Vector3d getLightPos(const char *lightName);
	bool enableNode(const char *nodeName, bool enable = true);
	bool isNodeEnabled(const char *nodeName);
	bool enableLight(const char *lightName, bool enable = true);
	bool isLightEnabled(const char *lightName);
	DECLARE_PERSISTENT(AdSceneGeometry, BaseObject)
	bool correctTargetPoint(const Math::Vector3d &source, Math::Vector3d *target);

	bool _lastValuesInitialized;
	Math::Matrix4 _lastWorldMat;
	Math::Matrix4 _lastViewMat;
	Math::Matrix4 _lastProjMat;
	int _lastOffsetX;
	int _lastOffsetY;
	Rect32 _drawingViewport;
	int _lastScrollX;
	int _lastScrollY;

	bool createLights();
	bool enableLights(Math::Vector3d Point, BaseArray<char *> &IgnoreLights);

	bool initLoop();
	float getPointsDist(Math::Vector3d p1, Math::Vector3d p2);
	void pathFinderStep();
	bool getPath(Math::Vector3d source, Math::Vector3d target, AdPath3D *path, bool rerun = false);
	bool convert2Dto3D(int x, int y, Math::Vector3d *pos);
	bool convert2Dto3DTolerant(int x, int y, Math::Vector3d *pos);
	bool convert3Dto2D(Math::Vector3d *pos, int32 *x, int32 *y);
	BaseSprite *_wptMarker;
	float _waypointHeight;
	bool directPathExists(Math::Vector3d *p1, Math::Vector3d *p2);
	float getHeightAt(Math::Vector3d pos, float Ttlerance = 0.0f, bool *intFound = NULL);

	bool storeDrawingParams();
	bool render(bool render);
	bool renderShadowGeometry();

	Math::Matrix4 *getViewMatrix();
	Math::Matrix4 _viewMatrix;
	bool setActiveCamera(const char *camera, float fov, float nearClipPlane, float farClipPlane);
	bool setActiveCamera(int camera, float fow, float nearClipPlane, float farClipPlane);
	//bool SetActiveCameraTwin(char* Camera);
	//bool SetActiveCameraTwin(int Camera);
	Camera3D *getActiveCamera();
	int32 _activeCamera;

	bool setActiveLight(char *light);
	bool setActiveLight(int light);
	int32 _activeLight;

	void cleanup();
	AdSceneGeometry(BaseGame *inGame);
	virtual ~AdSceneGeometry();
	bool loadFile(const char *filename);
	BaseArray<AdWalkplane *> _planes;
	BaseArray<AdBlock *> _blocks;
	BaseArray<AdGeneric *> _generics;
	BaseArray<Camera3D *> _cameras;
	BaseArray<Light3D *> _lights;
	BaseArray<AdWaypointGroup3D *> _waypointGroups;
	uint32 _PFMaxTime;

private:
	AdGeomExt *getGeometryExtension(char *filename);
	Math::Vector3d getBlockIntersection(Math::Vector3d *p1, Math::Vector3d *p2);
	bool _PFReady;
	Math::Vector3d _PFSource;
	Math::Vector3d _PFTarget;
	AdPath3D *_PFTargetPath;
	Math::Vector3d _PFAlternateTarget;
	float _PFAlternateDist;
	bool _PFRerun;
	BaseArray<AdPathPoint3D *> _PFPath;
};

} // namespace Wintermute

#endif
