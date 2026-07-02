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

#if !defined(SCUMM_INSANE_REBEL2_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_INSANE_REBEL2_H

#include "scumm/nut_renderer.h"

#include "scumm/smush/smush_player.h"

#include "scumm/insane/insane.h"
#include "scumm/insane/rebel/rebel_audio.h"
#include "scumm/insane/rebel/rebel_gamepad.h"

#include "common/keyboard.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/events.h"
#include "common/queue.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Audio {
class QueuingAudioStream;
class SoundHandle;
}

namespace Scumm {

class InsaneRebel2 : public Insane, public Common::EventObserver {

public:
	InsaneRebel2(ScummEngine_v7 *scumm);
	~InsaneRebel2();

	bool notifyEvent(const Common::Event &event) override;

	Common::Queue<Common::Event> _menuEventQueue;
	bool _menuInputActive;
	bool _virtualKeyboardActive;

	// Menu state.
	enum GameState {
		kStateIntro = 0,
		kStateMainMenu = 1,
		kStatePilotSelect = 2,
		kStateChapterSelect = 3,
		kStateGameplay = 4,
		kStateCredits = 5,
		kStateQuit = 6,
		kStateTopPilots = 8,
		kStateDifficultySelect = 7,
		kStateOptions = 9
	};

	enum MenuResult {
		kMenuQuit = -1,
		kMenuResumeDemo = 0,
		kMenuCredits = 1,
		kMenuNewGame = 2
	};

	GameState _gameState;
	int _menuSelection;
	int _menuItemCount;
	int _menuInactivityTimer;
	bool _menuInactivityTimedOut;
	int _lastMenuVariant;
	int _menuRepeatDelay;
	bool _menuSelectionConfirmed;
	bool _levelUnlocked[16];

	int runMainMenu();
	int processMenuInput();

	// Format-code-aware string rendering (^fNN=font, ^cNNN=color)
	// parseFormatCode: advances str past ^fNN/^cNNN/^^/^l codes.
	// Returns: fontIdx (>=0) on font change, -2 on color/newline, -1 on no match.
	static int parseFormatCode(const char *&str, int &outColor);

	int getMenuStringWidth(const char *str) const;
	void drawMenuString(byte *renderBitmap, const char *str, int x, int y, int defaultColor = 1);
	void drawMenuStringCentered(byte *renderBitmap, const char *str, int cx, int y, int defaultColor = 1);
	void drawMenuStringRight(byte *renderBitmap, const char *str, int rx, int y, int defaultColor = 1);

	void drawMenuItems(byte *renderBitmap, int pitch, int width, int height,
	                   const char **items, int numItems, int selection,
	                   bool leftAligned = false);
	void drawMenuOverlay(byte *renderBitmap, int pitch, int width, int height);
	Common::String getRandomMenuVideo();
	void resetMenu();
	bool isMenuTextInputActive() const;
	void setVirtualKeyboardVisible(bool visible);
	void updateMenuVirtualKeyboard();

	// Chapter selection.
	enum ChapterSelectResult {
		kChapterSelectBack = 2,
		kChapterSelectPlay = 5,
		kChapterSelectQuit = 0
	};

	int _chapterSelection;
	int _chapterItemCount;
	int _selectedChapter;
	Common::String _passwordInput;
	bool _chapterUnlocked[16];
	bool _debugUnlockAll;
	bool _noDamage;

	void unlockAllChapters();
	int runChapterSelect();
	void drawChapterSelectOverlay(byte *renderBitmap, int pitch, int width, int height);
	int processChapterSelectInput();
	void drawPreviewBox(byte *renderBitmap, int pitch, int width, int height);

	int16 _previewOffsetX;
	int16 _previewOffsetY;

	void drawChapterInfoLine(byte *renderBitmap, int pitch, int width, int height);
	Common::String getRankString(int rating);
	Common::String getChapterPassword(int level, int difficulty);

	// Top pilots.
	static const int kMaxRankings = 15;

	struct RankingEntry {
		char name[40];
		int32 score;
		int32 rating;
		int16 difficulty;
		int16 chapter;
	};

	RankingEntry _rankings[kMaxRankings];
	int _numRankings;

	void initDefaultRankings();
	void insertRanking(const char *name, int32 score, int32 rating, int16 difficulty, int16 chapter);
	void showTopPilots();
	void drawTopPilotsOverlay(byte *renderBitmap, int pitch, int width, int height);

	int _topPilotsFrameCount;
	int _topPilotsMaxFrames;

	// Options menu.
	void showOptionsMenu();
	void drawOptionsOverlay(byte *renderBitmap, int pitch, int width, int height);
	int processOptionsInput();

	int _optionsSelection;
	int _optionsItemCount;
	bool _optionsExitRequested;
	bool _optMusicEnabled;
	bool _optSfxEnabled;
	bool _optVoicesEnabled;
	bool _optTextEnabled;
	bool _optControlsFlipped;
	bool _optRapidFire;
	int _optVolumeLevel;

	// Pilot profiles.
	static const int kMaxPilots = 10;
	static const int kMaxPilotNameLen = 15;
	static const int kNumLevels = 16;

	struct PilotData {
		char name[kMaxPilotNameLen + 1];
		int32 score[kNumLevels];
		int32 lives[kNumLevels];
		int32 damage[kNumLevels];
		int16 rating[kNumLevels];
		int16 difficulty;

		void init() {
			memset(name, 0, sizeof(name));
			memset(rating, 0, sizeof(rating));
			difficulty = 2;
			score[0] = 0;
			lives[0] = 4;
			damage[0] = 0;
			for (int i = 1; i < kNumLevels; i++) {
				score[i] = 0;
				lives[i] = 0xFF;
				damage[i] = 0xFF;
			}
		}
	};

	PilotData _pilots[kMaxPilots];
	int _numPilots;
	int _activePilot;

	bool loadPilots();
	bool savePilots();

	int createNewPilot();
	void deletePilot(int index);
	void copyPilot(int srcIndex);

	void updatePilotProgress(int levelIndex, int32 score, int32 lives, int32 damage, int32 rating);

	enum LevelSelectResult {
		kLevelSelectBack = 0,
		kLevelSelectPlay = 1,
		kLevelSelectQuit = 2
	};

	enum PilotMenuMode {
		kPilotModeSelect = 0,
		kPilotModeNameInput = 1,
		kPilotModeDifficulty = 2,
		kPilotModeCopySelect = 3,
		kPilotModeDeleteSelect = 4
	};
	PilotMenuMode _pilotMenuMode;
	Common::String _pilotNameInput;
	int _pilotEditIndex;

	int _levelSelection;
	int _levelItemCount;
	int _selectedLevel;
	int _difficultySelection;

	int runLevelSelect();
	void drawLevelSelectOverlay(byte *renderBitmap, int pitch, int width, int height);
	int processLevelSelectInput();

	// Level playback.
	enum LevelResult {
		kLevelCompleted = 0,
		kLevelNextLevel = 1,
		kLevelGameOver = 2,
		kLevelQuit = 3,
		kLevelReturnToMenu = 4
	};

	void runGame();
	void playIntroSequence();
	void playMissionBriefing();
	int runLevel(int levelId);
	void playLevelBegin(int levelId);
	void playLevelEnd(int levelId);
	void playLevelEnd(int levelId, int accuracy, int flightErrors, bool skillBonus);
	void playLevelRetry(int levelId);
	void playLevelGameOver(int levelId);
	void playEndingSequence();
	void playCreditsSequence();
	Common::String getLevelDir(int levelId);
	Common::String getLevelPrefix(int levelId);

	// Per-level handlers.
	int runLevel1();
	int runLevel2();
	int runLevel3();
	int runLevel4();
	int runLevel5();
	int runLevel6();
	int runLevel7();
	int runLevel8();
	int runLevel9();
	int runLevel10();
	int runLevel11();
	int runLevel12();
	int runLevel13();
	int runLevel14();
	int runLevel15();

