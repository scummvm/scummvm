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

#include "glk/magnetic/magnetic.h"
#include "glk/magnetic/magnetic_defs.h"
#include "common/config-manager.h"

namespace Glk {
namespace Magnetic {

Magnetic *g_vm;

Magnetic::Magnetic(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		gms_gamma_mode(GAMMA_NORMAL), gms_animation_enabled(true),
		gms_prompt_enabled(true), gms_abbreviations_enabled(true), gms_commands_enabled(true),
		gms_graphics_enabled(true), GMS_PORT_VERSION(0x00010601),
		gms_main_window(nullptr), gms_status_window(nullptr), gms_graphics_window(nullptr),
		gms_hint_menu_window(nullptr), gms_hint_text_window(nullptr),
		gms_transcript_stream(nullptr), gms_readlog_stream(nullptr),
		gms_inputlog_stream(nullptr), gms_graphics_possible(true),
		GMS_INPUT_PROMPT(">"), gms_gameid_game_name(nullptr), gms_graphics_bitmap(nullptr),
		gms_graphics_width(0), gms_graphics_height(0), gms_graphics_animated(false),
		gms_graphics_picture(0), gms_graphics_new_picture(false),
		gms_graphics_repaint(false), gms_graphics_active(false),
		gms_graphics_interpreter(false), gms_graphics_off_screen(nullptr),
		gms_graphics_on_screen(nullptr), gms_graphics_current_gamma(Magnetic::GMS_GAMMA_TABLE),
		gms_graphics_color_count(GMS_PALETTE_SIZE), gms_status_length(0),
		gms_help_requested(false), gms_help_hints_silenced(false), gms_output_buffer(nullptr),
		gms_output_allocation(0),gms_output_length(0), gms_output_prompt(false),
		gms_hints(nullptr), gms_current_hint_node(0), gms_hint_cursor(nullptr),
		gms_input_length(0), gms_input_cursor(0), gms_undo_notification(false),
		gms_game_message(nullptr), gms_startup_called(false), gms_main_called(false),
		i_count(0), string_size(0), rseed(0), pc(0), arg1i(0), mem_size(0), properties(0),
		fl_sub(0), fl_tab(0), fl_size(0), fp_tab(0), fp_size(0), zflag(0), nflag(0),
		cflag(0), vflag(0), byte1(0), byte2(0), regnr(0), admode(0), opsize(0),
		arg1(nullptr), arg2(nullptr), is_reversible(0), running(0), lastchar(0), version(0),
		sd(0), decode_table(nullptr), restart(nullptr), code(nullptr), string(nullptr),
		string2(nullptr), string3(nullptr), dict(nullptr), quick_flag(0), gfx_ver(0),
		gfx_buf(nullptr), gfx_data(nullptr), gfx2_hdr(nullptr), gfx2_buf(nullptr),
		gfx2_name(nullptr), gfx2_hsize(0), gfx_fp(nullptr), snd_buf(nullptr), snd_hdr(nullptr),
		snd_hsize(0), snd_fp(nullptr), undo_pc(0), undo_size(0), gfxtable(0), table_dist(0),
		v4_id(0), next_table(1)
#ifndef NO_ANIMATION
		, pos_table_size(0), command_table(nullptr), command_index(-1),
		pos_table_index(-1), pos_table_max(-1), anim_repeat(0)
#endif
		, hints(nullptr), hint_contents(nullptr), xpos(0), bufpos(0), log_on(0),
		ms_gfx_enabled(0), _log1(nullptr), _log2(nullptr), GMS_LUMINANCE_WEIGHTS(299, 587, 114),
		linear_gamma(nullptr), pic_current_crc(0), hints_current_crc(0),
		hints_crc_initialized(false), _saveData(nullptr), _saveSize(0) {

	Common::fill(&gms_graphics_palette[0], &gms_graphics_palette[GMS_PALETTE_SIZE], 0);
	Common::fill(&gms_status_buffer[0], &gms_status_buffer[GMS_STATBUFFER_LENGTH], '\0');
	Common::fill(&gms_input_buffer[0], &gms_input_buffer[GMS_INPUTBUFFER_LENGTH], '\0');
	Common::fill(&dreg[0], &dreg[8], 0);
	Common::fill(&areg[0], &areg[8], 0);
	Common::fill(&tmparg[0], &tmparg[4], 0);
	Common::fill(&undo_regs[0][0], &undo_regs[2][0], 0);
	undo[0] = undo[1] = nullptr;
	undo_stat[0] = undo_stat[1] = 0;
	Common::fill(&buffer[0], &buffer[80], 0);
	Common::fill(&filename[0], &filename[256], 0);
	Common::fill(&crc_table[0], &crc_table[BYTE_MAX_VAL + 1], 0);

#ifndef NO_ANIMATION
	Common::fill(&pos_table_count[0], &pos_table_count[MAX_POSITIONS], 0);
#endif

	luminance_weighting = GMS_LUMINANCE_WEIGHTS.red + GMS_LUMINANCE_WEIGHTS.green
		+ GMS_LUMINANCE_WEIGHTS.blue;

	g_vm = this;
}

void Magnetic::runGame() {
	initialize();
	gms_main();
}

void Magnetic::initialize() {
	initializeSettings();
	initializeCRC();
	initializeLinearGamma();

	// Close the already opened gamefile, since the Magnetic code will open it on it's own
	_gameFile.close();
}

void Magnetic::initializeSettings() {
	// Local handling for Glk special commands
	if (ConfMan.hasKey("commands_enabled"))
		gms_commands_enabled = ConfMan.getBool("commands_enabled");
	// Abbreviation expansions
	if (ConfMan.hasKey("abbreviations_enabled"))
		gms_abbreviations_enabled = ConfMan.getBool("abbreviations_enabled");
	// Pictures enabled
	if (ConfMan.hasKey("graphics_enabled"))
		gms_graphics_enabled = ConfMan.getBool("graphics_enabled");
	// Automatic gamma correction on pictures
	if (ConfMan.hasKey("gamma_mode") && !ConfMan.getBool("gamma_mode"))
		gms_gamma_mode = GAMMA_OFF;
	// Animations
	if (ConfMan.hasKey("animation_enabled"))
		gms_animation_enabled = ConfMan.getBool("animation_enabled");
	// Prompt enabled
	if (ConfMan.hasKey("prompt_enabled"))
		gms_prompt_enabled = ConfMan.getBool("prompt_enabled");
}

void Magnetic::initializeCRC() {
	/* CRC table initialization polynomial. */
	const glui32 GMS_CRC_POLYNOMIAL = 0xedb88320;
	uint32 crc;

	for (uint index = 0; index < BYTE_MAX_VAL + 1; ++index) {
		int bit;

		crc = index;
		for (bit = 0; bit < BITS_PER_BYTE; bit++)
			crc = crc & 1 ? GMS_CRC_POLYNOMIAL ^ (crc >> 1) : crc >> 1;

		crc_table[index] = crc;
	}

	/* CRC lookup table self-test, after is_initialized set -- recursion. */
	assert(gms_get_buffer_crc("123456789", 9) == 0xcbf43926);
}

void Magnetic::initializeLinearGamma() {
	/* Find and cache the uncorrected gamma table entry. */
	gms_gammaref_t gamma;

	for (gamma = GMS_GAMMA_TABLE; gamma->level; gamma++) {
		if (!gamma->is_corrected) {
			linear_gamma = gamma;
			break;
		}
	}
}

type8 Magnetic::ms_load_file(const char *name, type8 *ptr, type16 size) {
	_saveData = ptr;
	_saveSize = size;

	return loadGame().getCode() == Common::kNoError ? 0 : 1;
}

Common::Error Magnetic::readSaveData(Common::SeekableReadStream *rs) {
	return rs->read(_saveData, _saveSize) == _saveSize ? Common::kNoError : Common::kReadingFailed;
}

type8 Magnetic::ms_save_file(const char *name, type8 *ptr, type16 size) {
	_saveData = ptr;
	_saveSize = size;

	return saveGame().getCode() == Common::kNoError ? 0 : 1;
}

Common::Error Magnetic::writeGameData(Common::WriteStream *ws) {
	return ws->write(_saveData, _saveSize) == _saveSize ? Common::kNoError : Common::kWritingFailed;
}

} // End of namespace Magnetic
} // End of namespace Glk
