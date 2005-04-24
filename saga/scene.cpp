/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Scene management module
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/animation.h"
#include "saga/console.h"
#include "saga/interface.h"
#include "saga/events.h"
#include "saga/isomap.h"
#include "saga/objectmap.h"
#include "saga/palanim.h"
#include "saga/render.h"
#include "saga/rscfile_mod.h"
#include "saga/script.h"
#include "saga/text.h"
#include "saga/sound.h"
#include "saga/music.h"

#include "saga/scene.h"
#include "saga/stream.h"
#include "saga/actor.h"

namespace Saga {

static int initSceneDoors[SCENE_DOORS_MAX] = {
0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff 
};

Scene::Scene(SagaEngine *vm) : _vm(vm), _initialized(false) {
	byte *scene_lut_p;
	size_t scene_lut_len;
	int result;
	int i;


	// Load scene module resource context
	_sceneContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (_sceneContext == NULL) {
		warning("Scene::Scene(): Couldn't load scene resource context");
		return;
	}


	// Load scene lookup table
	debug(0, "Loading scene LUT from resource %u.", RSC_ConvertID(_vm->getResourceDescription()->scene_lut_rn));
	result = RSC_LoadResource(_sceneContext, RSC_ConvertID(_vm->getResourceDescription()->scene_lut_rn), &scene_lut_p, &scene_lut_len);
	if (result != SUCCESS) {
		warning("Scene::Scene(): Error: couldn't load scene LUT");
		return;
	}
	if (scene_lut_len==0) {
		warning("Scene::Scene(): scene_lut_len==0");
		return;
	}
	_sceneCount = scene_lut_len / 2;
	_sceneMax = _sceneCount - 1;
	_sceneLUT = (int *)malloc(_sceneMax * sizeof(*_sceneLUT));
	if (_sceneLUT == NULL) {
		memoryError("Scene::Scene()");
	}

	MemoryReadStreamEndian readS(scene_lut_p, scene_lut_len, IS_BIG_ENDIAN);

	for (i = 0; i < _sceneMax; i++) {
		_sceneLUT[i] = readS.readUint16();
	}

	free(scene_lut_p);

	_firstScene = _vm->getStartSceneNumber();

	debug(0, "First scene set to %d.", _firstScene);

	debug(0, "LUT has %d entries.", _sceneMax);

	// Create scene module text list
	_textList = _vm->textCreateList();

	if (_textList == NULL) {
		warning("Scene::Scene(): Error: Couldn't create scene text list");
		return;
	}

	_sceneLoaded = false;
	_sceneNumber = 0;
	_sceneResourceId = 0;
	_inGame = false;
	_loadDesc = false;
	memset(&_desc, 0, sizeof(_desc));
	_resListEntries = 0;
	_resList = NULL;
	_animEntries = 0;
	_sceneProc = NULL;
	_objectMap = new ObjectMap(_vm);
	_actionMap = new ObjectMap(_vm);
	memset(&_bg, 0, sizeof(_bg));
	memset(&_bgMask, 0, sizeof(_bgMask));

	_initialized = true;
}

Scene::~Scene() {
	if (_initialized) {
		endScene();
		delete _actionMap;
		free(_sceneLUT);
	}
}

int Scene::queueScene(SCENE_QUEUE *scene_queue) {
	assert(_initialized);
	assert(scene_queue != NULL);

	_sceneQueue.push_back(*scene_queue);
	return SUCCESS;
}

int Scene::clearSceneQueue() {
	assert(_initialized);

	_sceneQueue.clear();

	return SUCCESS;
}

int Scene::startScene() {
	SceneQueueList::iterator queueIterator;
	SCENE_QUEUE *scene_qdat;
	EVENT event;

	assert(_initialized);

	if (_sceneLoaded) {
		warning("Scene::start(): Error: Can't start game...scene already loaded");
		return FAILURE;
	}

	if (_inGame) {
		warning("Scene::start(): Error: Can't start game...game already started");
		return FAILURE;
	}

	// Hide cursor during intro
	event.type = ONESHOT_EVENT;
	event.code = CURSOR_EVENT;
	event.op = EVENT_HIDE;
	_vm->_events->queue(&event);

	switch (_vm->getGameType()) {
	case GType_ITE:
		ITEStartProc();
		break;
	case GType_IHNM:
		IHNMStartProc();
		break;
	default:
		warning("Scene::start(): Error: Can't start game... gametype not supported");
		break;
	}

	// Load the head in scene queue
	queueIterator = _sceneQueue.begin();
	if (queueIterator == _sceneQueue.end()) {
		return SUCCESS;
	}

	scene_qdat = queueIterator.operator->();
	assert(scene_qdat != NULL);

	loadScene(scene_qdat->scene_n, scene_qdat->load_flag, scene_qdat->scene_proc, scene_qdat->sceneDescription, scene_qdat->fadeType, 0);

	return SUCCESS;
}

int Scene::nextScene() {
	SceneQueueList::iterator queueIterator;
	SCENE_QUEUE *scene_qdat;

	assert(_initialized);

	if (!_sceneLoaded) {
		warning("Scene::next(): Error: Can't advance scene...no scene loaded");
		return FAILURE;
	}

	if (_inGame) {
		warning("Scene::next(): Error: Can't advance scene...game already started");
		return FAILURE;
	}

	endScene();

	// Delete the current head  in scene queue
	queueIterator = _sceneQueue.begin();
	if (queueIterator == _sceneQueue.end()) {
		return SUCCESS;
	}
	
	queueIterator = _sceneQueue.erase(queueIterator);

	if (queueIterator == _sceneQueue.end()) {
		return SUCCESS;
	}

	// Load the head  in scene queue
	scene_qdat = queueIterator.operator->();
	assert(scene_qdat != NULL);

	loadScene(scene_qdat->scene_n, scene_qdat->load_flag, scene_qdat->scene_proc, scene_qdat->sceneDescription, scene_qdat->fadeType, 0);

	return SUCCESS;
}

int Scene::skipScene() {
	SceneQueueList::iterator queueIterator;

	SCENE_QUEUE *scene_qdat = NULL;
	SCENE_QUEUE *skip_qdat = NULL;

	assert(_initialized);

	if (!_sceneLoaded) {
		warning("Scene::skip(): Error: Can't skip scene...no scene loaded");
		return FAILURE;
	}

	if (_inGame) {
		warning("Scene::skip(): Error: Can't skip scene...game already started");
		return FAILURE;
	}

	// Walk down scene queue and try to find a skip target
	queueIterator = _sceneQueue.begin();
	if (queueIterator == _sceneQueue.end()) {
		warning("Scene::skip(): Error: Can't skip scene...no scenes in queue");
		return FAILURE;
	}

	++queueIterator;
	while (queueIterator != _sceneQueue.end()) {
		scene_qdat = queueIterator.operator->();
		assert(scene_qdat != NULL);

		if (scene_qdat->scene_skiptarget) {
			skip_qdat = scene_qdat;
			break;
		}
		++queueIterator;
	}

	// If skip target found, remove preceding scenes and load
	if (skip_qdat != NULL) {
		_sceneQueue.erase(_sceneQueue.begin(), queueIterator);

		endScene();
		loadScene(skip_qdat->scene_n, skip_qdat->load_flag, skip_qdat->scene_proc, skip_qdat->sceneDescription, skip_qdat->fadeType, 0);
	}
	// Search for a scene to skip to

	return SUCCESS;
}

int Scene::changeScene(int sceneNumber, int actorsEntrance, int fadeIn) {
	assert(_initialized);

	if (!_sceneLoaded) {
		warning("Scene::changeScene(): Error: Can't change scene. No scene currently loaded. Game in invalid state");
		return FAILURE;
	}

	if ((sceneNumber < 0) || (sceneNumber > _sceneMax)) {
		warning("Scene::changeScene(): Error: Can't change scene. Invalid scene number");
		return FAILURE;
	}

	if (_sceneLUT[sceneNumber] == 0) {
		warning("Scene::changeScene(): Error: Can't change scene; invalid scene descriptor resource number (0)");
		return FAILURE;
	}

	endScene();
	loadScene(sceneNumber, BY_SCENE, NULL, NULL, fadeIn, actorsEntrance);

	return SUCCESS;
}

void Scene::getSlopes(int &beginSlope, int &endSlope) {
	beginSlope = _vm->getSceneHeight() - _desc.beginSlope; 
	endSlope = _vm->getSceneHeight() - _desc.endSlope;
}

int Scene::getBGInfo(SCENE_BGINFO *bginfo) {
	int x, y;

	assert(_initialized);

	bginfo->bg_buf = _bg.buf;
	bginfo->bg_buflen = _bg.buf_len;
	bginfo->bg_w = _bg.w;
	bginfo->bg_h = _bg.h;
	bginfo->bg_p = _bg.p;

	x = 0;
	y = 0;

	if (_bg.w < _vm->getDisplayWidth()) {
		x = (_vm->getDisplayWidth() - _bg.w) / 2;
	}

	if (_bg.h < _vm->getSceneHeight()) {
		y = (_vm->getSceneHeight() - _bg.h) / 2;
	}

	bginfo->bg_x = x;
	bginfo->bg_y = y;

	return SUCCESS;
}

int Scene::getBGPal(PALENTRY **pal) {
	assert(_initialized);
	*pal = _bg.pal;

	return SUCCESS;
}

int Scene::getBGMaskType(const Point &testPoint) {
	uint offset;
	if (!_bgMask.loaded) {
		return 0;
	}
	offset = testPoint.x + testPoint.y * _bgMask.w;
	if (offset >= _bgMask.buf_len) {
		error("Scene::getBGMaskType offset 0x%X exceed bufferLength 0x%X", offset, _bgMask.buf_len);
	}

	return (_bgMask.buf[offset] >> 4) & 0x0f;
}

bool Scene::validBGMaskPoint(const Point &testPoint) {
	if (!_bgMask.loaded) {
		error("Scene::validBGMaskPoint _bgMask not loaded");
	}

	return !((testPoint.x < 0) || (testPoint.x >= _bgMask.w) ||
		(testPoint.y < 0) || (testPoint.y >= _bgMask.h));
}

bool Scene::canWalk(const Point &testPoint) {
	int maskType;

	if (!_bgMask.loaded) {
		return true;
	}
	if (!validBGMaskPoint(testPoint)) {
		return true;
	}

	maskType = getBGMaskType(testPoint);
	return getDoorState(maskType) == 0;
}

bool Scene::offscreenPath(Point &testPoint) {
	Point point;

	if (!_bgMask.loaded) {
		return false;
	}

	point.x = clamp( 0, testPoint.x, _bgMask.w - 1 );
	point.y = clamp( 0, testPoint.y, _bgMask.h - 1 );
	if (point == testPoint) {
		return false;
	}

	if (point.y >= _bgMask.h - 1) {
		point.y = _bgMask.h - 2;
	}
	testPoint = point;

	return true;
}


void Scene::getBGMaskInfo(int &width, int &height, byte *&buffer, size_t &bufferLength) {
	if (!_bgMask.loaded) {
		error("Scene::getBGMaskInfo _bgMask not loaded");
	}

	width = _bgMask.w;
	height = _bgMask.h;
	buffer = _bgMask.buf;
	bufferLength = _bgMask.buf_len;
}

void Scene::setDoorState(int doorNumber, int doorState) {
	if ((doorNumber < 0) || (doorNumber >= SCENE_DOORS_MAX))
		error("Scene::setDoorState wrong doorNumber");

	_sceneDoors[doorNumber] = doorState;
}

int Scene::getDoorState(int doorNumber) {
	if ((doorNumber < 0) || (doorNumber >= SCENE_DOORS_MAX))
		error("Scene::getDoorState wrong doorNumber");

	return _sceneDoors[doorNumber];
}

void Scene::initDoorsState() {
	memcpy(_sceneDoors, initSceneDoors, sizeof (_sceneDoors) );
}

int Scene::getInfo(SCENE_INFO *si) {
	assert(_initialized);
	assert(si != NULL);

	si->text_list = _textList;

	return SUCCESS;
}

int Scene::getSceneLUT(int scene_num) { 
	assert((scene_num > 0) && (scene_num < _sceneMax));

	return _sceneLUT[scene_num];
}

int Scene::loadScene(int scene_num, int load_flag, SCENE_PROC scene_proc, SceneDescription *scene_desc_param, int fadeType, int actorsEntrance) {
	SCENE_INFO scene_info;
	uint32 resourceId = 0;
	int result;
	int i;
	EVENT event;
	EVENT *q_event;
	static PALENTRY current_pal[PAL_ENTRIES];

	assert(_initialized);

	if (_sceneLoaded) {
		warning("Scene::loadScene(): Error, a scene is already loaded");
		return FAILURE;
	}

	_loadDesc = true;
	_sceneNumber = -1;

	switch (load_flag) {
	case BY_RESOURCE:
		resourceId = scene_num;
		break;
	case BY_SCENE:
		assert((scene_num > 0) && (scene_num < _sceneMax));
		resourceId = _sceneLUT[scene_num];
		_sceneNumber = scene_num;
		break;
	case BY_DESC:
		assert(scene_desc_param != NULL);
		assert(scene_desc_param->resList != NULL);
		_loadDesc = false;
		_desc = *scene_desc_param;
		_resList = scene_desc_param->resList;
		_resListEntries = scene_desc_param->resListCnt;
		break;
	default:
		warning("Scene::loadScene(): Error: Invalid scene load flag");
		return FAILURE;
		break;
	}

	// Load scene descriptor and resource list resources
	if (_loadDesc) {

		_sceneResourceId = resourceId;
		assert(_sceneResourceId != 0);
		debug(0, "Loading scene resource %u:", resourceId);

		if (loadSceneDescriptor(resourceId) != SUCCESS) {
			warning("Scene::loadScene(): Error reading scene descriptor");
			return FAILURE;
		}

		if (loadSceneResourceList(_desc.resListRN) != SUCCESS) {
			warning("Scene::loadScene(): Error reading scene resource list");
			return FAILURE;
		}
	} else {
		debug(0, "Loading memory scene resource.");
	}

	// Load resources from scene resource list
	for (i = 0; i < _resListEntries; i++) {
		result = RSC_LoadResource(_sceneContext, _resList[i].res_number,
								&_resList[i].res_data, &_resList[i].res_data_len);
		if (result != SUCCESS) {
			warning("Scene::loadScene(): Error: Allocation failure loading scene resource list");
			return FAILURE;
		}
	}

	// Process resources from scene resource list
	if (processSceneResources() != SUCCESS) {
		warning("Scene::loadScene(): Error loading scene resources");
		return FAILURE;
	}

	if (_desc.flags & kSceneFlagISO) {
		_outsetSceneNumber = _sceneNumber;
	} else {
		if (!(_bg.w < _vm->getDisplayWidth() || _bg.h < _vm->getSceneHeight()))
			_outsetSceneNumber = _sceneNumber;
	}

	_sceneLoaded = true;
	
	q_event = NULL;
	if (fadeType == SCENE_FADE || fadeType == SCENE_FADE_NO_INTERFACE) {

		_vm->_interface->rememberMode();
		_vm->_interface->setMode(kPanelFade, true);

		// Fade to black out
		_vm->_gfx->getCurrentPal(current_pal);
		event.type = IMMEDIATE_EVENT;
		event.code = PAL_EVENT;
		event.op = EVENT_PALTOBLACK;
		event.time = 0;
		event.duration = kNormalFadeDuration;
		event.data = current_pal;
		q_event = _vm->_events->queue(&event);

		if (fadeType != SCENE_FADE_NO_INTERFACE) {
			// Activate user interface
			event.type = IMMEDIATE_EVENT;
			event.code = INTERFACE_EVENT;
			event.op = EVENT_ACTIVATE;
			event.time = 0;
			event.duration = 0;
			q_event = _vm->_events->chain(q_event, &event);
		}

		// Display scene background, but stay with black palette
		event.type = IMMEDIATE_EVENT;
		event.code = BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = NO_SET_PALETTE;
		event.time = 0;
		event.duration = 0;
		q_event = _vm->_events->chain(q_event, &event);

		// Restore interface mode
		event.type = IMMEDIATE_EVENT;
		event.code = INTERFACE_EVENT;
		event.op = EVENT_RESTORE_MODE;
		event.time = 0;
		event.duration = 0;
		q_event = _vm->_events->chain(q_event, &event);
	}

	// Start the scene pre script, but stay with black palette
	if (_desc.startScriptEntrypointNumber > 0) {
		event.type = ONESHOT_EVENT;
		event.code = SCRIPT_EVENT;
		event.op = EVENT_EXEC_BLOCKING;
		event.time = 0;
		event.param = _desc.scriptModuleNumber;
		event.param2 = _desc.startScriptEntrypointNumber;
		event.param3 = 0;		// Action
		event.param4 = _sceneNumber;	// Object
		event.param5 = actorsEntrance;	// With Object
		event.param6 = 0;		// Actor

		q_event = _vm->_events->chain(q_event, &event);
	}

	if (fadeType == SCENE_FADE || fadeType == SCENE_FADE_NO_INTERFACE) {
		// Fade in from black to the scene background palette
		event.type = IMMEDIATE_EVENT;
		event.code = PAL_EVENT;
		event.op = EVENT_BLACKTOPAL;
		event.time = 0;
		event.duration = kNormalFadeDuration;
		event.data = _bg.pal;

		q_event = _vm->_events->chain(q_event, &event);
	}

	getInfo(&scene_info);

	if (scene_proc == NULL) {
		if (!_inGame) {
			_inGame = true;
			_vm->_interface->setMode(kPanelInventory);
		}

		_vm->_sound->stopVoice();
		_vm->_sound->stopSound();

		if (_desc.musicRN >= 0) {
			event.type = ONESHOT_EVENT;
			event.code = MUSIC_EVENT;
			event.param = _desc.musicRN;
			event.param2 = MUSIC_DEFAULT;
			event.op = EVENT_PLAY;
			event.time = 0;

			_vm->_events->queue(&event);
		} else {
			event.type = ONESHOT_EVENT;
			event.code = MUSIC_EVENT;
			event.op = EVENT_STOP;
			event.time = 0;

			_vm->_events->queue(&event);
		}

		// Set scene background
		event.type = ONESHOT_EVENT;
		event.code = BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;

		_vm->_events->queue(&event);

		// Activate user interface
		event.type = ONESHOT_EVENT;
		event.code = INTERFACE_EVENT;
		event.op = EVENT_ACTIVATE;
		event.time = 0;

		_vm->_events->queue(&event);

		// Begin palette cycle animation if present
		event.type = ONESHOT_EVENT;
		event.code = PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = _vm->_events->queue(&event);

		// Show cursor
		event.type = ONESHOT_EVENT;
		event.code = CURSOR_EVENT;
		event.op = EVENT_SHOW;
		_vm->_events->chain(q_event, &event);

		// Start the scene main script
		if (_desc.sceneScriptEntrypointNumber > 0) {
			event.type = ONESHOT_EVENT;
			event.code = SCRIPT_EVENT;
			event.op = EVENT_EXEC_NONBLOCKING;
			event.time = 0;
			event.param = _desc.scriptModuleNumber;
			event.param2 = _desc.sceneScriptEntrypointNumber;
			event.param3 = kVerbEnter;		// Action
			event.param4 = _sceneNumber;	// Object
			event.param5 = actorsEntrance;		// With Object
			event.param6 = 0;		// Actor

			_vm->_events->queue(&event);
		}

		debug(0, "Scene started");

	} else {
		scene_proc(SCENE_BEGIN, &scene_info, this);
	}



	// We probably don't want "followers" to go into scene -1. At the very
	// least we don't want garbage to be drawn that early in the ITE intro.
	if (_sceneNumber != -1)
		_vm->_actor->updateActorsScene(actorsEntrance);

	if (_desc.flags & kSceneFlagShowCursor)
		_vm->_interface->activate();

	return SUCCESS;
}

int Scene::loadSceneDescriptor(uint32 res_number) {
	byte *scene_desc_data;
	size_t scene_desc_len;
	int result;

	result = RSC_LoadResource(_sceneContext, res_number, &scene_desc_data, &scene_desc_len);
	if (result != SUCCESS) {
		warning("Scene::loadSceneDescriptor(): Error: couldn't load scene descriptor");
		return FAILURE;
	}

	MemoryReadStreamEndian readS(scene_desc_data, scene_desc_len, IS_BIG_ENDIAN);

	_desc.flags = readS.readSint16();
	_desc.resListRN = readS.readSint16();
	_desc.endSlope = readS.readSint16();
	_desc.beginSlope = readS.readSint16();
	_desc.scriptModuleNumber = readS.readUint16();
	_desc.sceneScriptEntrypointNumber = readS.readUint16();
	_desc.startScriptEntrypointNumber = readS.readUint16();
	_desc.musicRN = readS.readSint16();

	RSC_FreeResource(scene_desc_data);

	return SUCCESS;
}

int Scene::loadSceneResourceList(uint32 reslist_rn) {
	byte *resource_list;
	size_t resource_list_len;
	int result;
	int i;

	// Load the scene resource table
	result = RSC_LoadResource(_sceneContext, reslist_rn, &resource_list, &resource_list_len);
	if (result != SUCCESS) {
		warning("Scene::loadSceneResourceList(): Error: couldn't load scene resource list");
		return FAILURE;
	}

	MemoryReadStreamEndian readS(resource_list, resource_list_len, IS_BIG_ENDIAN);

	// Allocate memory for scene resource list 
	_resListEntries = resource_list_len / SAGA_RESLIST_ENTRY_LEN;
	debug(0, "Scene resource list contains %d entries.", _resListEntries);
	_resList = (SCENE_RESLIST *)calloc(_resListEntries, sizeof(*_resList));

	if (_resList == NULL) {
		memoryError("Scene::loadSceneResourceList()");
	}

	// Load scene resource list from raw scene 
	// resource table
	debug(0, "Loading scene resource list...");

	for (i = 0; i < _resListEntries; i++) {
		_resList[i].res_number = readS.readUint16();
		_resList[i].res_type = readS.readUint16();
	}

	RSC_FreeResource(resource_list);

	return SUCCESS;
}

int Scene::processSceneResources() {
	const byte *res_data;
	size_t res_data_len;
	const byte *pal_p;
	int i;

	// Process the scene resource list
	for (i = 0; i < _resListEntries; i++) {
		res_data = _resList[i].res_data;
		res_data_len = _resList[i].res_data_len;
		switch (_resList[i].res_type) {
		case SAGA_BG_IMAGE: // Scene background resource
			if (_bg.loaded) {
				warning("Scene::processSceneResources(): Multiple background resources encountered");
				return FAILURE;
			}

			debug(0, "Loading background resource.");
			_bg.res_buf = _resList[i].res_data;
			_bg.res_len = _resList[i].res_data_len;
			_bg.loaded = 1;

			if (_vm->decodeBGImage(_bg.res_buf,
				_bg.res_len,
				&_bg.buf,
				&_bg.buf_len,
				&_bg.w,
				&_bg.h) != SUCCESS) {
				warning("Scene::ProcessSceneResources(): Error loading background resource: %u", _resList[i].res_number);
				return FAILURE;
			}

			pal_p = _vm->getImagePal(_bg.res_buf, _bg.res_len);
			memcpy(_bg.pal, pal_p, sizeof(_bg.pal));
			break;
		case SAGA_BG_MASK: // Scene background mask resource
			if (_bgMask.loaded) {
				warning("Scene::ProcessSceneResources(): Duplicate background mask resource encountered");
			}
			debug(0, "Loading BACKGROUND MASK resource.");
			_bgMask.res_buf = _resList[i].res_data;
			_bgMask.res_len = _resList[i].res_data_len;
			_bgMask.loaded = 1;
			_vm->decodeBGImage(_bgMask.res_buf, _bgMask.res_len, &_bgMask.buf,
							&_bgMask.buf_len, &_bgMask.w, &_bgMask.h);
			debug(0, "BACKGROUND MASK width=%d height=%d length=%d", _bgMask.w, _bgMask.h, _bgMask.buf_len);
			break;
		case SAGA_STRINGS:
			debug(0, "Loading scene strings resource...");
			_vm->loadStrings(_sceneStrings, _resList[i].res_data, _resList[i].res_data_len);
			break;
		case SAGA_OBJECT_MAP:
			debug(0, "Loading object map resource...");
			_objectMap->load(res_data, res_data_len);			
			break;
		case SAGA_ACTION_MAP:
			debug(0, "Loading action map resource...");
			_actionMap->load(res_data, res_data_len);
			break;
		case SAGA_ISO_IMAGES:
			if (!(_desc.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(0, "Loading isometric images resource.");

			_vm->_isoMap->loadImages(res_data, res_data_len);
			break;
		case SAGA_ISO_MAP:
			if (!(_desc.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(0, "Loading isometric map resource.");

			_vm->_isoMap->loadMap(res_data, res_data_len);
			break;
		case SAGA_ISO_PLATFORMS:
			if (!(_desc.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(0, "Loading isometric platforms resource.");

			_vm->_isoMap->loadPlatforms(res_data, res_data_len);
			break;
		case SAGA_ISO_METATILES:
			if (!(_desc.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(0, "Loading isometric metatiles resource.");

			_vm->_isoMap->loadMetaTiles(res_data, res_data_len);
			break;			
		case SAGA_ANIM_1:
		case SAGA_ANIM_2:
		case SAGA_ANIM_3:
		case SAGA_ANIM_4:
		case SAGA_ANIM_5:
		case SAGA_ANIM_6:
		case SAGA_ANIM_7:
			{
				uint16 new_anim_id;

				debug(0, "Loading animation resource...");

				if (_vm->_anim->load(_resList[i].res_data,
					_resList[i].res_data_len, &new_anim_id) != SUCCESS) {
					warning("Scene::ProcessSceneResources(): Error loading animation resource");
					return FAILURE;
				}

				SCENE_ANIMINFO *new_animinfo;

				new_animinfo = _animList.pushBack().operator->();

				new_animinfo->anim_handle = new_anim_id;
				new_animinfo->anim_res_number =  _resList[i].res_number;
				_animEntries++;
			}
			break;
		case SAGA_ISO_MULTI:
			if (!(_desc.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(0, "Loading isometric multi resource.");

			_vm->_isoMap->loadMulti(res_data, res_data_len);
			break;			
		case SAGA_PAL_ANIM:
			debug(0, "Loading palette animation resource.");
			_vm->_palanim->loadPalAnim(_resList[i].res_data, _resList[i].res_data_len);
			break;
		case SAGA_ENTRY:
			debug(0, "Loading entry list resource...");
			loadSceneEntryList(res_data, res_data_len);
			break;
		case SAGA_FACES:
			_vm->_interface->loadScenePortraits(_resList[i].res_number);
			break;
		default:
			warning("Scene::ProcessSceneResources(): Encountered unknown resource type: %d", _resList[i].res_type);
			break;
		}
	}
	return SUCCESS;
}

int Scene::draw(SURFACE *dst_s) {
	BUFFER_INFO buf_info;
	Point bgPoint(0, 0);

	assert(_initialized);

	_vm->_render->getBufferInfo(&buf_info);

	if (_desc.flags & kSceneFlagISO) {
		_vm->_isoMap->adjustScroll(false);
		_vm->_isoMap->draw(dst_s);
	} else {
		bufToSurface(dst_s, buf_info.bg_buf, _vm->getDisplayWidth(),
						MAX(_vm->getSceneHeight(), _bg.h), NULL, &bgPoint);
	}

	return SUCCESS;
}

int Scene::endScene() {
	SCENE_INFO scene_info;

	assert(_initialized);

	if (!_sceneLoaded) {
		warning("Scene::endScene(): No scene to end");
		return -1;
	}

	debug(0, "Ending scene...");

	if (_sceneProc != NULL) {
		getInfo(&scene_info);

		_sceneProc(SCENE_END, &scene_info, this);
	}

	//
	_vm->_script->abortAllThreads();
	_vm->_script->_skipSpeeches = false;

	// Copy current screen to render buffer so inset rooms will get proper background
	SURFACE *back_buf = _vm->_gfx->getBackBuffer();
	BUFFER_INFO rbuf_info;

	_vm->_render->getBufferInfo(&rbuf_info);
	bufToBuffer(rbuf_info.bg_buf, rbuf_info.bg_buf_w, rbuf_info.bg_buf_h, (byte *)back_buf->pixels, back_buf->w, back_buf->h, NULL, NULL);
	// Free scene background
	if (_bg.loaded) {
		free(_bg.buf);
		_bg.loaded = 0;
	}

	// Free scene background mask
	if (_bgMask.loaded) {
		free(_bgMask.buf);
		_bgMask.loaded = 0;
	}

	// Free scene resource list
	if (_loadDesc) {

		free(_resList);
	}

	// Free animation info list
	_vm->_anim->reset();

	_vm->_palanim->freePalAnim();
	
	_objectMap->freeMem();
	_actionMap->freeMem();
	_entryList.freeMem();
	_sceneStrings.freeMem();
	_vm->_isoMap->freeMem();

	_animList.clear();

	_animEntries = 0;

	_vm->_events->clearList();
	_vm->textClearList(_textList);

	_sceneLoaded = false;

	return SUCCESS;
}

void Scene::cmdSceneChange(int argc, const char **argv) {
	int scene_num = 0;

	scene_num = atoi(argv[1]);

	if ((scene_num < 1) || (scene_num > _sceneMax)) {
		_vm->_console->DebugPrintf("Invalid scene number.\n");
		return;
	}

	clearSceneQueue();

	if (changeScene(scene_num, 0) == SUCCESS) {
		_vm->_console->DebugPrintf("Scene changed.\n");
	} else {
		_vm->_console->DebugPrintf("Couldn't change scene!\n");
	}
}

void Scene::cmdSceneInfo() {
	const char *fmt = "%-20s %d\n";

	_vm->_console->DebugPrintf(fmt, "Scene number:", _sceneNumber);
	_vm->_console->DebugPrintf(fmt, "Descriptor ResourceId:", _sceneResourceId);
	_vm->_console->DebugPrintf("-------------------------\n");
	_vm->_console->DebugPrintf(fmt, "Flags:", _desc.flags);
	_vm->_console->DebugPrintf(fmt, "Resource list R#:", _desc.resListRN);
	_vm->_console->DebugPrintf(fmt, "End slope:", _desc.endSlope);
	_vm->_console->DebugPrintf(fmt, "Begin slope:", _desc.beginSlope);
	_vm->_console->DebugPrintf(fmt, "scriptModuleNumber:", _desc.scriptModuleNumber);
	_vm->_console->DebugPrintf(fmt, "sceneScriptEntrypointNumber:", _desc.sceneScriptEntrypointNumber);
	_vm->_console->DebugPrintf(fmt, "startScriptEntrypointNumber:", _desc.startScriptEntrypointNumber);
	_vm->_console->DebugPrintf(fmt, "Music R#", _desc.musicRN);
}


void Scene::cmdActionMapInfo() {
	_actionMap->cmdInfo();
}

void Scene::cmdObjectMapInfo() {
	_objectMap->cmdInfo();
}


void Scene::loadSceneEntryList(const byte* resourcePointer, size_t resourceLength) {	
	int i;
	
	_entryList.entryListCount = resourceLength / 8;

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);


	if (_entryList.entryList)
		error("Scene::loadSceneEntryList entryList != NULL");

	_entryList.entryList = (SceneEntry *) malloc(_entryList.entryListCount * sizeof(*_entryList.entryList));
	if (_entryList.entryList == NULL) {
		memoryError("Scene::loadSceneEntryList");
	}

	for (i = 0; i < _entryList.entryListCount; i++) {
		_entryList.entryList[i].location.x = readS.readSint16();
		_entryList.entryList[i].location.y = readS.readSint16();
		_entryList.entryList[i].location.z = readS.readSint16();
		_entryList.entryList[i].facing = readS.readUint16();
	}
}

} // End of namespace Saga
