// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef ENGINE_H
#define ENGINE_H

#include "bits.h"
#include "scene.h"
#include "textobject.h"
#include "primitives.h"
#include "font.h"
#include "lua.h"

#include <cstdlib>
#include <list>

class Actor;

#define ENGINE_MODE_IDLE	0
#define ENGINE_MODE_PAUSE	1
#define ENGINE_MODE_NORMAL	2
#define ENGINE_MODE_SMUSH	3
#define ENGINE_MODE_DRAW	4

extern int g_flags;

#define GF_DEMO		1

class Engine {
public:

	void setMode(int mode) { _mode = mode; }
	int getMode() { return _mode; }
	void setPreviousMode(int mode) { _previousMode = mode; }
	int getPreviousMode() { return _previousMode; }
	void setSpeechMode(int mode) { _speechMode = mode; }
	int getSpeechMode() { return _speechMode; }
	SaveGame *savedState() { return _savedState; }

	void handleDebugLoadResource();
	void luaUpdate();
	void updateDisplayScene();
	void doFlip();
	void setFlipEnable(bool state) { _flipEnable = state; }
	bool getFlipEnable() { return _flipEnable; }
	void refreshDrawMode() { _refreshDrawNeeded = true; }
	void drawPrimitives();

	void mainLoop();
	unsigned frameStart() const { return _frameStart; }
	unsigned frameTime() const { return _frameTime; }

	// perSecond should allow rates of zero, some actors will accelerate
	// up to their normal speed (such as the bone wagon) so handling
	// a walking rate of zero should happen in the default actor creation
	float perSecond(float rate) const { return rate * _frameTime / 1000; }

	int getTextSpeed() { return _textSpeed; }
	void setTextSpeed(int speed);

	void enableControl(int num) { _controlsEnabled[num] = true; }
	void disableControl(int num) { _controlsEnabled[num] = false; }

	Scene *findScene(const char *name);
	void setSceneLock(const char *name, bool lockStatus);
	void setScene(const char *name);
	void setScene(Scene *scene);
	Scene *currScene() { return _currScene; }
	const char *sceneName() const { return _currScene->name(); }

	// Scene registration
	typedef std::list<Scene *> SceneListType;
	SceneListType::const_iterator scenesBegin() const {
		return _scenes.begin();
	}
	SceneListType::const_iterator scenesEnd() const {
		return _scenes.end();
	}
	void registerScene(Scene *a) { _scenes.push_back(a); }
	void removeScene(Scene *a) {
		_scenes.remove(a);
	}

	// Actor registration
	typedef std::list<Actor *> ActorListType;
	ActorListType::const_iterator actorsBegin() const {
		return _actors.begin();
	}
	ActorListType::const_iterator actorsEnd() const {
		return _actors.end();
	}
	void registerActor(Actor *a) { _actors.push_back(a); }
	void killActor(Actor *a) { _actors.remove(a); }
	void setSelectedActor(Actor *a) { _selectedActor = a; }
	Actor *selectedActor() { return _selectedActor; }

	// Text Object Registration
	typedef std::list<TextObject *> TextListType;
	TextListType::const_iterator textsBegin() const {
		return _textObjects.begin();
	}
	TextListType::const_iterator textsEnd() const {
		return _textObjects.end();
	}
	void registerTextObject(TextObject *a) { _textObjects.push_back(a); }
	void killTextObject(TextObject *a) {
		_textObjects.remove(a);
	}
	void killTextObjects() {
		while (!_textObjects.empty()) {
			delete _textObjects.back();
			_textObjects.pop_back();
		}
	}

	// Primitives Object Registration
	typedef std::list<PrimitiveObject *> PrimitiveListType;
	PrimitiveListType::const_iterator primitivesBegin() const {
		return _primitiveObjects.begin();
	}
	PrimitiveListType::const_iterator primitivesEnd() const {
		return _primitiveObjects.end();
	}

	void registerPrimitiveObject(PrimitiveObject *a) { _primitiveObjects.push_back(a); }
	void killPrimitiveObject(PrimitiveObject *a) {
		_primitiveObjects.remove(a);
	}
	void killPrimitiveObjects() {
		while (!_primitiveObjects.empty()) {
			delete _primitiveObjects.back();
			_primitiveObjects.pop_back();
		}
	}

	void savegameSave();
	void savegameRestore();
	void savegameCallback();

	bool _savegameLoadRequest;
	bool _savegameSaveRequest;
	char *_savegameFileName;
	SaveGame *_savedState;

	Engine();
	~Engine();

private:

	void handleButton(int operation, int key, int keyModifier, uint16 ascii);

	Scene *_currScene;
	int _mode, _previousMode;
	int _speechMode;
	int _textSpeed;
	bool _flipEnable;
	uint32 _lastUpdateTime;
	bool _refreshDrawNeeded;
	char _fps[8];
	bool _doFlip;

	unsigned _frameStart, _frameTime, _movieTime;
	unsigned int _frameTimeCollection;
	int _prevSmushFrame;
	unsigned int _frameCounter;
	unsigned int _timeAccum;

	bool *_controlsEnabled;

	SceneListType _scenes;
	ActorListType _actors;
	Actor *_selectedActor;
	TextListType _textObjects;
	PrimitiveListType _primitiveObjects;
};

extern Engine *g_engine;

extern int g_imuseState;

extern Actor *g_currentUpdatedActor;

#ifdef _WIN32

extern WIN32_FIND_DATAA g_find_file_data;
extern HANDLE g_searchFile;
extern bool g_firstFind;

#else

extern char g_find_file_data[100];
extern DIR *g_searchFile;

#endif

void vimaInit(uint16 *destTable);
void decompressVima(const byte *src, int16 *dest, int destLen, uint16 *destTable);

#endif
