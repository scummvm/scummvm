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

#ifndef POSIX_DRIVES_FILESYSTEM_H
#define POSIX_DRIVES_FILESYSTEM_H

#include "backends/fs/posix/posix-fs.h"

class StdioStream;

/**
 * POSIX file system node where the top-level directory is a hardcoded
 * list of drives.
 */
class DrivePOSIXFilesystemNode : public POSIXFilesystemNode {
protected:
	virtual AbstractFSNode *makeNode() const {
		return new DrivePOSIXFilesystemNode(_config);
	}
	AbstractFSNode *makeNode(const Common::String &path) const override {
		return new DrivePOSIXFilesystemNode(path, _config);
	}

public:
	enum BufferingMode {
		/** IO buffering is fully disabled */
		kBufferingModeDisabled,
		/** IO buffering is enabled and uses the libc implemenation */
		kBufferingModeStdio,
		/** IO buffering is enabled and uses ScummVM's buffering stream wraappers */
		kBufferingModeScummVM
	};

	struct Config {
		// Use the default stdio buffer size
		Config() : bufferingMode(kBufferingModeStdio), bufferSize(0) { }
		virtual ~Config() { }

		virtual bool getDrives(AbstractFSList &list, bool hidden) const = 0;
		virtual bool isDrive(const Common::String &path) const = 0;

		BufferingMode bufferingMode;
		uint32 bufferSize;
	};

	DrivePOSIXFilesystemNode(const Common::String &path, const Config &config);
	DrivePOSIXFilesystemNode(const Config &config);

	// AbstractFSNode API
	Common::SeekableReadStream *createReadStream() override;
	Common::SeekableWriteStream *createWriteStream(bool atomic) override;
	AbstractFSNode *getChild(const Common::String &n) const override;
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	AbstractFSNode *getParent() const override;

protected:
	const Config &_config;

private:
	bool _isPseudoRoot;

	DrivePOSIXFilesystemNode *getChildWithKnownType(const Common::String &n, bool isDirectoryFlag) const;
	bool isDrive(const Common::String &path) const;
	void configureStream(StdioStream *stream);
};

#endif
