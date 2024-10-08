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

#ifndef BACKENDS_FS_CHROOT_CHROOT_FS_H
#define BACKENDS_FS_CHROOT_CHROOT_FS_H

#include "backends/fs/posix/posix-fs.h"

class ChRootFilesystemNode final : public AbstractFSNode {
	Common::String _root;
	Common::String _drive;
	POSIXFilesystemNode *_realNode;

	ChRootFilesystemNode(const Common::String &root, POSIXFilesystemNode *, const Common::String &drive);

public:
	ChRootFilesystemNode(const Common::String &root, const Common::String &path, const Common::String &drive = Common::String());
	~ChRootFilesystemNode() override;

	bool exists() const override;
	Common::U32String getDisplayName() const override;
	Common::String getName() const override;
	Common::String getPath() const override;
	bool isDirectory() const override;
	bool isReadable() const override;
	bool isWritable() const override;

	AbstractFSNode *getChild(const Common::String &n) const override;
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	AbstractFSNode *getParent() const override;

	Common::SeekableReadStream *createReadStream() override;
	Common::SeekableWriteStream *createWriteStream(bool atomic) override;
	bool createDirectory() override;

private:
	static Common::String addPathComponent(const Common::String &path, const Common::String &component);
};

#endif /* BACKENDS_FS_CHROOT_CHROOT_FS_H */
