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

#include "ripper/modal_dialog.h"

#include "ripper/modal/constants.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/ripper.h"
#include "ripper/script.h"

namespace Ripper {

ModalDialogManager::ModalDialogManager(RipperEngine *engine) :
		_engine(engine), _textEntryStyle(kMenubPresentation),
		_textEntryMaximumLength(0), _textEntryHelpResourceId(0),
		_textEntryFirstVisible(0), _textEntryCursorPosition(0),
		_textEntryNextCaretMillis(0), _textEntryOverwrite(false),
		_textEntryCaretVisible(false), _textEntryActive(false),
		_textEntryRestoreCursor(false), _initialized(false) {
}

bool ModalDialogManager::initialize(ResourceManager &resources) {
	if (!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
			!resources.loadInterfaceBitmapFont("7pt_font.fnt", _primaryFont) ||
			!resources.loadGameText(_gameText))
		return false;

	_skin.clear();
	_wacSkin.clear();
	_primaryScrollSkin.clear();
	_modalPalette.clear();
	for (uint frameIndex = 0; frameIndex < kModalSkinFrameCount; ++frameIndex) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("menub%u", frameIndex), sequence) ||
				sequence.frames.empty())
			return false;
		_skin.push_back(Common::move(sequence.frames.front()));
		if (_modalPalette.empty() && _skin.back().palette.size() >= 256 * 3)
			_modalPalette = _skin.back().palette;
	}
	for (uint frameIndex = 0; frameIndex < kWacModalSkinFrameCount; ++frameIndex) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("wacmnu%u", frameIndex), sequence) ||
				sequence.frames.empty())
			return false;
		_wacSkin.push_back(Common::move(sequence.frames.front()));
	}
	for (uint frameIndex = 0; frameIndex < kPrimaryScrollSkinFrameCount;
			++frameIndex) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("mnarrow%u.bbm", frameIndex),
				sequence) ||
				sequence.frames.empty())
			return false;
		_primaryScrollSkin.push_back(Common::move(sequence.frames.front()));
	}

	_initialized = true;
	debugC(1, kDebugScene,
		"Ripper: initialized modal text dialogs menubFrames=%u "
		"wacmnuFrames=%u primaryScrollFrames=%u fonts=small.fnt,"
		"7pt_font.fnt strings=%u",
		_skin.size(), _wacSkin.size(), _primaryScrollSkin.size(),
		_gameText.size());
	debugC(3, kDebugScene,
		"Ripper: modal text font first=%u glyphs=%u lineHeight=%u spacing=%u spaceWidth=%u "
		"transparent=%u paletteBytes=%u",
		_font.firstCharacter, _font.glyphs.size(), _font.lineHeight,
		_font.characterSpacing, _font.spaceWidth, _font.transparentColor,
		_modalPalette.size());
	return true;
}

const Common::String &ModalDialogManager::resourceString(uint resourceId) const {
	static const Common::String empty;
	if (resourceId == 0 || resourceId > _gameText.size())
		return empty;
	return _gameText[resourceId - 1];
}

bool ModalDialogManager::captureDisplay() {
	return _savedDisplay.capture();
}

void ModalDialogManager::applyModalPalette() {
	if (_modalPalette.size() < 256 * 3)
		return;

	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	// InitializeSharedPresentationTemplates at 0x1196f captures these ranges
	// from the MENUB palette through CaptureSharedDisplayPalettePatch at
	// 0x205a9. ApplySharedDisplayPalettePatch at 0x205d0 restores them for
	// chooser text and frame pixels without replacing the active scene palette.
	memset(palette, 0, 3);
	memcpy(palette + 4 * 3, _modalPalette.data() + 4 * 3, 6 * 3);
	memcpy(palette + 246 * 3, _modalPalette.data() + 246 * 3, 10 * 3);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void ModalDialogManager::restoreDisplay() {
	if (!_savedDisplay.isValid())
		return;

	_savedDisplay.restore(true, false);
	_engine->getCursor()->refresh();
	g_system->updateScreen();
	_savedDisplay.clear();
}

void ModalDialogManager::drawTextEntry(const Common::String &prompt,
		const Common::String &text, uint firstVisible, uint cursorPosition,
		bool caretVisible, const Common::Rect &bounds, PresentationStyle style) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	const bool wacStyle = style == kWacPresentation;
	const bool sceneEntryStyle = style == kSceneEntryPresentation;
	const byte backgroundColor = wacStyle ? kWacModalBackgroundColor :
		(sceneEntryStyle ? kSceneEntryBackgroundColor : kModalBackgroundColor);
	const byte textColor = wacStyle ? kWacModalTextColor :
		(sceneEntryStyle ? kSceneEntryTextColor : kModalTextColor);
	if (wacStyle) {
		for (int y = bounds.top; y < bounds.bottom; ++y)
			memset(screen->getBasePtr(bounds.left, y), backgroundColor,
				bounds.width());
		drawFrame(pixels, screen->pitch, bounds, style);
	} else if (sceneEntryStyle) {
		// ConfigureSceneEntryChooserLayout at 0x18740 selects the unskinned
		// primary template at 0x8a2de for layout variant 2. Its five-pixel
		// inset places the editable row over the recess already rendered by
		// the scene movie; redraws restore that row with palette index 0.
		const int rowBottom = MIN(bounds.top + kTextEntryPadding +
			kModalRowHeight, (int)bounds.bottom);
		for (int y = bounds.top + kTextEntryPadding; y < rowBottom; ++y)
			memset(screen->getBasePtr(bounds.left + kTextEntryPadding, y),
				backgroundColor, bounds.width() - kTextEntryPadding * 2);
	} else {
		drawFrame(pixels, screen->pitch, bounds, style);
		for (int y = bounds.top + 2; y < bounds.bottom - 2; ++y)
			memset(screen->getBasePtr(bounds.left + kTextEntryPadding, y),
				backgroundColor, bounds.width() - kTextEntryPadding * 2);
	}

	int textTop;
	int textLeft;
	int textRight;
	if (wacStyle) {
		// CreateTextEntryChooserControl at 0x59d93 passes the prompt as the
		// chooser heading. BuildChooserControlVisuals centers it in the WAC
		// template's 20-pixel heading, above the single editable row.
		const int promptWidth = measureText(prompt);
		const int contentWidth =
			bounds.width() - kWacModalLeftPadding - kWacModalRightPadding;
		drawText(pixels, screen->pitch,
			bounds.left + kWacModalLeftPadding +
				(contentWidth - promptWidth) / 2,
			bounds.top +
				(kWacModalHeadingTopPadding - _font.lineHeight) / 2,
			prompt, textColor);
		textTop = bounds.top + kWacModalHeadingTopPadding +
			kModalTextVerticalInset +
			(kModalRowHeight - _font.lineHeight) / 2;
		textLeft = bounds.left + kWacModalLeftPadding;
		textRight = bounds.right - kWacModalRightPadding;
	} else {
		textTop = bounds.top + (bounds.height() - _font.lineHeight) / 2;
		textLeft = bounds.left + kTextEntryPadding;
		textRight = bounds.right - kTextEntryPadding;
		if (!prompt.empty()) {
			drawText(pixels, screen->pitch, textLeft, textTop, prompt, textColor);
			textLeft += measureText(prompt) + _font.spaceWidth;
		}
	}
	const uint availableWidth = textRight - textLeft;
	uint endVisible = firstVisible;
	while (endVisible < text.size()) {
		const Common::String candidate =
			text.substr(firstVisible, endVisible - firstVisible + 1);
		if (measureText(candidate) > availableWidth)
			break;
		++endVisible;
	}
	const Common::String visible = text.substr(firstVisible,
		endVisible - firstVisible);
	drawText(pixels, screen->pitch, textLeft, textTop, visible, textColor);
	if (caretVisible && cursorPosition >= firstVisible && cursorPosition <= endVisible) {
		const int caretLeft = textLeft + measureText(text.substr(firstVisible,
			cursorPosition - firstVisible));
		drawText(pixels, screen->pitch, caretLeft, textTop, "_", textColor);
	}

	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

void ModalDialogManager::drawBinaryPrompt(const Common::String &prompt,
		const Common::String *options, uint selectedIndex,
		const Common::Rect &bounds) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	drawFrame(pixels, screen->pitch, bounds, kMenubPresentation);
	for (int y = bounds.top + 2; y < bounds.bottom - 2; ++y)
		memset(screen->getBasePtr(bounds.left + kModalLeftPadding, y),
			kModalBackgroundColor,
			bounds.width() - kModalLeftPadding - kModalRightPadding);
	for (int y = bounds.top + 2;
			y < bounds.top + kModalHeadingTopPadding; ++y)
		memset(screen->getBasePtr(bounds.left + kModalLeftPadding, y),
			kModalHeadingColor,
			bounds.width() - kModalLeftPadding - kModalRightPadding);

	const int contentWidth =
		bounds.width() - kModalLeftPadding - kModalRightPadding;
	drawText(pixels, screen->pitch,
		bounds.left + kModalLeftPadding +
			(contentWidth - measureText(prompt)) / 2,
		bounds.top +
			(kModalHeadingTopPadding - _font.lineHeight) / 2,
		prompt, kModalTitleColor);
	for (uint option = 0; option < kBinaryPromptOptionCount; ++option) {
		const int rowTop = bounds.top + kModalHeadingTopPadding +
			option * kModalRowHeight;
		if (option == selectedIndex) {
			for (int y = rowTop; y < rowTop + kModalRowHeight; ++y)
				memset(screen->getBasePtr(
					bounds.left + kModalLeftPadding, y),
					kModalSelectedBackgroundColor, contentWidth);
		}
		drawText(pixels, screen->pitch,
			bounds.left + kModalLeftPadding +
				kModalTextHorizontalInset,
			rowTop + (kModalRowHeight - _font.lineHeight) / 2,
			options[option], kModalTextColor);
	}

	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

uint ModalDialogManager::textEntryCursorFromPoint(const Common::String &text,
		uint firstVisible, int x, const Common::Rect &bounds) const {
	const bool wacStyle = _textEntryStyle == kWacPresentation;
	const int leftPadding = wacStyle ? kWacModalLeftPadding : kTextEntryPadding;
	const int rightPadding = wacStyle ? kWacModalRightPadding : kTextEntryPadding;
	int textLeft = bounds.left + leftPadding;
	if (!wacStyle && !_textEntryPrompt.empty())
		textLeft += measureText(_textEntryPrompt) + _font.spaceWidth;
	const int relativeX = x - textLeft;
	if (relativeX <= 0)
		return firstVisible;
	for (uint position = firstVisible; position < text.size(); ++position) {
		const uint left = measureText(text.substr(firstVisible,
			position - firstVisible));
		const uint right = measureText(text.substr(firstVisible,
			position - firstVisible + 1));
		if ((uint)relativeX < left + (right - left) / 2)
			return position;
		if (right >= (uint)bounds.width() - leftPadding - rightPadding)
			return position + 1;
	}
	return text.size();
}

void ModalDialogManager::updateTextEntryFirstVisible(const Common::Rect &bounds) {
	const bool wacStyle = _textEntryStyle == kWacPresentation;
	const int leftPadding = wacStyle ? kWacModalLeftPadding : kTextEntryPadding;
	const int rightPadding = wacStyle ? kWacModalRightPadding : kTextEntryPadding;
	int textLeft = bounds.left + leftPadding;
	if (!wacStyle && !_textEntryPrompt.empty())
		textLeft += measureText(_textEntryPrompt) + _font.spaceWidth;
	const uint availableWidth = bounds.right - rightPadding - textLeft;
	const uint caretWidth = measureText("_");
	if (_textEntryCursorPosition < _textEntryFirstVisible)
		_textEntryFirstVisible = _textEntryCursorPosition;
	while (_textEntryFirstVisible < _textEntryCursorPosition &&
			measureText(_textEntryText.substr(_textEntryFirstVisible,
				_textEntryCursorPosition - _textEntryFirstVisible)) + caretWidth >
				availableWidth)
		++_textEntryFirstVisible;
	while (_textEntryFirstVisible != 0 &&
			measureText(_textEntryText.substr(_textEntryFirstVisible - 1,
				_textEntryCursorPosition - _textEntryFirstVisible + 1)) + caretWidth <=
				availableWidth)
		--_textEntryFirstVisible;
}

bool ModalDialogManager::beginTextEntry(const Common::String &prompt,
		uint maximumLength, uint helpResourceId, const char *source,
		PresentationStyle style, const Common::Rect &bounds) {
	if (!_initialized || _textEntryActive) {
		warning("Ripper: could not begin scene text request source='%s' active=%d",
			source, _textEntryActive);
		return false;
	}
	_textEntryPrompt = prompt;
	_textEntryText.clear();
	_textEntrySource = source;
	_textEntryStyle = style;
	_textEntryBounds = bounds.isEmpty() ? Common::Rect(kTextEntryLeft,
		kTextEntryTop, kTextEntryLeft + kTextEntryWidth,
		kTextEntryTop + kTextEntryHeight) : bounds;
	_textEntryMaximumLength = maximumLength;
	_textEntryHelpResourceId = helpResourceId;
	_textEntryFirstVisible = 0;
	_textEntryCursorPosition = 0;
	_textEntryOverwrite = false;
	_textEntryCaretVisible = true;
	_textEntryNextCaretMillis = g_system->getMillis() + kTextEntryCaretBlinkMillis;
	_textEntryRestoreCursor = _engine->getCursor()->isVisible();
	_textEntryActive = true;
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->update(kModalCursor);
	_engine->getCursor()->setVisible(true);
	if (_textEntryStyle == kMenubPresentation)
		applyModalPalette();
	drawTextEntry(_textEntryPrompt, _textEntryText, _textEntryFirstVisible,
		_textEntryCursorPosition, _textEntryCaretVisible, _textEntryBounds,
		_textEntryStyle);
	debugC(1, kDebugScene,
		"Ripper: entered scene text request source='%s' control=0x4e2 style=%s bounds=%d,%d,%d,%d maximumLength=%u helpResource=%u",
		source, _textEntryStyle == kWacPresentation ? "wacmnu" :
			(_textEntryStyle == kSceneEntryPresentation ? "scene-entry" : "menub"),
		_textEntryBounds.left, _textEntryBounds.top, _textEntryBounds.width(),
		_textEntryBounds.height(), maximumLength, helpResourceId);
	return true;
}

void ModalDialogManager::finishTextEntry(Common::String &text) {
	text = _textEntryText;
	_textEntryActive = false;
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(_textEntryRestoreCursor);
}

ModalDialogManager::TextEntryResult ModalDialogManager::serviceTextEntry(
		Common::String &text) {
	if (!_textEntryActive)
		return kTextEntryFailed;
	if (_engine->getInput()->pollEvents()) {
		_engine->quitGame();
		finishTextEntry(text);
		return kTextEntryFailed;
	}

	const Common::Rect &bounds = _textEntryBounds;
	TextEntryResult result = kTextEntryPending;
	bool edited = false;
	while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			debugC(3, kDebugInput,
				"Ripper: scene text request command=0x%04x cursor=%u length=%u overwrite=%d",
				command, _textEntryCursorPosition, _textEntryText.size(),
				_textEntryOverwrite);
			if (command == 0x0d) {
				result = kTextEntryAccepted;
				break;
			}
			if (command == 0x1b) {
				result = kTextEntryCancelled;
				break;
			}
			if (command == 0x3b00) {
				const bool sceneEntryStyle =
					_textEntryStyle == kSceneEntryPresentation;
				const PresentationStyle helpStyle = sceneEntryStyle ?
					kMenubPresentation : _textEntryStyle;
				if (!_engine->getModalDialog()->run(_textEntryHelpResourceId, true,
						helpStyle, _textEntryStyle == kWacPresentation ?
						kPreserveActivePalette : kApplyModalPalette)) {
					result = kTextEntryFailed;
					break;
				}
				if (_textEntryStyle == kMenubPresentation)
					applyModalPalette();
				continue;
			}

			const uint previousLength = _textEntryText.size();
			switch (command) {
			case 0x08:
				if (_textEntryCursorPosition != 0) {
					_textEntryText.deleteChar(--_textEntryCursorPosition);
					edited = true;
				}
				break;
			case 0x4700:
				_textEntryCursorPosition = 0;
				edited = true;
				break;
			case 0x4b00:
				if (_textEntryCursorPosition != 0) {
					--_textEntryCursorPosition;
					edited = true;
				}
				break;
			case 0x4d00:
				if (_textEntryCursorPosition < _textEntryText.size()) {
					++_textEntryCursorPosition;
					edited = true;
				}
				break;
			case 0x4f00:
				_textEntryCursorPosition = _textEntryText.size();
				edited = true;
				break;
			case 0x5200:
				_textEntryOverwrite = !_textEntryOverwrite;
				break;
			case 0x5300:
				if (_textEntryCursorPosition < _textEntryText.size()) {
					_textEntryText.deleteChar(_textEntryCursorPosition);
					edited = true;
				}
				break;
			default:
				if (command <= 0xff && (command == ' ' ||
						(command >= _font.firstCharacter &&
						command < _font.firstCharacter + _font.glyphs.size()))) {
					if (_textEntryOverwrite &&
							_textEntryCursorPosition < _textEntryText.size()) {
						_textEntryText.setChar((char)command,
							_textEntryCursorPosition++);
						edited = true;
					} else if (_textEntryText.size() < _textEntryMaximumLength) {
						_textEntryText.insertChar((char)command,
							_textEntryCursorPosition++);
						edited = true;
					}
				}
				break;
			}
			if (_textEntryText.size() != previousLength)
				debugC(2, kDebugInput,
					"Ripper: scene text request edited length=%u cursor=%u",
					_textEntryText.size(), _textEntryCursorPosition);
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	_engine->getCursor()->update(kModalCursor);
	if ((mouse.pressed & kMouseButtonLeft) != 0 && bounds.contains(mouse.position)) {
		_textEntryCursorPosition = textEntryCursorFromPoint(_textEntryText,
			_textEntryFirstVisible, mouse.position.x, bounds);
		edited = true;
		debugC(3, kDebugInput,
			"Ripper: scene text request caret point=%d,%d cursor=%u firstVisible=%u",
			mouse.position.x, mouse.position.y, _textEntryCursorPosition,
			_textEntryFirstVisible);
	}

	const uint32 now = g_system->getMillis();
	if (now >= _textEntryNextCaretMillis) {
		_textEntryCaretVisible = !_textEntryCaretVisible;
		_textEntryNextCaretMillis = now + kTextEntryCaretBlinkMillis;
	}
	if (edited) {
		_textEntryCaretVisible = true;
		_textEntryNextCaretMillis = now + kTextEntryCaretBlinkMillis;
	}
	updateTextEntryFirstVisible(bounds);
	if (result == kTextEntryPending) {
		if (_textEntryStyle == kMenubPresentation)
			applyModalPalette();
		drawTextEntry(_textEntryPrompt, _textEntryText, _textEntryFirstVisible,
			_textEntryCursorPosition, _textEntryCaretVisible, bounds,
			_textEntryStyle);
		return result;
	}

	const Common::String source = _textEntrySource;
	const uint length = _textEntryText.size();
	finishTextEntry(text);
	debugC(1, kDebugScene,
		"Ripper: exited scene text request source='%s' result=%d length=%u",
		source.c_str(), result, length);
	return result;
}

bool ModalDialogManager::run(uint bodyResourceId, bool retainSceneCursorRegions,
		PresentationStyle style, PaletteBehavior paletteBehavior) {
	const Common::String &title = resourceString(kModalTitleResourceId);
	const Common::String &body = resourceString(bodyResourceId);
	if (title.empty() || body.empty()) {
		warning("Ripper: could not present modal text resource=%u", bodyResourceId);
		return false;
	}
	return runTextInternal(title, body, bodyResourceId, "game text",
		retainSceneCursorRegions, style, paletteBehavior);
}

bool ModalDialogManager::runText(const Common::String &title,
		const Common::String &body, const char *source,
		bool retainSceneCursorRegions, PresentationStyle style,
		PaletteBehavior paletteBehavior) {
	return runTextInternal(title, body, 0, source, retainSceneCursorRegions,
		style, paletteBehavior);
}

bool ModalDialogManager::runBinaryPrompt(uint promptResourceId,
		bool defaultFirstOption) {
	const Common::String &prompt = resourceString(promptResourceId);
	Common::String options[kBinaryPromptOptionCount] = {
		resourceString(kBinaryPromptYesResourceId),
		resourceString(kBinaryPromptNoResourceId)
	};
	if (!_initialized || prompt.empty() || options[0].empty() ||
			options[1].empty() || !captureDisplay()) {
		warning("Ripper: could not present binary prompt resource=%u",
			promptResourceId);
		return false;
	}

	// RunBinaryPromptChooser at 0x1803c creates a two-row list using the
	// secondary chooser template at 0x8a392. Its auto-sized client is centered
	// and uses the template's five-pixel horizontal padding, 17-pixel heading,
	// and 14-pixel rows. A false default argument selects row one ("No").
	const uint contentWidth = MAX<uint>(measureText(prompt),
		MAX<uint>(measureText(options[0]), measureText(options[1])));
	const int width = contentWidth + kModalLeftPadding +
		kModalRightPadding + kModalTextHorizontalInset * 2;
	const int height = kModalHeadingTopPadding +
		kBinaryPromptOptionCount * kModalRowHeight + kModalBottomPadding;
	const int left = (640 - width) / 2;
	const int top = (400 - height) / 2;
	const Common::Rect bounds(left, top, left + width, top + height);
	uint selectedIndex = defaultFirstOption ? 0 : 1;
	int pressedIndex = -1;
	bool accepted = false;
	bool active = true;
	bool redraw = true;
	const bool restoreCursorVisible = _engine->getCursor()->isVisible();
	Common::Point cursorPoint =
		_engine->getInput()->peekMouseState().position;

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	applyModalPalette();
	_engine->getCursor()->update(kModalCursor);
	debugC(1, kDebugScene,
		"Ripper: entered binary prompt function=RunBinaryPromptChooser@0x1803c "
		"resource=%u prompt='%s' default=%u bounds=%d,%d,%d,%d",
		promptResourceId, prompt.c_str(), selectedIndex,
		bounds.left, bounds.top, bounds.width(), bounds.height());

	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			debugC(3, kDebugInput,
				"Ripper: binary prompt command=0x%04x selected=%u",
				command, selectedIndex);
			uint nextIndex = selectedIndex;
			switch (command) {
			case 0x1b:
				active = false;
				break;
			case 0x0d:
				accepted = true;
				active = false;
				break;
			case 0x4700:
			case 0x4800:
			case 'y':
			case 'Y':
				nextIndex = 0;
				break;
			case 0x4f00:
			case 0x5000:
			case 'n':
			case 'N':
				nextIndex = 1;
				break;
			default:
				break;
			}
			if (nextIndex != selectedIndex) {
				selectedIndex = nextIndex;
				redraw = true;
				debugC(2, kDebugInput,
					"Ripper: binary prompt keyboard selection=%u text='%s'",
					selectedIndex, options[selectedIndex].c_str());
			}
			if (!active)
				break;
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		cursorPoint = mouse.position;
		_engine->getCursor()->update(kModalCursor);
		int hoveredIndex = -1;
		for (uint option = 0; option < kBinaryPromptOptionCount; ++option) {
			const Common::Rect rowBounds(
				bounds.left + kModalLeftPadding,
				bounds.top + kModalHeadingTopPadding +
					option * kModalRowHeight,
				bounds.right - kModalRightPadding,
				bounds.top + kModalHeadingTopPadding +
					(option + 1) * kModalRowHeight);
			if (rowBounds.contains(mouse.position)) {
				hoveredIndex = option;
				break;
			}
		}
		if (hoveredIndex >= 0 && (uint)hoveredIndex != selectedIndex) {
			selectedIndex = hoveredIndex;
			redraw = true;
			debugC(2, kDebugInput,
				"Ripper: binary prompt hover selection=%u text='%s' point=%d,%d",
				selectedIndex, options[selectedIndex].c_str(),
				mouse.position.x, mouse.position.y);
		}
		if ((mouse.pressed & kMouseButtonLeft) != 0)
			pressedIndex = hoveredIndex;
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (pressedIndex >= 0 && pressedIndex == hoveredIndex) {
				selectedIndex = pressedIndex;
				accepted = true;
				active = false;
				debugC(2, kDebugInput,
					"Ripper: binary prompt mouse accepted selection=%u "
					"text='%s' point=%d,%d",
					selectedIndex, options[selectedIndex].c_str(),
					mouse.position.x, mouse.position.y);
			}
			pressedIndex = -1;
		}

		if (redraw) {
			drawBinaryPrompt(prompt, options, selectedIndex, bounds);
			redraw = false;
		} else {
			g_system->updateScreen();
		}
		g_system->delayMillis(10);
	}

	_engine->getInput()->discardMouseTransitions();
	restoreDisplay();
	if (!_engine->shouldQuit())
		_engine->getScripts()->updateModalSceneCursor(cursorPoint);
	_engine->getCursor()->setVisible(restoreCursorVisible);
	const bool firstOptionSelected = accepted && selectedIndex == 0;
	debugC(1, kDebugScene,
		"Ripper: exited binary prompt resource=%u accepted=%d selection=%u "
		"confirmed=%d quit=%d",
		promptResourceId, accepted, selectedIndex, firstOptionSelected,
		_engine->shouldQuit());
	return firstOptionSelected;
}

