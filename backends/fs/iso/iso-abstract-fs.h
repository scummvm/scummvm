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

/**
 * @file iso-abstract-fs.h
 * AbstractFSNode implementation backed by an ISO 9660 filesystem image.
 *
 * ISOAbstractFSNode wraps a single ISO9660FileSystem::DirEntry so that the
 * contents of a disc image appear as ordinary file-system nodes to
 * ScummVM's detection pipeline and Mass Add dialog.
 *
 * Factory function:
 * @code
 *   auto isoFS = Common::makeSharedPtr(
 *       new Common::ISO9660FileSystem(node.createReadStream()));
 *   Common::FSNode root = makeISOFSNodeRoot(isoFS);
 *   // root behaves like a directory node containing the ISO contents
 * @endcode
 */

#ifndef BACKENDS_FS_ISO_ISO_ABSTRACT_FS_H
#define BACKENDS_FS_ISO_ISO_ABSTRACT_FS_H

#include "backends/fs/abstract-fs.h"
#include "common/formats/iso9660.h"
#include "common/ptr.h"
#include "common/str.h"

/**
 * Virtual AbstractFSNode whose contents are read from an ISO 9660 image.
 *
 * The node stores a SharedPtr to the ISO9660FileSystem so that the
 * underlying stream stays alive as long as any node derived from it exists.
 */
class ISOAbstractFSNode : public AbstractFSNode {
protected:
	Common::SharedPtr<Common::ISO9660FileSystem> _fs;
	Common::ISO9660FileSystem::DirEntry          _entry;
	Common::String                               _virtualPath; // slash-separated, starts with '/'

public:
	/** Construct a node for the ISO root directory. */
	ISOAbstractFSNode(Common::SharedPtr<Common::ISO9660FileSystem> fs,
	                  const Common::String &isoImagePath);

	/** Construct a node for a non-root entry. */
	ISOAbstractFSNode(Common::SharedPtr<Common::ISO9660FileSystem> fs,
	                  const Common::ISO9660FileSystem::DirEntry &entry,
	                  const Common::String &virtualPath);

	~ISOAbstractFSNode() override {}

	// AbstractFSNode interface
	bool exists() const override { return true; }
	bool isDirectory() const override { return _entry.isDir; }
	bool isReadable() const override { return true; }
	bool isWritable() const override { return false; }

	Common::String getName() const override;
	Common::String getPath() const override { return _virtualPath; }
	Common::U32String getDisplayName() const override;

	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	AbstractFSNode *getChild(const Common::String &name) const override;
	AbstractFSNode *getParent() const override;

	Common::SeekableReadStream *createReadStream() override;
	Common::SeekableWriteStream *createWriteStream(bool atomic) override { return nullptr; }
	bool createDirectory() override { return false; }

	Common::SeekableReadStream *createReadStreamForAltStream(
		Common::AltStreamType altStreamType) override { return nullptr; }

private:
	Common::String lastComponent() const;
	Common::String childPath(const Common::String &childName) const;
};

/**
 * Create a Common::FSNode wrapping the root directory of an ISO image.
 *
 * @param fs            Shared pointer to an open ISO9660FileSystem.
 * @param isoImagePath  Display path of the .iso file (used as virtual root path).
 */
Common::FSNode makeISOFSNodeRoot(Common::SharedPtr<Common::ISO9660FileSystem> fs,
                                 const Common::String &isoImagePath = "");

#endif // BACKENDS_FS_ISO_ISO_ABSTRACT_FS_H
