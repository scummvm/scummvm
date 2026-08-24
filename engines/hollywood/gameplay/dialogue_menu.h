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

#ifndef HOLLYWOOD_GAMEPLAY_DIALOGUE_MENU_H
#define HOLLYWOOD_GAMEPLAY_DIALOGUE_MENU_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"

namespace Hollywood {

class HollywoodEngine;

struct DialogueChoiceRecord {
	DialogueChoiceRecord();

	byte enabled; // Visible in the menu.
	byte nextNodeIndex;
	byte transitionMode;
	byte playerTextRowId;
	byte responseFrameIndex;
	byte disableAfterUse;
	byte reserved;
	byte selectable; // Clickable/selectable when visible.
};

struct DialogueMenuLine {
	DialogueMenuLine();

	Common::String text;
	byte choiceIndex;
	bool firstLineOfChoice;
	bool selectable;
};

struct DialogueMenuState {
	DialogueMenuState();

	void clear();
	bool visible() const { return lineCount != 0; }
	byte choiceForLine(byte lineIndex) const;

	Common::Array<DialogueMenuLine> lines;
	byte lineCount;
	byte highlightedLineIndex;
};

/**
 * Supplies scene callbacks while DialogueMenu owns the choice loop.
 *
 * The delegate keeps the underlying scene advancing and drawing; presentation
 * combines that frame with the supplied menu state.
 */
class DialogueMenuDelegate {
public:
	virtual ~DialogueMenuDelegate();

	virtual Common::String dialogueMenuText(byte stageId, byte textRowId) const = 0;
	virtual void advanceDialogueMenu(uint32 deltaMillis) = 0;
	virtual void drawDialogueMenuFrame() = 0;
	virtual void presentDialogueMenuFrame(const DialogueMenuState &state) = 0;
};

/**
 * Resolves a dialogue choice, entering a blocking UI loop only when needed.
 *
 * While waiting, the menu owns input and cursor state and calls its delegate on
 * every tick so the underlying scene does not freeze.
 */
class DialogueMenu {
public:
	enum {
		kCancelledChoice = 0xff
	};

	DialogueMenu(HollywoodEngine *vm, DialogueMenuDelegate *delegate);

	byte choose(byte stageId, const Common::Array<DialogueChoiceRecord> &records,
		byte depthIndex, byte nodeIndex);

private:
	enum {
		kChoicesPerNode = 7,
		kNodesPerDepth = 10,
		kMaxDepthCount = 10,
		kMaxVisibleLines = 10
	};

	bool build(byte stageId, const Common::Array<DialogueChoiceRecord> &records,
		byte depthIndex, byte nodeIndex, byte &selectableChoiceCount);
	void appendWrappedChoiceText(byte choiceIndex, const Common::String &text, bool selectable);
	bool pollEvents(bool &selected, bool &cancelled);
	void updateHoverFromCursor();
	byte lineAt(uint16 cursorY) const;

	HollywoodEngine *_vm;
	DialogueMenuDelegate *_delegate;
	DialogueMenuState _state;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_DIALOGUE_MENU_H
