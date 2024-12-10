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

#include "common/random.h"
#include "common/ptr.h"
#include "engines/engine.h"
#include "twp/actorswitcher.h"
#include "twp/squirrel/squirrel.h"

#define SCREEN_MARGIN 100.f
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define VERBDEFAULT "verbDefault"

template<typename T, class DL = Common::DefaultDeleter<T> >
using unique_ptr = Common::ScopedPtr<T, DL>;

namespace Graphics {
class Screen;
}

struct ADGameDescription;

namespace Twp {

struct ActorSlot;
class AudioSystem;
class Callback;
class Camera;
class Cutscene;
class Dialog;
class FadeShader;
class GGPackSet;
class Hud;
class TwpImGui;
class InputState;
struct Light;
class Lighting;
class LightingNode;
class Motor;
class NoOverrideNode;
class Object;
class PathNode;
class ResManager;
class Room;
class SaveGameManager;
struct Scaling;
class Scene;
struct ShaderParams;
class Task;
class TextDb;
class ThreadBase;
struct TwpGameDescription;
struct Verb;
struct VerbId;
class Vm;
class WalkboxNode;

enum class FadeEffect;
enum class RoomEffect;
enum class UseFlag;

class TwpEngine : public Engine {
private:
	const TwpGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	TwpEngine(OSystem *syst, const TwpGameDescription *gameDesc);
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

	HSQUIRRELVM getVm();
	inline Gfx &getGfx() { return _gfx; }
	inline TextDb &getTextDb() { return *_textDb; }

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher) ||
			   (f == kSupportsChangingOptionsDuringRuntime);
	}

	void updateSettingVars();

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return !_cutscene;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	Common::Error loadGameState(int slot) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canSaveAutosaveCurrently() override { return false; }
	void capture(Graphics::Surface &surface, int width, int height);

	Math::Vector2d winToScreen(const Math::Vector2d &pos) const;
	Math::Vector2d screenToWin(const Math::Vector2d &pos) const;
	Math::Vector2d roomToScreen(const Math::Vector2d &pos) const;
	Math::Vector2d screenToRoom(const Math::Vector2d &pos) const;

	void setActor(Common::SharedPtr<Object> actor, bool userSelected = false);
	Common::SharedPtr<Object> objAt(const Math::Vector2d &pos);
	void flashSelectableActor(int flash);
	void sayLineAt(const Math::Vector2d &pos, const Color &color, Common::SharedPtr<Object> actor, float duration, const Common::String &text);
	void stopTalking();
	bool isSomeoneTalking() const;
	void walkFast(bool state = true);

	void actorEnter(Common::SharedPtr<Object> actor);
	void actorExit(Common::SharedPtr<Object> actor);
	Common::SharedPtr<Room> defineRoom(const Common::String &name, HSQOBJECT table, bool pseudo = false);
	void setRoom(Common::SharedPtr<Room> room, bool force = false);
	void enterRoom(Common::SharedPtr<Room> room, Common::SharedPtr<Object> door = nullptr);

	void cameraAt(const Math::Vector2d &at);
	// Returns the camera position: the position of the middle of the screen.
	Math::Vector2d cameraPos();
	void follow(Common::SharedPtr<Object> actor);
	void fadeTo(FadeEffect effect, float duration, bool fadeToSep = false);

	void execNutEntry(HSQUIRRELVM v, const Common::String &entry);
	void execBnutEntry(HSQUIRRELVM v, const Common::String &entry);
	bool callVerb(Common::SharedPtr<Object> actor, VerbId verbId, Common::SharedPtr<Object> noun1, Common::SharedPtr<Object> noun2 = nullptr);
	bool execSentence(Common::SharedPtr<Object> actor, VerbId verbId, Common::SharedPtr<Object> noun1, Common::SharedPtr<Object> noun2 = nullptr);

	float getRandom() const;
	float getRandom(float min, float max) const;

	int runDialog(GUI::Dialog &dialog) override;

