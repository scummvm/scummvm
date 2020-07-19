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

class SegaCDResource;
class SegaSequencePlayer;
class EoBEngine : public EoBCoreEngine {
friend class GUI_EoB;
friend class GUI_EoB_SegaCD;
friend class EoBSeqPlayerCommon;
friend class EoBIntroPlayer;
friend class EoBPC98FinalePlayer;
friend class EoBAmigaFinalePlayer;
friend class TextDisplayer_SegaCD;
friend class SegaSequencePlayer;
public:
	EoBEngine(OSystem *system, const GameFlags &flags);
	~EoBEngine() override;

private:
	// Init
	Common::Error init() override;
	void initStaticResource();
	void initSpells() override;
	void loadItemsAndDecorationsShapes() override;
	Common::SeekableReadStreamEndian *getItemDefinitionFile(int index) override;

	// Main Menu
	int mainMenu() override;
	int mainMenuLoop();
	int _menuChoiceInit;

	struct RenderModePalFile {
		int renderMode;
		char filename[12];
	};

	struct TitleScreenConfig {
		const Common::Platform platform;
		const Common::Language lang;
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
	static const TitleScreenConfig _titleConfig[5];
	const TitleScreenConfig *_ttlCfg;

	// Main loop
	void startupNew() override;
	void startupLoad() override;
	void startupReset() override;

	// Intro/Outro/Sequence Playback
	enum IntroPart {
		kOnlyCredits = 0,
		kOnlyIntro,
		kCreditsAndIntro
	};

	void seq_playIntro(int part);
	void seq_playFinale() override;
	void seq_xdeath() override;

	void seq_segaOpeningCredits(bool jumpToTitle);
	void seq_segaFinalCredits();
	void seq_segaShowStats();

	void seq_segaSetupSequence(int sequenceId);
	void seq_segaRestoreAfterSequence();
	bool seq_segaPlaySequence(int sequenceId, bool setupScreen = false);
	void seq_segaPausePlayer(bool pause) override;

	const char *const *_finBonusStrings;
	SegaSequencePlayer *_seqPlayer;
	bool _xdth;

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
	void loadMonsterShapes(const char *filename, int monsterIndex, bool hasDecorations, int encodeTableIndex) override;
	void replaceMonster(int unit, uint16 block, int d, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) override;
	bool killMonsterExtra(EoBMonsterInPlay *m) override;
	void updateScriptTimersExtra() override;

	// Level
	void readLevelFileData(int level) override;
	void loadVcnData(const char *file, const uint8 *cgaMapping) override;
	Common::SeekableReadStreamEndian *getVmpData(const char *file) override;
	const uint8 *getBlockFileData(int level) override;
	Common::SeekableReadStreamEndian *getDecDefinitions(const char *decFile) override;
	void loadDecShapesToPage3(const char *shpFile) override;
	const uint8 *loadDoorShapes(const char *filename, int doorIndex, const uint8 *shapeDefs) override { return 0; }
	void loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2) override;
	void drawDoorIntern(int type, int index, int x, int y, int w, int wall, int mDim, int16 y1, int16 y2) override;
	void setLevelPalettes(int level) override;

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

	const uint8 *const *_doorShapesSrc;
	const uint8 *const *_doorSwitchShapesSrc;

	int _dcrResCur;

	// Fight
	void playStrikeAnimation(uint8 pos, Item itm) override;

	const uint8 *_redGrid;
	const uint8 **_strikeAnimShapes[7];
	static const uint8 _monsterAcHitChanceTbl1[];
	static const uint8 _monsterAcHitChanceTbl2[];

	// Magic
	void turnUndeadAuto() override;
	void turnUndeadAutoHit() override;

	const char *const *_turnUndeadString;
	const uint8 *_scrYellow;

	// Sound
	void snd_loadAmigaSounds(int level, int) override;
	void snd_updateLevelScore() override;

	int _levelCurTrack;

	// Misc
	void displayParchment(int id) override;
	const uint8 **makePortalShapes() override;
	bool checkPartyStatusExtra() override;
	int resurrectionSelectDialogue() override;
	void healParty();

	int _sceneShakeOffsetX;
	int _sceneShakeOffsetY;
	uint8 *_shakeBackBuffer1;
	uint8 *_shakeBackBuffer2;

	// Resource
	SegaCDResource *_sres;

	// GUI
	int clickedCamp(Button *button) override;

	void gui_drawPlayField(bool refresh) override;
	void gui_setupPlayFieldHelperPages(bool keepText = false) override;
	void gui_drawWeaponSlotStatus(int x, int y, int status) override;
	void gui_printInventoryDigits(int x, int y, int val) override;
	void gui_drawCharacterStatsPage() override;
	void gui_displayMap() override;
	void gui_drawSpellbook() override;
	void gui_updateAnimations() override;
	void gui_resetAnimations();

	void makeNameShapes(int charId = -1) override;
	void makeFaceShapes(int charId = -1) override;
	void printStatsString(const char *str, int x, int y);
	void printSpellbookString(uint16 *dst, const char *str, uint16 ntbl);
	void drawMapButton(const char *str, int x, int y);
	void drawMapPage(int level);
	void drawMapSpots(int level, int animState);
	void drawDialogueButtons() override;

	const KyraRpgGUISettings *guiSettings() const override;
	void useMainMenuGUISettings(bool toggle) override { _useMainMenuGUISettings = toggle; }

	int _compassDirection2;
	int _compassAnimDest;
	int _compassAnimPhase;
	int _compassAnimStep;
	int _compassAnimDelayCounter;
	bool _compassAnimSwitch;
	bool _compassAnimDone;
	bool _compassTilesRestore;
	uint8 *_compassData;

	const char *const *_mapStrings1;
	const char *const *_mapStrings2;
	const char *const *_mapStrings3;
	const uint8 **_invSmallDigits;
	const uint8 **_weaponSlotShapes;
	const uint16 *_addrTbl1;
	const uint16 *_textFieldPattern;
	const uint16 *_playFldPattern1;
	const uint16 *_invPattern;
	const uint16 *_statsPattern;
	const uint8 *_charTilesTable;
	uint16 *_playFldPattern2;
	uint16 *_tempPattern;

	static const KyraRpgGUISettings _guiSettingsVGA;
	static const KyraRpgGUISettings _guiSettingsEGA;
	static const KyraRpgGUISettings _guiSettingsPC98;
	static const KyraRpgGUISettings _guiSettingsAmiga;
	static const KyraRpgGUISettings _guiSettingsAmigaMainMenu;
	static const KyraRpgGUISettings _guiSettingsSegaCD;
	static const uint8 _egaDefaultPalette[];
	static const uint8 _redGridTile[8];
	static const int8 _sceneShakeOffsets[66];
	static const uint16 _dlgButtonPosX_Sega[18];
	static const uint8 _dlgButtonPosY_Sega[18];
	bool _useMainMenuGUISettings;
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB
