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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section6.h"
#include "mads/madsv2/nebular/rooms/dialog.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _seenRatFl;
	bool _eyesRunningFl;
	bool _shouldRemoveEyes;
	bool _ratPresentFl;
	bool _duringDialogFl;
	bool _resetBatterieText;
	bool _hermitTalkingFl;
	bool _hermitMovingFl;
	bool _alreadyTalkingFl;
	bool _giveBatteriesFl;
	bool _startTradingFl;
	bool _check1Fl;
	bool _stickFingerFl;
	int16 _randVal;
	int16 _ratHotspotId;
	int16 _hermitDialogNode;
	int16 _hermitDisplayedQuestion;
	int16 _nextFrame;
	int16 _hermitMode;
	int16 _defaultDialogPos_x;
	int16 _defaultDialogPos_y;
	int32 _ratTimer;
	Dialog _dialog1;
	Dialog _dialog2;
};

static Scratch local;


static void handleRatMoves() {
	local._ratPresentFl = false;
	_scene->_sequences.remove(_globals._sequenceIndexes[1]);
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 1, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 11, -2);
	local._ratTimer = _game._player._priorTimer;
	_scene->_dynamicHotspots.remove(local._ratHotspotId);
}

static void handleTrading() {
	if (_game._objects.isInInventory(OBJ_DURAFAIL_CELLS))
		_game._objects.setRoom(OBJ_DURAFAIL_CELLS, 1);

	if (_game._objects.isInInventory(OBJ_PHONE_CELLS))
		_game._objects.setRoom(OBJ_PHONE_CELLS, 1);

	_game._objects.addToInventory(OBJ_FAKE_ID);
}

static void setDialogNode(int node) {
	if (node > 0)
		local._hermitDialogNode = node;

	_game._player._stepEnabled = true;

	switch (node) {
	case 0:
		_scene->_userInterface.setup(kInputBuildingSentences);
		local._duringDialogFl = false;
		local._hermitDialogNode = 0;
		break;

	case 1:
		local._dialog1.start();
		local._duringDialogFl = true;
		local._hermitDialogNode = 1;
		break;

	case 2:
		local._dialog2.start();
		local._duringDialogFl = true;
		local._hermitDialogNode = 2;
		break;

	default:
		break;
	}
}

static bool check2ChargedBatteries() {
	if ((_game._objects.isInInventory(OBJ_DURAFAIL_CELLS) && !_game._objects.isInInventory(OBJ_PHONE_CELLS))
		|| (!_game._objects.isInInventory(OBJ_DURAFAIL_CELLS) && _game._objects.isInInventory(OBJ_PHONE_CELLS)))
		return true;

	return false;
}

static bool check4ChargedBatteries() {
	if (_game._objects.isInInventory(OBJ_DURAFAIL_CELLS) && _game._objects.isInInventory(OBJ_PHONE_CELLS)
		&& _globals[kDurafailRecharged])
		return true;

	return false;
}

static void handleTalking(int delay) {
	if (local._hermitTalkingFl)
		local._alreadyTalkingFl = true;

	local._hermitTalkingFl = true;
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_scene->_sequences.addTimer(delay, 100);
}

