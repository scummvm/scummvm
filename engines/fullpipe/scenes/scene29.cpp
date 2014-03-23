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

void scene29_initScene(Scene *sc) {
	g_vars->scene29_var01 = 300;
	g_vars->scene29_var02 = 200;
	g_vars->scene29_var03 = 400;
	g_vars->scene29_var04 = 300;
	g_vars->scene29_porter = sc->getStaticANIObject1ById(ANI_PORTER, -1);
	g_vars->scene29_shooter1 = sc->getStaticANIObject1ById(ANI_SHOOTER1, -1);
	g_vars->scene29_shooter2 = sc->getStaticANIObject1ById(ANI_SHOOTER2, -1);
	g_vars->scene29_ass = sc->getStaticANIObject1ById(ANI_ASS, -1);

	v2 = g_vars->scene29_var05.pHead;
	if (g_vars->scene29_var05.pHead) {
		do
			v2 = v2->p0;
		while ( v2 );
	}

	g_vars->scene29_var05.numBalls = 0;
	g_vars->scene29_var05.pTail = 0;
	g_vars->scene29_var05.field_8 = 0;
	g_vars->scene29_var05.pHead = 0;
	CPlex::FreeDataChain(g_vars->scene29_var05.cPlex);
	v3 = g_vars->scene29_var08.pHead;
	g_vars->scene29_var05.cPlex = 0;

	if (g_vars->scene29_var08.pHead) {
		do
			v3 = v3->p0;
		while (v3);
	}

	StaticANIObject *ani;

	g_vars->scene29_var08.numBalls = 0;
	g_vars->scene29_var08.pTail = 0;
	g_vars->scene29_var08.field_8 = 0;
	g_vars->scene29_var08.pHead = 0;
	CPlex::FreeDataChain(g_vars->scene29_var08.cPlex);
	g_vars->scene29_var08.cPlex = 0;

	ani = sc->getStaticANIObject1ById(ANI_SHELL_GREEN, -1);
	v5 = g_vars->scene29_var05->sub04(g_vars->scene29_var05.field_8, 0);
	v5->ani = ani;

	if (g_vars->scene29_var05.field_8)
		g_vars->scene29_var05.field_8->p0 = v5;
	else
		g_vars->scene29_var05.pHead = v5;
	g_vars->scene29_var05.field_8 = v5;

	for (int i = 0; i < 2; i++) {
		ani = new StaticANIObject(ani);

		sc->addStaticANIObject(ani, 1);
		v8 = g_vars->scene29_var05->sub04(g_vars->scene29_var05.field_8, 0);
		v8->ani = ani;

		if (g_vars->scene29_var05.field_8)
			g_vars->scene29_var05.field_8->p0 = v8;
		else
			g_vars->scene29_var05.pHead = v8;
		g_vars->scene29_var05.field_8 = v8;
	}

	v9 = g_vars->scene29_var06.pHead;
	if (g_vars->scene29_var06.pHead) {
		do
			v9 = v9->p0;
		while (v9);
	}

	g_vars->scene29_var06.numBalls = 0;
	g_vars->scene29_var06.pTail = 0;
	g_vars->scene29_var06.field_8 = 0;
	g_vars->scene29_var06.pHead = 0;
	CPlex::FreeDataChain(g_vars->scene29_var06.cPlex);
	v10 = g_vars->scene29_var07.pHead;
	g_vars->scene29_var06.cPlex = 0;

	if (g_vars->scene29_var07.pHead) {
		do
			v10 = v10->p0;
		while (v10);
	}

	g_vars->scene29_var07.numBalls = 0;
	g_vars->scene29_var07.pTail = 0;
	g_vars->scene29_var07.field_8 = 0;
	g_vars->scene29_var07.pHead = 0;
	CPlex::FreeDataChain(g_vars->scene29_var07.cPlex);
	g_vars->scene29_var07.cPlex = 0;

	ani = sc->getStaticANIObject1ById(ANI_SHELL_RED, -1);
	v12 = g_vars->scene29_var06->sub04(g_vars->scene29_var06.field_8, 0);
	v12->ani = ani;

	if (g_vars->scene29_var06.field_8)
		g_vars->scene29_var06.field_8->p0 = v12;
	else
		g_vars->scene29_var06.pHead = v12;

	g_vars->scene29_var06.field_8 = v12;

	for (int i = 0; i < 2; i++) {
		ani = new StaticANIObject(ani);
		ac->addStaticANIObject(sc, ani, 1);
		v15 = g_vars->scene29_var06->sub04(g_vars->scene29_var06.field_8, 0);
		v15->ani = ani;

		if (g_vars->scene29_var06.field_8)
			g_vars->scene29_var06.field_8->p0 = v15;
		else
			g_vars->scene29_var06.pHead = v15;

		g_vars->scene29_var06.field_8 = v15;
	}

	ObArray_SetSize_0x0C((ObArray *)&g_vars->scene29_var19, 0, -1);

	ani = new StaticANIObject(accessScene(SC_COMMON)->getStaticANIObject1ById(ANI_BEARDED_CMN, -1));

	ani->_statics = ani->getStaticsById(ST_BRDCMN_EMPTY);

	sc->addStaticANIObject(ani, 1);

	v20 = g_vars->scene29_var19.m_nSize;
	ObArray_SetSize_0x0C((ObArray *)&g_vars->scene29_var19, g_vars->scene29_var19.m_nSize + 1, -1);
	v21 = &g_vars->scene29_var19.m_pData[v20];
	v21->ani = v19;
	v21->wbflag = 0;
	v21->wbcounter = 0;

	g_vars->scene29_var09 = 0;
	g_vars->scene29_var10 = 0;
	g_vars->scene29_var11 = 0;
	g_vars->scene29_var12 = 0;
	g_vars->scene29_var13 = 0;
	g_vars->scene29_var14 = 75;
	g_vars->scene29_var15 = 0;
	g_vars->scene29_var16 = 0;
	g_vars->scene29_var17 = 0;
	g_vars->scene29_var18 = 0;

	g_fp->setArcadeOverlay(PIC_CSR_ARCADE8);
}

