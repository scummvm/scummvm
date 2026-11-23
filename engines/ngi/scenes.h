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

#ifndef NGI_SCENES_H
#define NGI_SCENES_H

namespace NGI {

struct Bat;
struct BehaviorMove;
struct Hanger;
class AniHandler;
class MctlLadder;
struct Ring;
class AniObject;
struct Swinger;
struct WalkingBearder;

class Fader {
public:
	void start(uint32 fadeIn, uint32 hold, uint32 fadeOut);
	void update(uint32 elapsed);
	void fadeOut();
	bool active() const { return _stage != kDone; }
	byte brightness() const { return _brightness; }

private:
	enum Stage { kFadeIn, kHold, kFadeOut, kDone };
	Stage _stage = kDone;
	uint32 _durations[3] = {};
	uint32 _elapsed = 0;
	byte _brightness = 255;
};

int defaultUpdateCursor();
void sceneChapter_show(const char *chapterId);

int sceneIntro_updateCursor();
void sceneIntro_initScene(Scene *sc);
void sceneIntro_initMan(Scene &scene);
bool sceneIntro_updateAndroid(uint32 elapsed, byte &brightness);
int sceneHandlerIntro(Command *cmd);

int sceneIntroDemo_updateCursor();
void sceneIntroDemo_initScene(Scene *sc);
int sceneHandlerIntroDemo(Command *cmd);

void scene01_fixEntrance();
void scene01_initScene(Scene *sc, int entrance);
void scene01_initObjects(Scene *sc, int entrance, bool eggUncracked);
int sceneHandler01(Command *cmd);

void scene02_initScene(Scene *sc);
int sceneHandler02(Command *ex);

void scene03_setEaterState();
int scene03_updateCursor();
void scene03_initScene(Scene *sc);
int sceneHandler03(Command *cmd);

int scene04_updateCursor();
void scene04_initScene(Scene *sc);
int sceneHandler04(Command *cmd);

void scene05_initScene(Scene *sc);
int sceneHandler05(Command *cmd);

void scene06_initScene(Scene *sc);
void scene06_initMumsy();
int sceneHandler06(Command *cmd);
int scene06_updateCursor();

void scene07_initScene(Scene *sc);
int sceneHandler07(Command *cmd);

void scene08_initScene(Scene *sc);
void scene08_setupMusic();
int sceneHandler08(Command *cmd);
int scene08_updateCursor();

int scene09_updateCursor();
void scene09_initScene(Scene *sc);
int sceneHandler09(Command *cmd);

void scene10_initScene(Scene *sc);
int sceneHandler10(Command *cmd);
int scene10_updateCursor();

void scene11_initScene(Scene *sc);
void scene11_setupMusic();
int sceneHandler11(Command *cmd);
int scene11_updateCursor();

void scene12_initScene(Scene *sc);
int sceneHandler12(Command *ex);

void scene13_initScene(Scene *sc);
int sceneHandler13(Command *ex);

void scene14_initScene(Scene *sc);
void scene14_setupMusic();
int sceneHandler14(Command *cmd);
int scene14_updateCursor();

int scene15_updateCursor();
void scene15_initScene(Scene *sc);
int sceneHandler15(Command *cmd);

void scene16_initScene(Scene *sc);
int sceneHandler16(Command *cmd);
int scene16_updateCursor();

void scene17_initScene(Scene *sc);
void scene17_restoreState();
int sceneHandler17(Command *cmd);
int scene17_updateCursor();

void scene18_preload();
void scene18_setupEntrance();
void scene18_initScene1(Scene *sc);
void scene18_initScene2(Scene *sc);
int sceneHandler18(Command *cmd);
int scene18_updateCursor();

void scene19_preload();
void scene19_setMovements(Scene *sc, int entranceId);
void scene19_initScene2();
int sceneHandler19(Command *cmd);
int scene19_updateCursor();
void scene19_setSugarState(Scene *sc);

void scene20_initScene(Scene *sc);
int sceneHandler20(Command *ex);

int scene21_updateCursor();
void scene21_initScene(Scene *sc);
int sceneHandler21(Command *cmd);

void scene22_initScene(Scene *sc);
void scene22_setBagState();
int sceneHandler22(Command *cmd);
int scene22_updateCursor();

void scene23_initScene(Scene *sc);
void scene23_setGiraffeState();
int sceneHandler23(Command *cmd);
int scene23_updateCursor();

void scene24_initScene(Scene *sc);
void scene24_setPoolState();
int sceneHandler24(Command *cmd);

void scene25_initScene(Scene *sc, int entrance);
void scene25_setupWater(Scene *sc, int entrance);
int sceneHandler25(Command *cmd);
int scene25_updateCursor();

void scene26_initScene(Scene *sc);
void scene26_setupDrop(Scene *sc);
int sceneHandler26(Command *cmd);
int scene26_updateCursor();

void scene27_initScene(Scene *sc);
int sceneHandler27(Command *ex);
int scene27_updateCursor();

void scene28_initScene(Scene *sc);
int sceneHandler28(Command *ex);
int scene28_updateCursor();

int scene29_updateCursor();
void scene29_initScene(Scene *sc);
int sceneHandler29(Command *cmd);

int scene30_updateCursor();
void scene30_initScene(Scene *sc, int flag);
int sceneHandler30(Command *cmd);

void scene31_initScene(Scene *sc);
int sceneHandler31(Command *ex);

void scene32_initScene(Scene *sc);
void scene32_setupMusic();
int sceneHandler32(Command *cmd);
int scene32_updateCursor();

void scene33_initScene(Scene *sc);
void scene33_setupMusic();
int sceneHandler33(Command *cmd);
int scene33_updateCursor();

void scene34_initScene(Scene *sc);
void scene34_initBeh();
int sceneHandler34(Command *cmd);
int scene34_updateCursor();

void scene35_initScene(Scene *sc);
int sceneHandler35(Command *cmd);

int scene36_updateCursor();
void scene36_initScene(Scene *sc);
int sceneHandler36(Command *cmd);

void scene37_initScene(Scene *sc);
int sceneHandler37(Command *ex);
int scene37_updateCursor();

void scene38_initScene(Scene *sc);
int sceneHandler38(Command *ex);

int sceneFinal_updateCursor();
void sceneFinal_initScene();
int sceneHandlerFinal(Command *cmd);

void sceneDbgMenu_initScene(Scene *sc);
int sceneHandlerDbgMenu(Command *cmd);

class Vars {
public:
	Vars();

