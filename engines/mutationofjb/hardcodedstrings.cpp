/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mutationofjb/hardcodedstrings.h"

#include "mutationofjb/game.h"
#include "mutationofjb/util.h"

#include "common/file.h"

namespace MutationOfJB {

HardcodedStrings::HardcodedStrings(Game &game) : _strings(STRING_TYPES_TOTAL) {
	loadStrings(game.getLanguage());
}

const Common::String &HardcodedStrings::getString(HardcodedStrings::StringType strType) const {
	const StringArray::size_type index = static_cast<StringArray::size_type>(strType);
	assert(index < _strings.size());

	return _strings[index];
}

void HardcodedStrings::loadStrings(Common::Language lang) {
	Common::File file;
	const char *const fileName = "jb.ex_";
	if (!file.open(fileName)) {
		reportFileMissingError(fileName);
		return;
	}

	if (lang == Common::SK_SVK)
		file.seek(0xBAA8);
	else if (lang == Common::DE_DEU)
		file.seek(0xBC48);
	else
		return;

	Common::String str;

	file.readPascalString(); // WALK TO
	file.readPascalString(); // TALK TO
	file.readPascalString(); // PICK UP
	file.readPascalString(); // LOOK AT

	str = file.readPascalString();
	if (lang == Common::SK_SVK)
		_strings[WALK] = str;

	str = file.readPascalString();
	if (lang == Common::DE_DEU)
		_strings[WALK] = str;

	str = file.readPascalString();
	if (lang == Common::SK_SVK)
		_strings[LOOK] = str;

	file.readPascalString();
	file.readPascalString();

	str = file.readPascalString();
	if (lang == Common::DE_DEU)
		_strings[LOOK] = str;

	str = file.readPascalString();
	if (lang == Common::SK_SVK)
		_strings[PICKUP] = str;

	str = file.readPascalString();
	if (lang == Common::DE_DEU)
		_strings[PICKUP] = str;

	str = file.readPascalString();
	if (lang == Common::SK_SVK)
		_strings[TALK] = str;

	str = file.readPascalString();
	if (lang == Common::DE_DEU)
		_strings[TALK] = str;

	file.readPascalString(); // USE

	str = file.readPascalString();
	if (lang == Common::SK_SVK)
		_strings[USE] = str;

	str = file.readPascalString();
	if (lang == Common::DE_DEU)
		_strings[USE] = str;


	if (lang == Common::SK_SVK)
		file.seek(0x1982F);
	else if (lang == Common::DE_DEU)
		file.seek(0x199F0);
	else
		return;

	_strings[JOHNNY_CANNOT_USE_1] = file.readPascalString();
	_strings[SKEPTO_CANNOT_USE_1] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_USE_2] = file.readPascalString();
	_strings[SKEPTO_CANNOT_USE_2] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_USE_3] = file.readPascalString();
	_strings[SKEPTO_CANNOT_USE_3] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_USE_4] = file.readPascalString();
	_strings[SKEPTO_CANNOT_USE_4] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_TALK_1] = file.readPascalString();
	_strings[SKEPTO_CANNOT_TALK_1] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_TALK_2] = file.readPascalString();
	_strings[SKEPTO_CANNOT_TALK_2] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_TALK_3] = file.readPascalString();
	_strings[SKEPTO_CANNOT_TALK_3] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_TALK_4] = file.readPascalString();
	_strings[SKEPTO_CANNOT_TALK_4] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_LOOK_1] = file.readPascalString();
	_strings[SKEPTO_CANNOT_LOOK_1] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_LOOK_2] = file.readPascalString();
	_strings[SKEPTO_CANNOT_LOOK_2] = file.readPascalString();
	file.readPascalString();
	if (lang != Common::SK_SVK) // This sentence seems to be missing from the Slovak executable.
		_strings[JOHNNY_CANNOT_LOOK_3] = file.readPascalString();
	_strings[SKEPTO_CANNOT_LOOK_3] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_LOOK_4] = file.readPascalString();
	_strings[SKEPTO_CANNOT_LOOK_4] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_PICKUP_1] = file.readPascalString();
	_strings[SKEPTO_CANNOT_PICKUP_1] = file.readPascalString();
	file.readPascalString();
	if (lang != Common::SK_SVK) // This sentence seems to be missing from the Slovak executable.
		_strings[JOHNNY_CANNOT_PICKUP_2] = file.readPascalString();
	_strings[SKEPTO_CANNOT_PICKUP_2] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_PICKUP_3] = file.readPascalString();
	_strings[SKEPTO_CANNOT_PICKUP_3] = file.readPascalString();
	file.readPascalString();
	_strings[JOHNNY_CANNOT_PICKUP_4] = file.readPascalString();
	_strings[SKEPTO_CANNOT_PICKUP_4] = file.readPascalString();
}

}