	// Wave bookkeeping shared by looping combat sections.
	struct WaveEndResult {
		WaveEndResult() : creditedBits(0), died(false), quit(false), completed(false), skipped(false) {}

		bool shouldStop() const { return died || quit || completed || skipped; }

		uint16 creditedBits;
		bool died;
		bool quit;
		bool completed;
		bool skipped;
	};

	WaveEndResult processWaveEnd(int16 mask, int16 *budget, int16 threshold, uint16 flags);

	// Shared SAN segment dispatcher for scripted level handlers.
	bool playLevelSegment(const char *filename, uint16 flags, bool recordFrame = true);
	void enableIOSGamepadController();
	void restoreIOSGamepadController();

	int calculateAccuracy(int kills, int misses) const;

	bool handleLevelDeath(int levelId, int phase, const char *deathVideo, const char *retryVideo, int &levelResult);
	void resetLevelAttemptState(int initialPhase);
	void resetLevelPhaseState(bool clearEnemies);
	void clearEmbeddedHudFrames();
	void resetLevelWaveState();
	void resetExplosions();
	void resetHandler7FlightState();

	int getRandomVariant(int max);
	Common::String selectDeathVideoVariant(int levelId, int phase, int frame);
	void playCinematic(const char *filename);

	// Text is progressively revealed during [fadeInFrame, fadeOutFrame)
	void playVideoWithText(const char *filename, int textID, int textX, int textY,
	                       int fadeInFrame, int fadeOutFrame);

	bool _textOverlayActive;
	int _textOverlayID;
	int _textOverlayX;
	int _textOverlayY;
	int _textOverlayFadeIn;
	int _textOverlayFadeOut;

	void renderTextOverlay(byte *renderBitmap, int pitch, int width, int height, int curFrame);
	void renderLevelEndStatsOverlay(byte *renderBitmap, int pitch, int width, int height, int32 curFrame, int32 maxFrame);
	void drawLevelEndTextBlock(byte *renderBitmap, const char *text, int centerX, int y);
	void prepareLevelEndStats(int levelId, int accuracy, int flightErrors, bool skillBonus);
	int calculateLevelEndRating(int accuracy, int accLow, int accHigh, int flightErrors, int errLow, int errHigh, bool skillBonus) const;

	struct LevelEndStatsOverlay {
		bool active;
		int levelId;
		int textX;
		int textY;
		int titleStartBeforeEnd;
		int titleEndBeforeEnd;
		bool hasAccuracy;
		bool hasFlightErrors;
		bool skillBonus;
		int accuracy;
		int flightErrors;
		int bonus;
		int finalScore;
		int oldRating;
		int newRating;
	};
	LevelEndStatsOverlay _levelEndStats;
	void playLevelDeathVariant(int levelId, int phase, int frame);
	void playLevelRetryVariant(int levelId, int phase);

	void centerGameplayAim();
	bool _gameplaySectionActive;
	RebelIOSGamepadControllerState _iosGamepadControllerState;

	int _currentPhase;
	int _deathFrame;
	bool _skipSectionRequested;

	// Resources and fonts.
	NutRenderer *_smush_cockpitNut;
	NutRenderer *_rebelMsgFont;

	NutRenderer *_smush_talkfontNut;
	NutRenderer *_smush_smalfontNut;
	NutRenderer *_smush_titlefontNut;
	NutRenderer *_smush_povfontNut;

	byte _savedPausePalette[768];
	bool _pauseOverlayActive;

	bool _introCursorPushed;


	int32 processMouse() override;
	Common::Point getGameplayAimPoint();
	Common::Point getRebelAutoPlayAimPoint();
	void resetMenuGamepadAxis();
	bool handleMenuGamepadAxisEvent(const Common::Event &event);
	bool handleMenuRawJoystickAxisEvent(const Common::Event &event);
	void updateMenuGamepadAxisKey(int16 oldAxisX, int16 oldAxisY);
	void queueMenuGamepadAxisKey(Common::KeyCode keycode);
	void updateGameplayAimFromGamepad();
	void warpGameplayMouseNow(int x, int y);

