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

#pragma once

namespace QDEngine {

using namespace std;

typedef vector<string> ComboStrings;
typedef vector<wstring> ComboWStrings;

/// Вспомогательная функция для работы с комбо-листами
string cutTokenFromComboList(string &comboList);
void joinComboList(string &outComboList, const ComboStrings &strings, char delimeter = '|');
void splitComboList(ComboStrings &outComboStrings, const char *comboList, char delimeter = '|');
// TODO: переписать
int indexInComboListString(const char *comboList, const char *value);

void splitComboListW(ComboWStrings &combo_array, const wchar_t *ptr, wchar_t delimeter);
wstring getStringTokenByIndexW(const wchar_t *worker, int number);
int indexInComboListStringW(const wchar_t *comboList, const wchar_t *value);

} // namespace QDEngine
