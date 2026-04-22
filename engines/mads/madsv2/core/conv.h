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

#ifndef MADS_CORE_CONV_H
#define MADS_CORE_CONV_H

#include "common/array.h"
#include "common/stream.h"

namespace MADS {
namespace MADSV2 {

constexpr int CONV_MAX_SLOTS = 40;
constexpr int CONV_MAX_DATA = 5;

enum ConvStatus {
	CONV_STATUS_HOLDING   = -1,
	CONV_STATUS_NEXT_NODE = 0,
	CONV_STATUS_WAIT_AUTO = 1,
	CONV_STATUS_WAIT_ENTRY = 2,
	CONV_STATUS_EXECUTE   = 3,
	CONV_STATUS_REPLY     = 4,
	CONV_STATUS_DONE      = 10  // conversation finished; next update calls conv_abort
};

// Operator codes used inside conversation script expressions.
// Stored as int16 on disk; 0 = identity (return param1), 255 = no expression.
enum ConvOp : int16 {
	CONV_OP_VALUE = 0,   // No operator — return param1 directly
	CONV_OP_ADD   = 1,   // param1 + param2
	CONV_OP_SUB   = 2,   // param1 - param2
	CONV_OP_MUL   = 3,   // param1 * param2
	CONV_OP_DIV   = 4,   // param1 / param2  (signed integer)
	CONV_OP_MOD   = 5,   // param1 % param2  (signed remainder)
	CONV_OP_GE    = 6,   // param1 >= param2
	CONV_OP_LE    = 7,   // param1 <= param2
	CONV_OP_GT    = 8,   // param1 >  param2
	CONV_OP_LT    = 9,   // param1 <  param2
	CONV_OP_NE    = 10,  // param1 != param2
	CONV_OP_EQ    = 11,  // param1 == param2
	CONV_OP_AND   = 12,  // param1 && param2 (logical)
	CONV_OP_OR    = 13,  // param1 || param2 (logical)
	CONV_OP_NONE  = 255  // No expression — evaluates to -1
};

struct ConvNode {
	int16 index;
	int16 dialog_count;
	int16 active;
	int16 field_6;
	int16 field_8;

	static constexpr int SIZE = 2 * 5;
	void load(Common::SeekableReadStream *src);
};

struct ConvDialog {
	int16 text_line_index;
	int16 speech_index;
	int16 script_offset;
	int16 script_size;

	static constexpr int SIZE = 2 * 4;
	void load(Common::SeekableReadStream *src);
};

struct ConvScriptParams {
	ConvOp operation;
	byte   param1IsVar;
	byte  param2IsVar;
	int16 param1;
	int16 param2;

	static constexpr int SIZE = 2 + 1 + 1 + 2 + 2;
	void load(Common::SeekableReadStream *src);
};

struct ConvVariable {
	enum PtrType { PTRTYPE_GLOBAL = 1, PTRTYPE_CONV_CONTROL = 2 };

	bool isPtr = false;
	int16 val = 0, type = 0;
	int16 *ptr;

	static constexpr int SIZE = 2 * 3;
	void load(Common::SeekableReadStream *src);
	void save(Common::WriteStream *dest) const;
};

struct Conv {
	int16 node_count;
	int16 dialog_count;
	int16 message_count;
	int16 text_line_count;
	int16 num_variables;

	int16 max_imports;
	int16 speaker_count;
	char speaker_portraits[5][16];
	int16 speaker_frame[5];
	char speech_file[14];
	uint32 text_length;
	uint32 commands_length;

	struct LineSet {
		int lineStart = 0;
		int lineCount = 0;
	};

	Common::Array<ConvNode> nodes;
	Common::Array<ConvDialog> dialogs;
	Common::Array<LineSet> messages;
	Common::Array<char> text;
	Common::Array<byte> scripts;
	Common::Array<uint16> textLines;

	static constexpr int SIZE = (2 * 7 + 16 * 5 + 2 * 5 + 14 + 4 + 4) +
		// Padding for pointers in original structure
		4 * 6;
	void load(Common::SeekableReadStream *src);
};

struct ConvData {
	int16 currentNode;
	int16 entryFlagsCount;
	int16 variablesCount;
	int16 importsCount;
	int16 numImports;
	int16 optionListSize;
	int16 messageList1Size;
	int16 messageList2Size;
	int16 speechList1Size;
	int16 speechList2Size;
	int16 optionList[10];
	int16 messageList1[10];
	int16 messageList2[10];
	int16 speechList1[10];
	int16 speechList2[10];

	uint16 importsOffset;
	uint16 entryFlagsOffset;
	uint16 variablesOffset;
	Common::Array<int16> imports;
	Common::Array<uint16> entryFlags;
	Common::Array<ConvVariable> variables;

	static constexpr int SIZE = 2 * 10 + 2 * 10 * 5 + 2 * 3;
	void load(Common::SeekableReadStream *src);
	void save(Common::WriteStream *dest) const;
};

struct ConvControl {
	int16 running;
	int16 index;
	ConvStatus status;
	ConvStatus hold_status;
	int16 has_text;
	int16 popup_is_up;
	int16 popup_duration;
	long popup_clock;
	int16 speaker_active[CONV_MAX_DATA];
	int16 speaker_series[CONV_MAX_DATA];
	int16 speaker_frame[CONV_MAX_DATA];
	int16 x[CONV_MAX_DATA];
	int16 y[CONV_MAX_DATA];
	int16 width[CONV_MAX_DATA];
	int16 speaker_val;
	int16 person_speaking;
	int16 node;
	int16 entry;
	int16 me_trigger;
	int16 me_trigger_mode;
	int16 you_trigger;
	int16 you_trigger_mode;
	int16 commands_allowed;
	int16 input_mode;
};

extern Conv *conv[CONV_MAX_DATA];
extern ConvData *conv_data[CONV_MAX_DATA];
extern Conv *active_conv;
extern ConvData *active_conv_data;

extern int16 *conv_imports;
extern uint16 *conv_entry_flags;
extern ConvVariable *conv_varsDataPtr;
extern int16 *conv_vars0ValPtr;

extern int conv_restore_running;
extern ConvControl conv_control;
extern int16 *conv_my_next_start;

extern void conv_system_init();
extern void conv_system_cleanup();

extern void conv_start(ConvData *convData, Conv *convIn);
extern void conv_stop();
extern void conv_get(int convNum);
extern void conv_run(int convNum);
extern void conv_update(bool);
extern void conv_regenerate_last_message();
extern void conv_export_pointer(int16 *ptr);
extern void conv_abort();
extern void conv_me_trigger(int trigger);
extern void conv_you_trigger(int trigger);
extern int16 *conv_get_variable(int varNum);
extern void conv_export_value(int16 value);
extern void conv_hold();
extern void conv_release();
extern void conv_flush();
extern int conv_append(Common::WriteStream *handle);
extern int conv_expand(Common::SeekableReadStream *handle);

} // namespace MADSV2
} // namespace MADS

#endif
