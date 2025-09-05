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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_actor.h"
#include "engines/wintermute/ad/ad_entity.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_layer.h"
#include "engines/wintermute/ad/ad_node_state.h"
#include "engines/wintermute/ad/ad_object.h"
#include "engines/wintermute/ad/ad_path.h"
#include "engines/wintermute/ad/ad_path_point.h"
#include "engines/wintermute/ad/ad_rot_level.h"
#include "engines/wintermute/ad/ad_scale_level.h"
#include "engines/wintermute/ad/ad_scene_node.h"
#include "engines/wintermute/ad/ad_scene_state.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/ad/ad_waypoint_group.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_point.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/dcgf.h"

#ifdef ENABLE_WME3D
#include "engines/wintermute/ad/ad_actor_3dx.h"
#include "engines/wintermute/ad/ad_scene_geometry.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/3dcamera.h"
#include "engines/wintermute/base/gfx/3dlight.h"
#endif

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdScene, false)

//////////////////////////////////////////////////////////////////////////
AdScene::AdScene(BaseGame *inGame) : BaseObject(inGame) {
	_pfTarget = new BasePoint;
	setDefaults();
}


//////////////////////////////////////////////////////////////////////////
AdScene::~AdScene() {
	cleanup();
	_game->unregisterObject(_fader);
	SAFE_DELETE(_pfTarget);
}


//////////////////////////////////////////////////////////////////////////
void AdScene::setDefaults() {
	_initialized = false;
	_pfReady = true;
	_pfTargetPath = nullptr;
	_pfRequester = nullptr;
	_mainLayer = nullptr;
#ifdef ENABLE_WME3D
	_geom = nullptr;
#endif
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

#ifdef ENABLE_WME3D
	_showGeometry = false;
#endif

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

#ifdef ENABLE_WME3D
	_editorResolutionWidth = 0;
	_editorResolutionHeight = 0;
#endif

	_shieldWindow = nullptr;

	_fader = new BaseFader(_game);
	_game->registerObject(_fader);

#ifdef ENABLE_WME3D
	_fov = -1.0f;
#endif
	_viewport = nullptr;

#ifdef ENABLE_WME3D
	_nearClipPlane = -1.0f;
	_farClipPlane = -1.0f;

	_2DPathfinding = false;
	_maxShadowType = SHADOW_FLAT;

	_scroll3DCompatibility = false;
	_ambientLightColor = 0x00000000;

	_fogEnabled = false;
	_fogColor = 0x00FFFFFF;
	_fogStart = 0.0f;
	_fogEnd = 0.0f;
#endif
}


