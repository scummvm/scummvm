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

#include "backends/platform/android/jni-android.h"

#include "backends/fs/android/android-fs-factory.h"
#include "backends/fs/android/android-posix-fs.h"
#include "backends/fs/android/android-saf-fs.h"

namespace Common {
DECLARE_SINGLETON(AndroidFilesystemFactory);
}

AndroidFilesystemFactory::AndroidFilesystemFactory() : _withSAF(false), _config(this) {
}

void AndroidFilesystemFactory::initSAF() {
	_withSAF = true;
	AndroidSAFFilesystemNode::initJNI();
}

AbstractFSNode *AndroidFilesystemFactory::makeRootFileNode() const {
	return new AndroidPOSIXFilesystemNode(_config);
}

AbstractFSNode *AndroidFilesystemFactory::makeCurrentDirectoryFileNode() const {
	// As current working directory can point outside of our data don't take any risk
	return makeRootFileNode();
}

AbstractFSNode *AndroidFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	if (path.empty() || path.equals("/")) {
		return makeRootFileNode();
	}

	// If SAF works, it was a SAF URL
	if (_withSAF) {
		// Accept /saf as it can be used to create the tree in DumpFile
		if (path == AddSAFFakeNode::SAF_ADD_FAKE_PATH) {
			// Not a SAF mount point
			return new AddSAFFakeNode(true);
		}

		AbstractFSNode *node = AndroidSAFFilesystemNode::makeFromPath(path);
		if (node) {
			return node;
		}
	}

	return new AndroidPOSIXFilesystemNode(path, _config);
}

AndroidFilesystemFactory::Config::Config(const AndroidFilesystemFactory *factory) : _factory(factory),
	_storages(JNI::getAllStorageLocations()) {
}

void AndroidFilesystemFactory::getSAFTrees(AbstractFSList &list, bool allowSAFadd) const {
	if (!_withSAF) {
		// Nothing if no SAF
		return;
	}

	Common::Array<jobject> trees = JNI::getSAFTrees();

	list.reserve(trees.size() + (allowSAFadd ? 1 : 0));

	for (Common::Array<jobject>::iterator it = trees.begin(); it != trees.end(); it++) {
		AbstractFSNode *node = AndroidSAFFilesystemNode::makeFromTree(*it);
		if (!node) {
			continue;
		}

		list.push_back(node);
	}

	if (allowSAFadd) {
		list.push_back(new AddSAFFakeNode(false));
	}

}

bool AndroidFilesystemFactory::Config::getDrives(AbstractFSList &list, bool hidden) const {
	// For SAF
	_factory->getSAFTrees(list, true);

	list.reserve(list.size() + _storages.size() / 2);

	// For old POSIX way
	for (Common::Array<Common::String>::const_iterator it = _storages.begin(); it != _storages.end(); ++it) {
		const Common::String &driveName = *it;
		++it;
		const Common::String &drivePath = *it;

		AndroidPOSIXFilesystemNode *node = new AndroidPOSIXFilesystemNode(drivePath, *this);
		node->_displayName = driveName;

		list.push_back(node);
	}
	return true;
}

bool AndroidFilesystemFactory::Config::isDrive(const Common::String &path) const {
	// This function is called from DrivePOSIXFilesystemNode::isDrive
	// DrivePOSIXFilesystemNode is only used for POSIX code so no need to look for SAF

	for (Common::Array<Common::String>::const_iterator it = _storages.begin(); it != _storages.end(); it++) {
		++it;
		if (*it == path) {
			return true;
		}
	}
	return false;
}
