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

#include "glk/alan3/alan3.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/main.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/options.h"
#include "glk/alan3/syserr.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/error.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "glk/glk.h"
#include "glk/streams.h"

namespace Glk {
namespace Alan3 {

Alan3 *g_vm = nullptr;

Alan3::Alan3(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
	vm_exited_cleanly(false), _restartFlag(false), _saveSlot(-1), _pendingLook(false) {
	g_vm = this;
//	txtfil = nullptr;
//	logfil = nullptr;
	memory = nullptr;

	verboseOption = false;
	ignoreErrorOption = false;
	debugOption = false;
	traceSectionOption = false;
	tracePushOption = false;
	traceStackOption = false;
	traceSourceOption = false;
	traceInstructionOption = false;
	transcriptOption = false;
	logOption = false;
	statusLineOption = true;
	regressionTestOption = false;

	// syserr
	setSyserrHandler(nullptr);
}

void Alan3::runGame() {
	if (initialize())
		Glk::Alan3::run();

	deinitialize();
}

bool Alan3::initialize() {
	// Set up adventure name
	_advName = getFilename();
	if (_advName.size() > 4 && _advName[_advName.size() - 4] == '.')
		_advName = Common::String(_advName.c_str(), _advName.size() - 4);

	// first, open a window for error output
	glkMainWin = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (glkMainWin == nullptr)
		::error("FATAL ERROR: Cannot open initial window");

	g_vm->glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	glkStatusWin = g_vm->glk_window_open(glkMainWin, winmethod_Above |
	                                     winmethod_Fixed, 1, wintype_TextGrid, 0);
	g_vm->glk_set_window(glkMainWin);
	/*
	    // Set up the code file to point to the already opened game file
	    codfil = &_gameFile;
	    strncpy(codfnm, getFilename().c_str(), 255);
	    codfnm[255] = '\0';

	    if (_gameFile.size() < 8) {
	        GUIErrorMessage(_("This is too short to be a valid Alan3 file."));
	        return false;
	    }

	    if (_gameFile.readUint32BE() != MKTAG(2, 8, 1, 0)) {
	        GUIErrorMessage(_("This is not a valid Alan3 file."));
	        return false;
	    }

	    // Open up the text file
	    txtfil = new Common::File();
	    if (!txtfil->open(Common::String::format("%s.dat", _advName.c_str()))) {
	        GUIErrorMessage("Could not open adventure text data file");
	        delete txtfil;
	        return false;
	    }

	    // Check for a save being loaded directly from the launcher
	    _saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	*/
	return true;
}

void Alan3::deinitialize() {
	free(memory);
	/*
	    delete txtfil;
	    delete logfil;
	*/
}

Common::Error Alan3::readSaveData(Common::SeekableReadStream *rs) {
	Common::Serializer s(rs, nullptr);
	synchronizeSave(s);

	return Common::kNoError;
}

Common::Error Alan3::writeGameData(Common::WriteStream *ws) {
	Common::Serializer s(nullptr, ws);
	synchronizeSave(s);

	ws->flush();
	return Common::kNoError;
}

// This works around gcc errors for passing packed structure fields
void syncVal(Common::Serializer &s, uint32 *fld) {
	uint32 &v = *fld;
	s.syncAsUint32LE(v);
}

void Alan3::synchronizeSave(Common::Serializer &s) {
	// TODO
}

} // End of namespace Alan3
} // End of namespace Glk
