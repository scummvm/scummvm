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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section6.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
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
	kernel_seq_delete(g_sequence_ids[1]);
	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 12, 0, 0, 1);
	kernel_seq_range(g_sequence_ids[1], 11, -2);
	local._ratTimer = player.clock;
	kernel_delete_dynamic(local._ratHotspotId);
}

static void handleTrading() {
	if (player_has(OBJ_DURAFAIL_CELLS))
		inter_move_object(OBJ_DURAFAIL_CELLS, 1);

	if (player_has(OBJ_PHONE_CELLS))
		inter_move_object(OBJ_PHONE_CELLS, 1);

	inter_give_to_player(OBJ_FAKE_ID);
}

static void setDialogNode(int node) {
	if (node > 0)
		local._hermitDialogNode = node;

	player.commands_allowed = true;

	switch (node) {
	case 0:
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
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
	return bool(player_has(OBJ_DURAFAIL_CELLS)) != bool(player_has(OBJ_PHONE_CELLS));
}

static bool check4ChargedBatteries() {
	if (player_has(OBJ_DURAFAIL_CELLS) && player_has(OBJ_PHONE_CELLS)
		&& global[kDurafailRecharged])
		return true;

	return false;
}

static void handleTalking(int delay) {
	if (local._hermitTalkingFl)
		local._alreadyTalkingFl = true;

	local._hermitTalkingFl = true;
	kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
	kernel_timing_trigger(delay, 100);
}

static void displayHermitQuestions(int question) {
	kernel_message_purge();
	local._hermitDisplayedQuestion = question;

	switch (question) {
	case 1:
	{
		char *curQuote = quote_string(kernel.quotes, 0x281);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x282);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 2:
	{
		char *curQuote = quote_string(kernel.quotes, 0x283);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x284);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 3:
	{
		char *curQuote = quote_string(kernel.quotes, 0x285);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 4:
	{
		char *curQuote = quote_string(kernel.quotes, 0x286);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 5:
	{
		char *curQuote = quote_string(kernel.quotes, 0x297);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y - 14, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x298);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x299);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 6:
	{
		char *curQuote = quote_string(kernel.quotes, 0x29A);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x29B);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 7:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2A0);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2A1);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 8:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2A2);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2A3);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2A4);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 9:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2A5);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2A6);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 10:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2A8);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2A9);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2AA);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 11:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2AB);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2AC);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2AD);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2AE);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 12:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2AF);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2B0);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2B1);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2B2);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 13:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2B3);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 3, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2B4);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2B5);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2B6);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		kernel_message_add(quote_string(kernel.quotes, 0x2B7), 11, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x2B8), 11, local._defaultDialogPos_y + 73, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x2B9), 11, local._defaultDialogPos_y + 87, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 14:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2BA);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2BB);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2BC);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2BD);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 15:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2BE);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2BF);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2C0);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2C1);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 16:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2C2);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 3, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2C3);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2C4);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2C5);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2C6);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 17:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2C7);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2C8);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2C9);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2CA);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 18:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2CB);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2CC);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2CD);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 19:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2CE);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2CF);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2D0);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 20:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2E1);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 3, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2E2);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2E3);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2E4);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2E5);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
	}
	break;

	case 21:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2D3);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 3, 0xFDFC, 800, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2D4);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, 800, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2D5);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, 800, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2D6);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, 800, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2D7);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, 800, 0, 0);
	}
	break;

	case 22:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2D8);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, 700, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2D9);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, 700, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2DA);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, 700, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2DB);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, 700, 0, 0);
	}
	break;

	case 23:
	{
		char *curQuote = quote_string(kernel.quotes, 0x2DC);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 3, 0xFDFC, 700, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2DD);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 17, 0xFDFC, 700, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2DE);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 31, 0xFDFC, 700, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2DF);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 45, 0xFDFC, 700, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2E0);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 59, 0xFDFC, 700, 0, 0);
	}
	break;

	default:
		break;
	}
}

