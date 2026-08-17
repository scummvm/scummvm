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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/puzzles/calculator.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "math/utils.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

struct CalculatorButtonLayout {
	int y;
	int x;
	uint command;
};

static const CalculatorButtonLayout kButtonLayouts[] = {
	{  49, 388, 'o' }, {  50, 421, 'n' }, { 137, 368, '0' },
	{ 120, 367, '1' }, { 120, 389, '2' }, { 120, 411, '3' },
	{ 103, 367, '4' }, { 103, 388, '5' }, { 103, 409, '6' },
	{  86, 366, '7' }, {  86, 386, '8' }, {  87, 407, '9' },
	{  87, 434, '+' }, { 104, 434, '-' }, { 120, 436, '/' },
	{ 137, 437, '*' }, { 138, 412, 0x0d }, { 137, 390, '.' },
	{  85, 309, 'q' }, { 101, 309, 'x' }, { 119, 309, 'r' },
	{  86, 338, 'i' }, { 102, 338, 's' }, { 120, 339, 't' },
	{ 137, 339, 'c' }
};

static const uint kGlyphCount = 12;
static const uint kMaximumDisplayCharacters = 10;
static const int kSceneOriginY = 50;
static const int kDisplayY = kSceneOriginY + 0x39;
static const int kDisplayX = 0xdc;
static const int kDisplayHeight = 0xf;
static const int kDisplayWidth = 0x99;
static const byte kDisplayBackgroundColor = 0x89;
static const int kDisplayRightInset = 10;
static const int kGlyphOverlap = 3;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kExitCursor = 7;
static const uint16 kHelpCommand = 0x3b00;
static const uint kHelpSelectionTable = 0x1a0;
static const uint kUnlockPrerequisiteFlag = 2;
static const uint kUnlockDisabledFlag = 0x37;
static const uint kButtonFeedbackTicks = 3;
static const uint kDosTickDurationMillis = 55;

static const uint kUnlockSequence[] = {
	'4', '6', 'q', '/', '7', 'i', 0x0d, '5'
};

static bool isInsideCalculatorControl(const Common::Point &point) {
	return Common::Rect(160, kSceneOriginY + 20, 490,
		kSceneOriginY + 190).contains(point);
}

static bool isInsideExitControl(const Common::Point &point) {
	return Common::Rect(160, kSceneOriginY + 190, 490,
		kSceneOriginY + 300).contains(point);
}

} // End of anonymous namespace

CalculatorPuzzle::CalculatorPuzzle(RipperEngine *engine) : Puzzle(engine),
		_accumulator(0.0), _currentOperand(0.0), _pendingOperator(0xff),
		_entryMode(0), _unlockSequenceProgress(0), _hoveredButton(-1),
		_poweredOn(false), _displayCleared(false), _decimalPointEntered(false),
		_inverseTrigMode(false) {
}

bool CalculatorPuzzle::captureBackground() {
	return _backgroundDisplay.capture();
}

void CalculatorPuzzle::restoreBackground() const {
	_backgroundDisplay.restore();
}

bool CalculatorPuzzle::loadAssets() {
	_buttons.clear();
	for (uint i = 0; i < ARRAYSIZE(kButtonLayouts); ++i) {
		BitmapAssetSequence sequence;
		const Common::String name = Common::String::format("calc%u.bbm", i);
		if (!_engine->getResources()->loadBitmapSequence(name, sequence) ||
				sequence.frames.empty()) {
			warning("Ripper: failed to load calculator button '%s'", name.c_str());
			return false;
		}
		_buttons.push_back(Common::move(sequence.frames.front()));
	}

	_glyphs.clear();
	for (uint i = 0; i < kGlyphCount; ++i) {
		BitmapAssetSequence sequence;
		const Common::String name = Common::String::format("calcnm%u.bbm", i);
		if (!_engine->getResources()->loadBitmapSequence(name, sequence) ||
				sequence.frames.empty()) {
			warning("Ripper: failed to load calculator glyph '%s'", name.c_str());
			return false;
		}
		_glyphs.push_back(Common::move(sequence.frames.front()));
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded calculator puzzle assets buttons=%u glyphs=%u",
		_buttons.size(), _glyphs.size());
	return true;
}

