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

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/popup.h"
#include "mads/madsv2/core/speech.h"
#include "mads/mads.h"

namespace MADS {
namespace MADSV2 {

Conv *conv[CONV_MAX_DATA];
ConvData *conv_data[CONV_MAX_DATA];
Conv *active_conv;
ConvData *active_conv_data;
int16 *conv_imports;
uint16 *conv_entry_flags;
ConvVariable *conv_varsDataPtr;
int16 *conv_vars0ValPtr;
int conv_restore_running;
ConvControl conv_control;
Box conv_box;
int16 *conv_my_next_start;
int conv_error_code;
int conv_dlg_script_ptr, conv_dlg_script_end;

struct MemoryWriteStreamDynamic : public Common::MemoryWriteStreamDynamic {
public:
	MemoryWriteStreamDynamic() : Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES) {}
};
Common::HashMap<Common::String, MemoryWriteStreamDynamic,
	Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *savedConv;

static int conv_indexes[CONV_MAX_SLOTS];
static bool conv_slots[CONV_MAX_DATA];


// ====================================================================
// Struct serialization methods
// ====================================================================

void Conv::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(node_count, dialog_count, message_count, text_line_count,
		num_variables, max_imports, speaker_count);
	for (int i = 0; i < 5; ++i)
		src->read(speaker_portraits[i], 16);
	for (int i = 0; i < 5; ++i)
		speaker_frame[i] = src->readSint16LE();
	src->read(speech_file, 14);
	src->readMultipleLE(text_length, commands_length);

	// Skip over original padding for pointer fields
	src->skip(6 * sizeof(uint32));

	nodes.clear();
	dialogs.clear();
	messages.clear();
	text.clear();
	scripts.clear();
	textLines.clear();
}

void ConvNode::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(index, dialog_count, active, field_6, field_8);
}

void ConvDialog::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(text_line_index, speech_index, script_offset, script_size);
}

void ConvScriptParams::load(Common::SeekableReadStream *src) {
	operation = (ConvOp)src->readSint16LE();
	param1IsVar = src->readByte();
	param2IsVar = src->readByte();
	src->readMultipleLE(param1, param2);
}

void ConvVariable::load(Common::SeekableReadStream *src) {
	uint16 flag = src->readUint16LE();
	isPtr = flag == MKTAG16('V', 'M');

	val = src->readSint16LE();
	type = src->readSint16LE();

	if (flag == 0xffff) {
		// Original shouldn't have pointer variables by default, except for
		// some placeholder entries in the Manager's Office, which have
		// matching segment & offset values. So are obviously not used
		assert(val == type);
		val = type = 0;

	} else if (isPtr) {
		switch (type) {
		case PTRTYPE_GLOBAL:
			ptr = global + val;
			break;
		case PTRTYPE_CONV_CONTROL:
			if (val >= 0 && val < 20) {
				// Index into one of the sequential 5 element arrays:
				// speaker_frame[5], x[5], y[5], width[5]
				ptr = conv_control.speaker_frame + val;
			} else if (val == 20) {
				ptr = &conv_control.speaker_val;
			} else {
				error("Unknown ConvVariable conv_control pointer");
			}
			break;
		default:
			error("Unknown ConvVariable pointer type");
			break;
		}
	}
}

void ConvVariable::save(Common::WriteStream *dest) const {
	if (isPtr) {
		dest->writeUint16LE(MKTAG16('V', 'M'));
		dest->writeSint16LE(val);
		dest->writeSint16LE(type);
	} else {
		dest->writeUint16LE(0);
		dest->writeSint16LE(val);
		dest->writeSint16LE(0);
	}
}

void ConvData::load(Common::SeekableReadStream *src) {
	{
		byte buffer[SIZE];
		if (!src->read(buffer, SIZE))
			return;

		Common::MemoryReadStream mrs(buffer, SIZE);
		mrs.readMultipleLE(currentNode, entryFlagsCount, variablesCount,
			importsCount, numImports, optionListSize,
			messageList1Size, messageList2Size, speechList1Size, speechList2Size);
		mrs.readMultipleLE(optionList);
		mrs.readMultipleLE(messageList1);
		mrs.readMultipleLE(messageList2);
		mrs.readMultipleLE(speechList1);
		mrs.readMultipleLE(speechList2);
		mrs.readMultipleLE(importsOffset, entryFlagsOffset, variablesOffset);
	}

	// Imports: conditional — the original skips the loader_read when count <= 0
	imports.clear();
	if (importsCount > 0) {
		int16 *buffer = (int16 *)malloc(importsCount * 2);
		src->read(buffer, importsCount * 2);

		imports.resize(importsCount);
		for (int i = 0; i < importsCount; ++i)
			imports[i] = FROM_LE_16(buffer[i]);
		free(buffer);
	}

	// Entry flags: always read (no count guard in the original)
	{
		int16 *buffer = (int16 *)malloc(entryFlagsCount * 2);
		src->read(buffer, entryFlagsCount * 2);

		entryFlags.resize(entryFlagsCount);
		for (int i = 0; i < entryFlagsCount; ++i)
			entryFlags[i] = FROM_LE_16(buffer[i]);
		free(buffer);
	}

	// Variables
	{
		byte *buffer = (byte *)malloc(variablesCount * ConvVariable::SIZE);
		src->read(buffer, (long)variablesCount * ConvVariable::SIZE);

		Common::MemoryReadStream mrs(buffer, variablesCount * ConvVariable::SIZE);
		variables.resize(variablesCount);
		for (int i = 0; i < variablesCount; ++i) {
			variables[i].load(&mrs);

			// Zero the runtime isPtr flag for every variable; it is not meaningful as stored on disk
			variables[i].isPtr = false;
		}
		free(buffer);
	}
}

void ConvData::save(Common::WriteStream *dest) const {
	dest->writeMultipleLE(currentNode, entryFlagsCount, variablesCount,
		importsCount, numImports, optionListSize,
		messageList1Size, messageList2Size, speechList1Size, speechList2Size);
	dest->writeMultipleLE(optionList);
	dest->writeMultipleLE(messageList1);
	dest->writeMultipleLE(messageList2);
	dest->writeMultipleLE(speechList1);
	dest->writeMultipleLE(speechList2);

	// Figure out offset fields
	uint16 importsOffset1 = SIZE;
	uint16 entryFlagsOffset1 = importsOffset1 + (importsCount * 2);
	uint16 variablesOffset1 = entryFlagsOffset1 + (entryFlagsCount * 2);
	dest->writeMultipleLE(importsOffset1, entryFlagsOffset1, variablesOffset1);

	for (int i = 0; i < importsCount; ++i)
		dest->writeSint16LE(imports[i]);

	for (int i = 0; i < entryFlagsCount; ++i)
		dest->writeSint16LE(entryFlags[i]);

	for (int i = 0; i < variablesCount; ++i)
		variables[i].save(dest);
}


// ====================================================================
// Static helpers (ordered so no forward declarations are needed)
// ====================================================================

static const char *conv_get_filename(int convNum, int extType, char *name) {
	*name = '\0';

	if (extType != 2)
		Common::strcat_s(name, 40, "*");
	Common::strcat_s(name, 40, "conv");
	Common::strcat_s(name, 40, Common::String::format("%.3d", convNum).c_str());
	if (extType == 2)
		Common::strcat_s(name, 40, ".dat");

	return name;
}

// conv_open version for when mode is "rb"
static Common::SeekableReadStream *conv_open(int convNum) {
	char name[40];
	conv_get_filename(convNum, 2, name);
	Common::String fname = name;

	// If there's a pretend temporary file for it, use that instead
	if (savedConv->contains(fname))
		return new Common::MemoryReadStream((*savedConv)[fname].getData(),
			(*savedConv)[fname].size());

	// Fall back on normal resource open
	return env_open(name);
}

// conv_open version for when mode is "wb"
static Common::WriteStream *conv_open_write(int convNum) {
	char name[40];
	conv_get_filename(convNum, 2, name);
	Common::String fname = name;

	(*savedConv)[fname] = MemoryWriteStreamDynamic();
	return &(*savedConv)[fname];
}


// ---------------------------------------------------------------------------
// string_trim — strips trailing whitespace from a mutable C string in-place.
// ---------------------------------------------------------------------------
static void string_trim(char *str) {
	if (!str) return;
	size_t len = strlen(str);
	while (len > 0 && (unsigned char)str[len - 1] <= ' ')
		str[--len] = '\0';
}

// ---------------------------------------------------------------------------
// conv_string — returns a pointer to the null-terminated dialog string
// identified by textIdx.  textLines[textIdx] is a byte offset into the flat
// text character pool stored in conv->text.
// ---------------------------------------------------------------------------
static const char *conv_string(Conv *convIn, int textIdx) {
	uint16 strOffset = convIn->textLines[textIdx];
	return &convIn->text[0] + strOffset;
}

// Version to use if the isPtr parameter is true (-1)
static void conv_set_variable(int idx, int16 *ptr) {
	if (conv_control.running >= 0) {
		ConvData &cd = *conv_data[conv_control.index];
		ConvVariable &var = cd.variables[idx];

		var.isPtr = true;
		var.ptr = ptr;

		// We need to know what kind of pointer it is so it can be properly serialized
		if (ptr >= global && ptr < (global + GLOBAL_LIST_SIZE)) {
			var.type = ConvVariable::PTRTYPE_GLOBAL;
			var.val = ptr - global;
		} else if (ptr >= conv_control.speaker_frame && ptr < conv_control.speaker_frame + 20) {
			// Index into one of the sequential 5 element arrays:
			// speaker_frame[5], x[5], y[5], width[5]
			var.type = ConvVariable::PTRTYPE_CONV_CONTROL;
			var.val = ptr - conv_control.speaker_frame;
		} else if (ptr == &conv_control.speaker_val) {
			var.type = ConvVariable::PTRTYPE_CONV_CONTROL;
			var.val = 20;
		} else {
			error("Unhandled ConvVariable pointer type");
		}
	}
}

// Version to use if the isPtr parameter is false (0)
static void conv_set_variable(int idx, int16 val) {
	if (conv_control.running >= 0) {
		ConvData &cd = *conv_data[conv_control.index];
		ConvVariable &var = cd.variables[idx];

		var.isPtr = false;
		var.val = val;
		var.type = 0;
	}
}

static Conv *load_conv(const char *fname) {
	Load file;
	Conv *dataPtr = nullptr;
	Conv *result = nullptr;
	char filename[80];

	file.open = false;

	Common::strcpy_s(filename, fname);
	fileio_add_ext(filename, "cnv");

	if (loader_open(&file, filename, "rb", true)) {
		conv_error_code = 1;
		goto done;
	}

	dataPtr = new Conv();

	{
		// Read the fixed-size Conv header through a MemoryReadStream so that
		// the load() function handles field sizes and endianness correctly.
		byte buffer[Conv::SIZE];
		if (!loader_read(buffer, Conv::SIZE, 1, &file)) {
			conv_error_code = 2;
			goto done;
		}
		Common::MemoryReadStream hdrStream(buffer, Conv::SIZE);
		dataPtr->load(&hdrStream);
	}

	// Read each section from the file.  Error codes deliberately match the
	// originals (note: 6 is skipped, matching the disassembly).

	// Nodes
	{
		byte *buffer = (byte *)malloc(dataPtr->node_count * ConvNode::SIZE);
		if (!loader_read(buffer, dataPtr->node_count * ConvNode::SIZE, 1, &file)) {
			conv_error_code = 4;
			free(buffer);
			goto done;
		}

		Common::MemoryReadStream src(buffer, dataPtr->node_count * ConvNode::SIZE);
		dataPtr->nodes.resize(dataPtr->node_count);
		for (int i = 0; i < dataPtr->node_count; ++i)
			dataPtr->nodes[i].load(&src);

		free(buffer);
	}

	// Dialogs
	{
		byte *buffer = (byte *)malloc(dataPtr->dialog_count * ConvDialog::SIZE);
		if (!loader_read(buffer, dataPtr->dialog_count * ConvDialog::SIZE, 1, &file)) {
			conv_error_code = 5;
			free(buffer);
			goto done;
		}

		Common::MemoryReadStream src(buffer, dataPtr->dialog_count * ConvDialog::SIZE);
		dataPtr->dialogs.resize(dataPtr->dialog_count);
		for (int i = 0; i < dataPtr->dialog_count; ++i)
			dataPtr->dialogs[i].load(&src);

		free(buffer);
	}

	// Messages
	{
		byte *buffer = (byte *)malloc(dataPtr->message_count * 4);
		if (!loader_read(buffer, dataPtr->message_count * 4, 1, &file)) {
			conv_error_code = 7;
			free(buffer);
			goto done;
		}

		Common::MemoryReadStream src(buffer, dataPtr->message_count * 4);
		dataPtr->messages.resize(dataPtr->message_count);
		for (int i = 0; i < dataPtr->message_count; ++i) {
			dataPtr->messages[i].lineStart = src.readSint16LE();
			dataPtr->messages[i].lineCount = src.readSint16LE();
		}

		free(buffer);
	}

	// Text lines
	{
		byte *buffer = (byte *)malloc(dataPtr->text_line_count * 2);
		if (!loader_read(buffer, dataPtr->text_line_count * 2, 1, &file)) {
			conv_error_code = 8;
			free(buffer);
			goto done;
		}

		Common::MemoryReadStream src(buffer, dataPtr->text_line_count * 2);
		dataPtr->textLines.resize(dataPtr->text_line_count);
		for (int i = 0; i < dataPtr->text_line_count; ++i)
			dataPtr->textLines[i] = src.readUint16LE();

		free(buffer);
	}

	// Text block
	{
		dataPtr->text.resize(dataPtr->text_length);
		if (!loader_read(&dataPtr->text[0], dataPtr->text_length, 1, &file)) {
			conv_error_code = 9;
			goto done;
		}
	}

	// Scripts
	{
		dataPtr->scripts.resize(dataPtr->commands_length);
		if (!loader_read(&dataPtr->scripts[0], dataPtr->commands_length, 1, &file)) {
			conv_error_code = 10;
			goto done;
		}
	}

	result = dataPtr;

done:
	if (file.open)
		loader_close(&file);

	// Free the block if we bailed out before completing the load
	if (dataPtr && dataPtr != result)
		delete dataPtr;

	return result;
}

