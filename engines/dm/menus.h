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
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_MENUS_H
#define DM_MENUS_H

#include "dm.h"
#include "champion.h"
#include "dungeonman.h"

namespace DM {

#define kM1_damageCantReach -1 // @ CM1_DAMAGE_CANT_REACH    
#define kM2_damageNoAmmunition -2 // @ CM2_DAMAGE_NO_AMMUNITION 

#define k0x8000_hitNonMaterialCreatures 0x8000 // @ MASK0x8000_HIT_NON_MATERIAL_CREATURES 

extern Box g1_BoxActionArea; // @ G0001_s_Graphic562_Box_ActionArea
extern Box g0_BoxSpellArea; // @ G0000_s_Graphic562_Box_SpellArea

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

	bool _g508_refreshActionArea; // @ G0508_B_RefreshActionArea
	bool _g509_actionAreaContainsIcons; // @ G0509_B_ActionAreaContainsIcons
	int16 _g513_actionDamage; // @ G0513_i_ActionDamage
	ActionList _g713_actionList; // @ G0713_s_ActionList
	byte *_gK72_bitmapSpellAreaLine; // @ K0072_puc_Bitmap_SpellAreaLine
	Thing _g517_actionTargetGroupThing; // @ G0517_T_ActionTargetGroupThing
	uint16 _g507_actionCount; // @ G0507_ui_ActionCount

	void f388_clearActingChampion(); // @ F0388_MENUS_ClearActingChampion
	void f386_drawActionIcon(ChampionIndex championIndex); // @ F0386_MENUS_DrawActionIcon

	void f395_drawMovementArrows(); // @ F0395_MENUS_DrawMovementArrows
	void f456_drawDisabledMenu(); // @ F0456_START_DrawDisabledMenus
	void f390_refreshActionAreaAndSetChampDirMaxDamageReceived(); // @ F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived
	void f387_drawActionArea(); // @ F0387_MENUS_DrawActionArea
	const char* f384_getActionName(ChampionAction actionIndex); // @ F0384_MENUS_GetActionName
	void f393_drawSpellAreaControls(ChampionIndex champIndex); // @ F0393_MENUS_DrawSpellAreaControls
	void f392_buildSpellAreaLine(int16 spellAreaBitmapLine);// @ F0392_MENUS_BuildSpellAreaLine
	void f394_setMagicCasterAndDrawSpellArea(int16 champIndex); // @ F0394_MENUS_SetMagicCasterAndDrawSpellArea
	void f457_drawEnabledMenus(); // @ F0457_START_DrawEnabledMenus_CPSF
	int16 f408_getClickOnSpellCastResult(); // @ F0408_MENUS_GetClickOnSpellCastResult
	int16 f412_getChampionSpellCastResult(uint16 champIndex); // @ F0412_MENUS_GetChampionSpellCastResult
	Spell *f409_getSpellFromSymbols(byte *symbols); // @ F0409_MENUS_GetSpellFromSymbols
	void f410_menusPrintSpellFailureMessage(Champion *champ, uint16 failureType, uint16 skillIndex); // @ F0410_MENUS_PrintSpellFailureMessage
	Potion *f411_getEmptyFlaskInHand(Champion *champ, Thing *potionThing); // @ F0411_MENUS_GetEmptyFlaskInHand
	void f404_createEvent70_light(int16 lightPower, int16 ticks); // @ F0404_MENUS_CreateEvent70_Light
	bool f403_isPartySpellOrFireShieldSuccessful(Champion *champ, bool spellShield, uint16 ticks, bool useMana); // @ F0403_MENUS_IsPartySpellOrFireShieldSuccessful
	void f397_drawAvailableSymbols(uint16 symbolStep); // @ F0397_MENUS_DrawAvailableSymbols
	void f398_drawChampionSymbols(Champion *champ); // @ F0398_MENUS_DrawChampionSymbols
	void f399_addChampionSymbol(int16 symbolIndex); // @ F0399_MENUS_AddChampionSymbol
	void f400_deleteChampionSymbol(); // @ F0400_MENUS_DeleteChampionSymbol
	bool f391_didClickTriggerAction(int16 actionListIndex); // @ F0391_MENUS_DidClickTriggerAction
	bool f407_isActionPerformed(uint16 champIndex, int16 actionIndex); // @ F0407_MENUS_IsActionPerformed
	void f406_setChampionDirectionToPartyDirection(Champion *champ); // @ F0406_MENUS_SetChampionDirectionToPartyDirection
	void f405_decrementCharges(Champion *champ); // @ F0405_MENUS_DecrementCharges
	bool f402_isMeleeActionPerformed(int16 champIndex, Champion *champ, int16 actionIndex, int16 targetMapX,
									 int16 targetMapY, int16 skillIndex); // @ F0402_MENUS_IsMeleeActionPerformed
	bool  f401_isGroupFrightenedByAction(int16 champIndex, uint16 actionIndex, int16 mapX, int16 mapY); // @ F0401_MENUS_IsGroupFrightenedByAction
	void f381_printMessageAfterReplacements(char *str); // @ F0381_MENUS_PrintMessageAfterReplacements
	void f389_processCommands116To119_setActingChampion(uint16 champIndex); // @ F0389_MENUS_ProcessCommands116To119_SetActingChampion
	void f383_setActionList(ActionSet *actionSet); // @ F0383_MENUS_SetActionList
	int16 f382_getActionObjectChargeCount(); // @ F0382_MENUS_GetActionObjectChargeCount
};

}

#endif // !DM_MENUS_H
