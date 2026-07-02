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



#include "engines/engine.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/util.h"
#include "graphics/paletteman.h"

#include "audio/mixer.h"

#include "scumm/actor.h"
#include "scumm/file.h"
#include "scumm/resource.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"

#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse_engine.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/rebel/font_rebel2.h"

#include "scumm/insane/rebel2/rebel.h"

#include "common/config-manager.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Scumm {

const int kRA2MenuAxisThreshold = Common::JOYAXIS_MAX / 2;
const uint32 kRA2MenuGamepadNavigationDebounceMs = 250;
const uint32 kRA2MenuGamepadMouseSuppressMs = 250;
const int kRA2MenuKeyLikeFirstAxis = 8;
const int kRA2MenuKeyLikeAxisTolerance = Common::JOYAXIS_MAX / 14;
const int kRA2Handler7MouseSettleJumpThreshold = 40;
const int kRA2Handler7MouseSettleRelativeThreshold = 40;
const int kRA2Handler7MouseSettleEdgeMargin = 16;
const int kRA2GameplayMouseMaxX = 319;
const int kRA2GameplayMouseMaxY = 199;
const uint32 kRA2Handler7MouseSettleExtendMs = 1000;

int16 normalizeRebel2AxisMagnitude(int16 position) {
	return position == Common::JOYAXIS_MIN ? Common::JOYAXIS_MAX : ABS((int)position);
}

int16 combineRebel2MenuAxis(int16 mappedAxis, int16 rawAxis) {
	return ABS((int)rawAxis) > ABS((int)mappedAxis) ? rawAxis : mappedAxis;
}

int getRebel2MenuAxisDirection(int16 axisValue) {
	if (axisValue >= kRA2MenuAxisThreshold)
		return 1;
	if (axisValue <= -kRA2MenuAxisThreshold)
		return -1;
	return 0;
}

bool decodeRebel2MenuKeyLikeAxis(int axis, int16 position, int16 &axisX, int16 &axisY) {
	if (axis < kRA2MenuKeyLikeFirstAxis)
		return false;

	axisX = 0;
	axisY = 0;

	// Some backends expose a key-like POV as a single extra axis whose neutral
	// value is beyond the normal range and arrives clamped to JOYAXIS_MAX.
	if (position == 0 || position >= Common::JOYAXIS_MAX - kRA2MenuKeyLikeAxisTolerance)
		return true;

	static const int16 povValues[] = {
		-Common::JOYAXIS_MAX,
		-(Common::JOYAXIS_MAX * 5) / 7,
		-(Common::JOYAXIS_MAX * 3) / 7,
		-Common::JOYAXIS_MAX / 7,
		Common::JOYAXIS_MAX / 7,
		(Common::JOYAXIS_MAX * 3) / 7,
		(Common::JOYAXIS_MAX * 5) / 7
	};
	static const int16 povDirections[][2] = {
		{ 0, -Common::JOYAXIS_MAX },
		{ Common::JOYAXIS_MAX, -Common::JOYAXIS_MAX },
		{ Common::JOYAXIS_MAX, 0 },
		{ Common::JOYAXIS_MAX, Common::JOYAXIS_MAX },
		{ 0, Common::JOYAXIS_MAX },
		{ -Common::JOYAXIS_MAX, Common::JOYAXIS_MAX },
		{ -Common::JOYAXIS_MAX, 0 }
	};

	int bestIndex = -1;
	int bestDistance = Common::JOYAXIS_MAX;
	for (uint i = 0; i < ARRAYSIZE(povValues); i++) {
		const int distance = ABS((int)position - povValues[i]);
		if (distance < bestDistance) {
			bestDistance = distance;
			bestIndex = i;
		}
	}

	if (bestIndex >= 0 && bestDistance <= kRA2MenuKeyLikeAxisTolerance) {
		axisX = povDirections[bestIndex][0];
		axisY = povDirections[bestIndex][1];
		return true;
	}

	return false;
}

bool isRebel2MenuDirectionKey(Common::KeyCode keycode) {
	return keycode == Common::KEYCODE_UP ||
	       keycode == Common::KEYCODE_DOWN ||
	       keycode == Common::KEYCODE_LEFT ||
	       keycode == Common::KEYCODE_RIGHT;
}

bool isRebel2MenuState(InsaneRebel2::GameState state) {
	return state == InsaneRebel2::kStateMainMenu ||
	       state == InsaneRebel2::kStatePilotSelect ||
	       state == InsaneRebel2::kStateDifficultySelect ||
	       state == InsaneRebel2::kStateChapterSelect ||
	       state == InsaneRebel2::kStateOptions ||
	       state == InsaneRebel2::kStateTopPilots;
}

