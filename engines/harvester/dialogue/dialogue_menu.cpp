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

#include "harvester/dialogue/dialogue_session.h"
#include "harvester/dialogue/dialogue_render.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const uint kDialogueTextEntryMaxLength = 26;
static const uint32 kDialogueTextEntryBlinkTicks = 0x30;

static void splitDialogueMenuLine(const Common::String &line, Common::Array<Common::String> &parts) {
	parts.clear();
	if (line.empty())
		return;

	Common::String token;
	for (uint i = 0; i < line.size(); ++i) {
		if (line[i] == '/') {
			parts.push_back(Common::move(token));
			token.clear();
			continue;
		}

		token += line[i];
	}

	parts.push_back(Common::move(token));
}

static void logDialogueMenuItems(const char *label, int sourceLineIndex,
		const Common::String &rawLine, const Common::Array<Common::String> &items) {
	if (sourceLineIndex >= 0) {
		debugC(3, kDebugDialogue, "Harvester: %s source line 0x%x (%d) raw='%s'",
			label, sourceLineIndex, sourceLineIndex + 1, rawLine.c_str());
	} else {
		debugC(3, kDebugDialogue, "Harvester: %s source raw='%s'", label, rawLine.c_str());
	}

	if (items.empty()) {
		debugC(1, kDebugDialogue, "Harvester: %s has no explicit items", label);
		return;
	}

	for (uint i = 0; i < items.size(); ++i)
		debugC(3, kDebugDialogue, "Harvester: %s item[%u]='%s'", label, i + 1, items[i].c_str());
}

} // End of anonymous namespace

Common::Error RoomNpcDialogueSession::runKeywordMenu(const Common::String &topicBuffer, int topicBufferLineIndex,
		Common::String &selectedTopic,
		DialogueKeywordMenuSelectionState &selection) {
	selectedTopic.clear();
	selection = DialogueKeywordMenuSelectionState();
	Common::Array<Common::String> topics;
	splitDialogueMenuLine(topicBuffer, topics);
	logDialogueMenuItems("Keyword menu", topicBufferLineIndex, topicBuffer, topics);
	topics.push_back(_genericByeTopic);
	debugC(1, kDebugDialogue, "Harvester: keyword menu default topic='%s'", _genericByeTopic.c_str());
	bool editingOther = false;
	bool textEntryCursorVisible = false;
	uint32 textEntryBlinkTimestamp = 0;
	Common::String typedTopic;
	Common::Error releaseError = waitForPointerRelease();
	if (releaseError.getCode() != Common::kNoError)
		return releaseError;

	Graphics::FrameLimiter limiter(g_system, 60);
	for (;;) {
		const uint32 now = getRuntimeClockTicks();
		if (editingOther && now - textEntryBlinkTimestamp > kDialogueTextEntryBlinkTicks) {
			textEntryCursorVisible = !textEntryCursorVisible;
			textEntryBlinkTimestamp = now;
		}
		const int hoveredTopicIndex = editingOther ? -1 : DialogueRendering::getDialogueMenuItemAt(*_menuFont, topics.size(), _mousePos);
		const bool hoverOther = editingOther || DialogueRendering::isDialogueOtherHit(_mousePos);
		drawDialogueOverlay(&_keywordBitmap, nullptr, &topics, hoveredTopicIndex, hoverOther,
			editingOther ? &typedTopic : nullptr, textEntryCursorVisible);

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (_flow.handleSystemEvent(event, result)) {
				if (editingOther && _entityManager)
					_entityManager->showCursor();
				return result;
			}

			if (editingOther) {
				if (handleKeywordTextEntry(event, typedTopic, editingOther, selectedTopic, selection))
					return Common::kNoError;
				continue;
			}

			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					selectedTopic = _genericByeTopic;
					selection.fromEscape = true;
					selection.fromGenericBye = true;
					debugC(1, kDebugDialogue,
						"Harvester: keyword menu selected default topic via escape '%s'",
						selectedTopic.c_str());
					return Common::kNoError;
				}
				break;
			case Common::EVENT_LBUTTONDOWN: {
				const int clickedTopic = DialogueRendering::getDialogueMenuItemAt(*_menuFont, topics.size(), _mousePos);
				if (clickedTopic >= 0) {
					selectedTopic = topics[(uint)clickedTopic];
					selection.fromGenericBye = clickedTopic == (int)topics.size() - 1;
					debugC(2, kDebugDialogue, "Harvester: keyword menu selected topic[%d]='%s' via click",
						clickedTopic + 1, selectedTopic.c_str());
					return Common::kNoError;
				}
				if (DialogueRendering::isDialogueOtherHit(_mousePos)) {
					typedTopic.clear();
					editingOther = true;
					textEntryCursorVisible = false;
					textEntryBlinkTimestamp = getRuntimeClockTicks();
					debugC(1, kDebugDialogue, "Harvester: keyword menu entered Other input");
					if (_entityManager)
						_entityManager->hideCursor();
				}
				break;
			}
			default:
				break;
			}
		}

		if (_entityManager)
			(void)_entityManager->syncCursorEntityPosition(_mousePos);
		limiter.delayBeforeSwap();
		limiter.startFrame();
	}
}

bool RoomNpcDialogueSession::handleKeywordTextEntry(const Common::Event &event,
		Common::String &typedTopic, bool &editingOther, Common::String &selectedTopic,
		DialogueKeywordMenuSelectionState &selection) {
	bool acceptInput = false;
	bool cancelInput = false;
	if (event.type == Common::EVENT_KEYDOWN) {
		if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			cancelInput = true;
		} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE ||
				event.kbd.keycode == Common::KEYCODE_LEFT) {
			deleteTextEntryCharacter(typedTopic);
		} else if (event.kbd.keycode == Common::KEYCODE_HOME) {
			typedTopic.clear();
		} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
				event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
			acceptInput = true;
		} else if (event.kbd.ascii >= 0x1f && event.kbd.ascii <= 0x80 &&
				event.kbd.ascii != '~' &&
				(!_subtitleCftFont || _subtitleCftFont->hasGlyph(event.kbd.ascii))) {
			appendTextEntryCharacter(typedTopic, *_highlightFont, event.kbd.ascii,
				kDialogueTextEntryMaxLength, DialogueRendering::kDialogueTextEntryMaxWidth);
		}
	} else if (event.type == Common::EVENT_LBUTTONDOWN) {
		acceptInput = Common::Rect(3, 460, 318, 477).contains(event.mouse);
		cancelInput = Common::Rect(321, 460, 636, 477).contains(event.mouse);
		if (cancelInput)
			g_system->delayMillis(400);
	}
	if (acceptInput || cancelInput) {
		editingOther = false;
		if (_entityManager)
			_entityManager->showCursor();
		if (acceptInput && !typedTopic.empty()) {
			selectedTopic = typedTopic;
			selection.fromTypedInput = true;
			debugC(1, kDebugDialogue, "Harvester: keyword menu typed topic='%s'",
				selectedTopic.c_str());
			return true;
		}
		typedTopic.clear();
		debugC(1, kDebugDialogue, "Harvester: keyword menu cancelled Other input");
	}

	return false;
}

Common::Error RoomNpcDialogueSession::runResponseMenu(int responseLineIndex, int &selectedIndex) {
	const Common::String responseLine = _text->getDialogueResponseLine(responseLineIndex);
	return runResponseMenuTextInternal(responseLine, responseLineIndex, selectedIndex);
}

Common::Error RoomNpcDialogueSession::runResponseMenuText(const Common::String &responseLine, int &selectedIndex) {
	return runResponseMenuTextInternal(responseLine, -1, selectedIndex);
}

void RoomNpcDialogueSession::assignTopicBuffer(Common::String &topicBuffer, int &topicBufferLineIndex,
		int responseLineIndex, const char *) {
	topicBuffer = _text->getDialogueResponseLine(responseLineIndex);
	topicBufferLineIndex = responseLineIndex;
}

bool RoomNpcDialogueSession::matchesResponseLine(const Common::String &selectedTopic, int responseLineIndex) {
	const Common::String topicText = _text->getDialogueResponseLine(responseLineIndex);
	return !topicText.empty() && selectedTopic.equalsIgnoreCase(topicText);
}

bool RoomNpcDialogueSession::matchesAnyResponseLine(const Common::String &selectedTopic,
		const int *responseLineIndices, uint responseLineCount) {
	for (uint i = 0; i < responseLineCount; ++i) {
		if (matchesResponseLine(selectedTopic, responseLineIndices[i]))
			return true;
	}

	return false;
}

