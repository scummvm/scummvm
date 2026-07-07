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

#include "graphics/mactoolbox/toolbox.h"

namespace Graphics {
namespace MacToolbox {

OSErr Toolbox::GetFInfo(const Common::U32String &fileName, int16 vRefNum, Common::MacFinderInfo &fndrInfo) {
	Common::MacResManager resMan;
	Common::Path path(fileName, ':');
	// testing resource fork, load from the filesystem
	if (!resMan.getFileFinderInfo(path, fndrInfo))
		return kFnfErr;
	return kNoErr;
}

OSErr Toolbox::PBGetVol(ParamBlockRec &paramBlock) {
	debugC(0, kDebugLevelMacToolbox, "STUB: Toolbox::PBGetVol");
	return 0;
}

OSErr Toolbox::PBSetVol(ParamBlockRec &paramBlock) {
	debugC(0, kDebugLevelMacToolbox, "STUB: Toolbox::PBSetVol");
	return 0;
}

void Toolbox::SFGetFile(const Common::Point &where, const Common::U32String &prompt, ProcPtr fileFilter, int16 numTypes, const SFTypeList &typeList, const ProcPtr &dlgHook, SFReply &reply) {
	if (_fileModalCallback) {
		Common::String fileName = _fileModalCallback(false, Common::String(), prompt.encode());
		reply.fName = fileName;
	} else {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::SFGetFile: file modal callback not set");
		reply.fName.clear();
	}
	reply.good = !reply.fName.empty() ? 1 : 0;
}

void Toolbox::SFPutFile(const Common::Point &where, const Common::U32String &prompt, const Common::U32String &origName, const ProcPtr &dlgHook, SFReply &reply) {
	if (_fileModalCallback) {
		reply.fName = _fileModalCallback(true, origName.encode(), prompt.encode());
	} else {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::SFPutFile: file modal callback not set");
		reply.fName.clear();
	}
	reply.good = !reply.fName.empty() ? 1 : 0;
}

} // End of namespace MacToolbox
} // End of namespace Graphics