static ConvData *load_conv_data(const char *fname) {
	Load file;
	ConvData *convData = nullptr;
	char filename[80];

	file.open = false;

	Common::strcpy_s(filename, fname);
	fileio_add_ext(filename, "cnd");

	if (loader_open(&file, filename, "rb", true))
		return nullptr;

	convData = new ConvData();
	LoaderReadStream src(&file);
	convData->load(&src);

	loader_close(&file);
	return convData;
}

static ConvData *conv_read(Common::SeekableReadStream *src) {
	ConvData *cd = new ConvData();
	cd->load(src);
	return cd;
}

static int conv_write(Common::WriteStream *dest, const ConvData *convData) {
	convData->save(dest);
	return 0;
}

static void conv_init(ConvData *convData, int val) {
	conv_start(convData, nullptr);

	for (uint i = 0; i < convData->entryFlags.size(); ++i) {
		uint16 &flag = convData->entryFlags[i];
		flag &= 0x3fff;

		if ((flag & 1) || ((flag & 4) && val))
			flag |= 0x8000;
	}
}

static ConvData *conv_get_data(int convNum) {
	ConvData *convData = nullptr;
	char name[80];

	if (conv_indexes[convNum]) {
		Common::SeekableReadStream *handle = conv_open(convNum);
		if (handle) {
			convData = conv_read(handle);
			delete handle;
		}
	} else {
		convData = load_conv_data(conv_get_filename(convNum, 1, name));
		conv_init(convData, 0);
	}

	return convData;
}

static void conv_purge_any_popup() {
	if (conv_control.popup_is_up) {
		Box *my_box = box;
		box = &conv_box;
		popup_destroy();
		matte_frame(0, 0);

		if ((box->y + box->ys) >= 156)
			matte_inter_frame(-1, -1);

		box = my_box;
		conv_control.popup_is_up = 0;
		conv_control.popup_clock = kernel.clock;
	}
}

// ---------------------------------------------------------------------------
// conv_generate_text
//
// Builds and displays a conversation-line popup for the current speaker, then
// triggers the associated speech audio (if the speech system is active and at
// least one speech index was supplied).
//
// Parameters
//   convIn      — active Conv (provides the text pool and speech filename)
//   convData    — active ConvData (unused; callers already route through
//                 conv_control)
//   textIdx     — index into convIn->textLines that selects the dialog string
//   speechList  — array of speech-audio indices; speechList[0] is played
//   speechCount — number of valid entries in speechList (0 = no speech)
// ---------------------------------------------------------------------------
static void conv_generate_text(Conv *convIn, ConvData * /*convData*/,
		int textIdx, int16 *speechList, int speechCount) {
	int  person = conv_control.person_speaking;
	char textBuf[512];
	Box *savedBox;

	// Redirect popup operations to the conversation-private box so that the
	// bounds recorded by popup_draw can be used later by conv_purge_any_popup.
	savedBox = box;
	box = &conv_box;

	// Size and position the popup from the current speaker's slot data.
	int horiz_pieces = popup_estimate_pieces(conv_control.width[person]);
	popup_create(horiz_pieces,
	             conv_control.x[person],
	             conv_control.y[person]);

	// Attach the speaker portrait icon if a series was loaded for this slot.
	// The icon_id is the 1-based frame stored in speaker_frame.
	// center=0: left-aligned within the popup box.
	if (conv_control.speaker_series[person] >= 0) {
		popup_add_icon(series_list[conv_control.speaker_series[person]],
		               conv_control.speaker_frame[person],
		               0);
	}

	// Copy the dialog string to a local mutable buffer, strip trailing
	// whitespace (the raw text pool can have padding bytes), then write it.
	Common::strlcpy(textBuf, conv_string(convIn, textIdx), sizeof(textBuf));
	string_trim(textBuf);
	popup_write_string(textBuf);

	// Render the popup, saving the underlying screen region.
	popup_draw(true, false);

	// Restore the caller's box and record that a conversation popup is live.
	box = savedBox;
	conv_control.popup_is_up  = -1;
	conv_control.popup_clock  = kernel.clock + conv_control.popup_duration;

	// Play the associated speech audio when the speech system is on.
	if (speech_system_active && speech_on && speechCount > 0) {
		speech_play(convIn->speech_file, speechList[0]);
	}
}

static int conv_next_node() {
	active_conv_data->currentNode = *conv_vars0ValPtr;
	return active_conv->nodes[active_conv_data->currentNode].active;
}

// ---------------------------------------------------------------------------
// conv_list_options
//
// Scans the dialogs belonging to the current conversation node and builds the
// list of active (visible) options in active_conv_data->optionList[].
//
// A dialog entry is "active" when bit 15 of its entryFlags word is set.  Up
// to 10 active entries are recorded; when hasMore is zero the list is capped
// at 5 (normal single-page display), otherwise the full 10 are collected
// (extended/scrolled display).  The final count of ALL active entries (not
// just the ones stored) is written to active_conv_data->optionListSize.
//
// Parameters
//   hasMore — non-zero when more than one page of options should be collected
// ---------------------------------------------------------------------------
static int16 conv_list_options(int16 hasMore) {
	int16 nodeIndex  = *conv_vars0ValPtr;
	int   count      = 0;
	int   dialogIter = 0;

	int dialogBase  = active_conv->nodes[nodeIndex].index;
	int dialogCount = active_conv->nodes[nodeIndex].dialog_count;

	while (dialogIter < dialogCount) {
		int16 dialogIdx = (int16)(dialogBase + dialogIter);

		if (active_conv_data->entryFlags[dialogIdx] & 0x8000) {
			// Include in optionList only if within the allowed window size
			if ((count < 5 || hasMore) && count < 10)
				active_conv_data->optionList[count] = dialogIdx;
			++count;
		}
		++dialogIter;
	}

	active_conv_data->optionListSize = (int16)count;
	return (int16)count;
}

static void conv_flag_entry(int action, int index) {
	uint16 &entry = active_conv_data->entryFlags[index];

	switch (action) {
	case 1:
		entry |= 0x4000;
		entry &= 0x7fff;
		break;

	case 2:
		entry &= 0x7fff;
		break;

	case 3:
		if (!(entry & 0x4000))
			entry |= 0x8000;

	default:
		break;
	}
}

static byte conv_get_script_byte() {
	byte val = active_conv->scripts[conv_dlg_script_ptr];
	if (conv_dlg_script_ptr <= conv_dlg_script_end)
		++conv_dlg_script_ptr;

	return val;
}

