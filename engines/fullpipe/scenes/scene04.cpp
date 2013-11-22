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

int sceneHandler04(ExCommand *ex) {
#if 0
	if (ex->_messageKind != 17)
		return 0;

	switch (ex->_messageNum) {
	case MSG_UPDATEBOTTLE:
		sceneHandler04_msgUpdateBottle();
		break;
		
	case MSG_CLICKBOTTLE:
		sceneHandler04_clickBottle();
		break;

	case MSG_SHOOTKOZAW:
		sceneHandler04_chootKozyawka();
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
		sceneHandler04_testPlank((int)ex);
		break;

	case 33:
		g_vars->scene04_dudePosX = g_fullpipe->_aniMan->_ox;
		g_vars->scene04_dudePosY = g_fullpipe->_aniMan->_oy;

		int res = 0;

		if (g_fullpipe->_aniMan2) {
			if (g_fullpipe->_aniMan->_ox < g_fullpipe->_sceneRect.left + 200) {
				g_currentScene->_x = g_fullpipe->_aniMan->_ox - g_fullpipe->_sceneRect.left - 300;
				g_fullpipe->_aniMan->_ox = g_vars->scene04_dudePosX;
			}
			if (g_fullpipe->_aniMan->_ox > g_fullpipe->_sceneRect.right - 200) {
				g_currentScene->_x = g_fullpipe->_aniMan->_ox - g_fullpipe->_sceneRect.right + 300;
			}

			res = 1;

			if (g_vars->scene04_soundPlaying) {
				if (g_fullpipe->_aniMan->_movement) {
					if (g_fullpipe->_aniMan->_movement->_id == MV_MAN_TOLADDER) {
						g_fullpipe->_aniMan2 = 0;
						if (g_fullpipe->_sceneRect.left > 380)
							g_currentScene->bg.x = 380 - g_fullpipe->_sceneRect.left;
					}
				}
			}
		} else {
			v7 = ;
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
				startSceneTrack();

				g_fullpipe->_behaviorManager->updateBehaviors();
				return res;
			}

			v11 = __OFSUB__(g_vars->scene04_var14 + 1, 600);
			v9 = g_vars->scene04_var14 == 599;
			v10 = g_vars->scene04_var14++ - 599 < 0;
			if (!((unsigned __int8)(v10 ^ v11) | v9))
				sceneHandler04_sub17();
		}

		if (g_vars->scene04_soundPlaying) {
			g_fullpipe->_behaviorManager->updateBehaviors();

			return res;
		}

		startSceneTrack();

		g_fullpipe->_behaviorManager->updateBehaviors();

		return res;

	case 29:
		v12 = Scene_getPictureObjectIdAtPos(g_currentScene, ex->msg.sceneClickX, ex->msg.sceneClickY);
		LOWORD(sceneHandler_pic) = v12;
		if (g_vars->scene04_var10) {
			sceneHandler04_sub1(ex);
			break;
		}

		if (v12 == PIC_SC4_LADDER) {
			if (!g_vars->scene04_var04) {
				dword_476CB8 = ex->msg.sceneClickX;
				dword_476CBC = ex->msg.sceneClickY;
				sceneHandler04_clickLadder();
				ex->msg.messageKind = 0;

				break;
			}
			sceneHandler04_gotoLadder(0);

			break;
		}
		v13 = Scene_getStaticANIObjectAtPos(g_currentScene, ex->msg.sceneClickX, ex->msg.sceneClickY);
		v14 = (GameObject *)v13;
		if (v13 && v13->GameObject.id == ANI_PLANK
			|| Scene_getPictureObjectIdAtPos(g_currentScene, ex->msg.sceneClickX, ex->msg.sceneClickY) == PIC_SC4_PLANK) {
			sceneHandler04_clickPlank();
			ex->msg.messageKind = 0;
		} else if (g_vars->scene04_var01) {
			sceneHandler04_sub8(ex);
		} else if (!v14 || !canInteractAny(&g_fullpipe->_aniMan->GameObject, v14, LOWORD(ex->msg.keyCode))) {
			v15 = (GameObject *)Scene_getPictureObjectById(g_currentScene, sceneHandler_pic, 0);
			if (!v15 || !canInteractAny(&g_fullpipe->_aniMan->GameObject, v15, LOWORD(ex->msg.keyCode))) {
				if ((v16 = ex->msg.sceneClickX, g_fullpipe->_sceneRect.right - v16 < 47) && g_fullpipe->_sceneRect.right < g_sceneWidth - 1
					|| v16 - g_fullpipe->_sceneRect.left < 47 && g_fullpipe->_sceneRect.left > 0)
					sceneHandlers_sub01(ex);
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
		StaticANIObject_changeStatics2(g_vars->scene04_clock, ST_CLK_CLOSED);
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
		return v2;

	case MSG_SC4_CLICKLADDER:
		sceneHandler04_clickLadder();
		break;

	case MSG_SC4_MANTOBOTTLE:
		sceneHandler04_manToBottle();
		break;

	case MSG_SHOWCOIN:
		sceneHandler04_showCoin();
		return v2;

	case MSG_TAKEBOTTLE:
		sceneHandler04_takeBottle();
		return v2;

	case MSG_GOTOLADDER:
		sceneHandler04_gotoLadder(0);
		break;

	case MSG_SC4_COINPUT:
		g_vars->scene04_coinPut = 1;
		break;
	}

	return 0;

#endif
	warning("STUB: sceneHandler04()");

	return 0;
}

} // End of namespace Fullpipe
