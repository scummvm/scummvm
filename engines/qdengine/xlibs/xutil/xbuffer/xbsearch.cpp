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

#include "qdengine/xlibs/xutil/xglobal.h"


namespace QDEngine {

bool XBuffer::search(char* what, int mode, int cs) {
	int i, j;
	unsigned int wlen = strlen(what) - 1;
	if (mode == XB_GLOBAL) {
		i = 0;
		mode = XB_FORWARD;
	} else i = offset_;
	if (mode == XB_FORWARD) {
		if (cs == XB_CASEON) {
			while (buffer_[i]) {
				if (buffer_[i] == *what) {
					j = wlen;
					while (buffer_[i + j] == what[j] && j) j--;
					if (j <= 0) {
						offset_ = i;
						return 1;
					};
				}
				i++;
			}
		} else {
			while (buffer_[i]) {
				if (toupper(buffer_[i]) == toupper(*what)) {
					j = wlen;
					while (toupper(buffer_[i + j]) == toupper(what[j]) && j) j--;
					if (j <= 0) {
						offset_ = i;
						return 1;
					};
				}
				i++;
			}
		}
	} else {
		i -= wlen + 1;
		if (cs == XB_CASEON) {
			while (i >= 0) {
				if (buffer_[i] == *what) {
					j = wlen;
					while (buffer_[i + j] == what[j] && j) j--;
					if (j <= 0) {
						offset_ = i;
						return 1;
					};
				}
				i--;
			}
		} else {
			while (i >= 0) {
				if (toupper(buffer_[i]) == toupper(*what)) {
					j = wlen;
					while (toupper(buffer_[i + j]) == toupper(what[j]) && j) j--;
					if (j <= 0) {
						offset_ = i;
						return 1;
					};
				}
				i--;
			}
		}
	}
	return 0;
}
} // namespace QDEngine
