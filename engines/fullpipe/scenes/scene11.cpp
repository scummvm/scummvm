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

#include "fullpipe/gameloader.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"

#include "fullpipe/interaction.h"
#include "fullpipe/behavior.h"


namespace Fullpipe {

void scene11_dudeSwingCallback(int *arg) {
	warning("STUB: scene11_dudeSwingCallback()");
}

void scene11_setupMusic() {
	if (g_fp->getObjectState(sO_DudeHasJumped) == g_fp->getObjectEnumState(sO_DudeHasJumped, sO_Yes))
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_11"), "MUSIC2", 1);
}

void scene11_initScene(Scene *sc) {
	g_vars->scene11_swingie = sc->getStaticANIObject1ById(ANI_SWINGER, -1);
	g_vars->scene11_boots = sc->getStaticANIObject1ById(ANI_BOOTS_11, -1);
	g_vars->scene11_var01.clear();
	g_vars->scene11_dudeOnSwing = sc->getStaticANIObject1ById(ANI_MAN11, -1);
	g_vars->scene11_dudeOnSwing->_callback2 = scene11_dudeSwingCallback;
	g_vars->scene11_dudeOnSwing = sc->getStaticANIObject1ById(ANI_KACHELI, -1);
	g_vars->scene11_dudeOnSwing->_callback2 = scene11_dudeSwingCallback;
	g_vars->scene11_hint = sc->getPictureObjectById(PIC_SC11_HINT, 0);
	g_vars->scene11_hint->_flags &= 0xFFFB;

	g_vars->scene11_var02 = 0;
	g_vars->scene11_var03 = 0;
	g_vars->scene11_var04 = 0;
	g_vars->scene11_var05 = 0;
	g_vars->scene11_var06 = 0;
	g_vars->scene11_var07 = 0;
	g_vars->scene11_var08 = 1.0;
	g_vars->scene11_var09 = 1.0;
	g_vars->scene11_var10 = 1.0;
	g_vars->scene11_var11 = 1.0;
	g_vars->scene11_var12 = 1.9849218750000000;
	g_vars->scene11_var13 = 0;
	g_vars->scene11_var14 = 0;
	g_vars->scene11_var15 = 0;
	g_vars->scene11_var16 = 0;
	g_vars->scene11_var17 = 0;
	g_vars->scene11_var18 = 0;

	Scene *oldsc = g_fp->_currentScene;

	g_fp->_currentScene = sc;

	int swingie = g_fp->getObjectState(sO_Swingie);

	if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSwinging)
		|| swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSwingingWithBoot)) {
		g_vars->scene11_var19 = 1;
		g_vars->scene11_var20 = 0;

		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 0);

		((MctlCompound *)getCurrSceneSc2MotionController())->replaceNodeX(805, 905);

		getSc2MctlCompoundBySceneId(sc->_sceneId)->replaceNodeX(303, 353);
	} else if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInBoots)
			   || swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInCorner)) {
		g_vars->scene11_var19 = 0;
		g_vars->scene11_var20 = 1;

		g_vars->scene11_swingie->changeStatics2(ST_SWR_STAND3);

		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 0);

		((MctlCompound *)getCurrSceneSc2MotionController())->replaceNodeX(905, 805);
	} else {
		g_vars->scene11_var19 = 0;
		g_vars->scene11_var20 = 0;

		if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSitting)) {
			g_vars->scene11_swingie->_movement = 0;
			g_vars->scene11_swingie->_statics = g_vars->scene11_swingie->getStaticsById(ST_SWR_SIT);
			g_vars->scene11_swingie->setOXY(144, 389);

			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 1);
		} else {
			g_vars->scene11_swingie->_movement = 0;
			g_vars->scene11_swingie->_statics = g_vars->scene11_swingie->getStaticsById(ST_SWR_SITBALD);
			g_vars->scene11_swingie->setOXY(144, 415);

			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 1);
		}
	}

	if (!g_vars->scene11_var19) {
		g_vars->scene11_dudeOnSwing->changeStatics2(ST_KCH_STATIC);
		g_vars->scene11_dudeOnSwing->setOXY(691, 371);
		g_vars->scene11_dudeOnSwing->_priority = 20;

		g_vars->scene11_dudeOnSwing->_flags |= 4;
	}

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_11");
	g_fp->setArcadeOverlay(PIC_CSR_ARCADE5);
}

