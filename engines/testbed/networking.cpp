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

#include "testbed/networking.h"

namespace Testbed {

NetworkingTestSuite::NetworkingTestSuite() {
	addTest("IsConnectionLimited", Networkingtests::testConnectionLimit, true);
}

TestExitStatus Networkingtests::testConnectionLimit() {
	if (ConfParams.isSessionInteractive()) {
		if (Testsuite::handleInteractiveInput("Testing the IsConnectionLimited API implementation", "Continue", "Skip", kOptionRight)) {
			Testsuite::logPrintf("Info! IsConnectionLimited test skipped by the user.\n");
			return kTestSkipped;
		}

		bool isLimited = g_system->isConnectionLimited();

		if (ConfParams.isSessionInteractive()) {
			Common::String status = "We expect the internet connection to be ";
			status += (isLimited ? "limited." : "unlimited.");
			if (Testsuite::handleInteractiveInput(status, "Correct!", "Wrong", kOptionRight)) {
				Testsuite::logDetailedPrintf("Error! isConnectionLimited failed\n");
				return kTestFailed;
			}
		}

		Testsuite::logDetailedPrintf("isConnectionLimited worked\n");
		return kTestPassed;
	}
	return TestExitStatus();
}

} // namespace Testbed
