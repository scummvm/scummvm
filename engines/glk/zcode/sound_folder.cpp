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

#include "glk/zcode/sound_folder.h"
#include "common/file.h"
#include "common/unzip.h"

namespace Glk {
namespace ZCode {

void SoundSubfolder::check(const Common::FSNode &gameDir) {
	Common::FSNode sound = gameDir.getChild("sound");
	if (sound.isDirectory())
		SearchMan.add("sound", new SoundSubfolder(sound));
}

SoundSubfolder::SoundSubfolder(const Common::FSNode &folder) : _folder(folder) {
	Common::FSList files;
	if (folder.getChildren(files, Common::FSNode::kListFilesOnly)) {
		for (uint idx = 0; idx < files.size(); ++idx) {
			Common::String filename = files[idx].getName();
			if (filename.hasSuffixIgnoreCase(".snd")) {
				int fileNum = atoi(filename.c_str() + filename.size() - 6);
				Common::String newName = Common::String::format("sound%d.snd", fileNum);

				_filenames[newName] = filename;
			}
		}
	}
}

bool SoundSubfolder::hasFile(const Common::String &name) const {
	return _filenames.contains(name);
}

int SoundSubfolder::listMembers(Common::ArchiveMemberList &list) const {
	int total = 0;
	for (Common::StringMap::iterator i = _filenames.begin(); i != _filenames.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember((*i)._key, this)));
		++total;
	}

	return total;
}

const Common::ArchiveMemberPtr SoundSubfolder::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *SoundSubfolder::createReadStreamForMember(const Common::String &name) const {
	Common::File *f = new Common::File();
	if (_filenames.contains(name) && f->open(_folder.getChild(_filenames[name])))
		return f;

	delete f;
	return nullptr;
}

/*--------------------------------------------------------------------------*/

void SoundZip::check(const Common::FSNode &gameDir, Story story) {
	if (story != LURKING_HORROR && story != SHERLOCK)
		return;
	Common::String zipName = (story == LURKING_HORROR) ? "lhsound.zip" : "shsound.zip";
	
	// Check for the existance of the zip
	Common::FSNode zipNode = gameDir.getChild(zipName);
	if (!zipNode.exists())
		return;

	SearchMan.add("sound", new SoundZip(Common::makeZipArchive(zipNode)));
}

SoundZip::SoundZip(Common::Archive *zip) : _zip(zip) {
	Common::ArchiveMemberList files;
	zip->listMembers(files);

	for (Common::ArchiveMemberList::iterator i = files.begin(); i != files.end(); ++i) {
		Common::String filename = (*i)->getName();
		if (filename.hasSuffixIgnoreCase(".snd")) {
			int fileNum = atoi(filename.c_str() + filename.size() - 6);
			Common::String newName = Common::String::format("sound%d.snd", fileNum);

			_filenames[newName] = filename;
		}
	}
}

SoundZip::~SoundZip() {
	delete _zip;
}

bool SoundZip::hasFile(const Common::String &name) const {
	return _filenames.contains(name);
}

int SoundZip::listMembers(Common::ArchiveMemberList &list) const {
	int total = 0;

	for (Common::StringMap::iterator i = _filenames.begin(); i != _filenames.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember((*i)._key, this)));
		++total;
	}

	return total;
}

const Common::ArchiveMemberPtr SoundZip::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));

}

Common::SeekableReadStream *SoundZip::createReadStreamForMember(const Common::String &name) const {
	if (!_filenames.contains(name))
		return nullptr;

	return _zip->createReadStreamForMember(_filenames[name]);
}

} // End of namespace ZCode
} // End of namespace Glk