void RoomNpcDialogueSession::parseResponseMenuLine(const Common::String &responseLine,
		Common::Array<DialogueResponseOptionLayout> &options, uint &totalRows) {
	options.clear();
	totalRows = 0;
	if (responseLine.empty())
		return;

	Common::String segment;
	Common::Array<Common::String> explicitLines;
	Common::String optionText;

	auto flushOption = [&]() {
		if (explicitLines.empty())
			return;

		DialogueResponseOptionLayout option;
		option.text = optionText;
		if (optionText.size() > 1 && optionText[0] >= '1' && optionText[0] <= '9' &&
				optionText[1] == '.')
			option.hotkey = optionText[0];
		option.rowStart = (int)totalRows;
		for (const Common::String &explicitLine : explicitLines) {
			Common::Array<Common::String> wrappedExplicitLines;
			DialogueRendering::wrapDialogueTextLikeNative(*_menuFont, _menuFontUsesCft,
				explicitLine, DialogueRendering::kDialogueSubtitleTextWidth, wrappedExplicitLines);
			if (wrappedExplicitLines.empty())
				wrappedExplicitLines.push_back(explicitLine);
			for (const Common::String &wrappedExplicitLine : wrappedExplicitLines)
				option.wrappedLines.push_back(wrappedExplicitLine);
		}
		option.rowCount = MAX<int>(1, option.wrappedLines.size());
		totalRows += option.rowCount;
		options.push_back(Common::move(option));

		explicitLines.clear();
		optionText.clear();
	};

	auto consumeSegment = [&](const Common::String &rawSegment) {
		Common::String segmentText = rawSegment;
		if (segmentText.empty())
			return;

		const bool startsNewOption = segmentText.size() > 1 && segmentText[0] >= '1' &&
			segmentText[0] <= '9' && segmentText[1] == '.';
		if (startsNewOption) {
			flushOption();
			optionText = segmentText;
			explicitLines.push_back(Common::move(segmentText));
			return;
		}

		if (!optionText.empty())
			optionText += ' ';
		optionText += segmentText;
		explicitLines.push_back(Common::move(segmentText));
	};

	for (uint i = 0; i < responseLine.size(); ++i) {
		if (responseLine[i] == '/') {
			consumeSegment(segment);
			segment.clear();
			continue;
		}

		segment += responseLine[i];
	}

	consumeSegment(segment);
	flushOption();
}

int RoomNpcDialogueSession::getResponseMenuItemAt(const Common::Array<DialogueResponseOptionLayout> &options,
		uint totalRows, const Common::Point &mousePos) {
	if (mousePos.x < DialogueRendering::kDialogueTopicStartX || mousePos.x > DialogueRendering::kDialogueTopicEndX)
		return -1;
	if (mousePos.y < DialogueRendering::kDialogueKeywordTextY)
		return -1;

	const int lineHeight = DialogueRendering::getDialogueTextLineHeight(*_menuFont);
	const int row = (mousePos.y - DialogueRendering::kDialogueKeywordTextY) / MAX<int>(1, lineHeight);
	if (row < 0 || row >= (int)totalRows)
		return -1;

	for (uint i = 0; i < options.size(); ++i) {
		const DialogueResponseOptionLayout &option = options[i];
		if (row >= option.rowStart && row < option.rowStart + option.rowCount)
			return (int)i;
	}

	return -1;
}