static void displayHermitQuestions(int question) {
	_scene->_kernelMessages.reset();
	local._hermitDisplayedQuestion = question;

	switch (question) {
	case 1:
	{
		const char *curQuote = _game.getQuote(0x281);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x282);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 2:
	{
		const char *curQuote = _game.getQuote(0x283);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x284);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 3:
	{
		const char *curQuote = _game.getQuote(0x285);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 4:
	{
		const char *curQuote = _game.getQuote(0x286);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 5:
	{
		const char *curQuote = _game.getQuote(0x297);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y - 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x298);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x299);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 6:
	{
		const char *curQuote = _game.getQuote(0x29A);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x29B);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 7:
	{
		const char *curQuote = _game.getQuote(0x2A0);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A1);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 8:
	{
		const char *curQuote = _game.getQuote(0x2A2);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A3);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 9:
	{
		const char *curQuote = _game.getQuote(0x2A5);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A6);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 10:
	{
		const char *curQuote = _game.getQuote(0x2A8);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A9);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2AA);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 11:
	{
		const char *curQuote = _game.getQuote(0x2AB);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2AC);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2AD);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2AE);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 12:
	{
		const char *curQuote = _game.getQuote(0x2AF);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B1);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B2);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 13:
	{
		const char *curQuote = _game.getQuote(0x2B3);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 3), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B5);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B6);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		_scene->_kernelMessages.add(Common::Point(11, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x2B7));
		_scene->_kernelMessages.add(Common::Point(11, local._defaultDialogPos_y + 73), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x2B8));
		_scene->_kernelMessages.add(Common::Point(11, local._defaultDialogPos_y + 87), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x2B9));
	}
	break;

	case 14:
	{
		const char *curQuote = _game.getQuote(0x2BA);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2BB);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2BC);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2BD);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 15:
	{
		const char *curQuote = _game.getQuote(0x2BE);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2BF);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C1);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 16:
	{
		const char *curQuote = _game.getQuote(0x2C2);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 3), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C3);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C5);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C6);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 17:
	{
		const char *curQuote = _game.getQuote(0x2C7);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C8);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C9);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2CA);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 18:
	{
		const char *curQuote = _game.getQuote(0x2CB);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2CC);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2CD);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 19:
	{
		const char *curQuote = _game.getQuote(0x2CE);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2CF);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2D0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 20:
	{
		const char *curQuote = _game.getQuote(0x2E1);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 3), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2E2);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2E3);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2E4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2E5);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
	}
	break;

	case 21:
	{
		const char *curQuote = _game.getQuote(0x2D3);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 3), 0xFDFC, 0, 0, 800, curQuote);

		curQuote = _game.getQuote(0x2D4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, 800, curQuote);

		curQuote = _game.getQuote(0x2D5);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, 800, curQuote);

		curQuote = _game.getQuote(0x2D6);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, 800, curQuote);

		curQuote = _game.getQuote(0x2D7);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, 800, curQuote);
	}
	break;

	case 22:
	{
		const char *curQuote = _game.getQuote(0x2D8);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2D9);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DA);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DB);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, 700, curQuote);
	}
	break;

	case 23:
	{
		const char *curQuote = _game.getQuote(0x2DC);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 3), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DD);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 17), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DE);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 31), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DF);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 45), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2E0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 59), 0xFDFC, 0, 0, 700, curQuote);
	}
	break;

	default:
		break;
	}
}

