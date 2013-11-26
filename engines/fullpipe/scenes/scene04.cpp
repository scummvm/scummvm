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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"
#include "fullpipe/utils.h"
#include "fullpipe/gfx.h"
#include "fullpipe/scenes.h"
#include "fullpipe/messages.h"
#include "fullpipe/statics.h"
#include "fullpipe/scene.h"
#include "fullpipe/interaction.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/behavior.h"

namespace Fullpipe {

void scene04_callback(int *param) {
	warning("STUB: scene04_callback");
}

void scene04_initScene(Scene *sc) {
	g_vars->scene04_var01 = 0;
	g_vars->scene04_bottle = sc->getPictureObjectById(PIC_SC4_BOTTLE, 0);
	g_vars->scene04_hand = sc->getStaticANIObject1ById(ANI_HAND, -1);
	g_vars->scene04_plank = sc->getStaticANIObject1ById(ANI_PLANK, -1);
	g_vars->scene04_clock = sc->getStaticANIObject1ById(ANI_CLOCK, -1);
	g_vars->scene04_spring = sc->getStaticANIObject1ById(ANI_SPRING, -1);
	g_vars->scene04_mamasha = sc->getStaticANIObject1ById(ANI_MAMASHA_4, -1);
	g_vars->scene04_boot = sc->getStaticANIObject1ById(ANI_SC4_BOOT, -1);
	g_vars->scene04_ladder = 0;

	StaticANIObject *koz = sc->getStaticANIObject1ById(ANI_KOZAWKA, -1);

	if (koz) {
		Movement *kozmov = koz->getMovementById(MV_KZW_JUMP);
		if (kozmov) {
			uint kozsize = kozmov->_currMovement ? kozmov->_currMovement->_dynamicPhases.size() : kozmov->_dynamicPhases.size();

			for (uint i = 0; i < kozsize; i++) {
				kozmov->setDynamicPhaseIndex(i);

				if (kozmov->_framePosOffsets) {
					g_vars->scene04_jumpingKozyawki[i] = *kozmov->_framePosOffsets[kozmov->_currDynamicPhaseIndex];
				} else {
					kozmov->_somePoint.x = 0;
					kozmov->_somePoint.y = 0;
					g_vars->scene04_jumpingKozyawki[i] = kozmov->_somePoint;
				}
			}
		}

		kozmov = koz->getMovementById(MV_KZW_JUMPROTATE);
		if (kozmov) {
			uint kozsize = kozmov->_currMovement ? kozmov->_currMovement->_dynamicPhases.size() : kozmov->_dynamicPhases.size();

			for (uint i = 0; i < kozsize; i++) {
				kozmov->setDynamicPhaseIndex(i);

				if (kozmov->_framePosOffsets) {
					g_vars->scene04_jumpRotateKozyawki[i] = *kozmov->_framePosOffsets[kozmov->_currDynamicPhaseIndex];
				} else {
					kozmov->_somePoint.x = 0;
					kozmov->_somePoint.y = 0;
					g_vars->scene04_jumpRotateKozyawki[i] = kozmov->_somePoint;
				}
			}
		}
	}

	Interaction *plank = getGameLoaderInteractionController()->getInteractionByObjectIds(ANI_PLANK, 0, 0);
	if (plank)
		plank->_flags |= 8;

	if (g_fullpipe->getObjectState(sO_Jar_4) == g_fullpipe->getObjectEnumState(sO_Jar_4, sO_UpsideDown)) {
		g_vars->scene04_bottleObjList.clear();
		g_vars->scene04_kozyawkiObjList.clear();

		sc->getPictureObjectById(PIC_SC4_BOTTLE, 0)->_flags &= 0xfffb;
		sc->getPictureObjectById(PIC_SC4_MASK, 0)->_flags &= 0xfffb;
		sc->getStaticANIObject1ById(ANI_SPRING, 0)->_flags &= 0xfffb;

		g_vars->scene04_var18 = 0;
		g_vars->scene04_var19 = 0;
	} else {
		StaticANIObject *spring = sc->getStaticANIObject1ById(ANI_SPRING, -1);

		if (spring)
			spring->_callback2 = 0;

		g_vars->scene04_bottleObjList.clear();
		g_vars->scene04_bottleObjList.push_back(sc->getPictureObjectById(PIC_SC4_BOTTLE, 0));
		g_vars->scene04_bottleObjList.push_back(sc->getPictureObjectById(PIC_SC4_MASK, 0));

		g_vars->scene04_kozyawkiObjList.clear();

		if (koz) {
			koz->loadMovementsPixelData();

			koz->_statics = koz->getStaticsById(ST_KZW_EMPTY);
			koz->setOXY(0, 0);
			koz->hide();

			g_vars->scene04_kozyawkiObjList.push_back(koz);

			for (int i = 0; i < 6; i++) {
				StaticANIObject *koz1 = new StaticANIObject(koz);

				sc->addStaticANIObject(koz1, 1);
				koz1->_statics = koz->getStaticsById(ST_KZW_EMPTY);
				koz1->setOXY(0, 0);
				koz1->hide();
				g_vars->scene04_kozyawkiObjList.push_back(koz1);
			}
		}
		sc->getPictureObjectById(PIC_SC4_BOTTLE2, 0)->_flags &= 0xfffb;

		g_vars->scene04_var18 = 1;
		g_vars->scene04_var19 = 1;
	}

	g_vars->scene04_var02 = 0;
	g_vars->scene04_soundPlaying = 0;
	g_vars->scene04_var04 = 0;
	g_vars->scene04_var05 = 0;
	g_vars->scene04_var06 = 2;
	g_vars->scene04_dynamicPhaseIndex = 0;

	g_vars->scene04_kozyawkiAni.clear();

	g_fullpipe->setObjectState(sO_LowerPipe, g_fullpipe->getObjectEnumState(sO_LowerPipe, sO_IsClosed));

	g_vars->scene04_var07 = 0;
	g_vars->scene04_var08 = 0;
	g_vars->scene04_coinPut = 0;
	g_vars->scene04_var09 = 0;
	g_vars->scene04_var10 = 0;
	g_vars->scene04_var11 = 0;
	g_vars->scene04_var12 = 0;
	g_vars->scene04_var13 = 1;
	g_vars->scene04_var14 = 0;
	g_vars->scene04_var15 = 1;

	if (g_fullpipe->getObjectState(sO_BigMumsy) != g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_Gone))
		g_vars->scene04_mamasha->hide();

