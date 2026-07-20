/* ScummVM - Graphic Adventure Engine */

#include "ripper/dialogue.h"

#include "common/debug.h"
#include "common/serializer.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/script.h"

namespace Ripper {

static const uint kVisibleChoiceCount = 3;
static const int kChoiceRowHeight = 13;
static const int kChoiceTop = 344;
static const int kChoiceHorizontalPadding = 5;
static const int kArrowGap = 5;
static const int kSceneTop = 50;
static const int kSceneBottom = 350;
static const byte kNormalBackgroundColor = 0;
static const byte kNormalTextColor = 251;
static const byte kSelectedBackgroundColor = 248;
static const byte kSelectedTextColor = 4;
static const uint16 kEnterCommand = 0x0d;
static const uint16 kUpCommand = 0x4800;
static const uint16 kDownCommand = 0x5000;

bool DialogueChooser::initialize(ResourceManager &resources) {
	if (!resources.loadInterfaceBitmapFont("small.fnt", _font))
		return false;
	_arrowFrames.resize(4);
	for (uint i = 0; i < _arrowFrames.size(); ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("mnarrow%u.bbm", i), sequence) ||
				sequence.frames.empty())
			return false;
		_arrowFrames[i] = sequence.frames[0];
	}
	debugC(2, kDebugDialogue,
		"Ripper: initialized dialogue colors normal=%u/%u selected=%u/%u",
		kNormalTextColor, kNormalBackgroundColor,
		kSelectedTextColor, kSelectedBackgroundColor);
	return true;
}

void DialogueChooser::draw(bool captureBacking) {
	if (!_pending || _choices.empty())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	if (captureBacking) {
		Common::Rect currentBounds = visualBounds();
		currentBounds.clip(Common::Rect(0, 0, screen->w, screen->h));
		const bool initializeBacking = currentBounds != _backingBounds ||
			_backingPixels.size() != (uint)(currentBounds.width() * currentBounds.height());
		if (initializeBacking) {
			_backingBounds = currentBounds;
			_backingPixels.resize(_backingBounds.width() * _backingBounds.height());
		}
		const int captureTop = initializeBacking ? _backingBounds.top :
			MAX<int>(_backingBounds.top, kSceneTop);
		const int captureBottom = initializeBacking ? _backingBounds.bottom :
			MIN<int>(_backingBounds.bottom, kSceneBottom);
		for (int y = captureTop; y < captureBottom; ++y) {
			memcpy(_backingPixels.data() +
				(y - _backingBounds.top) * _backingBounds.width(),
				screen->getBasePtr(_backingBounds.left, y), _backingBounds.width());
		}
		debugC(11, kDebugDialogue,
			"Ripper: captured dialogue chooser backing bounds=%d,%d,%d,%d",
			_backingBounds.left, _backingBounds.top,
			_backingBounds.width(), _backingBounds.height());
	}
	for (uint visibleRow = 0; visibleRow < kVisibleChoiceCount; ++visibleRow) {
		const uint choiceIndex = _firstVisibleChoice + visibleRow;
		const int row = _chooserBounds.top + visibleRow * kChoiceRowHeight;
		const bool selected = choiceIndex == _selectedChoice;
		for (int y = row; y < row + kChoiceRowHeight; ++y)
			memset(screen->getBasePtr(_chooserBounds.left, y),
				selected ? kSelectedBackgroundColor : kNormalBackgroundColor,
				_chooserBounds.width());
		if (choiceIndex >= _choices.size())
			continue;
		int x = _chooserBounds.left + kChoiceHorizontalPadding;
		const int textY = row + (kChoiceRowHeight - _font.lineHeight) / 2;
		for (uint c = 0; c < _choices[choiceIndex].text.size() &&
				x < _chooserBounds.right - kChoiceHorizontalPadding; ++c) {
			const byte ch = (byte)_choices[choiceIndex].text[c];
			if (ch == ' ') {
				x += _font.spaceWidth;
				continue;
			}
			if (ch < _font.firstCharacter || ch >= _font.firstCharacter + _font.glyphs.size())
				continue;
			const BitmapFontGlyph &glyph = _font.glyphs[ch - _font.firstCharacter];
			for (int gy = 0; gy < glyph.height; ++gy) {
				for (int gx = 0; gx < glyph.width; ++gx) {
					const byte pixel = _font.pixels[glyph.pixelOffset + gy * glyph.width + gx];
					if (pixel != _font.transparentColor)
						*(byte *)screen->getBasePtr(x + glyph.xOffset + gx,
							textY + glyph.yOffset + gy) =
								selected ? kSelectedTextColor : kNormalTextColor;
				}
			}
			x += glyph.xOffset + glyph.width + _font.characterSpacing;
		}
	}
	g_system->unlockScreen();

	if (_choices.size() > kVisibleChoiceCount) {
		drawBitmap(_arrowFrames[_hoveredArrow == 1 && _firstVisibleChoice > 0 ? 1 : 0],
			_upArrowBounds.left, _upArrowBounds.top);
		drawBitmap(_arrowFrames[_hoveredArrow == 2 &&
				_firstVisibleChoice + kVisibleChoiceCount < _choices.size() ? 3 : 2],
			_downArrowBounds.left, _downArrowBounds.top);
	}
}

