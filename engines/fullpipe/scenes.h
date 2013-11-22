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

	Common::Array<StaticANIObject *> scene04_kozyawkiObjList;
	Common::Array<GameObject *> scene04_bottleObjList;
	Common::Array<StaticANIObject *> scene04_kozyawkiAni;

	int scene04_ladder;
	bool scene04_coinPut;
	bool scene04_soundPlaying;
	int scene04_dynamicPhaseIndex;
	int scene04_sceneClickX;
	int scene04_sceneClickY;
	int scene04_dudePosX;
	int scene04_dudePosY;

	int scene04_var01;
	int scene04_var02;
	int scene04_var04;
	StaticANIObject *scene04_var05;
	int scene04_var06;
	int scene04_var07;
	int scene04_var08;
	int scene04_var09;
	int scene04_var10;
	int scene04_var11;
	int scene04_var12;
	int scene04_var13;
	int scene04_var14;
	int scene04_var15;
	int scene04_var16;
	int scene04_var17;
	int scene04_var18;
	int scene04_var19;
	int scene04_var20;
	StaticANIObject *scene04_var24;

	PictureObject *selector;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_SCENES_H */