static void handleSubDialog1() {
	switch (_action._activeAction._verbId) {
	case 0x287:
		local._stickFingerFl = true;
		local._nextFrame = 34;
		local._hermitMovingFl = false;
		local._hermitMode = 5;
		displayHermitQuestions(5);
		local._dialog1.write(0x287, false);
		if (!local._dialog1.read(0x288))
			local._dialog1.write(0x28E, true);

		local._dialog2.write(0x29C, true);
		local._dialog2.write(0x29D, true);
		local._dialog2.write(0x29E, true);
		setDialogNode(2);
		break;

	case 0x288:
		handleTalking(500);
		displayHermitQuestions(6);
		local._dialog1.write(0x288, false);
		local._dialog1.write(0x289, true);
		if (!local._dialog1.read(0x287))
			local._dialog1.write(0x28E, true);

		setDialogNode(1);
		break;

	case 0x289:
		handleTalking(500);
		displayHermitQuestions(10);
		local._dialog1.write(0x289, false);
		local._dialog1.write(0x28A, true);
		local._dialog1.write(0x28B, true);
		setDialogNode(1);
		break;

	case 0x28A:
		handleTalking(500);
		displayHermitQuestions(11);
		local._dialog1.write(0x28A, false);
		setDialogNode(1);
		break;

	case 0x28B:
		handleTalking(500);
		displayHermitQuestions(12);
		local._dialog1.write(0x28C, true);
		local._dialog1.write(0x28D, true);
		local._dialog1.write(0x28B, false);
		setDialogNode(1);
		break;

	case 0x28C:
		handleTalking(500);
		displayHermitQuestions(13);
		local._dialog1.write(0x28C, false);
		setDialogNode(1);
		break;

	case 0x28D:
		handleTalking(500);
		displayHermitQuestions(14);
		local._dialog1.write(0x290, true);
		local._dialog1.write(0x28D, false);
		local._dialog1.write(0x28F, true);
		setDialogNode(1);
		break;

	case 0x28E:
		handleTalking(500);
		displayHermitQuestions(15);
		local._dialog1.write(0x295, true);
		local._dialog1.write(0x28E, false);
		setDialogNode(1);
		break;

	case 0x290:
		handleTalking(500);
		displayHermitQuestions(17);
		local._dialog1.write(0x290, false);
		local._dialog1.write(0x28e, false);

		if (!local._dialog1.read(0x28F))
			local._dialog1.write(0x291, true);

		setDialogNode(1);
		break;

	case 0x291:
		handleTalking(500);
		displayHermitQuestions(18);
		local._dialog1.write(0x291, false);
		if ((!_game._objects.isInInventory(OBJ_DURAFAIL_CELLS)) && (!_game._objects.isInInventory(OBJ_PHONE_CELLS))) {
			local._dialog1.write(0x292, true);
			local._dialog1.write(0x293, true);
		}

		if ((_game._objects.isInInventory(OBJ_DURAFAIL_CELLS)) || (_game._objects.isInInventory(OBJ_PHONE_CELLS)))
			local._dialog1.write(0x294, true);

		// WORKAROUND: Fix bug in the original where the option to give Hermit batteries
		// would be given before the player even has any batteries
		_globals[kHermitWantsBatteries] = true;

		setDialogNode(1);
		break;

	case 0x28F:
		handleTalking(500);
		displayHermitQuestions(16);
		local._dialog1.write(0x28F, false);
		if (!local._dialog1.read(0x290))
			local._dialog1.write(0x291, true);

		setDialogNode(1);
		break;

	case 0x295:
		handleTalking(500);
		displayHermitQuestions(20);
		local._dialog1.write(0x295, false);
		setDialogNode(1);
		break;

	case 0x292:
		handleTalking(500);
		displayHermitQuestions(19);
		local._dialog1.write(0x292, false);
		local._dialog1.write(0x293, false);
		setDialogNode(1);
		break;

	case 0x293:
	{
		handleTalking(200);
		_scene->_kernelMessages.reset();

		const char *curQuote = _game.getQuote(0x2D1);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y), 0xFDFC, 0, 0, 120, curQuote);

		curQuote = _game.getQuote(0x2D2);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, 120, curQuote);

		local._dialog1.write(0x293, false);
		setDialogNode(0);
	}
	break;

	case 0x294:
	{
		bool hermitPleasedFl = false;

		switch (_game._difficulty) {
		case DIFFICULTY_EASY:
			hermitPleasedFl = _game._objects.isInInventory(OBJ_DURAFAIL_CELLS) || _game._objects.isInInventory(OBJ_PHONE_CELLS);
			break;

		case DIFFICULTY_MEDIUM:
			hermitPleasedFl = _game._objects.isInInventory(OBJ_DURAFAIL_CELLS) && _game._objects.isInInventory(OBJ_PHONE_CELLS);
			break;

		default: // HARD
			hermitPleasedFl = check4ChargedBatteries();
			break;
		}

		if (hermitPleasedFl) {
			local._hermitDisplayedQuestion = 21;
			if (!local._giveBatteriesFl)
				setDialogNode(0);
			else
				local._giveBatteriesFl = false;
		} else if (((_game._difficulty == DIFFICULTY_MEDIUM) || (_game._difficulty == DIFFICULTY_HARD)) && check2ChargedBatteries()) {
			local._hermitDisplayedQuestion = 22;
			if (!local._giveBatteriesFl)
				setDialogNode(0);
			else
				local._giveBatteriesFl = false;
		} else {
			local._hermitDisplayedQuestion = 23;
			if (!local._giveBatteriesFl)
				setDialogNode(0);
			else
				local._giveBatteriesFl = false;
		}
		local._startTradingFl = true;
	}
	break;

	case 0x296:
	{
		_scene->_kernelMessages.reset();

		const char *curQuote = _game.getQuote(0x2E6);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, 120, curQuote);

		setDialogNode(0);
		handleTalking(200);
	}
	break;

	default:
		break;
	}
}

