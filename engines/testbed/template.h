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

#ifndef TESTBED_TEMPLATE_H
#define TESTBED_TEMPLATE_H

#include "testbed/testsuite.h"

// This file can be used as template for header files of other newer testsuites.

namespace Testbed {

namespace XXXtests {

// Helper functions for XXX tests

// will contain function declarations for XXX tests
// add more here

} // End of namespace XXXtests

class XXXTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the XXXTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	XXXTestSuite();
	~XXXTestSuite() {}
	const char *getName() const {
		return "Dummy Template";
	}

	const char *getDescription() const {
		return "Some Arbit description";
	}

};

} // End of namespace Testbed

#endif // TESTBED_TEMPLATE_H
