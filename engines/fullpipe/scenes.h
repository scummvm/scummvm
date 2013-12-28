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
class StaticANIObject;
class MctlLadder;
class MGM;

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

int scene15_updateCursor();
void scene15_initScene(Scene *sc);
int sceneHandler15(ExCommand *cmd);

void scene24_initScene(Scene *sc);
void scene24_setPoolState();
int sceneHandler24(ExCommand *cmd);

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

	int scene15_chantingCountdown;
	StaticANIObject *scene15_plusminus;
	PictureObject *scene15_ladder;
	StaticANIObject *scene15_boot;

	bool scene24_jetIsOn;
	bool scene24_flowIsLow;
	bool scene24_waterIsOn;
	StaticANIObject *scene24_water;
	StaticANIObject *scene24_jet;
	StaticANIObject *scene24_drop;

	PictureObject *selector;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_SCENES_H */
