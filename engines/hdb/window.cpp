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

	_gfxTL = g_hdb->_gfx->loadPic("menu_back_topleft");
	_gfxTM = g_hdb->_gfx->loadPic("menu_back_top");
	_gfxTR = g_hdb->_gfx->loadPic("menu_back_topright");

	_gfxL = g_hdb->_gfx->loadPic("menu_back_left");
	_gfxM = g_hdb->_gfx->loadPic("menu_back_middle");
	_gfxR = g_hdb->_gfx->loadPic("menu_back_right");

	_gfxBL = g_hdb->_gfx->loadPic("menu_back_btmleft");
	_gfxBM = g_hdb->_gfx->loadPic("menu_back_bottom");
	_gfxBR = g_hdb->_gfx->loadPic("menu_back_btmright");

	_gfxTitleL = g_hdb->_gfx->loadPic("menu_title_left");
	_gfxTitleM = g_hdb->_gfx->loadPic("menu_title_mid");
	_gfxTitleR = g_hdb->_gfx->loadPic("menu_title_right");

	_gGfxTL = g_hdb->_gfx->loadPic("g_menu_back_topleft");
	_gGfxTM = g_hdb->_gfx->loadPic("g_menu_back_top");
	_gGfxTR = g_hdb->_gfx->loadPic("g_menu_back_topright");

	_gGfxL = g_hdb->_gfx->loadPic("g_menu_back_left");
	_gGfxM = g_hdb->_gfx->loadPic("g_menu_back_middle");
	_gGfxR = g_hdb->_gfx->loadPic("g_menu_back_right");

	_gGfxBL = g_hdb->_gfx->loadPic("g_menu_back_btmleft");
	_gGfxBM = g_hdb->_gfx->loadPic("g_menu_back_bottom");
	_gGfxBR = g_hdb->_gfx->loadPic("g_menu_back_btmright");

	_gGfxTitleL = g_hdb->_gfx->loadPic("g_menu_title_left");
	_gGfxTitleM = g_hdb->_gfx->loadPic("g_menu_title_mid");
	_gGfxTitleR = g_hdb->_gfx->loadPic("g_menu_title_right");

	_gfxIndent = g_hdb->_gfx->loadPic("menu_delivery_indentation");
	_gfxArrowTo = g_hdb->_gfx->loadPic("menu_arrow_deliverto");

	_gfxTry = _gfxAgain = NULL; // They will be loaded when needed

	_gfxInvSelect = g_hdb->_gfx->loadPic("inventory_normal");
	_gfxHandright = g_hdb->_gfx->loadPic("menu_hand_pointright");

	_gfxInfobar = g_hdb->_gfx->loadPic("pic_infobar");
	_gfxDarken = g_hdb->_gfx->loadPic("screen_darken");
	_gfxPausePlaque = g_hdb->_gfx->loadPic("pause_plaque");

	_infobarDimmed = 0;

	_invWinInfo.width = kInvItemSpaceX * 3;
	_invWinInfo.height = kInvItemSpaceY * 4;
	_invWinInfo.x = kScreenWidth - _gfxInfobar->_width + 16;
	_invWinInfo.y = 40;

	_dlvsInfo.width = kInvItemSpaceX * 3;
	_dlvsInfo.height = kInvItemSpaceY * 4;
	_dlvsInfo.x = (kScreenWidth - _gfxInfobar->_width) + 16;
	_dlvsInfo.y = 272;

	restartSystem();

	return true;
}

void Window::restartSystem() {
	_numMsgQueue = 0;
	_msgInfo.active = false;
	_dialogInfo.active = false;
	_dialogChoiceInfo.active = false;
	_invWinInfo.active = false;
	_dialogDelay = _invWinInfo.selection = 0;
	_gemGfx = g_hdb->_gfx->loadTile("ent_gem_white_sit01");
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
	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(kDialogTextLeft, kDialogTextRight, 0, 480);
	g_hdb->_gfx->getDimensions(string, &width, &height);
	g_hdb->_gfx->getDimensions(title, &titleWidth, &titleHeight);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);
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
	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(_dialogInfo.x + 10, 480, 0, kScreenHeight);
	g_hdb->_gfx->setCursor(0, _dialogInfo.y - 7);
	if (_dialogInfo.title)
		g_hdb->_gfx->drawText(_dialogInfo.title);
	g_hdb->_gfx->setTextEdges(_dialogInfo.x + 16, _dialogInfo.x + _dialogInfo.width - 16, 0, kScreenHeight);
	g_hdb->_gfx->setCursor(0, _dialogInfo.y + 16);
	if (_dialogInfo.string)
		g_hdb->_gfx->drawText(_dialogInfo.string);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);
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

