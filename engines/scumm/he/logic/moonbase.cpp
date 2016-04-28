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

namespace Scumm {

/**
 * Logic code for:
 *     Moonbase Commander
 */
class LogicHEmoonbase : public LogicHE {
public:
	LogicHEmoonbase(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();

	int32 dispatch(int op, int numArgs, int32 *args);

private:
	void op_create_multi_state_wiz(int op, int numArgs, int32 *args);
	void op_load_multi_channel_wiz(int op, int numArgs, int32 *args);
	void op_wiz_from_multi_channel_wiz(int op, int numArgs, int32 *args);
	void op_dos_command(int op, int numArgs, int32 *args);
	void op_set_fow_sentinel(int op, int numArgs, int32 *args);
	void op_set_fow_information(int op, int numArgs, int32 *args);
	void op_set_fow_image(int op, int numArgs, int32 *args);
	void op_ai_test_kludge(int op, int numArgs, int32 *args);
	void op_ai_master_control_program(int op, int numArgs, int32 *args);
	void op_ai_reset(int op, int numArgs, int32 *args);
	void op_ai_set_type(int op, int numArgs, int32 *args);
	void op_ai_clean_up(int op, int numArgs, int32 *args);
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
		op_set_fow_sentinel(op, numArgs, args);
		break;
	case OP_SET_FOW_INFORMATION:
		op_set_fow_information(op, numArgs, args);
		break;
	case OP_SET_FOW_IMAGE:
		op_set_fow_image(op, numArgs, args);
		break;

	case OP_AI_TEST_KLUDGE:
		op_ai_test_kludge(op, numArgs, args);
		break;
	case OP_AI_MASTER_CONTROL_PROGRAM:
		op_ai_master_control_program(op, numArgs, args);
		break;
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

void LogicHEmoonbase::op_set_fow_sentinel(int op, int numArgs, int32 *args) {
	warning("STUB: op_set_fow_sentinel()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_set_fow_information(int op, int numArgs, int32 *args) {
	warning("STUB: op_set_fow_information()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_set_fow_image(int op, int numArgs, int32 *args) {
	warning("STUB: op_set_fow_image()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_ai_test_kludge(int op, int numArgs, int32 *args) {
	warning("STUB: op_ai_test_kludge()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_ai_master_control_program(int op, int numArgs, int32 *args) {
	warning("STUB: op_ai_master_control_program()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_ai_reset(int op, int numArgs, int32 *args) {
	warning("STUB: op_ai_reset)");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_ai_set_type(int op, int numArgs, int32 *args) {
	warning("STUB: op_ai_set_type()");
	LogicHE::dispatch(op, numArgs, args);
}

void LogicHEmoonbase::op_ai_clean_up(int op, int numArgs, int32 *args) {
	warning("STUB: op_ai_clean_up()");
	LogicHE::dispatch(op, numArgs, args);
}

LogicHE *makeLogicHEmoonbase(ScummEngine_v90he *vm) {
	return new LogicHEmoonbase(vm);
}

} // End of namespace Scumm
