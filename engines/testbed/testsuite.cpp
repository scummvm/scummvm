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

#include "graphics/fontman.h"
#include "graphics/surface.h"

#include "gui/message.h"

#include "testbed/testsuite.h"

namespace Testbed {

// Static public variable of Testsuite
bool Testsuite::isInteractive = true;

Testsuite::Testsuite() {
		_numTestsPassed = 0;
		_numTestsExecuted = 0;
}

Testsuite::~Testsuite() {
	for (Common::Array<Test*>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		delete (*i);
	}
}

void Testsuite::genReport() const {
	printf("\nSubsystem: %s\n",getName());
	printf("Tests executed: %d\n", _numTestsExecuted);
	printf("Tests Passed: %d\n", _numTestsPassed);
	printf("Tests Failed: %d\n\n", getNumTestsFailed());
}
	
bool Testsuite::handleInteractiveInput(const Common::String &textToDisplay, const char *opt1, const char *opt2, OptionSelected result) {
	GUI::MessageDialog	prompt(textToDisplay, opt1, opt2);
	return prompt.runModal() == result ? true : false;
}

void Testsuite::displayMessage(const Common::String &textToDisplay, const char *defaultButton, const char *altButton) {
	GUI::MessageDialog	prompt(textToDisplay, defaultButton);
	prompt.runModal();
}

Common::Rect Testsuite::writeOnScreen(const Common::String &textToDisplay, const Common::Point &pt, bool flag) {
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

void Testsuite::clearScreen(const Common::Rect &rect) {
	Graphics::Surface *screen = g_system->lockScreen();
	
	screen->fillRect(rect, kColorBlack);

	g_system->unlockScreen();
	g_system->updateScreen();
}
	
void Testsuite::clearScreen() {
	int numBytesPerLine = g_system->getWidth() * g_system->getScreenFormat().bytesPerPixel;
	int size =  g_system->getHeight() * numBytesPerLine;
	byte *buffer = new byte[size];
	memset(buffer, 0, size);
	g_system->copyRectToScreen(buffer, numBytesPerLine, 0, 0, g_system->getWidth(), g_system->getHeight());
	g_system->updateScreen();	
}
	
void Testsuite::clearScreen(bool flag) {
	Graphics::Surface *screen = g_system->lockScreen();
	uint fillColor = kColorBlack;

	if (flag) {
		fillColor = g_system->getScreenFormat().RGBToColor(0, 0, 0);
	}

	screen->fillRect(Common::Rect(0, 0, g_system->getWidth(), g_system->getHeight()), fillColor);

	g_system->unlockScreen();
	g_system->updateScreen();
}

void Testsuite::addTest(const Common::String &name, InvokingFunction f) {
	Test*  featureTest = new Test(name, f);
	_testsToExecute.push_back(featureTest);
}
	
void Testsuite::execute() {
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

} // end of namespace Testebed

