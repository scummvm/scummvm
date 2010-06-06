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

typedef bool (*invokingFunction)();

/**
 * This represents a feature to be tested
 */

struct Test {
	Test(Common::String name, invokingFunction f) : featureName(name), driver(f), enabled(true), passed(false) {}
	Common::String featureName;		///< Name of feature to be tested
	invokingFunction driver;	    ///< Pointer to the function that will invoke this feature test
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
	
	inline int getNumTests() { return _testsToExecute.size(); }
	inline int getNumTestsPassed() { return _numTestsPassed; }
	inline int getNumTestsFailed() { return _numTestsExecuted - _numTestsPassed; }
	inline void genReport() {
		printf("Subsystem:%s\n",getName());
		printf("Tests executed:%d\n", _numTestsExecuted);
		printf("Tests Passed:%d\n", _numTestsPassed);
		printf("Tests Failed:%d\n", getNumTestsFailed());
	}
	
	/**
	 * Prompts for User Input in form of "Yes" or "No" for interactive tests
	 * e.g: "Is this like you expect?" "Yes" or "No"
	 *
	 * @param	textToDisplay Display text
	 * @return	true if "Yes" false otherwise
	 */ 
	static inline bool handleInteractiveInput(Common::String& textToDisplay) {
		GUI::MessageDialog	prompt(textToDisplay, "Yes", "No");
		return prompt.runModal() == GUI::kMessageOK ? true : false;
	}
	
	static inline void displayMessage(const char *textToDisplay) {
		Common::String message(textToDisplay);
		GUI::MessageDialog	prompt(message);
		prompt.runModal();
	}

	static inline Common::Rect writeOnScreen(const char *textToDisplay, Common::Point &pt) {
		const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));
		
		Graphics::Surface *screen = g_system->lockScreen();
		
		int height = font.getFontHeight();
		int width = screen->w;

		Common::Rect rect(pt.x, pt.y, pt.x + width, pt.y + height);

		screen->fillRect(rect, kColorBlack);
		Common::String text(textToDisplay);
		font.drawString(screen, text, rect.left, rect.top, screen->w, kColorWhite, Graphics::kTextAlignCenter);

		g_system->unlockScreen();
		g_system->updateScreen();

		return rect;
	}

	static inline void clearScreen(Common::Rect rect) {
		Graphics::Surface *screen = g_system->lockScreen();
		
		screen->fillRect(rect, kColorBlack);	

		g_system->unlockScreen();
		g_system->updateScreen();
	}

	/**
	 * Adds a test to the list of tests to be executed
	 *
	 * @param	name the string description of the test, for display purposes
	 * @param	f pointer to the function that invokes this test
	 */
	inline void addTest(Common::String name, invokingFunction f) {
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
				_numTestsPassed++;
			}
		}
		genReport();
	}
	virtual const char *getName() = 0;

protected:
	Common::Array<Test*> _testsToExecute;			///< List of tests to be executed
	int		    _numTestsPassed;					///< Number of tests passed
	int  		_numTestsExecuted;					///< Number of tests executed
};

}	// End of namespace testbed

#endif
