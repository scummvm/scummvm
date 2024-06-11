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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/file.h"
#include "common/compression/unzip.h"

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/qd_file_manager.h"
#include "qdengine/core/qdcore/qd_game_dispatcher.h"


namespace QDEngine {

qdFilePackage::qdFilePackage() : container_index_(0),
#ifndef _NO_ZIP_
	container_(0),
#endif
	drive_ID_(LOCAL_DRIVE_ID) {
}

qdFilePackage::~qdFilePackage() {
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

	fname = Common::String::format("Resource/resource%d.pak", container_index_);

	return fname.c_str();
}

void qdFilePackage::init() {
}

qdFileManager::qdFileManager() {
	enable_packages_ = false;

	for (int i = 0; i < ARRAYSIZE(_packages); i++) {
		_packages[i].init();
		_packages[i].set_container_index(i);

		if (!_packages[i].check_container())
			_packageCount = i;
	}
}

qdFileManager::~qdFileManager() {
}

bool qdFileManager::init(int cd_count) {
	if (!enable_packages_) return false;

	return true;
}

qdFileManager &qdFileManager::instance() {
	static qdFileManager mgr;
	return mgr;
}

bool qdFileManager::open_file(XZipStream &fh, const char *file_name, bool err_message) {
	warning("STUB: qdFileManager::open_file(%s)", file_name);

	return false;
}

bool qdFileManager::open_file(Common::SeekableReadStream **fh, const char *file_name, bool err_message) {
	for (int i = 0; i < _packageCount; i++) {
		if (!_packages[i].is_open())
			_packages[i].open();

		if (!_packages[i].is_open())
			continue;

		if (_packages[i]._container->hasFile(file_name)) {
			*fh = _packages[i]._container->createReadStreamForMember(file_name);

			if (*fh)
				return true;

			return false;
		}
	}

	return false;
}

bool qdFileManager::is_package_available(const qdFileOwner &file_owner) {
	return true;
}

} // namespace QDEngine
