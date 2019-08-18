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

#include "kyra/engine/eobcommon.h"

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
	Common::Error init();
	void initStaticResource();
	void initSpells();

	// Main Menu
	int mainMenu();
	int mainMenuLoop();
	void townsUtilitiesMenu();

	int _menuChoiceInit;

	// Main loop
	void startupNew();
	void startupLoad();

	// Intro/Outro
	void seq_playIntro();
	void seq_playFinale();
	void seq_playCredits(DarkmoonSequenceHelper *sq, const uint8 *data, int sd, int backupPage, int tempPage, int speed);

	// Ingame sequence
	void seq_nightmare();
	void seq_kheldran();
	void seq_dranDragonTransformation();

	const int8 *_dreamSteps;
	const char *const *_kheldranStrings;

	// characters
	void drawNpcScene(int npcIndex);
	void runNpcDialogue(int npcIndex);

	const uint8 *_npcShpData;
	const char *const *_npcStrings[2];

	// items
	void updateUsedCharacterHandItem(int charIndex, int slot);

	// Monsters
	void generateMonsterPalettes(const char *file, int16 monsterIndex);
	void loadMonsterDecoration(Common::SeekableReadStream *stream, int16 monsterIndex);
	const uint8 *loadMonsterProperties(const uint8 *data);
	void replaceMonster(int unit, uint16 block, int d, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem);
	bool killMonsterExtra(EoBMonsterInPlay *m);

	// Level
	void loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2) {}
	const uint8 *loadDoorShapes(const char *filename, int doorIndex, const uint8 *shapeDefs);
	void drawDoorIntern(int type, int, int x, int y, int w, int wall, int mDim, int16, int16);

	const uint8 *_dscDoorType5Offs;

	// Magic
	void turnUndeadAutoHit();

	// Fight
	static const uint8 _monsterAcHitChanceTbl1[];
	static const uint8 _monsterAcHitChanceTbl2[];

	// Rest party
	void restParty_npc();
	bool restParty_extraAbortCondition();

	// Sound
	void snd_loadAmigaSounds(int level, int sub);

	const char *const *_amigaSoundFiles2;
	const char *const *_amigaSoundMapExtra;
	const int8 *_amigaSoundIndex1;
	const uint8 *_amigaSoundIndex2;
	const uint8 *_amigaSoundPatch;
	int _amigaSoundPatchSize;

	int _amigaCurSoundIndex;

	// misc
	void useHorn(int charIndex, int weaponSlot);
	bool checkPartyStatusExtra();
	void drawLightningColumn();
	int resurrectionSelectDialogue();
	int charSelectDialogue();
	void characterLevelGain(int charIndex);

	const KyraRpgGUISettings *guiSettings() const;

	const char *const *_hornStrings;
	const uint8 *_hornSounds;

	const char *const *_utilMenuStrings;

	static const KyraRpgGUISettings _guiSettingsDOS;
	static const KyraRpgGUISettings _guiSettingsFMTowns;
	static const KyraRpgGUISettings _guiSettingsAmiga;
	static const uint8 _egaDefaultPalette[];
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB
