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


#ifndef QDENGINE_QDCORE_QD_FILE_MANAGER_H
#define QDENGINE_QDCORE_QD_FILE_MANAGER_H

#include "qdengine/qdcore/qd_file_owner.h"

namespace Common {
	class Archive;
}

namespace QDEngine {

class qdFilePackage {
public:
	qdFilePackage();
	~qdFilePackage();

	const char *file_name() const;

	void set_drive_ID(int drive_id) {
	}
	void set_container_index(int idx) {
		_container_index = idx;
	}

	bool is_open() {
		return _container != nullptr;
	}

	void init();
	bool open();
	void close();

	bool check_container();

	Common::Archive *_container = nullptr;

private:
	int _container_index;
};

//! Менеджер файлов.
class qdFileManager {
public:
	~qdFileManager();

	bool init(int cd_count);
	void Finit();

	void enable_packages() {}

	bool open_file(Common::SeekableReadStream **fh, const Common::Path file_name, bool err_message = true);

	int last_CD_id() const {
		return 1;
	}
	void set_last_CD_id(int cd_id) {
	}

	int CD_count() const {
		return 1;
	}
	bool is_CD_available(int cd_id) const {
		return true;
	}
	bool is_package_available(int idx) {
		assert(idx >= 0 && idx < _packageCount);
		return _packages[idx].is_open();
	}
	bool is_package_available(const qdFileOwner &file_owner);
	int get_num_packages() { return _packageCount; }

	Common::Archive *get_package(int idx) {
		if (!_packages[idx].is_open())
			_packages[idx].open();

		return _packages[idx]._container;
	}

	bool scan_drives(const qdFileOwner *file_owner = NULL) { return true; }
	bool scan_drives(int cd_id) { return true; }
	void update_packages() {}

	void toggle_silent_update_mode(bool mode) {
	}

	bool check_drives(const char *cdkey = NULL) { return true; }
	bool check_drive(int drive_letter)  { return true; }

	static qdFileManager &instance();

private:

	qdFileManager();

	qdFilePackage _packages[3];

	int _packageCount = 0;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_FILE_MANAGER_H