static void handleSubDialog2() {
	switch (_action._activeAction._verbId) {
	case 0x29C:
		displayHermitQuestions(7);
		setDialogNode(1);
		handleTalking(500);
		break;

	case 0x29D:
		displayHermitQuestions(8);
		setDialogNode(1);
		handleTalking(500);
		break;

	case 0x29E:
		displayHermitQuestions(9);
		setDialogNode(1);
		handleTalking(500);
		break;

	case 0x29F:
	{
		_scene->_kernelMessages.reset();
		const char *curQuote = _game.getQuote(0x2A7);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, 120, curQuote);
		setDialogNode(0);
		local._dialog2.write(0x29F, false);
	}
	break;

	default:
		break;
	}
}

static void handleDialog() {
	if (_game._trigger == 0) {
		_scene->_kernelMessages.reset();
		_game._player._stepEnabled = false;

		const char *curQuote = _game.getQuote(_action._activeAction._verbId);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);

		if (width > 200) {
			static char subQuote1[34], subQuote2[34];
			_game.splitQuote(curQuote, subQuote1, subQuote2);
			_scene->_kernelMessages.add(Common::Point(0, -14), 0x1110, 34, 0, 150, subQuote1);

			if (_action._activeAction._verbId == 0x29D)
				_scene->_kernelMessages.add(Common::Point(-18, 0), 0x1110, 34, 1, 150, subQuote2);
			else if (_action._activeAction._verbId == 0x28A)
				_scene->_kernelMessages.add(Common::Point(-10, 0), 0x1110, 34, 1, 150, subQuote2);
			else
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 150, subQuote2);

			_scene->_sequences.addTimer(170, 50);
		} else {
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, curQuote);
			_scene->_sequences.addTimer(140, 50);
		}
	} else if (_game._trigger == 50) {
		if (local._hermitDialogNode == 1)
			handleSubDialog1();
		else if (local._hermitDialogNode == 2)
			handleSubDialog2();
	}
}

