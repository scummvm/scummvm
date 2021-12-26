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

#ifndef TESTBED_NETWORKING_H
#define TESTBED_NETWORKING_H

#include "testbed/testsuite.h"

// This file can be used as template for header files of other newer testsuites.

namespace Testbed {

namespace Networkingtests {

// Helper functions for Networking tests

TestExitStatus testConnectionLimit();

} // End of namespace Networkingtests

class NetworkingTestSuite : public Testsuite {
public:
	NetworkingTestSuite();
	~NetworkingTestSuite() {}

	const char *getName() const {
		return "Networking";
	}

	const char *getDescription() const {
		return "Network and internet subsystems";
	}

};

} // End of namespace Testbed

#endif // TESTBED_NETWORKING_H
