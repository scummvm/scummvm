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

#include "common/algorithm.h"
#include "common/debug.h"
#include "graphics/fontman.h"
#include "harvester/detection.h"
#include "harvester/flow/flow.h"
#include "harvester/flow/flow_support.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const int kDialogueGenericByeResponseIndex = 13;
static const char *const kDialogueKeywordBitmapPath = "1:/GRAPHIC/OTHER/KEYWORD.BM";
static const char *const kDialogueLocalizedKeywordBitmapPath = "GRAPHIC/OTHER/KEYWORD.BM";

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

} // End of namespace Harvester