bool DialogueChooser::execute(const CompiledScript &script, const ScriptCommand &command,
		bool includeChoice) {
	if (command.opcode == kAddDialogueChoice ||
		command.opcode == kAddConditionalDialogueChoice) {
		if (command.arguments.size() < 2 || command.arguments[0].data.empty())
			return false;
		Choice choice;
		for (uint i = 0; i < command.arguments[0].data.size() &&
				command.arguments[0].data[i] != 0; ++i)
			choice.text += (char)command.arguments[0].data[i];
		choice.result = command.arguments[1].value & 0xffff;
		if (includeChoice) {
			appendChoice(choice.text, choice.result);
			debugC(1, kDebugDialogue,
				"Ripper: dialogue choice appended index=%u result=%u text='%s'",
				_choices.size() - 1, choice.result, choice.text.c_str());
		} else {
			debugC(1, kDebugDialogue,
				"Ripper: dialogue choice omitted result=%u reason=response-played text='%s'",
				choice.result, choice.text.c_str());
		}
		return true;
	}

	if (command.opcode == kStartDialogue) {
		if (!activateChoices("scene-script"))
			return false;
		debugC(1, kDebugDialogue,
			"Ripper: dialogue chooser activated script='%s' offset=0x%x "
				"selector=%u choices=%u",
			script.getMemberName().c_str(), command.offset, command.selector, _choices.size());
		for (uint i = 0; i < _choices.size(); ++i)
			debugC(2, kDebugDialogue, "Ripper: dialogue choice index=%u result=%u text='%s'",
				i, _choices[i].result, _choices[i].text.c_str());
		return true;
	}

	debugC(1, kDebugDialogue,
		"Ripper: dialogue command script='%s' offset=0x%x selector=%u arguments=%u",
		script.getMemberName().c_str(), command.offset, command.selector,
		command.arguments.size());
	for (uint i = 0; i < command.arguments.size(); ++i) {
		const ScriptArgument &argument = command.arguments[i];
		Common::String text;
		for (uint j = 0; j < argument.data.size() && argument.data[j] != 0; ++j)
			text += (char)argument.data[j];
		debugC(2, kDebugDialogue,
			"Ripper: dialogue argument=%u type=%u value=0x%x bytes=%u text='%s'",
			i, argument.type, argument.value, argument.data.size(), text.c_str());
	}
	return true;
}

void DialogueChooser::appendChoice(const Common::String &text, uint16 result) {
	Choice choice;
	choice.text = text;
	choice.result = result;
	_choices.push_back(choice);
}

