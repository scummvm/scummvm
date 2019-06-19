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

#include "glk/alan2/alan2.h"
#include "glk/alan2/main.h"
#include "glk/alan2/glkio.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/error.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "glk/glk.h"
#include "glk/streams.h"

namespace Glk {
namespace Alan2 {

Alan2 *g_vm = nullptr;

Alan2::Alan2(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		vm_exited_cleanly(false) {
	g_vm = this;
}

void Alan2::runGame() {
	Common::String gameFileName = _gameFile.getName();

	if (!is_gamefile_valid())
		return;

	initialize();

	Glk::Alan2::run();
}

void Alan2::initialize() {
	// first, open a window for error output
	glkMainWin = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (glkMainWin == nullptr)
		::error("FATAL ERROR: Cannot open initial window");

	g_vm->glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	glkStatusWin = g_vm->glk_window_open(glkMainWin, winmethod_Above |
		winmethod_Fixed, 1, wintype_TextGrid, 0);
	g_vm->glk_set_window(glkMainWin);

	_advName = getFilename();
	while (_advName.contains('.'))
		_advName.deleteLastChar();
	advnam = _advName.c_str();

	codfil = &_gameFile;
	strncpy(codfnm, getFilename().c_str(), 255);
	codfnm[255] = '\0';
}

Common::Error Alan2::readSaveData(Common::SeekableReadStream *rs) {
	// TODO
	return Common::kReadingFailed;
}

Common::Error Alan2::writeGameData(Common::WriteStream *ws) {
	// TODO
	return Common::kWritingFailed;
}

bool Alan2::is_gamefile_valid() {
	if (_gameFile.size() < 8) {
		GUIErrorMessage(_("This is too short to be a valid Alan2 file."));
		return false;
	}

	if (_gameFile.readUint32BE() != MKTAG(2, 8, 1, 0)) {
		GUIErrorMessage(_("This is not a valid Alan2 file."));
		return false;
	}

	return true;
}

} // End of namespace Alan2
} // End of namespace Glk
