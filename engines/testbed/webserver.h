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

#ifndef TESTBED_WEBSERVER_H
#define TESTBED_WEBSERVER_H

#include "testbed/testsuite.h"

namespace Testbed {

namespace WebserverTests {

// Helper functions for Webserver tests

bool startServer();
TestExitStatus testIP();
TestExitStatus testIndexPage();
TestExitStatus testFilesPageInvalidParameterValue();
TestExitStatus testFilesPageCreateDirectory();
TestExitStatus testFilesPageUploadFile();
TestExitStatus testFilesPageUploadDirectory();
TestExitStatus testFilesPageDownloadFile();

} // End of namespace WebserverTests

class WebserverTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the WebserverTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	WebserverTestSuite();
	~WebserverTestSuite() override {}
	const char *getName() const override {
		return "Webserver";
	}

	const char *getDescription() const override {
		return "Webserver tests";
	}

};

} // End of namespace Testbed

#endif // TESTBED_TEMPLATE_H
