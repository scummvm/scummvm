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

#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/util/directory.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/textconsole.h"

namespace AGS3 {

int ags_fseek(Common::Stream *stream, file_off_t offset, int whence) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(stream);

	if (rs)
		return rs->seek(offset, whence) ? 0 : 1;
	else if (ws)
		return ws->seek(offset, whence) ? 0 : 1;
	else
		error("Seek on null stream");
}

file_off_t ags_ftell(Common::Stream *stream) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(stream);
	assert(rs || ws);
	return rs ? rs->pos() : ws->pos();
}

Common::FSNode getFSNode(const char *path) {
	Common::FSNode node;
	Common::String filePath(path);
	if (filePath.empty() || filePath == "." || filePath == "./")
		return Common::FSNode(ConfMan.get("path"));
	else if (filePath.hasPrefix("./")) {
		filePath = filePath.substr(2);
		node = Common::FSNode(ConfMan.get("path"));
	} else if (filePath.hasPrefixIgnoreCase(AGS::Shared::SAVE_FOLDER_PREFIX)) {
		filePath = filePath.substr(strlen(AGS::Shared::SAVE_FOLDER_PREFIX));
		node = Common::FSNode(ConfMan.get("savepath"));
	} else {
		node = Common::FSNode(filePath);
		if (node.isReadable())
			return node;
		node = Common::FSNode(ConfMan.get("path"));
	}

	// Use FSDirectory for case-insensitive search
	Common::SharedPtr<Common::FSDirectory> dir(new Common::FSDirectory(node));

	// Iterate through any further subfolders or filename
	size_t separator;
	while ((separator = filePath.find('/')) != Common::String::npos) {
		dir.reset(dir->getSubDirectory(filePath.substr(0, separator)));
		if (!dir)
			return Common::FSNode();
		filePath = Common::String(filePath.c_str() + separator + 1);
	}

	if (filePath.empty())
		return dir->getFSNode();

	if (dir->hasFile(filePath)) {
		Common::ArchiveMemberPtr file = dir->getMember(filePath);
		if (file)
			return dir->getFSNode().getChild(file->getName());
	}

	Common::FSDirectory *subDir = dir->getSubDirectory(filePath);
	if (subDir) {
		dir.reset(subDir);
		return dir->getFSNode();
	}

	// The files does not exist, but create the FSNode anyway so that
	// the code using this can report the correct error rather than assert.
	return dir->getFSNode().getChild(filePath);
}

int  ags_file_exists(const char *path) {
	Common::String sPath(path);

	if (sPath.hasPrefix(AGS::Shared::SAVE_FOLDER_PREFIX)) {
		sPath = path + strlen(AGS::Shared::SAVE_FOLDER_PREFIX);
		Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(sPath);
		bool result = saveFile != nullptr;
		delete saveFile;

		return result ? 1 : 0;
	} else {
		Common::FSNode node = getFSNode(path);
		return node.exists() && !node.isDirectory() ? 1 : 0;
	}
}

int ags_directory_exists(const char *path) {
	Common::FSNode node = getFSNode(path);
	return node.exists() && node.isDirectory() ? 1 : 0;
}

int ags_path_exists(const char *path) {
	Common::FSNode node = getFSNode(path);
	return node.exists() ? 1 : 0;
}

file_off_t ags_file_size(const char *path) {
	Common::FSNode node = getFSNode(path);
	Common::File f;

	return f.open(node) ? f.size() : (file_off_t)-1;
}

} // namespace AGS3
