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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hollywood/gameplay/dialogue_menu.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/cursor.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint32 kDialogueMenuTickMillis = 10;
const uint32 kDialogueMenuMaxFrameDeltaMillis = 250;
const uint16 kDialogueMenuLineHeight = 0x15;
const uint16 kDialogueMenuHoverTopBaseY = 0x1d3;
const uint16 kDialogueMenuHoverBottomY = 0x1d5;
const uint16 kDialogueMenuTextWidth = 0x266;
const uint kDialogueMenuMaxCharsPerLine = 100;

DialogueChoiceRecord::DialogueChoiceRecord() :
		enabled(0),
		nextNodeIndex(0),
		transitionMode(0),
		playerTextRowId(0),
		responseFrameIndex(0),
		disableAfterUse(0),
		reserved(0),
		selectable(1) {
}

DialogueMenuLine::DialogueMenuLine() :
		choiceIndex(0),
		firstLineOfChoice(false),
		selectable(true) {
}

DialogueMenuState::DialogueMenuState() :
		lineCount(0),
		highlightedLineIndex(0xff) {
}

void DialogueMenuState::clear() {
	lines.clear();
	lineCount = 0;
	highlightedLineIndex = 0xff;
}

byte DialogueMenuState::choiceForLine(byte lineIndex) const {
	if (lineIndex >= lines.size())
		return 0xff;
	if (!lines[lineIndex].selectable)
		return 0xff;

	return lines[lineIndex].choiceIndex;
}

DialogueMenuDelegate::~DialogueMenuDelegate() {
}

DialogueMenu::DialogueMenu(HollywoodEngine *vm, DialogueMenuDelegate *delegate) :
		_vm(vm),
		_delegate(delegate) {
}

byte DialogueMenu::choose(byte stageId, const Common::Array<DialogueChoiceRecord> &records,
		byte depthIndex, byte nodeIndex) {
	if (!_vm || !_delegate)
		return kCancelledChoice;

	byte selectableChoiceCount = 0;
	if (!build(stageId, records, depthIndex, nodeIndex, selectableChoiceCount))
		return kCancelledChoice;

	if (_state.lineCount == 0)
		return kCancelledChoice;

	if (selectableChoiceCount < 2) {
		for (byte lineIndex = 0; lineIndex < _state.lineCount; ++lineIndex) {
			const byte choiceIndex = _state.choiceForLine(lineIndex);
			if (choiceIndex != 0xff)
				return choiceIndex;
		}
		return kCancelledChoice;
	}

	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	updateHoverFromCursor();

	_delegate->drawDialogueMenuFrame();
	_delegate->presentDialogueMenuFrame(_state);

	uint32 lastMillis = g_system->getMillis();
	while (!Engine::shouldQuit()) {
		bool selected = false;
		bool cancelled = false;
		if (pollEvents(selected, cancelled)) {
			_vm->cursor()->leaveInteractiveMode();
			return kCancelledChoice;
		}
		if (cancelled) {
			_vm->cursor()->leaveInteractiveMode();
			return kCancelledChoice;
		}
		if (selected) {
			const byte choiceIndex = _state.choiceForLine(_state.highlightedLineIndex);
			_vm->cursor()->leaveInteractiveMode();
			return choiceIndex;
		}

		g_system->delayMillis(kDialogueMenuTickMillis);

		const uint32 now = g_system->getMillis();
		uint32 delta = now - lastMillis;
		lastMillis = now;
		if (delta > kDialogueMenuMaxFrameDeltaMillis)
			delta = kDialogueMenuMaxFrameDeltaMillis;

		_delegate->advanceDialogueMenu(delta);
		_vm->cursor()->advance(delta);
		updateHoverFromCursor();
		_delegate->drawDialogueMenuFrame();
		_delegate->presentDialogueMenuFrame(_state);
	}

	_vm->cursor()->leaveInteractiveMode();
	return kCancelledChoice;
}