void CalculatorPuzzle::drawDisplay(byte *screen, uint pitch) const {
	if (!_poweredOn)
		return;

	const uint characterCount = MIN<uint>(_displayText.size(), kMaximumDisplayCharacters);
	int glyphX = kDisplayX + kDisplayWidth - kDisplayRightInset;
	for (int characterIndex = characterCount - 1; characterIndex >= 0; --characterIndex) {
		const char character = _displayText[characterIndex];
		int glyphIndex = character < '/' ? character - '#' : character - '0';
		if (glyphIndex < 0 || glyphIndex >= (int)_glyphs.size()) {
			warning("Ripper: calculator display cannot map character 0x%02x", (byte)character);
			continue;
		}
		const BitmapAssetFrame &glyph = _glyphs[glyphIndex];
		glyphX -= glyph.width - kGlyphOverlap;
		IndexedBitmapRenderer::drawBitmap(screen, pitch, glyph, glyphX, kDisplayY,
			Common::Rect(0, 0, kRipperScreenWidth, kRipperScreenHeight));
	}
}

void CalculatorPuzzle::render() const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			_backgroundDisplay.pixels().size() != (uint)screen->w * screen->h) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = 0; y < screen->h; ++y)
		memcpy(screen->getBasePtr(0, y),
			_backgroundDisplay.pixels().data() + y * screen->w, screen->w);
	byte *pixels = (byte *)screen->getPixels();
	if (_displayCleared) {
		for (int y = kDisplayY; y < kDisplayY + kDisplayHeight; ++y)
			memset(screen->getBasePtr(kDisplayX, y), kDisplayBackgroundColor,
				kDisplayWidth);
	}
	drawDisplay(pixels, screen->pitch);
	g_system->unlockScreen();
	presentScreen();
}

void CalculatorPuzzle::showButtonFeedback(uint16 command) const {
	for (uint i = 0; i < _buttons.size(); ++i) {
		if (kButtonLayouts[i].command != command)
			continue;

		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen || screen->format.bytesPerPixel != 1) {
			if (screen)
				g_system->unlockScreen();
			return;
		}
		IndexedBitmapRenderer::drawBitmap((byte *)screen->getPixels(), screen->pitch,
			_buttons[i], kButtonLayouts[i].x,
			kButtonLayouts[i].y + kSceneOriginY,
			Common::Rect(0, 0, screen->w, screen->h));
		g_system->unlockScreen();
		presentScreen();
		debugC(3, kDebugPuzzles,
			"Ripper: calculator button feedback index=%u command=0x%04x ticks=%u",
			i, command, kButtonFeedbackTicks);
		g_system->delayMillis(kButtonFeedbackTicks * kDosTickDurationMillis);
		render();
		return;
	}
}

int CalculatorPuzzle::findButton(const Common::Point &point) const {
	for (uint i = 0; i < _buttons.size(); ++i) {
		const BitmapAssetFrame &frame = _buttons[i];
		const Common::Rect bounds(kButtonLayouts[i].x,
			kButtonLayouts[i].y + kSceneOriginY,
			kButtonLayouts[i].x + frame.width,
			kButtonLayouts[i].y + kSceneOriginY + frame.height);
		if (bounds.contains(point))
			return i;
	}
	return -1;
}

void CalculatorPuzzle::updateCursor(const Common::Point &point) {
	const int hoveredButton = findButton(point);
	uint cursor = 0;
	if (hoveredButton >= 0)
		cursor = kSelectionCursor;
	else if (isInsideExitControl(point))
		cursor = kExitCursor;
	else if (isInsideCalculatorControl(point))
		cursor = kDefaultCursor;

	if (hoveredButton != _hoveredButton) {
		debugC(2, kDebugPuzzles,
			"Ripper: calculator puzzle hover button=%d command=0x%04x point=%d,%d cursor=%u",
			hoveredButton,
			hoveredButton >= 0 ? kButtonLayouts[hoveredButton].command : 0,
			point.x, point.y, cursor);
		_hoveredButton = hoveredButton;
	}
	_engine->getCursor()->update(cursor);
}

double CalculatorPuzzle::parseDisplay() const {
	bool negative = false;
	uint position = 0;
	if (!_displayText.empty() && _displayText[0] == '-') {
		negative = true;
		position = 1;
	}

	double value = 0.0;
	while (position < _displayText.size() && _displayText[position] >= '0' &&
			_displayText[position] <= '9') {
		value = value * 10.0 + _displayText[position] - '0';
		++position;
	}
	if (position < _displayText.size() && _displayText[position] == '.') {
		double place = 0.1;
		for (++position; position < _displayText.size() &&
				_displayText[position] >= '0' && _displayText[position] <= '9'; ++position) {
			value += (_displayText[position] - '0') * place;
			place *= 0.1;
		}
	}
	return negative ? -value : value;
}