void Window::openDialogChoice(const char *title, const char *text, const char *func, int numChoices, const char *choices[10]) {
	int		width, height, titleWidth, titleHeight;
	int		e1, e2, e3, e4, i;

	if (true == _dialogInfo.active)
		return;

	memset(&_dialogChoiceInfo, 0, sizeof(_dialogChoiceInfo));
	strcpy(_dialogChoiceInfo.title, title);
	strcpy(_dialogChoiceInfo.text, text);
	strcpy(_dialogChoiceInfo.func, func);
	_dialogChoiceInfo.numChoices = numChoices;

	for (i = 0; i < numChoices; i++)
		strcpy(_dialogChoiceInfo.choices[i], choices[i]);
	_dialogChoiceInfo.active = true;

	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(kOpenDialogTextLeft, kOpenDialogTextRight, 0, 480);
	g_hdb->_gfx->getDimensions(text, &width, &height);
	g_hdb->_gfx->getDimensions(title, &titleWidth, &titleHeight);

	for (i = 0; i < 10; i++)
		if (choices[i]) {
			int	w, h;
			g_hdb->_gfx->getDimensions(choices[i], &w, &h);
			if (w > width)
				width = w;
		}

	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);
	_dialogChoiceInfo.textHeight = (height + 1) * 16;
	_dialogChoiceInfo.height = (height + 2 + numChoices) * 16;
	_dialogChoiceInfo.width = width + 48;
	_dialogChoiceInfo.titleWidth = titleWidth;

	_dialogChoiceInfo.x = (480 >> 1) - (_dialogChoiceInfo.width >> 1);
	_dialogChoiceInfo.y = (kScreenHeight >> 1) - ((_dialogChoiceInfo.height >> 1) + 32);
	if (_dialogChoiceInfo.y < 0)
		_dialogChoiceInfo.y = 0;

	_dialogChoiceInfo.selection = 0;
	_dialogChoiceInfo.timeout = 0;
	warning("STUB: Play SND_MOVE_SELECTION");
}

void Window::drawDialogChoice() {
	int		e1, e2, e3, e4, blocks, i, w;

	if (!_dialogChoiceInfo.active)
		return;

	// time out?
	if (_dialogChoiceInfo.timeout && _dialogChoiceInfo.timeout < g_hdb->getTimeSlice()) {
		closeDialogChoice();
		return;
	}

	bool guyTalking = !scumm_stricmp(_dialogChoiceInfo.title, "guy");

	w = _dialogChoiceInfo.width;
	if (_dialogChoiceInfo.titleWidth > w)
		w = _dialogChoiceInfo.titleWidth;

	drawBorder(_dialogChoiceInfo.x, _dialogChoiceInfo.y, w, _dialogChoiceInfo.height, guyTalking);

	if (!guyTalking) {
		_gfxTitleL->drawMasked(_dialogChoiceInfo.x, _dialogChoiceInfo.y - 10);
		blocks = _dialogChoiceInfo.titleWidth / 16;
		for (i = 0; i < blocks; i++)
			_gfxTitleM->drawMasked(_dialogChoiceInfo.x + 16 * (i + 1), _dialogChoiceInfo.y - 10);
		_gfxTitleR->drawMasked(_dialogChoiceInfo.x + (blocks + 1) * 16, _dialogChoiceInfo.y - 10);
	} else {
		_gGfxTitleL->drawMasked(_dialogChoiceInfo.x, _dialogChoiceInfo.y - 10);
		blocks = _dialogChoiceInfo.titleWidth / 16;
		for (i = 0; i < blocks; i++)
			_gGfxTitleM->drawMasked(_dialogChoiceInfo.x + 16 * (i + 1), _dialogChoiceInfo.y - 10);
		_gGfxTitleR->drawMasked(_dialogChoiceInfo.x + (blocks + 1) * 16, _dialogChoiceInfo.y - 10);
	}

	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(_dialogChoiceInfo.x + 10, kOpenDialogTextRight, 0, 480);
	g_hdb->_gfx->setCursor(0, _dialogChoiceInfo.y - 7);
	if (_dialogChoiceInfo.title)
		g_hdb->_gfx->drawText(_dialogChoiceInfo.title);
	g_hdb->_gfx->setTextEdges(_dialogChoiceInfo.x + 16, kOpenDialogTextRight, 0, 480);
	g_hdb->_gfx->setCursor(0, _dialogChoiceInfo.y + 16);
	if (_dialogChoiceInfo.text)
		g_hdb->_gfx->drawText(_dialogChoiceInfo.text);

	for (i = 0; i < _dialogChoiceInfo.numChoices; i++) {
		g_hdb->_gfx->setCursor(_dialogChoiceInfo.x + 48, _dialogChoiceInfo.y + _dialogChoiceInfo.textHeight + 16 * i);
		g_hdb->_gfx->drawText(_dialogChoiceInfo.choices[i]);
	}
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);

	_gfxHandright->drawMasked(_dialogChoiceInfo.x + 10, 4 + _dialogChoiceInfo.y + _dialogChoiceInfo.textHeight + 16 * _dialogChoiceInfo.selection);
}

