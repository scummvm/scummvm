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

#ifndef GLK_ZCODE_SOUND_FOLDER_H
#define GLK_ZCODE_SOUND_FOLDER_H

#include "glk/zcode/frotz_types.h"
#include "common/archive.h"
#include "common/fs.h"
#include "common/hash-str.h"

namespace Glk {
namespace ZCode {

/**
 * Acts as an interface to an Infocom sound subfolder for the Lurking Horror or
 * Sherlock. Any file which ends with a number and '.snd' will be accessible as
 * 'sound<num>.snd' in the outer Glk layer
 */
class SoundSubfolder : public Common::Archive {
private:
	Common::FSNode _folder;
	Common::StringMap _filenames;
private:
	/**
	 * Constructor
	 */
	SoundSubfolder(const Common::FSNode &folder);
public:
	/**
	 * Checks for a sound subfolder, and if so, instantiates the class for it
	 */
	static void check(const Common::FSNode &gameDir);

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::String &name) const override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
};

/**
 * Acts as an interface to a Zip file from if-archive for the Lurking Horror or
 * Sherlock. Any file which ends with a number and '.snd' will be accessible as
 * 'sound<num>.snd' in the outer Glk layer
 */
class SoundZip : public Common::Archive {
private:
	Common::Archive *_zip;
	Common::StringMap _filenames;
private:
	/**
	 * Constructor
	 */
	SoundZip(Common::Archive *zip);
public:
	/**
	 * Checks for a sound subfolder, and if so, instantiates the class for it
	 */
	static void check(const Common::FSNode &gameDir, Story story);

	/**
	 * Destructor
	 */
	~SoundZip() override;

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::String &name) const override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
