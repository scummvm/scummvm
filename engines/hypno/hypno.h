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
#include "common/compression/installshieldv3_archive.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/str-array.h"
#include "common/stream.h"
#include "engines/engine.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"

#include "hypno/grammar.h"
#include "hypno/libfile.h"

namespace Image {
class ImageDecoder;
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

// Player positions

enum PlayerPosition {
	kPlayerTop = 'T',
	kPlayerBottom = 'B',
	kPlayerLeft = 'L',
	kPlayerRight = 'R'
};

// Common colors
enum HypnoColors {
	kHypnoNoColor = -1,
	kHypnoColorRed = 250,
	kHypnoColorGreen = 251,
	kHypnoColorWhiteOrBlue = 252,
	kHypnoColorYellow = 253,
	kHypnoColorBlack = 254,
	kHypnoColorCyan = 255
};

// Spider colors
enum SpiderColors {
	kSpiderColorWhite = 248,
	kSpiderColorBlue = 252,
};


class HypnoEngine : public Engine {
private:
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
	bool _infiniteHealthCheat;
	bool _infiniteAmmoCheat;
	bool _unlockAllLevels;
	bool _restoredContentEnabled;

	Audio::SoundHandle _soundHandle;
	Common::InstallShieldV3 _installerArchive;
	Common::List<LibFile*> _archive;

	Common::Error run() override;
	Levels _levels;
	Common::HashMap<Common::String, int> _sceneState;
	virtual void resetSceneState();
	bool checkSceneCompleted();
	bool checkLevelWon();
	void runLevel(Common::String &name);
	void runScene(Scene *scene);
	virtual void runBeforeArcade(ArcadeShooting *arc);
	virtual void runAfterArcade(ArcadeShooting *arc);
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
	SegmentShootsSequence parseShootList(const Common::String &name, const Common::String &data);
	void loadArcadeLevel(const Common::String &current, const Common::String &nextWin, const Common::String &nextLose, const Common::String &prefix);
	void loadSceneLevel(const Common::String &current, const Common::String &next, const Common::String &prefix);
	void loadSceneLevel(const char *buf, const Common::String &name, const Common::String &next, const Common::String &prefix);

	LibFile *loadLib(const Common::Path &prefix, const Common::Path &filename, bool encrypted);

	// User input
	void clickedHotspot(Common::Point);
	virtual bool hoverHotspot(Common::Point);

	// Cursors
	bool cursorPauseMovie(Common::Point);
	bool cursorExit(Common::Point);
	bool cursorMask(Common::Point);

	virtual void loadGame(const Common::String &nextLevel, int score, int puzzleDifficulty, int combatDifficulty);
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override { return (isDemo() ? false : true); }
	bool canSaveAutosaveCurrently() override { return false; }
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override { return (isDemo() ? false : true); }
	Common::String _checkpoint;

	Common::Path _prefixDir;
	Common::Path convertPath(const Common::String &);
	void playVideo(MVideo &video);
	void skipVideo(MVideo &video);

	Graphics::Surface *decodeFrame(const Common::String &name, int frame, byte **palette = nullptr);
	Frames decodeFrames(const Common::String &name);
	void loadImage(const Common::String &file, int x, int y, bool transparent, bool palette = false, int frameNumber = 0);
	void drawImage(Graphics::Surface &image, int x, int y, bool transparent);
	void loadPalette(const Common::String &fname);
	void loadPalette(const byte *palette, uint32 offset, uint32 size);
	byte *getPalette(uint32 idx);

	// Cursors
	Common::String _defaultCursor;
	uint32 _defaultCursorIdx;
	void disableCursor();
	void defaultCursor();
	virtual void changeCursor(const Common::String &cursor, uint32 n, bool centerCursor = false);
	virtual void changeCursor(const Common::String &cursor);
	virtual void changeCursor(const Graphics::Surface &entry, byte *palette, bool centerCursor = false);

	// Actions
	virtual void runMenu(Hotspots *hs, bool only_menu = false);
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
	void runSound(Sound *a);
	void runPalette(Palette *a);
	void runAmbient(Ambient *a);
	void runWalN(WalN *a);
	bool runGlobal(Global *a);
	void runTalk(Talk *a);
	void runSwapPointer(SwapPointer *a);
	void runChangeLevel(ChangeLevel *a);
	virtual void drawBackToMenu(Hotspot *h);

	// Screen
	int _screenW, _screenH;
	Graphics::PixelFormat _pixelFormat;
	void changeScreenMode(const Common::String &mode);
	Graphics::Surface *_compositeSurface;
	uint32 _transparentColor;
	Common::Rect screenRect;
	void updateScreen(MVideo &video);
	void updateVideo(MVideo &video);
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
	MVideo *_masks;
	MVideo *_additionalVideo;
	const Graphics::Surface *_mask;

	// Sounds
	Filename _soundPath;
	Filename _music;
	int _musicRate;
	bool _musicStereo;
	bool _doNotStopSounds;
	void playSound(const Filename &filename, uint32 loops, uint32 sampleRate = 22050, bool stereo = false);
	void stopSound();

	// Arcade
	Common::String _arcadeMode;
	MVideo *_background;
	Filename _currentPalette;
	virtual bool availableObjectives();
	virtual bool checkArcadeObjectives();
	ArcadeTransitions _transitions;
	virtual bool checkTransition(ArcadeTransitions &transitions, ArcadeShooting *arc);
	virtual Common::Point getPlayerPosition(bool needsUpdate);
	virtual Common::Point computeTargetPosition(const Common::Point &mousePos);
	virtual int detectTarget(const Common::Point &mousePos);
	virtual void pressedKey(const int keycode);
	virtual bool clickedPrimaryShoot(const Common::Point &mousePos);
	virtual bool clickedSecondaryShoot(const Common::Point &mousePos);
	virtual void drawShoot(const Common::Point &mousePos);
	virtual bool shoot(const Common::Point &mousePos, ArcadeShooting *arc, bool secondary);
	virtual void hitPlayer();
	virtual void missedTarget(Shoot *s, ArcadeShooting *arc);
	virtual void missNoTarget(ArcadeShooting *arc);
	virtual byte *getTargetColor(Common::String name, int levelId);

	// Segments
	Segments _segments;
	uint32 _segmentIdx;
	uint32 _segmentOffset;
	uint32 _segmentRepetition;
	uint32 _segmentRepetitionMax;
	uint32 _segmentShootSequenceOffset;
	uint32 _segmentShootSequenceMax;
	ShootSequence _shootSequence;
	virtual void findNextSegment(ArcadeShooting *arc);
	virtual void initSegment(ArcadeShooting *arc);

	ArcadeStats _stats;
	void resetStatistics();
	void incLivesUsed();
	void incShotsFired();
	void incEnemyHits();
	void incEnemyTargets();
	void incTargetsDestroyed();
	void incTargetsMissed();
	void incFriendliesEncountered();
	void incInfoReceived();

	void incScore(int inc);
	void incBonus(int inc);

	uint32 killRatio();
	uint32 accuracyRatio();

	Common::String _difficulty;
	bool _skipLevel;
	bool _loseLevel;
	bool _skipDefeatVideo;
	bool _skipNextVideo;

	virtual void drawCursorArcade(const Common::Point &mousePos);
	virtual void drawPlayer();
	virtual void drawHealth();
	virtual void drawAmmo();
	int _health;
	int _maxHealth;

	int _ammo;
	int _maxAmmo;

	int _score;
	int _bonus;
	int _lives;

	Common::String _healthString;
	Common::String _scoreString;
	Common::String _objString;
	Common::String _targetString;
	Common::String _directionString;
	Common::String _enterNameString;

	Filename _shootSound;
	Filename _hitSound;
	Filename _additionalSound;
	Shoots _shoots;
	Frames _playerFrames;
	int _playerFrameIdx;
	Common::List<int> _playerFrameSeps;
	int _playerFrameStart;
	int _playerFrameSep;
	int _playerFrameEnd;

	// Objectives
	uint32 _objIdx;
	uint32 _objKillsCount[2];
	uint32 _objMissesCount[2];
	uint32 _objKillsRequired[2];
	uint32 _objMissesAllowed[2];

	// Fonts
	virtual void loadFonts();
	virtual void drawString(const Filename &name, const Common::String &str, int x, int y, int w, uint32 c);

	// Conversation
	Actions _conversation;
	bool _refreshConversation;
	virtual void showConversation();
	virtual void endConversation();
	virtual void rightClickedConversation(const Common::Point &mousePos);
	virtual void leftClickedConversation(const Common::Point &mousePos);
	virtual bool hoverConversation(const Common::Point &mousePos);
	// Credits
	virtual void showCredits();

	// Timers
	int32 _countdown;
	bool _timerStarted;
	bool _keepTimerDuringScenes;
	bool startAlarm(uint32, Common::String *);
	bool startCountdown(uint32);
	void removeTimers();

	// Random
	Common::RandomSource *_rnd;
};

struct chapterEntry {
	int id;
	int energyPos[2];
	int scorePos[2];
	int objectivesPos[2];
	int ammoPos[2];
	int ammoOffset;
	int targetColor;
};

class WetEngine : public HypnoEngine {
public:
	WetEngine(OSystem *syst, const ADGameDescription *gd);
	Common::HashMap<int, const struct chapterEntry*> _chapterTable;
	Common::Array<int> _ids;
	int _lastLevel;
	Common::String _name;