void Window::closeDialogChoice() {
	if (_dialogChoiceInfo.active) {
		_dialogChoiceInfo.active = false;
		g_hdb->_lua->pushFunction(_dialogChoiceInfo.func);
		g_hdb->_lua->pushInt(_dialogChoiceInfo.selection);
		g_hdb->_lua->call(1, 0);
		warning("STUB: Play SND_SWITCH_USE");
	}
}

bool Window::checkDialogChoiceClose(int x, int y) {
	if (!_dialogChoiceInfo.active || _dialogChoiceInfo.timeout)
		return false;

	if (x >= _dialogChoiceInfo.x && x < _dialogChoiceInfo.x + _dialogChoiceInfo.width &&
		y >= _dialogChoiceInfo.y + _dialogChoiceInfo.textHeight && y < _dialogChoiceInfo.y + _dialogChoiceInfo.textHeight + _dialogChoiceInfo.numChoices * 16) {
		warning("STUB: Play SND_SWITCH_USE");
		_dialogChoiceInfo.selection = (y - (_dialogChoiceInfo.y + _dialogChoiceInfo.textHeight)) >> 4;
		_dialogChoiceInfo.timeout = g_hdb->getTimeSlice() + 500;
		return true;
	}

	return false;
}

void Window::dialogChoiceMoveup() {
	_dialogChoiceInfo.selection--;
	if (_dialogChoiceInfo.selection < 0)
		_dialogChoiceInfo.selection = _dialogChoiceInfo.numChoices - 1;
	warning("STUB: Play SND_MOVE_SELECTION");
}

void Window::dialogChoiceMovedown() {
	_dialogChoiceInfo.selection++;
	if (_dialogChoiceInfo.selection >= _dialogChoiceInfo.numChoices)
		_dialogChoiceInfo.selection = 0;
	warning("STUB: Play SND_MOVE_SELECTION");
}

void Window::openMessageBar(const char *title, int time) {
	int	width, height;
	int	e1, e2, e3, e4;

	// is the messagebar already up?  if so, add this msg to the queue
	if (_msgInfo.active) {
		if (_numMsgQueue < kMaxMsgQueue) {
			int		i;
			if (!scumm_stricmp(_msgInfo.title, title))
				return;

			for (i = 0; i < _numMsgQueue; i++)
				if (!scumm_stricmp(_msgQueueStr[i], title))
					return;
			strcpy(_msgQueueStr[_numMsgQueue], title);
			_msgQueueWait[_numMsgQueue] = time;
			_numMsgQueue++;
		}
		return;
	}

	memset(&_msgInfo, 0, sizeof(_msgInfo));

	_msgInfo.timer = (time * kGameFPS);
	strcpy(_msgInfo.title, title);

	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(kDialogTextLeft, kDialogTextRight, 0, 480);
	g_hdb->_gfx->getDimensions(title, &width, &height);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);

	_msgInfo.height = (height + 2) * 16;
	_msgInfo.width = width + 32;

	_msgInfo.x = (480 >> 1) - (_msgInfo.width >> 1);
	_msgInfo.active = true;
}

