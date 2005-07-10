// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
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
#include <SDL_keysym.h>
#include <zlib.h>

class Actor;

#define ENGINE_MODE_IDLE	0
#define ENGINE_MODE_PAUSE	1
#define ENGINE_MODE_NORMAL	2
#define ENGINE_MODE_SMUSH	3
#define ENGINE_MODE_DRAW	4

// Fake SDLK_* values for joystick and mouse events
enum {
	SDLK_JOY1_B1 = SDLK_LAST,
	SDLK_JOY1_B2,
	SDLK_JOY1_B3,
	SDLK_JOY1_B4,
	SDLK_JOY1_B5,
	SDLK_JOY1_B6,
	SDLK_JOY1_B7,
	SDLK_JOY1_B8,
	SDLK_JOY1_B9,
	SDLK_JOY1_B10,
	SDLK_JOY1_HLEFT,
	SDLK_JOY1_HUP,
	SDLK_JOY1_HRIGHT,
	SDLK_JOY1_HDOWN,
	SDLK_JOY2_B1,
	SDLK_JOY2_B2,
	SDLK_JOY2_B3,
	SDLK_JOY2_B4,
	SDLK_JOY2_B5,
	SDLK_JOY2_B6,
	SDLK_JOY2_B7,
	SDLK_JOY2_B8,
	SDLK_JOY2_B9,
	SDLK_JOY2_B10,
	SDLK_JOY2_HLEFT,
	SDLK_JOY2_HUP,
	SDLK_JOY2_HRIGHT,
	SDLK_JOY2_HDOWN,
	SDLK_MOUSE_B1,
	SDLK_MOUSE_B2,
	SDLK_MOUSE_B3,
	SDLK_MOUSE_B4,
	SDLK_AXIS_JOY1_X,
	SDLK_AXIS_JOY1_Y,
	SDLK_AXIS_JOY1_Z,
	SDLK_AXIS_JOY1_R,
	SDLK_AXIS_JOY1_U,
	SDLK_AXIS_JOY1_V,
	SDLK_AXIS_JOY2_X,
	SDLK_AXIS_JOY2_Y,
	SDLK_AXIS_JOY2_Z,
	SDLK_AXIS_JOY2_R,
	SDLK_AXIS_JOY2_U,
	SDLK_AXIS_JOY2_V,
	SDLK_AXIS_MOUSE_X,
	SDLK_AXIS_MOUSE_Y,
	SDLK_AXIS_MOUSE_Z,
	SDLK_EXTRA_LAST
};

class Engine {
public:

	void setMode(int mode) { _mode = mode; }
	void setSpeechMode(int mode) { _speechMode = mode; }
	int getSpeechMode() { return _speechMode; }

	void mainLoop();
	unsigned frameStart() const { return _frameStart; }
	unsigned frameTime() const { return _frameTime; }

	float perSecond(float rate) const { return rate * _frameTime / 1000; }

	int getTextSpeed() { return _textSpeed; }
	void setTextSpeed(int speed);
	void setMenuMode(int mode) { _menuMode = mode; }
	int getMenuMode() { return _menuMode; }

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
	static void savegameGzread(void *data, int size);
	static void savegameGzwrite(void *data, int size);
	void savegameCallback();

	bool _savegameLoadRequest;
	bool _savegameSaveRequest;
	char *_savegameFileName;
	gzFile _savegameFileHandle;

	Engine();
	~Engine() {}

private:

	void handleButton(int operation, int key);

	Scene *_currScene;
	int _mode, _menuMode;
	int _speechMode;
	int _textSpeed;

	unsigned _frameStart, _frameTime, _movieTime;

	bool _controlsEnabled[SDLK_EXTRA_LAST];

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

extern DIR *g_searchFile;

#endif

void vimaInit(uint16 *destTable);
void decompressVima(const byte *src, int16 *dest, int destLen, uint16 *destTable);

#endif
