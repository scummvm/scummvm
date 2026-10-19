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
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/flow/flow_support.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/scene/room_presentation.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const int kDialogueOverlayX = 167;
static const int kDialogueOverlayY = 6;
static const int kDialogueLeftHeadX = 6;
static const int kDialogueRightHeadX = 474;
static const int kDialogueHeadY = 6;
static const int kDialogueSubtitleTextX = 178;
static const int kDialogueSubtitleTextY = 12;
static const int kDialogueKeywordTextX = 178;
static const int kDialogueKeywordTextWidth = 300;
static const int kDialogueOtherX = 171;
static const int kDialogueOtherY = 183;
static const int kDialogueTextEntryX = 208;
static const int kDialogueTextEntryY = 184;
static const uint kDialogueTextEntryMaxLength = 26;
static const int kDialogueTextEntryMaxWidth = 550;
static const uint32 kDialogueTextEntryBlinkTicks = 0x30;
static const int kDialogueOtherWidth = 120;
static const int kDialogueKeywordTitleYOffset = 4;
static const int kDialogueGenericByeResponseIndex = 13;
static const char *const kDialogueKeywordBitmapPath = "1:/GRAPHIC/OTHER/KEYWORD.BM";
static const char *const kDialogueLocalizedKeywordBitmapPath = "GRAPHIC/OTHER/KEYWORD.BM";

static const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;

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

static Common::String buildDialogueHeadId(const Common::String &speakerId, int headVariant) {
	return Common::String::format("%s%d", speakerId.c_str(), headVariant);
}

static bool loadDialogueHeadBitmap(HarvesterEngine &engine, const Common::String &speakerId, int headVariant,
		IndexedBitmap &bitmap) {
	Script *script = engine.getScript();
	ResourceManager *resources = engine.getResources();
	if (!script || !resources || speakerId.empty())
		return false;

	const Common::String headId = buildDialogueHeadId(speakerId, headVariant);
	const HeadRecord *head = script->findHeadRecord(headId);
	if (!head)
		return false;

	return loadBitmapResource(*resources, head->portraitPath + ".BM", bitmap);
}

} // End of anonymous namespace

RoomNpcDialogueSession::RoomNpcDialogueSession(HarvesterEngine &engine, Common::Point &mousePos, Flow &flow,
		const IndexedBitmap &backdrop, const byte *palette, float paletteBrightness,
		const NpcRecord &npc)
	: _engine(engine), _mousePos(mousePos), _flow(flow),
	  _menuTextConfig(flow.getMenuTextConfig()), _backdrop(backdrop),
	  _palette(palette), _paletteBrightness(paletteBrightness), _npc(npc),
	  _script(engine.getScript()), _text(engine.getText()), _art(engine.getArt()),
	  _entityManager(engine.getRuntimeEntities()),
	  _fallbackFont(FontMan.getFontByUsage(Graphics::FontManager::kGUIFont)),
	  _subtitleFont(nullptr), _menuFont(nullptr), _highlightFont(nullptr),
	  _subtitleFontUsesCft(false), _menuFontUsesCft(false),
	  _highlightFontUsesCft(false), _leftHeadVisible(false),
	  _rightHeadVisible(false) {
	ResourceManager *resources = _engine.getResources();
	if (!_script || !_text || !_art || !_fallbackFont || !_backdrop.isValid() ||
			!_palette || !resources) {
		return;
	}

	if (const CftFontResource *subtitleFontResource = _text->findFont("TEXTFONT")) {
		_subtitleCftFont.reset(new HarvesterCftFont(*subtitleFontResource));
		if (!_subtitleCftFont->isValid())
			_subtitleCftFont.reset();
	}
	if (const CftFontResource *menuFontResource = _text->findFont("TEXTFNT2")) {
		_menuCftFont.reset(new HarvesterCftFont(*menuFontResource));
		if (!_menuCftFont->isValid())
			_menuCftFont.reset();
	}

	_subtitleFont = _subtitleCftFont
		? static_cast<const Graphics::Font *>(_subtitleCftFont.get())
		: _fallbackFont;
	_menuFont = _menuCftFont
		? static_cast<const Graphics::Font *>(_menuCftFont.get())
		: _subtitleFont;
	_highlightFont = _subtitleFont;
	_subtitleFontUsesCft = _subtitleCftFont.get() != nullptr;
	_menuFontUsesCft = _menuCftFont.get() != nullptr;
	_highlightFontUsesCft = _subtitleFontUsesCft;

	const char *keywordBitmapPath = _menuTextConfig.hasDialogueKeywordLabel()
		? kDialogueLocalizedKeywordBitmapPath : kDialogueKeywordBitmapPath;
	if (!loadBitmapResource(*resources, keywordBitmapPath, _keywordBitmap))
		return;
	debugC(2, kDebugDialogue,
		"Harvester: dialogue keyword panel='%s' title='%s' other='%s' responses='%s'",
		keywordBitmapPath, _menuTextConfig.dialogueKeywordLabel.c_str(),
		_menuTextConfig.dialogueOtherLabel.c_str(),
		_menuTextConfig.dialogueResponsesLabel.c_str());

	_genericByeTopic = _text->getDialogueResponseLine(kDialogueGenericByeResponseIndex);
	if (_genericByeTopic.empty())
		_genericByeTopic = "BYE";

	_leftHeadSpeakerId = buildDialogueHeadId(_npc.npcName, 0);
	_rightHeadSpeakerId = buildDialogueHeadId("PC", 0);
	(void)loadDialogueHeadBitmap(_engine, _npc.npcName, 0, _leftHeadBitmap);
	(void)loadDialogueHeadBitmap(_engine, "PC", 0, _rightHeadBitmap);
	_leftHeadVisible = _leftHeadBitmap.isValid();
	_rightHeadVisible = _rightHeadBitmap.isValid();
}

bool RoomNpcDialogueSession::isValid() const {
	return _script && _text && _art && _fallbackFont && _backdrop.isValid() &&
		_palette && _keywordBitmap.isValid();
}

const Common::String &RoomNpcDialogueSession::getGenericByeTopic() const {
	return _genericByeTopic;
}

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
							kDialogueTextEntryMaxLength, kDialogueTextEntryMaxWidth);
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
						return Common::kNoError;
					}
					typedTopic.clear();
					debugC(1, kDebugDialogue, "Harvester: keyword menu cancelled Other input");
				}
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

void RoomNpcDialogueSession::queueDialogueInteractionIfNeeded(const InteractionResult &interaction) {
	if (interaction.abortRemainingCommandChain || interaction.mutatedRuntimeState ||
			!interaction.musicPath.empty() || !interaction.nextRoomName.empty() ||
			!interaction.cutscenePath.empty() ||
			!interaction.deathFlicPath.empty() || interaction.requestMainMenu ||
			interaction.requestDemoEnding ||
			interaction.requestRoomRestart ||
			interaction.requestCloseupExit ||
			interaction.cdChangeDisc > 0 ||
			!interaction.dialogueNpcName.empty() ||
			!interaction.dialogueContinuationTag.empty() ||
			!interaction.continuationTag.empty() ||
			!interaction.modalText.value.empty() ||
			interaction.lightingCommand != kStartupLightingCommandNone ||
			interaction.requestPlayerGotoXZ ||
			!interaction.audioCommands.empty()) {
		_flow.queueDialogueInteraction(interaction);
	}
}

void RoomNpcDialogueSession::applyImmediateDialogueInteractionEffects(InteractionResult &interaction) {
	if (!interaction.musicPath.empty())
		(void)_engine.playMusic(interaction.musicPath);
	if (!interaction.audioCommands.empty())
		_flow.executeStartupAudioCommands(interaction.audioCommands);
	interaction.musicPath.clear();
	interaction.audioCommands.clear();
}

int RoomNpcDialogueSession::getRandomNumber(int maxValue) {
	return _engine.getRandomNumber(maxValue);
}

void RoomNpcDialogueSession::setActiveSpeakerPortrait(const Common::String &speakerId, int headVariant) {
	if (speakerId.empty() || speakerId.equalsIgnoreCase("SPEECH"))
		return;

	ensureSpeakerPortrait(speakerId, headVariant);
	if (speakerId.equalsIgnoreCase("PC")) {
		_leftHeadVisible = false;
		_rightHeadVisible = _rightHeadBitmap.isValid();
	} else {
		_leftHeadVisible = _leftHeadBitmap.isValid();
		_rightHeadVisible = false;
	}
}

Graphics::Screen *RoomNpcDialogueSession::getActiveScreen() const {
	return _engine.getScreen();
}

void RoomNpcDialogueSession::drawFontString(const Graphics::Font &font, bool usesCft, const Common::String &text,
		int x, int y, int width, byte color) {
	Graphics::Screen *activeScreen = getActiveScreen();
	if (!activeScreen)
		return;

	if (usesCft)
		font.drawString(activeScreen, text, x, y, width, 0);
	else
		RoomPresentation::drawShadowedString(*activeScreen, font, text, x, y, width, color);
}

void RoomNpcDialogueSession::ensureSpeakerPortrait(const Common::String &speakerId, int headVariant) {
	if (speakerId.empty())
		return;

	const Common::String headId = buildDialogueHeadId(speakerId, headVariant);
	IndexedBitmap *targetBitmap = speakerId.equalsIgnoreCase("PC") ? &_rightHeadBitmap : &_leftHeadBitmap;
	Common::String *targetSpeakerId = speakerId.equalsIgnoreCase("PC") ? &_rightHeadSpeakerId : &_leftHeadSpeakerId;
	if (targetBitmap->isValid() && targetSpeakerId->equalsIgnoreCase(headId))
		return;

	IndexedBitmap updatedBitmap;
	if (loadDialogueHeadBitmap(_engine, speakerId, headVariant, updatedBitmap)) {
		*targetBitmap = Common::move(updatedBitmap);
		*targetSpeakerId = headId;
	} else {
		warning("Harvester: unable to load dialogue head for '%s'", headId.c_str());
	}
}

void RoomNpcDialogueSession::drawDialogueOverlay(const IndexedBitmap *overlayBitmap,
		const Common::Array<Common::String> *subtitleLines, const Common::Array<Common::String> *topics,
		int hoveredTopicIndex, bool hoverOther, const Common::String *textEntryValue,
		bool textEntryCursorVisible) {
	Graphics::Screen *activeScreen = getActiveScreen();
	if (!activeScreen)
		return;

	setScaledPalette(*activeScreen, _palette, _paletteBrightness);
	RoomPresentation::blitBitmap(*activeScreen, _backdrop, 0, 0);
	if (_leftHeadVisible && _leftHeadBitmap.isValid())
		blitTransparentBitmap(*activeScreen, _leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
	if (_rightHeadVisible && _rightHeadBitmap.isValid())
		blitTransparentBitmap(*activeScreen, _rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
	if (overlayBitmap && overlayBitmap->isValid())
		blitTransparentBitmap(*activeScreen, *overlayBitmap, kDialogueOverlayX, kDialogueOverlayY);

	if (subtitleLines) {
		if (_subtitleFontUsesCft) {
			DialogueRendering::drawDialogueTextLines(*activeScreen, *_subtitleFont, *subtitleLines,
				kDialogueSubtitleTextX, kDialogueSubtitleTextY, DialogueRendering::kDialogueSubtitleTextWidth);
		} else {
			const int lineHeight = DialogueRendering::getDialogueTextLineHeight(*_subtitleFont);
			for (uint i = 0; i < subtitleLines->size(); ++i) {
				RoomPresentation::drawShadowedString(*activeScreen, *_subtitleFont, (*subtitleLines)[i],
					kDialogueSubtitleTextX,
					kDialogueSubtitleTextY + (int)i * lineHeight,
					DialogueRendering::kDialogueSubtitleTextWidth, kTextColorNormal);
			}
		}
	}

	if (topics) {
		if (_menuTextConfig.hasDialogueKeywordLabel()) {
			const Common::String &title = _menuTextConfig.dialogueKeywordLabel;
			const int titleWidth = _highlightFont->getStringWidth(title);
			const int titleX = kDialogueOverlayX +
				MAX<int>(0, ((int)_keywordBitmap.width - titleWidth) / 2);
			drawFontString(*_highlightFont, _highlightFontUsesCft, title, titleX,
				kDialogueOverlayY + kDialogueKeywordTitleYOffset, titleWidth,
				kTextColorNormal);
		}

		const int lineHeight = DialogueRendering::getDialogueTextLineHeight(*_menuFont);
		for (uint i = 0; i < topics->size(); ++i) {
			const bool highlighted = (int)i == hoveredTopicIndex;
			const Graphics::Font &font = highlighted ? *_highlightFont : *_menuFont;
			const bool usesCft = highlighted ? _highlightFontUsesCft : _menuFontUsesCft;
			drawFontString(font, usesCft, (*topics)[i], kDialogueKeywordTextX,
				DialogueRendering::kDialogueKeywordTextY + (int)i * lineHeight, kDialogueKeywordTextWidth,
				highlighted ? kTextColorNormal : kTextColorHover);
		}

		const Graphics::Font &otherFont = hoverOther ? *_highlightFont : *_menuFont;
		const bool otherUsesCft = hoverOther ? _highlightFontUsesCft : _menuFontUsesCft;
		drawFontString(otherFont, otherUsesCft, _menuTextConfig.dialogueOtherLabel,
			kDialogueOtherX, kDialogueOtherY,
			kDialogueOtherWidth, hoverOther ? kTextColorNormal : kTextColorHover);
	}

	if (textEntryValue) {
		drawFontString(*_highlightFont, _highlightFontUsesCft,
			*textEntryValue + (textEntryCursorVisible ? "~" : ""),
			kDialogueTextEntryX, kDialogueTextEntryY, kDialogueTextEntryMaxWidth,
			kTextColorNormal);
	}

	if (_entityManager)
		_entityManager->drawCursor(*activeScreen);
	activeScreen->makeAllDirty();
	activeScreen->update();
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

void RoomNpcDialogueSession::drawDialogueResponseMenu(const IndexedBitmap *textboxBitmap,
		const Common::Array<DialogueResponseOptionLayout> &options, int hoveredOptionIndex) {
	Graphics::Screen *activeScreen = getActiveScreen();
	if (!activeScreen)
		return;

	setScaledPalette(*activeScreen, _palette, _paletteBrightness);
	RoomPresentation::blitBitmap(*activeScreen, _backdrop, 0, 0);
	if (_leftHeadVisible && _leftHeadBitmap.isValid())
		blitTransparentBitmap(*activeScreen, _leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
	if (_rightHeadVisible && _rightHeadBitmap.isValid())
		blitTransparentBitmap(*activeScreen, _rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
	if (textboxBitmap && textboxBitmap->isValid())
		blitTransparentBitmap(*activeScreen, *textboxBitmap, kDialogueOverlayX, kDialogueOverlayY);

	const Common::String &title = _menuTextConfig.dialogueResponsesLabel;
	const Graphics::Font &titleFont = *_highlightFont;
	const bool titleUsesCft = _highlightFontUsesCft;
	const int titleWidth = titleFont.getStringWidth(title);
	const int titleX = kDialogueSubtitleTextX + MAX<int>(0, (DialogueRendering::kDialogueSubtitleTextWidth - titleWidth) / 2);
	drawFontString(titleFont, titleUsesCft, title, titleX, 11, titleWidth, kTextColorNormal);

	const int lineHeight = DialogueRendering::getDialogueTextLineHeight(*_menuFont);
	int drawY = DialogueRendering::kDialogueKeywordTextY;
	for (uint i = 0; i < options.size(); ++i) {
		const bool highlighted = (int)i == hoveredOptionIndex;
		const Graphics::Font &font = highlighted ? *_highlightFont : *_menuFont;
		const bool usesCft = highlighted ? _highlightFontUsesCft : _menuFontUsesCft;
		const byte color = highlighted ? kTextColorNormal : kTextColorHover;

		for (const Common::String &wrappedLine : options[i].wrappedLines) {
			drawFontString(font, usesCft, wrappedLine, kDialogueSubtitleTextX, drawY,
				DialogueRendering::kDialogueSubtitleTextWidth, color);
			drawY += lineHeight;
		}
	}

	if (_entityManager)
		_entityManager->drawCursor(*activeScreen);
	activeScreen->makeAllDirty();
	activeScreen->update();
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
						return waitForPointerRelease();
					}
				}
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

} // End of namespace Harvester
