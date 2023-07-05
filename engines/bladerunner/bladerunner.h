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

#ifndef BLADERUNNER_BLADERUNNER_H
#define BLADERUNNER_BLADERUNNER_H

#include "bladerunner/archive.h"

#include "common/array.h"
#include "common/random.h"
#include "common/stream.h"
#include "common/keyboard.h"
#include "common/events.h"

#include "engines/engine.h"

#include "graphics/surface.h"

#include "math/cosinetables.h"
#include "math/sinetables.h"

//TODO: change this to debugflag
#define BLADERUNNER_DEBUG_CONSOLE     0
#define BLADERUNNER_ORIGINAL_SETTINGS 0
#define BLADERUNNER_ORIGINAL_BUGS     0

namespace Common {
class Archive;
struct Event;
}

namespace GUI {
class Debugger;
}

struct ADGameDescription;

namespace BladeRunner {

enum DebugLevels {
	kDebugScript = 1 << 0,
	kDebugSound = 1 << 1,
	kDebugAnimation = 1 << 2
};

class Actor;
class ActorDialogueQueue;
class ScreenEffects;
class AIScripts;
class AmbientSounds;
class AudioCache;
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
class Framelimiter;
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
class Shapes;
class SliceAnimations;
class SliceRenderer;
class Spinner;
class Subtitles;
class SuspectsDatabase;
class TextResource;
class Time;
class Vector3;
class View;
class VK;
class Waypoints;
class ZBuffer;

class BladeRunnerEngine : public Engine {
public:
	static const int kArchiveCount = 12; // +2 to original value (10) to accommodate for SUBTITLES.MIX and one extra resource file, to allow for capability of loading all VQAx.MIX and the MODE.MIX file (debug purposes)
	static const int kActorCount =  100;
	static const int kActorVoiceOver = kActorCount - 1;
	static const int kMaxCustomConcurrentRepeatableEvents = 20;

	static const int16 kOriginalGameWidth  = 640;
	static const int16 kOriginalGameHeight = 480;
	static const int16 kDemoGameWidth      = 320;
	static const int16 kDemoGameHeight     = 200;

	// Incremental number to keep track of significant revisions of the ScummVM bladerunner engine
	// that could potentially introduce incompatibilities with old save files or require special actions to restore compatibility
	// This is stored in game global variable "kVariableGameVersion"
	// Original (classic) save game files will have version number of 0
	// Values:
	// 1: alpha testing (from May 15, 2019 to July 17, 2019)
	// 2: all time code uses uint32 (since July 17 2019),
	static const int kBladeRunnerScummVMVersion = 2;

	static const char *kGameplayKeymapId;
	static const char *kKiaKeymapId;
	static const char *kCommonKeymapId;

	bool _gameIsRunning;
	bool _windowIsActive;
	int  _playerLosesControlCounter;
	int  _extraCPos;
	uint8 _extraCNotify;

	Common::String   _languageCode;
	Common::Language _language;
	bool             _russianCP1251;
	bool             _noMusicDriver; // If "Music Device" is set to "No Music" from Audio tab

	ActorDialogueQueue *_actorDialogueQueue;
	ScreenEffects      *_screenEffects;
	AIScripts          *_aiScripts;
	AmbientSounds      *_ambientSounds;
	AudioCache         *_audioCache;
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
	Framelimiter       *_framelimiter;
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

	Shapes *_shapes;

	Actor *_actors[kActorCount];
	Actor *_playerActor;

	Graphics::PixelFormat _screenPixelFormat;

	Graphics::Surface  _surfaceFront;
	Graphics::Surface  _surfaceBack;
	bool               _surfaceFrontCreated;
	bool               _surfaceBackCreated;

	ZBuffer           *_zbuffer;

	Common::RandomSource _rnd;

	Debugger *_debugger;

	Math::CosineTable *_cosTable1024;
	Math::SineTable   *_sinTable1024;

	bool _isWalkingInterruptible;
	bool _interruptWalking;
	bool _playerActorIdle;
	bool _playerDead;
	bool _actorIsSpeaking;
	bool _actorSpeakStopIsRequested;
	bool _gameOver;
	bool _gameJustLaunched;
	int  _gameAutoSaveTextId;
	bool _gameIsAutoSaving;
	bool _gameIsLoading;
	bool _sceneIsLoading;
	bool _vqaIsPlaying;
	bool _vqaStopIsRequested;
	bool _subtitlesEnabled;  // tracks the state of whether subtitles are enabled or disabled from ScummVM GUI option or KIA checkbox (the states are synched)
	bool _showSubtitlesForTextCrawl;
	bool _sitcomMode;
	bool _shortyMode;
	bool _noDelayMillisFramelimiter;
	bool _framesPerSecondMax;
	bool _disableStaminaDrain;
	bool _spanishCreditsCorrection;
	bool _cutContent;
	bool _enhancedEdition;
	bool _validBootParam;

	int _walkSoundId;
	int _walkSoundVolume;
	int _walkSoundPan;
	int _runningActorId;