void sceneHandler29_winArcade() {
	warning("STUB: sceneHandler29_winArcade()");
}

void sceneHandler29_shootGreen() {
	warning("STUB: sceneHandler29_shootGreen()");
}

void sceneHandler29_shootRed() {
	warning("STUB: sceneHandler29_shootRed()");
}

void sceneHandler29_clickPorter(ExCommand *cmd) {
	warning("STUB: sceneHandler29_clickPorter()");
}

void sceneHandler29_manJump() {
	if (!g_fp->_aniMan->_movement || g_fp->_aniMan->_movement->_id == MV_MAN29_RUN || g_fp->_aniMan->_movement->_id == MV_MAN29_STANDUP) {
		g_vars->scene29_var12 = 0;
		g_vars->scene29_var15 = 0;
		g_vars->scene29_var11 = 1;

		g_fp->_aniMan->changeStatics2(ST_MAN29_RUNR);
		g_fp->_aniMan->startAnim(MV_MAN29_JUMP, 0, -1);
	}

	g_vars->scene29_var20 = g_fp->_aniMan->_ox;
	g_vars->scene29_var21 = g_fp->_aniMan->_oy;
}

void sceneHandler29_manBend() {
	if (!g_fp->_aniMan->_movement || g_fp->_aniMan->_movement->_id == MV_MAN29_RUN || g_fp->_aniMan->_movement->_id == MV_MAN29_STANDUP) {
		g_vars->scene29_var12 = 0;
		g_vars->scene29_var15 = 0;
		g_vars->scene29_var11 = 1;

		g_fp->_aniMan->changeStatics2(ST_MAN29_RUNR);
		g_fp->_aniMan->startAnim(MV_MAN29_BEND, 0, -1);
	}

	g_vars->scene29_var20 = g_fp->_aniMan->_ox;
	g_vars->scene29_var21 = g_fp->_aniMan->_oy;
}

void sceneHandler29_sub03() {
	warning("STUB: sceneHandler29_sub03()");
}

void sceneHandler29_manFromL() {
	warning("STUB: sceneHandler29_manFromL()");
}

void sceneHandler29_sub05() {
	warning("STUB: sceneHandler29_sub05()");
}

void sceneHandler29_shootersEscape() {
	warning("STUB: sceneHandler29_shootersEscape()");
}

void sceneHandler29_sub07() {
	warning("STUB: sceneHandler29_sub07()");
}

void sceneHandler29_assHitGreen() {
	if (g_vars->scene29_ass->_statics->_staticsId == ST_ASS_NORM) {
		g_vars->scene29_ass->changeStatics2(ST_ASS_NORM);
		g_vars->scene29_ass->startAnim(MV_ASS_HITGREEN, 0, -1);
	}
}

void sceneHandler29_assHitRed() {
	if (g_vars->scene29_ass->_statics->_staticsId == ST_ASS_NORM) {
		g_vars->scene29_ass->changeStatics2(ST_ASS_NORM);
		g_vars->scene29_ass->startAnim(MV_ASS_HITRED, 0, -1);
	}
}

