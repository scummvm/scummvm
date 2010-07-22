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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/stream.h"

#include "graphics/fontman.h"
#include "graphics/surface.h"

#include "gui/message.h"

#include "testbed/graphics.h"
#include "testbed/testbed.h"
#include "testbed/testsuite.h"

namespace Testbed {

// Static public variable of Testsuite
bool Testsuite::isSessionInteractive = true;

// Static private variable of Testsuite
Common::String Testsuite::_logDirectory = "";
Common::String Testsuite::_logFilename = "";
Graphics::FontManager::FontUsage Testsuite::_displayFont = Graphics::FontManager::kGUIFont;
Common::WriteStream *Testsuite::_ws = 0;
uint Testsuite::toQuit = kLoopNormal;

void Testsuite::setLogDir(const char *dirname) {
	_logDirectory = dirname;
}

void Testsuite::setLogFile(const char *filename) {
	_logFilename = filename;
}

void Testsuite::deleteWriteStream() {
	if (_ws) {
		delete _ws;
	}
}

void Testsuite::initLogging(const char *logdir, const char *filename, bool enable) {
	setLogDir(logdir);
	setLogFile(filename);

	if (enable) {
		_ws = Common::FSNode(_logDirectory).getChild(_logFilename).createWriteStream();
	} else {
		_ws = 0;
	}
}

void Testsuite::initLogging(bool enable) {
	setLogDir(ConfMan.get("path").c_str());
	setLogFile("testbed.log");

	if (enable) {
		_ws = Common::FSNode(_logDirectory).getChild(_logFilename).createWriteStream();
	} else {
		_ws = 0;
	}
}

void Testsuite::logPrintf(const char *fmt, ...) {
	// Assuming log message size to be not greater than STRINGBUFLEN i.e 256
	char buffer[STRINGBUFLEN];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buffer, STRINGBUFLEN, fmt, vl);
	va_end(vl);

	if (_ws) {
		_ws->writeString(buffer);
	} else {
		debugCN(kTestbedLogOutput, "%s", buffer);
	}
}

void Testsuite::logDetailedPrintf(const char *fmt, ...) {
	// Assuming log message size to be not greater than STRINGBUFLEN i.e 256
	// Messages with this function would only be displayed if -d1 is specified on command line
	char buffer[STRINGBUFLEN];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buffer, STRINGBUFLEN, fmt, vl);
	va_end(vl);

	if (_ws) {
		_ws->writeString(buffer);
	} else {
		debugCN(1, kTestbedLogOutput, "%s", buffer);
	}
}

Testsuite::Testsuite() {
	_numTestsPassed = 0;
	_numTestsExecuted = 0;
	// Initially all testsuites are enabled, disable them by calling enableTestSuite(name, false)
	_isTsEnabled = true;
}

Testsuite::~Testsuite() {
	for (Common::Array<Test *>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		delete (*i);
	}
}

void Testsuite::genReport() const {
	logPrintf("\n");
	logPrintf("Consolidating results...\n");
	logPrintf("Subsystem: %s ", getName());
	logPrintf("(Tests Executed: %d)\n", _numTestsExecuted);
	logPrintf("Passed: %d ", _numTestsPassed);
	logPrintf("Failed: %d\n", getNumTestsFailed());
	logPrintf("\n");
}

bool Testsuite::handleInteractiveInput(const Common::String &textToDisplay, const char *opt1, const char *opt2, OptionSelected result) {
	GUI::MessageDialog prompt(textToDisplay, opt1, opt2);
	return prompt.runModal() == result ? true : false;
}

