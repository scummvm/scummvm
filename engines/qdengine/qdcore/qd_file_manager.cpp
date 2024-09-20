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

#include "common/debug.h"
#include "common/file.h"
#include "common/compression/unzip.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_file_manager.h"

namespace QDEngine {

qdFilePackage::qdFilePackage() : _container_index(0) {
}

qdFilePackage::~qdFilePackage() {
	close();
}

bool qdFilePackage::open() {
	Common::File zipFile;
	if (zipFile.exists(file_name())) {
		_container = Common::makeZipArchive(file_name());

		return is_open();
	}

	return false;
}

void qdFilePackage::close() {
	delete _container;
	_container = nullptr;
}

bool qdFilePackage::check_container() {
	Common::File zipFile;
	return zipFile.exists(file_name());
}

const char *qdFilePackage::file_name() const {
	static Common::String fname;

	fname = Common::String::format("Resource/resource%d.pak", _container_index);

	return fname.c_str();
}

void qdFilePackage::init() {
}

qdFileManager::qdFileManager() {
	for (int i = 0; i < ARRAYSIZE(_packages); i++) {
		_packages[i].init();
		_packages[i].set_container_index(i);

		if (_packages[i].check_container())
			_packageCount = i + 1;
	}

	debug(0, "qdFileManager(): Package count: %d", _packageCount);
}

qdFileManager::~qdFileManager() {
}

bool qdFileManager::init(int cd_count) {
	return true;
}

qdFileManager *mgr = nullptr;

qdFileManager &qdFileManager::instance() {
	if (!mgr)
		mgr = new qdFileManager();

	return *mgr;
}

void qdFileManager::Finit() {
	delete mgr;
}

bool qdFileManager::open_file(Common::SeekableReadStream **fh, const Common::Path file_name, bool err_message) {
	debugC(4, kDebugLoad, "qdFileManager::open_file(%s)", transCyrillic(file_name.toString()));

	if (SearchMan.hasFile((char *)transCyrillic(file_name.toString()))) {
		Common::File *f = new Common::File;

		if (f->open((char *)transCyrillic(file_name.toString())))
			*fh = f;
		else
			return false;

		return true;
	}

	for (int i = 0; i < _packageCount; i++) {
		if (!_packages[i].is_open())
			_packages[i].open();

		if (!_packages[i].is_open())
			continue;

		if (_packages[i]._container->hasFile(file_name)) {
			debugC(5, kDebugLoad, "qdFileManager::open_file(%s): found in %s", transCyrillic(file_name.toString()), _packages[i].file_name());

			*fh = _packages[i]._container->createReadStreamForMember(file_name);

			if (*fh)
				return true;

			debugC(4, kDebugLoad, "qdFileManager::open_file(%s): Cannot read file", transCyrillic(file_name.toString()));

			return false;
		}
	}

	warning("qdFileManager::open_file(%s) (%s): NOT FOUND", transCyrillic(file_name.toString()), file_name.toString().c_str());

	return false;
}

bool qdFileManager::is_package_available(const qdFileOwner &file_owner) {
	return true;
}

} // namespace QDEngine
