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
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/npc/dwayne_dialogue.h"
#include "harvester/npc/hank_dialogue.h"
#include "harvester/npc/jimmy_dialogue.h"
#include "harvester/npc/mom_dialogue.h"
#include "harvester/npc/pta_mom_dialogue.h"
#include "harvester/npc/stub_dialogue.h"
#include "harvester/npc/wasp_woman_dialogue.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/startup_art.h"
#include "harvester/startup_flow.h"
#include "harvester/startup_text.h"

namespace Harvester {

struct DialogueLineEntry {
	int wavId;
	const char *speakerId;
	int headVariant;
};

class DialogueRuntime {
public:
	typedef std::function<Common::Error(int, const Common::String &, int)> PlayDialogueLineWithVariantFn;
	typedef std::function<Common::Error(int, const Common::String &)> PlayDialogueLineFn;
	typedef std::function<Common::Error(const DialogueLineEntry *, uint)> PlayDialogueEntrySequenceFn;
	typedef std::function<Common::Error(const Common::String &)> PlayDialogueFstFn;
	typedef std::function<Common::Error(const Common::String &, int, Common::String &)> RunKeywordMenuFn;
	typedef std::function<Common::Error(int, int &)> RunResponseMenuFn;
	typedef std::function<void(Common::String &, int &, int, const char *)> AssignTopicBufferFn;
	typedef std::function<bool(const Common::String &, int)> MatchesResponseLineFn;
	typedef std::function<bool(const Common::String &, const int *, uint)> MatchesAnyResponseLineFn;
	typedef std::function<void(const StartupInteractionResult &)> QueueDialogueInteractionIfNeededFn;
	typedef std::function<void(StartupInteractionResult &)> ApplyImmediateDialogueInteractionEffectsFn;
	typedef std::function<int(int)> GetRandomNumberFn;
	typedef std::function<void(const Common::String &, int)> SetActiveSpeakerPortraitFn;

	DialogueRuntime(HarvesterEngine &engine, StartupScript &startupScript, StartupText &startupText,
			StartupFlow &startupFlow, const Common::String &currentRoomName,
			const Common::String &genericByeTopic,
			const PlayDialogueLineWithVariantFn &playDialogueLineWithVariant,
			const PlayDialogueLineFn &playDialogueLine,
			const PlayDialogueEntrySequenceFn &playDialogueEntrySequence,
			const PlayDialogueFstFn &playDialogueFst,
			const RunKeywordMenuFn &runKeywordMenu,
			const RunResponseMenuFn &runResponseMenu,
			const AssignTopicBufferFn &assignTopicBuffer,
			const MatchesResponseLineFn &matchesResponseLine,
			const MatchesAnyResponseLineFn &matchesAnyResponseLine,
			const QueueDialogueInteractionIfNeededFn &queueDialogueInteractionIfNeeded,
			const ApplyImmediateDialogueInteractionEffectsFn &applyImmediateDialogueInteractionEffects,
			const GetRandomNumberFn &getRandomNumber,
			const SetActiveSpeakerPortraitFn &setActiveSpeakerPortrait)
		: _engine(engine), _startupScript(startupScript), _startupText(startupText),
		  _startupFlow(startupFlow), _currentRoomName(currentRoomName),
		  _genericByeTopic(genericByeTopic),
		  _playDialogueLineWithVariant(playDialogueLineWithVariant),
		  _playDialogueLine(playDialogueLine), _playDialogueEntrySequence(playDialogueEntrySequence),
		  _playDialogueFst(playDialogueFst), _runKeywordMenu(runKeywordMenu),
		  _runResponseMenu(runResponseMenu), _assignTopicBuffer(assignTopicBuffer),
		  _matchesResponseLine(matchesResponseLine),
		  _matchesAnyResponseLine(matchesAnyResponseLine),
		  _queueDialogueInteractionIfNeeded(queueDialogueInteractionIfNeeded),
		  _applyImmediateDialogueInteractionEffects(applyImmediateDialogueInteractionEffects),
		  _getRandomNumber(getRandomNumber),
		  _setActiveSpeakerPortrait(setActiveSpeakerPortrait) {
	}

	HarvesterEngine &engine() const { return _engine; }
	StartupScript &startupScript() const { return _startupScript; }
	StartupText &startupText() const { return _startupText; }
	StartupFlow &startupFlow() const { return _startupFlow; }
	const Common::String &currentRoomName() const { return _currentRoomName; }
	const Common::String &genericByeTopic() const { return _genericByeTopic; }