bool DialogueChooser::activateChoices(const char *source) {
	if (_choices.empty()) {
		debugC(2, kDebugDialogue,
			"Ripper: dialogue chooser activation skipped source=%s reason=no-choices",
			source);
		clearPending();
		return true;
	}
	_pending = true;
	_selectedChoice = 0;
	_firstVisibleChoice = 0;
	_hoveredArrow = 0;
	_backingBounds = Common::Rect();
	_backingPixels.clear();
	updateLayout();
	rebuildPresentationBands("chooser-activation");
	debugC(1, kDebugDialogue,
		"Ripper: activated shared dialogue chooser source=%s choices=%u bounds=%d,%d,%d,%d",
		source, _choices.size(), _chooserBounds.left, _chooserBounds.top,
		_chooserBounds.width(), _chooserBounds.height());
	return true;
}

bool DialogueChooser::selectResult(uint16 result, const char *source) {
	if (!_pending)
		return false;
	for (uint choiceIndex = 0; choiceIndex < _choices.size(); ++choiceIndex) {
		if (_choices[choiceIndex].result != result)
			continue;
		_selectedChoice = choiceIndex;
		if (_selectedChoice < _firstVisibleChoice)
			_firstVisibleChoice = _selectedChoice;
		else if (_selectedChoice >= _firstVisibleChoice + kVisibleChoiceCount)
			_firstVisibleChoice = _selectedChoice - kVisibleChoiceCount + 1;
		debugC(2, kDebugDialogue,
			"Ripper: dialogue initial selection source=%s index=%u result=%u text='%s'",
			source, _selectedChoice, result, _choices[_selectedChoice].text.c_str());
		return true;
	}
	return false;
}

bool DialogueChooser::service(const MouseState &mouse, uint &result) {
	if (!_pending || _choices.empty())
		return false;
	updateHover(mouse.position);
	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return false;
	if (_choices.size() > kVisibleChoiceCount && _upArrowBounds.contains(mouse.position)) {
		if (_firstVisibleChoice > 0) {
			--_firstVisibleChoice;
			if (_selectedChoice >= _firstVisibleChoice + kVisibleChoiceCount)
				_selectedChoice = _firstVisibleChoice + kVisibleChoiceCount - 1;
			debugC(1, kDebugDialogue,
				"Ripper: dialogue scrolled up firstVisible=%u selected=%u",
				_firstVisibleChoice, _selectedChoice);
		}
		return false;
	}
	if (_choices.size() > kVisibleChoiceCount && _downArrowBounds.contains(mouse.position)) {
		if (_firstVisibleChoice + kVisibleChoiceCount < _choices.size()) {
			++_firstVisibleChoice;
			if (_selectedChoice < _firstVisibleChoice)
				_selectedChoice = _firstVisibleChoice;
			debugC(1, kDebugDialogue,
				"Ripper: dialogue scrolled down firstVisible=%u selected=%u",
				_firstVisibleChoice, _selectedChoice);
		}
		return false;
	}
	if (!_chooserBounds.contains(mouse.position))
		return false;
	const uint visibleRow = (mouse.position.y - _chooserBounds.top) / kChoiceRowHeight;
	const uint choiceIndex = _firstVisibleChoice + visibleRow;
	if (visibleRow >= kVisibleChoiceCount || choiceIndex >= _choices.size())
		return false;
	return selectChoice(choiceIndex, result, "mouse");
}

bool DialogueChooser::serviceKeyboard(uint16 command, uint &result) {
	if (!_pending || _choices.empty())
		return false;
	const uint previousChoice = _selectedChoice;
	if (command == kUpCommand) {
		if (_selectedChoice > 0)
			--_selectedChoice;
		if (_selectedChoice < _firstVisibleChoice)
			_firstVisibleChoice = _selectedChoice;
	} else if (command == kDownCommand) {
		if (_selectedChoice + 1 < _choices.size())
			++_selectedChoice;
		if (_selectedChoice >= _firstVisibleChoice + kVisibleChoiceCount)
			_firstVisibleChoice = _selectedChoice - kVisibleChoiceCount + 1;
	} else if (command == kEnterCommand) {
		return selectChoice(_selectedChoice, result, "keyboard");
	} else {
		return false;
	}
	if (_selectedChoice != previousChoice) {
		debugC(2, kDebugDialogue,
			"Ripper: dialogue keyboard selection command=0x%04x index=%u result=%u text='%s'",
			command, _selectedChoice, _choices[_selectedChoice].result,
			_choices[_selectedChoice].text.c_str());
	}
	return false;
}