	uint32 _mouseClickTimeLast;
	uint32 _mouseClickTimeDiff;

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

	int    _actorUpdateCounter;
	uint32 _actorUpdateTimeLast;

	uint32 _timeOfMainGameLoopTickPrevious;

	bool _isNonInteractiveDemo;

	// This addon is to emulate keeping a keyboard key pressed (continuous / repeated firing of the event)
	// -- code is pretty much identical from our common\events.cpp (KeyboardRepeatEventSourceWrapper)
	// for continuous events (keyDown)
	enum {
		kKeyRepeatInitialDelay = 400,
		kKeyRepeatSustainDelay = 100
	};

	Common::KeyState _currentKeyDown;
	uint32 _keyRepeatTimeLast;
	uint32 _keyRepeatTimeDelay;

	uint32 _customEventRepeatTimeLast;
	uint32 _customEventRepeatTimeDelay;
	typedef Common::Array<Common::Event> ActiveCustomEventsArray;

	// We do allow keys mapped to the same event,
	// so eg. a key (Enter) could cause 2 or more events to fire,
	// However, we should probably restrict the active events
	// (that can be repeated while holding the mapped keys down)
	// to a maximum of kMaxCustomConcurrentRepeatableEvents
	ActiveCustomEventsArray _activeCustomEvents[kMaxCustomConcurrentRepeatableEvents];

	// NOTE We still need keyboard functionality for naming saved games and also for the KIA Easter eggs.
	//      In KIA keyboard events should be accounted where possible - however some keymaps are still needed
	//      which is why we have the three separate common, gameplay-only and kia-only keymaps.
	//      If a valid keyboard key character eg. ("A") for text input (or Easter egg input)
	//      is also mapped to a common or KIA only custom event, then the custom event will be effected and not the key input.
	// NOTE We don't use a custom action for left click -- we just use the standard left click action event (kStandardActionLeftClick)
	// NOTE Dialogue Skip does not work for dialogue replayed when clicking on KIA clues (this is the original's behavior too)
	// NOTE Toggle KIA options does not work when McCoy is walking towards a character when the player clicks on McCoy
	//      (this is the original's behavior too).
	//      "Esc" (by default) or the mapped key to this action still works though.
	// NOTE A drawback of using customized keymapper for the game is that we can no longer replicate the original's behavior
	//      whereby holding down <SPACEBAR> would cause McCoy to keep switching quickly between combat mode and normal mode.
	//      This is because the original, when holding down right mouse button, would just toggle McCoy's mode once.
	//      We keep the behavior for "right mouse button".
	//      The continuous fast toggle behavior when holding down <SPACEBAR> feels more like a bug anyway.
	// NOTE In the original, the KP_PERIOD key with NUMLOCK on, would work as a normal '.' character
	//      in the KIA Save Game screen. With NUMLOCK off, it would work as a delete request for the selected entry.
	//      However, NUMLOCK is currently not working as a modifier key for keymaps,
	//      so maybe we can implement the original behavior more accurately,
	//      when that is fixed in the keymapper or hardware-input code.
	//      For now, KP_PERIOD will work (by default) as a delete request.
	enum BladeRunnerEngineMappableAction {
//		kMpActionLeftClick,        // default <left click> (select, walk-to, run-to, look-at, talk-to, use, shoot (combat mode), KIA (click on McCoy))
		kMpActionToggleCombat,     // default <right click> or <Spacebar>
		kMpActionCutsceneSkip,     // default <Return> or <KP_Enter> or <Esc> or <Spacebar>
		kMpActionDialogueSkip,     // default <Return> or <KP_Enter>
		kMpActionToggleKiaOptions, // default <Esc> opens/closes KIA, in Options tab
		kMpActionOpenKiaDatabase,  // default <Tab> - only opens KIA (if closed), in one of the database tabs (the last active one, or else the first)
		kMpActionOpenKIATabHelp,               // default <F1>
		kMpActionOpenKIATabSaveGame,           // default <F2>
		kMpActionOpenKIATabLoadGame,           // default <F3>
		kMpActionOpenKIATabCrimeSceneDatabase, // default <F4>
		kMpActionOpenKIATabSuspectDatabase,    // default <F5>
		kMpActionOpenKIATabClueDatabase,       // default <F6>
		kMpActionOpenKIATabQuitGame,           // default <F10>
		kMpActionScrollUp,                     // ScummVM addition (scroll list up)
		kMpActionScrollDown,                   // ScummVM addition (scroll list down)
		kMpConfirmDlg,                         // default <Return> or <KP_Enter>
		kMpDeleteSelectedSvdGame,              // default <Delete> or <KP_Period>
		kMpActionToggleCluePrivacy             // default <right click>
	};

private:
	MIXArchive _archives[kArchiveCount];
	Common::Archive *_archive;

public:
	BladeRunnerEngine(OSystem *syst, const ADGameDescription *desc);
	~BladeRunnerEngine() override;

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	bool canSaveGameStateCurrently() override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	/**
	* NOTE: Disable support for external autosave (ScummVM's feature).
	*       Main reason is that it's not easy to properly label this autosave,
	*       since currently it would translate "Autosave" to the ScummVM GUI language
	*       which ends up showing as "?????? ??????" on non-latin languages (eg. Greek),
	*       and in addition is inconsistent with the game's own GUI language.
	*       Secondary reason is that the game already has an autosaving mechanism,
	*       albeit only at the start of a new Act.
	*       And final reason would be to prevent an autosave at an unforeseen moment,
	*       if we've failed to account for all cases that the game should not save by itself;
	*       currently those are listed in BladeRunnerEngine::canSaveGameStateCurrently().
	*/
	int getAutosaveSlot() const override { return -1; }

