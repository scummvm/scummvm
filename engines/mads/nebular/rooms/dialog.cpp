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

#include "mads/core/global.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/quote.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

void Dialog::setup(int globalId, ...) {
	va_list va;
	va_start(va, globalId);

	// Load the list of conversation quotes
	_count = 0;

	int quoteId = va_arg(va, int);
	while (quoteId > 0) {
		_quotes[_count++] = quoteId;
		quoteId = va_arg(va, int);
	}
	va_end(va);
	assert(_count <= MAX_CONV_MESSAGE);

	if (quoteId < 0) {
		// For an ending value of -1, also initial the bitflags for the global
		// associated with the conversation entry, which enables all the quote Ids
		global[globalId] = -1;
	}

	_globalId = globalId;
}

void Dialog::set(int quoteId, ...) {
	global[_globalId] = 0;

	va_list va;
	va_start(va, quoteId);

	// Loop through handling each quote
	while (quoteId > 0) {
		for (int idx = 0; idx < _count; ++idx) {
			if (_quotes[idx] == quoteId) {
				// Found index, so set that bit in the global keeping track of conversation state
				global[_globalId] |= 1 << idx;
				break;
			}
		}

		quoteId = va_arg(va, int);
	}

	va_end(va);
}

int Dialog::read(int quoteId) {
	uint16 flags = global[_globalId];
	int count = 0;

	for (int idx = 0; idx < _count; ++idx) {
		if (flags & (1 << idx))
			++count;

		if (_quotes[idx] == quoteId)
			return flags & (1 << idx);
	}

	// Could not find it, simply return number of active quotes
	return count;
}

void Dialog::write(int quoteId, bool flag) {
	for (int idx = 0; idx < _count; ++idx) {
		if (_quotes[idx] == quoteId) {
			// Found index, so set or clear the flag
			if (flag) {
				// Set bit
				global[_globalId] |= 1 << idx;
			} else {
				// Clear bit
				global[_globalId] &= ~(1 << idx);
			}
			return;
		}
	}
}

void Dialog::start() {
	inter_reset_dialog();

	// Loop through each of the quotes loaded into the conversation
	for (int idx = 0; idx < _count; ++idx) {
		// Check whether the given quote is enabled or not
		if (global[_globalId] & (1 << idx)) {
			// Quote enabled, so add it to the list of talk selections
			inter_add_dialog(quote_string(kernel.quotes, _quotes[idx]), _quotes[idx]);
		}
	}

	kernel_set_interface_mode(INTER_CONVERSATION);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
