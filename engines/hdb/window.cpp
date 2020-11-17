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

#include "common/random.h"

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/gfx.h"
#include "hdb/lua-script.h"
#include "hdb/map.h"
#include "hdb/mpc.h"
#include "hdb/sound.h"
#include "hdb/window.h"

namespace HDB {

Window::Window() {
	if (g_hdb->isPPC()) {
		_weaponX = (g_hdb->_screenWidth - 34);
		_weaponY = 2;
		_invItemSpace = 36;
		_invItemPerLine = 5;
		_textOutCenterX = 120;
		_dlvItemSpaceX = 0;
		_dlvItemSpaceY = 0;
		_dlvItemPerLine = 0;
		_dlvItemTextY = 0;
		_dialogTextLeft = 8;
		_dialogTextRight = 208;
		_openDialogTextLeft = 0;
		_openDialogTextRight = 228;
		_tryY1 = 60;	// TRY
		_tryY2 = 100;			// AGAIN
		_tryRestartY = 240;	// (ok)
		_panicXStop = g_hdb->_screenDrawWidth / 4;
		_panicZoneFaceX = g_hdb->_screenDrawWidth - 32;

		// Fields not used in the PPC version
		_invItemSpaceX = 1;
		_invItemSpaceY = 1;
	} else {
		_weaponX = (480 - 34);
		_weaponY = 2;
		_invItemSpaceX = 48;
		_invItemSpaceY = 40;
		_invItemPerLine = 3;
		_textOutCenterX = ((g_hdb->_screenWidth - kTileWidth * 5) / 2);
		_dlvItemSpaceX = 48;
		_dlvItemSpaceY = kTileHeight * 2 + 16;
		_dlvItemPerLine = 3;
		_dlvItemTextY = (g_hdb->_screenHeight - 30);
		_dialogTextLeft = 64;
		_dialogTextRight = _dialogTextLeft + kTileWidth * 9;
		_openDialogTextLeft = _dialogTextLeft;
		_openDialogTextRight = _dialogTextRight + kTileWidth * 2;
		_tryY1 = (g_hdb->_screenHeight >> 2);	// TRY
		_tryY2 = (_tryY1 + 32);			// AGAIN
		_tryRestartY = ((g_hdb->_screenHeight >> 2) * 3);	// (ok)
		_panicXStop = g_hdb->_screenDrawWidth / 3;
		_panicZoneFaceX = g_hdb->_screenDrawWidth - 32;
		// Fields only used in the PPC version
		_invItemSpace = 0;
	}

	_pauseY = (g_hdb->_screenHeight / 2 - 64);

	_gGfxTM = nullptr;
	_gGfxTR = nullptr;
	_gGfxL = nullptr;
	_gGfxM = nullptr;
	_gGfxR = nullptr;
	_gGfxBL = nullptr;
	_gGfxBM = nullptr;
	_gGfxBR = nullptr;
	_gGfxTitleL = nullptr;
	_gGfxTitleM = nullptr;
	_gGfxTitleR = nullptr;
	_gfxIndent = nullptr;
	_gfxArrowTo = nullptr;
	_gfxHandright = nullptr;
	_gfxTry = nullptr;
	_gfxAgain = nullptr;
	_gfxInvSelect = nullptr;
	_gfxLevelRestart = nullptr;
	_gfxPausePlaque = nullptr;
	_gemGfx = nullptr;
	_mstoneGfx = nullptr;
	_gfxResources = nullptr;
	_gfxDeliveries = nullptr;
	_gfxInfobar = nullptr;
	_gfxDarken = nullptr;
	_infobarDimmed = 0;
	_dialogDelay = 0;
	_numMsgQueue = 0;
	_gfxTL = nullptr;
	_gfxTM = nullptr;
	_gfxTR = nullptr;
	_gfxL = nullptr;
	_gfxM = nullptr;
	_gfxR = nullptr;
	_gfxBL = nullptr;
	_gfxBM = nullptr;
	_gfxBR = nullptr;
	_gfxTitleL = nullptr;
	_gfxTitleM = nullptr;
	_gfxTitleR = nullptr;
	_gGfxTL = nullptr;
}

Window::~Window() {
	delete _gfxTL;
	delete _gfxTM;
	delete _gfxTR;

	delete _gfxL;
	delete _gfxM;
	delete _gfxR;

	delete _gfxBL;
	delete _gfxBM;
	delete _gfxBR;

	delete _gfxTitleL;
	delete _gfxTitleM;
	delete _gfxTitleR;

	delete _gGfxTL;
	delete _gGfxTM;
	delete _gGfxTR;

	delete _gGfxL;
	delete _gGfxM;
	delete _gGfxR;

	delete _gGfxBL;
	delete _gGfxBM;
	delete _gGfxBR;

	delete _gGfxTitleL;
	delete _gGfxTitleM;
	delete _gGfxTitleR;

	delete _gfxIndent;
	delete _gfxArrowTo;

	delete _gfxInvSelect;
	delete _gfxHandright;

	if (g_hdb->isPPC()) {
		delete _gfxResources;
		delete _gfxDeliveries;
	} else {
		delete _gfxInfobar;
		delete _gfxDarken;
		delete _gfxPausePlaque;
		delete _mstoneGfx;
	}


	delete _gemGfx;
}

void Window::init() {
	_gfxTL = g_hdb->_gfx->loadPic(MENU_BACK_TOPLEFT);
	_gfxTM = g_hdb->_gfx->loadPic(MENU_BACK_TOP);
	_gfxTR = g_hdb->_gfx->loadPic(MENU_BACK_TOPRIGHT);

	_gfxL = g_hdb->_gfx->loadPic(MENU_BACK_LEFT);
	_gfxM = g_hdb->_gfx->loadPic(MENU_BACK_MIDDLE);
	_gfxR = g_hdb->_gfx->loadPic(MENU_BACK_RIGHT);

	_gfxBL = g_hdb->_gfx->loadPic(MENU_BACK_BTMLEFT);
	_gfxBM = g_hdb->_gfx->loadPic(MENU_BACK_BOTTOM);
	_gfxBR = g_hdb->_gfx->loadPic(MENU_BACK_BTMRIGHT);

	_gfxTitleL = g_hdb->_gfx->loadPic(MENU_TITLE_LEFT);
	_gfxTitleM = g_hdb->_gfx->loadPic(MENU_TITLE_MID);
	_gfxTitleR = g_hdb->_gfx->loadPic(MENU_TITLE_RIGHT);

	_gGfxTL = g_hdb->_gfx->loadPic(G_MENU_BACK_TOPLEFT);
	_gGfxTM = g_hdb->_gfx->loadPic(G_MENU_BACK_TOP);
	_gGfxTR = g_hdb->_gfx->loadPic(G_MENU_BACK_TOPRIGHT);

	_gGfxL = g_hdb->_gfx->loadPic(G_MENU_BACK_LEFT);
	_gGfxM = g_hdb->_gfx->loadPic(G_MENU_BACK_MIDDLE);
	_gGfxR = g_hdb->_gfx->loadPic(G_MENU_BACK_RIGHT);

	_gGfxBL = g_hdb->_gfx->loadPic(G_MENU_BACK_BTMLEFT);
	_gGfxBM = g_hdb->_gfx->loadPic(G_MENU_BACK_BOTTOM);
	_gGfxBR = g_hdb->_gfx->loadPic(G_MENU_BACK_BTMRIGHT);

	_gGfxTitleL = g_hdb->_gfx->loadPic(G_MENU_TITLE_LEFT);
	_gGfxTitleM = g_hdb->_gfx->loadPic(G_MENU_TITLE_MID);
	_gGfxTitleR = g_hdb->_gfx->loadPic(G_MENU_TITLE_RIGHT);

	_gfxIndent = g_hdb->_gfx->loadPic(MENU_DELIVERY_INDENTATION);
	_gfxArrowTo = g_hdb->_gfx->loadPic(MENU_ARROW_DELIVERTO);

	_gfxTry = _gfxAgain = nullptr; // They will be loaded when needed

	_gfxInvSelect = g_hdb->_gfx->loadPic(INVENTORY_NORMAL);
	_gfxHandright = g_hdb->_gfx->loadPic(MENU_HAND_POINTRIGHT);

	if (g_hdb->isPPC()) {

		_gfxResources = g_hdb->_gfx->loadPic(MENU_TITLE_RESOURCES);
		_gfxDeliveries = g_hdb->_gfx->loadPic(MENU_TITLE_DELIVERIES);

		_invWinInfo.width = g_hdb->_screenWidth - 32;
		_invWinInfo.height = 112;

		_dlvsInfo.width = g_hdb->_screenWidth - 48;
		_dlvsInfo.height = 160;
		_dlvsInfo.selected = 0;

	} else {
		_gfxInfobar = g_hdb->_gfx->loadPic(PIC_INFOBAR);
		_gfxDarken = g_hdb->_gfx->loadPic(SCREEN_DARKEN);
		_gfxPausePlaque = g_hdb->_gfx->loadPic(PAUSE_PLAQUE);
		_mstoneGfx = g_hdb->_gfx->loadPic(MINI_MSTONE);

		_infobarDimmed = 0;

		_invWinInfo.width = _invItemSpaceX * 3;
		_invWinInfo.height = _invItemSpaceY * 4;
		_invWinInfo.x = g_hdb->_screenWidth - _gfxInfobar->_width + 16;
		_invWinInfo.y = 40;

		_dlvsInfo.width = _invItemSpaceX * 3;
		_dlvsInfo.height = _invItemSpaceY * 4;
		_dlvsInfo.x = (g_hdb->_screenWidth - _gfxInfobar->_width) + 16;
		_dlvsInfo.y = 272;
	}

	_gemGfx = nullptr;

	restartSystem();
}

void Window::save(Common::OutSaveFile *out) {

	int i;

	// Save out the various window and game state info

	// clear out gfx ptrs in _pzInfo struct before writing...

	// Copy, clear and save Panic Zone Info
	memcpy(&_tempPzInfo, &_pzInfo, sizeof(_pzInfo));

	for (i = 0; i < 10; i++) {
		_tempPzInfo.gfxNumber[i] = nullptr;
		if (i < 2)
			_tempPzInfo.gfxFace[i] = nullptr;
	}
	_tempPzInfo.gfxPanic = _tempPzInfo.gfxZone = nullptr;

	out->writeByte(_tempPzInfo.active);
	out->writeSint32LE(_tempPzInfo.sequence);
	out->writeSint32LE(_tempPzInfo.timer);
	out->writeSint32LE(_tempPzInfo.x1);
	out->writeSint32LE(_tempPzInfo.y1);
	out->writeSint32LE(_tempPzInfo.x2);
	out->writeSint32LE(_tempPzInfo.y2);
	out->writeSint32LE(_tempPzInfo.xv);
	out->writeSint32LE(_tempPzInfo.yv);
	out->writeSint32LE(_tempPzInfo.numberTime);
	out->writeSint32LE(_tempPzInfo.numberTimeMaster);
	out->writeSint32LE(_tempPzInfo.numberValue);

	// Save Dialog Info
	out->write(_dialogInfo.title, 64);
	out->writeSint32LE(_dialogInfo.tileIndex);
	out->write(_dialogInfo.string, 160);
	out->writeByte(_dialogInfo.active);
	out->writeSint32LE(_dialogInfo.x);
	out->writeSint32LE(_dialogInfo.y);
	out->writeSint32LE(_dialogInfo.width);
	out->writeSint32LE(_dialogInfo.height);
	out->writeSint32LE(_dialogInfo.titleWidth);
	out->writeSint32LE(_dialogInfo.more);
	out->writeSint32LE(_dialogInfo.el);
	out->writeSint32LE(_dialogInfo.er);
	out->writeSint32LE(_dialogInfo.et);
	out->writeSint32LE(_dialogInfo.eb);
	out->write(_dialogInfo.luaMore, 64);

	// Save Dialog Delay
	out->writeSint32LE(_dialogDelay);

	// Save Dialog Choice Info
	out->write(_dialogChoiceInfo.title, 64);
	out->write(_dialogChoiceInfo.text, 160);
	out->write(_dialogChoiceInfo.func, 64);
	out->writeByte(_dialogChoiceInfo.active);
	out->writeSint32LE(_dialogChoiceInfo.x);
	out->writeSint32LE(_dialogChoiceInfo.y);
	out->writeSint32LE(_dialogChoiceInfo.width);
	out->writeSint32LE(_dialogChoiceInfo.height);
	out->writeSint32LE(_dialogChoiceInfo.textHeight);
	out->writeSint32LE(_dialogChoiceInfo.titleWidth);
	out->writeSint32LE(_dialogChoiceInfo.el);
	out->writeSint32LE(_dialogChoiceInfo.er);
	out->writeSint32LE(_dialogChoiceInfo.et);
	out->writeSint32LE(_dialogChoiceInfo.eb);
	out->writeUint32LE(_dialogChoiceInfo.timeout);
	out->writeSint32LE(_dialogChoiceInfo.selection);
	out->writeSint32LE(_dialogChoiceInfo.numChoices);

	for (i = 0; i < 10; i++) {
		out->write(_dialogChoiceInfo.choices[i], 64);
	}

	// Save Msg Info
	out->writeByte(_msgInfo.active);
	out->write(_msgInfo.title, 128);
	out->writeSint32LE(_msgInfo.timer);
	out->writeSint32LE(_msgInfo.x);
	out->writeSint32LE(_msgInfo.y);
	out->writeSint32LE(_msgInfo.width);
	out->writeSint32LE(_msgInfo.height);

	for (i = 0; i < kMaxMsgQueue; i++)
		out->write(_msgQueueStr[i], 128);
	for (i = 0; i < kMaxMsgQueue; i++)
		out->writeSint32LE(_msgQueueWait[i]);
	out->writeSint32LE(_numMsgQueue);

	// Save Inventory Info
	out->writeSint32LE(_invWinInfo.x);
	out->writeSint32LE(_invWinInfo.y);
	out->writeSint32LE(_invWinInfo.width);
	out->writeSint32LE(_invWinInfo.height);
	out->writeSint32LE(_invWinInfo.selection);
	out->writeByte(_invWinInfo.active);

	// Save Deliveries Info
	out->writeSint32LE(_dlvsInfo.x);
	out->writeSint32LE(_dlvsInfo.y);
	out->writeSint32LE(_dlvsInfo.width);
	out->writeSint32LE(_dlvsInfo.height);
	out->writeByte(_dlvsInfo.active);
	out->writeSint32LE(_dlvsInfo.selected);
	out->writeByte(_dlvsInfo.animate);
	out->writeUint32LE(_dlvsInfo.delay1);
	out->writeUint32LE(_dlvsInfo.delay2);
	out->writeUint32LE(_dlvsInfo.delay3);
	out->writeByte(_dlvsInfo.go1);
	out->writeByte(_dlvsInfo.go2);
	out->writeByte(_dlvsInfo.go3);

	// Save Try Again Info
	out->writeDoubleLE(_tryAgainInfo.y1);
	out->writeDoubleLE(_tryAgainInfo.y2);
	out->writeDoubleLE(_tryAgainInfo.yv1);
	out->writeDoubleLE(_tryAgainInfo.yv2);
	out->writeDoubleLE(_tryAgainInfo.yv1v);
	out->writeDoubleLE(_tryAgainInfo.yv2v);
	out->writeDoubleLE(_tryAgainInfo.x1);
	out->writeDoubleLE(_tryAgainInfo.x2);

	// Save TextOut Info
	out->writeUint32LE(_textOutList.size());
	for (i = 0; (uint)i < _textOutList.size(); i++) {
		out->write(_textOutList[i]->text, 128);
		out->writeSint32LE(_textOutList[i]->x);
		out->writeSint32LE(_textOutList[i]->y);
		out->writeUint32LE(_textOutList[i]->timer);
	}

	out->writeSint32LE(_infobarDimmed);
}

void Window::loadSaveFile(Common::InSaveFile *in) {

	int i;

	// Clear out everything
	restartSystem();

	// Load out various Window and Game State Info

	// Load Panic Zone Info
	for (i = 0; i < 10; i++) {
		_pzInfo.gfxNumber[i] = nullptr;
		if (i < 2)
			_pzInfo.gfxFace[i] = nullptr;
	}
	_pzInfo.gfxPanic = _pzInfo.gfxZone = nullptr;

	_pzInfo.active = in->readByte();
	_pzInfo.sequence = in->readSint32LE();
	_pzInfo.timer = in->readSint32LE();
	_pzInfo.x1 = in->readSint32LE();
	_pzInfo.y1 = in->readSint32LE();
	_pzInfo.x2 = in->readSint32LE();
	_pzInfo.y2 = in->readSint32LE();
	_pzInfo.xv = in->readSint32LE();
	_pzInfo.yv = in->readSint32LE();
	_pzInfo.numberTime = in->readSint32LE();
	_pzInfo.numberTimeMaster = in->readSint32LE();
	_pzInfo.numberValue = in->readSint32LE();

	// Load Dialog Info
	in->read(_dialogInfo.title, 64);
	_dialogInfo.tileIndex = in->readSint32LE();
	in->read(_dialogInfo.string, 160);
	_dialogInfo.active = in->readByte();
	_dialogInfo.x = in->readSint32LE();
	_dialogInfo.y = in->readSint32LE();
	_dialogInfo.width = in->readSint32LE();
	_dialogInfo.height = in->readSint32LE();
	_dialogInfo.titleWidth = in->readSint32LE();
	_dialogInfo.more = in->readSint32LE();
	_dialogInfo.el = in->readSint32LE();
	_dialogInfo.er = in->readSint32LE();
	_dialogInfo.et = in->readSint32LE();
	_dialogInfo.eb = in->readSint32LE();
	in->read(_dialogInfo.luaMore, 64);

	// Load Dialog Delay
	_dialogDelay = in->readSint32LE();
	if (_dialogDelay)
		_dialogDelay = g_system->getMillis() + 1000;

	// Load Dialog Choice Info
	in->read(_dialogChoiceInfo.title, 64);
	in->read(_dialogChoiceInfo.text, 160);
	in->read(_dialogChoiceInfo.func, 64);
	_dialogChoiceInfo.active = in->readByte();
	_dialogChoiceInfo.x = in->readSint32LE();
	_dialogChoiceInfo.y = in->readSint32LE();
	_dialogChoiceInfo.width = in->readSint32LE();
	_dialogChoiceInfo.height = in->readSint32LE();
	_dialogChoiceInfo.textHeight = in->readSint32LE();
	_dialogChoiceInfo.titleWidth = in->readSint32LE();
	_dialogChoiceInfo.el = in->readSint32LE();
	_dialogChoiceInfo.er = in->readSint32LE();
	_dialogChoiceInfo.et = in->readSint32LE();
	_dialogChoiceInfo.eb = in->readSint32LE();
	_dialogChoiceInfo.timeout = in->readUint32LE();
	_dialogChoiceInfo.selection = in->readSint32LE();
	_dialogChoiceInfo.numChoices = in->readSint32LE();
	for (i = 0; i < 10; i++)
		in->read(_dialogChoiceInfo.choices[i], 64);

	_dialogChoiceInfo.timeout = g_system->getMillis() + 1000;

	// Load Msg Info
	_msgInfo.active = in->readByte();
	in->read(_msgInfo.title, 128);
	_msgInfo.timer = in->readSint32LE();
	_msgInfo.x = in->readSint32LE();
	_msgInfo.y = in->readSint32LE();
	_msgInfo.width = in->readSint32LE();
	_msgInfo.height = in->readSint32LE();

	for (i = 0; i < kMaxMsgQueue; i++)
		in->read(_msgQueueStr[i], 128);
	for (i = 0; i < kMaxMsgQueue; i++)
		_msgQueueWait[i] = in->readSint32LE();

	_numMsgQueue = in->readSint32LE();

	// Load Inventory Info
	_invWinInfo.x = in->readSint32LE();
	_invWinInfo.y = in->readSint32LE();
	_invWinInfo.width = in->readSint32LE();
	_invWinInfo.height = in->readSint32LE();
	_invWinInfo.selection = in->readSint32LE();
	_invWinInfo.active = in->readByte();

	// Load Deliveries Info
	_dlvsInfo.x = in->readSint32LE();
	_dlvsInfo.y = in->readSint32LE();
	_dlvsInfo.width = in->readSint32LE();
	_dlvsInfo.height = in->readSint32LE();
	_dlvsInfo.active = in->readByte();
	_dlvsInfo.selected = in->readSint32LE();
	_dlvsInfo.animate = in->readByte();
	_dlvsInfo.delay1 = in->readUint32LE();
	_dlvsInfo.delay2 = in->readUint32LE();
	_dlvsInfo.delay3 = in->readUint32LE();
	_dlvsInfo.go1 = in->readByte();
	_dlvsInfo.go2 = in->readByte();
	_dlvsInfo.go3 = in->readByte();

	// Load Try Again Info
	_tryAgainInfo.y1 = in->readDoubleLE();
	_tryAgainInfo.y2 = in->readDoubleLE();
	_tryAgainInfo.yv1 = in->readDoubleLE();
	_tryAgainInfo.yv2 = in->readDoubleLE();
	_tryAgainInfo.yv1v = in->readDoubleLE();
	_tryAgainInfo.yv2v = in->readDoubleLE();
	_tryAgainInfo.x1 = in->readDoubleLE();
	_tryAgainInfo.x2 = in->readDoubleLE();

	// Load Textout Info
	uint32 tsize = in->readUint32LE();
	for (i = 0; (uint)i < tsize; i++) {
		TOut *t = new TOut;

		in->read(t->text, 128);
		t->x = in->readSint32LE();
		t->y = in->readSint32LE();
		(void)in->readUint32LE(); // Skip timer value
		t->timer = g_system->getMillis() + 1000; // And override it to a second

		_textOutList.push_back(t);
	}

	// Load Infobar Info
	_infobarDimmed = in->readSint32LE();
}

void Window::restartSystem() {
	_numMsgQueue = 0;
	_msgInfo.active = false;
	_dialogInfo.active = false;
	_dialogChoiceInfo.active = false;
	_invWinInfo.active = false;
	_dialogDelay = _invWinInfo.selection = 0;
	if (_gemGfx)
		delete _gemGfx;
	_gemGfx = g_hdb->_gfx->loadTile(GEM_WHITE_GFX);
	_infobarDimmed = 0;
}

void Window::setInfobarDark(int value) {
	if (g_hdb->isPPC())
		return;
	_infobarDimmed = value;
}

void Window::drawPause() {
	if (g_hdb->isPPC())
		return;
	if (g_hdb->getPause())
		_gfxPausePlaque->drawMasked(g_hdb->_screenDrawWidth / 2 - _gfxPausePlaque->_width / 2, _pauseY);
}

void Window::checkPause(int x, int y) {
	if (x >= g_hdb->_screenDrawWidth / 2 - _gfxPausePlaque->_width / 2 && g_hdb->_screenDrawWidth / 2 + _gfxPausePlaque->_width / 2 > x
		&& y >= _pauseY && y < _pauseY + _gfxPausePlaque->_height) {
		g_hdb->togglePause();
		g_hdb->_sound->playSound(SND_POP);
	}
}

void Window::drawWeapon() {
	if (!g_hdb->isPPC())
		return;

	int xoff = 40 * _pzInfo.active;
	if (ITEM_CLUB != g_hdb->_ai->getPlayerWeapon()) {
		Common::String wordString = Common::String::format("%d", g_hdb->_ai->getGemAmount());
		g_hdb->_gfx->setCursor(_weaponX + 4 - xoff, _weaponY + kTileHeight + 2);
		g_hdb->_gfx->drawText(wordString.c_str());
	}
}

void Window::chooseWeapon(AIType wType) {
	static AIType lastWeaponSelected = AI_NONE;
	int	slot = g_hdb->_ai->queryInventoryTypeSlot(wType);

	if (slot == -1)
		return;

	g_hdb->_sound->playSound(SND_MENU_SLIDER);

	if (!g_hdb->getActionMode())
		return;

	Tile *gfx = g_hdb->_ai->getInvItemGfx(slot);

	switch (wType) {
	case ITEM_CLUB:
	case ITEM_ROBOSTUNNER:
	case ITEM_SLUGSLINGER:
		g_hdb->_ai->setPlayerWeapon(wType, gfx);
		if (wType == lastWeaponSelected)
			return;
		lastWeaponSelected = wType;
		g_hdb->_sound->playSound(SND_MENU_ACCEPT);
		return;
	default:
		break;
	}
	g_hdb->_sound->playSound(SND_CELLHOLDER_USE_REJECT);
}

void Window::closeAll() {
	closeDialog();
	closeDialogChoice();
	closeMsg();
	closeTextOut();

	if (g_hdb->isPPC()) {
		closeDlvs();
		closeInv();
	}
}

void Window::openDialog(const char *title, int tileIndex, const char *string, int more, const char *luaMore) {
	if (_dialogInfo.active)
		return;

	_dialogInfo.gfx = nullptr;
	_dialogInfo.el = _dialogInfo.er = _dialogInfo.et = _dialogInfo.eb = 0;
	_dialogInfo.luaMore[0] = 0;

	_dialogInfo.tileIndex = tileIndex;
	Common::strlcpy(_dialogInfo.title, title, 128);
	_dialogInfo.active = true;

	// This could need to be truncated
	Common::strlcpy(_dialogInfo.string, string, 128);

	int e1, e2, e3, e4;
	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(_dialogTextLeft, _dialogTextRight, 0, g_hdb->_screenDrawHeight);
	int width, height;
	g_hdb->_gfx->getDimensions(string, &width, &height);
	int titleWidth, titleHeight;
	g_hdb->_gfx->getDimensions(title, &titleWidth, &titleHeight);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);
	_dialogInfo.height = (height + 2) * 16;
	int w = _dialogInfo.width = width + 32;
	_dialogInfo.titleWidth = titleWidth;
	if (titleWidth > w)
		w = titleWidth;