void Window::drawMessageBar() {
	int	xx, py, my;
	int	e1, e2, e3, e4;

	// if msgbar's not up OR inventory is up, exit
	if (!_msgInfo.active || _invWinInfo.active || _dialogInfo.active)
		return;

	g_hdb->_ai->getPlayerXY(&xx, &py);	// don't care about the x
	g_hdb->_map->getMapXY(&xx, &my);
	_msgInfo.y = (py - my) - _msgInfo.height - 64;	// put msgbar directly above player
	if (_msgInfo.y < _msgInfo.height)
		_msgInfo.y = (py - my) + 40;					// if at top, but it directly below

	drawBorder(_msgInfo.x, _msgInfo.y, _msgInfo.width, _msgInfo.height, false);

	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(_msgInfo.x + 16, _msgInfo.x + _msgInfo.width - 16, 0, 320);
	g_hdb->_gfx->setCursor(_msgInfo.x + 16, _msgInfo.y + 16);
	g_hdb->_gfx->drawText(_msgInfo.title);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);

	//
	// time to go away?  see if we have any more msgs in the queue...
	//
	if (_msgInfo.timer-- < 1)
		nextMsgQueued();
}

bool Window::checkMsgClose(int x, int y) {
	if (x >= _msgInfo.x && x < _msgInfo.x + _msgInfo.width &&
		y >= _msgInfo.y && y < _msgInfo.y + _msgInfo.height) {
		closeMsg();
		return true;
	}

	return false;
}

void Window::nextMsgQueued() {
	int		xx;				// frameskip COULD be 0!
	int		width, height;
	int		e1, e2, e3, e4;

	if (!_numMsgQueue) {
		_msgInfo.active = false;
		return;
	}

	strcpy(_msgInfo.title, _msgQueueStr[0]);
	_msgInfo.timer = (_msgQueueWait[0] * kGameFPS);

	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(kDialogTextLeft, kDialogTextRight, 0, 480);
	g_hdb->_gfx->getDimensions(_msgInfo.title, &width, &height);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);

	_msgInfo.height = (height + 2) * 16;

	_msgInfo.width = width + 32;
	_msgInfo.x = (480 >> 1) - (_msgInfo.width >> 1);
	_msgInfo.y = (kScreenHeight >> 2) - (_msgInfo.height >> 1);

	for (xx = 0; xx < _numMsgQueue - 1; xx++)
	{
		strcpy(_msgQueueStr[xx], _msgQueueStr[xx + 1]);
		_msgQueueWait[xx] = _msgQueueWait[xx + 1];
	}
	_numMsgQueue--;
	_msgInfo.active = true;
}

