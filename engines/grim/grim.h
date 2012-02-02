/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#ifndef GRIM_ENGINE_H
#define GRIM_ENGINE_H

#include "engines/engine.h"

#include "common/str-array.h"
#include "common/hashmap.h"

#include "engines/advancedDetector.h"

#include "engines/grim/textobject.h"
#include "engines/grim/iris.h"

namespace Grim {

class Actor;
class SaveGame;
class Bitmap;
class Font;
class Color;
class ObjectState;
class Set;
class TextObject;
class PrimitiveObject;

enum GrimGameType {
	GType_GRIM,
	GType_MONKEY4
};

struct GrimGameDescription;

typedef Common::HashMap<Common::String, const char *>StringPtrHashMap;

struct ControlDescriptor {
	const char *name;
	int key;
};

class GrimEngine : public Engine {

protected:
	// Engine APIs
	virtual Common::Error run();

public:
	enum EngineMode {
		PauseMode = 1,
		NormalMode = 2,
		SmushMode = 3,
		DrawMode = 4
	};
	enum SpeechMode {
		TextOnly = 1,
		VoiceOnly = 2,
		TextAndVoice = 3
	};

	GrimEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language);
	virtual ~GrimEngine();

	void clearPools();

	int getGameFlags() { return _gameFlags; }
	GrimGameType getGameType() { return _gameType; }
	Common::Language getGameLanguage() { return _gameLanguage; }
	Common::Platform getGamePlatform() { return _gamePlatform; }

	bool loadSaveDirectory(void);
	void makeSystemMenu(void);
	int modifyGameSpeed(int speedChange);
	int getTimerDelay() const;

	void setMode(EngineMode mode) { _mode = mode; }
	EngineMode getMode() { return _mode; }
	void setPreviousMode(EngineMode mode) { _previousMode = mode; }
	EngineMode getPreviousMode() { return _previousMode; }
	void setSpeechMode(SpeechMode mode) { _speechMode = mode; }
	SpeechMode getSpeechMode() { return _speechMode; }
	SaveGame *savedState() { return _savedState; }

	void handleDebugLoadResource();
	void luaUpdate();
	void updateDisplayScene();
	void doFlip();
	void setFlipEnable(bool state) { _flipEnable = state; }
	bool getFlipEnable() { return _flipEnable; }
	void refreshDrawMode() { _refreshDrawNeeded = true; }
	void drawPrimitives();
	void playIrisAnimation(Iris::Direction dir, int x, int y, int time);

	void mainLoop();
	unsigned getFrameStart() const { return _frameStart; }
	unsigned getFrameTime() const { return _frameTime; }

	// perSecond should allow rates of zero, some actors will accelerate
	// up to their normal speed (such as the bone wagon) so handling
	// a walking rate of zero should happen in the default actor creation
	float getPerSecond(float rate) const;

	int getTextSpeed() { return _textSpeed; }
	void setTextSpeed(int speed);

	void enableControl(int num) { _controlsEnabled[num] = true; }
	void disableControl(int num) { _controlsEnabled[num] = false; }
	float getControlAxis(int num);
	bool getControlState(int num);
	void clearEventQueue();

	Set *findSet(const Common::String &name);
	void setSetLock(const char *name, bool lockStatus);
	Set *loadSet(const Common::String &name);
	void setSet(const char *name);
	void setSet(Set *scene);
	Set *getCurrSet() { return _currSet; }
	const Common::String &getSetName() const;
	void makeCurrentSetup(int num);

	void flagRefreshShadowMask(bool flag) { _refreshShadowMask = flag; }
	bool getFlagRefreshShadowMask() { return _refreshShadowMask; }

	Actor *getTalkingActor() const;
	void setTalkingActor(Actor *actor);

	void setSelectedActor(Actor *a) { _selectedActor = a; }
	Actor *getSelectedActor() { return _selectedActor; }

	void saveGame(const Common::String &file);
	void loadGame(const Common::String &file);

	void changeHardwareState();

	// Engine APIs
	bool hasFeature(EngineFeature f) const;

	Common::StringArray _listFiles;
	Common::StringArray::const_iterator _listFilesIter;

	TextObjectDefaults _sayLineDefaults, _printLineDefaults, _blastTextDefaults;

private:
	void handleControls(int operation, int key, int keyModifier, uint16 ascii);
	void handleChars(int operation, int key, int keyModifier, uint16 ascii);
	void handleExit();
	void handlePause();
	void handleUserPaint();
	void cameraChangeHandle(int prev, int next);
	void cameraPostChangeHandle(int num);
	void savegameCallback();

	void savegameSave();
	void saveGRIM();

	void savegameRestore();
	void restoreGRIM();

	void storeSaveGameImage(SaveGame *savedState);

	bool _savegameLoadRequest;
	bool _savegameSaveRequest;
	Common::String _savegameFileName;
	SaveGame *_savedState;

	Set *_currSet;
	EngineMode _mode, _previousMode;
	SpeechMode _speechMode;
	int _textSpeed;
	bool _flipEnable;
	bool _refreshDrawNeeded;
	char _fps[8];
	bool _doFlip;
	bool _refreshShadowMask;
	bool _shortFrame;

	unsigned _frameStart, _frameTime, _movieTime;
	int _prevSmushFrame;
	unsigned int _frameCounter;
	unsigned int _lastFrameTime;
	unsigned _speedLimitMs;
	bool _showFps;
	bool _softRenderer;

	bool *_controlsEnabled;
	bool *_controlsState;

	bool _changeHardwareState;
	bool _changeFullscreenState;

	Actor *_selectedActor;
	Actor *_talkingActor;
	Iris *_iris;

	uint32 _gameFlags;
	GrimGameType _gameType;
	Common::Platform _gamePlatform;
	Common::Language _gameLanguage;
};

extern GrimEngine *g_grim;

extern int g_imuseState;

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
	KEYCODE_JOY1_B11,
	KEYCODE_JOY1_B12,
	KEYCODE_JOY1_B13,
	KEYCODE_JOY1_B14,
	KEYCODE_JOY1_B15,
	KEYCODE_JOY1_B16,
	KEYCODE_JOY1_B17,
	KEYCODE_JOY1_B18,
	KEYCODE_JOY1_B19,
	KEYCODE_JOY1_B20,
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
	KEYCODE_JOY2_B11,
	KEYCODE_JOY2_B12,
	KEYCODE_JOY2_B13,
	KEYCODE_JOY2_B14,
	KEYCODE_JOY2_B15,
	KEYCODE_JOY2_B16,
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