static void conv_get_script_bytes(byte *dest, int count) {
	while (count-- > 0)
		*dest++ = conv_get_script_byte();
}

static int16 conv_get_script_word() {
	byte buffer[2];
	conv_get_script_bytes(buffer, 2);
	return READ_LE_INT16(buffer);
}

static void conv_param_load(ConvScriptParams *params) {
	params->operation = (ConvOp)conv_get_script_word();

	if (params->operation == CONV_OP_NONE) {
		params->param1IsVar = false;
		params->param1 = 0;
	} else {
		params->param1IsVar = conv_get_script_byte();
		params->param1 = conv_get_script_word();
	}

	if (params->operation == CONV_OP_VALUE || params->operation == CONV_OP_NONE) {
		params->param2IsVar = false;
		params->param2 = 0;
	} else {
		params->param2IsVar = conv_get_script_byte();
		params->param2 = conv_get_script_word();
	}
}

// ---------------------------------------------------------------------------
// conv_scr_get_param
//
// Resolves one operand of a script expression.  If the corresponding IsVar
// flag is set the stored value is treated as a variable index and the live
// value is fetched from conv_varsDataPtr; otherwise the value is used
// directly.
//
// paramNum: 0 = param1, 1 = param2
// ---------------------------------------------------------------------------
static int16 conv_scr_get_param(ConvScriptParams *params, int paramNum) {
	if (paramNum) {
		return params->param2IsVar == 1 ? *conv_get_variable(params->param2) : params->param2;
	} else {
		return params->param1IsVar == 1 ? *conv_get_variable(params->param1) : params->param1;
	}
}

// ---------------------------------------------------------------------------
// conv_param_evaluate
//
// Evaluates a script expression encoded in *params and returns the result as
// an int16.
//
// Special operation codes:
//   CONV_OP_NONE  (255) — no expression; returns -1 without reading params.
//   CONV_OP_VALUE (  0) — identity; returns param1 (resolved variable or
//                         literal) without a second operand.
//
// Operations 1–13 take two resolved operands (param1, param2) and compute
// arithmetic or logical results.  An out-of-range operation code is a
// non-fatal error (WARNING); the initialised result of -1 is returned.
// ---------------------------------------------------------------------------
static int16 conv_param_evaluate(ConvScriptParams *params) {
	int16 result = -1;

	if (params->operation == CONV_OP_NONE)
		return result;

	int16 param1 = conv_scr_get_param(params, 0);

	if (params->operation == CONV_OP_VALUE)
		return param1;

	int16 param2 = conv_scr_get_param(params, 1);

	switch (params->operation) {
	case CONV_OP_ADD:
		result = param1 + param2;
		break;
	case CONV_OP_SUB:
		result = param1 - param2;
		break;
	case CONV_OP_MUL:
		result = param1 * param2;
		break;
	case CONV_OP_DIV:
		result = param1 / param2;
		break;
	case CONV_OP_MOD:
		result = param1 % param2;
		break;
	case CONV_OP_GE:
		result = (param1 >= param2) ? 1 : 0;
		break;
	case CONV_OP_LE:
		result = (param1 <= param2) ? 1 : 0;
		break;
	case CONV_OP_GT:
		result = (param1 > param2) ? 1 : 0;
		break;
	case CONV_OP_LT:
		result = (param1 < param2) ? 1 : 0;
		break;
	case CONV_OP_NE:
		result = (param1 != param2) ? 1 : 0;
		break;
	case CONV_OP_EQ:
		result = (param1 == param2) ? 1 : 0;
		break;
	case CONV_OP_AND:
		result = (param1 != 0 && param2 != 0) ? 1 : 0;
		break;
	case CONV_OP_OR:
		result = (param1 != 0 || param2 != 0) ? 1 : 0;
		break;
	default:
		// Unknown operator — non-fatal; report and return the initialised -1.
		error_report(ERROR_CONV_BAD_OPERATOR, WARNING, MODULE_CONV,
		             conv_dlg_script_ptr, (int)params->operation);
		break;
	}

	return result;
}

// ---------------------------------------------------------------------------
// conv_generate_menu
//
// Prepares the player-choice menu for the current conversation node.
//
// Calls conv_list_options(0) to populate convData->optionList[] with the global
// dialog indices of all active entries for the current node and to obtain the
// total active count.
//
// If the node's field_8 value is >= count it acts as a redirect: variables
// are updated to the target node stored in field_6 and the function returns 0
// so the caller (conv_update) can loop and re-evaluate.
//
// Otherwise the function initialises the dialog interface, registers each
// active entry's display text with inter_add_dialog(), enables command input,
// and returns 2.
//
// Parameters
//   convData — ConvData for the active conversation (provides optionList)
//   convIn   — Conv for the active conversation (provides nodes and dialogs)
//
// Returns
//   0 — node redirected to another node (caller should re-run)
//   2 — menu built and ready for player input
// ---------------------------------------------------------------------------
static int conv_generate_menu(ConvData *convData, Conv *convIn) {
	int16 count     = conv_list_options(0);
	int16 nodeIndex = *conv_vars0ValPtr;
	ConvNode *node  = &convIn->nodes[nodeIndex];

	// If field_8 >= count the node is a redirect: jump to the node index
	// stored in field_6 and tell the caller to re-evaluate.
	if (node->field_8 >= count) {
		*conv_vars0ValPtr   = node->field_6;
		*conv_my_next_start = *conv_vars0ValPtr;
		return 0;
	}

	// Sanity-check: there should be at least one active dialog entry.
	if (count < 1)
		error_report(ERROR_CONV_MENU, ERROR, MODULE_CONV, nodeIndex, count);

	conv_control.popup_clock = kernel.clock;
	kernel_init_dialog();

	for (int i = 0; i < count; ++i) {
		int16 dialogIdx   = convData->optionList[i];
		int16 textLineIdx = convIn->dialogs[dialogIdx].text_line_index;

		if (textLineIdx < 0) {
			// Dialog entry has no associated text — report and skip.
			error_report(ERROR_CONV_NO_TEXT_LINE, ERROR, MODULE_CONV, dialogIdx, textLineIdx);
			continue;
		}

		inter_add_dialog(const_cast<char *>(conv_string(convIn, textLineIdx)), dialogIdx);
	}

	kernel_set_interface_mode(INTER_CONVERSATION);
	player.commands_allowed = -1;
	return 2;
}

static void conv_handle_cmd123(int cmd) {
	ConvScriptParams params;
	conv_param_load(&params);
	int value = conv_param_evaluate(&params);
	int numFlags = conv_get_script_byte();

	for (int i = 0; i < numFlags; ++i) {
		int entryIndex = conv_get_script_word();
		if (value)
			conv_flag_entry(cmd, entryIndex);
	}
}

