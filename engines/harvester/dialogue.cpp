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

#include <functional>
#include <memory>

#include "harvester/dialogue.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
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
	syncDialogueInt(s, state.serializedTalkStateFlagD2cd8, 4);
	syncDialogueInt(s, state.dwaynePendingKarinAliveFollowupState, 4);
	syncDialogueInt(s, state.dialogueStateD2eac, 4);
	syncDialogueInt(s, state.dialogueStateD2eb4, 4);
	syncDialogueInt(s, state.discussedNoteCheckbookEvidence, 4);
	syncDialogueInt(s, state.discussedTvDeedEvidence, 4);
	syncDialogueInt(s, state.discussedLedgerEvidence, 4);
	syncDialogueInt(s, state.discussedCasketPhotoEvidence, 4);
	syncDialogueInt(s, state.discussedWhaleyHerrillPhoto, 4);
	syncDialogueInt(s, state.discussedKarinPurse, 4);
	syncDialogueInt(s, state.momFatherTopicState, 4);
	syncDialogueInt(s, state.dadMeatPermissionState, 4);
	syncDialogueInt(s, state.sergeantCompletedFirstTaskState, 4);
	syncDialogueInt(s, state.dialogueStateD2f30, 4);
}

static const CftFontResource *findStartupFontByName(const HarvesterEngine &engine, const char *fontName) {
	const Text *startupText = engine.getStartupText();
	if (!startupText || !fontName)
		return nullptr;

	for (const CftFontResource &font : startupText->getFonts()) {
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
	const float gammaBrightness = g_engine ? g_engine->getStartupGammaBrightnessScale() : 1.0f;
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
	Script *startupScript = engine.getStartupScript();
	ResourceManager *resources = engine.getResources();
	if (!startupScript || !resources || speakerId.empty())
		return false;

	const Common::String headId = buildDialogueHeadId(speakerId, headVariant);
	const StartupHeadRecord *head = startupScript->findHeadRecord(headId);
	if (!head)
		return false;

	return loadBitmapResource(*resources, head->portraitPath + ".BM", bitmap);
}

static Common::String buildDialogueVoicePath(const Script &startupScript, int wavId) {
	if (wavId <= 0)
		return Common::String();

	return Common::String::format("%s%d.CMP", startupScript.getVoicePath().c_str(), wavId);
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

} // End of anonymous namespace

DialogueSystem::DialogueSystem(HarvesterEngine &engine, Common::Point &mousePos)
	: _engine(engine), _mousePos(mousePos) {
	registerNpcHandlers();
}

DialogueSystem::~DialogueSystem() {
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		delete _npcHandlers[i];
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
		float paletteBrightness, const StartupNpcRecord &npc, const Common::String &usedItemName,
		Flow &startupFlow) {
	struct DialogueResponseOptionLayout {
		Common::String text;
		Common::Array<Common::String> wrappedLines;
		int rowStart = 0;
		int rowCount = 0;
	};

	Graphics::Screen *screen = _engine.getScreen();
	Script *startupScript = _engine.getStartupScript();
	Text *startupText = _engine.getStartupText();
	Art *startupArt = _engine.getStartupArt();
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	const Graphics::Font *fallbackFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!screen || !startupScript || !startupText || !startupArt || !fallbackFont || !backdrop.isValid())
		return Common::kReadingFailed;

	const CftFontResource *subtitleFontResource = findStartupFontByName(_engine, "TEXTFONT");
	const CftFontResource *menuFontResource = findStartupFontByName(_engine, "TEXTFNT2");
	std::unique_ptr<HarvesterCftFont> subtitleCftFont;
	std::unique_ptr<HarvesterCftFont> menuCftFont;
	if (subtitleFontResource) {
		subtitleCftFont.reset(new HarvesterCftFont(*subtitleFontResource));
		if (!subtitleCftFont->isValid())
			subtitleCftFont.reset();
	}
	if (menuFontResource) {
		menuCftFont.reset(new HarvesterCftFont(*menuFontResource));
		if (!menuCftFont->isValid())
			menuCftFont.reset();
	}

	const Graphics::Font *subtitleFont = subtitleCftFont ? static_cast<const Graphics::Font *>(subtitleCftFont.get()) : fallbackFont;
	const Graphics::Font *menuFont = menuCftFont ? static_cast<const Graphics::Font *>(menuCftFont.get()) : subtitleFont;
	const Graphics::Font *highlightFont = subtitleFont;
	const bool subtitleFontUsesCft = subtitleCftFont.get() != nullptr;
	const bool menuFontUsesCft = menuCftFont.get() != nullptr;
	const bool highlightFontUsesCft = subtitleFontUsesCft;

	IndexedBitmap keywordBitmap;
	if (!loadBitmapResource(*_engine.getResources(), kDialogueKeywordBitmapPath, keywordBitmap))
		return Common::kReadingFailed;

	Common::String genericByeTopic = startupText->getDialogueResponseLine(kDialogueGenericByeResponseIndex);
	if (genericByeTopic.empty())
		genericByeTopic = "BYE";

	auto getActiveScreen = [&]() -> Graphics::Screen * {
		return _engine.getScreen();
	};

	IndexedBitmap leftHeadBitmap;
	IndexedBitmap rightHeadBitmap;
	Common::String leftHeadSpeakerId = buildDialogueHeadId(npc.npcName, 0);
	Common::String rightHeadSpeakerId = buildDialogueHeadId("PC", 0);
	bool leftHeadVisible = false;
	bool rightHeadVisible = false;
	(void)loadDialogueHeadBitmap(_engine, npc.npcName, 0, leftHeadBitmap);
	(void)loadDialogueHeadBitmap(_engine, "PC", 0, rightHeadBitmap);
	leftHeadVisible = leftHeadBitmap.isValid();
	rightHeadVisible = rightHeadBitmap.isValid();

	auto waitForPointerRelease = [&]() -> Common::Error {
		Graphics::FrameLimiter limiter(g_system, 60);
		while (!_engine.shouldQuit() && (isPrimaryMouseDown() || isSecondaryMouseDown())) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (startupFlow.handleSystemEvent(event, result))
					return result;
			}

			if (runtimeEntities)
				(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
			limiter.delayBeforeSwap();
			limiter.startFrame();
		}

		return Common::kNoError;
	};

	auto drawFontString = [&](const Graphics::Font &font, bool usesCft, const Common::String &text,
			int x, int y, int width, byte color) {
		Graphics::Screen *activeScreen = getActiveScreen();
		if (!activeScreen)
			return;

		if (usesCft)
			font.drawString(activeScreen, text, x, y, width, 0);
		else
			drawShadowedString(*activeScreen, font, text, x, y, width, color);
	};

	auto ensureSpeakerPortrait = [&](const Common::String &speakerId, int headVariant) {
		if (speakerId.empty())
			return;

		const Common::String headId = buildDialogueHeadId(speakerId, headVariant);
		IndexedBitmap *targetBitmap = speakerId.equalsIgnoreCase("PC") ? &rightHeadBitmap : &leftHeadBitmap;
		Common::String *targetSpeakerId = speakerId.equalsIgnoreCase("PC") ? &rightHeadSpeakerId : &leftHeadSpeakerId;
		if (targetBitmap->isValid() && targetSpeakerId->equalsIgnoreCase(headId))
			return;

		IndexedBitmap updatedBitmap;
		if (loadDialogueHeadBitmap(_engine, speakerId, headVariant, updatedBitmap)) {
			*targetBitmap = updatedBitmap;
			*targetSpeakerId = headId;
		} else {
			warning("Harvester: unable to load dialogue head for '%s'", headId.c_str());
		}
	};

	auto setActiveSpeakerPortrait = [&](const Common::String &speakerId, int headVariant) {
		if (speakerId.empty() || speakerId.equalsIgnoreCase("SPEECH"))
			return;

		ensureSpeakerPortrait(speakerId, headVariant);
		if (speakerId.equalsIgnoreCase("PC")) {
			leftHeadVisible = false;
			rightHeadVisible = rightHeadBitmap.isValid();
		} else {
			leftHeadVisible = leftHeadBitmap.isValid();
			rightHeadVisible = false;
		}
	};

	auto drawDialogueOverlay = [&](const IndexedBitmap *overlayBitmap,
			const Common::Array<Common::String> *subtitleLines, const Common::Array<Common::String> *topics,
			int hoveredTopicIndex, bool hoverOther, const Common::String *textEntryValue) {
		Graphics::Screen *activeScreen = getActiveScreen();
		if (!activeScreen)
			return;

		setScaledPalette(*activeScreen, palette, paletteBrightness);
		blitBitmap(*activeScreen, backdrop, 0, 0);
		if (leftHeadVisible && leftHeadBitmap.isValid())
			blitTransparentBitmap(*activeScreen, leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
		if (rightHeadVisible && rightHeadBitmap.isValid())
			blitTransparentBitmap(*activeScreen, rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
		if (overlayBitmap && overlayBitmap->isValid())
			blitTransparentBitmap(*activeScreen, *overlayBitmap, kDialogueOverlayX, kDialogueOverlayY);

		if (subtitleLines) {
			if (subtitleFontUsesCft)
				drawDialogueTextLines(*activeScreen, *subtitleFont, *subtitleLines,
					kDialogueSubtitleTextX, kDialogueSubtitleTextY, kDialogueSubtitleTextWidth);
			else {
				const int lineHeight = getDialogueTextLineHeight(*subtitleFont);
				for (uint i = 0; i < subtitleLines->size(); ++i) {
					drawShadowedString(*activeScreen, *subtitleFont, (*subtitleLines)[i],
						kDialogueSubtitleTextX,
						kDialogueSubtitleTextY + (int)i * lineHeight,
						kDialogueSubtitleTextWidth, kTextColorNormal);
				}
			}
		}

		if (topics) {
			const int lineHeight = getDialogueTextLineHeight(*menuFont);
			for (uint i = 0; i < topics->size(); ++i) {
				const bool highlighted = (int)i == hoveredTopicIndex;
				const Graphics::Font &font = highlighted ? *highlightFont : *menuFont;
				const bool usesCft = highlighted ? highlightFontUsesCft : menuFontUsesCft;
				drawFontString(font, usesCft, (*topics)[i], kDialogueKeywordTextX,
					kDialogueKeywordTextY + (int)i * lineHeight, kDialogueKeywordTextWidth,
					highlighted ? kTextColorNormal : kTextColorHover);
			}

			const Graphics::Font &otherFont = hoverOther ? *highlightFont : *menuFont;
			const bool otherUsesCft = hoverOther ? highlightFontUsesCft : menuFontUsesCft;
			drawFontString(otherFont, otherUsesCft, "Other", kDialogueOtherX, kDialogueOtherY,
				kDialogueOtherWidth, hoverOther ? kTextColorNormal : kTextColorHover);
		}

		if (textEntryValue) {
			drawFontString(*highlightFont, highlightFontUsesCft,
				*textEntryValue + "_", kDialogueTextEntryX, kDialogueTextEntryY,
				kDialogueKeywordTextWidth - 30, kTextColorNormal);
		}

		if (runtimeEntities)
			runtimeEntities->drawCursor(*activeScreen);
		activeScreen->makeAllDirty();
		activeScreen->update();
	};

	auto playDialogueLineWithVariant = [&](int wavId, const Common::String &speakerId, int headVariant) -> Common::Error {
		setActiveSpeakerPortrait(speakerId, headVariant);

		Common::String subtitleText;
		const bool textEnabled = startupScript->getDialogueTextMode() != kStartupDialogueTextNone &&
			startupText->resolveDialogueSubtitle(wavId, subtitleText);
		Common::Array<Common::String> subtitleLines;
		const IndexedBitmap *textboxBitmap = nullptr;
		if (textEnabled) {
			wrapDialogueTextLikeNative(*subtitleFont, subtitleFontUsesCft,
				subtitleText, kDialogueSubtitleTextWidth, subtitleLines);
			textboxBitmap = startupArt->getTextboxBitmap(resolveDialogueTextboxIndex(subtitleLines.size()));
		}

		const Common::String voicePath = buildDialogueVoicePath(*startupScript, wavId);
		const bool voiceStarted = !voicePath.empty() && _engine.playStartupSpeech(voicePath);
		debugC(2, kDebugDialogue,
			"Harvester: dialogue line wav=0x%x speaker='%s' headVariant=%d voice='%s' subtitle='%s'",
			wavId, speakerId.c_str(), headVariant, voicePath.c_str(),
			textEnabled ? subtitleText.c_str() : "");
		Common::Error releaseError = waitForPointerRelease();
		if (releaseError.getCode() != Common::kNoError) {
			_engine.stopStartupSpeech();
			return releaseError;
		}

		bool interrupted = false;
		Graphics::FrameLimiter limiter(g_system, 60);
		for (;;) {
			drawDialogueOverlay(textboxBitmap, textEnabled ? &subtitleLines : nullptr, nullptr, -1, false, nullptr);

			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (startupFlow.handleSystemEvent(event, result)) {
					_engine.stopStartupSpeech();
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

			if (runtimeEntities)
				(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
			if (interrupted || (!voiceStarted || !_engine.isStartupSpeechPlaying()))
				break;

			limiter.delayBeforeSwap();
			limiter.startFrame();
		}

		_engine.stopStartupSpeech();
		if (interrupted) {
			Common::Error releaseResult = waitForPointerRelease();
			if (releaseResult.getCode() != Common::kNoError)
				return releaseResult;
		} else if (textEnabled && startupScript->getDialogueTextMode() == kStartupDialogueTextClick) {
			Graphics::FrameLimiter clickLimiter(g_system, 60);
			for (;;) {
				drawDialogueOverlay(textboxBitmap, &subtitleLines, nullptr, -1, false, nullptr);

				bool continuePressed = false;
				Common::Event event;
				while (g_system->getEventManager()->pollEvent(event)) {
					Common::Error result = Common::kNoError;
					if (startupFlow.handleSystemEvent(event, result))
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

				if (runtimeEntities)
					(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
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
	};

	auto playDialogueLine = [&](int wavId, const Common::String &speakerId) -> Common::Error {
		return playDialogueLineWithVariant(wavId, speakerId, 0);
	};

	auto playDialogueEntrySequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		for (uint i = 0; i < count; ++i) {
			Common::Error lineError = playDialogueLineWithVariant(
				lines[i].wavId, lines[i].speakerId, lines[i].headVariant);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return Common::kNoError;
	};

	auto playDialogueFst = [&](const Common::String &path) -> Common::Error {
		FstPlayer fstPlayer(_engine);
		if (!fstPlayer.play(path))
			return Common::kReadingFailed;

		return Common::kNoError;
	};
	auto clearScreenToBlack = [&]() -> Common::Error {
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
	};
	auto showCdChangePrompt = [&](int discNumber) -> Common::Error {
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

			_engine.stopStartupMusic();

			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (startupFlow.handleSystemEvent(event, result))
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
					if (runtimeEntities)
						runtimeEntities->drawCursor(*screen);
					screen->makeAllDirty();
					screen->update();
					needsRedraw = false;
				}

				while (g_system->getEventManager()->pollEvent(event)) {
					Common::Error result = Common::kNoError;
					if (startupFlow.handleSystemEvent(event, result))
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

				if (runtimeEntities && runtimeEntities->syncCursorEntityPosition(_mousePos))
					needsRedraw = true;

				limiter.delayBeforeSwap();
				limiter.startFrame();
			}
		}

		if (!_engine.activateStartupDisc(discNumber)) {
			warning("Harvester: unable to activate disc %d resources", discNumber);
			return Common::kReadingFailed;
		}
		if (discNumber == 3 && previousDisc > 0 && previousDisc != resources->getCurrentDisc()) {
			Script *startupScript = _engine.getStartupScript();
			if (!startupScript || !startupScript->reloadTownWorld(*resources)) {
				warning("Harvester: unable to reload town script after disc prompt %d -> %d",
					previousDisc, discNumber);
				return Common::kReadingFailed;
			}
		}

		return Common::kNoError;
	};
	auto runGameOverScreen = [&]() -> Common::Error {
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

		_engine.stopStartupSpeech();
		_engine.stopStartupMusic();
		_engine.stopStartupSound();

		screen->fillRect(screen->getBounds(), 0);
		blitBitmap(*screen, gameOverBitmap, 0, 0);
		setScaledPalette(*screen, gameOverPalette, 1.0f);
		screen->makeAllDirty();
		screen->update();

		(void)_engine.playStartupMusic(kDialogueGameOverMusicPath);

		bool dismissed = false;
		Graphics::FrameLimiter limiter(g_system, 60);
		while (!dismissed && !SHOULD_QUIT) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (startupFlow.handleSystemEvent(event, result)) {
					_engine.stopStartupMusic();
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

			if (runtimeEntities)
				(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
			if (dismissed)
				break;

			limiter.delayBeforeSwap();
			limiter.startFrame();
		}

		_engine.stopStartupMusic();
		if (dismissed) {
			Common::Error releaseError = waitForPointerRelease();
			if (releaseError.getCode() != Common::kNoError)
				return releaseError;
		}

		startupFlow.requestMainMenuReturn();
		return Common::kNoError;
	};

	auto queueDialogueInteractionIfNeeded = [&](const StartupInteractionResult &interaction) {
		if (interaction.abortRemainingCommandChain || interaction.mutatedRuntimeState ||
				!interaction.musicPath.empty() || !interaction.nextRoomName.empty() ||
				!interaction.cutscenePath.empty() ||
				!interaction.deathFlicPath.empty() || interaction.requestMainMenu ||
				interaction.requestRoomRestart ||
				interaction.cdChangeDisc > 0 ||
				!interaction.dialogueNpcName.empty() ||
				!interaction.dialogueContinuationTag.empty() ||
				!interaction.continuationTag.empty() ||
				!interaction.modalText.value.empty() ||
				interaction.lightingCommand != kStartupLightingCommandNone ||
				interaction.requestPlayerGotoXZ ||
				!interaction.audioCommands.empty()) {
			startupFlow.queueDialogueInteraction(interaction);
		}
	};
	auto applyImmediateDialogueInteractionEffects = [&](StartupInteractionResult &interaction) {
		if (!interaction.musicPath.empty())
			(void)_engine.playStartupMusic(interaction.musicPath);
		if (!interaction.audioCommands.empty())
			startupFlow.executeStartupAudioCommands(interaction.audioCommands);
		interaction.musicPath.clear();
		interaction.audioCommands.clear();
	};

	auto parseResponseMenuLine = [&](const Common::String &responseLine,
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
				wrapDialogueTextLikeNative(*menuFont, menuFontUsesCft,
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
	};

	auto buildResponseMenuLayout = [&](const Common::String &responseLine,
			Common::Array<DialogueResponseOptionLayout> &options, uint &totalRows) {
		parseResponseMenuLine(responseLine, options, totalRows);
	};

	auto getResponseMenuItemAt = [&](const Common::Array<DialogueResponseOptionLayout> &options,
			uint totalRows, const Common::Point &mousePos) -> int {
		if (mousePos.x < kDialogueTopicStartX || mousePos.x > kDialogueTopicEndX)
			return -1;
		if (mousePos.y < kDialogueKeywordTextY)
			return -1;

		const int lineHeight = getDialogueTextLineHeight(*menuFont);
		const int row = (mousePos.y - kDialogueKeywordTextY) / MAX<int>(1, lineHeight);
		if (row < 0 || row >= (int)totalRows)
			return -1;

		for (uint i = 0; i < options.size(); ++i) {
			const DialogueResponseOptionLayout &option = options[i];
			if (row >= option.rowStart && row < option.rowStart + option.rowCount)
				return (int)i;
		}

		return -1;
	};

	auto drawDialogueResponseMenu = [&](const IndexedBitmap *textboxBitmap,
			const Common::Array<DialogueResponseOptionLayout> &options, int hoveredOptionIndex) {
		Graphics::Screen *activeScreen = getActiveScreen();
		if (!activeScreen)
			return;

		setScaledPalette(*activeScreen, palette, paletteBrightness);
		blitBitmap(*activeScreen, backdrop, 0, 0);
		if (leftHeadVisible && leftHeadBitmap.isValid())
			blitTransparentBitmap(*activeScreen, leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
		if (rightHeadVisible && rightHeadBitmap.isValid())
			blitTransparentBitmap(*activeScreen, rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
		if (textboxBitmap && textboxBitmap->isValid())
			blitTransparentBitmap(*activeScreen, *textboxBitmap, kDialogueOverlayX, kDialogueOverlayY);

		const Common::String title = "Responses";
		const Graphics::Font &titleFont = *highlightFont;
		const bool titleUsesCft = highlightFontUsesCft;
		const int titleWidth = titleFont.getStringWidth(title);
		const int titleX = kDialogueSubtitleTextX + MAX<int>(0, (kDialogueSubtitleTextWidth - titleWidth) / 2);
		drawFontString(titleFont, titleUsesCft, title, titleX, 11, titleWidth, kTextColorNormal);

		const int lineHeight = getDialogueTextLineHeight(*menuFont);
		int drawY = kDialogueKeywordTextY;
		for (uint i = 0; i < options.size(); ++i) {
			const bool highlighted = (int)i == hoveredOptionIndex;
			const Graphics::Font &font = highlighted ? *highlightFont : *menuFont;
			const bool usesCft = highlighted ? highlightFontUsesCft : menuFontUsesCft;
			const byte color = highlighted ? kTextColorNormal : kTextColorHover;

			for (const Common::String &wrappedLine : options[i].wrappedLines) {
				drawFontString(font, usesCft, wrappedLine, kDialogueSubtitleTextX, drawY,
					kDialogueSubtitleTextWidth, color);
				drawY += lineHeight;
			}
		}

		if (runtimeEntities)
			runtimeEntities->drawCursor(*activeScreen);
		activeScreen->makeAllDirty();
		activeScreen->update();
	};

	auto runResponseMenuText = [&](const Common::String &responseLine, int responseLineIndex,
			int &selectedIndex) -> Common::Error {
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
		buildResponseMenuLayout(responseLine, options, totalRows);
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

		const IndexedBitmap *textboxBitmap = startupArt->getTextboxBitmap(resolveDialogueResponseTextboxIndex(totalRows));
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
				if (startupFlow.handleSystemEvent(event, result))
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

			if (runtimeEntities)
				(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
			limiter.delayBeforeSwap();
			limiter.startFrame();
		}
	};

	auto runResponseMenu = [&](int responseLineIndex, int &selectedIndex) -> Common::Error {
		const Common::String responseLine = startupText->getDialogueResponseLine(responseLineIndex);
		return runResponseMenuText(responseLine, responseLineIndex, selectedIndex);
	};

	auto runKeywordMenu = [&](const Common::String &topicBuffer, int topicBufferLineIndex,
			Common::String &selectedTopic,
			DialogueRuntime::KeywordMenuSelectionState &selection) -> Common::Error {
		selectedTopic.clear();
		selection = DialogueRuntime::KeywordMenuSelectionState();
		Common::Array<Common::String> topics;
		splitDialogueMenuLine(topicBuffer, topics);
		logDialogueMenuItems("Keyword menu", topicBufferLineIndex, topicBuffer, topics);
		topics.push_back(genericByeTopic);
		debugC(1, kDebugDialogue, "Harvester: keyword menu default topic='%s'", genericByeTopic.c_str());
		bool editingOther = false;
		Common::String typedTopic;
		Common::Error releaseError = waitForPointerRelease();
		if (releaseError.getCode() != Common::kNoError)
			return releaseError;

		Graphics::FrameLimiter limiter(g_system, 60);
		for (;;) {
			const int hoveredTopicIndex = editingOther ? -1 : getDialogueMenuItemAt(*menuFont, topics.size(), _mousePos);
			const bool hoverOther = !editingOther && isDialogueOtherHit(_mousePos);
			drawDialogueOverlay(&keywordBitmap, nullptr, &topics, hoveredTopicIndex, hoverOther,
				editingOther ? &typedTopic : nullptr);

			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (startupFlow.handleSystemEvent(event, result))
					return result;

				if (editingOther) {
					if (event.type == Common::EVENT_KEYDOWN) {
						if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
							typedTopic.clear();
							editingOther = false;
							debugC(1, kDebugDialogue, "Harvester: keyword menu cancelled Other input");
							if (runtimeEntities)
								runtimeEntities->showCursor();
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
								if (runtimeEntities)
									runtimeEntities->showCursor();
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
						selectedTopic = genericByeTopic;
						selection.fromEscape = true;
						selection.fromGenericBye = true;
						debugC(1, kDebugDialogue,
							"Harvester: keyword menu selected default topic via escape '%s'",
							selectedTopic.c_str());
						return Common::kNoError;
					}
					break;
				case Common::EVENT_LBUTTONDOWN: {
					const int clickedTopic = getDialogueMenuItemAt(*menuFont, topics.size(), _mousePos);
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
						if (runtimeEntities)
							runtimeEntities->hideCursor();
					}
					break;
				}
				default:
					break;
				}
			}

			if (runtimeEntities)
				(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
			limiter.delayBeforeSwap();
			limiter.startFrame();
		}
	};

	auto matchesResponseLine = [&](const Common::String &selectedTopic, int responseLineIndex) {
		const Common::String topicText = startupText->getDialogueResponseLine(responseLineIndex);
		return !topicText.empty() && selectedTopic.equalsIgnoreCase(topicText);
	};

	auto matchesAnyResponseLine = [&](const Common::String &selectedTopic, const int *responseLineIndices,
			uint responseLineCount) {
		for (uint i = 0; i < responseLineCount; ++i) {
			if (matchesResponseLine(selectedTopic, responseLineIndices[i]))
				return true;
		}

		return false;
	};

	auto assignTopicBuffer = [&](Common::String &topicBuffer, int &topicBufferLineIndex,
			int responseLineIndex, const char *label) {
		topicBuffer = startupText->getDialogueResponseLine(responseLineIndex);
		topicBufferLineIndex = responseLineIndex;

		Common::Array<Common::String> topics;
		splitDialogueMenuLine(topicBuffer, topics);
		logDialogueMenuItems(label, responseLineIndex, topicBuffer, topics);
	};

	DialogueRuntime runtime(
		_engine, *startupScript, *startupText, startupFlow, npc.roomName, genericByeTopic,
		playDialogueLineWithVariant, playDialogueLine, playDialogueEntrySequence,
		playDialogueFst, clearScreenToBlack, showCdChangePrompt, runKeywordMenu, runResponseMenu,
		[&](const Common::String &responseLine, int &selectedIndex) {
			return runResponseMenuText(responseLine, -1, selectedIndex);
		},
		runGameOverScreen,
		assignTopicBuffer,
		matchesResponseLine, matchesAnyResponseLine, queueDialogueInteractionIfNeeded,
		applyImmediateDialogueInteractionEffects,
		[&](int maxValue) { return _engine.getRandomNumber(maxValue); },
		setActiveSpeakerPortrait);

	for (uint i = 0; i < _npcHandlers.size(); ++i) {
		if (_npcHandlers[i]->matchesNpc(npc.npcName))
			return _npcHandlers[i]->handleDialogue(runtime, usedItemName, _sharedDialogueState);
	}

	debug(1, "Harvester: unsupported room NPC dialogue handler '%s'", npc.npcName.c_str());
	return Common::kNoError;
}

} // End of namespace Harvester