bool DialogueMenu::build(byte stageId, const Common::Array<DialogueChoiceRecord> &records,
		byte depthIndex, byte nodeIndex, byte &selectableChoiceCount) {
	_state.clear();
	selectableChoiceCount = 0;

	if (depthIndex >= kMaxDepthCount || nodeIndex >= kNodesPerDepth)
		return false;

	const uint firstRecordIndex = ((uint)depthIndex * kNodesPerDepth + nodeIndex) * kChoicesPerNode;
	if (firstRecordIndex >= records.size())
		return false;

	for (byte choiceIndex = 0; choiceIndex < kChoicesPerNode; ++choiceIndex) {
		const uint recordIndex = firstRecordIndex + choiceIndex;
		if (recordIndex >= records.size())
			break;

		const DialogueChoiceRecord &record = records[recordIndex];
		if (record.enabled == 0)
			continue;

		if (record.selectable != 0)
			++selectableChoiceCount;
		appendWrappedChoiceText(choiceIndex, _delegate->dialogueMenuText(stageId, record.playerTextRowId),
			record.selectable != 0);
		if (_state.lineCount >= kMaxVisibleLines)
			break;
	}

	return true;
}

void DialogueMenu::appendWrappedChoiceText(byte choiceIndex, const Common::String &text, bool selectable) {
	if (text.empty() || _state.lineCount >= kMaxVisibleLines)
		return;

	HollywoodFont *font = _vm->font();
	const char *source = text.c_str();
	const uint textLength = text.size();
	uint cursor = 0;
	bool firstLine = true;

	while (cursor < textLength && _state.lineCount < kMaxVisibleLines) {
		uint end = cursor;
		uint bestBreak = cursor;
		uint charCount = 0;
		while (end < textLength && charCount < kDialogueMenuMaxCharsPerLine) {
			if ((byte)source[end] == 0x20)
				bestBreak = end;

			Common::String candidate(source + cursor, end + 1 - cursor);
			if (font && font->isLoaded() && font->getStringWidth(candidate) > kDialogueMenuTextWidth)
				break;

			++end;
			++charCount;
			if (source[end] == 0)
				break;
		}

		if (end < textLength && bestBreak > cursor)
			end = bestBreak;
		if (end == cursor)
			end = MIN<uint>(textLength, cursor + 1);

		while (end > cursor && (byte)source[end - 1] == 0x20)
			--end;

		DialogueMenuLine line;
		line.text = Common::String(source + cursor, end - cursor);
		line.choiceIndex = choiceIndex;
		line.firstLineOfChoice = firstLine;
		line.selectable = selectable;
		_state.lines.push_back(line);
		_state.lineCount = (byte)_state.lines.size();

		firstLine = false;
		cursor = end;
		while (cursor < textLength && (byte)source[cursor] == 0x20)
			++cursor;
	}
}

bool DialogueMenu::pollEvents(bool &selected, bool &cancelled) {
	selected = false;
	cancelled = false;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			return true;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			updateHoverFromCursor();
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (_state.highlightedLineIndex != 0xff)
				selected = true;
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				cancelled = true;
			} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_SPACE) {
				if (_state.highlightedLineIndex != 0xff)
					selected = true;
			}
			break;
		default:
			break;
		}
	}

	return false;
}

void DialogueMenu::updateHoverFromCursor() {
	const byte lineIndex = lineAt(_vm->cursor()->surfaceY());
	if (lineIndex == 0xff || lineIndex >= _state.lineCount ||
			(lineIndex < _state.lines.size() && !_state.lines[lineIndex].selectable)) {
		_state.highlightedLineIndex = 0xff;
		return;
	}

	_state.highlightedLineIndex = lineIndex;
}

byte DialogueMenu::lineAt(uint16 cursorY) const {
	if (_state.lineCount == 0)
		return 0xff;

	const int topY = (int)kDialogueMenuHoverTopBaseY - (int)_state.lineCount * kDialogueMenuLineHeight;
	if ((int)cursorY < topY || cursorY >= kDialogueMenuHoverBottomY)
		return 0xff;

	const byte lineIndex = (byte)(((int)cursorY - topY) / kDialogueMenuLineHeight);
	return lineIndex < _state.lineCount ? lineIndex : 0xff;
}

} // End of namespace Hollywood
