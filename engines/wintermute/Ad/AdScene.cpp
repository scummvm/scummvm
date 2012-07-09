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
	_pfTarget = new CBPoint;
	setDefaults();
}


//////////////////////////////////////////////////////////////////////////
CAdScene::~CAdScene() {
	cleanup();
	Game->unregisterObject(_fader);
	delete _pfTarget;
	_pfTarget = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::setDefaults() {
	_initialized = false;
	_pfReady = true;
	_pfTargetPath = NULL;
	_pfRequester = NULL;
	_mainLayer = NULL;

	_pfPointsNum = 0;
	_persistentState = false;
	_persistentStateSprites = true;

	_autoScroll = true;
	_offsetLeft = _offsetTop = 0;
	_targetOffsetLeft = _targetOffsetTop = 0;

	_lastTimeH = _lastTimeV = 0;
	_scrollTimeH = _scrollTimeV = 10;
	_scrollPixelsH = _scrollPixelsV = 1;

	_pfMaxTime = 15;

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
	Game->registerObject(_fader);

	_viewport = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::cleanup() {
	CBObject::cleanup();

	_mainLayer = NULL; // reference only

	int i;

	delete _shieldWindow;
	_shieldWindow = NULL;

	Game->unregisterObject(_fader);
	_fader = NULL;

	for (i = 0; i < _layers.GetSize(); i++)
		Game->unregisterObject(_layers[i]);
	_layers.RemoveAll();


	for (i = 0; i < _waypointGroups.GetSize(); i++)
		Game->unregisterObject(_waypointGroups[i]);
	_waypointGroups.RemoveAll();

	for (i = 0; i < _scaleLevels.GetSize(); i++)
		Game->unregisterObject(_scaleLevels[i]);
	_scaleLevels.RemoveAll();

	for (i = 0; i < _rotLevels.GetSize(); i++)
		Game->unregisterObject(_rotLevels[i]);
	_rotLevels.RemoveAll();


	for (i = 0; i < _pfPath.GetSize(); i++)
		delete _pfPath[i];
	_pfPath.RemoveAll();
	_pfPointsNum = 0;

	for (i = 0; i < _objects.GetSize(); i++)
		Game->unregisterObject(_objects[i]);
	_objects.RemoveAll();

	delete _viewport;
	_viewport = NULL;

	setDefaults();
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::getPath(CBPoint source, CBPoint target, CAdPath *path, CBObject *requester) {
	if (!_pfReady) return false;
	else {
		_pfReady = false;
		*_pfTarget = target;
		_pfTargetPath = path;
		_pfRequester = requester;

		_pfTargetPath->reset();
		_pfTargetPath->setReady(false);

		// prepare working path
		int i;
		pfPointsStart();

		// first point
		//_pfPath.Add(new CAdPathPoint(source.x, source.y, 0));

		// if we're one pixel stuck, get unstuck
		int startX = source.x;
		int startY = source.y;
		int bestDistance = 1000;
		if (isBlockedAt(startX, startY, true, requester)) {
			int tolerance = 2;
			for (int xxx = startX - tolerance; xxx <= startX + tolerance; xxx++) {
				for (int yyy = startY - tolerance; yyy <= startY + tolerance; yyy++) {
					if (isWalkableAt(xxx, yyy, true, requester)) {
						int distance = abs(xxx - source.x) + abs(yyy - source.y);
						if (distance < bestDistance) {
							startX = xxx;
							startY = yyy;

							bestDistance = distance;
						}
					}
				}
			}
		}

		pfPointsAdd(startX, startY, 0);

		//CorrectTargetPoint(&target.x, &target.y);

		// last point
		//_pfPath.Add(new CAdPathPoint(target.x, target.y, INT_MAX));
		pfPointsAdd(target.x, target.y, INT_MAX);

		// active waypoints
		for (i = 0; i < _waypointGroups.GetSize(); i++) {
			if (_waypointGroups[i]->_active) {
				pfAddWaypointGroup(_waypointGroups[i], requester);
			}
		}


		// free waypoints
		for (i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentWptGroup) {
				pfAddWaypointGroup(_objects[i]->_currentWptGroup, requester);
			}
		}
		CAdGame *adGame = (CAdGame *)Game;
		for (i = 0; i < adGame->_objects.GetSize(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentWptGroup) {
				pfAddWaypointGroup(adGame->_objects[i]->_currentWptGroup, requester);
			}
		}

		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::pfAddWaypointGroup(CAdWaypointGroup *wpt, CBObject *requester) {
	if (!wpt->_active) return;

	for (int i = 0; i < wpt->_points.GetSize(); i++) {
		if (isBlockedAt(wpt->_points[i]->x, wpt->_points[i]->y, true, requester)) continue;

		//_pfPath.Add(new CAdPathPoint(Wpt->_points[i]->x, Wpt->_points[i]->y, INT_MAX));
		pfPointsAdd(wpt->_points[i]->x, wpt->_points[i]->y, INT_MAX);
	}
}


//////////////////////////////////////////////////////////////////////////
float CAdScene::getZoomAt(int x, int y) {
	float ret = 100;

	bool found = false;
	if (_mainLayer) {
		for (int i = _mainLayer->_nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && !node->_region->_blocked && node->_region->pointInRegion(x, y)) {
				if (node->_region->_zoom != 0) {
					ret = node->_region->_zoom;
					found = true;
					break;
				}
			}
		}
	}
	if (!found) ret = getScaleAt(y);

	return ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 CAdScene::getAlphaAt(int x, int y, bool colorCheck) {
	if (!Game->_dEBUG_DebugMode) colorCheck = false;

	uint32 ret;
	if (colorCheck) ret = 0xFFFF0000;
	else ret = 0xFFFFFFFF;

	if (_mainLayer) {
		for (int i = _mainLayer->_nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && (colorCheck || !node->_region->_blocked) && node->_region->pointInRegion(x, y)) {
				if (!node->_region->_blocked) ret = node->_region->_alpha;
				break;
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::isBlockedAt(int x, int y, bool checkFreeObjects, CBObject *requester) {
	bool ret = true;


	if (checkFreeObjects) {
		for (int i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentBlockRegion) {
				if (_objects[i]->_currentBlockRegion->pointInRegion(x, y)) return true;
			}
		}
		CAdGame *adGame = (CAdGame *)Game;
		for (int i = 0; i < adGame->_objects.GetSize(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentBlockRegion) {
				if (adGame->_objects[i]->_currentBlockRegion->pointInRegion(x, y)) return true;
			}
		}
	}


	if (_mainLayer) {
		for (int i = 0; i < _mainLayer->_nodes.GetSize(); i++) {
			CAdSceneNode *node = _mainLayer->_nodes[i];
			/*
			if (Node->_type == OBJECT_REGION && Node->_region->_active && Node->_region->_blocked && Node->_region->PointInRegion(X, Y))
			{
			    ret = true;
			    break;
			}
			*/
			if (node->_type == OBJECT_REGION && node->_region->_active && !node->_region->_decoration && node->_region->pointInRegion(x, y)) {
				if (node->_region->_blocked) {
					ret = true;
					break;
				} else ret = false;
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::isWalkableAt(int x, int y, bool checkFreeObjects, CBObject *requester) {
	bool ret = false;

	if (checkFreeObjects) {
		for (int i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentBlockRegion) {
				if (_objects[i]->_currentBlockRegion->pointInRegion(x, y)) return false;
			}
		}
		CAdGame *adGame = (CAdGame *)Game;
		for (int i = 0; i < adGame->_objects.GetSize(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentBlockRegion) {
				if (adGame->_objects[i]->_currentBlockRegion->pointInRegion(x, y)) return false;
			}
		}
	}


	if (_mainLayer) {
		for (int i = 0; i < _mainLayer->_nodes.GetSize(); i++) {
			CAdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && !node->_region->_decoration && node->_region->pointInRegion(x, y)) {
				if (node->_region->_blocked) {
					ret = false;
					break;
				} else ret = true;
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
int CAdScene::getPointsDist(CBPoint p1, CBPoint p2, CBObject *requester) {
	double xStep, yStep, x, y;
	int xLength, yLength, xCount, yCount;
	int x1, y1, x2, y2;

	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;

	xLength = abs(x2 - x1);
	yLength = abs(y2 - y1);

	if (xLength > yLength) {
		if (x1 > x2) {
			CBUtils::swap(&x1, &x2);
			CBUtils::swap(&y1, &y2);
		}

		yStep = (double)(y2 - y1) / (double)(x2 - x1);
		y = y1;

		for (xCount = x1; xCount < x2; xCount++) {
			if (isBlockedAt(xCount, (int)y, true, requester)) return -1;
			y += yStep;
		}
	} else {
		if (y1 > y2) {
			CBUtils::swap(&x1, &x2);
			CBUtils::swap(&y1, &y2);
		}

		xStep = (double)(x2 - x1) / (double)(y2 - y1);
		x = x1;

		for (yCount = y1; yCount < y2; yCount++) {
			if (isBlockedAt((int)x, yCount, true, requester)) return -1;
			x += xStep;
		}
	}
	return MAX(xLength, yLength);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::pathFinderStep() {
	int i;
	// get lowest unmarked
	int lowestDist = INT_MAX;
	CAdPathPoint *lowestPt = NULL;

	for (i = 0; i < _pfPointsNum; i++)
		if (!_pfPath[i]->_marked && _pfPath[i]->_distance < lowestDist) {
			lowestDist = _pfPath[i]->_distance;
			lowestPt = _pfPath[i];
		}

	if (lowestPt == NULL) { // no path -> terminate PathFinder
		_pfReady = true;
		_pfTargetPath->setReady(true);
		return;
	}

	lowestPt->_marked = true;

	// target point marked, generate path and terminate
	if (lowestPt->x == _pfTarget->x && lowestPt->y == _pfTarget->y) {
		while (lowestPt != NULL) {
			_pfTargetPath->_points.InsertAt(0, new CBPoint(lowestPt->x, lowestPt->y));
			lowestPt = lowestPt->_origin;
		}

		_pfReady = true;
		_pfTargetPath->setReady(true);
		return;
	}

	// otherwise keep on searching
	for (i = 0; i < _pfPointsNum; i++)
		if (!_pfPath[i]->_marked) {
			int j = getPointsDist(*lowestPt, *_pfPath[i], _pfRequester);
			if (j != -1 && lowestPt->_distance + j < _pfPath[i]->_distance) {
				_pfPath[i]->_distance = lowestPt->_distance + j;
				_pfPath[i]->_origin = lowestPt;
			}
		}
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::initLoop() {
#ifdef _DEBUGxxxx
	int nu_steps = 0;
	uint32 start = Game->_currentTime;
	while (!_pfReady && CBPlatform::getTime() - start <= _pfMaxTime) {
		PathFinderStep();
		nu_steps++;
	}
	if (nu_steps > 0) Game->LOG(0, "STAT: PathFinder iterations in one loop: %d (%s)  _pfMaxTime=%d", nu_steps, _pfReady ? "finished" : "not yet done", _pfMaxTime);
#else
	uint32 start = Game->_currentTime;
	while (!_pfReady && CBPlatform::getTime() - start <= _pfMaxTime) pathFinderStep();
#endif

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CAdScene::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	ERRORCODE ret;

	delete[] _filename;
	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing SCENE file '%s'", filename);

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);


	delete [] buffer;

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
ERRORCODE CAdScene::loadBuffer(byte *buffer, bool complete) {
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

	cleanup();

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_SCENE) {
			Game->LOG(0, "'SCENE' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	int ar, ag, ab, aa;
	char camera[MAX_PATH_LENGTH] = "";
	/* float WaypointHeight = -1.0f; */

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_LAYER: {
			CAdLayer *layer = new CAdLayer(Game);
			if (!layer || DID_FAIL(layer->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete layer;
				layer = NULL;
			} else {
				Game->registerObject(layer);
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
			if (!wpt || DID_FAIL(wpt->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete wpt;
				wpt = NULL;
			} else {
				Game->registerObject(wpt);
				_waypointGroups.Add(wpt);
			}
		}
		break;

		case TOKEN_SCALE_LEVEL: {
			CAdScaleLevel *sl = new CAdScaleLevel(Game);
			if (!sl || DID_FAIL(sl->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete sl;
				sl = NULL;
			} else {
				Game->registerObject(sl);
				_scaleLevels.Add(sl);
			}
		}
		break;

		case TOKEN_ROTATION_LEVEL: {
			CAdRotLevel *rl = new CAdRotLevel(Game);
			if (!rl || DID_FAIL(rl->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete rl;
				rl = NULL;
			} else {
				Game->registerObject(rl);
				_rotLevels.Add(rl);
			}
		}
		break;

		case TOKEN_ENTITY: {
			CAdEntity *entity = new CAdEntity(Game);
			if (!entity || DID_FAIL(entity->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete entity;
				entity = NULL;
			} else {
				addObject(entity);
			}
		}
		break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || DID_FAIL(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CAMERA:
			strcpy(camera, (char *)params);
			break;

		case TOKEN_EDITOR_MARGIN_H:
			parser.scanStr((char *)params, "%d", &_editorMarginH);
			break;

		case TOKEN_EDITOR_MARGIN_V:
			parser.scanStr((char *)params, "%d", &_editorMarginV);
			break;

		case TOKEN_EDITOR_COLOR_FRAME:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColFrame = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColEntity = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColEntitySel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColRegionSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColDecorSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColBlockedSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColWaypointsSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColRegion = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColDecor = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColBlocked = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColWaypoints = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_SCALE:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColScale = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_SHOW_REGIONS:
			parser.scanStr((char *)params, "%b", &_editorShowRegions);
			break;

		case TOKEN_EDITOR_SHOW_BLOCKED:
			parser.scanStr((char *)params, "%b", &_editorShowBlocked);
			break;

		case TOKEN_EDITOR_SHOW_DECORATION:
			parser.scanStr((char *)params, "%b", &_editorShowDecor);
			break;

		case TOKEN_EDITOR_SHOW_ENTITIES:
			parser.scanStr((char *)params, "%b", &_editorShowEntities);
			break;

		case TOKEN_EDITOR_SHOW_SCALE:
			parser.scanStr((char *)params, "%b", &_editorShowScale);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_VIEWPORT: {
			Common::Rect rc;
			parser.scanStr((char *)params, "%d,%d,%d,%d", &rc.left, &rc.top, &rc.right, &rc.bottom);
			if (!_viewport) _viewport = new CBViewport(Game);
			if (_viewport) _viewport->setRect(rc.left, rc.top, rc.right, rc.bottom, true);
		}

		case TOKEN_PERSISTENT_STATE:
			parser.scanStr((char *)params, "%b", &_persistentState);
			break;

		case TOKEN_PERSISTENT_STATE_SPRITES:
			parser.scanStr((char *)params, "%b", &_persistentStateSprites);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in SCENE definition");
		return STATUS_FAILED;
	}

	if (_mainLayer == NULL) Game->LOG(0, "Warning: scene '%s' has no main layer.", _filename);


	sortScaleLevels();
	sortRotLevels();

	_initialized = true;


	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::traverseNodes(bool doUpdate) {
	if (!_initialized) return STATUS_OK;

	int j, k;
	CAdGame *adGame = (CAdGame *)Game;


	//////////////////////////////////////////////////////////////////////////
	// prepare viewport
	bool PopViewport = false;
	if (_viewport && !Game->_editorMode) {
		Game->pushViewport(_viewport);
		PopViewport = true;
	} else if (adGame->_sceneViewport && !Game->_editorMode) {
		Game->pushViewport(adGame->_sceneViewport);
		PopViewport = true;
	}


	//////////////////////////////////////////////////////////////////////////
	// *** adjust scroll offset
	if (doUpdate) {
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
	int viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	int viewportX, viewportY;
	getViewportOffset(&viewportX, &viewportY);

	int scrollableX = _width  - viewportWidth;
	int scrollableY = _height - viewportHeight;

	double widthRatio  = scrollableX <= 0 ? 0 : ((double)(_offsetLeft) / (double)scrollableX);
	double heightRatio = scrollableY <= 0 ? 0 : ((double)(_offsetTop)  / (double)scrollableY);

	int origX, origY;
	Game->getOffset(&origX, &origY);



	//////////////////////////////////////////////////////////////////////////
	// *** display/update everything
	Game->_renderer->setup2D();

	// for each layer
	/* int MainOffsetX = 0; */
	/* int MainOffsetY = 0; */

	for (j = 0; j < _layers.GetSize(); j++) {
		if (!_layers[j]->_active) continue;

		// make layer exclusive
		if (!doUpdate) {
			if (_layers[j]->_closeUp && !Game->_editorMode) {
				if (!_shieldWindow) _shieldWindow = new CUIWindow(Game);
				if (_shieldWindow) {
					_shieldWindow->_posX = _shieldWindow->_posY = 0;
					_shieldWindow->_width = Game->_renderer->_width;
					_shieldWindow->_height = Game->_renderer->_height;
					_shieldWindow->display();
				}
			}
		}

		if (_paralaxScrolling) {
			int offsetX = (int)(widthRatio  * (_layers[j]->_width  - viewportWidth) - viewportX);
			int offsetY = (int)(heightRatio * (_layers[j]->_height - viewportHeight) - viewportY);
			Game->setOffset(offsetX, offsetY);

			Game->_offsetPercentX = (float)offsetX / ((float)_layers[j]->_width - viewportWidth) * 100.0f;
			Game->_offsetPercentY = (float)offsetY / ((float)_layers[j]->_height - viewportHeight) * 100.0f;

			//Game->QuickMessageForm("%d %f", OffsetX+ViewportX, Game->_offsetPercentX);
		} else {
			Game->setOffset(_offsetLeft - viewportX, _offsetTop - viewportY);

			Game->_offsetPercentX = (float)(_offsetLeft - viewportX) / ((float)_layers[j]->_width - viewportWidth) * 100.0f;
			Game->_offsetPercentY = (float)(_offsetTop - viewportY) / ((float)_layers[j]->_height - viewportHeight) * 100.0f;
		}


		// for each node
		for (k = 0; k < _layers[j]->_nodes.GetSize(); k++) {
			CAdSceneNode *node = _layers[j]->_nodes[k];
			switch (node->_type) {
			case OBJECT_ENTITY:
				if (node->_entity->_active && (Game->_editorMode || !node->_entity->_editorOnly)) {
					Game->_renderer->setup2D();

					if (doUpdate) node->_entity->update();
					else node->_entity->display();
				}
				break;

			case OBJECT_REGION: {
				if (node->_region->_blocked) break;
				if (node->_region->_decoration) break;

				if (!doUpdate) displayRegionContent(node->_region);
			}
			break;
			default:
				error("AdScene::TraverseNodes - Unhandled enum");
				break;
			} // switch
		} // each node

		// display/update all objects which are off-regions
		if (_layers[j]->_main) {
			if (doUpdate) {
				updateFreeObjects();
			} else {
				displayRegionContent(NULL);
			}
		}
	} // each layer


	// restore state
	Game->setOffset(origX, origY);
	Game->_renderer->setup2D();

	// display/update fader
	if (_fader) {
		if (doUpdate) _fader->update();
		else _fader->display();
	}

	if (PopViewport) Game->popViewport();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::display() {
	return traverseNodes(false);
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::updateFreeObjects() {
	CAdGame *adGame = (CAdGame *)Game;
	bool is3DSet;

	// *** update all active objects
	is3DSet = false;
	for (int i = 0; i < adGame->_objects.GetSize(); i++) {
		if (!adGame->_objects[i]->_active) continue;

		adGame->_objects[i]->update();
		adGame->_objects[i]->_drawn = false;
	}


	for (int i = 0; i < _objects.GetSize(); i++) {
		if (!_objects[i]->_active) continue;

		_objects[i]->update();
		_objects[i]->_drawn = false;
	}


	if (_autoScroll && Game->_mainObject != NULL) {
		scrollToObject(Game->_mainObject);
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::displayRegionContent(CAdRegion *region, bool display3DOnly) {
	CAdGame *adGame = (CAdGame *)Game;
	CBArray<CAdObject *, CAdObject *> objects;
	CAdObject *obj;

	// global objects
	for (int i = 0; i < adGame->_objects.GetSize(); i++) {
		obj = adGame->_objects[i];
		if (obj->_active && !obj->_drawn && (obj->_stickRegion == region || region == NULL || (obj->_stickRegion == NULL && region->pointInRegion(obj->_posX, obj->_posY)))) {
			objects.Add(obj);
		}
	}

	// scene objects
	for (int i = 0; i < _objects.GetSize(); i++) {
		obj = _objects[i];
		if (obj->_active && !obj->_editorOnly && !obj->_drawn && (obj->_stickRegion == region || region == NULL || (obj->_stickRegion == NULL && region->pointInRegion(obj->_posX, obj->_posY)))) {
			objects.Add(obj);
		}
	}

	// sort by _posY
	qsort(objects.GetData(), objects.GetSize(), sizeof(CAdObject *), CAdScene::compareObjs);

	// display them
	for (int i = 0; i < objects.GetSize(); i++) {
		obj = objects[i];

		if (display3DOnly && !obj->_is3D) continue;

		Game->_renderer->setup2D();

		if (Game->_editorMode || !obj->_editorOnly) obj->display();
		obj->_drawn = true;
	}


	// display design only objects
	if (!display3DOnly) {
		if (Game->_editorMode && region == NULL) {
			for (int i = 0; i < _objects.GetSize(); i++) {
				if (_objects[i]->_active && _objects[i]->_editorOnly) {
					_objects[i]->display();
					_objects[i]->_drawn = true;
				}
			}
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
int CAdScene::compareObjs(const void *obj1, const void *obj2) {
	CAdObject *object1 = *(CAdObject **)obj1;
	CAdObject *object2 = *(CAdObject **)obj2;

	if (object1->_posY < object2->_posY) return -1;
	else if (object1->_posY > object2->_posY) return 1;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::displayRegionContentOld(CAdRegion *region) {
	CAdGame *adGame = (CAdGame *)Game;
	CAdObject *obj;

	// display all objects in region sorted by _posY
	do {
		obj = NULL;
		int minY = INT_MAX;

		// global objects
		for (int i = 0; i < adGame->_objects.GetSize(); i++) {
			if (adGame->_objects[i]->_active && !adGame->_objects[i]->_drawn && adGame->_objects[i]->_posY < minY && (adGame->_objects[i]->_stickRegion == region || region == NULL || (adGame->_objects[i]->_stickRegion == NULL && region->pointInRegion(adGame->_objects[i]->_posX, adGame->_objects[i]->_posY)))) {
				obj = adGame->_objects[i];
				minY = adGame->_objects[i]->_posY;
			}
		}

		// scene objects
		for (int i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && !_objects[i]->_editorOnly && !_objects[i]->_drawn && _objects[i]->_posY < minY && (_objects[i]->_stickRegion == region || region == NULL || (_objects[i]->_stickRegion == NULL && region->pointInRegion(_objects[i]->_posX, _objects[i]->_posY)))) {
				obj = _objects[i];
				minY = _objects[i]->_posY;
			}
		}


		if (obj != NULL) {
			Game->_renderer->setup2D();

			if (Game->_editorMode || !obj->_editorOnly) obj->display();
			obj->_drawn = true;
		}
	} while (obj != NULL);


	// design only objects
	if (Game->_editorMode && region == NULL) {
		for (int i = 0; i < _objects.GetSize(); i++) {
			if (_objects[i]->_active && _objects[i]->_editorOnly) {
				_objects[i]->display();
				_objects[i]->_drawn = true;
			}
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::update() {
	return traverseNodes(true);
}

//////////////////////////////////////////////////////////////////////////
void CAdScene::scrollTo(int offsetX, int offsetY) {
	int viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	int origOffsetLeft = _targetOffsetLeft;
	int origOffsetTop = _targetOffsetTop;

	_targetOffsetLeft = MAX(0, offsetX - viewportWidth / 2);
	_targetOffsetLeft = MIN(_targetOffsetLeft, _width - viewportWidth);

	_targetOffsetTop = MAX(0, offsetY - viewportHeight / 2);
	_targetOffsetTop = MIN(_targetOffsetTop, _height - viewportHeight);


	if (Game->_mainObject && Game->_mainObject->_is3D) {
		if (abs(origOffsetLeft - _targetOffsetLeft) < 5) _targetOffsetLeft = origOffsetLeft;
		if (abs(origOffsetTop - _targetOffsetTop) < 5) _targetOffsetTop = origOffsetTop;
		//_targetOffsetTop = 0;
	}

	_ready = false;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::scrollToObject(CBObject *object) {
	if (object) scrollTo(object->_posX, object->_posY - object->getHeight() / 2);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::skipToObject(CBObject *object) {
	if (object) skipTo(object->_posX, object->_posY - object->getHeight() / 2);
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::skipTo(int offsetX, int offsetY) {
	int viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	_offsetLeft = MAX(0, offsetX - viewportWidth / 2);
	_offsetLeft = MIN(_offsetLeft, _width - viewportWidth);

	_offsetTop = MAX(0, offsetY - viewportHeight / 2);
	_offsetTop = MIN(_offsetTop, _height - viewportHeight);

	_targetOffsetLeft = _offsetLeft;
	_targetOffsetTop = _offsetTop;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// LoadActor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "LoadActor") == 0) {
		stack->correctParams(1);
		CAdActor *act = new CAdActor(Game);
		if (act && DID_SUCCEED(act->loadFile(stack->pop()->getString()))) {
			addObject(act);
			stack->pushNative(act, true);
		} else {
			delete act;
			act = NULL;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadEntity") == 0) {
		stack->correctParams(1);
		CAdEntity *ent = new CAdEntity(Game);
		if (ent && DID_SUCCEED(ent->loadFile(stack->pop()->getString()))) {
			addObject(ent);
			stack->pushNative(ent, true);
		} else {
			delete ent;
			ent = NULL;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateEntity") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CAdEntity *ent = new CAdEntity(Game);
		addObject(ent);
		if (!val->isNULL()) ent->setName(val->getString());
		stack->pushNative(ent, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadObject / UnloadActor / UnloadEntity / UnloadActor3D / DeleteEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadObject") == 0 || strcmp(name, "UnloadActor") == 0 || strcmp(name, "UnloadEntity") == 0 || strcmp(name, "UnloadActor3D") == 0 || strcmp(name, "DeleteEntity") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		CAdObject *obj = (CAdObject *)val->getNative();
		removeObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) val->setNULL();

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SkipTo") == 0) {
		stack->correctParams(2);
		CScValue *val1 = stack->pop();
		CScValue *val2 = stack->pop();
		if (val1->isNative()) {
			skipToObject((CBObject *)val1->getNative());
		} else {
			skipTo(val1->getInt(), val2->getInt());
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollTo / ScrollToAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollTo") == 0 || strcmp(name, "ScrollToAsync") == 0) {
		stack->correctParams(2);
		CScValue *val1 = stack->pop();
		CScValue *val2 = stack->pop();
		if (val1->isNative()) {
			scrollToObject((CBObject *)val1->getNative());
		} else {
			scrollTo(val1->getInt(), val2->getInt());
		}
		if (strcmp(name, "ScrollTo") == 0) script->waitForExclusive(this);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLayer") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		if (val->isInt()) {
			int layer = val->getInt();
			if (layer < 0 || layer >= _layers.GetSize()) stack->pushNULL();
			else stack->pushNative(_layers[layer], true);
		} else {
			const char *LayerName = val->getString();
			bool LayerFound = false;
			for (int i = 0; i < _layers.GetSize(); i++) {
				if (scumm_stricmp(LayerName, _layers[i]->_name) == 0) {
					stack->pushNative(_layers[i], true);
					LayerFound = true;
					break;
				}
			}
			if (!LayerFound) stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaypointGroup
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWaypointGroup") == 0) {
		stack->correctParams(1);
		int group = stack->pop()->getInt();
		if (group < 0 || group >= _waypointGroups.GetSize()) stack->pushNULL();
		else stack->pushNative(_waypointGroups[group], true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNode") == 0) {
		stack->correctParams(1);
		const char *nodeName = stack->pop()->getString();

		CBObject *node = getNodeByName(nodeName);
		if (node) stack->pushNative((CBScriptable *)node, true);
		else stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFreeNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFreeNode") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CAdObject *ret = NULL;
		if (val->isInt()) {
			int index = val->getInt();
			if (index >= 0 && index < _objects.GetSize()) ret = _objects[index];
		} else {
			const char *nodeName = val->getString();
			for (int i = 0; i < _objects.GetSize(); i++) {
				if (_objects[i] && _objects[i]->_name && scumm_stricmp(_objects[i]->_name, nodeName) == 0) {
					ret = _objects[i];
					break;
				}
			}
		}
		if (ret) stack->pushNative(ret, true);
		else stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRegionAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetRegionAt") == 0) {
		stack->correctParams(3);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		CScValue *val = stack->pop();

		bool includeDecors = false;
		if (!val->isNULL()) includeDecors = val->getBool();

		if (_mainLayer) {
			for (int i = _mainLayer->_nodes.GetSize() - 1; i >= 0; i--) {
				CAdSceneNode *node = _mainLayer->_nodes[i];
				if (node->_type == OBJECT_REGION && node->_region->_active && node->_region->pointInRegion(x, y)) {
					if (node->_region->_decoration && !includeDecors) continue;

					stack->pushNative(node->_region, true);
					return STATUS_OK;
				}
			}
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsBlockedAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsBlockedAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		stack->pushBool(isBlockedAt(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsWalkableAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsWalkableAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		stack->pushBool(isWalkableAt(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetScaleAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetScaleAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		stack->pushFloat(getZoomAt(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRotationAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetRotationAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		stack->pushFloat(getRotationAt(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsScrolling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsScrolling") == 0) {
		stack->correctParams(0);
		bool ret = false;
		if (_autoScroll) {
			if (_targetOffsetLeft != _offsetLeft || _targetOffsetTop != _offsetTop) ret = true;
		}

		stack->pushBool(ret);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeOut / FadeOutAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeOut") == 0 || strcmp(name, "FadeOutAsync") == 0) {
		stack->correctParams(5);
		uint32 duration = stack->pop()->getInt(500);
		byte red = stack->pop()->getInt(0);
		byte green = stack->pop()->getInt(0);
		byte blue = stack->pop()->getInt(0);
		byte alpha = stack->pop()->getInt(0xFF);

		_fader->fadeOut(BYTETORGBA(red, green, blue, alpha), duration);
		if (strcmp(name, "FadeOutAsync") != 0) script->waitFor(_fader);

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeIn / FadeInAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeIn") == 0 || strcmp(name, "FadeInAsync") == 0) {
		stack->correctParams(5);
		uint32 duration = stack->pop()->getInt(500);
		byte red = stack->pop()->getInt(0);
		byte green = stack->pop()->getInt(0);
		byte blue = stack->pop()->getInt(0);
		byte alpha = stack->pop()->getInt(0xFF);

		_fader->fadeIn(BYTETORGBA(red, green, blue, alpha), duration);
		if (strcmp(name, "FadeInAsync") != 0) script->waitFor(_fader);

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFadeColor") == 0) {
		stack->correctParams(0);
		stack->pushInt(_fader->getCurrentColor());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsPointInViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsPointInViewport") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		stack->pushBool(pointInViewport(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetViewport") == 0) {
		stack->correctParams(4);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		int width = stack->pop()->getInt();
		int height = stack->pop()->getInt();

		if (width <= 0) width = Game->_renderer->_width;
		if (height <= 0) height = Game->_renderer->_height;

		if (!_viewport) _viewport = new CBViewport(Game);
		if (_viewport) _viewport->setRect(x, y, x + width, y + height);

		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddLayer") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CAdLayer *layer = new CAdLayer(Game);
		if (!val->isNULL()) layer->setName(val->getString());
		if (_mainLayer) {
			layer->_width = _mainLayer->_width;
			layer->_height = _mainLayer->_height;
		}
		_layers.Add(layer);
		Game->registerObject(layer);

		stack->pushNative(layer, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertLayer") == 0) {
		stack->correctParams(2);
		int index = stack->pop()->getInt();
		CScValue *val = stack->pop();

		CAdLayer *layer = new CAdLayer(Game);
		if (!val->isNULL()) layer->setName(val->getString());
		if (_mainLayer) {
			layer->_width = _mainLayer->_width;
			layer->_height = _mainLayer->_height;
		}
		if (index < 0) index = 0;
		if (index <= _layers.GetSize() - 1) _layers.InsertAt(index, layer);
		else _layers.Add(layer);

		Game->registerObject(layer);

		stack->pushNative(layer, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteLayer") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CAdLayer *toDelete = NULL;
		if (val->isNative()) {
			CBScriptable *temp = val->getNative();
			for (int i = 0; i < _layers.GetSize(); i++) {
				if (_layers[i] == temp) {
					toDelete = _layers[i];
					break;
				}
			}
		} else {
			int index = val->getInt();
			if (index >= 0 && index < _layers.GetSize()) {
				toDelete = _layers[index];
			}
		}
		if (toDelete == NULL) {
			stack->pushBool(false);
			return STATUS_OK;
		}

		if (toDelete->_main) {
			script->runtimeError("Scene.DeleteLayer - cannot delete main scene layer");
			stack->pushBool(false);
			return STATUS_OK;
		}

		for (int i = 0; i < _layers.GetSize(); i++) {
			if (_layers[i] == toDelete) {
				_layers.RemoveAt(i);
				Game->unregisterObject(toDelete);
				break;
			}
		}
		stack->pushBool(true);
		return STATUS_OK;
	}

	else return CBObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdScene::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("scene");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumLayers (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumLayers") == 0) {
		_scValue->setInt(_layers.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumWaypointGroups (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumWaypointGroups") == 0) {
		_scValue->setInt(_waypointGroups.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainLayer (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MainLayer") == 0) {
		if (_mainLayer) _scValue->setNative(_mainLayer, true);
		else _scValue->setNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFreeNodes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumFreeNodes") == 0) {
		_scValue->setInt(_objects.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseX") == 0) {
		int viewportX;
		getViewportOffset(&viewportX);

		_scValue->setInt(Game->_mousePos.x + _offsetLeft - viewportX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseY") == 0) {
		int viewportY;
		getViewportOffset(NULL, &viewportY);

		_scValue->setInt(Game->_mousePos.y + _offsetTop - viewportY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoScroll
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoScroll") == 0) {
		_scValue->setBool(_autoScroll);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PersistentState") == 0) {
		_scValue->setBool(_persistentState);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentStateSprites
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PersistentStateSprites") == 0) {
		_scValue->setBool(_persistentStateSprites);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollPixelsX") == 0) {
		_scValue->setInt(_scrollPixelsH);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollPixelsY") == 0) {
		_scValue->setInt(_scrollPixelsV);
		return _scValue;
	}


	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollSpeedX") == 0) {
		_scValue->setInt(_scrollTimeH);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollSpeedY") == 0) {
		_scValue->setInt(_scrollTimeV);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OffsetX") == 0) {
		_scValue->setInt(_offsetLeft);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OffsetY") == 0) {
		_scValue->setInt(_offsetTop);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		if (_mainLayer) _scValue->setInt(_mainLayer->_width);
		else _scValue->setInt(0);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		if (_mainLayer) _scValue->setInt(_mainLayer->_height);
		else _scValue->setInt(0);
		return _scValue;
	}

	else return CBObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoScroll
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoScroll") == 0) {
		_autoScroll = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PersistentState") == 0) {
		_persistentState = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentStateSprites
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PersistentStateSprites") == 0) {
		_persistentStateSprites = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollPixelsX") == 0) {
		_scrollPixelsH = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollPixelsY") == 0) {
		_scrollPixelsV = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollSpeedX") == 0) {
		_scrollTimeH = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollSpeedY") == 0) {
		_scrollTimeV = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OffsetX") == 0) {
		_offsetLeft = value->getInt();

		int viewportWidth, viewportHeight;
		getViewportSize(&viewportWidth, &viewportHeight);

		_offsetLeft = MAX(0, _offsetLeft - viewportWidth / 2);
		_offsetLeft = MIN(_offsetLeft, _width - viewportWidth);
		_targetOffsetLeft = _offsetLeft;

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OffsetY") == 0) {
		_offsetTop = value->getInt();

		int viewportWidth, viewportHeight;
		getViewportSize(&viewportWidth, &viewportHeight);

		_offsetTop = MAX(0, _offsetTop - viewportHeight / 2);
		_offsetTop = MIN(_offsetTop, _height - viewportHeight);
		_targetOffsetTop = _offsetTop;

		return STATUS_OK;
	}

	else return CBObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdScene::scToString() {
	return "[scene object]";
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::addObject(CAdObject *object) {
	_objects.Add(object);
	return Game->registerObject(object);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::removeObject(CAdObject *object) {
	for (int i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i] == object) {
			_objects.RemoveAt(i);
			return Game->unregisterObject(object);
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::saveAsText(CBDynBuffer *buffer, int indent) {
	int i;

	buffer->putTextIndent(indent, "SCENE {\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	if (_persistentState)
		buffer->putTextIndent(indent + 2, "PERSISTENT_STATE=%s\n", _persistentState ? "TRUE" : "FALSE");

	if (!_persistentStateSprites)
		buffer->putTextIndent(indent + 2, "PERSISTENT_STATE_SPRITES=%s\n", _persistentStateSprites ? "TRUE" : "FALSE");


	// scripts
	for (i = 0; i < _scripts.GetSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// properties
	if (_scProp) _scProp->saveAsText(buffer, indent + 2);

	// viewport
	if (_viewport) {
		Common::Rect *rc = _viewport->getRect();
		buffer->putTextIndent(indent + 2, "VIEWPORT { %d, %d, %d, %d }\n", rc->left, rc->top, rc->right, rc->bottom);
	}



	// editor settings
	buffer->putTextIndent(indent + 2, "; ----- editor settings\n");
	buffer->putTextIndent(indent + 2, "EDITOR_MARGIN_H=%d\n", _editorMarginH);
	buffer->putTextIndent(indent + 2, "EDITOR_MARGIN_V=%d\n", _editorMarginV);
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_FRAME { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColFrame), RGBCOLGetG(_editorColFrame), RGBCOLGetB(_editorColFrame), RGBCOLGetA(_editorColFrame));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_ENTITY_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColEntitySel), RGBCOLGetG(_editorColEntitySel), RGBCOLGetB(_editorColEntitySel), RGBCOLGetA(_editorColEntitySel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_REGION_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColRegionSel), RGBCOLGetG(_editorColRegionSel), RGBCOLGetB(_editorColRegionSel), RGBCOLGetA(_editorColRegionSel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_BLOCKED_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColBlockedSel), RGBCOLGetG(_editorColBlockedSel), RGBCOLGetB(_editorColBlockedSel), RGBCOLGetA(_editorColBlockedSel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_DECORATION_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColDecorSel), RGBCOLGetG(_editorColDecorSel), RGBCOLGetB(_editorColDecorSel), RGBCOLGetA(_editorColDecorSel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_WAYPOINTS_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColWaypointsSel), RGBCOLGetG(_editorColWaypointsSel), RGBCOLGetB(_editorColWaypointsSel), RGBCOLGetA(_editorColWaypointsSel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_ENTITY { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColEntity), RGBCOLGetG(_editorColEntity), RGBCOLGetB(_editorColEntity), RGBCOLGetA(_editorColEntity));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_REGION { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColRegion), RGBCOLGetG(_editorColRegion), RGBCOLGetB(_editorColRegion), RGBCOLGetA(_editorColRegion));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_DECORATION { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColDecor), RGBCOLGetG(_editorColDecor), RGBCOLGetB(_editorColDecor), RGBCOLGetA(_editorColDecor));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_BLOCKED { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColBlocked), RGBCOLGetG(_editorColBlocked), RGBCOLGetB(_editorColBlocked), RGBCOLGetA(_editorColBlocked));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_WAYPOINTS { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColWaypoints), RGBCOLGetG(_editorColWaypoints), RGBCOLGetB(_editorColWaypoints), RGBCOLGetA(_editorColWaypoints));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_SCALE { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColScale), RGBCOLGetG(_editorColScale), RGBCOLGetB(_editorColScale), RGBCOLGetA(_editorColScale));

	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_REGIONS=%s\n", _editorShowRegions ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_BLOCKED=%s\n", _editorShowBlocked ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_DECORATION=%s\n", _editorShowDecor ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_ENTITIES=%s\n", _editorShowEntities ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_SCALE=%s\n", _editorShowScale ? "TRUE" : "FALSE");

	buffer->putTextIndent(indent + 2, "\n");

	CBBase::saveAsText(buffer, indent + 2);

	// waypoints
	buffer->putTextIndent(indent + 2, "; ----- waypoints\n");
	for (i = 0; i < _waypointGroups.GetSize(); i++) _waypointGroups[i]->saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent + 2, "\n");

	// layers
	buffer->putTextIndent(indent + 2, "; ----- layers\n");
	for (i = 0; i < _layers.GetSize(); i++) _layers[i]->saveAsText(buffer, indent + 2);

	// scale levels
	buffer->putTextIndent(indent + 2, "; ----- scale levels\n");
	for (i = 0; i < _scaleLevels.GetSize(); i++) _scaleLevels[i]->saveAsText(buffer, indent + 2);

	// rotation levels
	buffer->putTextIndent(indent + 2, "; ----- rotation levels\n");
	for (i = 0; i < _rotLevels.GetSize(); i++) _rotLevels[i]->saveAsText(buffer, indent + 2);


	buffer->putTextIndent(indent + 2, "\n");

	// free entities
	buffer->putTextIndent(indent + 2, "; ----- free entities\n");
	for (i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY) {
			_objects[i]->saveAsText(buffer, indent + 2);

		}
	}



	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::sortScaleLevels() {
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

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::sortRotLevels() {
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

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float CAdScene::getScaleAt(int Y) {
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
ERRORCODE CAdScene::persist(CBPersistMgr *persistMgr) {
	CBObject::persist(persistMgr);

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
	_layers.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_mainLayer));
	_objects.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_offsetLeft));
	persistMgr->transfer(TMEMBER(_offsetTop));
	persistMgr->transfer(TMEMBER(_paralaxScrolling));
	persistMgr->transfer(TMEMBER(_persistentState));
	persistMgr->transfer(TMEMBER(_persistentStateSprites));
	persistMgr->transfer(TMEMBER(_pfMaxTime));
	_pfPath.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_pfPointsNum));
	persistMgr->transfer(TMEMBER(_pfReady));
	persistMgr->transfer(TMEMBER(_pfRequester));
	persistMgr->transfer(TMEMBER(_pfTarget));
	persistMgr->transfer(TMEMBER(_pfTargetPath));
	_rotLevels.persist(persistMgr);
	_scaleLevels.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_scrollPixelsH));
	persistMgr->transfer(TMEMBER(_scrollPixelsV));
	persistMgr->transfer(TMEMBER(_scrollTimeH));
	persistMgr->transfer(TMEMBER(_scrollTimeV));
	persistMgr->transfer(TMEMBER(_shieldWindow));
	persistMgr->transfer(TMEMBER(_targetOffsetLeft));
	persistMgr->transfer(TMEMBER(_targetOffsetTop));
	_waypointGroups.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_viewport));
	persistMgr->transfer(TMEMBER(_width));

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::afterLoad() {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::correctTargetPoint2(int startX, int startY, int *targetX, int *targetY, bool checkFreeObjects, CBObject *requester) {
	double xStep, yStep, x, y;
	int xLength, yLength, xCount, yCount;
	int x1, y1, x2, y2;

	x1 = *targetX;
	y1 = *targetY;
	x2 = startX;
	y2 = startY;


	xLength = abs(x2 - x1);
	yLength = abs(y2 - y1);

	if (xLength > yLength) {
		/*
		if (X1 > X2)
		{
		    Swap(&X1, &X2);
		    Swap(&Y1, &Y2);
		}
		*/

		yStep = fabs((double)(y2 - y1) / (double)(x2 - x1));
		y = y1;

		for (xCount = x1; xCount < x2; xCount++) {
			if (isWalkableAt(xCount, (int)y, checkFreeObjects, requester)) {
				*targetX = xCount;
				*targetY = (int)y;
				return STATUS_OK;
			}
			y += yStep;
		}
	} else {
		/*
		if (Y1 > Y2) {
		    Swap(&X1, &X2);
		    Swap(&Y1, &Y2);
		}
		*/

		xStep = fabs((double)(x2 - x1) / (double)(y2 - y1));
		x = x1;

		for (yCount = y1; yCount < y2; yCount++) {
			if (isWalkableAt((int)x, yCount, checkFreeObjects, requester)) {
				*targetX = (int)x;
				*targetY = yCount;
				return STATUS_OK;
			}
			x += xStep;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::correctTargetPoint(int startX, int startY, int *argX, int *argY, bool checkFreeObjects, CBObject *requester) {
	int x = *argX;
	int y = *argY;

	if (isWalkableAt(x, y, checkFreeObjects, requester) || !_mainLayer) {
		return STATUS_OK;
	}

	// right
	int length_right = 0;
	bool found_right = false;
	for (x = *argX, y = *argY; x < _mainLayer->_width; x++, length_right++) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x - 5, y, checkFreeObjects, requester)) {
			found_right = true;
			break;
		}
	}

	// left
	int length_left = 0;
	bool found_left = false;
	for (x = *argX, y = *argY; x >= 0; x--, length_left--) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x + 5, y, checkFreeObjects, requester)) {
			found_left = true;
			break;
		}
	}

	// up
	int length_up = 0;
	bool found_up = false;
	for (x = *argX, y = *argY; y >= 0; y--, length_up--) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x, y + 5, checkFreeObjects, requester)) {
			found_up = true;
			break;
		}
	}

	// down
	int length_down = 0;
	bool found_down = false;
	for (x = *argX, y = *argY; y < _mainLayer->_height; y++, length_down++) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x, y - 5, checkFreeObjects, requester)) {
			found_down = true;
			break;
		}
	}

	if (!found_left && !found_right && !found_up && !found_down) {
		return STATUS_OK;
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
		*argX = *argX + OffsetX;
	else
		*argY = *argY + OffsetY;

	if (!isWalkableAt(*argX, *argY)) return correctTargetPoint2(startX, startY, argX, argY, checkFreeObjects, requester);
	else return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::pfPointsStart() {
	_pfPointsNum = 0;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::pfPointsAdd(int x, int y, int distance) {
	if (_pfPointsNum >= _pfPath.GetSize()) {
		_pfPath.Add(new CAdPathPoint(x, y, distance));
	} else {
		_pfPath[_pfPointsNum]->x = x;
		_pfPath[_pfPointsNum]->y = y;
		_pfPath[_pfPointsNum]->_distance = distance;
		_pfPath[_pfPointsNum]->_marked = false;
		_pfPath[_pfPointsNum]->_origin = NULL;
	}

	_pfPointsNum++;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::getViewportOffset(int *offsetX, int *offsetY) {
	CAdGame *adGame = (CAdGame *)Game;
	if (_viewport && !Game->_editorMode) {
		if (offsetX) *offsetX = _viewport->_offsetX;
		if (offsetY) *offsetY = _viewport->_offsetY;
	} else if (adGame->_sceneViewport && !Game->_editorMode) {
		if (offsetX) *offsetX = adGame->_sceneViewport->_offsetX;
		if (offsetY) *offsetY = adGame->_sceneViewport->_offsetY;
	} else {
		if (offsetX) *offsetX = 0;
		if (offsetY) *offsetY = 0;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::getViewportSize(int *width, int *height) {
	CAdGame *adGame = (CAdGame *)Game;
	if (_viewport && !Game->_editorMode) {
		if (width)  *width  = _viewport->getWidth();
		if (height) *height = _viewport->getHeight();
	} else if (adGame->_sceneViewport && !Game->_editorMode) {
		if (width)  *width  = adGame->_sceneViewport->getWidth();
		if (height) *height = adGame->_sceneViewport->getHeight();
	} else {
		if (width)  *width  = Game->_renderer->_width;
		if (height) *height = Game->_renderer->_height;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int CAdScene::getOffsetLeft() {
	int viewportX;
	getViewportOffset(&viewportX);

	return _offsetLeft - viewportX;
}


//////////////////////////////////////////////////////////////////////////
int CAdScene::getOffsetTop() {
	int viewportY;
	getViewportOffset(NULL, &viewportY);

	return _offsetTop - viewportY;
}


//////////////////////////////////////////////////////////////////////////
bool CAdScene::pointInViewport(int x, int y) {
	int left, top, width, height;

	getViewportOffset(&left, &top);
	getViewportSize(&width, &height);

	return x >= left && x <= left + width && y >= top && y <= top + height;
}


//////////////////////////////////////////////////////////////////////////
void CAdScene::setOffset(int offsetLeft, int offsetTop) {
	_offsetLeft = offsetLeft;
	_offsetTop  = offsetTop;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdScene::getNodeByName(const char *name) {
	CBObject *ret = NULL;

	// dependent objects
	for (int i = 0; i < _layers.GetSize(); i++) {
		CAdLayer *layer = _layers[i];
		for (int j = 0; j < layer->_nodes.GetSize(); j++) {
			CAdSceneNode *node = layer->_nodes[j];
			if ((node->_type == OBJECT_ENTITY && !scumm_stricmp(name, node->_entity->_name)) ||
			        (node->_type == OBJECT_REGION && !scumm_stricmp(name, node->_region->_name))) {
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
	for (int i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY && !scumm_stricmp(name, _objects[i]->_name)) {
			return _objects[i];
		}
	}

	// waypoint groups
	for (int i = 0; i < _waypointGroups.GetSize(); i++) {
		if (!scumm_stricmp(name, _waypointGroups[i]->_name)) {
			return _waypointGroups[i];
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::saveState() {
	return persistState(true);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::loadState() {
	return persistState(false);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::persistState(bool saving) {
	if (!_persistentState) return STATUS_OK;

	CAdGame *adGame = (CAdGame *)Game;
	CAdSceneState *state = adGame->getSceneState(_filename, saving);
	if (!state) return STATUS_OK;

	CAdNodeState *nodeState;

	// dependent objects
	for (int i = 0; i < _layers.GetSize(); i++) {
		CAdLayer *layer = _layers[i];
		for (int j = 0; j < layer->_nodes.GetSize(); j++) {
			CAdSceneNode *node = layer->_nodes[j];
			switch (node->_type) {
			case OBJECT_ENTITY:
				if (!node->_entity->_saveState) continue;
				nodeState = state->getNodeState(node->_entity->_name, saving);
				if (nodeState) {
					nodeState->transferEntity(node->_entity, _persistentStateSprites, saving);
					//if(Saving) NodeState->_active = node->_entity->_active;
					//else node->_entity->_active = NodeState->_active;
				}
				break;
			case OBJECT_REGION:
				if (!node->_region->_saveState) continue;
				nodeState = state->getNodeState(node->_region->_name, saving);
				if (nodeState) {
					if (saving) nodeState->_active = node->_region->_active;
					else node->_region->_active = nodeState->_active;
				}
				break;
			default:
				warning("CAdScene::PersistState - unhandled enum");
				break;
			}
		}
	}

	// free entities
	for (int i = 0; i < _objects.GetSize(); i++) {
		if (!_objects[i]->_saveState) continue;
		if (_objects[i]->_type == OBJECT_ENTITY) {
			nodeState = state->getNodeState(_objects[i]->_name, saving);
			if (nodeState) {
				nodeState->transferEntity((CAdEntity *)_objects[i], _persistentStateSprites, saving);
				//if(Saving) NodeState->_active = _objects[i]->_active;
				//else _objects[i]->_active = NodeState->_active;
			}
		}
	}

	// waypoint groups
	for (int i = 0; i < _waypointGroups.GetSize(); i++) {
		nodeState = state->getNodeState(_waypointGroups[i]->_name, saving);
		if (nodeState) {
			if (saving) nodeState->_active = _waypointGroups[i]->_active;
			else _waypointGroups[i]->_active = nodeState->_active;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float CAdScene::getRotationAt(int x, int y) {
	CAdRotLevel *prev = NULL;
	CAdRotLevel *next = NULL;

	for (int i = 0; i < _rotLevels.GetSize(); i++) {
		/*  CAdRotLevel *xxx = _rotLevels[i];
		    int j = _rotLevels.GetSize();*/
		if (_rotLevels[i]->_posX < x) prev = _rotLevels[i];
		else {
			next = _rotLevels[i];
			break;
		}
	}

	if (prev == NULL || next == NULL) return 0;

	int delta_x = next->_posX - prev->_posX;
	float delta_rot = next->_rotation - prev->_rotation;
	x -= prev->_posX;

	float percent = (float)x / ((float)delta_x / 100.0f);
	return prev->_rotation + delta_rot / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::handleItemAssociations(const char *itemName, bool show) {
	for (int i = 0; i < _layers.GetSize(); i++) {
		CAdLayer *layer = _layers[i];
		for (int j = 0; j < layer->_nodes.GetSize(); j++) {
			if (layer->_nodes[j]->_type == OBJECT_ENTITY) {
				CAdEntity *ent = layer->_nodes[j]->_entity;

				if (ent->_item && strcmp(ent->_item, itemName) == 0) ent->_active = show;
			}
		}
	}

	for (int i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY) {
			CAdEntity *ent = (CAdEntity *)_objects[i];
			if (ent->_item && strcmp(ent->_item, itemName) == 0) ent->_active = show;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::getRegionsAt(int x, int y, CAdRegion **regionList, int numRegions) {
	int numUsed = 0;
	if (_mainLayer) {
		for (int i = _mainLayer->_nodes.GetSize() - 1; i >= 0; i--) {
			CAdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && node->_region->pointInRegion(x, y)) {
				if (numUsed < numRegions - 1) {
					regionList[numUsed] = node->_region;
					numUsed++;
				} else break;
			}
		}
	}
	for (int i = numUsed; i < numRegions; i++) {
		regionList[i] = NULL;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::restoreDeviceObjects() {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdScene::getNextAccessObject(CBObject *currObject) {
	CBArray<CAdObject *, CAdObject *> objects;
	getSceneObjects(objects, true);

	if (objects.GetSize() == 0) return NULL;
	else {
		if (currObject != NULL) {
			for (int i = 0; i < objects.GetSize(); i++) {
				if (objects[i] == currObject) {
					if (i < objects.GetSize() - 1) return objects[i + 1];
					else break;
				}
			}
		}
		return objects[0];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CBObject *CAdScene::getPrevAccessObject(CBObject *currObject) {
	CBArray<CAdObject *, CAdObject *> objects;
	getSceneObjects(objects, true);

	if (objects.GetSize() == 0) return NULL;
	else {
		if (currObject != NULL) {
			for (int i = objects.GetSize() - 1; i >= 0; i--) {
				if (objects[i] == currObject) {
					if (i > 0) return objects[i - 1];
					else break;
				}
			}
		}
		return objects[objects.GetSize() - 1];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::getSceneObjects(CBArray<CAdObject *, CAdObject *> &objects, bool interactiveOnly) {
	for (int i = 0; i < _layers.GetSize(); i++) {
		// close-up layer -> remove everything below it
		if (interactiveOnly && _layers[i]->_closeUp) objects.RemoveAll();


		for (int j = 0; j < _layers[i]->_nodes.GetSize(); j++) {
			CAdSceneNode *node = _layers[i]->_nodes[j];
			switch (node->_type) {
			case OBJECT_ENTITY: {
				CAdEntity *ent = node->_entity;
				if (ent->_active && (ent->_registrable || !interactiveOnly))
					objects.Add(ent);
			}
			break;

			case OBJECT_REGION: {
				CBArray<CAdObject *, CAdObject *> regionObj;
				getRegionObjects(node->_region, regionObj, interactiveOnly);
				for (int newIndex = 0; newIndex < regionObj.GetSize(); newIndex++) {
					bool found = false;
					for (int old = 0; old < objects.GetSize(); old++) {
						if (objects[old] == regionObj[newIndex]) {
							found = true;
							break;
						}
					}
					if (!found) objects.Add(regionObj[newIndex]);
				}
				//if(RegionObj.GetSize() > 0) Objects.Append(RegionObj);
			}
			break;
			default:
				warning("CAdScene::GetSceneObjects - Unhandled enum");
				break;
			}
		}
	}

	// objects outside any region
	CBArray<CAdObject *, CAdObject *> regionObj;
	getRegionObjects(NULL, regionObj, interactiveOnly);
	for (int newIndex = 0; newIndex < regionObj.GetSize(); newIndex++) {
		bool found = false;
		for (int old = 0; old < objects.GetSize(); old++) {
			if (objects[old] == regionObj[newIndex]) {
				found = true;
				break;
			}
		}
		if (!found) objects.Add(regionObj[newIndex]);
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdScene::getRegionObjects(CAdRegion *region, CBArray<CAdObject *, CAdObject *> &objects, bool interactiveOnly) {
	CAdGame *adGame = (CAdGame *)Game;
	CAdObject *obj;

	// global objects
	for (int i = 0; i < adGame->_objects.GetSize(); i++) {
		obj = adGame->_objects[i];
		if (obj->_active && (obj->_stickRegion == region || region == NULL || (obj->_stickRegion == NULL && region->pointInRegion(obj->_posX, obj->_posY)))) {
			if (interactiveOnly && !obj->_registrable) continue;

			objects.Add(obj);
		}
	}

	// scene objects
	for (int i = 0; i < _objects.GetSize(); i++) {
		obj = _objects[i];
		if (obj->_active && !obj->_editorOnly && (obj->_stickRegion == region || region == NULL || (obj->_stickRegion == NULL && region->pointInRegion(obj->_posX, obj->_posY)))) {
			if (interactiveOnly && !obj->_registrable) continue;

			objects.Add(obj);
		}
	}

	// sort by _posY
	qsort(objects.GetData(), objects.GetSize(), sizeof(CAdObject *), CAdScene::compareObjs);

	return STATUS_OK;
}

} // end of namespace WinterMute
