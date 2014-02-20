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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

namespace Wintermute {

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

	BaseObject *getNextAccessObject(BaseObject *currObject);
	BaseObject *getPrevAccessObject(BaseObject *currObject);
	bool getSceneObjects(BaseArray<AdObject *> &objects, bool interactiveOnly);
	bool getRegionObjects(AdRegion *region, BaseArray<AdObject *> &objects, bool interactiveOnly);

	bool afterLoad();

	bool getRegionsAt(int x, int y, AdRegion **regionList, int numRegions);
	bool handleItemAssociations(const char *itemName, bool show);
	UIWindow *_shieldWindow;
	float getRotationAt(int x, int y);
	bool loadState();
	bool saveState();
	bool _persistentState;
	bool _persistentStateSprites;
	BaseObject *getNodeByName(const char *name);
	void setOffset(int offsetLeft, int offsetTop);
	bool pointInViewport(int x, int y);
	int getOffsetTop();
	int getOffsetLeft();
	bool getViewportSize(int32 *width = nullptr, int32 *height = nullptr);
	bool getViewportOffset(int32 *offsetX = nullptr, int32 *offsetY = nullptr);
	BaseViewport *_viewport;
	BaseFader *_fader;
	int32 _pfPointsNum;
	void pfPointsAdd(int x, int y, int distance);
	void pfPointsStart();
	bool _initialized;
	bool correctTargetPoint(int32 startX, int32 startY, int32 *x, int32 *y, bool checkFreeObjects = false, BaseObject *requester = nullptr);
	bool correctTargetPoint2(int32 startX, int32 startY, int32 *targetX, int32 *targetY, bool checkFreeObjects, BaseObject *requester);
	DECLARE_PERSISTENT(AdScene, BaseObject)
	bool displayRegionContent(AdRegion *region = nullptr, bool display3DOnly = false);
	bool displayRegionContentOld(AdRegion *region = nullptr);
	static bool compareObjs(const AdObject *obj1, const AdObject *obj2);

	bool updateFreeObjects();
	bool traverseNodes(bool update = false);
	float getScaleAt(int y);
	bool sortScaleLevels();
	bool sortRotLevels();
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;
	uint32 getAlphaAt(int x, int y, bool colorCheck = false);
	bool _paralaxScrolling;
	void skipTo(int offsetX, int offsetY);
	void setDefaults();
	void cleanup();
	void skipToObject(BaseObject *object);
	void scrollToObject(BaseObject *object);
	void scrollTo(int offsetX, int offsetY);
	virtual bool update() override;
	bool _autoScroll;
	int32 _targetOffsetTop;
	int32 _targetOffsetLeft;

	int32 _scrollPixelsV;
	uint32 _scrollTimeV;
	uint32 _lastTimeV;

	int32 _scrollPixelsH;
	uint32 _scrollTimeH;
	uint32 _lastTimeH;

	virtual bool display();
	uint32 _pfMaxTime;
	bool initLoop();
	void pathFinderStep();
	bool isBlockedAt(int x, int y, bool checkFreeObjects = false, BaseObject *requester = nullptr);
	bool isWalkableAt(int x, int y, bool checkFreeObjects = false, BaseObject *requester = nullptr);
	AdLayer *_mainLayer;
	float getZoomAt(int x, int y);
	bool getPath(const BasePoint &source, const BasePoint &target, AdPath *path, BaseObject *requester = nullptr);
	AdScene(BaseGame *inGame);
	virtual ~AdScene();
	BaseArray<AdLayer *> _layers;
	BaseArray<AdObject *> _objects;
	BaseArray<AdWaypointGroup *> _waypointGroups;
	bool loadFile(const char *filename);
	bool loadBuffer(char *buffer, bool complete = true);
	int32 _width;
	int32 _height;
	bool addObject(AdObject *Object);
	bool removeObject(AdObject *Object);
	int32 _editorMarginH;
	int32 _editorMarginV;
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
	BaseArray<AdScaleLevel *> _scaleLevels;
	BaseArray<AdRotLevel *> _rotLevels;

	virtual bool restoreDeviceObjects();
	int getPointsDist(const BasePoint &p1, const BasePoint &p2, BaseObject *requester = nullptr);

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name) override;
	virtual bool scSetProperty(const char *name, ScValue *value) override;
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	virtual const char *scToString() override;


private:
	bool persistState(bool saving = true);
	void pfAddWaypointGroup(AdWaypointGroup *Wpt, BaseObject *requester = nullptr);
	bool _pfReady;
	BasePoint *_pfTarget;
	AdPath *_pfTargetPath;
	BaseObject *_pfRequester;
	BaseArray<AdPathPoint *> _pfPath;

	int32 _offsetTop;
	int32 _offsetLeft;

};

} // End of namespace Wintermute

#endif