static void room_611_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_9");

	_game.loadQuoteSet(0x279, 0x27A, 0x27B, 0x27C, 0x27D, 0x27E, 0x27F, 0x280, 0x281, 0x282, 0x283, 0x284,
		0x285, 0x286, 0x287, 0x288, 0x289, 0x28A, 0x28B, 0x28C, 0x28D, 0x28E, 0x28F, 0x290, 0x291, 0x292,
		0x293, 0x294, 0x295, 0x296, 0x297, 0x298, 0x299, 0x29A, 0x29B, 0x29C, 0x29D, 0x29E, 0x29F, 0x2A0,
		0x2A1, 0x2A2, 0x2A3, 0x2A4, 0x2A5, 0x2A6, 0x2A7, 0x2A8, 0x2A9, 0x2AA, 0x2AB, 0x2AC, 0x2AD, 0x2AE,
		0x2AF, 0x2B0, 0x2B1, 0x2B2, 0x2B3, 0x2B4, 0x2B5, 0x2B6, 0x2B7, 0x2B8, 0x2B9, 0x2BA, 0x2BB, 0x2BC,
		0x2BD, 0x2BE, 0x2BF, 0x2C0, 0x2C1, 0x2C2, 0x2C3, 0x2C4, 0x2C5, 0x2C6, 0x2C7, 0x2C8, 0x2C9, 0x2CA,
		0x2CB, 0x2CC, 0x2CD, 0x2CE, 0x2CF, 0x2D0, 0x2D1, 0x2D2, 0x2D3, 0x2D4, 0x2D5, 0x2D6, 0x2D7, 0x2D8,
		0x2D9, 0x2DA, 0x2DB, 0x2DC, 0x2DD, 0x2DE, 0x2DF, 0x2E0, 0x2E1, 0x2E2, 0x2E3, 0x2E4, 0x2E5, 0x2E6,
		0x323, 0x324, 0);

	local._dialog1.setup(kConvHermit1, 0x287, 0x288, 0x289, 0x28A, 0x28B, 0x28C, 0x28D, 0x28E, 0x28F, 0x290,
		0x291, 0x292, 0x293, 0x294, 0x295, 0x296, 0);

	local._dialog2.setup(kConvHermit2, 0x29C, 0x29D, 0x29E, 0x29F, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		local._dialog1.set(kConvHermit1, 0x287, 0x288, 0x296, 0);
		local._dialog2.set(kConvHermit2, 0x29F, 0);
	}

	_vm->_palette->setEntry(252, 51, 51, 47);
	_vm->_palette->setEntry(253, 37, 37, 37);

	local._ratPresentFl = false;
	local._seenRatFl = true;
	local._eyesRunningFl = false;
	local._shouldRemoveEyes = false;
	local._randVal = 0;
	local._defaultDialogPos_x = 264;
	local._defaultDialogPos_y = 43;
	local._giveBatteriesFl = false;
	local._resetBatterieText = false;
	local._alreadyTalkingFl = false;
	local._startTradingFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(22, 132);
		_game._player._facing = FACING_EAST;
		local._duringDialogFl = false;
	}

	if (!_globals[kHasTalkedToHermit]) {
		_scene->loadAnimation(Resources::formatName(611, 'h', -1, EXT_AA, ""), 0);
		local._nextFrame = 47;
		local._hermitMode = 1;
		local._hermitTalkingFl = false;
		local._hermitMovingFl = true;
		local._check1Fl = true;
		local._stickFingerFl = false;
	} else {
		local._hermitMode = 0;
		_scene->_hotspots.activate(words_hermit, false);
	}

	// WORKAROUND: Fix original adding 'give batteries' option even if you don't have them
	if (_globals[kHermitWantsBatteries]) {
		if ((_game._objects.isInInventory(OBJ_DURAFAIL_CELLS)) || (_game._objects.isInInventory(OBJ_PHONE_CELLS)))
			local._dialog1.write(0x294, true);
	}

	if (local._duringDialogFl) {
		_game._player._playerPos = Common::Point(237, 129);
		_game._player._facing = FACING_NORTHEAST;

		switch (local._hermitDialogNode) {
		case 0:
			_scene->_userInterface.setup(kInputBuildingSentences);
			local._hermitDialogNode = 1;
			break;

		case 1:
			local._dialog1.start();
			break;

		case 2:
			local._dialog2.start();
			break;

		default:
			break;
		}
		displayHermitQuestions(local._hermitDisplayedQuestion);
	}

	section_6_music();
}