	Common::Error playDialogueLineWithVariant(int wavId, const Common::String &speakerId,
			int headVariant) const {
		return _playDialogueLineWithVariant(wavId, speakerId, headVariant);
	}
	Common::Error playDialogueLine(int wavId, const Common::String &speakerId) const {
		return _playDialogueLine(wavId, speakerId);
	}
	Common::Error playDialogueEntrySequence(const DialogueLineEntry *lines, uint count) const {
		return _playDialogueEntrySequence(lines, count);
	}
	Common::Error playDialogueFst(const Common::String &path) const { return _playDialogueFst(path); }
	Common::Error runKeywordMenu(const Common::String &topicBuffer, int topicBufferLineIndex,
			Common::String &selectedTopic) const {
		return _runKeywordMenu(topicBuffer, topicBufferLineIndex, selectedTopic);
	}
	Common::Error runResponseMenu(int responseLineIndex, int &responseIndex) const {
		return _runResponseMenu(responseLineIndex, responseIndex);
	}
	void assignTopicBuffer(Common::String &topicBuffer, int &topicBufferLineIndex,
			int responseLineIndex, const char *label) const {
		_assignTopicBuffer(topicBuffer, topicBufferLineIndex, responseLineIndex, label);
	}
	bool matchesResponseLine(const Common::String &selectedTopic, int responseLineIndex) const {
		return _matchesResponseLine(selectedTopic, responseLineIndex);
	}
	bool matchesAnyResponseLine(const Common::String &selectedTopic,
			const int *responseLineIndices, uint responseLineCount) const {
		return _matchesAnyResponseLine(selectedTopic, responseLineIndices, responseLineCount);
	}
	void queueDialogueInteractionIfNeeded(const StartupInteractionResult &interaction) const {
		_queueDialogueInteractionIfNeeded(interaction);
	}
	void applyImmediateDialogueInteractionEffects(StartupInteractionResult &interaction) const {
		_applyImmediateDialogueInteractionEffects(interaction);
	}
	int getRandomNumber(int maxValue) const { return _getRandomNumber(maxValue); }
	void setActiveSpeakerPortrait(const Common::String &speakerId, int headVariant) const {
		_setActiveSpeakerPortrait(speakerId, headVariant);
	}

private:
	HarvesterEngine &_engine;
	StartupScript &_startupScript;
	StartupText &_startupText;
	StartupFlow &_startupFlow;
	const Common::String &_currentRoomName;
	const Common::String &_genericByeTopic;
	PlayDialogueLineWithVariantFn _playDialogueLineWithVariant;
	PlayDialogueLineFn _playDialogueLine;
	PlayDialogueEntrySequenceFn _playDialogueEntrySequence;
	PlayDialogueFstFn _playDialogueFst;
	RunKeywordMenuFn _runKeywordMenu;
	RunResponseMenuFn _runResponseMenu;
	AssignTopicBufferFn _assignTopicBuffer;
	MatchesResponseLineFn _matchesResponseLine;
	MatchesAnyResponseLineFn _matchesAnyResponseLine;
	QueueDialogueInteractionIfNeededFn _queueDialogueInteractionIfNeeded;
	ApplyImmediateDialogueInteractionEffectsFn _applyImmediateDialogueInteractionEffects;
	GetRandomNumberFn _getRandomNumber;
	SetActiveSpeakerPortraitFn _setActiveSpeakerPortrait;
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
static const char *const kPtaRespondToTvFlag = "PTA_RESPOND_TO_TV";
static const char *const kPtaMomSpeakerId = "PTA_MOM1";

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

static const DialogueLineEntry kPtaMomResponse1Lines[] = {
	{ 0x3204, kPtaMomSpeakerId, 2 },
	{ 0x320b, kPtaMomSpeakerId, 2 }
};

static const DialogueLineEntry kPtaMomResponse2Lines[] = {
	{ 0x320f, kPtaMomSpeakerId, 0 },
	{ 0x3213, kPtaMomSpeakerId, 0 }
};

static const DialogueLineEntry kPtaMomResponse3Lines[] = {
	{ 0x3217, kPtaMomSpeakerId, 0 },
	{ 0x321c, kPtaMomSpeakerId, 0 },
	{ 0x3221, "PC", 0 },
	{ 0x3226, kPtaMomSpeakerId, 0 }
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

DialogueSystem::DialogueSystem(HarvesterEngine &engine, Common::Point &mousePos)
	: _engine(engine), _mousePos(mousePos) {
	registerNpcHandlers();
}

DialogueSystem::~DialogueSystem() {
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		delete _npcHandlers[i];
}

void DialogueSystem::registerNpcHandlers() {
	_npcHandlers.push_back(new DwayneDialogueHandler());
	_npcHandlers.push_back(new JimmyDialogueHandler());
	_npcHandlers.push_back(new WaspWomanDialogueHandler());
	_npcHandlers.push_back(new MomDialogueHandler());
	_npcHandlers.push_back(new HankDialogueHandler());
	_npcHandlers.push_back(new PtaMomDialogueHandler());

	_npcHandlers.push_back(new StubNpcDialogueHandler("PTA_MOM"));
	_npcHandlers.push_back(new StubNpcDialogueHandler("EDNA"));
	_npcHandlers.push_back(new StubNpcDialogueHandler("HERRILL"));
	_npcHandlers.push_back(new StubNpcDialogueHandler("JOHNSON"));
}

void DialogueSystem::resetRoomNpcDialogueState() {
	_sharedDialogueState = DialogueSharedState();
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		_npcHandlers[i]->resetState();
}

Common::Error DialogueSystem::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
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
	auto applyImmediateDialogueInteractionEffects = [&](StartupInteractionResult &interaction) {
		if (!interaction.musicPath.empty())
			(void)_engine.playStartupMusic(interaction.musicPath);
		if (!interaction.audioCommands.empty())
			startupFlow.executeStartupAudioCommands(interaction.audioCommands);
		interaction.musicPath.clear();
		interaction.audioCommands.clear();
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

	DialogueRuntime runtime(
		_engine, *startupScript, *startupText, startupFlow, npc.roomName, genericByeTopic,
		playDialogueLineWithVariant, playDialogueLine, playDialogueEntrySequence,
		playDialogueFst, runKeywordMenu, runResponseMenu, assignTopicBuffer,
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

Common::Error DwayneDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	static const int kDwayneWhaleyTopicResponseLines[] = { 0x85, 0x86, 0x87 };
	static const int kDwayneLoomisTopicResponseLines[] = { 0x8c, 0x8d };

	DwayneRoomDialogueState &state = _state;
	Common::String &dwayneTopicBuffer = state.currentTopicBuffer;
	int &dwayneTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	auto assignDwayneTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(dwayneTopicBuffer, dwayneTopicBufferLineIndex,
			responseLineIndex, "Dwayne topic buffer");
	};
	auto playDwayneLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "DWAYNE", headVariant);
	};
	auto playEdnaLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "EDNA", headVariant);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<StartupObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const StartupObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}

		return false;
	};
	auto setDwayneIntroduced = [&]() {
		(void)runtime.startupScript().setRuntimeFlagValue("DWAYNE_INTRODUCED", true);
	};
	const bool sheriffInDiner = runtime.startupScript().getFlagValue("SHERIFF_IN_DINER");

	if (runtime.startupScript().getFlagValue("ARREST_FLAG")) {
		(void)runtime.startupScript().setRuntimeFlagValue("ARREST_FLAG", false);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_MOYNAHAN"))
			return playDwayneLine(0x38d6, 2);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_JIMMY"))
			return playDwayneLine(0x384d, 2);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_JOHNSON"))
			return playDwayneLine(0x386d, 2);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_PHELPS"))
			return playDwayneLine(0x38bc, 2);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_OREILLY"))
			return playDwayneLine(0x38b5, 2);
		if (runtime.startupScript().getFlagValue("IF_KILL_POTTSDAM_AT_GRAVE"))
			return playDwayneLine(0x37ed, 2);
		if (runtime.startupScript().getFlagValue("GENERIC_BUST"))
			return playDwayneLine(0x38fc, 2);
		if (runtime.startupScript().getFlagValue("DNALFT_PERVERT"))
			return playDwayneLine(0x3580, 2);
		if (runtime.startupScript().getFlagValue("PC_HAS_GOOJF_CARD") &&
				!runtime.startupScript().getFlagValue("PC_FRY_IN_CHAIR")) {
			return playDwayneLine(0x3814);
		}
		if (!runtime.startupScript().getFlagValue("PC_FRY_IN_CHAIR")) {
			if (runtime.startupScript().getFlagValue("BUSTED_THIRD"))
				return playDwayneLine(0x3928, 2);
			if (runtime.startupScript().getFlagValue("BUSTED_TWICE"))
				return playDwayneLine(0x3921, 2);
			if (runtime.startupScript().getFlagValue("BUSTED_ONCE"))
				return playDwayneLine(0x3918);
		}
		return Common::kNoError;
	}

	if (runtime.startupScript().getFlagValue("CHECK_EVIDENCE_DOOR"))
		return playDwayneLine(0x3386);
	if (runtime.startupScript().getFlagValue("PC_TRIES_TO_TAKE_STEPHANIES_REMAINS"))
		return playDwayneLine(0x34e3, 2);
	if (runtime.currentRoomName().equalsIgnoreCase("ST_BEDRM")) {
		if (!runtime.startupScript().getFlagValue("SD_TALKED_OF_CARD")) {
			Common::Error lineError = playDwayneLine(0x34da);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		return playDwayneLine(0x3395);
	}

	if (runtime.startupScript().getFlagValue("BRING_KARIN_TO_SHERIFF") &&
			!sheriffInDiner &&
			!state.bringKarinToSheriffLinePlayed) {
		state.pendingKarinAliveFollowup = true;
		state.pendingKarinAliveFollowupDayIndex =
			runtime.startupScript().getCurrentStoryDayIndex();
		state.bringKarinToSheriffLinePlayed = true;
		return playDwayneLine(0x3750);
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE")) {
		state.pendingKarinAliveFollowup = true;
		state.pendingKarinAliveFollowupDayIndex =
			runtime.startupScript().getCurrentStoryDayIndex();
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("BOYLES_BUTTON")) {
			state.discussedBoylesButton = true;
			return playDwayneLine(0x339b);
		}
		if (usedItemName.equalsIgnoreCase("K_PURSE")) {
			state.discussedKarinPurse = true;
			return playDwayneLine(0x37cb);
		}
		if (usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY")) {
			return playDwayneLine(0x33b9, 2);
		}
		if (usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("NOTE")) {
			if (state.presentedEvidenceReplyOverride)
				return playDwayneLine(0x342c);

			Common::Error lineError = playDwayneLine(0x33f4, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x70, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x33c3, 2);
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x3403, 2);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = playDwayneLine(0x3407, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playDwayneLine(0x340d);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x71, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 0)
				return Common::kNoError;
			if (responseIndex == 1)
				return playDwayneLine(0x341a, 2);
			if (responseIndex == 2)
				return playDwayneLine(0x3422, 2);
			return Common::kNoError;
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownLedgersToAnyoneFlag, true);
			return playDwayneLine(0x3452);
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
			return playDwayneLine(0x3975, 3);
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
			return playDwayneLine(0x3942, 2);
		}
		if (state.tvDeedReplyOverride &&
				usedItemName.equalsIgnoreCase("TV_DEED")) {
			return playDwayneLine(0x3686);
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			return playDwayneLine(0x3935);
		}
		return playDwayneLine(0x3608);
	}

	if (state.sheriffInDinerIntroPending && sheriffInDiner) {
		state.sheriffInDinerIntroPending = false;
		state.sheriffInDinerIntroPlayed = true;
		setDwayneIntroduced();

		const DialogueLineEntry introLines[] = {
			{ 0x3482, "DWAYNE", 0 },
			{ 0x3486, "PC", 0 },
			{ 0x348b, "DWAYNE", 1 },
			{ 0x3490, "PC", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED")) {
			sharedState.karinKidnapedDialogueState = true;
			lineError = playEdnaLine(0x3496, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = playDwayneLine(0x349b, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return playDwayneLine(0x34ac);
	}

	if (state.sheriffInDinerIntroPlayed && sheriffInDiner)
		return playDwayneLine(0x34ac);

	if (state.pendingInitialConversation && !sheriffInDiner) {
		state.pendingInitialConversation = false;
		state.eventFollowupGate = true;
		setDwayneIntroduced();

		const DialogueLineEntry initialLines[] = {
			{ 0x3245, "DWAYNE", 1 },
			{ 0x324a, "LOOMIS", 1 },
			{ 0x324e, "PC", 0 },
			{ 0x3252, "DWAYNE", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(initialLines, ARRAYSIZE(initialLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignDwayneTopicBuffer(0x78);
		if (sharedState.boyleGascanApplicationState)
			(void)runtime.startupScript().setRuntimeFlagValue("MOVE_SHERIFF", true);
	}

	if (state.eventFollowupGate && !sharedState.boyleGascanApplicationState) {
		Common::Error lineError = playDwayneLine(0x34d3, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (state.whaleyDisciplineFollowupState &&
			!state.whaleyDisciplineFollowupShown) {
		state.whaleyDisciplineFollowupShown = true;
		Common::Error lineError = playDwayneLine(0x34b3, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x79, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		lineError = Common::kNoError;
		if (responseIndex == 1) {
			lineError = playDwayneLine(0x34bf, 2);
		} else if (responseIndex == 2) {
			lineError = playDwayneLine(0x34c5, 1);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playDwayneLine(0x34cb, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue(kShownEvidenceOfBlackmailFlag) &&
			!state.noteCheckbookFollowupShown) {
		state.noteCheckbookFollowupShown = true;
		Common::Error lineError = playDwayneLine(0x355f, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!sheriffInDiner) {
		if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
				!state.scratchedTuckerShown) {
			state.scratchedTuckerShown = true;
			Common::Error lineError = playDwayneLine(0x358b, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
				!state.boltOfClothTakenShown) {
			state.boltOfClothTakenShown = true;
			Common::Error lineError = playDwayneLine(0x3594, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
				!state.barberPoleStolenShown) {
			state.barberPoleStolenShown = true;
			Common::Error lineError = playDwayneLine(0x359d, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
				!state.dinerBurnedShown) {
			state.dinerBurnedShown = true;
			Common::Error lineError = playDwayneLine(0x35bc, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") &&
				!state.escapedJailShown) {
			state.escapedJailShown = true;
			Common::Error lineError = playDwayneLine(0x35cd);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE") &&
				!state.gotRemainsForLodgeShown) {
			state.gotRemainsForLodgeShown = true;
			Common::Error lineError = playDwayneLine(0x35d7, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
				!state.burnedTvStationShown) {
			state.burnedTvStationShown = true;
			Common::Error lineError = playDwayneLine(0x35f6, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
				!state.karinKidnapedShown) {
			state.karinKidnapedShown = true;
			Common::Error lineError = playDwayneLine(0x36b2, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	const int currentStoryDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	if (state.pendingKarinAliveFollowup &&
			currentStoryDayIndex != state.pendingKarinAliveFollowupDayIndex &&
			!state.pendingKarinAliveFollowupLinePlayed) {
		state.pendingKarinAliveFollowupLinePlayed = true;
		state.pendingKarinAliveFollowup = false;
		state.completedKarinAliveFollowup = true;

		const DialogueLineEntry lines[] = {
			{ 0x3780, "DWAYNE", 0 },
			{ 0x3786, "PC", 4 },
			{ 0x378a, "DWAYNE", 2 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.discussedKarinPurse &&
			!sheriffInDiner &&
			!state.karinFoundDeadWithoutPurseShown) {
		state.karinFoundDeadWithoutPurseShown = true;
		Common::Error lineError = playDwayneLine(0x3793);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY") &&
			!sheriffInDiner &&
			!state.jimmyAbsentShown) {
		state.jimmyAbsentShown = true;
		Common::Error lineError = playDwayneLine(0x3855, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue("JIMMY_ATTACKED")) {
			lineError = playDwayneLine(0x37f8);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x81, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x3805, 1);
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x380c);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
			!sheriffInDiner &&
			!state.moynahanAbsentShown) {
		state.moynahanAbsentShown = true;
		Common::Error lineError = playDwayneLine(0x38ce);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("EDNA_HUNG") &&
			!state.ednaHungShown) {
		state.ednaHungShown = true;
		Common::Error lineError = playDwayneLine(0x38f1, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("MCKNIGHT") &&
			!sheriffInDiner &&
			!state.mcknightAbsentShown) {
		state.mcknightAbsentShown = true;
		Common::Error lineError = hasInventoryItem("TV_DEED")
			? playDwayneLine(0x3904, 3)
			: playDwayneLine(0x390f, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	Common::String selectedTopic;
	for (;;) {
		if (runtime.matchesResponseLine(selectedTopic, 0x82))
			return playDwayneLine(0x3395);

		Common::Error menuError = runtime.runKeywordMenu(
			dwayneTopicBuffer, dwayneTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (runtime.matchesResponseLine(selectedTopic, 0x83)) {
			Common::Error lineError = playDwayneLine(0x326e);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x84);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kDwayneWhaleyTopicResponseLines,
				ARRAYSIZE(kDwayneWhaleyTopicResponseLines))) {
			if (!sheriffInDiner) {
				const DialogueLineEntry lines[] = {
					{ 0x327b, "DWAYNE", 0 },
					{ 0x3280, "PC", 2 },
					{ 0x3285, "DWAYNE", 0 },
					{ 0x328b, "LOOMIS", 1 },
					{ 0x3290, "DWAYNE", 2 },
					{ 0x3294, "LOOMIS", 3 },
					{ 0x3299, "PC", 4 },
					{ 0x329e, "DWAYNE", 1 }
				};
				Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				sharedState.dialogueStateD2f04 = true;
				assignDwayneTopicBuffer(0x89);
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kDwayneLoomisTopicResponseLines,
				ARRAYSIZE(kDwayneLoomisTopicResponseLines))) {
			if (!sheriffInDiner) {
				const DialogueLineEntry lines[] = {
					{ 0x32bf, "DWAYNE", 1 },
					{ 0x32c1, "DWAYNE", 0 },
					{ 0x32c7, "LOOMIS", 3 },
					{ 0x32cb, "DWAYNE", 0 },
					{ 0x32d1, "DWAYNE", 1 }
				};
				Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignDwayneTopicBuffer(0x8e);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x8f)) {
			const DialogueLineEntry lines[] = {
				{ 0x32db, "DWAYNE", 0 },
				{ 0x32e2, "PC", 4 },
				{ 0x32e6, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x90);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x91)) {
			const DialogueLineEntry lines[] = {
				{ 0x32f2, "DWAYNE", 0 },
				{ 0x32f9, "PC", 0 },
				{ 0x32fd, "DWAYNE", 0 },
				{ 0x32fe, "DWAYNE", 0 },
				{ 0x32ff, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x92);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x93)) {
			if (!runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")) {
				Common::Error lineError = playDwayneLine(0x330d, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			const DialogueLineEntry lines[] = {
				{ 0x331f, "DWAYNE", 3 },
				{ 0x3320, "DWAYNE", 0 },
				{ 0x3321, "DWAYNE", 0 },
				{ 0x332c, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x94);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x95)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playDwayneLine(0x3339);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x96, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				const DialogueLineEntry responseLines[] = {
					{ 0x3345, "DWAYNE", 2 },
					{ 0x3346, "DWAYNE", 2 },
					{ 0x3347, "DWAYNE", 2 }
				};
				lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x3350);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x97);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x98)) {
			const DialogueLineEntry lines[] = {
				{ 0x335b, "DWAYNE", 2 },
				{ 0x3360, "PC", 2 },
				{ 0x3364, "DWAYNE", 3 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = sharedState.dialogueStateD2f04
				? playDwayneLine(0x3368, 2)
				: playDwayneLine(0x336c, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playDwayneLine(0x3370, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x99);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x9a)) {
			Common::Error lineError = playDwayneLine(0x337a, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x9b);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x9c)) {
			const DialogueLineEntry lines[] = {
				{ 0x3386, "DWAYNE", 0 },
				{ 0x3387, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x9d);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x9e))
			continue;

		Common::Error lineError = playDwayneLine(0x393c);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

Common::Error JimmyDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	JimmyRoomDialogueState &state = _state;

	auto playJimmyLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "JIMMY", headVariant);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<StartupObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
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
				runtime.startupScript().setRuntimeFlagValue("GIVEN_PAPER_TODAY", true);
			const bool changedNewspaperState =
				runtime.startupScript().resetRuntimeObjectToInitialState("NEWSPAPER");
			jimmyInteraction.mutatedRuntimeState = changedGivenPaperToday || changedNewspaperState;

			StartupInteractionResult actionInteraction;
			if (runtime.startupScript().executeNestedActionTag("ACTV_HOUSE_EXIT", actionInteraction)) {
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
				if (!actionInteraction.dialogueContinuationTag.empty())
					jimmyInteraction.dialogueContinuationTag = actionInteraction.dialogueContinuationTag;
				jimmyInteraction.requestMainMenu =
					jimmyInteraction.requestMainMenu || actionInteraction.requestMainMenu;
				for (const StartupAudioCommand &command : actionInteraction.audioCommands)
					jimmyInteraction.audioCommands.push_back(command);
			}
			runtime.queueDialogueInteractionIfNeeded(jimmyInteraction);

			if (!state.paperHandoffStateSet) {
				state.paperHandoffStateSet = true;
				Common::Error lineError = playJimmyLine(0x4a4c, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			return runtime.getRandomNumber(1) == 0
				? playJimmyLine(0x4acc, 0)
				: playJimmyLine(0x4a4b, 1);
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
			return playJimmyLine(0x4af8, 1);
		}
		if (((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
					usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
				(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
			} else {
				(void)runtime.startupScript().setRuntimeFlagValue(kShownLedgersToAnyoneFlag, true);
			}
			return playJimmyLine(0x4b00, 0);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownEvidenceOfBlackmailFlag, true);
			return playJimmyLine(0x4b21, 0);
		}
		if (usedItemName.equalsIgnoreCase("SNEAKERS")) {
			Common::Error lineError = playJimmyLine(0x4a9e, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return playJimmyLine(0x4af2, 0);
	}

	if (state.firstNoItemLinePending) {
		state.firstNoItemLinePending = false;
		if (!state.paperHandoffStateSet) {
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
	if (runtime.startupScript().getFlagValue("PAPER_CHK_4")) {
		Common::Error lineError = playJimmyLine(0x4ae2, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("PAPER_CHK_3")) {
		Common::Error lineError = playJimmyLine(0x4adb, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("PAPER_CHK_2")) {
		Common::Error lineError = playJimmyLine(0x4ad4, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return playJimmyLine(0x4b38, 0);
}

Common::Error PtaMomDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	(void)usedItemName;

	auto playPtaMomLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kPtaMomSpeakerId, headVariant);
	};

	if (runtime.startupScript().getFlagValue(kPtaRespondToTvFlag)) {
		Common::Error lineError = playPtaMomLine(0x3233, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue(kPtaRespondToTvFlag, false);
		return Common::kNoError;
	}

	Common::Error lineError = Common::kNoError;
	switch (runtime.getRandomNumber(2)) {
	case 0:
		lineError = playPtaMomLine(0x31ee, 1);
		break;
	case 1:
		lineError = playPtaMomLine(0x31f2, 1);
		break;
	default:
		lineError = playPtaMomLine(0x31f6, 1);
		break;
	}
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(0x297, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	switch (responseIndex) {
	case 1:
		return runtime.playDialogueEntrySequence(kPtaMomResponse1Lines, ARRAYSIZE(kPtaMomResponse1Lines));
	case 2:
		return runtime.playDialogueEntrySequence(kPtaMomResponse2Lines, ARRAYSIZE(kPtaMomResponse2Lines));
	case 3:
		return runtime.playDialogueEntrySequence(kPtaMomResponse3Lines, ARRAYSIZE(kPtaMomResponse3Lines));
	default:
		return Common::kNoError;
	}
}

Common::Error WaspWomanDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	WaspWomanRoomDialogueState &state = _state;
	Common::String waspWomanTopicBuffer;
	int waspWomanTopicBufferLineIndex = -1;
	auto assignWaspWomanTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(waspWomanTopicBuffer, waspWomanTopicBufferLineIndex,
			responseLineIndex, "Wasp Woman topic buffer");
	};
	auto playWaspWomanLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "WASP_WOMAN", headVariant);
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
			return runtime.playDialogueEntrySequence(kWaspWomanWhaleyLines,
				ARRAYSIZE(kWaspWomanWhaleyLines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
			return playWaspWomanLine(0x4cbd, 0);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownEvidenceOfBlackmailFlag, true);
			return playWaspWomanLine(0x4cc3, 0);
		}

		return playWaspWomanLine(0x4ca0, 0);
	}

	if (state.introPending) {
		state.introPending = false;

		Common::Error lineError = playWaspWomanLine(0x4bee, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.waspWomanDialogueState) {
			lineError = runtime.playDialogueLine(0x4bf2, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = runtime.playDialogueEntrySequence(kWaspWomanIntroTailLines,
			ARRAYSIZE(kWaspWomanIntroTailLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	Common::Error lineError = playWaspWomanLine(0x4bee, 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	assignWaspWomanTopicBuffer(kWaspWomanTopicBufferResponseLine);

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			waspWomanTopicBuffer, waspWomanTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty() || selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()))
			return Common::kNoError;

		if (runtime.matchesResponseLine(selectedTopic, 0x302)) {
			lineError = playWaspWomanLine(0x4c0a, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x305)) {
			lineError = playWaspWomanLine(0x4c31, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (!sharedState.waspWomanDialogueState)
				continue;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x307, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = runtime.playDialogueEntrySequence(kWaspWomanTopic305Response1Lines,
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
		if (runtime.matchesResponseLine(selectedTopic, 0x308)) {
			lineError = playWaspWomanLine(0x4c67, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x30a)) {
			lineError = playWaspWomanLine(0x4c74, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x30c)) {
			lineError = playWaspWomanLine(0x4c85, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x30e)) {
			lineError = playWaspWomanLine(0x4c9a, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return Common::kNoError;
		}
	}
}

Common::Error MomDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	const int currentStoryDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	MomRoomDialogueState &state = _state;
	Common::String momTopicBuffer;
	int momTopicBufferLineIndex = -1;
	auto assignMomTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(momTopicBuffer, momTopicBufferLineIndex,
			responseLineIndex, "Mom topic buffer");
	};
	auto runMomGoodbye = [&]() -> Common::Error {
		if (!runtime.startupScript().getFlagValue("DAY_FLAG"))
			return runtime.playDialogueLine(0x256e, "MOM");
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY"))
			return Common::kNoError;

		return runtime.playDialogueLine(0x2051, "MOM");
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownEvidenceOfBlackmailFlag, true);
			return runtime.playDialogueLine(0x2317, "MOM");
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownLedgersToAnyoneFlag, true);
			return runtime.playDialogueLine(0x2320, "MOM");
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
			return runtime.playDialogueLine(0x233b, "PC");
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
			return runtime.playDialogueLine(0x239c, "MOM");
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownEvidenceSheriffOwnsFlag, true);
			return runtime.playDialogueLine(0x2382, "MOM");
		}

		return runtime.playDialogueLine(0x26cc, "MOM");
	}

	bool skipMomDefaultKeywordSeed = false;
	if (state.introPending) {
		state.introPending = false;
		state.sameDayIntroLineEnabled = true;
		state.postIntroDefaultLineEnabled = true;
		state.introDayIndex = currentStoryDayIndex;
		Common::Error lineError = runtime.playDialogueEntrySequence(kMomIntroLines, ARRAYSIZE(kMomIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomInitialTopicBufferResponseLine);
		skipMomDefaultKeywordSeed = true;
	}
	if (!skipMomDefaultKeywordSeed && state.sameDayIntroLineEnabled &&
			currentStoryDayIndex == state.introDayIndex) {
		Common::Error lineError = runtime.playDialogueLine(0x2047, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomSameDayTopicBufferResponseLine);
		skipMomDefaultKeywordSeed = true;
	}
	if (!skipMomDefaultKeywordSeed && state.postIntroDefaultLineEnabled) {
		Common::Error lineError = runtime.playDialogueLine(0x2311, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomPostIntroTopicBufferResponseLine);
	}
	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x205a, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			(runtime.startupScript().getFlagValue("KARIN_KIDNAPED") ||
				runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD")) &&
			!state.dinerBurnedKarinMissingOrDeadShown) {
		state.dinerBurnedKarinMissingOrDeadShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2456, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2220, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
			!state.scratchedTuckerShown) {
		state.scratchedTuckerShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x23f3, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x23fb, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
			!state.boltOfClothTakenShown) {
		state.boltOfClothTakenShown = true;
		if (runtime.startupScript().isNamedNpcDeathTypeClear("SPARKY") &&
				runtime.startupScript().isNamedNpcDeathTypeClear("FIREMAN2")) {
			Common::Error lineError = runtime.playDialogueLine(0x2416, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		Common::Error lineError = runtime.playDialogueLine(0x2420, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.dinerBurnedKarinAliveShown) {
		state.dinerBurnedKarinAliveShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2434, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") &&
			!state.escapedJailShown) {
		state.escapedJailShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2495, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinKidnapedUnresolvedShown) {
		state.karinKidnapedUnresolvedShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x24d7, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2505, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2576, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!runtime.startupScript().isNamedNpcDeathTypeClear("BUTCHER") &&
			!state.butcherAbsentShown) {
		state.butcherAbsentShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2633, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
			!state.moynahanAbsentShown) {
		state.moynahanAbsentShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2647, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY") &&
			!state.jimmyAbsentShown) {
		state.jimmyAbsentShown = true;
		if (runtime.startupScript().getFlagValue("JIMMY_ATTACKED")) {
			Common::Error lineError = runtime.playDialogueLine(0x2659, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		Common::Error lineError = runtime.playDialogueLine(0x266c, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!runtime.startupScript().isNamedNpcDeathTypeClear("WASP_WOMAN") &&
			!state.waspWomanAbsentShown) {
		state.waspWomanAbsentShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2689, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") &&
			!runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephanieDeadPreMidgameShown) {
		state.stephanieDeadPreMidgameShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x217c, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DAY_5") && !state.day5Shown) {
		state.day5Shown = true;
		Common::Error lineError = runtime.playDialogueLine(0x218d, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomDay5TopicBufferResponseLine);
	}
	if (runtime.startupScript().getFlagValue("DAY_6") && !state.day6Shown) {
		state.day6Shown = true;
		Common::Error lineError = runtime.playDialogueLine(0x22a8, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomDay6TopicBufferResponseLine);
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			momTopicBuffer, momTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()))
			return runMomGoodbye();

		if (runtime.matchesResponseLine(selectedTopic, 0x116)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomCookiesLines, ARRAYSIZE(kMomCookiesLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x118);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x119)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic119Lines, ARRAYSIZE(kMomTopic119Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x11a);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x11d)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic11dOpeningLines, ARRAYSIZE(kMomTopic11dOpeningLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC008AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueEntrySequence(
				kMomTopic11dPostC008ALines, ARRAYSIZE(kMomTopic11dPostC008ALines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			StartupInteractionResult babyGurgleInteraction;
			if (runtime.startupScript().executeActionTag(kBabyGurgleActionTag, babyGurgleInteraction)) {
				runtime.applyImmediateDialogueInteractionEffects(babyGurgleInteraction);
				runtime.queueDialogueInteractionIfNeeded(babyGurgleInteraction);
			}

			lineError = runtime.playDialogueEntrySequence(
				kMomTopic11dClosingLines, ARRAYSIZE(kMomTopic11dClosingLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC008BFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x120);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x121)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic121Lines, ARRAYSIZE(kMomTopic121Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x122);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x123)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomSparkyLines, ARRAYSIZE(kMomSparkyLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x124);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomPtaTopicResponseLines,
				ARRAYSIZE(kMomPtaTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomPtaLines, ARRAYSIZE(kMomPtaLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomPtaFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12b)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic12bLines, ARRAYSIZE(kMomTopic12bLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x12c);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12d)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic12dLines, ARRAYSIZE(kMomTopic12dLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x12e);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12f)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic12fLines, ARRAYSIZE(kMomTopic12fLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x130);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomCookingTopicResponseLines,
				ARRAYSIZE(kMomCookingTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomCookingLines, ARRAYSIZE(kMomCookingLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomCookingFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x135)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic135Lines, ARRAYSIZE(kMomTopic135Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x136);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x137)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic137Lines, ARRAYSIZE(kMomTopic137Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13a)) {
			Common::Error lineError = runtime.playDialogueLine(0x1fa3, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x13b);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13c)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomSlaughterhouseLines, ARRAYSIZE(kMomSlaughterhouseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomSlaughterhouseFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13f)) {
			state.fatherTopicState = true;
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomFatherLines, ARRAYSIZE(kMomFatherLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().getFlagValue("HEARD_DAD_MOAN")) {
				lineError = runtime.playDialogueEntrySequence(
					kMomFatherHeardDadMoanLines, ARRAYSIZE(kMomFatherHeardDadMoanLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			assignMomTopicBuffer(kMomFatherFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x141)) {
			Common::Error lineError = Common::kNoError;
			if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")) {
				lineError = runtime.playDialogueEntrySequence(
					kMomStephMidgameLodgeLines, ARRAYSIZE(kMomStephMidgameLodgeLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			sharedState.discussedLodgeTopic = true;
			lineError = runtime.playDialogueEntrySequence(kMomLodgeLines, ARRAYSIZE(kMomLodgeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x142, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = runtime.playDialogueEntrySequence(
					kMomLodgeResponse1Lines, ARRAYSIZE(kMomLodgeResponse1Lines));
				break;
			case 2:
				lineError = runtime.playDialogueEntrySequence(
					kMomLodgeResponse2Lines, ARRAYSIZE(kMomLodgeResponse2Lines));
				break;
			default:
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomLodgeFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomBakeSaleTopicResponseLines,
				ARRAYSIZE(kMomBakeSaleTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomBakeSaleLines, ARRAYSIZE(kMomBakeSaleLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomBakeSaleFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x14a)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic14aLines, ARRAYSIZE(kMomTopic14aLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomPottsdamTopicResponseLines,
				ARRAYSIZE(kMomPottsdamTopicResponseLines))) {
			Common::Error lineError = Common::kNoError;
			if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED"))
				lineError = runtime.playDialogueLine(0x2086, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomPottsdamFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomMeatPlantTopicResponseLines,
				ARRAYSIZE(kMomMeatPlantTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomMeatPlantIntroLines, ARRAYSIZE(kMomMeatPlantIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x153, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = runtime.playDialogueEntrySequence(
					kMomMeatPlantResponse1Lines, ARRAYSIZE(kMomMeatPlantResponse1Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			case 2:
				lineError = runtime.playDialogueEntrySequence(
					kMomMeatPlantResponse2Lines, ARRAYSIZE(kMomMeatPlantResponse2Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomMeatPlantFollowupTopicBufferResponseLine);
				continue;
			default:
				continue;
			}
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x155)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic155Lines, ARRAYSIZE(kMomTopic155Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x156)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic156Lines, ARRAYSIZE(kMomTopic156Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x15b) && currentStoryDayIndex == 5) {
			Common::Error lineError = Common::kNoError;
			if (state.goodCauseDay5State) {
				lineError = runtime.playDialogueEntrySequence(
					kMomGoodCauseDay5Lines, ARRAYSIZE(kMomGoodCauseDay5Lines));
			} else {
				lineError = runtime.playDialogueLine(0x21dd, "PC");
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomGoodCauseDay5FollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomMoynahanTopicResponseLines,
				ARRAYSIZE(kMomMoynahanTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueLine(0x220a, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomNewspaperFireTopicResponseLines,
				ARRAYSIZE(kMomNewspaperFireTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueLine(0x229e, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x16d)) {
			Common::Error lineError = runtime.playDialogueLine(0x2696, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x16e)) {
			Common::Error lineError = runtime.playDialogueLine(0x26a3, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x170)) {
			Common::Error lineError = runtime.playDialogueLine(0x26bc, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}

		Common::Error lineError = runtime.playDialogueLine(0x26c6, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runMomGoodbye();
	}
}

Common::Error HankDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	const int currentStoryDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	HankRoomDialogueState &state = _state;
	Common::String &hankTopicBuffer = state.currentTopicBuffer;
	int &hankTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	auto assignHankTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(hankTopicBuffer, hankTopicBufferLineIndex,
			responseLineIndex, "Hank topic buffer");
	};

	debugC(1, kDebugDialogue,
		"Harvester: Hank dialogue start day=%d item='%s' initial=%d trackedDayValid=%d trackedDay=%d sameDayPending=%d rangshotPending=%d topicLine=%d topicBuffer='%s'",
		currentStoryDayIndex, usedItemName.empty() ? "<none>" : usedItemName.c_str(),
		(int)state.pendingInitialConversation,
		(int)state.hasTrackedDayState, state.trackedDayIndex,
		(int)state.pendingSameDayFollowup,
		(int)state.pendingRangshotSequence,
		hankTopicBufferLineIndex, hankTopicBuffer.c_str());

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfCorpseFlag, true);
			return runtime.playDialogueLine(0xa3e, "HANK");
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kShownPhotoOfWhaleyHerrillFlag, true);
			return runtime.playDialogueLine(0xa53, "HANK");
		}
		return runtime.playDialogueLine(0xa38, "HANK");
	}

	bool skipHankFollowupBranches = false;
	if (state.pendingInitialConversation) {
		state.trackedDayIndex = currentStoryDayIndex;
		state.hasTrackedDayState = true;
		state.pendingSameDayFollowup = true;
		state.pendingInitialConversation = false;
		hankTopicBuffer.clear();
		hankTopicBufferLineIndex = -1;
		skipHankFollowupBranches = true;

		Common::Error lineError = runtime.playDialogueLine(0x703, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xc7, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = runtime.playDialogueLine(0x70f, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xc8);
			break;
		case 2:
			lineError = runtime.playDialogueLine(0x715, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xc9);
			break;
		case 3:
			lineError = runtime.playDialogueLine(0x71c, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xca);
			break;
		default:
			break;
		}
	}

	if (!skipHankFollowupBranches && (!state.hasTrackedDayState ||
			currentStoryDayIndex != state.trackedDayIndex)) {
		if (state.pendingSameDayFollowup) {
			state.trackedDayIndex = currentStoryDayIndex;
			state.pendingSameDayFollowup = false;
			state.pendingRangshotSequence = true;

			Common::Error lineError = runtime.playDialogueLine(0x8f5, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (state.pendingRangshotSequence) {
			Common::Error lineError = currentStoryDayIndex > 5
				? runtime.playDialogueLine(0x8e2, "HANK")
				: runtime.playDialogueFst(kDialogueRangshotFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	} else if (!skipHankFollowupBranches) {
		state.pendingSameDayFollowup = true;
		Common::Error lineError = currentStoryDayIndex > 5
			? runtime.playDialogueLine(0x8e2, "HANK")
			: runtime.playDialogueFst(kDialogueRangshotFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgamePlayedShown) {
		state.stephMidgamePlayedShown = true;

		Common::Error lineError = runtime.playDialogueLine(0x92c, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xcc, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineEntry lines[] = {
				{ 0x939, "HANK", 0 },
				{ 0x93f, "MOM", 2 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2:
			lineError = runtime.playDialogueLine(0x944, "HANK");
			break;
		case 3:
			lineError = runtime.playDialogueLine(0x948, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLine(0x94f, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0xcd, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineEntry lines[] = {
				{ 0x95a, "HANK", 0 },
				{ 0x961, "PC", 2 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2: {
			const DialogueLineEntry lines[] = {
				{ 0x965, "HANK", 0 },
				{ 0x96b, "PC", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLine(0x971, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x987, "PC");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("BUSTED_ONCE") &&
			!state.bustedOnceShown) {
		state.bustedOnceShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x9b1, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xce, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineEntry lines[] = {
				{ 0x9bc, "HANK", 0 },
				{ 0x9c9, "HANK", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2: {
			const DialogueLineEntry lines[] = {
				{ 0x9c1, "HANK", 0 },
				{ 0x9c5, "PC", 0 },
				{ 0x9c9, "HANK", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedShown) {
		state.karinKidnapedShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x9d5, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (sharedState.karinKidnapedDialogueState) {
			lineError = runtime.playDialogueLineWithVariant(0x9da, "PC", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		const DialogueLineEntry lines[] = {
			{ 0x9de, "HANK", 0 },
			{ 0x9df, "HANK", 0 },
			{ 0x9e0, "HANK", 0 },
			{ 0x9e1, "HANK", 0 },
			{ 0x9e2, "HANK", 0 }
		};
		lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xcf, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineEntry responseLines[] = {
				{ 0x9f4, "HANK", 0 },
				{ 0x9fc, "HANK", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			break;
		}
		case 2:
			lineError = runtime.playDialogueLine(0xa01, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;
		Common::Error lineError = runtime.playDialogueLine(0xa0b, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		Common::Error lineError = runtime.playDialogueLine(0xa15, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xd0, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = runtime.playDialogueLine(0xa20, "HANK");
			break;
		case 2:
			lineError = runtime.playDialogueLine(0xa26, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLine(0xa2b, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			hankTopicBuffer, hankTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;

		if (selectedTopic.empty())
			return Common::kNoError;
		debugC(1, kDebugDialogue, "Harvester: Hank selected topic='%s'", selectedTopic.c_str());
		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()))
			return runtime.playDialogueLine(0x8dc, "HANK");

		const Common::String momTopic = runtime.startupText().getDialogueResponseLine(0xd2);
		Common::Array<Common::String> currentTopics;
		splitDialogueMenuLine(hankTopicBuffer, currentTopics);
		const bool matchesMomTopicByLine = !momTopic.empty() && selectedTopic.equalsIgnoreCase(momTopic);
		const bool matchesMomTopicFromIntroBuffer = hankTopicBufferLineIndex == 0xc8 &&
			!currentTopics.empty() && selectedTopic.equalsIgnoreCase(currentTopics[0]);
		if (matchesMomTopicByLine || matchesMomTopicFromIntroBuffer) {
			debugC(1, kDebugDialogue,
				"Harvester: Hank matched special Mom branch '%s' (lineMatch=%d bufferMatch=%d)",
				selectedTopic.c_str(), (int)matchesMomTopicByLine, (int)matchesMomTopicFromIntroBuffer);
			const DialogueLineEntry lines[] = {
				{ 0x725, "HANK", 0 },
				{ 0x729, "PC", 0 },
				{ 0x72d, "HANK", 0 },
				{ 0x733, "PC", 0 },
				{ 0x737, "HANK", 0 },
				{ 0x73b, "PC", 0 },
				{ 0x741, "HANK", 0 },
				{ 0x747, "MOM", 2 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.setActiveSpeakerPortrait("HANK", 0);
			assignHankTopicBuffer(0xd5);
			continue;
		}

		bool handledTopic = false;
		for (const HankDialogueTopicLine &topic : kHankDialogueTopicLines) {
			if (topic.responseLineIndex == 0xd2)
				continue;

			const Common::String topicText =
				runtime.startupText().getDialogueResponseLine(topic.responseLineIndex);
			if (topicText.empty() || !selectedTopic.equalsIgnoreCase(topicText))
				continue;

			if (topic.setDiscussedLodgeTopic)
				sharedState.discussedLodgeTopic = true;

			debugC(1, kDebugDialogue,
				"Harvester: Hank matched topic '%s' to response line 0x%x (%d) -> wav=0x%x speaker='%s'",
				selectedTopic.c_str(), topic.responseLineIndex, topic.responseLineIndex + 1,
				topic.wavId, topic.speakerId);
			Common::Error lineError = runtime.playDialogueLine(topic.wavId, topic.speakerId);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			handledTopic = true;
			break;
		}

		Common::Error lineError = runtime.playDialogueLine(0xa32, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (handledTopic) {
			debugC(1, kDebugDialogue,
				"Harvester: Hank handled topic '%s' and now falls through to generic response/exit",
				selectedTopic.c_str());
		} else {
			debugC(1, kDebugDialogue,
				"Harvester: Hank topic '%s' fell back to generic response and exits dialogue",
				selectedTopic.c_str());
		}

		return runtime.playDialogueLine(0x8dc, "HANK");
	}
}

} // End of namespace Harvester
