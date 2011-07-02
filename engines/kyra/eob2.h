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
 */

#ifdef ENABLE_EOB

#ifndef KYRA_EOB2_H
#define KYRA_EOB2_H

#include "kyra/eobcommon.h"

namespace Kyra {

class DarkmoonSequenceHelper;

struct EobSequenceStep {
	uint8 command;
	uint8 obj;
	int16 x1;
	uint8 y1;
	uint8 delay;
	uint8 pal;
	uint8 x2;
	uint8 y2;
	uint8 w;
	uint8 h;
};

class DarkMoonEngine : public EobCoreEngine {
friend class GUI_Eob;
friend class DarkmoonSequenceHelper;
public:
	DarkMoonEngine(OSystem *system, const GameFlags &flags);
	~DarkMoonEngine();

private:
	// Init / Release
	Common::Error init();
	void initStaticResource();
	void initSpells();

	// Main Menu
	int mainMenu();
	int mainMenuLoop();
	int _menuChoiceInit;

	// Main loop
	void startupNew();
	void startupLoad() {}

	// Intro/Outro
	void seq_playIntro();
	void seq_playFinale();
	void seq_playCredits(DarkmoonSequenceHelper *sq, const uint8 *data, int sd, int backupPage, int tempPage, int speed);

	const char * const*_introStrings;
	const char * const *_cpsFilesIntro;
	const EobSequenceStep **_seqIntro;
	const EobShapeDef **_shapesIntro;

	const char * const*_finaleStrings;
	const uint8 *_creditsData;
	const char * const *_cpsFilesFinale;
	const EobSequenceStep **_seqFinale;
	const EobShapeDef **_shapesFinale;

	static const char *_palFilesIntro[];
	static const char *_palFilesFinale[];

	// characters
	void npcSequence(int npcIndex);

	const uint8 *_npcShpData;
	const char *const *_npc1Strings;
	const char *const *_npc2Strings;

	// items
	void updateUsedCharacterHandItem(int charIndex, int slot);

	// Monsters
	void generateMonsterPalettes(const char *file, int16 monsterIndex);
	void loadMonsterDecoration(const char *file, int16 monsterIndex);
	void replaceMonster(int unit, uint16 block, int d, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem);

	// Level
	const uint8 *loadDoorShapes(const char *filename, int doorIndex, const uint8 *shapeDefs);
	void drawDoorIntern(int type, int, int x, int y, int w, int wall, int mDim, int16, int16);

	const uint8 *_dscDoorType5Offs;

	// misc
	void drawLightningColumn();
	int resurrectionSelectDialogue();
	int charSelectDialogue();
	void characterLevelGain(int charIndex);
};

}	// End of namespace Kyra

#endif

#endif // ENABLE_EOB
