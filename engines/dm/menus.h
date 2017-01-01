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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
-* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_MENUS_H
#define DM_MENUS_H

#include "dm/dm.h"
#include "dm/champion.h"
#include "dm/dungeonman.h"

namespace DM {

enum Damage {
	kDMDamageCantReach = -1,   // @ CM1_DAMAGE_CANT_REACH
	kDMDamageNoAmmunition = -2 // @ CM2_DAMAGE_NO_AMMUNITION
};

enum SpellArea {
	kDMSpellAreaAvailableSymbols = 2, // @ C2_SPELL_AREA_AVAILABLE_SYMBOLS
	kDMSpellAreaChampionSymbols = 3   // @ C3_SPELL_AREA_CHAMPION_SYMBOLS
};

enum ActionMask {
	kDMActionMaskRequiresCharge = 0x0080,         // @ MASK0x0080_ACTION_REQUIRES_CHARGE
	kDMActionMaskHitNonMaterialCreatures = 0x8000 // @ MASK0x8000_HIT_NON_MATERIAL_CREATURES
};

class ActionList {
public:
	byte _minimumSkillLevel[3]; /* Bit 7: requires charge, Bit 6-0: minimum skill level. */
	ChampionAction _actionIndices[3];
	ActionList() { resetToZero(); }
	void resetToZero() {
		for (uint16 i = 0; i < 3; ++i) {
			_minimumSkillLevel[i] = 0;
			_actionIndices[i] = (ChampionAction)0;
		}
	}
}; // @ ACTION_LIST

class ActionSet {
public:
	byte _actionIndices[3]; /* 1 byte of padding inserted by compiler on Atari ST, not on Amiga */
	byte _actionProperties[2]; /* Bit 7: requires charge, Bit 6-0: minimum skill level */
	ActionSet(byte a1, byte a2, byte a3, byte b1, byte b2) {
		_actionIndices[0] = a1;
		_actionIndices[1] = a2;
		_actionIndices[2] = a3;
		_actionProperties[0] = b1;
		_actionProperties[1] = b2;
	}
}; // @ ACTION_SET

class MenuMan {
	DMEngine *_vm;
public:
	explicit MenuMan(DMEngine *vm);
	~MenuMan();

	bool _refreshActionArea; // @ G0508_B_RefreshActionArea
	bool _actionAreaContainsIcons; // @ G0509_B_ActionAreaContainsIcons
	int16 _actionDamage; // @ G0513_i_ActionDamage
	ActionList _actionList; // @ G0713_s_ActionList
	byte *_bitmapSpellAreaLine; // @ K0072_puc_Bitmap_SpellAreaLine
	byte *_bitmapSpellAreaLines; // @ K0073_puc_Bitmap_SpellAreaLines
	Thing _actionTargetGroupThing; // @ G0517_T_ActionTargetGroupThing
	uint16 _actionCount; // @ G0507_ui_ActionCount

	void clearActingChampion(); // @ F0388_MENUS_ClearActingChampion
	void drawActionIcon(ChampionIndex championIndex); // @ F0386_MENUS_DrawActionIcon

	void drawMovementArrows(); // @ F0395_MENUS_DrawMovementArrows
	void drawDisabledMenu(); // @ F0456_START_DrawDisabledMenus
	void refreshActionAreaAndSetChampDirMaxDamageReceived(); // @ F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived
	void drawActionArea(); // @ F0387_MENUS_DrawActionArea
	const char *getActionName(ChampionAction actionIndex); // @ F0384_MENUS_GetActionName
	void drawSpellAreaControls(ChampionIndex champIndex); // @ F0393_MENUS_DrawSpellAreaControls
	void buildSpellAreaLine(int16 spellAreaBitmapLine);// @ F0392_MENUS_BuildSpellAreaLine
	void setMagicCasterAndDrawSpellArea(ChampionIndex champIndex); // @ F0394_MENUS_SetMagicCasterAndDrawSpellArea
	void drawEnabledMenus(); // @ F0457_START_DrawEnabledMenus_CPSF
	int16 getClickOnSpellCastResult(); // @ F0408_MENUS_GetClickOnSpellCastResult
	int16 getChampionSpellCastResult(uint16 champIndex); // @ F0412_MENUS_GetChampionSpellCastResult
	Spell *getSpellFromSymbols(byte *symbols); // @ F0409_MENUS_GetSpellFromSymbols
	void menusPrintSpellFailureMessage(Champion *champ, uint16 failureType, uint16 skillIndex); // @ F0410_MENUS_PrintSpellFailureMessage
	Potion *getEmptyFlaskInHand(Champion *champ, Thing *potionThing); // @ F0411_MENUS_GetEmptyFlaskInHand
	void createEvent70_light(int16 lightPower, int16 ticks); // @ F0404_MENUS_CreateEvent70_Light
	bool isPartySpellOrFireShieldSuccessful(Champion *champ, bool spellShield, uint16 ticks, bool useMana); // @ F0403_MENUS_IsPartySpellOrFireShieldSuccessful
	void drawAvailableSymbols(uint16 symbolStep); // @ F0397_MENUS_DrawAvailableSymbols
	void drawChampionSymbols(Champion *champ); // @ F0398_MENUS_DrawChampionSymbols
	void addChampionSymbol(int16 symbolIndex); // @ F0399_MENUS_AddChampionSymbol
	void deleteChampionSymbol(); // @ F0400_MENUS_DeleteChampionSymbol
	bool didClickTriggerAction(int16 actionListIndex); // @ F0391_MENUS_DidClickTriggerAction
	bool isActionPerformed(uint16 champIndex, int16 actionIndex); // @ F0407_MENUS_IsActionPerformed
	void setChampionDirectionToPartyDirection(Champion *champ); // @ F0406_MENUS_SetChampionDirectionToPartyDirection
	void decrementCharges(Champion *champ); // @ F0405_MENUS_DecrementCharges
	bool isMeleeActionPerformed(int16 champIndex, Champion *champ, int16 actionIndex, int16 targetMapX,
									 int16 targetMapY, int16 skillIndex); // @ F0402_MENUS_IsMeleeActionPerformed
	bool isGroupFrightenedByAction(int16 champIndex, uint16 actionIndex, int16 mapX, int16 mapY); // @ F0401_MENUS_IsGroupFrightenedByAction
	void printMessageAfterReplacements(const char *str); // @ F0381_MENUS_PrintMessageAfterReplacements
	void processCommands116To119_setActingChampion(uint16 champIndex); // @ F0389_MENUS_ProcessCommands116To119_SetActingChampion
	void setActionList(ActionSet *actionSet); // @ F0383_MENUS_SetActionList
	int16 getActionObjectChargeCount(); // @ F0382_MENUS_GetActionObjectChargeCount
	void drawActionDamage(int16 damage); // @ F0385_MENUS_DrawActionDamage

	Box _boxActionArea3ActionMenu; // @ G0499_s_Graphic560_Box_ActionArea3ActionsMenu
	Box _boxActionArea2ActionMenu; // @ G0500_s_Graphic560_Box_ActionArea2ActionsMenu
	Box _boxActionArea1ActionMenu; // @ G0501_s_Graphic560_Box_ActionArea1ActionMenu
	Box _boxActionArea; // @ G0001_s_Graphic562_Box_ActionArea
	Box _boxSpellArea;
	unsigned char _actionSkillIndex[44]; // @ G0496_auc_Graphic560_ActionSkillIndex
	unsigned char _actionDisabledTicks[44];

	void initConstants();
};
}

#endif // DM_MENUS_H
