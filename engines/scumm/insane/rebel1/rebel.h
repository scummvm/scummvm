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

#if !defined(SCUMM_INSANE_REBEL1_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_INSANE_REBEL1_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/events.h"
#include "scumm/insane/insane.h"
#include "scumm/insane/rebel/rebel_audio.h"
#include "scumm/insane/rebel/rebel_gamepad.h"
#include "scumm/smush/rebel/smush_player_ra1.h"

namespace Scumm {

class ScummEngine_v7;
class SmushFont;

enum RA1MenuCommand {
	kRA1MenuCommandNone = 0,
	kRA1MenuCommandUp,
	kRA1MenuCommandDown,
	kRA1MenuCommandLeft,
	kRA1MenuCommandRight,
	kRA1MenuCommandAccept,
	kRA1MenuCommandCancel,
	kRA1MenuCommandSelect1,
	kRA1MenuCommandSelect2,
	kRA1MenuCommandSelect3,
	kRA1MenuCommandSelect4,
	kRA1MenuCommandSelect5,
	kRA1MenuCommandSelect6
};

// Sprite bank for RA1 NUT files.
struct RA1Sprite {
	int16 xoffs;
	int16 yoffs;
	uint16 width;
	uint16 height;
	byte *data;
};

struct RA1SpriteBank {
	int numSprites;
	RA1Sprite *sprites;
	byte *decodedData;
	uint32 decodedSize;

	RA1SpriteBank() : numSprites(0), sprites(nullptr), decodedData(nullptr), decodedSize(0) {}
	~RA1SpriteBank() { delete[] sprites; free(decodedData); }
};

static const int16 kRA1CenterX = 160;
static const int16 kRA1CenterY = 100;
static const int16 kRA1MinX = 20;
static const int16 kRA1MaxX = 300;
static const int16 kRA1MinY = 20;
static const int16 kRA1MaxY = 180;
static const int16 kRA1FocalX = 43;
static const int16 kRA1FocalY = 25;

int ra1ShotDirection(int16 x1, int16 y1, int16 x2, int16 y2);

class InsaneRebel1 : public Insane, public Common::EventObserver {
public:
	InsaneRebel1(ScummEngine_v7 *scumm);
	~InsaneRebel1() override;

	SmushPlayerRebel1 *ra1Player() const { return static_cast<SmushPlayerRebel1 *>(_player); }

	bool notifyEvent(const Common::Event &event) override;

	void procPreRendering(byte *renderBitmap) override;
	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, int32 curFrame, int32 maxFrame) override;
	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
		int16 par1, int16 par2, int16 par3, int16 par4) override;
	void procSKIP(int32 subSize, Common::SeekableReadStream &b) override;

	void handleGameChunk(int32 subSize, Common::SeekableReadStream &b,
		byte *renderBitmap = nullptr, int width = 0, int height = 0);
	bool isInteractiveVideoActive() const { return _interactiveVideoActive; }
	// Touch devices use absolute aiming instead of cursor locking.
	bool isTouchscreenActive() const;
	void setFrameHasGameChunk(bool hasGameChunk) { _frameHasGameChunk = hasGameChunk; }
	void setCurrentSmushFrame(int32 frame);
	int getCurrentLevel() const { return _currentLevel; }
	int getLevelGameplayPhase() const { return _levelGameplayPhase; }
	uint16 getActiveGameOpcode() const { return _activeGameOpcode; }
	uint16 getEffectiveGameOpcode() const;
	uint16 getTargetHitGameOpcode() const;
	bool hasFrameGameOpcode(uint16 opcode) const {
		return opcode < 32 && (_frameGameOpcodeMask & (1u << opcode)) != 0;
	}
	void setFrameGameOpcodeHintMask(uint32 opcodeMask) { _frameGameOpcodeHintMask = opcodeMask; }
	void warpGameplayMouseNow(int x, int y);
	int16 getPerspectiveX() const { return _perspectiveX; }
	int16 getPerspectiveY() const { return _perspectiveY; }
	void projectGameplayPoint(int16 &x, int16 &y) const;
	void unprojectGameplayPoint(int16 &x, int16 &y) const;
	int16 getGameplayCursorX() const;
	int16 getGameplayCursorY() const;
	int16 getGameplayCursorX(uint16 opcode) const;
	int16 getGameplayCursorY(uint16 opcode) const;
	void setGameplayCursor(int16 x, int16 y);
	void setGameplayCursor(uint16 opcode, int16 x, int16 y);
	void updateFlightVariantCursor();
	bool handleFrameObjectTarget(int16 objectId, int16 left, int16 top, int16 width, int16 height,
		int codec, uint8 &ra1Param);
	void resetFrameObjectState();

	void runGame();
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false);
	Common::Error loadGameState(int slot, bool startupLoad = false);
	bool shouldAbortGameFlow() const { return _vm->shouldQuit() || _loadRequested; }