InsaneRebel2::InsaneRebel2(ScummEngine_v7 *scumm) {
	_vm = scumm;

	_smush_roadrashRip = nullptr;
	_smush_roadrsh2Rip = nullptr;
	_smush_roadrsh3Rip = nullptr;
	_smush_goglpaltRip = nullptr;
	_smush_tovista1Flu = nullptr;
	_smush_tovista2Flu = nullptr;
	_smush_toranchFlu = nullptr;
	_smush_minedrivFlu = nullptr;
	_smush_minefiteFlu = nullptr;
	_smush_bencutNut = nullptr;
	_smush_bensgoggNut = nullptr;

	const bool highRes = isHiRes();

	_smush_iconsNut = new NutRenderer(_vm, highRes ? "SYSTM/CPITIMHI.NUT" : "SYSTM/CPITIMAG.NUT");
	_smush_icons2Nut = nullptr;

	_laserTexture.pixels = nullptr;
	_laserTexture.width = 0;
	_laserTexture.height = 0;
	if (_smush_iconsNut && _smush_iconsNut->getNumChars() > 5) {
		initLaserTexture(_smush_iconsNut, 5);
	}

	initEdgeTable(nullptr);
	_rebelDetailMode = 1;
	_smush_cockpitNut = new NutRenderer(_vm, highRes ? "SYSTM/DIHIFONT.NUT" : "SYSTM/DISPFONT.NUT");

	_rebelMsgFont = makeRebel2Font(_vm, "SYSTM/DIHIFONT.NUT");

	_smush_talkfontNut = makeRebel2Font(_vm, highRes ? "SYSTM/TKHIFONT.NUT" : "SYSTM/TALKFONT.NUT");
	_smush_smalfontNut = makeRebel2Font(_vm, highRes ? "SYSTM/SMHIFONT.NUT" : "SYSTM/SMALFONT.NUT");
	_smush_titlefontNut = makeRebel2Font(_vm, highRes ? "SYSTM/TIHIFONT.NUT" : "SYSTM/TITLFONT.NUT");
	_smush_povfontNut = makeRebel2Font(_vm, highRes ? "SYSTM/POHIFONT.NUT" : "SYSTM/POVFONT.NUT");

	_pauseOverlayActive = false;
	memset(_savedPausePalette, 0, sizeof(_savedPausePalette));
	memset(_rebelEmbeddedCodec45Palette, 0, sizeof(_rebelEmbeddedCodec45Palette));
	memset(_rebelEmbeddedCodec45Lookup, 0, sizeof(_rebelEmbeddedCodec45Lookup));

	_enemies.clear();
	_rebelHandler = 0;
	_rebelLevelType = 0;
	_rebelStatusBarSprite = 0;
	_introCursorPushed = false;

	_playerDamage = 0;
	_playerShield = 255;
	_playerLives = 3;
	_playerScore = 0;
	_playerRating = 0;
	_noDamage = false;
	_viewX = 0;
	_viewY = 0;
	_hiResPresentationViewX = 0;
	_hiResPresentationViewY = 0;
	_gameplayPresentationClipBottom = 179;

	_damageFlashCounter = 0;
	_damageHighFlashCounter = 0;
	_damageShakeCounter = 0;
	memset(_damageSavedPalette, 0, sizeof(_damageSavedPalette));
	memset(_damageRestorePalette, 0, sizeof(_damageRestorePalette));
	_damageRestorePaletteValid = false;

	_textOverlayActive = false;
	_textOverlayID = 0;
	_textOverlayX = 0;
	_textOverlayY = 0;
	_textOverlayFadeIn = 0;
	_textOverlayFadeOut = 0;
	memset(&_levelEndStats, 0, sizeof(_levelEndStats));

	_rebelOp6Initialized = false;
	_rebelHitCounter = 0;
	_rebelKillCounter = 0;
	_rebelYodaMode = false;
	_rebelMovieMode = false;
	_rebelAutoPlay = false;
	_rebelWaveState = 0;
	_rebelPhaseState = 0;

	_rebelAutopilot = 0;
	_rebelDamageLevel = 0;
	_rebelFlightDir = 0;
	_rebelControlMode = 0;
	_rebelInputThrottle = 0;
	_rebelViewOffsetX = 0;
	_rebelViewOffsetY = 0;
	_rebelViewOffset2X = 0;
	_rebelViewOffset2Y = 0;
	_rebelViewMode1 = 0;
	_rebelViewMode2 = 0;

	for (int i = 0; i < 10; ++i) {
		_rebelValueCounters[i] = 0;
		_rebelGaugeBlink[i] = 0;
	}
	for (int i = 0; i < 15; ++i) {
		_turretShakeRingX[i] = 0;
		_turretShakeRingY[i] = 0;
	}
	_rebelLastCounter = 0;

	_rebelShieldGateActive = false;
	_rebelShieldDestroyed = false;
	_rebelReactorMode = false;
	_rebelGaugeArmed = false;
	_rebelLastArmedSlot = -1;

	_difficulty = 1;
	_targetLockTimer = 0;

	_speed = 12;
	_insaneIsRunning = false;

	_numberArray = 0;
	_emulateInterrupt = 0;
	_flag1d = 0;
	_objArray1Idx = 0;
	_objArray1Idx2 = 0;
	_objArray2Idx = 0;
	_objArray2Idx2 = 0;
	_currSceneId = 1;
	_timer6Id = 0;
	_timerSpriteId = 0;
	_temp2SceneId = 0;
	_tempSceneId = 0;
	_currEnemy = -1;
	_currScenePropIdx = 0;
	_currScenePropSubIdx = 0;
	_currTrsMsg = 0;
	_sceneData2Loaded = 0;
	_sceneData1Loaded = 0;
	_keyboardDisable = 0;
	_needSceneSwitch = false;
	_idx2Exceeded = 0;
	_tiresRustle = false;
	_keybOldDx = 0;
	_keybOldDy = 0;
	_velocityX = 0;
	_velocityY = 0;
	_keybX = 0;
	_keybY = 0;
	_firstBattle = false;
	_battleScene = true;
	_kickBenProgress = false;
	_weaponBenJustSwitched = false;
	_kickEnemyProgress = false;
	_weaponEnemyJustSwitched = false;
	_beenCheated = 0;
	_posBrokenTruck = 0;
	_posBrokenCar = 0;
	_posFatherTorque = 0;
	_posCave = 0;
	_posVista = 0;
	_roadBranch = false;
	_roadStop = false;
	_carIsBroken = false;
	_benHasGoggles = false;
	_mineCaveIsNear = false;
	_objectDetected = false;
	_approachAnim = -1;
	_val54d = 0;
	_val57d = 0;
	_val115_ = false;
	_roadBumps = false;
	_val211d = 0;
	_val213d = 0;
	_metEnemiesListTail = 0;
	_smlayer_room = 0;
	_smlayer_room2 = 0;
	_isBenCut = 0;
	_continueFrame = 0;
	_continueFrame1 = 0;
	_counter1 = 0;
	_iactSceneId = 0;
	_iactSceneId2 = 0;

	int i, j;

	for (i = 0; i < 12; i++)
		_metEnemiesList[i] = 0;

	for (i = 0; i < 9; i++)
		for (j = 0; j < 9; j++)
			_enHdlVar[i][j] = 0;

	for (i = 0; i < 0x401; i++)
		_iactBits[i] = 0;

	for (i = 0; i < 512; i++) {
		_rebelLinks[i][0] = 0;
		_rebelLinks[i][1] = 0;
		_rebelLinks[i][2] = 0;
	}

	for (i = 0; i < 5; i++) {
		_explosions[i].active = false;
		_explosions[i].counter = 0;
	}

	_primaryZoneCount = 0;
	_secondaryZoneCount = 0;
	for (i = 0; i < kMaxCollisionZones; i++) {
		_primaryZones[i].active = false;
		_secondaryZones[i].active = false;
	}
	_corridorLeftX = 0;
	_corridorTopY = 0;
	_corridorRightX = 0x1A8;
	_corridorBottomY = 0x104;
	_hitCooldown = 0;

	for (i = 0; i < 2; i++) {
		_turretShots[i].counter = 0;
		_turretShots[i].targetX = 0;
		_turretShots[i].targetY = 0;
		_turretShots[i].seqNum = 0;
		_turretShots[i].gunX = 0;
		_turretShots[i].gunY = 0;
	}
	_turretShotSeqCounter = 0;

	for (i = 0; i < 2; i++) {
		_vehicleShots[i].counter = 0;
		_vehicleShots[i].targetX = 0;
		_vehicleShots[i].targetY = 0;
	}
	for (i = 0; i < 7; i++) {
		_vehicleShotImpacts[i].counter = 0;
		_vehicleShotImpacts[i].x = 0;
		_vehicleShotImpacts[i].y = 0;
		_vehicleShotImpacts[i].spriteIndex = 0;
	}
	_vehicleShotImpactIndex = 0;

	for (i = 0; i < 2; i++) {
		_spaceShots[i].counter = 0;
		_spaceShots[i].targetX = 0;
		_spaceShots[i].targetY = 0;
		_spaceShots[i].leftGunX = 0;
		_spaceShots[i].leftGunY = 0;
		_spaceShots[i].rightGunX = 0;
		_spaceShots[i].rightGunY = 0;
		_spaceShots[i].variant = 0;
	}
	_spaceShotDirection = 0;
	memset(_flyLeftGunX, 0, sizeof(_flyLeftGunX));
	memset(_flyLeftGunY, 0, sizeof(_flyLeftGunY));
	memset(_flyRightGunX, 0, sizeof(_flyRightGunX));
	memset(_flyRightGunY, 0, sizeof(_flyRightGunY));
	_flyLeftGunTableLoaded = false;
	_flyRightGunTableLoaded = false;

	for (i = 0; i < 16; i++) {
		_rebelEmbeddedHud[i].pixels = nullptr;
		_rebelEmbeddedHud[i].width = 0;
		_rebelEmbeddedHud[i].height = 0;
		_rebelEmbeddedHud[i].renderX = 0;
		_rebelEmbeddedHud[i].renderY = 0;
		_rebelEmbeddedHud[i].valid = false;
	}

	_shipSprite = nullptr;
	_shipSprite2 = nullptr;
	_shipOverlay1 = nullptr;
	_shipOverlay2 = nullptr;
	_level2Background = nullptr;
	_level2BackgroundLoaded = false;
	_shipPosX = 0xa0;
	_shipPosY = 0x28;
	_shipTargetX = 0xa0;
	_shipTargetY = 0x28;
	_shipLevelMode = 0;
	_movementRangeLimit = 127;
	_flyControlMode = 0;
	_shipFiring = false;
	_prevMouseButtons = 0;
	_shipDirectionH = 2;
	_shipDirectionV = 3;
	_shipDirectionIndex = 2 * 7 + 3;

	_flyShipSprite = nullptr;
	_flyLaserSprite = nullptr;
	_flyTargetSprite = nullptr;
	_flyHiResSprite = nullptr;
	_flyEffectAnimCounter = 0;
	_flyOverlayRepeatCount = 0;
	_flyShipScreenX = 0xd4;
	_flyShipScreenY = 0x82;
	_level7ForkActive = false;
	_level7TookRightFork = false;
	_smoothedVelocity = 0;
	_verticalInput = 0;
	memset(_velocityHistory, 0, sizeof(_velocityHistory));
	memset(_windHistoryX, 0, sizeof(_windHistoryX));
	memset(_windHistoryY, 0, sizeof(_windHistoryY));
	_windParamX = 0;
	_windParamY = 0;
	_perspectiveX = 0;
	_perspectiveY = 0;
	_viewShift = 0;
	_facingRight = false;

	_grd001Sprite = nullptr;
	_grd002Sprite = nullptr;
	_grd005Sprite = nullptr;
	_grdSpriteMode = 0;
	memset(_grdShotOriginX, 0, sizeof(_grdShotOriginX));
	memset(_grdShotOriginY, 0, sizeof(_grdShotOriginY));
	_grdShotOriginTableLoaded = false;

	_hudOverlayNut = nullptr;
	_hudOverlay2Nut = nullptr;

	// Individual SMUSH audio blocks carry their own source rate.
	initAudio(11025);

	for (i = 0; i < kRA2NumSfx; i++) {
		_sfxData[i] = nullptr;
		_sfxSize[i] = 0;
	}
	loadSfx();

	for (i = 0; i < kRA2NumAuxSfx; i++) {
		_auxSfxData[i] = (byte *)calloc(kRA2AuxBufSize, 1);
		_auxSfxSize[i] = 0;
	}

	_gameState = kStateMainMenu;
	_menuSelection = 0;
	_menuItemCount = 7;
	_menuInactivityTimer = 0;
	_menuInactivityTimedOut = false;
	_lastMenuVariant = -1;
	_menuRepeatDelay = 0;
	_menuSelectionConfirmed = false;
	for (i = 0; i < 16; i++) {
		_levelUnlocked[i] = (i == 0);
	}

	_chapterSelection = 0;
	_chapterItemCount = 17;
	_selectedChapter = 0;
	_passwordInput = "";

	_debugUnlockAll = ConfMan.getBool("rebel2_unlock_all");
	_noDamage = ConfMan.getBool("rebel2_no_damage");
	_rebelYodaMode = ConfMan.getBool("rebel2_yoda_mode");

	for (i = 0; i < 16; i++) {
		_chapterUnlocked[i] = _debugUnlockAll || (i == 0);
	}

	_previewOffsetX = -90;
	_previewOffsetY = 75;

	_numPilots = 0;
	_activePilot = 0;
	for (i = 0; i < kMaxPilots; i++) {
		_pilots[i].init();
	}
	loadPilots();

	_levelSelection = 0;
	_levelItemCount = _numPilots + 4;
	_selectedLevel = 1;
	_difficultySelection = 2;
	_pilotMenuMode = kPilotModeSelect;
	_pilotNameInput = "";
	_pilotEditIndex = -1;

	_topPilotsFrameCount = 0;
	_topPilotsMaxFrames = 120;
	initDefaultRankings();

	_optionsSelection = 0;
	_optionsItemCount = 8;
	_optMusicEnabled = !_vm->_mixer->isSoundTypeMuted(Audio::Mixer::kMusicSoundType);
	_optSfxEnabled = !_vm->_mixer->isSoundTypeMuted(Audio::Mixer::kSFXSoundType);
	_optVoicesEnabled = !_vm->_mixer->isSoundTypeMuted(Audio::Mixer::kSpeechSoundType);
	// Initialize the dialogue-text (subtitles) toggle from the global setting so the
	// game and the in-game TEXT menu label reflect it. The menu toggle writes the same
	// "subtitles" key, which the text-render paths gate on.
	_optTextEnabled = ConfMan.getBool("subtitles");
	_optControlsFlipped = false;
	_optRapidFire = false;
	_optVolumeLevel = _vm->_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2;

	_menuInputActive = false;
	_virtualKeyboardActive = false;

	_joystickAxisX = 0;
	_joystickAxisY = 0;
	_gamepadAimActive = false;
	_gameplaySectionActive = false;
	_gameplayMouseSettleUntil = 0;
	_lastGameplayMenuCloseTime = 0;
	_lastMenuGamepadNavigationTime = 0;
	_menuGamepadAxisX = 0;
	_menuGamepadAxisY = 0;
	_menuGamepadRawAxis = -1;
	_menuGamepadRawAxisX = 0;
	_menuGamepadRawAxisY = 0;
	_handler8HudGlyph = '#';
	_handler8HudMessageTimer = 0;
	_handler8HudMessageIndex = 0;

	_currentPhase = 1;
	_deathFrame = 0;
	_skipSectionRequested = false;

	_vm->_system->getEventManager()->getEventDispatcher()->registerObserver(this, 1, false);
}


