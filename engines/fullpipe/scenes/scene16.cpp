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

void scene16_initScene(Scene *sc) {
	g_vars->scene16_figures.clear();
	g_vars->scene16_walkingBoy = 0;
	g_vars->scene16_walkingGirl = 0;
	g_vars->scene16_walkingCount = 200;
	g_vars->scene16_wire = sc->getStaticANIObject1ById(ANI_WIRE16, -1);
	g_vars->scene16_mug = sc->getStaticANIObject1ById(ANI_MUG, -1);
	g_vars->scene16_jettie = sc->getStaticANIObject1ById(ANI_JETTIE, -1);
	g_vars->scene16_boot = sc->getStaticANIObject1ById(ANI_BOOT_16, -1);
	g_vars->scene16_girlIsLaughing = false;
	g_vars->scene16_sound = SND_16_034;

	if (g_fp->getObjectState(sO_Bridge) == g_fp->getObjectEnumState(sO_Bridge, sO_Convoluted)) {
		g_vars->scene16_placeIsOccupied = true;

		StaticANIObject *boy[2];
		boy[0] = sc->getStaticANIObject1ById(ANI_BOY, -1);
		boy[0]->loadMovementsPixelData();

		boy[1] = new StaticANIObject(boy[0]);
		sc->addStaticANIObject(boy[1], 1);
		
		int idx = 0;
		
		for (int i = 0; i < 3; i++) {
			g_vars->scene16_figures.push_back(boy[idx]);

			idx++;

			if (idx >= 2)
				idx = 0;
		}
		
		g_vars->scene16_figures.push_back(sc->getStaticANIObject1ById(ANI_GIRL, -1));

		for (int i = 0; i < 4; i++) {
			g_vars->scene16_figures.push_back(boy[idx]);

			idx++;

			if (idx >= 2)
				idx = 0;
		}
	} else {
		g_fp->setObjectState(sO_Girl, g_fp->getObjectEnumState(sO_Girl, sO_IsSwinging));

		g_vars->scene16_placeIsOccupied = false;

		StaticANIObject *ani = new StaticANIObject(g_fp->accessScene(SC_COMMON)->getStaticANIObject1ById(ANI_BEARDED_CMN, -1));
		ani->_movement = 0;
		ani->_statics = (Statics *)ani->_staticsList[0];
		sc->addStaticANIObject(ani, 1);
	}

	if (g_fp->getObjectState(sO_Girl) == g_fp->getObjectEnumState(sO_Girl, sO_IsLaughing)) {
		StaticANIObject *girl = sc->getStaticANIObject1ById(ANI_GIRL, -1);

		girl->show1(554, 432, MV_GRL_LAUGH_POPA, 0);
		girl->_priority = 20;
	}

	if (g_fp->getObjectState(sO_Cup) == g_fp->getObjectEnumState(sO_Cup, sO_In_16)) {
		g_vars->scene16_mug->_statics = g_vars->scene16_mug->getStaticsById(ST_MUG_EMPTY);
		g_vars->scene16_mug->_movement = 0;
		g_vars->scene16_mug->setOXY(409, 459);
		g_vars->scene16_mug->_priority = 5;
		g_vars->scene16_mug->_flags |= 4;
	}
}

int scene16_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_objectIdAtCursor == PIC_SC16_TUMBA) {
		if (g_fp->_cursorId == PIC_CSR_DEFAULT)
			g_fp->_cursorId = PIC_CSR_ITN;
	} else {
		if (g_fp->_objectIdAtCursor == ANI_MUG && g_fp->_cursorId == PIC_CSR_ITN && g_vars->scene16_mug->_statics->_staticsId == ST_MUG_FULL)
			g_fp->_cursorId = PIC_CSR_ITN_GREEN;
	}

	return g_fp->_cursorId;
}

void sceneHandler16_laughSound() {
	int snd = SND_16_035;

	switch (g_vars->scene16_sound) {
	case SND_16_034:
		snd = SND_16_035;
		break;

	case SND_16_035:
		snd = SND_16_037;
		break;

	case SND_16_037:
		snd = SND_16_034;
		break;
	}

	g_vars->scene16_sound = snd;

	g_fp->playSound(snd, 0);
}

void sceneHandler16_showBearded() {
	if (g_fp->getObjectState(sO_Bridge) == g_fp->getObjectEnumState(sO_Bridge, sO_Unconvoluted)) {
		StaticANIObject *brd = g_fp->_currentScene->getStaticANIObject1ById(ANI_BEARDED_CMN, -1);

		if (!brd || !(brd->_flags & 4))
			chainQueue(QU_BRD16_STARTBEARDED, 0);
	}
}

void sceneHandler16_showMugFull() {
	g_vars->scene16_mug->changeStatics2(ST_MUG_FULL);
}