	g_vars->scene04_speaker = sc->getStaticANIObject1ById(ANI_SPEAKER_4, -1);
	g_vars->scene04_speaker->_callback2 = scene04_callback;
	g_vars->scene04_speaker->startAnim(MV_SPK4_PLAY, 0, -1);

	g_vars->scene04_var16 = 0;
	g_vars->scene04_var17 = 0;

	g_fullpipe->initArcadeKeys("SC_4");
}

bool sceneHandler04_friesAreWalking() {
	warning("STUB: sceneHandler04_friesAreWalking()");

	return false;
}

int scene04_updateCursor() {
	g_fullpipe->updateCursorCommon();

	if (g_fullpipe->_objectIdAtCursor == PIC_SC4_LRTRUBA) {
		if (!g_vars->scene04_var19) {
			g_fullpipe->_cursorId = PIC_CSR_DEFAULT;

			return g_fullpipe->_cursorId;
		}
	} else if (g_fullpipe->_objectIdAtCursor == ANI_PLANK || g_fullpipe->_objectIdAtCursor == PIC_SC4_PLANK) {
		if (g_fullpipe->_objectIdAtCursor == ANI_PLANK && g_fullpipe->_cursorId != PIC_CSR_ITN)
			return g_fullpipe->_cursorId;

		if (g_fullpipe->_objectIdAtCursor == ANI_PLANK || (g_fullpipe->_objectIdAtCursor == PIC_SC4_PLANK && g_fullpipe->_cursorId == PIC_CSR_DEFAULT)) {
			if (sceneHandler04_friesAreWalking()) {
				g_fullpipe->_cursorId = PIC_CSR_ARCADE1;
				return g_fullpipe->_cursorId;
			}
			if (g_vars->scene04_soundPlaying) {
				g_fullpipe->_cursorId = PIC_CSR_DEFAULT;
				return g_fullpipe->_cursorId;
			}
		}
	}

	if (g_fullpipe->_objectIdAtCursor == PIC_CSR_ITN && g_fullpipe->_objectIdAtCursor == PIC_SC4_DOWNTRUBA)
		g_fullpipe->_cursorId = PIC_CSR_GOD;

	return g_fullpipe->_cursorId;
}

void sceneHandlers_sub01(ExCommand *ex) {
	warning("sceneHandlers_sub01()");
}

void sceneHandler04_checkBigBallClick() {
	StaticANIObject *ball = g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_BIGBALL, -1);

	if (ball)
		for (uint i = 0; i < ball->_movements.size(); i++)
			((Movement *)ball->_movements[i])->_counterMax = 73;

	g_vars->scene04_var13 = 1;
}

