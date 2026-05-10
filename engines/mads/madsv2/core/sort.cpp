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

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

void sort_insertion(int elements, int *id, long *value) {
	int going, count, count2, my_id;
	int insertion, deletion;
	long my_value;

	going = true;

	while (going) {
		going = false;
		for (count = 0; (count < (elements - 1)) && !going; count++) {
			if (value[count] > value[count + 1]) {
				deletion = elements - (count + 1);
				my_value = value[count];
				my_id = id[count];
				if (deletion > 0) {
					memmove(&value[count], &value[count + 1], deletion * sizeof(long));
					memmove(&id[count], &id[count + 1], deletion * sizeof(int));
				}
				for (count2 = 0; (count2 < elements - 1) && !going; count2++) {
					going = (my_value < value[count2]);
				}

				// After the loop, count2 has been post-incremented one step past
				// the found insertion position.  If going was set to true inside
				// the loop (i.e. we found a value[count2-1] > my_value), the for
				// loop's count2++ still fired, so the real target index is
				// count2-1.  Decrement to correct for this.
				if (going) {
					count2--;
				}
				going = true;
				insertion = (elements - 1) - count2;
				if (insertion > 0) {
					memmove(&value[count2 + 1], &value[count2], insertion * sizeof(long));
					memmove(&id[count2 + 1], &id[count2], insertion * sizeof(int));
				}
				value[count2] = my_value;
				id[count2] = my_id;
			}
		}
	}
}

void sort_insertion_16(int elements, byte *id, word *value) {
	int  restart_mark = 0;
	word my_value;
	byte my_id;

	while (1) {  // restart_sort
		int si = restart_mark;
		int limit = elements - 1;

		// search_loop: scan forward from restart_mark for an out-of-order pair
		while (si < limit) {
			if (value[si + 1] < value[si])
				break;  // found_anomaly

			si++;
			restart_mark = si;
		}

		if (si >= limit)
			break;  // sort_done

		// found_anomaly: save the out-of-order element at si+1
		my_value = value[si + 1];
		my_id = id[si + 1];

		// Delete element at si+1 by shifting everything above it down one.
		// Value array uses word (2-byte) elements; id array uses bytes.
		int deletion = limit - si - 1;
		if (deletion > 0) {
			memmove(&value[si + 1], &value[si + 2], deletion * sizeof(word));
			memmove(&id[si + 1], &id[si + 2], deletion * sizeof(byte));
		}

		// quest_loop: find correct insertion point for my_value in [0..limit-1]
		int count2 = 0;
		while (count2 < limit) {
			if (my_value <= value[count2])
				break;  // found_spot
			count2++;
		}

		// found_spot: shift right from count2 to make room
		int insertion = limit - count2;
		if (insertion > 0) {
			memmove(&value[count2 + 1], &value[count2], insertion * sizeof(word));
			memmove(&id[count2 + 1], &id[count2], insertion * sizeof(byte));
		}

		// Insert saved element at its correct position
		value[count2] = my_value;
		id[count2] = my_id;
	}
}

void sort_insertion_8(int elements, byte *id, byte *value) {
	int  restart_mark = 0;
	byte my_value;
	byte my_id;

	while (1) {  // restart_sort
		int si = restart_mark;
		int limit = elements - 1;

		// search_loop: scan forward from restart_mark for an out-of-order pair
		while (si < limit) {
			if (value[si + 1] < value[si])
				break;  // found_anomaly

			si++;
			restart_mark = si;
		}

		if (si >= limit)
			break;  // sort_done

		// found_anomaly: save the out-of-order element at si+1
		my_value = value[si + 1];
		my_id = id[si + 1];

		// Delete element at si+1 by shifting everything above it down one
		int deletion = limit - si - 1;
		if (deletion > 0) {
			memmove(&value[si + 1], &value[si + 2], deletion * sizeof(byte));
			memmove(&id[si + 1], &id[si + 2], deletion * sizeof(byte));
		}

		// quest_loop: find correct insertion point for my_value in [0..limit-1]
		int count2 = 0;
		while (count2 < limit) {
			if (my_value <= value[count2])
				break;  // found_spot
			count2++;
		}

		// found_spot: shift right from count2 to make room
		int insertion = limit - count2;
		if (insertion > 0) {
			// shift backwards (std / rep movsb in original)
			memmove(&value[count2 + 1], &value[count2], insertion * sizeof(byte));
			memmove(&id[count2 + 1], &id[count2], insertion * sizeof(byte));
		}

		// Insert saved element at its correct position
		value[count2] = my_value;
		id[count2] = my_id;

		// restart_sort: loop back and try again from restart_mark
	}
}

} // namespace MADSV2
} // namespace MADS
