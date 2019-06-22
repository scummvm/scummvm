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
	_dialogDelay = 0;
}

void Window::openDialog(const char *title, int tileIndex, const char *string, int more, const char *luaMore) {
	warning("STUB: Window::openDialog() required");
}

void Window::setDialogDelay(int seconds) {
	warning("STUB: Window::setDialogDelay() required");
}

} // End of Namespace
