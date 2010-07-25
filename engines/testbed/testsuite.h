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

#include "graphics/fontman.h"

namespace Testbed {

enum {
	kColorBlack = 0,
	kColorWhite = 1,
	kColorCustom = 2,
	kColorSpecial = 5 	///< some random number
};

enum OptionSelected {
	kOptionLeft = 1,
	kOptionRight = 0
};

enum {
	kEngineQuit = 0,
	kSkipNext = 1,
	kLoopNormal = 2,
	// Event handling time,(in ms) used in parseEvent()
	kEventHandlingTime = 50
};

typedef bool (*InvokingFunction)();

/**
 * This represents a feature to be tested
 */

struct Test {
	Test(Common::String name, InvokingFunction f, bool interactive) : featureName(name) {
		driver = f;
		enabled = true;
		passed = false;
		isInteractive = interactive;
	}
	const Common::String featureName;	///< Name of feature to be tested
	InvokingFunction driver;		///< Pointer to the function that will invoke this feature test
	bool enabled;				    ///< Decides whether or not this test is to be executed
	bool passed;					///< Collects and stores result of this feature test
	bool isInteractive;				///< Decides if the test is interactive or not, An interactive testsuite may have non-interactive tests, hence this change.
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
	bool isEnabled() const { return _isTsEnabled; }
	virtual void enable(bool flag) {
		_isTsEnabled = flag;
	}
	bool enableTest(const Common::String &testName, bool enable);

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
	static void clearEntireScreen() {
		const int width = g_system->getWidth();
		const int height = g_system->getHeight();
		Common::Rect r(0, 0, width, height);
		clearScreen(r);
	}
	static void clearScreen();
	static void clearScreen(bool flag);

	/**
	 * Adds a test to the list of tests to be executed
	 *
	 * @param	name the string description of the test, for display purposes
	 * @param	f pointer to the function that invokes this test
	 * @param	isInteractive	decides if the test is to be executed in interactive mode/ default true
	 */
	void addTest(const Common::String &name, InvokingFunction f, bool isInteractive = true);

	/**
	 * The driver function for the testsuite
	 * All code should go in here.
	 */
	virtual void execute();
	static uint parseEvents();

	virtual const char *getName() const = 0;
	virtual const char *getDescription() const = 0;

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

	// Progress bar (Information Display) related methods.
	/** 
	 * Display region is in the bottom. Probably 1/4th of the game screen.
	 * It contains:
	 * 1) Information about executing testsuite.
	 * 2) Total progress within this testsuite.
	 * 3) Total overall progress in the number of testsuites
	 */

	static Common::Point getDisplayRegionCoordinates() {
		Common::Point pt(0, 0);
		// start from bottom
		pt.y = g_system->getHeight();
		// Will Contain 3 lines
		pt.y -= (FontMan.getFontByUsage(_displayFont)->getFontHeight() * 3 + 15); // Buffer of 5 pixels per line
		return pt;
	}

	static uint getLineSeparation() {
		return FontMan.getFontByUsage(_displayFont)->getFontHeight() + 5;
	}
	static Graphics::FontManager::FontUsage getCurrentFontUsageType() { return _displayFont; }
	static void setCurrentFontUsageType(Graphics::FontManager::FontUsage f) { _displayFont = f; }

	static void updateStats(const char *prefix, const char *info, uint numTests, uint testNum, Common::Point pt);
	const Common::Array<Test *>& getTestList() { return _testsToExecute; }


protected:
	Common::Array<Test *> _testsToExecute;			///< List of tests to be executed
	int		    _numTestsPassed;					///< Number of tests passed
	int			_numTestsExecuted;					///< Number of tests executed
	bool		_isTsEnabled;

public:

	/**
	 * Static variable of this class that determines if the user initiated testing session is interactive or not.
	 * Used by various tests to respond accordingly
	 */
	static bool isSessionInteractive;

	/**
	 * Used from the code to decide if the engine needs to exit
	 */
	static uint	toQuit;

private:
	/**
	 * Private variables related to logging files
	 */
	static Common::String _logDirectory;
	static Common::String _logFilename;
	static Common::WriteStream *_ws;

	/**
	 * Private variable used for font
	 */
	static Graphics::FontManager::FontUsage	_displayFont;
};

} // End of namespace Testbed

#endif // TESTBED_TESTSUITE_H
