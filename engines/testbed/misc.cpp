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
 * $URL$
 * $Id$
 */

#include "testbed/misc.h"

namespace Testbed {

void MiscTests::getHumanReadableFormat(TimeDate &td, Common::String &date) {
	// XXX: can use snprintf?
	char strDate[100];
	snprintf(strDate, 100, "%d:%d:%d on %d/%d/%d (dd/mm/yy)", td.tm_hour, td.tm_min, td.tm_sec, td.tm_mday, td.tm_mon, td.tm_year);
	date = strDate;
	return;
}

bool MiscTests::testDateTime() {
	TimeDate t1, t2;
	g_system->getTimeAndDate(t1);
	printf("LOG: Current Time and Date: ");
	Common::String dateTimeNow;
	getHumanReadableFormat(t1, dateTimeNow);
	printf("%s\n", dateTimeNow.c_str());

	if (Testsuite::isInteractive) {
		// Directly verify date
		dateTimeNow = "We expect the current date time to be " + dateTimeNow;
		Testsuite::clearScreen();
		if (Testsuite::handleInteractiveInput(dateTimeNow, "Correct!", "Wrong", kOptionRight)) {
			return false;
		}
	}

	// Now, Put some delay
	g_system->delayMillis(2000);
	g_system->getTimeAndDate(t2);
	printf("LOG: Time and Date 2s later: ");
	getHumanReadableFormat(t2, dateTimeNow);
	printf("%s\n", dateTimeNow.c_str());
	
	if (t1.tm_year == t2.tm_year && t1.tm_mon == t2.tm_mon && t1.tm_mday == t2.tm_mday) {
		if (t1.tm_mon == t2.tm_mon && t1.tm_year == t2.tm_year){
			// Ignore lag due to processing time
			if (t1.tm_sec + 2 == t2.tm_sec) {
				return true;
			}
		}
	}
	return false;
}

bool MiscTests::testTimers() {
	return true;
}

bool MiscTests::testMutexes() {
	return true;
}

MiscTestSuite::MiscTestSuite() {
	addTest("Date/time", &MiscTests::testDateTime);	
	addTest("Timers", &MiscTests::testTimers);	
	addTest("Mutexes", &MiscTests::testMutexes);	
}
const char *MiscTestSuite::getName() const {
	return "Misc. Tests: Datetime/Timer/Mutextes";
}

} // End of namespace Testbed
