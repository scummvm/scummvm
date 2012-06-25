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

#include "engines/wintermute/Base/BFader.h"

namespace WinterMute {

class CUIWindow;
class CAdObject;
class CAdRegion;
class CBViewport;
class CAdLayer;
class CBPoint;
class CAdWaypointGroup;
class CAdPath;
class CAdScaleLevel;
class CAdRotLevel;
class CAdPathPoint;
class CAdScene : public CBObject {
public:

	CBObject *GetNextAccessObject(CBObject *CurrObject);
	CBObject *GetPrevAccessObject(CBObject *CurrObject);
	HRESULT GetSceneObjects(CBArray<CAdObject *, CAdObject *> &Objects, bool InteractiveOnly);
	HRESULT GetRegionObjects(CAdRegion *Region, CBArray<CAdObject *, CAdObject *> &Objects, bool InteractiveOnly);

	HRESULT AfterLoad();

	HRESULT GetRegionsAt(int X, int Y, CAdRegion **RegionList, int NumRegions);
	HRESULT HandleItemAssociations(const char *ItemName, bool Show);
	CUIWindow *_shieldWindow;
	float GetRotationAt(int X, int Y);
	HRESULT LoadState();
	HRESULT SaveState();
	bool _persistentState;
	bool _persistentStateSprites;
	CBObject *GetNodeByName(const char *Name);
	void SetOffset(int OffsetLeft, int OffsetTop);
	bool PointInViewport(int X, int Y);
	int GetOffsetTop();
	int GetOffsetLeft();
	HRESULT GetViewportSize(int *Width = NULL, int *Height = NULL);
	HRESULT GetViewportOffset(int *OffsetX = NULL, int *OffsetY = NULL);
	CBViewport *_viewport;
	CBFader *_fader;
	int _pFPointsNum;
	void PFPointsAdd(int X, int Y, int Distance);
	void PFPointsStart();
	bool _initialized;
	HRESULT CorrectTargetPoint(int StartX, int StartY, int *X, int *Y, bool CheckFreeObjects = false, CBObject *Requester = NULL);
	HRESULT CorrectTargetPoint2(int StartX, int StartY, int *TargetX, int *TargetY, bool CheckFreeObjects, CBObject *Requester);
	DECLARE_PERSISTENT(CAdScene, CBObject)
	HRESULT DisplayRegionContent(CAdRegion *Region = NULL, bool Display3DOnly = false);
	HRESULT DisplayRegionContentOld(CAdRegion *Region = NULL);
	static int CompareObjs(const void *Obj1, const void *Obj2);

	HRESULT UpdateFreeObjects();
	HRESULT TraverseNodes(bool Update = false);
	float GetScaleAt(int Y);
	HRESULT SortScaleLevels();
	HRESULT SortRotLevels();
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);
	uint32 GetAlphaAt(int X, int Y, bool ColorCheck = false);
	bool _paralaxScrolling;
	void SkipTo(int OffsetX, int OffsetY);
	void SetDefaults();
	void Cleanup();
	void SkipToObject(CBObject *Object);
	void ScrollToObject(CBObject *Object);
	void ScrollTo(int OffsetX, int OffsetY);
	virtual HRESULT Update();
	bool _autoScroll;
	int _targetOffsetTop;
	int _targetOffsetLeft;

	int _scrollPixelsV;
	uint32 _scrollTimeV;
	uint32 _lastTimeV;

	int _scrollPixelsH;
	uint32 _scrollTimeH;
	uint32 _lastTimeH;

	virtual HRESULT Display();
	uint32 _pFMaxTime;
	HRESULT InitLoop();
	void PathFinderStep();
	bool IsBlockedAt(int X, int Y, bool CheckFreeObjects = false, CBObject *Requester = NULL);
	bool IsWalkableAt(int X, int Y, bool CheckFreeObjects = false, CBObject *Requester = NULL);
	CAdLayer *_mainLayer;
	float GetZoomAt(int X, int Y);
	bool GetPath(CBPoint source, CBPoint target, CAdPath *path, CBObject *requester = NULL);
	CAdScene(CBGame *inGame);
	virtual ~CAdScene();
	CBArray<CAdLayer *, CAdLayer *> _layers;
	CBArray<CAdObject *, CAdObject *> _objects;
	CBArray<CAdWaypointGroup *, CAdWaypointGroup *> _waypointGroups;
	HRESULT LoadFile(const char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);
	int _width;
	int _height;
	HRESULT AddObject(CAdObject *Object);
	HRESULT RemoveObject(CAdObject *Object);
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
	CBArray<CAdScaleLevel *, CAdScaleLevel *> _scaleLevels;
	CBArray<CAdRotLevel *, CAdRotLevel *> _rotLevels;

	virtual HRESULT RestoreDeviceObjects();
	int GetPointsDist(CBPoint p1, CBPoint p2, CBObject *requester = NULL);

	// scripting interface
	virtual CScValue *scGetProperty(const char *Name);
	virtual HRESULT scSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual const char *scToString();


private:
	HRESULT PersistState(bool Saving = true);
	void PFAddWaypointGroup(CAdWaypointGroup *Wpt, CBObject *Requester = NULL);
	bool _pFReady;
	CBPoint *_pFTarget;
	CAdPath *_pFTargetPath;
	CBObject *_pFRequester;
	CBArray<CAdPathPoint *, CAdPathPoint *> _pFPath;

	int _offsetTop;
	int _offsetLeft;

};

} // end of namespace WinterMute

#endif