void sceneHandler04_clickBottle() {
	if (!g_vars->scene04_var02)
		g_vars->scene04_var20 += 5;
}

void sceneHandler04_clickButton() {
	StaticANIObject *but = g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_BUTTON, -1);

	if (but) {
		if (!g_vars->scene04_clock->_movement || 
			(g_vars->scene04_clock->_movement->_id == MV_CLK_GO && g_vars->scene04_clock->_movement->_currDynamicPhaseIndex > 3 && 
			 g_vars->scene04_clock->_movement->_currDynamicPhaseIndex < 105)) {
			if (!g_vars->scene04_hand->_movement && !g_vars->scene04_var02) {
				but->startAnim(MV_BTN_CLICK, 0, -1);
				g_vars->scene04_hand->startAnim(MV_HND_POINT, 0, -1);
			}
		}
	}
}

void sceneHandler04_clickLadder() {
	warning("sceneHandler04_clickLadder()");
}

void sceneHandler04_sub13() {
	warning("sceneHandler04_sub13()");
}

void sceneHandler04_clickPlank() {
	if (sceneHandler04_friesAreWalking())
		sceneHandler04_sub13();
	else if (g_vars->scene04_var01)
		g_fullpipe->playSound(SND_4_033, 0);
	else if (!g_vars->scene04_soundPlaying)
		chainQueue(QU_PNK_CLICK, 0);
}

void sceneHandler04_dropBottle() {
	warning("sceneHandler04_dropBottle()");
}

void sceneHandler04_gotoLadder(int par) {
	warning("sceneHandler04_gotoLadder()");
}

void sceneHandler04_lowerPlank() {
	g_vars->scene04_plank->startAnim(MV_PNK_WEIGHTRIGHT, 0, -1);
}

void sceneHandler04_manFromBottle() {
	warning("sceneHandler04_manFromBottle()");
}

void sceneHandler04_manToBottle() {
	g_vars->scene04_bottleObjList.push_back(g_fullpipe->_aniMan);
	g_vars->scene04_var20 = 5;
	g_vars->scene04_var06 += 9;
	g_fullpipe->_aniMan2 = g_fullpipe->_aniMan;
	g_vars->scene04_var10 = 1;
}

void sceneHandler04_raisePlank() {
	g_vars->scene04_plank->startAnim(MV_PNK_WEIGHTLEFT, 0, -1);
}

void sceneHandler04_shootKozyawka() {
	warning("sceneHandler04_shootKozyawka()");
}

void sceneHandler04_showCoin() {
	StaticANIObject *ani = g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_SC4_COIN, -1);

	if (ani) {
		ani->show1(MV_BDG_OPEN, MV_MAN_GOU, MV_SC4_COIN_default, 0);

		ani->_priority = 40;
	}
}

void sceneHandler04_stopSound() {
	warning("sceneHandler04_stopSound()");
}

void sceneHandler04_sub1(ExCommand *ex) {
	g_fullpipe->_aniMan->changeStatics2(ST_MAN_SIT);

	MessageQueue *mq = new MessageQueue(g_fullpipe->_currentScene->getMessageQueueById(QU_SC4_MANFROMBOTTLE), 0, 0);

	if (ex) {
		ExCommand *newex = new ExCommand(ex);

		mq->_exCommands.push_back(newex);
	  }

	mq->_flags |= 1;
	mq->chain(0);

	g_vars->scene04_var10 = 0;
	g_fullpipe->_behaviorManager->setFlagByStaticAniObject(g_fullpipe->_aniMan, 1);
}

void sceneHandler04_sub3() {
	warning("sceneHandler04_sub3()");
}

void sceneHandler04_sub4() {
	warning("sceneHandler04_sub4()");
}

void sceneHandler04_sub5() {
	warning("sceneHandler04_sub5()");
}

void sceneHandler04_sub6() {
	warning("sceneHandler04_sub6()");
}

void sceneHandler04_sub7() {
	warning("sceneHandler04_sub7()");
}

void sceneHandler04_sub8(ExCommand *ex) {
	warning("sceneHandler04_sub8()");
}

void sceneHandler04_sub9(StaticANIObject *ani) {
	warning("sceneHandler04_sub9()");
}

void sceneHandler04_sub15() {
	warning("sceneHandler04_sub15()");
}

void sceneHandler04_sub17() {
	warning("sceneHandler04_sub17()");
}

void sceneHandler04_takeBottle() {
	warning("sceneHandler04_takeBottle()");
}

void sceneHandler04_takeKozyawka() {
	warning("sceneHandler04_takeKozyawka()");
}

