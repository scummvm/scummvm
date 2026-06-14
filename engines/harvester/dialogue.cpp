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

#include "harvester/dialogue.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/ptr.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/npc/authority_dialogue.h"
#include "harvester/npc/beggar_dialogue.h"
#include "harvester/npc/boyle_dialogue.h"
#include "harvester/npc/buster_dialogue.h"
#include "harvester/npc/butcher_dialogue.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/npc/chessmaster_dialogue.h"
#include "harvester/npc/cloak_atnd_dialogue.h"
#include "harvester/npc/curator_dialogue.h"
#include "harvester/npc/dad_dialogue.h"
#include "harvester/npc/dark_woman_dialogue.h"
#include "harvester/npc/dwayne_dialogue.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/npc/edna_dialogue.h"
#include "harvester/npc/gladiator_dialogue.h"
#include "harvester/npc/fireman1_dialogue.h"
#include "harvester/npc/fireman2_dialogue.h"
#include "harvester/npc/hank_dialogue.h"
#include "harvester/npc/herrill_dialogue.h"
#include "harvester/npc/inquisitor_dialogue.h"
#include "harvester/npc/jimmy_dialogue.h"
#include "harvester/npc/johnson_dialogue.h"
#include "harvester/npc/karin_dialogue.h"
#include "harvester/npc/librarian_dialogue.h"
#include "harvester/npc/loomis_dialogue.h"
#include "harvester/npc/lodge_chef_dialogue.h"
#include "harvester/npc/mcknight_dialogue.h"
#include "harvester/npc/memb_dir_dialogue.h"
#include "harvester/npc/maint_man_dialogue.h"
#include "harvester/npc/madam_dialogue.h"
#include "harvester/npc/mom_dialogue.h"
#include "harvester/npc/mother_dialogue.h"
#include "harvester/npc/moynahan_dialogue.h"
#include "harvester/npc/mr_potts_dialogue.h"
#include "harvester/npc/mrs_potts_dialogue.h"
#include "harvester/npc/parsons_dialogue.h"
#include "harvester/npc/pastorelli_dialogue.h"
#include "harvester/npc/phelps_dialogue.h"
#include "harvester/npc/priest_dialogue.h"
#include "harvester/npc/pta_mom_dialogue.h"
#include "harvester/npc/ryder_dialogue.h"
#include "harvester/npc/sergeant_dialogue.h"
#include "harvester/npc/nude_man_dialogue.h"
#include "harvester/palette_utils.h"
#include "harvester/npc/stephanie_dialogue.h"
#include "harvester/npc/stub_dialogue.h"
#include "harvester/npc/sparky_dialogue.h"
#include "harvester/npc/swell_dialogue.h"
#include "harvester/npc/valet_dialogue.h"
#include "harvester/npc/vet_dialogue.h"
#include "harvester/npc/whaley_dialogue.h"
#include "harvester/npc/wasp_woman_dialogue.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/art.h"
#include "harvester/flow.h"
#include "harvester/text.h"

namespace Harvester {

class DialogueFlowAccess {
public:
	static bool handleSystemEvent(Flow &flow, const Common::Event &event, Common::Error &result) {
		return flow.handleSystemEvent(event, result);
	}

	static void executeStartupAudioCommands(Flow &flow, const Common::Array<AudioCommand> &commands) {
		flow.executeStartupAudioCommands(commands);
	}

	static void queueDialogueInteraction(Flow &flow, const InteractionResult &interaction) {
		flow.queueDialogueInteraction(interaction);
	}

	static void requestMainMenuReturn(Flow &flow) {
		flow.requestMainMenuReturn();
	}
};

namespace {

static const int kDialogueOverlayX = 167;
static const int kDialogueOverlayY = 6;
static const int kDialogueLeftHeadX = 6;
static const int kDialogueRightHeadX = 474;
static const int kDialogueHeadY = 6;
static const int kDialogueSubtitleTextX = 178;
static const int kDialogueSubtitleTextY = 12;
static const int kDialogueSubtitleTextWidth = 316;
static const int kDialogueKeywordTextX = 178;
static const int kDialogueKeywordTextY = 34;
static const int kDialogueKeywordTextWidth = 300;
static const int kDialogueOtherX = 171;
static const int kDialogueOtherY = 183;
static const int kDialogueTextEntryX = 208;
static const int kDialogueTextEntryY = 184;
static const int kDialogueOtherWidth = 120;
static const int kDialogueTopicStartX = 168;
static const int kDialogueTopicEndX = 482;
static const int kDialogueOtherStartY = 170;
static const int kDialogueOtherEndY = 193;
static const int kDialogueGenericByeResponseIndex = 13;
static const char *const kCdChangePromptPalettePath = "1:/GRAPHIC/PAL/CD1.PAL";
static const char *const kDialogueKeywordBitmapPath = "1:/GRAPHIC/OTHER/KEYWORD.BM";
static const char *const kDialogueGameOverBitmapPath = "1:/GRAPHIC/OTHER/GAMEOVER.BM";
static const char *const kDialogueGameOverPalettePath = "1:/GRAPHIC/PAL/GAMEOVER.PAL";
static const char *const kDialogueGameOverMusicPath = "SOUND/MUSIC/ANXIETY.CMP";

static const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;
static const byte kShadowColor = 0;
static const byte kTransparentPaletteIndex = 0;

static void syncDialogueSharedState(Common::Serializer &s, DialogueSharedState &state) {
	syncDialogueBool(s, state.boyleGascanApplicationState);
	syncDialogueBool(s, state.dialogueStateD2e98);
	syncDialogueBool(s, state.dialogueStateD2eb0);
	syncDialogueBool(s, state.dialogueStateD2eb8);
	syncDialogueBool(s, state.dialogueStateD2ebc);
	syncDialogueBool(s, state.dialogueStateD2ec0);
	syncDialogueBool(s, state.dialogueStateD2ec8);
	syncDialogueBool(s, state.dialogueStateD2eec);
	syncDialogueBool(s, state.dialogueStateD2ed0);
	syncDialogueBool(s, state.dialogueStateD2ef4);
	syncDialogueBool(s, state.dialogueStateD2f04);
	syncDialogueBool(s, state.dialogueStateD2ea4);
	syncDialogueBool(s, state.dialogueStateD2ea8);
	syncDialogueBool(s, state.dialogueStateD2f00);
	syncDialogueBool(s, state.karinKidnapedDialogueState);
	syncDialogueBool(s, state.discussedLodgeTopic);
	syncDialogueBool(s, state.dialogueStateD2f08);
	syncDialogueBool(s, state.waspWomanDialogueState);
	syncDialogueBool(s, state.momGoodCauseDay5State);
	syncDialogueBool(s, state.moynahanKarinKidnapedDiscussionState);
	syncDialogueBool(s, state.dwayneWhaleyDisciplineFollowupState);
	syncDialogueBool(s, state.dwayneDiscussedBoylesButton);
	syncDialogueBool(s, state.dwayneCompletedKarinAliveFollowup);
	syncDialogueBool(s, state.discussedMrPottsTuesdayNightAlibi);
	syncDialogueBool(s, state.discussedMrsPottsTuesdayNightAlibi);
	syncDialogueBool(s, state.confrontedMrPottsAboutSpyhole);
	syncDialogueInt(s, state.serializedTalkStateFlagD2cd8);
	syncDialogueInt(s, state.dwaynePendingKarinAliveFollowupState);
	syncDialogueInt(s, state.dialogueStateD2eac);
	syncDialogueInt(s, state.dialogueStateD2eb4);
	syncDialogueInt(s, state.discussedNoteCheckbookEvidence);
	syncDialogueInt(s, state.discussedTvDeedEvidence);
	syncDialogueInt(s, state.discussedLedgerEvidence);
	syncDialogueInt(s, state.discussedCasketPhotoEvidence);
	syncDialogueInt(s, state.discussedWhaleyHerrillPhoto);
	syncDialogueInt(s, state.discussedKarinPurse);
	syncDialogueInt(s, state.momFatherTopicState);
	syncDialogueInt(s, state.dadMeatPermissionState);
	syncDialogueInt(s, state.sergeantCompletedFirstTaskState);
	syncDialogueInt(s, state.dialogueStateD2f30);
}

static const CftFontResource *findStartupFontByName(const HarvesterEngine &engine, const char *fontName) {
	const Text *text = engine.getText();
	if (!text || !fontName)
		return nullptr;

	for (const CftFontResource &font : text->getFonts()) {
		if (font.name.equalsIgnoreCase(fontName))
			return &font;
	}

	return nullptr;
}

static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	screen.copyRectToSurface(bitmap.pixels.data(), bitmap.width, x, y, bitmap.width, bitmap.height);
}

static void blitTransparentBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	int srcX = 0;
	int srcY = 0;
	int destX = x;
	int destY = y;
	int width = (int)bitmap.width;
	int height = (int)bitmap.height;

	if (destX < 0) {
		srcX = -destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0) {
		srcY = -destY;
		height += destY;
		destY = 0;
	}
	if (destX >= screen.w || destY >= screen.h || width <= 0 || height <= 0)
		return;

	width = MIN<int>(width, screen.w - destX);
	height = MIN<int>(height, screen.h - destY);
	if (width <= 0 || height <= 0)
		return;

	const byte *src = bitmap.pixels.data() + srcY * bitmap.width + srcX;
	byte *dst = (byte *)screen.getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, screen.pitch, bitmap.width, width, height,
		screen.format.bytesPerPixel, kTransparentPaletteIndex);
}

static void drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, Graphics::TextAlign align = Graphics::kTextAlignLeft) {
	font.drawString(&screen, text, x + 1, y + 1, width, kShadowColor, align);
	font.drawString(&screen, text, x, y, width, color, align);
}

static void setScaledPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

static bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap = IndexedBitmap();
	bitmap.width = READ_LE_UINT32(data.data());
	bitmap.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = bitmap.width * bitmap.height;
	if (bitmap.width == 0 || bitmap.height == 0 || data.size() < 12 + pixelCount)
		return false;

	bitmap.pixels.resize(pixelCount);
	memcpy(bitmap.pixels.data(), data.data() + 12, pixelCount);
	return true;
}

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *dest) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(dest, data.data(), 256 * 3);
	return true;
}

static bool isPrimaryMouseDown() {
	return g_system && g_system->getEventManager() &&
		((g_system->getEventManager()->getButtonState() & 1) != 0);
}

static bool isSecondaryMouseDown() {
	return g_system && g_system->getEventManager() &&
		((g_system->getEventManager()->getButtonState() & 2) != 0);
}

static int getDialogueTextLineHeight(const Graphics::Font &font) {
	return font.getFontHeight() + 2;
}

static int resolveDialogueTextboxIndex(uint lineCount) {
	if (lineCount <= 1)
		return 0;
	if (lineCount < 3)
		return 1;
	if (lineCount < 4)
		return 2;
	if (lineCount < 6)
		return 3;
	if (lineCount < 7)
		return 4;
	if (lineCount < 9)
		return 5;
	if (lineCount < 11)
		return 6;
	return 7;
}

static int resolveDialogueResponseTextboxIndex(uint responseRowCount) {
	if (responseRowCount == 0)
		return 0;
	if (responseRowCount < 3)
		return 3;
	if (responseRowCount < 4)
		return 4;
	if (responseRowCount < 6)
		return 5;
	return 6;
}

static void drawDialogueTextLines(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::Array<Common::String> &lines, int x, int y, int width) {
	const int lineHeight = getDialogueTextLineHeight(font);
	for (uint i = 0; i < lines.size(); ++i)
		font.drawString(&screen, lines[i], x, y + (int)i * lineHeight, width, 0);
}

static void wrapDialogueTextLikeNative(const Graphics::Font &font, bool usesCft,
		const Common::String &text, int width, Common::Array<Common::String> &lines) {
	lines.clear();
	if (text.empty())
		return;

	if (!usesCft || width <= 0) {
		font.wordWrapText(text, width, lines);
		if (lines.empty())
			lines.push_back(text);
		return;
	}

	Common::String wrappedText;
	for (uint i = 0; i < text.size(); ++i) {
		const char c = text[i];
		if (c != '\r')
			wrappedText += c;
	}

	const int wrapCharsPerLine = width / MAX<int>(1, font.getCharWidth(' ') - 1);
	if (wrapCharsPerLine <= 0) {
		lines.push_back(wrappedText);
		return;
	}

	uint lineStart = 0;
	while (lineStart < wrappedText.size()) {
		uint lineEnd = lineStart;
		while (lineEnd < wrappedText.size() && wrappedText[lineEnd] != '\n')
			++lineEnd;

		if (lineEnd - lineStart > (uint)wrapCharsPerLine) {
			uint breakPos = MIN<uint>(lineStart + (uint)wrapCharsPerLine, lineEnd - 1);
			while (breakPos > lineStart && wrappedText[breakPos] != ' ')
				--breakPos;

			if (breakPos > lineStart && wrappedText[breakPos] == ' ') {
				wrappedText.setChar('\n', breakPos);
				while (breakPos + 1 < wrappedText.size() && wrappedText[breakPos + 1] == ' ')
					wrappedText.deleteChar(breakPos + 1);
				lineStart = breakPos + 1;
				continue;
			}
		}

		lineStart = lineEnd + 1;
	}

	Common::String line;
	for (uint i = 0; i < wrappedText.size(); ++i) {
		if (wrappedText[i] == '\n') {
			lines.push_back(line);
			line.clear();
			continue;
		}

		line += wrappedText[i];
	}

	if (!line.empty() || lines.empty())
		lines.push_back(line);
}

static void splitDialogueMenuLine(const Common::String &line, Common::Array<Common::String> &parts) {
	parts.clear();
	if (line.empty())
		return;

	Common::String token;
	for (uint i = 0; i < line.size(); ++i) {
		if (line[i] == '/') {
			parts.push_back(token);
			token.clear();
			continue;
		}

		token += line[i];
	}

	parts.push_back(token);
}

static void logDialogueMenuItems(const char *label, int sourceLineIndex,
		const Common::String &rawLine, const Common::Array<Common::String> &items) {
	if (sourceLineIndex >= 0) {
		debugC(1, kDebugDialogue, "Harvester: %s source line 0x%x (%d) raw='%s'",
			label, sourceLineIndex, sourceLineIndex + 1, rawLine.c_str());
	} else {
		debugC(1, kDebugDialogue, "Harvester: %s source raw='%s'", label, rawLine.c_str());
	}

	if (items.empty()) {
		debugC(1, kDebugDialogue, "Harvester: %s has no explicit items", label);
		return;
	}

	for (uint i = 0; i < items.size(); ++i)
		debugC(1, kDebugDialogue, "Harvester: %s item[%u]='%s'", label, i + 1, items[i].c_str());
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

static Common::String buildDialogueVoicePath(const Script &script, int wavId) {
	if (wavId <= 0)
		return Common::String();

	return Common::String::format("%s%d.CMP", script.getVoicePath().c_str(), wavId);
}

static int getDialogueMenuItemAt(const Graphics::Font &font, uint itemCount, const Common::Point &mousePos) {
	if (itemCount == 0)
		return -1;
	if (mousePos.x < kDialogueTopicStartX || mousePos.x > kDialogueTopicEndX)
		return -1;
	if (mousePos.y < kDialogueKeywordTextY)
		return -1;

	const int lineHeight = getDialogueTextLineHeight(font);
	const int selection = (mousePos.y - kDialogueKeywordTextY) / MAX<int>(1, lineHeight);
	return selection >= 0 && selection < (int)itemCount ? selection : -1;
}

static bool isDialogueOtherHit(const Common::Point &mousePos) {
	return mousePos.x >= kDialogueTopicStartX && mousePos.x <= kDialogueTopicEndX &&
		mousePos.y >= kDialogueOtherStartY && mousePos.y <= kDialogueOtherEndY;
}

class RoomNpcDialogueSession : public DialogueRuntimeCallbacks {
public:
	struct DialogueResponseOptionLayout {
		Common::String text;
		Common::Array<Common::String> wrappedLines;
		int rowStart = 0;
		int rowCount = 0;
	};

	RoomNpcDialogueSession(HarvesterEngine &engine, Common::Point &mousePos, Flow &flow,
			const IndexedBitmap &backdrop, const byte *palette, float paletteBrightness,
			const NpcRecord &npc)
		: _engine(engine), _mousePos(mousePos), _flow(flow), _backdrop(backdrop),
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

		if (const CftFontResource *subtitleFontResource = findStartupFontByName(_engine, "TEXTFONT")) {
			_subtitleCftFont.reset(new HarvesterCftFont(*subtitleFontResource));
			if (!_subtitleCftFont->isValid())
				_subtitleCftFont.reset();
		}
		if (const CftFontResource *menuFontResource = findStartupFontByName(_engine, "TEXTFNT2")) {
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

		if (!loadBitmapResource(*resources, kDialogueKeywordBitmapPath, _keywordBitmap))
			return;

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

	bool isValid() const {
		return _script && _text && _art && _fallbackFont && _backdrop.isValid() &&
			_palette && _keywordBitmap.isValid();
	}

	const Common::String &getGenericByeTopic() const {
		return _genericByeTopic;
	}

	Common::Error playDialogueLineWithVariant(int wavId, const Common::String &speakerId,
			int headVariant) override {
		setActiveSpeakerPortrait(speakerId, headVariant);

		Common::String subtitleText;
		const bool textEnabled = _script->getDialogueTextMode() != kStartupDialogueTextNone &&
			_text->resolveDialogueSubtitle(wavId, subtitleText);
		Common::Array<Common::String> subtitleLines;
		const IndexedBitmap *textboxBitmap = nullptr;
		if (textEnabled) {
			wrapDialogueTextLikeNative(*_subtitleFont, _subtitleFontUsesCft,
				subtitleText, kDialogueSubtitleTextWidth, subtitleLines);
			textboxBitmap = _art->getTextboxBitmap(resolveDialogueTextboxIndex(subtitleLines.size()));
		}

		const Common::String voicePath = buildDialogueVoicePath(*_script, wavId);
		const bool voiceStarted = !voicePath.empty() && _engine.playSpeech(voicePath);
		debugC(2, kDebugDialogue,
			"Harvester: dialogue line wav=0x%x speaker='%s' headVariant=%d voice='%s' subtitle='%s'",
			wavId, speakerId.c_str(), headVariant, voicePath.c_str(),
			textEnabled ? subtitleText.c_str() : "");
		Common::Error releaseError = waitForPointerRelease();
		if (releaseError.getCode() != Common::kNoError) {
			_engine.stopSpeech();
			return releaseError;
		}

		bool interrupted = false;
		Graphics::FrameLimiter limiter(g_system, 60);
		for (;;) {
			drawDialogueOverlay(textboxBitmap, textEnabled ? &subtitleLines : nullptr, nullptr, -1, false, nullptr);

			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (DialogueFlowAccess::handleSystemEvent(_flow, event, result)) {
					_engine.stopSpeech();
					return result;
				}

				switch (event.type) {
				case Common::EVENT_LBUTTONDOWN:
				case Common::EVENT_RBUTTONDOWN:
					interrupted = true;
					break;
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
							event.kbd.keycode == Common::KEYCODE_RETURN ||
							event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
							event.kbd.keycode == Common::KEYCODE_SPACE) {
						interrupted = true;
					}
					break;
				default:
					break;
				}
			}

			if (_entityManager)
				(void)_entityManager->syncCursorEntityPosition(_mousePos);
			if (interrupted || (!voiceStarted || !_engine.isSpeechPlaying()))
				break;

			limiter.delayBeforeSwap();
			limiter.startFrame();
		}

		_engine.stopSpeech();
		if (interrupted) {
			Common::Error releaseResult = waitForPointerRelease();
			if (releaseResult.getCode() != Common::kNoError)
				return releaseResult;
		} else if (textEnabled && _script->getDialogueTextMode() == kStartupDialogueTextClick) {
			Graphics::FrameLimiter clickLimiter(g_system, 60);
			for (;;) {
				drawDialogueOverlay(textboxBitmap, &subtitleLines, nullptr, -1, false, nullptr);

				bool continuePressed = false;
				Common::Event event;
				while (g_system->getEventManager()->pollEvent(event)) {
					Common::Error result = Common::kNoError;
					if (DialogueFlowAccess::handleSystemEvent(_flow, event, result))
						return result;

					switch (event.type) {
					case Common::EVENT_LBUTTONDOWN:
					case Common::EVENT_RBUTTONDOWN:
						continuePressed = true;
						break;
					case Common::EVENT_KEYDOWN:
						if (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
								event.kbd.keycode == Common::KEYCODE_RETURN ||
								event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
								event.kbd.keycode == Common::KEYCODE_SPACE) {
							continuePressed = true;
						}
						break;
					default:
						break;
					}
				}

				if (_entityManager)
					(void)_entityManager->syncCursorEntityPosition(_mousePos);
				if (continuePressed)
					break;

				clickLimiter.delayBeforeSwap();
				clickLimiter.startFrame();
			}

			Common::Error releaseResult = waitForPointerRelease();
			if (releaseResult.getCode() != Common::kNoError)
				return releaseResult;
		}

		return Common::kNoError;
	}

	Common::Error playDialogueLine(int wavId, const Common::String &speakerId) override {
		return playDialogueLineWithVariant(wavId, speakerId, 0);
	}

	Common::Error playDialogueEntrySequence(const DialogueLineEntry *lines, uint count) override {
		for (uint i = 0; i < count; ++i) {
			Common::Error lineError = playDialogueLineWithVariant(
				lines[i].wavId, lines[i].speakerId, lines[i].headVariant);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return Common::kNoError;
	}

	Common::Error playDialogueFst(const Common::String &path) override {
		FstPlayer fstPlayer(_engine);
		if (!fstPlayer.play(path))
			return Common::kReadingFailed;

		return Common::kNoError;
	}

	Common::Error clearScreenToBlack() override {
		Graphics::Screen *screen = _engine.getScreen();
		if (!screen)
			return Common::kReadingFailed;

		byte blackPalette[256 * 3];
		memset(blackPalette, 0, sizeof(blackPalette));
		screen->fillRect(screen->getBounds(), 0);
		setScaledPalette(*screen, blackPalette, 1.0f);
		screen->makeAllDirty();
		screen->update();
		return Common::kNoError;
	}

	Common::Error showCdChangePrompt(int discNumber) override {
		if (discNumber <= 0)
			return Common::kNoError;

		ResourceManager *resources = _engine.getResources();
		if (!resources)
			return Common::kReadingFailed;
		const int previousDisc = resources->getCurrentDisc();

		if (_engine.shouldShowCdChangePrompts()) {
			Graphics::Screen *screen = _engine.getScreen();
			if (!screen)
				return Common::kReadingFailed;

			const Common::String bitmapPath =
				Common::String::format("1:/GRAPHIC/OTHER/CD%d.BM", discNumber);
			IndexedBitmap promptBitmap;
			byte promptPalette[256 * 3];
			if (!loadBitmapResource(*resources, bitmapPath, promptBitmap) ||
					!loadPaletteResource(*resources, kCdChangePromptPalettePath, promptPalette)) {
				warning("Harvester: unable to load CD change prompt assets for disc %d", discNumber);
				return Common::kReadingFailed;
			}

			_engine.stopMusic();

			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (DialogueFlowAccess::handleSystemEvent(_flow, event, result))
					return result;
				if (event.type == Common::EVENT_MOUSEMOVE)
					_mousePos = event.mouse;
			}

			bool waitingForRelease = false;
			bool needsRedraw = true;
			bool activateRequestedDisc = false;
			Graphics::FrameLimiter limiter(g_system, 60);

			while (!_engine.shouldQuit() && !activateRequestedDisc) {
				if (needsRedraw) {
					screen->fillRect(screen->getBounds(), 0);
					blitBitmap(*screen, promptBitmap, 0, 0);
					setScaledPalette(*screen, promptPalette, 1.0f);
					if (_entityManager)
						_entityManager->drawCursor(*screen);
					screen->makeAllDirty();
					screen->update();
					needsRedraw = false;
				}

				while (g_system->getEventManager()->pollEvent(event)) {
					Common::Error result = Common::kNoError;
					if (DialogueFlowAccess::handleSystemEvent(_flow, event, result))
						return result;

					switch (event.type) {
					case Common::EVENT_MOUSEMOVE:
						_mousePos = event.mouse;
						needsRedraw = true;
						break;
					case Common::EVENT_LBUTTONDOWN:
						waitingForRelease = true;
						break;
					case Common::EVENT_LBUTTONUP:
						if (waitingForRelease)
							activateRequestedDisc = true;
						break;
					default:
						break;
					}
				}

				if (_entityManager && _entityManager->syncCursorEntityPosition(_mousePos))
					needsRedraw = true;

				limiter.delayBeforeSwap();
				limiter.startFrame();
			}
		}

		if (!_engine.activateDisc(discNumber)) {
			warning("Harvester: unable to activate disc %d resources", discNumber);
			return Common::kReadingFailed;
		}
		if ((discNumber == 1 || discNumber == 3) &&
				previousDisc > 0 && previousDisc != resources->getCurrentDisc()) {
			Script *script = _engine.getScript();
			if (!script || !script->reloadTownWorld(*resources)) {
				warning("Harvester: unable to reload town script after disc prompt %d -> %d",
					previousDisc, discNumber);
				return Common::kReadingFailed;
			}
		}

		return Common::kNoError;
	}

	Common::Error runKeywordMenu(const Common::String &topicBuffer, int topicBufferLineIndex,
			Common::String &selectedTopic,
			DialogueKeywordMenuSelectionState &selection) override {
		selectedTopic.clear();
		selection = DialogueKeywordMenuSelectionState();
		Common::Array<Common::String> topics;
		splitDialogueMenuLine(topicBuffer, topics);
		logDialogueMenuItems("Keyword menu", topicBufferLineIndex, topicBuffer, topics);
		topics.push_back(_genericByeTopic);
		debugC(1, kDebugDialogue, "Harvester: keyword menu default topic='%s'", _genericByeTopic.c_str());
		bool editingOther = false;
		Common::String typedTopic;
		Common::Error releaseError = waitForPointerRelease();
		if (releaseError.getCode() != Common::kNoError)
			return releaseError;

		Graphics::FrameLimiter limiter(g_system, 60);
		for (;;) {
			const int hoveredTopicIndex = editingOther ? -1 : getDialogueMenuItemAt(*_menuFont, topics.size(), _mousePos);
			const bool hoverOther = !editingOther && isDialogueOtherHit(_mousePos);
			drawDialogueOverlay(&_keywordBitmap, nullptr, &topics, hoveredTopicIndex, hoverOther,
				editingOther ? &typedTopic : nullptr);

			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (DialogueFlowAccess::handleSystemEvent(_flow, event, result))
					return result;

				if (editingOther) {
					if (event.type == Common::EVENT_KEYDOWN) {
						if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
							typedTopic.clear();
							editingOther = false;
							debugC(1, kDebugDialogue, "Harvester: keyword menu cancelled Other input");
							if (_entityManager)
								_entityManager->showCursor();
						} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
							if (!typedTopic.empty())
								typedTopic.deleteLastChar();
						} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
								event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
							if (!typedTopic.empty()) {
								selectedTopic = typedTopic;
								selection.fromTypedInput = true;
								debugC(1, kDebugDialogue, "Harvester: keyword menu typed topic='%s'",
									selectedTopic.c_str());
								if (_entityManager)
									_entityManager->showCursor();
								return Common::kNoError;
							}
						} else if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7e &&
								typedTopic.size() < 79) {
							typedTopic += (char)event.kbd.ascii;
						}
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
					const int clickedTopic = getDialogueMenuItemAt(*_menuFont, topics.size(), _mousePos);
					if (clickedTopic >= 0) {
						selectedTopic = topics[(uint)clickedTopic];
						selection.fromGenericBye = clickedTopic == (int)topics.size() - 1;
						debugC(1, kDebugDialogue, "Harvester: keyword menu selected topic[%d]='%s'",
							clickedTopic + 1, selectedTopic.c_str());
						return Common::kNoError;
					}
					if (isDialogueOtherHit(_mousePos)) {
						typedTopic.clear();
						editingOther = true;
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

	Common::Error runResponseMenu(int responseLineIndex, int &selectedIndex) override {
		const Common::String responseLine = _text->getDialogueResponseLine(responseLineIndex);
		return runResponseMenuTextInternal(responseLine, responseLineIndex, selectedIndex);
	}

	Common::Error runResponseMenuText(const Common::String &responseLine, int &selectedIndex) override {
		return runResponseMenuTextInternal(responseLine, -1, selectedIndex);
	}

	Common::Error runGameOverScreen() override {
		Graphics::Screen *screen = _engine.getScreen();
		ResourceManager *resources = _engine.getResources();
		if (!screen || !resources)
			return Common::kReadingFailed;

		IndexedBitmap gameOverBitmap;
		byte gameOverPalette[256 * 3];
		if (!loadBitmapResource(*resources, kDialogueGameOverBitmapPath, gameOverBitmap) ||
				!loadPaletteResource(*resources, kDialogueGameOverPalettePath, gameOverPalette)) {
			return Common::kReadingFailed;
		}

		_engine.stopSpeech();
		_engine.stopMusic();
		_engine.stopSound();

		screen->fillRect(screen->getBounds(), 0);
		blitBitmap(*screen, gameOverBitmap, 0, 0);
		setScaledPalette(*screen, gameOverPalette, 1.0f);
		screen->makeAllDirty();
		screen->update();

		(void)_engine.playMusic(kDialogueGameOverMusicPath);

		bool dismissed = false;
		Graphics::FrameLimiter limiter(g_system, 60);
		while (!dismissed && !SHOULD_QUIT) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (DialogueFlowAccess::handleSystemEvent(_flow, event, result)) {
					_engine.stopMusic();
					return result;
				}

				switch (event.type) {
				case Common::EVENT_LBUTTONDOWN:
				case Common::EVENT_RBUTTONDOWN:
				case Common::EVENT_KEYDOWN:
					dismissed = true;
					break;
				default:
					break;
				}
			}

			if (_entityManager)
				(void)_entityManager->syncCursorEntityPosition(_mousePos);
			if (dismissed)
				break;

			limiter.delayBeforeSwap();
			limiter.startFrame();
		}

		_engine.stopMusic();
		if (dismissed) {
			Common::Error releaseError = waitForPointerRelease();
			if (releaseError.getCode() != Common::kNoError)
				return releaseError;
		}

		DialogueFlowAccess::requestMainMenuReturn(_flow);
		return Common::kNoError;
	}

	void assignTopicBuffer(Common::String &topicBuffer, int &topicBufferLineIndex,
			int responseLineIndex, const char *label) override {
		topicBuffer = _text->getDialogueResponseLine(responseLineIndex);
		topicBufferLineIndex = responseLineIndex;

		Common::Array<Common::String> topics;
		splitDialogueMenuLine(topicBuffer, topics);
		logDialogueMenuItems(label, responseLineIndex, topicBuffer, topics);
	}

	bool matchesResponseLine(const Common::String &selectedTopic, int responseLineIndex) override {
		const Common::String topicText = _text->getDialogueResponseLine(responseLineIndex);
		return !topicText.empty() && selectedTopic.equalsIgnoreCase(topicText);
	}

	bool matchesAnyResponseLine(const Common::String &selectedTopic,
			const int *responseLineIndices, uint responseLineCount) override {
		for (uint i = 0; i < responseLineCount; ++i) {
			if (matchesResponseLine(selectedTopic, responseLineIndices[i]))
				return true;
		}

		return false;
	}

	void queueDialogueInteractionIfNeeded(const InteractionResult &interaction) override {
		if (interaction.abortRemainingCommandChain || interaction.mutatedRuntimeState ||
				!interaction.musicPath.empty() || !interaction.nextRoomName.empty() ||
				!interaction.cutscenePath.empty() ||
				!interaction.deathFlicPath.empty() || interaction.requestMainMenu ||
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
			DialogueFlowAccess::queueDialogueInteraction(_flow, interaction);
		}
	}

	void applyImmediateDialogueInteractionEffects(InteractionResult &interaction) override {
		if (!interaction.musicPath.empty())
			(void)_engine.playMusic(interaction.musicPath);
		if (!interaction.audioCommands.empty())
			DialogueFlowAccess::executeStartupAudioCommands(_flow, interaction.audioCommands);
		interaction.musicPath.clear();
		interaction.audioCommands.clear();
	}

	int getRandomNumber(int maxValue) override {
		return _engine.getRandomNumber(maxValue);
	}

	void setActiveSpeakerPortrait(const Common::String &speakerId, int headVariant) override {
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

private:
	Graphics::Screen *getActiveScreen() const {
		return _engine.getScreen();
	}

	Common::Error waitForPointerRelease() {
		Graphics::FrameLimiter limiter(g_system, 60);
		while (!_engine.shouldQuit() && (isPrimaryMouseDown() || isSecondaryMouseDown())) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (DialogueFlowAccess::handleSystemEvent(_flow, event, result))
					return result;
			}

			if (_entityManager)
				(void)_entityManager->syncCursorEntityPosition(_mousePos);
			limiter.delayBeforeSwap();
			limiter.startFrame();
		}

		return Common::kNoError;
	}

	void drawFontString(const Graphics::Font &font, bool usesCft, const Common::String &text,
			int x, int y, int width, byte color) {
		Graphics::Screen *activeScreen = getActiveScreen();
		if (!activeScreen)
			return;

		if (usesCft)
			font.drawString(activeScreen, text, x, y, width, 0);
		else
			drawShadowedString(*activeScreen, font, text, x, y, width, color);
	}

	void ensureSpeakerPortrait(const Common::String &speakerId, int headVariant) {
		if (speakerId.empty())
			return;

		const Common::String headId = buildDialogueHeadId(speakerId, headVariant);
		IndexedBitmap *targetBitmap = speakerId.equalsIgnoreCase("PC") ? &_rightHeadBitmap : &_leftHeadBitmap;
		Common::String *targetSpeakerId = speakerId.equalsIgnoreCase("PC") ? &_rightHeadSpeakerId : &_leftHeadSpeakerId;
		if (targetBitmap->isValid() && targetSpeakerId->equalsIgnoreCase(headId))
			return;

		IndexedBitmap updatedBitmap;
		if (loadDialogueHeadBitmap(_engine, speakerId, headVariant, updatedBitmap)) {
			*targetBitmap = updatedBitmap;
			*targetSpeakerId = headId;
		} else {
			warning("Harvester: unable to load dialogue head for '%s'", headId.c_str());
		}
	}

	void drawDialogueOverlay(const IndexedBitmap *overlayBitmap,
			const Common::Array<Common::String> *subtitleLines, const Common::Array<Common::String> *topics,
			int hoveredTopicIndex, bool hoverOther, const Common::String *textEntryValue) {
		Graphics::Screen *activeScreen = getActiveScreen();
		if (!activeScreen)
			return;

		setScaledPalette(*activeScreen, _palette, _paletteBrightness);
		blitBitmap(*activeScreen, _backdrop, 0, 0);
		if (_leftHeadVisible && _leftHeadBitmap.isValid())
			blitTransparentBitmap(*activeScreen, _leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
		if (_rightHeadVisible && _rightHeadBitmap.isValid())
			blitTransparentBitmap(*activeScreen, _rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
		if (overlayBitmap && overlayBitmap->isValid())
			blitTransparentBitmap(*activeScreen, *overlayBitmap, kDialogueOverlayX, kDialogueOverlayY);

		if (subtitleLines) {
			if (_subtitleFontUsesCft) {
				drawDialogueTextLines(*activeScreen, *_subtitleFont, *subtitleLines,
					kDialogueSubtitleTextX, kDialogueSubtitleTextY, kDialogueSubtitleTextWidth);
			} else {
				const int lineHeight = getDialogueTextLineHeight(*_subtitleFont);
				for (uint i = 0; i < subtitleLines->size(); ++i) {
					drawShadowedString(*activeScreen, *_subtitleFont, (*subtitleLines)[i],
						kDialogueSubtitleTextX,
						kDialogueSubtitleTextY + (int)i * lineHeight,
						kDialogueSubtitleTextWidth, kTextColorNormal);
				}
			}
		}

		if (topics) {
			const int lineHeight = getDialogueTextLineHeight(*_menuFont);
			for (uint i = 0; i < topics->size(); ++i) {
				const bool highlighted = (int)i == hoveredTopicIndex;
				const Graphics::Font &font = highlighted ? *_highlightFont : *_menuFont;
				const bool usesCft = highlighted ? _highlightFontUsesCft : _menuFontUsesCft;
				drawFontString(font, usesCft, (*topics)[i], kDialogueKeywordTextX,
					kDialogueKeywordTextY + (int)i * lineHeight, kDialogueKeywordTextWidth,
					highlighted ? kTextColorNormal : kTextColorHover);
			}

			const Graphics::Font &otherFont = hoverOther ? *_highlightFont : *_menuFont;
			const bool otherUsesCft = hoverOther ? _highlightFontUsesCft : _menuFontUsesCft;
			drawFontString(otherFont, otherUsesCft, "Other", kDialogueOtherX, kDialogueOtherY,
				kDialogueOtherWidth, hoverOther ? kTextColorNormal : kTextColorHover);
		}

		if (textEntryValue) {
			drawFontString(*_highlightFont, _highlightFontUsesCft,
				*textEntryValue + "_", kDialogueTextEntryX, kDialogueTextEntryY,
				kDialogueKeywordTextWidth - 30, kTextColorNormal);
		}

		if (_entityManager)
			_entityManager->drawCursor(*activeScreen);
		activeScreen->makeAllDirty();
		activeScreen->update();
	}

	void parseResponseMenuLine(const Common::String &responseLine,
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
			option.rowStart = (int)totalRows;
			for (const Common::String &explicitLine : explicitLines) {
				Common::Array<Common::String> wrappedExplicitLines;
				wrapDialogueTextLikeNative(*_menuFont, _menuFontUsesCft,
					explicitLine, kDialogueSubtitleTextWidth, wrappedExplicitLines);
				if (wrappedExplicitLines.empty())
					wrappedExplicitLines.push_back(explicitLine);
				for (const Common::String &wrappedExplicitLine : wrappedExplicitLines)
					option.wrappedLines.push_back(wrappedExplicitLine);
			}
			option.rowCount = MAX<int>(1, option.wrappedLines.size());
			totalRows += option.rowCount;
			options.push_back(option);

			explicitLines.clear();
			optionText.clear();
		};

		auto consumeSegment = [&](const Common::String &rawSegment) {
			Common::String segmentText = rawSegment;
			if (segmentText.empty())
				return;

			uint digitEnd = 0;
			while (digitEnd < segmentText.size() && segmentText[digitEnd] >= '0' &&
					segmentText[digitEnd] <= '9') {
				++digitEnd;
			}
			const bool startsNewOption = digitEnd > 0 && digitEnd < segmentText.size() &&
				segmentText[digitEnd] == '.';
			if (startsNewOption) {
				flushOption();

				Common::String optionLine = segmentText.substr(digitEnd + 1);
				optionLine.trim();
				explicitLines.push_back(optionLine);
				optionText = optionLine;
				return;
			}

			explicitLines.push_back(segmentText);
			if (!optionText.empty())
				optionText += ' ';
			optionText += segmentText;
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

	int getResponseMenuItemAt(const Common::Array<DialogueResponseOptionLayout> &options,
			uint totalRows, const Common::Point &mousePos) {
		if (mousePos.x < kDialogueTopicStartX || mousePos.x > kDialogueTopicEndX)
			return -1;
		if (mousePos.y < kDialogueKeywordTextY)
			return -1;

		const int lineHeight = getDialogueTextLineHeight(*_menuFont);
		const int row = (mousePos.y - kDialogueKeywordTextY) / MAX<int>(1, lineHeight);
		if (row < 0 || row >= (int)totalRows)
			return -1;

		for (uint i = 0; i < options.size(); ++i) {
			const DialogueResponseOptionLayout &option = options[i];
			if (row >= option.rowStart && row < option.rowStart + option.rowCount)
				return (int)i;
		}

		return -1;
	}

	void drawDialogueResponseMenu(const IndexedBitmap *textboxBitmap,
			const Common::Array<DialogueResponseOptionLayout> &options, int hoveredOptionIndex) {
		Graphics::Screen *activeScreen = getActiveScreen();
		if (!activeScreen)
			return;

		setScaledPalette(*activeScreen, _palette, _paletteBrightness);
		blitBitmap(*activeScreen, _backdrop, 0, 0);
		if (_leftHeadVisible && _leftHeadBitmap.isValid())
			blitTransparentBitmap(*activeScreen, _leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
		if (_rightHeadVisible && _rightHeadBitmap.isValid())
			blitTransparentBitmap(*activeScreen, _rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
		if (textboxBitmap && textboxBitmap->isValid())
			blitTransparentBitmap(*activeScreen, *textboxBitmap, kDialogueOverlayX, kDialogueOverlayY);

		const Common::String title = "Responses";
		const Graphics::Font &titleFont = *_highlightFont;
		const bool titleUsesCft = _highlightFontUsesCft;
		const int titleWidth = titleFont.getStringWidth(title);
		const int titleX = kDialogueSubtitleTextX + MAX<int>(0, (kDialogueSubtitleTextWidth - titleWidth) / 2);
		drawFontString(titleFont, titleUsesCft, title, titleX, 11, titleWidth, kTextColorNormal);

		const int lineHeight = getDialogueTextLineHeight(*_menuFont);
		int drawY = kDialogueKeywordTextY;
		for (uint i = 0; i < options.size(); ++i) {
			const bool highlighted = (int)i == hoveredOptionIndex;
			const Graphics::Font &font = highlighted ? *_highlightFont : *_menuFont;
			const bool usesCft = highlighted ? _highlightFontUsesCft : _menuFontUsesCft;
			const byte color = highlighted ? kTextColorNormal : kTextColorHover;

			for (const Common::String &wrappedLine : options[i].wrappedLines) {
				drawFontString(font, usesCft, wrappedLine, kDialogueSubtitleTextX, drawY,
					kDialogueSubtitleTextWidth, color);
				drawY += lineHeight;
			}
		}

		if (_entityManager)
			_entityManager->drawCursor(*activeScreen);
		activeScreen->makeAllDirty();
		activeScreen->update();
	}

	Common::Error runResponseMenuTextInternal(const Common::String &responseLine, int responseLineIndex,
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
		for (const DialogueResponseOptionLayout &option : options)
			optionTexts.push_back(option.text);
		logDialogueMenuItems("Response menu", responseLineIndex, responseLine, optionTexts);

		const IndexedBitmap *textboxBitmap = _art->getTextboxBitmap(resolveDialogueResponseTextboxIndex(totalRows));
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
				if (DialogueFlowAccess::handleSystemEvent(_flow, event, result))
					return result;

				if (event.type == Common::EVENT_KEYDOWN) {
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						if (responseLineIndex >= 0) {
							debugC(1, kDebugDialogue, "Harvester: response menu line 0x%x (%d) cancelled",
								responseLineIndex, responseLineIndex + 1);
						} else {
							debugC(1, kDebugDialogue, "Harvester: response menu text cancelled");
						}
						return Common::kNoError;
					}
					if (event.kbd.ascii >= '1' && event.kbd.ascii <= '9') {
						const int menuIndex = event.kbd.ascii - '0';
						if (menuIndex >= 1 && menuIndex <= (int)options.size()) {
							selectedIndex = menuIndex;
							if (responseLineIndex >= 0) {
								debugC(1, kDebugDialogue,
									"Harvester: response menu line 0x%x (%d) selected option[%d]='%s' via hotkey",
									responseLineIndex, responseLineIndex + 1,
									selectedIndex, options[(uint)(selectedIndex - 1)].text.c_str());
							} else {
								debugC(1, kDebugDialogue,
									"Harvester: response menu text selected option[%d]='%s' via hotkey",
									selectedIndex, options[(uint)(selectedIndex - 1)].text.c_str());
							}
							return waitForPointerRelease();
						}
					}
				} else if (event.type == Common::EVENT_LBUTTONDOWN && hoveredOptionIndex >= 0) {
					selectedIndex = hoveredOptionIndex + 1;
					if (responseLineIndex >= 0) {
						debugC(1, kDebugDialogue,
							"Harvester: response menu line 0x%x (%d) selected option[%d]='%s' via click",
							responseLineIndex, responseLineIndex + 1,
							selectedIndex, options[(uint)hoveredOptionIndex].text.c_str());
					} else {
						debugC(1, kDebugDialogue,
							"Harvester: response menu text selected option[%d]='%s' via click",
							selectedIndex, options[(uint)hoveredOptionIndex].text.c_str());
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

	HarvesterEngine &_engine;
	Common::Point &_mousePos;
	Flow &_flow;
	const IndexedBitmap &_backdrop;
	const byte *_palette;
	const float _paletteBrightness;
	const NpcRecord &_npc;
	Script *_script;
	Text *_text;
	Art *_art;
	EntityManager *_entityManager;
	const Graphics::Font *_fallbackFont;
	Common::ScopedPtr<HarvesterCftFont> _subtitleCftFont;
	Common::ScopedPtr<HarvesterCftFont> _menuCftFont;
	const Graphics::Font *_subtitleFont;
	const Graphics::Font *_menuFont;
	const Graphics::Font *_highlightFont;
	bool _subtitleFontUsesCft;
	bool _menuFontUsesCft;
	bool _highlightFontUsesCft;
	IndexedBitmap _keywordBitmap;
	Common::String _genericByeTopic;
	IndexedBitmap _leftHeadBitmap;
	IndexedBitmap _rightHeadBitmap;
	Common::String _leftHeadSpeakerId;
	Common::String _rightHeadSpeakerId;
	bool _leftHeadVisible;
	bool _rightHeadVisible;
};

} // End of anonymous namespace

DialogueSystem::DialogueSystem(HarvesterEngine &engine, Common::Point &mousePos)
	: _engine(engine), _mousePos(mousePos) {
	registerNpcHandlers();
}

DialogueSystem::~DialogueSystem() {
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		delete _npcHandlers[i];
}

bool DialogueSystem::hasRoomNpcHandler(const Common::String &npcName) const {
	for (uint i = 0; i < _npcHandlers.size(); ++i) {
		if (_npcHandlers[i]->matchesNpc(npcName))
			return true;
	}

	return false;
}

void DialogueSystem::registerNpcHandlers() {
	_npcHandlers.push_back(new AuthorityDialogueHandler());
	_npcHandlers.push_back(new BeggarDialogueHandler());
	_npcHandlers.push_back(new BoyleDialogueHandler());
	_npcHandlers.push_back(new BusterDialogueHandler());
	_npcHandlers.push_back(new ButcherDialogueHandler());
	_npcHandlers.push_back(new DwayneDialogueHandler());
	_npcHandlers.push_back(new EdnaDialogueHandler());
	_npcHandlers.push_back(new Fireman1DialogueHandler());
	_npcHandlers.push_back(new Fireman2DialogueHandler());
	_npcHandlers.push_back(new ChessmasterDialogueHandler());
	_npcHandlers.push_back(new CloakAtndDialogueHandler());
	_npcHandlers.push_back(new CuratorDialogueHandler());
	_npcHandlers.push_back(new DadDialogueHandler());
	_npcHandlers.push_back(new DarkWomanDialogueHandler());
	_npcHandlers.push_back(new JimmyDialogueHandler());
	_npcHandlers.push_back(new InquisitorDialogueHandler());
	_npcHandlers.push_back(new KarinDialogueHandler());
	_npcHandlers.push_back(new WaspWomanDialogueHandler());
	_npcHandlers.push_back(new GladiatorDialogueHandler());
	_npcHandlers.push_back(new LibrarianDialogueHandler());
	_npcHandlers.push_back(new LoomisDialogueHandler());
	_npcHandlers.push_back(new McknightDialogueHandler());
	_npcHandlers.push_back(new MembDirDialogueHandler());
	_npcHandlers.push_back(new MaintManDialogueHandler());
	_npcHandlers.push_back(new MadamDialogueHandler());
	_npcHandlers.push_back(new MomDialogueHandler());
	_npcHandlers.push_back(new MotherDialogueHandler());
	_npcHandlers.push_back(new MoynahanDialogueHandler());
	_npcHandlers.push_back(new MrPottsDialogueHandler());
	_npcHandlers.push_back(new MrsPottsDialogueHandler());
	_npcHandlers.push_back(new LodgeChefDialogueHandler());
	_npcHandlers.push_back(new NudeManDialogueHandler());
	_npcHandlers.push_back(new ParsonsDialogueHandler());
	_npcHandlers.push_back(new PastorelliDialogueHandler());
	_npcHandlers.push_back(new PhelpsDialogueHandler());
	_npcHandlers.push_back(new PriestDialogueHandler());
	_npcHandlers.push_back(new RyderDialogueHandler());
	_npcHandlers.push_back(new ValetDialogueHandler());
	_npcHandlers.push_back(new VetDialogueHandler());
	_npcHandlers.push_back(new SparkyDialogueHandler());
	_npcHandlers.push_back(new StephanieDialogueHandler());
	_npcHandlers.push_back(new SwellDialogueHandler());
	_npcHandlers.push_back(new WhaleyDialogueHandler());
	_npcHandlers.push_back(new HankDialogueHandler());
	_npcHandlers.push_back(new HerrillDialogueHandler());
	_npcHandlers.push_back(new JohnsonDialogueHandler());
	_npcHandlers.push_back(new PtaMomDialogueHandler());

	_npcHandlers.push_back(new SergeantDialogueHandler());
	_npcHandlers.push_back(new StubNpcDialogueHandler("PTA_MOM"));
}

void DialogueSystem::resetRoomNpcDialogueState() {
	_sharedDialogueState = DialogueSharedState();
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		_npcHandlers[i]->resetState();
}

void DialogueSystem::syncRuntimeSaveState(Common::Serializer &s) {
	syncDialogueSharedState(s, _sharedDialogueState);
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		_npcHandlers[i]->syncState(s);
	if (s.isLoading()) {
		for (uint i = 0; i < _npcHandlers.size(); ++i)
			_npcHandlers[i]->migrateSharedState(_sharedDialogueState);
	}
}

Common::Error DialogueSystem::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const NpcRecord &npc, const Common::String &usedItemName,
		Flow &flow) {
	Script *script = _engine.getScript();
	Text *text = _engine.getText();
	if (!script || !text || !backdrop.isValid())
		return Common::kReadingFailed;
	if (!hasRoomNpcHandler(npc.npcName)) {
		debugC(1, kDebugDialogue,
			"Harvester: blocked room NPC dialogue npc='%s' reason='no handler'",
			npc.npcName.c_str());
		return Common::kNoError;
	}
	if (!script->isNamedNpcDeathTypeClear(npc.npcName)) {
		const NpcRecord *runtimeNpc = script->findRuntimeNpcRecord(npc.npcName);
		debugC(1, kDebugDialogue,
			"Harvester: blocked room NPC dialogue npc='%s' reason='death queued' damage_type=%d",
			npc.npcName.c_str(), runtimeNpc ? runtimeNpc->deathDamageType : -1);
		return Common::kNoError;
	}

	RoomNpcDialogueSession session(_engine, _mousePos, flow, backdrop, palette, paletteBrightness, npc);
	if (!session.isValid())
		return Common::kReadingFailed;
	DialogueRuntime runtime(
		_engine, *script, *text, flow, npc.roomName, session.getGenericByeTopic(), session);

	for (uint i = 0; i < _npcHandlers.size(); ++i) {
		if (_npcHandlers[i]->matchesNpc(npc.npcName))
			return _npcHandlers[i]->handleDialogue(runtime, usedItemName, _sharedDialogueState);
	}

	debug(1, "Harvester: unsupported room NPC dialogue handler '%s'", npc.npcName.c_str());
	return Common::kNoError;
}

} // End of namespace Harvester
