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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/str.h"
#include "common/textconsole.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/app_core.h"
#include "qdengine/qdengine.h"
#include "qdengine/xlibs/xutil/xutil.h"

namespace QDEngine {

void *app_hWnd = 0;

void *appGetHandle() {
	return app_hWnd;
}

void appSetHandle(void *hwnd) {
	app_hWnd = hwnd;
}

unsigned app_memory_usage() {
	unsigned size = 0;

	warning("STUB: app_memory_usage()");
#if 0
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	MEMORY_BASIC_INFORMATION Buffer;
	VirtualQuery(SystemInfo.lpMinimumApplicationAddress, &Buffer,  sizeof(Buffer));
	while (Buffer.BaseAddress < SystemInfo.lpMaximumApplicationAddress) {
		if (Buffer.State == MEM_COMMIT && !(Buffer.Type & MEM_MAPPED) && Buffer.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READ))
			size += Buffer.RegionSize;
		void *prev_address = Buffer.BaseAddress;
		VirtualQuery((char *)Buffer.BaseAddress + Buffer.RegionSize, &Buffer,  sizeof(Buffer));
		if (prev_address == Buffer.BaseAddress)
			break;
	}
#endif

	return size;
}

namespace app_io {

bool is_file_exist(const char *file_name) {
	/*  XStream fh(0);

	    if(fh.open(file_name,XS_IN)){
	        fh.close();
	        return true;
	    }

	    return false;*/
	warning("STUB: is_file_exist()");
#if 0
	return (GetFileAttributes(file_name) != -1);
#endif

	return false;
}

const char *strip_path(const char *file_name) {
	static XBuffer name_buf(_MAX_PATH);
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];


	_splitpath(file_name, NULL, NULL, fname, ext);

	name_buf.init();
	name_buf.operator < (fname).operator < (ext);


	return name_buf.c_str();
}

bool is_directory_exist(const char *dir_name) {
	warning("STUB: is_directory_exist()");
#if 0
	 DWORD attr = GetFileAttributes(dir_name);
	if (attr != -1 && attr & FILE_ATTRIBUTE_DIRECTORY)
		return true;
#endif
	return false;
}

const char *strip_file_name(const char *path) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];

	static XBuffer name_buf(_MAX_PATH);

	_splitpath(path, drive, dir, NULL, NULL);

	name_buf.init();
	name_buf.operator < (drive).operator < (dir);

	return name_buf.c_str();

}

const char *get_ext(const char *file_name) {
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	static XBuffer name_buf(_MAX_PATH);

	_splitpath(file_name, NULL, NULL, fname, ext);

	name_buf.init();
	name_buf.operator < (ext);

	return name_buf.c_str();
}

const char *change_ext(const char *file_name, const char *new_ext) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];

	static XBuffer name_buf(_MAX_PATH);

	_splitpath(file_name, drive, dir, fname, NULL);

	name_buf.init();
	name_buf.operator < (drive).operator < (dir).operator < (fname).operator < (new_ext);

	return name_buf.c_str();
}

bool create_directory(const char *path) {
	static char cur_path[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, cur_path);

	int path_length = strlen(path);
	int idx = 0;
	static char dir_name[MAX_PATH];

	if (path_length && path[0] == '\\') {
		SetCurrentDirectory("\\");
		idx++;
	} else if (path_length > 3 && path[1] == ':') {
		int dir_idx = 0;
		while (idx < 3) dir_name[dir_idx++] = path[idx++];
		dir_name[dir_idx] = 0;
		SetCurrentDirectory(dir_name);
	}

	while (idx < path_length) {
		int dir_idx = 0;
		while (path[idx] != '\\') {
			dir_name[dir_idx++] = path[idx++];
			if (idx >= path_length)
				break;
		}

		idx++;

		if (dir_idx) {
			dir_name[dir_idx] = 0;
			CreateDirectory(dir_name, NULL);
			bool ret = SetCurrentDirectory(dir_name);
		}
	}

	SetCurrentDirectory(cur_path);
	return true;

}

