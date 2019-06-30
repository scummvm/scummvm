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

#ifndef GLK_MAGNETIC_MAGNETIC
#define GLK_MAGNETIC_MAGNETIC

#include "common/scummsys.h"
#include "glk/glk_api.h"
#include "glk/magnetic/magnetic_types.h"

namespace Glk {
namespace Magnetic {

/**
 * Magnetic game interpreter
 */
class Magnetic : public GlkAPI {
public:
	Common::File _hintFile;
	Common::File _gfxFile;
	Common::File _sndFile;
	bool vm_exited_cleanly;
	uint dlimit, slimit;
	int log_on;

	// Emu fields
	uint32 dreg[8], areg[8], i_count, string_size, rseed, pc, arg1i, mem_size;
	uint16 properties, fl_sub, fl_tab, fl_size, fp_tab, fp_size;
	byte zflag, nflag, cflag, vflag, byte1, byte2, regnr, admode, opsize;
	byte *arg1, *arg2, is_reversible, running, tmparg[4];
	byte lastchar, version, sd;
	byte *decode_table, *restart, *code, *string, *string2;
	byte *string3, *dict;
	byte quick_flag, gfx_ver, *gfx_buf, *gfx_data;
	byte *gfx2_hdr, *gfx2_buf;
	Common::String gfx2_name;
	uint16 gfx2_hsize;
	byte *snd_buf, *snd_hdr;
	uint16 snd_hsize;
	Common::File gfx_fp;
	uint32 undo_regs[2][18], undo_pc, undo_size;
	byte *undo[2], undo_stat[2];
	uint16 gfxtable, table_dist;
	uint16 v4_id, next_table;

	ms_hint *hints;
	byte *hint_contents;
	picture anim_frame_table[MAX_ANIMS];
	uint16 pos_table_size;
	uint16 pos_table_count[MAX_POSITIONS];
	ms_position pos_table[MAX_POSITIONS][MAX_ANIMS];
	byte *command_table;
	int command_index;
	lookup anim_table[MAX_POSITIONS];
	int pos_table_index;
	int pos_table_max;
	ms_position pos_array[MAX_FRAMES];
	byte anim_repeat;


private:
	/**
	 * Validates the game file, and if it's invalid, displays an error dialog
	 */
	bool is_gamefile_valid();

	/**
	 * \defgroup Emu
	 * @{
	 */

	/**
	 * Loads the interpreter with a game
	 * @return	0 = failure, 1 = success(without graphics or graphics failed),
	 *		2 = success(with graphics)
	 */
	int ms_init(bool restarting = false);

	/**
	 * Stops further processing of opcodes
	 */
	void ms_stop() { running = false; }

	/**
	 * Detects if game is running
	 */
	bool ms_is_running() const { return running; }

	/**
	 * Returns true if running a Magnetic Windows game
	 */
	bool ms_is_magwin() const { return version == 4; }

	/**
	 * Frees all allocated ressources
	 */
	void ms_freemem();

	/**@}*/

	/**
	 * \defgroup Graphics support methods
	 * @{
	 */

	byte init_gfx1(size_t size);

	byte init_gfx2(size_t size);

	/**
	 * Displays or hides a picture
	 * @param c			number of image to be displayed
	 * @param mode		0 = means gfx off, 1 gfx on thumbnails, 2 gfx on normal
	 *
	 * @remarks		For retrieving the raw data of a picture call ms_extract
	 */
	void ms_showpic(int c, byte mode);

	/**
	 * Returns true if a given line is blank
	 */
	bool is_blank(uint16 line, uint16 width) const;

	byte *ms_extract1(byte pic, uint16 *w, uint16 *h, uint16 *pal);

	int16 find_name_in_header(const Common::String &name, bool upper);

	void extract_frame(const picture *pic);

	byte *ms_extract2(const char *name, uint16 *w, uint16 *h, uint16 *pal, byte *is_anim);

	byte *ms_extract(uint32 pic, uint16 *w, uint16 *h, uint16 *pal, byte *is_anim);

	byte ms_animate(ms_position **positions, uint16 *count);

	byte *ms_get_anim_frame(int16 number, uint16 *width, uint16 *height, byte **mask);

	bool ms_anim_is_repeating() const { return anim_repeat;	}

	void write_reg(int i, int s, uint32 val) {
		// TODO
	}

	/**@}*/

	/**
	 * \defgroup Sound support methods
	 * @{
	 */

	byte init_snd(size_t size);

	int16 find_name_in_sndheader(const Common::String &name);

	byte *sound_extract(const Common::String &name, uint32 *length, uint16 *tempo);

	/**@}*/
public:
	/**
	 * Constructor
	 */
	Magnetic(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame();

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override { return INTERPRETER_MAGNETIC; }

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	virtual Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	virtual Common::Error writeGameData(Common::WriteStream *ws) override;
};

} // End of namespace Magnetic
} // End of namespace Glk

#endif
