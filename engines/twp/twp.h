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

#ifndef TWP_H
#define TWP_H

#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"
#include "twp/detection.h"
#include "twp/vm.h"
#include "twp/shaders.h"
#include "twp/resmanager.h"
#include "twp/ggpack.h"
#include "twp/squirrel/squirrel.h"
#include "twp/camera.h"
#include "twp/prefs.h"
#include "twp/tsv.h"
#include "twp/scenegraph.h"
#include "twp/dialog.h"
#include "twp/hud.h"
#include "twp/callback.h"
#include "twp/walkboxnode.h"
#include "twp/audio.h"
#include "twp/actorswitcher.h"
#include "twp/savegame.h"

#define SCREEN_MARGIN 100.f
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define VERBDEFAULT "verbDefault"

namespace Twp {

class Lighting;
class Task;
class ThreadBase;
class Cutscene;
class Scene;
class Room;
class InputState;
class Object;
class Dialog;
struct TwpGameDescription;

class TwpEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	TwpEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~TwpEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets the random source
	 */
	Common::RandomSource &getRandomSource() { return _randomSource; }
	Preferences &getPrefs() { return _prefs; }

	HSQUIRRELVM getVm() { return _vm.get(); }
	inline Gfx &getGfx() { return _gfx; }
	inline TextDb &getTextDb() { return _textDb; }

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
		return true;
	}

	virtual Common::String getSaveStateName(int slot) const override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	Common::Error loadGameState(int slot) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	void capture(Common::WriteStream &stream, Math::Vector2d size);

	Math::Vector2d winToScreen(Math::Vector2d pos);
	Math::Vector2d roomToScreen(Math::Vector2d pos);
	Math::Vector2d screenToRoom(Math::Vector2d pos);

	void setActor(Object *actor, bool userSelected = false);
	Object *objAt(Math::Vector2d pos);
	void flashSelectableActor(int flash);
	void stopTalking();
	void walkFast(bool state = true);

	Room *defineRoom(const Common::String &name, HSQOBJECT table, bool pseudo = false);
	void setRoom(Room *room);
	void enterRoom(Room *room, Object *door = nullptr);

	void cameraAt(Math::Vector2d at);
	// Returns the camera position: the position of the middle of the screen.
	Math::Vector2d cameraPos();
	void follow(Object *actor);
	void fadeTo(FadeEffect effect, float duration, bool fadeToSep = false);

	void execNutEntry(HSQUIRRELVM v, const Common::String &entry);
	void execBnutEntry(HSQUIRRELVM v, const Common::String &entry);
	bool callVerb(Object *actor, VerbId verbId, Object *noun1, Object *noun2 = nullptr);
	bool execSentence(Object *actor, VerbId verbId, Object *noun1, Object *noun2 = nullptr);

	float getRandom() const;
	float getRandom(float min, float max) const;

private:
	void update(float elapsedMs);
	void draw(RenderTexture* texture = nullptr);
	void exitRoom(Room *nextRoom);
	void actorEnter();
	void actorExit();
	void cancelSentence(Object *actor = nullptr);
	void clickedAt(Math::Vector2d scrPos);
	bool clickedAtHandled(Math::Vector2d roomPos);
	void setShaderEffect(RoomEffect effect);
	bool selectable(Object *actor);
	void resetVerb();
	Common::String cursorText();
	Verb verb();
	bool preWalk(Object *actor, VerbId verbId, Object *noun1, Object *noun2);
	void updateTriggers();
	void callTrigger(Object *obj, HSQOBJECT trigger);
	Common::Array<ActorSwitcherSlot> actorSwitcherSlots();
	ActorSwitcherSlot actorSwitcherSlot(ActorSlot *slot);
	Scaling* getScaling(const Common::String& name);
	void skipCutscene();

public:
	Graphics::Screen *_screen = nullptr;
	GGPackSet _pack;
	ResManager _resManager;
	Common::Array<Room *> _rooms;
	Common::Array<Object *> _actors;
	Common::Array<Object *> _objects;
	Common::Array<ThreadBase *> _threads;
	Common::Array<Task *> _tasks;
	Common::Array<Callback *> _callbacks;
	Object *_actor = nullptr;
	Object *_followActor = nullptr;
	Room *_room = nullptr;
	float _time = 0.f;
	Object *_noun1 = nullptr;
	Object *_noun2 = nullptr;
	UseFlag _useFlag;
	HSQOBJECT _defaultObj;
	bool _walkFastState = false;
	bool _holdToMove = false;
	float _nextHoldToMoveTime = 0.f;
	int _frameCounter = 0;
	Lighting *_lighting = nullptr;
	Cutscene *_cutscene = nullptr;
	Scene _scene;
	Scene _screenScene;
	NoOverrideNode _noOverride;
	InputState _inputState;
	Camera _camera;
	TextDb _textDb;
	Dialog _dialog;
	struct Cursor {
		Math::Vector2d pos;
		bool oldLeftDown = false;
		bool leftDown = false;
		int leftDownTime = 0;
		bool oldRightDown = false;
		bool rightDown = false;
		int doubleClick = false;

		void update() {
			oldLeftDown = leftDown;
			oldRightDown = rightDown;
		}
		bool isLeftDown() { return !oldLeftDown && leftDown; }
		bool isRightDown() { return !oldRightDown && rightDown; }
	} _cursor;
	Hud _hud;
	Inventory _uiInv;
	ActorSwitcher _actorSwitcher;
	AudioSystem _audio;
	SaveGameManager _saveGameManager;
	ShaderParams _shaderParams;
	HotspotMarkerNode _hotspotMarker;

private:
	Gfx _gfx;
	Vm _vm;
	Preferences _prefs;
	unique_ptr<FadeShader> _fadeShader;
	SentenceNode _sentence;
	WalkboxNode _walkboxNode;
	PathNode _pathNode;
	Shader _bwShader;
	Shader _ghostShader;
	Shader _sepiaShader;
};

extern TwpEngine *g_engine;
#define SHOULD_QUIT ::Twp::g_engine->shouldQuit()

} // End of namespace Twp

#endif // TWP_H
