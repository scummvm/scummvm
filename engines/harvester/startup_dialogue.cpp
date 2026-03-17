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

#include <memory>

#include "harvester/startup_dialogue.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/startup_art.h"
#include "harvester/startup_flow.h"
#include "harvester/startup_text.h"

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
static const char *const kDialogueKeywordBitmapPath = "1:/GRAPHIC/OTHER/KEYWORD.BM";
static const char *const kDialogueRangshotFstPath = "GRAPHIC/FST/RANGSHOT.FST";
static const char *const kDialogueC008AFstPath = "GRAPHIC/FST/C008A.FST";
static const char *const kDialogueC008BFstPath = "GRAPHIC/FST/C008B.FST";
static const char *const kBabyGurgleActionTag = "BABY_GURGLE";
static const char *const kShownEvidenceOfBlackmailFlag = "SHOWN_EVIDENCE_OF_BLACKMAIL";
static const char *const kShownEvidenceSheriffOwnsFlag = "SHOWN_EVIDENCE_SHERIFF_OWNS";
static const char *const kShownLedgersToAnyoneFlag = "SHOWN_LEDGERS_TO_ANYONE_OTH";
static const char *const kShownPhotoOfCorpseFlag = "SHOWN_PHOTO_OF_CORPSE_AROUN";
static const char *const kShownPhotoOfWhaleyHerrillFlag = "SHOWN_PHOTO_OF_WHALEY_HERRI";

static const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;
static const byte kShadowColor = 0;
static const byte kTransparentPaletteIndex = 0;

struct HankDialogueTopicLine {
	int responseLineIndex;
	int wavId;
	const char *speakerId;
	bool setDiscussedLodgeTopic;
};

struct DialogueLineSpec {
	DialogueLineSpec(int wavId, const char *speakerId, int headVariant = 0)
		: wavId(wavId), speakerId(speakerId), headVariant(headVariant) {}

	int wavId;
	const char *speakerId;
	int headVariant;
};

struct DialogueLineEntry {
	int wavId;
	const char *speakerId;
	int headVariant;
};

static const HankDialogueTopicLine kHankDialogueTopicLines[] = {
	{ 0xd2, 0x725, "HANK", false },
	{ 0xd6, 0x74f, "PC", false },
	{ 0xd8, 0x76d, "PC", false },
	{ 0xd9, 0x76d, "PC", false },
	{ 0xdb, 0x77b, "PC", false },
	{ 0xdc, 0x77b, "PC", false },
	{ 0xe0, 0x7ad, "HANK", false },
	{ 0xe1, 0x7b7, "PC", false },
	{ 0xe3, 0x7d6, "PC", false },
	{ 0xe5, 0x7f4, "HANK", false },
	{ 0xe6, 0x7fc, "PC", false },
	{ 0xe9, 0x82d, "PC", false },
	{ 0xeb, 0x863, "PC", false },
	{ 0xed, 0x881, "PC", false },
	{ 0xee, 0x8a0, "HANK", false },
	{ 0xef, 0x8a0, "HANK", false },
	{ 0xf0, 0x8a0, "HANK", false },
	{ 0xf1, 0x8bf, "HANK", true }
};

static const DialogueLineEntry kMomIntroLines[] = {
	{ 0x1dd7, "MOM", 1 },
	{ 0x1ddc, "PC", 0 },
	{ 0x1de0, "MOM", 2 },
	{ 0x1de4, "PC", 2 }
};

static const DialogueLineEntry kMomCookiesLines[] = {
	{ 0x1e18, "PC", 0 },
	{ 0x1e1d, "MOM", 1 },
	{ 0x1e21, "PC", 0 },
	{ 0x1e25, "MOM", 0 },
	{ 0x1e29, "PC", 0 }
};

static const DialogueLineEntry kMomTopic119Lines[] = {
	{ 0x1e47, "PC", 0 },
	{ 0x1e4b, "MOM", 0 },
	{ 0x1e4f, "PC", 0 },
	{ 0x1e53, "MOM", 0 },
	{ 0x1f99, "MOM", 2 }
};

static const DialogueLineEntry kMomTopic11dOpeningLines[] = {
	{ 0x1e7c, "PC", 4 },
	{ 0x1e80, "MOM", 2 }
};

static const DialogueLineEntry kMomTopic11dPostC008ALines[] = {
	{ 0x1e8e, "MOM", 2 },
	{ 0x1e94, "PC", 2 }
};

static const DialogueLineEntry kMomTopic11dClosingLines[] = {
	{ 0x1e9d, "MOM", 3 },
	{ 0x1ea3, "MOM", 3 }
};

static const DialogueLineEntry kMomTopic121Lines[] = {
	{ 0x1ec0, "PC", 0 },
	{ 0x1ec4, "MOM", 2 },
	{ 0x1ec9, "PC", 0 },
	{ 0x1ecd, "MOM", 2 }
};