static void handleSubDialog1() {
	switch (player2.words[0]) {
	case words_air_purifier:
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

	case words_dummy:
		handleTalking(500);
		displayHermitQuestions(6);
		local._dialog1.write(0x288, false);
		local._dialog1.write(0x289, true);
		if (!local._dialog1.read(0x287))
			local._dialog1.write(0x28E, true);

		setDialogNode(1);
		break;

	case words_picture:
		handleTalking(500);
		displayHermitQuestions(10);
		local._dialog1.write(0x289, false);
		local._dialog1.write(0x28A, true);
		local._dialog1.write(0x28B, true);
		setDialogNode(1);
		break;

	case words_baggage_check:
		handleTalking(500);
		displayHermitQuestions(11);
		local._dialog1.write(0x28A, false);
		setDialogNode(1);
		break;

	case words_departure_schedule:
		handleTalking(500);
		displayHermitQuestions(12);
		local._dialog1.write(0x28C, true);
		local._dialog1.write(0x28D, true);
		local._dialog1.write(0x28B, false);
		setDialogNode(1);
		break;

	case words_shade_of_pale:
		handleTalking(500);
		displayHermitQuestions(13);
		local._dialog1.write(0x28C, false);
		setDialogNode(1);
		break;

	case words_itinerary:
		handleTalking(500);
		displayHermitQuestions(14);
		local._dialog1.write(0x290, true);
		local._dialog1.write(0x28D, false);
		local._dialog1.write(0x28F, true);
		setDialogNode(1);
		break;

	case words_bargain_vat:
		handleTalking(500);
		displayHermitQuestions(15);
		local._dialog1.write(0x295, true);
		local._dialog1.write(0x28E, false);
		setDialogNode(1);
		break;

	case words_pad_if_paper:
		handleTalking(500);
		displayHermitQuestions(17);
		local._dialog1.write(0x290, false);
		local._dialog1.write(0x28e, false);

		if (!local._dialog1.read(0x28F))
			local._dialog1.write(0x291, true);

		setDialogNode(1);
		break;

	case words_deepest_depths:
		handleTalking(500);
		displayHermitQuestions(18);
		local._dialog1.write(0x291, false);
		if ((!player_has(OBJ_DURAFAIL_CELLS)) && (!player_has(OBJ_PHONE_CELLS))) {
			local._dialog1.write(0x292, true);
			local._dialog1.write(0x293, true);
		}

		if ((player_has(OBJ_DURAFAIL_CELLS)) || (player_has(OBJ_PHONE_CELLS)))
			local._dialog1.write(0x294, true);

		// WORKAROUND: Fix bug in the original where the option to give Hermit batteries
		// would be given before the player even has any batteries
		global[kHermitWantsBatteries] = true;

		setDialogNode(1);
		break;

	case words_pencils:
		handleTalking(500);
		displayHermitQuestions(16);
		local._dialog1.write(0x28F, false);
		if (!local._dialog1.read(0x290))
			local._dialog1.write(0x291, true);

		setDialogNode(1);
		break;

	case words_space_questing_viii:
		handleTalking(500);
		displayHermitQuestions(20);
		local._dialog1.write(0x295, false);
		setDialogNode(1);
		break;

	case words_rolodex:
		handleTalking(500);
		displayHermitQuestions(19);
		local._dialog1.write(0x292, false);
		local._dialog1.write(0x293, false);
		setDialogNode(1);
		break;

	case words_mona_tisa:
	{
		handleTalking(200);
		kernel_message_purge();

		char *curQuote = quote_string(kernel.quotes, 0x2D1);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y, 0xFDFC, 120, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x2D2);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, 120, 0, 0);

		local._dialog1.write(0x293, false);
		setDialogNode(0);
	}
	break;

	case words_abstract_art:
	{
		bool hermitPleasedFl = false;

		switch (game.difficulty) {
		case DIFFICULTY_EASY:
			hermitPleasedFl = player_has(OBJ_DURAFAIL_CELLS) || player_has(OBJ_PHONE_CELLS);
			break;

		case DIFFICULTY_MEDIUM:
			hermitPleasedFl = player_has(OBJ_DURAFAIL_CELLS) && player_has(OBJ_PHONE_CELLS);
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
		} else if (((game.difficulty == DIFFICULTY_MEDIUM) || (game.difficulty == DIFFICULTY_HARD)) && check2ChargedBatteries()) {
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

	case words_theatrical_art:
	{
		kernel_message_purge();

		char *curQuote = quote_string(kernel.quotes, 0x2E6);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, 120, 0, 0);

		setDialogNode(0);
		handleTalking(200);
	}
	break;

	default:
		break;
	}
}

