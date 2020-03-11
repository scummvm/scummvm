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

#ifndef ULTIMA4_UTILS_H
#define ULTIMA4_UTILS_H

#include "ultima/ultima4/filesystem.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/shared/std/containers.h"
#include "common/savefile.h"
#include "common/hash-str.h"

namespace Ultima {
namespace Ultima4 {

/* The AdjustValue functions used to be #define'd macros, but these are 
 * evil for several reasons, *especially* when they contain multiple
 * statements, and have if statements in them. The macros did both.
 * See http://www.parashift.com/c++-faq-lite/inline-functions.html#faq-9.5
 * for more information. 
 */
inline void AdjustValueMax(int &v, int val, int max) { v += val; if (v > max) v = max; }
inline void AdjustValueMin(int &v, int val, int min) { v += val; if (v < min) v = min; }
inline void AdjustValue(int &v, int val, int max, int min) { v += val; if (v > max) v = max; if (v < min) v = min; }

inline void AdjustValueMax(short &v, int val, int max) { v += val; if (v > max) v = max; }
inline void AdjustValueMin(short &v, int val, int min) { v += val; if (v < min) v = min; }
inline void AdjustValue(short &v, int val, int max, int min) { v += val; if (v > max) v = max; if (v < min) v = min; }

inline void AdjustValueMax(unsigned short &v, int val, int max) { v += val; if (v > max) v = max; }
inline void AdjustValueMin(unsigned short &v, int val, int min) { v += val; if (v < min) v = min; }
inline void AdjustValue(unsigned short &v, int val, int max, int min) { v += val; if (v > max) v = max; if (v < min) v = min; }

void xu4_srandom(void);
int xu4_random(int upperval);
Common::String &trim(Common::String &val, const Common::String &chars_to_trim = "\t\013\014 \n\r");
Common::String &lowercase(Common::String &val);
Common::String &uppercase(Common::String &val);
Common::String xu4_to_string(int val);
Std::vector<Common::String> split(const Common::String &s, const Common::String &separators);

class Performance {
    typedef Common::HashMap<Common::String, uint32> TimeMap;
public:
	Performance(const Common::String &s);

	void init(const Common::String &s);

	void reset();

	void start();

	void end(const Common::String &funcName);

	void report(const char *pre = NULL);

private:
    Common::OutSaveFile *log;
    Common::String filename;
    uint32 s, e;
    TimeMap times;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
