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

#include "common/file.h"
#include "common/str.h"
#include "m4/adv_db_r/db_catalog.h"
#include "m4/adv_db_r/database.h"
#include "m4/core/cstring.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/vars.h"

namespace M4 {

static void sort_catalog();
static char *catalog_search(const char *name);
static char *db_get_catalog_entry(char *c, short *tag, short *room, char *name, char *path, short *c_size);
static int compare_catalog_entries_for_search(const void *n1, const void *n2);

bool read_catalog() {
	Common::File f;
	size_t size = 0;
	Common::Path myFilename("roomsdb.chk");

	if (!f.open(myFilename))
		return false;

	if (_G(catalog)._data)
		mem_free(_G(catalog)._data);

	size = f.size();
	_G(catalog)._data = (char *)mem_alloc(size + 4, "catalog");

	if (f.read(_G(catalog)._data, size) != size)
		error_show(FL, 'FRDF', "Could not read in the catalog.");

	f.close();

	sort_catalog();
	return true;
}

char *db_get_catalog() {
	return _G(catalog)._data;
}

char *db_rmlst_get_asset_room_path(const char *asset_name, char *result, int32 *sceneCode) {
	char name[MAX_FILENAME_SIZE] = { 0 }, path[MAX_FILENAME_SIZE] = { 0 };
	short tag = 0, room = 0;
	short c_size = 0;
	long str_len;

	result[0] = '\0';

	char *search_result = catalog_search(asset_name);

	if (search_result != nullptr) {
		db_get_catalog_entry(search_result, &tag, &room, name, path, &c_size);
		*sceneCode = room;
	}

	if (!scumm_strnicmp(asset_name, name, MAX_FILENAME_SIZE)) {
		if (path[0] == '\\')
			cstrncpy(result, &path[1], MAX_FILENAME_SIZE);
		else
			cstrncpy(result, path, MAX_FILENAME_SIZE);

		str_len = strlen(result);

		if (str_len) {
			if (result[str_len - 1] == '\\')
				result[str_len - 1] = '\0';
		}
	}

	return result;
}

static int32 count_catalog_items() {
	char *pCatIter = db_get_catalog();
	int32 my_catalog_size = 0;

	while (pCatIter) {
		pCatIter = db_get_catalog_entry(pCatIter, nullptr, nullptr, nullptr, nullptr, nullptr);
		if (pCatIter)
			++my_catalog_size;
	}

	return my_catalog_size;
}

static int compare_catalog_entries_for_sort(const void *n1, const void *n2) {
	const char *entry1 = *(const char * const *)n1;
	if (convert_intel16(*(const short *)&entry1[2]) == _T_ROOM)
		entry1 = &entry1[6];
	else
		entry1 = &entry1[4];

	const char *entry2 = *(const char * const *)n2;
	if (convert_intel16(*(const short *)&entry2[2]) == _T_ROOM)
		entry2 = &entry2[6];
	else
		entry2 = &entry2[4];

	return (scumm_strnicmp(entry1, entry2, 80));
}

static void sort_catalog() {
	char name[MAX_FILENAME_SIZE], path[MAX_FILENAME_SIZE];
	short tag;
	long i;

	_G(catalog)._size = count_catalog_items();
	_G(catalog)._catalog = (char **)calloc(_G(catalog)._size + 4, sizeof(char *));

	char *pCatalogIter = _G(catalog)._data;
	int32 index = 0;
	while (pCatalogIter) {
		_G(catalog)._catalog[index] = pCatalogIter;

		short room_number;
		char *nextItem = db_get_catalog_entry(pCatalogIter, &tag, &room_number, name, path, nullptr);

		switch (tag) {
		case _T_ROOM:
			snprintf(&pCatalogIter[(3 * sizeof(uint16))], 256, "%03d!", room_number);

			i = (3 * sizeof(uint16)) + (5 * sizeof(char));

			for (;;) {
				if (!pCatalogIter[i])
					break;
				pCatalogIter[i++] = '\0';
			}
			break;

		default:
			cstrupr(pCatalogIter + (2 * sizeof(uint16)));
			break;
		}

		pCatalogIter = nextItem;

		if (pCatalogIter)
			++index;

		if (index > _G(catalog)._size)
			error_show(FL, 'OOM!');
	}

	qsort(_G(catalog)._catalog, _G(catalog)._size, sizeof(char *), compare_catalog_entries_for_sort);
}

static char *catalog_search(const char *name) {
	char myString[80];
	cstrcpy(myString, name);
	cstrupr(myString);

	char **result = (char **)bsearch((void *)myString, (void *)_G(catalog)._catalog, _G(catalog)._size, sizeof(char *), compare_catalog_entries_for_search);
	if (!result)
		return nullptr;

	return *result;
}

static char *db_get_catalog_entry(char *c, short *tag, short *room, char *name, char *path, short *c_size) {
	int i;
	char s[MAX_FILENAME_SIZE];

	if (tag)
		*tag = 0;
	if (room)
		*room = 0;
	if (name)
		*name = '\0';
	if (path)
		*path = '\0';
	if (c_size)
		*c_size = 0;

	short size = convert_intel16(*(uint16 *)&c[0]);

	if (!size)
		return nullptr;

	if (c_size)
		*c_size = size;

	int32 x = sizeof(uint16);

	int16 theTag = convert_intel16(*(uint16 *)&c[x]);
	if (tag)
		*tag = theTag;

	x += sizeof(uint16); // tag

	if ((theTag == _T_ROOM) && room) {
		*room = convert_intel16(*(uint16 *)&c[x]);
		x += sizeof(uint16);
	}

	for (i = 0; c[x]; x++) {
		s[i++] = c[x];

		if (i == MAX_FILENAME_SIZE)
			error_show(FL, 'OOM!');
	}
	s[i] = '\0';

	if (name)
		snprintf(name, MAX_FILENAME_SIZE, "%s", s);

	for (;;) {
		if (c[x])
			break;
		x++;
	}

	switch (theTag) {
	case _T_DEPTH:
	case _T_SPECIAL:
	case _T_WALK:
		break;

	default:
		for (i = 0; c[x]; x++) {
			s[i++] = c[x];
		}
		s[i] = '\0';
		if (path)
			snprintf(path, MAX_FILENAME_SIZE, "%s", s);
		break;
	}

	if ((theTag != _T_ROOM) && room)
		*room = convert_intel16(*(uint16 *)&c[size - (sizeof(uint16))]);

	return &c[size];
}

static int compare_catalog_entries_for_search(const void *n1, const void *n2) {
	const char *cat_entry = *(const char * const *)n2;

	if (convert_intel16(*(const short *)&cat_entry[2]) == _T_ROOM)
		cat_entry = &cat_entry[6];
	else
		cat_entry = &cat_entry[4];

	return (scumm_strnicmp((const char *)n1, cat_entry, 80));
}

} // End of namespace M4
