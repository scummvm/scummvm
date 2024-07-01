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

#ifndef QDENGINE_CORE_PARSER_QDSCR_CONVERT_H
#define QDENGINE_CORE_PARSER_QDSCR_CONVERT_H

namespace QDEngine {

//! Преобразование старого скрипта в новый XML формат.
bool qdscr_convert_to_XML(const char *file_name, const char *new_file_name = NULL);
//! Возвращает true, если скрипт в новом формате (проверяет только соответствие расширения имени файла).
bool qdscr_is_XML(const char *file_name);
//! Меняет расширение имени файла (на .qds).
const char *qdscr_get_XML_file_name(const char *file_name);

} // namespace QDEngine

#endif // QDENGINE_CORE_PARSER_QDSCR_CONVERT_H