private:
	void update(float elapsedMs);
	void draw(RenderTexture *texture = nullptr);
	void exitRoom(Common::SharedPtr<Room> nextRoom);
	void cancelSentence(Common::SharedPtr<Object> actor = nullptr);
	void clickedAt(const Math::Vector2d &scrPos);
	bool clickedAtHandled(const Math::Vector2d &roomPos);
	void setShaderEffect(RoomEffect effect);
	bool selectable(Common::SharedPtr<Object> actor);
	void resetVerb();
	Common::String cursorText();
	Verb verb();
	bool preWalk(Common::SharedPtr<Object> actor, VerbId verbId, Common::SharedPtr<Object> noun1, Common::SharedPtr<Object> noun2);
	void updateTriggers();
	void callTrigger(Common::SharedPtr<Object> obj, HSQOBJECT trigger);
	Common::Array<ActorSwitcherSlot> actorSwitcherSlots();
	ActorSwitcherSlot actorSwitcherSlot(ActorSlot *slot);
	Scaling *getScaling(const Common::String &name);
	void skipCutscene();

private:
	unique_ptr<Vm> _vm;

public:
	unique_ptr<GGPackSet> _pack;
	unique_ptr<ResManager> _resManager;
	Common::Array<Common::SharedPtr<Room> > _rooms;
	Common::Array<Common::SharedPtr<Object> > _actors;
	Common::Array<Common::SharedPtr<ThreadBase> > _threads;
	Common::Array<Common::SharedPtr<Task> > _tasks;
	Common::Array<Common::SharedPtr<Callback> > _callbacks;
	Common::SharedPtr<Object> _actor;
	Common::SharedPtr<Object> _followActor;
	Common::SharedPtr<Room> _room;
	float _time = 0.f;
	Common::SharedPtr<Object> _noun1;
	Common::SharedPtr<Object> _noun2;
	UseFlag _useFlag;
	HSQOBJECT _defaultObj;
	bool _walkFastState = false;
	bool _holdToMove = false;
	float _nextHoldToMoveTime = 0.f;
	int _frameCounter = 0;
	Common::SharedPtr<Lighting> _lighting;
	Common::SharedPtr<Cutscene> _cutscene;
	unique_ptr<Scene> _scene;
	unique_ptr<Scene> _screenScene;
	unique_ptr<NoOverrideNode> _noOverride;
	InputState _inputState;
	unique_ptr<Camera> _camera;
	unique_ptr<TextDb> _textDb;
	unique_ptr<Dialog> _dialog;
	struct Cursor {
		Math::Vector2d pos;
		bool oldLeftDown = false;
		bool leftDown = false;
		int leftDownTime = 0;
		bool oldRightDown = false;
		bool rightDown = false;
		int doubleClick = false;
		bool holdLeft = false;
		bool holdRight = false;
		bool holdUp = false;
		bool holdDown = false;

		void update() {
			oldLeftDown = leftDown;
			oldRightDown = rightDown;
		}
		bool isLeftDown() { return !oldLeftDown && leftDown; }
		bool isRightDown() { return !oldRightDown && rightDown; }
	} _cursor;

	struct Stats {
		uint32 totalUpdateTime = 0;
		uint32 updateRoomTime = 0;
		uint32 updateTasksTime = 0;
		uint32 updateMiscTime = 0;
		uint32 updateCutsceneTime = 0;
		uint32 updateThreadsTime = 0;
		uint32 updateCallbacksTime = 0;
		uint32 drawTime = 0;
	} _stats;
	unique_ptr<Hud> _hud;
	Inventory _uiInv;
	ActorSwitcher _actorSwitcher;
	unique_ptr<AudioSystem> _audio;
	unique_ptr<SaveGameManager> _saveGameManager;
	unique_ptr<ShaderParams> _shaderParams;
	unique_ptr<HotspotMarkerNode> _hotspotMarker;
	unique_ptr<FadeShader> _fadeShader;
	unique_ptr<LightingNode> _lightingNode;
	unique_ptr<Motor> _moveCursorTo;

private:
	Gfx _gfx;
	SentenceNode _sentence;
	unique_ptr<WalkboxNode> _walkboxNode;
	unique_ptr<PathNode> _pathNode;
	unique_ptr<Shader> _bwShader;
	unique_ptr<Shader> _ghostShader;
	unique_ptr<Shader> _sepiaShader;
	int _speed = 1;
	bool _control = false;
	unique_ptr<Motor> _talking;
};

extern TwpEngine *g_twp;
#define SHOULD_QUIT ::Twp::g_twp->shouldQuit()

} // End of namespace Twp

#endif // TWP_H