	GameVar *swallowedEgg1;
	GameVar *swallowedEgg2;
	GameVar *swallowedEgg3;

	AniObject *sceneIntro_aniin1man;
	bool sceneIntro_needSleep;
	Fader sceneIntro_fader;
	bool sceneIntro_needGetup;
	bool sceneIntro_skipIntro;
	bool sceneIntro_playing;
	bool sceneIntro_needBlackout;

	PictureObject *scene01_picSc01Osk;
	PictureObject *scene01_picSc01Osk2;

	AniObject *scene02_guvTheDrawer;
	int scene02_boxDelay;
	bool scene02_boxOpen;

	AniObject *scene03_eggeater;
	AniObject *scene03_domino;

	PictureObject *scene04_bottle;
	AniObject *scene04_hand;
	AniObject *scene04_plank;
	AniObject *scene04_clock;
	AniObject *scene04_spring;
	AniObject *scene04_mamasha;
	AniObject *scene04_boot;
	AniObject *scene04_speaker;

	Common::Point scene04_jumpingKozyawki[20];
	Common::Point scene04_jumpRotateKozyawki[20];

	Common::List<AniObject *> scene04_kozyawkiObjList;
	Common::List<GameObject *> scene04_bottleObjList;
	Common::List<AniObject *> scene04_kozyawkiAni;

	MctlLadder *scene04_ladder;
	int scene04_ladderOffset;

	bool scene04_coinPut;
	bool scene04_soundPlaying;
	bool scene04_dudeOnLadder;

	int scene04_dynamicPhaseIndex;
	int scene04_sceneClickX;
	int scene04_sceneClickY;
	int scene04_dudePosX;
	int scene04_dudePosY;
	int scene04_bottleY;

	AniObject *scene04_walkingKozyawka;

	int scene04_speakerVariant;
	int scene04_speakerPhase;

	bool scene04_bottleIsTaken;
	bool scene04_kozyawkaOnLadder;
	int scene04_bottleWeight;
	bool scene04_var07;
	bool scene04_ladderClickable;
	bool scene04_handIsDown;
	bool scene04_dudeInBottle;
	bool scene04_kozHeadRaised;
	bool scene04_bottleIsDropped;
	bool scene04_bigBallIn;
	int scene04_bigBallCounter;
	bool scene04_bigBallFromLeft;
	bool scene04_clockCanGo;
	bool scene04_objectIsTaken;
	int scene04_springOffset;
	AniObject *scene04_lastKozyawka;
	int scene04_springDelay;
	int scene04_musicStage;


	AniObject *scene05_handle;
	AniObject *scene05_wacko;
	AniObject *scene05_bigHatch;
	int scene05_wackoTicker;
	int scene05_handleFlipper;
	int scene05_floatersTicker;

	AniObject *scene06_mumsy;
	int scene06_manX;
	int scene06_manY;
	int scene06_ballX;
	int scene06_ballY;
	AniObject *scene06_someBall;
	AniObject *scene06_invHandle;
	AniObject *scene06_liftButton;
	AniObject *scene06_ballDrop;
	bool scene06_arcadeEnabled;
	bool scene06_aimingBall;
	AniObject *scene06_currentBall;
	AniObject *scene06_ballInHands;
	AniObject *scene06_flyingBall;
	Common::Array<AniObject *> scene06_balls;
	int scene06_numBallsGiven;
	int scene06_mumsyNumBalls;
	int scene06_eggieTimeout;
	int scene06_eggieDirection;
	int scene06_mumsyGotBall;
	int scene06_ballDeltaX;
	int scene06_ballDeltaY;
	int scene06_sceneClickX;
	int scene06_sceneClickY;
	int scene06_mumsyPos;
	BehaviorMove *scene06_mumsyJumpBk;
	BehaviorMove *scene06_mumsyJumpFw;
	int scene06_mumsyJumpBkPercent;
	int scene06_mumsyJumpFwPercent;

	BehaviorMove *scene07_lukeAnim;
	int scene07_lukePercent;
	AniObject *scene07_plusMinus;

	AniObject *scene08_batuta;
	AniObject *scene08_vmyats;
	AniObject *scene08_clock;
	bool scene08_inAir;
	bool scene08_flyingUp;
	int scene08_onBelly;
	int scene08_stairsOffset;
	int scene08_snoringCountdown;
	bool scene08_inArcade;
	bool scene08_stairsVisible;
	int scene08_manOffsetY;

	int scene09_dudeY;
	AniObject *scene09_flyingBall;
	int scene09_numSwallenBalls;
	AniObject *scene09_gulper;
	AniObject *scene09_spitter;
	AniObject *scene09_grit;
	bool scene09_gulperIsPresent;
	bool scene09_dudeIsOnLadder;
	int scene09_interactingHanger;
	int scene09_intHangerPhase;
	int scene09_intHangerMaxPhase;
	Common::Array<AniObject *> scene09_flyingBalls;
	Common::Array<Hanger *> scene09_hangers;
	Common::Array<AniObject *> scene09_sceneBalls;
	int scene09_numMovingHangers;
	int scene09_clickY;
	Common::Point scene09_hangerOffsets[4];

	AniObject *scene10_gum;
	AniObject *scene10_packet;
	AniObject *scene10_packet2;
	AniObject *scene10_inflater;
	PictureObject *scene10_ladder;
	int scene10_hasGum;

	AniObject *scene11_swingie;
	AniObject *scene11_boots;
	AniObject *scene11_dudeOnSwing;
	PictureObject *scene11_hint;
	AniHandler scene11_aniHandler;
	bool scene11_arcadeIsOn;
	bool scene11_scrollIsEnabled;
	bool scene11_scrollIsMaximized;
	int scene11_hintCounter;
	int scene11_swingieScreenEdge;
	int scene11_crySound;
	double scene11_swingAngle;
	double scene11_swingOldAngle;
	double scene11_swingSpeed;
	double scene11_swingAngleDiff;
	double scene11_swingInertia;
	int scene11_swingCounter;
	int scene11_swingCounterPrevTurn;
	int scene11_swingDirection;
	int scene11_swingDirectionPrevTurn;
	bool scene11_swingIsSwinging;
	bool scene11_swingieStands;
	int scene11_dudeX;
	int scene11_dudeY;
	int scene11_swingMaxAngle;

	int scene12_fly;
	int scene12_flyCountdown;

	AniObject *scene13_whirlgig;
	AniObject *scene13_guard;
	AniObject *scene13_handleR;
	AniObject *scene13_handleL;
	AniObject *scene13_bridge;
	bool scene13_guardDirection;
	int scene13_dudeX;

	AniObject *scene14_grandma;
	int scene14_sceneDeltaX;
	int scene14_sceneDeltaY;
	bool scene14_arcadeIsOn;
	bool scene14_dudeIsKicking;
	bool scene14_ballIsFlying;
	bool scene14_dudeCanKick;
	int scene14_sceneDiffX;
	int scene14_sceneDiffY;
	AniObject *scene14_pink;
	AniObject *scene14_flyingBall;
	Common::List<AniObject *> scene14_balls;
	bool scene14_grandmaIsHere;
	int scene14_dudeX;
	int scene14_dudeY;
	int scene14_grandmaX;
	int scene14_grandmaY;
	int scene14_dude2X;
	int scene14_ballDeltaX;
	int scene14_ballDeltaY;
	int scene14_ballX;
	int scene14_ballY;
	int scene14_hitsLeft;
	Common::Point scene14_mouseCursorPos;

	int scene15_chantingCountdown;
	AniObject *scene15_plusminus;
	PictureObject *scene15_ladder;
	AniObject *scene15_boot;

	Common::List<AniObject *> scene16_figures;
	AniObject *scene16_walkingBoy;
	AniObject *scene16_walkingGirl;
	int scene16_walkingCount;
	AniObject *scene16_wire;
	AniObject *scene16_mug;
	AniObject *scene16_jettie;
	AniObject *scene16_boot;
	bool scene16_girlIsLaughing;
	int scene16_sound;
	bool scene16_placeIsOccupied;

	int scene17_flyState;
	bool scene17_sugarIsShown;
	int scene17_sceneOldEdgeX;
	int scene17_flyCountdown;
	AniObject *scene17_hand;
	bool scene17_handPhase;
	int scene17_sceneEdgeX;

	bool scene18_inScene18p1;
	AniObject *scene18_whirlgig;
	Common::Array<Swinger *> scene18_swingers;
	int scene18_wheelCenterX;
	int scene18_wheelCenterY;
	bool scene18_bridgeIsConvoluted;
	int scene18_whirlgigMovMum;
	bool scene18_girlIsSwinging;
	int scene18_rotationCounter;
	int scene18_manY;
	bool scene18_wheelFlipper;
	bool scene18_wheelIsTurning;
	int scene18_kidIsOnWheel;
	int scene18_boyIsOnWheel;
	int scene18_girlIsOnWheel;
	bool scene18_boyJumpedOff;
	int scene18_manWheelPos;
	int scene18_manWheelPosTo;
	int scene18_kidWheelPos;
	int scene18_kidWheelPosTo;
	int scene18_jumpDistance;
	int scene18_jumpAngle;
	bool scene18_manIsReady;
	bool scene18_enteredTrubaRight;
	AniObject *scene18_boy;
	AniObject *scene18_girl;
	AniObject *scene18_domino;
	int scene18_boyJumpX;
	int scene18_boyJumpY;
	int scene18_girlJumpX;
	int scene18_girlJumpY;

	bool scene19_enteredTruba3;

	int scene20_fliesCountdown;
	AniObject *scene20_grandma;

	AniObject *scene21_giraffeBottom;
	int scene21_giraffeBottomX;
	int scene21_giraffeBottomY;
	int scene21_pipeIsOpen;
	double scene21_wigglePos;
	bool scene21_wiggleTrigger;

	AniObject *scene22_bag;
	AniObject *scene22_giraffeMiddle;
	bool scene22_dudeIsOnStool;
	bool scene22_interactionIsDisabled;
	bool scene22_craneIsOut;
	int scene22_numBagFalls;

	AniObject *scene23_calend0;
	AniObject *scene23_calend1;
	AniObject *scene23_calend2;
	AniObject *scene23_calend3;
	bool scene23_topReached;
	bool scene23_isOnStool;
	int scene23_someVar;
	AniObject *scene23_giraffeTop;
	AniObject *scene23_giraffee;

	bool scene24_jetIsOn;
	bool scene24_flowIsLow;
	bool scene24_waterIsOn;
	AniObject *scene24_water;
	AniObject *scene24_jet;
	AniObject *scene24_drop;

	AniObject *scene25_water;
	AniObject *scene25_board;
	AniObject *scene25_drop;
	bool scene25_dudeIsOnBoard;
	bool scene25_waterIsPresent;
	bool scene25_boardIsSelectable;
	bool scene25_beardersAreThere;
	int scene25_beardersCounter;
	Common::Array<AniObject *> scene25_bearders;
	bool scene25_sneezeFlipper;

	AniObject *scene26_chhi;
	AniObject *scene26_drop;
	PictureObject *scene26_sockPic;
	AniObject *scene26_sock;
	AniObject *scene26_activeVent;

	PictureObject *scene27_hitZone;
	AniObject *scene27_driver;
	AniObject *scene27_maid;
	AniObject *scene27_batHandler;
	bool scene27_driverHasVent;
	AniObject *scene27_bat;
	bool scene27_dudeIsAiming;
	bool scene27_maxPhaseReached;
	bool scene27_wipeIsNeeded;
	bool scene27_driverPushedButton;
	int scene27_numLostBats;
	int scene27_knockCount;
	int scene27_aimStartX;
	int scene27_aimStartY;
	int scene27_launchPhase;
	Common::Array<AniObject *> scene27_balls;
	Common::Array<Bat *> scene27_bats;
	Common::Array<Bat *> scene27_var07;

	bool scene28_fliesArePresent;
	bool scene28_beardedDirection;
	PictureObject *scene28_darkeningObject;
	PictureObject *scene28_lighteningObject;
	bool scene28_headDirection;
	bool scene28_headBeardedFlipper;
	bool scene28_lift6inside;

	AniObject *scene29_porter;
	AniObject *scene29_shooter1;
	AniObject *scene29_shooter2;
	AniObject *scene29_ass;
	Common::Array<AniObject *> scene29_greenBalls;
	Common::Array<AniObject *> scene29_redBalls;
	Common::Array<AniObject *> scene29_flyingRedBalls;
	Common::Array<AniObject *> scene29_flyingGreenBalls;
	bool scene29_manIsRiding;
	bool scene29_arcadeIsOn;
	bool scene29_reachedFarRight;
	bool scene29_rideBackEnabled;
	int scene29_shootCountdown;
	int scene29_shootDistance;
	int scene29_manIsHit;
	int scene29_scrollSpeed;
	bool scene29_scrollingDisabled;
	int scene29_hitBall;
	Common::Array<WalkingBearder *> scene29_bearders;
	int scene29_manX;
	int scene29_manY;
	AniHandler scene29_aniHandler;

	AniObject *scene30_leg;
	int scene30_liftFlag;

	int scene31_chantingCountdown;
	AniObject *scene31_cactus;
	AniObject *scene31_plusMinus;

	bool scene32_flagIsWaving;
	bool scene32_flagNeedsStopping;
	bool scene32_dudeIsSitting;
	int scene32_cactusCounter;
	bool scene32_dudeOnLadder;
	bool scene32_cactusIsGrowing;
	AniObject *scene32_flag;
	AniObject *scene32_cactus;
	AniObject *scene32_massOrange;
	AniObject *scene32_massBlue;
	AniObject *scene32_massGreen;
	AniObject *scene32_button;

	AniObject *scene33_mug;
	AniObject *scene33_jettie;
	AniObject *scene33_cube;
	int scene33_cubeX;
	bool scene33_handleIsDown;
	int scene33_ventsX[9];
	int scene33_ventsState[9];

	AniObject *scene34_cactus;
	AniObject *scene34_vent;
	AniObject *scene34_hatch;
	AniObject *scene34_boot;
	bool scene34_dudeClimbed;
	bool scene34_dudeOnBoard;
	bool scene34_dudeOnCactus;
	int scene34_fliesCountdown;

	AniObject *scene35_hose;
	AniObject *scene35_bellyInflater;
	int scene35_flowCounter;
	int scene35_fliesCounter;

	AniObject *scene36_rotohrust;
	AniObject *scene36_scissors;

	Common::Array<Ring *> scene37_rings;
	int scene37_lastDudeX;
	bool scene37_pipeIsOpen;
	AniObject *scene37_plusMinus1;
	AniObject *scene37_plusMinus2;
	AniObject *scene37_plusMinus3;
	int scene37_soundFlipper;
	int scene37_dudeX;

	AniObject *scene38_boss;
	AniObject *scene38_tally;
	AniObject *scene38_shorty;
	AniObject *scene38_domino0;
	AniObject *scene38_dominos;
	AniObject *scene38_domino1;
	AniObject *scene38_bottle;
	int scene38_bossCounter;
	int scene38_lastBossAnim;
	int scene38_bossAnimCounter;
	int scene38_tallyCounter;
	int scene38_lastTallyAnim;
	int scene38_tallyAnimCounter;
	int scene38_shortyCounter;
	int scene38_lastShortyAnim;
	int scene38_shortyAnimCounter;

	int sceneFinal_var01;
	int sceneFinal_var02;
	int sceneFinal_var03;
	bool sceneFinal_trackHasStarted;

	PictureObject *selector;
};

struct Ring {
	AniObject *ani;
	int x;
	int y;
	int numSubRings;
	int subRings[10];
	bool state;

	Ring();
};

} // End of namespace NGI

#endif /* NGI_SCENES_H */
