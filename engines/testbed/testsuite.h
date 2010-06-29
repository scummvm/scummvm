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

#ifndef TESTBED_TESTSUITE_H
#define TESTBED_TESTSUITE_H

#include "common/system.h"
#include "common/str.h"
#include "common/array.h"


namespace Testbed {

enum {
	kColorBlack = 0,
	kColorWhite = 1,
	kColorCustom = 2
};

enum OptionSelected {
	kOptionLeft = 1,
	kOptionRight = 0
};

typedef bool (*InvokingFunction)();

/**
 * This represents a feature to be tested
 */

struct Test {
	Test(Common::String name, InvokingFunction f) : featureName(name), driver(f), enabled(true), passed(false) {}
	Common::String featureName;		///< Name of feature to be tested
	InvokingFunction driver;	    ///< Pointer to the function that will invoke this feature test
	bool enabled;				    ///< Decides whether or not this test is to be executed
	bool passed;					///< Collects and stores result of this feature test
};


/**
 * The basic Testsuite class
 * All the other testsuites would inherit it and override its virtual methods
 */

class Testsuite {
public:
	Testsuite();
	virtual ~Testsuite();	
	int getNumTests() const { return _testsToExecute.size(); }
	int getNumTestsPassed() const { return _numTestsPassed; }
	int getNumTestsFailed() const { return _numTestsExecuted - _numTestsPassed; }
	void genReport() const;

	/**
	 * Prompts for User Input in form of "Yes" or "No" for interactive tests
	 * e.g: "Is this like you expect?" "Yes" or "No"
	 *
	 * @param	textToDisplay Display text
	 * @return	true if "Yes" false otherwise
	 */ 
	static bool handleInteractiveInput(const Common::String &textToDisplay, const char *opt1 = "Yes", const char *opt2 = "No", OptionSelected result = kOptionLeft);
	
	static void displayMessage(const Common::String &textToDisplay, const char *defaultButton = "OK", const char *altButton = 0);
	static Common::Rect writeOnScreen(const Common::String &textToDisplay, const Common::Point &pt, bool flag = false);	
	static void clearScreen(const Common::Rect &rect);
	static void clearScreen();
	static void clearScreen(bool flag);

	/**
	 * Adds a test to the list of tests to be executed
	 *
	 * @param	name the string description of the test, for display purposes
	 * @param	f pointer to the function that invokes this test
	 */
	void addTest(const Common::String &name, InvokingFunction f);

	/**
	 * The driver function for the testsuite
	 * All code should go in here.
	 */
	virtual void execute();
	
	virtual const char *getName() const = 0;

	static void logPrintf(const char *s, ...) GCC_PRINTF(1, 2);
	static void logDetailedPrintf(const char *s, ...) GCC_PRINTF(1, 2);
	/**
	 * Note: To enable logging, this function must be called once first.
	 */
	static void initLogging(const char *dirname, const char *filename, bool enable = true);
	static void initLogging(bool enable = true);
	static void setLogDir(const char *dirname);
	static void setLogFile(const char *filename);
	
	static void deleteWriteStream();

protected:
	Common::Array<Test*> _testsToExecute;			///< List of tests to be executed
	int		    _numTestsPassed;					///< Number of tests passed
	int  		_numTestsExecuted;					///< Number of tests executed

public:

	/**
	 * Static variable of this class that determines if the tests are interactive or not.
	 * Used by various tests to respond accordingly
	 */
	static bool isInteractive;

private:
	/**
	 * Private variables related to logging files
	 */
	static Common::String _logDirectory;
	static Common::String _logFilename;
	static Common::WriteStream *_ws;
};

}	// End of namespace testbed

#endif