private:
	void playIntroSequence();
	void clearVideoBuffer();

	int runMainMenu();
	int runLevelSelectMenu();

	bool runLevel1();

	bool runLevel2();
	bool runLevel3();
	bool runLevel4();
	bool runLevel5();
	bool runLevel6();
	bool runLevel7();
	bool runLevel8();
	bool runLevel9();
	bool runLevel10();
	bool runLevel11();
	bool runLevel12();
	bool runLevel13();
	bool runLevel14();
	bool runLevel15();

	// Passive, skippable cinematic playback.
	void playCinematic(const char *filename, int32 startFrame = 0);
	void playLevelTransitionCutscene(int level);
	void playChapterCompleteCinematic(const char *filename, int16 unlockedChapter,
		int revealOffsetFromEnd, int stopOffsetFromEnd,
		const char *bonusLabel1 = nullptr, const char *detailText1 = nullptr, int bonusValue1 = 0,
		const char *bonusLabel2 = nullptr, const char *detailText2 = nullptr, int bonusValue2 = 0,
		int passwordIndex = 0);
	bool playDeathOrRetry(const char *retryVideo, const char *gameOverVideo);
	void autosaveProgress();
	int getAutosaveTargetSlot() const;
	int getCurrentSaveLevel() const;
	struct SaveState {
		int resumeLevel;
		int lives;
		int score;
		int prevScore;
		int difficulty;
		int maxChapterUnlocked;
	};
	Common::Error writeSaveState(int slot, const Common::String &desc, const SaveState &state) const;
	bool readSaveState(int slot, SaveState &state, Common::String *desc = nullptr) const;
	void resetLevelDamageState();
	void resetLevelFrameState();
	void resetLevelTargetingState(bool resetKillCount = true);
	void resetLevelFlightState(int16 shipDirIndex = 17);
	void resetLevelInputHistory(bool resetAxisDeltaX = false);
	void resetLevelAttemptState(int16 flyControlMode, int16 gameplayPhase,
		int16 shipDirIndex = 17, bool resetAxisDeltaX = false);

	void playInteractiveVideo(const char *filename, int32 startFrame = 0);
	void resetInteractiveVideoAudio();
	void preserveInteractiveVideoAudioState();
	void restoreInteractiveVideoAudioState();
	void setupInteractiveVideoState(int32 startFrame);
	void resolveSeek(const char *filename, int32 startFrame, int32 &videoOffset, int32 &videoStartFrame);
	void captureInteractiveVideoInput();
	void releaseInteractiveVideoInput();
	void playInteractiveVideoFile(const char *filename, int32 videoOffset, int32 videoStartFrame);
	void enableIOSGamepadController();
	void restoreIOSGamepadController();
	void openGameplayMainMenu();
	bool loadRA1Nut(const char *filename, RA1SpriteBank &bank, bool warnIfMissing = true);
	void loadLevelSprites(int level);
	void handleGameOpcode5EReset(uint32 param1);
	void handleGameOpcode5DLinkLatch(uint32 param1);
	void handleGameOpcode5FRandomHitLatch(uint32 param1);
	void handleGameOpcode07ShipFlight(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameOpcode0DCorridor(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameOpcode0EZone(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameOpcode0BFirstPerson(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameOpcode5ATarget(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameCounterOpcode(uint32 opcode, int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void updateShipPhysics();
	void updateTurretPhysics();
	void updateTurretShipDirection(int16 offsetY);
	void getCollisionShipCenter(int16 &x, int16 &y) const;
	bool isOp0BReticleControlLevel() const;
	bool shouldInvertTouchYSettingForCurrentLevel() const;
	bool usesGamepadReticleAimForCurrentFrame() const;
	void resetGamepadReticleAim();
	bool updateGamepadReticleAim(int16 &inputX, int16 &inputY, bool *usedJoystick);
	void preprocessMouseAxes(int16 &inputX, int16 &inputY, bool *usedJoystick = nullptr);
	void rebuildProjectionTable(int16 curveStep, int16 curveExtent);
	void resetProjectionTable();
	void checkDynamicLevelBranch(int32 curFrame = -1);
	void renderShip(byte *dst, int pitch, int width, int height);
	void renderHUD(byte *dst, int pitch, int width, int height);
	void renderMainMenuOverlay(byte *dst, int pitch, int width, int height);
	void renderTextEntryOverlay(byte *dst, int pitch, int width, int height);
	void renderExplosions(byte *dst, int pitch, int width, int height);
	void renderTargetBoxes(byte *dst, int pitch, int width, int height);
	void renderTargeting(byte *dst, int pitch, int width, int height);
	void renderGostSlots(byte *dst, int pitch, int width, int height);
	void renderGostScorePopup(byte *dst, int pitch, int width, int height,
							  int16 centerX, int16 centerY, int16 frame);
	void renderLaserShots(byte *dst, int pitch, int width, int height);
	void renderShotOverlayPipeline(byte *dst, int pitch, int width, int height,
		bool drawTargetBoxes, bool drawTargeting = true);
	void renderGameOp0BOverlayDuringChunk(byte *dst, int pitch, int width, int height);
	void handleLevel14Play2BSplice(int32 curFrame, int32 maxFrame);
	void renderLevel7RouteOverlays(byte *dst, int pitch, int width, int height);
	void renderLevel5Part2Overlay(byte *dst, int pitch, int width, int height, int32 curFrame);
	void renderLevelHitsOverlay(byte *dst, int pitch, int width, int height, int y,
		bool screenSpace);
	void resetEnemyShotSlots();
	void renderLevel13EnemyShots(byte *dst, int pitch, int width, int height);
	void getTurretShipCenter(int16 &x, int16 &y) const;
	void renderLevel8Overlay(byte *dst, int pitch, int width, int height,
		int viewportX, int viewportY);
	void updateLevel8WalkerState();
	void syncViewportOffset(bool usePerspectiveViewport);
	void renderSprite(byte *dst, int pitch, int width, int height,
					  int x, int y, const RA1Sprite &sprite);
	void updateGostSlotPosition(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom);
	void applyFrameObjectHitState(int16 targetIdx);
	bool isFrameObjectPrimarySet(int16 objectId) const;
	bool areFrameObjectPrimaryBitsSet(int byteIndex, byte mask) const;
	void clearFrameObjectPrimaryBits(int byteIndex, byte mask);
	bool areLevel14Phase1TargetsDestroyed() const;
	bool areLevel14Phase2TargetsDestroyed() const;

	bool isTorpedoModeActive() const;
	void processShot();
	void checkTargetHit(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom);

	// Audio
	void initAudio(int sampleRate);
	void terminateAudio();
	void loadSfx();
	void freeSfx();
	void playSfx(int slot, int volume, int pan);
	void stopSfx(int slot);
	void applyAudioOptions();
	void queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan);
public:
	void drawFontBankString(byte *dst, int pitch, int width, int height, int x, int y, const char *text);
	int getFontBankStringWidth(const char *text);
	int getFontBankLineAdvance(const char *text);
	void processAudioFrame(int16 feedSize);
private:

	ScummEngine_v7 *_vm;

	// Sprite and font banks.
	RA1SpriteBank _shipBank;
	RA1SpriteBank _shipBankAlt;
	RA1SpriteBank _displayBank;
	RA1SpriteBank _titleFontBank;
	RA1SpriteBank _hudFontBank;
	RA1SpriteBank _techFontBank;
	RA1SpriteBank _bangBank;
	RA1SpriteBank _laserBank;
	RA1SpriteBank _enemyLaserBank;
	SmushFont *_menuFont;

	int _screenWidth;
	int _screenHeight;

	// Gameplay input and projection state.
	int16 _shipPosX;
	int16 _shipPosY;
	int16 _flightAimX;
	int16 _flightAimY;
	int16 _shipDirIndex;

	int16 _corridorLeftX;
	int16 _corridorTopY;
	int16 _corridorRightX;
	int16 _corridorBottomY;

	int32 _rollAccum;
	int32 _liftSmooth;
	int32 _posAccumX;
	int32 _posAccumY;
	int16 _turretFrameShipOffsetX;
	int16 _turretFrameShipOffsetY;
	int16 _turretFrameShipCenterX;
	int16 _turretFrameShipCenterY;
	bool _turretFrameShipCenterValid;

	int16 _driftParam;

	int16 _perspectiveX;
	int16 _perspectiveY;
	static const int kProjectionTableSize = 80;
	int16 _projectionCurveExtent;
	int16 _projectionTable[kProjectionTableSize];

	static const int kInputHistorySize = 10;
	int16 _inputHistoryX[kInputHistorySize];
	int16 _inputHistoryY[kInputHistorySize];
	int16 _viewHistoryX[kInputHistorySize];
	int16 _viewHistoryY[kInputHistorySize];
	int16 _inputAxisDeltaX;
	int16 _avgInputX;
	int16 _avgInputY;
	int16 _joystickAxisX;
	int16 _joystickAxisY;
	uint32 _lastJoystickAxisEventTime;
	int16 _level2JoystickFilteredX;
	int16 _level2JoystickFilteredY;
	int16 _gamepadAimAxisX;
	int16 _gamepadAimAxisY;
	bool _gamepadAimActive;
	// Suppress delayed lock/warp mousemove artifacts after centering interactive gameplay.
	uint32 _gameplayMouseSettleUntil;
	enum InputSource {
		kInputSourceMouse,
		kInputSourceJoystickAnalog,
		kInputSourceJoystickDigital,
		kInputSourceJoystickReticle
	};
	InputSource _activeInputSource;

	// GAME opcode 0x0B physics update (scrolling cockpit/surface levels)
	void updateGameOp0BPhysics();
	void updateScreenFlashPalette();
	void restoreScreenFlashPalette();

	// 0x19/0x1A on-foot handler (Level 9 Stormtroopers)
	void initOnFootSequence();
	void preprocessOnFootAim(int16 &inputX, int16 &inputY, bool *usedJoystick = nullptr);
	void updateOnFootSequence();
	void updateOnFootAimVariant();
	void finishOnFootFrame();
	int16 _onFootCharX;
	int16 _onFootCharY;
	int16 _onFootAnimCounter;
	bool _onFootInitialized;

	// Runtime level state.
	int _currentLevel;
	int _resumeLevel;
	int _activeSaveSlot;
	bool _loadRequested;

	bool _introTextActive;
	int32 _introTextStartFrame;
	int32 _introTextEndFrame;
	int _introTextLevel;
	void beginLevelTitleOverlay(int level);
	void drawLevelTitleOverlay(byte *dst, int pitch, int width, int height, int32 curFrame, int32 maxFrame);

	static const int kChapterSummaryTextSize = 80;
	struct ChapterSummaryState {
		bool active;
		int revealOffsetFromEnd;
		int stopOffsetFromEnd;
		bool hasBonus1;
		bool hasBonus2;
		char bonusLabel1[kChapterSummaryTextSize];
		char detailText1[kChapterSummaryTextSize];
		int bonusValue1;
		char bonusLabel2[kChapterSummaryTextSize];
		char detailText2[kChapterSummaryTextSize];
		int bonusValue2;
		int passwordIndex;
	};
	ChapterSummaryState _chapterSummary;
	void beginChapterSummaryOverlay(int revealOffsetFromEnd, int stopOffsetFromEnd,
		const char *bonusLabel1, const char *detailText1, int bonusValue1,
		const char *bonusLabel2, const char *detailText2, int bonusValue2,
		int passwordIndex);
	void drawChapterSummaryOverlay(byte *dst, int pitch, int width, int height, int32 curFrame, int32 maxFrame);

	// Interactive GAME chunk state.
	int16 _flyControlMode;
	int16 _turretEmitterLeftX;
	int16 _turretEmitterLeftY;
	int16 _turretEmitterRightX;
	int16 _turretEmitterRightY;
	uint16 _activeGameOpcode;
	uint32 _frameGameOpcodeHintMask;
	uint32 _frameGameOpcodeMask;
	bool _frameHasGameChunk;
	uint16 _frameDispatchFlags;
	bool _gameOp0BPhysicsUpdatedThisFrame;
	bool _gameOp0BOverlayRenderedThisFrame;

	int _difficulty;

	// 21 sub-levels (1A,1B,2,3,4A,4B,5A,5B,6,7,8,9A,9B,10,11,12,13,14A,14B,15A,15B)
	struct TuningParams {
		int16 roll;
		int16 lift;
		int16 slide;
		int16 drift;
		int16 snap;
		int16 miss;
		int16 wham;
		int16 shot;
		int16 kill;
		int16 time;
		int16 levelPts;
		int16 bonus;
		int16 flags;
	};
	TuningParams _tuning;

	void loadTuningForLevel(int level);
	void resetGameplayFlagsFromTuning();

	int16 _health;
	int16 _lives;
	int _score;
	int _prevScore;
	byte _damageFlags;
	byte _prevDamageFlags;
	uint16 _gameLatch5D;
	uint16 _gameLatch5F;
	int16 _damageCooldown;
	int16 _deathTimer;
	int16 _screenFlash;
	uint32 _frameCounter;        // Gameplay handler frame accumulator
	int32 _currentSmushFrame;
	bool _screenShakeEnabled;
	byte _screenFlashBasePalette[0x300];
	bool _screenFlashBasePaletteValid;
	byte _deathCauseIndicator;
	byte _hudRenderFlag;
	byte _hudDirtyFlag;
	int16 _maxChapterUnlocked;
	bool _unlockAllLevels;
	bool _noDamage;

	static const int16 kMaxHealth = 98;
	static const int16 kDeathTimerInit = 30;
	static const int16 kDamageCooldownInit = 10;
	enum { kNumLevels = 15 };

	// Streamed SMUSH audio.
	RebelAudio _audio;
	bool _restoreInteractiveVideoAudioState;
	int16 _savedInteractiveVideoTrackState[SMUSH_MAX_TRACKS];
	int _savedInteractiveVideoTrackGroupId[SMUSH_MAX_TRACKS];
	int _savedInteractiveVideoTrackCount;
	static const int kNumSfx = 8;
	enum SfxSlot {
		kSfxLaserShot = 0,
		kSfxExplode = 1,
		kSfxBoom = 2,
		kSfxKlaxon = 3,
		kSfxLockOn = 4,
		kSfxAlert = 5,
		kSfxBonus = 6,
		kSfxBlast = 7
	};
	byte *_sfxData[kNumSfx];
	uint32 _sfxSize[kNumSfx];
	Audio::SoundHandle _sfxHandles[kNumSfx];

	bool _interactiveVideoActive;
	bool _preserveInteractiveRuntimeState;
	bool _interactiveVideoCheatSkipped;
	RebelIOSGamepadControllerState _iosGamepadControllerState;

	// Path branching for levels with left/right alternative videos.
	static const int32 kPathBranchCounter = 394;
	int32 _gameCounter;
	bool _pathBranchEnabled;
	bool _rightPathSelected;
	int _levelRouteIndex;
	int _pendingRouteIndex;
	int32 _pendingRouteStartFrame;
	int32 _pendingRouteCutoverFrame;
	int32 _pendingRouteVideoStartFrame;
	int16 _level7WarningFrames;
	int16 _level7WarningThreshold;
	int _levelGameplayPhase;
	bool _level14Play2BSplicePending;
	bool _level14Play2BSpliced;
	int32 _level14Play2BSpliceFrame;

	void runOptionsMenu();
	int runPasscodeEntryDialog();
	bool runHighScoreNameEntry();
	bool handleMenuCommand(RA1MenuCommand command);
	bool handleControllerMenuAction(ScummAction action);
	bool handleControllerMenuAxis(int16 oldAxisX, int16 oldAxisY);
	bool handleMenuMouse(const Common::Event &event);
	bool handleTextEntryAction(ScummAction action);
	bool handleTextEntryKey(const Common::Event &event);
	int getMainMenuItemCount() const;
	int getMainMenuResultForSelection(int selection) const;
	void playMenuBackground();
	bool runTextEntryMenuLoop();
	void setVirtualKeyboardVisible(bool visible);
	void beginTextEntry(bool passcodeMode);
	void finishTextEntry(bool canceled);
	void selectTextEntryChar();
	const char *getChapterCompletePassword(int passwordIndex) const;
	bool _menuActive;
	bool _menuConfirmed;
	int _menuSelection;
	int _menuFrameCounter;

	static const int kOptionsItemCount = 9;
	bool _optionsActive;
	int _optionsSel;
	bool _levelSelectActive;
	int _levelSelectSel;
	int _startLevel;

	bool _optRookieOneFemale;
	bool _optMusicEnabled;
	bool _optSfxEnabled;
	bool _optTextEnabled;
	bool _optControlsYFlip;
	bool _optRapidFire;
	int  _optVolume;

	static const int kHighScoreCount = 10;
	struct HighScoreEntry {
		char name[20];
		int32 score;
		byte difficulty;
	};
	HighScoreEntry _highScores[kHighScoreCount];
	bool _highScoresActive;
	void showHighScores();
	int getMenuTalkTextWidth(const char *text);
	void drawMenuTalkText(byte *dst, int pitch, int width, int height, int x, int y, const char *text);
	void drawMenuTitleText(byte *dst, int pitch, int width, int height, int x, int y, const char *text);
	void renderHighScoresOverlay(byte *dst, int pitch, int width, int height);
	void renderOptionsOverlay(byte *dst, int pitch, int width, int height);
	void renderLevelSelectOverlay(byte *dst, int pitch, int width, int height);
	void renderMainMenuItems(byte *dst, int pitch, int width, int height);

	// Character picker shared by passcode and high-score entry.
	static const int kTextEntryBufferSize = 20;
	bool _textEntryActive;
	bool _textEntryPasscodeMode;
	bool _textEntryDone;
	bool _textEntryCanceled;
	bool _virtualKeyboardActive;
	int _textEntryPickerIndex;
	int _textEntryPickerOffsetX;
	int _textEntryMaxChars;
	int _highScoreEntryIndex;
	char _textEntryBuffer[kTextEntryBufferSize];

	bool _playerFired;
	// Secondary held button used by L9 on-foot controls.
	bool _playerSecondaryHeld;
	int16 _fireCooldown;
	int16 _rapidFirePhase;
	uint16 _gameplayFlags75fe;
	uint16 _gameplayFlags75ff;

	static const int kMaxShotSlots = 2;
	struct ShotSlot {
		int16 timer;
		int16 posX;
		int16 posY;
		int16 centerX;
		int16 centerY;
		int16 variant;
	};
	ShotSlot _shotSlots[kMaxShotSlots];
	int16 _shotAlternator;
	bool _shotSideToggle;

	int16 _targetProximity;
	int16 _prevTargetProx;
	int16 _targetAnimCounter;
	int16 _targetCount;
	int16 _prevTargetCount;
	static const int kMaxTargetBoxes = 20;
	int16 _targetBoxX[kMaxTargetBoxes];
	int16 _targetBoxY[kMaxTargetBoxes];
	int16 _targetBoxVariant[kMaxTargetBoxes];

	static const int kMaxGostSlots = 10;
	struct GostSlot {
		int16 targetId;
		int16 frame;
		int16 posX;
		int16 posY;
	};
	GostSlot _gostSlots[kMaxGostSlots];
	int16 _gostSlotIdx;

	int16 _killCount;
	int16 _lastHitTarget;
	bool _frameObjectHitRevealPending;

	// Incoming enemy projectile slots.
	static const int kMaxEnemyShotSlots = 5;
	struct EnemyShotSlot {
		int16 timer;
		int16 startX;
		int16 startY;
		int16 targetX;
		int16 targetY;
		int16 direction;
		uint16 flags;
	};
	EnemyShotSlot _enemyShotSlots[kMaxEnemyShotSlots];

	// Targets listed here can be hit repeatedly (no event mask toggle).
	// Used by Level 4 (shield generators) and Level 15 (torpedo targets).
	int16 _protectedTargetA;
	int16 _protectedTargetB;

	// Per-target hit counters for shield generator tracking.
	int16 _shieldGenHitsA;
	int16 _shieldGenHitsB;
	int16 _level5SuccessFramesRemaining;
	int16 _level14SuccessFrames;

	bool _torpedoFired;

	int16 _walkerHealth;
	int16 _walkerTimer;
	int16 _walkerBranchChoice;
	bool _walkerRoundReplay;

	// Attack window frame numbers per route.
	static const int16 kWalkerAttackWindow1[3];
	static const int16 kWalkerAttackWindow2[3];
	static const int16 kWalkerAttackWindow3[3];

	static const int kFrameObjectStateBytes = 300;
	byte _frameObjectState[kFrameObjectStateBytes];
};

} // namespace Scumm

#endif
