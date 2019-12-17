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

#include "ultima/shared/std/string.h"

#ifdef MACOS
#include <stat.h>
#elif !defined(UNDER_CE)
#include <sys/stat.h>
#endif

#include "Utils.h"

namespace Ultima {
namespace Ultima6 {

using std::string;

static bool base_to_uppercase(string &str, int count);

/*
 *  Convert just the last 'count' parts of a filename to uppercase.
 *  returns false if there are less than 'count' parts
 */

static bool base_to_uppercase(string &str, int count) {
	if (count <= 0) return true;

	int todo = count;
	// Go backwards.
	string::reverse_iterator X;
	for (X = str.rbegin(); X != str.rend(); ++X) {
		// Stop at separator.
		if (*X == '/' || *X == '\\' || *X == ':')
			todo--;
		if (todo <= 0)
			break;

#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__) || defined(_MSC_VER))
		if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#else
		*X = static_cast<char>(std::toupper(*X));
#endif
	}
	if (X == str.rend())
		todo--; // start of pathname counts as separator too

	// false if it didn't reach 'count' parts
	return (todo <= 0);
}


/*
 *  Open a file for input,
 *  trying the original name (lower case), and the upper case version
 *  of the name.
 *
 *  Output: 0 if couldn't open.
 */

bool openFile
(
    std::ifstream &in,          // Input stream to open.
    const char *fname           // May be converted to upper-case.
) {
#if defined(MACOS) || (__GNUG__ > 2)
	std::ios_base::openmode mode = std::ios::in;
#else
	int mode = std::ios::in;
#endif
	string name = fname;
	int uppercasecount = 0;
	do {
		// We first "clear" the stream object. This is done to prevent
		// problems when re-using stream objects
		in.clear();
		try {
			//std::cout << "trying: " << name << std::endl;
			in.open(name.c_str(), mode);        // Try to open
		} catch (std::exception &) {
		}
		if (in.good() && !in.fail()) {
			//std::cout << "got it!" << std::endl;
			return true; // found it!
		}
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found.
	throw (file_open_exception(fname));
	return false;
}

/*
 *  See if a file exists.
 */

bool fileExists
(
    const char *fname         // May be converted to upper-case.
) {
	string name = fname;

#ifdef UNDER_CE // This is a bit of a hack for WinCE
	const char *n = name.c_str();
	int nLen = std::strlen(n) + 1;
	LPTSTR lpszT = (LPTSTR) alloca(nLen * 2);
	MultiByteToWideChar(CP_ACP, 0, n, -1, lpszT, nLen);
	return GetFileAttributes(lpszT) != 0xFFFFFFFF;
#else

	bool    exists;
	struct stat sbuf;

	int uppercasecount = 0;
	do {
		exists = (stat(name.c_str(), &sbuf) == 0);
		if (exists)
			return true; // found it!
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found
	return false;
#endif
}

} // End of namespace Ultima6
} // End of namespace Ultima
