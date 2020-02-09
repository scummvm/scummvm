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

#ifndef ULTIMA_SHARED_ENGINE_RESOURCES_H
#define ULTIMA_SHARED_ENGINE_RESOURCES_H

#include "common/algorithm.h"
#include "common/archive.h"
#include "common/array.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/serializer.h"
#include "ultima/shared/core/file.h"

#define STRING_BUFFER_SIZE 32768

namespace Ultima {
namespace Shared {

class Resources;

/**
 * Base class for classes that exposes a set of strings, arrays, and other data from a resource
 */
class ResourceFile {
private:
	File _file;
	char _buffer[STRING_BUFFER_SIZE];
	char *_bufferP;
protected:
	Common::String _filename;
protected:
	/**
	 * Constructor
	 */
	ResourceFile(const Common::String &filename);

	/**
	 * Destructor
	 */
	virtual ~ResourceFile() {
	}

	/**
	 * Synchronizes the data for the resource
	 */
	virtual void synchronize() = 0;

	virtual void syncString(const char *&str);
	virtual void syncStrings(const char **str, size_t count);
	virtual void syncStrings2D(const char **str, size_t count1, size_t count2);
	virtual void syncNumber(int &val);
	virtual void syncNumbers(int *vals, size_t count);
	virtual void syncNumbers2D(int *vals, size_t count1, size_t count2);
	virtual void syncNumbers3D(int *vals, size_t count1, size_t count2, size_t count3);
	virtual void syncBytes(byte *vals, size_t count);
	virtual void syncBytes2D(byte *vals, size_t count1, size_t count2);
public:
	/**
	 * Loads in a resource
	 */
	void load();
};

/**
 * Derived base class for resources that have their contents within the executable rather than a data file.
 * This will allow the data for a particular Ultima game to be gradually built up without repeatedly
 * regenerating a data file. Once a game has been properly tested, then it can be moved out.
 */
class LocalResourceFile : public ResourceFile {
private:
	Common::MemoryWriteStreamDynamic _file;
	Resources *_owner;
protected:
	/**
	 * Constructor
	 */
	LocalResourceFile(const Common::String &filename) : ResourceFile(filename), _owner(nullptr), _file(DisposeAfterUse::YES) {}

	/**
	 * Constructor
	 */
	LocalResourceFile(Resources *owner, const Common::String &filename) : ResourceFile(filename),
		_owner(owner), _file(DisposeAfterUse::YES) {}

	/**
	 * Return true if in saving mode
	 */
	bool isSaving() const { return _owner != nullptr; }

	void syncString(const char *&str) override;
	void syncStrings(const char **str, size_t count) override;
	void syncStrings2D(const char **str, size_t count1, size_t count2) override;
	void syncNumber(int &val) override;
	void syncNumbers(int *vals, size_t count) override;
	void syncNumbers2D(int *vals, size_t count1, size_t count2) override;
	void syncNumbers3D(int *vals, size_t count1, size_t count2, size_t count3) override;
	void syncBytes(byte *vals, size_t count) override;
	void syncBytes2D(byte *vals, size_t count1, size_t count2) override;
public:
	/**
	 * Write out the resource to the in-memory resource store
	 */
	void save();
};

/**
 * Resources manager
 */
class Resources : public Common::Archive {
	struct LocalResource {
		Common::String _name;
		Common::Array<byte> _data;
	};
	struct FileResource {
		Common::String _name;
		size_t _offset, _size;

		/**
		 * Load an index entry from the datafile
		 */
		void load(File &f);
	};
private:
	Common::Array<LocalResource> _localResources;
public:
	/**
	 * Constructor
	 */
	Resources() {
	}

	/**
	 * Sets up the resources for the engine
	 * @returns		False if setup failed
	 */
	bool open();

	/**
	 * Adds a resource created in memory to the ScummVM archive manager, so that it can be
	 * later opened like a normal file. Just as it will when eventually shifted to the
	 * data file for the engine
	 */
	void addResource(const Common::String &name, const byte *data, size_t size);

	// Archive implementation
	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::String &name) const  override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const  override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const  override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const  override;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