void DialogueChooser::updateHover(const Common::Point &point) {
	if (!_pending || _choices.empty())
		return;
	_hoveredArrow = _upArrowBounds.contains(point) ? 1 :
		(_downArrowBounds.contains(point) ? 2 : 0);
	if (!_chooserBounds.contains(point))
		return;
	const uint visibleRow = (point.y - _chooserBounds.top) / kChoiceRowHeight;
	const uint choiceIndex = _firstVisibleChoice + visibleRow;
	if (visibleRow >= kVisibleChoiceCount || choiceIndex >= _choices.size() ||
		choiceIndex == _selectedChoice)
		return;
	_selectedChoice = choiceIndex;
	debugC(2, kDebugDialogue,
		"Ripper: dialogue hover index=%u result=%u text='%s' point=%d,%d",
		_selectedChoice, _choices[_selectedChoice].result,
		_choices[_selectedChoice].text.c_str(), point.x, point.y);
}

bool DialogueChooser::contains(const Common::Point &point) const {
	return _pending && !_choices.empty() && (_chooserBounds.contains(point) ||
		(_choices.size() > kVisibleChoiceCount &&
			(_upArrowBounds.contains(point) || _downArrowBounds.contains(point))));
}

void DialogueChooser::clearPending() {
	_pending = false;
	_choices.clear();
	_firstVisibleChoice = 0;
	_hoveredArrow = 0;
	_backingBounds = Common::Rect();
	_backingPixels.clear();
}

void DialogueChooser::dismissForSceneTransition(const char *reason) {
	if (!_pending && _choices.empty())
		return;
	const uint choiceCount = _choices.size();
	const bool restored = _pending && restoreBacking();
	clearPending();
	rebuildPresentationBands(reason);
	g_system->updateScreen();
	debugC(1, kDebugDialogue,
		"Ripper: dismissed dialogue chooser before scene transition reason=%s choices=%u backing=%d",
		reason, choiceCount, restored);
}

bool DialogueChooser::syncGame(Common::Serializer &serializer) {
	byte pending = _pending ? 1 : 0;
	serializer.syncAsByte(pending);

	uint32 choiceCount = _choices.size();
	serializer.syncAsUint32LE(choiceCount);
	if (serializer.isLoading()) {
		if (choiceCount > 256)
			return false;
		_backingBounds = Common::Rect();
		_backingPixels.clear();
		_choices.clear();
		_choices.resize(choiceCount);
	}
	for (uint i = 0; i < choiceCount; ++i) {
		serializer.syncString(_choices[i].text);
		serializer.syncAsUint16LE(_choices[i].result);
		if (serializer.isLoading() && _choices[i].text.size() > 1024)
			return false;
	}

	serializer.syncAsUint32LE(_selectedChoice);
	serializer.syncAsUint32LE(_firstVisibleChoice);
	if (serializer.isLoading()) {
		_pending = pending != 0;
		_hoveredArrow = 0;
		if (_choices.empty()) {
			_pending = false;
			_selectedChoice = 0;
			_firstVisibleChoice = 0;
		} else {
			if (_selectedChoice >= _choices.size() || _firstVisibleChoice >= _choices.size())
				return false;
			updateLayout();
		}
	}
	return !serializer.err();
}