void sceneHandler04_testPlank(ExCommand *ex) {
	warning("sceneHandler04_testPlank()");
}

void sceneHandler04_bottleUpdateObjects(int off) {
	warning("sceneHandler04_bottleUpdateObjects()");
}

void sceneHandler04_updateBottle() {
	Common::Point point;

	int yoff;

	if (g_vars->scene04_hand->_movement)
		yoff = g_vars->scene04_hand->_movement->_oy;
	else
		yoff = g_vars->scene04_hand->_oy;

	int newy = g_vars->scene04_hand->getSomeXY(point)->y + yoff + 140;

	sceneHandler04_bottleUpdateObjects(newy - g_vars->scene04_spring->_oy);

	g_vars->scene04_spring->setOXY(g_vars->scene04_spring->_ox, newy);
}

void sceneHandler04_winArcade() {
	warning("sceneHandler04_winArcade()");
}

int sceneHandler04(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch (ex->_messageNum) {
	case MSG_UPDATEBOTTLE:
		sceneHandler04_updateBottle();
		break;
		
	case MSG_CLICKBOTTLE:
		sceneHandler04_clickBottle();
		break;

	case MSG_SHOOTKOZAW:
		sceneHandler04_shootKozyawka();
		break;

	case MSG_SHAKEBOTTLE:
		if (!g_vars->scene04_var02)
			++g_vars->scene04_var20;
		break;

	case MSG_STARTHAND:
		g_vars->scene04_var09 = 1;
		g_vars->scene04_coinPut = 0;

		if (g_vars->scene04_var10)
			sceneHandler04_sub1(0);

		sceneHandler04_sub15();
		sceneHandler04_stopSound();
		break;

	case MSG_TAKEKOZAW:
		sceneHandler04_takeKozyawka();
		break;

	case MSG_CLICKBUTTON:
		sceneHandler04_clickButton();
		break;

	case MSG_CLICKPLANK:
		sceneHandler04_clickPlank();
		break;

	case MSG_RAISEPLANK:
		sceneHandler04_raisePlank();
		break;

	case MSG_KOZAWRESTART:
		if (g_vars->scene04_var05) {
			g_vars->scene04_kozyawkiObjList.push_back(g_vars->scene04_var05);
			g_vars->scene04_var05->hide();
			g_vars->scene04_var05 = 0;
		}
		if (g_vars->scene04_soundPlaying)
			sceneHandler04_sub3();

		break;

	case MSG_LOWERPLANK:
		sceneHandler04_lowerPlank();
		break;

	case MSG_TESTPLANK:
		sceneHandler04_testPlank(ex);
		break;

	case 33:
		{
			g_vars->scene04_dudePosX = g_fullpipe->_aniMan->_ox;
			g_vars->scene04_dudePosY = g_fullpipe->_aniMan->_oy;

			int res = 0;

			if (g_fullpipe->_aniMan2) {
				if (g_fullpipe->_aniMan->_ox < g_fullpipe->_sceneRect.left + 200) {
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan->_ox - g_fullpipe->_sceneRect.left - 300;
					g_fullpipe->_aniMan->_ox = g_vars->scene04_dudePosX;
				}
				if (g_fullpipe->_aniMan->_ox > g_fullpipe->_sceneRect.right - 200) {
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan->_ox - g_fullpipe->_sceneRect.right + 300;
				}

				res = 1;

				if (g_vars->scene04_soundPlaying) {
					if (g_fullpipe->_aniMan->_movement) {
						if (g_fullpipe->_aniMan->_movement->_id == MV_MAN_TOLADDER) {
							g_fullpipe->_aniMan2 = 0;

							if (g_fullpipe->_sceneRect.left > 380)
								g_fullpipe->_currentScene->_x = 380 - g_fullpipe->_sceneRect.left;
						}
					}
				}
			} else {
				if (g_fullpipe->_aniMan->_movement && g_fullpipe->_aniMan->_movement->_id == MV_MAN_GOD)
					g_fullpipe->_aniMan2 = g_fullpipe->_aniMan;
			}

			sceneHandler04_sub4();

			if (g_vars->scene04_var07 && !g_vars->scene04_var09)
				sceneHandler04_sub5();

			if (g_vars->scene04_var12)
				sceneHandler04_sub6();

			if (g_vars->scene04_var08)
				sceneHandler04_clickLadder();

			if (g_vars->scene04_var10 && g_vars->scene04_hand->_movement)
				sceneHandler04_sub1(0);

			if (g_vars->scene04_coinPut && g_vars->scene04_var18 && !g_vars->scene04_var09 && !g_vars->scene04_soundPlaying)
				sceneHandler04_sub7();

			if (g_vars->scene04_var01) {
				if (!g_vars->scene04_soundPlaying) {
					g_fullpipe->startSceneTrack();

					g_fullpipe->_behaviorManager->updateBehaviors();
					return res;
				}

				g_vars->scene04_var14++;

				if (g_vars->scene04_var14 > 600)
					sceneHandler04_sub17();
			}

			if (g_vars->scene04_soundPlaying) {
				g_fullpipe->_behaviorManager->updateBehaviors();

				return res;
			}

			g_fullpipe->startSceneTrack();

			g_fullpipe->_behaviorManager->updateBehaviors();

			return res;
		}

	case 29:
		{
			int picid = g_fullpipe->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if (g_vars->scene04_var10) {
				sceneHandler04_sub1(ex);

				break;
			}

			if (picid == PIC_SC4_LADDER) {
				if (!g_vars->scene04_var04) {
					g_vars->scene04_sceneClickX = ex->_sceneClickX;
					g_vars->scene04_sceneClickY = ex->_sceneClickY;

					sceneHandler04_clickLadder();

					ex->_messageKind = 0;

					break;
				}

				sceneHandler04_gotoLadder(0);

				break;
			}

			StaticANIObject *ani = g_fullpipe->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if ((ani && ani->_id == ANI_PLANK) || picid == PIC_SC4_PLANK) {
				sceneHandler04_clickPlank();

				ex->_messageKind = 0;
			} else if (g_vars->scene04_var01) {
				sceneHandler04_sub8(ex);
			} else if (!ani || !canInteractAny(g_fullpipe->_aniMan, ani, ex->_keyCode)) {
				PictureObject *pic = g_fullpipe->_currentScene->getPictureObjectById(picid, 0);

				if (!pic || !canInteractAny(g_fullpipe->_aniMan, pic,ex->_keyCode)) {
					if ((g_fullpipe->_sceneRect.right - ex->_sceneClickX < 47 && g_fullpipe->_sceneRect.right < g_fullpipe->_sceneWidth - 1)
						|| (ex->_sceneClickX - g_fullpipe->_sceneRect.left < 47 && g_fullpipe->_sceneRect.left > 0))
						sceneHandlers_sub01(ex);
				}
			}
		}

		break;

	case MSG_SC4_HIDEBOOT:
		g_vars->scene04_boot->_flags &= 0xfffb;
		break;

	case MSG_CMN_WINARCADE:
		sceneHandler04_winArcade();
		break;

	case MSG_SC4_HANDOVER:
		g_vars->scene04_var09 = 0;
		g_vars->scene04_var19 = 1;
		break;

	case MSG_SC4_DROPBOTTLE:
		sceneHandler04_dropBottle();
		break;
		
	case MSG_SC4_COINOUT:
		g_vars->scene04_clock->changeStatics2(ST_CLK_CLOSED);
		g_vars->scene04_coinPut = 0;
		sceneHandler04_stopSound();

		if (g_vars->scene04_kozyawkiAni.size() && !g_vars->scene04_var02) {
			g_vars->scene04_var09 = 1;

			if (g_vars->scene04_var10)
				sceneHandler04_sub1(0);

			sceneHandler04_sub15();
		}

		break;

	case MSG_SC4_KOZAWFALL:
		{
			ExCommand *exnew;

			if (g_vars->scene04_var11) {
				sceneHandler04_sub9(g_vars->scene04_var24);

				g_vars->scene04_var11 = 0;

				exnew = new ExCommand(0, 35, SND_4_010, 0, 0, 0, 1, 0, 0, 0);
			} else {
				exnew = new ExCommand(0, 35, SND_4_012, 0, 0, 0, 1, 0, 0, 0);
			}

			exnew->_field_14 = 5;
			exnew->_excFlags |= 2;
			exnew->postMessage();
			break;
		}

	case MSG_SC4_MANFROMBOTTLE:
		sceneHandler04_manFromBottle();
		break;

	case MSG_SC4_CLICKLADDER:
		sceneHandler04_clickLadder();
		break;

	case MSG_SC4_MANTOBOTTLE:
		sceneHandler04_manToBottle();
		break;

	case MSG_SHOWCOIN:
		sceneHandler04_showCoin();
		break;

	case MSG_TAKEBOTTLE:
		sceneHandler04_takeBottle();
		break;

	case MSG_GOTOLADDER:
		sceneHandler04_gotoLadder(0);
		break;

	case MSG_SC4_COINPUT:
		g_vars->scene04_coinPut = 1;
		break;
	}

	return 0;
}

} // End of namespace Fullpipe