static void room_611_daemon() {
	if (local._seenRatFl && (_vm->getRandomNumber(1, 100) == 10)) {
		local._seenRatFl = false;
		_scene->_sequences.addTimer(1, 80);
	}

	if (_game._trigger == 80) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 8);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		local._ratPresentFl = true;
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
	} else if (_game._trigger == 81) {
		int syncId = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 20, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(words_rat, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		local._ratHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(272, 154), FACING_SOUTHEAST);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 9, 10);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncId);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		local._ratTimer = _game._player._priorTimer;
	}

	if (local._ratPresentFl && ((_game._player._priorTimer - local._ratTimer) > 1200))
		handleRatMoves();

	if (!local._eyesRunningFl) {
		local._randVal = _vm->getRandomNumber(1, 30);
		local._eyesRunningFl = true;
		_scene->_sequences.addTimer(1, 70);
	}

	if (_game._trigger == 70) {
		switch (local._randVal) {
		case 2:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			local._shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 6:
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 12, 3, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 4);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 7:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			local._shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 9:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			local._shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 13:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 7);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			local._shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 14:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 8);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			local._shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 15:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 24, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, 8);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 17:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 20, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 9, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 21:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 9);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			local._shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 25:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 10);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			local._shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 27:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			local._shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 29:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 20, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		default:
			_scene->_sequences.addTimer(1, 71);
			break;
		}
	}

	if (_game._trigger == 71) {
		if (local._shouldRemoveEyes) {
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			local._shouldRemoveEyes = false;
		}
		local._eyesRunningFl = false;
		local._randVal = 0;
	}

	if (_game._trigger == 100) {
		if (local._alreadyTalkingFl)
			local._alreadyTalkingFl = false;
		else
			local._hermitMovingFl = true;
	}

	if (local._stickFingerFl && (_scene->_animation[0]->getCurrentFrame() == 47)) {
		local._stickFingerFl = false;
		local._hermitMovingFl = true;
		local._hermitMode = 1;
	}

	if (_scene->_animation[0] != nullptr && (_scene->_animation[0]->getCurrentFrame() == 240) && local._check1Fl) {
		local._check1Fl = false;
		_scene->_kernelMessages.add(Common::Point(33, 88), 0xFDFC, 0, 0, 90, _game.getQuote(0x27E));
		_scene->_sequences.addTimer(120, 120);
	}

	if (_game._trigger == 120) {
		int msgIdx = _scene->_kernelMessages.add(Common::Point(28, 102), 0xFDFC, 0, 0, 90, _game.getQuote(0x27F));
		_scene->_kernelMessages.setQuoted(msgIdx, 4, true);
		_scene->_sequences.addTimer(100, 121);
	}

	if (_game._trigger == 121) {
		int msgIdx = _scene->_kernelMessages.add(Common::Point(23, 116), 0xFDFC, 0, 0, 90, _game.getQuote(0x280));
		_scene->_kernelMessages.setQuoted(msgIdx, 4, true);
	}

	if (local._hermitMode == 1) {
		if (local._startTradingFl) {
			local._hermitMode = 6;
			local._hermitMovingFl = false;
			local._hermitTalkingFl = false;
			_scene->_sequences.addTimer(1, 110);
		} else if (local._hermitTalkingFl) {
			local._hermitMode = 2;
			local._nextFrame = 18;
			local._hermitMovingFl = false;
		} else {
			switch (_vm->getRandomNumber(1, 5)) {
			case 1:
				local._nextFrame = 46;
				break;

			case 2:
				local._nextFrame = 47;
				break;

			case 3:
				local._nextFrame = 48;
				break;

			case 4:
				local._nextFrame = 49;
				break;

			case 5:
				local._nextFrame = 50;
				break;

			default:
				break;
			}
		}
	}

	if (local._hermitMode == 2) {
		if (local._startTradingFl) {
			local._hermitMode = 6;
			local._hermitMovingFl = false;
			local._hermitTalkingFl = false;
			_scene->_sequences.addTimer(1, 110);
		} else if (local._hermitMovingFl) {
			local._hermitMode = 1;
			local._nextFrame = 47;
			local._hermitTalkingFl = false;
		} else {
			switch (_vm->getRandomNumber(1, 4)) {
			case 1:
				local._nextFrame = 18;
				break;

			case 2:
				local._nextFrame = 20;
				break;

			case 3:
				local._nextFrame = 22;
				break;

			case 4:
				local._nextFrame = 24;
				break;

			default:
				break;
			}
		}
	}

	if (_scene->_animation[0] != nullptr && _scene->_animation[0]->getCurrentFrame() == 254)
		_game._player._stepEnabled = true;

	if (_game._trigger == 110) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 111);
	}

	if (_game._trigger == 111) {
		int syncIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		local._nextFrame = 1;
	}

	if (_game._trigger == 112) {
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._visible = true;
	}

	if (local._hermitMode == 6) {
		if ((_scene->_animation[0]->getCurrentFrame() == 9) && local._check1Fl) {
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 112);
			local._check1Fl = false;
		}

		if ((_scene->_animation[0]->getCurrentFrame() == 17) && !local._check1Fl) {
			local._nextFrame = 26;
			local._hermitMode = 4;
			local._check1Fl = true;
		}
	}

	if (local._hermitMode == 4) {
		if ((_scene->_animation[0]->getCurrentFrame() == 33) && local._check1Fl) {
			displayHermitQuestions(local._hermitDisplayedQuestion);
			local._nextFrame = 1;
			local._check1Fl = false;
		}

		if ((_scene->_animation[0]->getCurrentFrame() == 9) && !local._check1Fl) {
			local._nextFrame = 8;
			_scene->_sequences.addTimer(1, 113);
			local._check1Fl = true;
		}
	}

	if (_game._trigger == 113) {
		_game._player._visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 114);
	}

	if (_game._trigger == 114) {
		local._resetBatterieText = true;
		int syncIdx = _globals._sequenceIndexes[3];
		local._nextFrame = 10;
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 115);
	}

	if ((local._nextFrame >= 0) && (local._nextFrame != _scene->_animation[0]->getCurrentFrame())) {
		_scene->_animation[0]->setCurrentFrame(local._nextFrame);
		local._nextFrame = -1;
	}

	if (_game._trigger == 115) {
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._visible = true;
		if (local._hermitDisplayedQuestion == 21) {
			_game._player._stepEnabled = false;
			handleTrading();
			local._hermitMode = 0;
			local._startTradingFl = false;
			local._nextFrame = 52;
			_globals[kHasTalkedToHermit] = true;
			_scene->_hotspots.activate(words_hermit, false);
		} else {
			_game._player._stepEnabled = true;
			local._hermitMode = 1;
			local._nextFrame = 47;
			local._hermitTalkingFl = false;
			local._startTradingFl = false;
			local._check1Fl = true;
		}
	}
}

