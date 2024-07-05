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

#ifndef QDENGINE_SYSTEM_APP_CORE_H
#define QDENGINE_SYSTEM_APP_CORE_H



namespace QDEngine {

// Main window handle
void *appGetHandle();
void appSetHandle(void *hwnd);

unsigned app_memory_usage();

typedef void (*SetFunc)(const char *);
typedef const char *(*GetFunc)(void);

namespace app_io {

bool is_file_exist(const char *file_name);
bool is_directory_exist(const char *dir_name);

bool set_current_directory(const char *file_name);
const char *strip_path(const char *file_name);
const char *strip_file_name(const char *path);
const char *get_ext(const char *file_name);
const char *change_ext(const char *file_name, const char *new_ext);

unsigned file_size(const char *file_name);

std::string path_to_file_name(const std::string &str);

bool relat_path(const char *path);
void adjust_dir_end_slash(std::string &str);
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_APP_CORE_H
