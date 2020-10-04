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

#include "ngi/ngi.h"

#include "ngi/utils.h"
#include "ngi/objects.h"
#include "ngi/statics.h"
#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/input.h"
#include "ngi/behavior.h"

#include "ngi/constants.h"
#include "ngi/objectnames.h"
#include "ngi/scenes.h"
#include "ngi/interaction.h"

namespace NGI {

Vars::Vars() {
	sceneIntro_aniin1man = 0;
	sceneIntro_needSleep = true;
	sceneIntro_needGetup = false;
	sceneIntro_skipIntro = true;
	sceneIntro_playing = false;
	sceneIntro_needBlackout = false;

	swallowedEgg1 = 0;
	swallowedEgg2 = 0;
	swallowedEgg3 = 0;

	scene01_picSc01Osk = 0;
	scene01_picSc01Osk2 = 0;

	scene02_guvTheDrawer = 0;
	scene02_boxDelay = 0;
	scene02_boxOpen = false;

	scene03_eggeater = 0;
	scene03_domino = 0;

	scene04_bottle = 0;
	scene04_hand = 0;
	scene04_plank = 0;
	scene04_clock = 0;
	scene04_hand = 0;
	scene04_spring = 0;
	scene04_mamasha = 0;
	scene04_boot = 0;
	scene04_speaker = 0;
	scene04_musicStage = 0;

	scene04_ladder = 0;
	scene04_coinPut = false;
	scene04_soundPlaying = false;
	scene04_dynamicPhaseIndex = 0;
	scene04_dudeOnLadder = false;

	scene04_sceneClickX = 0;
	scene04_sceneClickY = 0;

	scene04_dudePosX = 0;
	scene04_dudePosY = 0;

	scene04_bottleIsTaken = false;
	scene04_kozyawkaOnLadder = false;
	scene04_walkingKozyawka = 0;
	scene04_bottleWeight = 0;
	scene04_var07 = false;
	scene04_ladderClickable = false;
	scene04_handIsDown = false;
	scene04_dudeInBottle = false;
	scene04_kozHeadRaised = false;
	scene04_bottleIsDropped = false;
	scene04_bigBallIn = false;
	scene04_bigBallCounter = 0;
	scene04_bigBallFromLeft = false;
	scene04_speakerVariant = 0;
	scene04_speakerPhase = 0;
	scene04_clockCanGo = false;
	scene04_objectIsTaken = false;
	scene04_springOffset = 0;
	scene04_lastKozyawka = 0;
	scene04_springDelay = 0;
	scene04_bottleY = 0;
	scene04_ladderOffset = 0;

	scene05_handle = 0;
	scene05_wacko = 0;
	scene05_bigHatch = 0;
	scene05_wackoTicker = 0;
	scene05_handleFlipper = 0;
	scene05_floatersTicker = 0;

	scene06_manX = 0;
	scene06_manY = 0;
	scene06_ballX = 0;
	scene06_ballY = 0;
	scene06_mumsy = 0;
	scene06_someBall = 0;
	scene06_invHandle = 0;
	scene06_liftButton = 0;
	scene06_ballDrop = 0;
	scene06_arcadeEnabled = false;
	scene06_aimingBall = false;
	scene06_currentBall = 0;
	scene06_ballInHands = 0;
	scene06_flyingBall = 0;
	scene06_numBallsGiven = 0;
	scene06_mumsyNumBalls = 0;
	scene06_eggieTimeout = 0;
	scene06_eggieDirection = true;
	scene06_mumsyGotBall = 0;
	scene06_ballDeltaX = 0;
	scene06_ballDeltaY = 0;
	scene06_sceneClickX = 0;
	scene06_sceneClickY = 0;
	scene06_mumsyPos = 0;
	scene06_mumsyJumpBk = 0;
	scene06_mumsyJumpFw = 0;
	scene06_mumsyJumpBkPercent = 0;
	scene06_mumsyJumpFwPercent = 0;

	scene07_lukeAnim = 0;
	scene07_lukePercent = 0;
	scene07_plusMinus = 0;

	scene08_batuta = 0;
	scene08_vmyats = 0;
	scene08_clock = 0;
	scene08_inAir = false;
	scene08_flyingUp = false;
	scene08_onBelly = false;
	scene08_stairsOffset = -37;
	scene08_snoringCountdown = -1;
	scene08_inArcade = false;
	scene08_stairsVisible = true;
	scene08_manOffsetY = 0;

	scene09_flyingBall = 0;
	scene09_numSwallenBalls = 0;
	scene09_gulper = 0;
	scene09_spitter = 0;
	scene09_grit = 0;
	scene09_dudeY = 0;
	scene09_gulperIsPresent = true;
	scene09_dudeIsOnLadder = false;
	scene09_interactingHanger = -1;
	scene09_intHangerPhase = -1;
	scene09_intHangerMaxPhase = -1000;
	scene09_numMovingHangers = 0;
	scene09_clickY = 0;
	scene09_hangerOffsets[0].x = 0;
	scene09_hangerOffsets[0].y = -15;
	scene09_hangerOffsets[1].x = 15;
	scene09_hangerOffsets[1].y = 0;
	scene09_hangerOffsets[2].x = 0;
	scene09_hangerOffsets[2].y = 0;
	scene09_hangerOffsets[3].x = 0;
	scene09_hangerOffsets[3].y = 0;

	scene10_gum = 0;
	scene10_packet = 0;
	scene10_packet2 = 0;
	scene10_inflater = 0;
	scene10_ladder = 0;
	scene10_hasGum = 0;

	scene11_swingie = 0;
	scene11_boots = 0;
	scene11_dudeOnSwing = 0;
	scene11_hint = 0;
	scene11_arcadeIsOn = false;
	scene11_scrollIsEnabled = false;
	scene11_scrollIsMaximized = false;
	scene11_hintCounter = 0;
	scene11_swingieScreenEdge = 0;
	scene11_crySound = 0;
	scene11_swingAngle = 1.0;
	scene11_swingOldAngle = 1.0;
	scene11_swingSpeed = 1.0;
	scene11_swingAngleDiff = 1.0;
	scene11_swingInertia = 0.01;
	scene11_swingCounter = 0;
	scene11_swingCounterPrevTurn = 0;
	scene11_swingDirection = 0;
	scene11_swingDirectionPrevTurn = 0;
	scene11_swingIsSwinging = false;
	scene11_swingieStands = false;
	scene11_dudeX = 0;
	scene11_dudeY = 0;
	scene11_swingMaxAngle = 45;

	scene12_fly = 0;
	scene12_flyCountdown = 0;

	scene13_whirlgig = 0;
	scene13_guard = 0;
	scene13_handleR = 0;
	scene13_handleL = 0;
	scene13_bridge = 0;
	scene13_guardDirection = false;
	scene13_dudeX = 0;

	scene14_grandma = 0;
	scene14_sceneDeltaX = 0;
	scene14_sceneDeltaY = 0;
	scene14_arcadeIsOn = false;
	scene14_dudeIsKicking = false;
	scene14_ballIsFlying = false;
	scene14_dudeCanKick = false;
	scene14_sceneDiffX = 0;
	scene14_sceneDiffY = 0;
	scene14_pink = 0;
	scene14_flyingBall = 0;
	scene14_balls.clear();
	scene14_grandmaIsHere = false;
	scene14_dudeX = 0;
	scene14_dudeY = 0;
	scene14_grandmaX = 0;
	scene14_grandmaY = 0;
	scene14_dude2X = 0;
	scene14_ballDeltaX = 0;
	scene14_ballDeltaY = 0;
	scene14_ballX = 0;
	scene14_ballY = 0;
	scene14_hitsLeft = 0;

	scene15_chantingCountdown = 0;
	scene15_plusminus = 0;
	scene15_ladder = 0;
	scene15_boot = 0;

	scene16_figures.clear();
	scene16_walkingBoy = 0;
	scene16_walkingGirl = 0;
	scene16_walkingCount = 0;
	scene16_wire = 0;
	scene16_mug = 0;
	scene16_jettie = 0;
	scene16_boot = 0;
	scene16_girlIsLaughing = false;
	scene16_sound = 0;
	scene16_placeIsOccupied = false;

	scene17_flyState = 0;
	scene17_sugarIsShown = false;
	scene17_sceneOldEdgeX = 0;
	scene17_flyCountdown = 0;
	scene17_hand = 0;
	scene17_handPhase = false;
	scene17_sceneEdgeX = 0;

	scene18_inScene18p1 = false;
	scene18_whirlgig = 0;
	scene18_wheelCenterX = 0;
	scene18_wheelCenterY = 0;
	scene18_bridgeIsConvoluted = false;
	scene18_whirlgigMovMum = 0;
	scene18_girlIsSwinging = false;
	scene18_rotationCounter = 0;
	scene18_manY = 0;
	scene18_wheelFlipper = false;
	scene18_wheelIsTurning = true;
	scene18_kidIsOnWheel = -1;
	scene18_boyIsOnWheel = 0;
	scene18_girlIsOnWheel = 0;
	scene18_boyJumpedOff = true;
	scene18_jumpDistance = -1;
	scene18_jumpAngle = -1;
	scene18_manIsReady = false;
	scene18_enteredTrubaRight = false;
	scene18_manWheelPos = 0;
	scene18_manWheelPosTo = -1;
	scene18_kidWheelPos = 0;
	scene18_kidWheelPosTo = 0;
	scene18_boy = 0;
	scene18_girl = 0;
	scene18_domino = 0;
	scene18_boyJumpX = 290;
	scene18_boyJumpY = -363;
	scene18_girlJumpX = 283;
	scene18_girlJumpY = -350;

	scene19_enteredTruba3 = false;

	scene20_fliesCountdown = 0;
	scene20_grandma = 0;

	scene21_giraffeBottom = 0;
	scene21_giraffeBottomX = 0;
	scene21_giraffeBottomY = 0;
	scene21_pipeIsOpen = false;
	scene21_wigglePos = 0.0;
	scene21_wiggleTrigger = 0;

	scene22_bag = 0;
	scene22_giraffeMiddle = 0;
	scene22_dudeIsOnStool = false;
	scene22_interactionIsDisabled = false;
	scene22_craneIsOut = true;
	scene22_numBagFalls = 1;

	scene23_calend0 = 0;
	scene23_calend1 = 0;
	scene23_calend2 = 0;
	scene23_calend3 = 0;
	scene23_topReached = false;
	scene23_isOnStool = false;
	scene23_someVar = 0;
	scene23_giraffeTop = 0;
	scene23_giraffee = 0;

	scene24_jetIsOn = false;
	scene24_flowIsLow = false;
	scene24_waterIsOn = false;
	scene24_water = 0;
	scene24_jet = 0;
	scene24_drop = 0;

	scene25_water = 0;
	scene25_board = 0;
	scene25_drop = 0;
	scene25_dudeIsOnBoard = false;
	scene25_waterIsPresent = false;
	scene25_boardIsSelectable = false;
	scene25_beardersAreThere = false;
	scene25_beardersCounter = 0;
	scene25_bearders.clear();
	scene25_sneezeFlipper = false;

	scene26_chhi = 0;
	scene26_drop = 0;
	scene26_sockPic = 0;
	scene26_sock = 0;
	scene26_activeVent = 0;

	scene27_hitZone = 0;
	scene27_driver = 0;
	scene27_maid = 0;
	scene27_batHandler = 0;
	scene27_driverHasVent = true;
	scene27_bat = 0;
	scene27_dudeIsAiming = false;
	scene27_maxPhaseReached = false;
	scene27_wipeIsNeeded = false;
	scene27_driverPushedButton = false;
	scene27_numLostBats = 0;
	scene27_knockCount = 0;
	scene27_aimStartX = 0;
	scene27_aimStartY = 0;
	scene27_launchPhase = 0;

	scene28_fliesArePresent = true;
	scene28_beardedDirection = true;
	scene28_darkeningObject = 0;
	scene28_lighteningObject = 0;
	scene28_headDirection = false;
	scene28_headBeardedFlipper = false;
	scene28_lift6inside = false;

	scene29_porter = 0;
	scene29_shooter1 = 0;
	scene29_shooter2 = 0;
	scene29_ass = 0;
	scene29_manIsRiding = false;
	scene29_arcadeIsOn = false;
	scene29_reachedFarRight = false;
	scene29_rideBackEnabled = false;
	scene29_shootCountdown = 0;
	scene29_shootDistance = 75;
	scene29_manIsHit = 0;
	scene29_scrollSpeed = 0;
	scene29_scrollingDisabled = 0;
	scene29_hitBall = 0;
	scene29_manX = 0;
	scene29_manY = 0;

	scene30_leg = 0;
	scene30_liftFlag = 1;

	scene31_chantingCountdown = 0;
	scene31_cactus = 0;
	scene31_plusMinus = 0;

	scene32_flagIsWaving = false;
	scene32_flagNeedsStopping = false;
	scene32_dudeIsSitting = false;
	scene32_cactusCounter = -1;
	scene32_dudeOnLadder = false;
	scene32_cactusIsGrowing = false;
	scene32_flag = 0;
	scene32_cactus = 0;
	scene32_massOrange = 0;
	scene32_massBlue = 0;
	scene32_massGreen = 0;
	scene32_button = 0;

	scene33_mug = 0;
	scene33_jettie = 0;
	scene33_cube = 0;
	scene33_cubeX = -1;
	scene33_handleIsDown = false;

	for (int i = 0; i < 9; i++) {
		scene33_ventsX[i] = 0;
		scene33_ventsState[i] = 0;
	}

	scene34_cactus = 0;
	scene34_vent = 0;
	scene34_hatch = 0;
	scene34_boot = 0;
	scene34_dudeClimbed = false;
	scene34_dudeOnBoard = false;
	scene34_dudeOnCactus = false;
	scene34_fliesCountdown = 0;

	scene35_hose = 0;
	scene35_bellyInflater = 0;
	scene35_flowCounter = 0;
	scene35_fliesCounter = 0;

	scene36_rotohrust = 0;
	scene36_scissors = 0;

	scene37_rings.clear();
	scene37_lastDudeX = -1;
	scene37_pipeIsOpen = 0;
	scene37_plusMinus1 = 0;
	scene37_plusMinus2 = 0;
	scene37_plusMinus3 = 0;
	scene37_soundFlipper = 0;
	scene37_dudeX = 0;

	scene38_boss = 0;
	scene38_tally = 0;
	scene38_shorty = 0;
	scene38_domino0 = 0;
	scene38_dominos = 0;
	scene38_domino1 = 0;
	scene38_bottle = 0;
	scene38_bossCounter = 0;
	scene38_lastBossAnim = 0;
	scene38_bossAnimCounter = 0;
	scene38_tallyCounter = 0;
	scene38_lastTallyAnim = 0;
	scene38_tallyAnimCounter = 0;
	scene38_shortyCounter = 0;
	scene38_lastShortyAnim = 0;
	scene38_shortyAnimCounter = 0;

	sceneFinal_var01 = 0;
	sceneFinal_var02 = 0;
	sceneFinal_var03 = 0;
	sceneFinal_trackHasStarted = false;

	selector = 0;
}

static int scenes[] = {
	SC_1,  SC_2,  SC_3,  SC_4,  SC_5,  SC_6,  SC_7,  SC_8,  SC_9,  SC_10,
	SC_11, SC_12, SC_13, SC_14, SC_15, SC_16, SC_17, SC_18, SC_19, SC_20,
	SC_21, SC_22, SC_23, SC_24, SC_25, SC_26, SC_27, SC_28, SC_29, SC_30,
	SC_31, SC_32, SC_33, SC_34, SC_35, SC_36, SC_37, SC_38, SC_FINAL1, SC_DBGMENU
};

static int scenesD[] = {
	PIC_SCD_1,  PIC_SCD_2,  PIC_SCD_3,  PIC_SCD_4,  PIC_SCD_5,  PIC_SCD_6,  PIC_SCD_7,  PIC_SCD_8,  PIC_SCD_9,  PIC_SCD_10,
	PIC_SCD_11, PIC_SCD_12, PIC_SCD_13, PIC_SCD_14, PIC_SCD_15, PIC_SCD_16, PIC_SCD_17, PIC_SCD_18, PIC_SCD_19, PIC_SCD_20,
	PIC_SCD_21, PIC_SCD_22, PIC_SCD_23, PIC_SCD_24, PIC_SCD_25, PIC_SCD_26, PIC_SCD_27, PIC_SCD_28, PIC_SCD_29, PIC_SCD_30,
	PIC_SCD_31, PIC_SCD_32, PIC_SCD_33, PIC_SCD_34, PIC_SCD_35, PIC_SCD_36, PIC_SCD_37, PIC_SCD_38, PIC_SCD_FIN, 0
};

int NGIEngine::convertScene(int scene) {
	if (!scene || scene >= SC_1)
		return scene;

	if (scene < 1 || scene > 40)
		return SC_1;

	return scenes[scene - 1];
}

int NGIEngine::getSceneEntrance(int scene) {
	for (int i = 0; i < 40; i++)
		if (scenes[i] == scene)
			return scenesD[i];

	return 0;
}

int NGIEngine::getSceneFromTag(int tag) {
	for (int i = 0; i < ARRAYSIZE(scenes); i++) {
		if (scenes[i] == tag)
			return i + 1;
	}

	return 1;
}

void NGIEngine::sceneAutoScrolling() {
	if (_aniMan2 == _aniMan && _currentScene && !_currentScene->_messageQueueId) {
		if (800 - _mouseScreenPos.x >= 47 || _sceneRect.right >= _sceneWidth - 1 || _aniMan->_ox <= _sceneRect.left + 230) {
			if (_mouseScreenPos.x < 47 && _sceneRect.left > 0 && _aniMan->_ox < _sceneRect.right - 230)
				_currentScene->_x = -10;
		} else {
			_currentScene->_x = 10;
		}
	}
}

bool NGIEngine::sceneSwitcher(const EntranceInfo &entrance) {
	GameVar *sceneVar;
	Common::Point sceneDim;

	Scene *scene = accessScene(entrance._sceneId);

	if (!scene)
		return 0;

	sceneDim = scene->_picObjList.front()->getDimensions();
	_sceneWidth = sceneDim.x;
	_sceneHeight = sceneDim.y;

	_sceneRect.top = 0;
	_sceneRect.left = 0;
	_sceneRect.right = 799;
	_sceneRect.bottom = 599;

	scene->_x = 0;
	scene->_y = 0;

	_aniMan->setOXY(0, 0);
	_aniMan->clearFlags();
	_aniMan->_callback1 = 0; // Really NULL
	_aniMan->_callback2 = 0; // Really NULL
	_aniMan->_shadowsOn = 1;

	_scrollSpeed = 8;

	_isSaveAllowed = true;
	_updateFlag = true;
	_flgCanOpenMap = true;

	if (entrance._sceneId == SC_DBGMENU) {
		_inventoryScene = 0;
	} else {
		_gameLoader->loadScene(SC_INV);
		getGameLoaderInventory()->rebuildItemRects();
		_inventoryScene = getGameLoaderInventory()->getScene();
	}
	if (_soundEnabled) {
		if (scene->_soundList) {
			_currSoundListCount = 2;
			_currSoundList1[0] = accessScene(SC_COMMON)->_soundList.get();
			_currSoundList1[1] = scene->_soundList.get();

			for (int i = 0; i < scene->_soundList->getCount(); i++) {
				scene->_soundList->getSoundByIndex(i).updateVolume();
			}
		} else {
			_currSoundListCount = 1;
			_currSoundList1[0] = accessScene(SC_COMMON)->_soundList.get();
		}
	}

	getGameLoaderInteractionController()->sortInteractions(scene->_sceneId);
	_currentScene = scene;
	scene->addStaticANIObject(_aniMan, 1);
	_scene2 = scene;
	_aniMan->_movement = 0;
	_aniMan->_statics = _aniMan->getStaticsById(ST_MAN_EMPTY);
	_aniMan->setOXY(0, 0);

	_aniMan2 = _aniMan;
	MctlCompound *cmp = getSc2MctlCompoundBySceneId(entrance._sceneId);
	cmp->initMctlGraph();
	cmp->attachObject(_aniMan);
	cmp->activate();
	getGameLoaderInteractionController()->enableFlag24();
	setInputDisabled(0);

	scene->setPictureObjectsFlag4();

	for (uint i = 0; i < scene->_staticANIObjectList1.size(); i++)
		scene->_staticANIObjectList1[i]->_flags &= 0xFE7F;

	PictureObject *p = accessScene(SC_INV)->getPictureObjectById(PIC_INV_MENU, 0);
	p->setFlags(p->_flags & 0xFFFB);

	removeMessageHandler(2, -1);
	_updateScreenCallback = 0;

	switch (entrance._sceneId) {
	case SC_INTRO1:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_INTRO1");
		scene->preloadMovements(sceneVar);

		if (!(g_nmi->isDemo() && g_nmi->getLanguage() == Common::RU_RUS))
			sceneIntro_initScene(scene);
		else
			sceneIntroDemo_initScene(scene);

		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_INTRO1");
		setSceneMusicParameters(sceneVar);

		if (!(g_nmi->isDemo() && g_nmi->getLanguage() == Common::RU_RUS)) {
			addMessageHandler(sceneHandlerIntro, 2);
			_updateCursorCallback = sceneIntro_updateCursor;
		} else {
			addMessageHandler(sceneHandlerIntroDemo, 2);
			_updateCursorCallback = sceneIntroDemo_updateCursor;
		}
		break;

	case SC_1:
		scene01_fixEntrance();
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_1");
		scene->preloadMovements(sceneVar);
		scene01_initScene(scene, entrance._field_4);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_1");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler01, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_2:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_2");
		scene->preloadMovements(sceneVar);
		scene02_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_2");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler02, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_3:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_3");
		scene->preloadMovements(sceneVar);
		scene03_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_3");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler03, 2);
		scene03_setEaterState();
		_updateCursorCallback = scene03_updateCursor;
		break;

	case SC_4:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_4");
		scene->preloadMovements(sceneVar);
		scene04_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_4");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler04, 2, 2);
		_updateCursorCallback = scene04_updateCursor;
		break;

	case SC_5:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_5");
		scene->preloadMovements(sceneVar);
		scene05_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_5");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler05, 2, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_6:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_6");
		scene->preloadMovements(sceneVar);
		scene06_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_6");
		setSceneMusicParameters(sceneVar);
		scene06_initMumsy();
		insertMessageHandler(sceneHandler06, 2, 2);
		_updateCursorCallback = scene06_updateCursor;
		break;

	case SC_7:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_7");
		scene->preloadMovements(sceneVar);
		scene07_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_7");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler07, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_8:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_8");
		scene->preloadMovements(sceneVar);
		scene08_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_8");
		setSceneMusicParameters(sceneVar);
		scene08_setupMusic();
		addMessageHandler(sceneHandler08, 2);
		_updateCursorCallback = scene08_updateCursor;
		break;

	case SC_9:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_9");
		scene->preloadMovements(sceneVar);
		scene09_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_9");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler09, 2, 2);
		_updateCursorCallback = scene09_updateCursor;
		break;

	case SC_10:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_10");
		scene->preloadMovements(sceneVar);
		scene10_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_10");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler10, 2, 2);
		_updateCursorCallback = scene10_updateCursor;
		break;

	case SC_11:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_11");
		scene->preloadMovements(sceneVar);
		scene11_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_11");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler11, 2, 2);
		scene11_setupMusic();
		_updateCursorCallback = scene11_updateCursor;
		break;

	case SC_12:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_12");
		scene->preloadMovements(sceneVar);
		scene12_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_12");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler12, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_13:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_13");
		scene->preloadMovements(sceneVar);
		scene13_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_13");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler13, 2, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_14:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_14");
		scene->preloadMovements(sceneVar);
		scene14_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_14");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler14, 2, 2);
		scene14_setupMusic();
		_updateCursorCallback = scene14_updateCursor;
		break;

	case SC_15:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_15");
		scene->preloadMovements(sceneVar);
		scene15_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_15");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler15, 2, 2);
		_updateCursorCallback = scene15_updateCursor;
		break;

	case SC_16:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_16");
		scene->preloadMovements(sceneVar);
		scene16_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_16");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler16, 2);
		_updateCursorCallback = scene16_updateCursor;
		break;

	case SC_17:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_17");
		scene->preloadMovements(sceneVar);
		scene17_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_17");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler17, 2);
		scene17_restoreState();
		_updateCursorCallback = scene17_updateCursor;
		break;

	case SC_18:
		scene18_setupEntrance();
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_18");
		scene->preloadMovements(sceneVar);
		g_nmi->stopAllSounds();

		if (g_vars->scene18_inScene18p1)
			scene18_initScene1(scene);
		else
			scene18_initScene2(scene);

		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_18");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler18, 2, 2);
		_updateCursorCallback = scene18_updateCursor;
		break;

	case SC_19:
		if (!g_nmi->_scene3) {
			g_nmi->_scene3 = accessScene(SC_18);
			g_nmi->_gameLoader->loadScene(SC_18);

			scene18_initScene2(g_nmi->_scene3);
			scene18_preload();
			scene19_setMovements(g_nmi->_scene3, entrance._field_4);

			g_vars->scene18_inScene18p1 = true;
		}

		scene19_preload();
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_19");
		scene->preloadMovements(sceneVar);
		g_nmi->stopAllSounds();

		if (g_vars->scene18_inScene18p1)
			scene18_initScene1(scene);
		else
			scene19_initScene2();

		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_19");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler19, 2);
		scene19_setSugarState(scene);
		_updateCursorCallback = scene19_updateCursor;
		break;

	case SC_20:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_20");
		scene->preloadMovements(sceneVar);
		scene20_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_20");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler20, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_21:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_21");
		scene->preloadMovements(sceneVar);
		scene21_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_21");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler21, 2, 2);
		_updateCursorCallback = scene21_updateCursor;
		break;

	case SC_22:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_22");
		scene->preloadMovements(sceneVar);
		scene22_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_22");
		setSceneMusicParameters(sceneVar);
		scene22_setBagState();
		insertMessageHandler(sceneHandler22, 2, 2);
		_updateCursorCallback = scene22_updateCursor;
		break;

	case SC_23:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_23");
		scene->preloadMovements(sceneVar);
		scene23_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_23");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler23, 2, 2);
		scene23_setGiraffeState();
		_updateCursorCallback = scene23_updateCursor;
		break;

	case SC_24:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_24");
		scene->preloadMovements(sceneVar);
		scene24_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_24");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler24, 2);
		scene24_setPoolState();
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_25:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_25");
		scene->preloadMovements(sceneVar);
		scene25_initScene(scene, entrance._field_4);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_25");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler25, 2);
		scene25_setupWater(scene, entrance._field_4);
		_updateCursorCallback = scene25_updateCursor;
		break;

	case SC_26:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_26");
		scene->preloadMovements(sceneVar);
		scene26_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_26");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler26, 2, 2);
		scene26_setupDrop(scene);
		_updateCursorCallback = scene26_updateCursor;
		break;

	case SC_27:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_27");
		scene->preloadMovements(sceneVar);
		scene27_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_27");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler27, 2);
		_updateCursorCallback = scene27_updateCursor;
		break;

	case SC_28:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_28");
		scene->preloadMovements(sceneVar);
		scene28_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_28");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler28, 2, 2);
		_updateCursorCallback = scene28_updateCursor;
		break;

	case SC_29:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_29");
		scene->preloadMovements(sceneVar);
		scene29_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_29");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler29, 2);
		_updateCursorCallback = scene29_updateCursor;
		break;

	case SC_30:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_30");
		scene->preloadMovements(sceneVar);
		scene30_initScene(scene, entrance._field_4);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_30");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler30, 2);
		_updateCursorCallback = scene30_updateCursor;
		break;

	case SC_31:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_31");
		scene->preloadMovements(sceneVar);
		scene31_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_31");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler31, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_32:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_32");
		scene->preloadMovements(sceneVar);
		scene32_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_32");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler32, 2, 2);
		scene32_setupMusic();
		_updateCursorCallback = scene32_updateCursor;
		break;

	case SC_33:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_33");
		scene->preloadMovements(sceneVar);
		scene33_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_33");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler33, 2, 2);
		scene33_setupMusic();
		_updateCursorCallback = scene33_updateCursor;
		break;

	case SC_34:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_34");
		scene->preloadMovements(sceneVar);
		scene34_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_34");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler34, 2, 2);
		scene34_initBeh();
		_updateCursorCallback = scene34_updateCursor;
		break;

	case SC_35:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_35");
		scene->preloadMovements(sceneVar);
		scene35_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_35");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler35, 2, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_36:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_36");
		scene->preloadMovements(sceneVar);
		scene36_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_36");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler36, 2);
		_updateCursorCallback = scene36_updateCursor;
		break;

	case SC_37:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_37");
		scene->preloadMovements(sceneVar);
		scene37_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_37");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler37, 2, 2);
		_updateCursorCallback = scene37_updateCursor;
		break;

	case SC_38:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_38");
		scene->preloadMovements(sceneVar);
		scene38_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_38");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler38, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_FINAL1:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_FINAL1");
		scene->preloadMovements(sceneVar);
		sceneFinal_initScene();
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_FINAL1");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandlerFinal, 2);
		_updateCursorCallback = sceneFinal_updateCursor;
		break;

	case SC_DBGMENU:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_DBGMENU");
		scene->preloadMovements(sceneVar);
		sceneDbgMenu_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_DBGMENU");
		addMessageHandler(sceneHandlerDbgMenu, 2);
		break;

	default:
		error("Unknown scene %d", entrance._sceneId);
		break;
	}

	return true;
}

int defaultUpdateCursor() {
	g_nmi->updateCursorCommon();

	return g_nmi->_cursorId;
}

void NGIEngine::updateMapPiece(int mapId, int update) {
	for (int i = 0; i < 200; i++) {
		int hiWord = (_mapTable[i] >> 16) & 0xffff;

		if (hiWord == mapId) {
			_mapTable[i] |= update;
			return;
		}
		if (!hiWord) {
			_mapTable[i] = (mapId << 16) | update;
			return;
		}
	}
}

void NGIEngine::updateMap(PreloadItem *pre) {
	switch (pre->sceneId) {
	case SC_1:
		updateMapPiece(PIC_MAP_S01, 1);

		if (pre->param == TrubaUp)
			updateMapPiece(PIC_MAP_P01, 1);

		if (pre->param == TrubaLeft)
			updateMapPiece(PIC_MAP_A13, 1);
		break;

	case SC_2:
		updateMapPiece(PIC_MAP_S02, 1);

		if (pre->param == TrubaLeft)
			updateMapPiece(PIC_MAP_P01, 1);

		break;

	case SC_3:
		updateMapPiece(PIC_MAP_S03, 1);
		break;

	case SC_4:
		updateMapPiece(PIC_MAP_S04, 1);

		if (pre->param == TrubaRight)
			updateMapPiece(PIC_MAP_P04, 1);

		break;

	case SC_5:
		updateMapPiece(PIC_MAP_S05, 1);

		if (pre->param == TrubaLeft) {
			updateMapPiece(PIC_MAP_P04, 1);
		}

		if (pre->param == TrubaUp) {
			updateMapPiece(PIC_MAP_P05, 1);
			updateMapPiece(PIC_MAP_A11, 1);
		}

		break;

	case SC_6:
		updateMapPiece(PIC_MAP_S06, 1);

		if (pre->param == TrubaUp)
			updateMapPiece(PIC_MAP_A12, 1);

		break;

	case SC_7:
		updateMapPiece(PIC_MAP_S07, 1);

		if (pre->param == TrubaLeft)
			updateMapPiece(PIC_MAP_P18, 1);

		break;

	case SC_8:
		updateMapPiece(PIC_MAP_S08, 1);

		if (pre->param == TrubaUp)
			updateMapPiece(PIC_MAP_P11, 1);

		if (pre->param == TrubaRight)
			updateMapPiece(PIC_MAP_P18, 1);

		return;

	case SC_9:
		updateMapPiece(PIC_MAP_S09, 1);

		if (pre->param == TrubaDown)
			updateMapPiece(PIC_MAP_P11, 1);

		return;

	case SC_10:
		updateMapPiece(PIC_MAP_S10, 1);

		if (pre->param == TrubaRight)
			updateMapPiece(PIC_MAP_P02, 1);

		break;

	case SC_11:
		updateMapPiece(PIC_MAP_S11, 1);

		if (pre->param == TrubaLeft)
			updateMapPiece(PIC_MAP_P02, 1);

		break;

	case SC_12:
		updateMapPiece(PIC_MAP_S12, 1);
		break;

	case SC_13:
		updateMapPiece(PIC_MAP_S13, 1);

		if (pre->param == TrubaUp) {
			updateMapPiece(PIC_MAP_P06, 1);
			updateMapPiece(PIC_MAP_A10, 1);
		}
		break;

	case SC_14:
		updateMapPiece(PIC_MAP_S14, 1);
		break;

	case SC_15:
		updateMapPiece(PIC_MAP_S15, 1);

		if (pre->param == TrubaUp) {
			updateMapPiece(PIC_MAP_P08, 1);
			updateMapPiece(PIC_MAP_A14, 1);
		}

		break;

	case SC_16:
		updateMapPiece(PIC_MAP_S16, 1);
		break;

	case SC_17:
		updateMapPiece(PIC_MAP_S17, 1);
		break;

	case SC_18:
		updateMapPiece(PIC_MAP_S1819, 1);

		if (pre->param == PIC_SC18_RTRUBA)
			updateMapPiece(PIC_MAP_P14, 1);

		break;

	case SC_19:
		updateMapPiece(PIC_MAP_S1819, 1);

		if (pre->param == PIC_SC19_RTRUBA3) {
			updateMapPiece(PIC_MAP_P15, 1);
			updateMapPiece(PIC_MAP_A09, 1);
		}

		break;

	case SC_20:
		updateMapPiece(PIC_MAP_S20, 1);
		break;

	case SC_21:
		updateMapPiece(PIC_MAP_S21, 1);

		if (pre->param == TrubaLeft) {
			updateMapPiece(PIC_MAP_P15, 1);
			updateMapPiece(PIC_MAP_A09, 1);
		}

		if (pre->param == TrubaDown)
			updateMapPiece(PIC_MAP_A08, 1);

		break;

	case SC_22:
		updateMapPiece(PIC_MAP_S22, 1);
		break;

	case SC_23:
		if (getObjectState(sO_UpperHatch_23) == getObjectEnumState(sO_UpperHatch_23, sO_Opened)) {
			updateMapPiece(PIC_MAP_S23_1, 0);
			updateMapPiece(PIC_MAP_S23_2, 1);
			updateMapPiece(PIC_MAP_P07, 1);
		} else {
			updateMapPiece(PIC_MAP_S23_1, 1);
			updateMapPiece(PIC_MAP_S23_2, 0);
		}
		break;

	case SC_24:
		updateMapPiece(PIC_MAP_S24, 1);

		if (pre->param == TrubaUp)
			updateMapPiece(PIC_MAP_A08, 1);

		if (pre->param == TrubaDown) {
			updateMapPiece(PIC_MAP_P13, 1);
			updateMapPiece(PIC_MAP_A07, 1);
		}
		break;

	case SC_25:
		updateMapPiece(PIC_MAP_S25, 1);
		break;

	case SC_26:
		updateMapPiece(PIC_MAP_S26, 1);

		if (pre->param == TrubaLeft)
			updateMapPiece(PIC_MAP_A06, 1);

		if (pre->param == TrubaUp) {
			updateMapPiece(PIC_MAP_P13, 1);
			updateMapPiece(PIC_MAP_A07, 1);
		}

		break;

	case SC_27:
		updateMapPiece(PIC_MAP_S27, 1);
		break;

	case SC_28:
		updateMapPiece(PIC_MAP_S28, 1);

		if (pre->param == TrubaRight)
			updateMapPiece(PIC_MAP_A06, 1);

		break;

	case SC_29:
		updateMapPiece(PIC_MAP_S29, 1);

		if (pre->param == TrubaUp)
			updateMapPiece(PIC_MAP_A05, 1);

		break;

	case SC_30:
		updateMapPiece(PIC_MAP_S30, 1);

		if (pre->param == TrubaLeft)
			updateMapPiece(PIC_MAP_P09, 1);

		if (pre->param == TrubaRight)
			updateMapPiece(PIC_MAP_A04, 1);

		break;

	case SC_31:
		updateMapPiece(PIC_MAP_S31_2, 1);

		if (getObjectState(sO_Cactus) == getObjectEnumState(sO_Cactus, sO_HasGrown))
			updateMapPiece(PIC_MAP_S31_1, 1);

		if (pre->param == TrubaRight)
			updateMapPiece(PIC_MAP_P09, 1);

		break;

	case SC_32:
		updateMapPiece(PIC_MAP_S32_2, 1);

		if (getObjectState(sO_Cactus) == getObjectEnumState(sO_Cactus, sO_HasGrown))
			updateMapPiece(PIC_MAP_S32_1, 1);

		break;

	case SC_33:
		updateMapPiece(PIC_MAP_S33, 1);
		break;

	case SC_34:
		updateMapPiece(PIC_MAP_S34, 1);

		if (pre->param == TrubaUp)
			updateMapPiece(PIC_MAP_A03, 1);

		break;

	case SC_35:
		updateMapPiece(PIC_MAP_S35, 1);

		if (pre->param == TrubaLeft)
			updateMapPiece(PIC_MAP_A02, 1);

		if (pre->param == TrubaDown)
			updateMapPiece(PIC_MAP_A03, 1);

		break;

	case SC_36:
		updateMapPiece(PIC_MAP_S36, 1);
		break;

	case SC_37:
		updateMapPiece(PIC_MAP_S37, 1);
		updateMapPiece(PIC_MAP_A01, 1);
		break;

	case SC_38:
		updateMapPiece(PIC_MAP_S38, 1);

		switch (pre->preloadId1) {
		case SC_15:
			updateMapPiece(PIC_MAP_P16, 1);
			break;

		case SC_1:
			updateMapPiece(PIC_MAP_P10, 1);
			break;

		case SC_10:
			updateMapPiece(PIC_MAP_P17, 1);
			break;

		case SC_19:
			updateMapPiece(PIC_MAP_P12, 1);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

} // End of namespace NGI
