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

#ifndef KYRA_EOB1_H
#define KYRA_EOB1_H

#include "kyra/eobcommon.h"

namespace Kyra {

class EoBEngine : public EoBCoreEngine {
friend class GUI_EoB;
friend class EoBIntroPlayer;
public:
	EoBEngine(OSystem *system, const GameFlags &flags);
	~EoBEngine();

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
	void startupLoad() override;

	// Intro/Outro
	void seq_playIntro();
	void seq_playFinale() override;
	void seq_xdeath() override;

	const char *const *_finBonusStrings;

	// characters
	void drawNpcScene(int npcIndex) override;
	void encodeDrawNpcSeqShape(int npcIndex, int drawX, int drawY);
	void runNpcDialogue(int npcIndex) override;

	const uint8 *_npcShpData;
	const uint8 *_npcSubShpIndex1;
	const uint8 *_npcSubShpIndex2;
	const uint8 *_npcSubShpY;
	const char *const *_npcStrings[11];

	// items
	void updateUsedCharacterHandItem(int charIndex, int slot) override;

	// Monsters
	void replaceMonster(int unit, uint16 block, int d, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) override;
	bool killMonsterExtra(EoBMonsterInPlay *m) override;
	void updateScriptTimersExtra() override;

	// Level
	const uint8 *loadDoorShapes(const char *filename, int doorIndex, const uint8 *shapeDefs) override { return 0; }
	void loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2) override;
	void drawDoorIntern(int type, int index, int x, int y, int w, int wall, int mDim, int16 y1, int16 y2) override;

	const int16 *_dscDoorCoordsExt;
	const uint8 *_dscDoorScaleMult4;
	const uint8 *_dscDoorScaleMult5;
	const uint8 *_dscDoorScaleMult6;
	const uint8 *_dscDoorY3;
	const uint8 *_dscDoorY4;
	const uint8 *_dscDoorY5;
	const uint8 *_dscDoorY6;
	const uint8 *_dscDoorY7;

	const uint8 *_doorShapeEncodeDefs;
	const uint8 *_doorSwitchShapeEncodeDefs;
	const uint8 *_doorSwitchCoords;

	// Magic
	void turnUndeadAuto() override;
	void turnUndeadAutoHit() override;

	const char *const *_turnUndeadString;

	// Misc
	bool checkPartyStatusExtra() override;
	int resurrectionSelectDialogue() override;
	void healParty();

	const KyraRpgGUISettings *guiSettings() override;

	static const KyraRpgGUISettings _guiSettingsVGA;
	static const KyraRpgGUISettings _guiSettingsEGA;
	static const uint8 _egaDefaultPalette[];
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB
