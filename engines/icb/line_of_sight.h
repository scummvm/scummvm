/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_LINEOFSIGHT_H_INCLUDED
#define ICB_LINEOFSIGHT_H_INCLUDED

#include "engines/icb/p4.h"
#include "engines/icb/debug.h"
#include "engines/icb/event_list.h"
#include "engines/icb/event_manager.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_3drealpoint.h"

namespace ICB {

#define LOS_LOG "LOS_log.txt"

// This sets the default field-of-view for mega characters.
#define LOS_DEFAULT_MEGA_FIELD_OF_VIEW 180

// This sets the default view height for mega characters (+/- this linear distance in cm).
// BIG NOTE: This value must be LESS than the difference between the crouched height and the normal eye-height,
// otherwise LOS will always return false when one mega is crouched and another is standing.
#define LOS_DEFAULT_OBJECT_HEIGHT_OF_VIEW (100 * REAL_ONE)

// This sets how far an object can see by default.
#define LOS_DEFAULT_OBJECT_SEEING_DISTANCE 2000

// This defines the default number of subscribers to process per cycle of the LOS logic.
#define LOS_DEFAULT_SUBSCRIBERS_PER_CYCLE 3

// This is the size of the 2D and 1D tables.
#define LOS_1D_SIZE (MAX_session_objects)
#define LOS_2D_ROWSIZE_PACKED (MAX_session_objects / 8)
#define LOS_2D_SIZE_PACKED (MAX_session_objects * LOS_2D_ROWSIZE_PACKED)

// Module to calculate which objects (voxel characters and other objects) can see which other objects.  The information
// is used to generate events, which are given to the event manager, so it can decide what needs to hear about the change.
class _line_of_sight {
public:
	// Definitions used by this class.
	enum { ACTOR_EYE_HEIGHT = 160 };
	enum { ACTOR_CROUCHED_HEIGHT = 61 };
	enum ActorEyeMode { USE_OBJECT_VALUE = 0, FORCE_EYE_HEIGHT, FORCE_CROUCHED_HEIGHT };

	// Default constructor and destructor.
	inline _line_of_sight();
	inline ~_line_of_sight();

	// This attaches this object to its data files and sets other start-up values.
	void Initialise();

	// These add and remove objects from the service.
	void Subscribe(uint32 nObserverID, uint32 nTargetID);
	void UnSubscribe(uint32 nObserverID, uint32 nTargetID);

	// These allow an object to be temporarily suspended in the LOS engine, but the objects they
	// are registered to see are remembered for when the suspension is removed.
	void Suspend(uint32 nObserverID);
	inline void Unsuspend(uint32 nObserverID);

	// These two turn off the whole of line-of-sight processing for everybody.  All the registrations are kept
	// though, so it can resume where it left off.
	void SwitchOff();
	void SwitchOn() { m_bSwitchedOn = TRUE8; }

	// The engine actually calls this function instead of Cycle() directly, so that the amount of time
	// spent in line-of-sight calculation can be controlled.
	void DutyCycle();

	// This function allows the how many subscribers to do per cycle
	inline void SetDutyCycle(uint32 nSubsPerCycle) { m_nSubsPerCycle = nSubsPerCycle; }

	// This allows the field-of-view to be changed from the default 180 for a mega character.
	void SetFieldOfView(uint32 nID, uint32 nFieldOfView);

	// This allows the range an object sees to be changed from the default.
	void SetSightRange(uint32 nID, uint32 nRange);

	// This allows the height range an object sees to be changed from the default.
	void SetSightHeight(uint32 nID, uint32 nHeight);

	// This sets whether or not an object can see in the dark.
	inline void SetCanSeeInDarkFlag(uint32 nID, bool8 bState);

	// This sets whether or not a mega is always seen regardless of shadows or not.
	inline void SetNeverInShadowFlag(uint32 nID, bool8 bState);

	// This turns off shadow-handling for the whole LOS engine.
	inline void ShadowsOnOff(bool8 bState) { m_bHandleShadows = bState; }

	// This checks line-of-sight between two objects, accounting for field-of-view if observer is an actor,
	bool8 ObjectToObject(uint32 nObserverID, uint32 nTargetID, _barrier_ray_type eRayType, bool8 bCanSeeUs, ActorEyeMode eEyeMode, bool8 bOverrideHeightLimit = FALSE8);

	// This checks line-of-sight using the current truth table values and so is fast.
	inline bool8 LineOfSight(uint32 nObserverID, uint32 nTargetID);

	// And these two return extra information after a call to ObjectToObject().
	const px3DRealPoint GetLastImpactPoint() const { return (m_oImpactPoint); }
	_barrier_logic_value GetLastImpactType() const { return (m_eImpactType); }

	// This allows other classes to get at the barrier slices (the Remora needs this).
	_linked_data_file *GetSlicesPointer() const { return (m_pyLOSData); }

