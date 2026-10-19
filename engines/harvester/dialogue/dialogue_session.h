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

#ifndef HARVESTER_DIALOGUE_SESSION_H
#define HARVESTER_DIALOGUE_SESSION_H

#include "common/ptr.h"
#include "common/rect.h"
#include "harvester/bitmap.h"
#include "harvester/cft_font.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Common {
struct Event;
}

namespace Graphics {
class Screen;
}

namespace Harvester {

class Art;
class EntityManager;
class ResourceManager;
struct MenuTextConfig;

class RoomNpcDialogueSession : public DialogueRuntimeCallbacks {
public:
	struct DialogueResponseOptionLayout {
		Common::String text;
		Common::Array<Common::String> wrappedLines;
		char hotkey = 0;
		int rowStart = 0;
		int rowCount = 0;
	};

	RoomNpcDialogueSession(HarvesterEngine &engine, Common::Point &mousePos, Flow &flow,
		const IndexedBitmap &backdrop, const byte *palette, float paletteBrightness,
		const NpcRecord &npc);
	bool isValid() const;
	const Common::String &getGenericByeTopic() const;
	Common::Error playDialogueLineWithVariant(int wavId, const Common::String &speakerId,
		int headVariant) override;
	Common::Error playDialogueLine(int wavId, const Common::String &speakerId) override;
	Common::Error playDialogueEntrySequence(const DialogueLineEntry *lines, uint count) override;
	Common::Error playDialogueFst(const Common::String &path) override;
	Common::Error clearScreenToBlack() override;
	Common::Error showCdChangePrompt(int discNumber) override;
	Common::Error runKeywordMenu(const Common::String &topicBuffer, int topicBufferLineIndex,
		Common::String &selectedTopic,
		DialogueKeywordMenuSelectionState &selection) override;
	Common::Error runResponseMenu(int responseLineIndex, int &selectedIndex) override;
	Common::Error runResponseMenuText(const Common::String &responseLine, int &selectedIndex) override;
	Common::Error runGameOverScreen() override;
	void assignTopicBuffer(Common::String &topicBuffer, int &topicBufferLineIndex,
		int responseLineIndex, const char *) override;
	bool matchesResponseLine(const Common::String &selectedTopic, int responseLineIndex) override;
	bool matchesAnyResponseLine(const Common::String &selectedTopic,
		const int *responseLineIndices, uint responseLineCount) override;
	void queueDialogueInteractionIfNeeded(const InteractionResult &interaction) override;
	void applyImmediateDialogueInteractionEffects(InteractionResult &interaction) override;
	int getRandomNumber(int maxValue) override;
	void setActiveSpeakerPortrait(const Common::String &speakerId, int headVariant) override;

private:
	Graphics::Screen *getActiveScreen() const;
	Common::Error waitForPointerRelease();
	Common::Error waitForDialogueContinue(const IndexedBitmap *textboxBitmap,
		const Common::Array<Common::String> *subtitleLines);
	Common::Error activateDialogueDisc(ResourceManager &resources, int discNumber, int previousDisc);
	void drawFontString(const Graphics::Font &font, bool usesCft, const Common::String &text,
		int x, int y, int width, byte color);
	void ensureSpeakerPortrait(const Common::String &speakerId, int headVariant);
	void drawDialogueOverlay(const IndexedBitmap *overlayBitmap,
		const Common::Array<Common::String> *subtitleLines, const Common::Array<Common::String> *topics,
		int hoveredTopicIndex, bool hoverOther, const Common::String *textEntryValue,
		bool textEntryCursorVisible = false);
	bool handleKeywordTextEntry(const Common::Event &event, Common::String &typedTopic,
		bool &editingOther, Common::String &selectedTopic, DialogueKeywordMenuSelectionState &selection);
	bool selectResponseMenuHotkey(const Common::Event &event,
		const Common::Array<DialogueResponseOptionLayout> &options, int responseLineIndex,
		int &selectedIndex);
	void parseResponseMenuLine(const Common::String &responseLine,
		Common::Array<DialogueResponseOptionLayout> &options, uint &totalRows);
	int getResponseMenuItemAt(const Common::Array<DialogueResponseOptionLayout> &options,
		uint totalRows, const Common::Point &mousePos);
	void drawDialogueResponseMenu(const IndexedBitmap *textboxBitmap,
		const Common::Array<DialogueResponseOptionLayout> &options, int hoveredOptionIndex);
	Common::Error runResponseMenuTextInternal(const Common::String &responseLine, int responseLineIndex,
		int &selectedIndex);

	HarvesterEngine &_engine;
	Common::Point &_mousePos;
	Flow &_flow;
	const MenuTextConfig &_menuTextConfig;
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

} // End of namespace Harvester

#endif // HARVESTER_DIALOGUE_SESSION_H
