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
#include "movesens.h"
#include "text.h"


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
				_vm->_projexpl->f220_explosionProcessEvent25_explosion(L0681_ps_Event);
				break;
			case k7_TMEventTypeFakeWall:
				f242_timelineProcessEvent7_squareFakewall(L0681_ps_Event);
				break;
			case k2_TMEventTypeDoorDestruction:
				f243_timelineProcessEvent2_doorDestruction(L0681_ps_Event);
				break;
			case k10_TMEventTypeDoor:
				f244_timelineProcessEvent10_squareDoor(L0681_ps_Event);
				break;
			case k9_TMEventTypePit:
				f251_timelineProcessEvent9_squarePit(L0681_ps_Event);
				break;
			case k8_TMEventTypeTeleporter:
				f250_timelineProcessEvent8_squareTeleporter(L0681_ps_Event);
				break;
			case k6_TMEventTypeWall:
				f248_timelineProcessEvent6_squareWall(L0681_ps_Event);
				break;
			case k5_TMEventTypeCorridor:
				f245_timlineProcessEvent5_squareCorridor(L0681_ps_Event);
				break;
			case k60_TMEventTypeMoveGroupSilent:
			case k61_TMEventTypeMoveGroupAudible:
				f252_timelineProcessEvents60to61_moveGroup(L0681_ps_Event);
				break;
			case k65_TMEventTypeEnableGroupGenerator:
				f246_timelineProcesEvent65_enableGroupGenerator(L0681_ps_Event);
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
	warning(false, "possibly dangerous cast to uint32");
	return (_vm->_timeline->_g372_eventCount && ((uint32)M30_time(_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[0]]._mapTime) <= _vm->_g313_gameTime));
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

void Timeline::f242_timelineProcessEvent7_squareFakewall(TimelineEvent* event) {
	uint16 L0603_ui_MapX;
	uint16 L0604_ui_MapY;
	int16 L0605_i_Effect;
	Thing L0606_T_Thing;
	byte* L0607_puc_Square;


	L0607_puc_Square = &_vm->_dungeonMan->_g271_currMapData[L0603_ui_MapX = event->_B._location._mapX][L0604_ui_MapY = event->_B._location._mapY];
	L0605_i_Effect = event->_C.A._effect;
	if (L0605_i_Effect == k2_SensorEffToggle) {
		L0605_i_Effect = getFlag(*L0607_puc_Square, k0x0004_FakeWallOpen) ? k1_SensorEffClear : k0_SensorEffSet;
	}
	if (L0605_i_Effect == k1_SensorEffClear) {
		if ((_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) && (L0603_ui_MapX == _vm->_dungeonMan->_g306_partyMapX) && (L0604_ui_MapY == _vm->_dungeonMan->_g307_partyMapY)) {
			event->_mapTime++;
			_vm->_timeline->f238_addEventGetEventIndex(event);
		} else {
			if (((L0606_T_Thing = _vm->_groupMan->f175_groupGetThing(L0603_ui_MapX, L0604_ui_MapY)) != Thing::_endOfList) && !getFlag(_vm->_dungeonMan->f144_getCreatureAttributes(L0606_T_Thing), k0x0040_MaskCreatureInfo_nonMaterial)) {
				event->_mapTime++;
				_vm->_timeline->f238_addEventGetEventIndex(event);
			} else {
				clearFlag(*L0607_puc_Square, k0x0004_FakeWallOpen);
			}
		}
	} else {
		setFlag(*L0607_puc_Square, k0x0004_FakeWallOpen);
	}
}

void Timeline::f243_timelineProcessEvent2_doorDestruction(TimelineEvent* event) {
	Square* L0608_puc_Square;

	L0608_puc_Square = (Square*)&_vm->_dungeonMan->_g271_currMapData[event->_B._location._mapX][event->_B._location._mapY];
	L0608_puc_Square->setDoorState(k5_doorState_DESTROYED);
}

void Timeline::f244_timelineProcessEvent10_squareDoor(TimelineEvent* event) {
	int16 L0609_i_DoorState;


	if ((L0609_i_DoorState = Square(_vm->_dungeonMan->_g271_currMapData[event->_B._location._mapX][event->_B._location._mapY]).getDoorState()) == k5_doorState_DESTROYED) {
		return;
	}
	if (event->_C.A._effect == k2_SensorEffToggle) {
		event->_C.A._effect = (L0609_i_DoorState == k0_doorState_OPEN) ? k1_SensorEffClear : k0_SensorEffSet;
	} else {
		if (event->_C.A._effect == k0_SensorEffSet) {
			if (L0609_i_DoorState == k0_doorState_OPEN) {
				return;
			}
		} else {
			if (L0609_i_DoorState == k4_doorState_CLOSED) {
				return;
			}
		}
	}
	event->_type = k1_TMEventTypeDoorAnimation;
	_vm->_timeline->f238_addEventGetEventIndex(event);
}

void Timeline::f251_timelineProcessEvent9_squarePit(TimelineEvent* event) {
	uint16 L0653_ui_MapX;
	uint16 L0654_ui_MapY;
	byte* L0655_puc_Square;


	L0655_puc_Square = &_vm->_dungeonMan->_g271_currMapData[L0653_ui_MapX = event->_B._location._mapX][L0654_ui_MapY = event->_B._location._mapY];
	if (event->_C.A._effect == k2_SensorEffToggle) {
		event->_C.A._effect = getFlag(*L0655_puc_Square, k0x0008_PitOpen) ? k1_SensorEffClear : k0_SensorEffSet;
	}
	if (event->_C.A._effect == k0_SensorEffSet) {
		setFlag(*L0655_puc_Square, k0x0008_PitOpen);
		f249_moveTeleporterOrPitSquareThings(L0653_ui_MapX, L0654_ui_MapY);
	} else {
		clearFlag(*L0655_puc_Square, k0x0008_PitOpen);
	}
}

void Timeline::f249_moveTeleporterOrPitSquareThings(uint16 mapX, uint16 mapY) {
	uint16 L0644_ui_Multiple;
#define AL0644_ui_ThingType  L0644_ui_Multiple
#define AL0644_ui_EventIndex L0644_ui_Multiple
	Thing L0645_T_Thing;
	Projectile* L0646_ps_Projectile;
	TimelineEvent* L0647_ps_Event;
	Thing L0648_T_NextThing;
	int16 L0649_i_ThingsToMoveCount;


	if ((_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) && (mapX == _vm->_dungeonMan->_g306_partyMapX) && (mapY == _vm->_dungeonMan->_g307_partyMapY)) {
		_vm->_movsens->f267_getMoveResult(Thing::_party, mapX, mapY, mapX, mapY);
		_vm->_championMan->f296_drawChangedObjectIcons();
	}
	if ((L0645_T_Thing = _vm->_groupMan->f175_groupGetThing(mapX, mapY)) != Thing::_endOfList) {
		_vm->_movsens->f267_getMoveResult(L0645_T_Thing, mapX, mapY, mapX, mapY);
	}
	L0645_T_Thing = _vm->_dungeonMan->f162_getSquareFirstObject(mapX, mapY);
	L0648_T_NextThing = L0645_T_Thing;
	L0649_i_ThingsToMoveCount = 0;
	while (L0645_T_Thing != Thing::_endOfList) {
		if (L0645_T_Thing.getType() > k4_GroupThingType) {
			L0649_i_ThingsToMoveCount++;
		}
		L0645_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0645_T_Thing);
	}
	L0645_T_Thing = L0648_T_NextThing;
	while ((L0645_T_Thing != Thing::_endOfList) && L0649_i_ThingsToMoveCount) {
		L0649_i_ThingsToMoveCount--;
		L0648_T_NextThing = _vm->_dungeonMan->f159_getNextThing(L0645_T_Thing);
		AL0644_ui_ThingType = L0645_T_Thing.getType();
		if (AL0644_ui_ThingType > k4_GroupThingType) {
			_vm->_movsens->f267_getMoveResult(L0645_T_Thing, mapX, mapY, mapX, mapY);
		}
		if (AL0644_ui_ThingType == k14_ProjectileThingType) {
			L0646_ps_Projectile = (Projectile*)_vm->_dungeonMan->f156_getThingData(L0645_T_Thing);
			L0647_ps_Event = &_vm->_timeline->_g370_events[L0646_ps_Projectile->_eventIndex];
			L0647_ps_Event->_C._projectile.setMapX(_vm->_movsens->_g397_moveResultMapX);
			L0647_ps_Event->_C._projectile.setMapY(_vm->_movsens->_g398_moveResultMapY);
			L0647_ps_Event->_C._projectile.setDir((direction)_vm->_movsens->_g400_moveResultDir);
			L0647_ps_Event->_B._slot = M15_thingWithNewCell(L0645_T_Thing, _vm->_movsens->_g401_moveResultCell).toUint16();
			M31_setMap(L0647_ps_Event->_mapTime, _vm->_movsens->_g399_moveResultMapIndex);
		} else {
			if (AL0644_ui_ThingType == k15_ExplosionThingType) {
				for (AL0644_ui_EventIndex = 0, L0647_ps_Event = _vm->_timeline->_g370_events; AL0644_ui_EventIndex < _vm->_timeline->_g369_eventMaxCount; L0647_ps_Event++, AL0644_ui_EventIndex++) {
					if ((L0647_ps_Event->_type == k25_TMEventTypeExplosion) && (L0647_ps_Event->_C._slot == L0645_T_Thing.toUint16())) { /* BUG0_23 A Fluxcage explosion remains on a square forever. If you open a pit or teleporter on a square where there is a Fluxcage explosion, the Fluxcage explosion is moved but the associated event is not updated (because Fluxcage explosions do not use k25_TMEventTypeExplosion but rather k24_TMEventTypeRemoveFluxcage) causing the Fluxcage explosion to remain in the dungeon forever on its destination square. When the k24_TMEventTypeRemoveFluxcage expires the explosion thing is not removed, but it is marked as unused. Consequently, any objects placed on the Fluxcage square after it was moved but before it expires become orphans upon expiration. After expiration, any object placed on the fluxcage square is cloned when picked up */
						L0647_ps_Event->_B._location._mapX = _vm->_movsens->_g397_moveResultMapX;
						L0647_ps_Event->_B._location._mapY = _vm->_movsens->_g398_moveResultMapY;
						L0647_ps_Event->_C._slot = M15_thingWithNewCell(L0645_T_Thing, _vm->_movsens->_g401_moveResultCell).toUint16();
						M31_setMap(L0647_ps_Event->_mapTime, _vm->_movsens->_g399_moveResultMapIndex);
					}
				}
			}
		}
		L0645_T_Thing = L0648_T_NextThing;
	}
}

void Timeline::f250_timelineProcessEvent8_squareTeleporter(TimelineEvent* event) {
	uint16 L0650_ui_MapX;
	uint16 L0651_ui_MapY;
	byte* L0652_puc_Square;


	L0652_puc_Square = &_vm->_dungeonMan->_g271_currMapData[L0650_ui_MapX = event->_B._location._mapX][L0651_ui_MapY = event->_B._location._mapY];
	if (event->_C.A._effect == k2_SensorEffToggle) {
		event->_C.A._effect = getFlag(*L0652_puc_Square, k0x0008_TeleporterOpen) ? k1_SensorEffClear : k0_SensorEffSet;
	}
	if (event->_C.A._effect == k0_SensorEffSet) {
		setFlag(*L0652_puc_Square, k0x0008_TeleporterOpen);
		f249_moveTeleporterOrPitSquareThings(L0650_ui_MapX, L0651_ui_MapY);
	} else {
		clearFlag(*L0652_puc_Square, k0x0008_TeleporterOpen);
	}
}

