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

#include "glk/glulx/glulx.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace Glk {
namespace Glulx {

Glulx *g_vm;

Glulx::Glulx(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		vm_exited_cleanly(false), gamefile_start(0), gamefile_len(0), memmap(nullptr), stack(nullptr),
		ramstart(0), endgamefile(0), origendmem(0),  stacksize(0), startfuncaddr(0), checksum(0),
		stackptr(0), frameptr(0), pc(0), prevpc(0), origstringtable(0), stringtable(0), valstackbase(0),
		localsbase(0), endmem(0), protectstart(0), protectend(0),
		stream_char_handler(nullptr), stream_unichar_handler(nullptr),
		// main
		library_autorestore_hook(nullptr),
		// accel
		classes_table(0), indiv_prop_start(0), class_metaclass(0), object_metaclass(0),
		routine_metaclass(0), string_metaclass(0), self(0), num_attr_bytes(0), cpv__start(0),
		accelentries(nullptr),
		// heap
		heap_start(0), alloc_count(0), heap_head(nullptr), heap_tail(nullptr),
		// serial
		max_undo_level(8), undo_chain_size(0), undo_chain_num(0), undo_chain(nullptr), ramcache(nullptr),
		// string
		iosys_mode(0), iosys_rock(0), tablecache_valid(false), glkio_unichar_han_ptr(nullptr) {
	g_vm = this;

	glkopInit();
}

void Glulx::runGame() {
	if (!is_gamefile_valid())
		return;

	gamefile_start = 0;
	gamefile_len = _gameFile.size();
	setup_vm();

	if (!init_dispatch())
		return;

	if (library_autorestore_hook)
		library_autorestore_hook();

	execute_loop();
	finalize_vm();

	gamefile_start = 0;
	gamefile_len = 0;
	init_err = nullptr;
	vm_exited_cleanly = true;

	profile_quit();
}

bool Glulx::is_gamefile_valid() {
	if (_gameFile.size() < 8) {
		GUIErrorMessage(_("This is too short to be a valid Glulx file."));
		return false;
	}

	if (_gameFile.readUint32BE() != MKTAG('G', 'l', 'u', 'l')) {
		GUIErrorMessage(_("This is not a valid Glulx file."));
		return false;
	}

	// We support version 2.0 through 3.1.*
	uint version = _gameFile.readUint32BE();
	if (version < 0x20000) {
		GUIErrorMessage(_("This Glulx file is too old a version to execute."));
		return false;
	}
	if (version >= 0x30200) {
		GUIErrorMessage(_("This Glulx file is too new a version to execute."));
		return false;
	}

	return true;
}

void Glulx::fatal_error_handler(const char *str, const char *arg, bool useVal, int val) {
	Common::String msg = Common::String::format("Glulx fatal error: %s", str);

	if (arg || useVal) {
		msg += " (";

		if (arg)
			msg += Common::String::format("%s", arg);
		if (arg && useVal)
			msg += " ";
		if (useVal)
			msg += Common::String::format("%x", val);

		msg += ")";
	}

	error("%s", msg.c_str());
}

void Glulx::nonfatal_warning_handler(const char *str, const char *arg, bool useVal, int val) {
	Common::String msg = Common::String::format("Glulx warning: %s", str);

	if (arg || useVal) {
		msg += " (";

		if (arg)
			msg += Common::String::format("%s", arg);
		if (arg && useVal)
			msg += " ";
		if (useVal)
			msg += Common::String::format("%x", val);

		msg += ")";
	}

	warning("%s", msg.c_str());
}

void Glulx::glulx_sort(void *addr, int count, int size, int(*comparefunc)(const void *p1, const void *p2)) {
	qsort(addr, count, size, comparefunc);
}

} // End of namespace Glulx
} // End of namespace Glk