	int16 _joystickAxisX;
	int16 _joystickAxisY;

	bool _gamepadAimActive;
	uint32 _gameplayMouseSettleUntil;
	uint32 _lastGameplayMenuCloseTime;
	uint32 _lastMenuGamepadNavigationTime;
	int16 _menuGamepadAxisX;
	int16 _menuGamepadAxisY;
	int _menuGamepadRawAxis;
	int16 _menuGamepadRawAxisX;
	int16 _menuGamepadRawAxisY;
	void openGameplayMainMenu(SmushPlayer *splayer);
	void openMenuMainMenu(SmushPlayer *splayer);
	bool isBitSet(int n) override;
	void setBit(int n) override;

	int getHandler() const { return _rebelHandler; }
	int getHandler25GrdSpriteMode() const { return _grdSpriteMode; }
	bool isHiRes() const;

	void iactRebel2Scene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
				  int16 par1, int16 par2, int16 par3, int16 par4);

	void iactRebel2Opcode2(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode3(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4);
	void updateOpcode6Handler(int16 par2);
	void handleOpcode6Handler8(Common::SeekableReadStream &b, int16 par4);
	void handleOpcode6Handler7(Common::SeekableReadStream &b, int16 par4);
	void handleOpcode6Handler25(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void handleOpcode6Turret(Common::SeekableReadStream &b, int16 par4);
	void handleOpcode6GenericInit(int16 par4);
	void updateOpcode6GenericFlightState();
	void scanOpcode6EmbeddedAnim(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par4);
	void iactRebel2Opcode8(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4);
	bool loadOpcode8Handler7FlySprites(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	bool loadOpcode8Handler7ShotTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	bool loadOpcode8EdgeTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	bool loadOpcode8AuxSfx(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	bool loadOpcode8ShotOriginTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	void loadOpcode8EmbeddedAnim(byte *renderBitmap, Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par3, int16 par4);
	bool handleOpcode8EmbeddedAnim(byte *renderBitmap, byte *animData, int32 animDataSize, int16 par3, int16 par4);
	void iactRebel2Opcode9(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);

	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) override;

	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) override;

	void renderStatusBarBackground(byte *renderBitmap, int pitch, int width, int height,
								   int videoWidth, int videoHeight, int statusBarY);

	void updatePostRenderScroll(int width, int height);
	void updatePostRenderDeath();
	void renderPostRenderMenuCursor(byte *renderBitmap, int pitch, int width, int height);
	bool handlePostRenderMenuModes(byte *renderBitmap, int pitch, int width, int height, bool introPlaying);
	bool handlePostRenderIntro(byte *renderBitmap, int pitch, int width, int height, int32 curFrame, int32 maxFrame);
	void prepareHandler7Viewport(byte *renderBitmap, int pitch, int width, int height);
	void renderGameplayPostFrame(byte *renderBitmap, int pitch, int width, int height,
								 int videoWidth, int videoHeight, int statusBarY, int32 curFrame);
	void updateGameplayDamageEffects(byte *renderBitmap, int pitch, int width, int height);
	void updateGameplayTimedTick(int32 curFrame);
	void checkGameplayPostRenderCollisions(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);

	void renderTurretHudOverlays(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);
	void renderEmbeddedHudOverlays(byte *renderBitmap, int pitch, int width, int height);
	void renderStatusBarSprites(byte *renderBitmap, int pitch, int width, int height,
								int statusBarY, int32 curFrame);

	void renderHandler7Ship(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler7FlySprite(byte *renderBitmap, int pitch, int width, int height,
		bool renderHiRes, int renderScale, int nativeViewX, int nativeViewY,
		int nativeX, int nativeY, NutRenderer *nut, int spriteIndex);

	void renderHandler8Ship(byte *renderBitmap, int pitch, int width, int height);
	void renderVehicleShotImpacts(byte *renderBitmap, int pitch, int width, int height);

	void renderFallbackShip(byte *renderBitmap, int pitch, int width, int height);
	void renderEnemyOverlays(byte *renderBitmap, int pitch, int width, int height, int videoWidth);
	void renderExplosions(byte *renderBitmap, int pitch, int width, int height);
	void renderLaserShots(byte *renderBitmap, int pitch, int width, int height);
	void renderCrosshair(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler8MonitorEffect(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler8PovOverlay(byte *renderBitmap, int pitch, int width, int height);
	Common::Point getHandler7ProjectedPointFor(int16 x, int16 y);
	Common::Point getHandler7ProjectedPoint();
	Common::Point getHandler7ShotTargetPoint();
	Common::Point getHandler8ShotTargetPoint();

	void frameEndCleanup();

	// Opcode 8 resource loading.
	bool loadHandler7FlySprites(Common::SeekableReadStream &b, int64 remaining, int16 par4);
	bool loadHandler7ShotTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);

	bool loadTurretHudOverlay(byte *animData, int32 size, int16 selector);
	bool loadHandler8ShipSprites(byte *animData, int32 size, int16 par4);
	bool loadHandler25GrdSprites(byte *animData, int32 size, int16 par4);

	// Handler 25 shot-origin table from opcode 8 text.
	bool loadHandler25ShotOriginTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining);

	bool loadLevel2Background(byte *animData, int32 size, byte *renderBitmap);

	void procSKIP(int32 subSize, Common::SeekableReadStream &b) override;
	void procPreRendering(byte *renderBitmap) override;

	// Laser beams use color 231 as an opaque texel.
	void drawTexturedLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, NutRenderer *nut, int spriteIdx, int v, bool mask231 = true);

	// Laser rendering.
	struct LaserTexture {
		byte *pixels;
		int16 width;
		int16 height;
	};
	LaserTexture _laserTexture;

	// Per-level palette lookup for laser edge glow.
	byte _edgeTable[256 * 256];
	int16 _rebelDetailMode;

	void initEdgeTable(const byte *data);

	void drawEdgeHighlightLine(byte *dst, int pitch, int width, int height,
	                           int16 x0, int16 y0, int16 x1, int16 y1,
	                           int16 clipLeft, int16 clipTop, int16 clipRight, int16 clipBottom);

	void initLaserTexture(NutRenderer *nut, int spriteIdx);
	void freeLaserTexture();

	void drawLaserBeam(byte *dst, int pitch, int width, int height,
	                   int16 gunX, int16 gunY, int16 targetX, int16 targetY,
	                   int16 animFrame, int16 maxFrames,
	                   int16 widthScale, int16 heightScale, int16 thickness);
	void renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx);
	void renderNutSpriteMirrored(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx, bool mirror);

	struct enemy {
		int id;
		int type;
		Common::Rect rect;
		bool active;
		bool destroyed;
		int explosionFrame;
		bool explosionComplete;
		byte *savedBackground;
		int savedBgWidth;
		int savedBgHeight;
	};

	void initEnemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame, int type = 0);
	void enemyUpdate(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);