void Timeline::f248_timelineProcessEvent6_squareWall(TimelineEvent* event) {
	Thing L0634_T_Thing;
	int16 L0635_i_ThingType;
	int16 L0636_i_Multiple;
#define AL0636_B_TriggerSetEffect L0636_i_Multiple
#define AL0636_i_BitMask          L0636_i_Multiple
	uint16 L0637_ui_SensorData;
	Sensor* L0638_ps_Sensor;
	TextString* L0639_ps_TextString;
	uint16 L0640_ui_SensorType;
	int16 L0641_i_MapX;
	int16 L0642_i_MapY;
	uint16 L0643_ui_Cell;


	L0634_T_Thing = _vm->_dungeonMan->f161_getSquareFirstThing(L0641_i_MapX = event->_B._location._mapX, L0642_i_MapY = event->_B._location._mapY);
	L0643_ui_Cell = event->_C.A._cell;
	while (L0634_T_Thing != Thing::_endOfList) {
		if (((L0635_i_ThingType = L0634_T_Thing.getType()) == k2_TextstringType) && (L0634_T_Thing.getCell() == event->_C.A._cell)) {
			L0639_ps_TextString = (TextString*)_vm->_dungeonMan->f156_getThingData(L0634_T_Thing);
			if (event->_C.A._effect == k2_SensorEffToggle) {
				L0639_ps_TextString->setVisible(!L0639_ps_TextString->isVisible());
			} else {
				L0639_ps_TextString->setVisible(event->_C.A._effect == k0_SensorEffSet);
			}
		} else {
			if (L0635_i_ThingType == k3_SensorThingType) {
				L0638_ps_Sensor = (Sensor*)_vm->_dungeonMan->f156_getThingData(L0634_T_Thing);
				L0640_ui_SensorType = L0638_ps_Sensor->getType();
				L0637_ui_SensorData = L0638_ps_Sensor->getData();
				if (L0640_ui_SensorType == k6_SensorWallCountdown) {
					if (L0637_ui_SensorData > 0) {
						if (event->_C.A._effect == k0_SensorEffSet) {
							if (L0637_ui_SensorData < 511) {
								L0637_ui_SensorData++;
							}
						} else {
							L0637_ui_SensorData--;
						}
						L0638_ps_Sensor->setData(L0637_ui_SensorData);
						if (L0638_ps_Sensor->getEffectA() == k3_SensorEffHold) {
							AL0636_B_TriggerSetEffect = ((L0637_ui_SensorData == 0) != L0638_ps_Sensor->getRevertEffectA());
							_vm->_movsens->f272_sensorTriggerEffect(L0638_ps_Sensor, AL0636_B_TriggerSetEffect ? k0_SensorEffSet : k1_SensorEffClear, L0641_i_MapX, L0642_i_MapY, L0643_ui_Cell);
						} else {
							if (L0637_ui_SensorData == 0) {
								_vm->_movsens->f272_sensorTriggerEffect(L0638_ps_Sensor, L0638_ps_Sensor->getEffectA(), L0641_i_MapX, L0642_i_MapY, L0643_ui_Cell);
							}
						}
					}
				} else {
					if (L0640_ui_SensorType == k5_SensorWallAndOrGate) {
						AL0636_i_BitMask = 1 << (event->_C.A._cell);
						if (event->_C.A._effect == k2_SensorEffToggle) {
							if (getFlag(L0637_ui_SensorData, AL0636_i_BitMask)) {
								clearFlag(L0637_ui_SensorData, AL0636_i_BitMask);
							} else {
								setFlag(L0637_ui_SensorData, AL0636_i_BitMask);
							}
						} else {
							if (event->_C.A._effect) {
								clearFlag(L0637_ui_SensorData, AL0636_i_BitMask);
							} else {
								setFlag(L0637_ui_SensorData, AL0636_i_BitMask);
							}
						}
						L0638_ps_Sensor->setData(L0637_ui_SensorData);
						AL0636_B_TriggerSetEffect = (Sensor::getDataMask1(L0637_ui_SensorData) == Sensor::getDataMask2(L0637_ui_SensorData)) != L0638_ps_Sensor->getRevertEffectA();
						if (L0638_ps_Sensor->getEffectA() == k3_SensorEffHold) {
							_vm->_movsens->f272_sensorTriggerEffect(L0638_ps_Sensor, AL0636_B_TriggerSetEffect ? k0_SensorEffSet : k1_SensorEffClear, L0641_i_MapX, L0642_i_MapY, L0643_ui_Cell);
						} else {
							if (AL0636_B_TriggerSetEffect) {
								_vm->_movsens->f272_sensorTriggerEffect(L0638_ps_Sensor, L0638_ps_Sensor->getEffectA(), L0641_i_MapX, L0642_i_MapY, L0643_ui_Cell);
							}
						}
					} else {
						if ((((L0640_ui_SensorType >= k7_SensorWallSingleProjLauncherNewObj) && (L0640_ui_SensorType <= k10_SensorWallDoubleProjLauncherExplosion)) || (L0640_ui_SensorType == k14_SensorWallSingleProjLauncherSquareObj) || (L0640_ui_SensorType == k15_SensorWallDoubleProjLauncherSquareObj)) && (L0634_T_Thing.getCell() == event->_C.A._cell)) {
							f247_triggerProjectileLauncher(L0638_ps_Sensor, event);
							if (L0638_ps_Sensor->getOnlyOnce()) {
								L0638_ps_Sensor->setTypeDisabled();
							}
						} else {
							if (L0640_ui_SensorType == k18_SensorWallEndGame) {
								_vm->f22_delay(60 * L0638_ps_Sensor->getValue());
								_vm->_g524_restartGameAllowed = false;
								_vm->_g302_gameWon = true;
								warning(false, "MISSING CODE: F0444_STARTEND_Endgame");
							}
						}
					}
				}
			}
		}
		L0634_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0634_T_Thing);
	}
	_vm->_movsens->f271_processRotationEffect();
}

