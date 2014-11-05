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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

struct DarkMoonAnimCommand {
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

class DarkMoonEngine : public EoBCoreEngine {
friend class GUI_EoB;
friend class DarkmoonSequenceHelper;
public:
	DarkMoonEngine(OSystem *system, const GameFlags &flags);
	~DarkMoonEngine();

private:
	// Init / Release
	Common::Error init() override;
	void initStaticResource();
	void initSpells() override;

	// Main Menu
	int mainMenu() override;
	int mainMenuLoop();

	int _menuChoiceInit;

	// Main loop
	void startupNew() override;
	void startupLoad() override {}

	// Intro/Outro
	void seq_playIntro();
	void seq_playFinale() override;
	void seq_playCredits(DarkmoonSequenceHelper *sq, const uint8 *data, int sd, int backupPage, int tempPage, int speed);

	const char *const *_introStrings;
	const char *const *_cpsFilesIntro;
	const DarkMoonAnimCommand **_animIntro;
	const DarkMoonShapeDef **_shapesIntro;

	const char *const *_finaleStrings;
	const uint8 *_creditsData;
	const char *const *_cpsFilesFinale;
	const DarkMoonAnimCommand **_animFinale;
	const DarkMoonShapeDef **_shapesFinale;

	static const char *const _palFilesIntroVGA[];
	static const char *const _palFilesIntroEGA[];
	static const char *const _palFilesFinaleVGA[];
	static const char *const _palFilesFinaleEGA[];

	// Ingame sequence
	void seq_nightmare();
	void seq_kheldran();
	void seq_dranDragonTransformation();

	const int8 *_dreamSteps;
	const char *const *_kheldranStrings;

	// characters
	void drawNpcScene(int npcIndex) override;
	void runNpcDialogue(int npcIndex) override;

	const uint8 *_npcShpData;
	const char *const *_npcStrings[2];

	// items
	void updateUsedCharacterHandItem(int charIndex, int slot) override;

	// Monsters
	void generateMonsterPalettes(const char *file, int16 monsterIndex) override;
	void loadMonsterDecoration(const char *file, int16 monsterIndex) override;
	void replaceMonster(int unit, uint16 block, int d, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) override;
	bool killMonsterExtra(EoBMonsterInPlay *m) override;

	// Level
	void loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2) override {}
	const uint8 *loadDoorShapes(const char *filename, int doorIndex, const uint8 *shapeDefs) override;
	void drawDoorIntern(int type, int, int x, int y, int w, int wall, int mDim, int16, int16) override;

	const uint8 *_dscDoorType5Offs;

	// Rest party
	void restParty_npc() override;
	bool restParty_extraAbortCondition() override;

	// misc
	void useHorn(int charIndex, int weaponSlot) override;
	bool checkPartyStatusExtra() override;
	void drawLightningColumn() override;
	int resurrectionSelectDialogue() override;
	int charSelectDialogue() override;
	void characterLevelGain(int charIndex) override;

	const KyraRpgGUISettings *guiSettings() override;

	const char *const *_hornStrings;
	const uint8 *_hornSounds;

	static const KyraRpgGUISettings _guiSettings;
	static const uint8 _egaDefaultPalette[];
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB
