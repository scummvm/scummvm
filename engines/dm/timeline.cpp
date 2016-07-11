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

#include "timeline.h"
#include "dungeonman.h"
#include "champion.h"
#include "inventory.h"
#include "group.h"
#include "projexpl.h"


namespace DM {

Timeline::Timeline(DMEngine* vm) : _vm(vm) {
	_g369_eventMaxCount = 0;
	_g370_events = nullptr;
	_g372_eventCount = 0;
	_g371_timeline = nullptr;
	_g373_firstUnusedEventIndex = 0;
}

Timeline::~Timeline() {
	delete[] _g370_events;
	delete[] _g371_timeline;
}

void Timeline::f233_initTimeline() {
	_g370_events = new TimelineEvent[_g369_eventMaxCount];
	_g371_timeline = new uint16[_g369_eventMaxCount];
	if (_vm->_g298_newGame) {
		for (int16 i = 0; i < _g369_eventMaxCount; ++i)
			_g370_events->_type = k0_TMEventTypeNone;
		_g372_eventCount = 0;
		_g373_firstUnusedEventIndex = 0;
	}
}

void Timeline::f237_deleteEvent(uint16 eventIndex) {
	uint16 L0586_ui_TimelineIndex;
	uint16 L0587_ui_EventCount;


	_vm->_timeline->_g370_events[eventIndex]._type = k0_TMEventTypeNone;
	if (eventIndex < _vm->_timeline->_g373_firstUnusedEventIndex) {
		_vm->_timeline->_g373_firstUnusedEventIndex = eventIndex;
	}
	_vm->_timeline->_g372_eventCount--;
	if ((L0587_ui_EventCount = _vm->_timeline->_g372_eventCount) == 0) {
		return;
	}
	L0586_ui_TimelineIndex = f235_getIndex(eventIndex);
	if (L0586_ui_TimelineIndex == L0587_ui_EventCount) {
		return;
	}
	_vm->_timeline->_g371_timeline[L0586_ui_TimelineIndex] = _vm->_timeline->_g371_timeline[L0587_ui_EventCount];
	f236_fixChronology(L0586_ui_TimelineIndex);
}

void Timeline::f236_fixChronology(uint16 timelineIndex) {
	uint16 L0581_ui_TimelineIndex;
	uint16 L0582_ui_EventIndex;
	uint16 L0583_ui_EventCount;
	TimelineEvent* L0584_ps_Event;
	bool L0585_B_ChronologyFixed;


	if ((L0583_ui_EventCount = _vm->_timeline->_g372_eventCount) == 1) {
		return;
	}

	L0584_ps_Event = &_vm->_timeline->_g370_events[L0582_ui_EventIndex = _vm->_timeline->_g371_timeline[timelineIndex]];
	L0585_B_ChronologyFixed = false;
	while (timelineIndex > 0) { /* Check if the event should be moved earlier in the timeline */
		L0581_ui_TimelineIndex = (timelineIndex - 1) >> 1;
		if (f234_isEventABeforeB(L0584_ps_Event, &_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex]])) {
			_vm->_timeline->_g371_timeline[timelineIndex] = _vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex];
			timelineIndex = L0581_ui_TimelineIndex;
			L0585_B_ChronologyFixed = true;
		} else {
			break;
		}
	}
	if (L0585_B_ChronologyFixed)
		goto T0236011;
	L0583_ui_EventCount = ((L0583_ui_EventCount - 1) - 1) >> 1;
	while (timelineIndex <= L0583_ui_EventCount) { /* Check if the event should be moved later in the timeline */
		L0581_ui_TimelineIndex = (timelineIndex << 1) + 1;
		if (((L0581_ui_TimelineIndex + 1) < _vm->_timeline->_g372_eventCount) && (f234_isEventABeforeB(&_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex + 1]], &_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex]]))) {
			L0581_ui_TimelineIndex++;
		}
		if (f234_isEventABeforeB(&_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex]], L0584_ps_Event)) {
			_vm->_timeline->_g371_timeline[timelineIndex] = _vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex];
			timelineIndex = L0581_ui_TimelineIndex;
		} else {
			break;
		}
	}
T0236011:
	_vm->_timeline->_g371_timeline[timelineIndex] = L0582_ui_EventIndex;
}

bool Timeline::f234_isEventABeforeB(TimelineEvent* eventA, TimelineEvent* eventB) {
	bool L0578_B_Simultaneous;

	return (M30_time(eventA->_mapTime) < M30_time(eventB->_mapTime)) ||
		((L0578_B_Simultaneous = (M30_time(eventA->_mapTime) == M30_time(eventB->_mapTime))) && (eventA->getTypePriority() > eventB->getTypePriority())) ||
		(L0578_B_Simultaneous && (eventA->getTypePriority() == eventB->getTypePriority()) && (eventA <= eventB));
}

uint16 Timeline::f235_getIndex(uint16 eventIndex) {
	uint16 L0579_ui_TimelineIndex;
	uint16* L0580_pui_TimelineEntry;


	for (L0579_ui_TimelineIndex = 0, L0580_pui_TimelineEntry = _vm->_timeline->_g371_timeline; L0579_ui_TimelineIndex < _vm->_timeline->_g369_eventMaxCount; L0579_ui_TimelineIndex++) {
		if (*L0580_pui_TimelineEntry++ == eventIndex)
			break;
	}
	if (L0579_ui_TimelineIndex >= _vm->_timeline->_g369_eventMaxCount) { /* BUG0_00 Useless code. The function is always called with event indices that are in the timeline */
		L0579_ui_TimelineIndex = 0; /* BUG0_01 Coding error without consequence. Wrong return value. If the specified event index is not found in the timeline the function returns 0 which is the same value that is returned if the event index is found in the first timeline entry. No consequence because this code is never executed */
	}
	return L0579_ui_TimelineIndex;
}

uint16 Timeline::f238_addEventGetEventIndex(TimelineEvent* event) {
	uint16 L0588_ui_EventIndex;
	uint16 L0590_ui_NewEventIndex;
	TimelineEvent* L0591_ps_Event;


	if (_vm->_timeline->_g372_eventCount == _vm->_timeline->_g369_eventMaxCount) {
		_vm->f19_displayErrorAndStop(45);
	}
	if ((event->_type >= k5_TMEventTypeCorridor) && (event->_type <= k10_TMEventTypeDoor)) {
		for (L0588_ui_EventIndex = 0, L0591_ps_Event = _vm->_timeline->_g370_events; L0588_ui_EventIndex < _vm->_timeline->_g369_eventMaxCount; L0588_ui_EventIndex++, L0591_ps_Event++) {
			if ((L0591_ps_Event->_type >= k5_TMEventTypeCorridor) && (L0591_ps_Event->_type <= k10_TMEventTypeDoor)) {
				if ((event->_mapTime == L0591_ps_Event->_mapTime) && (event->getMapXY() == L0591_ps_Event->getMapXY()) && ((L0591_ps_Event->_type != k6_TMEventTypeWall) || (L0591_ps_Event->_C.A._cell == event->_C.A._cell))) {
					L0591_ps_Event->_C.A._effect = event->_C.A._effect;
					return L0588_ui_EventIndex;
				}
				continue;
			} else {
				if ((L0591_ps_Event->_type == k1_TMEventTypeDoorAnimation) && (event->_mapTime == L0591_ps_Event->_mapTime) && (event->getMapXY() == L0591_ps_Event->getMapXY())) {
					if (event->_C.A._effect == k2_SensorEffToggle) {
						event->_C.A._effect = 1 - L0591_ps_Event->_C.A._effect;
					}
					f237_deleteEvent(L0588_ui_EventIndex);
					break;
				}
			}
		}
	} else {
		if (event->_type == k1_TMEventTypeDoorAnimation) {
			for (L0588_ui_EventIndex = 0, L0591_ps_Event = _vm->_timeline->_g370_events; L0588_ui_EventIndex < _vm->_timeline->_g369_eventMaxCount; L0588_ui_EventIndex++, L0591_ps_Event++) {
				if ((event->_mapTime == L0591_ps_Event->_mapTime) && (event->getMapXY() == L0591_ps_Event->getMapXY())) {
					if (L0591_ps_Event->_type == k10_TMEventTypeDoor) {
						if (L0591_ps_Event->_C.A._effect == k2_SensorEffToggle) {
							L0591_ps_Event->_C.A._effect = 1 - event->_C.A._effect;
						}
						return L0588_ui_EventIndex;
					}
					if (L0591_ps_Event->_type == k1_TMEventTypeDoorAnimation) {
						L0591_ps_Event->_C.A._effect = event->_C.A._effect;
						return L0588_ui_EventIndex;
					}
				}
			}
		} else {
			if (event->_type == k2_TMEventTypeDoorDestruction) {
				for (L0588_ui_EventIndex = 0, L0591_ps_Event = _vm->_timeline->_g370_events; L0588_ui_EventIndex < _vm->_timeline->_g369_eventMaxCount; L0588_ui_EventIndex++, L0591_ps_Event++) {
					if ((event->getMapXY() == L0591_ps_Event->getMapXY()) && (M29_map(event->_mapTime) == M29_map(L0591_ps_Event->_mapTime))) {
						if ((L0591_ps_Event->_type == k1_TMEventTypeDoorAnimation) || (L0591_ps_Event->_type == k10_TMEventTypeDoor)) {
							f237_deleteEvent(L0588_ui_EventIndex);
						}
					}
				}
			}
		}
	}
	_vm->_timeline->_g370_events[L0590_ui_NewEventIndex = _vm->_timeline->_g373_firstUnusedEventIndex] = *event; /* Copy the event data (Megamax C can assign structures) */
	do {
		if (_vm->_timeline->_g373_firstUnusedEventIndex == _vm->_timeline->_g369_eventMaxCount)
			break;
		_vm->_timeline->_g373_firstUnusedEventIndex++;
	} while ((_vm->_timeline->_g370_events[_vm->_timeline->_g373_firstUnusedEventIndex])._type != k0_TMEventTypeNone);
	_vm->_timeline->_g371_timeline[_vm->_timeline->_g372_eventCount] = L0590_ui_NewEventIndex;
	f236_fixChronology(_vm->_timeline->_g372_eventCount++);
	return L0590_ui_NewEventIndex;
}

void Timeline::f261_processTimeline() {
	uint16 L0680_ui_Multiple;
#define AL0680_ui_EventType     L0680_ui_Multiple
#define AL0680_ui_ChampionIndex L0680_ui_Multiple
	TimelineEvent* L0681_ps_Event;
	TimelineEvent L0682_s_Event;


	while (f240_isFirstEventExpiered()) {
		L0681_ps_Event = &L0682_s_Event;
		f239_timelineExtractFirstEvent(L0681_ps_Event);
		_vm->_dungeonMan->f173_setCurrentMap(M29_map(L0682_s_Event._mapTime));
		AL0680_ui_EventType = L0682_s_Event._type;
		if ((AL0680_ui_EventType > (k29_TMEventTypeGroupReactionDangerOnSquare - 1)) && (AL0680_ui_EventType < (k41_TMEventTypeUpdateBehaviour_3 + 1))) {
			_vm->_groupMan->f209_processEvents29to41(L0682_s_Event._B._location._mapX, L0682_s_Event._B._location._mapY, AL0680_ui_EventType, L0682_s_Event._C._ticks);
		} else {
			switch (AL0680_ui_EventType) {
			case k48_TMEventTypeMoveProjectileIgnoreImpacts:
			case k49_TMEventTypeMoveProjectile:
				_vm->_projexpl->f219_processEvents48To49_projectile(L0681_ps_Event);
				break;
			case k1_TMEventTypeDoorAnimation:
				f241_timelineProcessEvent1_doorAnimation(L0681_ps_Event);
				break;
			case k25_TMEventTypeExplosion:
				//F0220_EXPLOSION_ProcessEvent25_Explosion(L0681_ps_Event);
				break;
			case k7_TMEventTypeFakeWall:
				//F0242_TIMELINE_ProcessEvent7_Square_FakeWall(L0681_ps_Event);
				break;
			case k2_TMEventTypeDoorDestruction:
				//F0243_TIMELINE_ProcessEvent2_DoorDestruction(L0681_ps_Event);
				break;
			case k10_TMEventTypeDoor:
				//F0244_TIMELINE_ProcessEvent10_Square_Door(L0681_ps_Event);
				break;
			case k9_TMEventTypePit:
				//F0251_TIMELINE_ProcessEvent9_Square_Pit(L0681_ps_Event);
				break;
			case k8_TMEventTypeTeleporter:
				//F0250_TIMELINE_ProcessEvent8_Square_Teleporter(L0681_ps_Event);
				break;
			case k6_TMEventTypeWall:
				//F0248_TIMELINE_ProcessEvent6_Square_Wall(L0681_ps_Event);
				break;
			case k5_TMEventTypeCorridor:
				//F0245_TIMELINE_ProcessEvent5_Square_Corridor(L0681_ps_Event);
				break;
			case k60_TMEventTypeMoveGroupSilent:
			case k61_TMEventTypeMoveGroupAudible:
				//F0252_TIMELINE_ProcessEvents60to61_MoveGroup(L0681_ps_Event);
				break;
			case k65_TMEventTypeEnableGroupGenerator:
				//F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator(L0681_ps_Event);
				break;
			case k20_TMEventTypePlaySound:
				warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
				break;
			case k24_TMEventTypeRemoveFluxcage:
				if (!_vm->_g302_gameWon) {
					_vm->_dungeonMan->f164_unlinkThingFromList(Thing(L0682_s_Event._C._slot), Thing(0), L0682_s_Event._B._location._mapX, L0682_s_Event._B._location._mapY);
					L0681_ps_Event = (TimelineEvent*)_vm->_dungeonMan->f156_getThingData(Thing(L0682_s_Event._C._slot));
					((Explosion*)L0681_ps_Event)->setNextThing(Thing::_none);
				}
				break;
			case k11_TMEventTypeEnableChampionAction:
				//F0253_TIMELINE_ProcessEvent11Part1_EnableChampionAction(L0682_s_Event._priority);
				if (L0682_s_Event._B._slotOrdinal) {
					//F0259_TIMELINE_ProcessEvent11Part2_MoveWeaponFromQuiverToSlot(L0682_s_Event._priority, _vm->M1_ordinalToIndex(L0682_s_Event._B._slotOrdinal));
				}
				goto T0261048;
			case k12_TMEventTypeHideDamageReceived:
				//F0254_TIMELINE_ProcessEvent12_HideDamageReceived(L0682_s_Event._priority);
				break;
			case k70_TMEventTypeLight:
				_vm->_dungeonMan->f173_setCurrentMap(_vm->_dungeonMan->_g309_partyMapIndex);
				//F0257_TIMELINE_ProcessEvent70_Light(L0681_ps_Event);
				_vm->_inventoryMan->f337_setDungeonViewPalette();
				break;
			case k71_TMEventTypeInvisibility:
				_vm->_championMan->_g407_party._event71Count_Invisibility--;
				break;
			case k72_TMEventTypeChampionShield:
				_vm->_championMan->_gK71_champions[L0682_s_Event._priority]._shieldDefense -= L0682_s_Event._B._defense;
				setFlag(_vm->_championMan->_gK71_champions[L0682_s_Event._priority]._attributes, k0x1000_ChampionAttributeStatusBox);
T0261048:
				_vm->_championMan->f292_drawChampionState((ChampionIndex)L0682_s_Event._priority);
				break;
			case k73_TMEventTypeThievesEye:
				_vm->_championMan->_g407_party._event73Count_ThievesEye--;
				break;
			case k74_TMEventTypePartyShield:
				_vm->_championMan->_g407_party._shieldDefense -= L0682_s_Event._B._defense;
T0261053:
				//F0260_TIMELINE_RefreshAllChampionStatusBoxes();
				break;
			case k77_TMEventTypeSpellShield:
				_vm->_championMan->_g407_party._spellShieldDefense -= L0682_s_Event._B._defense;
				goto T0261053;
			case k78_TMEventTypeFireShield:
				_vm->_championMan->_g407_party._fireShieldDefense -= L0682_s_Event._B._defense;
				goto T0261053;
			case k75_TMEventTypePoisonChampion:
				_vm->_championMan->_gK71_champions[AL0680_ui_ChampionIndex = L0682_s_Event._priority]._poisonEventCount--;
				_vm->_championMan->f322_championPoison(AL0680_ui_ChampionIndex, L0682_s_Event._B._attack);
				break;
			case k13_TMEventTypeViAltarRebirth:
				//F0255_TIMELINE_ProcessEvent13_ViAltarRebirth(L0681_ps_Event);
				break;
			case k79_TMEventTypeFootprints:
				_vm->_championMan->_g407_party._event79Count_Footprints--;
			}
		}
		_vm->_dungeonMan->f173_setCurrentMap(_vm->_dungeonMan->_g309_partyMapIndex);
	}
}