void Timeline::f247_triggerProjectileLauncher(Sensor* sensor, TimelineEvent* event) {
	Thing L0622_T_FirstProjectileAssociatedThing;
	Thing L0623_T_SecondProjectileAssociatedThing;
	uint16 L0624_ui_Cell;
	int16 L0625_i_SensorType;
	int16 L0626_i_MapX;
	int16 L0627_i_MapY;
	uint16 L0628_ui_ProjectileCell;
	int16 L0629_i_SensorData;
	int16 L0630_i_KineticEnergy;
	int16 L0631_i_StepEnergy;
	bool L0632_B_LaunchSingleProjectile;
	uint16 L0633_ui_ThingCell;


	L0626_i_MapX = event->_B._location._mapX;
	L0627_i_MapY = event->_B._location._mapY;
	L0624_ui_Cell = event->_C.A._cell;
	L0628_ui_ProjectileCell = returnOppositeDir((direction)L0624_ui_Cell);
	L0625_i_SensorType = sensor->getType();
	L0629_i_SensorData = sensor->getData();
	L0630_i_KineticEnergy = sensor->M47_kineticEnergy();
	L0631_i_StepEnergy = sensor->M48_stepEnergy();
	L0632_B_LaunchSingleProjectile = (L0625_i_SensorType == k7_SensorWallSingleProjLauncherNewObj) ||
		(L0625_i_SensorType == k8_SensorWallSingleProjLauncherExplosion) ||
		(L0625_i_SensorType == k14_SensorWallSingleProjLauncherSquareObj);
	if ((L0625_i_SensorType == k8_SensorWallSingleProjLauncherExplosion) || (L0625_i_SensorType == k10_SensorWallDoubleProjLauncherExplosion)) {
		L0622_T_FirstProjectileAssociatedThing = L0623_T_SecondProjectileAssociatedThing = Thing(L0629_i_SensorData + Thing::_firstExplosion.toUint16());
	} else {
		if ((L0625_i_SensorType == k14_SensorWallSingleProjLauncherSquareObj) || (L0625_i_SensorType == k15_SensorWallDoubleProjLauncherSquareObj)) {
			L0622_T_FirstProjectileAssociatedThing = _vm->_dungeonMan->f161_getSquareFirstThing(L0626_i_MapX, L0627_i_MapY);
			while (L0622_T_FirstProjectileAssociatedThing != Thing::_none) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. Thing::_none should be Thing::_endOfList. If there are no more objects on the square then this loop may return an undefined value, this can crash the game. In the original DM and CSB dungeons, the number of times that these sensors are triggered is always controlled to be equal to the number of available objects (with a countdown sensor or a number of once only sensors) */
				L0633_ui_ThingCell = L0622_T_FirstProjectileAssociatedThing.getCell();
				if ((L0622_T_FirstProjectileAssociatedThing.getType() > k3_SensorThingType) && ((L0633_ui_ThingCell == L0624_ui_Cell) || (L0633_ui_ThingCell == returnNextVal(L0624_ui_Cell))))
					break;
				L0622_T_FirstProjectileAssociatedThing = _vm->_dungeonMan->f159_getNextThing(L0622_T_FirstProjectileAssociatedThing);
			}
			if (L0622_T_FirstProjectileAssociatedThing == Thing::_none) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. Thing::_none should be Thing::_endOfList */
				return;
			}
			_vm->_dungeonMan->f164_unlinkThingFromList(L0622_T_FirstProjectileAssociatedThing, Thing(0), L0626_i_MapX, L0627_i_MapY); /* The object is removed without triggering any sensor effects */
			if (!L0632_B_LaunchSingleProjectile) {
				L0623_T_SecondProjectileAssociatedThing = _vm->_dungeonMan->f161_getSquareFirstThing(L0626_i_MapX, L0627_i_MapY);
				while (L0623_T_SecondProjectileAssociatedThing != Thing::_none) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. Thing::_none should be Thing::_endOfList. If there are no more objects on the square then this loop may return an undefined value, this can crash the game */
					L0633_ui_ThingCell = L0623_T_SecondProjectileAssociatedThing.getCell();
					if ((L0623_T_SecondProjectileAssociatedThing.getType() > k3_SensorThingType) && ((L0633_ui_ThingCell == L0624_ui_Cell) || (L0633_ui_ThingCell == returnNextVal(L0624_ui_Cell))))
						break;
					L0623_T_SecondProjectileAssociatedThing = _vm->_dungeonMan->f159_getNextThing(L0623_T_SecondProjectileAssociatedThing);
				}
				if (L0623_T_SecondProjectileAssociatedThing == Thing::_none) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. Thing::_none should be Thing::_endOfList */
					L0632_B_LaunchSingleProjectile = true;
				} else {
					_vm->_dungeonMan->f164_unlinkThingFromList(L0623_T_SecondProjectileAssociatedThing, Thing(0), L0626_i_MapX, L0627_i_MapY); /* The object is removed without triggering any sensor effects */
				}
			}
		} else {
			if ((L0622_T_FirstProjectileAssociatedThing = _vm->_dungeonMan->f167_getObjForProjectileLaucherOrObjGen(L0629_i_SensorData)) == Thing::_none) {
				return;
			}
			if (!L0632_B_LaunchSingleProjectile && ((L0623_T_SecondProjectileAssociatedThing = _vm->_dungeonMan->f167_getObjForProjectileLaucherOrObjGen(L0629_i_SensorData)) == Thing::_none)) {
				L0632_B_LaunchSingleProjectile = true;
			}
		}
	}
	if (L0632_B_LaunchSingleProjectile) {
		L0628_ui_ProjectileCell = M21_normalizeModulo4(L0628_ui_ProjectileCell + _vm->getRandomNumber(2));
	}
	L0626_i_MapX += _vm->_dirIntoStepCountEast[L0624_ui_Cell], L0627_i_MapY += _vm->_dirIntoStepCountNorth[L0624_ui_Cell]; /* BUG0_20 The game crashes if the launcher sensor is on a map boundary and shoots in a direction outside the map */
	_vm->_projexpl->_g365_createLanucherProjectile = true;
	_vm->_projexpl->f212_projectileCreate(L0622_T_FirstProjectileAssociatedThing, L0626_i_MapX, L0627_i_MapY, L0628_ui_ProjectileCell, (direction)L0624_ui_Cell, L0630_i_KineticEnergy, 100, L0631_i_StepEnergy);
	if (!L0632_B_LaunchSingleProjectile) {
		_vm->_projexpl->f212_projectileCreate(L0623_T_SecondProjectileAssociatedThing, L0626_i_MapX, L0627_i_MapY, returnNextVal(L0628_ui_ProjectileCell), (direction)L0624_ui_Cell, L0630_i_KineticEnergy, 100, L0631_i_StepEnergy);
	}
	_vm->_projexpl->_g365_createLanucherProjectile = false;
}

void Timeline::f245_timlineProcessEvent5_squareCorridor(TimelineEvent* event) {
#define k0x0008_randomizeGeneratedCreatureCount 0x0008 // @ MASK0x0008_RANDOMIZE_GENERATED_CREATURE_COUNT
#define k0x0007_generatedCreatureCount 0x0007	// @ MASK0x0007_GENERATED_CREATURE_COUNT

	int16 L0610_i_ThingType;
	bool L0611_B_TextCurrentlyVisible;
	int16 L0612_i_CreatureCount;
	Thing L0613_T_Thing;
	Sensor* L0614_ps_Sensor;
	TextString* L0615_ps_TextString;
	uint16 L0616_ui_MapX;
	uint16 L0617_ui_MapY;
	uint16 L0618_ui_Multiple;
#define AL0618_ui_HealthMultiplier L0618_ui_Multiple
#define AL0618_ui_Ticks            L0618_ui_Multiple
	TimelineEvent L0619_s_Event;


	L0613_T_Thing = _vm->_dungeonMan->f161_getSquareFirstThing(L0616_ui_MapX = event->_B._location._mapX, L0617_ui_MapY = event->_B._location._mapY);
	while (L0613_T_Thing != Thing::_endOfList) {
		if ((L0610_i_ThingType = L0613_T_Thing.getType()) == k2_TextstringType) {
			L0615_ps_TextString = (TextString*)_vm->_dungeonMan->f156_getThingData(L0613_T_Thing);
			L0611_B_TextCurrentlyVisible = L0615_ps_TextString->isVisible();
			if (event->_C.A._effect == k2_SensorEffToggle) {
				L0615_ps_TextString->setVisible(!L0611_B_TextCurrentlyVisible);
			} else {
				L0615_ps_TextString->setVisible((event->_C.A._effect == k0_SensorEffSet));
			}
			if (!L0611_B_TextCurrentlyVisible && L0615_ps_TextString->isVisible() && (_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) && (L0616_ui_MapX == _vm->_dungeonMan->_g306_partyMapX) && (L0617_ui_MapY == _vm->_dungeonMan->_g307_partyMapY)) {
				_vm->_dungeonMan->f168_decodeText(_vm->_g353_stringBuildBuffer, L0613_T_Thing, k1_TextTypeMessage);
				_vm->_textMan->f47_messageAreaPrintMessage(k15_ColorWhite, _vm->_g353_stringBuildBuffer);
			}
		} else {
			if (L0610_i_ThingType == k3_SensorThingType) {
				L0614_ps_Sensor = (Sensor*)_vm->_dungeonMan->f156_getThingData(L0613_T_Thing);
				if (L0614_ps_Sensor->getType() == k6_SensorFloorGroupGenerator) {
					L0612_i_CreatureCount = L0614_ps_Sensor->getValue();
					if (getFlag(L0612_i_CreatureCount, k0x0008_randomizeGeneratedCreatureCount)) {
						L0612_i_CreatureCount = _vm->getRandomNumber(getFlag(L0612_i_CreatureCount, k0x0007_generatedCreatureCount));
					} else {
						L0612_i_CreatureCount--;
					}
					if ((AL0618_ui_HealthMultiplier = L0614_ps_Sensor->M45_healthMultiplier()) == 0) {
						AL0618_ui_HealthMultiplier = _vm->_dungeonMan->_g269_currMap->_difficulty;
					}
					_vm->_groupMan->f185_groupGetGenerated(L0614_ps_Sensor->getData(), AL0618_ui_HealthMultiplier, L0612_i_CreatureCount, (direction)_vm->getRandomNumber(4), L0616_ui_MapX, L0617_ui_MapY);
					if (L0614_ps_Sensor->getAudibleA()) {
						warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
					}
					if (L0614_ps_Sensor->getOnlyOnce()) {
						L0614_ps_Sensor->setTypeDisabled();
					} else {
						if ((AL0618_ui_Ticks = L0614_ps_Sensor->M46_ticks()) != 0) {
							L0614_ps_Sensor->setTypeDisabled();
							if (AL0618_ui_Ticks > 127) {
								AL0618_ui_Ticks = (AL0618_ui_Ticks - 126) << 6;
							}
							L0619_s_Event._type = k65_TMEventTypeEnableGroupGenerator;
							M33_setMapAndTime(L0619_s_Event._mapTime, _vm->_dungeonMan->_g272_currMapIndex, _vm->_g313_gameTime + AL0618_ui_Ticks);
							L0619_s_Event._priority = 0;
							L0619_s_Event._B._location._mapX = L0616_ui_MapX;
							L0619_s_Event._B._location._mapY = L0617_ui_MapY;
							L0619_s_Event._B._location._mapY = L0617_ui_MapY;
							_vm->_timeline->f238_addEventGetEventIndex(&L0619_s_Event);
						}
					}
				}
			}
		}
		L0613_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0613_T_Thing);
	}
}

