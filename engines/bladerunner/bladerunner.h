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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_BLADERUNNER_H
#define BLADERUNNER_BLADERUNNER_H

#include "bladerunner/archive.h"

#include "common/array.h"
#include "common/cosinetables.h"
#include "common/random.h"
#include "common/sinetables.h"
#include "common/stream.h"

#include "engines/engine.h"

#include "graphics/surface.h"

//TODO: change this to debugflag
#define BLADERUNNER_DEBUG_CONSOLE 0
#define BLADERUNNER_ORIGINAL_SETTINGS 0

namespace Common {
struct Event;
}

namespace GUI {
class Debugger;
}

struct ADGameDescription;

namespace BladeRunner {

enum DebugLevels {
	kDebugScript = 1 << 0
};

class Actor;
class ActorDialogueQueue;
class ScreenEffects;
class AIScripts;
class AmbientSounds;
class AudioMixer;
class AudioPlayer;
class AudioSpeech;
class Chapters;
class CrimesDatabase;
class Combat;
class Debugger;
class DialogueMenu;
class Elevator;
class EndCredits;
class ESPER;
class Font;
class GameFlags;
class GameInfo;
class ItemPickup;
class Items;
class KIA;
class Lights;
class Mouse;
class Music;
class Obstacles;
class Overlays;
class PoliceMaze;
class Scene;
class SceneObjects;
class SceneScript;
class Scores;
class Settings;
class Shape;
class SliceAnimations;
class SliceRenderer;
class Spinner;
class Subtitles;
class SuspectsDatabase;
class TextResource;
class Time;
class KIAShapes;
class Vector3;
class View;
class VK;
class Waypoints;
class ZBuffer;

class BladeRunnerEngine : public Engine {
public:
	static const int kArchiveCount = 11; // +1 to original value (10) to accommodate for SUBTITLES.MIX resource
	static const int kActorCount = 100;
	static const int kActorVoiceOver = kActorCount - 1;

	bool           _gameIsRunning;
	bool           _windowIsActive;
	int            _playerLosesControlCounter;
	Common::String _languageCode;

	ActorDialogueQueue *_actorDialogueQueue;
	ScreenEffects      *_screenEffects;
	AIScripts          *_aiScripts;
	AmbientSounds      *_ambientSounds;
	AudioMixer         *_audioMixer;
	AudioPlayer        *_audioPlayer;
	AudioSpeech        *_audioSpeech;
	Chapters           *_chapters;
	CrimesDatabase     *_crimesDatabase;
	Combat             *_combat;
	DialogueMenu       *_dialogueMenu;
	Elevator           *_elevator;
	EndCredits         *_endCredits;
	ESPER              *_esper;
	GameFlags          *_gameFlags;
	GameInfo           *_gameInfo;
	ItemPickup         *_itemPickup;
	Items              *_items;
	KIA                *_kia;
	Lights             *_lights;
	Font               *_mainFont;
	Subtitles          *_subtitles;
	Mouse              *_mouse;
	Music              *_music;
	Obstacles          *_obstacles;
	Overlays           *_overlays;
	PoliceMaze         *_policeMaze;
	Scene              *_scene;
	SceneObjects       *_sceneObjects;
	SceneScript        *_sceneScript;
	Scores             *_scores;
	Settings           *_settings;
	SliceAnimations    *_sliceAnimations;
	SliceRenderer      *_sliceRenderer;
	Spinner            *_spinner;
	SuspectsDatabase   *_suspectsDatabase;
	Time               *_time;
	View               *_view;
	VK                 *_vk;
	Waypoints          *_waypoints;
	int                *_gameVars;

	TextResource       *_textActorNames;
	TextResource       *_textCrimes;
	TextResource       *_textClueTypes;
	TextResource       *_textKIA;
	TextResource       *_textSpinnerDestinations;
	TextResource       *_textVK;
	TextResource       *_textOptions;

	Common::Array<Shape*> _shapes;

	Actor *_actors[kActorCount];
	Actor *_playerActor;

	Graphics::Surface  _surfaceFront;
	Graphics::Surface  _surfaceBack;
	Graphics::Surface  _surface4;