void sceneHandler11_winArcade() {
	warning("STUB: sceneHandler11_winArcade()");
}

void sceneHandler11_restartMan() {
	warning("STUB: sceneHandler11_restartMan()");
}

void sceneHandler11_hitMan() {
	warning("STUB: sceneHandler11_hitMan()");
}

void sceneHandler11_manToSwing() {
	warning("STUB: sceneHandler11_manToSwing()");
}

void sceneHandler11_putBoot() {
	warning("STUB: sceneHandler11_putBoot()");
}

void sceneHandler11_showSwing() {
	warning("STUB: sceneHandler11_showSwing()");
}

void sceneHandler11_sub01() {
	warning("STUB: sceneHandler11_sub01()");
}

void sceneHandler11_sub02() {
	warning("STUB: sceneHandler11_sub02()");
}

void sceneHandler11_sub03() {
	warning("STUB: sceneHandler11_sub03()");
}

void sceneHandler11_sub04() {
	warning("STUB: sceneHandler11_sub04()");
}

void sceneHandler11_sub05() {
	warning("STUB: sceneHandler11_sub05()");
}

void sceneHandler11_sub06() {
	warning("STUB: sceneHandler11_sub06()");
}

void sceneHandler11_sub07() {
	warning("STUB: sceneHandler11_sub07()");
}

void sceneHandler11_sub08() {
	warning("STUB: sceneHandler11_sub08()");
}

void sceneHandler11_sub09() {
	warning("STUB: sceneHandler11_sub09()");
}

void sceneHandler11_sub10() {
	warning("STUB: sceneHandler11_sub10()");
}

void sceneHandler11_sub11() {
	warning("STUB: sceneHandler11_sub11()");
}

void sceneHandler11_sub12() {
	warning("STUB: sceneHandler11_sub12()");
}

