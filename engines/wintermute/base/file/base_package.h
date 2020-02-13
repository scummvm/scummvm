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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_PACKAGE_H
#define WINTERMUTE_BASE_PACKAGE_H

#include "common/archive.h"
#include "common/stream.h"
#include "common/fs.h"

namespace Wintermute {
class BasePackage {
public:
	Common::SeekableReadStream *getFilePointer();
	Common::FSNode _fsnode;
	bool _boundToExe;
	byte _priority;
	Common::String _name;
	int32 _cd;
	BasePackage();
};

class PackageSet : public Common::Archive {
public:
	~PackageSet() override;

	PackageSet(Common::FSNode package, const Common::String &filename = "", bool searchSignature = false);
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

	int getPriority() const { return _priority; }
	uint32 getVersion() const { return _version; }

private:
	byte _priority;
	uint32 _version;
	Common::Array<BasePackage *> _packages;
	Common::HashMap<Common::String, Common::ArchiveMemberPtr> _files;
	Common::HashMap<Common::String, Common::ArchiveMemberPtr>::iterator _filesIter;
};

} // End of namespace Wintermute

#endif