void sceneHandler29_shoot() {
	warning("STUB: sceneHandler29_shoot()");
}

void sceneHandler29_animBearded() {
	warning("STUB: sceneHandler29_animBearded()");
}



int sceneHandler29(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_CMN_WINARCADE:
		sceneHandler29_winArcade();
		break;

	case MSG_SC29_LAUGH:
		if (g_vars->scene29_var18 == ANI_SHELL_GREEN) {
			playSound(SND_29_028, 0);
			break;
		}

		playSound(SND_29_029, 0);

		break;

	case MSG_SC29_SHOWLASTRED:
		if (g_vars->scene29_var05.numBalls) {
			g_vars->scene29_var06.field_8->ani->show1(-1, -1, -1, 0);
			g_vars->scene29_var06.field_8->ani->startAnim(MV_SHR_HITASS, 0, -1);
		}

		break;

	case MSG_SC29_SHOOTGREEN:
		sceneHandler29_shootGreen();
		break;

	case MSG_SC29_SHOOTRED:
		sceneHandler29_shootRed();
		break;

	case MSG_SC29_SHOWLASTGREEN:
		if (g_vars->scene29_var05.numBalls) {
			g_vars->scene29_var05.field_8->ani->show1(-1, -1, (Objects)-1, 0);
			g_vars->scene29_var05.field_8->ani->startAnim(MV_SHG_HITASS, 0, -1);
		}

		break;

	case MSG_SC29_STOPRIDE:
		g_vars->scene29_var09 = 0;
		g_vars->scene29_var10 = 0;
		g_vars->scene29_var11 = 0;
		g_vars->scene29_var12 = 0;

		getCurrSceneSc2MotionController()->setEnabled();
		getGameLoaderInteractionController()->enableFlag24();
		break;

	case MSG_SC29_STOPRIDE:
		g_vars->scene29_var12 = 0;
		break;

	case MSG_SC29_DISABLERIDEBACK:
		g_vars->scene29_var12 = 1;
		// fall through

	case MSG_SC29_DISABLEPORTER:
		g_vars->scene29_var11 = 0;
		break;

	case MSG_SC29_ENABLEPORTER:
		g_vars->scene29_var11 = 1;
		g_vars->scene29_var12 = 0;
		g_vars->scene29_var15 = 0;
		break;

	case 29:
		if (!g_vars->scene29_var09 || g_vars->scene29_var10) {
			if (!g_vars->scene29_var10) {
				v6 = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);
				if (v6 && v6 == g_vars->scene29_porter) {
					sceneHandler29_clickPorter(cmd);

					cmd->_messageKind = 0;
					break;
				}
				break;
			}

			sceneHandler29_manJump();

			cmd->_messageKind = 0;
			break;
		}
		break;

	case 107:
		if (g_vars->scene29_var10)
			sceneHandler29_manBend();

		break;

	case 33:
		if (g_vars->scene29_var10) {
			v5 = g_vars->scene29_var20;
			if (g_vars->scene29_var20 > g_fp->_sceneRect.right - 500) {
				g_fp->_currentScene->_x = g_fp->_sceneRect.right - g_vars->scene29_var20 - 350;
				v5 = g_vars->scene29_var20;
			}

			if (v5 < g_sceneRect.left + 100)
				g_fp->_currentScene->_x = v5 - g_fp->_sceneRect.left - 100;

		} else if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + g_vars->scene29_var01)
				g_fp->_currentScene->_x = x - g_vars->scene29_var03 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - g_vars->scene29_var01)
				g_fp->_currentScene->_x = x + g_vars->scene29_var03 - g_fp->_sceneRect.right;
		}

		g_vars->scene29_var20 = g_fp->_aniMan->_ox;
		g_vars->scene29_var21 = g_fp->_aniMan->_oy;

		sceneHandler29_sub03();

		if (!g_vars->scene29_porter->_movement)
			g_vars->scene29_porter->startAnim(MV_PTR_MOVEFAST, 0, -1);

		if (g_vars->scene29_var09) {
			sceneHandler29_manFromL();
		else if (g_vars->scene29_var10 && !g_aniMan->_movement)
			sceneHandler29_sub05();

		if (g_vars->scene29_var11)
			sceneHandler29_shootersEscape();
		else if (g_vars->scene29_var12)
			sceneHandler29_sub07();

		g_vars->scene29_var13++;

		if (g_vars->scene29_var13 > g_vars->scene29_var14)
			sceneHandler29_shoot();

		sceneHandler29_animBearded();

		g_fp->_behaviorManager->updateBehaviors();
		startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
