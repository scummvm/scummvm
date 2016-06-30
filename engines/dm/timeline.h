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

#ifndef DM_TIMELINE_H
#define DM_TIMELINE_H

#include "dm.h"

namespace DM {

/* Event types */
enum TimelineEventType {
/* Used when a creature in a group was damaged or killed by a Poison Cloud or by a closing door or if Lord Chaos is surrounded by = 3, Fluxcages */
kTMEventTypeCreateReactionEvent29DangerOnSquare = 253, // @ CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE 
/* Used when a projectile impacts with a creature in a group */
kTMEventTypeCreateReactionEvent30HitByProjectile = 254, // @ CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE
/* Used when the party bumps into a group or performs a melee attack */
kTMEventTypeCreateReactionEvent31ParyIsAdjacent = 255,  // @ CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT
kTMEventTypeNone = 0, // @ C00_EVENT_NONE                                       
kTMEventTypeDoorAnimation = 1, // @ C01_EVENT_DOOR_ANIMATION                             
kTMEventTypeDoorDestruction = 2, // @ C02_EVENT_DOOR_DESTRUCTION                           
kTMEventTypeCorridor = 5, // @ C05_EVENT_CORRIDOR                                   
kTMEventTypeWall = 6, // @ C06_EVENT_WALL                                       
kTMEventTypeFakeWall = 7, // @ C07_EVENT_FAKEWALL                                   
kTMEventTypeTeleporter = 8, // @ C08_EVENT_TELEPORTER                                 
kTMEventTypePit = 9, // @ C09_EVENT_PIT                                        
kTMEventTypeDoor = 10, // @ C10_EVENT_DOOR                                      
kTMEventTypeEnableChampionAction = 11, // @ C11_EVENT_ENABLE_CHAMPION_ACTION                    
kTMEventTypeHideDamageReceived = 12, // @ C12_EVENT_HIDE_DAMAGE_RECEIVED                      
kTMEventTypeViAltarRebirth = 13, // @ C13_EVENT_VI_ALTAR_REBIRTH                          
kTMEventTypePlaySound = 20, // @ C20_EVENT_PLAY_SOUND                                
kTMEventTypeCPSE = 22, // @ C22_EVENT_CPSE                                      
kTMEventTypeRemoveFluxcage = 24, // @ C24_EVENT_REMOVE_FLUXCAGE                           
kTMEventTypeExplosion = 25, // @ C25_EVENT_EXPLOSION                                 
kTMEventTypeGroupReactionDangerOnSquare = 29, // @ C29_EVENT_GROUP_REACTION_DANGER_ON_SQUARE           
kTMEventTypeGroupReacionHitByProjectile = 30, // @ C30_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE          
kTMEventTypeGroupReactionPartyIsAdjecent = 31, // @ C31_EVENT_GROUP_REACTION_PARTY_IS_ADJACENT          
kTMEventTypeUpdateAspectGroup = 32, // @ C32_EVENT_UPDATE_ASPECT_GROUP                       
/* Events = 33,-36 and = 38,-41 are used for individual creatures only while the group is attacking the party */
kTMEventTypeUpdateAspectCreature_0 = 33,  // @ C33_EVENT_UPDATE_ASPECT_CREATURE_0                  
kTMEventTypeUpdateAspectCreature_1 = 34, // @ C34_EVENT_UPDATE_ASPECT_CREATURE_1                  
kTMEventTypeUpdateAspectCreature_2 = 35, // @ C35_EVENT_UPDATE_ASPECT_CREATURE_2                  
kTMEventTypeUpdateAspectCreature_3 = 36, // @ C36_EVENT_UPDATE_ASPECT_CREATURE_3                  
kTMEventTypeUpdateBehaviourGroup = 37, // @ C37_EVENT_UPDATE_BEHAVIOR_GROUP                     
kTMEventTypeUpdateBehaviour_0 = 38, // @ C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0                
kTMEventTypeUpdateBehaviour_1 = 39, // @ C39_EVENT_UPDATE_BEHAVIOR_CREATURE_1                
kTMEventTypeUpdateBehaviour_2 = 40, // @ C40_EVENT_UPDATE_BEHAVIOR_CREATURE_2                
kTMEventTypeUpdateBehaviour_3 = 41, // @ C41_EVENT_UPDATE_BEHAVIOR_CREATURE_3                
/* Projectiles created by a champion (by casting a spell, shooting a weapon or throwing an object) or by a creature (by casting a spell) ignore impacts during their first movement otherwise an impact would always occur immediately as these projectiles are created on the champion or creature square */
kTMEventTypeMoveProjectileIgnoreImpacts = 48, // @ C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS            
/* Projectiles created by projectile launcher sensors never ignore impacts as well as all other projectiles after their first movement */
kTMEventTypeMoveProjectile = 49,  // @ C49_EVENT_MOVE_PROJECTILE                           
kTMEventTypeWatchdoge = 53, // @ C53_EVENT_WATCHDOG                                  
kTMEventTypeMoveGroupSilent = 60, // @ C60_EVENT_MOVE_GROUP_SILENT                         
kTMEventTypeMoveGroupAudible = 61, // @ C61_EVENT_MOVE_GROUP_AUDIBLE                        
kTMEventTypeEnableGroupGenerator = 65, // @ C65_EVENT_ENABLE_GROUP_GENERATOR                    
kTMEventTypeLight = 70, // @ C70_EVENT_LIGHT                                     
kTMEventTypeInvisibility = 71, // @ C71_EVENT_INVISIBILITY                              
kTMEventTypeChampionShield = 72, // @ C72_EVENT_CHAMPION_SHIELD                           
kTMEventTypeThievesEye = 73, // @ C73_EVENT_THIEVES_EYE                               
kTMEventTypePartyShield = 74, // @ C74_EVENT_PARTY_SHIELD                              
kTMEventTypePoisonChampion = 75, // @ C75_EVENT_POISON_CHAMPION                           
kTMEventTypeSpellShield = 77, // @ C77_EVENT_SPELLSHIELD                               
kTMEventTypeFireShield = 78, // @ C78_EVENT_FIRESHIELD                                
kTMEventTypeFootprints = 79, // @ C79_EVENT_FOOTPRINTS                                
kTMEventTypeMagicMap_C80 = 80, // @ C80_EVENT_MAGIC_MAP                                 
kTMEventTypeMagicMap_C81 = 81, // @ C81_EVENT_MAGIC_MAP                                 
kTMEventTypeMagicMap_C82 = 82, // @ C82_EVENT_MAGIC_MAP                                 
kTMEventTypeMagicMap_C83 = 83  // @ C83_EVENT_MAGIC_MAP    
};

class TimelineEvent {
public:
	int32 _mapTime;
	byte _type;
	byte _priority;