	void loadAssets() override;
	void loadAssetsDemoDisc();
	void loadAssetsGen4();
	void loadAssetsPCW();
	void loadAssetsPCG();
	void loadAssetsFullGame();
	void loadAssetsNI();

	void loadFonts() override;
	void drawString(const Filename &name, const Common::String &str, int x, int y, int w, uint32 c) override;
	void changeCursor(const Common::String &cursor) override;

	void showCredits() override;
	bool clickedSecondaryShoot(const Common::Point &mousePos) override;
	void drawShoot(const Common::Point &target) override;
	void drawPlayer() override;
	void drawHealth() override;
	void drawAmmo() override;
	void hitPlayer() override;
	void drawCursorArcade(const Common::Point &mousePos) override;
	Common::Point computeTargetPosition(const Common::Point &mousePos) override;
	void missedTarget(Shoot *s, ArcadeShooting *arc) override;
	void missNoTarget(ArcadeShooting *arc) override;

	void runCode(Code *code) override;
	Common::String findNextLevel(const Common::String &level) override;
	Common::String findNextLevel(const Transition *trans) override;

	// Saves
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	bool loadProfile(const Common::String &name);
	void saveProfile(const Common::String &name, int levelId);

	// Arcade
	Common::Point getPlayerPosition(bool needsUpdate) override;
	bool checkTransition(ArcadeTransitions &transitions, ArcadeShooting *arc) override;
	void pressedKey(const int keycode) override;
	void runBeforeArcade(ArcadeShooting *arc) override;
	void runAfterArcade(ArcadeShooting *arc) override;
	void findNextSegment(ArcadeShooting *arc) override;
	void initSegment(ArcadeShooting *arc) override;
	byte *getTargetColor(Common::String name, int levelId) override;

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsReturnToLauncher);
	}

private:
	Common::String getLocalizedString(const Common::String name);
	uint16 getNextChar(const Common::String &str, uint32 &c);
	void drawGlyph(const Common::BitArray &font, int x, int y, int bitoffset, int width, int height, int pitch, uint32 color, bool invert);
	void drawKoreanChar(uint16 chr, int &curx, int y, uint32 color);
	void runMainMenu(Code *code);
	void runLevelMenu(Code *code);
	void runCheckLives(Code *code);
	void endCredits(Code *code);
	void showDemoScore();
	uint32 findPaletteIndexZones(uint32 id);

	Common::List<int> _scoreMilestones;
	void restoreScoreMilestones(int score);
	bool checkScoreMilestones(int score);


	Frames _c33PlayerCursor;
	Common::Point _c33PlayerPosition;
	Common::List<PlayerPosition> _c33PlayerDirection;
	bool _c33UseMouse;
	void generateStaticEffect();

	Common::BitArray _font05;
	Common::BitArray _font08;
	Common::BitArray _fontg9a;
	Common::Array<uint32> _c40SegmentPath;
	Common::Array<uint32> _c40SegmentNext;
	int _c40SegmentIdx;
	int _c40lastTurn;
	int _c50LeftTurns;
	int _c50RigthTurns;
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
	void missedTarget(Shoot *s, ArcadeShooting *arc) override;
	void hitPlayer() override;

	// Arcade
	void pressedKey(const int keycode) override;
	void runBeforeArcade(ArcadeShooting *arc) override;
	void runAfterArcade(ArcadeShooting *arc) override;
	void findNextSegment(ArcadeShooting *arc) override;
	void initSegment(ArcadeShooting *arc) override;
	byte *getTargetColor(Common::String name, int levelId) override;

	void drawBackToMenu(Hotspot *h) override;
	void runCode(Code *code) override;
	Common::String findNextLevel(const Common::String &level) override;
	Common::String findNextLevel(const Transition *trans) override;

	void loadFonts() override;
	void drawString(const Filename &name, const Common::String &str, int x, int y, int w, uint32 c) override;

	void showConversation() override;
	void endConversation() override;
	void rightClickedConversation(const Common::Point &mousePos) override;
	void leftClickedConversation(const Common::Point &mousePos) override;
	bool hoverConversation(const Common::Point &mousePos) override;

	void loadGame(const Common::String &nextLevel, int score, int puzzleDifficulty, int combatDifficulty) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	bool canSaveAutosaveCurrently() override {
		return false; // No hypno engine should perform autosave using the default implementation
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsSavingDuringRuntime || f == kSupportsLoadingDuringRuntime || f == kSupportsReturnToLauncher);
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
	void runGiveUp();
	void showScore(const Common::String prefix);

	uint32 _currentPlayerPosition;
	uint32 _lastPlayerPosition;

	bool _fuseState[2][10] = {};
	bool _isFuseRust = true;
	bool _isFuseUnreadable = false;
	bool ingredients[7] = {};

	Common::Rect _h1Area;
	Common::Rect _h2Area;
	Common::Rect _h3Area;

	Common::BitArray _font05;
	Common::BitArray _font08;
	const Graphics::Font *_font;
};

