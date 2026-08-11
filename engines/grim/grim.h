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

#ifndef GRIM_ENGINE_H
#define GRIM_ENGINE_H

#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "common/str-array.h"
#include "common/hashmap.h"
#include "common/events.h"

#include "graphics/renderer.h"

#include "engines/grim/textobject.h"
#include "engines/grim/iris.h"
#include "engines/grim/detection.h"

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
class LuaBase;
class Commentary;
class GfxBase;

struct ControlDescriptor {
	const char *name;
	int key;
};

class GrimEngine : public Engine {

protected:
	// Engine APIs
	Common::Error run() override;

public:
	enum EngineMode {
		PauseMode = 1,
		NormalMode = 2,
		SmushMode = 3,
		DrawMode = 4,
		OverworldMode = 5
	};
	enum SpeechMode {
		TextOnly = 1,
		VoiceOnly = 2,
		TextAndVoice = 3
	};

	GrimEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language);
	virtual ~GrimEngine();

	void clearPools();

	uint32 getGameFlags() { return _gameFlags; }
	GrimGameType getGameType() { return _gameType; }
	Common::Language getGameLanguage() { return _gameLanguage; }
	Common::Platform getGamePlatform() { return _gamePlatform; }
	virtual const char *getUpdateFilename();
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override { return true; }
	Common::Error loadGameState(int slot) override;
	bool isRemastered() const { return !!(_gameFlags & ADGF_REMASTERED); }

	void setMode(EngineMode mode);
	EngineMode getMode() { return _mode; }
	void setPreviousMode(EngineMode mode) { _previousMode = mode; }
	EngineMode getPreviousMode() { return _previousMode; }
	void setSpeechMode(SpeechMode mode) { _speechMode = mode; }
	SpeechMode getSpeechMode() { return _speechMode; }
	SaveGame *savedState() { return _savedState; }
	bool getJustSaveLoaded() { return _justSaveLoaded; }
	void setJustSaveLoaded(bool state) { _justSaveLoaded = state; }

	void handleDebugLoadResource();
	void luaUpdate();
	void updateDisplayScene();
	void doFlip();
	void setFlipEnable(bool state) { _flipEnable = state; }
	bool getFlipEnable() { return _flipEnable; }
	virtual void drawTextObjects();
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
	void makeCurrentSetup(int num);

	void flagRefreshShadowMask(bool flag) { _refreshShadowMask = flag; }
	bool getFlagRefreshShadowMask() { return _refreshShadowMask; }

	void setSelectedActor(Actor *a) { _selectedActor = a; }
	Actor *getSelectedActor() { return _selectedActor; }

	/**
	 * Tell the engine that an actor has been moved into/outside a set,
	 * and so that it should rebuild the list of active ones.
	 */
	virtual void invalidateActiveActorsList();
	virtual void invalidateTextObjectsSortOrder() {};
	/**
	 * Return a list of the currently active actors, i. e. the actors in the current set.
	 */
	const Common::List<Actor *> &getActiveActors() const { return _activeActors; }

	/**
	 * Add an actor to the list of actors that are talking
	 */
	void addTalkingActor(Actor *actor);
	inline const Common::List<Actor *> &getTalkingActors() const { return _talkingActors; }
	bool areActorsTalking() const;
	void immediatelyRemoveActor(Actor *actor);

	void drawMovieSubtitle();
	void setMovieSubtitle(TextObject *to);
	void setMovieSetup();

	int getLanguage() const { return _language; }
	void setLanguage(int langId) { _language = langId; }
	Common::String getLanguagePrefix() const;

	bool isConceptEnabled(uint32 number) const;
	void enableConcept(uint32 number);

	bool isCutsceneEnabled(uint32 number) const;
	void enableCutscene(uint32 number);

	Commentary *getCommentary() { return _commentary; }

	Graphics::RendererType getRendererType();

	// TODO: Refactor.
	void setSaveMetaData(const char*, int, const char*);

	void saveGame(const Common::String &file);
	void loadGame(const Common::String &file);

	void changeHardwareState();

	// Engine APIs
	bool hasFeature(EngineFeature f) const override;

	static Common::Array<Common::Keymap *> initKeymapsGrim(const char *target);
	static Common::Array<Common::Keymap *> initKeymapsEMI(const char *target);

	Common::StringArray _listFiles;
	Common::StringArray::const_iterator _listFilesIter;

	TextObjectDefaults _sayLineDefaults, _printLineDefaults, _blastTextDefaults;

	void debugLua(const Common::String &str);

protected:
	void pauseEngineIntern(bool pause) override;

	void handleControls(Common::EventType type, const Common::KeyState &key);
	void handleChars(Common::EventType type, const Common::KeyState &key);
	void handleJoyAxis(byte axis, int16 position);
	void handleMouseAxis(byte axis, int16 position);
	void handleJoyButton(Common::EventType type, byte button);
	void handleExit();
	void handlePause();
	void handleUserPaint();
	void cameraChangeHandle(int prev, int next);
	void cameraPostChangeHandle(int num);
	void buildActiveActorsList();
	void savegameCallback();
	GfxBase *createRenderer(int screenW, int screenH);
	void playAspyrLogo();
	virtual LuaBase *createLua();
	virtual void updateNormalMode();
	virtual void updateDrawMode();
	virtual void drawNormalMode();

	void savegameSave();
	void saveGRIM();

	void savegameRestore();
	void restoreGRIM();

	virtual void storeSaveGameMetadata(SaveGame *state);
	virtual void storeSaveGameImage(SaveGame *savedState);

	bool _savegameLoadRequest = false;
	bool _savegameSaveRequest = false;
	Common::String _savegameFileName;
	SaveGame *_savedState;
	bool _justSaveLoaded;

	Set *_currSet;
	EngineMode _mode, _previousMode;
	SpeechMode _speechMode;
	int _textSpeed;
	bool _flipEnable;
	char _fps[8];
	bool _doFlip;
	bool _refreshShadowMask = false;
	bool _shortFrame = false;
	bool _setupChanged = true;
	// This holds the name of the setup in which the movie must be drawed
	Common::String _movieSetup;

	unsigned _frameStart = 0, _frameTime = 0, _movieTime = 0;
	int _prevSmushFrame = 0;
	unsigned int _frameCounter = 0;
	unsigned int _lastFrameTime = 0;
	unsigned _speedLimitMs;
	bool _showFps;
	bool _softRenderer;

	bool *_controlsEnabled;
	bool *_controlsState;
	float *_joyAxisPosition;

	bool _changeHardwareState = false;

	Actor *_selectedActor;
	Iris *_iris;
	TextObject::Ptr _movieSubtitle;

	bool _buildActiveActorsList;
	Common::List<Actor *> _activeActors;
	Common::List<Actor *> _talkingActors;

	uint32 _gameFlags;
	GrimGameType _gameType;
	Common::Platform _gamePlatform;
	Common::Language _gameLanguage;
	uint32 _pauseStartTime;

	// Remastered;
	uint32 _language;
	static const uint32 kNumConcepts = 98;
	static const uint32 kNumCutscenes = 40;
	bool _cutsceneEnabled[kNumCutscenes]; // TODO, could probably use a different data structure
	bool _conceptEnabled[kNumConcepts];

	Common::String _saveMeta1;
	int _saveMeta2 = 0;
	Common::String _saveMeta3;

	Commentary *_commentary;

public:
	int _cursorX = 0;
	int _cursorY = 0;
	bool _isUtf8 = false;
	bool _transcodeChineseToSimplified = false;
	Font *_overrideFont = nullptr;
};

extern GrimEngine *g_grim;

extern int g_imuseState;

// Fake KEYCODE_* values for joystick and mouse events

enum {
	KEYCODE_JOY1_A = 512,
	KEYCODE_JOY1_B,
	KEYCODE_JOY1_X,
	KEYCODE_JOY1_Y,
	KEYCODE_JOY1_BACK,
	KEYCODE_JOY1_GUIDE,
	KEYCODE_JOY1_START,
	KEYCODE_JOY1_LEFTSTICK,
	KEYCODE_JOY1_RIGHTSTICK,
	KEYCODE_JOY1_L1,
	KEYCODE_JOY1_R1,
	KEYCODE_JOY1_HUP,
	KEYCODE_JOY1_HDOWN,
	KEYCODE_JOY1_HLEFT,
	KEYCODE_JOY1_HRIGHT,
	KEYCODE_JOY1_L2,
	KEYCODE_JOY1_R2,
	KEYCODE_JOY1_B14,
	KEYCODE_JOY1_B15,
	KEYCODE_JOY1_B16,
	KEYCODE_JOY1_B17,
	KEYCODE_JOY1_B18,
	KEYCODE_JOY1_B19,
	KEYCODE_JOY1_B20,
	KEYCODE_JOY2_A,
	KEYCODE_JOY2_B,
	KEYCODE_JOY2_X,
	KEYCODE_JOY2_Y,
	KEYCODE_JOY2_BACK,
	KEYCODE_JOY2_GUIDE,
	KEYCODE_JOY2_START,
	KEYCODE_JOY2_LEFTSTICK,
	KEYCODE_JOY2_RIGHTSTICK,
	KEYCODE_JOY2_L1,
	KEYCODE_JOY2_R1,
	KEYCODE_JOY2_HUP,
	KEYCODE_JOY2_HDOWN,
	KEYCODE_JOY2_HLEFT,
	KEYCODE_JOY2_HRIGHT,
	KEYCODE_JOY2_L2,
	KEYCODE_JOY2_R2,
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

#define NUM_JOY_AXES (KEYCODE_AXIS_JOY1_V - KEYCODE_AXIS_JOY1_X + 1)
#define NUM_JOY_BUTTONS (KEYCODE_JOY1_R2 - KEYCODE_JOY1_A + 1)

extern const ControlDescriptor controls[];

} // end of namespace Grim

#endif
