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

#ifndef TESTSUITE_H
#define TESTSUITE_H

#include "common/system.h"
#include "common/str.h"
#include "common/array.h"

#include "graphics/fontman.h"
#include "graphics/surface.h"

#include "gui/message.h"

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
	Testsuite() {
		_numTestsPassed = 0;
		_numTestsExecuted = 0;
	}
	
	virtual ~Testsuite() {
		for (Common::Array<Test*>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
			delete (*i);
		}	
	}
	
	int getNumTests() const { return _testsToExecute.size(); }
	int getNumTestsPassed() const { return _numTestsPassed; }
	int getNumTestsFailed() const { return _numTestsExecuted - _numTestsPassed; }
	void genReport() const {
		printf("\nSubsystem: %s\n",getName());
		printf("Tests executed: %d\n", _numTestsExecuted);
		printf("Tests Passed: %d\n", _numTestsPassed);
		printf("Tests Failed: %d\n\n", getNumTestsFailed());
	}
	
	/**
	 * Prompts for User Input in form of "Yes" or "No" for interactive tests
	 * e.g: "Is this like you expect?" "Yes" or "No"
	 *
	 * @param	textToDisplay Display text
	 * @return	true if "Yes" false otherwise
	 */ 
	static bool handleInteractiveInput(const Common::String &textToDisplay, const char *opt1 = "Yes", const char *opt2 = "No", OptionSelected result = kOptionLeft) {
		GUI::MessageDialog	prompt(textToDisplay, opt1, opt2);
		return prompt.runModal() == result ? true : false;
	}
	
	static void displayMessage(const Common::String &textToDisplay, const char *defaultButton = "OK", const char *altButton = 0) {
		GUI::MessageDialog	prompt(textToDisplay, defaultButton);
		prompt.runModal();
	}

	static Common::Rect writeOnScreen(const Common::String &textToDisplay, const Common::Point &pt, bool flag = false) {
		const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));
		uint fillColor = kColorBlack;
		uint textColor = kColorWhite;
		
		Graphics::Surface *screen = g_system->lockScreen();
		
		int height = font.getFontHeight();
		int width = screen->w;

		Common::Rect rect(pt.x, pt.y, pt.x + width, pt.y + height);

		if (flag) {
			Graphics::PixelFormat pf = g_system->getScreenFormat();
			fillColor = pf.RGBToColor(0 , 0, 0);
			textColor = pf.RGBToColor(255 , 255, 255);
		}
		
		screen->fillRect(rect, fillColor);
		font.drawString(screen, textToDisplay, rect.left, rect.top, screen->w, textColor, Graphics::kTextAlignCenter);

		g_system->unlockScreen();
		g_system->updateScreen();

		return rect;
	}

	static void clearScreen(const Common::Rect &rect) {
		Graphics::Surface *screen = g_system->lockScreen();
		
		screen->fillRect(rect, kColorBlack);	

		g_system->unlockScreen();
		g_system->updateScreen();
	}
	
	static void clearScreen() {
		int numBytesPerLine = g_system->getWidth() * g_system->getScreenFormat().bytesPerPixel;
		int size =  g_system->getHeight() * numBytesPerLine;
		byte *buffer = new byte[size];
		memset(buffer, 0, size);
		g_system->copyRectToScreen(buffer, numBytesPerLine, 0, 0, g_system->getWidth(), g_system->getHeight());
		g_system->updateScreen();	
	}
	
	static void clearScreen(bool flag) {
		Graphics::Surface *screen = g_system->lockScreen();
		uint fillColor = kColorBlack;
		
		if (flag) {
			fillColor = g_system->getScreenFormat().RGBToColor(0, 0, 0);
		}
		
		screen->fillRect(Common::Rect(0, 0, g_system->getWidth(), g_system->getHeight()), fillColor);

		g_system->unlockScreen();
		g_system->updateScreen();
	}

	/**
	 * Adds a test to the list of tests to be executed
	 *
	 * @param	name the string description of the test, for display purposes
	 * @param	f pointer to the function that invokes this test
	 */
	void addTest(const Common::String &name, InvokingFunction f) {
		Test*  featureTest = new Test(name, f);
		_testsToExecute.push_back(featureTest);
	}
	
	/**
	 * The driver function for the testsuite
	 * All code should go in here.
	 */
	virtual void execute() {
		for (Common::Array<Test*>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
			printf("Executing Test:%s\n", ((*i)->featureName).c_str());
			_numTestsExecuted++;
			if ((*i)->driver()) {
				printf("RESULT: Passed\n");
				_numTestsPassed++;
			} else {
				printf("RESULT: Failed\n");
			}
		}
		genReport();
	}
	virtual const char *getName() const = 0;

protected:
	Common::Array<Test*> _testsToExecute;			///< List of tests to be executed
	int		    _numTestsPassed;					///< Number of tests passed
	int  		_numTestsExecuted;					///< Number of tests executed
};

}	// End of namespace testbed

#endif