	_dialogInfo.x = (g_hdb->_screenDrawWidth >> 1) - (w >> 1);

	int px, py;
	g_hdb->_ai->getPlayerXY(&px, &py);
	if (py < (g_hdb->_screenHeight >> 1) - 16)
		_dialogInfo.y = (g_hdb->_screenHeight >> 1) + 16;
	else
		_dialogInfo.y = (g_hdb->_screenHeight >> 1) - (_dialogInfo.height + 64);

	if (_dialogInfo.y < 16)
		_dialogInfo.y = 16;

	_dialogInfo.more = more;
	if (luaMore)
		Common::strlcpy(_dialogInfo.luaMore, luaMore, 64);
	g_hdb->_sound->playSound(SND_MOVE_SELECTION);
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

	if (g_hdb->getActionMode()) {
		Tile *gfx2 = g_hdb->_ai->getPlayerWeaponGfx();
		if (gfx2) {
			int xOff = 40 * _pzInfo.active;
			Tile *gfx = g_hdb->_ai->getPlayerWeaponSelGfx();
			gfx->drawMasked(_weaponX - xOff - 1, _weaponY);
			gfx2->drawMasked(_weaponX - xOff, _weaponY);
			drawWeapon();
		}
	}

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
	g_hdb->_gfx->setTextEdges(_dialogInfo.x + 10, g_hdb->_screenDrawWidth, 0, g_hdb->_screenHeight);
	g_hdb->_gfx->setCursor(0, _dialogInfo.y - 7);
	if (strlen(_dialogInfo.title) > 0)
		g_hdb->_gfx->drawText(_dialogInfo.title);
	g_hdb->_gfx->setTextEdges(_dialogInfo.x + 16, _dialogInfo.x + _dialogInfo.width - 16, 0, g_hdb->_screenHeight);
	g_hdb->_gfx->setCursor(0, _dialogInfo.y + 16);
	if (strlen(_dialogInfo.string) > 0)
		g_hdb->_gfx->drawText(_dialogInfo.string);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);
}

void Window::closeDialog() {
	if (_dialogInfo.active) {
		g_hdb->_sound->playSound(SND_SWITCH_USE);
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
	int		e1, e2, e3, e4;

	if (_dialogInfo.active)
		return;

	_dialogChoiceInfo.el = _dialogChoiceInfo.er = _dialogChoiceInfo.et = _dialogChoiceInfo.eb = 0;
	for (int i = 0; i < 10; i++)
		_dialogChoiceInfo.choices[i][0] = 0;

	Common::strlcpy(_dialogChoiceInfo.title, title, 64);
	Common::strlcpy(_dialogChoiceInfo.text, text, 160);
	Common::strlcpy(_dialogChoiceInfo.func, func, 64);
	_dialogChoiceInfo.numChoices = numChoices;

	for (int i = 0; i < numChoices; i++)
		Common::strlcpy(_dialogChoiceInfo.choices[i], choices[i], 64);
	_dialogChoiceInfo.active = true;

	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(_openDialogTextLeft, _openDialogTextRight, 0, g_hdb->_screenDrawHeight);
	g_hdb->_gfx->getDimensions(text, &width, &height);
	g_hdb->_gfx->getDimensions(title, &titleWidth, &titleHeight);

	for (int i = 0; i < 10; i++)
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

	_dialogChoiceInfo.x = (g_hdb->_screenDrawWidth >> 1) - (_dialogChoiceInfo.width >> 1);
	_dialogChoiceInfo.y = (g_hdb->_screenHeight >> 1) - ((_dialogChoiceInfo.height >> 1) + 32);
	if (_dialogChoiceInfo.y < 0)
		_dialogChoiceInfo.y = 0;

	_dialogChoiceInfo.selection = 0;
	_dialogChoiceInfo.timeout = 0;
	g_hdb->_sound->playSound(SND_MOVE_SELECTION);
}

void Window::drawDialogChoice() {
	if (!_dialogChoiceInfo.active)
		return;

	// time out?
	if (_dialogChoiceInfo.timeout && _dialogChoiceInfo.timeout < g_hdb->getTimeSlice()) {
		closeDialogChoice();
		return;
	}

	bool guyTalking = !scumm_stricmp(_dialogChoiceInfo.title, "guy");

	int w = _dialogChoiceInfo.width;
	if (_dialogChoiceInfo.titleWidth > w)
		w = _dialogChoiceInfo.titleWidth;

	drawBorder(_dialogChoiceInfo.x, _dialogChoiceInfo.y, w, _dialogChoiceInfo.height, guyTalking);

	if (!guyTalking) {
		_gfxTitleL->drawMasked(_dialogChoiceInfo.x, _dialogChoiceInfo.y - 10);
		int blocks = _dialogChoiceInfo.titleWidth / 16;
		for (int i = 0; i < blocks; i++)
			_gfxTitleM->drawMasked(_dialogChoiceInfo.x + 16 * (i + 1), _dialogChoiceInfo.y - 10);
		_gfxTitleR->drawMasked(_dialogChoiceInfo.x + (blocks + 1) * 16, _dialogChoiceInfo.y - 10);
	} else {
		_gGfxTitleL->drawMasked(_dialogChoiceInfo.x, _dialogChoiceInfo.y - 10);
		int blocks = _dialogChoiceInfo.titleWidth / 16;
		for (int i = 0; i < blocks; i++)
			_gGfxTitleM->drawMasked(_dialogChoiceInfo.x + 16 * (i + 1), _dialogChoiceInfo.y - 10);
		_gGfxTitleR->drawMasked(_dialogChoiceInfo.x + (blocks + 1) * 16, _dialogChoiceInfo.y - 10);
	}

	int	e1, e2, e3, e4;
	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(_dialogChoiceInfo.x + 10, _openDialogTextRight, 0, g_hdb->_screenDrawHeight);
	g_hdb->_gfx->setCursor(0, _dialogChoiceInfo.y - 7);
	if (strlen(_dialogChoiceInfo.title) > 0)
		g_hdb->_gfx->drawText(_dialogChoiceInfo.title);
	g_hdb->_gfx->setTextEdges(_dialogChoiceInfo.x + 16, _openDialogTextRight, 0, g_hdb->_screenDrawHeight);
	g_hdb->_gfx->setCursor(0, _dialogChoiceInfo.y + 16);
	if (strlen(_dialogChoiceInfo.text) > 0)
		g_hdb->_gfx->drawText(_dialogChoiceInfo.text);

	for (int i = 0; i < _dialogChoiceInfo.numChoices; i++) {
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
		g_hdb->_sound->playSound(SND_SWITCH_USE);
	}
}

bool Window::checkDialogChoiceClose(int x, int y) {
	if (!_dialogChoiceInfo.active || _dialogChoiceInfo.timeout)
		return false;

	if (x >= _dialogChoiceInfo.x && x < _dialogChoiceInfo.x + _dialogChoiceInfo.width &&
		y >= _dialogChoiceInfo.y + _dialogChoiceInfo.textHeight && y < _dialogChoiceInfo.y + _dialogChoiceInfo.textHeight + _dialogChoiceInfo.numChoices * 16) {
		g_hdb->_sound->playSound(SND_SWITCH_USE);
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
	g_hdb->_sound->playSound(SND_MOVE_SELECTION);
}

void Window::dialogChoiceMovedown() {
	_dialogChoiceInfo.selection++;
	if (_dialogChoiceInfo.selection >= _dialogChoiceInfo.numChoices)
		_dialogChoiceInfo.selection = 0;
	g_hdb->_sound->playSound(SND_MOVE_SELECTION);
}

void Window::openMessageBar(const char *title, int time) {
	// is the messagebar already up?  if so, add this msg to the queue
	if (_msgInfo.active) {
		if (_numMsgQueue < kMaxMsgQueue) {
			int		i;
			if (!scumm_stricmp(_msgInfo.title, title))
				return;

			for (i = 0; i < _numMsgQueue; i++)
				if (!scumm_stricmp(_msgQueueStr[i], title))
					return;
			Common::strlcpy(_msgQueueStr[_numMsgQueue], title, 128);
			_msgQueueWait[_numMsgQueue] = time;
			_numMsgQueue++;
		}
		return;
	}

	_msgInfo.y = 0;
	_msgInfo.timer = (time * kGameFPS);
	Common::strlcpy(_msgInfo.title, title, 128);

	int	e1, e2, e3, e4;
	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(_dialogTextLeft, _dialogTextRight, 0, g_hdb->_screenDrawHeight);
	int	width, height;
	g_hdb->_gfx->getDimensions(title, &width, &height);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);

	_msgInfo.height = (height + 2) * 16;
	_msgInfo.width = width + 32;

	_msgInfo.x = (g_hdb->_screenDrawWidth >> 1) - (_msgInfo.width >> 1);
	_msgInfo.active = true;
}

void Window::drawMessageBar() {
	// if msgbar's not up OR inventory is up, exit
	if (!_msgInfo.active || _invWinInfo.active || _dialogInfo.active)
		return;

	int	xx, py, my;
	g_hdb->_ai->getPlayerXY(&xx, &py);	// don't care about the x
	g_hdb->_map->getMapXY(&xx, &my);
	_msgInfo.y = (py - my) - _msgInfo.height - 64;	// put msgbar directly above player
	if (_msgInfo.y < _msgInfo.height)
		_msgInfo.y = (py - my) + 40;					// if at top, but it directly below

	drawBorder(_msgInfo.x, _msgInfo.y, _msgInfo.width, _msgInfo.height, false);

	int	e1, e2, e3, e4;
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
	if (!_numMsgQueue) {
		_msgInfo.active = false;
		return;
	}

	Common::strlcpy(_msgInfo.title, _msgQueueStr[0], 128);
	_msgInfo.timer = (_msgQueueWait[0] * kGameFPS);

	int e1, e2, e3, e4;
	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(_dialogTextLeft, _dialogTextRight, 0, g_hdb->_screenDrawHeight);
	int width, height;
	g_hdb->_gfx->getDimensions(_msgInfo.title, &width, &height);
	g_hdb->_gfx->setTextEdges(e1, e2, e3, e4);

	_msgInfo.height = (height + 2) * 16;

	_msgInfo.width = width + 32;
	_msgInfo.x = (g_hdb->_screenDrawWidth >> 1) - (_msgInfo.width >> 1);
	_msgInfo.y = (g_hdb->_screenHeight >> 2) - (_msgInfo.height >> 1);

	for (int xx = 0; xx < _numMsgQueue - 1; xx++) {
		Common::strlcpy(_msgQueueStr[xx], _msgQueueStr[xx + 1], 128);
		_msgQueueWait[xx] = _msgQueueWait[xx + 1];
	}

	_numMsgQueue--;
	_msgInfo.active = true;
}

void Window::closeMsg() {
	nextMsgQueued();
	g_hdb->_sound->playSound(SND_DIALOG_CLOSE);
}

void Window::drawInventory() {
	static uint32 timer = g_hdb->getTimeSlice() + 300; // unused

	if (g_hdb->isPPC()) {
		if (!_invWinInfo.active)
			return;

		drawBorder(_invWinInfo.x, _invWinInfo.y, _invWinInfo.width, _invWinInfo.height, false);

		_gfxTitleL->drawMasked(_invWinInfo.x, _invWinInfo.y - 10);
		int blocks = _gfxResources->_width / 16;
		for (int i = 0; i < blocks; i++)
			_gfxTitleM->drawMasked(_invWinInfo.x + 16 * (i + 1), _invWinInfo.y - 10);
		_gfxTitleR->drawMasked(_invWinInfo.x + 16 * (blocks + 1), _invWinInfo.y - 10);

		_gfxResources->drawMasked(_invWinInfo.x + 16, _invWinInfo.y - 6);

		//
		// blink the INV hand...
		//
		if (g_hdb->getTimeSlice() > timer - 150)
			_gfxHandright->drawMasked(g_hdb->_screenWidth - _gfxHandright->_width, _invWinInfo.y + _invItemSpace * 2);

		if (timer < g_hdb->getTimeSlice())
			timer = g_hdb->getTimeSlice() + 300;

		int drawX = _invWinInfo.x + 16;
		int baseX = drawX;
		int drawY = _invWinInfo.y + 16;

		// Draw Inv Items
		AIEntity *sel = nullptr;
		if (_invWinInfo.selection >= g_hdb->_ai->getInvAmount())
			_invWinInfo.selection = g_hdb->_ai->getInvAmount() - 1;

		for (int inv = 0; inv < g_hdb->_ai->getInvAmount(); inv++) {
			AIEntity *e = g_hdb->_ai->getInvItem(inv);
			if (inv == _invWinInfo.selection)
				sel = e;

			e->standdownGfx[0]->drawMasked(drawX, drawY);

			drawX += _invItemSpace;
			if (drawX >= baseX + (_invItemSpace * _invItemPerLine)) {
				drawX = baseX;
				drawY += _invItemSpace;
			}
		}

		// Draw the gems
		drawY = _invWinInfo.y + 8 + _invItemSpace * 2;
		drawX = baseX + _invItemSpace * 4 + 8;
		_gemGfx->drawMasked(drawX, drawY);
		int gems = g_hdb->_ai->getGemAmount();
		Common::String gemString = Common::String::format("%d", gems);
		g_hdb->_gfx->setCursor(drawX + 22, drawY + 8);
		g_hdb->_gfx->drawText(gemString.c_str());

		// if you have an inventory, draw the selection cursor
		if (g_hdb->_ai->getInvAmount()) {
			if (_invWinInfo.selection < 0)
				_invWinInfo.selection = 0;

			// Draw the inventory select cursor
			if (_invWinInfo.selection > 4)
				drawX = baseX + (_invWinInfo.selection - 5) * _invItemSpace;
			else
				drawX = baseX + _invWinInfo.selection * _invItemSpace;

			if (_invWinInfo.selection < 5)
				drawY = _invWinInfo.y + 16;
			else
				drawY = _invWinInfo.y + 16 + _invItemSpace;

			_gfxInvSelect->drawMasked(drawX, drawY);

			// Draw the name of the inventory item
			if (sel) {
				drawY = _invWinInfo.y + 8 + _invItemSpace * 2;
				g_hdb->_gfx->setCursor(baseX, drawY + 8);
				g_hdb->_gfx->drawText(sel->printedName);
			}
		}
	} else {
		// INFOBAR blit - only once per frame
		// note: if 2, don't draw ANY info at all
		if (_infobarDimmed > 1)
			return;

		_gfxInfobar->draw(g_hdb->_screenWidth - _gfxInfobar->_width, 0);

		int drawX = _invWinInfo.x;
		int baseX = drawX;
		int drawY = _invWinInfo.y;

		// Draw Inv Items
		AIEntity *sel = nullptr;
		if (_invWinInfo.selection >= g_hdb->_ai->getInvAmount())
			_invWinInfo.selection = g_hdb->_ai->getInvAmount() - 1;

		for (int inv = 0; inv < g_hdb->_ai->getInvAmount(); inv++) {
			AIEntity *e = g_hdb->_ai->getInvItem(inv);
			if (inv == _invWinInfo.selection)
				sel = e;

			e->standdownGfx[0]->drawMasked(drawX, drawY);

			drawX += _invItemSpaceX;
			if (drawX >= baseX + (_invItemSpaceX * _invItemPerLine)) {
				drawX = baseX;
				drawY += _invItemSpaceY;
			}
		}

		// Draw the Gem
		drawY = _invWinInfo.y + _invItemSpaceY * 4 - 8;
		drawX = baseX - 8;
		_gemGfx->drawMasked(drawX, drawY);

		// Draw the Gem Amount
		int gems = g_hdb->_ai->getGemAmount();
		Common::String gemString = Common::String::format("%d", gems);
		g_hdb->_gfx->setCursor(drawX + 32, drawY + 8);
		g_hdb->_gfx->drawText(gemString.c_str());

		// Draw the mini monkeystone
		int mstones = g_hdb->_ai->getMonkeystoneAmount();
		if (mstones) {
			drawX = baseX + _invItemSpaceX * 2 - 8;
			_mstoneGfx->drawMasked(drawX, drawY + 8);

			// Draw the monkeystone amount
			Common::String stoneString = Common::String::format("%d", mstones);
			g_hdb->_gfx->setCursor(drawX + 28, drawY + 8);
			g_hdb->_gfx->drawText(stoneString.c_str());
		}

		// If you have an inventory, draw the selection cursor
		if (g_hdb->_ai->getInvAmount()) {
			if (_invWinInfo.selection < 0)
				_invWinInfo.selection = 0;

			// Draw the Inventory Select Cursor
			drawX = baseX + (_invWinInfo.selection % _invItemPerLine) * _invItemSpaceX;
			drawY = _invWinInfo.y + (_invWinInfo.selection / _invItemPerLine) * _invItemSpaceY;
			_gfxInvSelect->drawMasked(drawX, drawY);

			if (sel) {
				int centerX = baseX - 4 + (g_hdb->_screenWidth - baseX) / 2;
				drawY = _invWinInfo.y + (_invItemSpaceY * 4) + 16;
				g_hdb->_gfx->setCursor(centerX - g_hdb->_gfx->stringLength(sel->printedName) / 2, drawY);
				g_hdb->_gfx->drawText(sel->printedName);
			}
		}
	}
}

void Window::checkInvSelect(int x, int y) {
	if (x >= _invWinInfo.x && x < _invWinInfo.x + _invWinInfo.width && y >= _invWinInfo.y && y < _invWinInfo.y + _invWinInfo.height) {
		int xc = (x - _invWinInfo.x) / _invItemSpaceX;
		int yc = (y - _invWinInfo.y) / _invItemSpaceY;
		if (yc * _invItemPerLine + xc > g_hdb->_ai->getInvAmount())
			return;

		_invWinInfo.selection = yc * _invItemPerLine + xc;

		// If this is a weapon, choose it
		AIType t = g_hdb->_ai->getInvItemType(_invWinInfo.selection);
		switch (t) {
		case ITEM_CLUB:
		case ITEM_ROBOSTUNNER:
		case ITEM_SLUGSLINGER:
			chooseWeapon(t);
			if (t == ITEM_CLUB)
				g_hdb->_sound->playSound(SND_GET_CLUB);
			else if (t == ITEM_ROBOSTUNNER)
				g_hdb->_sound->playSound(SND_GET_STUNNER);
			else if (t == ITEM_SLUGSLINGER)
				g_hdb->_sound->playSound(SND_GET_SLUG);
			return;
		default:
			break;
		}

		g_hdb->_sound->playSound(SND_POP);
	}

	return;
}

void Window::openInventory() {
	if (!g_hdb->isPPC())
		return;

	if (_invWinInfo.active)
		return;

	centerTextOut(g_hdb->getInMapName(), 304, 60 * 3);

	if (!g_hdb->_ai->getInvAmount() && !g_hdb->_ai->getGemAmount()) {
		if (!g_hdb->_ai->getDeliveriesAmount())
			openMessageBar("You have nothing.", 3);
		else
			openDeliveries(false);
		return;
	}

	_invWinInfo.x = (g_hdb->_screenWidth >> 1) - (_invWinInfo.width >> 1) - 8;

	int	px, py;
	g_hdb->_ai->getPlayerXY(&px, &py);
	if (py < (g_hdb->_screenHeight >> 1) - 16)
		_invWinInfo.y = (g_hdb->_screenHeight >> 1) + 16;
	else
		_invWinInfo.y = 16;

	g_hdb->_sound->playSound(SND_POP);
	_invWinInfo.active = true;
}

bool Window::checkInvClose(int x, int y) {
	if (!g_hdb->isPPC())
		return false;

	if (x >= g_hdb->_screenWidth - _gfxHandright->_width &&
		y >= _invWinInfo.y && y < _invWinInfo.y + _invItemSpace * 3) {
		closeInv();
		openDeliveries(0);
		return true;
	} else if (x >= _invWinInfo.x && x < _invWinInfo.x + _invWinInfo.width &&
		y >= _invWinInfo.y && y < _invWinInfo.y + _invWinInfo.height) {
		int xc = (x - _invWinInfo.x) / _invItemSpace;
		int yc = (y - _invWinInfo.y) / _invItemSpace;
		if (yc * 5 + xc > g_hdb->_ai->getInvAmount()) {
			closeInv();
			return true;
		}
		_invWinInfo.selection = yc * 5 + xc;
		g_hdb->_sound->playSound(SND_MENU_SLIDER);

		static AIType lastWeaponSelected = AI_NONE;

		if (!g_hdb->getActionMode())
			return false;

		AIType t = g_hdb->_ai->getInvItemType(_invWinInfo.selection);
		Tile *gfx = g_hdb->_ai->getInvItemGfx(_invWinInfo.selection);

		switch (t) {
		case ITEM_CLUB:
		case ITEM_ROBOSTUNNER:
		case ITEM_SLUGSLINGER:
			g_hdb->_ai->setPlayerWeapon(t, gfx);
			if (t == lastWeaponSelected) {
				closeInv();
				return false;
			}
			lastWeaponSelected = t;
			g_hdb->_sound->playSound(SND_MENU_ACCEPT);
			return true;
		default:
			break;
		}
		g_hdb->_sound->playSound(SND_CELLHOLDER_USE_REJECT);
	}

	return false;
}

void Window::closeInv() {
	if (!g_hdb->isPPC())
		return;

	g_hdb->_sound->playSound(SND_DIALOG_CLOSE);
	_invWinInfo.active = false;
}

void Window::openDeliveries(bool animate) {
	// Load Gfx
	for (int i = 0; i < g_hdb->_ai->getDeliveriesAmount(); i++) {
		DlvEnt *d = g_hdb->_ai->getDeliveryItem(i);
		if (d->itemGfxName[0])
			d->itemGfx = g_hdb->_gfx->loadTile(d->itemGfxName);
		if (d->destGfxName[0])
			d->destGfx = g_hdb->_gfx->loadTile(d->destGfxName);
	}

	if (g_hdb->isPPC()) {
		// calc the X & Y for the window
		_dlvsInfo.x = (g_hdb->_screenWidth >> 1) - (_dlvsInfo.width >> 1) - 8;

		int px, py;
		g_hdb->_ai->getPlayerXY(&px, &py);
		if (py < (g_hdb->_screenHeight >> 1) - 16)
			_dlvsInfo.y = (g_hdb->_screenHeight >> 1);
		else
			_dlvsInfo.y = 16;
	}

	g_hdb->_sound->playSound(SND_POP);
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

	if (g_hdb->isPPC())
		_dlvsInfo.active = true;
}

void Window::drawDeliveries() {
	static uint32 timer = g_hdb->getTimeSlice() + 300; //unused
	static const int crazySounds[kNumCrazy] = {
		SND_GUI_INPUT,
		SND_MAIL_PROCESS,
		SND_MONKEY_OOHOOH,
		SND_GET_GEM,
		SND_MENU_ACCEPT,
		SND_MENU_BACKOUT,
		SND_MENU_SLIDER,
		SND_DIALOG_CLOSE,
		SND_POP,
		SND_SPLASH,
		SND_CHICKEN_AMBIENT,
		SND_SWITCH_USE,
		SND_BARREL_EXPLODE,
		SND_BARREL_MELTING,
		SND_MOVE_SELECTION,
		SND_NOTICE,
		SND_DIALOG_OPEN,
		SND_TOUCHPLATE_CLICK,
		SND_MBOT_HYEAH,
		SND_MBOT_YEAH,
		SND_MBOT_WHISTLE1,
		SND_CLUB_MISS,
		SND_CLUB_HIT_METAL,
		SND_CLUB_HIT_FLESH,
		SND_FROG_LICK,
		SND_ROBOT_STUNNED,
		SND_BRIDGE_EXTEND,
		SND_BRIDGE_END,
		SND_AIRLOCK_CLOSE,
		SND_FART,
		SND_FART2,
		SND_GEM_THROW,
		SND_INV_SELECT,
		SND_INFOCOMP,
		SND_CLOCK_BONK,
		SND_GET_GOO,
		SND_MANNY_CRASH
	};

	if (g_hdb->isPPC()) {
		if (!_dlvsInfo.active)
			return;

		// draw the window first
		drawBorder(_dlvsInfo.x, _dlvsInfo.y, _dlvsInfo.width, _dlvsInfo.height, false);

		_gfxTitleL->drawMasked(_dlvsInfo.x, _dlvsInfo.y - 10);
		int blocks = _gfxDeliveries->_width / 16;
		for (int i = 0; i < blocks; i++)
			_gfxTitleM->drawMasked(_dlvsInfo.x + 16 * (i + 1), _dlvsInfo.y - 10);
		_gfxTitleR->drawMasked(_dlvsInfo.x + (blocks + 1) * 16, _dlvsInfo.y - 10);

		_gfxDeliveries->drawMasked(_dlvsInfo.x + 16, _dlvsInfo.y - 6);

		//
		// blink the DELIVERY hand...
		//
		if (g_hdb->_ai->getInvAmount()) {
			if (g_hdb->getTimeSlice() > timer - 150)
				_gfxHandright->drawMasked(_dlvsInfo.x + _dlvsInfo.width, _dlvsInfo.y + _invItemSpace * 2);

			if (timer < g_hdb->getTimeSlice())
				timer = g_hdb->getTimeSlice() + 300;
		}

		int drawX = _dlvsInfo.x + 16;
		int drawY = _dlvsInfo.y + 16;
		int baseX = drawX;

		// Draw delivery items
		int inv = 0;
		for (; inv < g_hdb->_ai->getDeliveriesAmount(); inv++) {
			DlvEnt *d = g_hdb->_ai->getDeliveryItem(inv);
			if (_dlvsInfo.animate && inv == g_hdb->_ai->getDeliveriesAmount() - 1) {
				if (_dlvsInfo.go1) {
					if (_dlvsInfo.delay1 < g_hdb->getTimeSlice()) {
						// draw Item
						_gfxIndent->draw(drawX, drawY);
						d->itemGfx->drawMasked(drawX, drawY);

						g_hdb->_gfx->setCursor(drawX, drawY + _invItemSpace * 2 + 20);
						g_hdb->_gfx->centerPrint(d->itemTextName);
						if (!_dlvsInfo.go2) {
							_dlvsInfo.go2 = true;
							_dlvsInfo.delay2 = g_hdb->getTimeSlice() + 500;
							g_hdb->_sound->playSound(crazySounds[g_hdb->_rnd->getRandomNumber(kNumCrazy - 1)]);
						}
					}
				}
				if (_dlvsInfo.go2) {
					if (_dlvsInfo.delay2 < g_hdb->getTimeSlice()) {
						// arrow TO...
						_gfxArrowTo->drawMasked(_dlvsInfo.x + 24 + _invItemSpace * _dlvsInfo.selected, drawY + _invItemSpace);

						g_hdb->_gfx->setCursor(drawX, drawY + _invItemSpace * 2 + 32);
						g_hdb->_gfx->centerPrint("to");
						if (!_dlvsInfo.go3) {
							_dlvsInfo.go3 = true;
							_dlvsInfo.delay3 = g_hdb->getTimeSlice() + 500;
							g_hdb->_sound->playSound(crazySounds[g_hdb->_rnd->getRandomNumber(kNumCrazy - 1)]);
						}
					}
				}
				if (_dlvsInfo.go3) {
					if (_dlvsInfo.delay3 < g_hdb->getTimeSlice()) {
						// draw Deliveree
						_gfxIndent->draw(drawX, drawY + _invItemSpace + 16);
						d->destGfx->drawMasked(drawX, drawY + _invItemSpace + 16);

						g_hdb->_gfx->setCursor(drawX, drawY + _invItemSpace * 2 + 44);
						g_hdb->_gfx->centerPrint(d->destTextName);

						g_hdb->_sound->playSound(crazySounds[g_hdb->_rnd->getRandomNumber(kNumCrazy - 1)]);
						_dlvsInfo.animate = false;
					}
				}
			} else {
				// draw Item
				_gfxIndent->draw(drawX, drawY);
				d->itemGfx->drawMasked(drawX, drawY);

				// draw Deliveree
				_gfxIndent->draw(drawX, drawY + _invItemSpace + 16);
				d->destGfx->drawMasked(drawX, drawY + _invItemSpace + 16);

				if (!_dlvsInfo.animate && inv == _dlvsInfo.selected) {
					g_hdb->_gfx->setCursor(drawX, drawY + _invItemSpace * 2 + 20);
					g_hdb->_gfx->centerPrint(d->itemTextName);
					g_hdb->_gfx->setCursor(drawX, drawY + _invItemSpace * 2 + 32);
					g_hdb->_gfx->centerPrint("to");
					g_hdb->_gfx->setCursor(drawX, drawY + _invItemSpace * 2 + 44);
					g_hdb->_gfx->centerPrint(d->destTextName);
				}

				drawX += _invItemSpace;
			}
		}

		if (!inv) {
			g_hdb->_gfx->setCursor(baseX, _dlvsInfo.y + 32);
			g_hdb->_gfx->drawText("No Deliveries");
			_dlvsInfo.animate = false;
		} else if (!_dlvsInfo.animate) {
			// arrow TO...
			_gfxArrowTo->drawMasked(_dlvsInfo.x + 24 + _invItemSpace * _dlvsInfo.selected, drawY + _invItemSpace);
		}
	} else {
		if (_infobarDimmed > 1)
			return;

		int baseX = _dlvsInfo.x;
		int drawX = _dlvsInfo.x;
		int drawY = _dlvsInfo.y;

		if (_dlvsInfo.selected >= g_hdb->_ai->getDeliveriesAmount())
			_dlvsInfo.selected = g_hdb->_ai->getDeliveriesAmount() - 1;

		// Draw Delivery Items
		int inv = 0;
		for (; inv < g_hdb->_ai->getDeliveriesAmount(); inv++) {
			int centerX = baseX + (g_hdb->_screenWidth - baseX) / 2;
			DlvEnt *d = g_hdb->_ai->getDeliveryItem(inv);
			if (_dlvsInfo.animate && inv == g_hdb->_ai->getDeliveriesAmount() - 1) {
				if (_dlvsInfo.go1) {
					if (_dlvsInfo.delay1 < g_hdb->getTimeSlice()) {
						// Draw Item
						_gfxIndent->draw(drawX, drawY);
						if (d->itemGfx)
							d->itemGfx->drawMasked(drawX, drawY);

						g_hdb->_gfx->setCursor(centerX - g_hdb->_gfx->stringLength(d->itemTextName) / 2, _dlvItemTextY);
						g_hdb->_gfx->drawText(d->itemTextName);
						if (!_dlvsInfo.go2) {
							_dlvsInfo.go2 = true;
							_dlvsInfo.delay2 = g_hdb->getTimeSlice() + 500;
							g_hdb->_sound->playSound(crazySounds[g_hdb->_rnd->getRandomNumber(kNumCrazy - 1)]);
						}
					}
				}
				if (_dlvsInfo.go2) {
					if (_dlvsInfo.delay2 < g_hdb->getTimeSlice()) {
						// Draw TO
						_gfxArrowTo->drawMasked(_dlvsInfo.x + _dlvItemSpaceX * _dlvsInfo.selected + 8, drawY + kTileHeight);

						g_hdb->_gfx->setCursor(centerX - g_hdb->_gfx->stringLength("to") / 2, _dlvItemTextY + 12);
						g_hdb->_gfx->drawText("to");
						if (!_dlvsInfo.go3) {
							_dlvsInfo.go3 = true;
							_dlvsInfo.delay3 = g_hdb->getTimeSlice() + 500;
							g_hdb->_sound->playSound(crazySounds[g_hdb->_rnd->getRandomNumber(kNumCrazy - 1)]);
						}
					}
				}
				if (_dlvsInfo.go3) {
					if (_dlvsInfo.delay3 < g_hdb->getTimeSlice()) {
						// Draw Delivery
						_gfxIndent->draw(drawX, drawY + kTileHeight + 16);
						if (d->destGfx)
							d->destGfx->drawMasked(drawX, drawY + kTileHeight + 16);

						g_hdb->_gfx->setCursor(centerX - (g_hdb->_gfx->stringLength(d->destTextName) + g_hdb->_gfx->stringLength("to")) / 2, _dlvItemTextY + 12);
						g_hdb->_gfx->drawText("to ");
						g_hdb->_gfx->drawText(d->destTextName);

						g_hdb->_sound->playSound(crazySounds[g_hdb->_rnd->getRandomNumber(kNumCrazy - 1)]);
						_dlvsInfo.animate = false;
					}
				}

			} else {
				// Draw Item
				_gfxIndent->draw(drawX, drawY);
				if (d->itemGfx)
					d->itemGfx->drawMasked(drawX, drawY);
				// Draw Delivery
				_gfxIndent->draw(drawX, drawY + kTileHeight + 16);
				if (d->destGfx)
					d->destGfx->drawMasked(drawX, drawY + kTileHeight + 16);

				if (!_dlvsInfo.animate && inv == _dlvsInfo.selected) {
					g_hdb->_gfx->setCursor(centerX - g_hdb->_gfx->stringLength(d->itemTextName) / 2, _dlvItemTextY);
					g_hdb->_gfx->drawText(d->itemTextName);
					g_hdb->_gfx->setCursor(centerX - (g_hdb->_gfx->stringLength(d->destTextName) + g_hdb->_gfx->stringLength("to ")) / 2, _dlvItemTextY + 12);
					g_hdb->_gfx->drawText("to ");
					g_hdb->_gfx->drawText(d->destTextName);
				}

				drawX += _dlvItemSpaceX;
				if (drawX >= g_hdb->_screenWidth) {
					drawX = baseX;
					drawY += _dlvItemSpaceY + 8;
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
			dx = 8 + _dlvsInfo.x + _dlvItemSpaceX * (_dlvsInfo.selected % 3);
			dy = _dlvsInfo.y + kTileHeight + (_dlvItemSpaceY + 8) * rowtwo;
			_gfxArrowTo->drawMasked(dx, dy);
		}

		// If the infobar is dimmed out, this where we dim the whole thing
		if (_infobarDimmed) {
			for (int j = 0; j < g_hdb->_screenHeight; j += kTileHeight) {
				for (int i = (g_hdb->_screenWidth - _gfxInfobar->_width); i < g_hdb->_screenWidth; i += kTileWidth)
					_gfxDarken->drawMasked(i, j);
			}
		}
	}
}

void Window::setSelectedDelivery(int which) {
	_dlvsInfo.selected = which;
	g_hdb->_sound->playSound(SND_MENU_SLIDER);
}

void Window::checkDlvSelect(int x, int y) {
	if (_dlvsInfo.animate)
		return;

	int amt = g_hdb->_ai->getDeliveriesAmount();

	// Click on a Delivery to select it for inspection?
	if (x >= _dlvsInfo.x && x < _dlvsInfo.x + _dlvsInfo.width && y >= _dlvsInfo.y && y < _dlvsInfo.y + _dlvsInfo.height) {
		int xc = (x - _dlvsInfo.x) / _dlvItemSpaceX;
		int yc = (y - _dlvsInfo.y) / _dlvItemSpaceY;
		int value = yc * _dlvItemPerLine + xc;
		if (value < amt)
			setSelectedDelivery(value);
	}
}

bool Window::checkDlvsClose(int x, int y) {
	if (!g_hdb->isPPC())
		return false;

	if (_dlvsInfo.animate)
		return false;

	int amt = g_hdb->_ai->getDeliveriesAmount();

	// click on a delivery to select it for inspection?
	if (x >= _dlvsInfo.x + 16 && x < _dlvsInfo.x + 16 + amt * _invItemSpace &&
		y >= _dlvsInfo.y && y < _dlvsInfo.y + _invItemSpace * 3) {
		setSelectedDelivery(((x - _dlvsInfo.x + 16) / _invItemSpace) - 1);
	} else if (g_hdb->_ai->getInvAmount() &&
		x >= g_hdb->_screenWidth - _gfxHandright->_width &&
		y >= _dlvsInfo.y && y < _dlvsInfo.y + _invItemSpace * 3) {
		// click on HAND?
		closeDlvs();
		openInventory();
		return true;
	} else if (x >= _dlvsInfo.x && x < _dlvsInfo.x + _dlvsInfo.width &&
		y >= _dlvsInfo.y && y < _dlvsInfo.y + _dlvsInfo.height) {
		// click anywhere else in window to close it?
		closeDlvs();
		return true;
	}

	return false;
}

void Window::closeDlvs() {
	if (!g_hdb->isPPC())
		return;

	if (_dlvsInfo.animate)
		return;

	g_hdb->_sound->playSound(SND_DIALOG_CLOSE);
	_dlvsInfo.active = false;
}

void Window::drawTryAgain() {
	if (!g_hdb->_ai->playerDead())
		return;

	if (nullptr == _gfxTry) {
		_gfxTry = g_hdb->_gfx->loadPic(GAME_TRY);
		_gfxAgain = g_hdb->_gfx->loadPic(GAME_AGAIN);
		_gfxLevelRestart = g_hdb->_gfx->loadPic(GAME_TA_LEVELRESTART);

		_tryAgainInfo.y1 = _tryY1;
		_tryAgainInfo.y2 = _tryY2;
		_tryAgainInfo.x1 = g_hdb->_screenDrawWidth / 2 - _gfxTry->_width / 2;
		_tryAgainInfo.x2 = g_hdb->_screenDrawWidth / 2 - _gfxAgain->_width / 2;
	}

	int xv = g_hdb->_rnd->getRandomNumber(3) - 2, yv = g_hdb->_rnd->getRandomNumber(3) - 2;

	_gfxTry->drawMasked((int)_tryAgainInfo.x1 + xv, (int)_tryAgainInfo.y1 + yv);
	_gfxAgain->drawMasked((int)_tryAgainInfo.x2 + yv, (int)_tryAgainInfo.y2 + xv);
	_gfxLevelRestart->drawMasked((int)(g_hdb->_screenDrawWidth / 2 - _gfxLevelRestart->_width / 2 + xv), g_hdb->_window->_tryRestartY + yv);
}

void Window::clearTryAgain() {
	delete _gfxTry;
	delete _gfxAgain;
	delete _gfxLevelRestart;

	_gfxTry = _gfxAgain = _gfxLevelRestart = nullptr;
}

void Window::loadPanicZoneGfx() {
	_pzInfo.gfxPanic = g_hdb->_gfx->loadPic(PANIC_PANIC);
	_pzInfo.gfxZone	= g_hdb->_gfx->loadPic(PANIC_ZONE);
	_pzInfo.gfxFace[0] = g_hdb->_gfx->loadPic(PANIC_POINTER1);
	_pzInfo.gfxFace[1] = g_hdb->_gfx->loadPic(PANIC_POINTER2);
	_pzInfo.gfxNumber[0] = g_hdb->_gfx->loadPic(PANIC_NUM0);
	_pzInfo.gfxNumber[1] = g_hdb->_gfx->loadPic(PANIC_NUM1);
	_pzInfo.gfxNumber[2] = g_hdb->_gfx->loadPic(PANIC_NUM2);
	_pzInfo.gfxNumber[3] = g_hdb->_gfx->loadPic(PANIC_NUM3);
	_pzInfo.gfxNumber[4] = g_hdb->_gfx->loadPic(PANIC_NUM4);
	_pzInfo.gfxNumber[5] = g_hdb->_gfx->loadPic(PANIC_NUM5);
	_pzInfo.gfxNumber[6] = g_hdb->_gfx->loadPic(PANIC_NUM6);
	_pzInfo.gfxNumber[7] = g_hdb->_gfx->loadPic(PANIC_NUM7);
	_pzInfo.gfxNumber[8] = g_hdb->_gfx->loadPic(PANIC_NUM8);
	_pzInfo.gfxNumber[9] = g_hdb->_gfx->loadPic(PANIC_NUM9);
}

void Window::drawPanicZone() {
	if (!_pzInfo.active || !g_hdb->isDemo())
		return;

	switch (_pzInfo.sequence) {
		// Wait before displaying PANIC ZONE
	case PANICZONE_TIMER:
		_pzInfo.timer--;
		if (!_pzInfo.timer) {
			_pzInfo.sequence++;
			g_hdb->_sound->playSound(SND_PANIC);
		}
		break;

		// Move PANIC ZONE to screen center
	case PANICZONE_START:
		{
			int xx = g_hdb->_rnd->getRandomNumber(9) - 5;
			int yy = g_hdb->_rnd->getRandomNumber(9) - 5;
			_pzInfo.x1 += _pzInfo.xv;
			_pzInfo.y1++;
			_pzInfo.x2 += _pzInfo.yv;
			_pzInfo.y2--;
			if (_pzInfo.x1 > _panicXStop) {
				_pzInfo.timer = 30;
				_pzInfo.sequence++;
			}
			_pzInfo.gfxPanic->drawMasked(_pzInfo.x1 + xx, _pzInfo.y1 + yy);
			_pzInfo.gfxZone->drawMasked(_pzInfo.x2 + yy, _pzInfo.y2 + xx);
		}
		break;
	case PANICZONE_TITLESTOP:
		{
			int xx = g_hdb->_rnd->getRandomNumber(9) - 5;
			int yy = g_hdb->_rnd->getRandomNumber(9) - 5;
			_pzInfo.gfxPanic->drawMasked(_pzInfo.x1 + xx, _pzInfo.y1 + yy);
			_pzInfo.gfxZone->drawMasked(_pzInfo.x2 + yy, _pzInfo.y2 + xx);
			_pzInfo.timer--;
			if (!_pzInfo.timer)
				_pzInfo.sequence++;
		}
		break;
	case PANICZONE_BLASTOFF:
		{
			int xx = g_hdb->_rnd->getRandomNumber(9) - 5;
			int yy = g_hdb->_rnd->getRandomNumber(9) - 5;
			_pzInfo.y1 -= 10;
			_pzInfo.y2 += 10;
			_pzInfo.gfxPanic->drawMasked(_pzInfo.x1 + xx, _pzInfo.y1 + yy);
			_pzInfo.gfxZone->drawMasked(_pzInfo.x2 + yy, _pzInfo.y2 + xx);
			if (_pzInfo.y1 < -_pzInfo.gfxPanic->_height &&
				_pzInfo.y2 > g_hdb->_screenHeight) {
				g_hdb->_sound->playSound(SND_PANIC_COUNT);
				_pzInfo.sequence++;
				_pzInfo.timer = 30 + g_hdb->getTime();
			}
		}
		break;
	case PANICZONE_COUNTDOWN:
		{
			static int last_seconds = 0, seconds = 0;
			_pzInfo.gfxFace[seconds & 1]->drawMasked(_panicZoneFaceX, kPanicZoneFaceY);

			// make knocking timer sound
			if (last_seconds != seconds)
				g_hdb->_sound->playSound(SND_PANIC_COUNT);

			last_seconds = seconds;
			seconds = _pzInfo.timer - g_hdb->getTime();
			if (seconds >= 10) {
				_pzInfo.gfxNumber[seconds / 10]->drawMasked(_panicZoneFaceX, kPanicZoneFaceY + 32);
				_pzInfo.gfxNumber[seconds % 10]->drawMasked(_panicZoneFaceX + 16, kPanicZoneFaceY + 32);
			} else
				_pzInfo.gfxNumber[seconds]->drawMasked(_panicZoneFaceX + 8, kPanicZoneFaceY + 32);

			// time until death!
			if (!seconds) {
				// dead
				g_hdb->_ai->killPlayer(DEATH_PANICZONE);
				_pzInfo.active = false;
				return;
			}
		}
		break;
	default:
		break;
	}
}

void Window::startPanicZone() {
	_pzInfo.active = true;
	_pzInfo.sequence = PANICZONE_TIMER;

	// load PANIC ZONE gfx if they aren't loaded
	if (!_pzInfo.gfxPanic)
		loadPanicZoneGfx();

	_pzInfo.x1 = -(_pzInfo.gfxPanic->_width + 5);
	_pzInfo.y1 = (g_hdb->_screenHeight / 4) - (_pzInfo.gfxPanic->_height >> 1);
	_pzInfo.x2 = g_hdb->_screenDrawWidth + (_pzInfo.gfxZone->_width >> 1);
	_pzInfo.y2 = (g_hdb->_screenHeight / 4) * 3 - (_pzInfo.gfxZone->_height >> 1);
	_pzInfo.xv = 10;			// horizontal speed
	_pzInfo.yv = -12;			// vertical speed
	_pzInfo.timer = 30;			// 30 seconds to get out!
}

void Window::stopPanicZone() {
	_pzInfo.active = false;
}

void Window::textOut(const char *text, int x, int y, int timer) {
	TOut *t = new TOut;

	t->x = x;
	t->y = y;
	Common::strlcpy(t->text, text, 128);
	t->timer = g_system->getMillis() + (uint32)(timer << 4);

	if (x < 0) {
		int pw, lines;
		g_hdb->_gfx->getDimensions(t->text, &pw, &lines);
		t->x = _textOutCenterX - pw / 2;
	}

	_textOutList.push_back(t);
}

void Window::centerTextOut(const char *text, int y, int timer) {
	int width, lines;
	g_hdb->_gfx->getDimensions(text, &width, &lines);
	textOut(text, _textOutCenterX - ((width - 8) >> 1), y, timer);
}

void Window::drawTextOut() {
	if (_textOutList.empty())
		return;

	int e1, e2, e3, e4;
	g_hdb->_gfx->getTextEdges(&e1, &e2, &e3, &e4);
	g_hdb->_gfx->setTextEdges(0, g_hdb->_screenDrawWidth, 0, g_hdb->_screenHeight);

	uint32 time = g_system->getMillis();

	for (uint i = 0; i < _textOutList.size(); i++) {
		TOut *t = _textOutList[i];
		g_hdb->_gfx->setCursor(t->x, t->y);
		g_hdb->_gfx->drawText(t->text);

		if (t->timer < time) {
			delete _textOutList[i];
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
