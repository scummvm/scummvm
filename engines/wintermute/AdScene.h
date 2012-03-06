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

#ifndef __WmeAdScene_H__
#define __WmeAdScene_H__

#include "BFader.h"

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
	HRESULT GetSceneObjects(CBArray<CAdObject *, CAdObject *>& Objects, bool InteractiveOnly);
	HRESULT GetRegionObjects(CAdRegion *Region, CBArray<CAdObject *, CAdObject *>& Objects, bool InteractiveOnly);

	HRESULT AfterLoad();

	HRESULT GetRegionsAt(int X, int Y, CAdRegion **RegionList, int NumRegions);
	HRESULT HandleItemAssociations(char *ItemName, bool Show);
	CUIWindow *m_ShieldWindow;
	float GetRotationAt(int X, int Y);
	HRESULT LoadState();
	HRESULT SaveState();
	bool m_PersistentState;
	bool m_PersistentStateSprites;
	CBObject *GetNodeByName(char *Name);
	void SetOffset(int OffsetLeft, int OffsetTop);
	bool PointInViewport(int X, int Y);
	int GetOffsetTop();
	int GetOffsetLeft();
	HRESULT GetViewportSize(int *Width = NULL, int *Height = NULL);
	HRESULT GetViewportOffset(int *OffsetX = NULL, int *OffsetY = NULL);
	CBViewport *m_Viewport;
	CBFader *m_Fader;
	int m_PFPointsNum;
	void PFPointsAdd(int X, int Y, int Distance);
	void PFPointsStart();
	bool m_Initialized;
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
	bool m_ParalaxScrolling;
	void SkipTo(int OffsetX, int OffsetY);
	void SetDefaults();
	void Cleanup();
	void SkipToObject(CBObject *Object);
	void ScrollToObject(CBObject *Object);
	void ScrollTo(int OffsetX, int OffsetY);
	virtual HRESULT Update();
	bool m_AutoScroll;
	int m_TargetOffsetTop;
	int m_TargetOffsetLeft;

	int m_ScrollPixelsV;
	uint32 m_ScrollTimeV;
	uint32 m_LastTimeV;

	int m_ScrollPixelsH;
	uint32 m_ScrollTimeH;
	uint32 m_LastTimeH;

	virtual HRESULT Display();
	uint32 m_PFMaxTime;
	HRESULT InitLoop();
	void PathFinderStep();
	bool IsBlockedAt(int X, int Y, bool CheckFreeObjects = false, CBObject *Requester = NULL);
	bool IsWalkableAt(int X, int Y, bool CheckFreeObjects = false, CBObject *Requester = NULL);
	CAdLayer *m_MainLayer;
	float GetZoomAt(int X, int Y);
	bool GetPath(CBPoint source, CBPoint target, CAdPath *path, CBObject *requester = NULL);
	CAdScene(CBGame *inGame);
	virtual ~CAdScene();
	CBArray<CAdLayer *, CAdLayer *> m_Layers;
	CBArray<CAdObject *, CAdObject *> m_Objects;
	CBArray<CAdWaypointGroup *, CAdWaypointGroup *> m_WaypointGroups;
	HRESULT LoadFile(char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);
	int m_Width;
	int m_Height;
	HRESULT AddObject(CAdObject *Object);
	HRESULT RemoveObject(CAdObject *Object);
	int m_EditorMarginH;
	int m_EditorMarginV;
	uint32 m_EditorColFrame;
	uint32 m_EditorColEntity;
	uint32 m_EditorColRegion;
	uint32 m_EditorColBlocked;
	uint32 m_EditorColWaypoints;
	uint32 m_EditorColEntitySel;
	uint32 m_EditorColRegionSel;
	uint32 m_EditorColBlockedSel;
	uint32 m_EditorColWaypointsSel;
	uint32 m_EditorColScale;
	uint32 m_EditorColDecor;
	uint32 m_EditorColDecorSel;

	bool m_EditorShowRegions;
	bool m_EditorShowBlocked;
	bool m_EditorShowDecor;
	bool m_EditorShowEntities;
	bool m_EditorShowScale;
	CBArray<CAdScaleLevel *, CAdScaleLevel *> m_ScaleLevels;
	CBArray<CAdRotLevel *, CAdRotLevel *> m_RotLevels;

	virtual HRESULT RestoreDeviceObjects();
	int GetPointsDist(CBPoint p1, CBPoint p2, CBObject *requester = NULL);

	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	virtual char *ScToString();


private:
	HRESULT PersistState(bool Saving = true);
	void PFAddWaypointGroup(CAdWaypointGroup *Wpt, CBObject *Requester = NULL);
	bool m_PFReady;
	CBPoint *m_PFTarget;
	CAdPath *m_PFTargetPath;
	CBObject *m_PFRequester;
	CBArray<CAdPathPoint *, CAdPathPoint *> m_PFPath;

	int m_OffsetTop;
	int m_OffsetLeft;

};

} // end of namespace WinterMute

#endif
