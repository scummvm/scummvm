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

#ifndef HYPNO_H
#define HYPNO_H

#include "common/array.h"
#include "common/installshieldv3_archive.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/str-array.h"
#include "common/stream.h"
#include "engines/engine.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"

#include "hypno/grammar.h"
#include "hypno/libfile.h"

namespace Image {
class ImageDecoder;
}

namespace Graphics {
class ManagedSurface;
}

struct ADGameDescription;

namespace Hypno {

// debug channels
enum {
	kHypnoDebugMedia = 1 << 0,
	kHypnoDebugParser = 1 << 1,
	kHypnoDebugArcade = 1 << 2,
	kHypnoDebugScene = 1 << 3
};

typedef Common::Array<Graphics::Surface *> Frames;

// Player positions

enum PlayerPosition {
	PlayerTop = 'T',
	PlayerBottom = 'B',
	PlayerLeft = 'L',
	PlayerRight = 'R'
};

class HypnoEngine : public Engine {
private:
	Common::RandomSource *_rnd;
	Image::ImageDecoder *_image;

public:
	HypnoEngine(OSystem *syst, const ADGameDescription *gd);
	~HypnoEngine();

	const ADGameDescription *_gameDescription;
	bool isDemo() const;
	Common::Language _language;
	Common::Platform _platform;
	Common::String _variant;
	bool _cheatsEnabled;

	Audio::SoundHandle _soundHandle;
	Common::InstallShieldV3 _installerArchive;
	Common::List<LibFile*> _archive;

	Common::Error run() override;
	Levels _levels;
	Common::HashMap<Common::String, int> _sceneState;
	void resetSceneState();
	bool checkSceneCompleted();
	bool checkLevelWon();
	void runLevel(Common::String &name);
	void runScene(Scene *scene);
	void runArcade(ArcadeShooting *arc);
	// For some menus and hardcoded puzzles
	virtual void runCode(Code *code);
	// Level transitions
	void runTransition(Transition *trans);

	void restartGame();
	void clearAreas();
	void initializePath(const Common::FSNode &gamePath) override;
	virtual void loadAssets();

	// Parsing
	void splitArcadeFile(const Common::String &filename, Common::String &arc, Common::String &list);
	void parseArcadeShooting(const Common::String &prefix, const Common::String &name, const Common::String &data);
	ShootSequence parseShootList(const Common::String &name, const Common::String &data);
	void loadArcadeLevel(const Common::String &current, const Common::String &next, const Common::String &prefix);
	void loadSceneLevel(const Common::String &current, const Common::String &next, const Common::String &prefix);
	LibFile *loadLib(const Filename &prefix, const Filename &filename, bool encrypted);

	// User input
	void clickedHotspot(Common::Point);
	bool hoverHotspot(Common::Point);

	// Cursors
	bool cursorPauseMovie(Common::Point);
	bool cursorExit(Common::Point);
	bool cursorMask(Common::Point);

	virtual void loadGame(const Common::String &nextLevel, int puzzleDifficulty, int combatDifficulty);
	bool canLoadGameStateCurrently() override { return (isDemo() ? false : true); }
	bool canSaveAutosaveCurrently() override { return false; }
	bool canSaveGameStateCurrently() override { return (isDemo() ? false : true); }
	Common::String _checkpoint;

	Common::String _prefixDir;
	Common::String convertPath(const Common::String &);
	void playVideo(MVideo &video);
	void skipVideo(MVideo &video);

	Common::File *fixSmackerHeader(Common::File *file);
	Graphics::Surface *decodeFrame(const Common::String &name, int frame, byte **palette = nullptr);
	Frames decodeFrames(const Common::String &name);
	void loadImage(const Common::String &file, int x, int y, bool transparent, bool palette = false, int frameNumber = 0);
	void drawImage(Graphics::Surface &image, int x, int y, bool transparent);
	void loadPalette(const Common::String &fname);
	void loadPalette(const byte *palette, uint32 offset, uint32 size);

	// Cursors
	Common::String _defaultCursor;
	void disableCursor();
	void defaultCursor();
	void changeCursor(const Common::String &cursor, uint32 n);
	void changeCursor(const Common::String &cursor);

	// Actions
	void runMenu(Hotspots hs);
	void runBackground(Background *a);
	void runOverlay(Overlay *a);
	void runMice(Mice *a);
	void runEscape();
	void runSave(Save *a);
	void runLoad(Load *a);
	void runLoadCheckpoint(LoadCheckpoint *a);
	void runTimer(Timer *a);
	void runQuit(Quit *a);
	void runCutscene(Cutscene *a);
	void runIntro(Intro *a);
	void runPlay(Play *a);
	void runPalette(Palette *a);
	void runAmbient(Ambient *a);
	void runWalN(WalN *a);
	bool runGlobal(Global *a);
	void runTalk(Talk *a);
	void runChangeLevel(ChangeLevel *a);

	// Screen
	int _screenW, _screenH;
	Graphics::PixelFormat _pixelFormat;
	void changeScreenMode(const Common::String &mode);
	Graphics::ManagedSurface *_compositeSurface;
	uint32 _transparentColor;
	Common::Rect screenRect;
	void updateScreen(MVideo &video);
	void drawScreen();

