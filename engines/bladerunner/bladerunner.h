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
#include "common/random.h"
#include "common/stream.h"

#include "engines/engine.h"

#include "graphics/surface.h"

//TODO: remove these when game is playable
#define BLADERUNNER_DEBUG_RENDERING 0
#define BLADERUNNER_DEBUG_CONSOLE 0
#define BLADERUNNER_DEBUG_GAME 0

namespace Common {
struct Event;
}

struct ADGameDescription;

namespace BladeRunner {

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
class DialogueMenu;
class Elevator;
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
class Scene;
class SceneObjects;
class SceneScript;
class Settings;
class Shape;
class SliceAnimations;
class SliceRenderer;
class Spinner;
class SuspectsDatabase;
class TextResource;
class KIAShapes;
class Vector3;
class View;
class VK;
class Waypoints;
class ZBuffer;

class BladeRunnerEngine : public Engine {
public:
	static const int kArchiveCount = 10;
	static const int kActorCount = 100;
	static const int kActorVoiceOver = kActorCount - 1;

	bool       _gameIsRunning;
	bool       _windowIsActive;
	int        _playerLosesControlCounter;
	const char *_languageCode;

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
	ESPER              *_esper;
	GameFlags          *_gameFlags;
	GameInfo           *_gameInfo;
	ItemPickup         *_itemPickup;
	Items              *_items;
	KIA                *_kia;
	Lights             *_lights;
	Font               *_mainFont;
	Mouse              *_mouse;
	Music              *_music;
	Obstacles          *_obstacles;
	Overlays           *_overlays;
	Scene              *_scene;
	SceneObjects       *_sceneObjects;
	SceneScript        *_sceneScript;
	Settings           *_settings;
	SliceAnimations    *_sliceAnimations;
	SliceRenderer      *_sliceRenderer;
	Spinner            *_spinner;
	SuspectsDatabase   *_suspectsDatabase;
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

	bool _isWalkingInterruptible;
	bool _interruptWalking;
	bool _playerActorIdle;
	bool _playerDead;
	bool _speechSkipped;
	bool _gameOver;
	int  _gameAutoSave;
	bool _gameIsLoading;
	bool _sceneIsLoading;

	int _walkSoundId;
	int _walkSoundVolume;
	int _walkSoundBalance;
	int _walkingActorId;

private:
	MIXArchive _archives[kArchiveCount];

public:
	BladeRunnerEngine(OSystem *syst, const ADGameDescription *desc);
	~BladeRunnerEngine();

	bool hasFeature(EngineFeature f) const;

	Common::Error run();

	bool startup(bool hasSavegames = false);
	void initChapterAndScene();
	void shutdown();

	bool loadSplash();
	bool init2();

	Common::Point getMousePos() const;
	bool isMouseButtonDown() const;

	void gameLoop();
	void gameTick();
	void actorsUpdate();
	void handleEvents();
	void handleKeyUp(Common::Event &event);
	void handleKeyDown(Common::Event &event);
	void handleMouseAction(int x, int y, bool buttonLeft, bool buttonDown);
	void handleMouseClickExit(int x, int y, int exitIndex);
	void handleMouseClickRegion(int x, int y, int regionIndex);
	void handleMouseClickItem(int x, int y, int itemId);
	void handleMouseClickActor(int x, int y, int actorId);
	void handleMouseClick3DObject(int x, int y, int objectId, bool isClickable, bool isTarget);
	void handleMouseClickEmpty(int x, int y, Vector3 &mousePosition);
	void gameWaitForActive();
	void loopActorSpeaking();

	void outtakePlay(int id, bool no_localization, int container = -1);

	bool openArchive(const Common::String &name);
	bool closeArchive(const Common::String &name);
	bool isArchiveOpen(const Common::String &name) const;

	Common::SeekableReadStream *getResourceStream(const Common::String &name);

	bool playerHasControl();
	void playerLosesControl();
	void playerGainsControl();

	void ISez(const char *str);

	void blitToScreen(const Graphics::Surface &src);
};

static inline const Graphics::PixelFormat createRGB555() {
	return Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
}

void blit(const Graphics::Surface &src, Graphics::Surface &dst);

} // End of namespace BladeRunner

#endif
