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

#include "common/scummsys.h"
#include "common/config-manager.h"

#include "backends/platform/sdl/atari/atari.h"
#include "backends/fs/atari/atari-fs-factory.h"

void OSystem_SDL_Atari::init() {
	_fsFactory = new AtariFilesystemFactory();

	// OSystem_POSIX::init() would create its own file system factory
	OSystem_SDL::init();
}

void OSystem_SDL_Atari::initBackend() {
	if (!ConfMan.hasKey("themepath", Common::ConfigManager::kApplicationDomain))
		ConfMan.setPath("themepath", "themes", Common::ConfigManager::kApplicationDomain);

	if (!ConfMan.hasKey("audio_buffer_size", Common::ConfigManager::kApplicationDomain))
		ConfMan.setInt("audio_buffer_size", 2048, Common::ConfigManager::kApplicationDomain);

	OSystem_POSIX::initBackend();
}

void OSystem_SDL_Atari::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	AbstractFSNode *currentDirectory = getFilesystemFactory()->makeCurrentDirectoryFileNode();
	if (currentDirectory) {
		const Common::FSNode dataNode = Common::FSNode(Common::Path(currentDirectory->getPath())).getChild("data");
		if (dataNode.exists() && dataNode.isDirectory() && dataNode.isReadable())
			s.addDirectory(dataNode.getPath(), priority);

		delete currentDirectory;
	}

	OSystem_POSIX::addSysArchivesToSearchSet(s, priority);
}
