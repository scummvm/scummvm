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


#ifndef __QD_FILE_MANAGER_H__
#define __QD_FILE_MANAGER_H__

#define _NO_ZIP_

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/qd_file_owner.h"
#include "qdengine/core/qdcore/util/zip_container.h"

namespace Common {
	class Archive;
}

namespace QDEngine {

//! Контейнер с файлами.
class qdFilePackage {
public:
	qdFilePackage();
	~qdFilePackage();

	enum {
		LOCAL_DRIVE_ID = -1
	};

	//! Возвращает имя файла контейнера.
	const char *file_name() const;

	void set_drive_ID(int drive_id) {
		drive_ID_ = drive_id;
	}
	void set_container_index(int idx) {
		container_index_ = idx;
	}

	bool is_open() {
		return _container != nullptr;
	}

	void init();
	bool open();
	void close();

	//! Проверяет наличие файла контейнера.
	/**
	Если файл отсутствует - закрывает контейнер и возвращает false.
	*/
	bool check_container();

	Common::Archive *_container = nullptr;

private:
	//! Номер диска, на котором расположен контейнер.
	/**
	если равен LOCAL_DRIVE_ID - контейнер скопирован к игре в директорию Resources
	иначе лежит на диске A + drive_ID_
	*/
	int drive_ID_;

	//! Номер контейнера.
	/**
	имя файла контейнера - resourceX.pak
	где X - container_index_
	*/
	int container_index_;
};

//! Менеджер файлов.
class qdFileManager {
public:
	~qdFileManager();

	bool init(int cd_count);

	void enable_packages() {
		enable_packages_ = true;
	}

	bool open_file(XZipStream &fh, const char *file_name, bool err_message = true);
	bool open_file(Common::SeekableReadStream **fh, const char *file_name, bool err_message = true);

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

	const char *CD_path(const qdFileOwner &file_owner) const { return ""; }

	bool scan_drives(const qdFileOwner *file_owner = NULL) { return true; }
	bool scan_drives(int cd_id) { return true; }
	void update_packages() {}

	void toggle_silent_update_mode(bool mode) {
	}

	bool check_drives(const char *cdkey = NULL) { return true; }
	bool check_drive(int drive_letter)  { return true; }

	static qdFileManager &instance();

	typedef bool (*request_CD_handler_t)(int cd_id);
	void set_request_CD_handler(request_CD_handler_t new_handler) {}

private:

	qdFileManager();

	bool enable_packages_;
	qdFilePackage _packages[3];

	int _packageCount = 0;
};

} // namespace QDEngine

#endif // __QD_FILE_MANAGER_H__