void Testsuite::displayMessage(const Common::String &textToDisplay, const char *defaultButton, const char *altButton) {
	GUI::MessageDialog prompt(textToDisplay, defaultButton);
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
		fillColor = pf.RGBToColor(0, 0, 0);
		textColor = pf.RGBToColor(255, 255, 255);
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
	int height = getDisplayRegionCoordinates().y;
	
	// Don't clear test info display region
	int size =  height * numBytesPerLine;
	byte *buffer = new byte[size];
	memset(buffer, 0, size);
	g_system->copyRectToScreen(buffer, numBytesPerLine, 0, 0, g_system->getWidth(), height);
	g_system->updateScreen();
	delete[] buffer;
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

void Testsuite::addTest(const Common::String &name, InvokingFunction f, bool isInteractive) {
	Test *featureTest = new Test(name, f, isInteractive);
	_testsToExecute.push_back(featureTest);
}

uint Testsuite::parseEvents() {
	uint startTime = g_system->getMillis();
	uint end = startTime + kEventHandlingTime;
	do {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_KEYDOWN:
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					return kSkipNext;
				}
				break;
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				return kEngineQuit;
				break;
			default:
				break;
			}
		}
		g_system->delayMillis(10);
		startTime = g_system->getMillis();
	} while (startTime <= end);

	return kLoopNormal;
}

void Testsuite::updateStats(const char *prefix, const char *info, uint testNum, uint numTests, Common::Point pt) {
	Common::String text = Common::String::printf(" Running %s: %s (%d of %d) ", prefix, info, testNum, numTests);
	writeOnScreen(text, pt);
	// below the text a rectangle denoting the progress in the testsuite can be drawn.
	int separation = getLineSeparation();
	pt.y += separation;
	int wRect = 200;
	int lRect = 7;
	pt.x = g_system->getWidth() / 2 - 100;
	byte *buffer = new byte[lRect * wRect];
	memset(buffer, 0, sizeof(byte) * lRect * wRect);

	int wShaded = (int) (wRect * (((float)testNum - 1) / numTests));

	// draw the boundary
	memset(buffer, 1, sizeof(byte) * wRect);
	memset(buffer + (wRect * (lRect - 1)) , 1, sizeof(byte) * wRect);
	
	for (int i = 0; i < lRect; i++) {
		for (int j = 0; j < wRect; j++) {
			if (j < wShaded) {
				buffer[i * wRect + j] = 1;
			}
		}
		buffer[i * wRect + 0] = 1;
		buffer[i * wRect + wRect - 1] = 1;
	}
	g_system->copyRectToScreen(buffer, wRect, pt.x, pt.y, wRect, lRect);
	g_system->updateScreen();
}

void Testsuite::execute() {
	// Main Loop for a testsuite

	// Do nothing if meant to exit
	if (toQuit == kEngineQuit) {
		return;
	}

	uint count = 1;
	Common::Point pt = getDisplayRegionCoordinates();
	pt.y += getLineSeparation();

	for (Common::Array<Test *>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		if (toQuit == kSkipNext) {
			logPrintf("Info! Skipping Test: %s, Skipped by user.\n", ((*i)->featureName).c_str());
			toQuit = kLoopNormal;
			continue;
		}

		if((*i)->isInteractive && !isSessionInteractive) {
			logPrintf("Info! Skipping Test: %s, non-interactive environment is selected\n", ((*i)->featureName).c_str());
			continue;
		}

		logPrintf("Info! Executing Test: %s\n", ((*i)->featureName).c_str());
		updateStats("Test", ((*i)->featureName).c_str(), count++, _testsToExecute.size(), pt);
		_numTestsExecuted++;
		if ((*i)->driver()) {
			logPrintf("Result: Passed\n");
			_numTestsPassed++;
		} else {
			logPrintf("Result: Failed\n");
		}
		updateStats("Test", ((*i)->featureName).c_str(), count, _testsToExecute.size(), pt);
		// TODO: Display a screen here to user with details of upcoming test, he can skip it or Quit or RTL
		// Check if user wants to quit/RTL/Skip next test by parsing events.
		// Quit directly if explicitly requested

		if (Engine::shouldQuit()) {
			toQuit = kEngineQuit;
			genReport();
			return;
		}

		toQuit = parseEvents();
	}
	genReport();
}

} // End of namespace Testebed