bool ModalDialogManager::runTextInternal(const Common::String &title,
		const Common::String &body, uint bodyResourceId, const char *source,
		bool retainSceneCursorRegions, PresentationStyle style,
		PaletteBehavior paletteBehavior) {
	if (!_initialized || title.empty() || !captureDisplay()) {
		warning("Ripper: could not present modal text source='%s'", source);
		return false;
	}

	const bool wacStyle = style == kWacPresentation;
	const int headingTopPadding = wacStyle ?
		kWacModalHeadingTopPadding : kModalHeadingTopPadding;
	const int bottomPadding = wacStyle ? kWacModalBottomPadding : kModalBottomPadding;
	const int leftPadding = wacStyle ? kWacModalLeftPadding : kModalLeftPadding;
	const int rightPadding = wacStyle ? kWacModalRightPadding : kModalRightPadding;
	const Common::Array<BitmapAssetFrame> &skin =
		wacStyle ? _wacSkin : _skin;
	const int scrollEdgeInset = wacStyle ? 0 : kModalScrollEdgeInset;
	int wrapWidth = kModalWidth - leftPadding - rightPadding;
	Common::Array<Common::String> lines;
	wrapText(body, wrapWidth, lines);
	if (lines.size() > kModalMaximumRows &&
			skin.size() > kModalScrollTrackFrame) {
		// ComputeChooserControlLayout at 0x54e9a through 0x54f17 adjusts
		// an overflowing text client's width by the scrollbar track width
		// plus its edge inset, less the right padding already reserved by
		// the chooser template.
		wrapWidth -= skin[kModalScrollTrackFrame].width +
			scrollEdgeInset - rightPadding;
		if (wrapWidth > 0)
			wrapText(body, wrapWidth, lines);
	}
	const uint visibleRows = MIN<uint>(lines.size(), kModalMaximumRows);
	const int width = kModalWidth;
	const int height = headingTopPadding + visibleRows * kModalRowHeight +
		bottomPadding;
	const int left = (640 - width) / 2;
	const int top = (400 - height) / 2;
	const Common::Rect bounds(left, top, left + width, top + height);
	uint firstVisible = 0;
	int modalCursorRegion = -1;
	Common::Point cursorPoint = _engine->getInput()->peekMouseState().position;

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	const bool applyPalette = !wacStyle && paletteBehavior == kApplyModalPalette;
	if (applyPalette)
		applyModalPalette();
	drawDialog(title, lines, firstVisible, visibleRows, bounds, style);
	debugC(3, kDebugScene,
		"Ripper: modal text layout source='%s' resource=%u wrapWidth=%d "
		"scrollTrackWidth=%u edgeInset=%d rightPadding=%d",
		source, bodyResourceId, wrapWidth,
		skin.size() > kModalScrollTrackFrame ?
			skin[kModalScrollTrackFrame].width : 0,
		scrollEdgeInset, rightPadding);
	debugC(1, kDebugScene,
		"Ripper: entered modal text dialog source='%s' resource=%u style=%s palette=%s title='%s' lines=%u bounds=%d,%d,%d,%d",
		source, bodyResourceId, wacStyle ? "wacmnu" : "menub",
		applyPalette ? "menub" : "active", title.c_str(),
		lines.size(), bounds.left, bounds.top, bounds.width(), bounds.height());

	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		bool redraw = false;
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b || command == 0x0d) {
				active = false;
				break;
			}
			const uint maximumFirst = lines.size() > visibleRows ?
				lines.size() - visibleRows : 0;
			uint nextFirst = firstVisible;
			switch (command) {
			case 0x4700:
				nextFirst = 0;
				break;
			case 0x4800:
				nextFirst = firstVisible > 0 ? firstVisible - 1 : 0;
				break;
			case 0x4900:
				nextFirst = firstVisible > visibleRows ? firstVisible - visibleRows : 0;
				break;
			case 0x4f00:
				nextFirst = maximumFirst;
				break;
			case 0x5000:
				nextFirst = MIN(firstVisible + 1, maximumFirst);
				break;
			case 0x5100:
				nextFirst = MIN(firstVisible + visibleRows, maximumFirst);
				break;
			default:
				break;
			}
			if (nextFirst != firstVisible) {
				firstVisible = nextFirst;
				redraw = true;
				debugC(2, kDebugScene,
					"Ripper: scrolled modal text source='%s' resource=%u firstLine=%u visibleRows=%u",
					source, bodyResourceId, firstVisible, visibleRows);
			}
		}
		const MouseState mouse = _engine->getInput()->publishMouseState();
		cursorPoint = mouse.position;
		const bool nextModalCursorActive = !retainSceneCursorRegions ||
			bounds.contains(mouse.position);
		if (nextModalCursorActive)
			_engine->getCursor()->update(kModalCursor);
		else
			_engine->getScripts()->updateModalSceneCursor(mouse.position);
		if ((int)nextModalCursorActive != modalCursorRegion) {
			modalCursorRegion = nextModalCursorActive;
			debugC(2, kDebugCursor,
				"Ripper: modal cursor region=%s source='%s' resource=%u point=%d,%d bounds=%d,%d,%d,%d",
				modalCursorRegion != 0 ? "dialog" : "scene", source, bodyResourceId,
				mouse.position.x, mouse.position.y, bounds.left, bounds.top,
				bounds.width(), bounds.height());
		}
		if (mouse.wheel != 0) {
			const uint maximumFirst = lines.size() > visibleRows ?
				lines.size() - visibleRows : 0;
			int nextFirst = (int)firstVisible - mouse.wheel;
			if (nextFirst < 0)
				nextFirst = 0;
			else if ((uint)nextFirst > maximumFirst)
				nextFirst = maximumFirst;
			if ((uint)nextFirst != firstVisible) {
				firstVisible = nextFirst;
				redraw = true;
				debugC(3, kDebugScene,
					"Ripper: mouse-wheel scrolled modal text source='%s' resource=%u "
					"delta=%d firstLine=%u visibleRows=%u",
					source, bodyResourceId, mouse.wheel, firstVisible, visibleRows);
			}
		}
		if (redraw)
			drawDialog(title, lines, firstVisible, visibleRows, bounds, style);
		else
			// The modal owns the main loop, so it must keep presenting frames
			// for CursorMan's software cursor to follow mouse-motion events.
			g_system->updateScreen();
		g_system->delayMillis(10);
	}

	_engine->getInput()->discardMouseTransitions();
	restoreDisplay();
	if (retainSceneCursorRegions)
		_engine->getScripts()->updateModalSceneCursor(cursorPoint);
	debugC(1, kDebugScene,
		"Ripper: exited modal text dialog source='%s' resource=%u",
		source, bodyResourceId);
	return true;
}

} // End of namespace Ripper
