/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRIM_MD5CHECK_H
#define GRIM_MD5CHECK_H

#include "common/array.h"

namespace Grim {

class MD5Check {
public:
	static bool checkFiles();
	static void startCheckFiles();
	static bool advanceCheck(int *pos, int *total);
	inline static bool advanceCheck() { return advanceCheck(NULL, NULL); }
	static void clear();

private:
	static void init();

	struct MD5Sum {
		MD5Sum(const char *fn, const char **s, int n) : filename(fn), sums(s), numSums(n) {}
		const char *filename;
		const char **sums;
		int numSums;
	};
	static bool checkMD5(const MD5Sum &sums, const char *md5);

	static bool _initted;
	static Common::Array<MD5Sum> *_files;
	static int _iterator;
};

}

#endif
