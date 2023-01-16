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

#ifndef ANDROID_FILESYSTEM_FACTORY_H
#define ANDROID_FILESYSTEM_FACTORY_H

#include "backends/fs/fs-factory.h"
#include "common/singleton.h"
#include "backends/fs/posix-drives/posix-drives-fs.h"

/**
 * Creates AndroidFilesystemNode objects.
 *
 * Parts of this class are documented in the base interface class, FilesystemFactory.
 */
class AndroidFilesystemFactory final : public FilesystemFactory,
	public Common::Singleton<AndroidFilesystemFactory> {
	friend class Common::Singleton<SingletonBaseType>;
protected:
	AndroidFilesystemFactory();
public:
	AbstractFSNode *makeRootFileNode() const override;
	AbstractFSNode *makeCurrentDirectoryFileNode() const override;
	AbstractFSNode *makeFileNodePath(const Common::String &path) const override;

	void initSAF();
	bool hasSAF() const { return _withSAF; }
	void getSAFTrees(AbstractFSList &list, bool allowSAFadd) const;
private:
	struct Config : public DrivePOSIXFilesystemNode::Config {
		Config(const AndroidFilesystemFactory *factory);

		bool getDrives(AbstractFSList &list, bool hidden) const override;
		bool isDrive(const Common::String &path) const override;

	private:
		const AndroidFilesystemFactory *_factory;
		Common::Array<Common::String> _storages;
	};

	bool _withSAF;
	Config _config;
};

#endif /*ANDROID_FILESYSTEM_FACTORY_H*/