//////////////////////////////////////////////////////////////////////////
void AdScene::cleanup() {
	BaseObject::cleanup();

	_mainLayer = nullptr; // reference only

	SAFE_DELETE(_shieldWindow);

	_game->unregisterObject(_fader);
	_fader = nullptr;

	for (int32 i = 0; i < _layers.getSize(); i++) {
		_game->unregisterObject(_layers[i]);
	}
	_layers.removeAll();


	for (int32 i = 0; i < _waypointGroups.getSize(); i++) {
		_game->unregisterObject(_waypointGroups[i]);
	}
	_waypointGroups.removeAll();

	for (int32 i = 0; i < _scaleLevels.getSize(); i++) {
		_game->unregisterObject(_scaleLevels[i]);
	}
	_scaleLevels.removeAll();

	for (int32 i = 0; i < _rotLevels.getSize(); i++) {
		_game->unregisterObject(_rotLevels[i]);
	}
	_rotLevels.removeAll();


	for (int32 i = 0; i < _pfPath.getSize(); i++) {
		delete _pfPath[i];
	}
	_pfPath.removeAll();
	_pfPointsNum = 0;

	for (int32 i = 0; i < _objects.getSize(); i++) {
		_game->unregisterObject(_objects[i]);
	}
	_objects.removeAll();

#ifdef ENABLE_WME3D
	SAFE_DELETE(_geom);
#endif
	SAFE_DELETE(_viewport);

	setDefaults();
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getPath(const BasePoint &source, const BasePoint &target, AdPath *path, BaseObject *requester) {
	if (!_pfReady) {
		return false;
	} else {
		_pfReady = false;
		*_pfTarget = target;
		_pfTargetPath = path;
		_pfRequester = requester;

		_pfTargetPath->reset();
		_pfTargetPath->setReady(false);

		// prepare working path
		pfPointsStart();

		// first point
		//_pfPath.add(new AdPathPoint(source.x, source.y, 0));

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
		//_pfPath.add(new AdPathPoint(target.x, target.y, INT_MAX));
		pfPointsAdd(target.x, target.y, INT_MAX_VALUE);

		// active waypoints
		for (int32 i = 0; i < _waypointGroups.getSize(); i++) {
			if (_waypointGroups[i]->_active) {
				pfAddWaypointGroup(_waypointGroups[i], requester);
			}
		}


		// free waypoints
		for (int32 i = 0; i < _objects.getSize(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentWptGroup) {
				pfAddWaypointGroup(_objects[i]->_currentWptGroup, requester);
			}
		}
		AdGame *adGame = (AdGame *)_game;
		for (int32 i = 0; i < adGame->_objects.getSize(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentWptGroup) {
				pfAddWaypointGroup(adGame->_objects[i]->_currentWptGroup, requester);
			}
		}

		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
void AdScene::pfAddWaypointGroup(AdWaypointGroup *wpt, BaseObject *requester) {
	if (!wpt->_active) {
		return;
	}

	for (int32 i = 0; i < wpt->_points.getSize(); i++) {
		if (isBlockedAt(wpt->_points[i]->x, wpt->_points[i]->y, true, requester)) {
			continue;
		}

		//_pfPath.add(new AdPathPoint(Wpt->_points[i]->x, Wpt->_points[i]->y, INT_MAX));
		pfPointsAdd(wpt->_points[i]->x, wpt->_points[i]->y, INT_MAX_VALUE);
	}
}


//////////////////////////////////////////////////////////////////////////
float AdScene::getZoomAt(int x, int y) {
	float ret = 100;

	bool found = false;
	if (_mainLayer) {
		for (int32 i = _mainLayer->_nodes.getSize() - 1; i >= 0; i--) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && !node->_region->_blocked && node->_region->pointInRegion(x, y)) {
				if (node->_region->_zoom != 0) {
					ret = node->_region->_zoom;
					found = true;
					break;
				}
			}
		}
	}
	if (!found) {
		ret = getScaleAt(y);
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 AdScene::getAlphaAt(int x, int y, bool colorCheck) {
	if (!_game->_debugDebugMode) {
		colorCheck = false;
	}

	uint32 ret;
	if (colorCheck) {
		ret = 0xFFFF0000;
	} else {
		ret = 0xFFFFFFFF;
	}

	if (_mainLayer) {
		for (int32 i = _mainLayer->_nodes.getSize() - 1; i >= 0; i--) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && (colorCheck || !node->_region->_blocked) && node->_region->pointInRegion(x, y)) {
				if (!node->_region->_blocked) {
					ret = node->_region->_alpha;
				}
				break;
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::isBlockedAt(int x, int y, bool checkFreeObjects, BaseObject *requester) {
	bool ret = true;

	if (checkFreeObjects) {
		for (int32 i = 0; i < _objects.getSize(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentBlockRegion) {
				if (_objects[i]->_currentBlockRegion->pointInRegion(x, y)) {
					return true;
				}
			}
		}
		AdGame *adGame = (AdGame *)_game;
		for (int32 i = 0; i < adGame->_objects.getSize(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentBlockRegion) {
				if (adGame->_objects[i]->_currentBlockRegion->pointInRegion(x, y)) {
					return true;
				}
			}
		}
	}


	if (_mainLayer) {
		for (int32 i = 0; i < _mainLayer->_nodes.getSize(); i++) {
			AdSceneNode *node = _mainLayer->_nodes[i];
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
				} else {
					ret = false;
				}
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::isWalkableAt(int x, int y, bool checkFreeObjects, BaseObject *requester) {
	bool ret = false;

	if (checkFreeObjects) {
		for (int32 i = 0; i < _objects.getSize(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentBlockRegion) {
				if (_objects[i]->_currentBlockRegion->pointInRegion(x, y)) {
					return false;
				}
			}
		}
		AdGame *adGame = (AdGame *)_game;
		for (int32 i = 0; i < adGame->_objects.getSize(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentBlockRegion) {
				if (adGame->_objects[i]->_currentBlockRegion->pointInRegion(x, y)) {
					return false;
				}
			}
		}
	}


	if (_mainLayer) {
		for (int32 i = 0; i < _mainLayer->_nodes.getSize(); i++) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && !node->_region->_decoration && node->_region->pointInRegion(x, y)) {
				if (node->_region->_blocked) {
					ret = false;
					break;
				} else {
					ret = true;
				}
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
int AdScene::getPointsDist(const BasePoint &p1, const BasePoint &p2, BaseObject *requester) {
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
			BaseUtils::swap(&x1, &x2);
			BaseUtils::swap(&y1, &y2);
		}

		yStep = (double)(y2 - y1) / (double)(x2 - x1);
		y = y1;

		for (xCount = x1; xCount < x2; xCount++) {
			if (isBlockedAt(xCount, (int)y, true, requester)) {
				return -1;
			}
			y += yStep;
		}
	} else {
		if (y1 > y2) {
			BaseUtils::swap(&x1, &x2);
			BaseUtils::swap(&y1, &y2);
		}

		xStep = (double)(x2 - x1) / (double)(y2 - y1);
		x = x1;

		for (yCount = y1; yCount < y2; yCount++) {
			if (isBlockedAt((int)x, yCount, true, requester)) {
				return -1;
			}
			x += xStep;
		}
	}
	return MAX(xLength, yLength);
}


//////////////////////////////////////////////////////////////////////////
void AdScene::pathFinderStep() {
	int i;
	// get lowest unmarked
	int lowestDist = INT_MAX_VALUE;
	AdPathPoint *lowestPt = nullptr;

	for (i = 0; i < _pfPointsNum; i++)
		if (!_pfPath[i]->_marked && _pfPath[i]->_distance < lowestDist) {
			lowestDist = _pfPath[i]->_distance;
			lowestPt = _pfPath[i];
		}

	if (lowestPt == nullptr) { // no path -> terminate PathFinder
		_pfReady = true;
		_pfTargetPath->setReady(true);
		return;
	}

	lowestPt->_marked = true;

	// target point marked, generate path and terminate
	if (lowestPt->x == _pfTarget->x && lowestPt->y == _pfTarget->y) {
		while (lowestPt != nullptr) {
			_pfTargetPath->_points.insertAt(0, new BasePoint(lowestPt->x, lowestPt->y));
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
bool AdScene::initLoop() {
#ifdef _DEBUGxxxx
	int nu_steps = 0;
	uint32 start = _game->_currentTime;
	while (!_pfReady && g_system->getMillis() - start <= _pfMaxTime) {
		PathFinderStep();
		nu_steps++;
	}
	if (nu_steps > 0) {
		_game->LOG(0, "STAT: PathFinder iterations in one loop: %d (%s)  _pfMaxTime=%d", nu_steps, _pfReady ? "finished" : "not yet done", _pfMaxTime);
	}
#else
	uint32 start = _game->_currentTime;
	while (!_pfReady && g_system->getMillis() - start <= _pfMaxTime) {
		pathFinderStep();
	}
#endif

#ifdef ENABLE_WME3D
	if (_geom) {
		return _geom->initLoop();
	}
#endif
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_game->LOG(0, "AdScene::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_game->LOG(0, "Error parsing SCENE file '%s'", filename);
	}

	setFilename(filename);

	delete[] buffer;

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
TOKEN_DEF(GEOMETRY) // WME3D
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
TOKEN_DEF(EDITOR_SHOW_GEOMETRY) // WME3D
TOKEN_DEF(EDITOR_RESOLUTION_WIDTH) // WME3D
TOKEN_DEF(EDITOR_RESOLUTION_HEIGHT) // WME3D
TOKEN_DEF(FOV_OVERRIDE) // WME3D
TOKEN_DEF(WAYPOINT_HEIGHT) // WME3D
TOKEN_DEF(NEAR_CLIPPING_PLANE) // WME3D
TOKEN_DEF(FAR_CLIPPING_PLANE) // WME3D
TOKEN_DEF(2D_PATHFINDING) // WME3D
TOKEN_DEF(MAX_SHADOW_TYPE) // WME3D
TOKEN_DEF(SCROLL_3D_COMPATIBILITY) // WME3D
TOKEN_DEF(AMBIENT_LIGHT_COLOR) // WME3D
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdScene::loadBuffer(char *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(SCENE)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(LAYER)
	TOKEN_TABLE(WAYPOINTS)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(GEOMETRY) // WME3D
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
	TOKEN_TABLE(EDITOR_SHOW_GEOMETRY) // WME3D
	TOKEN_TABLE(EDITOR_RESOLUTION_WIDTH) // WME3D
	TOKEN_TABLE(EDITOR_RESOLUTION_HEIGHT) // WME3D
	TOKEN_TABLE(FOV_OVERRIDE) // WME3D
	TOKEN_TABLE(WAYPOINT_HEIGHT) // WME3D
	TOKEN_TABLE(NEAR_CLIPPING_PLANE) // WME3D
	TOKEN_TABLE(FAR_CLIPPING_PLANE) // WME3D
	TOKEN_TABLE(2D_PATHFINDING) // WME3D
	TOKEN_TABLE(MAX_SHADOW_TYPE) // WME3D
	TOKEN_TABLE(SCROLL_3D_COMPATIBILITY) // WME3D
	TOKEN_TABLE(AMBIENT_LIGHT_COLOR) // WME3D
	TOKEN_TABLE_END

	cleanup();

	char *params;
	int cmd;
	BaseParser parser(_game);

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_SCENE) {
			_game->LOG(0, "'SCENE' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	int ar, ag, ab, aa;
	char camera[MAX_PATH_LENGTH] = "";
#ifdef ENABLE_WME3D
	float waypointHeight = -1.0f;
#endif

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile(params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName(params);
			break;

		case TOKEN_CAPTION:
			setCaption(params);
			break;

		case TOKEN_LAYER: {
			AdLayer *layer = new AdLayer(_game);
			if (!layer || DID_FAIL(layer->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				SAFE_DELETE(layer);
			} else {
				_game->registerObject(layer);
				_layers.add(layer);
				if (layer->_main) {
					_mainLayer = layer;
					_width = layer->_width;
					_height = layer->_height;
				}
			}
		}
		break;

		case TOKEN_WAYPOINTS: {
			AdWaypointGroup *wpt = new AdWaypointGroup(_game);
			if (!wpt || DID_FAIL(wpt->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				SAFE_DELETE(wpt);
			} else {
				_game->registerObject(wpt);
				_waypointGroups.add(wpt);
			}
		}
		break;

		case TOKEN_SCALE_LEVEL: {
			AdScaleLevel *sl = new AdScaleLevel(_game);
			if (!sl || DID_FAIL(sl->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				SAFE_DELETE(sl);
			} else {
				_game->registerObject(sl);
				_scaleLevels.add(sl);
			}
		}
		break;

		case TOKEN_ROTATION_LEVEL: {
			AdRotLevel *rl = new AdRotLevel(_game);
			if (!rl || DID_FAIL(rl->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				SAFE_DELETE(rl);
			} else {
				_game->registerObject(rl);
				_rotLevels.add(rl);
			}
		}
		break;

		case TOKEN_ENTITY: {
			AdEntity *entity = new AdEntity(_game);
			if (!entity || DID_FAIL(entity->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				SAFE_DELETE(entity);
			} else {
				addObject(entity);
			}
		}
		break;

		case TOKEN_CURSOR:
			SAFE_DELETE(_cursor);
			_cursor = new BaseSprite(_game);
			if (!_cursor || DID_FAIL(_cursor->loadFile(params))) {
				SAFE_DELETE(_cursor);
				cmd = PARSERR_GENERIC;
			}
			break;

#ifdef ENABLE_WME3D
		case TOKEN_GEOMETRY:
			SAFE_DELETE(_geom);
			if (!_game->_useD3D) {
				break;
			}
			_geom = new AdSceneGeometry(_game);
			if (!_geom || !_geom->loadFile(params)) {
				SAFE_DELETE(_geom);
				cmd = PARSERR_GENERIC;
			}
			break;
#endif

		case TOKEN_CAMERA:
			Common::strlcpy(camera, params, MAX_PATH_LENGTH);
			break;

		case TOKEN_EDITOR_MARGIN_H:
			parser.scanStr(params, "%d", &_editorMarginH);
			break;

		case TOKEN_EDITOR_MARGIN_V:
			parser.scanStr(params, "%d", &_editorMarginV);
			break;

		case TOKEN_EDITOR_COLOR_FRAME:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColFrame = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColEntity = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY_SEL:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColEntitySel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION_SEL:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColRegionSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION_SEL:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColDecorSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED_SEL:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColBlockedSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS_SEL:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColWaypointsSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColRegion = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColDecor = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColBlocked = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColWaypoints = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_SCALE:
			parser.scanStr(params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColScale = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_SHOW_REGIONS:
			parser.scanStr(params, "%b", &_editorShowRegions);
			break;

		case TOKEN_EDITOR_SHOW_BLOCKED:
			parser.scanStr(params, "%b", &_editorShowBlocked);
			break;

		case TOKEN_EDITOR_SHOW_DECORATION:
			parser.scanStr(params, "%b", &_editorShowDecor);
			break;

		case TOKEN_EDITOR_SHOW_ENTITIES:
			parser.scanStr(params, "%b", &_editorShowEntities);
			break;

		case TOKEN_EDITOR_SHOW_SCALE:
			parser.scanStr(params, "%b", &_editorShowScale);
			break;

#ifdef ENABLE_WME3D
		case TOKEN_EDITOR_SHOW_GEOMETRY:
			parser.scanStr(params, "%b", &_showGeometry);
			if (!_game->_editorMode)
				_showGeometry = false;
			break;
#endif

		case TOKEN_SCRIPT:
			addScript(params);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_VIEWPORT: {
			Common::Rect32 rc;
			parser.scanStr(params, "%d,%d,%d,%d", &rc.left, &rc.top, &rc.right, &rc.bottom);
			if (!_viewport) {
				_viewport = new BaseViewport(_game);
			}
			if (_viewport) {
				_viewport->setRect(rc.left, rc.top, rc.right, rc.bottom, true);
			}
		}
		break;

		case TOKEN_PERSISTENT_STATE:
			parser.scanStr(params, "%b", &_persistentState);
			break;

		case TOKEN_PERSISTENT_STATE_SPRITES:
			parser.scanStr(params, "%b", &_persistentStateSprites);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

#ifdef ENABLE_WME3D
		case TOKEN_EDITOR_RESOLUTION_WIDTH:
			parser.scanStr(params, "%d", &_editorResolutionWidth);
			break;

		case TOKEN_EDITOR_RESOLUTION_HEIGHT:
			parser.scanStr(params, "%d", &_editorResolutionHeight);
			break;

		case TOKEN_FOV_OVERRIDE:
			parser.scanStr(params, "%f", &_fov);
			break;

		case TOKEN_WAYPOINT_HEIGHT:
			parser.scanStr(params, "%f", &waypointHeight);
			break;

		case TOKEN_NEAR_CLIPPING_PLANE:
			parser.scanStr(params, "%f", &_nearClipPlane);
			break;

		case TOKEN_FAR_CLIPPING_PLANE:
			parser.scanStr(params, "%f", &_farClipPlane);
			break;

		case TOKEN_2D_PATHFINDING:
			parser.scanStr(params, "%b", &_2DPathfinding);
			break;

		case TOKEN_MAX_SHADOW_TYPE: {
			int maxShadowType = SHADOW_NONE;
			parser.scanStr(params, "%d", &maxShadowType);
			setMaxShadowType(static_cast<TShadowType>(maxShadowType));
			}
			break;

		case TOKEN_SCROLL_3D_COMPATIBILITY:
			parser.scanStr(params, "%b", &_scroll3DCompatibility);
			break;

		case TOKEN_AMBIENT_LIGHT_COLOR:
			parser.scanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			_ambientLightColor = BYTETORGBA(ar, ag, ab, 255);
			break;
#endif

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_game->LOG(0, "Syntax error in SCENE definition");
		return STATUS_FAILED;
	}

	if (_mainLayer == nullptr) {
		_game->LOG(0, "Warning: scene '%s' has no main layer.", getFilename());
	}

#ifdef ENABLE_WME3D
	if (_geom && camera[0] != '\0') {
		_geom->setActiveCamera(camera, _fov, _nearClipPlane, _farClipPlane);
	}
#endif

	sortScaleLevels();
	sortRotLevels();

	_initialized = true;

#ifdef ENABLE_WME3D
	if (_geom) {
		if (waypointHeight >= 0.0f) {
			_geom->_waypointHeight = waypointHeight;
			_geom->dropWaypoints();
		}

		if (_geom->_activeCamera >= 0 && _geom->_activeCamera < _geom->_cameras.getSize()) {
			_game->_renderer->setup3D(_geom->_cameras[_geom->_activeCamera]);
			_game->_renderer->setScreenViewport();
			_geom->render(false);
		}
	}

	if (_mainLayer) {
		if (_editorResolutionWidth <= 0)
			_editorResolutionWidth = _mainLayer->_width;
		if (_editorResolutionHeight <= 0)
			_editorResolutionHeight = _mainLayer->_height;
	}
#endif

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::traverseNodes(bool doUpdate) {
	if (!_initialized) {
		return STATUS_OK;
	}

	AdGame *adGame = (AdGame *)_game;


	//////////////////////////////////////////////////////////////////////////
	// prepare viewport
	bool popViewport = false;
	if (_viewport && !_game->_editorMode) {
		_game->pushViewport(_viewport);
		popViewport = true;
	} else if (adGame->_sceneViewport && !_game->_editorMode) {
		_game->pushViewport(adGame->_sceneViewport);
		popViewport = true;
	}


	//////////////////////////////////////////////////////////////////////////
	// *** adjust scroll offset
	if (doUpdate) {
		/*
		if (_autoScroll && _game->_mainObject != nullptr)
		{
		    ScrollToObject(_game->_mainObject);
		}
		*/

		if (_autoScroll) {
			// adjust horizontal scroll
			if (_game->_timer - _lastTimeH >= _scrollTimeH) {

				// Cap the amount of catch-up to avoid jittery characters.
				int timesMissed = (_game->_timer - _lastTimeH) / _scrollTimeH;
				if (timesMissed > 2) {
					timesMissed = 2;
				}

				_lastTimeH = _game->_timer;
				if (_offsetLeft < _targetOffsetLeft) {
					_offsetLeft += _scrollPixelsH * timesMissed;
					_offsetLeft = MIN(_offsetLeft, _targetOffsetLeft);
				} else if (_offsetLeft > _targetOffsetLeft) {
					_offsetLeft -= _scrollPixelsH * timesMissed;
					_offsetLeft = MAX(_offsetLeft, _targetOffsetLeft);
				}
			}

			// adjust vertical scroll
			if (_game->_timer - _lastTimeV >= _scrollTimeV) {

				// Cap the amount of catch-up to avoid jittery characters.
				int timesMissed = (_game->_timer - _lastTimeV) / _scrollTimeV;
				if (timesMissed > 2) {
					timesMissed = 2;
				}

				_lastTimeV = _game->_timer;
				if (_offsetTop < _targetOffsetTop) {
					_offsetTop += _scrollPixelsV * timesMissed;
					_offsetTop = MIN(_offsetTop, _targetOffsetTop);
				} else if (_offsetTop > _targetOffsetTop) {
					_offsetTop -= _scrollPixelsV * timesMissed;
					_offsetTop = MAX(_offsetTop, _targetOffsetTop);
				}
			}

			if (_offsetTop == _targetOffsetTop && _offsetLeft == _targetOffsetLeft) {
				_ready = true;
			}
		} else {
			_ready = true;    // not scrolling, i.e. always ready
		}
	}




	//////////////////////////////////////////////////////////////////////////
	int32 viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	int32 viewportX, viewportY;
	getViewportOffset(&viewportX, &viewportY);

	int32 scrollableX = _width  - viewportWidth;
	int32 scrollableY = _height - viewportHeight;

	double widthRatio  = scrollableX <= 0 ? 0 : ((double)(_offsetLeft) / (double)scrollableX);
	double heightRatio = scrollableY <= 0 ? 0 : ((double)(_offsetTop)  / (double)scrollableY);

	int origX, origY;
	_game->getOffset(&origX, &origY);



	//////////////////////////////////////////////////////////////////////////
	// *** display/update everything
	_game->_renderer->setup2D();

	// for each layer
#ifdef ENABLE_WME3D
	int mainOffsetX = 0;
	int mainOffsetY = 0;
#endif

	for (int32 j = 0; j < _layers.getSize(); j++) {
		if (!_layers[j]->_active) {
			continue;
		}

		// make layer exclusive
		if (!doUpdate) {
			if (_layers[j]->_closeUp && !_game->_editorMode) {
				if (!_shieldWindow) {
					_shieldWindow = new UIWindow(_game);
				}
				if (_shieldWindow) {
					_shieldWindow->_posX = _shieldWindow->_posY = 0;
					_shieldWindow->_width = _game->_renderer->getWidth();
					_shieldWindow->_height = _game->_renderer->getHeight();
					_shieldWindow->display();
				}
			}
		}

		if (_paralaxScrolling) {
			int offsetX = (int)(widthRatio  * (_layers[j]->_width  - viewportWidth) - viewportX);
			int offsetY = (int)(heightRatio * (_layers[j]->_height - viewportHeight) - viewportY);
			_game->setOffset(offsetX, offsetY);

			_game->_offsetPercentX = (float)offsetX / ((float)_layers[j]->_width - viewportWidth) * 100.0f;
			_game->_offsetPercentY = (float)offsetY / ((float)_layers[j]->_height - viewportHeight) * 100.0f;

			//_game->QuickMessageForm("%d %f", OffsetX+ViewportX, _game->_offsetPercentX);
		} else {
			_game->setOffset(_offsetLeft - viewportX, _offsetTop - viewportY);

			_game->_offsetPercentX = (float)(_offsetLeft - viewportX) / ((float)_layers[j]->_width - viewportWidth) * 100.0f;
			_game->_offsetPercentY = (float)(_offsetTop - viewportY) / ((float)_layers[j]->_height - viewportHeight) * 100.0f;
		}

#ifdef ENABLE_WME3D
		// render depth info for stencil shadows
		if (!doUpdate && _geom && _layers[j]->_main) {
			_game->getOffset(&mainOffsetX, &mainOffsetY);

			TShadowType shadowType = _game->getMaxShadowType();
			if (shadowType >= SHADOW_STENCIL) {
				_game->renderShadowGeometry();
			}
		}
#endif

		// for each node
		for (int32 k = 0; k < _layers[j]->_nodes.getSize(); k++) {
			AdSceneNode *node = _layers[j]->_nodes[k];
			switch (node->_type) {
			case OBJECT_ENTITY:
				if (node->_entity->_active && (_game->_editorMode || !node->_entity->_editorOnly)) {
#ifndef ENABLE_WME3D
					_game->_renderer->setup2D();

					if (doUpdate) {
						node->_entity->update();
					} else {
						node->_entity->display();
					}
#else
					if (node->_entity->_is3D) {
						_game->_renderer->setup3D();
					} else {
						_game->_renderer->setup2D();
					}

					// only display 3D if geometry is set
					if (!node->_entity->_is3D || _geom) {
						if (doUpdate) {
							node->_entity->update();
						} else {
							node->_entity->display();
						}
					}
#endif
				}
				break;

			case OBJECT_REGION: {
				if (node->_region->_blocked) {
					break;
				}
				if (node->_region->_decoration) {
					break;
				}

				if (!doUpdate) {
					displayRegionContent(node->_region);
				}
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
				displayRegionContent(nullptr);
			}
		}
	} // each layer

#ifdef ENABLE_WME3D
	// display hidden geometry
	if (!doUpdate && _geom) {
		_game->setOffset(mainOffsetX, mainOffsetY);
		_geom->render(_showGeometry);
	}
#endif

	// restore state
	_game->setOffset(origX, origY);
	_game->_renderer->setup2D();

	// display/update fader
	if (_fader) {
		if (doUpdate) {
			_fader->update();
		} else {
			_fader->display();
		}
	}

	if (popViewport) {
		_game->popViewport();
	}

	return STATUS_OK;
}

#ifdef ENABLE_WME3D
//////////////////////////////////////////////////////////////////////////
bool AdScene::display3DContent(DXMatrix &viewMat, DXMatrix &projMat) {
	if (!_geom)
		return STATUS_FAILED;

	_game->_renderer3D->setup3DCustom(viewMat, projMat);

	// for each layer
	for (int32 j = 0; j < _layers.getSize(); j++) {
		if (!_layers[j]->_active)
			continue;

		// render depth info for stencil shadows
		TShadowType shadowType = _game->getMaxShadowType();
		if (shadowType >= SHADOW_STENCIL) {
			_game->renderShadowGeometry();
		}


		// for each node
		for (int32 k = 0; k < _layers[j]->_nodes.getSize(); k++) {
			AdSceneNode *node = _layers[j]->_nodes[k];
			if (node->_type == OBJECT_REGION) {
				if (node->_region->_blocked)
					continue;
				if (node->_region->_decoration)
					continue;

				displayRegionContent(node->_region, true);
			}
		} // each node

		// display/update all objects which are off-regions
		if (_layers[j]->_main) {
			displayRegionContent(nullptr, true);
		}
	} // each layer

	return STATUS_OK;
}
#endif

//////////////////////////////////////////////////////////////////////////
bool AdScene::display() {
	return traverseNodes(false);
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::updateFreeObjects() {
	AdGame *adGame = (AdGame *)_game;
#ifdef ENABLE_WME3D
	bool is3DSet = false;
#endif

	// *** update all active objects
	for (int32 i = 0; i < adGame->_objects.getSize(); i++) {
		if (!adGame->_objects[i]->_active) {
			continue;
		}

#ifdef ENABLE_WME3D
		if (adGame->_objects[i]->_is3D && _geom) {
			if (_geom->_activeCamera >= 0 && _geom->_activeCamera < _geom->_cameras.getSize()) {
				_game->_renderer->setup3D(_geom->_cameras[_geom->_activeCamera], !is3DSet);
				is3DSet = true;
			}
		}
#endif
		adGame->_objects[i]->update();
		adGame->_objects[i]->_drawn = false;
	}


	for (int32 i = 0; i < _objects.getSize(); i++) {
		if (!_objects[i]->_active) {
			continue;
		}
#ifdef ENABLE_WME3D
		if (_objects[i]->_is3D && _geom) {
			if (_geom->_activeCamera >= 0 && _geom->_activeCamera < _geom->_cameras.getSize()) {
				_game->_renderer->setup3D(_geom->_cameras[_geom->_activeCamera], !is3DSet);
				is3DSet = true;
			}
		}
#endif

		_objects[i]->update();
		_objects[i]->_drawn = false;
	}


	if (_autoScroll && _game->_mainObject != nullptr) {
		scrollToObject(_game->_mainObject);
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::displayRegionContent(AdRegion *region, bool display3DOnly) {
	AdGame *adGame = (AdGame *)_game;
	BaseArray<AdObject *> objects;
	AdObject *obj;

	// global objects
	for (int32 i = 0; i < adGame->_objects.getSize(); i++) {
		obj = adGame->_objects[i];
		if (obj->_active && !obj->_drawn && (obj->_stickRegion == region || region == nullptr || (obj->_stickRegion == nullptr && region->pointInRegion(obj->_posX, obj->_posY)))) {
			objects.add(obj);
		}
	}

	// scene objects
	for (int32 i = 0; i < _objects.getSize(); i++) {
		obj = _objects[i];
		if (obj->_active && !obj->_editorOnly && !obj->_drawn && (obj->_stickRegion == region || region == nullptr || (obj->_stickRegion == nullptr && region->pointInRegion(obj->_posX, obj->_posY)))) {
			objects.add(obj);
		}
	}

	// sort by _posY
	qsort_msvc(objects.getData(), objects.getSize(), sizeof(AdObject *), AdScene::compareObjs);

	// display them
	for (int32 i = 0; i < objects.getSize(); i++) {
		obj = objects[i];

		if (display3DOnly && !obj->_is3D) {
			continue;
		}

#ifndef ENABLE_WME3D
		_game->_renderer->setup2D();
#else
		if (objects[i]->_is3D && _geom) {
			if (_geom->_activeCamera >= 0 && _geom->_activeCamera < _geom->_cameras.getSize()) {
				_game->_renderer->setup3D(_geom->_cameras[_geom->_activeCamera]);
			}
		} else {
			_game->_renderer->setup2D();
		}
#endif

		if (_game->_editorMode || !obj->_editorOnly) {
			obj->display();
		}
		obj->_drawn = true;
	}


	// display design only objects
	if (!display3DOnly) {
		if (_game->_editorMode && region == nullptr) {
			for (int32 i = 0; i < _objects.getSize(); i++) {
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
int AdScene::compareObjs(const void *obj1, const void *obj2) {
	void *o1 = const_cast<void *>(obj1);
	void *o2 = const_cast<void *>(obj2);
	AdObject *object1 = *(AdObject **)o1;
	AdObject *object2 = *(AdObject **)o2;

	if (object1->_posY < object2->_posY)
		return -1;
	else if (object1->_posY > object2->_posY)
		return 1;
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::displayRegionContentOld(AdRegion *region) {
	AdGame *adGame = (AdGame *)_game;
	AdObject *obj;

	// display all objects in region sorted by _posY
	do {
		obj = nullptr;
		int minY = INT_MAX_VALUE;

		// global objects
		for (int32 i = 0; i < adGame->_objects.getSize(); i++) {
			if (adGame->_objects[i]->_active && !adGame->_objects[i]->_drawn && adGame->_objects[i]->_posY < minY && (adGame->_objects[i]->_stickRegion == region || region == nullptr || (adGame->_objects[i]->_stickRegion == nullptr && region->pointInRegion(adGame->_objects[i]->_posX, adGame->_objects[i]->_posY)))) {
				obj = adGame->_objects[i];
				minY = adGame->_objects[i]->_posY;
			}
		}

		// scene objects
		for (int32 i = 0; i < _objects.getSize(); i++) {
			if (_objects[i]->_active && !_objects[i]->_editorOnly && !_objects[i]->_drawn && _objects[i]->_posY < minY && (_objects[i]->_stickRegion == region || region == nullptr || (_objects[i]->_stickRegion == nullptr && region->pointInRegion(_objects[i]->_posX, _objects[i]->_posY)))) {
				obj = _objects[i];
				minY = _objects[i]->_posY;
			}
		}


		if (obj != nullptr) {
#ifndef ENABLE_WME3D
			_game->_renderer->setup2D();
#else
			if (obj->_is3D && _geom) {
				if (_geom->_activeCamera >= 0 && _geom->_activeCamera < _geom->_cameras.getSize()) {
					_game->_renderer->setup3D(_geom->_cameras[_geom->_activeCamera]);
				}
			} else {
				_game->_renderer->setup2D();
			}
#endif
			if (_game->_editorMode || !obj->_editorOnly) {
				obj->display();
			}
			obj->_drawn = true;
		}
	} while (obj != nullptr);


	// design only objects
	if (_game->_editorMode && region == nullptr) {
		for (int32 i = 0; i < _objects.getSize(); i++) {
			if (_objects[i]->_active && _objects[i]->_editorOnly) {
				_objects[i]->display();
				_objects[i]->_drawn = true;
			}
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::update() {
	return traverseNodes(true);
}

//////////////////////////////////////////////////////////////////////////
void AdScene::scrollTo(int offsetX, int offsetY) {
	int32 viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	int32 origOffsetLeft = _targetOffsetLeft;
	int32 origOffsetTop = _targetOffsetTop;

	_targetOffsetLeft = MAX<int32>(0, offsetX - viewportWidth / 2);
	_targetOffsetLeft = MIN(_targetOffsetLeft, _width - viewportWidth);

	_targetOffsetTop = MAX<int32>(0, offsetY - viewportHeight / 2);
	_targetOffsetTop = MIN(_targetOffsetTop, _height - viewportHeight);


	if (_game->_mainObject && _game->_mainObject->_is3D) {
		if (abs(origOffsetLeft - _targetOffsetLeft) < 5) {
			_targetOffsetLeft = origOffsetLeft;
		}
		if (abs(origOffsetTop - _targetOffsetTop) < 5) {
			_targetOffsetTop = origOffsetTop;
		}
		//_targetOffsetTop = 0;
	}

	_ready = false;
}


//////////////////////////////////////////////////////////////////////////
void AdScene::scrollToObject(BaseObject *object) {
	if (object) {
		scrollTo(object->_posX, object->_posY - object->getHeight() / 2);
	}
}


//////////////////////////////////////////////////////////////////////////
void AdScene::skipToObject(BaseObject *object) {
	if (object) {
		skipTo(object->_posX, object->_posY - object->getHeight() / 2);
	}
}


//////////////////////////////////////////////////////////////////////////
void AdScene::skipTo(int offsetX, int offsetY) {
	int32 viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	_offsetLeft = MAX<int32>(0, offsetX - viewportWidth / 2);
	_offsetLeft = MIN(_offsetLeft, _width - viewportWidth);

	_offsetTop = MAX<int32>(0, offsetY - viewportHeight / 2);
	_offsetTop = MIN(_offsetTop, _height - viewportHeight);

	_targetOffsetLeft = _offsetLeft;
	_targetOffsetTop = _offsetTop;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdScene::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// LoadActor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "LoadActor") == 0) {
		stack->correctParams(1);
		AdActor *act = new AdActor(_game);
		if (act && DID_SUCCEED(act->loadFile(stack->pop()->getString()))) {
			addObject(act);
			stack->pushNative(act, true);
		} else {
			delete act;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// LoadActor3D
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "LoadActor3D") == 0) {
		stack->correctParams(1);
		AdActor3DX *act = new AdActor3DX(_game);
		if (act && DID_SUCCEED(act->loadFile(stack->pop()->getString()))) {
			addObject(act);
			stack->pushNative(act, true);
		} else {
			stack->pushNULL();
		}
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// LoadEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadEntity") == 0) {
		stack->correctParams(1);
		AdEntity *ent = new AdEntity(_game);
		if (ent && DID_SUCCEED(ent->loadFile(stack->pop()->getString()))) {
			addObject(ent);
			stack->pushNative(ent, true);
		} else {
			delete ent;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateEntity") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdEntity *ent = new AdEntity(_game);
		addObject(ent);
		if (!val->isNULL()) {
			ent->setName(val->getString());
		}
		stack->pushNative(ent, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadObject / UnloadActor / UnloadEntity / UnloadActor3D / DeleteEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadObject") == 0 || strcmp(name, "UnloadActor") == 0 || strcmp(name, "UnloadEntity") == 0 || strcmp(name, "UnloadActor3D") == 0 || strcmp(name, "DeleteEntity") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		AdObject *obj = (AdObject *)val->getNative();
		removeObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) {
			val->setNULL();
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SkipTo") == 0) {
		stack->correctParams(2);
		ScValue *val1 = stack->pop();
		ScValue *val2 = stack->pop();
		if (val1->isNative()) {
			skipToObject((BaseObject *)val1->getNative());
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
		ScValue *val1 = stack->pop();
		ScValue *val2 = stack->pop();
		if (val1->isNative()) {
			scrollToObject((BaseObject *)val1->getNative());
		} else {
			scrollTo(val1->getInt(), val2->getInt());
		}
		if (strcmp(name, "ScrollTo") == 0) {
			script->waitForExclusive(this);
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLayer") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		if (val->isInt()) {
			int layer = val->getInt();
			if (layer < 0 || layer >= _layers.getSize()) {
				stack->pushNULL();
			} else {
				stack->pushNative(_layers[layer], true);
			}
		} else {
			const char *layerName = val->getString();
			bool layerFound = false;
			for (int32 i = 0; i < _layers.getSize(); i++) {
				if (scumm_stricmp(layerName, _layers[i]->_name) == 0) {
					stack->pushNative(_layers[i], true);
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				stack->pushNULL();
			}
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaypointGroup
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWaypointGroup") == 0) {
		stack->correctParams(1);
		int group = stack->pop()->getInt();
		if (group < 0 || group >= _waypointGroups.getSize()) {
			stack->pushNULL();
		} else {
			stack->pushNative(_waypointGroups[group], true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNode") == 0) {
		stack->correctParams(1);
		const char *nodeName = stack->pop()->getString();

		BaseObject *node = getNodeByName(nodeName);
		if (node) {
			stack->pushNative((BaseScriptable *)node, true);
		}

#ifdef ENABLE_HEROCRAFT
		//////////////////////////////////////////////////////////////////////////
		// [HeroCraft] papasEntity
		// Used in some self-check at "Papa's Daughters 2"
		//////////////////////////////////////////////////////////////////////////
		else if (BaseEngine::instance().getTargetExecutable() == WME_HEROCRAFT && strcmp(nodeName,"papasEntity") == 0) {
			stack->pushInt(777);
		}
#endif

		else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFreeNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFreeNode") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdObject *ret = nullptr;
		if (val->isInt()) {
			int index = val->getInt();
			if (index >= 0 && index < _objects.getSize()) {
				ret = _objects[index];
			}
		} else {
			const char *nodeName = val->getString();
			for (int32 i = 0; i < _objects.getSize(); i++) {
				if (_objects[i] && _objects[i]->_name && scumm_stricmp(_objects[i]->_name, nodeName) == 0) {
					ret = _objects[i];
					break;
				}
			}
		}
		if (ret) {
			stack->pushNative(ret, true);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRegionAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetRegionAt") == 0) {
		stack->correctParams(3);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		ScValue *val = stack->pop();

		bool includeDecors = false;
		if (!val->isNULL()) {
			includeDecors = val->getBool();
		}

		if (_mainLayer) {
			for (int32 i = _mainLayer->_nodes.getSize() - 1; i >= 0; i--) {
				AdSceneNode *node = _mainLayer->_nodes[i];
				if (node->_type == OBJECT_REGION && node->_region->_active && node->_region->pointInRegion(x, y)) {
					if (node->_region->_decoration && !includeDecors) {
						continue;
					}

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
			if (_targetOffsetLeft != _offsetLeft || _targetOffsetTop != _offsetTop) {
				ret = true;
			}
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
		if (strcmp(name, "FadeOutAsync") != 0) {
			script->waitFor(_fader);
		}

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
		if (strcmp(name, "FadeInAsync") != 0) {
			script->waitFor(_fader);
		}

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

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// EnableNode3D
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EnableNode3D") == 0) {
		stack->correctParams(1);
		const char *nodeName = stack->pop()->getString();

		if (!_geom) {
			script->runtimeError("Scene.EnableNode3D: Scene doesn't contain any geometry");
			stack->pushBool(false);
		} else {
			bool ret = _geom->enableNode(nodeName, true);
			stack->pushBool(ret);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisableNode3D
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisableNode3D") == 0) {
		stack->correctParams(1);
		const char *nodeName = stack->pop()->getString();

		if (!_geom) {
			script->runtimeError("Scene.EnableNode3D: Scene doesn't contain any geometry");
			stack->pushBool(false);
		} else {
			bool ret = _geom->enableNode(nodeName, false);
			stack->pushBool(ret);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsNode3DEnabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsNode3DEnabled") == 0) {
		stack->correctParams(1);
		const char *nodeName = stack->pop()->getString();

		if (_geom) {
			stack->pushBool(_geom->isNodeEnabled(nodeName));
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EnableLight
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EnableLight") == 0) {
		stack->correctParams(1);

		const char *lightName = stack->pop()->getString();

		if (!_geom) {
			script->runtimeError("Scene.EnableLight: Scene doesn't contain any geometry");
			stack->pushBool(false);
		} else {
			bool res = _geom->enableLight(lightName, true);
			stack->pushBool(res);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisableLight
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisableLight") == 0) {
		stack->correctParams(1);

		const char *lightName = stack->pop()->getString();

		if (!_geom) {
			script->runtimeError("Scene.DisableLight: Scene doesn't contain any geometry");
			stack->pushBool(false);
		} else {
			bool res = _geom->enableLight(lightName, false);
			stack->pushBool(res);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsLightEnabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsLightEnabled") == 0) {
		stack->correctParams(1);

		const char *lightName = stack->pop()->getString();

		if (_geom) {
			stack->pushBool(_geom->isLightEnabled(lightName));
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLightName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLightName") == 0) {
		stack->correctParams(1);

		int32 index = stack->pop()->getInt();
        if (_geom ) {
			if (index >= 0 && index < _geom->_lights.getSize()) {
				stack->pushString(_geom->_lights[index]->_name);
			} else {
				stack->pushNULL();
			}
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetLightColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetLightColor") == 0) {
		stack->correctParams(2);

		const char *lightName = stack->pop()->getString();
		uint32 color = static_cast<uint32>(stack->pop()->getInt());

		if (!_geom) {
			script->runtimeError("Scene.SetLightColor: Scene doesn't contain any geometry");
			stack->pushBool(false);
		} else {
			bool ret = _geom->setLightColor(lightName, color);
			stack->pushBool(ret);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLightColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLightColor") == 0) {
		stack->correctParams(1);
		const char *lightName = stack->pop()->getString();

		if (!_geom) {
			script->runtimeError("Scene.GetLightColor: Scene doesn't contain any geometry");
			stack->pushInt(0);
		} else {
			stack->pushInt(_geom->getLightColor(lightName));
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLightPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLightPosition") == 0) {
		stack->correctParams(1);
		const char *lightName = stack->pop()->getString();

		if (!_geom) {
			script->runtimeError("Scene.GetLightPosition: Scene doesn't contain any geometry");
			stack->pushInt(0);
		} else {
			DXVector3 pos = _geom->getLightPos(lightName);
			ScValue *val = stack->getPushValue();

			if (val) {
				val->setProperty("X", pos._x);
				val->setProperty("Y", pos._y);
				val->setProperty("Z", pos._z);
			}
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetActiveCamera
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetActiveCamera") == 0) {
		stack->correctParams(1);

		const char *cameraName = stack->pop()->getString();

		if (!_geom) {
			script->runtimeError("Scene.SetActiveCamera: Scene doesn't contain any geometry");
			stack->pushBool(false);
		} else {
			bool res = _geom->setActiveCamera(cameraName, _fov, _nearClipPlane, _farClipPlane);
			if (!res) {
				script->runtimeError("Scene.SetActiveCamera failed");
			}
			stack->pushBool(res);
		}

		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// SetViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetViewport") == 0) {
		stack->correctParams(4);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		int width = stack->pop()->getInt();
		int height = stack->pop()->getInt();

		if (width <= 0) {
			width = _game->_renderer->getWidth();
		}
		if (height <= 0) {
			height = _game->_renderer->getHeight();
		}

		if (!_viewport) {
			_viewport = new BaseViewport(_game);
		}
		if (_viewport) {
			_viewport->setRect(x, y, x + width, y + height);
		}

		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddLayer") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdLayer *layer = new AdLayer(_game);
		if (!val->isNULL()) {
			layer->setName(val->getString());
		}
		if (_mainLayer) {
			layer->_width = _mainLayer->_width;
			layer->_height = _mainLayer->_height;
		}
		_layers.add(layer);
		_game->registerObject(layer);

		stack->pushNative(layer, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertLayer") == 0) {
		stack->correctParams(2);
		int32 index = stack->pop()->getInt();
		ScValue *val = stack->pop();

		AdLayer *layer = new AdLayer(_game);
		if (!val->isNULL()) {
			layer->setName(val->getString());
		}
		if (_mainLayer) {
			layer->_width = _mainLayer->_width;
			layer->_height = _mainLayer->_height;
		}
		if (index < 0) {
			index = 0;
		}
		if (index <= _layers.getSize() - 1) {
			_layers.insertAt(index, layer);
		} else {
			_layers.add(layer);
		}

		_game->registerObject(layer);

		stack->pushNative(layer, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteLayer") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdLayer *toDelete = nullptr;
		if (val->isNative()) {
			BaseScriptable *temp = val->getNative();
			for (int32 i = 0; i < _layers.getSize(); i++) {
				if (_layers[i] == temp) {
					toDelete = _layers[i];
					break;
				}
			}
		} else {
			int32 index = val->getInt();
			if (index >= 0 && index < _layers.getSize()) {
				toDelete = _layers[index];
			}
		}
		if (toDelete == nullptr) {
			stack->pushBool(false);
			return STATUS_OK;
		}

		if (toDelete->_main) {
			script->runtimeError("Scene.DeleteLayer - cannot delete main scene layer");
			stack->pushBool(false);
			return STATUS_OK;
		}

		for (int32 i = 0; i < _layers.getSize(); i++) {
			if (_layers[i] == toDelete) {
				_layers.removeAt(i);
				_game->unregisterObject(toDelete);
				break;
			}
		}
		stack->pushBool(true);
		return STATUS_OK;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// EnableFog
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EnableFog") == 0) {
		stack->correctParams(3);
		_fogEnabled = true;
		_fogColor = stack->pop()->getInt();
		_fogStart = stack->pop()->getFloat();
		_fogEnd = stack->pop()->getFloat();

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisableFog
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisableFog") == 0) {
		stack->correctParams(0);
		_fogEnabled = false;

		stack->pushNULL();
		return STATUS_OK;
	}
#endif

	else {
		return BaseObject::scCallMethod(script, stack, thisStack, name);
	}
}

//////////////////////////////////////////////////////////////////////////
ScValue *AdScene::scGetProperty(const char *name) {
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
		_scValue->setInt(_layers.getSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumWaypointGroups (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumWaypointGroups") == 0) {
		_scValue->setInt(_waypointGroups.getSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainLayer (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MainLayer") == 0) {
		if (_mainLayer) {
			_scValue->setNative(_mainLayer, true);
		} else {
			_scValue->setNULL();
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFreeNodes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumFreeNodes") == 0) {
		_scValue->setInt(_objects.getSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseX") == 0) {
		int32 viewportX;
		getViewportOffset(&viewportX);

		_scValue->setInt(_game->_mousePos.x + _offsetLeft - viewportX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseY") == 0) {
		int32 viewportY;
		getViewportOffset(nullptr, &viewportY);

		_scValue->setInt(_game->_mousePos.y + _offsetTop - viewportY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoScroll
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoScroll") == 0) {
		_scValue->setBool(_autoScroll);
		return _scValue;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// ShowGeometry
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ShowGeometry") == 0) {
		_scValue->setBool(_showGeometry);
		return _scValue;
	}
#endif

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

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// GeometryFile
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GeometryFile") == 0) {
		if (_geom && _geom->getFilename()) {
			_scValue->setString(_geom->getFilename());
		} else {
			_scValue->setNULL();
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WaypointsHeight
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WaypointsHeight") == 0) {
		if (_geom) {
			_scValue->setFloat(_geom->_waypointHeight);
		} else {
			_scValue->setFloat(0.0f);
		}
		return _scValue;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// Width (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		if (_mainLayer) {
			_scValue->setInt(_mainLayer->_width);
		} else {
			_scValue->setInt(0);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		if (_mainLayer) {
			_scValue->setInt(_mainLayer->_height);
		} else {
			_scValue->setInt(0);
		}
		return _scValue;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// MaxShadowType
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxShadowType") == 0) {
		_scValue->setInt(_maxShadowType);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmbientLightColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmbientLightColor") == 0) {
		_scValue->setInt(_ambientLightColor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumLights
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumLights") == 0) {
		if (!_geom) {
			_scValue->setInt(0);
		} else {
			_scValue->setInt(_geom->_lights.getSize());
		}
		return _scValue;
	}
#endif

	else {
		return BaseObject::scGetProperty(name);
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::scSetProperty(const char *name, ScValue *value) {
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

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// ShowGeometry
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ShowGeometry") == 0) {
		_showGeometry = value->getBool();
		return _scValue;
	}
#endif

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
		if (_scrollTimeH == 0) {
			warning("_scrollTimeH can't be 0, resetting to default");
			_scrollTimeH = 10;
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollSpeedY") == 0) {
		_scrollTimeV = value->getInt();
		if (_scrollTimeV == 0) {
			warning("_scrollTimeV can't be 0, resetting to default");
			_scrollTimeV = 10;
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OffsetX") == 0) {
		_offsetLeft = value->getInt();

		int32 viewportWidth, viewportHeight;
		getViewportSize(&viewportWidth, &viewportHeight);

		_offsetLeft = MAX<int32>(0, _offsetLeft - viewportWidth / 2);
		_offsetLeft = MIN(_offsetLeft, _width - viewportWidth);
		_targetOffsetLeft = _offsetLeft;

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OffsetY") == 0) {
		_offsetTop = value->getInt();

		int32 viewportWidth, viewportHeight;
		getViewportSize(&viewportWidth, &viewportHeight);

		_offsetTop = MAX<int32>(0, _offsetTop - viewportHeight / 2);
		_offsetTop = MIN(_offsetTop, _height - viewportHeight);
		_targetOffsetTop = _offsetTop;

		return STATUS_OK;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// WaypointsHeight
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WaypointsHeight") == 0) {
		if (_geom) {
			_geom->_waypointHeight = value->getFloat();
			_geom->dropWaypoints();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxShadowType
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxShadowType") == 0) {
		setMaxShadowType(static_cast<TShadowType>(value->getInt()));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmbientLightColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmbientLightColor") == 0) {
		_ambientLightColor = value->getInt();
		return STATUS_OK;
	}
#endif

	else {
		return BaseObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *AdScene::scToString() {
	return "[scene object]";
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::addObject(AdObject *object) {
	_objects.add(object);
	return _game->registerObject(object);
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::removeObject(AdObject *object) {
	for (int32 i = 0; i < _objects.getSize(); i++) {
		if (_objects[i] == object) {
			_objects.removeAt(i);
			return _game->unregisterObject(object);
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "SCENE {\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	if (_persistentState) {
		buffer->putTextIndent(indent + 2, "PERSISTENT_STATE=%s\n", _persistentState ? "TRUE" : "FALSE");
	}

	if (!_persistentStateSprites) {
		buffer->putTextIndent(indent + 2, "PERSISTENT_STATE_SPRITES=%s\n", _persistentStateSprites ? "TRUE" : "FALSE");
	}

#ifdef ENABLE_WME3D
	if (_geom) {
		if (_geom->getFilename())
			buffer->putTextIndent(indent + 2, "GEOMETRY=\"%s\"\n", _geom->getFilename());
		if (_geom->_activeCamera >= 0 && _geom->_activeCamera < _geom->_cameras.getSize()) {
			buffer->putTextIndent(indent + 2, "CAMERA=\"%s\"\n", _geom->_cameras[_geom->_activeCamera]->_name);
		}

		if (_fov >= 0.0f)
			buffer->putTextIndent(indent + 2, "FOV_OVERRIDE=%f\n", _fov);

		if (_nearClipPlane >= 0.0f)
			buffer->putTextIndent(indent + 2, "NEAR_CLIPPING_PLANE=%f\n", _nearClipPlane);

		if (_farClipPlane >= 0.0f)
			buffer->putTextIndent(indent + 2, "FAR_CLIPPING_PLANE=%f\n", _farClipPlane);

		if (_showGeometry)
			buffer->putTextIndent(indent + 2, "EDITOR_SHOW_GEOMETRY=%s\n", "TRUE");

		if (_2DPathfinding)
			buffer->putTextIndent(indent + 2, "2D_PATHFINDING=%s\n", "TRUE");

		buffer->putTextIndent(indent + 2, "MAX_SHADOW_TYPE=%d\n", _maxShadowType);
		if (_scroll3DCompatibility)
			buffer->putTextIndent(indent + 2, "SCROLL_3D_COMPATIBILITY=%s\n", "TRUE");

		if (_ambientLightColor != 0x00000000)
			buffer->putTextIndent(indent + 2, "AMBIENT_LIGHT_COLOR { %d,%d,%d }\n", RGBCOLGetR(_ambientLightColor), RGBCOLGetG(_ambientLightColor), RGBCOLGetB(_ambientLightColor));

		buffer->putTextIndent(indent + 2, "WAYPOINT_HEIGHT=%f\n", _geom->_waypointHeight);

		buffer->putTextIndent(indent + 2, "EDITOR_RESOLUTION_WIDTH=%d\n", _editorResolutionWidth);
		buffer->putTextIndent(indent + 2, "EDITOR_RESOLUTION_HEIGHT=%d\n", _editorResolutionHeight);

		buffer->putTextIndent(indent + 2, "\n");
	}
#endif

	// scripts
	for (int32 i = 0; i < _scripts.getSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// properties
	if (_scProp) {
		_scProp->saveAsText(buffer, indent + 2);
	}

	// viewport
	if (_viewport) {
		Common::Rect32 *rc = _viewport->getRect();
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

	BaseClass::saveAsText(buffer, indent + 2);

	// waypoints
	buffer->putTextIndent(indent + 2, "; ----- waypoints\n");
	for (int32 i = 0; i < _waypointGroups.getSize(); i++) {
		_waypointGroups[i]->saveAsText(buffer, indent + 2);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// layers
	buffer->putTextIndent(indent + 2, "; ----- layers\n");
	for (int32 i = 0; i < _layers.getSize(); i++) {
		_layers[i]->saveAsText(buffer, indent + 2);
	}

	// scale levels
	buffer->putTextIndent(indent + 2, "; ----- scale levels\n");
	for (int32 i = 0; i < _scaleLevels.getSize(); i++) {
		_scaleLevels[i]->saveAsText(buffer, indent + 2);
	}

	// rotation levels
	buffer->putTextIndent(indent + 2, "; ----- rotation levels\n");
	for (int32 i = 0; i < _rotLevels.getSize(); i++) {
		_rotLevels[i]->saveAsText(buffer, indent + 2);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// free entities
	buffer->putTextIndent(indent + 2, "; ----- free entities\n");
	for (int32 i = 0; i < _objects.getSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY) {
			_objects[i]->saveAsText(buffer, indent + 2);
		}
	}

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::sortScaleLevels() {

	if (_scaleLevels.getSize() == 0) {
		return STATUS_OK;
	}

	bool changed;
	do {
		changed = false;
		for (int32 i = 0; i < _scaleLevels.getSize() - 1; i++) {
			if (_scaleLevels[i]->_posY > _scaleLevels[i + 1]->_posY) {
				AdScaleLevel *sl = _scaleLevels[i];
				_scaleLevels[i] = _scaleLevels[i + 1];
				_scaleLevels[i + 1] = sl;

				changed = true;
			}
		}

	} while (changed);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::sortRotLevels() {

	if (_rotLevels.getSize() == 0) {
		return STATUS_OK;
	}

	bool changed;
	do {
		changed = false;
		for (int32 i = 0; i < _rotLevels.getSize() - 1; i++) {
			if (_rotLevels[i]->_posX > _rotLevels[i + 1]->_posX) {
				AdRotLevel *rl = _rotLevels[i];
				_rotLevels[i] = _rotLevels[i + 1];
				_rotLevels[i + 1] = rl;

				changed = true;
			}
		}

	} while (changed);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float AdScene::getScaleAt(int y) {
	AdScaleLevel *prev = nullptr;
	AdScaleLevel *next = nullptr;

	for (int32 i = 0; i < _scaleLevels.getSize(); i++) {
		/* AdScaleLevel *xxx = _scaleLevels[i];*/
		/* int j = _scaleLevels.size(); */
		if (_scaleLevels[i]->_posY < y) {
			prev = _scaleLevels[i];
		} else {
			next = _scaleLevels[i];
			break;
		}
	}

	if (prev == nullptr || next == nullptr) {
		return 100;
	}

	int delta_y = next->_posY - prev->_posY;
	float delta_scale = next->_scale - prev->_scale;
	y -= prev->_posY;

	float percent = (float)y / ((float)delta_y / 100.0f);
	return prev->_scale + delta_scale / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transferBool(TMEMBER(_autoScroll));
	persistMgr->transferUint32(TMEMBER(_editorColBlocked));
	persistMgr->transferUint32(TMEMBER(_editorColBlockedSel));
	persistMgr->transferUint32(TMEMBER(_editorColDecor));
	persistMgr->transferUint32(TMEMBER(_editorColDecorSel));
	persistMgr->transferUint32(TMEMBER(_editorColEntity));
	persistMgr->transferUint32(TMEMBER(_editorColEntitySel));
	persistMgr->transferUint32(TMEMBER(_editorColFrame));
	persistMgr->transferUint32(TMEMBER(_editorColRegion));
	persistMgr->transferUint32(TMEMBER(_editorColRegionSel));
	persistMgr->transferUint32(TMEMBER(_editorColScale));
	persistMgr->transferUint32(TMEMBER(_editorColWaypoints));
	persistMgr->transferUint32(TMEMBER(_editorColWaypointsSel));
	persistMgr->transferSint32(TMEMBER(_editorMarginH));
	persistMgr->transferSint32(TMEMBER(_editorMarginV));
	persistMgr->transferBool(TMEMBER(_editorShowBlocked));
	persistMgr->transferBool(TMEMBER(_editorShowDecor));
	persistMgr->transferBool(TMEMBER(_editorShowEntities));
	persistMgr->transferBool(TMEMBER(_editorShowRegions));
	persistMgr->transferBool(TMEMBER(_editorShowScale));
	persistMgr->transferPtr(TMEMBER_PTR(_fader));
#ifdef ENABLE_WME3D
	if (BaseEngine::instance().getFlags() & GF_3D) {
		persistMgr->transferPtr(TMEMBER(_geom));
	} else {
		_geom = nullptr;
	}
#endif
	persistMgr->transferSint32(TMEMBER(_height));
	persistMgr->transferBool(TMEMBER(_initialized));
	persistMgr->transferUint32(TMEMBER(_lastTimeH));
	persistMgr->transferUint32(TMEMBER(_lastTimeV));
	_layers.persist(persistMgr);
	persistMgr->transferPtr(TMEMBER_PTR(_mainLayer));
	_objects.persist(persistMgr);
	persistMgr->transferSint32(TMEMBER(_offsetLeft));
	persistMgr->transferSint32(TMEMBER(_offsetTop));
	persistMgr->transferBool(TMEMBER(_paralaxScrolling));
	persistMgr->transferBool(TMEMBER(_persistentState));
	persistMgr->transferBool(TMEMBER(_persistentStateSprites));
	persistMgr->transferUint32(TMEMBER(_pfMaxTime));
	_pfPath.persist(persistMgr);
	persistMgr->transferSint32(TMEMBER(_pfPointsNum));
	persistMgr->transferBool(TMEMBER(_pfReady));
	persistMgr->transferPtr(TMEMBER_PTR(_pfRequester));
	persistMgr->transferPtr(TMEMBER_PTR(_pfTarget));
	persistMgr->transferPtr(TMEMBER_PTR(_pfTargetPath));
	_rotLevels.persist(persistMgr);
	_scaleLevels.persist(persistMgr);
	persistMgr->transferSint32(TMEMBER(_scrollPixelsH));
	persistMgr->transferSint32(TMEMBER(_scrollPixelsV));
	persistMgr->transferUint32(TMEMBER(_scrollTimeH));
	persistMgr->transferUint32(TMEMBER(_scrollTimeV));
	persistMgr->transferPtr(TMEMBER_PTR(_shieldWindow));
#ifdef ENABLE_WME3D
	if (BaseEngine::instance().getFlags() & GF_3D) {
		persistMgr->transferBool(TMEMBER(_showGeometry));
	} else {
		_showGeometry = false;
	}
#endif
	persistMgr->transferSint32(TMEMBER(_targetOffsetLeft));
	persistMgr->transferSint32(TMEMBER(_targetOffsetTop));
	_waypointGroups.persist(persistMgr);
	persistMgr->transferPtr(TMEMBER_PTR(_viewport));
	persistMgr->transferSint32(TMEMBER(_width));

#ifdef ENABLE_WME3D
	if (BaseEngine::instance().getFlags() & GF_3D) {
		persistMgr->transferSint32(TMEMBER(_editorResolutionWidth));
		persistMgr->transferSint32(TMEMBER(_editorResolutionHeight));

		persistMgr->transferFloat(TMEMBER(_fov));
		persistMgr->transferFloat(TMEMBER(_nearClipPlane));
		persistMgr->transferFloat(TMEMBER(_farClipPlane));

		persistMgr->transferBool(TMEMBER(_2DPathfinding));
		persistMgr->transferSint32(TMEMBER_INT(_maxShadowType));

		persistMgr->transferBool(TMEMBER(_scroll3DCompatibility));

		persistMgr->transferUint32(TMEMBER(_ambientLightColor));

		persistMgr->transferBool(TMEMBER(_fogEnabled));
		persistMgr->transferUint32(TMEMBER(_fogColor));
		persistMgr->transferFloat(TMEMBER(_fogStart));
		persistMgr->transferFloat(TMEMBER(_fogEnd));
	} else {
		_editorResolutionWidth = _editorResolutionHeight = 0;
		_fov = _nearClipPlane = _farClipPlane = -1.0f;
		_2DPathfinding = false;
		_maxShadowType = SHADOW_SIMPLE;
		_scroll3DCompatibility = false;
		_ambientLightColor = 0x00000000;
		_fogEnabled = false;
		_fogColor = 0x00FFFFFF;
		_fogStart = 0.0f;
		_fogEnd = 0.0f;
	}
#endif

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::afterLoad() {
#ifdef ENABLE_WME3D
	if (_geom && _geom->_activeCamera >= 0 && _geom->_activeCamera < _geom->_cameras.getSize()) {
		_geom->setActiveCamera(_geom->_activeCamera, _fov, _nearClipPlane, _farClipPlane);
	}
#endif
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::correctTargetPoint2(int32 startX, int32 startY, int32 *targetX, int32 *targetY, bool checkFreeObjects, BaseObject *requester) {
	double xStep, yStep, x, y;
	int32 xLength, yLength, xCount, yCount;
	int32 x1, y1, x2, y2;

	x1 = *targetX;
	y1 = *targetY;
	x2 = startX;
	y2 = startY;

	xLength = abs(x2 - x1);
	yLength = abs(y2 - y1);

	if (xLength > yLength) {

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
bool AdScene::correctTargetPoint(int32 startX, int32 startY, int32 *argX, int32 *argY, bool checkFreeObjects, BaseObject *requester) {
	int32 x = *argX;
	int32 y = *argY;

	if (isWalkableAt(x, y, checkFreeObjects, requester) || !_mainLayer) {
		return STATUS_OK;
	}

	// right
	int lengthRight = 0;
	bool foundRight = false;
	for (x = *argX, y = *argY; x < _mainLayer->_width; x++, lengthRight++) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x - 5, y, checkFreeObjects, requester)) {
			foundRight = true;
			break;
		}
	}

	// left
	int lengthLeft = 0;
	bool foundLeft = false;
	for (x = *argX, y = *argY; x >= 0; x--, lengthLeft--) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x + 5, y, checkFreeObjects, requester)) {
			foundLeft = true;
			break;
		}
	}

	// up
	int lengthUp = 0;
	bool foundUp = false;
	for (x = *argX, y = *argY; y >= 0; y--, lengthUp--) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x, y + 5, checkFreeObjects, requester)) {
			foundUp = true;
			break;
		}
	}

	// down
	int lengthDown = 0;
	bool foundDown = false;
	for (x = *argX, y = *argY; y < _mainLayer->_height; y++, lengthDown++) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x, y - 5, checkFreeObjects, requester)) {
			foundDown = true;
			break;
		}
	}

	if (!foundLeft && !foundRight && !foundUp && !foundDown) {
		return STATUS_OK;
	}

	int offsetX = INT_MAX_VALUE, offsetY = INT_MAX_VALUE;

	if (foundLeft && foundRight) {
		if (abs(lengthLeft) < abs(lengthRight)) {
			offsetX = lengthLeft;
		} else {
			offsetX = lengthRight;
		}
	} else if (foundLeft) {
		offsetX = lengthLeft;
	} else if (foundRight) {
		offsetX = lengthRight;
	}

	if (foundUp && foundDown) {
		if (abs(lengthUp) < abs(lengthDown)) {
			offsetY = lengthUp;
		} else {
			offsetY = lengthDown;
		}
	} else if (foundUp) {
		offsetY = lengthUp;
	} else if (foundDown) {
		offsetY = lengthDown;
	}

	if (abs(offsetX) < abs(offsetY)) {
		*argX = *argX + offsetX;
	} else {
		*argY = *argY + offsetY;
	}

	if (!isWalkableAt(*argX, *argY)) {
		return correctTargetPoint2(startX, startY, argX, argY, checkFreeObjects, requester);
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
void AdScene::pfPointsStart() {
	_pfPointsNum = 0;
}


//////////////////////////////////////////////////////////////////////////
void AdScene::pfPointsAdd(int x, int y, int distance) {
	if (_pfPointsNum >= _pfPath.getSize()) {
		_pfPath.add(new AdPathPoint(x, y, distance));
	} else {
		_pfPath[_pfPointsNum]->x = x;
		_pfPath[_pfPointsNum]->y = y;
		_pfPath[_pfPointsNum]->_distance = distance;
		_pfPath[_pfPointsNum]->_marked = false;
		_pfPath[_pfPointsNum]->_origin = nullptr;
	}

	_pfPointsNum++;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getViewportOffset(int32 *offsetX, int32 *offsetY) {
	AdGame *adGame = (AdGame *)_game;
	if (_viewport && !_game->_editorMode) {
		if (offsetX) {
			*offsetX = _viewport->_offsetX;
		}
		if (offsetY) {
			*offsetY = _viewport->_offsetY;
		}
	} else if (adGame->_sceneViewport && !_game->_editorMode) {
		if (offsetX) {
			*offsetX = adGame->_sceneViewport->_offsetX;
		}
		if (offsetY) {
			*offsetY = adGame->_sceneViewport->_offsetY;
		}
	} else {
		if (offsetX) {
			*offsetX = 0;
		}
		if (offsetY) {
			*offsetY = 0;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getViewportSize(int32 *width, int32 *height) {
	AdGame *adGame = (AdGame *)_game;
	if (_viewport && !_game->_editorMode) {
		if (width) {
			*width  = _viewport->getWidth();
		}
		if (height) {
			*height = _viewport->getHeight();
		}
	} else if (adGame->_sceneViewport && !_game->_editorMode) {
		if (width) {
			*width  = adGame->_sceneViewport->getWidth();
		}
		if (height) {
			*height = adGame->_sceneViewport->getHeight();
		}
	} else {
		if (width) {
			*width  = _game->_renderer->getWidth();
		}
		if (height) {
			*height = _game->_renderer->getHeight();
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int AdScene::getOffsetLeft() {
	int32 viewportX;
	getViewportOffset(&viewportX);

	return _offsetLeft - viewportX;
}


//////////////////////////////////////////////////////////////////////////
int AdScene::getOffsetTop() {
	int32 viewportY;
	getViewportOffset(nullptr, &viewportY);

	return _offsetTop - viewportY;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::pointInViewport(int x, int y) {
	int32 left, top, width, height;

	getViewportOffset(&left, &top);
	getViewportSize(&width, &height);

	return x >= left && x <= left + width && y >= top && y <= top + height;
}


//////////////////////////////////////////////////////////////////////////
void AdScene::setOffset(int offsetLeft, int offsetTop) {
	_offsetLeft = offsetLeft;
	_offsetTop  = offsetTop;
}


//////////////////////////////////////////////////////////////////////////
BaseObject *AdScene::getNodeByName(const char *name) {
	BaseObject *ret = nullptr;

	// dependent objects
	for (int32 i = 0; i < _layers.getSize(); i++) {
		AdLayer *layer = _layers[i];
		for (int32 j = 0; j < layer->_nodes.getSize(); j++) {
			AdSceneNode *node = layer->_nodes[j];
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
					ret = nullptr;
				}
				return ret;
			}
		}
	}

	// free entities
	for (int32 i = 0; i < _objects.getSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY && !scumm_stricmp(name, _objects[i]->_name)) {
			return _objects[i];
		}
	}

	// waypoint groups
	for (int32 i = 0; i < _waypointGroups.getSize(); i++) {
		if (!scumm_stricmp(name, _waypointGroups[i]->_name)) {
			return _waypointGroups[i];
		}
	}

	return nullptr;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::saveState() {
	return persistState(true);
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::loadState() {
	return persistState(false);
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::persistState(bool saving) {
	if (!_persistentState) {
		return STATUS_OK;
	}

	AdGame *adGame = (AdGame *)_game;
	AdSceneState *state = adGame->getSceneState(getFilename(), saving);
	if (!state) {
		return STATUS_OK;
	}

	AdNodeState *nodeState;

	// dependent objects
	for (int32 i = 0; i < _layers.getSize(); i++) {
		AdLayer *layer = _layers[i];
		for (int32 j = 0; j < layer->_nodes.getSize(); j++) {
			AdSceneNode *node = layer->_nodes[j];
			switch (node->_type) {
			case OBJECT_ENTITY:
				if (!node->_entity->_saveState) {
					continue;
				}
				nodeState = state->getNodeState(node->_entity->_name, saving);
				if (nodeState) {
					nodeState->transferEntity(node->_entity, _persistentStateSprites, saving);
					//if (Saving) NodeState->_active = node->_entity->_active;
					//else node->_entity->_active = NodeState->_active;
				}
				break;
			case OBJECT_REGION:
				if (!node->_region->_saveState) {
					continue;
				}
				nodeState = state->getNodeState(node->_region->_name, saving);
				if (nodeState) {
					if (saving) {
						nodeState->_active = node->_region->_active;
					} else {
						node->_region->_active = nodeState->_active;
					}
				}
				break;
			default:
				warning("AdScene::PersistState - unhandled enum");
				break;
			}
		}
	}

	// free entities
	for (int32 i = 0; i < _objects.getSize(); i++) {
		if (!_objects[i]->_saveState) {
			continue;
		}
		if (_objects[i]->_type == OBJECT_ENTITY) {
			nodeState = state->getNodeState(_objects[i]->_name, saving);
			if (nodeState) {
				nodeState->transferEntity((AdEntity *)_objects[i], _persistentStateSprites, saving);
				//if (Saving) NodeState->_active = _objects[i]->_active;
				//else _objects[i]->_active = NodeState->_active;
			}
		}
	}

	// waypoint groups
	for (int32 i = 0; i < _waypointGroups.getSize(); i++) {
		nodeState = state->getNodeState(_waypointGroups[i]->_name, saving);
		if (nodeState) {
			if (saving) {
				nodeState->_active = _waypointGroups[i]->_active;
			} else {
				_waypointGroups[i]->_active = nodeState->_active;
			}
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float AdScene::getRotationAt(int x, int y) {
	AdRotLevel *prev = nullptr;
	AdRotLevel *next = nullptr;

	for (int32 i = 0; i < _rotLevels.getSize(); i++) {
		/*  AdRotLevel *xxx = _rotLevels[i];
		    int j = _rotLevels.size();*/
		if (_rotLevels[i]->_posX < x) {
			prev = _rotLevels[i];
		} else {
			next = _rotLevels[i];
			break;
		}
	}

	if (prev == nullptr || next == nullptr) {
		return 0;
	}

	int delta_x = next->_posX - prev->_posX;
	float delta_rot = next->_rotation - prev->_rotation;
	x -= prev->_posX;

	float percent = (float)x / ((float)delta_x / 100.0f);
	return prev->_rotation + delta_rot / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::handleItemAssociations(const char *itemName, bool show) {
	for (int32 i = 0; i < _layers.getSize(); i++) {
		AdLayer *layer = _layers[i];
		for (int32 j = 0; j < layer->_nodes.getSize(); j++) {
			if (layer->_nodes[j]->_type == OBJECT_ENTITY) {
				AdEntity *ent = layer->_nodes[j]->_entity;

				if (ent->_item && strcmp(ent->_item, itemName) == 0) {
					ent->_active = show;
				}
			}
		}
	}

	for (int32 i = 0; i < _objects.getSize(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY) {
			AdEntity *ent = (AdEntity *)_objects[i];
			if (ent->_item && strcmp(ent->_item, itemName) == 0) {
				ent->_active = show;
			}
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getRegionsAt(int x, int y, AdRegion **regionList, int numRegions) {
	int numUsed = 0;
	if (_mainLayer) {
		for (int32 i = _mainLayer->_nodes.getSize() - 1; i >= 0; i--) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && node->_region->pointInRegion(x, y)) {
				if (numUsed < numRegions - 1) {
					regionList[numUsed] = node->_region;
					numUsed++;
				} else {
					break;
				}
			}
		}
	}
	for (int32 i = numUsed; i < numRegions; i++) {
		regionList[i] = nullptr;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::restoreDeviceObjects() {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
BaseObject *AdScene::getNextAccessObject(BaseObject *currObject) {
	BaseArray<AdObject *> objects;
	getSceneObjects(objects, true);

	if (objects.getSize() == 0) {
		return nullptr;
	} else {
		if (currObject != nullptr) {
			for (int32 i = 0; i < objects.getSize(); i++) {
				if (objects[i] == currObject) {
					if (i < (int32)objects.getSize() - 1) {
						return objects[i + 1];
					} else {
						break;
					}
				}
			}
		}
		return objects[0];
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
BaseObject *AdScene::getPrevAccessObject(BaseObject *currObject) {
	BaseArray<AdObject *> objects;
	getSceneObjects(objects, true);

	if (objects.getSize() == 0) {
		return nullptr;
	} else {
		if (currObject != nullptr) {
			for (int32 i = objects.getSize() - 1; i >= 0; i--) {
				if (objects[i] == currObject) {
					if (i > 0) {
						return objects[i - 1];
					} else {
						break;
					}
				}
			}
		}
		return objects[objects.getSize() - 1];
	}
	return nullptr;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getSceneObjects(BaseArray<AdObject *> &objects, bool interactiveOnly) {
	for (int32 i = 0; i < _layers.getSize(); i++) {
		// close-up layer -> remove everything below it
		if (interactiveOnly && _layers[i]->_closeUp) {
			objects.removeAll();
		}


		for (int32 j = 0; j < _layers[i]->_nodes.getSize(); j++) {
			AdSceneNode *node = _layers[i]->_nodes[j];
			switch (node->_type) {
			case OBJECT_ENTITY: {
				AdEntity *ent = node->_entity;
				if (ent->_active && (ent->_registrable || !interactiveOnly)) {
					objects.add(ent);
				}
			}
			break;

			case OBJECT_REGION: {
				BaseArray<AdObject *> regionObj;
				getRegionObjects(node->_region, regionObj, interactiveOnly);
				for (int32 newIndex = 0; newIndex < regionObj.getSize(); newIndex++) {
					bool found = false;
					for (int32 old = 0; old < objects.getSize(); old++) {
						if (objects[old] == regionObj[newIndex]) {
							found = true;
							break;
						}
					}
					if (!found) {
						objects.add(regionObj[newIndex]);
					}
				}
				//if (regionObj.size() > 0) Objects.Append(RegionObj);
			}
			break;

			default:
				debugC(kWintermuteDebugGeneral, "AdScene::GetSceneObjects - Unhandled enum");
				break;
			}
		}
	}

	// objects outside any region
	BaseArray<AdObject *> regionObj;
	getRegionObjects(NULL, regionObj, interactiveOnly);
	for (int32 newIndex = 0; newIndex < regionObj.getSize(); newIndex++) {
		bool found = false;
		for (int32 old = 0; old < objects.getSize(); old++) {
			if (objects[old] == regionObj[newIndex]) {
				found = true;
				break;
			}
		}
		if (!found) {
			objects.add(regionObj[newIndex]);
		}
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getRegionObjects(AdRegion *region, BaseArray<AdObject *> &objects, bool interactiveOnly) {
	AdGame *adGame = (AdGame *)_game;
	AdObject *obj;

	// global objects
	for (int32 i = 0; i < adGame->_objects.getSize(); i++) {
		obj = adGame->_objects[i];
		if (obj->_active && (obj->_stickRegion == region || region == nullptr || (obj->_stickRegion == nullptr && region->pointInRegion(obj->_posX, obj->_posY)))) {
			if (interactiveOnly && !obj->_registrable) {
				continue;
			}

			objects.add(obj);
		}
	}

	// scene objects
	for (int32 i = 0; i < _objects.getSize(); i++) {
		obj = _objects[i];
		if (obj->_active && !obj->_editorOnly && (obj->_stickRegion == region || region == nullptr || (obj->_stickRegion == nullptr && region->pointInRegion(obj->_posX, obj->_posY)))) {
			if (interactiveOnly && !obj->_registrable) {
				continue;
			}

			objects.add(obj);
		}
	}

	// sort by _posY
	qsort_msvc(objects.getData(), objects.getSize(), sizeof(AdObject *), AdScene::compareObjs);

	return STATUS_OK;
}

#ifdef ENABLE_WME3D
//////////////////////////////////////////////////////////////////////////
void AdScene::setMaxShadowType(TShadowType shadowType) {
	if (shadowType > SHADOW_STENCIL) {
		shadowType = SHADOW_STENCIL;
	}

	if (shadowType < 0) {
		shadowType = SHADOW_NONE;
	}

	_maxShadowType = shadowType;
}
#endif

//////////////////////////////////////////////////////////////////////////
void AdScene::onLayerResized(AdLayer *layer) {
	if (layer != nullptr && layer == _mainLayer) {
		_width = layer->_width;
		_height = layer->_height;
	}
}

Common::String AdScene::debuggerToString() const {
	return Common::String::format("%p: Scene \"%s\", paralax: %d, autoscroll: %d", (const void *)this, _name, _paralaxScrolling, _autoScroll);
}

} // End of namespace Wintermute
