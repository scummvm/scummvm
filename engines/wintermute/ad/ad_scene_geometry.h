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

#ifndef WINTERMUTE_AD_SCENE_GEOMETRY_H
#define WINTERMUTE_AD_SCENE_GEOMETRY_H

#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/math/rect32.h"

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
	DXVector3 getLightPos(const char *lightName);
	bool enableNode(const char *nodeName, bool enable = true);
	bool isNodeEnabled(const char *nodeName);
	bool enableLight(const char *lightName, bool enable = true);
	bool isLightEnabled(const char *lightName);
	DECLARE_PERSISTENT(AdSceneGeometry, BaseObject)
	bool correctTargetPoint(const DXVector3 &source, DXVector3 *target);

	bool _lastValuesInitialized;
	DXMatrix _lastWorldMat;
	DXMatrix _lastViewMat;
	DXMatrix _lastProjMat;
	int _lastOffsetX;
	int _lastOffsetY;
	DXViewport _drawingViewport;
	int _lastScrollX;
	int _lastScrollY;

	bool createLights();
	bool enableLights(DXVector3 Point, BaseArray<char *> &IgnoreLights);
	static int compareLights(const void *obj1, const void *obj2);

	bool initLoop();
	float getPointsDist(DXVector3 p1, DXVector3 p2);
	void pathFinderStep();
	bool getPath(DXVector3 source, DXVector3 target, AdPath3D *path, bool rerun = false);
	bool convert2Dto3D(int x, int y, DXVector3 *pos);
	bool convert2Dto3DTolerant(int x, int y, DXVector3 *pos);
	bool convert3Dto2D(DXVector3 *pos, int32 *x, int32 *y);
	BaseSprite *_wptMarker;
	float _waypointHeight;
	bool directPathExists(DXVector3 *p1, DXVector3 *p2);
	float getHeightAt(DXVector3 pos, float Ttlerance = 0.0f, bool *intFound = NULL);

	bool storeDrawingParams();
	bool render(bool render);
	bool renderShadowGeometry();

	DXMatrix *getViewMatrix();
	DXMatrix _viewMatrix;
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
	DXVector3 getBlockIntersection(DXVector3 *p1, DXVector3 *p2);
	bool _PFReady;
	DXVector3 _PFSource;
	DXVector3 _PFTarget;
	AdPath3D *_PFTargetPath;
	DXVector3 _PFAlternateTarget;
	float _PFAlternateDist;
	bool _PFRerun;
	BaseArray<AdPathPoint3D *> _PFPath;
};

} // namespace Wintermute

#endif
