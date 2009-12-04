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
 * $URL$
 * $Id$
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
#include "saga/puzzle.h"
#include "saga/render.h"
#include "saga/script.h"
#include "saga/sound.h"
#include "saga/music.h"

#include "saga/scene.h"
#include "saga/actor.h"
#include "saga/resource.h"

#include "graphics/iff.h"
#include "common/util.h"

namespace Saga {

static int initSceneDoors[SCENE_DOORS_MAX] = {
	0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static SAGAResourceTypes ITESceneResourceTypes[26] = {
	SAGA_ACTOR,
	SAGA_OBJECT,
	SAGA_BG_IMAGE,
	SAGA_BG_MASK,
SAGA_UNKNOWN,
	SAGA_STRINGS,
	SAGA_OBJECT_MAP,
	SAGA_ACTION_MAP,
	SAGA_ISO_IMAGES,
	SAGA_ISO_MAP,
	SAGA_ISO_PLATFORMS,
	SAGA_ISO_METATILES,
	SAGA_ENTRY,
SAGA_UNKNOWN,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ISO_MULTI,
	SAGA_PAL_ANIM,
	SAGA_FACES,
	SAGA_PALETTE
};

#ifdef ENABLE_IHNM
static SAGAResourceTypes IHNMSceneResourceTypes[28] = {
	SAGA_ACTOR,
SAGA_UNKNOWN,
	SAGA_BG_IMAGE,
	SAGA_BG_MASK,
SAGA_UNKNOWN,
	SAGA_STRINGS,
	SAGA_OBJECT_MAP,
	SAGA_ACTION_MAP,
	SAGA_ISO_IMAGES,
	SAGA_ISO_MAP,
	SAGA_ISO_PLATFORMS,
	SAGA_ISO_METATILES,
	SAGA_ENTRY,
SAGA_UNKNOWN,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ANIM,
	SAGA_ISO_MULTI,
	SAGA_PAL_ANIM,
	SAGA_FACES,
	SAGA_PALETTE
};
#endif

const char *SAGAResourceTypesString[] = {
	"SAGA_UNKNOWN",
	"SAGA_ACTOR",
	"SAGA_OBJECT",
	"SAGA_BG_IMAGE",
	"SAGA_BG_MASK",
	"SAGA_STRINGS",
	"SAGA_OBJECT_MAP",
	"SAGA_ACTION_MAP",
	"SAGA_ISO_IMAGES",
	"SAGA_ISO_MAP",
	"SAGA_ISO_PLATFORMS",
	"SAGA_ISO_METATILES",
	"SAGA_ENTRY",
	"SAGA_ANIM",
	"SAGA_ISO_MULTI",
	"SAGA_PAL_ANIM",
	"SAGA_FACES",
	"SAGA_PALETTE"
};

Scene::Scene(SagaEngine *vm) : _vm(vm) {
	byte *sceneLUTPointer;
	size_t sceneLUTLength;
	uint32 resourceId;
	int i;

	// Do nothing for SAGA2 games for now
	if (_vm->isSaga2()) {
		_inGame = false;
		_sceneLoaded = false;
		return;
	}

	// Load scene module resource context
	_sceneContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (_sceneContext == NULL) {
		error("Scene::Scene() scene context not found");
	}

	// Load scene lookup table
	resourceId = _vm->_resource->convertResourceId(_vm->getResourceDescription()->sceneLUTResourceId);
	debug(3, "Loading scene LUT from resource %i", resourceId);
	_vm->_resource->loadResource(_sceneContext, resourceId, sceneLUTPointer, sceneLUTLength);
	if (sceneLUTLength == 0) {
		error("Scene::Scene() sceneLUTLength == 0");
	}
	_sceneCount = sceneLUTLength / 2;
	_sceneLUT = (int *)malloc(_sceneCount * sizeof(*_sceneLUT));
	if (_sceneLUT == NULL) {
		memoryError("Scene::Scene()");
	}

	MemoryReadStreamEndian readS(sceneLUTPointer, sceneLUTLength, _sceneContext->isBigEndian());

	for (i = 0; i < _sceneCount; i++) {
		_sceneLUT[i] = readS.readUint16();
		debug(8, "sceneNumber %i has resourceId %i", i, _sceneLUT[i]);
	}

	free(sceneLUTPointer);

#ifdef SAGA_DEBUG

#define DUMP_SCENES_LEVEL 10

	if (DUMP_SCENES_LEVEL <= gDebugLevel) {
		uint j;
		int backUpDebugLevel = gDebugLevel;
		SAGAResourceTypes *types;
		int typesCount;
		SAGAResourceTypes resType;

		getResourceTypes(types, typesCount);

		for (i = 0; i < _sceneCount; i++) {
			gDebugLevel = -1;
			loadSceneDescriptor(_sceneLUT[i]);
			loadSceneResourceList(_sceneDescription.resourceListResourceId);
			gDebugLevel = backUpDebugLevel;
			debug(DUMP_SCENES_LEVEL, "Dump Scene: number %i, descriptor resourceId %i, resourceList resourceId %i", i, _sceneLUT[i], _sceneDescription.resourceListResourceId);
			debug(DUMP_SCENES_LEVEL, "\tresourceListCount %i", (int)_resourceListCount);
			for (j = 0; j < _resourceListCount; j++) {
				if (_resourceList[j].resourceType >= typesCount) {
					error("wrong resource type %i", _resourceList[j].resourceType);
				}
				resType = types[_resourceList[j].resourceType];

				debug(DUMP_SCENES_LEVEL, "\t%s resourceId %i", SAGAResourceTypesString[resType], _resourceList[j].resourceId);
			}
			free(_resourceList);
		}
	}
#endif

	debug(3, "LUT has %d entries.", _sceneCount);

	_sceneLoaded = false;
	_sceneNumber = 0;
	_chapterNumber = 0;
	_sceneResourceId = 0;
	_inGame = false;
	_loadDescription = false;
	memset(&_sceneDescription, 0, sizeof(_sceneDescription));
	_resourceListCount = 0;
	_resourceList = NULL;
	_sceneProc = NULL;
	_objectMap = new ObjectMap(_vm);
	_actionMap = new ObjectMap(_vm);
	memset(&_bg, 0, sizeof(_bg));
	memset(&_bgMask, 0, sizeof(_bgMask));
}

Scene::~Scene() {
	// Do nothing for SAGA2 games for now
	if (_vm->isSaga2()) {
		return;
	}

	delete _actionMap;
	delete _objectMap;
	free(_sceneLUT);
}

void Scene::getResourceTypes(SAGAResourceTypes *&types, int &typesCount) {
	if (_vm->getGameId() == GID_ITE) {
		typesCount = ARRAYSIZE(ITESceneResourceTypes);
		types = ITESceneResourceTypes;
#ifdef ENABLE_IHNM
	} else if (_vm->getGameId() == GID_IHNM) {
		typesCount = ARRAYSIZE(IHNMSceneResourceTypes);
		types = IHNMSceneResourceTypes;
#endif
	}
}

void Scene::drawTextList() {
	for (TextList::iterator entry = _textList.begin(); entry != _textList.end(); ++entry) {

		if (entry->display) {

			if (entry->useRect) {
				_vm->_font->textDrawRect(entry->font, entry->text, entry->rect, _vm->KnownColor2ColorId(entry->knownColor), _vm->KnownColor2ColorId(entry->effectKnownColor), entry->flags);
			} else {
				_vm->_font->textDraw(entry->font, entry->text, entry->point, _vm->KnownColor2ColorId(entry->knownColor), _vm->KnownColor2ColorId(entry->effectKnownColor), entry->flags);
			}
		}
	}
}

void Scene::startScene() {
	SceneQueueList::iterator queueIterator;
	Event event;

	if (_sceneLoaded) {
		error("Scene::start(): Error: Can't start game...scene already loaded");
	}

	if (_inGame) {
		error("Scene::start(): Error: Can't start game...game already started");
	}

	// Hide cursor during intro
	event.type = kEvTOneshot;
	event.code = kCursorEvent;
	event.op = kEventHide;
	_vm->_events->queue(&event);

	switch (_vm->getGameId()) {
	case GID_ITE:
		ITEStartProc();
		break;
#ifdef ENABLE_IHNM
	case GID_IHNM:
		IHNMStartProc();
		break;
#endif
#ifdef ENABLE_SAGA2
	case GID_DINO:
		DinoStartProc();
		break;
	case GID_FTA2:
		FTA2StartProc();
		break;
#endif
	default:
		error("Scene::start(): Error: Can't start game... gametype not supported");
		break;
	}

	// Stop the intro music
	_vm->_music->stop();

	// Load the head in scene queue
	queueIterator = _sceneQueue.begin();
	if (queueIterator == _sceneQueue.end()) {
		return;
	}

	loadScene(*queueIterator);
}

#ifdef ENABLE_IHNM

void Scene::creditsScene() {
	// End the last game ending scene
	_vm->_scene->endScene();
	// We're not in the game anymore
	_inGame = false;

	// Hide cursor during credits
	_vm->_gfx->showCursor(false);

	switch (_vm->getGameId()) {
	case GID_ITE:
		// Not called by ITE
		break;
	case GID_IHNM:
		IHNMCreditsProc();
		break;
	default:
		error("Scene::creditsScene(): Error: Can't start credits scene... gametype not supported");
		break;
	}

	_vm->quitGame();
	return;
}

#endif

void Scene::nextScene() {
	SceneQueueList::iterator queueIterator;

	if (!_sceneLoaded) {
		error("Scene::next(): Error: Can't advance scene...no scene loaded");
	}

	if (_inGame) {
		error("Scene::next(): Error: Can't advance scene...game already started");
	}

	endScene();

	// Delete the current head in scene queue
	queueIterator = _sceneQueue.begin();
	if (queueIterator == _sceneQueue.end()) {
		return;
	}

	queueIterator = _sceneQueue.erase(queueIterator);

	if (queueIterator == _sceneQueue.end()) {
		return;
	}

	// Load the head in scene queue
	loadScene(*queueIterator);
}

void Scene::skipScene() {
	SceneQueueList::iterator queueIterator;

	if (!_sceneLoaded) {
		error("Scene::skip(): Error: Can't skip scene...no scene loaded");
	}

	if (_inGame) {
		error("Scene::skip(): Error: Can't skip scene...game already started");
	}

	// Walk down scene queue and try to find a skip target
	queueIterator = _sceneQueue.begin();
	if (queueIterator == _sceneQueue.end()) {
		error("Scene::skip(): Error: Can't skip scene...no scenes in queue");
	}

	++queueIterator;
	while (queueIterator != _sceneQueue.end()) {
		if (queueIterator->sceneSkipTarget) {
			// If skip target found, remove preceding scenes and load
			_sceneQueue.erase(_sceneQueue.begin(), queueIterator);

			endScene();

			loadScene(*_sceneQueue.begin());
			break;
		}
		++queueIterator;
	}

}

static struct SceneSubstitutes {
	int sceneId;
	const char *message;
	const char *title;
	const char *image;
} sceneSubstitutes[] = {
	{
		7,
		"Tycho says he knows much about the northern lands. Can Rif convince "
		"the Dog to share this knowledge?",
		"The Home of Tycho Northpaw",
		"tycho.bbm"
	},

	{
		27,
		"The scene of the crime may hold many clues, but will the servants of "
		"the Sanctuary trust Rif?",
		"The Sanctuary of the Orb",
		"sanctuar.bbm"
	},

	{
		5,
		"The Rats hold many secrets that could guide Rif on his quest -- assuming "
		"he can get past the doorkeeper.",
		"The Rat Complex",
		"ratdoor.bbm"
	},

	{
		2,
		"The Ferrets enjoy making things and have the materials to do so. How can "
		"that help Rif?",
		"The Ferret Village",
		"ferrets.bbm"
	},

	{
		67,
		"What aid can the noble King of the Elks provide to Rif and his companions?",
		"The Realm of the Forest King",
		"elkenter.bbm"
	},

	{
		3,
		"The King holds Rif's sweetheart hostage. Will the Boar provide any "
		"assistance to Rif?",
		"The Great Hall of the Boar King",
		"boarhall.bbm"
	}
};

void Scene::changeScene(int16 sceneNumber, int actorsEntrance, SceneTransitionType transitionType, int chapter) {

	debug(5, "Scene::changeScene(%d, %d, %d, %d)", sceneNumber, actorsEntrance, transitionType, chapter);

	// This is used for latter ITE demos where all places on world map except
	// Tent Faire are substituted with LBM picture and short description
	if (_vm->getFeatures() & GF_SCENE_SUBSTITUTES) {
		for (int i = 0; i < ARRAYSIZE(sceneSubstitutes); i++) {
			if (sceneSubstitutes[i].sceneId == sceneNumber) {
				Surface bbmBuffer;
				byte *pal, colors[768];
				Common::File file;
				Rect rect;
				PalEntry cPal[PAL_ENTRIES];

				_vm->_interface->setMode(kPanelSceneSubstitute);

				if (file.open(sceneSubstitutes[i].image)) {
					Graphics::decodePBM(file, bbmBuffer, colors);
					pal = colors;
					rect.setWidth(bbmBuffer.w);
					rect.setHeight(bbmBuffer.h);
					_vm->_gfx->drawRegion(rect, (const byte*)bbmBuffer.pixels);
					for (int j = 0; j < PAL_ENTRIES; j++) {
						cPal[j].red = *pal++;
						cPal[j].green = *pal++;
						cPal[j].blue = *pal++;
					}
					_vm->_gfx->setPalette(cPal);

				}

				_vm->_interface->setStatusText("Click or Press Return to continue. Press Q to quit.", 96);
				_vm->_font->textDrawRect(kKnownFontMedium, sceneSubstitutes[i].title,
					 Common::Rect(0, 7, _vm->getDisplayInfo().width, 27), _vm->KnownColor2ColorId(kKnownColorBrightWhite), _vm->KnownColor2ColorId(kKnownColorBlack), kFontOutline);
				_vm->_font->textDrawRect(kKnownFontMedium, sceneSubstitutes[i].message,
					 Common::Rect(24, getHeight() - 33, _vm->getDisplayInfo().width - 11,
								  getHeight()), _vm->KnownColor2ColorId(kKnownColorBrightWhite), _vm->KnownColor2ColorId(kKnownColorBlack), kFontOutline);
				return;
			}
		}
	}

	LoadSceneParams sceneParams;

	sceneParams.actorsEntrance = actorsEntrance;
	sceneParams.loadFlag = kLoadBySceneNumber;
	sceneParams.sceneDescriptor = sceneNumber;
	sceneParams.transitionType = transitionType;
	sceneParams.sceneProc = NULL;
	sceneParams.sceneSkipTarget = false;
	sceneParams.chapter = chapter;

	if (sceneNumber != -2) {
		endScene();
	}

	loadScene(sceneParams);
}

void Scene::getSlopes(int &beginSlope, int &endSlope) {
	beginSlope = getHeight() - _sceneDescription.beginSlope;
	endSlope = getHeight() - _sceneDescription.endSlope;
}

void Scene::getBGInfo(BGInfo &bgInfo) {
	bgInfo.buffer = _bg.buf;
	bgInfo.bufferLength = _bg.buf_len;
	bgInfo.bounds.left = 0;
	bgInfo.bounds.top = 0;

	if (_bg.w < _vm->getDisplayInfo().width) {
		bgInfo.bounds.left = (_vm->getDisplayInfo().width - _bg.w) / 2;
	}

	if (_bg.h < getHeight()) {
		bgInfo.bounds.top = (getHeight() - _bg.h) / 2;
	}

	bgInfo.bounds.setWidth(_bg.w);
	bgInfo.bounds.setHeight(_bg.h);
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

	point.x = CLIP<int>(testPoint.x, 0, _vm->getDisplayInfo().width - 1);
	point.y = CLIP<int>(testPoint.y, 0, _bgMask.h - 1);
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

void Scene::initDoorsState() {
	memcpy(_sceneDoors, initSceneDoors, sizeof (_sceneDoors) );
}

void Scene::loadScene(LoadSceneParams &loadSceneParams) {
	size_t i;
	Event event;
	Event *q_event;
	static PalEntry current_pal[PAL_ENTRIES];

	if (loadSceneParams.transitionType == kTransitionFade)
		_vm->_interface->setFadeMode(kFadeOut);

	// Change the cursor to an hourglass in IHNM
	event.type = kEvTOneshot;
	event.code = kCursorEvent;
	event.op = kEventSetBusyCursor;
	event.time = 0;
	_vm->_events->queue(&event);

	_chapterPointsChanged = false;

#ifdef ENABLE_IHNM
	if ((_vm->getGameId() == GID_IHNM) && (loadSceneParams.chapter != NO_CHAPTER_CHANGE)) {
		if (loadSceneParams.loadFlag != kLoadBySceneNumber) {
			error("loadScene wrong usage");
		}

		if (loadSceneParams.chapter == 6 || loadSceneParams.chapter == 8)
			_vm->_interface->setLeftPortrait(0);

		_vm->_anim->freeCutawayList();
		_vm->_script->freeModules();

		// deleteAllScenes();

		// installSomeAlarm()

		_vm->_interface->clearInventory();
		_vm->_resource->loadGlobalResources(loadSceneParams.chapter, loadSceneParams.actorsEntrance);
		_vm->_interface->addToInventory(IHNM_OBJ_PROFILE);
		_vm->_interface->activate();

		if (loadSceneParams.chapter == 8 || loadSceneParams.chapter == -1) {
			if (!(_vm->getFeatures() & GF_IHNM_DEMO))
				_vm->_interface->setMode(kPanelChapterSelection);
			else
				_vm->_interface->setMode(kPanelNull);
		} else {
			_vm->_interface->setMode(kPanelMain);
		}

		_inGame = true;

		_vm->_script->setVerb(_vm->_script->getVerbType(kVerbWalkTo));

		if (loadSceneParams.sceneDescriptor == -2) {
			_vm->_interface->setFadeMode(kNoFade);
			return;
		}
	}
#endif

	if (_sceneLoaded) {
		error("Scene::loadScene(): Error, a scene is already loaded");
	}

	_loadDescription = true;

#ifdef ENABLE_IHNM
	if (_vm->getGameId() == GID_IHNM) {
		if (loadSceneParams.loadFlag == kLoadBySceneNumber) // When will we get rid of it?
			if (loadSceneParams.sceneDescriptor <= 0)
				loadSceneParams.sceneDescriptor = _vm->_resource->getMetaResource()->sceneIndex;
	}
#endif

	switch (loadSceneParams.loadFlag) {
	case kLoadByResourceId:
		_sceneNumber = 0;		// original assign zero for loaded by resource id
		_sceneResourceId = loadSceneParams.sceneDescriptor;
		break;
	case kLoadBySceneNumber:
		_sceneNumber = loadSceneParams.sceneDescriptor;
		_sceneResourceId = getSceneResourceId(_sceneNumber);
		break;
	case kLoadByDescription:
		_sceneNumber = -1;
		_sceneResourceId = -1;
		assert(loadSceneParams.sceneDescription != NULL);
		assert(loadSceneParams.sceneDescription->resourceList != NULL);
		_loadDescription = false;
		_sceneDescription = *loadSceneParams.sceneDescription;
		_resourceList = loadSceneParams.sceneDescription->resourceList;
		_resourceListCount = loadSceneParams.sceneDescription->resourceListCount;
		break;
	}

	debug(3, "Loading scene number %d:", _sceneNumber);

	if (isNonInteractiveIHNMDemoPart()) {
		// WORKAROUND for the non-interactive part of the IHNM demo: When restarting the
		// non-interactive demo, opcode sfMainMode is incorrectly called. Therefore, if any
		// of the scenes of the non-interactive demo are loaded (scenes 144-149), set panel
		// to null and lock the user interface
		_vm->_interface->deactivate();
		_vm->_interface->setMode(kPanelNull);
	}

	// Load scene descriptor and resource list resources
	if (_loadDescription) {
		debug(3, "Loading scene resource %i", _sceneResourceId);

		loadSceneDescriptor(_sceneResourceId);

		loadSceneResourceList(_sceneDescription.resourceListResourceId);
	} else {
		debug(3, "Loading memory scene resource");
	}

	// Load resources from scene resource list
	for (i = 0; i < _resourceListCount; i++) {
		if (!_resourceList[i].invalid) {
			_vm->_resource->loadResource(_sceneContext, _resourceList[i].resourceId,
				_resourceList[i].buffer, _resourceList[i].size);


			if (_resourceList[i].size >= 6) {
				if (!memcmp(_resourceList[i].buffer, "DUMMY!", 6)) {
					_resourceList[i].invalid = true;
					warning("DUMMY resource %i", _resourceList[i].resourceId);
				}
			}
		}
	}

	// Process resources from scene resource list
	processSceneResources();

	if (_sceneDescription.flags & kSceneFlagISO) {
		_outsetSceneNumber = _sceneNumber;

		_sceneClip.left = 0;
		_sceneClip.top = 0;
		_sceneClip.right = _vm->getDisplayInfo().width;
		_sceneClip.bottom = getHeight();
	} else {
		BGInfo backGroundInfo;
		getBGInfo(backGroundInfo);
		_sceneClip = backGroundInfo.bounds;
		if (!(_bg.w < _vm->getDisplayInfo().width || _bg.h < getHeight()))
			_outsetSceneNumber = _sceneNumber;
	}

	_sceneLoaded = true;

	q_event = NULL;

	if (loadSceneParams.transitionType == kTransitionFade) {

		_vm->_interface->setFadeMode(kFadeOut);

		// Fade to black out
		_vm->_gfx->getCurrentPal(current_pal);
		event.type = kEvTImmediate;
		event.code = kPalEvent;
		event.op = kEventPalToBlack;
		event.time = 0;
		event.duration = kNormalFadeDuration;
		event.data = current_pal;
		q_event = _vm->_events->queue(&event);

		// set fade mode
		event.type = kEvTImmediate;
		event.code = kInterfaceEvent;
		event.op = kEventSetFadeMode;
		event.param = kNoFade;
		event.time = 0;
		event.duration = 0;
		q_event = _vm->_events->chain(q_event, &event);

		// Display scene background, but stay with black palette
		event.type = kEvTImmediate;
		event.code = kBgEvent;
		event.op = kEventDisplay;
		event.param = kEvPNoSetPalette;
		event.time = 0;
		event.duration = 0;
		q_event = _vm->_events->chain(q_event, &event);

	}

	// Start the scene pre script, but stay with black palette
	if (_sceneDescription.startScriptEntrypointNumber > 0) {
		event.type = kEvTOneshot;
		event.code = kScriptEvent;
		event.op = kEventExecBlocking;
		event.time = 0;
		event.param = _sceneDescription.scriptModuleNumber;
		event.param2 = _sceneDescription.startScriptEntrypointNumber;
		event.param3 = 0;		// Action
		event.param4 = _sceneNumber;	// Object
		event.param5 = loadSceneParams.actorsEntrance;	// With Object
		event.param6 = 0;		// Actor
		q_event = _vm->_events->chain(q_event, &event);
	}

	if (loadSceneParams.transitionType == kTransitionFade) {

		// set fade mode
		event.type = kEvTImmediate;
		event.code = kInterfaceEvent;
		event.op = kEventSetFadeMode;
		event.param = kFadeIn;
		event.time = 0;
		event.duration = 0;
		q_event = _vm->_events->chain(q_event, &event);

		// Fade in from black to the scene background palette
		event.type = kEvTImmediate;
		event.code = kPalEvent;
		event.op = kEventBlackToPal;
		event.time = 0;
		event.duration = kNormalFadeDuration;
		event.data = _bg.pal;
		q_event = _vm->_events->chain(q_event, &event);

		// set fade mode
		event.type = kEvTImmediate;
		event.code = kInterfaceEvent;
		event.op = kEventSetFadeMode;
		event.param = kNoFade;
		event.time = 0;
		event.duration = 0;
		q_event = _vm->_events->chain(q_event, &event);
	}

	if (loadSceneParams.sceneProc == NULL) {
		if (!_inGame && _vm->getGameId() == GID_ITE) {
			_inGame = true;
			_vm->_interface->setMode(kPanelMain);
		}

		_vm->_sound->stopAll();

		if (_vm->getGameId() == GID_ITE) {
			if (_sceneDescription.musicResourceId >= 0) {
				event.type = kEvTOneshot;
				event.code = kMusicEvent;
				event.param = _sceneDescription.musicResourceId;
				event.param2 = MUSIC_DEFAULT;
				event.op = kEventPlay;
				event.time = 0;
				_vm->_events->queue(&event);
			} else {
				event.type = kEvTOneshot;
				event.code = kMusicEvent;
				event.op = kEventStop;
				event.time = 0;
				_vm->_events->queue(&event);
			}
		}

		// Set scene background
		event.type = kEvTOneshot;
		event.code = kBgEvent;
		event.op = kEventDisplay;
		event.param = kEvPSetPalette;
		event.time = 0;
		_vm->_events->queue(&event);

		// Begin palette cycle animation if present
		event.type = kEvTOneshot;
		event.code = kPalAnimEvent;
		event.op = kEventCycleStart;
		event.time = 0;
		q_event = _vm->_events->queue(&event);

		// Start the scene main script
		if (_sceneDescription.sceneScriptEntrypointNumber > 0) {
			event.type = kEvTOneshot;
			event.code = kScriptEvent;
			event.op = kEventExecNonBlocking;
			event.time = 0;
			event.param = _sceneDescription.scriptModuleNumber;
			event.param2 = _sceneDescription.sceneScriptEntrypointNumber;
			event.param3 = _vm->_script->getVerbType(kVerbEnter);		// Action
			event.param4 = _sceneNumber;	// Object
			event.param5 = loadSceneParams.actorsEntrance;		// With Object
			event.param6 = 0;		// Actor
			_vm->_events->queue(&event);
		}

		debug(3, "Scene started");

	} else {
		loadSceneParams.sceneProc(SCENE_BEGIN, this);
	}

	// We probably don't want "followers" to go into scene -1 , 0. At the very
	// least we don't want garbage to be drawn that early in the ITE intro.
	if (_sceneNumber > 0 && _sceneNumber != ITE_SCENE_PUZZLE)
		_vm->_actor->updateActorsScene(loadSceneParams.actorsEntrance);

	if (_sceneNumber == ITE_SCENE_PUZZLE)
		_vm->_puzzle->execute();

	if (getFlags() & kSceneFlagShowCursor) {
		// Activate user interface
		event.type = kEvTOneshot;
		event.code = kInterfaceEvent;
		event.op = kEventActivate;
		event.time = 0;
		_vm->_events->queue(&event);
	}

	// Change the cursor back to a crosshair in IHNM
	event.type = kEvTOneshot;
	event.code = kCursorEvent;
	event.op = kEventSetNormalCursor;
	event.time = 0;
	_vm->_events->queue(&event);
}

void Scene::loadSceneDescriptor(uint32 resourceId) {
	byte *sceneDescriptorData;
	size_t sceneDescriptorDataLength;

	memset(&_sceneDescription, 0, sizeof(_sceneDescription));

	if (resourceId == 0) {
		return;
	}

	_vm->_resource->loadResource(_sceneContext, resourceId, sceneDescriptorData, sceneDescriptorDataLength);

	if (sceneDescriptorDataLength == 16) {
		MemoryReadStreamEndian readS(sceneDescriptorData, sceneDescriptorDataLength, _sceneContext->isBigEndian());

		_sceneDescription.flags = readS.readSint16();
		_sceneDescription.resourceListResourceId = readS.readSint16();
		_sceneDescription.endSlope = readS.readSint16();
		_sceneDescription.beginSlope = readS.readSint16();
		_sceneDescription.scriptModuleNumber = readS.readUint16();
		_sceneDescription.sceneScriptEntrypointNumber = readS.readUint16();
		_sceneDescription.startScriptEntrypointNumber = readS.readUint16();
		_sceneDescription.musicResourceId = readS.readSint16();
	}

	free(sceneDescriptorData);
}

void Scene::loadSceneResourceList(uint32 resourceId) {
	byte *resourceListData;
	size_t resourceListDataLength;
	size_t i;

	_resourceListCount = 0;
	_resourceList = NULL;

	if (resourceId == 0) {
		return;
	}

	// Load the scene resource table
	_vm->_resource->loadResource(_sceneContext, resourceId, resourceListData, resourceListDataLength);

	if ((resourceListDataLength % SAGA_RESLIST_ENTRY_LEN) == 0) {
		MemoryReadStreamEndian readS(resourceListData, resourceListDataLength, _sceneContext->isBigEndian());

		// Allocate memory for scene resource list
		_resourceListCount = resourceListDataLength / SAGA_RESLIST_ENTRY_LEN;
		debug(3, "Scene resource list contains %i entries", (int)_resourceListCount);
		_resourceList = (SceneResourceData *)calloc(_resourceListCount, sizeof(*_resourceList));

		// Load scene resource list from raw scene
		// resource table
		debug(3, "Loading scene resource list");

		for (i = 0; i < _resourceListCount; i++) {
			_resourceList[i].resourceId = readS.readUint16();
			_resourceList[i].resourceType = readS.readUint16();
			// demo version may contain invalid resourceId
			_resourceList[i].invalid = !_sceneContext->validResourceId(_resourceList[i].resourceId);
		}

	}
	free(resourceListData);
}

void Scene::processSceneResources() {
	byte *resourceData;
	size_t resourceDataLength;
	const byte *palPointer;
	size_t i;
	SAGAResourceTypes *types;
	int typesCount;
	SAGAResourceTypes resType;

	getResourceTypes(types, typesCount);

	// Process the scene resource list
	for (i = 0; i < _resourceListCount; i++) {
		if (_resourceList[i].invalid) {
			continue;
		}
		resourceData = _resourceList[i].buffer;
		resourceDataLength = _resourceList[i].size;

		if (_resourceList[i].resourceType >= typesCount) {
			error("Scene::processSceneResources() wrong resource type %i", _resourceList[i].resourceType);
		}

		resType = types[_resourceList[i].resourceType];

		switch (resType) {
		case SAGA_UNKNOWN:
			warning("UNKNOWN resourceType %i", _resourceList[i].resourceType);
			break;
		case SAGA_ACTOR:
			//for (a = actorsInScene; a; a = a->nextInScene)
			//	if (a->obj.figID == glist->file_id)
			//		if (_vm->getGameId() == GID_ITE ||
			//			((a->obj.flags & ACTORF_FINAL_FACE) & 0xff))
			//			a->sprites = (xSpriteSet *)glist->offset;
			warning("STUB: unimplemeted handler of SAGA_ACTOR resource");
			break;
		case SAGA_OBJECT:
			break;
		case SAGA_BG_IMAGE: // Scene background resource
			if (_bg.loaded) {
				error("Scene::processSceneResources() Multiple background resources encountered");
			}

			debug(3, "Loading background resource.");
			_bg.res_buf = resourceData;
			_bg.res_len = resourceDataLength;
			_bg.loaded = 1;

			if (_vm->decodeBGImage(_bg.res_buf,
				_bg.res_len,
				&_bg.buf,
				&_bg.buf_len,
				&_bg.w,
				&_bg.h) != SUCCESS) {
				error("Scene::processSceneResources() Error loading background resource %i", _resourceList[i].resourceId);
			}

			palPointer = _vm->getImagePal(_bg.res_buf, _bg.res_len);
			memcpy(_bg.pal, palPointer, sizeof(_bg.pal));
			break;
		case SAGA_BG_MASK: // Scene background mask resource
			if (_bgMask.loaded) {
				error("Scene::ProcessSceneResources(): Duplicate background mask resource encountered");
			}
			debug(3, "Loading BACKGROUND MASK resource.");
			_bgMask.res_buf = resourceData;
			_bgMask.res_len = resourceDataLength;
			_bgMask.loaded = 1;
			_vm->decodeBGImage(_bgMask.res_buf, _bgMask.res_len, &_bgMask.buf,
							   &_bgMask.buf_len, &_bgMask.w, &_bgMask.h, true);

			// At least in ITE the mask needs to be clipped.

			_bgMask.w = MIN(_bgMask.w, _vm->getDisplayInfo().width);
			_bgMask.h = MIN(_bgMask.h, getHeight());

			debug(4, "BACKGROUND MASK width=%d height=%d length=%d", _bgMask.w, _bgMask.h, (int)_bgMask.buf_len);
			break;
		case SAGA_STRINGS:
			debug(3, "Loading scene strings resource...");
			_vm->loadStrings(_sceneStrings, resourceData, resourceDataLength);
			break;
		case SAGA_OBJECT_MAP:
			debug(3, "Loading object map resource...");
			_objectMap->load(resourceData, resourceDataLength);
			break;
		case SAGA_ACTION_MAP:
			debug(3, "Loading action map resource...");
			_actionMap->load(resourceData, resourceDataLength);
			break;
		case SAGA_ISO_IMAGES:
			if (!(_sceneDescription.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(3, "Loading isometric images resource.");

			_vm->_isoMap->loadImages(resourceData, resourceDataLength);
			break;
		case SAGA_ISO_MAP:
			if (!(_sceneDescription.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(3, "Loading isometric map resource.");

			_vm->_isoMap->loadMap(resourceData, resourceDataLength);
			break;
		case SAGA_ISO_PLATFORMS:
			if (!(_sceneDescription.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(3, "Loading isometric platforms resource.");

			_vm->_isoMap->loadPlatforms(resourceData, resourceDataLength);
			break;
		case SAGA_ISO_METATILES:
			if (!(_sceneDescription.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(3, "Loading isometric metatiles resource.");

			_vm->_isoMap->loadMetaTiles(resourceData, resourceDataLength);
			break;
		case SAGA_ANIM:
			{
				uint16 animId = _resourceList[i].resourceType - 14;

				debug(3, "Loading animation resource animId=%i", animId);

				_vm->_anim->load(animId, resourceData, resourceDataLength);
			}
			break;
		case SAGA_ENTRY:
			debug(3, "Loading entry list resource...");
			loadSceneEntryList(resourceData, resourceDataLength);
			break;
		case SAGA_ISO_MULTI:
			if (!(_sceneDescription.flags & kSceneFlagISO)) {
				error("Scene::ProcessSceneResources(): not Iso mode");
			}

			debug(3, "Loading isometric multi resource.");

			_vm->_isoMap->loadMulti(resourceData, resourceDataLength);
			break;
		case SAGA_PAL_ANIM:
			debug(3, "Loading palette animation resource.");
			_vm->_palanim->loadPalAnim(resourceData, resourceDataLength);
			break;
		case SAGA_FACES:
			if (_vm->getGameId() == GID_ITE)
				_vm->_interface->loadScenePortraits(_resourceList[i].resourceId);
			break;
		case SAGA_PALETTE:
			{
				PalEntry pal[PAL_ENTRIES];
				byte *palPtr = resourceData;

				if (resourceDataLength < 3 * PAL_ENTRIES)
					error("Too small scene palette %i", (int)resourceDataLength);

				for (uint16 c = 0; c < PAL_ENTRIES; c++) {
					pal[c].red = *palPtr++;
					pal[c].green = *palPtr++;
					pal[c].blue = *palPtr++;
				}
				_vm->_gfx->setPalette(pal);
			}
			break;
		default:
			error("Scene::ProcessSceneResources() Encountered unknown resource type %i", _resourceList[i].resourceType);
			break;
		}
	}
}

void Scene::draw() {
	// Do nothing for SAGA2 games for now
	if (_vm->isSaga2()) {
		return;
	}

	if (_sceneDescription.flags & kSceneFlagISO) {
		_vm->_isoMap->adjustScroll(false);
		_vm->_isoMap->draw();
	} else {
		Rect rect;
		_vm->_render->getBackGroundSurface()->getRect(rect);
		rect.bottom = (_sceneClip.bottom < rect.bottom) ? getHeight() : rect.bottom;
		if (_vm->_render->isFullRefresh())
			_vm->_gfx->drawRegion(rect, (const byte *)_vm->_render->getBackGroundSurface()->pixels);
		else
			_vm->_gfx->drawBgRegion(rect, (const byte *)_vm->_render->getBackGroundSurface()->pixels);
	}
}

void Scene::endScene() {
	Rect rect;
	size_t i;

	if (!_sceneLoaded)
		return;

	debug(3, "Ending scene...");

	if (_sceneProc != NULL) {
		_sceneProc(SCENE_END, this);
	}

	// Stop showing actors till the next scene's background is drawn from loadScene
	_vm->_render->setFlag(RF_DISABLE_ACTORS);

	_vm->_script->abortAllThreads();
	_vm->_script->_skipSpeeches = false;

	// Copy current screen to render buffer so inset rooms will get proper background
	if (!(_sceneDescription.flags & kSceneFlagISO) && !_vm->_scene->isInIntro()) {
		BGInfo bgInfo;

		_vm->_scene->getBGInfo(bgInfo);
		_vm->_render->getBackGroundSurface()->blit(bgInfo.bounds, bgInfo.buffer);
		_vm->_render->addDirtyRect(bgInfo.bounds);
	} else {
		_vm->_gfx->getBackBufferRect(rect);
		_vm->_render->getBackGroundSurface()->blit(rect, (const byte *)_vm->_gfx->getBackBufferPixels());
		_vm->_render->addDirtyRect(rect);
	}

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
	for (i = 0; i < _resourceListCount; i++) {
		free(_resourceList[i].buffer);
	}

	if (_loadDescription) {
		free(_resourceList);
	}

	// Free animation info list
	_vm->_anim->reset();

	_vm->_palanim->freePalAnim();

	_objectMap->freeMem();
	_actionMap->freeMem();
	_entryList.freeMem();
	_sceneStrings.freeMem();

	if (_vm->getGameId() == GID_ITE)
		_vm->_isoMap->freeMem();

	_vm->_events->clearList();
	_textList.clear();

	_sceneLoaded = false;
}

void Scene::restoreScene() {
	// There is no implementation for tiled scenes, since this function is only used
	// in IHNM, which has no tiled scenes

	Event event;

	_vm->_gfx->showCursor(false);
	_vm->_gfx->restorePalette();

	event.type = kEvTImmediate;
	event.code = kBgEvent;
	event.op = kEventDisplay;
	event.param = kEvPNoSetPalette;
	event.time = 0;
	event.duration = 0;
	_vm->_events->queue(&event);

	_vm->_gfx->showCursor(true);
}

void Scene::cmdSceneChange(int argc, const char **argv) {
	int scene_num = 0;

	scene_num = atoi(argv[1]);

	if ((scene_num < 1) || (scene_num >= _sceneCount)) {
		_vm->_console->DebugPrintf("Invalid scene number.\n");
		return;
	}

	clearSceneQueue();

	changeScene(scene_num, 0, kTransitionNoFade);
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

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, _sceneContext->isBigEndian());


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

void Scene::clearPlacard() {
	static PalEntry cur_pal[PAL_ENTRIES];
	Event event;
	Event *q_event;

	_vm->_interface->setFadeMode(kFadeOut);

	// Fade to black out
	_vm->_gfx->getCurrentPal(cur_pal);
	event.type = kEvTImmediate;
	event.code = kPalEvent;
	event.op = kEventPalToBlack;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = cur_pal;
	q_event = _vm->_events->queue(&event);

	// set fade mode
	event.type = kEvTImmediate;
	event.code = kInterfaceEvent;
	event.op = kEventSetFadeMode;
	event.param = kNoFade;
	event.time = 0;
	event.duration = 0;
	q_event = _vm->_events->chain(q_event, &event);

	if (_vm->getGameId() == GID_ITE) {
		event.type = kEvTOneshot;
		event.code = kTextEvent;
		event.op = kEventRemove;
		event.data = _vm->_script->getPlacardTextEntry();
		q_event = _vm->_events->chain(q_event, &event);
	} else {
		_vm->_scene->_textList.clear();
	}

	event.type = kEvTImmediate;
	event.code = kInterfaceEvent;
	event.op = kEventRestoreMode;
	event.time = 0;
	event.duration = 0;
	q_event = _vm->_events->chain(q_event, &event);

#ifdef ENABLE_IHNM
	if (_vm->getGameId() == GID_IHNM) {
		// set mode to main
		event.type = kEvTImmediate;
		event.code = kInterfaceEvent;
		event.op = kEventSetMode;
		event.param = kPanelMain;
		event.time = 0;
		event.duration = 0;
		q_event = _vm->_events->chain(q_event, &event);
	}
#endif

	// Display scene background, but stay with black palette
	event.type = kEvTImmediate;
	event.code = kBgEvent;
	event.op = kEventDisplay;
	event.param = kEvPNoSetPalette;
	event.time = 0;
	event.duration = 0;
	q_event = _vm->_events->chain(q_event, &event);

	// set fade mode
	event.type = kEvTImmediate;
	event.code = kInterfaceEvent;
	event.op = kEventSetFadeMode;
	event.param = kFadeIn;
	event.time = 0;
	event.duration = 0;
	q_event = _vm->_events->chain(q_event, &event);

	// Fade in from black to the scene background palette
	event.type = kEvTImmediate;
	event.code = kPalEvent;
	event.op = kEventBlackToPal;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = _bg.pal;
	q_event = _vm->_events->chain(q_event, &event);

	// set fade mode
	event.type = kEvTImmediate;
	event.code = kInterfaceEvent;
	event.op = kEventSetFadeMode;
	event.param = kNoFade;
	event.time = 0;
	event.duration = 0;
	q_event = _vm->_events->chain(q_event, &event);

	event.type = kEvTOneshot;
	event.code = kCursorEvent;
	event.op = kEventShow;
	q_event = _vm->_events->chain(q_event, &event);

	event.type = kEvTOneshot;
	event.code = kScriptEvent;
	event.op = kEventThreadWake;
	event.param = kWaitTypePlacard;
	q_event = _vm->_events->chain(q_event, &event);
}

void Scene::showPsychicProfile(const char *text) {
	int textHeight;
	static PalEntry cur_pal[PAL_ENTRIES];
	PalEntry *pal;
	TextListEntry textEntry;
	Event event;
	Event *q_event;

	if (_vm->_interface->getMode() == kPanelPlacard)
		return;

	_vm->_interface->rememberMode();
	_vm->_interface->setMode(kPanelPlacard);
	_vm->_gfx->savePalette();

	event.type = kEvTOneshot;
	event.code = kCursorEvent;
	event.op = kEventHide;
	q_event = _vm->_events->queue(&event);

	_vm->_interface->setFadeMode(kFadeOut);

	// Fade to black out
	_vm->_gfx->getCurrentPal(cur_pal);
	event.type = kEvTImmediate;
	event.code = kPalEvent;
	event.op = kEventPalToBlack;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = cur_pal;
	q_event = _vm->_events->chain(q_event, &event);

	// set fade mode
	event.type = kEvTImmediate;
	event.code = kInterfaceEvent;
	event.op = kEventSetFadeMode;
	event.param = kNoFade;
	event.time = 0;
	event.duration = 0;
	q_event = _vm->_events->chain(q_event, &event);

	event.type = kEvTOneshot;
	event.code = kInterfaceEvent;
	event.op = kEventClearStatus;
	q_event = _vm->_events->chain(q_event, &event);

	// Set the background and palette for the psychic profile
	event.type = kEvTOneshot;
	event.code = kPsychicProfileBgEvent;
	q_event = _vm->_events->chain(q_event, &event);

	_vm->_scene->_textList.clear();

	if (text != NULL) {
		textHeight = _vm->_font->getHeight(kKnownFontVerb, text, 226, kFontCentered);

		textEntry.knownColor = kKnownColorBlack;
		textEntry.useRect = true;
		textEntry.rect.left = 245;
		textEntry.rect.setHeight(210 + 76);
		textEntry.rect.setWidth(226);
		textEntry.rect.top = 210 - textHeight;
		textEntry.font = kKnownFontVerb;
		textEntry.flags = (FontEffectFlags)(kFontCentered);
		textEntry.text = text;

		TextListEntry *_psychicProfileTextEntry = _vm->_scene->_textList.addEntry(textEntry);

		event.type = kEvTOneshot;
		event.code = kTextEvent;
		event.op = kEventDisplay;
		event.data = _psychicProfileTextEntry;
		q_event = _vm->_events->chain(q_event, &event);
	}

	_vm->_scene->getBGPal(pal);

	event.type = kEvTImmediate;
	event.code = kPalEvent;
	event.op = kEventBlackToPal;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = pal;
	q_event = _vm->_events->chain(q_event, &event);

	event.type = kEvTOneshot;
	event.code = kScriptEvent;
	event.op = kEventThreadWake;
	event.param = kWaitTypePlacard;
	q_event = _vm->_events->chain(q_event, &event);
}

void Scene::clearPsychicProfile() {
	if (_vm->_interface->getMode() == kPanelPlacard || _vm->getFeatures() & GF_IHNM_DEMO) {
		_vm->_render->setFlag(RF_DISABLE_ACTORS);
		_vm->_scene->clearPlacard();
		_vm->_interface->activate();
	}
}

void Scene::showIHNMDemoSpecialScreen() {
	_vm->_gfx->showCursor(true);
	_vm->_interface->clearInventory();
	_vm->_scene->changeScene(150, 0, kTransitionFade);
}

} // End of namespace Saga