	Common::List<enemy> _enemies;

	// Current gameplay handler.
	int _rebelHandler;
	int _rebelLevelType;
	int _rebelStatusBarSprite;

	// Embedded SAN HUD overlays.
	struct EmbeddedSanFrame {
		byte *pixels;
		int width;
		int height;
		int renderX;
		int renderY;
		bool valid;
	};

	EmbeddedSanFrame _rebelEmbeddedHud[16];
	byte _rebelEmbeddedCodec45Palette[0x300];
	byte _rebelEmbeddedCodec45Lookup[0x8000];

	void loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) override;
	void renderEmbeddedFrame(byte *renderBitmap, const EmbeddedSanFrame &frame, int userId);
	void drawHandler25CorridorOverlay(byte *renderBitmap);

	int16 _rebelLinks[512][3];
	void clearBit(int n);
	bool isShootingAllowed();

	struct Explosion {
		int x, y;
		int width, height;
		int counter;
		int scale;
		bool active;
	};

	enum ExplosionFrameAdvance {
		kExplosionAdvanceAfterDraw,
		kExplosionAdvanceBeforeDraw
	};

	Explosion _explosions[5];
	void spawnExplosion(int x, int y, int objectHalfWidth);

	// Collision zones registered by IACT opcode 5.
	struct CollisionZone {
		int16 x1, y1;
		int16 x2, y2;
		int16 x3, y3;
		int16 x4, y4;
		int16 field1;
		int16 field2;
		int16 filterValue;
		int16 subOpcode;
		bool active;
	};

	static const int kMaxCollisionZones = 5;
	CollisionZone _primaryZones[kMaxCollisionZones];
	CollisionZone _secondaryZones[kMaxCollisionZones];
	int _primaryZoneCount;
	int _secondaryZoneCount;

	int16 _corridorLeftX;
	int16 _corridorTopY;
	int16 _corridorRightX;
	int16 _corridorBottomY;

	int16 _hitCooldown;

	void registerCollisionZone(Common::SeekableReadStream &b, int16 subOpcode, int16 par4);
	void resetCollisionZones();
	void checkCollisionZones(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);

	// Handler 7 collision and warning cues.
	void checkHandler7CollisionZones(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);
	bool isHandler7ShipInsideObstacleZone(const CollisionZone &zone, int margin);
	void applyHandler7ObstacleHit(const CollisionZone &zone, int zoneIndex);
	void awardHandler7DodgeScore();
	void checkHandler7ObstacleZones(uint16 &warningMask);
	bool applyPlayerDamage(int damage);
	bool applyHandler7WallDamage(int wallDamage);
	void resetHandler7HorizontalVelocity(int16 velocity);
	void checkHandler7TopBoundary(const CollisionZone &zone, int16 vMargin, int wallDamage, uint16 &warningMask);
	void checkHandler7BottomBoundary(const CollisionZone &zone, int16 vMargin, int wallDamage, uint16 &warningMask);
	void checkHandler7LeftBoundary(const CollisionZone &zone, int16 hMargin, int wallDamage);
	void checkHandler7RightBoundary(const CollisionZone &zone, int16 hMargin, int wallDamage);
	void checkHandler7BoundaryZones(uint16 &warningMask);
	void renderHandler7WarningCues(byte *renderBitmap, int pitch, int width, int height, int32 curFrame, uint16 warningMask);
	void updateLevel7Fork(int32 curFrame);
	void updateLevel15TypeSwitch(int32 curFrame);

	int16 _playerDamage;
	int16 _playerShield;
	int16 _playerLives;
	int32 _playerScore;
	int32 _playerRating;

	int _viewX;
	int _viewY;
	int _hiResPresentationViewX;
	int _hiResPresentationViewY;
	int _gameplayPresentationClipBottom;

	// Palette flash and screen shake after damage.
	void triggerDamageEffect();
	void initDamageFlash();
	void updateDamageFlashPalette();
	void updateDamageEffect(byte *renderBitmap, int pitch, int width, int height);
	void resetDamageFlash();
	void restoreDamageFlashPalette();

	int16 _damageFlashCounter;
	int16 _damageHighFlashCounter;
	int16 _damageShakeCounter;
	byte _damageSavedPalette[0x300];
	byte _damageRestorePalette[0x300];
	bool _damageRestorePaletteValid;

	// Per-level counters and debug flags.
	bool _rebelOp6Initialized;
	int _rebelHitCounter;
	int _rebelKillCounter;
	bool _rebelYodaMode;
	bool _rebelMovieMode;
	bool _rebelAutoPlay;

	int _rebelWaveState;
	int _rebelPhaseState;

	int _rebelAutopilot;
	int _rebelDamageLevel;
	int _rebelFlightDir;
	int _rebelControlMode;
	int _rebelInputThrottle;

	int _rebelViewOffsetX;
	int _rebelViewOffsetY;
	int _rebelViewOffset2X;
	int _rebelViewOffset2Y;
	int _rebelViewMode1;
	int _rebelViewMode2;

	// Turret (0x26) gauge groups, addressed by value 100-109 or bitmask > 0x3ff (bit k-1 -> slot k); surfaces show while nonzero, then blink out.
	short _rebelValueCounters[10];
	short _rebelGaugeBlink[10];
	int _rebelLastCounter;

	// Turret dodge-fail view-shake impulses, ring-filtered into the scroll offset.
	int16 _turretShakeRingX[15];
	int16 _turretShakeRingY[15];

	bool _rebelShieldGateActive;
	bool _rebelShieldDestroyed;
	bool _rebelReactorMode;
	bool _rebelGaugeArmed;
	int _rebelLastArmedSlot;
	void resetShieldGauge();
	void decrementGaugeGroup(int slot, int targetId);

	// Handler-specific shot state.
	struct TurretShot {
		int16 counter;
		int16 targetX;
		int16 targetY;
		int16 seqNum;
		int16 gunX;
		int16 gunY;
	};
	TurretShot _turretShots[2];
	int16 _turretShotSeqCounter;

	struct VehicleShot {
		int16 counter;
		int16 targetX;
		int16 targetY;
	};
	VehicleShot _vehicleShots[2];

	struct VehicleShotImpact {
		int16 counter;
		int16 x;
		int16 y;
		int16 spriteIndex;
	};
	VehicleShotImpact _vehicleShotImpacts[7];
	int16 _vehicleShotImpactIndex;

	struct SpaceShot {
		int16 counter;
		int16 targetX;
		int16 targetY;
		int16 leftGunX;
		int16 leftGunY;
		int16 rightGunX;
		int16 rightGunY;
		int16 variant;
	};
	SpaceShot _spaceShots[2];
	int16 _spaceShotDirection;
	int16 _flyLeftGunX[35];
	int16 _flyLeftGunY[35];
	int16 _flyRightGunX[35];
	int16 _flyRightGunY[35];
	bool _flyLeftGunTableLoaded;
	bool _flyRightGunTableLoaded;

	void spawnTurretShot(int x, int y);
	void spawnVehicleShot(int x, int y);
	void spawnSpaceShot(int x, int y);
	void spawnHandler25Shot(int x, int y);
	void spawnShot(int x, int y);

	void renderExplosionFrame(byte *renderBitmap, int pitch, int width, int height,
	                          Explosion &explosion, int screenX, int screenY, ExplosionFrameAdvance advance,
	                          bool resolutionDependentScale);
	void renderTurretExplosions(byte *renderBitmap, int pitch, int width, int height);
	void renderVehicleExplosions(byte *renderBitmap, int pitch, int width, int height);
	void renderSpaceExplosions(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler25Explosions(byte *renderBitmap, int pitch, int width, int height);

	void renderTurretLaserShots(byte *renderBitmap, int pitch, int width, int height);
	void renderVehicleLaserShots(byte *renderBitmap, int pitch, int width, int height);
	void renderSpaceLaserShots(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler25LaserShots(byte *renderBitmap, int pitch, int width, int height);

	int16 getShotMaxDuration();

	// Handler 8 ship and background state.
	NutRenderer *_shipSprite;
	NutRenderer *_shipSprite2;
	NutRenderer *_shipOverlay1;
	NutRenderer *_shipOverlay2;

	byte *_level2Background;
	bool _level2BackgroundLoaded;

	int16 _shipPosX;
	int16 _shipPosY;
	int16 _shipTargetX;
	int16 _shipTargetY;
	int16 _shipLevelMode;
	char _handler8HudGlyph;
	int16 _handler8HudMessageTimer;
	int16 _handler8HudMessageIndex;

	// Handler 8 movement range transitions between cover and shooting states.
	int16 _movementRangeLimit;

	// Handler 7 control mode selects shooting and collision behavior.
	int16 _flyControlMode;

	bool _shipFiring;
	uint32 _prevMouseButtons;

	int16 _shipDirectionIndex;
	int16 _shipDirectionH;
	int16 _shipDirectionV;

	// Handler 7 FLY ship state.
	NutRenderer *_flyShipSprite;
	NutRenderer *_flyLaserSprite;
	NutRenderer *_flyTargetSprite;
	NutRenderer *_flyHiResSprite;
	int16 _flyEffectAnimCounter;
	int16 _flyOverlayRepeatCount;

	int16 _flyShipScreenX;
	int16 _flyShipScreenY;

	// Level 7 (handler-7 flight) corridor fork: 07PLAY -> 07PLAYB right branch.
	bool _level7ForkActive;     // armed only while 07PLAY is playing
	bool _level7TookRightFork;  // sampled at frame 0x638: ship on the right half

	int16 _smoothedVelocity;
	int16 _verticalInput;
	int16 _velocityHistory[25];
	int16 _windHistoryX[15];
	int16 _windHistoryY[15];
	int16 _windParamX;
	int16 _windParamY;

	int16 _perspectiveX;
	int16 _perspectiveY;
	int16 _viewShift;
	bool _facingRight;

	// Handler 25 GRD ship state.
	NutRenderer *_grd001Sprite;
	NutRenderer *_grd002Sprite;
	NutRenderer *_grd005Sprite;

	int16 _grdShotOriginX[30];
	int16 _grdShotOriginY[30];
	bool _grdShotOriginTableLoaded;

	int16 _grdSpriteMode;

	void renderHandler25ShipPre(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler25Ship(byte *renderBitmap, int pitch, int width, int height);

	// Handler 0x26 turret HUD overlays.
	NutRenderer *_hudOverlayNut;
	NutRenderer *_hudOverlay2Nut;

	int _difficulty;

	// Per-level difficulty parameters.
	struct LevelDifficultyParams {
		int16 laserDelay;
		int16 snapDistance;
		int16 missDamage;
		int16 dodgeDamage;
		int16 shotDamage;
		int16 specialDamage;
		int16 shotAccuracy;
		int16 hitPoints;
		int16 dodgePoints;
		int16 timePoints;
		int16 levelPoints;
		int16 specialPoints;
		int16 flags;
		int16 rollRate;
		int16 liftRate;
		int16 slideRate;
		int16 driftRate;
	};

	static const LevelDifficultyParams kDifficultyTable[6][17];

	int getDifficultyRow() const;
	LevelDifficultyParams getDifficultyParams() const;
	int16 getWaveBudgetBase(int phase) const;
	bool _level15SecondHalf;

	void addScore(int points);
	void renderScoreHUD(byte *renderBitmap, int pitch, int width, int height, int statusBarY);

	void showPauseOverlay();

	int _targetLockTimer;

	// Audio.
	RebelAudio _audio;

	void initAudio(int sampleRate);
	void terminateAudio();
	void resetVideoAudio();
	void processAudioFrame(int16 feedSize);
	void queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan);

	// One-shot SFX.
	static const int kRA2NumSfx = 8;

	byte *_sfxData[kRA2NumSfx];
	uint32 _sfxSize[kRA2NumSfx];
	Audio::SoundHandle _sfxHandles[kRA2NumSfx];

	void loadSfx();
	void freeSfx();
	void playSfx(int slot, int volume, int pan);

	// Embedded sound effects.
	static const int kRA2NumAuxSfx = 4;
	static const int kRA2AuxBufSize = 30000;

	byte *_auxSfxData[kRA2NumAuxSfx];
	uint32 _auxSfxSize[kRA2NumAuxSfx];
	Audio::SoundHandle _auxSfxHandles[kRA2NumAuxSfx];

	void loadAuxSfx(int buffer, const byte *data, uint32 size);
	void playAuxSfx(int buffer, int volume, int pan);

};

} // namespace Scumm

#endif