uint DialogueChooser::measureText(const Common::String &text) const {
	uint width = 0;
	for (uint i = 0; i < text.size(); ++i) {
		const byte ch = (byte)text[i];
		if (ch == ' ') {
			width += _font.spaceWidth;
			continue;
		}
		if (ch < _font.firstCharacter || ch >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[ch - _font.firstCharacter];
		width += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
	return width;
}

void DialogueChooser::updateLayout() {
	uint textWidth = 0;
	for (uint i = 0; i < _choices.size(); ++i)
		textWidth = MAX(textWidth, measureText(_choices[i].text));
	const int width = MIN<int>(textWidth + kChoiceHorizontalPadding * 2, 620);
	const int left = (640 - width) / 2;
	_chooserBounds = Common::Rect(left, kChoiceTop, left + width,
		kChoiceTop + kVisibleChoiceCount * kChoiceRowHeight);
	if (_arrowFrames.size() < 4)
		return;
	const int arrowLeft = _chooserBounds.right + kArrowGap;
	_upArrowBounds = Common::Rect(arrowLeft, _chooserBounds.top,
		arrowLeft + _arrowFrames[0].width, _chooserBounds.top + _arrowFrames[0].height);
	_downArrowBounds = Common::Rect(arrowLeft,
		_chooserBounds.bottom - _arrowFrames[2].height,
		arrowLeft + _arrowFrames[2].width, _chooserBounds.bottom);
}

Common::Rect DialogueChooser::visualBounds() const {
	Common::Rect bounds = _chooserBounds;
	if (_choices.size() > kVisibleChoiceCount && _arrowFrames.size() >= 4) {
		bounds.left = MIN(bounds.left, MIN(_upArrowBounds.left, _downArrowBounds.left));
		bounds.top = MIN(bounds.top, MIN(_upArrowBounds.top, _downArrowBounds.top));
		bounds.right = MAX(bounds.right, MAX(_upArrowBounds.right, _downArrowBounds.right));
		bounds.bottom = MAX(bounds.bottom, MAX(_upArrowBounds.bottom, _downArrowBounds.bottom));
	}
	return bounds;
}

bool DialogueChooser::restoreBacking() {
	if (_backingBounds.isEmpty() || _backingPixels.size() !=
			(uint)(_backingBounds.width() * _backingBounds.height()))
		return false;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (int y = 0; y < _backingBounds.height(); ++y) {
		memcpy(screen->getBasePtr(_backingBounds.left, _backingBounds.top + y),
			_backingPixels.data() + y * _backingBounds.width(), _backingBounds.width());
	}
	g_system->unlockScreen();
	return true;
}

bool DialogueChooser::selectChoice(uint choiceIndex, uint &result, const char *source) {
	if (!_pending || choiceIndex >= _choices.size())
		return false;
	_selectedChoice = choiceIndex;
	result = _choices[_selectedChoice].result;
	const Common::String text = _choices[_selectedChoice].text;
	const bool restored = restoreBacking();
	debugC(1, kDebugDialogue,
		"Ripper: dialogue selected source=%s index=%u result=%u text='%s' backing=%d",
		source, _selectedChoice, result, text.c_str(), restored);
	clearPending();
	rebuildPresentationBands("chooser-selection");
	g_system->updateScreen();
	return true;
}

void DialogueChooser::rebuildPresentationBands(const char *reason) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w < 640 || screen->h < 400) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	// HandleSceneEntryChoiceListLifecycle at 0x1523d and RunMediaPresentation
	// at 0x168af restore the display around chooser activation and controlled
	// media completion. The scene movie only owns y=50..349, so rebuild the
	// uncovered indexed bands instead of retaining pixels from the full-screen
	// response under the next scene palette.
	for (int y = 0; y < kSceneTop; ++y)
		memset(screen->getBasePtr(0, y), 0, 640);
	for (int y = kSceneBottom; y < 400; ++y)
		memset(screen->getBasePtr(0, y), 0, 640);
	g_system->unlockScreen();
	debugC(2, kDebugDialogue,
		"Ripper: rebuilt dialogue presentation bands reason=%s top=0..%d bottom=%d..399",
		reason, kSceneTop - 1, kSceneBottom);
}

void DialogueChooser::drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (uint row = 0; row < bitmap.height; ++row) {
		byte *destination = (byte *)screen->getBasePtr(x, y + row);
		const byte *source = bitmap.pixels.data() + row * bitmap.width;
		for (uint column = 0; column < bitmap.width; ++column) {
			if (source[column] != bitmap.transparentColor)
				destination[column] = source[column];
		}
	}
	g_system->unlockScreen();
}

} // End of namespace Ripper
