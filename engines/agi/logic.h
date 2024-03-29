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

#ifndef AGI_LOGIC_H
#define AGI_LOGIC_H

namespace Agi {

/**
 * AGI logic resource structure.
 */
struct AgiLogic {
	uint8 *data;        /**< raw resource data */
	int size;           /**< size of data (excluding message section) */
	int sIP;            /**< saved IP */
	int cIP;            /**< current IP */
	int numTexts;       /**< number of messages */
	const char **texts; /**< message list */

	void reset() {
		data = nullptr;
		size = 0;
		sIP = 0;
		cIP = 0;
		numTexts = 0;
		texts = nullptr;
	}

	AgiLogic() { reset(); }
};

} // End of namespace Agi

#endif /* AGI_LOGIC_H */
