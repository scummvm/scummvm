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

#include "fullpipe/constants.h"

#include "fullpipe/gameloader.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"
#include "fullpipe/input.h"

#include "fullpipe/interaction.h"

namespace Fullpipe {

void sceneDbgMenu_initScene(Scene *sc) {
	g_vars->selector = sc->getPictureObjectById(PIC_SCD_SEL, 0);
	getGameLoaderInteractionController()->disableFlag24();
	setInputDisabled(0);
}

GameObject *sceneHandlerDbgMenu_getObjectAtXY(int x, int y) {
	if (!g_fp->_currentScene)
		return 0;

	for (uint i = 1; i < g_fp->_currentScene->_picObjList.size(); i++) {
		PictureObject *pic = g_fp->_currentScene->_picObjList[i];

		if (x >= pic->_ox && y >= pic->_oy) {
			const Dims dims = pic->getDimensions();
			if (x <= pic->_ox + dims.x && y <= pic->_oy + dims.y && pic != g_vars->selector)
				return pic;
		}
	}

	return 0;
}

int sceneHandlerDbgMenu(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	int mx = g_fp->_mouseScreenPos.x + g_fp->_sceneRect.left;
	int my = g_fp->_mouseScreenPos.y + g_fp->_sceneRect.top;

	if (ex->_messageNum == 29) {
		GameObject *obj = sceneHandlerDbgMenu_getObjectAtXY(mx, my);
		if (obj && canInteractAny(0, obj, -3) ) {
			getGameLoaderInteractionController()->enableFlag24();
			handleObjectInteraction(0, obj, 0);
		}
		return 0;
	}
	if (ex->_messageNum != 33) {
		if (ex->_messageNum == MSG_RESTARTGAME) {
			g_fp->_needRestart = true;
			return 0;
		}
		return 0;
	}

	g_fp->_cursorId = PIC_CSR_DEFAULT;
	GameObject *obj = g_fp->_currentScene->getStaticANIObjectAtPos(mx, my);
	if (obj) {
		if (canInteractAny(0, obj, -3)) {
			g_fp->_cursorId = PIC_CSR_DEFAULT;
			g_fp->setCursor(PIC_CSR_DEFAULT);
			return 0;
		}
	} else {
		obj = sceneHandlerDbgMenu_getObjectAtXY(mx, my);
		if (obj && canInteractAny(0, obj, -3) ) {
			g_vars->selector->_flags |= 4;
			g_vars->selector->setOXY(obj->_ox, obj->_oy);
			g_fp->_cursorId = PIC_CSR_DEFAULT;
			g_fp->setCursor(PIC_CSR_DEFAULT);
			return 0;
		}
		g_vars->selector->_flags &= 0xFFFB;
	}
	g_fp->setCursor(g_fp->_cursorId);

	return 0;
}

} // End of namespace Fullpipe
