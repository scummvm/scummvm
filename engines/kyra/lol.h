/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_LOL_H
#define KYRA_LOL_H

#include "kyra/kyra_v1.h"
#include "kyra/script_tim.h"

#include "common/list.h"

namespace Kyra {

class Screen_LoL;
class WSAMovie_v2;
struct Button;

class LoLEngine : public KyraEngine_v1 {
public:
	LoLEngine(OSystem *system, const GameFlags &flags);
	~LoLEngine();

	Screen *screen();
private:
	Screen_LoL *_screen;
	TIMInterpreter *_tim;

	Common::Error init();
	Common::Error go();

	// initialization
	void preInit();

	void initializeCursors();

	int mainMenu();

	// intro
	void setupPrologueData(bool load);

	void showIntro();

	struct CharacterPrev {
		const char *name;
		int x, y;
		int attrib[3];
	};

	static const CharacterPrev _charPreviews[];

	WSAMovie_v2 *_chargenWSA;
	static const uint8 _chargenFrameTable[];
	int chooseCharacter();

	void kingSelectionIntro();
	void kingSelectionReminder();
	void kingSelectionOutro();
	void processCharacterSelection();
	void updateSelectionAnims();
	int selectionCharInfo(int character);
	void selectionCharInfoIntro(char *file);

	int getCharSelection();
	int selectionCharAccept();

	int _charSelection;
	int _charSelectionInfoResult;

	uint32 _selectionAnimTimers[4];
	uint8 _selectionAnimFrames[4];
	static const uint8 _selectionAnimIndexTable[];

	static const uint16 _selectionPosTable[];

	static const uint8 _selectionChar1IdxTable[];
	static const uint8 _selectionChar2IdxTable[];
	static const uint8 _selectionChar3IdxTable[];
	static const uint8 _selectionChar4IdxTable[];

	static const uint8 _reminderChar1IdxTable[];
	static const uint8 _reminderChar2IdxTable[];
	static const uint8 _reminderChar3IdxTable[];
	static const uint8 _reminderChar4IdxTable[];

	static const uint8 _charInfoFrameTable[];

	// timer
	void setupTimers() {}

	// sound
	void snd_playVoiceFile(int) { /* XXX */ }

	// opcode
	void setupOpcodeTable();

	Common::Array<const TIMOpcode*> _timIntroOpcodes;
	int tlol_setupPaletteFade(const TIM *tim, const uint16 *param);
	int tlol_loadPalette(const TIM *tim, const uint16 *param);
	int tlol_setupPaletteFadeEx(const TIM *tim, const uint16 *param);
	int tlol_processWsaFrame(const TIM *tim, const uint16 *param);
	int tlol_displayText(const TIM *tim, const uint16 *param);

	// translation
	int _lang;

	uint8 *_landsFile;

	int _lastUsedStringBuffer;
	char _stringBuffer[5][512];	// TODO: The original used a size of 512, it looks a bit large.
								// Maybe we can someday reduce the size.
	const char *getLangString(uint16 id);
	uint8 *getTableEntry(uint8 *buffer, uint16 id);

	static const char * const _languageExt[];

	// graphics
	uint8 *_shapes[138];

	// unneeded
	void setWalkspeed(uint8) {}
	void setHandItem(uint16) {}
	void removeHandItem() {}
	bool lineIsPassable(int, int) { return false; }

	// save
	Common::Error loadGameState(int slot) { return Common::kNoError; }
	Common::Error saveGameState(int slot, const char *saveName, const Graphics::Surface *thumbnail) { return Common::kNoError; }
};

} // end of namespace Kyra

#endif

