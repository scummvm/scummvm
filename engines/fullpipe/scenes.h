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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef FULLPIPE_SCENES_H
#define FULLPIPE_SCENES_H

namespace Fullpipe {

struct BehaviorEntryInfo;
class MGM;
class MctlLadder;
struct Ring;
class StaticANIObject;

int defaultUpdateCursor();

int sceneIntro_updateCursor();
void sceneIntro_initScene(Scene *sc);
int sceneHandlerIntro(ExCommand *cmd);

void scene01_fixEntrance();
void scene01_initScene(Scene *sc, int entrance);
int sceneHandler01(ExCommand *cmd);

void scene02_initScene(Scene *sc);
int sceneHandler02(ExCommand *ex);

void scene03_setEaterState();
int scene03_updateCursor();
void scene03_initScene(Scene *sc);
int sceneHandler03(ExCommand *cmd);

int scene04_updateCursor();
void scene04_initScene(Scene *sc);
int sceneHandler04(ExCommand *cmd);

void scene05_initScene(Scene *sc);
int sceneHandler05(ExCommand *cmd);

void scene06_initScene(Scene *sc);
void scene06_initMumsy();
int sceneHandler06(ExCommand *cmd);
int scene06_updateCursor();

void scene07_initScene(Scene *sc);
int sceneHandler07(ExCommand *cmd);

void scene08_initScene(Scene *sc);
void scene08_setupMusic();
int sceneHandler08(ExCommand *cmd);
int scene08_updateCursor();

void scene10_initScene(Scene *sc);
int sceneHandler10(ExCommand *cmd);
int scene10_updateCursor();

void scene11_initScene(Scene *sc);
void scene11_setupMusic();
int sceneHandler11(ExCommand *cmd);
int scene11_updateCursor();

void scene12_initScene(Scene *sc);
int sceneHandler12(ExCommand *ex);

void scene13_initScene(Scene *sc);
int sceneHandler13(ExCommand *ex);

void scene14_initScene(Scene *sc);
void scene14_setupMusic();
int sceneHandler14(ExCommand *cmd);
int scene14_updateCursor();

int scene15_updateCursor();
void scene15_initScene(Scene *sc);
int sceneHandler15(ExCommand *cmd);

void scene16_initScene(Scene *sc);
int sceneHandler16(ExCommand *cmd);
int scene16_updateCursor();

void scene17_initScene(Scene *sc);
void scene17_restoreState();
int sceneHandler17(ExCommand *cmd);
int scene17_updateCursor();

void scene18_preload();
void scene19_preload(Scene *sc, int key);

void scene20_initScene(Scene *sc);
int sceneHandler20(ExCommand *ex);

int scene21_updateCursor();
void scene21_initScene(Scene *sc);
int sceneHandler21(ExCommand *cmd);

void scene22_initScene(Scene *sc);
void scene22_setBagState();
int sceneHandler22(ExCommand *cmd);
int scene22_updateCursor();

void scene23_initScene(Scene *sc);
void scene23_setGiraffeState();
int sceneHandler23(ExCommand *cmd);
int scene23_updateCursor();

void scene24_initScene(Scene *sc);
void scene24_setPoolState();
int sceneHandler24(ExCommand *cmd);

void scene25_initScene(Scene *sc, int entrance);
void scene25_setupWater(Scene *sc, int entrance);
int sceneHandler25(ExCommand *cmd);
int scene25_updateCursor();

void scene26_initScene(Scene *sc);
void scene26_setupDrop(Scene *sc);
int sceneHandler26(ExCommand *cmd);
int scene26_updateCursor();

void scene28_initScene(Scene *sc);
 int sceneHandler28(ExCommand *ex);
int scene28_updateCursor();

int scene30_updateCursor();
void scene30_initScene(Scene *sc, int flag);
int sceneHandler30(ExCommand *cmd);

void scene31_initScene(Scene *sc);
int sceneHandler31(ExCommand *ex);

void scene32_initScene(Scene *sc);
void scene32_setupMusic();
int sceneHandler32(ExCommand *cmd);
int scene32_updateCursor();

void scene33_initScene(Scene *sc);
void scene33_setupMusic();
int sceneHandler33(ExCommand *cmd);
int scene33_updateCursor();

void scene34_initScene(Scene *sc);
void scene34_initBeh();
int sceneHandler34(ExCommand *cmd);
int scene34_updateCursor();

void scene35_initScene(Scene *sc);
int sceneHandler35(ExCommand *cmd);

int scene36_updateCursor();
void scene36_initScene(Scene *sc);
int sceneHandler36(ExCommand *cmd);

void scene37_initScene(Scene *sc);
int sceneHandler37(ExCommand *ex);
int scene37_updateCursor();

void sceneDbgMenu_initScene(Scene *sc);
int sceneHandlerDbgMenu(ExCommand *cmd);

class Vars {
public:
	Vars();

	GameVar *swallowedEgg1;
	GameVar *swallowedEgg2;
	GameVar *swallowedEgg3;

	StaticANIObject *sceneIntro_aniin1man;
	bool sceneIntro_needSleep;
	bool sceneIntro_needGetup;
	bool sceneIntro_skipIntro;
	bool sceneIntro_playing;
	bool sceneIntro_needBlackout;

	PictureObject *scene01_picSc01Osk;
	PictureObject *scene01_picSc01Osk2;

	StaticANIObject *scene02_guvTheDrawer;
	int scene02_boxDelay;
	bool scene02_boxOpen;

	StaticANIObject *scene03_eggeater;
	StaticANIObject *scene03_domino;

	PictureObject *scene04_bottle;
	StaticANIObject *scene04_hand;
	StaticANIObject *scene04_plank;
	StaticANIObject *scene04_clock;
	StaticANIObject *scene04_spring;
	StaticANIObject *scene04_mamasha;
	StaticANIObject *scene04_boot;
	StaticANIObject *scene04_speaker;

	Common::Point scene04_jumpingKozyawki[20];
	Common::Point scene04_jumpRotateKozyawki[20];

	Common::List<StaticANIObject *> scene04_kozyawkiObjList;
	Common::List<GameObject *> scene04_bottleObjList;
	Common::List<StaticANIObject *> scene04_kozyawkiAni;

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

	StaticANIObject *scene04_walkingKozyawka;

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
	StaticANIObject *scene04_lastKozyawka;
	int scene04_springDelay;

	StaticANIObject *scene05_handle;
	StaticANIObject *scene05_wacko;
	StaticANIObject *scene05_bigHatch;
	int scene05_wackoTicker;
	int scene05_handleFlipper;
	int scene05_floatersTicker;

	StaticANIObject *scene06_mumsy;
	int scene06_manX;
	int scene06_manY;
	int scene06_ballX;
	int scene06_ballY;
	StaticANIObject *scene06_someBall;
	StaticANIObject *scene06_invHandle;
	StaticANIObject *scene06_liftButton;
	StaticANIObject *scene06_ballDrop;
	bool scene06_arcadeEnabled;
	bool scene06_aimingBall;
	StaticANIObject *scene06_currentBall;
	StaticANIObject *scene06_ballInHands;
	StaticANIObject *scene06_flyingBall;
	Common::Array<StaticANIObject *> scene06_balls;
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
	BehaviorEntryInfo *scene06_mumsyJumpBk;
	BehaviorEntryInfo *scene06_mumsyJumpFw;
	int scene06_mumsyJumpBkPercent;
	int scene06_mumsyJumpFwPercent;

	BehaviorEntryInfo *scene07_lukeAnim;
	int scene07_lukePercent;
	StaticANIObject *scene07_plusMinus;

	StaticANIObject *scene08_batuta;
	StaticANIObject *scene08_vmyats;
	StaticANIObject *scene08_clock;
	bool scene08_inAir;
	bool scene08_flyingUp;
	int scene08_onBelly;
	int scene08_stairsOffset;
	int scene08_snoringCountdown;
	bool scene08_inArcade;
	bool scene08_stairsVisible;
	int scene08_manOffsetY;

	StaticANIObject *scene10_gum;
	StaticANIObject *scene10_packet;
	StaticANIObject *scene10_packet2;
	StaticANIObject *scene10_inflater;
	PictureObject *scene10_ladder;
	int scene10_hasGum;

	StaticANIObject *scene11_swingie;
	StaticANIObject *scene11_boots;
	StaticANIObject *scene11_dudeOnSwing;
	PictureObject *scene11_hint;
	MGM scene11_mgm;
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

	StaticANIObject *scene13_whirlgig;
	StaticANIObject *scene13_guard;
	StaticANIObject *scene13_handleR;
	StaticANIObject *scene13_handleL;
	StaticANIObject *scene13_bridge;
	bool scene13_guardDirection;
	int scene13_dudeX;

	StaticANIObject *scene14_grandma;
	int scene14_sceneDeltaX;
	int scene14_sceneDeltaY;
	bool scene14_arcadeIsOn;
	bool scene14_dudeIsKicking;
	bool scene14_ballIsFlying;
	bool scene14_dudeCanKick;
	int scene14_sceneDiffX;
	int scene14_sceneDiffY;
	StaticANIObject *scene14_pink;
	StaticANIObject *scene14_flyingBall;
	Common::List<StaticANIObject *> scene14_balls;
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
	StaticANIObject *scene15_plusminus;
	PictureObject *scene15_ladder;
	StaticANIObject *scene15_boot;

	Common::List<StaticANIObject *> scene16_figures;
	StaticANIObject *scene16_walkingBoy;
	StaticANIObject *scene16_walkingGirl;
	int scene16_walkingCount;
	StaticANIObject *scene16_wire;
	StaticANIObject *scene16_mug;
	StaticANIObject *scene16_jettie;
	StaticANIObject *scene16_boot;
	bool scene16_girlIsLaughing;
	int scene16_sound;
	bool scene16_placeIsOccupied;

	int scene17_flyState;
	bool scene17_sugarIsShown;
	int scene17_sceneOldEdgeX;
	int scene17_flyCountdown;
	StaticANIObject *scene17_hand;
	bool scene17_handPhase;
	int scene17_sceneEdgeX;

	int scene18_var01;

	int scene20_fliesCountdown;
	StaticANIObject *scene20_grandma;

	StaticANIObject *scene21_giraffeBottom;
	int scene21_giraffeBottomX;
	int scene21_giraffeBottomY;
	int scene21_pipeIsOpen;
	double scene21_wigglePos;
	bool scene21_wiggleTrigger;

	StaticANIObject *scene22_bag;
	StaticANIObject *scene22_giraffeMiddle;
	bool scene22_dudeIsOnStool;
	bool scene22_interactionIsDisabled;
	bool scene22_craneIsOut;
	int scene22_numBagFalls;

	StaticANIObject *scene23_calend0;
	StaticANIObject *scene23_calend1;
	StaticANIObject *scene23_calend2;
	StaticANIObject *scene23_calend3;
	bool scene23_topReached;
	bool scene23_isOnStool;
	int scene23_someVar;
	StaticANIObject *scene23_giraffeTop;
	StaticANIObject *scene23_giraffee;

	bool scene24_jetIsOn;
	bool scene24_flowIsLow;
	bool scene24_waterIsOn;
	StaticANIObject *scene24_water;
	StaticANIObject *scene24_jet;
	StaticANIObject *scene24_drop;

	StaticANIObject *scene25_water;
	StaticANIObject *scene25_board;
	StaticANIObject *scene25_drop;
	bool scene25_dudeIsOnBoard;
	bool scene25_waterIsPresent;
	bool scene25_boardIsSelectable;
	bool scene25_beardersAreThere;
	int scene25_beardersCounter;
	Common::Array<StaticANIObject *> scene25_bearders;
	bool scene25_sneezeFlipper;

	StaticANIObject *scene26_chhi;
	StaticANIObject *scene26_drop;
	PictureObject *scene26_sockPic;
	StaticANIObject *scene26_sock;
	StaticANIObject *scene26_activeVent;

	bool scene28_fliesArePresent;
	bool scene28_beardedDirection;
	PictureObject *scene28_darkeningObject;
	PictureObject *scene28_lighteningObject;
	bool scene28_headDirection;
	bool scene28_headBeardedFlipper;
	bool scene28_lift6inside;

	StaticANIObject *scene30_leg;
	int scene30_liftFlag;

	int scene31_chantingCountdown;
	StaticANIObject *scene31_cactus;
	StaticANIObject *scene31_plusMinus;

	bool scene32_flagIsWaving;
	bool scene32_flagNeedsStopping;
	bool scene32_dudeIsSitting;
	int scene32_cactusCounter;
	bool scene32_dudeOnLadder;
	bool scene32_cactusIsGrowing;
	StaticANIObject *scene32_flag;
	StaticANIObject *scene32_cactus;
	StaticANIObject *scene32_massOrange;
	StaticANIObject *scene32_massBlue;
	StaticANIObject *scene32_massGreen;
	StaticANIObject *scene32_button;

	StaticANIObject *scene33_mug;
	StaticANIObject *scene33_jettie;
	StaticANIObject *scene33_cube;
	int scene33_cubeX;
	bool scene33_handleIsDown;
	int scene33_ventsX[9];
	int scene33_ventsState[9];

	StaticANIObject *scene34_cactus;
	StaticANIObject *scene34_vent;
	StaticANIObject *scene34_hatch;
	StaticANIObject *scene34_boot;
	bool scene34_dudeClimbed;
	bool scene34_dudeOnBoard;
	bool scene34_dudeOnCactus;
	int scene34_fliesCountdown;

	StaticANIObject *scene35_hose;
	StaticANIObject *scene35_bellyInflater;
	int scene35_flowCounter;
	int scene35_fliesCounter;

	StaticANIObject *scene36_rotohrust;
	StaticANIObject *scene36_scissors;

	Common::Array<Ring *> scene37_rings;
	int scene37_lastDudeX;
	bool scene37_cursorIsLocked;
	StaticANIObject *scene37_plusMinus1;
	StaticANIObject *scene37_plusMinus2;
	StaticANIObject *scene37_plusMinus3;
	int scene37_soundFlipper;
	int scene37_dudeX;

	int scene38_var01;
	int scene38_var02;
	int scene38_var03;
	int scene38_var04;
	StaticANIObject *scene38_boss;
	StaticANIObject *scene38_tally;
	StaticANIObject *scene38_shorty;
	StaticANIObject *scene38_domino0;
	StaticANIObject *scene38_dominos;
	StaticANIObject *scene38_domino1;
	StaticANIObject *scene38_bottle;
	int scene38_var05;
	int scene38_var06;
	int scene38_var07;
	int scene38_var08;
	int scene38_var09;
	int scene38_var10;
	int scene38_var11;
	int scene38_var12;
	int scene38_var13;

	PictureObject *selector;
};

struct Ring {
	StaticANIObject *ani;
	int x;
	int y;
	int numSubRings;
	int subRings[10];
	bool state;

	Ring();
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_SCENES_H */
