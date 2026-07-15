/* ScummVM - Graphic Adventure Engine */

#include "ripper/dialogue.h"

#include "common/debug.h"
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

bool DialogueManager::initialize(ResourceManager &resources) {
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
	return true;
}

void DialogueManager::draw() const {
	if (!_pending || _choices.empty())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (uint visibleRow = 0; visibleRow < kVisibleChoiceCount; ++visibleRow) {
		const uint choiceIndex = _firstVisibleChoice + visibleRow;
		const int row = _chooserBounds.top + visibleRow * kChoiceRowHeight;
		const bool selected = choiceIndex == _selectedChoice;
		for (int y = row; y < row + kChoiceRowHeight; ++y)
			memset(screen->getBasePtr(_chooserBounds.left, y), selected ? 253 : 0,
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
							textY + glyph.yOffset + gy) = selected ? 0 : 4;
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

bool DialogueManager::execute(const CompiledScript &script, const ScriptCommand &command,
		bool includeChoice) {
	if (command.opcode == 0x15 || command.opcode == 0x16) {
		if (command.arguments.size() < 2 || command.arguments[0].data.empty())
			return false;
		Choice choice;
		for (uint i = 0; i < command.arguments[0].data.size() &&
				command.arguments[0].data[i] != 0; ++i)
			choice.text += (char)command.arguments[0].data[i];
		choice.result = command.arguments[1].value & 0xffff;
		if (includeChoice) {
			_choices.push_back(choice);
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

	if (command.opcode == 0x17) {
		_pending = true;
		_selectedChoice = 0;
		_firstVisibleChoice = 0;
		_hoveredArrow = 0;
		updateLayout();
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

bool DialogueManager::service(const MouseState &mouse, uint &result) {
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
	_selectedChoice = choiceIndex;
	result = _choices[_selectedChoice].result;
	debugC(1, kDebugDialogue,
		"Ripper: dialogue selected index=%u resultFrame=%u text='%s'",
		_selectedChoice, result, _choices[_selectedChoice].text.c_str());
	_pending = false;
	_choices.clear();
	return true;
}

void DialogueManager::updateHover(const Common::Point &point) {
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

bool DialogueManager::contains(const Common::Point &point) const {
	return _pending && !_choices.empty() && (_chooserBounds.contains(point) ||
		(_choices.size() > kVisibleChoiceCount &&
			(_upArrowBounds.contains(point) || _downArrowBounds.contains(point))));
}

void DialogueManager::clearPending() {
	_pending = false;
	_choices.clear();
	_firstVisibleChoice = 0;
	_hoveredArrow = 0;
}

uint DialogueManager::measureText(const Common::String &text) const {
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

void DialogueManager::updateLayout() {
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

void DialogueManager::drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const {
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
