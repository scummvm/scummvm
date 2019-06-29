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

	_gfxInfobar = g_hdb->_drawMan->loadPic("pic_infobar");
	_gfxDarken = g_hdb->_drawMan->loadPic("screen_darken");
	_gfxPausePlaque = g_hdb->_drawMan->loadPic("pause_plaque");

	_infobarDimmed = 0;

	_invWinInfo.width = kInvItemSpaceX * 3;
	_invWinInfo.height = kInvItemSpaceY * 4;
	_invWinInfo.x = kScreenWidth - _gfxInfobar->_width + 16;
	_invWinInfo.y = 40;

	restartSystem();

	return true;
}

void Window::restartSystem() {
	_dialogInfo.active = false;
	_invWinInfo.active = false;
	_dialogDelay = _invWinInfo.selection = 0;
	_gemGfx = g_hdb->_drawMan->loadTile("ent_gem_white_sit01");
	_infobarDimmed = 0;
}

void Window::setInfobarDark(int value) {
	_infobarDimmed = value;
}

void Window::drawPause() {
	if (g_hdb->getPause())
		_gfxPausePlaque->drawMasked(480 / 2 - _gfxPausePlaque->_width / 2, kPauseY);
}

void Window::checkPause(uint x, uint y) {
	if (x >= 480 / 2 - _gfxPausePlaque->_width / 2 && 480 / 2 + _gfxPausePlaque->_width / 2 > x && y >= kPauseY && y < kPauseY + _gfxPausePlaque->_height) {
		g_hdb->togglePause();
		warning("STUB: checkPause: Play SND_POP");
	}
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

void Window::drawDialog() {

#if 0
	_gfxTL->drawMasked(0, 0);
	_gfxTM->drawMasked(_gfxTL->_width, 0);
	_gfxTR->drawMasked(_gfxTL->_width + _gfxTM->_width, 0);

	_gfxL->drawMasked(0, _gfxTL->_height);
	_gfxM->drawMasked(_gfxL->_width, _gfxTL->_height);
	_gfxR->drawMasked(_gfxL->_width + _gfxM->_width, _gfxTL->_height);

	_gfxBL->drawMasked(0, _gfxTL->_height + _gfxL->_height);
	_gfxBM->drawMasked(_gfxBL->_width, _gfxTL->_height + _gfxL->_height);
	_gfxBR->drawMasked(_gfxBL->_width + _gfxBM->_width, _gfxTL->_height + _gfxL->_height);
#endif

	if (g_hdb->getActionMode())
		debug(9, "STUB: drawDialog: Draw Player Weapon");

	if (!_dialogInfo.active)
		return;
	bool guyTalking = !scumm_stricmp(_dialogInfo.title, "guy");

	int w = _dialogInfo.width;
	if (_dialogInfo.titleWidth > w)
		w = _dialogInfo.titleWidth;

	drawBorder(_dialogInfo.x, _dialogInfo.y, w, _dialogInfo.height, guyTalking);

	int blocks;
	if (!guyTalking) {
		_gfxTitleL->drawMasked(_dialogInfo.x, _dialogInfo.y - 10);
		blocks = _dialogInfo.titleWidth / 16;
		for (int i = 0; i < blocks; i++)
			_gfxTitleM->drawMasked(_dialogInfo.x + 16 * (i + 1), _dialogInfo.y - 10);
		_gfxTitleR->drawMasked(_dialogInfo.x + (blocks + 1) * 16, _dialogInfo.y - 10);
	} else {
		_gGfxTitleL->drawMasked(_dialogInfo.x, _dialogInfo.y - 10);
		blocks = _dialogInfo.titleWidth / 16;
		for (int i = 0; i < blocks; i++)
			_gGfxTitleM->drawMasked(_dialogInfo.x + 16 * (i + 1), _dialogInfo.y - 10);
		_gGfxTitleR->drawMasked(_dialogInfo.x + (blocks + 1) * 16, _dialogInfo.y - 10);
	}

	int e1, e2, e3, e4;
	g_hdb->_drawMan->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_drawMan->setTextEdges(_dialogInfo.x + 10, 480, 0, kScreenHeight);
	g_hdb->_drawMan->setCursor(0, _dialogInfo.y - 7);
	if (_dialogInfo.title)
		g_hdb->_drawMan->drawText(_dialogInfo.title);
	g_hdb->_drawMan->setTextEdges(_dialogInfo.x + 16, _dialogInfo.x + _dialogInfo.width - 16, 0, kScreenHeight);
	g_hdb->_drawMan->setCursor(0, _dialogInfo.y + 16);
	if (_dialogInfo.string)
		g_hdb->_drawMan->drawText(_dialogInfo.string);
	g_hdb->_drawMan->setTextEdges(e1, e2, e3, e4);
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

bool Window::checkDialogClose(int x, int y) {
	if (!_dialogInfo.active)
		return false;

	if (x >= _dialogInfo.x && x < _dialogInfo.x + _dialogInfo.width && y >= _dialogInfo.y && y < _dialogInfo.y + _dialogInfo.height) {
		closeDialog();
		return true;
	}

	// If Cinematics are on, we need to timeout instead of get a click
	if (g_hdb->_ai->cinematicsActive()) {
		if (_dialogDelay < g_hdb->getTimeSlice()) {
			closeDialog();
			_dialogDelay = 0;
			return true;
		}
	}

	return false;
}

void Window::drawBorder(int x, int y, int width, int height, bool guyTalking) {
	int wide, high, i, j;
	if (!guyTalking) {
		wide = (width / 16) + 1;
		high = height / 16;
		for (j = 0; j < high; j++)
			for (i = 0; i < wide; i++) {
				// Top Line
				if (!i && !j)
					_gfxTL->drawMasked(x, y);
				else if ((i == wide - 1) && !j)
					_gfxTR->drawMasked(x + i * 16, y + j * 16);
				else if (!j)
					_gfxTM->drawMasked(x + i * 16, y + j * 16);
				else {

					// Bottom Lines
					if ((j == high - 1) && !i)
						_gfxBL->drawMasked(x + i * 16, y + j * 16);
					else if ((j == high - 1) && (i == wide - 1))
						_gfxBR->drawMasked(x + i * 16, y + j * 16);
					else if ((j == high - 1) && i < wide)
						_gfxBM->drawMasked(x + i * 16, y + j * 16);
					else {
						// Middle lines
						if (!i && j < high)
							_gfxL->drawMasked(x + i * 16, y + j * 16);
						else if ((i == wide - 1) && j < high)
							_gfxR->drawMasked(x + i * 16, y + j * 16);
						else
							// Most drawn = middle block
							_gfxM->drawMasked(x + i * 16, y + j * 16);
					}
				}
			}
	} else {
		wide = (width / 16) + 1;
		high = height / 16;
		for (j = 0; j < high; j++)
			for (i = 0; i < wide; i++) {
				// Top Line
				if (!i && !j)
					_gGfxTL->drawMasked(x, y);
				else if ((i == wide - 1) && !j)
					_gGfxTR->drawMasked(x + i * 16, y + j * 16);
				else if (!j)
					_gGfxTM->draw(x + i * 16, y + j * 16);
				else {

					// Bottom Lines
					if ((j == high - 1) && !i)
						_gGfxBL->drawMasked(x + i * 16, y + j * 16);
					else if ((j == high - 1) && (i == wide - 1))
						_gGfxBR->drawMasked(x + i * 16, y + j * 16);
					else if ((j == high - 1) && i < wide)
						_gGfxBM->draw(x + i * 16, y + j * 16);
					else {

						// Middle lines
						if (!i && j < high)
							_gGfxL->draw(x + i * 16, y + j * 16);
						else if ((i == wide - 1) && j < high)
							_gGfxR->draw(x + i * 16, y + j * 16);
						else
							// Most drawn = middle block
							_gGfxM->draw(x + i * 16, y + j * 16);
					}
				}
			}
	}
}

void Window::setDialogDelay(int delay) {
	_dialogDelay = g_system->getMillis() + 1000 * delay;
}

void Window::openMessageBar(const char *msg, int count) {
	warning("STUB: Add openMessageBar()");
}

void Window::drawInventory() {
	int baseX, drawX, drawY;
	static uint32 timer = g_hdb->getTimeSlice() + 300;
	AIEntity *e, *sel;
	char string[8];
	int gems, mstones;

	// INFOBAR blit - only once per frame
	// note: if 2, don't draw ANY info at all
	if (_infobarDimmed > 1)
		return;

	_gfxInfobar->draw(kScreenWidth - _gfxInfobar->_width, 0);

	baseX = drawX = _invWinInfo.x;
	drawY = _invWinInfo.y;

	// Draw Inv Items
	sel = NULL;
	if (_invWinInfo.selection >= g_hdb->_ai->getInvAmount())
		_invWinInfo.selection = g_hdb->_ai->getInvAmount();

	for (int inv = 0; inv < g_hdb->_ai->getInvAmount(); inv++) {
		e = g_hdb->_ai->getInvItem(inv);
		if (inv == _invWinInfo.selection)
			sel = e;

		e->standdownGfx[0]->drawMasked(drawX, drawY);

		drawX += kInvItemSpaceX;
		if (drawX >= baseX + (kInvItemSpaceX * kInvItemPerLine)) {
			drawX = baseX;
			drawY += kInvItemSpaceY;
		}
	}

	// Draw the Gem
	drawY = _invWinInfo.y + kInvItemSpaceY * 4 - 8;
	drawX = baseX - 8;
	_gemGfx->drawMasked(drawX, drawY);

	// Draw the Gem Amount
	gems = g_hdb->_ai->getGemAmount();
	sprintf(string, "%d", gems);
	g_hdb->_drawMan->setCursor(drawX + 32, drawY + 8);
	g_hdb->_drawMan->drawText(string);

	// Draw the mini monkeystone
	mstones = g_hdb->_ai->getMonkeystoneAmount();
	if (mstones) {
		drawX = baseX + kInvItemSpaceX * 2 - 8;
		_mstoneGfx->drawMasked(drawX, drawY + 8);

		// Draw the monkeystone amount
		sprintf(string, "%d", mstones);
		g_hdb->_drawMan->setCursor(drawX + 28, drawY + 8);
		g_hdb->_drawMan->drawText(string);
	}

	// If you have an inventory, draw the selection cursor
	if (g_hdb->_ai->getInvAmount()) {
		if (_invWinInfo.selection < 0)
			_invWinInfo.selection = 0;

		// Draw the Inventory Select Cursor
		drawX = baseX + (_invWinInfo.selection % kInvItemPerLine) * kInvItemSpaceX;
		drawY = _invWinInfo.y + (_invWinInfo.selection / kInvItemPerLine) * kInvItemSpaceY;
		_gfxInvSelect->drawMasked(drawX, drawY);

		if (sel) {
			int centerX = baseX - 4 + (kScreenWidth - baseX) / 2;
			drawY = _invWinInfo.y + (kInvItemSpaceY * 4) + 16;
			g_hdb->_drawMan->setCursor(centerX - g_hdb->_drawMan->stringLength(sel->printedName) / 2, drawY);
			g_hdb->_drawMan->drawText(sel->printedName);
		}
	}
}

void Window::checkInvSelect(int x, int y) {
	if (x >= _invWinInfo.x && x < _invWinInfo.x + _invWinInfo.width && y >= _invWinInfo.y && y < _invWinInfo.y + _invWinInfo.height) {
		int xc = (x - _invWinInfo.x) / kInvItemSpaceX;
		int yc = (y - _invWinInfo.y) / kInvItemSpaceY;
		if (yc * kInvItemPerLine + xc > g_hdb->_ai->getInvAmount())
			return;

		_invWinInfo.selection = yc * kInvItemPerLine + xc;

		// If this is a weapon, choose it
		warning("STUB: checkInvSelect: ChooseWeapon() required");

		warning("STUB: checkInvSelect: Play SND_POP");
	}
}

void Window::textOut(const char *text, int x, int y, int timer) {
	TOut *t = new TOut;

	t->x = x;
	t->y = y;
	strcpy(t->text, text);
	t->timer = g_system->getMillis() + (uint32)(timer << 4);

	if (x < 0) {
		int pw, lines;
		g_hdb->_drawMan->getDimensions(t->text, &pw, &lines);
		t->x = kTextOutCenterX - pw / 2;
	}

	_textOutList.push_back(t);
}

void Window::centerTextOut(const char *text, int y, int timer) {
	int width, lines;
	g_hdb->_drawMan->getDimensions(text, &width, &lines);
	textOut(text, kTextOutCenterX - ((width - 8) >> 1), y, timer);
}

void Window::drawTextOut() {
	int e1, e2, e3, e4;
	uint32 time;
	TOut *t;

	if (_textOutList.empty())
		return;

	g_hdb->_drawMan->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_drawMan->setTextEdges(0, 480, 0, kScreenHeight);

	time = g_system->getMillis();
	
	for (uint i = 0; i < _textOutList.size(); i++) {
		t = _textOutList[i];
		g_hdb->_drawMan->setCursor(t->x, t->y);
		g_hdb->_drawMan->drawText(t->text);

		if (t->timer < time) {
			_textOutList.remove_at(i);
			i--;
		}
	}

	g_hdb->_drawMan->setTextEdges(e1, e2, e3, e4);
}

void Window::closeTextOut() {
	_textOutList.clear();
}
} // End of Namespace
