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
#include "common/timer.h"

namespace Testbed {

void MiscTests::getHumanReadableFormat(TimeDate &td, Common::String &date) {
	// XXX: can use snprintf?
	char strDate[100];
	snprintf(strDate, 100, "%d:%d:%d on %d/%d/%d (dd/mm/yy)", td.tm_hour, td.tm_min, td.tm_sec, td.tm_mday, td.tm_mon, td.tm_year);
	date = strDate;
	return;
}

void MiscTests::timerCallback(void *arg) {
	// Increment arg which actually points to an int
	// arg must point to a static data, threads otherwise have their own stack
	int &valToModify = *((int *) arg);
	valToModify = 999; // some arbitrary value
}

void MiscTests::criticalSection(void *arg) {
	SharedVars &sv = *((SharedVars *) arg);

	Testsuite::logDetailedPrintf("Before critical section: %d %d\n", sv.first, sv.second);
	g_system->lockMutex(sv.mutex);

	// In any case, the two vars must be equal at entry, if mutex works fine.
	// verify this here.
	if (sv.first != sv.second) {
		sv.resultSoFar = false;
	}

	sv.first++;
	g_system->delayMillis(1000);
	// This should bring no change as well in the difference between vars
	// verify this too.
	if (sv.second + 1 != sv.first) {
		sv.resultSoFar = false;
	}

	sv.second *= sv.first;
	Testsuite::logDetailedPrintf("After critical section: %d %d\n", sv.first, sv.second);
	g_system->unlockMutex(sv.mutex);

	g_system->getTimerManager()->removeTimerProc(criticalSection);
}

bool MiscTests::testDateTime() {
	TimeDate t1, t2;
	g_system->getTimeAndDate(t1);
	Testsuite::logDetailedPrintf("Current Time and Date: ");
	Common::String dateTimeNow;
	getHumanReadableFormat(t1, dateTimeNow);
	Testsuite::logDetailedPrintf("%s\n", dateTimeNow.c_str());

	if (Testsuite::isSessionInteractive) {
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
	Testsuite::logDetailedPrintf("Time and Date 2s later: ");
	getHumanReadableFormat(t2, dateTimeNow);
	Testsuite::logDetailedPrintf("%s\n", dateTimeNow.c_str());

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
	static int valToModify = 0;
	if (g_system->getTimerManager()->installTimerProc(timerCallback, 100000, &valToModify)) {
		g_system->delayMillis(150);
		g_system->getTimerManager()->removeTimerProc(timerCallback);

		if (999 == valToModify) {
			return true;
		}
	}
	return false;
}

bool MiscTests::testMutexes() {
	static SharedVars sv = {1, 1, true, g_system->createMutex()};

	if (g_system->getTimerManager()->installTimerProc(criticalSection, 100000, &sv)) {
		g_system->delayMillis(150);
	}

	g_system->lockMutex(sv.mutex);
	sv.first++;
	g_system->delayMillis(1000);
	sv.second *= sv.first;
	g_system->unlockMutex(sv.mutex);
	// wait till timed process exits
	g_system->delayMillis(3000);

	Testsuite::logDetailedPrintf("Final Value: %d %d\n", sv.first, sv.second);
	g_system->deleteMutex(sv.mutex);

	if (sv.resultSoFar && 6 == sv.second) {
		return true;
	}

	return false;
}

MiscTestSuite::MiscTestSuite() {
	addTest("Date/time", &MiscTests::testDateTime, false);
	addTest("Timers", &MiscTests::testTimers, false);
	addTest("Mutexes", &MiscTests::testMutexes, false);
}
const char *MiscTestSuite::getName() const {
	return "Misc";
}

} // End of namespace Testbed
