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

#if defined(__ANDROID__)

/* TODO: Only purpose of this file is to return false to the call to
 * AndroidFilesystemFactory::instance().hasSAF() in base/main.cpp used
 * to show the SAF system popup, which would need android backend enabled.
 * Cleaner fix to be found.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/android/android-fs-factory.h"
#include "libretro-fs.h"

namespace Common {
DECLARE_SINGLETON(AndroidFilesystemFactory);
}

AndroidFilesystemFactory::AndroidFilesystemFactory() : _withSAF(false), _config(this) {
}

void AndroidFilesystemFactory::initSAF() {
	_withSAF = false;
}

AbstractFSNode *AndroidFilesystemFactory::makeRootFileNode() const {
	return new LibRetroFilesystemNode("/");
}

AbstractFSNode *AndroidFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new LibRetroFilesystemNode("/");
}

AbstractFSNode *AndroidFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new LibRetroFilesystemNode(path);
}

AndroidFilesystemFactory::Config::Config(const AndroidFilesystemFactory *factory) {
}

void AndroidFilesystemFactory::getSAFTrees(AbstractFSList &list, bool allowSAFadd) const {
	return;
}

bool AndroidFilesystemFactory::Config::getDrives(AbstractFSList &list, bool hidden) const {
	return false;
}

bool AndroidFilesystemFactory::Config::isDrive(const Common::String &path) const {
	return false;
}

#endif