static void conv_message(int cmd) {
	ConvScriptParams params;
	conv_param_load(&params);
	int value = conv_param_evaluate(&params);
	int count1 = conv_get_script_byte();
	int count2 = conv_get_script_byte();
	int total = 0;
	int val1 = 0;
	int array1[10], array2[10], array3[10];

	for (int i = 0; i < count1; ++i) {
		val1 = conv_get_script_byte();
		if (i < 10) {
			array1[i] = val1;
			total += val1;
		}
	}

	for (int i = 0; i < count1; ++i) {
		val1 = conv_get_script_word();
		if (i < 10)
			array2[i] = val1;
	}

	for (int i = 0; i < count2; ++i) {
		val1 = conv_get_script_word();
		if (i < 10)
			array3[i] = val1;
	}

	if (!value)
		return;

	// Weighted random selection: pick one of the count1 choices using the
	// byte weights in array1.  randomIndex starts at -1 and is incremented
	// before each subtraction so that the first iteration checks array1[0].
	// The loop exits (jg / jump-if-greater) when randomVal drops to <= 0,
	// leaving randomIndex pointing at the selected entry.
	int randomVal   = imath_random(1, total);
	int randomIndex = -1;
	do {
		++randomIndex;
		randomVal -= array1[randomIndex];
	} while (randomVal > 0);

	int entryVal = array2[randomIndex];

	if (cmd == 4) {
		// Player/"you" line
		if (active_conv_data->messageList2Size < 10)
			active_conv_data->messageList2[active_conv_data->messageList2Size++] = (int16)entryVal;

		if (count1 > 1) {
			// Multiple weighted choices: append only the matching data entry
			if (randomIndex < count2 && active_conv_data->speechList2Size < 10)
				active_conv_data->speechList2[active_conv_data->speechList2Size++] = (int16)array3[randomIndex];
		} else {
			// Single choice: append every data entry from array3
			for (int i = 0; i < count2; ++i) {
				if (active_conv_data->speechList2Size < 10)
					active_conv_data->speechList2[active_conv_data->speechList2Size++] = (int16)array3[i];
			}
		}
	} else {
		// NPC/"me" line
		if (active_conv_data->messageList1Size < 10)
			active_conv_data->messageList1[active_conv_data->messageList1Size++] = (int16)entryVal;

		if (count1 > 1) {
			// Multiple weighted choices: append only the matching speech entry
			if (randomIndex < count2 && active_conv_data->speechList1Size < 10)
				active_conv_data->speechList1[active_conv_data->speechList1Size++] = (int16)array3[randomIndex];
		} else {
			// Single choice: append every speech entry from array3
			for (int i = 0; i < count2; ++i) {
				if (active_conv_data->speechList1Size < 10)
					active_conv_data->speechList1[active_conv_data->speechList1Size++] = (int16)array3[i];
			}
		}
	}
}

static int conv_cmd_node() {
	ConvScriptParams params1, params2, params3;
	int result = 0;
	conv_param_load(&params1);
	conv_param_load(&params2);
	conv_param_load(&params3);

	if (conv_param_evaluate(&params1)) {
		int val2 = conv_param_evaluate(&params2);
		int val3 = conv_param_evaluate(&params3);

		*conv_vars0ValPtr = val2;
		if (val2 >= 0)
			*conv_my_next_start = val2;
		else if (val3 >= 0)
			*conv_my_next_start = val3;

		result = -1;
	}

	return result;
}

static void conv_cmd_assign() {
	ConvScriptParams params1, params2;
	conv_param_load(&params1);
	conv_param_load(&params2);
	int varIndex = conv_get_script_word();

	if (conv_param_evaluate(&params1)) {
		int val = conv_param_evaluate(&params2);
		int16 *varPtr = conv_get_variable(varIndex);
		*varPtr = val;
	}
}

static void conv_cmd_goto() {
	ConvScriptParams params1;
	conv_param_load(&params1);
	int newOffset = conv_get_script_word();

	if (conv_param_evaluate(&params1))
		conv_dlg_script_ptr = newOffset;
}

// ---------------------------------------------------------------------------
// conv_execute_entry
//
// Executes the byte-code script attached to dialog entry 'index' within the
// active conversation.  The script is consumed as a stream of command bytes,
// each optionally followed by parameters read via the conv_get_script_*
// helpers.  Command byte values:
//
//   0         — no-op (continue to next command)
//   1, 2, 3  — flag/show/hide entry           → conv_handle_cmd123(cmd)
//   4, 5     — NPC/player message             → conv_message(cmd)
//   6        — invalid command (error)
//   7        — conditional node jump          → conv_cmd_node()
//   8        — conditional script goto        → conv_cmd_goto()
//   9        — conditional variable assign    → conv_cmd_assign()
//   >9       — invalid (same error path as 6)
//
// Execution continues until the script pointer advances past the end of the
// script block, or until conv_cmd_node() signals "done" by returning
// non-zero (which sets flag → 0 → loop exit).
//
// Returns *conv_vars0ValPtr: set to -1 when the script ran to natural
// completion; left at whatever conv_cmd_node() wrote there when a node
// jump caused an early exit.
// ---------------------------------------------------------------------------
static int16 conv_execute_entry(int index) {
	// Point the script stream at this dialog entry's byte-code block.
	ConvDialog &dlg     = active_conv->dialogs[index];
	conv_dlg_script_ptr = dlg.script_offset;
	conv_dlg_script_end = dlg.script_offset + dlg.script_size;

	// Reset per-execution message lists.
	active_conv_data->messageList1Size = 0;
	active_conv_data->messageList2Size = 0;
	active_conv_data->speechList1Size    = 0;
	active_conv_data->speechList2Size = 0;

	// Restore the current node pointer from the saved "next start" slot.
	*conv_vars0ValPtr = *conv_my_next_start;

	// flag: -1 = keep looping; 0 = early exit requested by cmd7.
	int16 flag = -1;

	for (;;) {
		// Exit when the script pointer has advanced past the end of the block
		// (matches: cmp conv_dlg_script_end, ax; jnb; else jmp exit).
		if (conv_dlg_script_ptr > conv_dlg_script_end)
			break;

		int commandId = (uint8)conv_get_script_byte();

		if (commandId > 9) {
			// Unknown command — non-fatal error; loop will naturally exhaust
			// the script or hit a real end.
			error_report(-50, ERROR, MODULE_CONV, conv_dlg_script_ptr, commandId);
		} else {
			switch (commandId) {
			case 0:
				// No-op: read next command.
				break;
			case 1:
			case 2:
			case 3:
				conv_handle_cmd123(commandId);
				break;
			case 4:
			case 5:
				conv_message(commandId);
				break;
			case 6:
				error_report(-50, ERROR, MODULE_CONV, conv_dlg_script_ptr, commandId);
				break;
			case 7:
				// conv_cmd_node returns non-zero when a node jump was taken
				// (signal done → flag=0 → exit loop); 0 means no jump taken
				// (flag=1 → continue).
				// Matches: cmp ax,1; sbb ax,ax; neg ax
				flag = conv_cmd_node() ? 0 : 1;
				break;
			case 8:
				conv_cmd_goto();
				break;
			case 9:
				conv_cmd_assign();
				break;
			}
		}

		// Exit early if a command set flag to 0.
		if (flag == 0)
			break;
	}

	// Natural completion (flag still non-zero): mark node as finished.
	// Early exit (flag == 0): conv_cmd_node already wrote the new target node.
	if (flag != 0)
		*conv_vars0ValPtr = -1;

	return *conv_vars0ValPtr;
}