bool remove_directory(const char *path) {
	char cur_path[MAX_PATH];
	warning("STUB: remove_directory()");
#if 0
	GetCurrentDirectory(MAX_PATH, cur_path);

	if (!SetCurrentDirectory(path)) return false;

	WIN32_FIND_DATA fd;
	HANDLE  fh = FindFirstFile("*.*", &fd);

	if (fh != INVALID_HANDLE_VALUE) {
		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (fd.cFileName[0] != '.') {
					remove_directory(fd.cFileName);
					RemoveDirectory(fd.cFileName);
				}
			} else {
				SetFileAttributes(fd.cFileName, FILE_ATTRIBUTE_ARCHIVE);
				DeleteFile(fd.cFileName);
			}
		} while (FindNextFile(fh, &fd));
		FindClose(fh);
	}

	SetCurrentDirectory(cur_path);
#endif
	return true;
}

unsigned file_size(const char *file_name) {
	XStream fh(0);
	if (fh.open(file_name, XS_IN))
		return fh.size();

	return 0;
}

// Вспомогательная функция копирования (дополнительно создает дирректорию и
// если файл уже существует убирает с него read-only/system/hidden аттрибуты)
bool copy_file(const char *target, const char *source) {
	app_io::create_directory(app_io::strip_file_name(target));
	warning("STUB: copy_file()");
#if 0
	DWORD attr = GetFileAttributes(target);
	if (attr != -1)
		SetFileAttributes(target, attr & ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));

	return CopyFile(source, target, 0);
#endif
}

std::string path_to_file_name(const std::string &str) {
	std::string conv_str = str;
	for (int i = 0; i < conv_str.length(); i++)
		if ('\\' == conv_str[i])
			conv_str[i] = '_';
	return conv_str;
}

bool dupe_resolve_file_copy(std::string &target, const char *source) {
	if (false == is_file_exist(target.c_str()))
		return copy_file(target.c_str(), source);

	std::string correct_path = target;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	while ((true == is_file_exist(correct_path.c_str())) && (correct_path.size() < MAX_PATH)) {
		_splitpath(correct_path.c_str(), drive, dir, fname, ext);
		correct_path = drive;
		correct_path += dir;
		correct_path += fname;
		correct_path += '2'; // Добавляем двоечку перед именем файла
		correct_path += ext;
	}

	if (true == is_file_exist(correct_path.c_str()))
		return false;

	if (copy_file(correct_path.c_str(), source)) {
		target = correct_path;
		return true;
	}
	return false;
}

bool copy_file_list(const std::list<std::string> &file_list, const char *target_dir, const char *file_extension) {
	bool copy_ok = true;
	std::string save_str;
	for (qdFileNameList::const_iterator it = file_list.begin(); it != file_list.end(); it++)
		if (0 == scumm_stricmp(file_extension, get_ext(it->c_str()))) {
			// Формируем имя файла и прикрепляем к нему путь папки-хранилища файлов
			save_str = app_io::path_to_file_name(it->c_str());
			save_str = '\\' + save_str;
			save_str = target_dir + save_str;
			// Копируем и сообщаем об ошибке, если произошла
			if (!app_io::copy_file(save_str.c_str(), it->c_str())) {
				debugC(3, kDebugLog, "Error: could not copy %s to directory %s", transCyrillic(it->c_str()), transCyrillic(target_dir));
				copy_ok = false;
			}
		}
	return copy_ok;
}

bool relat_path(const char *path) {
	int len = strlen(path);
	if (len < 2) return true;

	if (('.' == path[0]) && ('.' == path[1])) return false;
	if (':' == path[1]) return false;

	return true;
}

void adjust_dir_end_slash(std::string &str) {
	if ((str.size() > 0) && ('\\' != str[str.size() - 1]))
		str = str + '\\';
}

}; /* namespace app_io */

} // namespace QDEngine
