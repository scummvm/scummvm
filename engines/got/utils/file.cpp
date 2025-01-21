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
#include "got/utils/file.h"
#include "got/vars.h"

namespace Got {

bool File::open(const Common::Path &filename) {
	if (!Common::File::open(filename))
		error("Could not open - %s", filename.baseName().c_str());
	return true;
}

bool loadActor(int /*file*/, int num) {
	Common::String fname = Common::String::format("ACTOR%d", num);

	if (resourceRead(fname, _G(tmp_buff), true) < 0)
		return false;

	return true;
}

bool loadSpeech(int index) {
	char tmps[5];

	Common::String fname = Common::String::format("SPEAK%d", _G(area));
	char *sp = new char[30000];

	if (resourceRead(fname, sp) < 0) {
		delete[] sp;
		return false;
	}

	char *p = sp;

	int cnt = 0;
	for (;;) {
		if (*p == ':') {
			p++;
			cnt++;
			strncpy(tmps, p, 4);
			tmps[4] = '\0';

			if (atoi(tmps) == index) {
				break;
			}
		}

		p++;
		cnt++;
	}

	while (*p != 10)
		p++;
	p++;

	char *pm = p;
	cnt = 0;

	for (;;) {
		if (*p == 13)
			*p = 32;
		if (*p == ':') {
			if ((*(p + 1) == 'E') && (*(p + 2) == 'N') && (*(p + 3) == 'D'))
				break;
		}

		p++;
		cnt++;

		if (cnt > 5799) {
			delete[] sp;
			return false;
		}
	}

	if (*(p - 1) == 10)
		*(p - 1) = 0;
	*p = 0;

	Common::copy(pm, pm + cnt, _G(tmp_buff));
	_G(tmp_buff)
	[cnt] = 0;

	delete[] sp;
	return true;
}

long resourceRead(const Common::String &name, void *buff, bool failAllowed) {
	Common::File f;
	if (f.open(Common::Path(name))) {
		return f.read(buff, f.size());
	}

	if (!failAllowed)
		error("Could not load - %s", name.c_str());

	return -1;
}

void *resourceAllocRead(const Common::String &name) {
	Common::File f;
	if (f.open(Common::Path(name))) {
		byte *result = (byte *)malloc(f.size());
		f.read(result, f.size());
		return result;
	}

	return nullptr;
}

} // End of namespace Got