// ====================================================================
// Public API
// ====================================================================

void conv_system_init() {
	Common::fill((byte *)&conv_control, (byte *)&conv_control + sizeof(ConvControl), 0);
	conv_control.running = -1;

	Common::fill(conv_indexes, conv_indexes + CONV_MAX_SLOTS, 0);
	Common::fill(conv_slots, conv_slots + CONV_MAX_DATA, false);
	Common::fill(conv, conv + CONV_MAX_DATA, (Conv *)nullptr);
	Common::fill(conv_data, conv_data + CONV_MAX_DATA, (ConvData *)nullptr);

	savedConv = new Common::HashMap<Common::String, MemoryWriteStreamDynamic,
		Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>();
}

void conv_system_cleanup() {
	delete savedConv;
}


void conv_start(ConvData *convData, Conv *convIn) {
	active_conv      = convIn;
	active_conv_data = convData;

	// Resolve the byte-offset sub-array pointers stored in the ConvData block
	conv_imports = convData->imports.empty() ? nullptr : &convData->imports[0];
	conv_entry_flags = convData->entryFlags.empty() ? nullptr : & convData->entryFlags[0];
	conv_varsDataPtr = convData->variables.empty() ? nullptr : & convData->variables[0];

	// conv_vars0ValPtr -> variables[0].val (skips the isPtr field)
	conv_vars0ValPtr   = &conv_varsDataPtr[0].val;

	// conv_my_next_start -> variables[1].val
	// (offset = offsetof(ConvVariable, val) + sizeof(ConvVariable) from base)
	conv_my_next_start = &conv_varsDataPtr[1].val;

	convData->currentNode = -1;
	convData->numImports  = 0;

	// Initialise variables[0].val from variables[1].val
	*conv_vars0ValPtr = (int16)*conv_my_next_start;
}

void conv_get(int convNum) {
	char fname[40];
	int free_slot = -1;
	int error_occurred = -1;   // matches original si: -1 = error, 0 = success
	int stage_error   = 0;     // which stage failed (1/2/3); used as data2 in error_report

	// Find first free slot (stops as soon as one is found, matching original loop)
	for (int i = 0; i < CONV_MAX_DATA; ++i) {
		if (!conv_slots[i]) {
			free_slot = i;
			break;
		}
	}

	if (free_slot < 0) {
		stage_error = 1;
		goto report;
	}

	conv_slots[free_slot] = true;

	conv[free_slot] = load_conv(conv_get_filename(convNum, 0, fname));
	if (!conv[free_slot]) {
		stage_error = 2;
		goto report;
	}

	conv_data[free_slot] = conv_get_data(convNum);
	if (!conv_data[free_slot]) {
		stage_error = 3;
		goto report;
	}

	// Encode slot as (free_slot + 2) so that 0 means "not loaded"
	conv_indexes[convNum] = free_slot + 2;
	error_occurred = 0;

report:
	if (error_occurred)
		error_report(ERROR_CONV_GET, ERROR, MODULE_CONV, convNum, stage_error);
}

void conv_run(int convId) {
	char name[80];
	int idx;

	// Validate convId is loaded (non-fatal: report error but continue, matching original)
	if (conv_indexes[convId] < 2)
		error_report(ERROR_CONV_RUN, ERROR, MODULE_CONV, convId, 0);

	// Stop any conversation already in progress
	if (conv_control.running >= 0)
		conv_abort();

	conv_control.running          = convId;
	conv_control.index            = conv_indexes[convId] - 2;
	conv_control.status           = CONV_STATUS_NEXT_NODE;
	conv_control.popup_duration   = 0x7FFF;			// Display for a long time
	conv_control.popup_clock      = kernel.clock;
	conv_control.entry            = -1;
	conv_control.commands_allowed = player.commands_allowed;
	conv_control.input_mode       = inter_input_mode;
	conv_control.popup_is_up      = 0;
	conv_control.me_trigger       = 0;
	conv_control.you_trigger      = 0;

	// Initialise per-speaker slots.
	// speaker_val and person_speaking are written inside the loop in the original
	// (compiler artefact from an unrolled version); preserved here for fidelity.
	for (idx = 0; idx < CONV_MAX_DATA; ++idx) {
		conv_control.speaker_active[idx] = 0;
		conv_control.speaker_series[idx] = -1;
		conv_control.speaker_frame[idx]  = 1;
		conv_control.x[idx]              = (int16)0x8000;
		conv_control.y[idx]              = (int16)0x8000;
		conv_control.width[idx]          = 30;
		conv_control.speaker_val         = 1;
		conv_control.person_speaking     = 1;
	}

	int slot = conv_control.index;
	conv_start(conv_data[slot], conv[slot]);

	// Bind conv variables 2–22 to live ConvControl fields.
	// Variable 2 maps to speaker_val (not contiguous with the arrays below).
	// Variables 3–22 map to speaker_frame[5], x[5], y[5], width[5] — 20
	// consecutive int16s — so a single pointer walk replaces 20 separate calls.
	conv_set_variable(2, &conv_control.speaker_val);
	int16 *p = conv_control.speaker_frame;
	for (int i = 3; i <= 22; ++i, ++p)
		conv_set_variable(i, p);

	// Load speaker portrait series for each declared speaker
	for (idx = 0; idx < conv[slot]->speaker_count; ++idx) {
		Common::strcpy_s(name, conv[slot]->speaker_portraits[idx]);
		int series = kernel_load_series(name, 0x4000);
		conv_control.speaker_series[idx] = series;
		if (series > 0) {
			conv_control.speaker_active[idx] = -1;
			conv_control.speaker_frame[idx]  = conv[slot]->speaker_frame[idx];
		}
	}

	if (kernel_mode == KERNEL_ACTIVE_CODE)
		kernel_new_palette();

	player.commands_allowed = 0;
}