void Timeline::f252_timelineProcessEvents60to61_moveGroup(TimelineEvent* event) {
	uint16 L0656_ui_MapX;
	uint16 L0657_ui_MapY;
	Group* L0658_ps_Group;
	bool L0659_B_RandomDirectionMoveRetried;


	L0659_B_RandomDirectionMoveRetried = false;
	L0656_ui_MapX = event->_B._location._mapX;
	L0657_ui_MapY = event->_B._location._mapY;
	L0657_ui_MapY = event->_B._location._mapY;
T0252001:
	if (((_vm->_dungeonMan->_g272_currMapIndex != _vm->_dungeonMan->_g309_partyMapIndex) || (L0656_ui_MapX != _vm->_dungeonMan->_g306_partyMapX) || (L0657_ui_MapY != _vm->_dungeonMan->_g307_partyMapY)) && (_vm->_groupMan->f175_groupGetThing(L0656_ui_MapX, L0657_ui_MapY) == Thing::_endOfList)) { /* BUG0_24 Lord Chaos may teleport into one of the Black Flames and become invisible until the Black Flame is killed. In this case, _vm->_groupMan->f175_groupGetThing returns the Black Flame thing and the Lord Chaos thing is not moved into the dungeon until the Black Flame is killed */
		if (event->_type == k61_TMEventTypeMoveGroupAudible) {
			warning(false, "F0064_SOUND_RequestPlay_CPSD");
		}
		_vm->_movsens->f267_getMoveResult(Thing(event->_C._slot), kM1_MapXNotOnASquare, 0, L0656_ui_MapX, L0657_ui_MapY);
	} else {
		if (!L0659_B_RandomDirectionMoveRetried) {
			L0659_B_RandomDirectionMoveRetried = true;
			L0658_ps_Group = (Group*)_vm->_dungeonMan->f156_getThingData(Thing(event->_C._slot));
			if ((L0658_ps_Group->_type == k23_CreatureTypeLordChaos) && !_vm->getRandomNumber(4)) {
				switch (_vm->getRandomNumber(4)) {
				case 0:
					L0656_ui_MapX--;
					break;
				case 1:
					L0656_ui_MapX++;
					break;
				case 2:
					L0657_ui_MapY--;
					break;
				case 3:
					L0657_ui_MapY++;
				}
				if (_vm->_groupMan->f223_isSquareACorridorTeleporterPitOrDoor(L0656_ui_MapX, L0657_ui_MapY))
					goto T0252001;
			}
		}
		event->_mapTime += 5;
		_vm->_timeline->f238_addEventGetEventIndex(event);
	}
}

void Timeline::f246_timelineProcesEvent65_enableGroupGenerator(TimelineEvent* event) {
	Thing L0620_T_Thing;
	Sensor* L0621_ps_Sensor;

	L0620_T_Thing = _vm->_dungeonMan->f161_getSquareFirstThing(event->_B._location._mapX, event->_B._location._mapY);
	L0620_T_Thing = _vm->_dungeonMan->f161_getSquareFirstThing(event->_B._location._mapX, event->_B._location._mapY);
	while (L0620_T_Thing != Thing::_none) {
		if ((L0620_T_Thing.getType()) == k3_SensorThingType) {
			L0621_ps_Sensor = (Sensor*)_vm->_dungeonMan->f156_getThingData(L0620_T_Thing);
			if (L0621_ps_Sensor->getType() == k0_SensorDisabled) {
				L0621_ps_Sensor->setDatAndTypeWithOr(k6_SensorFloorGroupGenerator);
				return;
			}
		}
		L0620_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0620_T_Thing);
	}
}
}