	ZBuffer           *_zbuffer;

	Common::RandomSource _rnd;

	Debugger *_debugger;

	Common::CosineTable *_cosTable1024;
	Common::SineTable   *_sinTable1024;

	bool _isWalkingInterruptible;
	bool _interruptWalking;
	bool _playerActorIdle;
	bool _playerDead;
	bool _speechSkipped;
	bool _gameOver;
	int  _gameAutoSave;
	bool _gameIsLoading;
	bool _sceneIsLoading;
	bool _vqaIsPlaying;
	bool _vqaStopIsRequested;
	bool _subtitlesEnabled; // tracks the state of whether subtitles are enabled or disabled from ScummVM GUI option or KIA checkbox (the states are synched)

	int _walkSoundId;
	int _walkSoundVolume;
	int _walkSoundBalance;
	int _runningActorId;

	int _mouseClickTimeLast;
	int _mouseClickTimeDiff;

	int  _walkingToExitId;
	bool _isInsideScriptExit;
	int  _walkingToRegionId;
	bool _isInsideScriptRegion;
	int  _walkingToObjectId;
	bool _isInsideScriptObject;
	int  _walkingToItemId;
	bool _isInsideScriptItem;
	bool _walkingToEmpty;
	int  _walkingToEmptyX;
	int  _walkingToEmptyY;
	bool _isInsideScriptEmpty;
	int  _walkingToActorId;
	bool _isInsideScriptActor;

	int _actorUpdateCounter;

private:
	MIXArchive _archives[kArchiveCount];

public:
	BladeRunnerEngine(OSystem *syst, const ADGameDescription *desc);
	~BladeRunnerEngine();

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	bool canSaveGameStateCurrently() override;
	Common::Error saveGameState(int slot, const Common::String &desc) override;

	Common::Error run();

	bool startup(bool hasSavegames = false);
	void initChapterAndScene();
	void shutdown();

	bool loadSplash();

	Common::Point getMousePos() const;
	bool isMouseButtonDown() const;

	void gameLoop();
	void gameTick();

	void actorsUpdate();

	void walkingReset();

	void handleEvents();
	void handleKeyUp(Common::Event &event);
	void handleKeyDown(Common::Event &event);
	void handleMouseAction(int x, int y, bool mainButton, bool buttonDown);
	void handleMouseClickExit(int exitId, int x, int y, bool buttonDown);
	void handleMouseClickRegion(int regionId, int x, int y, bool buttonDown);
	void handleMouseClickItem(int itemId, bool buttonDown);
	void handleMouseClickActor(int actorId, bool mainButton, bool buttonDown, Vector3 &scenePosition, int x, int y);
	void handleMouseClick3DObject(int objectId, bool buttonDown, bool isClickable, bool isTarget);
	void handleMouseClickEmpty(int x, int y, Vector3 &scenePosition, bool buttonDown);

	void gameWaitForActive();
	void loopActorSpeaking();

	void outtakePlay(int id, bool no_localization, int container = -1);

	bool openArchive(const Common::String &name);
	bool closeArchive(const Common::String &name);
	bool isArchiveOpen(const Common::String &name) const;

	void syncSoundSettings();
	bool isSubtitlesEnabled();
	void setSubtitlesEnabled(bool newVal);


	Common::SeekableReadStream *getResourceStream(const Common::String &name);

	bool playerHasControl();
	void playerLosesControl();
	void playerGainsControl();
	void playerDied();

	bool saveGame(Common::WriteStream &stream, const Graphics::Surface &thumbnail);
	bool loadGame(Common::SeekableReadStream &stream);
	void newGame(int difficulty);
	void autoSaveGame(int textId, bool endgame);

	void ISez(const Common::String &str);

	void blitToScreen(const Graphics::Surface &src) const;
	Graphics::Surface generateThumbnail() const;

	GUI::Debugger *getDebugger();
	Common::String getTargetName() const;
};

static inline const Graphics::PixelFormat createRGB555() {
	return Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
}

void blit(const Graphics::Surface &src, Graphics::Surface &dst);

} // End of namespace BladeRunner

#endif
