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

#include "common/system.h"
#include "common/savefile.h"

#include "gui/filebrowser-dialog.h"

#include "fool/fool.h"
#include "fool/toolbox.h"

namespace Fool {

Common::String getFileNameFromModal(bool save, const Common::String &suggested, const Common::String &title) {
	Common::String prefix = g_engine->getGameId() + '-';
	Common::String mask = prefix + "*";
	GUI::FileBrowserDialog browser(title.c_str(), "fool", save ? GUI::kFBModeSave : GUI::kFBModeLoad, mask.c_str(), suggested.c_str());
	if (browser.runModal() <= 0) {
		return Common::String();
	}
	Common::String result = browser.getResult();
	if (!result.empty() && !result.hasPrefixIgnoreCase(prefix))
		result = prefix + result;
	return result;
}

OSErr Toolbox::PBGetVol(ParamBlockRec &paramBlock) {
	warning("STUB: Toolbox::PBGetVol");
	return 0;
}

OSErr Toolbox::PBSetVol(ParamBlockRec &paramBlock) {
	warning("STUB: Toolbox::PBSetVol");
	return 0;
}

void Toolbox::SFGetFile(const Common::Point &where, const Common::U32String &prompt, ProcPtr fileFilter, int16 numTypes, const SFTypeList &typeList, const ProcPtr &dlgHook, SFReply &reply) {
	Common::String fileName = getFileNameFromModal(false, Common::String(), prompt.encode());
	reply.fName = fileName;
	reply.good = !fileName.empty() ? 1 : 0;
}

void Toolbox::SFPutFile(const Common::Point &where, const Common::U32String &prompt, const Common::U32String &origName, const ProcPtr &dlgHook, SFReply &reply) {
	Common::String fileName = getFileNameFromModal(true, origName.encode(), prompt.encode());
	reply.fName = fileName;
	reply.good = !fileName.empty() ? 1 : 0;
}

} // namespace Fool
