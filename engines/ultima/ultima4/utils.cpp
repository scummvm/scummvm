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

#include "ultima/ultima4/utils.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Seed the random number generator.
 */
void xu4_srandom() {
//    srand((unsigned int)time(NULL));
}

/**
 * Generate a random number between 0 and (upperRange - 1).  This
 * routine uses the upper bits of the random number provided by rand()
 * to compensate for older generators that have low entropy in the
 * lower bits (e.g. MacOS X).
 */
int xu4_random(int upperRange) {
#ifdef TODO
	int r = rand();
#else
	error("TODO");
#endif
}

/**
 * Trims whitespace from a Common::String
 * @param val The Common::String you are trimming
 * @param chars_to_trim A list of characters that will be trimmed
 */
Common::String &trim(Common::String &val, const Common::String &chars_to_trim) {
    using namespace std;
    Common::String::iterator i;
    if (val.size()) {
        Common::String::value_type pos;
        for (i = val.begin(); (i != val.end()) && (pos = chars_to_trim.find(*i)) != Common::String::npos; )
            i = val.erase(i);    
        for (i = val.end()-1; (i != val.begin()) && (pos = chars_to_trim.find(*i)) != Common::String::npos; )
            i = val.erase(i)-1;
    }
    return val;
}

/**
 * Converts the Common::String to lowercase
 */ 
Common::String& lowercase(Common::String &val) {
    using namespace std;
    Common::String::iterator i;
    for (i = val.begin(); i != val.end(); i++)
        *i = tolower(*i);
    return val;
}

/**
 * Converts the Common::String to uppercase
 */ 
Common::String& uppercase(Common::String &val) {
    using namespace std;
    Common::String::iterator i;
    for (i = val.begin(); i != val.end(); i++)
        *i = toupper(*i);
    return val;
}

/**
 * Converts an integer value to a Common::String
 */ 
Common::String xu4_to_string(int val) {
    char buffer[16];    
    sprintf(buffer, "%d", val);
    return buffer;
}

/**
 * Splits a Common::String into substrings, divided by the charactars in
 * separators.  Multiple adjacent seperators are treated as one.
 */
Std::vector<Common::String> split(const Common::String &s, const Common::String &separators) {
    Std::vector<Common::String> result;
    Common::String current;

    for (unsigned i = 0; i < s.size(); i++) {
        if (separators.find(s[i]) != Common::String::npos) {
            if (current.size() > 0)
                result.push_back(current);
            current.clear();
        } else
            current += s[i];
    }

    if (current.size() > 0)
        result.push_back(current);

    return result;
}

Performance::Performance(const Common::String &s) {
#ifndef NPERF
	init(s);
#endif
}

void Performance::init(const Common::String &s) {
#ifndef NPERF
	Path path(s);
	FileSystem::createDirectory(path);
#ifdef TODO
	filename = path.getPath();
	log = g_system->getSavefileManager()->openForSaving(filename);
	if (!log)
		// FIXME: throw exception
		return;
#endif
#endif
}

void Performance::reset() {
#ifdef TODO
#ifndef NPERF
	if (!log) {
		log = g_system->getSavefileManager()->openForSaving(filename);
		if (!log)
			// FIXME: throw exception
			return;
	}
#endif
#endif
}

void Performance::start() {
#ifdef TODO
#ifndef NPERF
	s = g_system->getMillis();
#endif
#endif
}

void  Performance::end(const Common::String &funcName) {
#ifdef TODO
#ifndef NPERF
	e = g_system->getMillis();
	times[funcName] = e - s;
#endif
#endif
}

void  Performance::report(const char *pre) {
#ifdef TODO
#ifndef NPERF
	static const double msec = double(CLOCKS_PER_SEC) / double(1000);
	TimeMap::const_iterator i;
	uint32 total = 0;
	Common::HashMap<double, Common::String> percentages;
	Common::HashMap<double, Common::String>::iterator perc;

	if (pre)
		log->write(pre, strlen(pre));

	for (i = times.begin(); i != times.end(); i++) {
		Common::String str = Common::String::format("%s [%0.2f msecs]\n", i->_key.c_str(), double(i->_value) / msec);
		log->write(str.c_str(), str.size());
		total += i->_value;
	}

	for (i = times.begin(); i != times.end(); i++) {
		double perc = 100.0 * double(i->_value) / total;
		percentages[perc] = i->_key;
	}

	log->writeByte('\n');
	for (perc = percentages.begin(); perc != percentages.end(); perc++) {
		Common::String str = Common::String::format("%0.1f%% - %s\n", perc->_key, perc->_value.c_str());
		log->write(str.c_str(), str.size());
	}

	Common::String final = Common::String::format("\nTotal [%0.2f msecs]\n", double(total) / msec);
	log->write(final.c_str(), final.size());

	log->finalize();
	delete log;
	log = NULL;
	times.clear();
#endif
#endif
}

} // End of namespace Ultima4
} // End of namespace Ultima
