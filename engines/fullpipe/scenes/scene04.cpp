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

} // End of namespace Fullpipe
