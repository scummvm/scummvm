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


#ifndef BACKENDS_FS_CHROOT_CHROOT_FS_FACTORY_H
#define BACKENDS_FS_CHROOT_CHROOT_FS_FACTORY_H

#include "backends/fs/fs-factory.h"

/**
 * FIXME: Warning, using this factory in your backend may silently break some
 * features. Instances are, for example, the FluidSynth code, and the POSIX
 * plugin code.
 */
class ChRootFilesystemFactory final : public FilesystemFactory {
public:
	explicit ChRootFilesystemFactory(const Common::String &root);

	AbstractFSNode *makeRootFileNode() const override;
	AbstractFSNode *makeCurrentDirectoryFileNode() const override;
	AbstractFSNode *makeFileNodePath(const Common::String &path) const override;

private:
	const Common::String _root;
};

#endif /* BACKENDS_FS_CHROOT_CHROOT_FS_FACTORY_H */