class BoyzEngine : public HypnoEngine {
public:
	BoyzEngine(OSystem *syst, const ADGameDescription *gd);
	~BoyzEngine();
	Common::String _name;
	Common::Array<int> _ids;
	int _lastLevel;
	bool _flashbackMode;
	void loadAssets() override;
	void runCode(Code *code) override;
	Common::String findNextLevel(const Common::String &level) override;
	Common::String findNextLevel(const Transition *trans) override;

	// Scenes
	void resetSceneState() override;
	void runMenu(Hotspots *hs, bool only_menu = false) override;
	bool hoverHotspot(Common::Point) override;

	// Arcade
	void runBeforeArcade(ArcadeShooting *arc) override;
	void runAfterArcade(ArcadeShooting *arc) override;
	void pressedKey(const int keycode) override;
	int detectTarget(const Common::Point &mousePos) override;
	void drawCursorArcade(const Common::Point &mousePos) override;
	bool shoot(const Common::Point &mousePos, ArcadeShooting *arc, bool secondary) override;
	bool clickedSecondaryShoot(const Common::Point &mousePos) override;
	void showCredits() override;
	// Stats
	void showArcadeStats(int territory, const ArcadeStats &data);
	ArcadeStats _lastStats;
	ArcadeStats _globalStats;

	void missedTarget(Shoot *s, ArcadeShooting *arc) override;
	void drawHealth() override;
	void drawAmmo() override;
	void drawShoot(const Common::Point &target) override;
	void hitPlayer() override;
	void drawPlayer() override;
	void findNextSegment(ArcadeShooting *arc) override;
	void initSegment(ArcadeShooting *arc) override;
	bool checkTransition(ArcadeTransitions &transitions, ArcadeShooting *arc) override;

	void loadFonts() override;
	void drawString(const Filename &name, const Common::String &str, int x, int y, int w, uint32 c) override;

	// Saves
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::StringArray listProfiles();
	bool loadProfile(const Common::String &name);
	void saveProfile(const Common::String &name, int levelId);

	private:
	void renderHighlights(Hotspots *hs);
	void waitForUserClick(uint32 timeout);
	int pickABox();
	int _selectedCorrectBox;
	char selectDirection();

	void runMainMenu(Code *code);
	bool runExitMenu();
	void runRetryMenu(Code *code);
	void runCheckC3(Code *code);
	void runCheckHo(Code *code);
	void runCheckC5(Code *code);
	void runAlarmC5(Code *code);
	void runDifficultyMenu(Code *code);
	void endCredits(Code *code);
	int getTerritory(const Common::String &level);
	Common::String lastLevelTerritory(const Common::String &level);
	Common::String firstLevelTerritory(const Common::String &level);

	void loadSceneState(Common::SeekableReadStream *stream);
	void saveSceneState(Common::WriteStream *stream);
	void unlockAllLevels();

	int _previousHealth;
	Graphics::Surface _healthBar[7];
	Graphics::Surface _ammoBar[7];
	Graphics::Surface _portrait[7];
	Filename _deathDay[7];
	Filename _deathNight[7];

	Filename _weaponShootSound[8];
	Filename _weaponReloadSound[8];
	Filename _heySound[7];
	int _weaponMaxAmmo[8];

	byte *_crosshairsPalette;
	Graphics::Surface _crosshairsInactive[8];
	Graphics::Surface _crosshairsActive[8];
	Graphics::Surface _crosshairsTarget[8];
	Graphics::Surface _leftArrowPointer;
	Graphics::Surface _rightArrowPointer;
	Graphics::Surface _crossPointer;

	void updateFromScript();
	bool checkCup(const Common::String &name);

	Script _currentScript;
	ScriptMode _currentMode;
	uint32 _currentActor;
	uint32 _currentWeapon;
	Common::Array<Filename> _warningVideosDay;
	Common::Array<Filename> _warningVideosNight;
	Common::Array<Filename> _warningAlarmVideos;
	Filename _warningHostage;

	Common::Array<Filename> _deathVideo;
	Common::HashMap<Common::String, bool> _shootsDestroyed;

	Common::BitArray _font05;
	Common::BitArray _font08;

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsReturnToLauncher);
	}
};

} // End of namespace Hypno

#endif