void Window::closeMsg() {
	nextMsgQueued();
	warning("STUB: Play SND_DIALOG_CLOSE");
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
	g_hdb->_gfx->setCursor(drawX + 32, drawY + 8);
	g_hdb->_gfx->drawText(string);

	// Draw the mini monkeystone
	mstones = g_hdb->_ai->getMonkeystoneAmount();
	if (mstones) {
		drawX = baseX + kInvItemSpaceX * 2 - 8;
		_mstoneGfx->drawMasked(drawX, drawY + 8);

		// Draw the monkeystone amount
		sprintf(string, "%d", mstones);
		g_hdb->_gfx->setCursor(drawX + 28, drawY + 8);
		g_hdb->_gfx->drawText(string);
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
			g_hdb->_gfx->setCursor(centerX - g_hdb->_gfx->stringLength(sel->printedName) / 2, drawY);
			g_hdb->_gfx->drawText(sel->printedName);
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

void Window::openDeliveries(bool animate) {
	DlvEnt *d;

	// Load Gfx
	for (int i = 0; i < g_hdb->_ai->getDeliveriesAmount(); i++) {
		d = g_hdb->_ai->getDeliveryItem(i);
		if (d->itemGfxName[0])
			d->itemGfx = g_hdb->_gfx->loadTile(d->itemGfxName);
		if (d->destGfxName[0])
			d->destGfx = g_hdb->_gfx->loadTile(d->destGfxName);
	}

	warning("STUB: Play SND_POP");
	_dlvsInfo.animate = animate;
	_dlvsInfo.delay1 = g_hdb->getTimeSlice() + 500;
	_dlvsInfo.go1 = _dlvsInfo.go2 = _dlvsInfo.go3 = false;

	if (animate) {
		_dlvsInfo.go1 = true;
		_dlvsInfo.selected = g_hdb->_ai->getDeliveriesAmount() - 1;
	}

	// Make sure cursor isn't on an empty delivery
	if (_dlvsInfo.selected >= g_hdb->_ai->getDeliveriesAmount())
		_dlvsInfo.selected = g_hdb->_ai->getDeliveriesAmount() - 1;
}

void Window::drawDeliveries() {
	int baseX, drawX, drawY;
	DlvEnt *d;
	static uint32 timer = g_hdb->getTimeSlice() + 300;

	debug(9, "STUB: Add Crazy Sounds");

	if (_infobarDimmed > 1)
		return;

	baseX = drawX = _dlvsInfo.x;
	drawY = _dlvsInfo.y;

	if (_dlvsInfo.selected >= g_hdb->_ai->getDeliveriesAmount())
		_dlvsInfo.selected = g_hdb->_ai->getDeliveriesAmount() - 1;

	// Draw Delivery Items
	int inv;
	for (inv = 0; inv < g_hdb->_ai->getDeliveriesAmount(); inv++) {
		int centerX = baseX + (kScreenWidth - baseX) / 2;
		d = g_hdb->_ai->getDeliveryItem(inv);
		if (_dlvsInfo.animate && inv == g_hdb->_ai->getDeliveriesAmount() - 1) {
			if (_dlvsInfo.go1) {
				if (_dlvsInfo.delay1 < g_hdb->getTimeSlice()) {
					// Draw Item
					_gfxIndent->draw(drawX, drawY);
					d->itemGfx->drawMasked(drawX, drawY);

					g_hdb->_gfx->setCursor(centerX - g_hdb->_gfx->stringLength(d->itemTextName) / 2, kDlvItemTextY);
					g_hdb->_gfx->drawText(d->itemTextName);
					if (!_dlvsInfo.go2) {
						_dlvsInfo.go2 = true;
						_dlvsInfo.delay2 = g_hdb->getTimeSlice() + 500;
						warning("STUB: Play crazy sound");
					}
				}
			}
			if (_dlvsInfo.go2) {
				if (_dlvsInfo.delay2 < g_hdb->getTimeSlice()) {
					// Draw TO
					_gfxArrowTo->drawMasked(_dlvsInfo.x + kDlvItemSpaceX * _dlvsInfo.selected + 8, drawY + kTileHeight);

					g_hdb->_gfx->setCursor(centerX - g_hdb->_gfx->stringLength("to") / 2, kDlvItemTextY + 12);
					g_hdb->_gfx->drawText("to");
					if (!_dlvsInfo.go3) {
						_dlvsInfo.go3 = true;
						_dlvsInfo.delay3 = g_hdb->getTimeSlice() + 500;
						warning("STUB: Play crazy sound");
					}
				}
			}
			if (_dlvsInfo.go3) {
				if (_dlvsInfo.delay3 < g_hdb->getTimeSlice()) {
					// Draw Delivery
					_gfxIndent->draw(drawX, drawY + kTileHeight + 16);
					d->destGfx->drawMasked(drawX, drawY + kTileHeight + 16);

					g_hdb->_gfx->setCursor(centerX - (g_hdb->_gfx->stringLength(d->destTextName) + g_hdb->_gfx->stringLength("to")) / 2, kDlvItemTextY + 12);
					g_hdb->_gfx->drawText("to ");
					g_hdb->_gfx->drawText(d->destTextName);

					warning("STUB: Play crazy sound");
					_dlvsInfo.animate = false;
				}
			}

		} else {
			// Draw Item
			_gfxIndent->draw(drawX, drawY);
			d->itemGfx->drawMasked(drawX, drawY);
			// Draw Delivery
			_gfxIndent->draw(drawX, drawY + kTileHeight + 16);
			d->destGfx->drawMasked(drawX, drawY + kTileHeight + 16);

			if (!_dlvsInfo.animate && inv == _dlvsInfo.selected) {
				g_hdb->_gfx->setCursor(centerX - g_hdb->_gfx->stringLength(d->itemTextName)/2, kDlvItemTextY);
				g_hdb->_gfx->drawText(d->itemTextName);
				g_hdb->_gfx->setCursor(centerX - (g_hdb->_gfx->stringLength(d->destTextName) + g_hdb->_gfx->stringLength("to ")) / 2, kDlvItemTextY + 12);
				g_hdb->_gfx->drawText("to ");
				g_hdb->_gfx->drawText(d->destTextName);
			}

			drawX += kDlvItemSpaceX;
			if (drawX >= kScreenWidth) {
				drawX = baseX;
				drawY += kDlvItemSpaceY + 8;
			}
		}
	}

	// Draw "No Deliveries" or the arrow that points to the currently selected one
	if (!inv) {
		g_hdb->_gfx->setCursor(baseX + 16, _dlvsInfo.y);
		g_hdb->_gfx->drawText("No Deliveries");
	} else if (!_dlvsInfo.animate) {
		int dx, dy, rowtwo;

		rowtwo = _dlvsInfo.selected > 2;
		dx = 8 + _dlvsInfo.x + kDlvItemSpaceX * (_dlvsInfo.selected % 3);
		dy = _dlvsInfo.y + kTileHeight + (kDlvItemSpaceY + 8) * rowtwo;
		_gfxArrowTo->drawMasked(dx, dy);
	}

	// If the infobar is dimmed out, this where we dim the whole thing
	if (_infobarDimmed) {
		for (int j = 0; j < kScreenHeight; j += kTileHeight) {
			for (int i = (kScreenWidth - _gfxInfobar->_width); i < kScreenWidth; i += kTileWidth)
				_gfxDarken->drawMasked(i, j);
		}
	}
}

void Window::setSelectedDelivery(int which) {
	_dlvsInfo.selected = which;
	warning("STUB: Play SND_MENU_SLIDER");
}

void Window::checkDlvSelect(int x, int y) {
	int xc, yc;

	if (_dlvsInfo.animate)
		return;

	int amt = g_hdb->_ai->getDeliveriesAmount();

	// Click on a Delivery to select it for inspection?
	if (x >= _dlvsInfo.x && x < _dlvsInfo.x + _dlvsInfo.width && y >= _dlvsInfo.y && y < _dlvsInfo.y + _dlvsInfo.height) {
		xc = (x - _dlvsInfo.x) / kDlvItemSpaceX;
		yc = (y - _dlvsInfo.y) / kDlvItemSpaceY;
		int value = yc * kDlvItemPerLine + xc;
		if (value < amt)
			setSelectedDelivery(value);
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
		g_hdb->_gfx->getDimensions(t->text, &pw, &lines);
		t->x = kTextOutCenterX - pw / 2;
	}

	_textOutList.push_back(t);
}

void Window::centerTextOut(const char *text, int y, int timer) {
	int width, lines;
	g_hdb->_gfx->getDimensions(text, &width, &lines);
	textOut(text, kTextOutCenterX - ((width - 8) >> 1), y, timer);
}

void Window::drawTextOut() {
	int e1, e2, e3, e4;
	uint32 time;
	TOut *t;

	if (_textOutList.empty())
		return;

	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(0, 480, 0, kScreenHeight);

	time = g_system->getMillis();

	for (uint i = 0; i < _textOutList.size(); i++) {
		t = _textOutList[i];
		g_hdb->_gfx->setCursor(t->x, t->y);
		g_hdb->_gfx->drawText(t->text);

		if (t->timer < time) {
			_textOutList.remove_at(i);
			i--;
		}
	}

	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);
}

void Window::closeTextOut() {
	_textOutList.clear();
}
} // End of Namespace
