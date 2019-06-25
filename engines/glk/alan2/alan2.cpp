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
#include "glk/alan2/exe.h"
#include "glk/alan2/main.h"
#include "glk/alan2/glkio.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/error.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "glk/glk.h"
#include "glk/streams.h"

namespace Glk {
namespace Alan2 {

Alan2 *g_vm = nullptr;

Alan2::Alan2(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		vm_exited_cleanly(false), _restartFlag(false), _saveSlot(-1), _pendingLook(false) {
	g_vm = this;
	txtfil = nullptr;
	logfil = nullptr;
	memory = nullptr;
}

void Alan2::runGame() {
	if (initialize())
		Glk::Alan2::run();

	deinitialize();
}

bool Alan2::initialize() {
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

	// Set up the code file to point to the already opened game file
	codfil = &_gameFile;
	strncpy(codfnm, getFilename().c_str(), 255);
	codfnm[255] = '\0';

	if (_gameFile.size() < 8) {
		GUIErrorMessage(_("This is too short to be a valid Alan2 file."));
		return false;
	}

	if (_gameFile.readUint32BE() != MKTAG(2, 8, 1, 0)) {
		GUIErrorMessage(_("This is not a valid Alan2 file."));
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

	return true;
}

void Alan2::deinitialize() {
	free(memory);

	delete txtfil;
	delete logfil;
}

Common::Error Alan2::readSaveData(Common::SeekableReadStream *rs) {
	Common::Serializer s(rs, nullptr);
	synchronizeSave(s);

	return Common::kNoError;
}

Common::Error Alan2::writeGameData(Common::WriteStream *ws) {
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

void Alan2::synchronizeSave(Common::Serializer &s) {
	AtrElem *atr;
	Aword i;

	// Sync current values
	cur.synchronize(s);

	// Save actors
	for (i = ACTMIN; i <= ACTMAX; ++i) {
		syncVal(s, &acts[i - ACTMIN].loc);
		syncVal(s, &acts[i - ACTMIN].script);
		syncVal(s, &acts[i - ACTMIN].step);
		syncVal(s, &acts[i - ACTMIN].count);

		if (acts[i - ACTMIN].atrs) {
			for (atr = (AtrElem *)addrTo(acts[i - ACTMIN].atrs); !endOfTable(atr); ++atr)
				syncVal(s, &atr->val);
		}
	}

	// Sync locations
	for (i = LOCMIN; i <= LOCMAX; ++i) {
		syncVal(s, &locs[i - LOCMIN].describe);
		if (locs[i - LOCMIN].atrs)
			for (atr = (AtrElem *)addrTo(locs[i - LOCMIN].atrs); !endOfTable(atr); atr++)
				syncVal(s, &atr->val);
	}

	// Sync objects
	for (i = OBJMIN; i <= OBJMAX; ++i) {
		syncVal(s, &objs[i - OBJMIN].loc);
		if (objs[i - OBJMIN].atrs)
			for (atr = (AtrElem *)addrTo(objs[i - OBJMIN].atrs); !endOfTable(atr); atr++)
				syncVal(s, &atr->val);
	}

	// Sync the event queue
	if (s.isSaving()) {
		eventq[etop].time = 0;        // Mark the top
		for (i = 0; i <= (Aword)etop; ++i)
			eventq[i].synchronize(s);
	} else {
		for (etop = 0; eventq[etop - 1].time; ++etop)
			eventq[etop].synchronize(s);
		--etop;
	}

	// Sync scores
	for (i = 0; scores[i] != EOD; i++)
		syncVal(s, &scores[i]);
}

} // End of namespace Alan2
} // End of namespace Glk
