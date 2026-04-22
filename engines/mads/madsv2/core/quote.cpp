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

#include "common/stream.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/sort.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {

int quote_emergency = false;

char *quote_load(int quote_id, ...) {
	va_list marker;
	int quote_error = 0;
	int id;
	int mark;
	int now_reading = 0;
	int now_finding = 0;
	word list[QUOTE_MAX_LIST_LENGTH];
	char work[QUOTE_MAX_LIST_LENGTH];
	char *result = NULL;
	char *buffer = NULL;
	char *pointer;
	long total_mem_avail;
	long total_mem_needed = 1;
	long mem_needed;
	Common::SeekableReadStream *handle = NULL;

	mem_save_free();

	fileio_suppress_unbuffering = true;

	handle = env_open("*quotes.dat", "rb");
	quote_error = 1;
	if (handle == NULL) goto done;

	mark = 0;
	va_start(marker, quote_id);
	for (id = quote_id; id > 0; id = va_arg(marker, int)) {
		list[mark++] = id;
		if (mark > QUOTE_MAX_LIST_LENGTH) {
			quote_error = 10;
			goto done;
		}
	}

	sort_insertion_16(mark, (byte *)work, list);

	total_mem_avail = 65536;
	buffer = pointer = (char *)mem_get_name(total_mem_avail - 64, "$quotes$");
	if (buffer == NULL) {
		quote_error = 2;
		goto done;
	}

	now_reading = 1;
	while (now_finding < mark) {
		Common::String quoteStr = handle->readString();

		if (now_reading == list[now_finding]) {
			mem_needed = quoteStr.size() + 3;
			total_mem_needed += mem_needed;
			if (total_mem_needed > total_mem_avail) {
				quote_error = 4;
				goto done;
			}

			Common::strcpy_s(pointer, QUOTE_MAX_LIST_LENGTH, quoteStr.c_str());
			pointer += (mem_needed - 2);
			*(uint16 *)(pointer) = list[now_finding];
			pointer += 2;
			now_finding++;

		} else if (now_reading > list[now_finding]) {
			error_report(ERROR_QUOTE_DUPLICATE_LOAD, WARNING, MODULE_QUOTE, quote_error, list[now_finding]);
		}

		now_reading++;
	}

	if (mem_adjust(buffer, total_mem_needed)) {
		quote_error = 6;
		goto done;
	}

	*pointer = 0;
	result = buffer;

done:
	if (result != buffer)
		mem_free(buffer);
	delete handle;

	if (result == NULL) {
		error_report(ERROR_QUOTE_LOAD_FAILED, WARNING, MODULE_QUOTE, now_finding, list[now_finding]);
	}

	fileio_suppress_unbuffering = false;

	mem_restore_free();
	return result;
}

char *quote_string(char *quote_list, int quote_id) {
	int id;
	char *result = NULL;
	char *search;
	char *marker;

	for (marker = quote_list; *marker && (result == NULL); marker = search + 2) {
		for (search = marker; *search; search++);
		search++;
		id = *((uint16 *)search);
		if (id == quote_id) result = marker;
	}

	return result;
}

void quote_split_string(const char *source, char *target1, char *target2) {
	int count, len, half;
	const char *mark;

	len = strlen(source);

	mark = source;
	half = len >> 1;

	for (count = 0; count < half; count++) {
		*(target1++) = *(mark++);
	}

	while (*mark && (*mark != ' ')) {
		*(target1++) = *(mark++);
	}

	while (*mark == ' ') mark++;

	// (target1++) = '"';
	*(target1++) = 0;

	do {
		*(target2++) = *mark;
	} while (*(mark++));
}

} // namespace MADSV2
} // namespace MADS