InsaneRebel2::~InsaneRebel2() {
	restoreIOSGamepadController();
	setVirtualKeyboardVisible(false);

	_vm->_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	terminateAudio();
	freeSfx();
	delete _rebelMsgFont;
	delete _smush_talkfontNut;
	delete _smush_smalfontNut;
	delete _smush_titlefontNut;
	delete _smush_povfontNut;

	delete _shipSprite;
	delete _shipSprite2;
	delete _shipOverlay1;
	delete _shipOverlay2;
	free(_level2Background);
	_level2Background = nullptr;

	delete _flyShipSprite;
	delete _flyLaserSprite;
	delete _flyTargetSprite;
	delete _flyHiResSprite;

	delete _grd001Sprite;
	delete _grd002Sprite;
	delete _grd005Sprite;

	delete _hudOverlayNut;
	delete _hudOverlay2Nut;

	freeLaserTexture();

	for (int i = 0; i < 16; i++) {
		free(_rebelEmbeddedHud[i].pixels);
		_rebelEmbeddedHud[i].pixels = nullptr;
	}
}

bool InsaneRebel2::isHiRes() const {
	return _vm->_screenWidth >= 640 && _vm->_screenHeight >= 400;
}

void InsaneRebel2::openGameplayMainMenu(SmushPlayer *splayer) {
	if (!splayer)
		return;

	if (_pauseOverlayActive) {
		_vm->_system->getPaletteManager()->setPalette(_savedPausePalette, 0, 256);
		_pauseOverlayActive = false;
	}

	if (!splayer->_paused)
		splayer->pause();

	const bool restoreGamepad = _iosGamepadControllerState.isEnabled();
	restoreIOSGamepadController();
	_vm->openMainMenuDialog();
	if (restoreGamepad && _gameState == kStateGameplay && _rebelHandler != 0 && !_vm->shouldQuit())
		enableIOSGamepadController();
	splayer->unpause();
	_lastGameplayMenuCloseTime = _vm->_system->getMillis();
}

void InsaneRebel2::openMenuMainMenu(SmushPlayer *splayer) {
	if (splayer && !splayer->_paused) {
		splayer->pause();
		_vm->openMainMenuDialog();
		splayer->unpause();
		return;
	}

	_vm->openMainMenuDialog();
}

void InsaneRebel2::enableIOSGamepadController() {
	_iosGamepadControllerState.enable();
}

void InsaneRebel2::restoreIOSGamepadController() {
	_iosGamepadControllerState.restore();
}

void InsaneRebel2::resetMenuGamepadAxis() {
	_menuGamepadAxisX = 0;
	_menuGamepadAxisY = 0;
	_menuGamepadRawAxis = -1;
	_menuGamepadRawAxisX = 0;
	_menuGamepadRawAxisY = 0;
}

void InsaneRebel2::queueMenuGamepadAxisKey(Common::KeyCode keycode) {
	Common::Event syntheticEvent = Common::Event();
	syntheticEvent.type = Common::EVENT_KEYDOWN;
	syntheticEvent.kbd.keycode = keycode;
	_menuEventQueue.push(syntheticEvent);

	_lastMenuGamepadNavigationTime = _vm->_system->getMillis();
}

void InsaneRebel2::updateMenuGamepadAxisKey(int16 oldAxisX, int16 oldAxisY) {
	const int16 axisX = combineRebel2MenuAxis(_menuGamepadAxisX, _menuGamepadRawAxisX);
	const int16 axisY = combineRebel2MenuAxis(_menuGamepadAxisY, _menuGamepadRawAxisY);
	const int oldX = getRebel2MenuAxisDirection(oldAxisX);
	const int oldY = getRebel2MenuAxisDirection(oldAxisY);
	const int newX = getRebel2MenuAxisDirection(axisX);
	const int newY = getRebel2MenuAxisDirection(axisY);

	if (newY != oldY && newY != 0)
		queueMenuGamepadAxisKey(newY > 0 ? Common::KEYCODE_DOWN : Common::KEYCODE_UP);
	else if (newX != oldX && newX != 0)
		queueMenuGamepadAxisKey(newX > 0 ? Common::KEYCODE_RIGHT : Common::KEYCODE_LEFT);
}

bool InsaneRebel2::handleMenuGamepadAxisEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_AXIS)
		return false;

	const int16 oldAxisX = combineRebel2MenuAxis(_menuGamepadAxisX, _menuGamepadRawAxisX);
	const int16 oldAxisY = combineRebel2MenuAxis(_menuGamepadAxisY, _menuGamepadRawAxisY);
	const int16 axisPosition = normalizeRebel2AxisMagnitude(event.joystick.position);
	const bool pressed = axisPosition >= kRA2MenuAxisThreshold;

	switch (event.customType) {
	case kScummBackendActionRebel2AxisUp:
		if (pressed)
			_menuGamepadAxisY = -axisPosition;
		else
			_menuGamepadAxisY = 0;
		break;

	case kScummBackendActionRebel2AxisDown:
		if (pressed)
			_menuGamepadAxisY = axisPosition;
		else
			_menuGamepadAxisY = 0;
		break;

	case kScummBackendActionRebel2AxisLeft:
		if (pressed)
			_menuGamepadAxisX = -axisPosition;
		else
			_menuGamepadAxisX = 0;
		break;

	case kScummBackendActionRebel2AxisRight:
		if (pressed)
			_menuGamepadAxisX = axisPosition;
		else
			_menuGamepadAxisX = 0;
		break;

	default:
		return false;
	}

	updateMenuGamepadAxisKey(oldAxisX, oldAxisY);
	return true;
}

bool InsaneRebel2::handleMenuRawJoystickAxisEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_JOYAXIS_MOTION)
		return false;

	int16 axisX = 0;
	int16 axisY = 0;
	if (!decodeRebel2MenuKeyLikeAxis(event.joystick.axis, event.joystick.position, axisX, axisY))
		return false;

	const bool active = axisX != 0 || axisY != 0;
	if (!active && _menuGamepadRawAxis != event.joystick.axis && _menuGamepadRawAxis != -1)
		return false;

	const int16 oldAxisX = combineRebel2MenuAxis(_menuGamepadAxisX, _menuGamepadRawAxisX);
	const int16 oldAxisY = combineRebel2MenuAxis(_menuGamepadAxisY, _menuGamepadRawAxisY);
	_menuGamepadRawAxis = active ? event.joystick.axis : -1;
	_menuGamepadRawAxisX = axisX;
	_menuGamepadRawAxisY = axisY;
	updateMenuGamepadAxisKey(oldAxisX, oldAxisY);
	return true;
}

bool InsaneRebel2::notifyEvent(const Common::Event &event) {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Global dialogs pause the engine while their modal event loop runs.
	// Do not consume those events as RA2 input: a key seen here would otherwise
	// trip RA2's "any key unpauses gameplay" path while the Smush player must
	// remain paused for the dialog/focus interval.
	if (_vm->isPaused())
		return false;

	if (_rebelYodaMode && event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat && event.kbd.hasFlags(Common::KBD_ALT)) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_m:
			_rebelMovieMode = !_rebelMovieMode;
			debugC(DEBUG_INSANE, "Movie mode %s", _rebelMovieMode ? "enabled" : "disabled");
			if (_rebelMovieMode && splayer && _gameState == kStateGameplay && _rebelHandler != 0)
				_vm->_smushVideoShouldFinish = true;
			return true;

		case Common::KEYCODE_p:
			_rebelAutoPlay = !_rebelAutoPlay;
			debugC(DEBUG_INSANE, "Auto play %s", _rebelAutoPlay ? "enabled" : "disabled");
			return true;

		default:
			break;
		}
	}

	if (_gameState == kStateGameplay && _rebelHandler == 7 &&
			event.type == Common::EVENT_MOUSEMOVE) {
		if (_gameplayMouseSettleUntil != 0) {
			const uint32 now = _vm->_system->getMillis();
			if (now < _gameplayMouseSettleUntil) {
				const int mouseScale = isHiRes() ? 2 : 1;
				const int jumpX = event.mouse.x - _vm->_mouse.x;
				const int jumpY = event.mouse.y - _vm->_mouse.y;
				const bool largeAbsoluteJump =
					ABS(jumpX) >= kRA2Handler7MouseSettleJumpThreshold * mouseScale ||
					ABS(jumpY) >= kRA2Handler7MouseSettleJumpThreshold * mouseScale;
				const bool smallRelativeMove =
					ABS((int)event.relMouse.x) < kRA2Handler7MouseSettleRelativeThreshold * mouseScale &&
					ABS((int)event.relMouse.y) < kRA2Handler7MouseSettleRelativeThreshold * mouseScale;
				const bool nearWindowEdge =
					event.mouse.x <= kRA2Handler7MouseSettleEdgeMargin * mouseScale ||
					event.mouse.x >= kRA2GameplayMouseMaxX * mouseScale - kRA2Handler7MouseSettleEdgeMargin * mouseScale ||
					event.mouse.y <= kRA2Handler7MouseSettleEdgeMargin * mouseScale ||
					event.mouse.y >= kRA2GameplayMouseMaxY * mouseScale - kRA2Handler7MouseSettleEdgeMargin * mouseScale;

				if (largeAbsoluteJump && smallRelativeMove && nearWindowEdge) {
					const Common::Point recenter = getGameplayAimPoint();
					_gameplayMouseSettleUntil = now + kRA2Handler7MouseSettleExtendMs;
					warpGameplayMouseNow(recenter.x, recenter.y);

					debugC(DEBUG_INSANE, "H7 mouse settle: suppress pos=(%d,%d) rel=(%d,%d) current=(%d,%d) until=%u",
						event.mouse.x, event.mouse.y, event.relMouse.x, event.relMouse.y,
						_vm->_mouse.x, _vm->_mouse.y, _gameplayMouseSettleUntil);
					return true;
				}
			}

			_gameplayMouseSettleUntil = 0;
		}

		debugC(DEBUG_INSANE, "H7 mouse event: pos=(%d,%d) rel=(%d,%d) gamepadAim=%d menuInput=%d",
			event.mouse.x, event.mouse.y, event.relMouse.x, event.relMouse.y,
			_gamepadAimActive ? 1 : 0, _menuInputActive ? 1 : 0);
	}

	// Suppress locked-cursor artifacts while the gamepad owns the reticle.
	if (_gamepadAimActive && _gameState == kStateGameplay && !_menuInputActive) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			if (event.relMouse.x != 0 || event.relMouse.y != 0) {
				_gamepadAimActive = false;
				break;
			}
			return true;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			return true;
			// Let LBUTTONUP/RBUTTONUP fall through so the backend _buttonState latch
			// always clears and can never stick when a real mouse later takes over.
		default:
			break;
		}
	}

	if (_menuInputActive && event.type == Common::EVENT_JOYAXIS_MOTION) {
		if (handleMenuRawJoystickAxisEvent(event))
			return true;
	}

	if (event.type == Common::EVENT_CUSTOM_BACKEND_ACTION_AXIS) {
		if (_menuInputActive && handleMenuGamepadAxisEvent(event))
			return true;

		const int16 axisPosition = (event.joystick.position == Common::JOYAXIS_MIN)
			? Common::JOYAXIS_MAX : event.joystick.position;

		switch (event.customType) {
		case kScummBackendActionRebel2AxisUp:
			if (event.joystick.position == 0 && _joystickAxisY > 0)
				return true;
			_joystickAxisY = -axisPosition;
			return true;
		case kScummBackendActionRebel2AxisDown:
			if (event.joystick.position == 0 && _joystickAxisY < 0)
				return true;
			_joystickAxisY = axisPosition;
			return true;
		case kScummBackendActionRebel2AxisLeft:
			if (event.joystick.position == 0 && _joystickAxisX > 0)
				return true;
			_joystickAxisX = -axisPosition;
			return true;
		case kScummBackendActionRebel2AxisRight:
			if (event.joystick.position == 0 && _joystickAxisX < 0)
				return true;
			_joystickAxisX = axisPosition;
			return true;
		default:
			break;
		}
	}

	if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START ||
		event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
		const bool pressed = (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START);
		const bool menuState = isRebel2MenuState(_gameState);

		if (event.customType == kScummActionInsaneSkip) {
			if (!pressed)
				return true;

			if (_menuInputActive && menuState) {
				Common::Event syntheticEvent = Common::Event();
				syntheticEvent.type = Common::EVENT_KEYDOWN;
				syntheticEvent.kbd.keycode = Common::KEYCODE_ESCAPE;
				syntheticEvent.kbd.ascii = Common::ASCII_ESCAPE;
				_menuEventQueue.push(syntheticEvent);
				return true;
			}

			if (_gameState == kStateGameplay && _rebelHandler != 0) {
				debugC(DEBUG_INSANE, "Skip/back action ignored during gameplay");
				return true;
			}

			debugC(DEBUG_INSANE, "Skip/back action - skipping video");
			_vm->_smushVideoShouldFinish = true;
			return true;
		}

		if (event.customType == kScummActionInsaneBack) {
			if (!pressed)
				return true;

			if (_menuInputActive && menuState) {
				debugC(DEBUG_INSANE, "Back/menu action in menu - opening global menu");
				openMenuMainMenu(splayer);
				return true;
			}

			if (_gameState == kStateGameplay && _rebelHandler != 0) {
				if (_lastGameplayMenuCloseTime != 0) {
					const uint32 elapsedSinceMenuClose = _vm->_system->getMillis() - _lastGameplayMenuCloseTime;
					if (elapsedSinceMenuClose < 500) {
						debugC(DEBUG_INSANE, "Ignoring repeated gameplay menu action (%u ms)", elapsedSinceMenuClose);
						return true;
					}
				}

				debugC(DEBUG_INSANE, "Back/menu action during gameplay - opening global menu");
				openGameplayMainMenu(splayer);
				return true;
			}

			debugC(DEBUG_INSANE, "Back/menu action - skipping video");
			_vm->_smushVideoShouldFinish = true;
			return true;
		}

		if (pressed && splayer && splayer->_paused && _gameState == kStateGameplay) {
			debugC(DEBUG_INSANE, "Joystick action while paused - unpausing");
			if (_pauseOverlayActive) {
				_vm->_system->getPaletteManager()->setPalette(_savedPausePalette, 0, 256);
				_pauseOverlayActive = false;
			}
			splayer->unpause();
			return true;
		}

		if (_menuInputActive && pressed) {
			Common::KeyCode keycode = Common::KEYCODE_INVALID;

			switch (_gameState) {
			case kStateMainMenu:
			case kStatePilotSelect:
			case kStateDifficultySelect:
			case kStateChapterSelect:
			case kStateOptions:
				switch (event.customType) {
				case kScummActionInsaneUp:
					keycode = Common::KEYCODE_UP;
					break;
				case kScummActionInsaneDown:
					keycode = Common::KEYCODE_DOWN;
					break;
				case kScummActionInsaneLeft:
					keycode = Common::KEYCODE_LEFT;
					break;
				case kScummActionInsaneRight:
					keycode = Common::KEYCODE_RIGHT;
					break;
				case kScummActionInsaneAttack:
					keycode = Common::KEYCODE_RETURN;
					break;
				case kScummActionInsaneBack:
					keycode = Common::KEYCODE_ESCAPE;
					break;
				case kScummActionInsaneSkip:
					keycode = Common::KEYCODE_ESCAPE;
					break;
				default:
					break;
				}
				break;

			case kStateTopPilots:
				if (event.customType == kScummActionInsaneAttack)
					keycode = Common::KEYCODE_RETURN;
				else if (event.customType == kScummActionInsaneBack ||
				         event.customType == kScummActionInsaneSkip)
					keycode = Common::KEYCODE_ESCAPE;
				break;

			default:
				break;
			}

			if (keycode != Common::KEYCODE_INVALID) {
				if (isRebel2MenuDirectionKey(keycode)) {
					const uint32 now = _vm->_system->getMillis();
					if (_lastMenuGamepadNavigationTime != 0 &&
							now - _lastMenuGamepadNavigationTime < kRA2MenuGamepadNavigationDebounceMs)
						return true;
					_lastMenuGamepadNavigationTime = now;
				}

				Common::Event syntheticEvent = Common::Event();
				syntheticEvent.type = Common::EVENT_KEYDOWN;
				syntheticEvent.kbd.keycode = keycode;
				syntheticEvent.kbd.ascii = (keycode == Common::KEYCODE_RETURN) ? '\r' :
					(keycode == Common::KEYCODE_ESCAPE) ? Common::ASCII_ESCAPE : 0;
				_menuEventQueue.push(syntheticEvent);
				return true;
			}
		}

		// Gameplay Attack/Switch actions must still reach ScummEngine::parseEvent().
	}

	if (_menuInputActive && isRebel2MenuState(_gameState) &&
			(event.type == Common::EVENT_MAINMENU ||
			 (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE))) {
		if (event.type == Common::EVENT_KEYDOWN && event.kbdRepeat) {
			debugC(DEBUG_INSANE, "Ignoring repeated ESC keydown in menu");
			return true;
		}

		debugC(DEBUG_INSANE, "Opening global menu from menu state");
		openMenuMainMenu(splayer);
		return true;
	}

	if (_menuInputActive && isMenuTextInputActive() && event.type == Common::EVENT_KEYDOWN) {
		_menuEventQueue.push(event);
		return true;
	}

	const bool gameplayMenuTrigger = (event.type == Common::EVENT_MAINMENU) ||
		(event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE);
	if (gameplayMenuTrigger && _gameState == kStateGameplay && _lastGameplayMenuCloseTime != 0) {
		const uint32 elapsedSinceMenuClose = _vm->_system->getMillis() - _lastGameplayMenuCloseTime;
		if (elapsedSinceMenuClose < 500) {
			debugC(DEBUG_INSANE, "Ignoring repeated gameplay menu trigger (%u ms)", elapsedSinceMenuClose);
			return true;
		}
	}

	if (event.type == Common::EVENT_MAINMENU && splayer &&
			_gameState == kStateGameplay && _rebelHandler != 0) {
		debugC(DEBUG_INSANE, "Main menu action during gameplay - opening global menu");
		openGameplayMainMenu(splayer);
		return true;
	}

	if (event.type == Common::EVENT_KEYDOWN) {
		if (event.kbd.keycode == Common::KEYCODE_ESCAPE && event.kbdRepeat) {
			debugC(DEBUG_INSANE, "Ignoring repeated ESC keydown");
			return true;
		}
		if (_menuInputActive && event.kbdRepeat &&
				(event.kbd.keycode == Common::KEYCODE_UP ||
				 event.kbd.keycode == Common::KEYCODE_DOWN ||
				 event.kbd.keycode == Common::KEYCODE_LEFT ||
				 event.kbd.keycode == Common::KEYCODE_RIGHT)) {
			debugC(DEBUG_INSANE, "Ignoring repeated menu direction keydown");
			return true;
		}

		if (splayer && splayer->_paused && _gameState == kStateGameplay) {
			debugC(DEBUG_INSANE, "Key pressed while paused - unpausing");
			if (_pauseOverlayActive) {
				_vm->_system->getPaletteManager()->setPalette(_savedPausePalette, 0, 256);
				_pauseOverlayActive = false;
			}
			splayer->unpause();
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE && _rebelHandler != 0) {
				debugC(DEBUG_INSANE, "ESC during pause - opening global menu");
				openGameplayMainMenu(splayer);
			}
			return true;
		}

		switch (event.kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
			if (splayer) {
				if (_gameState == kStateGameplay && _rebelHandler != 0) {
					debugC(DEBUG_INSANE, "ESC pressed during gameplay - opening global menu");
					openGameplayMainMenu(splayer);
				} else {
					debugC(DEBUG_INSANE, "ESC pressed - skipping video");
					_vm->_smushVideoShouldFinish = true;
				}
				return true;
			}
			break;

		case Common::KEYCODE_SPACE:
			if (splayer && _gameState == kStateGameplay && !splayer->_paused) {
				debugC(DEBUG_INSANE, "SPACE pressed - pausing");
				splayer->pause();
				showPauseOverlay();
				return true;
			}
			break;

		case Common::KEYCODE_s:
			if (splayer &&
			    _gameState == kStateGameplay &&
			    _rebelHandler != 0 &&
			    event.kbd.hasFlags(Common::KBD_SHIFT)) {
				_skipSectionRequested = true;
				debugC(DEBUG_INSANE, "Shift+S pressed - requesting gameplay section skip");
				_vm->_smushVideoShouldFinish = true;
				return true;
			}
			break;

		case Common::KEYCODE_d:
			if (splayer &&
			    _gameState == kStateGameplay &&
			    _rebelHandler != 0 &&
			    event.kbd.hasFlags(Common::KBD_SHIFT)) {
				if (!_noDamage) {
					_playerDamage = 255;
					_playerShield = 0;
					debugC(DEBUG_INSANE, "Shift+D pressed - forcing player death");
					_vm->_smushVideoShouldFinish = true;
				} else {
					debugC(DEBUG_INSANE, "Shift+D pressed - no damage mode prevents forced death");
				}
				return true;
			}
			break;

		default:
			break;
		}
	}

	if (!_menuInputActive)
		return false;

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		_menuEventQueue.push(event);
		break;
	case Common::EVENT_MOUSEMOVE:
		if (_lastMenuGamepadNavigationTime == 0 ||
				_vm->_system->getMillis() - _lastMenuGamepadNavigationTime >= kRA2MenuGamepadMouseSuppressMs)
			_menuEventQueue.push(event);
		break;
	default:
		break;
	}

	return false;
}

// 17 fields per entry (34 bytes), 17 entries per difficulty, 6 difficulties
// Field names from official Difficulty Editor: {laserDelay, snapDistance, missDamage, dodgeDamage,
//   shotDamage, specialDamage, shotAccuracy, hitPoints, dodgePoints, timePoints,
//   levelPoints, specialPoints, flags, rollRate, liftRate, slideRate, driftRate}
// -1 = not applicable for this level type
const InsaneRebel2::LevelDifficultyParams InsaneRebel2::kDifficultyTable[6][17] = {
	// Difficulty 0 (Beginner) - 17 level types
	{
		{   5,    3,   15,   -1,    2,   -1,   75,   25,   -1,    2,  500,  250,    8,    5,    5,    6,   -1}, // Lv1
		{   4,    3,   -1,   -1,    2,   -1,   40,   25,   -1,    0,  500,  250,    8,   90,   90,  120,   25}, // Lv2
		{   6,    5,   15,   30,    3,   12,   75,   25,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    3,   18,   17,    2,   20,   75,   25,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv4
		{   5,    4,   18,   -1,    3,   -1,   75,   25,   -1,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv5
		{   5,    5,   -1,   19,    2,   15,   75,   25,   50,    2,  500,   -1,    8,   -1,   -1,   -1,   -1}, // Lv6A
		{   5,    5,  180,   27,    3,   -1,   75,   25,   50,    2,  500,  250,    8,  120,  120,  120,   75}, // Lv6B
		{  -1,   -1,   -1,   40,   -1,   -1,   -1,   -1,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    5,   21,   -1,    3,   -1,   50,   25,   -1,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    6,   15,   30,    4,   -1,   60,   25,   50,    2,  500,  250,    8,   90,   90,   90,  135}, // Lv9
		{   5,   15,   -1,   30,   70,   -1,   75,   25,   50,    2,  500,  250,    8,   10,    6,    7,   -1}, // Lv10
		{   4,    4,   10,   -1,    5,   -1,   65,   25,   -1,    0,  500,  250,    8,    5,    6,    7,    8}, // Lv11
		{   4,    2,   -1,   -1,    6,   -1,   65,   25,   -1,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    6,   20,   30,    1,   20,   85,   25,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    3,   24,   -1,    2,   -1,   75,   25,   -1,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    8,   -1,   -1,    3,   -1,   75,   25,   -1,    2,  500,   -1,    8,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    6,  255,   30,    4,   10,   75,   25,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv15B
	},
	// Difficulty 1 (Novice) - 17 level types
	{
		{   6,    1,   25,   -1,    3,   -1,   75,   50,   -1,    4, 1000,  500,   16,    6,    6,    7,   -1}, // Lv1
		{   4,    2,   -1,   -1,    4,   -1,   40,   50,   -1,    0, 1000,  500,   16,  100,  100,  135,   30}, // Lv2
		{   6,    4,   17,   35,    5,   12,   75,   50,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    2,   60,   30,    4,   20,   75,   50,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv4
		{   5,    1,   28,   -1,    3,   -1,   75,   50,   -1,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv5
		{   5,    2,   -1,   25,    4,   15,   75,   50,  100,    4, 1000,   -1,   16,   -1,   -1,   -1,   -1}, // Lv6A
		{   5,    2,  190,   35,    4,   -1,   75,   50,  100,    4, 1000,  500,   16,  140,  140,  140,   90}, // Lv6B
		{  -1,   -1,   -1,   65,   -1,   -1,   -1,   -1,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    3,   24,   -1,    3,   -1,   50,   50,   -1,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    4,   17,   45,    5,   -1,   75,   50,  100,    4, 1000,  500,   16,  100,  100,  100,  140}, // Lv9
		{   5,   12,   -1,   35,   75,   -1,   75,   50,  100,    4, 1000,  500,   16,   10,    6,    7,   -1}, // Lv10
		{   4,    2,   15,   -1,    5,   -1,   75,   50,   -1,    0, 1000,  500,   16,    5,    6,    7,    8}, // Lv11
		{   4,    1,   -1,   -1,    8,   -1,   75,   50,   -1,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    5,   30,   35,    1,   20,   85,   50,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    2,   28,   -1,    2,   -1,   75,   50,   -1,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    7,   -1,   -1,    4,   -1,   75,   50,   -1,    4, 1000,   -1,   16,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    6,  255,   35,    4,   10,   75,   50,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv15B
	},
	// Difficulty 2 (Standard) - 17 level types
	{
		{   7,    0,   35,   -1,    5,   -1,   75,   75,   -1,    6, 1500,  750,    0,    7,    7,    8,   -1}, // Lv1
		{   4,    1,   -1,   -1,    6,   -1,   40,   75,   -1,    0, 1500,  750,    0,  110,  110,  150,   35}, // Lv2
		{   6,    1,   20,   38,    7,   12,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    1,  100,   35,    6,   20,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv4
		{   6,    1,   30,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv5
		{   6,    1,   -1,   30,    6,   15,   75,   75,  150,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv6A
		{   6,    1,  200,   50,   12,   -1,   75,   75,  150,    6, 1500,  750,    0,  160,  160,  160,  105}, // Lv6B
		{  -1,   -1,   -1,   80,   -1,   -1,   -1,   -1,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    1,   27,   -1,    3,   -1,   60,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    3,   19,   60,    7,   -1,   75,   75,  150,    6, 1500,  750,    0,  110,  110,  110,  150}, // Lv9
		{   5,    9,   -1,   40,  100,   -1,   85,   75,  150,    6, 1500,  750,    0,   11,    7,    8,   -1}, // Lv10
		{   4,    1,   20,   -1,    6,   -1,   75,   75,   -1,    0, 1500,  750,    0,    6,    7,    8,    9}, // Lv11
		{   4,    0,   -1,   -1,   11,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    3,   40,   40,    3,   15,   76,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    0,   38,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    4,   -1,   -1,    7,   -1,   75,   75,   -1,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    5,  255,   40,    7,   10,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv15B
	},
	// Difficulty 3 (Expert) - 17 level types
	{
		{   8,    0,   77,   -1,    7,   -1,   80,  100,   -1,    8, 2000, 1000,    4,    8,    8,    9,   -1}, // Lv1
		{   4,    0,   -1,   -1,    7,   -1,   40,  100,   -1,    0, 2000, 1000,    4,  120,  120,  165,   40}, // Lv2
		{   6,    0,   23,   42,   12,   10,   75,  100,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    0,  120,   50,   16,   10,   75,  100,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv4
		{   5,    0,   55,   -1,    4,   -1,   75,  100,   -1,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv5
		{   6,    0,    0,   50,   15,   15,   79,  100,  200,    8, 2000,   -1,    4,   -1,   -1,   -1,   -1}, // Lv6A
		{   6,    0,  220,   90,   15,   -1,   90,  100,  200,    8, 2000, 1000,    4,  180,  180,  180,  140}, // Lv6B
		{  -1,   -1,   -1,   90,   -1,   -1,   -1,   -1,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    0,   35,   -1,    3,   -1,   68,  100,   -1,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    2,   30,   75,   20,   -1,   80,  100,  200,    8, 2000, 1000,    4,  120,  120,  120,  200}, // Lv9
		{   5,    8,   -1,   50,  110,   -1,   90,  100,  200,    8, 2000, 1000,    4,   12,    8,    9,   -1}, // Lv10
		{   4,    0,   30,   -1,    7,   -1,   75,  100,   -1,    0, 2000, 1000,    4,    7,    8,    9,   10}, // Lv11
		{   4,    0,   -1,   -1,   13,   -1,   75,  100,   -1,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    3,   55,   55,    5,   12,   77,  100,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    0,   49,   -1,    4,   -1,   75,  100,   -1,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    4,   -1,   -1,   10,   -1,   79,  100,   -1,    8, 2000,   -1,    4,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    4,  255,   45,    8,    5,   80,  100,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv15B
	},
	{
		{   7,    0,   35,   -1,    5,   -1,   75,   75,   -1,    6, 1500,  750,    0,    7,    7,    8,   -1}, // Lv1
		{   4,    1,   -1,   -1,    6,   -1,   40,   75,   -1,    0, 1500,  750,    0,  110,  110,  150,   35}, // Lv2
		{   6,    1,   20,   38,    7,   12,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    1,  100,   35,    6,   20,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv4
		{   6,    1,   30,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv5
		{   6,    1,   -1,   30,    6,   15,   75,   75,  150,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv6A
		{   6,    1,  200,   50,   12,   -1,   75,   75,  150,    6, 1500,  750,    0,  160,  160,  160,  105}, // Lv6B
		{  -1,   -1,   -1,   80,   -1,   -1,   -1,   -1,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    1,   27,   -1,    3,   -1,   60,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    3,   19,   60,    7,   -1,   75,   75,  150,    6, 1500,  750,    0,  110,  110,  110,  150}, // Lv9
		{   5,    9,   -1,   40,  100,   -1,   85,   75,  150,    6, 1500,  750,    0,   11,    7,    8,   -1}, // Lv10
		{   4,    1,   20,   -1,    6,   -1,   75,   75,   -1,    0, 1500,  750,    0,    6,    7,    8,    9}, // Lv11
		{   4,    0,   -1,   -1,   11,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    3,   40,   40,    3,   15,   76,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    0,   38,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    4,   -1,   -1,    7,   -1,   75,   75,   -1,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    5,  255,   40,    7,   10,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv15B
	},
	// Difficulty 5 (Custom2) matches Custom1 except Lv15B movement rates are 0.
	{
		{   7,    0,   35,   -1,    5,   -1,   75,   75,   -1,    6, 1500,  750,    0,    7,    7,    8,   -1}, // Lv1
		{   4,    1,   -1,   -1,    6,   -1,   40,   75,   -1,    0, 1500,  750,    0,  110,  110,  150,   35}, // Lv2
		{   6,    1,   20,   38,    7,   12,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    1,  100,   35,    6,   20,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv4
		{   6,    1,   30,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv5
		{   6,    1,   -1,   30,    6,   15,   75,   75,  150,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv6A
		{   6,    1,  200,   50,   12,   -1,   75,   75,  150,    6, 1500,  750,    0,  160,  160,  160,  105}, // Lv6B
		{  -1,   -1,   -1,   80,   -1,   -1,   -1,   -1,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    1,   27,   -1,    3,   -1,   60,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    3,   19,   60,    7,   -1,   75,   75,  150,    6, 1500,  750,    0,  110,  110,  110,  150}, // Lv9
		{   5,    9,   -1,   40,  100,   -1,   85,   75,  150,    6, 1500,  750,    0,   11,    7,    8,   -1}, // Lv10
		{   4,    1,   20,   -1,    6,   -1,   75,   75,   -1,    0, 1500,  750,    0,    6,    7,    8,    9}, // Lv11
		{   4,    0,   -1,   -1,   11,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    3,   40,   40,    3,   15,   76,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    0,   38,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    4,   -1,   -1,    7,   -1,   75,   75,   -1,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    5,  255,   40,    7,   10,   75,   75,  150,    6, 1500,  750,    0,    0,    0,    0,    0}, // Lv15B
	},
};

InsaneRebel2::LevelDifficultyParams InsaneRebel2::getDifficultyParams() const {
	int diff = CLIP(_difficulty, 0, 5);
	int lvIdx = 0;

	// Index mapping reconstructed from level handlers:
	//   Lv1->0, Lv2->1, Lv3->2, Lv4->3, Lv5->4,
	//   Lv6A->5, Lv6B->6, Lv7->7 ... Lv14->14, Lv15A->15, Lv15B->16.
	// Our Level 6 phase flow sets _currentPhase to 1/2 accordingly.
	// Level 15 switches to type 0x10 at frame 0x21e (updateLevel15TypeSwitch).
	if (_selectedLevel <= 0) {
		lvIdx = CLIP((int)_rebelLevelType, 0, 16);
	} else if (_selectedLevel <= 5) {
		lvIdx = _selectedLevel - 1;
	} else if (_selectedLevel == 6) {
		lvIdx = (_currentPhase >= 2) ? 6 : 5;
	} else if (_selectedLevel <= 14) {
		lvIdx = _selectedLevel;
	} else { // _selectedLevel == 15
		lvIdx = (_rebelLevelType >= 0x10) ? 16 : 15;
	}

	lvIdx = CLIP(lvIdx, 0, 16);
	return kDifficultyTable[diff][lvIdx];
}

bool InsaneRebel2::applyPlayerDamage(int damage) {
	if (_noDamage || _rebelAutoPlay || damage <= 0)
		return false;

	_playerDamage += damage;
	if (_playerDamage > 255)
		_playerDamage = 255;

	return true;
}

void InsaneRebel2::addScore(int points) {
	int threshold;
	if (_difficulty < 4) {
		threshold = (_difficulty * 5 + 5) * 1000;
	} else {
		threshold = 15000;
	}

	if (threshold > 0) {
		int oldMilestone = _playerScore / threshold;
		int newMilestone = (_playerScore + points) / threshold;
		if (oldMilestone < newMilestone) {
			_playerLives++;
			playSfx(5, 127, 0);
			debugC(DEBUG_INSANE, "BONUS LIFE! Score crossed %d threshold. Lives=%d", threshold, _playerLives);
		}
	}

	_playerScore += points;
	debugC(DEBUG_INSANE, "Score +%d = %d", points, _playerScore);
}

void InsaneRebel2::renderScoreHUD(byte *renderBitmap, int pitch, int width, int height, int statusBarY) {
	NutRenderer *statusFont = _smush_cockpitNut;
	if (!statusFont)
		return;

	char scoreStr[16];
	Common::sprintf_s(scoreStr, "%07d", _playerScore);

	const int statusScale = isHiRes() ? 2 : 1;
	int scoreX = 257 * statusScale + _viewX;
	int scoreY = statusBarY + 4 * statusScale + _viewY;

	int x = scoreX;
	for (int i = 0; scoreStr[i] != '\0'; i++) {
		byte ch = (byte)scoreStr[i];
		if (ch < statusFont->getNumChars()) {
			int charX = x + statusFont->getCharXOffset(ch);
			int charY = scoreY + statusFont->getCharYOffset(ch);
			renderNutSprite(renderBitmap, pitch, width, height, charX, charY, statusFont, ch);
			x += statusFont->getCharWidth(ch);
		}
	}
}

const uint32 kPilotSaveMagic = MKTAG('R', 'A', '2', 'P');
const uint16 kPilotSaveVersion = 2;

bool InsaneRebel2::loadPilots() {
	_numPilots = 0;

	for (int i = 0; i < kMaxPilots; i++) {
		Common::String filename = _vm->makeSavegameName(i, false);
		Common::InSaveFile *sf = _vm->_saveFileMan->openForLoading(filename);
		if (!sf)
			break; // Slots are contiguous

		uint32 magic = sf->readUint32BE();
		if (magic != kPilotSaveMagic) {
			delete sf;
			break;
		}

		uint16 version = sf->readUint16LE();

		PilotData &p = _pilots[i];
		sf->read(p.name, kMaxPilotNameLen + 1);
		p.name[kMaxPilotNameLen] = '\0';
		for (int j = 0; j < kNumLevels; j++)
			p.score[j] = sf->readSint32LE();
		for (int j = 0; j < kNumLevels; j++)
			p.lives[j] = sf->readSint32LE();
		for (int j = 0; j < kNumLevels; j++)
			p.damage[j] = sf->readSint32LE();
		p.difficulty = sf->readSint16LE();
		if (version >= 2) {
			for (int j = 0; j < kNumLevels; j++)
				p.rating[j] = sf->readSint16LE();
		}
		delete sf;

		_numPilots = i + 1;
	}

	debugC(DEBUG_INSANE, "Loaded %d pilot(s)", _numPilots);
	return _numPilots > 0;
}

bool InsaneRebel2::savePilots() {
	bool ok = true;

	for (int i = 0; i < _numPilots; i++) {
		Common::String filename = _vm->makeSavegameName(i, false);
		Common::OutSaveFile *sf = _vm->_saveFileMan->openForSaving(filename, false);
		if (!sf) {
			warning("Rebel2: Failed to save pilot %d", i);
			ok = false;
			continue;
		}

		sf->writeUint32BE(kPilotSaveMagic);
		sf->writeUint16LE(kPilotSaveVersion);

		const PilotData &p = _pilots[i];
		sf->write(p.name, kMaxPilotNameLen + 1);
		for (int j = 0; j < kNumLevels; j++)
			sf->writeSint32LE(p.score[j]);
		for (int j = 0; j < kNumLevels; j++)
			sf->writeSint32LE(p.lives[j]);
		for (int j = 0; j < kNumLevels; j++)
			sf->writeSint32LE(p.damage[j]);
		sf->writeSint16LE(p.difficulty);
		for (int j = 0; j < kNumLevels; j++)
			sf->writeSint16LE(p.rating[j]);

		sf->finalize();
		delete sf;
	}

	for (int i = _numPilots; i < kMaxPilots; i++) {
		Common::String filename = _vm->makeSavegameName(i, false);
		_vm->_saveFileMan->removeSavefile(filename);
	}

	debugC(DEBUG_INSANE, "Saved %d pilot(s)", _numPilots);
	return ok;
}

int InsaneRebel2::createNewPilot() {
	if (_numPilots >= kMaxPilots)
		return -1;

	int idx = _numPilots;
	_pilots[idx].init();
	_numPilots++;
	return idx;
}

void InsaneRebel2::deletePilot(int index) {
	if (index < 0 || index >= _numPilots)
		return;

	for (int i = index; i < _numPilots - 1; i++) {
		_pilots[i] = _pilots[i + 1];
	}
	_numPilots--;

	_pilots[_numPilots].init();
}

void InsaneRebel2::copyPilot(int srcIndex) {
	if (srcIndex < 0 || srcIndex >= _numPilots || _numPilots >= kMaxPilots)
		return;

	int newIdx = _numPilots;
	_pilots[newIdx] = _pilots[srcIndex];

	Common::String name(_pilots[newIdx].name);
	if (name.size() + 5 <= kMaxPilotNameLen) {
		name += " COPY";
	} else if (name.size() > 0) {
		name = name.substr(0, kMaxPilotNameLen - 2) + " C";
	}
	Common::strlcpy(_pilots[newIdx].name, name.c_str(), sizeof(_pilots[newIdx].name));

	_numPilots++;
}

void InsaneRebel2::updatePilotProgress(int levelIndex, int32 score, int32 lives, int32 damage, int32 rating) {
	if (_activePilot < 0 || _activePilot >= _numPilots)
		return;
	if (levelIndex < 0 || levelIndex >= kNumLevels)
		return;

	PilotData &pilot = _pilots[_activePilot];
	pilot.score[levelIndex] = score;
	pilot.lives[levelIndex] = lives;
	pilot.damage[levelIndex] = damage;
	pilot.rating[levelIndex] = rating;

	savePilots();
}

int32 InsaneRebel2::processMouse() {
	int32 buttons = 0;

	uint32 currentButtons = _vm->_system->getEventManager()->getButtonState();
	if (_vm->getActionState(kScummActionInsaneAttack))
		currentButtons |= 1;
	if (_vm->getActionState(kScummActionInsaneSwitch))
		currentButtons |= 2;

	bool leftPressed = (currentButtons & 1) != 0;
	bool leftWasPressed = (_prevMouseButtons & 1) != 0;
	bool rightPressed = (currentButtons & 2) != 0;
	bool rightWasPressed = (_prevMouseButtons & 2) != 0;

	_prevMouseButtons = currentButtons;

	if (_rebelHandler == 25) {
		if (rightPressed && !rightWasPressed) {
			_rebelControlMode |= 2;
		}
		if (leftPressed && !leftWasPressed) {
			_rebelControlMode |= 1;
		}
	} else {
		_rebelControlMode = 0;
		if (rightPressed) {
			_rebelControlMode |= 2;
		}
		if (leftPressed) {
			_rebelControlMode |= 1;
		}
	}

	bool canShoot = isShootingAllowed();
	bool autoFire = _rebelAutoPlay && canShoot && _gameState == kStateGameplay && _rebelHandler != 0;
	if (autoFire && _player) {
		const int autoFirePeriod = (_rebelHandler == 8) ? 6 : 7;
		autoFire = (_player->_frame % autoFirePeriod) == 0;
	}
	if (autoFire)
		_rebelControlMode |= 1;

	bool triggerShot = ((_rebelHandler == 25) ? (leftPressed && !leftWasPressed) : leftPressed) || autoFire;
	if (_rebelHandler == 8) {
		_shipFiring = triggerShot && canShoot;
	}
	if (triggerShot && canShoot) {
		Common::Point mousePos;
		if (autoFire) {
			mousePos = getRebelAutoPlayAimPoint();
		} else if (_rebelHandler == 7) {
			mousePos = getHandler7ShotTargetPoint();
		} else if (_rebelHandler == 8) {
			mousePos = getHandler8ShotTargetPoint();
		} else {
			mousePos = getGameplayAimPoint();
		}
		Common::Point gameplayAim = getGameplayAimPoint();
		debugC(DEBUG_INSANE, "Click: Mouse=(%d,%d) Target=(%d,%d) Enemies=%d",
			gameplayAim.x, gameplayAim.y, mousePos.x, mousePos.y, _enemies.size());

		spawnShot(mousePos.x, mousePos.y);

		Common::Point worldMousePos = mousePos;
		if (_rebelHandler == 8) {
			worldMousePos.x += _shipPosX;
			worldMousePos.y += _shipPosY;
		} else if (_rebelHandler != 7) {
			worldMousePos.x += _viewX;
			worldMousePos.y += _viewY;
		}

		Common::List<enemy>::iterator it;
		for (it = _enemies.begin(); it != _enemies.end(); ++it) {
			debugC(DEBUG_INSANE, "  Enemy ID=%d active=%d destroyed=%d rect=(%d,%d)-(%d,%d) contains=%d",
				it->id, it->active, it->destroyed,
				it->rect.left, it->rect.top, it->rect.right, it->rect.bottom,
				it->rect.contains(worldMousePos));

			if (it->active && it->rect.contains(worldMousePos)) {
				it->active = false;
				it->destroyed = true;
				debugC(DEBUG_INSANE, "HIT enemy ID=%d type=%d at (%d,%d) - Rect: (%d,%d)-(%d,%d)",
					it->id, it->type, mousePos.x, mousePos.y,
					it->rect.left, it->rect.top, it->rect.right, it->rect.bottom);

				int explosionHalfWidth = it->rect.width() / 2;
				if (_rebelHandler == 25) {
					LevelDifficultyParams dparams = getDifficultyParams();
					explosionHalfWidth += dparams.snapDistance;
					if (it->type == 100)
						explosionHalfWidth *= 2;
				}

				if (_rebelHandler != 8 && _rebelHandler != 25) {
					spawnExplosion((it->rect.left + it->rect.right) / 2,
								   (it->rect.top + it->rect.bottom) / 2,
								   explosionHalfWidth);
				} else if (_rebelHandler == 8 && it->type == 0) {
					spawnExplosion((it->rect.left + it->rect.right) / 2,
								   (it->rect.top + it->rect.bottom) / 2,
								   explosionHalfWidth);
				} else if (_rebelHandler == 25 && it->type > 3) {
					spawnExplosion((it->rect.left + it->rect.right) / 2,
								   (it->rect.top + it->rect.bottom) / 2,
								   explosionHalfWidth);
				}

				setBit(it->id);

				// Shield hit-point gauge (handler 0x26): the target's type field addresses
				// the gauge group(s) it feeds — 100-109 directly, > 0x3ff as a bitmask —
				// and destroying it decrements each of them.
				if (_rebelHandler == 0x26) {
					if (it->type >= 100 && it->type < 110) {
						decrementGaugeGroup(it->type - 100, it->id);
					} else if (it->type > 0x3ff) {
						for (int slot = 1; slot <= 9; ++slot) {
							if ((it->type & (1 << (slot - 1))) != 0)
								decrementGaugeGroup(slot, it->id);
						}
					}
				}

				if (it->type > 0 && it->type < 32) {
					_rebelWaveState |= (1 << it->type);
					debugC(DEBUG_INSANE, "Wave state updated: 0x%x (set bit %d)", _rebelWaveState, it->type);
				}

				_rebelKillCounter++;

				bool handleDependencies = !(_rebelHandler == 25 && it->type > 3);
				int id = it->id;
				if (handleDependencies && id >= 0 && id < 512) {
					if (_rebelLinks[id][2] != 0) {
						clearBit(_rebelLinks[id][2]);
						debugC(DEBUG_INSANE, "Enabled dependency Slot 2 (ID=%d) for Parent %d", _rebelLinks[id][2], id);
					}
					if (_rebelLinks[id][1] != 0) {
						clearBit(_rebelLinks[id][1]);
						debugC(DEBUG_INSANE, "Enabled dependency Slot 1 (ID=%d) for Parent %d", _rebelLinks[id][1], id);
					}
					if (_rebelLinks[id][0] != 0) {
						setBit(_rebelLinks[id][0]);
						debugC(DEBUG_INSANE, "Disabled dependency Slot 0 (ID=%d) for Parent %d", _rebelLinks[id][0], id);
					}
				}

				{
					int cameraX = (_rebelHandler == 8) ? _shipPosX : _viewX;
					int enemyCenterX = (it->rect.left + it->rect.right) / 2 - cameraX;
					int sfxPan = CLIP((enemyCenterX - 160) * 127 / 160, -127, 127);
					if (_rebelHandler == 8 && it->type >= 1 && it->type <= 4) {
						playAuxSfx(0, 127, sfxPan);
					} else {
						playSfx(2, 127, sfxPan);
					}
				}

				{
					LevelDifficultyParams dparams = getDifficultyParams();
					if (dparams.hitPoints > 0) {
						addScore(dparams.hitPoints);
					}
				}

				break;
			}
		}
	}
	return buttons;
}

Common::Point InsaneRebel2::getRebelAutoPlayAimPoint() {
	Common::Point target(160, 100);
	int bestDistance = 0x7fffffff;

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (!it->active || it->destroyed)
			continue;

		int x = (it->rect.left + it->rect.right) / 2;
		int y = (it->rect.top + it->rect.bottom) / 2;
		if (_rebelHandler == 8) {
			x -= _shipPosX;
			y -= _shipPosY;
		} else if (_rebelHandler != 7) {
			x -= _viewX;
			y -= _viewY;
		}

		if (x < -32 || x > 351 || y < -32 || y > 231)
			continue;

		const int dx = x - 160;
		const int dy = y - 100;
		const int distance = dx * dx + dy * dy;
		if (distance < bestDistance) {
			bestDistance = distance;
			target.x = CLIP<int>(x, 0, 319);
			target.y = CLIP<int>(y, 0, 199);
		}
	}

	return target;
}

Common::Point InsaneRebel2::getGameplayAimPoint() {
	if (_rebelAutoPlay && _gameState == kStateGameplay && !_menuInputActive)
		return getRebelAutoPlayAimPoint();

	int x = _vm->_mouse.x;
	int y = _vm->_mouse.y;
	if (isHiRes()) {
		x /= 2;
		y /= 2;
	}
	x = CLIP<int>(x, 0, 319);
	y = CLIP<int>(y, 0, 199);
	if (_optControlsFlipped) {
		y = CLIP<int>(200 - y, 0, 199);
	}
	return Common::Point(x, y);
}

int16 applyRebel2AnalogDeadzone(int16 axisValue) {
	const int deadZone = MAX(0, ConfMan.getInt("joystick_deadzone")) * 1000;
	return (ABS((int)axisValue) <= deadZone) ? 0 : axisValue;
}

void InsaneRebel2::updateGameplayAimFromGamepad() {
	if (_menuInputActive || _gameState != kStateGameplay || _rebelAutoPlay)
		return;

	const int dpadX = (_vm->getActionState(kScummActionInsaneRight) ? 1 : 0) -
	                  (_vm->getActionState(kScummActionInsaneLeft) ? 1 : 0);
	const int dpadY = (_vm->getActionState(kScummActionInsaneDown) ? 1 : 0) -
	                  (_vm->getActionState(kScummActionInsaneUp) ? 1 : 0);

	const int16 ax = applyRebel2AnalogDeadzone(_joystickAxisX);
	const int16 ay = applyRebel2AnalogDeadzone(_joystickAxisY);
	const int velX = CLIP<int>((int)ax * 127 / Common::JOYAXIS_MAX, -127, 127);
	const int velY = CLIP<int>((int)ay * 127 / Common::JOYAXIS_MAX, -127, 127);

	int deltaX = 0;
	int deltaY = 0;
	bool activeGamepadAim = false;

	if (_rebelHandler == 7) {
		int axisX = 0;
		int axisY = 0;
		if (dpadX || dpadY) {
			axisX = dpadX * 127;
			axisY = dpadY * 127;
		} else {
			axisX = velX;
			axisY = velY;
		}

		const bool useDirectHandler7Gamepad = _selectedLevel == 10;
		const bool useCenteredHandler7Gamepad = (_selectedLevel == 3 && _flyControlMode == 1);
		if (!useDirectHandler7Gamepad && !useCenteredHandler7Gamepad && !axisX && !axisY)
			return;

		if (axisX || axisY || _gamepadAimActive) {
			const Common::Point aimPos = getGameplayAimPoint();
			const int centerX = 160;
			const int centerY = 100;
			int targetX;
			int targetY;
			if (useDirectHandler7Gamepad) {
				targetX = (axisX < 0) ?
					centerX + axisX * centerX / 127 :
					centerX + axisX * (319 - centerX) / 127;
				targetY = (axisY < 0) ?
					centerY + axisY * centerY / 127 :
					centerY + axisY * (199 - centerY) / 127;
			} else {
				const int kHandler7HorizontalRange = 120;
				const int kHandler7VerticalRange = 80;
				targetX = axisX ?
					CLIP<int>(centerX + axisX * kHandler7HorizontalRange / 127, 0, 319) :
					(useCenteredHandler7Gamepad ? centerX : aimPos.x);
				targetY = axisY ?
					CLIP<int>(centerY + axisY * kHandler7VerticalRange / 127, 0, 199) :
					(useCenteredHandler7Gamepad ? centerY : aimPos.y);
			}
			const int distX = targetX - aimPos.x;
			const int distY = targetY - aimPos.y;

			if (distX || distY) {
				if (useDirectHandler7Gamepad || useCenteredHandler7Gamepad) {
					deltaX = distX;
					deltaY = distY;
				} else {
					const int kHandler7HorizontalMaxStep = 24;
					const int kHandler7VerticalMaxStep = 32;
					deltaX = CLIP<int>(distX, -kHandler7HorizontalMaxStep, kHandler7HorizontalMaxStep);
					deltaY = CLIP<int>(distY, -kHandler7VerticalMaxStep, kHandler7VerticalMaxStep);
				}
				activeGamepadAim = true;
			} else {
				_gamepadAimActive = true;
				return;
			}
		}
	} else {
		if (dpadX || dpadY) {
			const int kOriginalDigitalStep = 3;
			deltaX = dpadX * kOriginalDigitalStep;
			deltaY = dpadY * kOriginalDigitalStep;
			activeGamepadAim = true;
		} else if (velX || velY) {
			const int kAnalogMaxStep = 8;
			deltaX = velX * kAnalogMaxStep / 127;
			deltaY = velY * kAnalogMaxStep / 127;
			activeGamepadAim = true;
		}
	}

	if (!activeGamepadAim)
		return;

	_gamepadAimActive = true;

	Common::Point aimPos = getGameplayAimPoint();
	const int scale = isHiRes() ? 2 : 1;
	_vm->_mouse.x = (int16)(CLIP<int>(aimPos.x + deltaX, 0, 319) * scale);
	_vm->_mouse.y = (int16)(CLIP<int>(aimPos.y + deltaY, 0, 199) * scale);
}

bool InsaneRebel2::isBitSet(int n) {
	if (n < 1) {
		return false;
	}
	assert (n < 0x401);

	return (_iactBits[n] != 0);
}

void InsaneRebel2::setBit(int n) {
	if (n < 1) {
		for (int i = 0; i < 0x401; i++)
			_iactBits[i] = 1;
		return;
	}
	assert (n < 0x401);
	_iactBits[n] = 1;
}

void InsaneRebel2::clearBit(int n) {
	if (n < 1) {
		for (int i = 0; i < 0x401; i++)
			_iactBits[i] = 0;
		return;
	}
	assert (n < 0x401);
	_iactBits[n] = 0;
}

bool InsaneRebel2::isShootingAllowed() {
	if (_rebelHandler == 7) {
		return (_flyControlMode == 2);
	}

	if (_rebelHandler == 8) {
		return (_shipLevelMode != 4 && _shipLevelMode != 5);
	}

	if (_rebelHandler == 25) {
		return (_rebelDamageLevel == 0);
	}

	return (_rebelHandler != 0);
}

void InsaneRebel2::procSKIP(int32 subSize, Common::SeekableReadStream &b) {

	int16 par1, par2;
	_player->_skipNext = false;

	assert(subSize >= 4);
	par1 = b.readUint16LE();
	par2 = b.readUint16LE();

	if (!par2) {
		bool bit1 = isBitSet(par1);
		if (bit1) {
			_player->_skipNext = true;
		}
		debugC(DEBUG_INSANE, "SKIP: single ID=%d bit=%d skip=%d frame=%d", par1, bit1 ? 1 : 0, _player->_skipNext ? 1 : 0, _player->_frame);
	} else {
		bool bit1 = isBitSet(par1);
		bool bit2 = isBitSet(par2);
		if (bit1 != bit2) {
			_player->_skipNext = true;
		}
		debugC(DEBUG_INSANE, "SKIP: dual ID1=%d(bit=%d) ID2=%d(bit=%d) skip=%d frame=%d", par1, bit1 ? 1 : 0, par2, bit2 ? 1 : 0, _player->_skipNext ? 1 : 0, _player->_frame);
	}
}


} // End of namespace Scumm
