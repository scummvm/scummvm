/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_ENGINE_H
#define GRIM_ENGINE_H

#include "engines/engine.h"

#include "engines/grim/textobject.h"

namespace Grim {

class Actor;
class SaveGame;

#define ENGINE_MODE_IDLE	0
#define ENGINE_MODE_PAUSE	1
#define ENGINE_MODE_NORMAL	2
#define ENGINE_MODE_SMUSH	3
#define ENGINE_MODE_DRAW	4

enum GrimGameType {
	GType_GRIM = 1,
	GType_MONKEY
};

enum GrimGameFeatures {
	GF_DEMO =   1 << 0
};

struct GrimGameDescription;

typedef Common::HashMap<Common::String, const char *> StringPtrHashMap;

extern int g_flags;

#define GF_DEMO		1

struct ControlDescriptor {
	const char *name;
	int key;
};

class GrimEngine : public Engine {

protected:
	// Engine APIs
	virtual Common::Error run();

public:

	GrimEngine(OSystem *syst, const GrimGameDescription *gameDesc);
	virtual ~GrimEngine();

	bool loadSaveDirectory(void);
	void makeSystemMenu(void);
	int modifyGameSpeed(int speedChange);
	int getTimerDelay() const;

	const GrimGameDescription *_gameDescription;

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
	float getControlAxis(int num);
	bool getControlState(int num);

	Scene *findScene(const char *name);
	void setSceneLock(const char *name, bool lockStatus);
	void setScene(const char *name);
	void setScene(Scene *scene);
	Scene *currScene() { return _currScene; }
	const char *sceneName() const { return _currScene->name(); }
	void makeCurrentSetup(int num);

	// Scene registration
	typedef Common::List<Scene *> SceneListType;
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

	void flagRefreshShadowMask(bool flag) {
		_refreshShadowMask = flag;
	}
	bool getFlagRefreshShadowMask() {
		return _refreshShadowMask;
	}

	// Actor registration
	typedef Common::List<Actor *> ActorListType;
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
	typedef Common::List<TextObject *> TextListType;
	TextListType::const_iterator textsBegin() const {
		return _textObjects.begin();
	}
	TextListType::const_iterator textsEnd() const {
		return _textObjects.end();
	}
	void registerTextObject(TextObject *a) { _textObjects.push_back(a); }
	void killTextObject(TextObject *a) {
		_textObjects.remove(a);
		delete a;
	}
	void killTextObjects() {
		while (!_textObjects.empty()) {
			delete _textObjects.back();
			_textObjects.pop_back();
		}
	}

	// Primitives Object Registration
	typedef Common::List<PrimitiveObject *> PrimitiveListType;
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
	static void savegameReadStream(void *data, int32 size);
	static void savegameWriteStream(void *data, int32 size);
	static int32 savegameReadSint32();
	static void savegameWriteSint32(int32 val);
	static uint32 savegameReadUint32();
	static void savegameWriteUint32(uint32 val);

	void storeSaveGameImage(SaveGame *savedState);

	bool _savegameLoadRequest;
	bool _savegameSaveRequest;
	const char *_savegameFileName;
	SaveGame *_savedState;

private:

	void handleControls(int operation, int key, int keyModifier, uint16 ascii);
	void handleChars(int operation, int key, int keyModifier, uint16 ascii);
	void handleUserPaint();
	void handleExit();
	void handlePause();

	Scene *_currScene;
	int _mode, _previousMode;
	int _speechMode;
	int _textSpeed;
	bool _flipEnable;
	bool _refreshDrawNeeded;
	char _fps[8];
	bool _doFlip;
	bool _refreshShadowMask;

	unsigned _frameStart, _frameTime, _movieTime;
	unsigned int _frameTimeCollection;
	int _prevSmushFrame;
	unsigned int _frameCounter;
	unsigned int _timeAccum;
	unsigned _speedLimitMs;
	bool _showFps;
	bool _softRenderer;

	bool *_controlsEnabled;
	bool *_controlsState;

	SceneListType _scenes;
	ActorListType _actors;
	Actor *_selectedActor;
	TextListType _textObjects;
	PrimitiveListType _primitiveObjects;
};

extern GrimEngine *g_grim;

extern int g_imuseState;

extern Actor *g_currentUpdatedActor;

void vimaInit(uint16 *destTable);
void decompressVima(const byte *src, int16 *dest, int destLen, uint16 *destTable);

// Fake KEYCODE_* values for joystick and mouse events

enum {
	KEYCODE_JOY1_B1 = 512,
	KEYCODE_JOY1_B2,
	KEYCODE_JOY1_B3,
	KEYCODE_JOY1_B4,
	KEYCODE_JOY1_B5,
	KEYCODE_JOY1_B6,
	KEYCODE_JOY1_B7,
	KEYCODE_JOY1_B8,
	KEYCODE_JOY1_B9,
	KEYCODE_JOY1_B10,
	KEYCODE_JOY1_HLEFT,
	KEYCODE_JOY1_HUP,
	KEYCODE_JOY1_HRIGHT,
	KEYCODE_JOY1_HDOWN,
	KEYCODE_JOY2_B1,
	KEYCODE_JOY2_B2,
	KEYCODE_JOY2_B3,
	KEYCODE_JOY2_B4,
	KEYCODE_JOY2_B5,
	KEYCODE_JOY2_B6,
	KEYCODE_JOY2_B7,
	KEYCODE_JOY2_B8,
	KEYCODE_JOY2_B9,
	KEYCODE_JOY2_B10,
	KEYCODE_JOY2_HLEFT,
	KEYCODE_JOY2_HUP,
	KEYCODE_JOY2_HRIGHT,
	KEYCODE_JOY2_HDOWN,
	KEYCODE_MOUSE_B1,
	KEYCODE_MOUSE_B2,
	KEYCODE_MOUSE_B3,
	KEYCODE_MOUSE_B4,
	KEYCODE_AXIS_JOY1_X,
	KEYCODE_AXIS_JOY1_Y,
	KEYCODE_AXIS_JOY1_Z,
	KEYCODE_AXIS_JOY1_R,
	KEYCODE_AXIS_JOY1_U,
	KEYCODE_AXIS_JOY1_V,
	KEYCODE_AXIS_JOY2_X,
	KEYCODE_AXIS_JOY2_Y,
	KEYCODE_AXIS_JOY2_Z,
	KEYCODE_AXIS_JOY2_R,
	KEYCODE_AXIS_JOY2_U,
	KEYCODE_AXIS_JOY2_V,
	KEYCODE_AXIS_MOUSE_X,
	KEYCODE_AXIS_MOUSE_Y,
	KEYCODE_AXIS_MOUSE_Z,
	KEYCODE_EXTRA_LAST
};

extern const ControlDescriptor controls[];

} // end of namespace Grim

#endif
