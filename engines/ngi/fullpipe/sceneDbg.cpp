/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ngi/ngi.h"

#include "ngi/constants.h"

#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"
#include "ngi/input.h"

#include "ngi/interaction.h"

namespace NGI {

void sceneDbgMenu_initScene(Scene *sc) {
	g_vars->selector = sc->findPictureObjectById(PIC_SCD_SEL, 0);
	getGameLoaderInteractionController()->disable();
	setInputDisabled(false);
}

GameObject *sceneHandlerDbgMenu_getObjectAtXY(int x, int y) {
	if (!g_nmi->_currentScene)
		return nullptr;

	for (uint i = 1; i < g_nmi->_currentScene->_pictureObjects.size(); i++) {
		PictureObject *pic = g_nmi->_currentScene->_pictureObjects[i];

		if (x >= pic->_x && y >= pic->_y) {
			const Dims dims = pic->getFullSize();
			if (x <= pic->_x + dims.x && y <= pic->_y + dims.y && pic != g_vars->selector)
				return pic;
		}
	}

	return nullptr;
}

int sceneHandlerDbgMenu(Command *ex) {
	if (ex->_cmd != 17)
		return 0;

	int mx = g_nmi->_mouseScreenPos.x + g_nmi->_sceneRect.left;
	int my = g_nmi->_mouseScreenPos.y + g_nmi->_sceneRect.top;

	if (ex->_messageNum == 29) {
		GameObject *obj = sceneHandlerDbgMenu_getObjectAtXY(mx, my);
		if (obj && canInteractAny(nullptr, obj, -3) ) {
			getGameLoaderInteractionController()->enable();
			handleObjectInteraction(nullptr, obj, 0);
		}
		return 0;
	}
	if (ex->_messageNum != 33) {
		if (ex->_messageNum == MSG_RESTARTGAME) {
			g_nmi->_needRestart = true;
			return 0;
		}
		return 0;
	}

	g_nmi->_cursorId = PIC_CSR_DEFAULT;
	GameObject *obj = g_nmi->_currentScene->findAniObjectAtPos(mx, my);
	if (obj) {
		if (canInteractAny(nullptr, obj, -3)) {
			g_nmi->_cursorId = PIC_CSR_DEFAULT;
			g_nmi->setCursor(PIC_CSR_DEFAULT);
			return 0;
		}
	} else {
		obj = sceneHandlerDbgMenu_getObjectAtXY(mx, my);
		if (obj && canInteractAny(nullptr, obj, -3) ) {
			g_vars->selector->_flags |= 4;
			g_vars->selector->setPos(obj->_x, obj->_y);
			g_nmi->_cursorId = PIC_CSR_DEFAULT;
			g_nmi->setCursor(PIC_CSR_DEFAULT);
			return 0;
		}
		g_vars->selector->_flags &= 0xFFFB;
	}
	g_nmi->setCursor(g_nmi->_cursorId);

	return 0;
}

} // End of namespace NGI
