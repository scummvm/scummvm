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
#include "suspects_database.h"

namespace BladeRunner {

enum AnimationModes {
	kAnimationModeIdle = 0,
	kAnimationModeWalk = 1,
	kAnimationModeRun = 2,
	kAnimationModeCombatIdle = 4,
	kAnimationModeCombatWalk = 7,
	kAnimationModeCombatRun = 8
};

class Actor;
class ADQ;
class AIScripts;
class AmbientSounds;
class AudioPlayer;
class AudioSpeech;
class Chapters;
class CrimesDatabase;
class Combat;
class Font;
class GameFlags;
class GameInfo;
class ItemPickup;
class Items;
class Lights;
class Mouse;
class Obstacles;
class Scene;
class SceneObjects;
class SceneScript;
class Settings;
class Shape;
class SliceAnimations;
class SliceRenderer;
class TextResource;
class View;
class Waypoints;
class ZBuffer;

#define ACTORS_COUNT 100
#define VOICEOVER_ACTOR (ACTORS_COUNT - 1)

class BladeRunnerEngine : public Engine {
public:
	bool      _gameIsRunning;
	bool      _windowIsActive;
	int       _playerLosesControlCounter;

	ADQ              *_adq;
	AIScripts        *_aiScripts;
	AmbientSounds    *_ambientSounds;
	AudioPlayer      *_audioPlayer;
	AudioSpeech      *_audioSpeech;
	Chapters         *_chapters;
	CrimesDatabase   *_crimesDatabase;
	Combat           *_combat;
	GameFlags        *_gameFlags;
	GameInfo         *_gameInfo;
	ItemPickup       *_itemPickup;
	Items            *_items;
	Lights           *_lights;
	Font             *_mainFont;
	Mouse            *_mouse;
	Obstacles        *_obstacles;
	Scene            *_scene;
	SceneObjects     *_sceneObjects;
	SceneScript      *_sceneScript;
	Settings         *_settings;
	SliceAnimations  *_sliceAnimations;
	SliceRenderer    *_sliceRenderer;
	SuspectsDatabase *_suspectsDatabase;
	View             *_view;
	Waypoints        *_waypoints;
	int              *_gameVars;

	TextResource    *_textActorNames;
	TextResource    *_textCrimes;
	TextResource    *_textCluetype;
	TextResource    *_textKIA;
	TextResource    *_textSpinnerDestinations;
	TextResource    *_textVK;
	TextResource    *_textOptions;

	Common::Array<Shape*> _shapes;

	Actor *_actors[ACTORS_COUNT];
	Actor *_playerActor;

	int in_script_counter;

	Graphics::Surface  _surface1;
	Graphics::Surface  _surface2;
	ZBuffer           *_zbuffer;

	Common::RandomSource _rnd;

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
	static const uint kArchiveCount = 10;
	MIXArchive _archives[kArchiveCount];

public:
	BladeRunnerEngine(OSystem *syst);
	~BladeRunnerEngine();

	bool hasFeature(EngineFeature f) const;

	Common::Error run();

	bool startup(bool hasSavegames = false);
	void initChapterAndScene();
	void shutdown();

	bool loadSplash();
	bool init2();

	void gameLoop();
	void gameTick();
	void actorsUpdate();
	void handleEvents();
	void handleMouseClick(int x, int y);
	void handleMouseClickExit(int x, int y, int exitIndex);
	void handleMouseClickRegion(int x, int y, int regionIndex);
	void handleMouseClickItem(int x, int y, int itemId);
	void handleMouseClickActor(int x, int y, int actorId);
	void handleMouseClick3DObject(int x, int y, int objectId, bool isClickable, bool isTarget);
	void gameWaitForActive();
	void loopActorSpeaking();

	void outtakePlay(int id, bool no_localization, int container = -1);

	bool openArchive(const Common::String &name);
	bool closeArchive(const Common::String &name);
	bool isArchiveOpen(const Common::String &name);

	Common::SeekableReadStream *getResourceStream(const Common::String &name);

	bool playerHasControl();
	void playerLosesControl();
	void playerGainsControl();

	void ISez(const char *str);
};

static inline const Graphics::PixelFormat createRGB555() {
	return Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
}

} // End of namespace BladeRunner

#endif