static const DialogueLineEntry kMomPtaLines[] = {
	{ 0x1eee, "PC", 2 },
	{ 0x1ef2, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic12bLines[] = {
	{ 0x1efd, "PC", 0 },
	{ 0x1f01, "MOM", 0 },
	{ 0x1f07, "PC", 0 },
	{ 0x1f0b, "MOM", 0 },
	{ 0x22cd, "PC", 2 },
	{ 0x22d1, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic12dLines[] = {
	{ 0x1f14, "PC", 0 },
	{ 0x1f18, "MOM", 0 },
	{ 0x1f1d, "PC", 0 },
	{ 0x1f21, "MOM", 0 },
	{ 0x1f28, "PC", 0 },
	{ 0x1f2c, "MOM", 2 },
	{ 0x1f31, "PC", 2 },
	{ 0x1f35, "MOM", 2 }
};

static const DialogueLineEntry kMomTopic12fLines[] = {
	{ 0x1f41, "PC", 4 },
	{ 0x1f45, "MOM", 3 },
	{ 0x1f4b, "PC", 0 },
	{ 0x1f4f, "MOM", 2 }
};

static const DialogueLineEntry kMomCookingLines[] = {
	{ 0x1f58, "PC", 0 },
	{ 0x1f5c, "MOM", 0 },
	{ 0x1f63, "PC", 0 },
	{ 0x1f68, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic135Lines[] = {
	{ 0x1f71, "PC", 0 },
	{ 0x1f75, "MOM", 2 }
};

static const DialogueLineEntry kMomTopic137Lines[] = {
	{ 0x1f7f, "PC", 2 },
	{ 0x1f84, "MOM", 3 },
	{ 0x1f8a, "PC", 2 },
	{ 0x1f8e, "MOM", 2 }
};

static const DialogueLineEntry kMomSparkyLines[] = {
	{ 0x1ed7, "PC", 0 },
	{ 0x1edb, "MOM", 2 },
	{ 0x1ee1, "PC", 0 },
	{ 0x1ee5, "MOM", 2 }
};

static const DialogueLineEntry kMomFatherLines[] = {
	{ 0x1fc8, "PC", 0 },
	{ 0x1fcc, "MOM", 0 }
};

static const DialogueLineEntry kMomFatherHeardDadMoanLines[] = {
	{ 0x1fd3, "PC", 0 },
	{ 0x1fd8, "MOM", 0 }
};

static const DialogueLineEntry kMomLodgeLines[] = {
	{ 0x1fe1, "PC", 0 },
	{ 0x1fe5, "MOM", 1 },
	{ 0x1feb, "PC", 0 },
	{ 0x1fef, "MOM", 1 }
};

static const DialogueLineEntry kMomLodgeResponse1Lines[] = {
	{ 0x1ffe, "MOM", 1 }
};

static const DialogueLineEntry kMomLodgeResponse2Lines[] = {
	{ 0x2005, "MOM", 2 },
	{ 0x200a, "PC", 0 },
	{ 0x200e, "MOM", 0 }
};

static const DialogueLineEntry kMomStephMidgameLodgeLines[] = {
	{ 0x2154, "PC", 0 },
	{ 0x215a, "MOM", 0 }
};

static const DialogueLineEntry kMomBakeSaleLines[] = {
	{ 0x2017, "PC", 0 },
	{ 0x201b, "MOM", 1 },
	{ 0x2020, "PC", 0 },
	{ 0x2024, "MOM", 0 },
	{ 0x2029, "PC", 0 },
	{ 0x202d, "MOM", 0 },
	{ 0x2164, "PC", 0 },
	{ 0x2168, "MOM", 1 },
	{ 0x216d, "PC", 0 },
	{ 0x2171, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic14aLines[] = {
	{ 0x2036, "MOM", 0 },
	{ 0x203a, "PC", 0 },
	{ 0x203e, "MOM", 2 }
};

static const DialogueLineEntry kMomMeatPlantIntroLines[] = {
	{ 0x20f7, "PC", 0 },
	{ 0x20fb, "MOM", 0 }
};

static const DialogueLineEntry kMomMeatPlantResponse1Lines[] = {
	{ 0x2106, "PC", 0 },
	{ 0x210a, "MOM", 0 },
	{ 0x210e, "PC", 0 }
};

static const DialogueLineEntry kMomMeatPlantResponse2Lines[] = {
	{ 0x2114, "MOM", 1 },
	{ 0x2119, "PC", 0 },
	{ 0x211d, "MOM", 1 }
};

static const DialogueLineEntry kMomTopic155Lines[] = {
	{ 0x2125, "MOM", 1 },
	{ 0x212a, "PC", 0 },
	{ 0x212e, "MOM", 1 },
	{ 0x2134, "PC", 0 },
	{ 0x2138, "MOM", 1 }
};

static const DialogueLineEntry kMomTopic156Lines[] = {
	{ 0x2142, "MOM", 0 },
	{ 0x2147, "PC", 0 },
	{ 0x214b, "MOM", 0 }
};

static const DialogueLineEntry kMomGoodCauseDay5Lines[] = {
	{ 0x21a3, "PC", 0 },
	{ 0x21a8, "MOM", 1 },
	{ 0x21ad, "PC", 0 },
	{ 0x21b2, "MOM", 0 },
	{ 0x21b7, "PC", 0 }
};

static const DialogueLineEntry kMomSlaughterhouseLines[] = {
	{ 0x1faf, "PC", 4 },
	{ 0x1fb4, "MOM", 0 },
	{ 0x1fb9, "PC", 2 },
	{ 0x1fbd, "MOM", 0 }
};

static const DialogueLineEntry kWaspWomanWhaleyLines[] = {
	{ 0x4ca6, "WASP_WOMAN", 0 },
	{ 0x4cae, "WASP_WOMAN", 1 },
	{ 0x4cb2, "PC", 0 },
	{ 0x4cb6, "WASP_WOMAN", 0 }
};

static const DialogueLineEntry kWaspWomanIntroTailLines[] = {
	{ 0x4bf6, "WASP_WOMAN", 0 },
	{ 0x4bfc, "PC", 0 }
};

static const DialogueLineEntry kWaspWomanTopic305Response1Lines[] = {
	{ 0x4c4d, "WASP_WOMAN", 1 },
	{ 0x4c53, "WASP_WOMAN", 1 }
};

static const int kMomPtaTopicResponseLines[] = { 0x125, 0x126, 0x127, 0x128 };
static const int kMomCookingTopicResponseLines[] = { 0x131, 0x132, 0x133 };
static const int kMomBakeSaleTopicResponseLines[] = { 0x145, 0x146, 0x147 };
static const int kMomPottsdamTopicResponseLines[] = { 0x14b, 0x14c };
static const int kMomMeatPlantTopicResponseLines[] = { 0x151, 0x152 };
static const int kMomMoynahanTopicResponseLines[] = { 0x160, 0x161 };
static const int kMomNewspaperFireTopicResponseLines[] = { 0x162, 0x163 };
static const int kMomInitialTopicBufferResponseLine = 0x102;
static const int kMomSameDayTopicBufferResponseLine = 0x104;
static const int kMomPostIntroTopicBufferResponseLine = 0x105;
static const int kMomDay5TopicBufferResponseLine = 0x113;
static const int kMomDay6TopicBufferResponseLine = 0x114;
static const int kMomSlaughterhouseFollowupTopicBufferResponseLine = 0x13e;
static const int kMomFatherFollowupTopicBufferResponseLine = 0x140;
static const int kMomLodgeFollowupTopicBufferResponseLine = 0x144;
static const int kMomPtaFollowupTopicBufferResponseLine = 0x12a;
static const int kMomCookingFollowupTopicBufferResponseLine = 0x134;
static const int kMomBakeSaleFollowupTopicBufferResponseLine = 0x148;
static const int kMomPottsdamFollowupTopicBufferResponseLine = 0x150;
static const int kMomMeatPlantFollowupTopicBufferResponseLine = 0x154;
static const int kMomGoodCauseDay5FollowupTopicBufferResponseLine = 0x15c;
static const int kWaspWomanTopicBufferResponseLine = 0x301;

static const CftFontResource *findStartupFontByName(const HarvesterEngine &engine, const char *fontName) {
	const StartupText *startupText = engine.getStartupText();
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

static void buildScaledPalette(const byte *source, float brightness, byte *dest) {
	memset(dest, 0, 256 * 3);
	if (!source)
		return;

	for (uint color = 1; color < 256; ++color) {
		for (uint channel = 0; channel < 3; ++channel) {
			const uint index = color * 3 + channel;
			const int scaled = (int)(source[index] * brightness + 0.5f);
			dest[index] = (byte)MIN<int>(scaled, 255);
		}
	}
}

static void setScaledPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	buildScaledPalette(palette, brightness, scaledPalette);
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
	StartupScript *startupScript = engine.getStartupScript();
	ResourceManager *resources = engine.getResources();
	if (!startupScript || !resources || speakerId.empty())
		return false;

	const Common::String headId = buildDialogueHeadId(speakerId, headVariant);
	const StartupHeadRecord *head = startupScript->findHeadRecord(headId);
	if (!head)
		return false;

	return loadBitmapResource(*resources, head->portraitPath + ".BM", bitmap);
}

static Common::String buildDialogueVoicePath(const StartupScript &startupScript, int wavId) {
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

StartupDialogueSystem::StartupDialogueSystem(HarvesterEngine &engine, Common::Point &mousePos)
	: _engine(engine), _mousePos(mousePos) {
}

void StartupDialogueSystem::resetRoomNpcDialogueState() {
	_hankRoomDialogueState = HankRoomDialogueState();
	_momRoomDialogueState = MomRoomDialogueState();
	_jimmyRoomDialogueState = JimmyRoomDialogueState();
	_waspWomanRoomDialogueState = WaspWomanRoomDialogueState();
	_sharedKarinKidnapedDialogueState = false;
	_sharedDiscussedLodgeTopic = false;
	_sharedWaspWomanDialogueState = false;
}

Common::Error StartupDialogueSystem::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const StartupNpcRecord &npc, const Common::String &usedItemName,
		StartupFlow &startupFlow) {
	struct DialogueResponseOptionLayout {
		Common::String text;
		Common::Array<Common::String> wrappedLines;
		int rowStart = 0;
		int rowCount = 0;
	};

	Graphics::Screen *screen = _engine.getScreen();
	StartupScript *startupScript = _engine.getStartupScript();
	StartupText *startupText = _engine.getStartupText();
	StartupArt *startupArt = _engine.getStartupArt();
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
		if (usesCft)
			font.drawString(screen, text, x, y, width, 0);
		else
			drawShadowedString(*screen, font, text, x, y, width, color);
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
		setScaledPalette(*screen, palette, paletteBrightness);
		blitBitmap(*screen, backdrop, 0, 0);
		if (leftHeadVisible && leftHeadBitmap.isValid())
			blitTransparentBitmap(*screen, leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
		if (rightHeadVisible && rightHeadBitmap.isValid())
			blitTransparentBitmap(*screen, rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
		if (overlayBitmap && overlayBitmap->isValid())
			blitTransparentBitmap(*screen, *overlayBitmap, kDialogueOverlayX, kDialogueOverlayY);

		if (subtitleLines) {
			if (subtitleFontUsesCft)
				drawDialogueTextLines(*screen, *subtitleFont, *subtitleLines,
					kDialogueSubtitleTextX, kDialogueSubtitleTextY, kDialogueSubtitleTextWidth);
			else {
				const int lineHeight = getDialogueTextLineHeight(*subtitleFont);
				for (uint i = 0; i < subtitleLines->size(); ++i) {
					drawShadowedString(*screen, *subtitleFont, (*subtitleLines)[i],
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
			runtimeEntities->drawCursor(*screen);
		screen->makeAllDirty();
		screen->update();
	};

	auto playDialogueLineWithVariant = [&](int wavId, const Common::String &speakerId, int headVariant) -> Common::Error {
		setActiveSpeakerPortrait(speakerId, headVariant);

		Common::String subtitleText;
		const bool textEnabled = startupScript->getDialogueTextMode() != kStartupDialogueTextNone &&
			startupText->resolveDialogueSubtitle(wavId, subtitleText);
		Common::Array<Common::String> subtitleLines;
		const IndexedBitmap *textboxBitmap = nullptr;
		if (textEnabled) {
			subtitleFont->wordWrapText(subtitleText, kDialogueSubtitleTextWidth, subtitleLines);
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

	// Multi-line exchanges in the original binary are emitted as consecutive play_dialogue_line calls.
	auto playDialogueSequence = [&](const DialogueLineSpec *lines, uint count) -> Common::Error {
		for (uint i = 0; i < count; ++i) {
			Common::Error lineError = playDialogueLineWithVariant(
				lines[i].wavId, lines[i].speakerId, lines[i].headVariant);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return Common::kNoError;
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

	auto queueDialogueInteractionIfNeeded = [&](const StartupInteractionResult &interaction) {
		if (interaction.abortRemainingCommandChain || interaction.mutatedRuntimeState ||
				!interaction.musicPath.empty() || !interaction.nextRoomName.empty() ||
				!interaction.deathFlicPath.empty() || interaction.requestMainMenu ||
				!interaction.dialogueNpcName.empty() ||
				!interaction.dialogueContinuationTag.empty() ||
				!interaction.audioCommands.empty()) {
			startupFlow.queueDialogueInteraction(interaction);
		}
	};

	auto buildResponseMenuLayout = [&](const Common::String &responseLine,
			Common::Array<DialogueResponseOptionLayout> &options, uint &totalRows) {
		options.clear();
		totalRows = 0;

		Common::Array<Common::String> rawOptions;
		splitDialogueMenuLine(responseLine, rawOptions);
		for (const Common::String &rawOption : rawOptions) {
			DialogueResponseOptionLayout option;
			option.text = rawOption;
			menuFont->wordWrapText(rawOption, kDialogueSubtitleTextWidth, option.wrappedLines);
			if (option.wrappedLines.empty())
				option.wrappedLines.push_back(rawOption);
			option.rowStart = (int)totalRows;
			option.rowCount = MAX<int>(1, option.wrappedLines.size());
			totalRows += option.rowCount;
			options.push_back(option);
		}
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
		setScaledPalette(*screen, palette, paletteBrightness);
		blitBitmap(*screen, backdrop, 0, 0);
		if (leftHeadVisible && leftHeadBitmap.isValid())
			blitTransparentBitmap(*screen, leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
		if (rightHeadVisible && rightHeadBitmap.isValid())
			blitTransparentBitmap(*screen, rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
		if (textboxBitmap && textboxBitmap->isValid())
			blitTransparentBitmap(*screen, *textboxBitmap, kDialogueOverlayX, kDialogueOverlayY);

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
			runtimeEntities->drawCursor(*screen);
		screen->makeAllDirty();
		screen->update();
	};

	auto runResponseMenu = [&](int responseLineIndex, int &selectedIndex) -> Common::Error {
		selectedIndex = 0;

		const Common::String responseLine = startupText->getDialogueResponseLine(responseLineIndex);
		if (responseLine.empty()) {
			debugC(1, kDebugDialogue, "Harvester: response menu line 0x%x (%d) is empty",
				responseLineIndex, responseLineIndex + 1);
			return Common::kNoError;
		}

		Common::Array<DialogueResponseOptionLayout> options;
		uint totalRows = 0;
		buildResponseMenuLayout(responseLine, options, totalRows);
		if (options.empty()) {
			debugC(1, kDebugDialogue, "Harvester: response menu line 0x%x (%d) produced no options",
				responseLineIndex, responseLineIndex + 1);
			return Common::kNoError;
		}

		Common::Array<Common::String> rawOptions;
		splitDialogueMenuLine(responseLine, rawOptions);
		logDialogueMenuItems("Response menu", responseLineIndex, responseLine, rawOptions);

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
						debugC(1, kDebugDialogue, "Harvester: response menu line 0x%x (%d) cancelled",
							responseLineIndex, responseLineIndex + 1);
						return Common::kNoError;
					}
					if (event.kbd.ascii >= '1' && event.kbd.ascii <= '9') {
						const int menuIndex = event.kbd.ascii - '0';
						if (menuIndex >= 1 && menuIndex <= (int)options.size()) {
							selectedIndex = menuIndex;
							debugC(1, kDebugDialogue,
								"Harvester: response menu line 0x%x (%d) selected option[%d]='%s' via hotkey",
								responseLineIndex, responseLineIndex + 1,
								selectedIndex, options[(uint)(selectedIndex - 1)].text.c_str());
							return waitForPointerRelease();
						}
					}
				} else if (event.type == Common::EVENT_LBUTTONDOWN && hoveredOptionIndex >= 0) {
					selectedIndex = hoveredOptionIndex + 1;
					debugC(1, kDebugDialogue,
						"Harvester: response menu line 0x%x (%d) selected option[%d]='%s' via click",
						responseLineIndex, responseLineIndex + 1,
						selectedIndex, options[(uint)hoveredOptionIndex].text.c_str());
					return waitForPointerRelease();
				}
			}

			if (runtimeEntities)
				(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
			limiter.delayBeforeSwap();
			limiter.startFrame();
		}
	};

	auto runKeywordMenu = [&](const Common::String &topicBuffer, int topicBufferLineIndex,
			Common::String &selectedTopic) -> Common::Error {
		selectedTopic.clear();
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

	if (npc.npcName.equalsIgnoreCase("JIMMY")) {
		auto playJimmyLine = [&](int wavId, int headVariant) -> Common::Error {
			return playDialogueLineWithVariant(wavId, "JIMMY", headVariant);
		};
		auto hasInventoryItem = [&](const char *objectName) {
			Common::Array<StartupObjectRecord> inventoryObjects;
			startupScript->getVisibleInventoryObjects(inventoryObjects);
			for (const StartupObjectRecord &inventoryObject : inventoryObjects) {
				if (inventoryObject.objectName.equalsIgnoreCase(objectName))
					return true;
			}

			return false;
		};

		if (!usedItemName.empty()) {
			if (usedItemName.equalsIgnoreCase("NEWSPAPER")) {
				StartupInteractionResult jimmyInteraction;
				const bool changedGivenPaperToday =
					startupScript->setRuntimeFlagValue("GIVEN_PAPER_TODAY", true);
				const bool changedNewspaperState =
					startupScript->resetRuntimeObjectToInitialState("NEWSPAPER");
				jimmyInteraction.mutatedRuntimeState = changedGivenPaperToday || changedNewspaperState;

				StartupInteractionResult actionInteraction;
				if (startupScript->executeNestedActionTag("ACTV_HOUSE_EXIT", actionInteraction)) {
					jimmyInteraction.abortRemainingCommandChain =
						jimmyInteraction.abortRemainingCommandChain ||
						actionInteraction.abortRemainingCommandChain;
					jimmyInteraction.mutatedRuntimeState =
						jimmyInteraction.mutatedRuntimeState || actionInteraction.mutatedRuntimeState;
					if (!actionInteraction.musicPath.empty())
						jimmyInteraction.musicPath = actionInteraction.musicPath;
					if (!actionInteraction.nextRoomName.empty())
						jimmyInteraction.nextRoomName = actionInteraction.nextRoomName;
					if (!actionInteraction.deathFlicPath.empty())
						jimmyInteraction.deathFlicPath = actionInteraction.deathFlicPath;
					if (!actionInteraction.dialogueNpcName.empty())
						jimmyInteraction.dialogueNpcName = actionInteraction.dialogueNpcName;
					if (!actionInteraction.dialogueContinuationTag.empty()) {
						jimmyInteraction.dialogueContinuationTag =
							actionInteraction.dialogueContinuationTag;
					}
					jimmyInteraction.requestMainMenu =
						jimmyInteraction.requestMainMenu || actionInteraction.requestMainMenu;
					for (const StartupAudioCommand &command : actionInteraction.audioCommands)
						jimmyInteraction.audioCommands.push_back(command);
				}
				if (jimmyInteraction.abortRemainingCommandChain || jimmyInteraction.mutatedRuntimeState ||
						!jimmyInteraction.musicPath.empty() ||
						!jimmyInteraction.nextRoomName.empty() || !jimmyInteraction.deathFlicPath.empty() ||
						jimmyInteraction.requestMainMenu || !jimmyInteraction.dialogueNpcName.empty() ||
						!jimmyInteraction.dialogueContinuationTag.empty() ||
						!jimmyInteraction.audioCommands.empty()) {
					startupFlow.queueDialogueInteraction(jimmyInteraction);
				}

				if (!_jimmyRoomDialogueState.paperHandoffStateSet) {
					_jimmyRoomDialogueState.paperHandoffStateSet = true;
					Common::Error lineError = playJimmyLine(0x4a4c, 1);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}

				return _engine.getRandomNumber(1) == 0
					? playJimmyLine(0x4acc, 0)
					: playJimmyLine(0x4a4b, 1);
			}
			if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
				(void)startupScript->setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
				return playJimmyLine(0x4af8, 1);
			}
			if (((usedItemName.equalsIgnoreCase("LEDGER") ||
						usedItemName.equalsIgnoreCase("LEDGER2")) &&
					startupScript->getFlagValue("HAVE_BOTH_LEDGERS")) ||
					usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
					usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
				if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
						usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
					(void)startupScript->setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
				} else {
					(void)startupScript->setRuntimeFlagValue(kShownLedgersToAnyoneFlag, true);
				}
				return playJimmyLine(0x4b00, 0);
			}
			if (usedItemName.equalsIgnoreCase("NOTE") ||
					usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
					usedItemName.equalsIgnoreCase("CHECKBOOK") ||
					usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
				(void)startupScript->setRuntimeFlagValue(kShownEvidenceOfBlackmailFlag, true);
				return playJimmyLine(0x4b21, 0);
			}
			if (usedItemName.equalsIgnoreCase("SNEAKERS")) {
				Common::Error lineError = playJimmyLine(0x4a9e, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			return playJimmyLine(0x4af2, 0);
		}

		if (_jimmyRoomDialogueState.firstNoItemLinePending) {
			_jimmyRoomDialogueState.firstNoItemLinePending = false;
			if (!_jimmyRoomDialogueState.paperHandoffStateSet) {
				Common::Error lineError = playJimmyLine(0x4a4c, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			Common::Error lineError = playJimmyLine(0x4a58, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (hasInventoryItem("SNEAKERS") && !hasInventoryItem("BROOMKEY")) {
			Common::Error lineError = playJimmyLine(0x4ac3, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("PAPER_CHK_4")) {
			Common::Error lineError = playJimmyLine(0x4ae2, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("PAPER_CHK_3")) {
			Common::Error lineError = playJimmyLine(0x4adb, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("PAPER_CHK_2")) {
			Common::Error lineError = playJimmyLine(0x4ad4, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return playJimmyLine(0x4b38, 0);
	}

	if (npc.npcName.equalsIgnoreCase("WASP_WOMAN")) {
		Common::String waspWomanTopicBuffer;
		int waspWomanTopicBufferLineIndex = -1;
		auto assignWaspWomanTopicBuffer = [&](int responseLineIndex) {
			assignTopicBuffer(waspWomanTopicBuffer, waspWomanTopicBufferLineIndex, responseLineIndex,
				"Wasp Woman topic buffer");
		};
		auto playWaspWomanLine = [&](int wavId, int headVariant) -> Common::Error {
			return playDialogueLineWithVariant(wavId, "WASP_WOMAN", headVariant);
		};

		if (!usedItemName.empty()) {
			if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
				(void)startupScript->setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
				return playDialogueEntrySequence(kWaspWomanWhaleyLines, ARRAYSIZE(kWaspWomanWhaleyLines));
			}
			if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
					usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
				(void)startupScript->setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
				return playWaspWomanLine(0x4cbd, 0);
			}
			if (usedItemName.equalsIgnoreCase("NOTE") ||
					usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
					usedItemName.equalsIgnoreCase("CHECKBOOK") ||
					usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
				(void)startupScript->setRuntimeFlagValue(kShownEvidenceOfBlackmailFlag, true);
				return playWaspWomanLine(0x4cc3, 0);
			}

			return playWaspWomanLine(0x4ca0, 0);
		}

		if (_waspWomanRoomDialogueState.introPending) {
			_waspWomanRoomDialogueState.introPending = false;

			Common::Error lineError = playWaspWomanLine(0x4bee, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (_sharedWaspWomanDialogueState) {
				lineError = playDialogueLine(0x4bf2, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			lineError = playDialogueEntrySequence(kWaspWomanIntroTailLines, ARRAYSIZE(kWaspWomanIntroTailLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		Common::Error lineError = playWaspWomanLine(0x4bee, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignWaspWomanTopicBuffer(kWaspWomanTopicBufferResponseLine);

		for (;;) {
			Common::String selectedTopic;
			Common::Error menuError = runKeywordMenu(
				waspWomanTopicBuffer, waspWomanTopicBufferLineIndex, selectedTopic);
			if (menuError.getCode() != Common::kNoError)
				return menuError;
			if (selectedTopic.empty() || selectedTopic.equalsIgnoreCase(genericByeTopic))
				return Common::kNoError;

			if (matchesResponseLine(selectedTopic, 0x302)) {
				lineError = playWaspWomanLine(0x4c0a, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x305)) {
				lineError = playWaspWomanLine(0x4c31, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				if (!_sharedWaspWomanDialogueState)
					continue;

				int responseIndex = 0;
				Common::Error responseError = runResponseMenu(0x307, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				switch (responseIndex) {
				case 1:
					lineError = playDialogueEntrySequence(kWaspWomanTopic305Response1Lines,
						ARRAYSIZE(kWaspWomanTopic305Response1Lines));
					break;
				case 2:
					lineError = playWaspWomanLine(0x4c5e, 2);
					break;
				default:
					lineError = Common::kNoError;
					break;
				}
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x308)) {
				lineError = playWaspWomanLine(0x4c67, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x30a)) {
				lineError = playWaspWomanLine(0x4c74, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x30c)) {
				lineError = playWaspWomanLine(0x4c85, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x30e)) {
				lineError = playWaspWomanLine(0x4c9a, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				return Common::kNoError;
			}
		}
	}

	if (npc.npcName.equalsIgnoreCase("MOM")) {
		const int currentStoryDayIndex = startupScript->getCurrentStoryDayIndex();
		Common::String momTopicBuffer;
		int momTopicBufferLineIndex = -1;
		auto assignMomTopicBuffer = [&](int responseLineIndex) {
			assignTopicBuffer(momTopicBuffer, momTopicBufferLineIndex, responseLineIndex, "Mom topic buffer");
		};
		auto runMomGoodbye = [&]() -> Common::Error {
			if (!startupScript->getFlagValue("DAY_FLAG")) {
				return playDialogueLine(0x256e, "MOM");
			}
			if (!startupScript->isNamedNpcDeathTypeClear("JIMMY"))
				return Common::kNoError;

			return playDialogueLine(0x2051, "MOM");
		};
		momTopicBuffer.clear();
		momTopicBufferLineIndex = -1;

		if (!usedItemName.empty()) {
			if (usedItemName.equalsIgnoreCase("NOTE") ||
					usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
					usedItemName.equalsIgnoreCase("CHECKBOOK") ||
					usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
				(void)startupScript->setRuntimeFlagValue(kShownEvidenceOfBlackmailFlag, true);
				return playDialogueLine(0x2317, "MOM");
			}
			if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
					startupScript->getFlagValue("HAVE_BOTH_LEDGERS")) {
				(void)startupScript->setRuntimeFlagValue(kShownLedgersToAnyoneFlag, true);
				return playDialogueLine(0x2320, "MOM");
			}
			if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
					usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
				(void)startupScript->setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
				return playDialogueLine(0x233b, "PC");
			}
			if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
				(void)startupScript->setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
				return playDialogueLine(0x239c, "MOM");
			}
			if (usedItemName.equalsIgnoreCase("TV_DEED") ||
					usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
				(void)startupScript->setRuntimeFlagValue(kShownEvidenceSheriffOwnsFlag, true);
				return playDialogueLine(0x2382, "MOM");
			}

			return playDialogueLine(0x26cc, "MOM");
		}

		bool skipMomDefaultKeywordSeed = false;
		if (_momRoomDialogueState.introPending) {
			_momRoomDialogueState.introPending = false;
			_momRoomDialogueState.sameDayIntroLineEnabled = true;
			_momRoomDialogueState.postIntroDefaultLineEnabled = true;
			_momRoomDialogueState.introDayIndex = currentStoryDayIndex;
			Common::Error lineError = playDialogueEntrySequence(kMomIntroLines, ARRAYSIZE(kMomIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomInitialTopicBufferResponseLine);
			skipMomDefaultKeywordSeed = true;
		}
		if (!skipMomDefaultKeywordSeed && _momRoomDialogueState.sameDayIntroLineEnabled &&
				currentStoryDayIndex == _momRoomDialogueState.introDayIndex) {
			Common::Error lineError = playDialogueLine(0x2047, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomSameDayTopicBufferResponseLine);
			skipMomDefaultKeywordSeed = true;
		}
		if (!skipMomDefaultKeywordSeed && _momRoomDialogueState.postIntroDefaultLineEnabled) {
			Common::Error lineError = playDialogueLine(0x2311, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomPostIntroTopicBufferResponseLine);
		}
		if (startupScript->getFlagValue("STEPH_MIDGAME_PLAYED") &&
				!_momRoomDialogueState.stephMidgameShown) {
			_momRoomDialogueState.stephMidgameShown = true;
			Common::Error lineError = playDialogueLine(0x205a, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("DINER_BURNED") &&
				(startupScript->getFlagValue("KARIN_KIDNAPED") ||
					startupScript->getFlagValue("KARIN_FOUND_DEAD")) &&
				!_momRoomDialogueState.dinerBurnedKarinMissingOrDeadShown) {
			_momRoomDialogueState.dinerBurnedKarinMissingOrDeadShown = true;
			Common::Error lineError = playDialogueLine(0x2456, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("BURNED_TV_STATION") &&
				!_momRoomDialogueState.burnedTvStationShown) {
			_momRoomDialogueState.burnedTvStationShown = true;
			Common::Error lineError = playDialogueLine(0x2220, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("SCRATCHED_TUCKER") &&
				!_momRoomDialogueState.scratchedTuckerShown) {
			_momRoomDialogueState.scratchedTuckerShown = true;
			Common::Error lineError = playDialogueLine(0x23f3, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("BARBER_POLE_STOLEN") &&
				!_momRoomDialogueState.barberPoleStolenShown) {
			_momRoomDialogueState.barberPoleStolenShown = true;
			Common::Error lineError = playDialogueLine(0x23fb, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
				!_momRoomDialogueState.boltOfClothTakenShown) {
			_momRoomDialogueState.boltOfClothTakenShown = true;
			if (startupScript->isNamedNpcDeathTypeClear("SPARKY") &&
					startupScript->isNamedNpcDeathTypeClear("FIREMAN2")) {
				Common::Error lineError = playDialogueLine(0x2416, "MOM");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			Common::Error lineError = playDialogueLine(0x2420, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("DINER_BURNED") &&
				startupScript->getFlagValue("KARIN_FOUND_ALIVE") &&
				!_momRoomDialogueState.dinerBurnedKarinAliveShown) {
			_momRoomDialogueState.dinerBurnedKarinAliveShown = true;
			Common::Error lineError = playDialogueLine(0x2434, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("PC_ESCAPED_JAIL") &&
				!_momRoomDialogueState.escapedJailShown) {
			_momRoomDialogueState.escapedJailShown = true;
			Common::Error lineError = playDialogueLine(0x2495, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("KARIN_KIDNAPED") &&
				!startupScript->getFlagValue("KARIN_FOUND_DEAD") &&
				!startupScript->getFlagValue("KARIN_FOUND_ALIVE") &&
				!_momRoomDialogueState.karinKidnapedUnresolvedShown) {
			_momRoomDialogueState.karinKidnapedUnresolvedShown = true;
			Common::Error lineError = playDialogueLine(0x24d7, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("KARIN_FOUND_ALIVE") &&
				!_momRoomDialogueState.karinFoundAliveShown) {
			_momRoomDialogueState.karinFoundAliveShown = true;
			Common::Error lineError = playDialogueLine(0x2505, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("KARIN_FOUND_DEAD") &&
				!_momRoomDialogueState.karinFoundDeadShown) {
			_momRoomDialogueState.karinFoundDeadShown = true;
			Common::Error lineError = playDialogueLine(0x2576, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!startupScript->isNamedNpcDeathTypeClear("BUTCHER") &&
				!_momRoomDialogueState.butcherAbsentShown) {
			_momRoomDialogueState.butcherAbsentShown = true;
			Common::Error lineError = playDialogueLine(0x2633, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!startupScript->isNamedNpcDeathTypeClear("MOYNAHAN") &&
				!_momRoomDialogueState.moynahanAbsentShown) {
			_momRoomDialogueState.moynahanAbsentShown = true;
			Common::Error lineError = playDialogueLine(0x2647, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!startupScript->isNamedNpcDeathTypeClear("JIMMY") &&
				!_momRoomDialogueState.jimmyAbsentShown) {
			_momRoomDialogueState.jimmyAbsentShown = true;
			if (startupScript->getFlagValue("JIMMY_ATTACKED")) {
				Common::Error lineError = playDialogueLine(0x2659, "MOM");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			Common::Error lineError = playDialogueLine(0x266c, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!startupScript->isNamedNpcDeathTypeClear("WASP_WOMAN") &&
				!_momRoomDialogueState.waspWomanAbsentShown) {
			_momRoomDialogueState.waspWomanAbsentShown = true;
			Common::Error lineError = playDialogueLine(0x2689, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("STEPHANIE_IS_DEAD") &&
				!startupScript->getFlagValue("STEPH_MIDGAME_PLAYED") &&
				!_momRoomDialogueState.stephanieDeadPreMidgameShown) {
			_momRoomDialogueState.stephanieDeadPreMidgameShown = true;
			Common::Error lineError = playDialogueLine(0x217c, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript->getFlagValue("DAY_5") && !_momRoomDialogueState.day5Shown) {
			_momRoomDialogueState.day5Shown = true;
			Common::Error lineError = playDialogueLine(0x218d, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomDay5TopicBufferResponseLine);
		}
		if (startupScript->getFlagValue("DAY_6") && !_momRoomDialogueState.day6Shown) {
			_momRoomDialogueState.day6Shown = true;
			Common::Error lineError = playDialogueLine(0x22a8, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomDay6TopicBufferResponseLine);
		}

		for (;;) {
			Common::String selectedTopic;
			Common::Error menuError = runKeywordMenu(momTopicBuffer, momTopicBufferLineIndex, selectedTopic);
			if (menuError.getCode() != Common::kNoError)
				return menuError;
			if (selectedTopic.empty())
				return Common::kNoError;
			if (selectedTopic.equalsIgnoreCase(genericByeTopic))
				return runMomGoodbye();

			if (matchesResponseLine(selectedTopic, 0x116)) {
				Common::Error lineError = playDialogueEntrySequence(kMomCookiesLines, ARRAYSIZE(kMomCookiesLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x118);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x119)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic119Lines, ARRAYSIZE(kMomTopic119Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x11a);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x11d)) {
				Common::Error lineError = playDialogueEntrySequence(
					kMomTopic11dOpeningLines, ARRAYSIZE(kMomTopic11dOpeningLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playDialogueFst(kDialogueC008AFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playDialogueEntrySequence(
					kMomTopic11dPostC008ALines, ARRAYSIZE(kMomTopic11dPostC008ALines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				StartupInteractionResult babyGurgleInteraction;
				if (startupScript->executeActionTag(kBabyGurgleActionTag, babyGurgleInteraction)) {
					if (!babyGurgleInteraction.musicPath.empty())
						(void)_engine.playStartupMusic(babyGurgleInteraction.musicPath);
					if (!babyGurgleInteraction.audioCommands.empty())
						startupFlow.executeStartupAudioCommands(babyGurgleInteraction.audioCommands);

					babyGurgleInteraction.musicPath.clear();
					babyGurgleInteraction.audioCommands.clear();
					queueDialogueInteractionIfNeeded(babyGurgleInteraction);
				}

				lineError = playDialogueEntrySequence(
					kMomTopic11dClosingLines, ARRAYSIZE(kMomTopic11dClosingLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playDialogueFst(kDialogueC008BFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x120);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x121)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic121Lines, ARRAYSIZE(kMomTopic121Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x122);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x123)) {
				Common::Error lineError = playDialogueEntrySequence(kMomSparkyLines, ARRAYSIZE(kMomSparkyLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x124);
				continue;
			}
			if (matchesAnyResponseLine(selectedTopic, kMomPtaTopicResponseLines,
					ARRAYSIZE(kMomPtaTopicResponseLines))) {
				Common::Error lineError = playDialogueEntrySequence(kMomPtaLines, ARRAYSIZE(kMomPtaLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomPtaFollowupTopicBufferResponseLine);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x12b)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic12bLines, ARRAYSIZE(kMomTopic12bLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x12c);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x12d)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic12dLines, ARRAYSIZE(kMomTopic12dLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x12e);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x12f)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic12fLines, ARRAYSIZE(kMomTopic12fLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x130);
				continue;
			}
			if (matchesAnyResponseLine(selectedTopic, kMomCookingTopicResponseLines,
					ARRAYSIZE(kMomCookingTopicResponseLines))) {
				Common::Error lineError = playDialogueEntrySequence(kMomCookingLines, ARRAYSIZE(kMomCookingLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomCookingFollowupTopicBufferResponseLine);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x135)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic135Lines, ARRAYSIZE(kMomTopic135Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x136);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x137)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic137Lines, ARRAYSIZE(kMomTopic137Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x13a)) {
				Common::Error lineError = playDialogueLine(0x1fa3, "MOM");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(0x13b);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x13c)) {
				Common::Error lineError = playDialogueEntrySequence(kMomSlaughterhouseLines, ARRAYSIZE(kMomSlaughterhouseLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomSlaughterhouseFollowupTopicBufferResponseLine);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x13f)) {
				_momRoomDialogueState.fatherTopicState = true;
				Common::Error lineError = playDialogueEntrySequence(kMomFatherLines, ARRAYSIZE(kMomFatherLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				if (startupScript->getFlagValue("HEARD_DAD_MOAN")) {
					lineError = playDialogueEntrySequence(kMomFatherHeardDadMoanLines,
						ARRAYSIZE(kMomFatherHeardDadMoanLines));
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
				assignMomTopicBuffer(kMomFatherFollowupTopicBufferResponseLine);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x141)) {
				Common::Error lineError = Common::kNoError;
				if (startupScript->getFlagValue("STEPH_MIDGAME_PLAYED")) {
					lineError = playDialogueEntrySequence(kMomStephMidgameLodgeLines,
						ARRAYSIZE(kMomStephMidgameLodgeLines));
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					continue;
				}
				_sharedDiscussedLodgeTopic = true;
				lineError = playDialogueEntrySequence(kMomLodgeLines, ARRAYSIZE(kMomLodgeLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int responseIndex = 0;
				Common::Error responseError = runResponseMenu(0x142, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				switch (responseIndex) {
				case 1:
					lineError = playDialogueEntrySequence(kMomLodgeResponse1Lines,
						ARRAYSIZE(kMomLodgeResponse1Lines));
					break;
				case 2:
					lineError = playDialogueEntrySequence(kMomLodgeResponse2Lines,
						ARRAYSIZE(kMomLodgeResponse2Lines));
					break;
				default:
					break;
				}
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomLodgeFollowupTopicBufferResponseLine);
				continue;
			}
			if (matchesAnyResponseLine(selectedTopic, kMomBakeSaleTopicResponseLines,
					ARRAYSIZE(kMomBakeSaleTopicResponseLines))) {
				Common::Error lineError = playDialogueEntrySequence(kMomBakeSaleLines, ARRAYSIZE(kMomBakeSaleLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomBakeSaleFollowupTopicBufferResponseLine);
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x14a)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic14aLines, ARRAYSIZE(kMomTopic14aLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesAnyResponseLine(selectedTopic, kMomPottsdamTopicResponseLines,
					ARRAYSIZE(kMomPottsdamTopicResponseLines))) {
				Common::Error lineError = Common::kNoError;
				if (startupScript->getFlagValue("STEPH_MIDGAME_PLAYED"))
					lineError = playDialogueLine(0x2086, "MOM");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomPottsdamFollowupTopicBufferResponseLine);
				continue;
			}
			if (matchesAnyResponseLine(selectedTopic, kMomMeatPlantTopicResponseLines,
					ARRAYSIZE(kMomMeatPlantTopicResponseLines))) {
				Common::Error lineError = playDialogueEntrySequence(kMomMeatPlantIntroLines,
					ARRAYSIZE(kMomMeatPlantIntroLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int responseIndex = 0;
				Common::Error responseError = runResponseMenu(0x153, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				switch (responseIndex) {
				case 1:
					lineError = playDialogueEntrySequence(kMomMeatPlantResponse1Lines,
						ARRAYSIZE(kMomMeatPlantResponse1Lines));
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					continue;
				case 2:
					lineError = playDialogueEntrySequence(kMomMeatPlantResponse2Lines,
						ARRAYSIZE(kMomMeatPlantResponse2Lines));
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					assignMomTopicBuffer(kMomMeatPlantFollowupTopicBufferResponseLine);
					continue;
				default:
					continue;
				}
			}
			if (matchesResponseLine(selectedTopic, 0x155)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic155Lines, ARRAYSIZE(kMomTopic155Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x156)) {
				Common::Error lineError = playDialogueEntrySequence(kMomTopic156Lines, ARRAYSIZE(kMomTopic156Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x15b) && currentStoryDayIndex == 5) {
				Common::Error lineError = Common::kNoError;
				if (_momRoomDialogueState.goodCauseDay5State)
					lineError = playDialogueEntrySequence(kMomGoodCauseDay5Lines, ARRAYSIZE(kMomGoodCauseDay5Lines));
				else
					lineError = playDialogueLine(0x21dd, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomGoodCauseDay5FollowupTopicBufferResponseLine);
				continue;
			}
			if (matchesAnyResponseLine(selectedTopic, kMomMoynahanTopicResponseLines,
					ARRAYSIZE(kMomMoynahanTopicResponseLines))) {
				Common::Error lineError = playDialogueLine(0x220a, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesAnyResponseLine(selectedTopic, kMomNewspaperFireTopicResponseLines,
					ARRAYSIZE(kMomNewspaperFireTopicResponseLines))) {
				Common::Error lineError = playDialogueLine(0x229e, "MOM");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x16d)) {
				Common::Error lineError = playDialogueLine(0x2696, "MOM");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x16e)) {
				Common::Error lineError = playDialogueLine(0x26a3, "MOM");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (matchesResponseLine(selectedTopic, 0x170)) {
				Common::Error lineError = playDialogueLine(0x26bc, "MOM");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}

			Common::Error lineError = playDialogueLine(0x26c6, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return runMomGoodbye();
		}
	}

	if (!npc.npcName.equalsIgnoreCase("HANK")) {
		debug(1, "Harvester: unsupported room NPC dialogue handler '%s'", npc.npcName.c_str());
		return Common::kNoError;
	}

	const int currentStoryDayIndex = startupScript->getCurrentStoryDayIndex();
	Common::String &hankTopicBuffer = _hankRoomDialogueState.currentTopicBuffer;
	int &hankTopicBufferLineIndex = _hankRoomDialogueState.currentTopicBufferLineIndex;
	auto assignHankTopicBuffer = [&](int responseLineIndex) {
		assignTopicBuffer(hankTopicBuffer, hankTopicBufferLineIndex, responseLineIndex, "Hank topic buffer");
	};

	debugC(1, kDebugDialogue,
		"Harvester: Hank dialogue start day=%d item='%s' initial=%d trackedDayValid=%d trackedDay=%d sameDayPending=%d rangshotPending=%d topicLine=%d topicBuffer='%s'",
		currentStoryDayIndex, usedItemName.empty() ? "<none>" : usedItemName.c_str(),
		(int)_hankRoomDialogueState.pendingInitialConversation,
		(int)_hankRoomDialogueState.hasTrackedDayState, _hankRoomDialogueState.trackedDayIndex,
		(int)_hankRoomDialogueState.pendingSameDayFollowup,
		(int)_hankRoomDialogueState.pendingRangshotSequence,
		hankTopicBufferLineIndex, hankTopicBuffer.c_str());

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)startupScript->setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
			return playDialogueLine(0xa3e, "HANK");
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)startupScript->setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
			return playDialogueLine(0xa53, "HANK");
		}
		return playDialogueLine(0xa38, "HANK");
	}
	bool skipHankFollowupBranches = false;
	if (_hankRoomDialogueState.pendingInitialConversation) {
		_hankRoomDialogueState.trackedDayIndex = currentStoryDayIndex;
		_hankRoomDialogueState.hasTrackedDayState = true;
		_hankRoomDialogueState.pendingSameDayFollowup = true;
		_hankRoomDialogueState.pendingInitialConversation = false;
		hankTopicBuffer.clear();
		hankTopicBufferLineIndex = -1;
		skipHankFollowupBranches = true;

		Common::Error lineError = playDialogueLine(0x703, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runResponseMenu(0xc7, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = playDialogueLine(0x70f, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xc8);
			break;
		case 2:
			lineError = playDialogueLine(0x715, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xc9);
			break;
		case 3:
			lineError = playDialogueLine(0x71c, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xca);
			break;
		default:
			break;
		}
	}

	if (!skipHankFollowupBranches && (!_hankRoomDialogueState.hasTrackedDayState ||
			currentStoryDayIndex != _hankRoomDialogueState.trackedDayIndex)) {
		if (_hankRoomDialogueState.pendingSameDayFollowup) {
			_hankRoomDialogueState.trackedDayIndex = currentStoryDayIndex;
			_hankRoomDialogueState.pendingSameDayFollowup = false;
			_hankRoomDialogueState.pendingRangshotSequence = true;

			Common::Error lineError = playDialogueLine(0x8f5, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (_hankRoomDialogueState.pendingRangshotSequence) {
			if (currentStoryDayIndex > 5) {
				Common::Error lineError = playDialogueLine(0x8e2, "HANK");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else {
				FstPlayer fstPlayer(_engine);
				if (!fstPlayer.play(kDialogueRangshotFstPath))
					return Common::kReadingFailed;
			}
		}
	} else if (!skipHankFollowupBranches) {
		_hankRoomDialogueState.pendingSameDayFollowup = true;
		if (currentStoryDayIndex > 5) {
			Common::Error lineError = playDialogueLine(0x8e2, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			FstPlayer fstPlayer(_engine);
			if (!fstPlayer.play(kDialogueRangshotFstPath))
				return Common::kReadingFailed;
		}
	}

	if (!skipHankFollowupBranches && startupScript->getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!_hankRoomDialogueState.stephMidgamePlayedShown) {
		_hankRoomDialogueState.stephMidgamePlayedShown = true;

		Common::Error lineError = playDialogueLine(0x92c, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runResponseMenu(0xcc, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineSpec lines[] = {
				DialogueLineSpec(0x939, "HANK"),
				DialogueLineSpec(0x93f, "MOM", 2)
			};
			lineError = playDialogueSequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2:
			lineError = playDialogueLine(0x944, "HANK");
			break;
		case 3:
			lineError = playDialogueLine(0x948, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = playDialogueLine(0x94f, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runResponseMenu(0xcd, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineSpec lines[] = {
				DialogueLineSpec(0x95a, "HANK"),
				DialogueLineSpec(0x961, "PC", 2)
			};
			lineError = playDialogueSequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2: {
			const DialogueLineSpec lines[] = {
				DialogueLineSpec(0x965, "HANK"),
				DialogueLineSpec(0x96b, "PC")
			};
			lineError = playDialogueSequence(lines, ARRAYSIZE(lines));
			break;
		}
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = playDialogueLine(0x971, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && startupScript->getFlagValue("BURNED_TV_STATION") &&
			!_hankRoomDialogueState.burnedTvStationShown) {
		_hankRoomDialogueState.burnedTvStationShown = true;
		Common::Error lineError = playDialogueLine(0x987, "PC");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && startupScript->getFlagValue("BUSTED_ONCE") &&
			!_hankRoomDialogueState.bustedOnceShown) {
		_hankRoomDialogueState.bustedOnceShown = true;
		Common::Error lineError = playDialogueLine(0x9b1, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runResponseMenu(0xce, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineSpec lines[] = {
				DialogueLineSpec(0x9bc, "HANK"),
				DialogueLineSpec(0x9c9, "HANK")
			};
			lineError = playDialogueSequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2: {
			const DialogueLineSpec lines[] = {
				DialogueLineSpec(0x9c1, "HANK"),
				DialogueLineSpec(0x9c5, "PC"),
				DialogueLineSpec(0x9c9, "HANK")
			};
			lineError = playDialogueSequence(lines, ARRAYSIZE(lines));
			break;
		}
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && startupScript->getFlagValue("KARIN_KIDNAPED") &&
			!_hankRoomDialogueState.karinKidnapedShown) {
		_hankRoomDialogueState.karinKidnapedShown = true;
		Common::Error lineError = playDialogueLine(0x9d5, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (_sharedKarinKidnapedDialogueState) {
			lineError = playDialogueLineWithVariant(0x9da, "PC", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		const DialogueLineSpec lines[] = {
			DialogueLineSpec(0x9de, "HANK"),
			DialogueLineSpec(0x9df, "HANK"),
			DialogueLineSpec(0x9e0, "HANK"),
			DialogueLineSpec(0x9e1, "HANK"),
			DialogueLineSpec(0x9e2, "HANK")
		};
		lineError = playDialogueSequence(lines, ARRAYSIZE(lines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runResponseMenu(0xcf, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineSpec responseLines[] = {
				DialogueLineSpec(0x9f4, "HANK"),
				DialogueLineSpec(0x9fc, "HANK")
			};
			lineError = playDialogueSequence(responseLines, ARRAYSIZE(responseLines));
			break;
		}
		case 2:
			lineError = playDialogueLine(0xa01, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && startupScript->getFlagValue("KARIN_FOUND_ALIVE") &&
			!_hankRoomDialogueState.karinFoundAliveShown) {
		_hankRoomDialogueState.karinFoundAliveShown = true;
		Common::Error lineError = playDialogueLine(0xa0b, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && startupScript->getFlagValue("KARIN_FOUND_DEAD") &&
			!_hankRoomDialogueState.karinFoundDeadShown) {
		_hankRoomDialogueState.karinFoundDeadShown = true;
		Common::Error lineError = playDialogueLine(0xa15, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runResponseMenu(0xd0, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = playDialogueLine(0xa20, "HANK");
			break;
		case 2:
			lineError = playDialogueLine(0xa26, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = playDialogueLine(0xa2b, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runKeywordMenu(hankTopicBuffer, hankTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;

		if (selectedTopic.empty())
			return Common::kNoError;
		debugC(1, kDebugDialogue, "Harvester: Hank selected topic='%s'", selectedTopic.c_str());
		if (selectedTopic.equalsIgnoreCase(genericByeTopic))
			return playDialogueLine(0x8dc, "HANK");

		const Common::String momTopic = startupText->getDialogueResponseLine(0xd2);
		Common::Array<Common::String> currentTopics;
		splitDialogueMenuLine(hankTopicBuffer, currentTopics);
		const bool matchesMomTopicByLine = !momTopic.empty() && selectedTopic.equalsIgnoreCase(momTopic);
		const bool matchesMomTopicFromIntroBuffer = hankTopicBufferLineIndex == 0xc8 &&
			!currentTopics.empty() && selectedTopic.equalsIgnoreCase(currentTopics[0]);
		if (matchesMomTopicByLine || matchesMomTopicFromIntroBuffer) {
			debugC(1, kDebugDialogue,
				"Harvester: Hank matched special Mom branch '%s' (lineMatch=%d bufferMatch=%d)",
				selectedTopic.c_str(), (int)matchesMomTopicByLine, (int)matchesMomTopicFromIntroBuffer);
			const DialogueLineSpec lines[] = {
				DialogueLineSpec(0x725, "HANK"),
				DialogueLineSpec(0x729, "PC"),
				DialogueLineSpec(0x72d, "HANK"),
				DialogueLineSpec(0x733, "PC"),
				DialogueLineSpec(0x737, "HANK"),
				DialogueLineSpec(0x73b, "PC"),
				DialogueLineSpec(0x741, "HANK"),
				DialogueLineSpec(0x747, "MOM", 2)
			};
			Common::Error lineError = playDialogueSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			setActiveSpeakerPortrait("HANK", 0);
			assignHankTopicBuffer(0xd5);
			continue;
		}

		bool handledTopic = false;
		for (const HankDialogueTopicLine &topic : kHankDialogueTopicLines) {
			if (topic.responseLineIndex == 0xd2)
				continue;

			const Common::String topicText = startupText->getDialogueResponseLine(topic.responseLineIndex);
			if (topicText.empty() || !selectedTopic.equalsIgnoreCase(topicText))
				continue;

			if (topic.setDiscussedLodgeTopic)
				_sharedDiscussedLodgeTopic = true;

			debugC(1, kDebugDialogue,
				"Harvester: Hank matched topic '%s' to response line 0x%x (%d) -> wav=0x%x speaker='%s'",
				selectedTopic.c_str(), topic.responseLineIndex, topic.responseLineIndex + 1,
				topic.wavId, topic.speakerId);
			Common::Error lineError = playDialogueLine(topic.wavId, topic.speakerId);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			handledTopic = true;
			break;
		}

		if (handledTopic) {
			debugC(1, kDebugDialogue,
				"Harvester: Hank handled topic '%s' and now falls through to generic response/exit",
				selectedTopic.c_str());
			Common::Error lineError = playDialogueLine(0xa32, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return playDialogueLine(0x8dc, "HANK");
		}

		debugC(1, kDebugDialogue,
			"Harvester: Hank topic '%s' fell back to generic response and exits dialogue",
			selectedTopic.c_str());
		Common::Error lineError = playDialogueLine(0xa32, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return playDialogueLine(0x8dc, "HANK");
	}
}

} // End of namespace Harvester