	bool8 FailingOnHeight() const { return (m_bFailingOnHeight); }

private:
	_linked_data_file *m_pyLOSData;            // Pointer to the loaded line-of-sight data file.
	uint32 m_nSubsPerCycle;                    // How many subscribers to process per cycle.
	uint32 m_nFirstSubscriber;                 // Number of first subscriber to process
	uint32 m_nNumObjects;                      // Number of subscribers to this service.
	uint32 m_nTotalCurrentSubscribers;         // Total number of current subscribers to this service.
	uint32 m_pnFieldOfView[LOS_1D_SIZE];       // Current field-of-view for each mega.
	uint32 m_pnSeeingDistance[LOS_1D_SIZE];    // Maximum distance an object can see.
	PXreal m_pfHeightOfView[LOS_1D_SIZE];      // Height restriction on view.
	int32 m_pnSubscribeNum[LOS_1D_SIZE];       // How many times a game object has subscribed.
	uint8 m_pnTable[LOS_2D_SIZE_PACKED];       // The truth-table of who can see who.
	uint8 m_pnSubscribers[LOS_2D_SIZE_PACKED]; // Housekeeping table of current subscribers to the service.
	bool8 m_pbSuspended[LOS_1D_SIZE];          // If true, the object has been temporarily suspended.
	bool8 m_pbCanSeeInDark[LOS_1D_SIZE];       // Housekeeping table of objects that should ignore shadows.
	bool8 m_pbIgnoreShadows[LOS_1D_SIZE];      // Table of megas that are always seen regardless of shadows.
	px3DRealPoint m_oImpactPoint;              // Holds impact point from last call to ObjectToObject().
	_barrier_logic_value m_eImpactType;        // Holds type of impact from last call to ObjectToObject().
	bool8 m_bSwitchedOn;                       // Flag that allows LOS processing to be suspended.
	bool8 m_bFailingOnHeight;                  // Debug flag that gets set when a barrier height check fails.
	bool8 m_bHandleShadows;                    // Turns shadow handling on/off.
	uint8 m_nPad;

	// Here I block the use of the default '='.
	_line_of_sight(const _line_of_sight &) {}
	void operator=(const _line_of_sight &) {}

	// Functions used internally by this class.
	void WhatSeesWhat();

	bool8 InFieldOfView(PXreal fLookingX, PXreal fLookingZ, PXfloat fLookingDirection, PXreal fObservedX, PXreal fObservedZ, uint32 nFieldOfView) const;

	inline void SetPackedBit(uint8 *pnArray, uint32 i, uint32 j, bool8 bValue);
	inline bool8 GetPackedBit(uint8 *pnArray, uint32 i, uint32 j) const;
};

extern _line_of_sight *g_oLineOfSight;

inline _line_of_sight::_line_of_sight() {
	m_bSwitchedOn = TRUE8;
	m_nSubsPerCycle = LOS_DEFAULT_SUBSCRIBERS_PER_CYCLE;
	m_nFirstSubscriber = 0;
	m_pyLOSData = NULL;
	m_nNumObjects = 0;
	m_nTotalCurrentSubscribers = 0;
	m_bHandleShadows = TRUE8;
	m_nPad = 0;
}

inline _line_of_sight::~_line_of_sight() {
	m_pyLOSData = NULL;
	m_nNumObjects = 0;

	Zdebug("Destroyed line-of-sight object");
}

inline void _line_of_sight::Unsuspend(uint32 nObserverID) {
	// Set the flag for the object.
	m_pbSuspended[nObserverID] = FALSE8;
}

inline void _line_of_sight::SetCanSeeInDarkFlag(uint32 nID, bool8 bState) {
	// Set the flag for the object.
	m_pbCanSeeInDark[nID] = bState;
}

inline void _line_of_sight::SetNeverInShadowFlag(uint32 nID, bool8 bState) {
	// Set the flag for the object.
	m_pbIgnoreShadows[nID] = bState;
}

inline bool8 _line_of_sight::LineOfSight(uint32 nObserverID, uint32 nTargetID) { return (GetPackedBit(m_pnTable, nObserverID, nTargetID)); }

inline void _line_of_sight::SwitchOff() {
	m_bSwitchedOn = FALSE8;

	memset((unsigned char *)m_pnTable, 0, (size_t)(LOS_2D_SIZE_PACKED * sizeof(uint8)));
}

inline void _line_of_sight::SetPackedBit(uint8 *pnArray, uint32 i, uint32 j, bool8 bValue) {
	uint32 nJIndex = j >> 3;
	uint32 nJRemainder = (j & 0x00000007);

	if (bValue)
		pnArray[i * LOS_2D_ROWSIZE_PACKED + nJIndex] |= (uint8)(1 << nJRemainder);
	else
		pnArray[i * LOS_2D_ROWSIZE_PACKED + nJIndex] &= (uint8)(~(1 << nJRemainder));
}

inline bool8 _line_of_sight::GetPackedBit(uint8 *pnArray, uint32 i, uint32 j) const {
	uint32 nJIndex = j >> 3;
	uint32 nJRemainder = (j & 0x00000007);

	if ((pnArray[i * LOS_2D_ROWSIZE_PACKED + nJIndex] & (uint8)(1 << nJRemainder)) != 0)
		return (TRUE8);
	else
		return (FALSE8);
}

} // End of namespace ICB

#endif // #if !defined( LINEOFSIGHT_H_INCLUDED )
