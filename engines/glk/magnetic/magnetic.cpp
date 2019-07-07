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
#include "common/config-manager.h"
#include "common/translation.h"

namespace Glk {
namespace Magnetic {

Magnetic::Magnetic(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
	
		vm_exited_cleanly(false), dlimit(0xffffffff), slimit(0xffffffff), log_on(0),
		i_count(0), string_size(0), rseed(0), pc(0), arg1i(0), mem_size(0), properties(0), fl_sub(0),
		fl_tab(0), fl_size(0), fp_tab(0), fp_size(0), zflag(0), nflag(0), cflag(0), vflag(0), byte1(0),
		byte2(0), regnr(0), admode(0), opsize(0), arg1(nullptr), arg2(nullptr), is_reversible(0),
		lastchar(0), version(0), sd(0), decode_table(nullptr), restart(nullptr), code(nullptr),
		string(nullptr), string2(nullptr), string3(nullptr), dict(nullptr), quick_flag(0), gfx_ver(0),
		gfx_buf(nullptr), gfx_data(nullptr), gfx2_hdr(0), gfx2_buf(nullptr), gfx2_hsize(0),
		snd_buf(nullptr), snd_hdr(nullptr), snd_hsize(0), undo_pc(0), undo_size(0),
		gfxtable(0), table_dist(0), v4_id(0), next_table(1), pos_table_size(0),
		command_table(nullptr), command_index(-1), pos_table_index(-1), pos_table_max(-1),
		anim_repeat(0) {

	// Emu fields
	undo[0] = undo[1] = nullptr;
	undo_stat[0] = undo_stat[1] = 0;
	Common::fill(&dreg[0], &dreg[8], 0);
	Common::fill(&areg[0], &areg[8], 0);
	Common::fill(&tmparg[0], &tmparg[4], 0);
	Common::fill(&undo_regs[0][0], &undo_regs[2][18], 0), 
	Common::fill(&pos_table_count[0], &pos_table_count[MAX_POSITIONS], 0);
}

void Magnetic::runGame() {
	if (!is_gamefile_valid())
		return;

	// TODO
}

Common::Error Magnetic::readSaveData(Common::SeekableReadStream *rs) {
	// TODO
	return Common::kReadingFailed;
}

Common::Error Magnetic::writeGameData(Common::WriteStream *ws) {
	// TODO
	return Common::kWritingFailed;
}

bool Magnetic::is_gamefile_valid() {
	if (_gameFile.size() < 8) {
		GUIErrorMessage(_("This is too short to be a valid Glulx file."));
		return false;
	}

	if (_gameFile.readUint32BE() != MKTAG('G', 'l', 'u', 'l')) {
		GUIErrorMessage(_("This is not a valid Glulx file."));
		return false;
	}

	// We support version 2.0 through 3.1.*
	uint32 vers = _gameFile.readUint32BE();
	if (vers < 0x20000) {
		GUIErrorMessage(_("This Glulx file is too old a version to execute."));
		return false;
	}
	if (vers >= 0x30200) {
		GUIErrorMessage(_("This Glulx file is too new a version to execute."));
		return false;
	}

	return true;
}

} // End of namespace Magnetic
} // End of namespace Glk
