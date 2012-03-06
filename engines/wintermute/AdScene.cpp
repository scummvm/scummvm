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

#include "AdScene.h"
#include "AdActor.h"
#include "AdEntity.h"
#include "AdGame.h"
#include "AdLayer.h"
#include "AdNodeState.h"
#include "AdObject.h"
#include "AdPath.h"
#include "AdPathPoint.h"
#include "AdRotLevel.h"
#include "AdScaleLevel.h"
#include "AdSceneNode.h"
#include "AdSceneState.h"
#include "AdSentence.h"
#include "AdWaypointGroup.h"
#include "BDynBuffer.h"
#include "BFileManager.h"
#include "BFont.h"
#include "BGame.h"
#include "BObject.h"
#include "BParser.h"
#include "BPoint.h"
#include "BRegion.h"
#include "BScriptable.h"
#include "BSprite.h"
#include "BViewport.h"
#include "PlatformSDL.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "UIWindow.h"
#include "utils.h"
#include <math.h>
#include <limits.h>

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdScene, false)

//////////////////////////////////////////////////////////////////////////
CAdScene::CAdScene(CBGame *inGame): CBObject(inGame) {
	m_PFTarget = new CBPoint;
	SetDefaults();
}


//////////////////////////////////////////////////////////////////////////
CAdScene::~CAdScene() {
	Cleanup();
	Game->UnregisterObject(m_Fader);
	delete m_PFTarget;
	m_PFTarget = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::SetDefaults() {
	m_Initialized = false;
	m_PFReady = true;
	m_PFTargetPath = NULL;
	m_PFRequester = NULL;
	m_MainLayer = NULL;

	m_PFPointsNum = 0;
	m_PersistentState = false;
	m_PersistentStateSprites = true;

	m_AutoScroll = true;
	m_OffsetLeft = m_OffsetTop = 0;
	m_TargetOffsetLeft = m_TargetOffsetTop = 0;

	m_LastTimeH = m_LastTimeV = 0;
	m_ScrollTimeH = m_ScrollTimeV = 10;
	m_ScrollPixelsH = m_ScrollPixelsV = 1;

	m_PFMaxTime = 15;

	m_ParalaxScrolling = true;

	// editor settings
	m_EditorMarginH = m_EditorMarginV = 100;

	m_EditorColFrame        = 0xE0888888;
	m_EditorColEntity       = 0xFF008000;
	m_EditorColRegion       = 0xFF0000FF;
	m_EditorColBlocked      = 0xFF800080;
	m_EditorColWaypoints    = 0xFF0000FF;
	m_EditorColEntitySel    = 0xFFFF0000;
	m_EditorColRegionSel    = 0xFFFF0000;
	m_EditorColBlockedSel   = 0xFFFF0000;
	m_EditorColWaypointsSel = 0xFFFF0000;
	m_EditorColScale        = 0xFF00FF00;
	m_EditorColDecor        = 0xFF00FFFF;
	m_EditorColDecorSel     = 0xFFFF0000;

	m_EditorShowRegions  = true;
	m_EditorShowBlocked  = true;
	m_EditorShowDecor    = true;
	m_EditorShowEntities = true;
	m_EditorShowScale    = true;

	m_ShieldWindow = NULL;

	m_Fader = new CBFader(Game);
	Game->RegisterObject(m_Fader);

	m_Viewport = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::Cleanup() {
	CBObject::Cleanup();

	m_MainLayer = NULL; // reference only

	int i;

	delete m_ShieldWindow;
	m_ShieldWindow = NULL;

	Game->UnregisterObject(m_Fader);
	m_Fader = NULL;

	for (i = 0; i < m_Layers.GetSize(); i++)
		Game->UnregisterObject(m_Layers[i]);
	m_Layers.RemoveAll();


	for (i = 0; i < m_WaypointGroups.GetSize(); i++)
		Game->UnregisterObject(m_WaypointGroups[i]);
	m_WaypointGroups.RemoveAll();

	for (i = 0; i < m_ScaleLevels.GetSize(); i++)
		Game->UnregisterObject(m_ScaleLevels[i]);
	m_ScaleLevels.RemoveAll();

	for (i = 0; i < m_RotLevels.GetSize(); i++)
		Game->UnregisterObject(m_RotLevels[i]);
	m_RotLevels.RemoveAll();


	for (i = 0; i < m_PFPath.GetSize(); i++)
		delete m_PFPath[i];
	m_PFPath.RemoveAll();
	m_PFPointsNum = 0;

	for (i = 0; i < m_Objects.GetSize(); i++)
		Game->UnregisterObject(m_Objects[i]);
	m_Objects.RemoveAll();

	delete m_Viewport;
	m_Viewport = NULL;

	SetDefaults();
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::GetPath(CBPoint source, CBPoint target, CAdPath *path, CBObject *requester) {
	if (!m_PFReady) return false;
	else {
		m_PFReady = false;
		*m_PFTarget = target;
		m_PFTargetPath = path;
		m_PFRequester = requester;

		m_PFTargetPath->Reset();
		m_PFTargetPath->SetReady(false);

		// prepare working path
		int i;
		PFPointsStart();

		// first point
		//m_PFPath.Add(new CAdPathPoint(source.x, source.y, 0));

		// if we're one pixel stuck, get unstuck
		int StartX = source.x;
		int StartY = source.y;
		int BestDistance = 1000;
		if (IsBlockedAt(StartX, StartY, true, requester)) {
			int Tolerance = 2;
			for (int xxx = StartX - Tolerance; xxx <= StartX + Tolerance; xxx++) {
				for (int yyy = StartY - Tolerance; yyy <= StartY + Tolerance; yyy++) {
					if (IsWalkableAt(xxx, yyy, true, requester)) {
						int Distance = abs(xxx - source.x) + abs(yyy - source.y);
						if (Distance < BestDistance) {
							StartX = xxx;
							StartY = yyy;

							BestDistance = Distance;
						}
					}
				}
			}
		}

		PFPointsAdd(StartX, StartY, 0);

		//CorrectTargetPoint(&target.x, &target.y);

		// last point
		//m_PFPath.Add(new CAdPathPoint(target.x, target.y, INT_MAX));
		PFPointsAdd(target.x, target.y, INT_MAX);

		// active waypoints
		for (i = 0; i < m_WaypointGroups.GetSize(); i++) {
			if (m_WaypointGroups[i]->m_Active) {
				PFAddWaypointGroup(m_WaypointGroups[i], requester);
			}
		}


		// free waypoints
		for (i = 0; i < m_Objects.GetSize(); i++) {
			if (m_Objects[i]->m_Active && m_Objects[i] != requester && m_Objects[i]->m_CurrentWptGroup) {
				PFAddWaypointGroup(m_Objects[i]->m_CurrentWptGroup, requester);
			}
		}
		CAdGame *AdGame = (CAdGame *)Game;
		for (i = 0; i < AdGame->m_Objects.GetSize(); i++) {
			if (AdGame->m_Objects[i]->m_Active && AdGame->m_Objects[i] != requester && AdGame->m_Objects[i]->m_CurrentWptGroup) {
				PFAddWaypointGroup(AdGame->m_Objects[i]->m_CurrentWptGroup, requester);
			}
		}

		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::PFAddWaypointGroup(CAdWaypointGroup *Wpt, CBObject *Requester) {
	if (!Wpt->m_Active) return;

	for (int i = 0; i < Wpt->m_Points.GetSize(); i++) {
		if (IsBlockedAt(Wpt->m_Points[i]->x, Wpt->m_Points[i]->y, true, Requester)) continue;

		//m_PFPath.Add(new CAdPathPoint(Wpt->m_Points[i]->x, Wpt->m_Points[i]->y, INT_MAX));
		PFPointsAdd(Wpt->m_Points[i]->x, Wpt->m_Points[i]->y, INT_MAX);
	}
}


//////////////////////////////////////////////////////////////////////////
float CAdScene::GetZoomAt(int X, int Y) {
	float ret = 100;

	bool found = false;
	if (m_MainLayer) {
		for (int i = m_MainLayer->m_Nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *Node = m_MainLayer->m_Nodes[i];
			if (Node->m_Type == OBJECT_REGION && Node->m_Region->m_Active && !Node->m_Region->m_Blocked && Node->m_Region->PointInRegion(X, Y)) {
				if (Node->m_Region->m_Zoom != 0) {
					ret = Node->m_Region->m_Zoom;
					found = true;
					break;
				}
			}
		}
	}
	if (!found) ret = GetScaleAt(Y);

	return ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 CAdScene::GetAlphaAt(int X, int Y, bool ColorCheck) {
	if (!Game->m_DEBUG_DebugMode) ColorCheck = false;

	uint32 ret;
	if (ColorCheck) ret = 0xFFFF0000;
	else ret = 0xFFFFFFFF;

	if (m_MainLayer) {
		for (int i = m_MainLayer->m_Nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *Node = m_MainLayer->m_Nodes[i];
			if (Node->m_Type == OBJECT_REGION && Node->m_Region->m_Active && (ColorCheck || !Node->m_Region->m_Blocked) && Node->m_Region->PointInRegion(X, Y)) {
				if (!Node->m_Region->m_Blocked) ret = Node->m_Region->m_Alpha;
				break;
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::IsBlockedAt(int X, int Y, bool CheckFreeObjects, CBObject *Requester) {
	bool ret = true;


	if (CheckFreeObjects) {
		int i;
		for (i = 0; i < m_Objects.GetSize(); i++) {
			if (m_Objects[i]->m_Active && m_Objects[i] != Requester && m_Objects[i]->m_CurrentBlockRegion) {
				if (m_Objects[i]->m_CurrentBlockRegion->PointInRegion(X, Y)) return true;
			}
		}
		CAdGame *AdGame = (CAdGame *)Game;
		for (i = 0; i < AdGame->m_Objects.GetSize(); i++) {
			if (AdGame->m_Objects[i]->m_Active && AdGame->m_Objects[i] != Requester && AdGame->m_Objects[i]->m_CurrentBlockRegion) {
				if (AdGame->m_Objects[i]->m_CurrentBlockRegion->PointInRegion(X, Y)) return true;
			}
		}
	}


	if (m_MainLayer) {
		for (int i = 0; i < m_MainLayer->m_Nodes.GetSize(); i++) {
			CAdSceneNode *Node = m_MainLayer->m_Nodes[i];
			/*
			if(Node->m_Type == OBJECT_REGION && Node->m_Region->m_Active && Node->m_Region->m_Blocked && Node->m_Region->PointInRegion(X, Y))
			{
			    ret = true;
			    break;
			}
			*/
			if (Node->m_Type == OBJECT_REGION && Node->m_Region->m_Active && !Node->m_Region->m_Decoration && Node->m_Region->PointInRegion(X, Y)) {
				if (Node->m_Region->m_Blocked) {
					ret = true;
					break;
				} else ret = false;
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::IsWalkableAt(int X, int Y, bool CheckFreeObjects, CBObject *Requester) {
	bool ret = false;


	if (CheckFreeObjects) {
		int i;
		for (i = 0; i < m_Objects.GetSize(); i++) {
			if (m_Objects[i]->m_Active && m_Objects[i] != Requester && m_Objects[i]->m_CurrentBlockRegion) {
				if (m_Objects[i]->m_CurrentBlockRegion->PointInRegion(X, Y)) return false;
			}
		}
		CAdGame *AdGame = (CAdGame *)Game;
		for (i = 0; i < AdGame->m_Objects.GetSize(); i++) {
			if (AdGame->m_Objects[i]->m_Active && AdGame->m_Objects[i] != Requester && AdGame->m_Objects[i]->m_CurrentBlockRegion) {
				if (AdGame->m_Objects[i]->m_CurrentBlockRegion->PointInRegion(X, Y)) return false;
			}
		}
	}


	if (m_MainLayer) {
		for (int i = 0; i < m_MainLayer->m_Nodes.GetSize(); i++) {
			CAdSceneNode *Node = m_MainLayer->m_Nodes[i];
			if (Node->m_Type == OBJECT_REGION && Node->m_Region->m_Active && !Node->m_Region->m_Decoration && Node->m_Region->PointInRegion(X, Y)) {
				if (Node->m_Region->m_Blocked) {
					ret = false;
					break;
				} else ret = true;
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
int CAdScene::GetPointsDist(CBPoint p1, CBPoint p2, CBObject *requester) {
	double xStep, yStep, X, Y;
	int xLength, yLength, xCount, yCount;
	int X1, Y1, X2, Y2;

	X1 = p1.x;
	Y1 = p1.y;
	X2 = p2.x;
	Y2 = p2.y;

	xLength = abs(X2 - X1);
	yLength = abs(Y2 - Y1);

	if (xLength > yLength) {
		if (X1 > X2) {
			CBUtils::Swap(&X1, &X2);
			CBUtils::Swap(&Y1, &Y2);
		}

		yStep = (double)(Y2 - Y1) / (double)(X2 - X1);
		Y = Y1;

		for (xCount = X1; xCount < X2; xCount++) {
			if (IsBlockedAt(xCount, (int)Y, true, requester)) return -1;
			Y += yStep;
		}
	} else {
		if (Y1 > Y2) {
			CBUtils::Swap(&X1, &X2);
			CBUtils::Swap(&Y1, &Y2);
		}

		xStep = (double)(X2 - X1) / (double)(Y2 - Y1);
		X = X1;

		for (yCount = Y1; yCount < Y2; yCount++) {
			if (IsBlockedAt((int)X, yCount, true, requester)) return -1;
			X += xStep;
		}
	}
	return std::max(xLength, yLength);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::PathFinderStep() {
	int i;
	// get lowest unmarked
	int lowest_dist = INT_MAX;
	CAdPathPoint *lowest_pt = NULL;

	for (i = 0; i < m_PFPointsNum; i++)
		if (!m_PFPath[i]->m_Marked && m_PFPath[i]->m_Distance < lowest_dist) {
			lowest_dist = m_PFPath[i]->m_Distance;
			lowest_pt = m_PFPath[i];
		}

	if (lowest_pt == NULL) { // no path -> terminate PathFinder
		m_PFReady = true;
		m_PFTargetPath->SetReady(true);
		return;
	}

	lowest_pt->m_Marked = true;

	// target point marked, generate path and terminate
	if (lowest_pt->x == m_PFTarget->x && lowest_pt->y == m_PFTarget->y) {
		while (lowest_pt != NULL) {
			m_PFTargetPath->m_Points.InsertAt(0, new CBPoint(lowest_pt->x, lowest_pt->y));
			lowest_pt = lowest_pt->m_Origin;
		}

		m_PFReady = true;
		m_PFTargetPath->SetReady(true);
		return;
	}

	// otherwise keep on searching
	for (i = 0; i < m_PFPointsNum; i++)
		if (!m_PFPath[i]->m_Marked) {
			int j = GetPointsDist(*lowest_pt, *m_PFPath[i], m_PFRequester);
			if (j != -1 && lowest_pt->m_Distance + j < m_PFPath[i]->m_Distance) {
				m_PFPath[i]->m_Distance = lowest_pt->m_Distance + j;
				m_PFPath[i]->m_Origin = lowest_pt;
			}
		}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::InitLoop() {
#ifdef _DEBUGxxxx
	int num_steps = 0;
	uint32 start = Game->m_CurrentTime;
	while (!m_PFReady && CBPlatform::GetTime() - start <= m_PFMaxTime) {
		PathFinderStep();
		num_steps++;
	}
	if (num_steps > 0) Game->LOG(0, "STAT: PathFinder iterations in one loop: %d (%s)  m_PFMaxTime=%d", num_steps, m_PFReady ? "finished" : "not yet done", m_PFMaxTime);
#else
	uint32 start = Game->m_CurrentTime;
	while (!m_PFReady && CBPlatform::GetTime() - start <= m_PFMaxTime) PathFinderStep();
#endif

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdScene::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	delete[] m_Filename;
	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing SCENE file '%s'", Filename);

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(SCENE)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(LAYER)
TOKEN_DEF(WAYPOINTS)
TOKEN_DEF(EVENTS)
TOKEN_DEF(CURSOR)
TOKEN_DEF(CAMERA)
TOKEN_DEF(ENTITY)
TOKEN_DEF(SCALE_LEVEL)
TOKEN_DEF(ROTATION_LEVEL)
TOKEN_DEF(EDITOR_MARGIN_H)
TOKEN_DEF(EDITOR_MARGIN_V)
TOKEN_DEF(EDITOR_COLOR_FRAME)
TOKEN_DEF(EDITOR_COLOR_ENTITY_SEL)
TOKEN_DEF(EDITOR_COLOR_REGION_SEL)
TOKEN_DEF(EDITOR_COLOR_DECORATION_SEL)
TOKEN_DEF(EDITOR_COLOR_BLOCKED_SEL)
TOKEN_DEF(EDITOR_COLOR_WAYPOINTS_SEL)
TOKEN_DEF(EDITOR_COLOR_REGION)
TOKEN_DEF(EDITOR_COLOR_DECORATION)
TOKEN_DEF(EDITOR_COLOR_BLOCKED)
TOKEN_DEF(EDITOR_COLOR_ENTITY)
TOKEN_DEF(EDITOR_COLOR_WAYPOINTS)
TOKEN_DEF(EDITOR_COLOR_SCALE)
TOKEN_DEF(EDITOR_SHOW_REGIONS)
TOKEN_DEF(EDITOR_SHOW_BLOCKED)
TOKEN_DEF(EDITOR_SHOW_DECORATION)
TOKEN_DEF(EDITOR_SHOW_ENTITIES)
TOKEN_DEF(EDITOR_SHOW_SCALE)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(VIEWPORT)
TOKEN_DEF(PERSISTENT_STATE_SPRITES)
TOKEN_DEF(PERSISTENT_STATE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(SCENE)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(LAYER)
	TOKEN_TABLE(WAYPOINTS)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(CAMERA)
	TOKEN_TABLE(ENTITY)
	TOKEN_TABLE(SCALE_LEVEL)
	TOKEN_TABLE(ROTATION_LEVEL)
	TOKEN_TABLE(EDITOR_MARGIN_H)
	TOKEN_TABLE(EDITOR_MARGIN_V)
	TOKEN_TABLE(EDITOR_COLOR_FRAME)
	TOKEN_TABLE(EDITOR_COLOR_ENTITY_SEL)
	TOKEN_TABLE(EDITOR_COLOR_REGION_SEL)
	TOKEN_TABLE(EDITOR_COLOR_DECORATION_SEL)
	TOKEN_TABLE(EDITOR_COLOR_BLOCKED_SEL)
	TOKEN_TABLE(EDITOR_COLOR_WAYPOINTS_SEL)
	TOKEN_TABLE(EDITOR_COLOR_REGION)
	TOKEN_TABLE(EDITOR_COLOR_DECORATION)
	TOKEN_TABLE(EDITOR_COLOR_BLOCKED)
	TOKEN_TABLE(EDITOR_COLOR_ENTITY)
	TOKEN_TABLE(EDITOR_COLOR_WAYPOINTS)
	TOKEN_TABLE(EDITOR_COLOR_SCALE)
	TOKEN_TABLE(EDITOR_SHOW_REGIONS)
	TOKEN_TABLE(EDITOR_SHOW_DECORATION)
	TOKEN_TABLE(EDITOR_SHOW_BLOCKED)
	TOKEN_TABLE(EDITOR_SHOW_ENTITIES)
	TOKEN_TABLE(EDITOR_SHOW_SCALE)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(VIEWPORT)
	TOKEN_TABLE(PERSISTENT_STATE_SPRITES)
	TOKEN_TABLE(PERSISTENT_STATE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	Cleanup();

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_SCENE) {
			Game->LOG(0, "'SCENE' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	int ar, ag, ab, aa;
	char camera[MAX_PATH] = "";
	float WaypointHeight = -1.0f;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_CAPTION:
			SetCaption((char *)params);
			break;

		case TOKEN_LAYER: {
			CAdLayer *layer = new CAdLayer(Game);
			if (!layer || FAILED(layer->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete layer;
				layer = NULL;
			} else {
				Game->RegisterObject(layer);
				m_Layers.Add(layer);
				if (layer->m_Main) {
					m_MainLayer = layer;
					m_Width = layer->m_Width;
					m_Height = layer->m_Height;
				}
			}
		}
		break;

		case TOKEN_WAYPOINTS: {
			CAdWaypointGroup *wpt = new CAdWaypointGroup(Game);
			if (!wpt || FAILED(wpt->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete wpt;
				wpt = NULL;
			} else {
				Game->RegisterObject(wpt);
				m_WaypointGroups.Add(wpt);
			}
		}
		break;

		case TOKEN_SCALE_LEVEL: {
			CAdScaleLevel *sl = new CAdScaleLevel(Game);
			if (!sl || FAILED(sl->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete sl;
				sl = NULL;
			} else {
				Game->RegisterObject(sl);
				m_ScaleLevels.Add(sl);
			}
		}
		break;

		case TOKEN_ROTATION_LEVEL: {
			CAdRotLevel *rl = new CAdRotLevel(Game);
			if (!rl || FAILED(rl->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete rl;
				rl = NULL;
			} else {
				Game->RegisterObject(rl);
				m_RotLevels.Add(rl);
			}
		}
		break;

		case TOKEN_ENTITY: {
			CAdEntity *entity = new CAdEntity(Game);
			if (!entity || FAILED(entity->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete entity;
				entity = NULL;
			} else {
				AddObject(entity);
			}
		}
		break;

		case TOKEN_CURSOR:
			delete m_Cursor;
			m_Cursor = new CBSprite(Game);
			if (!m_Cursor || FAILED(m_Cursor->LoadFile((char *)params))) {
				delete m_Cursor;
				m_Cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CAMERA:
			strcpy(camera, (char *)params);
			break;

		case TOKEN_EDITOR_MARGIN_H:
			parser.ScanStr((char *)params, "%d", &m_EditorMarginH);
			break;

		case TOKEN_EDITOR_MARGIN_V:
			parser.ScanStr((char *)params, "%d", &m_EditorMarginV);
			break;

		case TOKEN_EDITOR_COLOR_FRAME:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColFrame = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColEntity = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColEntitySel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColRegionSel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColDecorSel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColBlockedSel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColWaypointsSel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColRegion = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColDecor = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColBlocked = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColWaypoints = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_SCALE:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			m_EditorColScale = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_SHOW_REGIONS:
			parser.ScanStr((char *)params, "%b", &m_EditorShowRegions);
			break;

		case TOKEN_EDITOR_SHOW_BLOCKED:
			parser.ScanStr((char *)params, "%b", &m_EditorShowBlocked);
			break;

		case TOKEN_EDITOR_SHOW_DECORATION:
			parser.ScanStr((char *)params, "%b", &m_EditorShowDecor);
			break;

		case TOKEN_EDITOR_SHOW_ENTITIES:
			parser.ScanStr((char *)params, "%b", &m_EditorShowEntities);
			break;

		case TOKEN_EDITOR_SHOW_SCALE:
			parser.ScanStr((char *)params, "%b", &m_EditorShowScale);
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			ParseProperty(params, false);
			break;

		case TOKEN_VIEWPORT: {
			RECT rc;
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &rc.left, &rc.top, &rc.right, &rc.bottom);
			if (!m_Viewport) m_Viewport = new CBViewport(Game);
			if (m_Viewport) m_Viewport->SetRect(rc.left, rc.top, rc.right, rc.bottom, true);
		}

		case TOKEN_PERSISTENT_STATE:
			parser.ScanStr((char *)params, "%b", &m_PersistentState);
			break;

		case TOKEN_PERSISTENT_STATE_SPRITES:
			parser.ScanStr((char *)params, "%b", &m_PersistentStateSprites);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;

		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in SCENE definition");
		return E_FAIL;
	}

	if (m_MainLayer == NULL) Game->LOG(0, "Warning: scene '%s' has no main layer.", m_Filename);


	SortScaleLevels();
	SortRotLevels();

	m_Initialized = true;


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::TraverseNodes(bool Update) {
	if (!m_Initialized) return S_OK;

	int j, k;
	CAdGame *AdGame = (CAdGame *)Game;


	//////////////////////////////////////////////////////////////////////////
	// prepare viewport
	bool PopViewport = false;
	if (m_Viewport && !Game->m_EditorMode) {
		Game->PushViewport(m_Viewport);
		PopViewport = true;
	} else if (AdGame->m_SceneViewport && !Game->m_EditorMode) {
		Game->PushViewport(AdGame->m_SceneViewport);
		PopViewport = true;
	}


	//////////////////////////////////////////////////////////////////////////
	// *** adjust scroll offset
	if (Update) {
		/*
		if(m_AutoScroll && Game->m_MainObject != NULL)
		{
		    ScrollToObject(Game->m_MainObject);
		}
		*/

		if (m_AutoScroll) {
			// adjust horizontal scroll
			if (Game->m_Timer - m_LastTimeH >= m_ScrollTimeH) {
				m_LastTimeH = Game->m_Timer;
				if (m_OffsetLeft < m_TargetOffsetLeft) {
					m_OffsetLeft += m_ScrollPixelsH;
					m_OffsetLeft = std::min(m_OffsetLeft, m_TargetOffsetLeft);
				} else if (m_OffsetLeft > m_TargetOffsetLeft) {
					m_OffsetLeft -= m_ScrollPixelsH;
					m_OffsetLeft = std::max(m_OffsetLeft, m_TargetOffsetLeft);
				}
			}

			// adjust vertical scroll
			if (Game->m_Timer - m_LastTimeV >= m_ScrollTimeV) {
				m_LastTimeV = Game->m_Timer;
				if (m_OffsetTop < m_TargetOffsetTop) {
					m_OffsetTop += m_ScrollPixelsV;
					m_OffsetTop = std::min(m_OffsetTop, m_TargetOffsetTop);
				} else if (m_OffsetTop > m_TargetOffsetTop) {
					m_OffsetTop -= m_ScrollPixelsV;
					m_OffsetTop = std::max(m_OffsetTop, m_TargetOffsetTop);
				}
			}

			if (m_OffsetTop == m_TargetOffsetTop && m_OffsetLeft == m_TargetOffsetLeft) m_Ready = true;
		} else m_Ready = true; // not scrolling, i.e. always ready
	}




	//////////////////////////////////////////////////////////////////////////
	int ViewportWidth, ViewportHeight;
	GetViewportSize(&ViewportWidth, &ViewportHeight);

	int ViewportX, ViewportY;
	GetViewportOffset(&ViewportX, &ViewportY);

	int ScrollableX = m_Width  - ViewportWidth;
	int ScrollableY = m_Height - ViewportHeight;

	double WidthRatio  = ScrollableX <= 0 ? 0 : ((double)(m_OffsetLeft) / (double)ScrollableX);
	double HeightRatio = ScrollableY <= 0 ? 0 : ((double)(m_OffsetTop)  / (double)ScrollableY);

	int OrigX, OrigY;
	Game->GetOffset(&OrigX, &OrigY);



	//////////////////////////////////////////////////////////////////////////
	// *** display/update everything
	Game->m_Renderer->Setup2D();

	// for each layer
	int MainOffsetX = 0;
	int MainOffsetY = 0;

	for (j = 0; j < m_Layers.GetSize(); j++) {
		if (!m_Layers[j]->m_Active) continue;

		// make layer exclusive
		if (!Update) {
			if (m_Layers[j]->m_CloseUp && !Game->m_EditorMode) {
				if (!m_ShieldWindow) m_ShieldWindow = new CUIWindow(Game);
				if (m_ShieldWindow) {
					m_ShieldWindow->m_PosX = m_ShieldWindow->m_PosY = 0;
					m_ShieldWindow->m_Width = Game->m_Renderer->m_Width;
					m_ShieldWindow->m_Height = Game->m_Renderer->m_Height;
					m_ShieldWindow->Display();
				}
			}
		}

		if (m_ParalaxScrolling) {
			int OffsetX = (int)(WidthRatio  * (m_Layers[j]->m_Width  - ViewportWidth) - ViewportX);
			int OffsetY = (int)(HeightRatio * (m_Layers[j]->m_Height - ViewportHeight) - ViewportY);
			Game->SetOffset(OffsetX, OffsetY);

			Game->m_OffsetPercentX = (float)OffsetX / ((float)m_Layers[j]->m_Width - ViewportWidth) * 100.0f;
			Game->m_OffsetPercentY = (float)OffsetY / ((float)m_Layers[j]->m_Height - ViewportHeight) * 100.0f;

			//Game->QuickMessageForm("%d %f", OffsetX+ViewportX, Game->m_OffsetPercentX);
		} else {
			Game->SetOffset(m_OffsetLeft - ViewportX, m_OffsetTop - ViewportY);

			Game->m_OffsetPercentX = (float)(m_OffsetLeft - ViewportX) / ((float)m_Layers[j]->m_Width - ViewportWidth) * 100.0f;
			Game->m_OffsetPercentY = (float)(m_OffsetTop - ViewportY) / ((float)m_Layers[j]->m_Height - ViewportHeight) * 100.0f;
		}


		// for each node
		for (k = 0; k < m_Layers[j]->m_Nodes.GetSize(); k++) {
			CAdSceneNode *Node = m_Layers[j]->m_Nodes[k];
			switch (Node->m_Type) {
			case OBJECT_ENTITY:
				if (Node->m_Entity->m_Active && (Game->m_EditorMode || !Node->m_Entity->m_EditorOnly)) {
					Game->m_Renderer->Setup2D();

					if (Update) Node->m_Entity->Update();
					else Node->m_Entity->Display();
				}
				break;

			case OBJECT_REGION: {
				if (Node->m_Region->m_Blocked) break;
				if (Node->m_Region->m_Decoration) break;

				if (!Update) DisplayRegionContent(Node->m_Region);
			}
			break;
			} // switch
		} // each node

		// display/update all objects which are off-regions
		if (m_Layers[j]->m_Main) {
			if (Update) {
				UpdateFreeObjects();
			} else {
				DisplayRegionContent(NULL);
			}
		}
	} // each layer


	// restore state
	Game->SetOffset(OrigX, OrigY);
	Game->m_Renderer->Setup2D();

	// display/update fader
	if (m_Fader) {
		if (Update) m_Fader->Update();
		else m_Fader->Display();
	}

	if (PopViewport) Game->PopViewport();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::Display() {
	return TraverseNodes(false);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::UpdateFreeObjects() {
	CAdGame *AdGame = (CAdGame *)Game;
	int i;

	bool Is3DSet;

	// *** update all active objects
	Is3DSet = false;
	for (i = 0; i < AdGame->m_Objects.GetSize(); i++) {
		if (!AdGame->m_Objects[i]->m_Active) continue;

		AdGame->m_Objects[i]->Update();
		AdGame->m_Objects[i]->m_Drawn = false;
	}


	for (i = 0; i < m_Objects.GetSize(); i++) {
		if (!m_Objects[i]->m_Active) continue;

		m_Objects[i]->Update();
		m_Objects[i]->m_Drawn = false;
	}


	if (m_AutoScroll && Game->m_MainObject != NULL) {
		ScrollToObject(Game->m_MainObject);
	}


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::DisplayRegionContent(CAdRegion *Region, bool Display3DOnly) {
	CAdGame *AdGame = (CAdGame *)Game;
	CBArray<CAdObject *, CAdObject *> Objects;
	CAdObject *Obj;

	int i;

	// global objects
	for (i = 0; i < AdGame->m_Objects.GetSize(); i++) {
		Obj = AdGame->m_Objects[i];
		if (Obj->m_Active && !Obj->m_Drawn && (Obj->m_StickRegion == Region || Region == NULL || (Obj->m_StickRegion == NULL && Region->PointInRegion(Obj->m_PosX, Obj->m_PosY)))) {
			Objects.Add(Obj);
		}
	}

	// scene objects
	for (i = 0; i < m_Objects.GetSize(); i++) {
		Obj = m_Objects[i];
		if (Obj->m_Active && !Obj->m_EditorOnly && !Obj->m_Drawn && (Obj->m_StickRegion == Region || Region == NULL || (Obj->m_StickRegion == NULL && Region->PointInRegion(Obj->m_PosX, Obj->m_PosY)))) {
			Objects.Add(Obj);
		}
	}

	// sort by m_PosY
	qsort(Objects.GetData(), Objects.GetSize(), sizeof(CAdObject *), CAdScene::CompareObjs);

	// display them
	for (i = 0; i < Objects.GetSize(); i++) {
		Obj = Objects[i];

		if (Display3DOnly && !Obj->m_Is3D) continue;

		Game->m_Renderer->Setup2D();

		if (Game->m_EditorMode || !Obj->m_EditorOnly) Obj->Display();
		Obj->m_Drawn = true;
	}


	// display design only objects
	if (!Display3DOnly) {
		if (Game->m_EditorMode && Region == NULL) {
			for (i = 0; i < m_Objects.GetSize(); i++) {
				if (m_Objects[i]->m_Active && m_Objects[i]->m_EditorOnly) {
					m_Objects[i]->Display();
					m_Objects[i]->m_Drawn = true;
				}
			}
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
int CAdScene::CompareObjs(const void *Obj1, const void *Obj2) {
	CAdObject *Object1 = *(CAdObject **)Obj1;
	CAdObject *Object2 = *(CAdObject **)Obj2;

	if (Object1->m_PosY < Object2->m_PosY) return -1;
	else if (Object1->m_PosY > Object2->m_PosY) return 1;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::DisplayRegionContentOld(CAdRegion *Region) {
	CAdGame *AdGame = (CAdGame *)Game;
	CAdObject *obj;
	int i;

	// display all objects in region sorted by m_PosY
	do {
		obj = NULL;
		int minY = INT_MAX;

		// global objects
		for (i = 0; i < AdGame->m_Objects.GetSize(); i++) {
			if (AdGame->m_Objects[i]->m_Active && !AdGame->m_Objects[i]->m_Drawn && AdGame->m_Objects[i]->m_PosY < minY && (AdGame->m_Objects[i]->m_StickRegion == Region || Region == NULL || (AdGame->m_Objects[i]->m_StickRegion == NULL && Region->PointInRegion(AdGame->m_Objects[i]->m_PosX, AdGame->m_Objects[i]->m_PosY)))) {
				obj = AdGame->m_Objects[i];
				minY = AdGame->m_Objects[i]->m_PosY;
			}
		}

		// scene objects
		for (i = 0; i < m_Objects.GetSize(); i++) {
			if (m_Objects[i]->m_Active && !m_Objects[i]->m_EditorOnly && !m_Objects[i]->m_Drawn && m_Objects[i]->m_PosY < minY && (m_Objects[i]->m_StickRegion == Region || Region == NULL || (m_Objects[i]->m_StickRegion == NULL && Region->PointInRegion(m_Objects[i]->m_PosX, m_Objects[i]->m_PosY)))) {
				obj = m_Objects[i];
				minY = m_Objects[i]->m_PosY;
			}
		}


		if (obj != NULL) {
			Game->m_Renderer->Setup2D();

			if (Game->m_EditorMode || !obj->m_EditorOnly) obj->Display();
			obj->m_Drawn = true;
		}
	} while (obj != NULL);


	// design only objects
	if (Game->m_EditorMode && Region == NULL) {
		for (i = 0; i < m_Objects.GetSize(); i++) {
			if (m_Objects[i]->m_Active && m_Objects[i]->m_EditorOnly) {
				m_Objects[i]->Display();
				m_Objects[i]->m_Drawn = true;
			}
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::Update() {
	return TraverseNodes(true);
}

//////////////////////////////////////////////////////////////////////////
void CAdScene::ScrollTo(int OffsetX, int OffsetY) {
	int ViewportWidth, ViewportHeight;
	GetViewportSize(&ViewportWidth, &ViewportHeight);

	int OrigOffsetLeft = m_TargetOffsetLeft;
	int OrigOffsetTop = m_TargetOffsetTop;

	m_TargetOffsetLeft = std::max(0, OffsetX - ViewportWidth / 2);
	m_TargetOffsetLeft = std::min(m_TargetOffsetLeft, m_Width - ViewportWidth);

	m_TargetOffsetTop = std::max(0, OffsetY - ViewportHeight / 2);
	m_TargetOffsetTop = std::min(m_TargetOffsetTop, m_Height - ViewportHeight);


	if (Game->m_MainObject && Game->m_MainObject->m_Is3D) {
		if (abs(OrigOffsetLeft - m_TargetOffsetLeft) < 5) m_TargetOffsetLeft = OrigOffsetLeft;
		if (abs(OrigOffsetTop - m_TargetOffsetTop) < 5) m_TargetOffsetTop = OrigOffsetTop;
		//m_TargetOffsetTop = 0;
	}

	m_Ready = false;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::ScrollToObject(CBObject *Object) {
	if (Object) ScrollTo(Object->m_PosX, Object->m_PosY - Object->GetHeight() / 2);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::SkipToObject(CBObject *Object) {
	if (Object) SkipTo(Object->m_PosX, Object->m_PosY - Object->GetHeight() / 2);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::SkipTo(int OffsetX, int OffsetY) {
	int ViewportWidth, ViewportHeight;
	GetViewportSize(&ViewportWidth, &ViewportHeight);

	m_OffsetLeft = std::max(0, OffsetX - ViewportWidth / 2);
	m_OffsetLeft = std::min(m_OffsetLeft, m_Width - ViewportWidth);

	m_OffsetTop = std::max(0, OffsetY - ViewportHeight / 2);
	m_OffsetTop = std::min(m_OffsetTop, m_Height - ViewportHeight);

	m_TargetOffsetLeft = m_OffsetLeft;
	m_TargetOffsetTop = m_OffsetTop;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// LoadActor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "LoadActor") == 0) {
		Stack->CorrectParams(1);
		CAdActor *act = new CAdActor(Game);
		if (act && SUCCEEDED(act->LoadFile(Stack->Pop()->GetString()))) {
			AddObject(act);
			Stack->PushNative(act, true);
		} else {
			delete act;
			act = NULL;
			Stack->PushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadEntity") == 0) {
		Stack->CorrectParams(1);
		CAdEntity *ent = new CAdEntity(Game);
		if (ent && SUCCEEDED(ent->LoadFile(Stack->Pop()->GetString()))) {
			AddObject(ent);
			Stack->PushNative(ent, true);
		} else {
			delete ent;
			ent = NULL;
			Stack->PushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateEntity") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdEntity *Ent = new CAdEntity(Game);
		AddObject(Ent);
		if (!Val->IsNULL()) Ent->SetName(Val->GetString());
		Stack->PushNative(Ent, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadObject / UnloadActor / UnloadEntity / UnloadActor3D / DeleteEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "UnloadObject") == 0 || strcmp(Name, "UnloadActor") == 0 || strcmp(Name, "UnloadEntity") == 0 || strcmp(Name, "UnloadActor3D") == 0 || strcmp(Name, "DeleteEntity") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		CAdObject *obj = (CAdObject *)val->GetNative();
		RemoveObject(obj);
		if (val->GetType() == VAL_VARIABLE_REF) val->SetNULL();

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SkipTo") == 0) {
		Stack->CorrectParams(2);
		CScValue *val1 = Stack->Pop();
		CScValue *val2 = Stack->Pop();
		if (val1->IsNative()) {
			SkipToObject((CBObject *)val1->GetNative());
		} else {
			SkipTo(val1->GetInt(), val2->GetInt());
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollTo / ScrollToAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollTo") == 0 || strcmp(Name, "ScrollToAsync") == 0) {
		Stack->CorrectParams(2);
		CScValue *val1 = Stack->Pop();
		CScValue *val2 = Stack->Pop();
		if (val1->IsNative()) {
			ScrollToObject((CBObject *)val1->GetNative());
		} else {
			ScrollTo(val1->GetInt(), val2->GetInt());
		}
		if (strcmp(Name, "ScrollTo") == 0) Script->WaitForExclusive(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetLayer") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		if (val->IsInt()) {
			int layer = val->GetInt();
			if (layer < 0 || layer >= m_Layers.GetSize()) Stack->PushNULL();
			else Stack->PushNative(m_Layers[layer], true);
		} else {
			char *LayerName = val->GetString();
			bool LayerFound = false;
			for (int i = 0; i < m_Layers.GetSize(); i++) {
				if (scumm_stricmp(LayerName, m_Layers[i]->m_Name) == 0) {
					Stack->PushNative(m_Layers[i], true);
					LayerFound = true;
					break;
				}
			}
			if (!LayerFound) Stack->PushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaypointGroup
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetWaypointGroup") == 0) {
		Stack->CorrectParams(1);
		int group = Stack->Pop()->GetInt();
		if (group < 0 || group >= m_WaypointGroups.GetSize()) Stack->PushNULL();
		else Stack->PushNative(m_WaypointGroups[group], true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetNode") == 0) {
		Stack->CorrectParams(1);
		char *Name = Stack->Pop()->GetString();

		CBObject *node = GetNodeByName(Name);
		if (node) Stack->PushNative((CBScriptable *)node, true);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFreeNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFreeNode") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdObject *Ret = NULL;
		if (Val->IsInt()) {
			int Index = Val->GetInt();
			if (Index >= 0 && Index < m_Objects.GetSize()) Ret = m_Objects[Index];
		} else {
			char *Name = Val->GetString();
			for (int i = 0; i < m_Objects.GetSize(); i++) {
				if (m_Objects[i] && m_Objects[i]->m_Name && scumm_stricmp(m_Objects[i]->m_Name, Name) == 0) {
					Ret = m_Objects[i];
					break;
				}
			}
		}
		if (Ret) Stack->PushNative(Ret, true);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRegionAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetRegionAt") == 0) {
		Stack->CorrectParams(3);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();
		CScValue *Val = Stack->Pop();

		bool IncludeDecors = false;
		if (!Val->IsNULL()) IncludeDecors = Val->GetBool();

		if (m_MainLayer) {
			for (int i = m_MainLayer->m_Nodes.GetSize() - 1; i >= 0; i--) {
				CAdSceneNode *Node = m_MainLayer->m_Nodes[i];
				if (Node->m_Type == OBJECT_REGION && Node->m_Region->m_Active && Node->m_Region->PointInRegion(X, Y)) {
					if (Node->m_Region->m_Decoration && !IncludeDecors) continue;

					Stack->PushNative(Node->m_Region, true);
					return S_OK;
				}
			}
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsBlockedAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsBlockedAt") == 0) {
		Stack->CorrectParams(2);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();

		Stack->PushBool(IsBlockedAt(X, Y));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsWalkableAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsWalkableAt") == 0) {
		Stack->CorrectParams(2);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();

		Stack->PushBool(IsWalkableAt(X, Y));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetScaleAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetScaleAt") == 0) {
		Stack->CorrectParams(2);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();

		Stack->PushFloat(GetZoomAt(X, Y));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRotationAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetRotationAt") == 0) {
		Stack->CorrectParams(2);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();

		Stack->PushFloat(GetRotationAt(X, Y));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsScrolling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsScrolling") == 0) {
		Stack->CorrectParams(0);
		bool Ret = false;
		if (m_AutoScroll) {
			if (m_TargetOffsetLeft != m_OffsetLeft || m_TargetOffsetTop != m_OffsetTop) Ret = true;
		}

		Stack->PushBool(Ret);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeOut / FadeOutAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeOut") == 0 || strcmp(Name, "FadeOutAsync") == 0) {
		Stack->CorrectParams(5);
		uint32 Duration = Stack->Pop()->GetInt(500);
		byte Red = Stack->Pop()->GetInt(0);
		byte Green = Stack->Pop()->GetInt(0);
		byte Blue = Stack->Pop()->GetInt(0);
		byte Alpha = Stack->Pop()->GetInt(0xFF);

		m_Fader->FadeOut(DRGBA(Red, Green, Blue, Alpha), Duration);
		if (strcmp(Name, "FadeOutAsync") != 0) Script->WaitFor(m_Fader);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeIn / FadeInAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeIn") == 0 || strcmp(Name, "FadeInAsync") == 0) {
		Stack->CorrectParams(5);
		uint32 Duration = Stack->Pop()->GetInt(500);
		byte Red = Stack->Pop()->GetInt(0);
		byte Green = Stack->Pop()->GetInt(0);
		byte Blue = Stack->Pop()->GetInt(0);
		byte Alpha = Stack->Pop()->GetInt(0xFF);

		m_Fader->FadeIn(DRGBA(Red, Green, Blue, Alpha), Duration);
		if (strcmp(Name, "FadeInAsync") != 0) Script->WaitFor(m_Fader);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFadeColor") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(m_Fader->GetCurrentColor());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsPointInViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsPointInViewport") == 0) {
		Stack->CorrectParams(2);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();
		Stack->PushBool(PointInViewport(X, Y));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetViewport") == 0) {
		Stack->CorrectParams(4);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();
		int Width = Stack->Pop()->GetInt();
		int Height = Stack->Pop()->GetInt();

		if (Width <= 0) Width = Game->m_Renderer->m_Width;
		if (Height <= 0) Height = Game->m_Renderer->m_Height;

		if (!m_Viewport) m_Viewport = new CBViewport(Game);
		if (m_Viewport) m_Viewport->SetRect(X, Y, X + Width, Y + Height);

		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddLayer") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdLayer *Layer = new CAdLayer(Game);
		if (!Val->IsNULL()) Layer->SetName(Val->GetString());
		if (m_MainLayer) {
			Layer->m_Width = m_MainLayer->m_Width;
			Layer->m_Height = m_MainLayer->m_Height;
		}
		m_Layers.Add(Layer);
		Game->RegisterObject(Layer);

		Stack->PushNative(Layer, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InsertLayer") == 0) {
		Stack->CorrectParams(2);
		int Index = Stack->Pop()->GetInt();
		CScValue *Val = Stack->Pop();

		CAdLayer *Layer = new CAdLayer(Game);
		if (!Val->IsNULL()) Layer->SetName(Val->GetString());
		if (m_MainLayer) {
			Layer->m_Width = m_MainLayer->m_Width;
			Layer->m_Height = m_MainLayer->m_Height;
		}
		if (Index < 0) Index = 0;
		if (Index <= m_Layers.GetSize() - 1) m_Layers.InsertAt(Index, Layer);
		else m_Layers.Add(Layer);

		Game->RegisterObject(Layer);

		Stack->PushNative(Layer, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteLayer") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdLayer *ToDelete = NULL;
		if (Val->IsNative()) {
			CBScriptable *Temp = Val->GetNative();
			for (int i = 0; i < m_Layers.GetSize(); i++) {
				if (m_Layers[i] == Temp) {
					ToDelete = m_Layers[i];
					break;
				}
			}
		} else {
			int Index = Val->GetInt();
			if (Index >= 0 && Index < m_Layers.GetSize()) {
				ToDelete = m_Layers[Index];
			}
		}
		if (ToDelete == NULL) {
			Stack->PushBool(false);
			return S_OK;
		}

		if (ToDelete->m_Main) {
			Script->RuntimeError("Scene.DeleteLayer - cannot delete main scene layer");
			Stack->PushBool(false);
			return S_OK;
		}

		for (int i = 0; i < m_Layers.GetSize(); i++) {
			if (m_Layers[i] == ToDelete) {
				m_Layers.RemoveAt(i);
				Game->UnregisterObject(ToDelete);
				break;
			}
		}
		Stack->PushBool(true);
		return S_OK;
	}

	else return CBObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdScene::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("scene");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumLayers (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumLayers") == 0) {
		m_ScValue->SetInt(m_Layers.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumWaypointGroups (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumWaypointGroups") == 0) {
		m_ScValue->SetInt(m_WaypointGroups.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainLayer (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MainLayer") == 0) {
		if (m_MainLayer) m_ScValue->SetNative(m_MainLayer, true);
		else m_ScValue->SetNULL();

		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFreeNodes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumFreeNodes") == 0) {
		m_ScValue->SetInt(m_Objects.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MouseX") == 0) {
		int ViewportX;
		GetViewportOffset(&ViewportX);

		m_ScValue->SetInt(Game->m_MousePos.x + m_OffsetLeft - ViewportX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MouseY") == 0) {
		int ViewportY;
		GetViewportOffset(NULL, &ViewportY);

		m_ScValue->SetInt(Game->m_MousePos.y + m_OffsetTop - ViewportY);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoScroll
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutoScroll") == 0) {
		m_ScValue->SetBool(m_AutoScroll);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PersistentState") == 0) {
		m_ScValue->SetBool(m_PersistentState);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentStateSprites
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PersistentStateSprites") == 0) {
		m_ScValue->SetBool(m_PersistentStateSprites);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollPixelsX") == 0) {
		m_ScValue->SetInt(m_ScrollPixelsH);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollPixelsY") == 0) {
		m_ScValue->SetInt(m_ScrollPixelsV);
		return m_ScValue;
	}


	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollSpeedX") == 0) {
		m_ScValue->SetInt(m_ScrollTimeH);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollSpeedY") == 0) {
		m_ScValue->SetInt(m_ScrollTimeV);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OffsetX") == 0) {
		m_ScValue->SetInt(m_OffsetLeft);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OffsetY") == 0) {
		m_ScValue->SetInt(m_OffsetTop);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		if (m_MainLayer) m_ScValue->SetInt(m_MainLayer->m_Width);
		else m_ScValue->SetInt(0);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		if (m_MainLayer) m_ScValue->SetInt(m_MainLayer->m_Height);
		else m_ScValue->SetInt(0);
		return m_ScValue;
	}

	else return CBObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoScroll
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutoScroll") == 0) {
		m_AutoScroll = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PersistentState") == 0) {
		m_PersistentState = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentStateSprites
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PersistentStateSprites") == 0) {
		m_PersistentStateSprites = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollPixelsX") == 0) {
		m_ScrollPixelsH = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollPixelsY") == 0) {
		m_ScrollPixelsV = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollSpeedX") == 0) {
		m_ScrollTimeH = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollSpeedY") == 0) {
		m_ScrollTimeV = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OffsetX") == 0) {
		m_OffsetLeft = Value->GetInt();

		int ViewportWidth, ViewportHeight;
		GetViewportSize(&ViewportWidth, &ViewportHeight);

		m_OffsetLeft = std::max(0, m_OffsetLeft - ViewportWidth / 2);
		m_OffsetLeft = std::min(m_OffsetLeft, m_Width - ViewportWidth);
		m_TargetOffsetLeft = m_OffsetLeft;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OffsetY") == 0) {
		m_OffsetTop = Value->GetInt();

		int ViewportWidth, ViewportHeight;
		GetViewportSize(&ViewportWidth, &ViewportHeight);

		m_OffsetTop = std::max(0, m_OffsetTop - ViewportHeight / 2);
		m_OffsetTop = std::min(m_OffsetTop, m_Height - ViewportHeight);
		m_TargetOffsetTop = m_OffsetTop;

		return S_OK;
	}

	else return CBObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdScene::ScToString() {
	return "[scene object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::AddObject(CAdObject *Object) {
	m_Objects.Add(Object);
	return Game->RegisterObject(Object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::RemoveObject(CAdObject *Object) {
	for (int i = 0; i < m_Objects.GetSize(); i++) {
		if (m_Objects[i] == Object) {
			m_Objects.RemoveAt(i);
			return Game->UnregisterObject(Object);
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	int i;

	Buffer->PutTextIndent(Indent, "SCENE {\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	if (m_PersistentState)
		Buffer->PutTextIndent(Indent + 2, "PERSISTENT_STATE=%s\n", m_PersistentState ? "TRUE" : "FALSE");

	if (!m_PersistentStateSprites)
		Buffer->PutTextIndent(Indent + 2, "PERSISTENT_STATE_SPRITES=%s\n", m_PersistentStateSprites ? "TRUE" : "FALSE");


	// scripts
	for (i = 0; i < m_Scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", m_Scripts[i]->m_Filename);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	// properties
	if (m_ScProp) m_ScProp->SaveAsText(Buffer, Indent + 2);

	// viewport
	if (m_Viewport) {
		RECT *rc = m_Viewport->GetRect();
		Buffer->PutTextIndent(Indent + 2, "VIEWPORT { %d, %d, %d, %d }\n", rc->left, rc->top, rc->right, rc->bottom);
	}



	// editor settings
	Buffer->PutTextIndent(Indent + 2, "; ----- editor settings\n");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_MARGIN_H=%d\n", m_EditorMarginH);
	Buffer->PutTextIndent(Indent + 2, "EDITOR_MARGIN_V=%d\n", m_EditorMarginV);
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_FRAME { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColFrame), D3DCOLGetG(m_EditorColFrame), D3DCOLGetB(m_EditorColFrame), D3DCOLGetA(m_EditorColFrame));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_ENTITY_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColEntitySel), D3DCOLGetG(m_EditorColEntitySel), D3DCOLGetB(m_EditorColEntitySel), D3DCOLGetA(m_EditorColEntitySel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_REGION_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColRegionSel), D3DCOLGetG(m_EditorColRegionSel), D3DCOLGetB(m_EditorColRegionSel), D3DCOLGetA(m_EditorColRegionSel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_BLOCKED_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColBlockedSel), D3DCOLGetG(m_EditorColBlockedSel), D3DCOLGetB(m_EditorColBlockedSel), D3DCOLGetA(m_EditorColBlockedSel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_DECORATION_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColDecorSel), D3DCOLGetG(m_EditorColDecorSel), D3DCOLGetB(m_EditorColDecorSel), D3DCOLGetA(m_EditorColDecorSel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_WAYPOINTS_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColWaypointsSel), D3DCOLGetG(m_EditorColWaypointsSel), D3DCOLGetB(m_EditorColWaypointsSel), D3DCOLGetA(m_EditorColWaypointsSel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_ENTITY { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColEntity), D3DCOLGetG(m_EditorColEntity), D3DCOLGetB(m_EditorColEntity), D3DCOLGetA(m_EditorColEntity));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_REGION { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColRegion), D3DCOLGetG(m_EditorColRegion), D3DCOLGetB(m_EditorColRegion), D3DCOLGetA(m_EditorColRegion));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_DECORATION { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColDecor), D3DCOLGetG(m_EditorColDecor), D3DCOLGetB(m_EditorColDecor), D3DCOLGetA(m_EditorColDecor));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_BLOCKED { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColBlocked), D3DCOLGetG(m_EditorColBlocked), D3DCOLGetB(m_EditorColBlocked), D3DCOLGetA(m_EditorColBlocked));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_WAYPOINTS { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColWaypoints), D3DCOLGetG(m_EditorColWaypoints), D3DCOLGetB(m_EditorColWaypoints), D3DCOLGetA(m_EditorColWaypoints));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_SCALE { %d,%d,%d,%d }\n", D3DCOLGetR(m_EditorColScale), D3DCOLGetG(m_EditorColScale), D3DCOLGetB(m_EditorColScale), D3DCOLGetA(m_EditorColScale));

	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_REGIONS=%s\n", m_EditorShowRegions ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_BLOCKED=%s\n", m_EditorShowBlocked ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_DECORATION=%s\n", m_EditorShowDecor ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_ENTITIES=%s\n", m_EditorShowEntities ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_SCALE=%s\n", m_EditorShowScale ? "TRUE" : "FALSE");

	Buffer->PutTextIndent(Indent + 2, "\n");

	CBBase::SaveAsText(Buffer, Indent + 2);

	// waypoints
	Buffer->PutTextIndent(Indent + 2, "; ----- waypoints\n");
	for (i = 0; i < m_WaypointGroups.GetSize(); i++) m_WaypointGroups[i]->SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// layers
	Buffer->PutTextIndent(Indent + 2, "; ----- layers\n");
	for (i = 0; i < m_Layers.GetSize(); i++) m_Layers[i]->SaveAsText(Buffer, Indent + 2);

	// scale levels
	Buffer->PutTextIndent(Indent + 2, "; ----- scale levels\n");
	for (i = 0; i < m_ScaleLevels.GetSize(); i++) m_ScaleLevels[i]->SaveAsText(Buffer, Indent + 2);

	// rotation levels
	Buffer->PutTextIndent(Indent + 2, "; ----- rotation levels\n");
	for (i = 0; i < m_RotLevels.GetSize(); i++) m_RotLevels[i]->SaveAsText(Buffer, Indent + 2);


	Buffer->PutTextIndent(Indent + 2, "\n");

	// free entities
	Buffer->PutTextIndent(Indent + 2, "; ----- free entities\n");
	for (i = 0; i < m_Objects.GetSize(); i++) {
		if (m_Objects[i]->m_Type == OBJECT_ENTITY) {
			m_Objects[i]->SaveAsText(Buffer, Indent + 2);

		}
	}



	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::SortScaleLevels() {
	bool changed;
	do {
		changed = false;
		for (int i = 0; i < m_ScaleLevels.GetSize() - 1; i++) {
			if (m_ScaleLevels[i]->m_PosY > m_ScaleLevels[i + 1]->m_PosY) {
				CAdScaleLevel *sl = m_ScaleLevels[i];
				m_ScaleLevels[i] = m_ScaleLevels[i + 1];
				m_ScaleLevels[i + 1] = sl;

				changed = true;
			}
		}

	} while (changed);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::SortRotLevels() {
	bool changed;
	do {
		changed = false;
		for (int i = 0; i < m_RotLevels.GetSize() - 1; i++) {
			if (m_RotLevels[i]->m_PosX > m_RotLevels[i + 1]->m_PosX) {
				CAdRotLevel *rl = m_RotLevels[i];
				m_RotLevels[i] = m_RotLevels[i + 1];
				m_RotLevels[i + 1] = rl;

				changed = true;
			}
		}

	} while (changed);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
float CAdScene::GetScaleAt(int Y) {
	CAdScaleLevel *prev = NULL;
	CAdScaleLevel *next = NULL;

	for (int i = 0; i < m_ScaleLevels.GetSize(); i++) {
		CAdScaleLevel *xxx = m_ScaleLevels[i];
		int j = m_ScaleLevels.GetSize();
		if (m_ScaleLevels[i]->m_PosY < Y) prev = m_ScaleLevels[i];
		else {
			next = m_ScaleLevels[i];
			break;
		}
	}

	if (prev == NULL || next == NULL) return 100;

	int delta_y = next->m_PosY - prev->m_PosY;
	float delta_scale = next->m_Scale - prev->m_Scale;
	Y -= prev->m_PosY;

	float percent = (float)Y / ((float)delta_y / 100.0f);
	return prev->m_Scale + delta_scale / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_AutoScroll));
	PersistMgr->Transfer(TMEMBER(m_EditorColBlocked));
	PersistMgr->Transfer(TMEMBER(m_EditorColBlockedSel));
	PersistMgr->Transfer(TMEMBER(m_EditorColDecor));
	PersistMgr->Transfer(TMEMBER(m_EditorColDecorSel));
	PersistMgr->Transfer(TMEMBER(m_EditorColEntity));
	PersistMgr->Transfer(TMEMBER(m_EditorColEntitySel));
	PersistMgr->Transfer(TMEMBER(m_EditorColFrame));
	PersistMgr->Transfer(TMEMBER(m_EditorColRegion));
	PersistMgr->Transfer(TMEMBER(m_EditorColRegionSel));
	PersistMgr->Transfer(TMEMBER(m_EditorColScale));
	PersistMgr->Transfer(TMEMBER(m_EditorColWaypoints));
	PersistMgr->Transfer(TMEMBER(m_EditorColWaypointsSel));
	PersistMgr->Transfer(TMEMBER(m_EditorMarginH));
	PersistMgr->Transfer(TMEMBER(m_EditorMarginV));
	PersistMgr->Transfer(TMEMBER(m_EditorShowBlocked));
	PersistMgr->Transfer(TMEMBER(m_EditorShowDecor));
	PersistMgr->Transfer(TMEMBER(m_EditorShowEntities));
	PersistMgr->Transfer(TMEMBER(m_EditorShowRegions));
	PersistMgr->Transfer(TMEMBER(m_EditorShowScale));
	PersistMgr->Transfer(TMEMBER(m_Fader));
	PersistMgr->Transfer(TMEMBER(m_Height));
	PersistMgr->Transfer(TMEMBER(m_Initialized));
	PersistMgr->Transfer(TMEMBER(m_LastTimeH));
	PersistMgr->Transfer(TMEMBER(m_LastTimeV));
	m_Layers.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_MainLayer));
	m_Objects.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_OffsetLeft));
	PersistMgr->Transfer(TMEMBER(m_OffsetTop));
	PersistMgr->Transfer(TMEMBER(m_ParalaxScrolling));
	PersistMgr->Transfer(TMEMBER(m_PersistentState));
	PersistMgr->Transfer(TMEMBER(m_PersistentStateSprites));
	PersistMgr->Transfer(TMEMBER(m_PFMaxTime));
	m_PFPath.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_PFPointsNum));
	PersistMgr->Transfer(TMEMBER(m_PFReady));
	PersistMgr->Transfer(TMEMBER(m_PFRequester));
	PersistMgr->Transfer(TMEMBER(m_PFTarget));
	PersistMgr->Transfer(TMEMBER(m_PFTargetPath));
	m_RotLevels.Persist(PersistMgr);
	m_ScaleLevels.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_ScrollPixelsH));
	PersistMgr->Transfer(TMEMBER(m_ScrollPixelsV));
	PersistMgr->Transfer(TMEMBER(m_ScrollTimeH));
	PersistMgr->Transfer(TMEMBER(m_ScrollTimeV));
	PersistMgr->Transfer(TMEMBER(m_ShieldWindow));
	PersistMgr->Transfer(TMEMBER(m_TargetOffsetLeft));
	PersistMgr->Transfer(TMEMBER(m_TargetOffsetTop));
	m_WaypointGroups.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_Viewport));
	PersistMgr->Transfer(TMEMBER(m_Width));

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::AfterLoad() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::CorrectTargetPoint2(int StartX, int StartY, int *TargetX, int *TargetY, bool CheckFreeObjects, CBObject *Requester) {
	double xStep, yStep, X, Y;
	int xLength, yLength, xCount, yCount;
	int X1, Y1, X2, Y2;

	X1 = *TargetX;
	Y1 = *TargetY;
	X2 = StartX;
	Y2 = StartY;


	xLength = abs(X2 - X1);
	yLength = abs(Y2 - Y1);

	if (xLength > yLength) {
		/*
		if(X1 > X2)
		{
		    Swap(&X1, &X2);
		    Swap(&Y1, &Y2);
		}
		*/

		yStep = fabs((double)(Y2 - Y1) / (double)(X2 - X1));
		Y = Y1;

		for (xCount = X1; xCount < X2; xCount++) {
			if (IsWalkableAt(xCount, (int)Y, CheckFreeObjects, Requester)) {
				*TargetX = xCount;
				*TargetY = (int)Y;
				return S_OK;
			}
			Y += yStep;
		}
	} else {
		/*
		if(Y1 > Y2) {
		    Swap(&X1, &X2);
		    Swap(&Y1, &Y2);
		}
		*/

		xStep = fabs((double)(X2 - X1) / (double)(Y2 - Y1));
		X = X1;

		for (yCount = Y1; yCount < Y2; yCount++) {
			if (IsWalkableAt((int)X, yCount, CheckFreeObjects, Requester)) {
				*TargetX = (int)X;
				*TargetY = yCount;
				return S_OK;
			}
			X += xStep;
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::CorrectTargetPoint(int StartX, int StartY, int *X, int *Y, bool CheckFreeObjects, CBObject *Requester) {
	int x = *X;
	int y = *Y;

	if (IsWalkableAt(x, y, CheckFreeObjects, Requester) || !m_MainLayer) {
		return S_OK;
	}

	// right
	int length_right = 0;
	bool found_right = false;
	for (x = *X, y = *Y; x < m_MainLayer->m_Width; x++, length_right++) {
		if (IsWalkableAt(x, y, CheckFreeObjects, Requester) && IsWalkableAt(x - 5, y, CheckFreeObjects, Requester)) {
			found_right = true;
			break;
		}
	}

	// left
	int length_left = 0;
	bool found_left = false;
	for (x = *X, y = *Y; x >= 0; x--, length_left--) {
		if (IsWalkableAt(x, y, CheckFreeObjects, Requester) && IsWalkableAt(x + 5, y, CheckFreeObjects, Requester)) {
			found_left = true;
			break;
		}
	}

	// up
	int length_up = 0;
	bool found_up = false;
	for (x = *X, y = *Y; y >= 0; y--, length_up--) {
		if (IsWalkableAt(x, y, CheckFreeObjects, Requester) && IsWalkableAt(x, y + 5, CheckFreeObjects, Requester)) {
			found_up = true;
			break;
		}
	}

	// down
	int length_down = 0;
	bool found_down = false;
	for (x = *X, y = *Y; y < m_MainLayer->m_Height; y++, length_down++) {
		if (IsWalkableAt(x, y, CheckFreeObjects, Requester) && IsWalkableAt(x, y - 5, CheckFreeObjects, Requester)) {
			found_down = true;
			break;
		}
	}

	if (!found_left && !found_right && !found_up && !found_down) {
		return S_OK;
	}

	int OffsetX = INT_MAX, OffsetY = INT_MAX;

	if (found_left && found_right) {
		if (abs(length_left) < abs(length_right)) OffsetX = length_left;
		else OffsetX = length_right;
	} else if (found_left) OffsetX = length_left;
	else if (found_right) OffsetX = length_right;

	if (found_up && found_down) {
		if (abs(length_up) < abs(length_down)) OffsetY = length_up;
		else OffsetY = length_down;
	} else if (found_up) OffsetY = length_up;
	else if (found_down) OffsetY = length_down;

	if (abs(OffsetX) < abs(OffsetY))
		*X = *X + OffsetX;
	else
		*Y = *Y + OffsetY;

	if (!IsWalkableAt(*X, *Y)) return CorrectTargetPoint2(StartX, StartY, X, Y, CheckFreeObjects, Requester);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::PFPointsStart() {
	m_PFPointsNum = 0;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::PFPointsAdd(int X, int Y, int Distance) {
	if (m_PFPointsNum >= m_PFPath.GetSize()) {
		m_PFPath.Add(new CAdPathPoint(X, Y, Distance));
	} else {
		m_PFPath[m_PFPointsNum]->x = X;
		m_PFPath[m_PFPointsNum]->y = Y;
		m_PFPath[m_PFPointsNum]->m_Distance = Distance;
		m_PFPath[m_PFPointsNum]->m_Marked = false;
		m_PFPath[m_PFPointsNum]->m_Origin = NULL;
	}

	m_PFPointsNum++;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::GetViewportOffset(int *OffsetX, int *OffsetY) {
	CAdGame *AdGame = (CAdGame *)Game;
	if (m_Viewport && !Game->m_EditorMode) {
		if (OffsetX) *OffsetX = m_Viewport->m_OffsetX;
		if (OffsetY) *OffsetY = m_Viewport->m_OffsetY;
	} else if (AdGame->m_SceneViewport && !Game->m_EditorMode) {
		if (OffsetX) *OffsetX = AdGame->m_SceneViewport->m_OffsetX;
		if (OffsetY) *OffsetY = AdGame->m_SceneViewport->m_OffsetY;
	} else {
		if (OffsetX) *OffsetX = 0;
		if (OffsetY) *OffsetY = 0;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::GetViewportSize(int *Width, int *Height) {
	CAdGame *AdGame = (CAdGame *)Game;
	if (m_Viewport && !Game->m_EditorMode) {
		if (Width)  *Width  = m_Viewport->GetWidth();
		if (Height) *Height = m_Viewport->GetHeight();
	} else if (AdGame->m_SceneViewport && !Game->m_EditorMode) {
		if (Width)  *Width  = AdGame->m_SceneViewport->GetWidth();
		if (Height) *Height = AdGame->m_SceneViewport->GetHeight();
	} else {
		if (Width)  *Width  = Game->m_Renderer->m_Width;
		if (Height) *Height = Game->m_Renderer->m_Height;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CAdScene::GetOffsetLeft() {
	int ViewportX;
	GetViewportOffset(&ViewportX);

	return m_OffsetLeft - ViewportX;
}


//////////////////////////////////////////////////////////////////////////
int CAdScene::GetOffsetTop() {
	int ViewportY;
	GetViewportOffset(NULL, &ViewportY);

	return m_OffsetTop - ViewportY;
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::PointInViewport(int X, int Y) {
	int Left, Top, Width, Height;

	GetViewportOffset(&Left, &Top);
	GetViewportSize(&Width, &Height);

	return X >= Left && X <= Left + Width && Y >= Top && Y <= Top + Height;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::SetOffset(int OffsetLeft, int OffsetTop) {
	m_OffsetLeft = OffsetLeft;
	m_OffsetTop  = OffsetTop;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdScene::GetNodeByName(char *Name) {
	int i;
	CBObject *ret = NULL;

	// dependent objects
	for (i = 0; i < m_Layers.GetSize(); i++) {
		CAdLayer *layer = m_Layers[i];
		for (int j = 0; j < layer->m_Nodes.GetSize(); j++) {
			CAdSceneNode *node = layer->m_Nodes[j];
			if ((node->m_Type == OBJECT_ENTITY && !scumm_stricmp(Name, node->m_Entity->m_Name)) ||
			        (node->m_Type == OBJECT_REGION && !scumm_stricmp(Name, node->m_Region->m_Name))) {
				switch (node->m_Type) {
				case OBJECT_ENTITY:
					ret = node->m_Entity;
					break;
				case OBJECT_REGION:
					ret = node->m_Region;
					break;
				default:
					ret = NULL;
				}
				return ret;
			}
		}
	}

	// free entities
	for (i = 0; i < m_Objects.GetSize(); i++) {
		if (m_Objects[i]->m_Type == OBJECT_ENTITY && !scumm_stricmp(Name, m_Objects[i]->m_Name)) {
			return m_Objects[i];
		}
	}

	// waypoint groups
	for (i = 0; i < m_WaypointGroups.GetSize(); i++) {
		if (!scumm_stricmp(Name, m_WaypointGroups[i]->m_Name)) {
			return m_WaypointGroups[i];
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::SaveState() {
	return PersistState(true);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::LoadState() {
	return PersistState(false);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::PersistState(bool Saving) {
	if (!m_PersistentState) return S_OK;

	CAdGame *AdGame = (CAdGame *)Game;
	CAdSceneState *State = AdGame->GetSceneState(m_Filename, Saving);
	if (!State) return S_OK;


	int i;
	CAdNodeState *NodeState;

	// dependent objects
	for (i = 0; i < m_Layers.GetSize(); i++) {
		CAdLayer *layer = m_Layers[i];
		for (int j = 0; j < layer->m_Nodes.GetSize(); j++) {
			CAdSceneNode *node = layer->m_Nodes[j];
			switch (node->m_Type) {
			case OBJECT_ENTITY:
				if (!node->m_Entity->m_SaveState) continue;
				NodeState = State->GetNodeState(node->m_Entity->m_Name, Saving);
				if (NodeState) {
					NodeState->TransferEntity(node->m_Entity, m_PersistentStateSprites, Saving);
					//if(Saving) NodeState->m_Active = node->m_Entity->m_Active;
					//else node->m_Entity->m_Active = NodeState->m_Active;
				}
				break;
			case OBJECT_REGION:
				if (!node->m_Region->m_SaveState) continue;
				NodeState = State->GetNodeState(node->m_Region->m_Name, Saving);
				if (NodeState) {
					if (Saving) NodeState->m_Active = node->m_Region->m_Active;
					else node->m_Region->m_Active = NodeState->m_Active;
				}
				break;
			}
		}
	}

	// free entities
	for (i = 0; i < m_Objects.GetSize(); i++) {
		if (!m_Objects[i]->m_SaveState) continue;
		if (m_Objects[i]->m_Type == OBJECT_ENTITY) {
			NodeState = State->GetNodeState(m_Objects[i]->m_Name, Saving);
			if (NodeState) {
				NodeState->TransferEntity((CAdEntity *)m_Objects[i], m_PersistentStateSprites, Saving);
				//if(Saving) NodeState->m_Active = m_Objects[i]->m_Active;
				//else m_Objects[i]->m_Active = NodeState->m_Active;
			}
		}
	}

	// waypoint groups
	for (i = 0; i < m_WaypointGroups.GetSize(); i++) {
		NodeState = State->GetNodeState(m_WaypointGroups[i]->m_Name, Saving);
		if (NodeState) {
			if (Saving) NodeState->m_Active = m_WaypointGroups[i]->m_Active;
			else m_WaypointGroups[i]->m_Active = NodeState->m_Active;
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
float CAdScene::GetRotationAt(int X, int Y) {
	CAdRotLevel *prev = NULL;
	CAdRotLevel *next = NULL;

	for (int i = 0; i < m_RotLevels.GetSize(); i++) {
		CAdRotLevel *xxx = m_RotLevels[i];
		int j = m_RotLevels.GetSize();
		if (m_RotLevels[i]->m_PosX < X) prev = m_RotLevels[i];
		else {
			next = m_RotLevels[i];
			break;
		}
	}

	if (prev == NULL || next == NULL) return 0;

	int delta_x = next->m_PosX - prev->m_PosX;
	float delta_rot = next->m_Rotation - prev->m_Rotation;
	X -= prev->m_PosX;

	float percent = (float)X / ((float)delta_x / 100.0f);
	return prev->m_Rotation + delta_rot / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::HandleItemAssociations(char *ItemName, bool Show) {
	int i;

	for (i = 0; i < m_Layers.GetSize(); i++) {
		CAdLayer *Layer = m_Layers[i];
		for (int j = 0; j < Layer->m_Nodes.GetSize(); j++) {
			if (Layer->m_Nodes[j]->m_Type == OBJECT_ENTITY) {
				CAdEntity *Ent = Layer->m_Nodes[j]->m_Entity;

				if (Ent->m_Item && strcmp(Ent->m_Item, ItemName) == 0) Ent->m_Active = Show;
			}
		}
	}

	for (i = 0; i < m_Objects.GetSize(); i++) {
		if (m_Objects[i]->m_Type == OBJECT_ENTITY) {
			CAdEntity *Ent = (CAdEntity *)m_Objects[i];
			if (Ent->m_Item && strcmp(Ent->m_Item, ItemName) == 0) Ent->m_Active = Show;
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::GetRegionsAt(int X, int Y, CAdRegion **RegionList, int NumRegions) {
	int i;
	int NumUsed = 0;
	if (m_MainLayer) {
		for (i = m_MainLayer->m_Nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *Node = m_MainLayer->m_Nodes[i];
			if (Node->m_Type == OBJECT_REGION && Node->m_Region->m_Active && Node->m_Region->PointInRegion(X, Y)) {
				if (NumUsed < NumRegions - 1) {
					RegionList[NumUsed] = Node->m_Region;
					NumUsed++;
				} else break;
			}
		}
	}
	for (i = NumUsed; i < NumRegions; i++) {
		RegionList[i] = NULL;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::RestoreDeviceObjects() {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdScene::GetNextAccessObject(CBObject *CurrObject) {
	CBArray<CAdObject *, CAdObject *> Objects;
	GetSceneObjects(Objects, true);

	if (Objects.GetSize() == 0) return NULL;
	else {
		if (CurrObject != NULL) {
			for (int i = 0; i < Objects.GetSize(); i++) {
				if (Objects[i] == CurrObject) {
					if (i < Objects.GetSize() - 1) return Objects[i + 1];
					else break;
				}
			}
		}
		return Objects[0];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CBObject *CAdScene::GetPrevAccessObject(CBObject *CurrObject) {
	CBArray<CAdObject *, CAdObject *> Objects;
	GetSceneObjects(Objects, true);

	if (Objects.GetSize() == 0) return NULL;
	else {
		if (CurrObject != NULL) {
			for (int i = Objects.GetSize() - 1; i >= 0; i--) {
				if (Objects[i] == CurrObject) {
					if (i > 0) return Objects[i - 1];
					else break;
				}
			}
		}
		return Objects[Objects.GetSize() - 1];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::GetSceneObjects(CBArray<CAdObject *, CAdObject *>& Objects, bool InteractiveOnly) {
	for (int i = 0; i < m_Layers.GetSize(); i++) {
		// close-up layer -> remove everything below it
		if (InteractiveOnly && m_Layers[i]->m_CloseUp) Objects.RemoveAll();


		for (int j = 0; j < m_Layers[i]->m_Nodes.GetSize(); j++) {
			CAdSceneNode *Node = m_Layers[i]->m_Nodes[j];
			switch (Node->m_Type) {
			case OBJECT_ENTITY: {
				CAdEntity *Ent = Node->m_Entity;
				if (Ent->m_Active && (Ent->m_Registrable || !InteractiveOnly))
					Objects.Add(Ent);
			}
			break;

			case OBJECT_REGION: {
				CBArray<CAdObject *, CAdObject *> RegionObj;
				GetRegionObjects(Node->m_Region, RegionObj, InteractiveOnly);
				for (int New = 0; New < RegionObj.GetSize(); New++) {
					bool Found = false;
					for (int Old = 0; Old < Objects.GetSize(); Old++) {
						if (Objects[Old] == RegionObj[New]) {
							Found = true;
							break;
						}
					}
					if (!Found) Objects.Add(RegionObj[New]);
				}
				//if(RegionObj.GetSize() > 0) Objects.Append(RegionObj);
			}
			break;
			}
		}
	}

	// objects outside any region
	CBArray<CAdObject *, CAdObject *> RegionObj;
	GetRegionObjects(NULL, RegionObj, InteractiveOnly);
	for (int New = 0; New < RegionObj.GetSize(); New++) {
		bool Found = false;
		for (int Old = 0; Old < Objects.GetSize(); Old++) {
			if (Objects[Old] == RegionObj[New]) {
				Found = true;
				break;
			}
		}
		if (!Found) Objects.Add(RegionObj[New]);
	}


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::GetRegionObjects(CAdRegion *Region, CBArray<CAdObject *, CAdObject *>& Objects, bool InteractiveOnly) {
	CAdGame *AdGame = (CAdGame *)Game;
	CAdObject *Obj;

	int i;

	// global objects
	for (i = 0; i < AdGame->m_Objects.GetSize(); i++) {
		Obj = AdGame->m_Objects[i];
		if (Obj->m_Active && (Obj->m_StickRegion == Region || Region == NULL || (Obj->m_StickRegion == NULL && Region->PointInRegion(Obj->m_PosX, Obj->m_PosY)))) {
			if (InteractiveOnly && !Obj->m_Registrable) continue;

			Objects.Add(Obj);
		}
	}

	// scene objects
	for (i = 0; i < m_Objects.GetSize(); i++) {
		Obj = m_Objects[i];
		if (Obj->m_Active && !Obj->m_EditorOnly && (Obj->m_StickRegion == Region || Region == NULL || (Obj->m_StickRegion == NULL && Region->PointInRegion(Obj->m_PosX, Obj->m_PosY)))) {
			if (InteractiveOnly && !Obj->m_Registrable) continue;

			Objects.Add(Obj);
		}
	}

	// sort by m_PosY
	qsort(Objects.GetData(), Objects.GetSize(), sizeof(CAdObject *), CAdScene::CompareObjs);

	return S_OK;
}

} // end of namespace WinterMute