static void handleSubDialog2() {
	switch (player2.words[0]) {
	case words_our_town:
		displayHermitQuestions(7);
		setDialogNode(1);
		handleTalking(500);
		break;

	case words_embroidered_art:
		displayHermitQuestions(8);
		setDialogNode(1);
		handleTalking(500);
		break;

	case words_george_bush_alike:
		displayHermitQuestions(9);
		setDialogNode(1);
		handleTalking(500);
		break;

	case words_counter:
	{
		kernel_message_purge();
		char *curQuote = quote_string(kernel.quotes, 0x2A7);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, 120, 0, 0);
		setDialogNode(0);
		local._dialog2.write(0x29F, false);
	}
	break;

	default:
		break;
	}
}

static void handleDialog() {
	if (kernel.trigger == 0) {
		kernel_message_purge();
		player.commands_allowed = false;

		char *curQuote = quote_string(kernel.quotes, player2.words[0]);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);

		if (width > 200) {
			static char subQuote1[34], subQuote2[34];
			quote_split_string(curQuote, subQuote1, subQuote2);
			kernel_message_add(subQuote1, 0, -14, 0x1110, 150, 0, 34);

			if (player2.words[0] == words_embroidered_art)
				kernel_message_add(subQuote2, -18, 0, 0x1110, 150, 1, 34);
			else if (player2.words[0] == words_baggage_check)
				kernel_message_add(subQuote2, -10, 0, 0x1110, 150, 1, 34);
			else
				kernel_message_add(subQuote2, 0, 0, 0x1110, 150, 1, 34);

			kernel_timing_trigger(170, 50);
		} else {
			kernel_message_add(curQuote, 0, 0, 0x1110, 120, 1, 34);
			kernel_timing_trigger(140, 50);
		}
	} else if (kernel.trigger == 50) {
		if (local._hermitDialogNode == 1)
			handleSubDialog1();
		else if (local._hermitDialogNode == 2)
			handleSubDialog2();
	}
}

static void room_611_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[3] = kernel_load_series("*RXMRC_9", 0);

	kernel.quotes = quote_load(633, 634, 635, 636, 637, 638, 639, 640, 641, 642, 643, 644,
		645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658,
		659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672,
		673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686,
		687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700,
		701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714,
		715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728,
		729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742,
		803, 804, 0);

	local._dialog1.setup(kConvHermit1, 0x287, 0x288, 0x289, 0x28A, 0x28B, 0x28C, 0x28D, 0x28E, 0x28F, 0x290,
		0x291, 0x292, 0x293, 0x294, 0x295, 0x296, 0);

	local._dialog2.setup(kConvHermit2, 0x29C, 0x29D, 0x29E, 0x29F, 0);

	if (!player.been_here_before) {
		local._dialog1.set(kConvHermit1, 0x287, 0x288, 0x296, 0);
		local._dialog2.set(kConvHermit2, 0x29F, 0);
	}

	pal_change_color(252, 51, 51, 47);
	pal_change_color(253, 37, 37, 37);

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

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 22;
		player.y = 132;
		player.facing = FACING_EAST;
		local._duringDialogFl = false;
	}

	if (!global[kHasTalkedToHermit]) {
		kernel_run_animation(kernel_full_name(611, 'h', -1, "", KERNEL_AA), 0);
		local._nextFrame = 47;
		local._hermitMode = 1;
		local._hermitTalkingFl = false;
		local._hermitMovingFl = true;
		local._check1Fl = true;
		local._stickFingerFl = false;
	} else {
		local._hermitMode = 0;
		kernel_flip_hotspot(words_hermit, false);
	}

	// WORKAROUND: Fix original adding 'give batteries' option even if you don't have them
	if (global[kHermitWantsBatteries]) {
		if ((player_has(OBJ_DURAFAIL_CELLS)) || (player_has(OBJ_PHONE_CELLS)))
			local._dialog1.write(0x294, true);
	}

	if (local._duringDialogFl) {
		player.x = 237;
		player.y = 129;
		player.facing = FACING_NORTHEAST;

		switch (local._hermitDialogNode) {
		case 0:
			kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
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
	if (local._seenRatFl && (g_engine->getRandomNumber(1, 100) == 10)) {
		local._seenRatFl = false;
		kernel_timing_trigger(1, 80);
	}

	if (kernel.trigger == 80) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[1], 1, 8);
		kernel_seq_depth(g_sequence_ids[1], 1);
		local._ratPresentFl = true;
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 81);
	} else if (kernel.trigger == 81) {
		int syncId = g_sequence_ids[1];
		g_sequence_ids[1] = kernel_seq_pingpong(g_sprite_ids[1], false, 20, 0, 0, 0);
		int idx = kernel_add_dynamic(words_rat, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 272, 154, FACING_SOUTHEAST);
		local._ratHotspotId = idx;
		kernel_seq_range(g_sequence_ids[1], 9, 10);
		kernel_seq_timeout(syncId, g_sequence_ids[1]);
		kernel_seq_depth(g_sequence_ids[1], 1);
		local._ratTimer = player.clock;
	}

	if (local._ratPresentFl && ((player.clock - local._ratTimer) > 1200))
		handleRatMoves();

	if (!local._eyesRunningFl) {
		local._randVal = g_engine->getRandomNumber(1, 30);
		local._eyesRunningFl = true;
		kernel_timing_trigger(1, 70);
	}

	if (kernel.trigger == 70) {
		switch (local._randVal) {
		case 2:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
			kernel_seq_depth(g_sequence_ids[2], 1);
			local._shouldRemoveEyes = true;
			kernel_timing_trigger(60, 71);
			break;

		case 6:
			g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 12, 0, 0, 3);
			kernel_seq_range(g_sequence_ids[2], 2, 4);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 71);
			break;

		case 7:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 5);
			kernel_seq_depth(g_sequence_ids[2], 1);
			local._shouldRemoveEyes = true;
			kernel_timing_trigger(60, 71);
			break;

		case 9:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 6);
			kernel_seq_depth(g_sequence_ids[2], 1);
			local._shouldRemoveEyes = true;
			kernel_timing_trigger(60, 71);
			break;

		case 13:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 7);
			kernel_seq_depth(g_sequence_ids[2], 1);
			local._shouldRemoveEyes = true;
			kernel_timing_trigger(60, 71);
			break;

		case 14:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 8);
			kernel_seq_depth(g_sequence_ids[2], 1);
			local._shouldRemoveEyes = true;
			kernel_timing_trigger(60, 71);
			break;

		case 15:
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 24, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[2], 5, 8);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 71);
			break;

		case 17:
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 20, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[2], 9, 11);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 71);
			break;

		case 21:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 9);
			kernel_seq_depth(g_sequence_ids[2], 1);
			local._shouldRemoveEyes = true;
			kernel_timing_trigger(60, 71);
			break;

		case 25:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 10);
			kernel_seq_depth(g_sequence_ids[2], 1);
			local._shouldRemoveEyes = true;
			kernel_timing_trigger(60, 71);
			break;

		case 27:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 11);
			kernel_seq_depth(g_sequence_ids[2], 1);
			local._shouldRemoveEyes = true;
			kernel_timing_trigger(60, 71);
			break;

		case 29:
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 20, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 71);
			break;

		default:
			kernel_timing_trigger(1, 71);
			break;
		}
	}

	if (kernel.trigger == 71) {
		if (local._shouldRemoveEyes) {
			kernel_seq_delete(g_sequence_ids[2]);
			local._shouldRemoveEyes = false;
		}
		local._eyesRunningFl = false;
		local._randVal = 0;
	}

	if (kernel.trigger == 100) {
		if (local._alreadyTalkingFl)
			local._alreadyTalkingFl = false;
		else
			local._hermitMovingFl = true;
	}

	if (local._stickFingerFl && (kernel_anim[0].frame == 47)) {
		local._stickFingerFl = false;
		local._hermitMovingFl = true;
		local._hermitMode = 1;
	}

	if (kernel_anim[0].anim != nullptr && (kernel_anim[0].frame == 240) && local._check1Fl) {
		local._check1Fl = false;
		kernel_message_add(quote_string(kernel.quotes, 0x27E), 33, 88, 0xFDFC, 90, 0, 0);
		kernel_timing_trigger(120, 120);
	}

	if (kernel.trigger == 120) {
		int msgIdx = kernel_message_add(quote_string(kernel.quotes, 0x27F), 28, 102, 0xFDFC, 90, 0, 0);
		kernel_message_teletype(msgIdx, 4, true);
		kernel_timing_trigger(100, 121);
	}

	if (kernel.trigger == 121) {
		int msgIdx = kernel_message_add(quote_string(kernel.quotes, 0x280), 23, 116, 0xFDFC, 90, 0, 0);
		kernel_message_teletype(msgIdx, 4, true);
	}

	if (local._hermitMode == 1) {
		if (local._startTradingFl) {
			local._hermitMode = 6;
			local._hermitMovingFl = false;
			local._hermitTalkingFl = false;
			kernel_timing_trigger(1, 110);
		} else if (local._hermitTalkingFl) {
			local._hermitMode = 2;
			local._nextFrame = 18;
			local._hermitMovingFl = false;
		} else {
			switch (g_engine->getRandomNumber(1, 5)) {
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
			kernel_timing_trigger(1, 110);
		} else if (local._hermitMovingFl) {
			local._hermitMode = 1;
			local._nextFrame = 47;
			local._hermitTalkingFl = false;
		} else {
			switch (g_engine->getRandomNumber(1, 4)) {
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

	if (kernel_anim[0].anim != nullptr && kernel_anim[0].frame == 254)
		player.commands_allowed = true;

	if (kernel.trigger == 110) {
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 1, 2);
		kernel_seq_player(g_sequence_ids[3], false);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 111);
	}

	if (kernel.trigger == 111) {
		int syncIdx = g_sequence_ids[3];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 2);
		kernel_seq_player(g_sequence_ids[3], false);
		kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
		local._nextFrame = 1;
	}

	if (kernel.trigger == 112) {
		player.clock = kernel.clock - player.frame_delay;
		player.walker_visible = true;
	}

	if (local._hermitMode == 6) {
		if ((kernel_anim[0].frame == 9) && local._check1Fl) {
			kernel_seq_delete(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 7, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 1, 2);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 112);
			local._check1Fl = false;
		}

		if ((kernel_anim[0].frame == 17) && !local._check1Fl) {
			local._nextFrame = 26;
			local._hermitMode = 4;
			local._check1Fl = true;
		}
	}

	if (local._hermitMode == 4) {
		if ((kernel_anim[0].frame == 33) && local._check1Fl) {
			displayHermitQuestions(local._hermitDisplayedQuestion);
			local._nextFrame = 1;
			local._check1Fl = false;
		}

		if ((kernel_anim[0].frame == 9) && !local._check1Fl) {
			local._nextFrame = 8;
			kernel_timing_trigger(1, 113);
			local._check1Fl = true;
		}
	}

	if (kernel.trigger == 113) {
		player.walker_visible = false;
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 1, 2);
		kernel_seq_player(g_sequence_ids[3], false);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 114);
	}

	if (kernel.trigger == 114) {
		local._resetBatterieText = true;
		int syncIdx = g_sequence_ids[3];
		local._nextFrame = 10;
		g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 1, 2);
		kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
		kernel_seq_player(g_sequence_ids[3], false);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 115);
	}

	if ((local._nextFrame >= 0) && (local._nextFrame != kernel_anim[0].frame)) {
		kernel_reset_animation(0, local._nextFrame);
		local._nextFrame = -1;
	}

	if (kernel.trigger == 115) {
		player.clock = kernel.clock - player.frame_delay;
		player.walker_visible = true;
		if (local._hermitDisplayedQuestion == 21) {
			player.commands_allowed = false;
			handleTrading();
			local._hermitMode = 0;
			local._startTradingFl = false;
			local._nextFrame = 52;
			global[kHasTalkedToHermit] = true;
			kernel_flip_hotspot(words_hermit, false);
		} else {
			player.commands_allowed = true;
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
		player.walk_off_edge_to_room = 609;

	if (local._resetBatterieText)
		kernel_message_purge();
}

static void room_611_parser() {
	if (inter_input_mode == INTER_CONVERSATION)
		handleDialog();
	else if ((player_said_3(give, phone_cells, hermit)) || (player_said_3(give, durafail_cells, hermit))) {
		player2.words[0] = 0x294;
		local._giveBatteriesFl = true;
		handleSubDialog1();
	} else if (player_said_2(give, hermit)) {
		kernel_message_purge();

		char *curQuote = quote_string(kernel.quotes, 0x323);
		int width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		int quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y, 0xFDFC, 120, 0, 0);

		curQuote = quote_string(kernel.quotes, 0x324);
		width = g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing);
		quotePosX = local._defaultDialogPos_x - (width / 2);
		kernel_message_add(curQuote, quotePosX, local._defaultDialogPos_y + 14, 0xFDFC, 120, 0, 0);
	} else if (kernel.trigger == 90) {
		if (local._dialog2.read(0x29C) && local._dialog2.read(0x29D) && local._dialog2.read(0x29E)) {
			handleTalking(180);
			if (g_engine->getRandomNumber(1, 2) == 1)
				displayHermitQuestions(1);
			else
				displayHermitQuestions(2);
		} else {
			handleTalking(180);
			if (g_engine->getRandomNumber(1, 2) == 1)
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
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x27A), 0, 0, 0x1110, 120, 0, 34);
			kernel_timing_trigger(120, 90);
		} else {
			int nextQuote = 0;
			switch (g_engine->getRandomNumber(1, 3)) {
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
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, nextQuote), 0, 0, 0x1110, 120, 0, 34);
			kernel_timing_trigger(120, 90);
		}
	} else if ((player_said_1(walkto) || player_said_1(look)) && player_said_1(rat)) {
		switch (kernel.trigger) {
		case 0:
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x279), 0, 0, 0x1110, 120, 0, 34);
			kernel_timing_trigger(60, 1);
			break;

		case 1:
			handleRatMoves();
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player.look_around) {
		if (global[kHasTalkedToHermit])
			text_show(61111);
		else
			text_show(61110);
	} else if (player_said_2(look, hermit))
		text_show(61112);
	else if (player_said_2(look, trash))
		text_show(61113);
	else if (player_said_2(take, trash))
		text_show(61114);
	else if (player_said_2(look, cardboard_box))
		text_show(61115);
	else if (player_said_2(take, cardboard_box))
		text_show(61116);
	else if (player_said_2(open, cardboard_box))
		text_show(61117);
	else if (player_said_2(look, refrigerator))
		text_show(61118);
	else if (player_said_2(open, refrigerator))
		text_show(61119);
	else if (player_said_2(take, refrigerator))
		text_show(61120);
	else if (player_said_2(look, building))
		text_show(61121);
	else if (player_said_2(look, graffiti))
		text_show(61122);
	else if (player_said_2(look, metal_pipe))
		text_show(61123);
	else
		return;

	player.command_ready = false;
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
	vocab_make_active(words_rat);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