void CalculatorPuzzle::formatDisplay(double value) {
	_displayText = Common::String::format("%.10f", value);
	while (!_displayText.empty() && _displayText.lastChar() == '0')
		_displayText.deleteLastChar();
	if (_displayText.empty())
		_displayText = "0";
}

void CalculatorPuzzle::applyPendingOperator() {
	switch (_pendingOperator) {
	case '*':
		_accumulator *= _currentOperand;
		break;
	case '+':
		_accumulator += _currentOperand;
		break;
	case '-':
		_accumulator -= _currentOperand;
		break;
	case '/':
		_accumulator = _currentOperand == 0.0 ? 0.0 : _accumulator / _currentOperand;
		break;
	default:
		_accumulator = _currentOperand;
		break;
	}
	debugC(3, kDebugPuzzles,
		"Ripper: calculator applied binary command=0x%02x accumulator=%f operand=%f",
		_pendingOperator, _accumulator, _currentOperand);
}

void CalculatorPuzzle::applyUnaryOperator(uint command) {
	const uint effectiveCommand = _inverseTrigMode ? command + 1000 : command;
	switch (effectiveCommand) {
	case 'q':
		_currentOperand *= _currentOperand;
		break;
	case 'r':
		_currentOperand = _currentOperand > 0.0 ? sqrt(_currentOperand) : 0.0;
		break;
	case 'c':
		_currentOperand = cos(Math::deg2rad<double>(_currentOperand));
		break;
	case 's':
		_currentOperand = sin(Math::deg2rad<double>(_currentOperand));
		break;
	case 't':
		_currentOperand = tan(Math::deg2rad<double>(_currentOperand));
		break;
	case 'x':
		if (_currentOperand != 0.0)
			_currentOperand = 1.0 / _currentOperand;
		break;
	case 'c' + 1000:
		_currentOperand = _currentOperand <= 1.0 ?
			Math::rad2deg<double>(acos(_currentOperand)) : 0.0;
		break;
	case 's' + 1000:
		_currentOperand = _currentOperand <= 1.0 ?
			Math::rad2deg<double>(asin(_currentOperand)) : 0.0;
		break;
	case 't' + 1000:
		_currentOperand = Math::rad2deg<double>(atan(_currentOperand));
		break;
	default:
		break;
	}
	debugC(3, kDebugPuzzles,
		"Ripper: calculator applied unary command=0x%x operand=%f inverse=%d",
		effectiveCommand, _currentOperand, _inverseTrigMode);
}

void CalculatorPuzzle::resetEntry() {
	_decimalPointEntered = false;
	_displayText = "0";
	_entryMode = 0;
}

bool CalculatorPuzzle::advanceUnlockSequence(uint command, uint completionFlag, Result &result) {
	if (!_engine->getMilestones()->isSet(kUnlockPrerequisiteFlag) ||
			_engine->getMilestones()->isSet(kUnlockDisabledFlag) ||
			_unlockSequenceProgress >= ARRAYSIZE(kUnlockSequence))
		return false;

	if (command == kUnlockSequence[_unlockSequenceProgress]) {
		debugC(3, kDebugPuzzles,
			"Ripper: calculator unlock sequence matched step=%u command=0x%04x",
			_unlockSequenceProgress, command);
		if (_unlockSequenceProgress == ARRAYSIZE(kUnlockSequence) - 1) {
			if (!markSolved(completionFlag, "calculator-puzzle")) {
				result = kLoadFailed;
				return true;
			}
			debugC(1, kDebugPuzzles,
				"Ripper: calculator puzzle completed milestone=%u", completionFlag);
			result = kSolved;
			return true;
		}
	} else {
		debugC(3, kDebugPuzzles,
			"Ripper: calculator unlock sequence rejected step=%u expected=0x%04x actual=0x%04x",
			_unlockSequenceProgress, kUnlockSequence[_unlockSequenceProgress], command);
		_unlockSequenceProgress = ARRAYSIZE(kUnlockSequence) + 1;
	}
	++_unlockSequenceProgress;
	return false;
}