Common::Error RoomNpcDialogueSession::runResponseMenuTextInternal(const Common::String &responseLine, int responseLineIndex,
		int &selectedIndex) {
	selectedIndex = 0;

	if (responseLine.empty()) {
		if (responseLineIndex >= 0) {
			debugC(1, kDebugDialogue, "Harvester: response menu line 0x%x (%d) is empty",
				responseLineIndex, responseLineIndex + 1);
		} else {
			debugC(1, kDebugDialogue, "Harvester: response menu text is empty");
		}
		return Common::kNoError;
	}

	Common::Array<DialogueResponseOptionLayout> options;
	uint totalRows = 0;
	parseResponseMenuLine(responseLine, options, totalRows);
	if (options.empty()) {
		if (responseLineIndex >= 0) {
			debugC(1, kDebugDialogue, "Harvester: response menu line 0x%x (%d) produced no options",
				responseLineIndex, responseLineIndex + 1);
		} else {
			debugC(1, kDebugDialogue, "Harvester: response menu text produced no options");
		}
		return Common::kNoError;
	}

	Common::Array<Common::String> optionTexts;
	for (const DialogueResponseOptionLayout &option : options) {
		optionTexts.push_back(option.text);
		debugC(3, kDebugDialogue, "Harvester: response menu option[%u] hotkey=%c rowStart=%d rowCount=%d",
			optionTexts.size(), option.hotkey ? option.hotkey : '-', option.rowStart, option.rowCount);
	}
	logDialogueMenuItems("Response menu", responseLineIndex, responseLine, optionTexts);

	const IndexedBitmap *textboxBitmap = _art->getTextboxBitmap(DialogueRendering::resolveDialogueResponseTextboxIndex(totalRows));
	Common::Error releaseError = waitForPointerRelease();
	if (releaseError.getCode() != Common::kNoError)
		return releaseError;

	Graphics::FrameLimiter limiter(g_system, 60);
	for (;;) {
		const int hoveredOptionIndex = getResponseMenuItemAt(options, totalRows, _mousePos);
		drawDialogueResponseMenu(textboxBitmap, options, hoveredOptionIndex);

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (_flow.handleSystemEvent(event, result))
				return result;

			if (event.type == Common::EVENT_KEYDOWN) {
				if (selectResponseMenuHotkey(event, options, responseLineIndex, selectedIndex))
					return waitForPointerRelease();
			} else if (event.type == Common::EVENT_LBUTTONDOWN) {
				const int clickedOptionIndex = getResponseMenuItemAt(options, totalRows, event.mouse);
				if (clickedOptionIndex < 0)
					continue;
				selectedIndex = clickedOptionIndex + 1;
				if (responseLineIndex >= 0) {
					debugC(2, kDebugDialogue,
						"Harvester: response menu line 0x%x (%d) selected option[%d]='%s' via click x=%d y=%d result=%d",
						responseLineIndex, responseLineIndex + 1,
						selectedIndex, options[(uint)clickedOptionIndex].text.c_str(),
						event.mouse.x, event.mouse.y, selectedIndex);
				} else {
					debugC(2, kDebugDialogue,
						"Harvester: response menu text selected option[%d]='%s' via click x=%d y=%d result=%d",
						selectedIndex, options[(uint)clickedOptionIndex].text.c_str(),
						event.mouse.x, event.mouse.y, selectedIndex);
				}
				return waitForPointerRelease();
			}
		}

		if (_entityManager)
			(void)_entityManager->syncCursorEntityPosition(_mousePos);
		limiter.delayBeforeSwap();
		limiter.startFrame();
	}
}

bool RoomNpcDialogueSession::selectResponseMenuHotkey(const Common::Event &event,
		const Common::Array<DialogueResponseOptionLayout> &options, int responseLineIndex,
		int &selectedIndex) {
	debugC(3, kDebugDialogue, "Harvester: response menu keycode=%d ascii=%u",
		(int)event.kbd.keycode, event.kbd.ascii);
	if (event.kbd.keycode >= Common::KEYCODE_1 && event.kbd.keycode <= Common::KEYCODE_9) {
		const char hotkey = '1' + event.kbd.keycode - Common::KEYCODE_1;
		for (int optionIndex = (int)options.size() - 1; optionIndex >= 0; --optionIndex) {
			const DialogueResponseOptionLayout &option = options[(uint)optionIndex];
			if (option.hotkey != hotkey)
				continue;
			selectedIndex = optionIndex + 1;
			if (responseLineIndex >= 0) {
				debugC(2, kDebugDialogue,
					"Harvester: response menu line 0x%x (%d) selected option[%d]='%s' via hotkey=%c result=%d",
					responseLineIndex, responseLineIndex + 1,
					optionIndex + 1, option.text.c_str(), hotkey, selectedIndex);
			} else {
				debugC(2, kDebugDialogue,
					"Harvester: response menu text selected option[%d]='%s' via hotkey=%c result=%d",
					optionIndex + 1, option.text.c_str(), hotkey, selectedIndex);
			}
			return true;
		}
	}

	return false;
}

} // End of namespace Harvester