	uint16 getTypePriority() { return (_type << 8) + _priority; }

	union B_unionTimelineEvent {
		struct {
			byte _mapX;
			byte _mapY;
		} _location;
		int16 _attack;
		int16 _defense;
		int16 _lightPower;
		Thing _slot;
		int16 _slotOrdinal;
		B_unionTimelineEvent() {}
	} _B;

	int16 getMapXY() { return (_B._location._mapX << 8) + _B._location._mapY; }

	union C_uionTimelineEvent {
		struct {
			byte _cell;
			byte _effect;
		} A;

		class {
			uint16 _backing;
		public:
			uint16 getMapX() { return _backing & 0x1F; }
			uint16 getMapY() { return (_backing >> 5) & 0x1F; }
			direction getDir() { return (direction)((_backing >> 10) & 0x3); }
			uint16 getStepEnergy() { return (_backing >> 12) & 0xF; }
			void setMapX(uint16 val) { _backing = (_backing & ~0x1F) | (val & 0x1F); }
			void setMapY(uint16 val) { _backing = (_backing & ~(0x1F << 5)) | ((val & 0x1F) << 5); }
			void setDir(direction val) { _backing = (_backing & ~(0x3 << 10)) | ((val & 0x3) << 10); }
			void setStepEnergy(uint16 val) { _backing = (_backing & ~(0xF << 12)) | ((val & 0xF) << 12); }
		} _projectile;

		Thing _slot;
		int16 _soundIndex;
		byte _ticks;
		C_uionTimelineEvent() {}
	} _C;
}; // @ EVENT

class Timeline {
	DMEngine *_vm;
public:
	uint16 _eventMaxCount; // @ G0369_ui_EventMaximumCount
	TimelineEvent *_events; // @ G0370_ps_Events
	uint16 _eventCount; // @ G0372_ui_EventCount 
	uint16 *_timeline; // @ G0371_pui_Timeline
	uint16 _firstUnusedEventIndex; // @ G0373_ui_FirstUnusedEventIndex

	Timeline(DMEngine *vm);
	~Timeline();
	void initTimeline(); // @ F0233_TIMELINE_Initialize
};


}

#endif
