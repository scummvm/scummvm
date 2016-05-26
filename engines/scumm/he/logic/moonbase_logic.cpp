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

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"
#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/ai_main.h"

namespace Scumm {

/**
 * Logic code for:
 *     Moonbase Commander
 */
class LogicHEmoonbase : public LogicHE {
public:
	LogicHEmoonbase(ScummEngine_v100he *vm) : LogicHE(vm) { _vm1 = vm; }

	int versionID();

	int32 dispatch(int op, int numArgs, int32 *args);

private:
	void op_create_multi_state_wiz(int op, int numArgs, int32 *args);
	void op_load_multi_channel_wiz(int op, int numArgs, int32 *args);
	void op_wiz_from_multi_channel_wiz(int op, int numArgs, int32 *args);
	void op_dos_command(int op, int numArgs, int32 *args);
	void op_set_fow_sentinel(int32 *args);
	void op_set_fow_information(int op, int numArgs, int32 *args);
	int op_set_fow_image(int op, int numArgs, int32 *args);

	void op_ai_test_kludge(int op, int numArgs, int32 *args);
	int op_ai_master_control_program(int op, int numArgs, int32 *args);
	void op_ai_reset(int op, int numArgs, int32 *args);
	void op_ai_set_type(int op, int numArgs, int32 *args);
	void op_ai_clean_up(int op, int numArgs, int32 *args);

private:
	ScummEngine_v100he *_vm1;
};

int LogicHEmoonbase::versionID() {
	if (_vm->_game.features & GF_DEMO)
		return -100;
	else if (strcmp(_vm->_game.variant, "1.1") == 0)
		return 110;
	else
		return 100;
}

#define OP_CREATE_MULTI_STATE_WIZ		100
#define OP_LOAD_MULTI_CHANNEL_WIZ		101
#define OP_WIZ_FROM_MULTI_CHANNEL_WIZ	102
#define OP_DOS_COMMAND					103
#define OP_SET_FOW_SENTINEL				104
#define OP_SET_FOW_INFORMATION			105
#define OP_SET_FOW_IMAGE				106

#define OP_AI_TEST_KLUDGE				10000
#define OP_AI_MASTER_CONTROL_PROGRAM	10001
#define OP_AI_RESET						10002
#define OP_AI_SET_TYPE					10003
#define OP_AI_CLEAN_UP					10004

#define OP_NET_REMOTE_START_SCRIPT			1492
#define OP_NET_DO_INIT_ALL					1493
#define OP_NET_DO_INIT_PROVIDER				1494
#define OP_NET_DO_INIT_SESSION				1495
#define OP_NET_DO_INIT_USER					1496
#define OP_NET_QUERY_PROVIDERS				1497
#define OP_NET_GET_PROVIDER_NAME			1498
#define OP_NET_SET_PROVIDER					1499
#define OP_NET_CLOSE_PROVIDER				1500
#define OP_NET_QUERY_SESSIONS				1501
#define OP_NET_GET_SESSION_NAME				1502
#define OP_NET_CREATE_SESSION				1503
#define OP_NET_JOIN_SESSION					1504
#define OP_NET_END_SESSION					1505
#define OP_NET_ADD_USER						1506
#define OP_NET_REMOVE_USER					1507
#define OP_NET_WHO_SENT_THIS				1508
#define OP_NET_REMOTE_SEND_ARRAY			1509
#define OP_NET_WHO_AM_I						1510
#define OP_NET_REMOTE_START_FUNCTION		1511
#define OP_NET_GET_PLAYER_LONG_NAME			1512
#define OP_NET_GET_PLAYER_SHORT_NAME		1513
#define OP_NET_SET_PROVIDER_BY_NAME			1516
#define OP_NET_HOST_TCPIP_GAME				1517
#define OP_NET_JOIN_TCPIP_GAME				1518
#define OP_NET_SET_FAKE_LAG					1555
#define OP_NET_GET_HOST_NAME				1556
#define OP_NET_GET_IP_FROM_NAME				1557
#define OP_NET_GET_SESSION_PLAYER_COUNT		1558
#define OP_NET_DISABLE_SESSION_PLAYER_JOIN	1559
#define OP_NET_START_QUERY_SESSIONS			1560
#define OP_NET_UPDATE_QUERY_SESSIONS		1561
#define OP_NET_STOP_QUERY_SESSIONS			1562
#define OP_NET_DESTROY_PLAYER				1563
#define OP_NET_ENABLE_SESSION_PLAYER_JOIN	1564
#define OP_NET_SET_AI_PLAYER_COUNT			1565


int32 LogicHEmoonbase::dispatch(int op, int numArgs, int32 *args) {
	switch (op) {
	case OP_CREATE_MULTI_STATE_WIZ:
		op_create_multi_state_wiz(op, numArgs, args);
		break;
	case OP_LOAD_MULTI_CHANNEL_WIZ:
		op_load_multi_channel_wiz(op, numArgs, args);
		break;
	case OP_WIZ_FROM_MULTI_CHANNEL_WIZ:
		op_wiz_from_multi_channel_wiz(op, numArgs, args);
		break;
	case OP_DOS_COMMAND:
		op_dos_command(op, numArgs, args);
		break;
	case OP_SET_FOW_SENTINEL:
		op_set_fow_sentinel(args);
		break;
	case OP_SET_FOW_INFORMATION:
		op_set_fow_information(op, numArgs, args);
		break;
	case OP_SET_FOW_IMAGE:
		return op_set_fow_image(op, numArgs, args);

	case OP_AI_TEST_KLUDGE:
		op_ai_test_kludge(op, numArgs, args);
		break;
	case OP_AI_MASTER_CONTROL_PROGRAM:
		return op_ai_master_control_program(op, numArgs, args);
	case OP_AI_RESET:
		op_ai_reset(op, numArgs, args);
		break;
	case OP_AI_SET_TYPE:
		op_ai_set_type(op, numArgs, args);
		break;
	case OP_AI_CLEAN_UP:
		op_ai_clean_up(op, numArgs, args);
		break;

	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return 0;
}

void LogicHEmoonbase::op_create_multi_state_wiz(int op, int numArgs, int32 *args) {
	warning("STUB: op_create_multi_state_wiz()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_load_multi_channel_wiz(int op, int numArgs, int32 *args) {
	warning("STUB: op_load_multi_channel_wiz()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_wiz_from_multi_channel_wiz(int op, int numArgs, int32 *args) {
	warning("STUB: op_wiz_from_multi_channel_wiz()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_dos_command(int op, int numArgs, int32 *args) {
	warning("STUB: op_dos_command()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_set_fow_sentinel(int32 *args) {
	debug(2, "op_set_fow_sentinel(%d, %d, %d)", args[0], args[1], args[2]);

	_vm1->_moonbase->_fowSentinelImage = args[0];
	_vm1->_moonbase->_fowSentinelState = args[1];
	_vm1->_moonbase->_fowSentinelConditionBits = args[2];
}

void LogicHEmoonbase::op_set_fow_information(int op, int numArgs, int32 *args) {
	Common::String str;

	str = Common::String::format("op_set_fow_information(%d", args[0]);
	for (int i = 1; i < numArgs; i++) {
		str += Common::String::format(", %d", args[i]);
	}
	str += ")";

	debug(2, "%s", str.c_str());

	_vm1->_moonbase->setFOWInfo(
		args[0],		// array
		args[1],		// array down dimension
		args[2],		// array across dimension
		args[3],		// logical view X coordinate
		args[4],		// logical view Y coordinate
		args[5],		// screen draw clip rect x1
		args[6],		// screen draw clip rect y1
		args[7],		// screen draw clip rect x2
		args[8],		// screen draw clip rect y2
		args[9],		// techinque
		args[10]		// frame
	);
}

int LogicHEmoonbase::op_set_fow_image(int op, int numArgs, int32 *args) {
	debug(2, "op_set_fow_image(%d)", args[0]);
	return _vm1->_moonbase->setFOWImage(args[0]) ? 1 : 0;
}

void LogicHEmoonbase::op_ai_test_kludge(int op, int numArgs, int32 *args) {
	warning("STUB: op_ai_test_kludge()");
	LogicHE::dispatch(op, numArgs, args);
}

int LogicHEmoonbase::op_ai_master_control_program(int op, int numArgs, int32 *args) {
	warning("op_ai_master_control_program()");
	return _vm1->_moonbase->_ai->masterControlProgram(numArgs, args);
}

void LogicHEmoonbase::op_ai_reset(int op, int numArgs, int32 *args) {
	warning("op_ai_reset())");
	_vm1->_moonbase->_ai->resetAI();
}

void LogicHEmoonbase::op_ai_set_type(int op, int numArgs, int32 *args) {
	warning("op_ai_set_type()");
	_vm1->_moonbase->_ai->setAIType(numArgs, args);
}

void LogicHEmoonbase::op_ai_clean_up(int op, int numArgs, int32 *args) {
	warning("op_ai_clean_up()");
	_vm1->_moonbase->_ai->cleanUpAI();
}

LogicHE *makeLogicHEmoonbase(ScummEngine_v100he *vm) {
	return new LogicHEmoonbase(vm);
}

} // End of namespace Scumm
