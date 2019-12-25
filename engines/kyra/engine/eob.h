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

#include "kyra/engine/eobcommon.h"

namespace Kyra {

class EoBEngine : public EoBCoreEngine {
friend class GUI_EoB;
friend class EoBSeqPlayerCommon;
friend class EoBIntroPlayer;
friend class EoBPC98FinalePlayer;
friend class EoBAmigaFinalePlayer;
public:
	EoBEngine(OSystem *system, const GameFlags &flags);
	~EoBEngine();

private:
	// Init / Release
	Common::Error init();
	void initStaticResource();
	void initSpells();

	// Main Menu
	int mainMenu();
	int mainMenuLoop();
	int _menuChoiceInit;

	struct RenderModePalFile {
		int renderMode;
		char filename[12];
	};

	struct TitleScreenConfig {
		const Common::Platform platform;
		const char bmpFile[12];
		const RenderModePalFile *palFiles;
		const int pc98PaletteID;
		const int page;
		const bool fade;
		const int menu1X, menu1Y, menu1W, menu1H, menu1col1, menu1col2, menu1col3;
		const int menu2X, menu2Y, menu2W, menu2H, menu2col1, menu2col2, menu2col3;
		const int versionStrYOffs;
	};

	static const RenderModePalFile _renderModePalFiles[3];
	static const TitleScreenConfig _titleConfig[3];
	const TitleScreenConfig *_ttlCfg;

	// Main loop
	void startupNew();
	void startupLoad();

	// Intro/Outro
	enum IntroPart {
		kOnlyCredits = 0,
		kOnlyIntro,
		kCreditsAndIntro
	};

	void seq_playIntro(int part);
	void seq_playFinale();
	void seq_xdeath();

	const char *const *_finBonusStrings;
	bool _xdth;

	// characters
	void drawNpcScene(int npcIndex);
	void encodeDrawNpcSeqShape(int npcIndex, int drawX, int drawY);
	void runNpcDialogue(int npcIndex);

	const uint8 *_npcShpData;
	const uint8 *_npcSubShpIndex1;
	const uint8 *_npcSubShpIndex2;
	const uint8 *_npcSubShpY;
	const char *const *_npcStrings[11];

	// items
	void updateUsedCharacterHandItem(int charIndex, int slot);

	// Monsters
	void replaceMonster(int unit, uint16 block, int d, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem);
	bool killMonsterExtra(EoBMonsterInPlay *m);
	void updateScriptTimersExtra();

	// Level
	const uint8 *loadDoorShapes(const char *filename, int doorIndex, const uint8 *shapeDefs) { return 0; }
	void loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2);
	void drawDoorIntern(int type, int index, int x, int y, int w, int wall, int mDim, int16 y1, int16 y2);

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

	// Fight
	static const uint8 _monsterAcHitChanceTbl1[];
	static const uint8 _monsterAcHitChanceTbl2[];

	// Magic
	void turnUndeadAuto();
	void turnUndeadAutoHit();

	const char *const *_turnUndeadString;

	// Sound
	void snd_loadAmigaSounds(int level, int);

	// Misc
	bool checkPartyStatusExtra();
	int resurrectionSelectDialogue();
	void healParty();

	const KyraRpgGUISettings *guiSettings() const;
	void useMainMenuGUISettings(bool toggle) { _useMainMenuGUISettings = toggle; }

	static const KyraRpgGUISettings _guiSettingsVGA;
	static const KyraRpgGUISettings _guiSettingsEGA;
	static const KyraRpgGUISettings _guiSettingsPC98;
	static const KyraRpgGUISettings _guiSettingsAmiga;
	static const KyraRpgGUISettings _guiSettingsAmigaMainMenu;
	static const uint8 _egaDefaultPalette[];
	bool _useMainMenuGUISettings;
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB
