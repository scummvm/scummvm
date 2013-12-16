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
	int scene06_var01;
	int scene06_var02;
	int scene06_var03;
	int scene06_var04;
	int scene06_var06;
	StaticANIObject *scene06_invHandle;
	StaticANIObject *scene06_liftButton;
	StaticANIObject *scene06_ballDrop;
	int scene06_var07;
	int scene06_var08;
	StaticANIObject *scene06_var09;
	int scene06_var10;
	StaticANIObject *scene06_var11;
	Common::Array<StaticANIObject *> scene06_balls;
	int scene06_var12;
	int scene06_var13;
	int scene06_var14;
	int scene06_var15;
	int scene06_var16;
	int scene06_var17;
	int scene06_var18;
	int scene06_sceneClickX;
	int scene06_sceneClickY;
	int scene06_mumsyPos;
	BehaviorEntryInfo *scene06_mumsyJumpBk;
	BehaviorEntryInfo *scene06_mumsyJumpFw;
	int scene06_mumsyJumpBkPercent;
	int scene06_mumsyJumpFwPercent;

	PictureObject *selector;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_SCENES_H */
