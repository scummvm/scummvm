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

#include "hdb/hdb.h"

namespace HDB {

bool Window::init() {

	warning("STUB: Window::init: Incomplete");

	_gfxTL = g_hdb->_drawMan->loadPic("menu_back_topleft");
	_gfxTM = g_hdb->_drawMan->loadPic("menu_back_top");
	_gfxTR = g_hdb->_drawMan->loadPic("menu_back_topright");

	_gfxL = g_hdb->_drawMan->loadPic("menu_back_left");
	_gfxM = g_hdb->_drawMan->loadPic("menu_back_middle");
	_gfxR = g_hdb->_drawMan->loadPic("menu_back_right");

	_gfxBL = g_hdb->_drawMan->loadPic("menu_back_btmleft");
	_gfxBM = g_hdb->_drawMan->loadPic("menu_back_bottom");
	_gfxBR = g_hdb->_drawMan->loadPic("menu_back_btmright");

	_gfxTitleL = g_hdb->_drawMan->loadPic("menu_title_left");
	_gfxTitleM = g_hdb->_drawMan->loadPic("menu_title_mid");
	_gfxTitleR = g_hdb->_drawMan->loadPic("menu_title_right");

	_gGfxTL = g_hdb->_drawMan->loadPic("g_menu_back_topleft");
	_gGfxTM = g_hdb->_drawMan->loadPic("g_menu_back_top");
	_gGfxTR = g_hdb->_drawMan->loadPic("g_menu_back_topright");
	
	_gGfxL = g_hdb->_drawMan->loadPic("g_menu_back_left");
	_gGfxM = g_hdb->_drawMan->loadPic("g_menu_back_middle");
	_gGfxR = g_hdb->_drawMan->loadPic("g_menu_back_right");
	
	_gGfxBL = g_hdb->_drawMan->loadPic("g_menu_back_btmleft");
	_gGfxBM = g_hdb->_drawMan->loadPic("g_menu_back_bottom");
	_gGfxBR = g_hdb->_drawMan->loadPic("g_menu_back_btmright");
	
	_gGfxTitleL = g_hdb->_drawMan->loadPic("g_menu_title_left");
	_gGfxTitleM = g_hdb->_drawMan->loadPic("g_menu_title_mid");
	_gGfxTitleR = g_hdb->_drawMan->loadPic("g_menu_title_right");

	_gfxIndent = g_hdb->_drawMan->loadPic("menu_delivery_indentation");
	_gfxArrowTo = g_hdb->_drawMan->loadPic("menu_arrow_deliverto");

	_gfxTry = _gfxAgain = NULL; // They will be loaded when needed

	_gfxInvSelect = g_hdb->_drawMan->loadPic("inventory_normal");
	_gfxHandright = g_hdb->_drawMan->loadPic("menu_hand_pointright");

	restartSystem();

	return true;
}

void Window::restartSystem() {
	_dialogInfo.active = false;
	_dialogDelay = 0;
}

void Window::openDialog(const char *title, int tileIndex, const char *string, int more, const char *luaMore) {
	if (_dialogInfo.active)
		return;

	_dialogInfo.tileIndex = tileIndex;
	strcpy(_dialogInfo.title, title);
	_dialogInfo.active = true;

	int e1, e2, e3, e4;
	int width, height;
	int titleWidth, titleHeight;
	int w;

	if (strlen(string) > sizeof(_dialogInfo.string))
		strncpy(_dialogInfo.string, string, sizeof(_dialogInfo.string) - 1);
	else
		strcpy(_dialogInfo.string, string);
	g_hdb->_drawMan->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_drawMan->setTextEdges(kDialogTextLeft, kDialogTextRight, 0, 480);
	g_hdb->_drawMan->getDimensions(string, &width, &height);
	g_hdb->_drawMan->getDimensions(title, &titleWidth, &titleHeight);
	g_hdb->_drawMan->setTextEdges(e1, e2, e3, e4);
	_dialogInfo.height = (height + 2) * 16;
	w = _dialogInfo.width = width + 32;
	_dialogInfo.titleWidth = titleWidth;
	if (titleWidth > w)
		w = titleWidth;

	_dialogInfo.x = (480 >> 1) - (w >> 1);

	int px, py;
	g_hdb->_ai->getPlayerXY(&px, &py);
	if (py < (kScreenHeight >> 1) - 16)
		_dialogInfo.y = (kScreenHeight >> 1) + 16;
	else
		_dialogInfo.y = (kScreenHeight >> 1) - (_dialogInfo.height + 64);

	if (_dialogInfo.y < 16)
		_dialogInfo.y = 16;

	_dialogInfo.more = more;
	if (luaMore)
		strcpy(_dialogInfo.luaMore, luaMore);
	warning("STUB: openDialog: Play SND_MOVE_SELECTION");
}

void Window::closeDialog() {
	if (_dialogInfo.active) {
		warning("STUB: closeDialog: Play SND_SWITCH_USE");
		_dialogInfo.active = false;
		_dialogDelay = 0;
		if (_dialogInfo.luaMore[0] && !g_hdb->_ai->cinematicsActive())
			g_hdb->_lua->callFunction(_dialogInfo.luaMore, 0);
	}
}

void Window::setDialogDelay(int delay) {
	_dialogDelay = g_system->getMillis() + 1000 * delay;
}

} // End of Namespace
