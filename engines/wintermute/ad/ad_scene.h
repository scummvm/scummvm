/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_ADSCENE_H
#define WINTERMUTE_ADSCENE_H

#include "engines/wintermute/base/base_fader.h"

namespace WinterMute {

class UIWindow;
class AdObject;
class AdRegion;
class BaseViewport;
class AdLayer;
class BasePoint;
class AdWaypointGroup;
class AdPath;
class AdScaleLevel;
class AdRotLevel;
class AdPathPoint;
class AdScene : public BaseObject {
public:

	BaseObject *getNextAccessObject(BaseObject *CurrObject);
	BaseObject *getPrevAccessObject(BaseObject *CurrObject);
	bool getSceneObjects(BaseArray<AdObject *, AdObject *> &Objects, bool InteractiveOnly);
	bool getRegionObjects(AdRegion *Region, BaseArray<AdObject *, AdObject *> &Objects, bool InteractiveOnly);

	bool afterLoad();

	bool getRegionsAt(int X, int Y, AdRegion **RegionList, int NumRegions);
	bool handleItemAssociations(const char *ItemName, bool Show);
	UIWindow *_shieldWindow;
	float getRotationAt(int X, int Y);
	bool loadState();
	bool saveState();
	bool _persistentState;
	bool _persistentStateSprites;
	BaseObject *getNodeByName(const char *name);
	void setOffset(int OffsetLeft, int OffsetTop);
	bool pointInViewport(int X, int Y);
	int getOffsetTop();
	int getOffsetLeft();
	bool getViewportSize(int *Width = NULL, int *Height = NULL);
	bool getViewportOffset(int *OffsetX = NULL, int *OffsetY = NULL);
	BaseViewport *_viewport;
	BaseFader *_fader;
	int _pfPointsNum;
	void pfPointsAdd(int X, int Y, int Distance);
	void pfPointsStart();
	bool _initialized;
	bool correctTargetPoint(int StartX, int StartY, int *X, int *Y, bool CheckFreeObjects = false, BaseObject *Requester = NULL);
	bool correctTargetPoint2(int StartX, int StartY, int *TargetX, int *TargetY, bool CheckFreeObjects, BaseObject *Requester);
	DECLARE_PERSISTENT(AdScene, BaseObject)
	bool displayRegionContent(AdRegion *Region = NULL, bool Display3DOnly = false);
	bool displayRegionContentOld(AdRegion *Region = NULL);
	static int compareObjs(const void *Obj1, const void *Obj2);

	bool updateFreeObjects();
	bool traverseNodes(bool Update = false);
	float getScaleAt(int Y);
	bool sortScaleLevels();
	bool sortRotLevels();
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent);
	uint32 getAlphaAt(int X, int Y, bool ColorCheck = false);
	bool _paralaxScrolling;
	void skipTo(int OffsetX, int OffsetY);
	void setDefaults();
	void cleanup();
	void skipToObject(BaseObject *Object);
	void scrollToObject(BaseObject *Object);
	void scrollTo(int OffsetX, int OffsetY);
	virtual bool update();
	bool _autoScroll;
	int _targetOffsetTop;
	int _targetOffsetLeft;

	int _scrollPixelsV;
	uint32 _scrollTimeV;
	uint32 _lastTimeV;

	int _scrollPixelsH;
	uint32 _scrollTimeH;
	uint32 _lastTimeH;

	virtual bool display();
	uint32 _pfMaxTime;
	bool initLoop();
	void pathFinderStep();
	bool isBlockedAt(int X, int Y, bool CheckFreeObjects = false, BaseObject *Requester = NULL);
	bool isWalkableAt(int X, int Y, bool CheckFreeObjects = false, BaseObject *Requester = NULL);
	AdLayer *_mainLayer;
	float getZoomAt(int X, int Y);
	bool getPath(BasePoint source, BasePoint target, AdPath *path, BaseObject *requester = NULL);
	AdScene(BaseGame *inGame);
	virtual ~AdScene();
	BaseArray<AdLayer *, AdLayer *> _layers;
	BaseArray<AdObject *, AdObject *> _objects;
	BaseArray<AdWaypointGroup *, AdWaypointGroup *> _waypointGroups;
	bool loadFile(const char *filename);
	bool loadBuffer(byte *buffer, bool complete = true);
	int _width;
	int _height;
	bool addObject(AdObject *Object);
	bool removeObject(AdObject *Object);
	int _editorMarginH;
	int _editorMarginV;
	uint32 _editorColFrame;
	uint32 _editorColEntity;
	uint32 _editorColRegion;
	uint32 _editorColBlocked;
	uint32 _editorColWaypoints;
	uint32 _editorColEntitySel;
	uint32 _editorColRegionSel;
	uint32 _editorColBlockedSel;
	uint32 _editorColWaypointsSel;
	uint32 _editorColScale;
	uint32 _editorColDecor;
	uint32 _editorColDecorSel;

	bool _editorShowRegions;
	bool _editorShowBlocked;
	bool _editorShowDecor;
	bool _editorShowEntities;
	bool _editorShowScale;
	BaseArray<AdScaleLevel *, AdScaleLevel *> _scaleLevels;
	BaseArray<AdRotLevel *, AdRotLevel *> _rotLevels;

	virtual bool restoreDeviceObjects();
	int getPointsDist(BasePoint p1, BasePoint p2, BaseObject *requester = NULL);

	// scripting interface
	virtual ScValue *scGetProperty(const char *name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();


private:
	bool persistState(bool Saving = true);
	void pfAddWaypointGroup(AdWaypointGroup *Wpt, BaseObject *Requester = NULL);
	bool _pfReady;
	BasePoint *_pfTarget;
	AdPath *_pfTargetPath;
	BaseObject *_pfRequester;
	BaseArray<AdPathPoint *, AdPathPoint *> _pfPath;

	int _offsetTop;
	int _offsetLeft;

};

} // end of namespace WinterMute

#endif
