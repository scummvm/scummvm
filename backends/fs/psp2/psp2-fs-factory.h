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

#ifndef PSP2_FILESYSTEM_FACTORY_H
#define PSP2_FILESYSTEM_FACTORY_H

#include "common/singleton.h"
#include "backends/fs/fs-factory.h"
#include "backends/fs/psp2/psp2-fs.h"

/**
 * Creates PSP2FilesystemNode objects.
 *
 * Parts of this class are documented in the base interface class, FilesystemFactory.
 */
class PSP2FilesystemFactory final : public FilesystemFactory, public Common::Singleton<PSP2FilesystemFactory> {
public:
    PSP2FilesystemFactory() : _config(this) { }

    /**
     * Add a drive to the top-level directory
     */
	void addDrive(const Common::String &name);

protected:
	AbstractFSNode *makeRootFileNode() const override;
	AbstractFSNode *makeCurrentDirectoryFileNode() const override;
	AbstractFSNode *makeFileNodePath(const Common::String &path) const override;

    typedef Common::Array<Common::String> DrivesArray;
	struct StaticDrivesConfig : public PSP2FilesystemNode::Config {
		StaticDrivesConfig(const PSP2FilesystemFactory *factory) : _factory(factory) { }

		bool getDrives(AbstractFSList &list, bool hidden) const override;
		bool isDrive(const Common::String &path) const override;

		DrivesArray drives;

	private:
		const PSP2FilesystemFactory *_factory;
	};

	StaticDrivesConfig _config;

private:
	friend class Common::Singleton<SingletonBaseType>;
};

#endif /*PSP2_FILESYSTEM_FACTORY_H*/