bool CalculatorPuzzle::processCommand(uint16 command, uint completionFlag, Result &result) {
	showButtonFeedback(command);
	if (command == '=')
		command = 0x0d;
	if (!_poweredOn && command != 'n' && command != 'N' &&
			command != kHelpCommand && command != 0x1b)
		command = 0;
	if (command == 0)
		return false;

	debugC(3, kDebugPuzzles,
		"Ripper: calculator input command=0x%04x powered=%d display='%s' entryMode=%u pending=0x%02x",
		command, _poweredOn, _displayText.c_str(), _entryMode, _pendingOperator);
	if (command == 0x1b) {
		result = kExited;
		return true;
	}
	if (command == kHelpCommand) {
		debugC(1, kDebugPuzzles,
			"Ripper: calculator puzzle opening modal help table=0x%x",
			kHelpSelectionTable);
		if (!_engine->getModalDialog()->run(kHelpSelectionTable))
			warning("Ripper: calculator puzzle modal help failed");
		render();
	}
	if (advanceUnlockSequence(command, completionFlag, result))
		return true;

	if (command == 'n' || command == 'N') {
		_poweredOn = true;
		_displayCleared = true;
		if (_displayText == "0") {
			_accumulator = 0.0;
			_pendingOperator = 0xff;
			_entryMode = 0;
		} else {
			_currentOperand = 0.0;
			_displayText = "0";
		}
		_unlockSequenceProgress = 0;
		render();
		debugC(2, kDebugPuzzles, "Ripper: calculator powered on display='%s'",
			_displayText.c_str());
	} else if (command == 'o' || command == 'O') {
		_poweredOn = false;
		_displayCleared = true;
		render();
		debugC(2, kDebugPuzzles, "Ripper: calculator powered off");
	} else if (command == 'i') {
		_inverseTrigMode = true;
		debugC(2, kDebugPuzzles, "Ripper: calculator inverse mode armed");
	} else if (command == 'q' || command == 'r' || command == 's' ||
			command == 't' || command == 'c' || command == 'x') {
		applyUnaryOperator(command);
		formatDisplay(_currentOperand);
		_currentOperand = parseDisplay();
		render();
		_entryMode = 2;
	} else if (command == '*' || command == '+' || command == '-' ||
			command == '/' || command == 0x0d) {
		if (_entryMode != 1) {
			applyPendingOperator();
			formatDisplay(_accumulator);
			_accumulator = parseDisplay();
			render();
			_entryMode = 1;
		}
		_pendingOperator = command;
		if (command == 0x0d)
			_currentOperand = _accumulator;
	} else if ((command >= '0' && command <= '9') || command == '.') {
		if (_entryMode != 0)
			resetEntry();
		if (_displayText.size() < kMaximumDisplayCharacters) {
			if (command == '.') {
				if (_decimalPointEntered)
					return false;
				_decimalPointEntered = true;
			}
			if (_displayText == "0" && command != '.')
				_displayText.clear();
			_displayText += (char)command;
			_currentOperand = parseDisplay();
			render();
		}
	}

	if (command != 'i')
		_inverseTrigMode = false;
	return false;
}

CalculatorPuzzle::Result CalculatorPuzzle::run(uint completionFlag) {
	if (!captureBackground()) {
		warning("Ripper: failed to capture calculator puzzle background");
		return kLoadFailed;
	}
	if (!loadAssets()) {
		restoreBackground();
		return kLoadFailed;
	}

	_displayText = "0";
	debugC(1, kDebugPuzzles,
		"Ripper: entered calculator puzzle buttons=%u glyphs=%u completionFlag=%u "
		"unlockGate=%u unlockDisabled=%u",
		_buttons.size(), _glyphs.size(), completionFlag,
		kUnlockPrerequisiteFlag, kUnlockDisabledFlag);
	_engine->getInput()->discardMouseTransitions();
	render();

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;

		while (_engine->getInput()->hasPendingKey()) {
			if (processCommand(_engine->getInput()->consumeKey(), completionFlag, result)) {
				active = false;
				break;
			}
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		updateCursor(mouse.position);
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			const int button = findButton(mouse.position);
			uint16 command = 0;
			if (button >= 0)
				command = kButtonLayouts[button].command;
			else if (isInsideExitControl(mouse.position))
				command = 0x1b;
			if (command != 0 && processCommand(command, completionFlag, result))
				active = false;
		}

		presentScreen();
		g_system->delayMillis(10);
	}

	restoreBackground();
	_engine->getCursor()->update(0);
	debugC(1, kDebugPuzzles,
		"Ripper: exited calculator puzzle result=%d milestone=%u",
		result, completionFlag);
	return result;
}

} // End of namespace Ripper