void sceneHandler16_fillMug() {
	if (g_vars->scene16_mug->_flags & 4) {
		g_vars->scene16_jettie->_priority = 2;
		g_vars->scene16_jettie->startAnim(MV_JTI_FLOWIN, 0, -1);

		if (g_fp->_aniMan->_movement) {
			if (g_fp->_aniMan->_movement->_id == MV_MAN16_TAKEMUG) {
				g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT);

				g_vars->scene16_mug->show1(-1, -1, -1, 0);

				g_fp->setObjectState(sO_Cup, g_fp->getObjectEnumState(sO_Cup, sO_DudeHas));
			}
		}
		return;
	}

	MessageQueue *mq;

	if (!(g_vars->scene16_boot->_flags & 4)) {
		g_vars->scene16_jettie->_priority = 15;
		g_vars->scene16_jettie->startAnim(MV_JTI_FLOWBY, 0, -1);

		if (g_vars->scene16_walkingBoy) {
			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_BOYOUT), 0, 1);

			mq->replaceKeyCode(-1, g_vars->scene16_walkingBoy->_okeyCode);
			if (mq->chain(g_vars->scene16_walkingBoy) || !mq)
				return;
		} else {
			if (!g_vars->scene16_walkingGirl)
				return;

			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_GIRLOUT), 0, 1);

			mq->replaceKeyCode(-1, g_vars->scene16_walkingGirl->_okeyCode);
			if (mq->chain(g_vars->scene16_walkingGirl))
				return;
		}
		delete mq;

		return;
	}

	g_vars->scene16_jettie->_priority = 15;

	g_vars->scene16_boot->startAnim(MV_BT16_FILL, 0, -1);

	StaticANIObject *ani;

	if (g_vars->scene16_walkingBoy) {
		mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_BOYOUT), 0, 1);

		mq->replaceKeyCode(-1, g_vars->scene16_walkingBoy->_okeyCode);

		ani = g_vars->scene16_walkingBoy;
	} else {
		if (!g_vars->scene16_walkingGirl)
			return;

		mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_GIRLOUT), 0, 1);

		mq->replaceKeyCode(-1, g_vars->scene16_walkingGirl->_okeyCode);
		ani = g_vars->scene16_walkingGirl;
	}

	if (!mq->chain(ani))
		delete mq;
}

void sceneHandler16_startLaugh() {
	StaticANIObject *girl = g_fp->_currentScene->getStaticANIObject1ById(ANI_GIRL, -1);

	girl->changeStatics2(ST_GRL_STAND);

	MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_GIRLLAUGH), 0, 1);

	mq->replaceKeyCode(-1, girl->_okeyCode);
	mq->setFlags(mq->getFlags() | 1);
	mq->chain(0);

	g_fp->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->setSubVarAsInt(sO_DudeSwinged, 0);

	g_vars->scene16_girlIsLaughing = true;
}

void sceneHandler16_drink() {
	if (g_vars->scene16_mug->_flags & 4) {
		if (!g_vars->scene16_jettie->_movement) {
			if (!g_vars->scene16_walkingBoy || !g_vars->scene16_walkingBoy->_movement || g_vars->scene16_walkingBoy->_movement->_id != MV_BOY_DRINK) {
				if (!g_vars->scene16_walkingGirl || !g_vars->scene16_walkingGirl->_movement || g_vars->scene16_walkingGirl->_movement->_id != MV_GRL_DRINK) {
					if (g_vars->scene16_mug->_statics->_staticsId == ST_MUG_FULL) {
						MessageQueue *mq;
						ExCommand *ex;

						if (g_vars->scene16_walkingBoy) {
							g_fp->_aniMan->_flags |= 0x180;

							g_vars->scene16_walkingBoy->changeStatics2(ST_BOY_STAND);
							g_vars->scene16_walkingBoy->queueMessageQueue(0);

							mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_BOYKICK), 0, 1);

							mq->replaceKeyCode(-1, g_vars->scene16_walkingBoy->_okeyCode);
							
							ex = new ExCommand(ANI_MAN, 34, 384, 0, 0, 0, 1, 0, 0, 0);
							ex->_excFlags |= 3u;
							ex->_field_14 = 384;
							ex->_messageNum = 0;

							mq->insertExCommandAt(2, ex);
							mq->setFlags(mq->getFlags() | 1);
							mq->chain(0);
						} else {
							g_fp->_aniMan->_flags |= 1;

							mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_MANDRINK), 0, 1);

							ex = new ExCommand(ANI_MAN, 34, 256, 0, 0, 0, 1, 0, 0, 0);
							ex->_excFlags |= 3u;
							ex->_field_14 = 256;
							ex->_messageNum = 0;

							mq->addExCommandToEnd(ex);
							mq->setFlags(mq->getFlags() | 1);
							mq->chain(0);

							g_fp->_currentScene->getStaticANIObject1ById(ANI_GIRL, -1)->changeStatics2(ST_GRL_STAND);
						}

						g_fp->_currentScene->getStaticANIObject1ById(ANI_WIRE16, -1)->show1(-1, -1, -1, 0);
					} else {
						chainObjQueue(g_fp->_aniMan, QU_SC16_TAKEMUG, 1);
					}
				}
			}
		}
	}
}

