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

#include "engines/wintermute/Ad/AdScene.h"
#include "engines/wintermute/Ad/AdActor.h"
#include "engines/wintermute/Ad/AdEntity.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdLayer.h"
#include "engines/wintermute/Ad/AdNodeState.h"
#include "engines/wintermute/Ad/AdObject.h"
#include "engines/wintermute/Ad/AdPath.h"
#include "engines/wintermute/Ad/AdPathPoint.h"
#include "engines/wintermute/Ad/AdRotLevel.h"
#include "engines/wintermute/Ad/AdScaleLevel.h"
#include "engines/wintermute/Ad/AdSceneNode.h"
#include "engines/wintermute/Ad/AdSceneState.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Ad/AdWaypointGroup.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BPoint.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/Base/BScriptable.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BViewport.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/UI/UIWindow.h"
#include "engines/wintermute/utils/utils.h"
#include <limits.h>

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdScene, false)

//////////////////////////////////////////////////////////////////////////
CAdScene::CAdScene(CBGame *inGame): CBObject(inGame) {
	_pFTarget = new CBPoint;
	SetDefaults();
}


//////////////////////////////////////////////////////////////////////////
CAdScene::~CAdScene() {
	Cleanup();
	Game->UnregisterObject(_fader);
	delete _pFTarget;
	_pFTarget = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::SetDefaults() {
	_initialized = false;
	_pFReady = true;
	_pFTargetPath = NULL;
	_pFRequester = NULL;
	_mainLayer = NULL;

	_pFPointsNum = 0;
	_persistentState = false;
	_persistentStateSprites = true;

	_autoScroll = true;
	_offsetLeft = _offsetTop = 0;
	_targetOffsetLeft = _targetOffsetTop = 0;

	_lastTimeH = _lastTimeV = 0;
	_scrollTimeH = _scrollTimeV = 10;
	_scrollPixelsH = _scrollPixelsV = 1;

	_pFMaxTime = 15;

	_paralaxScrolling = true;

	// editor settings
	_editorMarginH = _editorMarginV = 100;

	_editorColFrame        = 0xE0888888;
	_editorColEntity       = 0xFF008000;
	_editorColRegion       = 0xFF0000FF;
	_editorColBlocked      = 0xFF800080;
	_editorColWaypoints    = 0xFF0000FF;
	_editorColEntitySel    = 0xFFFF0000;
	_editorColRegionSel    = 0xFFFF0000;
	_editorColBlockedSel   = 0xFFFF0000;
	_editorColWaypointsSel = 0xFFFF0000;
	_editorColScale        = 0xFF00FF00;
	_editorColDecor        = 0xFF00FFFF;
	_editorColDecorSel     = 0xFFFF0000;

	_editorShowRegions  = true;
	_editorShowBlocked  = true;
	_editorShowDecor    = true;
	_editorShowEntities = true;
	_editorShowScale    = true;

	_shieldWindow = NULL;

	_fader = new CBFader(Game);
	Game->RegisterObject(_fader);

	_viewport = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::Cleanup() {
	CBObject::Cleanup();

	_mainLayer = NULL; // reference only

	int i;

	delete _shieldWindow;
	_shieldWindow = NULL;

	Game->UnregisterObject(_fader);
	_fader = NULL;

	for (i = 0; i < _layers.GetSize(); i++)
		Game->UnregisterObject(_layers[i]);
	_layers.RemoveAll();


	for (i = 0; i < _waypointGroups.GetSize(); i++)
		Game->UnregisterObject(_waypointGroups[i]);
	_waypointGroups.RemoveAll();

	for (i = 0; i < _scaleLevels.GetSize(); i++)
		Game->UnregisterObject(_scaleLevels[i]);
	_scaleLevels.RemoveAll();

	for (i = 0; i < _rotLevels.GetSize(); i++)
		Game->UnregisterObject(_rotLevels[i]);
	_rotLevels.RemoveAll();


	for (i = 0; i < _pFPath.GetSize(); i++)
		delete _pFPath[i];
	_pFPath.RemoveAll();
	_pFPointsNum = 0;

	for (i = 0; i < _objects.GetSize(); i++)
		Game->UnregisterObject(_objects[i]);
	_objects.RemoveAll();

	delete _viewport;
	_viewport = NULL;

	SetDefaults();
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::GetPath(CBPoint source, CBPoint target, CAdPath *path, CBObject *requester) {
	if (!_pFReady) return false;
	else {
		_pFReady = false;
		*_pFTarget = target;
		_pFTargetPath = path;
		_pFRequester = requester;

		_pFTargetPath->Reset();
		_pFTargetPath->SetReady(false);

		// prepare working path
		int i;
		PFPointsStart();

		// first point
		//_pFPath.Add(new CAdPathPoint(source.x, source.y, 0));

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
		//_pFPath.Add(new CAdPathPoint(target.x, target.y, INT_MAX));
		PFPointsAdd(target.x, target.y, INT_MAX);

		// active waypoints
		for (i = 0; i < _waypointGroups.GetSize(); i++) {
			if (_waypointGroups[i]->_active) {
				PFAddWaypointGroup(_waypointGroups[i], requester);
			}
		}


		// free waypoints
		for (i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentWptGroup) {
				PFAddWaypointGroup(_objects[i]->_currentWptGroup, requester);
			}
		}
		CAdGame *AdGame = (CAdGame *)Game;
		for (i = 0; i < AdGame->_objects.GetSize(); i++) {
			if (AdGame->_objects[i]->_active && AdGame->_objects[i] != requester && AdGame->_objects[i]->_currentWptGroup) {
				PFAddWaypointGroup(AdGame->_objects[i]->_currentWptGroup, requester);
			}
		}

		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::PFAddWaypointGroup(CAdWaypointGroup *Wpt, CBObject *Requester) {
	if (!Wpt->_active) return;

	for (int i = 0; i < Wpt->_points.GetSize(); i++) {
		if (IsBlockedAt(Wpt->_points[i]->x, Wpt->_points[i]->y, true, Requester)) continue;

		//_pFPath.Add(new CAdPathPoint(Wpt->_points[i]->x, Wpt->_points[i]->y, INT_MAX));
		PFPointsAdd(Wpt->_points[i]->x, Wpt->_points[i]->y, INT_MAX);
	}
}


//////////////////////////////////////////////////////////////////////////
float CAdScene::GetZoomAt(int X, int Y) {
	float ret = 100;

	bool found = false;
	if (_mainLayer) {
		for (int i = _mainLayer->_nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *Node = _mainLayer->_nodes[i];
			if (Node->_type == OBJECT_REGION && Node->_region->_active && !Node->_region->_blocked && Node->_region->PointInRegion(X, Y)) {
				if (Node->_region->_zoom != 0) {
					ret = Node->_region->_zoom;
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
	if (!Game->_dEBUG_DebugMode) ColorCheck = false;

	uint32 ret;
	if (ColorCheck) ret = 0xFFFF0000;
	else ret = 0xFFFFFFFF;

	if (_mainLayer) {
		for (int i = _mainLayer->_nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *Node = _mainLayer->_nodes[i];
			if (Node->_type == OBJECT_REGION && Node->_region->_active && (ColorCheck || !Node->_region->_blocked) && Node->_region->PointInRegion(X, Y)) {
				if (!Node->_region->_blocked) ret = Node->_region->_alpha;
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
		for (i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && _objects[i] != Requester && _objects[i]->_currentBlockRegion) {
				if (_objects[i]->_currentBlockRegion->PointInRegion(X, Y)) return true;
			}
		}
		CAdGame *AdGame = (CAdGame *)Game;
		for (i = 0; i < AdGame->_objects.GetSize(); i++) {
			if (AdGame->_objects[i]->_active && AdGame->_objects[i] != Requester && AdGame->_objects[i]->_currentBlockRegion) {
				if (AdGame->_objects[i]->_currentBlockRegion->PointInRegion(X, Y)) return true;
			}
		}
	}


	if (_mainLayer) {
		for (int i = 0; i < _mainLayer->_nodes.GetSize(); i++) {
			CAdSceneNode *Node = _mainLayer->_nodes[i];
			/*
			if (Node->_type == OBJECT_REGION && Node->_region->_active && Node->_region->_blocked && Node->_region->PointInRegion(X, Y))
			{
			    ret = true;
			    break;
			}
			*/
			if (Node->_type == OBJECT_REGION && Node->_region->_active && !Node->_region->_decoration && Node->_region->PointInRegion(X, Y)) {
				if (Node->_region->_blocked) {
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
		for (i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && _objects[i] != Requester && _objects[i]->_currentBlockRegion) {
				if (_objects[i]->_currentBlockRegion->PointInRegion(X, Y)) return false;
			}
		}
		CAdGame *AdGame = (CAdGame *)Game;
		for (i = 0; i < AdGame->_objects.GetSize(); i++) {
			if (AdGame->_objects[i]->_active && AdGame->_objects[i] != Requester && AdGame->_objects[i]->_currentBlockRegion) {
				if (AdGame->_objects[i]->_currentBlockRegion->PointInRegion(X, Y)) return false;
			}
		}
	}


	if (_mainLayer) {
		for (int i = 0; i < _mainLayer->_nodes.GetSize(); i++) {
			CAdSceneNode *Node = _mainLayer->_nodes[i];
			if (Node->_type == OBJECT_REGION && Node->_region->_active && !Node->_region->_decoration && Node->_region->PointInRegion(X, Y)) {
				if (Node->_region->_blocked) {
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
	return MAX(xLength, yLength);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::PathFinderStep() {
	int i;
	// get lowest unmarked
	int lowest_dist = INT_MAX;
	CAdPathPoint *lowest_pt = NULL;

	for (i = 0; i < _pFPointsNum; i++)
		if (!_pFPath[i]->_marked && _pFPath[i]->_distance < lowest_dist) {
			lowest_dist = _pFPath[i]->_distance;
			lowest_pt = _pFPath[i];
		}

	if (lowest_pt == NULL) { // no path -> terminate PathFinder
		_pFReady = true;
		_pFTargetPath->SetReady(true);
		return;
	}

	lowest_pt->_marked = true;

	// target point marked, generate path and terminate
	if (lowest_pt->x == _pFTarget->x && lowest_pt->y == _pFTarget->y) {
		while (lowest_pt != NULL) {
			_pFTargetPath->_points.InsertAt(0, new CBPoint(lowest_pt->x, lowest_pt->y));
			lowest_pt = lowest_pt->_origin;
		}

		_pFReady = true;
		_pFTargetPath->SetReady(true);
		return;
	}

	// otherwise keep on searching
	for (i = 0; i < _pFPointsNum; i++)
		if (!_pFPath[i]->_marked) {
			int j = GetPointsDist(*lowest_pt, *_pFPath[i], _pFRequester);
			if (j != -1 && lowest_pt->_distance + j < _pFPath[i]->_distance) {
				_pFPath[i]->_distance = lowest_pt->_distance + j;
				_pFPath[i]->_origin = lowest_pt;
			}
		}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::InitLoop() {
#ifdef _DEBUGxxxx
	int nu_steps = 0;
	uint32 start = Game->_currentTime;
	while (!_pFReady && CBPlatform::GetTime() - start <= _pFMaxTime) {
		PathFinderStep();
		nu_steps++;
	}
	if (nu_steps > 0) Game->LOG(0, "STAT: PathFinder iterations in one loop: %d (%s)  _pFMaxTime=%d", nu_steps, _pFReady ? "finished" : "not yet done", _pFMaxTime);
#else
	uint32 start = Game->_currentTime;
	while (!_pFReady && CBPlatform::GetTime() - start <= _pFMaxTime) PathFinderStep();
#endif

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdScene::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	delete[] _filename;
	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing SCENE file '%s'", Filename);

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);


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
	/* float WaypointHeight = -1.0f; */

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
				_layers.Add(layer);
				if (layer->_main) {
					_mainLayer = layer;
					_width = layer->_width;
					_height = layer->_height;
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
				_waypointGroups.Add(wpt);
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
				_scaleLevels.Add(sl);
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
				_rotLevels.Add(rl);
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
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->LoadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CAMERA:
			strcpy(camera, (char *)params);
			break;

		case TOKEN_EDITOR_MARGIN_H:
			parser.ScanStr((char *)params, "%d", &_editorMarginH);
			break;

		case TOKEN_EDITOR_MARGIN_V:
			parser.ScanStr((char *)params, "%d", &_editorMarginV);
			break;

		case TOKEN_EDITOR_COLOR_FRAME:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColFrame = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColEntity = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColEntitySel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColRegionSel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColDecorSel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColBlockedSel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS_SEL:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColWaypointsSel = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColRegion = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColDecor = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColBlocked = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColWaypoints = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_SCALE:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColScale = DRGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_SHOW_REGIONS:
			parser.ScanStr((char *)params, "%b", &_editorShowRegions);
			break;

		case TOKEN_EDITOR_SHOW_BLOCKED:
			parser.ScanStr((char *)params, "%b", &_editorShowBlocked);
			break;

		case TOKEN_EDITOR_SHOW_DECORATION:
			parser.ScanStr((char *)params, "%b", &_editorShowDecor);
			break;

		case TOKEN_EDITOR_SHOW_ENTITIES:
			parser.ScanStr((char *)params, "%b", &_editorShowEntities);
			break;

		case TOKEN_EDITOR_SHOW_SCALE:
			parser.ScanStr((char *)params, "%b", &_editorShowScale);
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
			if (!_viewport) _viewport = new CBViewport(Game);
			if (_viewport) _viewport->SetRect(rc.left, rc.top, rc.right, rc.bottom, true);
		}

		case TOKEN_PERSISTENT_STATE:
			parser.ScanStr((char *)params, "%b", &_persistentState);
			break;

		case TOKEN_PERSISTENT_STATE_SPRITES:
			parser.ScanStr((char *)params, "%b", &_persistentStateSprites);
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

	if (_mainLayer == NULL) Game->LOG(0, "Warning: scene '%s' has no main layer.", _filename);


	SortScaleLevels();
	SortRotLevels();

	_initialized = true;


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::TraverseNodes(bool Update) {
	if (!_initialized) return S_OK;

	int j, k;
	CAdGame *AdGame = (CAdGame *)Game;


	//////////////////////////////////////////////////////////////////////////
	// prepare viewport
	bool PopViewport = false;
	if (_viewport && !Game->_editorMode) {
		Game->PushViewport(_viewport);
		PopViewport = true;
	} else if (AdGame->_sceneViewport && !Game->_editorMode) {
		Game->PushViewport(AdGame->_sceneViewport);
		PopViewport = true;
	}


	//////////////////////////////////////////////////////////////////////////
	// *** adjust scroll offset
	if (Update) {
		/*
		if (_autoScroll && Game->_mainObject != NULL)
		{
		    ScrollToObject(Game->_mainObject);
		}
		*/

		if (_autoScroll) {
			// adjust horizontal scroll
			if (Game->_timer - _lastTimeH >= _scrollTimeH) {
				_lastTimeH = Game->_timer;
				if (_offsetLeft < _targetOffsetLeft) {
					_offsetLeft += _scrollPixelsH;
					_offsetLeft = MIN(_offsetLeft, _targetOffsetLeft);
				} else if (_offsetLeft > _targetOffsetLeft) {
					_offsetLeft -= _scrollPixelsH;
					_offsetLeft = MAX(_offsetLeft, _targetOffsetLeft);
				}
			}

			// adjust vertical scroll
			if (Game->_timer - _lastTimeV >= _scrollTimeV) {
				_lastTimeV = Game->_timer;
				if (_offsetTop < _targetOffsetTop) {
					_offsetTop += _scrollPixelsV;
					_offsetTop = MIN(_offsetTop, _targetOffsetTop);
				} else if (_offsetTop > _targetOffsetTop) {
					_offsetTop -= _scrollPixelsV;
					_offsetTop = MAX(_offsetTop, _targetOffsetTop);
				}
			}

			if (_offsetTop == _targetOffsetTop && _offsetLeft == _targetOffsetLeft) _ready = true;
		} else _ready = true; // not scrolling, i.e. always ready
	}




	//////////////////////////////////////////////////////////////////////////
	int ViewportWidth, ViewportHeight;
	GetViewportSize(&ViewportWidth, &ViewportHeight);

	int ViewportX, ViewportY;
	GetViewportOffset(&ViewportX, &ViewportY);

	int ScrollableX = _width  - ViewportWidth;
	int ScrollableY = _height - ViewportHeight;

	double WidthRatio  = ScrollableX <= 0 ? 0 : ((double)(_offsetLeft) / (double)ScrollableX);
	double HeightRatio = ScrollableY <= 0 ? 0 : ((double)(_offsetTop)  / (double)ScrollableY);

	int OrigX, OrigY;
	Game->GetOffset(&OrigX, &OrigY);



	//////////////////////////////////////////////////////////////////////////
	// *** display/update everything
	Game->_renderer->Setup2D();

	// for each layer
	/* int MainOffsetX = 0; */
	/* int MainOffsetY = 0; */

	for (j = 0; j < _layers.GetSize(); j++) {
		if (!_layers[j]->_active) continue;

		// make layer exclusive
		if (!Update) {
			if (_layers[j]->_closeUp && !Game->_editorMode) {
				if (!_shieldWindow) _shieldWindow = new CUIWindow(Game);
				if (_shieldWindow) {
					_shieldWindow->_posX = _shieldWindow->_posY = 0;
					_shieldWindow->_width = Game->_renderer->_width;
					_shieldWindow->_height = Game->_renderer->_height;
					_shieldWindow->Display();
				}
			}
		}

		if (_paralaxScrolling) {
			int OffsetX = (int)(WidthRatio  * (_layers[j]->_width  - ViewportWidth) - ViewportX);
			int OffsetY = (int)(HeightRatio * (_layers[j]->_height - ViewportHeight) - ViewportY);
			Game->SetOffset(OffsetX, OffsetY);

			Game->_offsetPercentX = (float)OffsetX / ((float)_layers[j]->_width - ViewportWidth) * 100.0f;
			Game->_offsetPercentY = (float)OffsetY / ((float)_layers[j]->_height - ViewportHeight) * 100.0f;

			//Game->QuickMessageForm("%d %f", OffsetX+ViewportX, Game->_offsetPercentX);
		} else {
			Game->SetOffset(_offsetLeft - ViewportX, _offsetTop - ViewportY);

			Game->_offsetPercentX = (float)(_offsetLeft - ViewportX) / ((float)_layers[j]->_width - ViewportWidth) * 100.0f;
			Game->_offsetPercentY = (float)(_offsetTop - ViewportY) / ((float)_layers[j]->_height - ViewportHeight) * 100.0f;
		}


		// for each node
		for (k = 0; k < _layers[j]->_nodes.GetSize(); k++) {
			CAdSceneNode *Node = _layers[j]->_nodes[k];
			switch (Node->_type) {
			case OBJECT_ENTITY:
				if (Node->_entity->_active && (Game->_editorMode || !Node->_entity->_editorOnly)) {
					Game->_renderer->Setup2D();

					if (Update) Node->_entity->Update();
					else Node->_entity->Display();
				}
				break;

			case OBJECT_REGION: {
				if (Node->_region->_blocked) break;
				if (Node->_region->_decoration) break;

				if (!Update) DisplayRegionContent(Node->_region);
			}
			break;
			default:
				error("AdScene::TraverseNodes - Unhandled enum");
				break;
			} // switch
		} // each node

		// display/update all objects which are off-regions
		if (_layers[j]->_main) {
			if (Update) {
				UpdateFreeObjects();
			} else {
				DisplayRegionContent(NULL);
			}
		}
	} // each layer


	// restore state
	Game->SetOffset(OrigX, OrigY);
	Game->_renderer->Setup2D();

	// display/update fader
	if (_fader) {
		if (Update) _fader->Update();
		else _fader->Display();
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
	for (i = 0; i < AdGame->_objects.GetSize(); i++) {
		if (!AdGame->_objects[i]->_active) continue;

		AdGame->_objects[i]->Update();
		AdGame->_objects[i]->_drawn = false;
	}


	for (i = 0; i < _objects.GetSize(); i++) {
		if (!_objects[i]->_active) continue;

		_objects[i]->Update();
		_objects[i]->_drawn = false;
	}


	if (_autoScroll && Game->_mainObject != NULL) {
		ScrollToObject(Game->_mainObject);
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
	for (i = 0; i < AdGame->_objects.GetSize(); i++) {
		Obj = AdGame->_objects[i];
		if (Obj->_active && !Obj->_drawn && (Obj->_stickRegion == Region || Region == NULL || (Obj->_stickRegion == NULL && Region->PointInRegion(Obj->_posX, Obj->_posY)))) {
			Objects.Add(Obj);
		}
	}

	// scene objects
	for (i = 0; i < _objects.GetSize(); i++) {
		Obj = _objects[i];
		if (Obj->_active && !Obj->_editorOnly && !Obj->_drawn && (Obj->_stickRegion == Region || Region == NULL || (Obj->_stickRegion == NULL && Region->PointInRegion(Obj->_posX, Obj->_posY)))) {
			Objects.Add(Obj);
		}
	}

	// sort by _posY
	qsort(Objects.GetData(), Objects.GetSize(), sizeof(CAdObject *), CAdScene::CompareObjs);

	// display them
	for (i = 0; i < Objects.GetSize(); i++) {
		Obj = Objects[i];

		if (Display3DOnly && !Obj->_is3D) continue;

		Game->_renderer->Setup2D();

		if (Game->_editorMode || !Obj->_editorOnly) Obj->Display();
		Obj->_drawn = true;
	}


	// display design only objects
	if (!Display3DOnly) {
		if (Game->_editorMode && Region == NULL) {
			for (i = 0; i < _objects.GetSize(); i++) {
				if (_objects[i]->_active && _objects[i]->_editorOnly) {
					_objects[i]->Display();
					_objects[i]->_drawn = true;
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

	if (Object1->_posY < Object2->_posY) return -1;
	else if (Object1->_posY > Object2->_posY) return 1;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::DisplayRegionContentOld(CAdRegion *Region) {
	CAdGame *AdGame = (CAdGame *)Game;
	CAdObject *obj;
	int i;

	// display all objects in region sorted by _posY
	do {
		obj = NULL;
		int minY = INT_MAX;

		// global objects
		for (i = 0; i < AdGame->_objects.GetSize(); i++) {
			if (AdGame->_objects[i]->_active && !AdGame->_objects[i]->_drawn && AdGame->_objects[i]->_posY < minY && (AdGame->_objects[i]->_stickRegion == Region || Region == NULL || (AdGame->_objects[i]->_stickRegion == NULL && Region->PointInRegion(AdGame->_objects[i]->_posX, AdGame->_objects[i]->_posY)))) {
				obj = AdGame->_objects[i];
				minY = AdGame->_objects[i]->_posY;
			}
		}

		// scene objects
		for (i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && !_objects[i]->_editorOnly && !_objects[i]->_drawn && _objects[i]->_posY < minY && (_objects[i]->_stickRegion == Region || Region == NULL || (_objects[i]->_stickRegion == NULL && Region->PointInRegion(_objects[i]->_posX, _objects[i]->_posY)))) {
				obj = _objects[i];
				minY = _objects[i]->_posY;
			}
		}


		if (obj != NULL) {
			Game->_renderer->Setup2D();

			if (Game->_editorMode || !obj->_editorOnly) obj->Display();
			obj->_drawn = true;
		}
	} while (obj != NULL);


	// design only objects
	if (Game->_editorMode && Region == NULL) {
		for (i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && _objects[i]->_editorOnly) {
				_objects[i]->Display();
				_objects[i]->_drawn = true;
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

	int OrigOffsetLeft = _targetOffsetLeft;
	int OrigOffsetTop = _targetOffsetTop;

	_targetOffsetLeft = MAX(0, OffsetX - ViewportWidth / 2);
	_targetOffsetLeft = MIN(_targetOffsetLeft, _width - ViewportWidth);

	_targetOffsetTop = MAX(0, OffsetY - ViewportHeight / 2);
	_targetOffsetTop = MIN(_targetOffsetTop, _height - ViewportHeight);


	if (Game->_mainObject && Game->_mainObject->_is3D) {
		if (abs(OrigOffsetLeft - _targetOffsetLeft) < 5) _targetOffsetLeft = OrigOffsetLeft;
		if (abs(OrigOffsetTop - _targetOffsetTop) < 5) _targetOffsetTop = OrigOffsetTop;
		//_targetOffsetTop = 0;
	}

	_ready = false;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::ScrollToObject(CBObject *Object) {
	if (Object) ScrollTo(Object->_posX, Object->_posY - Object->GetHeight() / 2);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::SkipToObject(CBObject *Object) {
	if (Object) SkipTo(Object->_posX, Object->_posY - Object->GetHeight() / 2);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::SkipTo(int OffsetX, int OffsetY) {
	int ViewportWidth, ViewportHeight;
	GetViewportSize(&ViewportWidth, &ViewportHeight);

	_offsetLeft = MAX(0, OffsetX - ViewportWidth / 2);
	_offsetLeft = MIN(_offsetLeft, _width - ViewportWidth);

	_offsetTop = MAX(0, OffsetY - ViewportHeight / 2);
	_offsetTop = MIN(_offsetTop, _height - ViewportHeight);

	_targetOffsetLeft = _offsetLeft;
	_targetOffsetTop = _offsetTop;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
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
			if (layer < 0 || layer >= _layers.GetSize()) Stack->PushNULL();
			else Stack->PushNative(_layers[layer], true);
		} else {
			const char *LayerName = val->GetString();
			bool LayerFound = false;
			for (int i = 0; i < _layers.GetSize(); i++) {
				if (scumm_stricmp(LayerName, _layers[i]->_name) == 0) {
					Stack->PushNative(_layers[i], true);
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
		if (group < 0 || group >= _waypointGroups.GetSize()) Stack->PushNULL();
		else Stack->PushNative(_waypointGroups[group], true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetNode") == 0) {
		Stack->CorrectParams(1);
		const char *nodeName = Stack->Pop()->GetString();

		CBObject *node = GetNodeByName(nodeName);
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
			if (Index >= 0 && Index < _objects.GetSize()) Ret = _objects[Index];
		} else {
			const char *nodeName = Val->GetString();
			for (int i = 0; i < _objects.GetSize(); i++) {
				if (_objects[i] && _objects[i]->_name && scumm_stricmp(_objects[i]->_name, nodeName) == 0) {
					Ret = _objects[i];
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

		if (_mainLayer) {
			for (int i = _mainLayer->_nodes.GetSize() - 1; i >= 0; i--) {
				CAdSceneNode *Node = _mainLayer->_nodes[i];
				if (Node->_type == OBJECT_REGION && Node->_region->_active && Node->_region->PointInRegion(X, Y)) {
					if (Node->_region->_decoration && !IncludeDecors) continue;

					Stack->PushNative(Node->_region, true);
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
		if (_autoScroll) {
			if (_targetOffsetLeft != _offsetLeft || _targetOffsetTop != _offsetTop) Ret = true;
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

		_fader->FadeOut(DRGBA(Red, Green, Blue, Alpha), Duration);
		if (strcmp(Name, "FadeOutAsync") != 0) Script->WaitFor(_fader);

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

		_fader->FadeIn(DRGBA(Red, Green, Blue, Alpha), Duration);
		if (strcmp(Name, "FadeInAsync") != 0) Script->WaitFor(_fader);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFadeColor") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_fader->GetCurrentColor());
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

		if (Width <= 0) Width = Game->_renderer->_width;
		if (Height <= 0) Height = Game->_renderer->_height;

		if (!_viewport) _viewport = new CBViewport(Game);
		if (_viewport) _viewport->SetRect(X, Y, X + Width, Y + Height);

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
		if (_mainLayer) {
			Layer->_width = _mainLayer->_width;
			Layer->_height = _mainLayer->_height;
		}
		_layers.Add(Layer);
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
		if (_mainLayer) {
			Layer->_width = _mainLayer->_width;
			Layer->_height = _mainLayer->_height;
		}
		if (Index < 0) Index = 0;
		if (Index <= _layers.GetSize() - 1) _layers.InsertAt(Index, Layer);
		else _layers.Add(Layer);

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
			for (int i = 0; i < _layers.GetSize(); i++) {
				if (_layers[i] == Temp) {
					ToDelete = _layers[i];
					break;
				}
			}
		} else {
			int Index = Val->GetInt();
			if (Index >= 0 && Index < _layers.GetSize()) {
				ToDelete = _layers[Index];
			}
		}
		if (ToDelete == NULL) {
			Stack->PushBool(false);
			return S_OK;
		}

		if (ToDelete->_main) {
			Script->RuntimeError("Scene.DeleteLayer - cannot delete main scene layer");
			Stack->PushBool(false);
			return S_OK;
		}

		for (int i = 0; i < _layers.GetSize(); i++) {
			if (_layers[i] == ToDelete) {
				_layers.RemoveAt(i);
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
CScValue *CAdScene::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("scene");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumLayers (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumLayers") == 0) {
		_scValue->SetInt(_layers.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumWaypointGroups (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumWaypointGroups") == 0) {
		_scValue->SetInt(_waypointGroups.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainLayer (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MainLayer") == 0) {
		if (_mainLayer) _scValue->SetNative(_mainLayer, true);
		else _scValue->SetNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFreeNodes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumFreeNodes") == 0) {
		_scValue->SetInt(_objects.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MouseX") == 0) {
		int ViewportX;
		GetViewportOffset(&ViewportX);

		_scValue->SetInt(Game->_mousePos.x + _offsetLeft - ViewportX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MouseY") == 0) {
		int ViewportY;
		GetViewportOffset(NULL, &ViewportY);

		_scValue->SetInt(Game->_mousePos.y + _offsetTop - ViewportY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoScroll
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutoScroll") == 0) {
		_scValue->SetBool(_autoScroll);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PersistentState") == 0) {
		_scValue->SetBool(_persistentState);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentStateSprites
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PersistentStateSprites") == 0) {
		_scValue->SetBool(_persistentStateSprites);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollPixelsX") == 0) {
		_scValue->SetInt(_scrollPixelsH);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollPixelsY") == 0) {
		_scValue->SetInt(_scrollPixelsV);
		return _scValue;
	}


	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollSpeedX") == 0) {
		_scValue->SetInt(_scrollTimeH);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollSpeedY") == 0) {
		_scValue->SetInt(_scrollTimeV);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OffsetX") == 0) {
		_scValue->SetInt(_offsetLeft);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OffsetY") == 0) {
		_scValue->SetInt(_offsetTop);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		if (_mainLayer) _scValue->SetInt(_mainLayer->_width);
		else _scValue->SetInt(0);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		if (_mainLayer) _scValue->SetInt(_mainLayer->_height);
		else _scValue->SetInt(0);
		return _scValue;
	}

	else return CBObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::ScSetProperty(const char *Name, CScValue *Value) {
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
		_autoScroll = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PersistentState") == 0) {
		_persistentState = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentStateSprites
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PersistentStateSprites") == 0) {
		_persistentStateSprites = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollPixelsX") == 0) {
		_scrollPixelsH = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollPixelsY") == 0) {
		_scrollPixelsV = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollSpeedX") == 0) {
		_scrollTimeH = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScrollSpeedY") == 0) {
		_scrollTimeV = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OffsetX") == 0) {
		_offsetLeft = Value->GetInt();

		int ViewportWidth, ViewportHeight;
		GetViewportSize(&ViewportWidth, &ViewportHeight);

		_offsetLeft = MAX(0, _offsetLeft - ViewportWidth / 2);
		_offsetLeft = MIN(_offsetLeft, _width - ViewportWidth);
		_targetOffsetLeft = _offsetLeft;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OffsetY") == 0) {
		_offsetTop = Value->GetInt();

		int ViewportWidth, ViewportHeight;
		GetViewportSize(&ViewportWidth, &ViewportHeight);

		_offsetTop = MAX(0, _offsetTop - ViewportHeight / 2);
		_offsetTop = MIN(_offsetTop, _height - ViewportHeight);
		_targetOffsetTop = _offsetTop;

		return S_OK;
	}

	else return CBObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdScene::ScToString() {
	return "[scene object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::AddObject(CAdObject *Object) {
	_objects.Add(Object);
	return Game->RegisterObject(Object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::RemoveObject(CAdObject *Object) {
	for (int i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i] == Object) {
			_objects.RemoveAt(i);
			return Game->UnregisterObject(Object);
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	int i;

	Buffer->PutTextIndent(Indent, "SCENE {\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	if (_persistentState)
		Buffer->PutTextIndent(Indent + 2, "PERSISTENT_STATE=%s\n", _persistentState ? "TRUE" : "FALSE");

	if (!_persistentStateSprites)
		Buffer->PutTextIndent(Indent + 2, "PERSISTENT_STATE_SPRITES=%s\n", _persistentStateSprites ? "TRUE" : "FALSE");


	// scripts
	for (i = 0; i < _scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	// properties
	if (_scProp) _scProp->SaveAsText(Buffer, Indent + 2);

	// viewport
	if (_viewport) {
		RECT *rc = _viewport->GetRect();
		Buffer->PutTextIndent(Indent + 2, "VIEWPORT { %d, %d, %d, %d }\n", rc->left, rc->top, rc->right, rc->bottom);
	}



	// editor settings
	Buffer->PutTextIndent(Indent + 2, "; ----- editor settings\n");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_MARGIN_H=%d\n", _editorMarginH);
	Buffer->PutTextIndent(Indent + 2, "EDITOR_MARGIN_V=%d\n", _editorMarginV);
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_FRAME { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColFrame), D3DCOLGetG(_editorColFrame), D3DCOLGetB(_editorColFrame), D3DCOLGetA(_editorColFrame));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_ENTITY_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColEntitySel), D3DCOLGetG(_editorColEntitySel), D3DCOLGetB(_editorColEntitySel), D3DCOLGetA(_editorColEntitySel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_REGION_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColRegionSel), D3DCOLGetG(_editorColRegionSel), D3DCOLGetB(_editorColRegionSel), D3DCOLGetA(_editorColRegionSel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_BLOCKED_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColBlockedSel), D3DCOLGetG(_editorColBlockedSel), D3DCOLGetB(_editorColBlockedSel), D3DCOLGetA(_editorColBlockedSel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_DECORATION_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColDecorSel), D3DCOLGetG(_editorColDecorSel), D3DCOLGetB(_editorColDecorSel), D3DCOLGetA(_editorColDecorSel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_WAYPOINTS_SEL { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColWaypointsSel), D3DCOLGetG(_editorColWaypointsSel), D3DCOLGetB(_editorColWaypointsSel), D3DCOLGetA(_editorColWaypointsSel));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_ENTITY { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColEntity), D3DCOLGetG(_editorColEntity), D3DCOLGetB(_editorColEntity), D3DCOLGetA(_editorColEntity));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_REGION { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColRegion), D3DCOLGetG(_editorColRegion), D3DCOLGetB(_editorColRegion), D3DCOLGetA(_editorColRegion));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_DECORATION { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColDecor), D3DCOLGetG(_editorColDecor), D3DCOLGetB(_editorColDecor), D3DCOLGetA(_editorColDecor));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_BLOCKED { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColBlocked), D3DCOLGetG(_editorColBlocked), D3DCOLGetB(_editorColBlocked), D3DCOLGetA(_editorColBlocked));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_WAYPOINTS { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColWaypoints), D3DCOLGetG(_editorColWaypoints), D3DCOLGetB(_editorColWaypoints), D3DCOLGetA(_editorColWaypoints));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_COLOR_SCALE { %d,%d,%d,%d }\n", D3DCOLGetR(_editorColScale), D3DCOLGetG(_editorColScale), D3DCOLGetB(_editorColScale), D3DCOLGetA(_editorColScale));

	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_REGIONS=%s\n", _editorShowRegions ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_BLOCKED=%s\n", _editorShowBlocked ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_DECORATION=%s\n", _editorShowDecor ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_ENTITIES=%s\n", _editorShowEntities ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SHOW_SCALE=%s\n", _editorShowScale ? "TRUE" : "FALSE");

	Buffer->PutTextIndent(Indent + 2, "\n");

	CBBase::SaveAsText(Buffer, Indent + 2);

	// waypoints
	Buffer->PutTextIndent(Indent + 2, "; ----- waypoints\n");
	for (i = 0; i < _waypointGroups.GetSize(); i++) _waypointGroups[i]->SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// layers
	Buffer->PutTextIndent(Indent + 2, "; ----- layers\n");
	for (i = 0; i < _layers.GetSize(); i++) _layers[i]->SaveAsText(Buffer, Indent + 2);

	// scale levels
	Buffer->PutTextIndent(Indent + 2, "; ----- scale levels\n");
	for (i = 0; i < _scaleLevels.GetSize(); i++) _scaleLevels[i]->SaveAsText(Buffer, Indent + 2);

	// rotation levels
	Buffer->PutTextIndent(Indent + 2, "; ----- rotation levels\n");
	for (i = 0; i < _rotLevels.GetSize(); i++) _rotLevels[i]->SaveAsText(Buffer, Indent + 2);


	Buffer->PutTextIndent(Indent + 2, "\n");

	// free entities
	Buffer->PutTextIndent(Indent + 2, "; ----- free entities\n");
	for (i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY) {
			_objects[i]->SaveAsText(Buffer, Indent + 2);

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
		for (int i = 0; i < _scaleLevels.GetSize() - 1; i++) {
			if (_scaleLevels[i]->_posY > _scaleLevels[i + 1]->_posY) {
				CAdScaleLevel *sl = _scaleLevels[i];
				_scaleLevels[i] = _scaleLevels[i + 1];
				_scaleLevels[i + 1] = sl;

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
		for (int i = 0; i < _rotLevels.GetSize() - 1; i++) {
			if (_rotLevels[i]->_posX > _rotLevels[i + 1]->_posX) {
				CAdRotLevel *rl = _rotLevels[i];
				_rotLevels[i] = _rotLevels[i + 1];
				_rotLevels[i + 1] = rl;

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

	for (int i = 0; i < _scaleLevels.GetSize(); i++) {
		/* CAdScaleLevel *xxx = _scaleLevels[i];*/
		/* int j = _scaleLevels.GetSize(); */
		if (_scaleLevels[i]->_posY < Y) prev = _scaleLevels[i];
		else {
			next = _scaleLevels[i];
			break;
		}
	}

	if (prev == NULL || next == NULL) return 100;

	int delta_y = next->_posY - prev->_posY;
	float delta_scale = next->_scale - prev->_scale;
	Y -= prev->_posY;

	float percent = (float)Y / ((float)delta_y / 100.0f);
	return prev->_scale + delta_scale / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::Persist(CBPersistMgr *persistMgr) {
	CBObject::Persist(persistMgr);

	persistMgr->transfer(TMEMBER(_autoScroll));
	persistMgr->transfer(TMEMBER(_editorColBlocked));
	persistMgr->transfer(TMEMBER(_editorColBlockedSel));
	persistMgr->transfer(TMEMBER(_editorColDecor));
	persistMgr->transfer(TMEMBER(_editorColDecorSel));
	persistMgr->transfer(TMEMBER(_editorColEntity));
	persistMgr->transfer(TMEMBER(_editorColEntitySel));
	persistMgr->transfer(TMEMBER(_editorColFrame));
	persistMgr->transfer(TMEMBER(_editorColRegion));
	persistMgr->transfer(TMEMBER(_editorColRegionSel));
	persistMgr->transfer(TMEMBER(_editorColScale));
	persistMgr->transfer(TMEMBER(_editorColWaypoints));
	persistMgr->transfer(TMEMBER(_editorColWaypointsSel));
	persistMgr->transfer(TMEMBER(_editorMarginH));
	persistMgr->transfer(TMEMBER(_editorMarginV));
	persistMgr->transfer(TMEMBER(_editorShowBlocked));
	persistMgr->transfer(TMEMBER(_editorShowDecor));
	persistMgr->transfer(TMEMBER(_editorShowEntities));
	persistMgr->transfer(TMEMBER(_editorShowRegions));
	persistMgr->transfer(TMEMBER(_editorShowScale));
	persistMgr->transfer(TMEMBER(_fader));
	persistMgr->transfer(TMEMBER(_height));
	persistMgr->transfer(TMEMBER(_initialized));
	persistMgr->transfer(TMEMBER(_lastTimeH));
	persistMgr->transfer(TMEMBER(_lastTimeV));
	_layers.Persist(persistMgr);
	persistMgr->transfer(TMEMBER(_mainLayer));
	_objects.Persist(persistMgr);
	persistMgr->transfer(TMEMBER(_offsetLeft));
	persistMgr->transfer(TMEMBER(_offsetTop));
	persistMgr->transfer(TMEMBER(_paralaxScrolling));
	persistMgr->transfer(TMEMBER(_persistentState));
	persistMgr->transfer(TMEMBER(_persistentStateSprites));
	persistMgr->transfer(TMEMBER(_pFMaxTime));
	_pFPath.Persist(persistMgr);
	persistMgr->transfer(TMEMBER(_pFPointsNum));
	persistMgr->transfer(TMEMBER(_pFReady));
	persistMgr->transfer(TMEMBER(_pFRequester));
	persistMgr->transfer(TMEMBER(_pFTarget));
	persistMgr->transfer(TMEMBER(_pFTargetPath));
	_rotLevels.Persist(persistMgr);
	_scaleLevels.Persist(persistMgr);
	persistMgr->transfer(TMEMBER(_scrollPixelsH));
	persistMgr->transfer(TMEMBER(_scrollPixelsV));
	persistMgr->transfer(TMEMBER(_scrollTimeH));
	persistMgr->transfer(TMEMBER(_scrollTimeV));
	persistMgr->transfer(TMEMBER(_shieldWindow));
	persistMgr->transfer(TMEMBER(_targetOffsetLeft));
	persistMgr->transfer(TMEMBER(_targetOffsetTop));
	_waypointGroups.Persist(persistMgr);
	persistMgr->transfer(TMEMBER(_viewport));
	persistMgr->transfer(TMEMBER(_width));

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
		if (X1 > X2)
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
		if (Y1 > Y2) {
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

	if (IsWalkableAt(x, y, CheckFreeObjects, Requester) || !_mainLayer) {
		return S_OK;
	}

	// right
	int length_right = 0;
	bool found_right = false;
	for (x = *X, y = *Y; x < _mainLayer->_width; x++, length_right++) {
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
	for (x = *X, y = *Y; y < _mainLayer->_height; y++, length_down++) {
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
	_pFPointsNum = 0;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::PFPointsAdd(int X, int Y, int Distance) {
	if (_pFPointsNum >= _pFPath.GetSize()) {
		_pFPath.Add(new CAdPathPoint(X, Y, Distance));
	} else {
		_pFPath[_pFPointsNum]->x = X;
		_pFPath[_pFPointsNum]->y = Y;
		_pFPath[_pFPointsNum]->_distance = Distance;
		_pFPath[_pFPointsNum]->_marked = false;
		_pFPath[_pFPointsNum]->_origin = NULL;
	}

	_pFPointsNum++;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::GetViewportOffset(int *OffsetX, int *OffsetY) {
	CAdGame *AdGame = (CAdGame *)Game;
	if (_viewport && !Game->_editorMode) {
		if (OffsetX) *OffsetX = _viewport->_offsetX;
		if (OffsetY) *OffsetY = _viewport->_offsetY;
	} else if (AdGame->_sceneViewport && !Game->_editorMode) {
		if (OffsetX) *OffsetX = AdGame->_sceneViewport->_offsetX;
		if (OffsetY) *OffsetY = AdGame->_sceneViewport->_offsetY;
	} else {
		if (OffsetX) *OffsetX = 0;
		if (OffsetY) *OffsetY = 0;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::GetViewportSize(int *Width, int *Height) {
	CAdGame *AdGame = (CAdGame *)Game;
	if (_viewport && !Game->_editorMode) {
		if (Width)  *Width  = _viewport->GetWidth();
		if (Height) *Height = _viewport->GetHeight();
	} else if (AdGame->_sceneViewport && !Game->_editorMode) {
		if (Width)  *Width  = AdGame->_sceneViewport->GetWidth();
		if (Height) *Height = AdGame->_sceneViewport->GetHeight();
	} else {
		if (Width)  *Width  = Game->_renderer->_width;
		if (Height) *Height = Game->_renderer->_height;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CAdScene::GetOffsetLeft() {
	int ViewportX;
	GetViewportOffset(&ViewportX);

	return _offsetLeft - ViewportX;
}


//////////////////////////////////////////////////////////////////////////
int CAdScene::GetOffsetTop() {
	int ViewportY;
	GetViewportOffset(NULL, &ViewportY);

	return _offsetTop - ViewportY;
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
	_offsetLeft = OffsetLeft;
	_offsetTop  = OffsetTop;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdScene::GetNodeByName(const char *Name) {
	int i;
	CBObject *ret = NULL;

	// dependent objects
	for (i = 0; i < _layers.GetSize(); i++) {
		CAdLayer *layer = _layers[i];
		for (int j = 0; j < layer->_nodes.GetSize(); j++) {
			CAdSceneNode *node = layer->_nodes[j];
			if ((node->_type == OBJECT_ENTITY && !scumm_stricmp(Name, node->_entity->_name)) ||
			        (node->_type == OBJECT_REGION && !scumm_stricmp(Name, node->_region->_name))) {
				switch (node->_type) {
				case OBJECT_ENTITY:
					ret = node->_entity;
					break;
				case OBJECT_REGION:
					ret = node->_region;
					break;
				default:
					ret = NULL;
				}
				return ret;
			}
		}
	}

	// free entities
	for (i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY && !scumm_stricmp(Name, _objects[i]->_name)) {
			return _objects[i];
		}
	}

	// waypoint groups
	for (i = 0; i < _waypointGroups.GetSize(); i++) {
		if (!scumm_stricmp(Name, _waypointGroups[i]->_name)) {
			return _waypointGroups[i];
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
	if (!_persistentState) return S_OK;

	CAdGame *AdGame = (CAdGame *)Game;
	CAdSceneState *State = AdGame->GetSceneState(_filename, Saving);
	if (!State) return S_OK;


	int i;
	CAdNodeState *NodeState;

	// dependent objects
	for (i = 0; i < _layers.GetSize(); i++) {
		CAdLayer *layer = _layers[i];
		for (int j = 0; j < layer->_nodes.GetSize(); j++) {
			CAdSceneNode *node = layer->_nodes[j];
			switch (node->_type) {
			case OBJECT_ENTITY:
				if (!node->_entity->_saveState) continue;
				NodeState = State->GetNodeState(node->_entity->_name, Saving);
				if (NodeState) {
					NodeState->TransferEntity(node->_entity, _persistentStateSprites, Saving);
					//if(Saving) NodeState->_active = node->_entity->_active;
					//else node->_entity->_active = NodeState->_active;
				}
				break;
			case OBJECT_REGION:
				if (!node->_region->_saveState) continue;
				NodeState = State->GetNodeState(node->_region->_name, Saving);
				if (NodeState) {
					if (Saving) NodeState->_active = node->_region->_active;
					else node->_region->_active = NodeState->_active;
				}
				break;
			}
		}
	}

	// free entities
	for (i = 0; i < _objects.GetSize(); i++) {
		if (!_objects[i]->_saveState) continue;
		if (_objects[i]->_type == OBJECT_ENTITY) {
			NodeState = State->GetNodeState(_objects[i]->_name, Saving);
			if (NodeState) {
				NodeState->TransferEntity((CAdEntity *)_objects[i], _persistentStateSprites, Saving);
				//if(Saving) NodeState->_active = _objects[i]->_active;
				//else _objects[i]->_active = NodeState->_active;
			}
		}
	}

	// waypoint groups
	for (i = 0; i < _waypointGroups.GetSize(); i++) {
		NodeState = State->GetNodeState(_waypointGroups[i]->_name, Saving);
		if (NodeState) {
			if (Saving) NodeState->_active = _waypointGroups[i]->_active;
			else _waypointGroups[i]->_active = NodeState->_active;
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
float CAdScene::GetRotationAt(int X, int Y) {
	CAdRotLevel *prev = NULL;
	CAdRotLevel *next = NULL;

	for (int i = 0; i < _rotLevels.GetSize(); i++) {
	/*	CAdRotLevel *xxx = _rotLevels[i];
		int j = _rotLevels.GetSize();*/
		if (_rotLevels[i]->_posX < X) prev = _rotLevels[i];
		else {
			next = _rotLevels[i];
			break;
		}
	}

	if (prev == NULL || next == NULL) return 0;

	int delta_x = next->_posX - prev->_posX;
	float delta_rot = next->_rotation - prev->_rotation;
	X -= prev->_posX;

	float percent = (float)X / ((float)delta_x / 100.0f);
	return prev->_rotation + delta_rot / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::HandleItemAssociations(const char *ItemName, bool Show) {
	int i;

	for (i = 0; i < _layers.GetSize(); i++) {
		CAdLayer *Layer = _layers[i];
		for (int j = 0; j < Layer->_nodes.GetSize(); j++) {
			if (Layer->_nodes[j]->_type == OBJECT_ENTITY) {
				CAdEntity *Ent = Layer->_nodes[j]->_entity;

				if (Ent->_item && strcmp(Ent->_item, ItemName) == 0) Ent->_active = Show;
			}
		}
	}

	for (i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY) {
			CAdEntity *Ent = (CAdEntity *)_objects[i];
			if (Ent->_item && strcmp(Ent->_item, ItemName) == 0) Ent->_active = Show;
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdScene::GetRegionsAt(int X, int Y, CAdRegion **RegionList, int NumRegions) {
	int i;
	int NumUsed = 0;
	if (_mainLayer) {
		for (i = _mainLayer->_nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *Node = _mainLayer->_nodes[i];
			if (Node->_type == OBJECT_REGION && Node->_region->_active && Node->_region->PointInRegion(X, Y)) {
				if (NumUsed < NumRegions - 1) {
					RegionList[NumUsed] = Node->_region;
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
HRESULT CAdScene::GetSceneObjects(CBArray<CAdObject *, CAdObject *> &Objects, bool InteractiveOnly) {
	for (int i = 0; i < _layers.GetSize(); i++) {
		// close-up layer -> remove everything below it
		if (InteractiveOnly && _layers[i]->_closeUp) Objects.RemoveAll();


		for (int j = 0; j < _layers[i]->_nodes.GetSize(); j++) {
			CAdSceneNode *Node = _layers[i]->_nodes[j];
			switch (Node->_type) {
			case OBJECT_ENTITY: {
				CAdEntity *Ent = Node->_entity;
				if (Ent->_active && (Ent->_registrable || !InteractiveOnly))
					Objects.Add(Ent);
			}
			break;

			case OBJECT_REGION: {
				CBArray<CAdObject *, CAdObject *> RegionObj;
				GetRegionObjects(Node->_region, RegionObj, InteractiveOnly);
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
HRESULT CAdScene::GetRegionObjects(CAdRegion *Region, CBArray<CAdObject *, CAdObject *> &Objects, bool InteractiveOnly) {
	CAdGame *AdGame = (CAdGame *)Game;
	CAdObject *Obj;

	int i;

	// global objects
	for (i = 0; i < AdGame->_objects.GetSize(); i++) {
		Obj = AdGame->_objects[i];
		if (Obj->_active && (Obj->_stickRegion == Region || Region == NULL || (Obj->_stickRegion == NULL && Region->PointInRegion(Obj->_posX, Obj->_posY)))) {
			if (InteractiveOnly && !Obj->_registrable) continue;

			Objects.Add(Obj);
		}
	}

	// scene objects
	for (i = 0; i < _objects.GetSize(); i++) {
		Obj = _objects[i];
		if (Obj->_active && !Obj->_editorOnly && (Obj->_stickRegion == Region || Region == NULL || (Obj->_stickRegion == NULL && Region->PointInRegion(Obj->_posX, Obj->_posY)))) {
			if (InteractiveOnly && !Obj->_registrable) continue;

			Objects.Add(Obj);
		}
	}

	// sort by _posY
	qsort(Objects.GetData(), Objects.GetSize(), sizeof(CAdObject *), CAdScene::CompareObjs);

	return S_OK;
}

} // end of namespace WinterMute
