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

#include "testbed/networking.h"

#ifdef USE_BASIC_NET
#include "backends/networking/basic/socket.h"
#include "backends/networking/basic/url.h"
#endif

namespace Testbed {

NetworkingTestSuite::NetworkingTestSuite() {
	addTest("IsConnectionLimited", Networkingtests::testConnectionLimit, true);
#ifdef USE_BASIC_NET
	addTest("URL", Networkingtests::testURL, true);
	addTest("Socket", Networkingtests::testSocket, true);
#endif
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

#ifdef USE_BASIC_NET
TestExitStatus Networkingtests::testURL() {
	if (ConfParams.isSessionInteractive()) {
		if (Testsuite::handleInteractiveInput("Testing the URL API implementation", "Continue", "Skip", kOptionRight)) {
			Testsuite::logPrintf("Info! URL test skipped by the user.\n");
			return kTestSkipped;
		}

		Common::String lobbyUrl = "https://multiplayer.scummvm.org:9130";
		Networking::URL *url = Networking::URL::parseURL(lobbyUrl);
		if (!url) {
			Testsuite::logDetailedPrintf("Error! URL parsing failed\n");
			return kTestFailed;
		}

		if (url->getScheme() != "https") {
			Testsuite::logDetailedPrintf("Error! URL scheme was unexpected\n");
			return kTestFailed;
		}

		if (url->getHost() != "multiplayer.scummvm.org") {
			Testsuite::logDetailedPrintf("Error! URL host was unexpected\n");
			return kTestFailed;
		}

		if (url->getPort() != 9130) {
			Testsuite::logDetailedPrintf("Error! URL port was unexpected\n");
			return kTestFailed;
		}

		Testsuite::logDetailedPrintf("URL test worked\n");
		return kTestPassed;
	}
	return TestExitStatus();
}

TestExitStatus Networkingtests::testSocket() {
	if (ConfParams.isSessionInteractive()) {
		if (Testsuite::handleInteractiveInput("Testing the Socket API implementation", "Continue", "Skip", kOptionRight)) {
			Testsuite::logPrintf("Info! Socket test skipped by the user.\n");
			return kTestSkipped;
		}

		Common::String lobbyUrl = "https://multiplayer.scummvm.org:9130";
		Networking::Socket *socket = Networking::Socket::connect(lobbyUrl);
		if (!socket) {
			Testsuite::logDetailedPrintf("Error! Socket connection failed\n");
			return kTestFailed;
		}

		const char *send = "{\"cmd\": \"get_population\", \"area\": 8}\n";
		size_t length = strlen(send);

		if (socket->send(send, length) != length) {
			Testsuite::logDetailedPrintf("Error! Socket send failed\n");
			return kTestFailed;
		}

		int i;
		for (i = 0; i < 10; i++) {
			if (socket->ready()) {
				break;
			}
			g_system->delayMillis(1000);
		}

		if (i == 10) {
			Testsuite::logDetailedPrintf("Error! Socket ready check failed\n");
			return kTestFailed;
		}

		char reply[1024];
		length = socket->recv(reply, sizeof(reply));
		if (length == 0) {
			Testsuite::logDetailedPrintf("Error! Socket receive failed\n");
			return kTestFailed;
		}
		reply[length] = 0;

		debug("Socket received: %s", reply);

		Testsuite::logDetailedPrintf("URL test worked\n");
		return kTestPassed;
	}
	return TestExitStatus();
}
#endif

} // namespace Testbed