int sceneHandler11(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_CMN_WINARCADE:
		sceneHandler11_winArcade();
		break;

	case MSG_SC11_SITSWINGER:
		if (g_fp->getObjectState(sO_Swingie) == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInBoots)
			    || g_fp->getObjectState(sO_Swingie) == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInCorner)) {
			g_fp->setObjectState(sO_Swingie, g_fp->getObjectEnumState(sO_Swingie, sO_IsSitting));
		}
		break;

	case MSG_SC11_MANCRY:
		g_fp->playSound(g_vars->scene11_var07, 0);

		g_vars->scene11_var07 = 0;
		break;

	case MSG_SC11_RESTARTMAN:
		sceneHandler11_restartMan();
		break;

	case MSG_SC11_HITMAN:
		sceneHandler11_hitMan();
		break;

	case MSG_SC11_MANTOSWING:
		sceneHandler11_manToSwing();
		break;

	case MSG_SC11_PUTBOOT:
		sceneHandler11_putBoot();
		break;

	case MSG_SC11_SHOWSWING:
		sceneHandler11_showSwing();
		break;

	case 107:
		if (g_vars->scene11_var02)
			sceneHandler11_sub01();
		break;

	case 33:
		{
			int res = 0;
			int x, y;

			if (!g_fp->_aniMan2)
				goto LABEL_27;

			x = g_fp->_aniMan2->_ox;
			y = g_fp->_aniMan2->_oy;

			g_vars->scene11_var21 = x;
			g_vars->scene11_var22 = y;

			if (g_vars->scene11_var03) {
				if (x > g_fp->_sceneRect.right - 200)
					g_fp->_currentScene->_x = x - g_fp->_sceneRect.right + 300;
				goto LABEL_26;
			}

			if (g_vars->scene11_var04) {
				g_fp->_currentScene->_x = g_fp->_sceneWidth - x;

				if (g_vars->scene11_var21 < 910)
					g_vars->scene11_var04 = 0;

			LABEL_26:
				res = 1;
			LABEL_27:
				if (g_vars->scene11_var20) {
					if (g_fp->_sceneRect.left >= 534 && g_vars->scene11_var06 < 534)
						sceneHandler11_sub06();
					g_vars->scene11_var06 = g_fp->_sceneRect.left;
				}

				if (!g_vars->scene11_var02)
					goto LABEL_50;

				if (g_vars->scene11_var16 <= 0 || g_vars->scene11_var15 - g_vars->scene11_var16 <= 72) {
				} else {
					sceneHandler11_sub02();
					g_vars->scene11_var18 = 0;
					g_vars->scene11_var16 = 0;
				}

				if (!g_vars->scene11_var02)
					goto LABEL_50;

				if (g_vars->scene11_var17 == g_vars->scene11_var18 || g_vars->scene11_var16 <= 0 || g_vars->scene11_var15 - g_vars->scene11_var16 <= 2) {
				LABEL_49:
					if (g_vars->scene11_var02) {
						g_fp->_behaviorManager->updateBehaviors();
						g_fp->startSceneTrack();
						return res;
					}
				LABEL_50:
					if (g_vars->scene11_var19
						|| (0.0 == g_vars->scene11_var10
							&& g_vars->scene11_dudeOnSwing->_movement != 0
							&& g_vars->scene11_dudeOnSwing->_movement->_currDynamicPhaseIndex == 45
							&& (g_vars->scene11_dudeOnSwing->changeStatics2(ST_KCH_STATIC), !g_vars->scene11_var02)
							&& g_vars->scene11_var19)) {
						if (!g_vars->scene11_swingie->_movement) {
							if ((g_vars->scene11_boots->_flags & 4) && g_vars->scene11_boots->_statics->_staticsId == ST_BTS11_2) {
								sceneHandler11_sub07();
								g_fp->_behaviorManager->updateBehaviors();
								g_fp->startSceneTrack();
								return res;
							}
							g_vars->scene11_swingie->startAnim(MV_SWR_SWING, 0, -1);
						}
					}
					g_fp->_behaviorManager->updateBehaviors();
					g_fp->startSceneTrack();
					return res;
				}

				if (g_vars->scene11_var18 == 1) {
					if (!g_vars->scene11_var17)
						sceneHandler11_sub03();
					else
						sceneHandler11_sub02();
				} else if (g_vars->scene11_var18 == 2) {
					if (!g_vars->scene11_var17)
						sceneHandler11_sub04();
					else
						sceneHandler11_sub02();
				}

				g_vars->scene11_var16 = g_vars->scene11_var15;
				goto LABEL_49;
			}
			if (x >= g_fp->_sceneRect.left + 200) {
				if (x <= g_fp->_sceneRect.right - 200) {
				LABEL_18:
					if (y < g_fp->_sceneRect.top + 200) {
						g_fp->_currentScene->_y = y - g_fp->_sceneRect.top - 300;
						y = g_vars->scene11_var22;
						x = g_vars->scene11_var21;
					}
					if (y > g_fp->_sceneRect.bottom - 300) {
						g_fp->_currentScene->_y = y - g_fp->_sceneRect.bottom + 300;
						x = g_vars->scene11_var21;
					}
					if (x >= 940)
						g_vars->scene11_var04 = 1;
					goto LABEL_26;
				}
				g_fp->_currentScene->_x = x - g_fp->_sceneRect.right + 300;
			} else {
				g_fp->_currentScene->_x = x - g_fp->_sceneRect.left - 300;
			}
			y = g_vars->scene11_var22;
			x = g_vars->scene11_var21;
			goto LABEL_18;
		}

		break;

	case 29:
		if (g_vars->scene11_var19) {
			if (g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y) == g_vars->scene11_swingie
				&& cmd->_keyCode == ANI_INV_BOOT)
				sceneHandler11_putBoot();
		} else {
			if (g_vars->scene11_var02) {
				sceneHandler11_sub05();
				g_vars->scene11_var16 = g_vars->scene11_var15;
			}
		}

		if (!g_vars->scene11_var02) {
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);

						return 0;
					}
				}
			}
			return 0;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