	void pauseEngineIntern(bool pause) override;

	Common::Error run() override;

	bool checkFiles(Common::Array<Common::String> &missingFiles);

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
	void handleMouseAction(int x, int y, bool mainButton, bool buttonDown, int scrollDirection = 0);
	void handleMouseClickExit(int exitId, int x, int y, bool buttonDown);
	void handleMouseClickRegion(int regionId, int x, int y, bool buttonDown);
	void handleMouseClickItem(int itemId, bool buttonDown);
	void handleMouseClickActor(int actorId, bool mainButton, bool buttonDown, Vector3 &scenePosition, int x, int y);
	void handleMouseClick3DObject(int objectId, bool buttonDown, bool isClickable, bool isTarget);
	void handleMouseClickEmpty(int x, int y, Vector3 &scenePosition, bool buttonDown);

	bool isAllowedRepeatedKey(const Common::KeyState &currKeyState);

	void handleCustomEventStart(Common::Event &event);
	void handleCustomEventStop(Common::Event &event);
	bool isAllowedRepeatedCustomEvent(const Common::Event &currEvent);

	bool shouldDropRogueCustomEvent(const Common::Event &evt);
	void cleanupPendingRepeatingEvents(const Common::String &keymapperId);

	void gameWaitForActive();
	void loopActorSpeaking();
	void loopQueuedDialogueStillPlaying();

	void outtakePlay(int id, bool no_localization, int container = -1);
	void outtakePlay(const Common::String &basenameNoExt, bool no_localization, int container = -3);

	bool openArchive(const Common::String &name);
	bool closeArchive(const Common::String &name);
	bool isArchiveOpen(const Common::String &name) const;

	bool openArchiveEnhancedEdition();

	void syncSoundSettings() override;
	bool isSubtitlesEnabled();
	void setSubtitlesEnabled(bool newVal);

	Common::SeekableReadStream *getResourceStream(const Common::String &name);

	bool playerHasControl();
	void playerLosesControl();
	void playerGainsControl(bool force = false);
	void playerDied();

	bool saveGame(Common::WriteStream &stream, Graphics::Surface *thumb = NULL, bool origformat = false);
	bool loadGame(Common::SeekableReadStream &stream, int version);
	void newGame(int difficulty);
	void autoSaveGame(int textId, bool endgame);

	void ISez(const Common::String &str);

	void blitToScreen(const Graphics::Surface &src) const;
	Graphics::Surface generateThumbnail() const;

	Common::String getTargetName() const;

	uint8 getExtraCNotify();
	void  setExtraCNotify(uint8 val);
};

static inline const Graphics::PixelFormat gameDataPixelFormat() {
	return Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15);
}

static inline void getGameDataColor(uint16 color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) {
	// gameDataPixelFormat().colorToARGB(vqaColor, a, r, g, b);
	// using pixel format functions is too slow on some ports because of runtime checks
	uint8 r5 = (color >> 10) & 0x1F;
	uint8 g5 = (color >>  5) & 0x1F;
	uint8 b5 = (color      ) & 0x1F;
	a = color >> 15;
	r = (r5 << 3) | (r5 >> 2);
	g = (g5 << 3) | (g5 >> 2);
	b = (b5 << 3) | (b5 >> 2);
}

static inline const Graphics::PixelFormat screenPixelFormat() {
	return ((BladeRunnerEngine*)g_engine)->_screenPixelFormat;
}

static inline void drawPixel(Graphics::Surface &surface, void* dst, uint32 value) {
	switch (surface.format.bytesPerPixel) {
	case 1:
		*(uint8*)dst = (uint8)value;
		break;
	case 2:
		*(uint16*)dst = (uint16)value;
		break;
	case 4:
		*(uint32*)dst = (uint32)value;
		break;
	default:
		break;
	}
}

static inline void getPixel(Graphics::Surface &surface, void* dst, uint32 &value) {
	switch (surface.format.bytesPerPixel) {
	case 1:
		 value = (uint8)(*(uint8*)dst);
		break;
	case 2:
		 value = (uint16)(*(uint16*)dst);
		break;
	case 4:
		value = (uint32)(*(uint32*)dst);
		break;
	default:
		break;
	}
}

void blit(const Graphics::Surface &src, Graphics::Surface &dst);

} // End of namespace BladeRunner

#endif