	// intros
	void runIntro(MVideo &video);
	void runIntros(Videos &videos);
	Common::HashMap<Filename, bool> _intros;

	// levels
	Common::String _nextLevel;
	Common::String _currentLevel;
	virtual Common::String findNextLevel(const Common::String &level);
	virtual Common::String findNextLevel(const Transition *trans);
	uint32 _levelId;

	// hotspots
	Hotspots *_nextHotsToAdd;
	Hotspots *_nextHotsToRemove;
	HotspotsStack stack;

	// Movies
	Videos _nextSequentialVideoToPlay;
	Videos _nextParallelVideoToPlay;
	Videos _escapeSequentialVideoToPlay;
	Videos _videosPlaying;
	Videos _videosLooping;

	// Sounds
	Filename _soundPath;
	Filename _music;
	void playSound(const Filename &filename, uint32);
	void stopSound();
	bool isSoundActive();
	bool _noStopSounds;

	// Arcade
	Common::String _arcadeMode;
	uint32 _currentPlayerPosition;
	uint32 _lastPlayerPosition;
	int detectTarget(const Common::Point &mousePos);
	virtual bool clickedPrimaryShoot(const Common::Point &mousePos);
	virtual bool clickedSecondaryShoot(const Common::Point &mousePos);
	virtual void drawShoot(const Common::Point &mousePos);
	virtual void shoot(const Common::Point &mousePos);
	virtual void hitPlayer();
	virtual bool checkArcadeLevelCompleted(MVideo &background);
	Common::String _difficulty;
	bool _skipLevel;

	virtual void drawCursorArcade(const Common::Point &mousePos);
	virtual void drawPlayer();
	virtual void drawHealth();
	int _health;
	int _maxHealth;
	int _score;
	Filename _shootSound;
	Filename _hitSound;
	Shoots _shoots;
	Frames _playerFrames;
	int _playerFrameIdx;
	int _playerFrameSep;
	const Graphics::Font *_font;

	// Conversation
	Actions _conversation;
	bool _refreshConversation;
	virtual void showConversation();
	virtual void rightClickedConversation(const Common::Point &mousePos);
	virtual void leftClickedConversation(const Common::Point &mousePos);
	virtual bool hoverConversation(const Common::Point &mousePos);
	// Credits
	virtual void showCredits();

	// Timers
	int32 _countdown;
	bool _timerStarted;
	bool startAlarm(uint32, Common::String *);
	bool startCountdown(uint32);
	void removeTimers();
};

struct chapterEntry {
	int id;
	int energyPos[2];
	int scorePos[2];
	int objectivesPos[2];
};

class WetEngine : public HypnoEngine {
public:
	WetEngine(OSystem *syst, const ADGameDescription *gd);
	Common::HashMap<int, const struct chapterEntry*> _chapterTable;
	void loadAssets() override;
	void loadAssetsDemoDisc();
	void loadAssetsPCW();
	void loadAssetsPCG();
	void loadAssetsFullGame();
	void showCredits() override;
	bool clickedSecondaryShoot(const Common::Point &mousePos) override;
	void drawShoot(const Common::Point &target) override;
	void drawPlayer() override;
	void drawHealth() override;
	void runCode(Code *code) override;
	Common::String findNextLevel(const Common::String &level) override;
	Common::String findNextLevel(const Transition *trans) override;

	//virtual Common::Error loadGameStream(Common::SeekableReadStream *stream) = 0;
	//virtual Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) = 0;


private:
	void runMainMenu(Code *code);
};

class SpiderEngine : public HypnoEngine {
public:
	SpiderEngine(OSystem *syst, const ADGameDescription *gd);
	void loadAssets() override;
	void loadAssetsDemo();
	void loadAssetsFullGame();
	void showCredits() override;

	void drawCursorArcade(const Common::Point &mousePos) override;
	void drawShoot(const Common::Point &target) override;
	void drawPlayer() override;
	void drawHealth() override;
	bool checkArcadeLevelCompleted(MVideo &background) override;

	void runCode(Code *code) override;
	Common::String findNextLevel(const Common::String &level) override;
	Common::String findNextLevel(const Transition *trans) override;

	void showConversation() override;
	void rightClickedConversation(const Common::Point &mousePos) override;
	void leftClickedConversation(const Common::Point &mousePos) override;
	bool hoverConversation(const Common::Point &mousePos) override;

	void loadGame(const Common::String &nextLevel, int puzzleDifficulty, int combatDifficulty) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsSavingDuringRuntime || f == kSupportsLoadingDuringRuntime);
	}

private:
	void runMatrix(Code *code);
	void addIngredient(Code *code);
	void checkMixture(Code *code);
	void runNote(Code *code);
	void runFusePanel(Code *code);
	void runRecept(Code *code);
	void runOffice(Code *code);
	void runFileCabinet(Code *code);
	void runLock(Code *code);
	void runFuseBox(Code *code);

	bool _fuseState[2][10] = {};
	bool _isFuseRust = true;
	bool _isFuseUnreadable = false;
	bool ingredients[7] = {};
};

class BoyzEngine : public HypnoEngine {
public:
	BoyzEngine(OSystem *syst, const ADGameDescription *gd);
	void loadAssets() override;
};

} // End of namespace Hypno

#endif

