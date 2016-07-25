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
	class Champion;
	class Sensor;

	/* Event types */
enum TimelineEventType {
/* Used when a creature in a group was damaged or killed by a Poison Cloud or by a closing door or if Lord Chaos is surrounded by = 3, Fluxcages */
kM3_TMEventTypeCreateReactionEvent29DangerOnSquare = -3, // @ CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE 
/* Used when a projectile impacts with a creature in a group */
kM2_TMEventTypeCreateReactionEvent30HitByProjectile = -2, // @ CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE
/* Used when the party bumps into a group or performs a melee attack */
kM1_TMEventTypeCreateReactionEvent31ParyIsAdjacent = -1,  // @ CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT
k0_TMEventTypeNone = 0, // @ C00_EVENT_NONE                                       
k1_TMEventTypeDoorAnimation = 1, // @ C01_EVENT_DOOR_ANIMATION                             
k2_TMEventTypeDoorDestruction = 2, // @ C02_EVENT_DOOR_DESTRUCTION                           
k5_TMEventTypeCorridor = 5, // @ C05_EVENT_CORRIDOR                                   
k6_TMEventTypeWall = 6, // @ C06_EVENT_WALL                                       
k7_TMEventTypeFakeWall = 7, // @ C07_EVENT_FAKEWALL                                   
k8_TMEventTypeTeleporter = 8, // @ C08_EVENT_TELEPORTER                                 
k9_TMEventTypePit = 9, // @ C09_EVENT_PIT                                        
k10_TMEventTypeDoor = 10, // @ C10_EVENT_DOOR                                      
k11_TMEventTypeEnableChampionAction = 11, // @ C11_EVENT_ENABLE_CHAMPION_ACTION                    
k12_TMEventTypeHideDamageReceived = 12, // @ C12_EVENT_HIDE_DAMAGE_RECEIVED                      
k13_TMEventTypeViAltarRebirth = 13, // @ C13_EVENT_VI_ALTAR_REBIRTH                          
k20_TMEventTypePlaySound = 20, // @ C20_EVENT_PLAY_SOUND                                
k22_TMEventTypeCPSE = 22, // @ C22_EVENT_CPSE                                      
k24_TMEventTypeRemoveFluxcage = 24, // @ C24_EVENT_REMOVE_FLUXCAGE                           
k25_TMEventTypeExplosion = 25, // @ C25_EVENT_EXPLOSION                                 
k29_TMEventTypeGroupReactionDangerOnSquare = 29, // @ C29_EVENT_GROUP_REACTION_DANGER_ON_SQUARE           
k30_TMEventTypeGroupReacionHitByProjectile = 30, // @ C30_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE          
k31_TMEventTypeGroupReactionPartyIsAdjecent = 31, // @ C31_EVENT_GROUP_REACTION_PARTY_IS_ADJACENT          
k32_TMEventTypeUpdateAspectGroup = 32, // @ C32_EVENT_UPDATE_ASPECT_GROUP                       
/* Events = 33,-36 and = 38,-41 are used for individual creatures only while the group is attacking the party */
k33_TMEventTypeUpdateAspectCreature_0 = 33,  // @ C33_EVENT_UPDATE_ASPECT_CREATURE_0                  
k34_TMEventTypeUpdateAspectCreature_1 = 34, // @ C34_EVENT_UPDATE_ASPECT_CREATURE_1                  
k35_TMEventTypeUpdateAspectCreature_2 = 35, // @ C35_EVENT_UPDATE_ASPECT_CREATURE_2                  
k36_TMEventTypeUpdateAspectCreature_3 = 36, // @ C36_EVENT_UPDATE_ASPECT_CREATURE_3                  
k37_TMEventTypeUpdateBehaviourGroup = 37, // @ C37_EVENT_UPDATE_BEHAVIOR_GROUP                     
k38_TMEventTypeUpdateBehaviour_0 = 38, // @ C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0                
k39_TMEventTypeUpdateBehaviour_1 = 39, // @ C39_EVENT_UPDATE_BEHAVIOR_CREATURE_1                
k40_TMEventTypeUpdateBehaviour_2 = 40, // @ C40_EVENT_UPDATE_BEHAVIOR_CREATURE_2                
k41_TMEventTypeUpdateBehaviour_3 = 41, // @ C41_EVENT_UPDATE_BEHAVIOR_CREATURE_3                
/* Projectiles created by a champion (by casting a spell, shooting a weapon or throwing an object) or by a creature (by casting a spell) ignore impacts during their first movement otherwise an impact would always occur immediately as these projectiles are created on the champion or creature square */
k48_TMEventTypeMoveProjectileIgnoreImpacts = 48, // @ C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS            
/* Projectiles created by projectile launcher sensors never ignore impacts as well as all other projectiles after their first movement */
k49_TMEventTypeMoveProjectile = 49,  // @ C49_EVENT_MOVE_PROJECTILE                           
k53_TMEventTypeWatchdoge = 53, // @ C53_EVENT_WATCHDOG                                  
k60_TMEventTypeMoveGroupSilent = 60, // @ C60_EVENT_MOVE_GROUP_SILENT                         
k61_TMEventTypeMoveGroupAudible = 61, // @ C61_EVENT_MOVE_GROUP_AUDIBLE                        
k65_TMEventTypeEnableGroupGenerator = 65, // @ C65_EVENT_ENABLE_GROUP_GENERATOR                    
k70_TMEventTypeLight = 70, // @ C70_EVENT_LIGHT                                     
k71_TMEventTypeInvisibility = 71, // @ C71_EVENT_INVISIBILITY                              
k72_TMEventTypeChampionShield = 72, // @ C72_EVENT_CHAMPION_SHIELD                           
k73_TMEventTypeThievesEye = 73, // @ C73_EVENT_THIEVES_EYE                               
k74_TMEventTypePartyShield = 74, // @ C74_EVENT_PARTY_SHIELD                              
k75_TMEventTypePoisonChampion = 75, // @ C75_EVENT_POISON_CHAMPION                           
k77_TMEventTypeSpellShield = 77, // @ C77_EVENT_SPELLSHIELD                               
k78_TMEventTypeFireShield = 78, // @ C78_EVENT_FIRESHIELD                                
k79_TMEventTypeFootprints = 79, // @ C79_EVENT_FOOTPRINTS                                
k80_TMEventTypeMagicMap_C80 = 80, // @ C80_EVENT_MAGIC_MAP                                 
k81_TMEventTypeMagicMap_C81 = 81, // @ C81_EVENT_MAGIC_MAP                                 
k82_TMEventTypeMagicMap_C82 = 82, // @ C82_EVENT_MAGIC_MAP                                 
k83_TMEventTypeMagicMap_C83 = 83  // @ C83_EVENT_MAGIC_MAP    
};