static void room_611_pre_parser() {
	if (player_said_2(walk_down, alley))
		_game._player._walkOffScreenSceneId = 609;

	if (local._resetBatterieText)
		_scene->_kernelMessages.reset();
}

static void room_611_parser() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleDialog();
	else if ((player_said_3(give, phone_cells, hermit)) || (player_said_3(give, durafail_cells, hermit))) {
		_action._activeAction._verbId = 0x294;
		local._giveBatteriesFl = true;
		handleSubDialog1();
	} else if (player_said_2(give, hermit)) {
		_scene->_kernelMessages.reset();

		const char *curQuote = _game.getQuote(0x323);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y), 0xFDFC, 0, 0, 120, curQuote);

		curQuote = _game.getQuote(0x324);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, local._defaultDialogPos_y + 14), 0xFDFC, 0, 0, 120, curQuote);
	} else if (_game._trigger == 90) {
		if (local._dialog2.read(0x29C) && local._dialog2.read(0x29D) && local._dialog2.read(0x29E)) {
			handleTalking(180);
			if (_vm->getRandomNumber(1, 2) == 1)
				displayHermitQuestions(1);
			else
				displayHermitQuestions(2);
		} else {
			handleTalking(180);
			if (_vm->getRandomNumber(1, 2) == 1)
				displayHermitQuestions(3);
			else
				displayHermitQuestions(4);
		}

		local._duringDialogFl = true;
		if (local._dialog2.read(0x29F)) {
			local._hermitDialogNode = 1;
			local._dialog1.start();
			local._duringDialogFl = true;
		} else {
			local._hermitDialogNode = 2;
			local._dialog2.write(0x29F, true);
			local._dialog2.start();
			local._duringDialogFl = true;
		}
	} else if (player_said_2(talkto, hermit)) {
		if (!local._dialog1.read(0x287)) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x27A));
			_scene->_sequences.addTimer(120, 90);
		} else {
			int nextQuote = 0;
			switch (_vm->getRandomNumber(1, 3)) {
			case 1:
				nextQuote = 0x27B;
				break;

			case 2:
				nextQuote = 0x27C;
				break;

			case 3:
				nextQuote = 0x27D;
				break;

			default:
				break;
			}
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(nextQuote));
			_scene->_sequences.addTimer(120, 90);
		}
	} else if ((player_said_1(walkto) || player_said_1(look)) && player_said_1(rat)) {
		switch (_game._trigger) {
		case 0:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x279));
			_scene->_sequences.addTimer(60, 1);
			break;

		case 1:
			handleRatMoves();
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action._lookFlag) {
		if (_globals[kHasTalkedToHermit])
			_vm->_dialogs->show(61111);
		else
			_vm->_dialogs->show(61110);
	} else if (player_said_2(look, hermit))
		_vm->_dialogs->show(61112);
	else if (player_said_2(look, trash))
		_vm->_dialogs->show(61113);
	else if (player_said_2(take, trash))
		_vm->_dialogs->show(61114);
	else if (player_said_2(look, cardboard_box))
		_vm->_dialogs->show(61115);
	else if (player_said_2(take, cardboard_box))
		_vm->_dialogs->show(61116);
	else if (player_said_2(open, cardboard_box))
		_vm->_dialogs->show(61117);
	else if (player_said_2(look, refrigerator))
		_vm->_dialogs->show(61118);
	else if (player_said_2(open, refrigerator))
		_vm->_dialogs->show(61119);
	else if (player_said_2(take, refrigerator))
		_vm->_dialogs->show(61120);
	else if (player_said_2(look, building))
		_vm->_dialogs->show(61121);
	else if (player_said_2(look, graffiti))
		_vm->_dialogs->show(61122);
	else if (player_said_2(look, metal_pipe))
		_vm->_dialogs->show(61123);
	else
		return;

	_action._inProgress = false;
}

void room_611_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._seenRatFl);
	s.syncAsByte(local._eyesRunningFl);
	s.syncAsByte(local._shouldRemoveEyes);
	s.syncAsByte(local._ratPresentFl);
	s.syncAsByte(local._duringDialogFl);
	s.syncAsByte(local._resetBatterieText);
	s.syncAsByte(local._hermitTalkingFl);
	s.syncAsByte(local._hermitMovingFl);
	s.syncAsByte(local._alreadyTalkingFl);
	s.syncAsByte(local._giveBatteriesFl);
	s.syncAsByte(local._startTradingFl);
	s.syncAsByte(local._check1Fl);
	s.syncAsByte(local._stickFingerFl);

	s.syncAsSint16LE(local._randVal);
	s.syncAsSint16LE(local._ratHotspotId);
	s.syncAsSint16LE(local._hermitDialogNode);
	s.syncAsSint16LE(local._hermitDisplayedQuestion);
	s.syncAsSint16LE(local._nextFrame);
	s.syncAsSint16LE(local._hermitMode);
	s.syncAsSint16LE(local._defaultDialogPos_x);
	s.syncAsSint16LE(local._defaultDialogPos_y);

	s.syncAsUint32LE(local._ratTimer);
}

void room_611_preload() {
	room_init_code_pointer = room_611_init;
	room_daemon_code_pointer = room_611_daemon;
	room_pre_parser_code_pointer = room_611_pre_parser;
	room_parser_code_pointer = room_611_parser;

	section_6_walker();
	section_6_interface();
	_scene->addActiveVocab(words_rat);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
