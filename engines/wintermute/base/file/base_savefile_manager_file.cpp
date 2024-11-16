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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/file/base_savefile_manager_file.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_engine.h"
#include "common/system.h"
#include "common/savefile.h"


namespace Wintermute {

Common::String makeSfmFilename(const Common::String &filename) {
	Common::String smFilename = filename;
	for (size_t i = 0; i < smFilename.size(); i++) {
		if (smFilename[i] == '/' || smFilename[i] == '\\') {
			smFilename.setChar('_', i);
		}
	}
	while (smFilename.hasPrefix("._")) {
		smFilename = smFilename.substr(2);
	}
	return BaseEngine::instance().getGameTargetName() + "." + smFilename;
}

bool sfmFileExists(const Common::String &filename) {
	Common::String smFilename = makeSfmFilename(filename);
	return g_system->getSavefileManager()->listSavefiles(smFilename).size() > 0;
}

Common::SeekableReadStream *openSfmFile(const Common::String &filename) {
	Common::String smFilename = makeSfmFilename(filename);
	return g_system->getSavefileManager()->openRawFile(smFilename);
}

Common::WriteStream *openSfmFileForWrite(const Common::String &filename) {
	Common::String smFilename = makeSfmFilename(filename);
	return g_system->getSavefileManager()->openForSaving(smFilename, false);
}

Common::StringArray sfmFileList(const Common::String &mask) {
	Common::String prefix = BaseEngine::instance().getGameTargetName() + ".";
	Common::String smMask = makeSfmFilename(mask);
	Common::StringArray array = g_system->getSavefileManager()->listSavefiles(smMask);
	for (uint32 i = 0; i < array.size(); i++) {
		array[i] = array[i].substr(prefix.size());
	}
	return array;
}

bool sfmFileRemove(const Common::String &filename) {
	Common::String smFilename = makeSfmFilename(filename);
	return g_system->getSavefileManager()->removeSavefile(smFilename);
}

bool sfmFileRename(const Common::String &oldName, const Common::String &newName) {
	Common::String smOldName = makeSfmFilename(oldName);
	Common::String smNewName = makeSfmFilename(newName);
	return g_system->getSavefileManager()->renameSavefile(smOldName, smNewName, false);
}

} // End of namespace Wintermute
