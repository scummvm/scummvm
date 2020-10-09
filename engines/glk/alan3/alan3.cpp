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
#include "glk/alan3/output.h"
#include "glk/alan3/save.h"
#include "glk/alan3/syserr.h"
#include "common/system.h"
#include "common/error.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "glk/glk.h"
#include "glk/streams.h"

namespace Glk {
namespace Alan3 {

Alan3 *g_vm = nullptr;

Alan3::Alan3(OSystem *syst, const GlkGameDescription &gameDesc) : GlkIO(syst, gameDesc) {
	g_vm = this;

	// main
	codfil = nullptr;
	textFile = nullptr;
	memory = nullptr;

	// exe
	printFlag = false;

	// options
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

	// output
	anyOutput = false;
	capitalize = false;
	needSpace = false;
	skipSpace = false;

	// syserr
	setSyserrHandler(nullptr);
}

void Alan3::runGame() {
	if (initialize())
		Glk::Alan3::run();

	deinitialize();
}

bool Alan3::initialize() {
	if (!GlkIO::initialize())
		syserr("FATAL ERROR: Cannot open initial window");

	// Set up adventure name
	_advName = getFilename();
	if (_advName.size() > 4 && _advName[_advName.size() - 4] == '.')
		_advName = Common::String(_advName.c_str(), _advName.size() - 4);

	// In Alan 3, the text data comes from the adventure file itself
	Common::File *txt = new Common::File();
	if (!txt->open(getFilename())) {
	    GUIErrorMessage("Could not open adventure file for text data");
	    delete txt;
	    return false;
	}
	textFile = txt;

	// Set up the code file to point to the already opened game file
	codfil = &_gameFile;

	return true;
}

void Alan3::deinitialize() {
	free(memory);
	delete textFile;
//  delete logfil;
}

Common::Error Alan3::readSaveData(Common::SeekableReadStream *rs) {
	return Glk::Alan3::restoreGame(rs) ? Common::kNoError : Common::kReadingFailed;
}

Common::Error Alan3::writeGameData(Common::WriteStream *ws) {
	Glk::Alan3::saveGame(ws);
	return Common::kNoError;
}

} // End of namespace Alan3
} // End of namespace Glk