extern signed char g495_actionDefense[44]; // @ G0495_ac_Graphic560_ActionDefense

class TimelineEvent {
public:
	int32 _mapTime;
	byte _type;
	byte _priority; // CHECKME: byte? or int16? Inconsistency in the code

	uint16 getTypePriority() { return (_type << 8) + _priority; }

	union B_unionTimelineEvent {
		struct {
			byte _mapX;
			byte _mapY;
		} _location;
		int16 _attack;
		int16 _defense;
		int16 _lightPower;
		uint16 _slot; // Thing
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

		uint16 _slot;
		int16 _soundIndex;
		byte _ticks;
		C_uionTimelineEvent() {}
	} _C;
}; // @ EVENT

class Timeline {
	DMEngine *_vm;
public:
	uint16 _g369_eventMaxCount; // @ G0369_ui_EventMaximumCount
	TimelineEvent *_g370_events; // @ G0370_ps_Events
	uint16 _g372_eventCount; // @ G0372_ui_EventCount 
	uint16 *_g371_timeline; // @ G0371_pui_Timeline
	uint16 _g373_firstUnusedEventIndex; // @ G0373_ui_FirstUnusedEventIndex

	explicit Timeline(DMEngine *vm);
	~Timeline();
	void f233_initTimeline(); // @ F0233_TIMELINE_Initialize
	void f237_deleteEvent(uint16 eventIndex);// @ F0237_TIMELINE_DeleteEvent
	void f236_fixChronology(uint16 timelineIndex); // @ F0236_TIMELINE_FixChronology
	bool f234_isEventABeforeB(TimelineEvent *eventA, TimelineEvent *eventB); // @ F0234_TIMELINE_IsEventABeforeEventB
	uint16 f235_getIndex(uint16 eventIndex); // @ F0235_TIMELINE_GetIndex
	uint16 f238_addEventGetEventIndex(TimelineEvent *event); // @ F0238_TIMELINE_AddEvent_GetEventIndex_CPSE
	void f261_processTimeline(); // @ F0261_TIMELINE_Process_CPSEF
	bool f240_isFirstEventExpiered(); // @ F0240_TIMELINE_IsFirstEventExpired_CPSE
	void f239_timelineExtractFirstEvent(TimelineEvent *event); // @ F0239_TIMELINE_ExtractFirstEvent
	void f241_timelineProcessEvent1_doorAnimation(TimelineEvent *event); // @ F0241_TIMELINE_ProcessEvent1_DoorAnimation
	void f242_timelineProcessEvent7_squareFakewall(TimelineEvent *event); // @ F0242_TIMELINE_ProcessEvent7_Square_FakeWall
	void f243_timelineProcessEvent2_doorDestruction(TimelineEvent *event); // @ F0243_TIMELINE_ProcessEvent2_DoorDestruction
	void f244_timelineProcessEvent10_squareDoor(TimelineEvent *event); // @ F0244_TIMELINE_ProcessEvent10_Square_Door
	void f251_timelineProcessEvent9_squarePit(TimelineEvent *event); // @ F0251_TIMELINE_ProcessEvent9_Square_Pit
	void f249_moveTeleporterOrPitSquareThings(uint16 mapX, uint16 mapY); // @ F0249_TIMELINE_MoveTeleporterOrPitSquareThings
	void f250_timelineProcessEvent8_squareTeleporter(TimelineEvent *event); // @ F0250_TIMELINE_ProcessEvent8_Square_Teleporter
	void f248_timelineProcessEvent6_squareWall(TimelineEvent *event); // @ F0248_TIMELINE_ProcessEvent6_Square_Wall
	void f247_triggerProjectileLauncher(Sensor *sensor, TimelineEvent *event); // @ F0247_TIMELINE_TriggerProjectileLauncher
	void f245_timlineProcessEvent5_squareCorridor(TimelineEvent *event); // @ F0245_TIMELINE_ProcessEvent5_Square_Corridor
	void f252_timelineProcessEvents60to61_moveGroup(TimelineEvent *event); // @ F0252_TIMELINE_ProcessEvents60to61_MoveGroup
	void f246_timelineProcesEvent65_enableGroupGenerator(TimelineEvent *event); // @ F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator
	void f253_timelineProcessEvent11Part1_enableChampionAction(uint16 champIndex); // @ F0253_TIMELINE_ProcessEvent11Part1_EnableChampionAction
	void f259_timelineProcessEvent11Part2_moveWeaponFromQuiverToSlot(uint16 champIndex,
																	 uint16 slotIndex);// @ F0259_TIMELINE_ProcessEvent11Part2_MoveWeaponFromQuiverToSlot
	bool f258_timelineHasWeaponMovedSlot(int16 champIndex, Champion *champ,
										 uint16 sourceSlotIndex, int16 destSlotIndex); // @ F0258_TIMELINE_HasWeaponMovedToSlot
	void f254_timelineProcessEvent12_hideDamageReceived(uint16 champIndex); // @ F0254_TIMELINE_ProcessEvent12_HideDamageReceived
	void f257_timelineProcessEvent70_light(TimelineEvent *event); // @ F0257_TIMELINE_ProcessEvent70_Light
	void f260_timelineRefreshAllChampionStatusBoxes(); // @ F0260_TIMELINE_RefreshAllChampionStatusBoxes
	void f255_timelineProcessEvent13_ViAltarRebirth(TimelineEvent *event); // @ F0255_TIMELINE_ProcessEvent13_ViAltarRebirth
	void save3_eventsPart(Common::OutSaveFile *file);
	void save4_timelinePart(Common::OutSaveFile *file);
	void load3_eventsPart(Common::InSaveFile* file);
	void load4_timelinePart(Common::InSaveFile* file);
};


}

#endif
