/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
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

namespace Grim {

class Actor;
class SaveGame;
class Bitmap;
class Font;
class Color;
class ObjectState;
class Scene;
class TextObject;
class PrimitiveObject;

enum enDebugLevels {
	DEBUG_NONE, DEBUG_NORMAL, DEBUG_WARN, DEBUG_ERROR, DEBUG_LUA, DEBUG_BITMAPS, DEBUG_MODEL, DEBUG_STUB,
	DEBUG_SMUSH, DEBUG_IMUSE, DEBUG_CHORES, DEBUG_ALL
};

#define ENGINE_MODE_IDLE	0
#define ENGINE_MODE_PAUSE	1
#define ENGINE_MODE_NORMAL	2
#define ENGINE_MODE_SMUSH	3
#define ENGINE_MODE_DRAW	4

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

	typedef Common::HashMap<int32, Bitmap *> BitmapListType;
	typedef Common::HashMap<int32, Font *> FontListType;
	typedef Common::HashMap<int32, Color *> ColorListType;
	typedef Common::HashMap<int32, ObjectState *> StateListType;
	typedef Common::HashMap<int, Scene *> SceneListType;
	typedef Common::HashMap<int, Actor *> ActorListType;
	typedef Common::HashMap<int32, TextObject *> TextListType;
	typedef Common::HashMap<int, PrimitiveObject *> PrimitiveListType;

	GrimEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language);
	virtual ~GrimEngine();

	int getGameFlags() { return _gameFlags; }
	GrimGameType getGameType() { return _gameType; }
	Common::Language getGameLanguage() { return _gameLanguage; }

	bool loadSaveDirectory(void);
	void makeSystemMenu(void);
	int modifyGameSpeed(int speedChange);
	int getTimerDelay() const;

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
	unsigned getFrameStart() const { return _frameStart; }
	unsigned getFrameTime() const { return _frameTime; }

	int bundle_dofile(const char *filename);
	int single_dofile(const char *filename);

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

	Scene *findScene(const Common::String &name);
	void setSceneLock(const char *name, bool lockStatus);
	void setScene(const char *name);
	void setScene(Scene *scene);
	Scene *getCurrScene() { return _currScene; }
	const Common::String &getSceneName() const { return _currScene->getName(); }
	void makeCurrentSetup(int num);

	// Scene registration
	SceneListType::const_iterator scenesBegin() const { return _scenes.begin(); }
	SceneListType::const_iterator scenesEnd() const { return _scenes.end(); }
	void registerScene(Scene *a);
	void removeScene(Scene *a);
	void killScenes();
	int sceneId(Scene *s) const;

	void flagRefreshShadowMask(bool flag) { _refreshShadowMask = flag; }
	bool getFlagRefreshShadowMask() { return _refreshShadowMask; }

	Bitmap *registerBitmap(const char *filename, const char *data, int len);
	Bitmap *registerBitmap(const char *data, int width, int height, const char *filename);
	void registerBitmap(Bitmap *bitmap);
	void killBitmap(Bitmap *b);
	void killBitmaps();
	Bitmap *getBitmap(int32 id) const;

	// Actor registration
	ActorListType::const_iterator actorsBegin() const { return _actors.begin(); }
	ActorListType::const_iterator actorsEnd() const { return _actors.end(); }
	void registerActor(Actor *a);
	void killActor(Actor *a);
	Actor *getActor(int id) const;
	Actor *getTalkingActor() const;
	void setTalkingActor(Actor *actor);

	void setSelectedActor(Actor *a) { _selectedActor = a; }
	Actor *getSelectedActor() { return _selectedActor; }
	void killActors();

	// Text Object Registration
	TextListType::const_iterator textsBegin() const { return _textObjects.begin(); }
	TextListType::const_iterator textsEnd() const { return _textObjects.end(); }
	void registerTextObject(TextObject *a);
	void killTextObject(TextObject *a);
	void killTextObjects();
	TextObject *getTextObject(int id) const;


	// Primitives Object Registration
	PrimitiveListType::const_iterator primitivesBegin() const { return _primitiveObjects.begin(); }
	PrimitiveListType::const_iterator primitivesEnd() const { return _primitiveObjects.end(); }
	void registerPrimitiveObject(PrimitiveObject *a);
	void killPrimitiveObject(PrimitiveObject *a);
	void killPrimitiveObjects();
	PrimitiveObject *getPrimitiveObject(int id) const;

	void registerObjectState(ObjectState *o);
	void killObjectState(ObjectState *o);
	void killObjectStates();
	ObjectState *getObjectState(int id) const;

	void registerFont(Font *f);
	void killFont(Font *f);
	void killFonts();
	Font *getFont(int32 id) const;

	void registerColor(Color *c);
	void killColor(Color *c);
	void killColors();
	Color *getColor(int32 id) const;

	void savegameSave();
	void saveActors(SaveGame *savedState);
	void saveTextObjects(SaveGame *savedState);
	void savePrimitives(SaveGame *savedState);
	void saveScenes(SaveGame *savedState);
	void saveObjectStates(SaveGame *savedState);
	void saveBitmaps(SaveGame *savedState);
	void saveFonts(SaveGame *savedState);
	void saveColors(SaveGame *savedState);

	void savegameRestore();
	void restoreActors(SaveGame *savedState);
	void restoreTextObjects(SaveGame *savedState);
	void restorePrimitives(SaveGame *savedState);
	void restoreScenes(SaveGame *savedState);
	void restoreObjectStates(SaveGame *savedState);
	void restoreBitmaps(SaveGame *savedState);
	void restoreFonts(SaveGame *savedState);
	void restoreColors(SaveGame *savedState);

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
	Common::String _savegameFileName;
	SaveGame *_savedState;

	Common::StringArray _listFiles;
	Common::StringArray::const_iterator _listFilesIter;

	TextObjectDefaults _sayLineDefaults, _printLineDefaults, _blastTextDefaults;

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

	Actor *_selectedActor;
	Actor *_talkingActor;

	SceneListType _scenes;
	ActorListType _actors;
	TextListType _textObjects;
	PrimitiveListType _primitiveObjects;
	BitmapListType _bitmaps;
	StateListType _objectStates;
	FontListType _fonts;
	ColorListType _colors;

	uint32 _gameFlags;
	GrimGameType _gameType;
	Common::Platform _gamePlatform;
	Common::Language _gameLanguage;
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