void sceneHandler16_mugClick() {
	if (abs(310 - g_fp->_aniMan->_ox) >= 1 || abs(449 - g_fp->_aniMan->_oy) >= 1
		|| g_fp->_aniMan->_movement || g_fp->_aniMan->_statics->_staticsId != ST_MAN_RIGHT) {
		MessageQueue *mq = getCurrSceneSc2MotionController()->method34(g_fp->_aniMan, 310, 449, 1, ST_MAN_RIGHT);

		if (mq) {
			ExCommand *ex = new ExCommand(0, 17, MSG_SC16_MUGCLICK, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags = 2;
			mq->addExCommandToEnd(ex);

			postExCommand(g_fp->_aniMan->_id, 2, 310, 449, 0, -1);
		}
	} else {
		sceneHandler16_drink();
	}
}

void sceneHandler16_showMan() {
	g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT);
	g_fp->_aniMan->show1(-1, -1, -1, 0);

	g_vars->scene16_mug->show1(-1, -1, -1, 0);
}

void sceneHandler16_showMug() {
	chainQueue(QU_SC16_SHOWMUG, 0);
}

void sceneHandler16_hideMan() {
	g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT);
	g_fp->_aniMan->hide();

	g_vars->scene16_mug->hide();
}

void sceneHandler16_hideMug() {
	g_vars->scene16_mug->hide();
}

void sceneHandler16_hideWire() {
	g_vars->scene16_wire->hide();
}

void sceneHandler16_showWire() {
	g_vars->scene16_wire->show1(-1, -1, -1, 0);
}

void sceneHandler16_putOnWheel() {
	StaticANIObject *ani = g_vars->scene16_walkingBoy;

	if (!ani)
		ani = g_vars->scene16_walkingGirl;

	if (ani)
		g_vars->scene16_figures.push_back(ani);

	ani = g_vars->scene16_figures.front();

	g_vars->scene16_figures.pop_front();

	if (ani) {
		MessageQueue *mq;

		if (ani->_id == ANI_BOY) {
			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_GOBOY), 0, 1);

			mq->replaceKeyCode(-1, ani->_okeyCode);
			mq->chain(0);

			g_vars->scene16_walkingBoy = ani;
			g_vars->scene16_walkingGirl = 0;
		} else if (ani->_id == ANI_GIRL) {
			if (g_fp->getObjectState(sO_Girl) == g_fp->getObjectEnumState(sO_Girl, sO_IsSwinging)) {
				mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC16_GOGIRL), 0, 1);

				mq->replaceKeyCode(-1, ani->_okeyCode);
				mq->chain(0);

				g_vars->scene16_walkingBoy = 0;
				g_vars->scene16_walkingGirl = ani;
			}
		}
	}
}

void sceneHandler16_girlROTFL() {
	StaticANIObject *girl = g_fp->_currentScene->getStaticANIObject1ById(ANI_GIRL, -1);

	girl->changeStatics2(ST_GRL_LAUGH);
	girl->startAnim(MV_GRL_FALL, 0, -1);

	g_vars->scene16_girlIsLaughing = false;
}

int sceneHandler16(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch(cmd->_messageNum) {
	case MSG_SC16_LAUGHSOUND:
		sceneHandler16_laughSound();
		break;

	case MSG_SC16_SHOWBEARDED:
		sceneHandler16_showBearded();
		break;

	case MSG_SC16_SHOWMUGFULL:
		sceneHandler16_showMugFull();
		break;

	case MSG_SC16_FILLMUG:
		sceneHandler16_fillMug();
		break;

	case MSG_SC16_STARTLAUGH:
		sceneHandler16_startLaugh();
		break;

	case MSG_SC16_MUGCLICK:
		if (!g_fp->_aniMan->isIdle() || g_fp->_aniMan->_flags & 0x100)
			cmd->_messageKind = 0;
		else
			sceneHandler16_mugClick();

		break;

	case MSG_SC16_SHOWMAN:
		sceneHandler16_showMan();
		break;

	case MSG_SC16_SHOWMUG:
		sceneHandler16_showMug();
		break;

	case MSG_SC16_HIDEMAN:
		sceneHandler16_hideMan();
		break;

	case MSG_SC16_HIDEMUG:
		sceneHandler16_hideMug();
		break;

	case MSG_SC16_HIDEWIRE:
		sceneHandler16_hideWire();
		break;

	case MSG_SC16_SHOWWIRE:
		sceneHandler16_showWire();
		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene16_placeIsOccupied) {
			g_vars->scene16_walkingCount++;

			if (g_vars->scene16_walkingCount < 280) {
				sceneHandler16_putOnWheel();

				g_vars->scene16_walkingCount = 0;
			}
		}

		if (g_vars->scene16_girlIsLaughing) {
			if (g_fp->_aniMan->_movement)
				if (g_fp->_aniMan->_movement->_id == MV_MAN_TURN_RL)
					sceneHandler16_girlROTFL();
		}

		g_fp->_behaviorManager->updateBehaviors();
		g_fp->startSceneTrack();

	}

	return 0;
}

} // End of namespace Fullpipe