// ---------------------------------------------------------------------------
// conv_generate_message
//
// Generates and displays a conversation message popup for either the NPC
// ("me") or player ("you") side, drawing from message and voice/data index
// lists that conv_execute_entry populated.
//
// Parameters (DOS push order — rightmost pushed first):
//   convIn        — active Conv
//   convData      — active ConvData
//   msgList       — array of dialog text indices  (messageList1 or messageList2)
//   msgListSize   — number of valid entries in msgList  (in ax)
//   voiceList     — array of speech/data indices (speechList1 or speechList2)
//   voiceListSize — number of valid entries in voiceList (in dx)
// ---------------------------------------------------------------------------
static void conv_generate_message(Conv *convIn, ConvData *convData,
		int16 *msgList, int msgListSize, int16 *voiceList, int voiceListSize) {
	Box *priorBox = box;
	box = &conv_box;
	conv_control.has_text = 0;
	int personSpeaking;
	int messageId;
	int lineStart, lineCount;
	char tempString[256];

	if (msgListSize != 0) {
		personSpeaking = conv_control.person_speaking;
		if (!popup_create(popup_estimate_pieces(conv_control.width[personSpeaking]),
				conv_control.x[personSpeaking], conv_control.y[personSpeaking])) {
			if (conv_control.speaker_series[personSpeaking] >= 0) {
				popup_add_icon(series_list[conv_control.speaker_series[personSpeaking]],
					conv_control.speaker_frame[personSpeaking], 0);
			}

			for (int msgIndex = 0; msgIndex < msgListSize; ++msgIndex) {
				messageId = msgList[msgIndex];
				lineStart = convIn->messages[messageId].lineStart;
				lineCount = convIn->messages[messageId].lineCount;

				for (int lineCtr = 0; lineCtr < lineCount; ++lineCtr) {
					Common::strcpy_s(tempString, conv_string(convIn, lineStart + lineCtr));
					string_trim(tempString);
					popup_write_string(tempString);
				}
			}

			popup_next_line();
			if (!popup_draw(-1, -1)) {
				conv_control.popup_is_up = -1;
				conv_control.popup_clock = kernel.clock + conv_control.popup_duration;

				if (speech_system_active && speech_on) {
					if (voiceListSize != 0) {
						speech_play(convIn->speech_file, *voiceList);
					}
				}
			}
		}
	}

	box = priorBox;
}

// ---------------------------------------------------------------------------
// conv_update
//
// Main per-tick conversation state machine.  Called once per game tick while
// a conversation is running.  The 'flag' parameter indicates whether the
// engine has a pending player command ready (mirrors player.command_ready in
// the callers for modes 1 and 2).
//
// Status dispatch table (off_2D438):
//   0  (NEXT_NODE)   — advance to next node or build player menu
//   1  (WAIT_AUTO)   — wait for auto-trigger then advance to EXECUTE
//   2  (WAIT_ENTRY)  — player chose an option; execute it + show NPC portrait
//   3  (EXECUTE)     — (clock-gated) execute entry script + show NPC message
//   4  (REPLY)       — (clock-gated) show player reply message
//   5–9              — no-op (exit)
//   10 (DONE)        — call conv_abort
//   >10              — no-op (exit)
// ---------------------------------------------------------------------------
void conv_update(bool flag) {
	if (conv_control.running < 0)
		return;

	int slot = conv_control.index;
	Conv     *my_conv      = conv[slot];
	ConvData *my_conv_data = conv_data[slot];

	switch (conv_control.status) {

	// ------------------------------------------------------------------
	// Mode 0 — NEXT_NODE
	// ------------------------------------------------------------------
	case CONV_STATUS_NEXT_NODE: {
		if (*conv_vars0ValPtr < 0) {
			// Node is exhausted — wait for the popup clock then clean up.
			if (kernel.clock < conv_control.popup_clock)
				return;

			conv_purge_any_popup();

			if (conv_control.me_trigger) {
				player_verb            = conv_control.entry;
				kernel.trigger         = conv_control.me_trigger;
				kernel.trigger_mode    = conv_control.me_trigger_mode;
				conv_control.me_trigger = 0;
			}

			conv_control.status = CONV_STATUS_DONE;
			return;
		}

		// Advance to the next node in the script.
		int var_4 = conv_next_node();
		conv_control.node = *conv_vars0ValPtr;

		if (!var_4) {
			// Node is a player-choice menu node.
			conv_control.status = (ConvStatus)conv_generate_menu(my_conv_data, my_conv);
			return;
		}

		// Node is an auto-play NPC node: pick the first dialog entry and
		// fire a synthetic player command so mode 1 can advance to EXECUTE.
		int16 nodeIndex       = *conv_vars0ValPtr;
		conv_control.entry    = my_conv->nodes[nodeIndex].index;
		conv_control.status   = CONV_STATUS_WAIT_AUTO;
		player_verb           = conv_control.entry;
		player.command_ready  = -1;
		player.command_error  = 0;
		return;
	}

	// ------------------------------------------------------------------
	// Mode 10 — DONE: conversation finished, tear everything down.
	// ------------------------------------------------------------------
	case CONV_STATUS_DONE:
		conv_abort();
		return;

	// ------------------------------------------------------------------
	// Mode 1 — WAIT_AUTO: waiting for the synthetic command to fire.
	// ------------------------------------------------------------------
	case CONV_STATUS_WAIT_AUTO:
		if (!flag)
			return;
		conv_control.status = CONV_STATUS_EXECUTE;
		return;

	// ------------------------------------------------------------------
	// Mode 2 — WAIT_ENTRY: player has chosen a dialog option.
	// ------------------------------------------------------------------
	case CONV_STATUS_WAIT_ENTRY: {
		if (!flag)
			return;

		player.commands_allowed = 0;
		conv_control.entry      = player_verb;

		// Auto-hide the entry unless it is flagged as always-visible (bit 1).
		if (!(active_conv_data->entryFlags[conv_control.entry] & 2))
			conv_flag_entry(2, conv_control.entry);

		conv_purge_any_popup();
		kernel_init_dialog();
		kernel_set_interface_mode(INTER_CONVERSATION);
		conv_control.person_speaking = 0;

		conv_execute_entry(conv_control.entry);

		// If the dialog has a dedicated speech index, override the speech list
		// with that single index (takes priority over anything conv_execute_entry
		// populated via conv_message).
		int16 speech_idx = my_conv->dialogs[conv_control.entry].speech_index;
		if (speech_idx) {
			my_conv_data->speechList1[0] = speech_idx;
			my_conv_data->speechList1Size = 1;
		}

		conv_generate_text(my_conv, my_conv_data,
			my_conv->dialogs[conv_control.entry].text_line_index,
			my_conv_data->speechList1, my_conv_data->speechList1Size);

		conv_control.status = CONV_STATUS_REPLY;

		if (conv_control.me_trigger) {
			player_verb            = conv_control.entry;
			kernel.trigger         = conv_control.me_trigger;
			kernel.trigger_mode    = conv_control.me_trigger_mode;
			conv_control.me_trigger = 0;
			return;
		}
		return;
	}

	// ------------------------------------------------------------------
	// Mode 3 — EXECUTE: execute the NPC entry's script and show message.
	// ------------------------------------------------------------------
	case CONV_STATUS_EXECUTE: {
		if (kernel.clock < conv_control.popup_clock)
			return;

		conv_purge_any_popup();
		conv_control.person_speaking = 0;

		conv_execute_entry(conv_control.entry);

		conv_generate_message(my_conv, my_conv_data,
			my_conv_data->messageList1, my_conv_data->messageList1Size,
			my_conv_data->speechList1, my_conv_data->speechList1Size);

		// Fire me_trigger if one is pending and a popup is visible.
		if (conv_control.me_trigger && conv_control.popup_is_up) {
			player_verb            = conv_control.entry;
			kernel.trigger         = conv_control.me_trigger;
			kernel.trigger_mode    = conv_control.me_trigger_mode;
			conv_control.me_trigger = 0;
		}

		conv_control.status = CONV_STATUS_REPLY;
		return;
	}

	// ------------------------------------------------------------------
	// Mode 4 — REPLY: show the player's reply message.
	// ------------------------------------------------------------------
	case CONV_STATUS_REPLY: {
		if (kernel.clock < conv_control.popup_clock)
			return;

		conv_purge_any_popup();
		conv_control.person_speaking = conv_control.speaker_val;

		conv_generate_message(my_conv, my_conv_data,
			my_conv_data->messageList2, my_conv_data->messageList2Size,
			my_conv_data->speechList2, my_conv_data->speechList2Size);

		conv_control.status = CONV_STATUS_NEXT_NODE;

		if (conv_control.you_trigger && conv_control.popup_is_up) {
			player_verb             = conv_control.entry;
			kernel.trigger          = conv_control.you_trigger;
			kernel.trigger_mode     = conv_control.you_trigger_mode;
			conv_control.you_trigger = 0;
			return;
		}
		return;
	}

	default:
		// Statuses 5–9 and anything above 10: no-op.
		return;
	}
}