bool Timeline::f240_isFirstEventExpiered() {
	warning(false, "possibly dangerous cast to int32");
	return (_vm->_timeline->_g372_eventCount && ((int32)M30_time(_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[0]]._mapTime) <= _vm->_g313_gameTime));
}

void Timeline::f239_timelineExtractFirstEvent(TimelineEvent* event) {
	uint16 L0592_ui_EventIndex;

	*event = _vm->_timeline->_g370_events[L0592_ui_EventIndex = _vm->_timeline->_g371_timeline[0]];
	f237_deleteEvent(L0592_ui_EventIndex);
}

void Timeline::f241_timelineProcessEvent1_doorAnimation(TimelineEvent* event) {
	uint16 L0593_ui_MapX;
	uint16 L0594_ui_MapY;
	int16 L0595_i_Effect;
	int16 L0596_i_DoorState;
	Square* L0597_puc_Square;
	Door* L0598_ps_Door;
	Thing L0599_T_GroupThing;
	uint16 L0600_ui_CreatureAttributes;
	uint16 L0602_ui_Multiple;
#define AL0602_ui_VerticalDoor L0602_ui_Multiple
#define AL0602_ui_Height       L0602_ui_Multiple

	L0597_puc_Square = (Square*)&_vm->_dungeonMan->_g271_currMapData[L0593_ui_MapX = event->_B._location._mapX][L0594_ui_MapY = event->_B._location._mapY];
	if ((L0596_i_DoorState = Square(*L0597_puc_Square).getDoorState()) == k5_doorState_DESTROYED) {
		return;
	}
	event->_mapTime++;
	L0595_i_Effect = event->_C.A._effect;
	if (L0595_i_Effect == k1_SensorEffClear) {
		L0598_ps_Door = (Door*)_vm->_dungeonMan->f157_getSquareFirstThingData(L0593_ui_MapX, L0594_ui_MapY);
		AL0602_ui_VerticalDoor = L0598_ps_Door->opensVertically();
		if ((_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) && (L0593_ui_MapX == _vm->_dungeonMan->_g306_partyMapX) && (L0594_ui_MapY == _vm->_dungeonMan->_g307_partyMapY) && (L0596_i_DoorState != k0_doorState_OPEN)) {
			if (_vm->_championMan->_g305_partyChampionCount > 0) {
				L0597_puc_Square->setDoorState(k0_doorState_OPEN);

				// Strangerke
				// Original bug fixed - A closing horizontal door wounds champions to the head instead of to the hands. Missing parenthesis in the condition cause all doors to wound the head in addition to the torso
				// See BUG0_78
				if (_vm->_championMan->f324_damageAll_getDamagedChampionCount(5, k0x0008_ChampionWoundTorso | (AL0602_ui_VerticalDoor ? k0x0004_ChampionWoundHead : k0x0001_ChampionWoundReadHand | k0x0002_ChampionWoundActionHand), k2_attackType_SELF)) {
					warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
				}
			}
			event->_mapTime++;
			_vm->_timeline->f238_addEventGetEventIndex(event);
			return;
		}
		if (((L0599_T_GroupThing = _vm->_groupMan->f175_groupGetThing(L0593_ui_MapX, L0594_ui_MapY)) != Thing::_endOfList) && !getFlag(L0600_ui_CreatureAttributes = _vm->_dungeonMan->f144_getCreatureAttributes(L0599_T_GroupThing), k0x0040_MaskCreatureInfo_nonMaterial)) {
			if (L0596_i_DoorState >= (AL0602_ui_Height ? CreatureInfo::M51_height(L0600_ui_CreatureAttributes) : 1)) { /* Creature height or 1 */
				if (_vm->_groupMan->f191_getDamageAllCreaturesOutcome((Group*)_vm->_dungeonMan->f156_getThingData(L0599_T_GroupThing), L0593_ui_MapX, L0594_ui_MapY, 5, true) != k2_outcomeKilledAllCreaturesInGroup) {
					_vm->_groupMan->f209_processEvents29to41(L0593_ui_MapX, L0594_ui_MapY, kM3_TMEventTypeCreateReactionEvent29DangerOnSquare, 0);
				}
				L0596_i_DoorState = (L0596_i_DoorState == k0_doorState_OPEN) ? k0_doorState_OPEN : (L0596_i_DoorState - 1);
				L0597_puc_Square->setDoorState(L0596_i_DoorState);
				warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
				event->_mapTime++;
				_vm->_timeline->f238_addEventGetEventIndex(event);
				return;
			}
		}
	}
	if (((L0595_i_Effect == k0_SensorEffSet) && (L0596_i_DoorState == k0_doorState_OPEN)) || ((L0595_i_Effect == k1_SensorEffClear) && (L0596_i_DoorState == k4_doorState_CLOSED))) {
		goto T0241020_Return;
	}
	L0596_i_DoorState += (L0595_i_Effect == k0_SensorEffSet) ? -1 : 1;
	L0597_puc_Square->setDoorState(L0596_i_DoorState);
	warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
	if (L0595_i_Effect == k0_SensorEffSet) {
		if (L0596_i_DoorState == k0_doorState_OPEN) {
			return;
		}
	} else {
		if (L0596_i_DoorState == k4_doorState_CLOSED) {
			return;
		}
	}
	_vm->_timeline->f238_addEventGetEventIndex(event);
T0241020_Return:
	;
}
}