void conv_regenerate_last_message() {
	if (conv_control.running && conv_control.popup_is_up) {
		Conv &c = *conv[conv_control.index];
		ConvData &cd = *conv_data[conv_control.index];

		conv_purge_any_popup();

		if (conv_control.has_text) {
			conv_generate_text(&c, &cd, c.dialogs[conv_control.entry].text_line_index,
				cd.speechList1, cd.speechList1Size);
		} else if (conv_control.person_speaking) {
			conv_generate_message(&c, &cd, cd.messageList2, cd.messageList2Size,
				cd.speechList2, cd.speechList2Size);
		} else {

		}
	}
}

void conv_export_pointer(int16 *ptr) {
	if (conv_control.running) {
		Conv &c = *conv[conv_control.index];
		ConvData &cd = *conv_data[conv_control.index];

		if (cd.numImports < c.max_imports) {
			int idx = conv_imports[cd.numImports++];
			conv_set_variable(idx, ptr);
		}
	}
}

void conv_abort() {
	if (conv_control.running >= 0) {
		if (kernel_mode == KERNEL_ACTIVE_CODE)
			player.commands_allowed = conv_control.commands_allowed;

		conv_control.running = -1;
		conv_purge_any_popup();

		for (int i = conv[conv_control.index]->speaker_count - 1; i >= 0; --i) {
			if (conv_control.speaker_active[i])
				matte_deallocate_series(conv_control.speaker_series[i], -1);
		}

		if (conv_control.input_mode == INTER_CONVERSATION)
			kernel_init_dialog();

		kernel_set_interface_mode(conv_control.input_mode);
	}
}

void conv_me_trigger(int trigger) {
	conv_control.me_trigger = trigger;
	conv_control.me_trigger_mode = kernel.trigger_setup_mode;
}

void conv_you_trigger(int trigger) {
	conv_control.you_trigger = trigger;
	conv_control.you_trigger_mode = kernel.trigger_setup_mode;
}

int16 *conv_get_variable(int varNum) {
	assert(varNum >= 0 && varNum < active_conv_data->variablesCount);
	ConvVariable &var = active_conv_data->variables[varNum];

	return var.isPtr ? var.ptr : &var.val;
}

void conv_export_value(int16 value) {
	if (conv_control.running >= 0) {
		Conv &c = *conv[conv_control.index];
		ConvData &cd = *conv_data[conv_control.index];

		if (cd.numImports < c.max_imports) {
			int idx = conv_imports[cd.numImports++];
			conv_set_variable(idx, value);
		}
	}
}

void conv_hold() {
	if (conv_control.status != CONV_STATUS_HOLDING) {
		conv_control.hold_status = conv_control.status;
		conv_control.status = CONV_STATUS_HOLDING;
	}
}

void conv_release() {
	if (conv_control.status == CONV_STATUS_HOLDING) {
		conv_control.status = conv_control.hold_status;

		if (conv_control.status == CONV_STATUS_WAIT_AUTO ||
			conv_control.status == CONV_STATUS_WAIT_ENTRY)
			conv_update(true);
	}
}

void conv_flush() {
	bool errorFlag = true;
	int errCode = 0;
	Common::WriteStream *dest;
	int i;

	for (i = 0; i < CONV_MAX_SLOTS; ++i) {
		if (conv_indexes[i] >= 2) {
			dest = conv_open_write(i);
			if (!dest)
				goto done;

			ConvData *convData = conv_data[conv_indexes[i] - 2];
			errCode = conv_write(dest, convData);
			if (errCode) goto done;

			conv_indexes[i] = 1;
		}
	}

	for (i = CONV_MAX_DATA - 1; i >= 0; --i) {
		if (conv_slots[i]) {
			delete conv_data[i];
			delete conv[i];
			conv_data[i] = nullptr;
			conv[i] = nullptr;
			conv_slots[i] = false;
		}
	}

	errorFlag = false;
done:
	if (errorFlag)
		error("Error flushing conversation data");
}

int conv_append(Common::WriteStream *handle) {
	int count = 0;
	int16 list[CONV_MAX_SLOTS];
	ConvData *convData;
	int convNum, convIndex;
	int i, errCode;

	// Generate a list of conv_indexes that are present
	for (i = 0; i < CONV_MAX_SLOTS; ++i) {
		if (conv_indexes[i])
			list[count++] = i;
	}

	// Write out count and list
	handle->writeUint16LE(count);
	for (i = 0; i < count; ++i)
		handle->writeSint16LE(list[i]);

	for (i = 0; i < count; ++i) {
		convNum = list[i];
		convIndex = conv_indexes[convNum];

		if (convIndex == 1) {
			convData = conv_get_data(convNum);
			if (!convData)
				break;

			errCode = conv_write(handle, convData);
			delete convData;
		} else {
			convData = conv_data[convIndex - 2];
			errCode = conv_write(handle, convData);
		}

		if (errCode)
			break;
	}

	return 0;
}

int conv_expand(Common::SeekableReadStream *handle) {
	int count;
	int16 list[CONV_MAX_SLOTS];
	ConvData *convData;

	Common::fill(conv_indexes, conv_indexes + CONV_MAX_SLOTS, 0);

	// Read count and list
	count = handle->readUint16LE();
	for (int i = 0; i < count; ++i)
		list[i] = handle->readSint16LE();

	for (int i = 0; i < count; ++i) {
		int index = list[i];
		conv_indexes[index] = 1;

		// Open a temporary file for the conversation
		Common::WriteStream *dest = conv_open_write(index);
		if (!dest)
			break;

		// Read it's data from the savegame
		convData = conv_read(handle);

		// Write it out to the temporary file
		bool success = false;
		if (convData)
			success = !conv_write(dest, convData);

		if (!success) {
			delete convData;
			break;
		}
	}

	return 0;
}

} // namespace MADSV2
} // namespace MADS
