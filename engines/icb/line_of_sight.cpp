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

#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/debug.h"
#include "engines/icb/line_of_sight.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/mission.h"
#include "engines/icb/tracer.h"
#include "engines/icb/event_manager.h"
#include "engines/icb/res_man.h"

namespace ICB {

int32 john_number_traces = 0;
int32 john_total_traces = 0;

void _line_of_sight::Initialise() {
	uint32 i;
	_floor_world *pFloorWorld;
	_linked_data_file *pyBarriers;
	uint32 oFileName_hash = NULL_HASH;

	// Set the number of subscribers processed per cycle back to its starting value.
	m_nSubsPerCycle = LOS_DEFAULT_SUBSCRIBERS_PER_CYCLE;

	// This is where in the list of subscribers processing will begin next cycle.
	m_nFirstSubscriber = 0;

	// No subscribers to start with.
	m_nTotalCurrentSubscribers = 0;

// Now open the list of barriers and set a pointer to them.
#if defined(_PSX)
	char *oFileName = PX_FILENAME_LINEOFSIGHT;

#else
	char oFileName[ENGINE_STRING_LEN];

	// When clustered the session files have the base stripped
	strcpy(oFileName, PX_FILENAME_LINEOFSIGHT);
#endif

	uint32 cluster_hash = MS->Fetch_session_cluster_hash();
	m_pyLOSData = (_linked_data_file *)private_session_resman->Res_open(oFileName, oFileName_hash, MS->Fetch_session_cluster(), cluster_hash);

	Zdebug("private_session_resman opened %s", (const char *)oFileName);

	// Check file version is correct.
	if (m_pyLOSData->GetHeaderVersion() != VERSION_PXWGLINEOFSIGHT)
		Fatal_error(".pxwglineofsight version check failed (file has %d, engine has %d)", m_pyLOSData->GetHeaderVersion(), VERSION_PXWGLINEOFSIGHT);

	// The tracer object can be initialised now we have the barrier map.
	g_oTracer.SetUpParameters(m_pyLOSData);

	// Set the number of objects from the list that should already have been created in the session.
	m_nNumObjects = MS->total_objects;

	// Clear the tables.
	memset((unsigned char *)m_pnTable, 0, (size_t)(LOS_2D_SIZE_PACKED * sizeof(uint8)));
	memset((unsigned char *)m_pnSubscribers, 0, (size_t)(LOS_2D_SIZE_PACKED * sizeof(uint8)));
	memset((unsigned char *)m_pbSuspended, 0, (size_t)(LOS_1D_SIZE * sizeof(bool8)));
	memset((unsigned char *)m_pbCanSeeInDark, 0, LOS_1D_SIZE * sizeof(bool8));
	memset((unsigned char *)m_pbIgnoreShadows, 0, LOS_1D_SIZE * sizeof(bool8));

	// Field-of-view for megas defaults to 180.  Note that this array is not accessed for non-megas, so it doesn't
	// matter that their entries get set to 180 too.  Also, by default you are not subscribed to be looking for anything!
	for (i = 0; i < m_nNumObjects; ++i) {
		m_pnFieldOfView[i] = LOS_DEFAULT_MEGA_FIELD_OF_VIEW;
		m_pfHeightOfView[i] = LOS_DEFAULT_OBJECT_HEIGHT_OF_VIEW;
		m_pnSeeingDistance[i] = LOS_DEFAULT_OBJECT_SEEING_DISTANCE;
		m_pnSubscribeNum[i] = 0;
	}

	// Get the pointer to the raw barriers, make sure it is valid and set it in the tracer.
	pyBarriers = MS->session_barriers->Get_barrier_pointer();

	if (!pyBarriers)
		Fatal_error("Barriers pointer NULL in _line_of_sight::Initialise()");

	g_oTracer.SetBarrierPointer(pyBarriers);

	// The tracer also needs access to the routing data to get the floor rectangles.
	pFloorWorld = MS->floor_def;
	g_oTracer.SetFloorsPointer(pFloorWorld);

	// These probably don't need setting, but we'll do it for completeness.
	m_oImpactPoint.Set(REAL_ZERO, REAL_ZERO, REAL_ZERO);
	m_eImpactType = NO_IMPACT;
}

void _line_of_sight::DutyCycle() {
	// If there's 0 or 1 objects, there can be no events generated between them.
	if (m_nNumObjects < 2)
		return;

	// Now I need to do the what-sees-what calculations fresh for the new array.
	// Don't do anything if the line-of-sight engine is turned off.
	if (m_bSwitchedOn)
		WhatSeesWhat();
}

void _line_of_sight::Subscribe(uint32 nObserverID, uint32 nTargetID) {
	// Stop an object from subscribing to itself.
	if (nObserverID == nTargetID)
		return;

	// Nothing to do if this subscription is in place already.
	if (!GetPackedBit(m_pnSubscribers, nObserverID, nTargetID)) {
		// Simply set the corresponding entry in the subscriber's table to true.
		SetPackedBit(m_pnSubscribers, nObserverID, nTargetID, TRUE8);

		// Keep track of total number of subscribers.
		++m_nTotalCurrentSubscribers;

		// This keeps track of how many objects the current object is subscribed to look for.  Like a
		// reference count, the object no longer requires LOS processing if this hits 0.
		m_pnSubscribeNum[nObserverID]++;
	}
}

void _line_of_sight::UnSubscribe(uint32 nObserverID, uint32 nTargetID) {
	// Nothing to do if there is no subscription to remove.
	if (GetPackedBit(m_pnSubscribers, nObserverID, nTargetID)) {
		// Simply set the corresponding entry in the subscriber's table to false.
		SetPackedBit(m_pnSubscribers, nObserverID, nTargetID, FALSE8);
		--m_nTotalCurrentSubscribers;

		// Undo the number of subscriptions for this observerID
		m_pnSubscribeNum[nObserverID]--;

		// Reset the truth table entry to 0 to make sure A can't see B anymore.
		SetPackedBit(m_pnTable, nObserverID, nTargetID, FALSE8);
	}
}

bool8 _line_of_sight::ObjectToObject(uint32 nObserverID, uint32 nTargetID, _barrier_ray_type eRayType, bool8 bCanSeeUs, ActorEyeMode eEyeMode, bool8 bOverrideHeightLimit) {
	_logic *pObserver;
	_logic *pTarget;
	bool8 bObserverIsActor, bTargetIsActor;
	PXreal fObserverX, fObserverY, fObserverZ;
	PXfloat fObserverDirection;
	PXreal fTargetX, fTargetY, fTargetZ;
	uint32 nFieldOfView;
	px3DRealPoint oFrom, oTo;
	bool8 nRetVal;

	// If the line-of-sight engine is turned off, this function always returns false.
	if (!m_bSwitchedOn)
		return (FALSE8);

	// Yes, this observer wants to know about line-of-sight for this target.
	pObserver = MS->logic_structs[nObserverID];
	pTarget = MS->logic_structs[nTargetID];

	// Check if the observer is an actor (with 180-degree vision) or an object (with
	// 360-degree 'vision').  Also need to set type for target, though field-of-view doesn't
	// matter.
	bObserverIsActor = (pObserver->image_type == VOXEL) ? TRUE8 : FALSE8;
	bTargetIsActor = (pTarget->image_type == VOXEL) ? TRUE8 : FALSE8;

	// The following shadow check only gets done if shadows are enabled.
	if (m_bHandleShadows) {
		// If the observer is unable to see in the dark and the target is in shade and we
		// are not ignoring shadows for that target then observer cannot see target.
		if (!m_pbCanSeeInDark[nObserverID] && pTarget->mega->in_shade && !m_pbIgnoreShadows[nTargetID])
			return (FALSE8);
	}

	// Set observer's position.
	if (bObserverIsActor) {
		fObserverX = pObserver->mega->actor_xyz.x;
		fObserverZ = pObserver->mega->actor_xyz.z;
		fObserverDirection = pObserver->pan;

		switch (eEyeMode) {
		case USE_OBJECT_VALUE:
			if (pObserver->mega->Is_crouched())
				fObserverY = pObserver->mega->actor_xyz.y + ACTOR_CROUCHED_HEIGHT;
			else
				fObserverY = pObserver->mega->actor_xyz.y + ACTOR_EYE_HEIGHT;
			break;

		case FORCE_EYE_HEIGHT:
			fObserverY = pObserver->mega->actor_xyz.y + ACTOR_EYE_HEIGHT;
			break;

		case FORCE_CROUCHED_HEIGHT:
			fObserverY = pObserver->mega->actor_xyz.y + ACTOR_CROUCHED_HEIGHT;
			break;

		default:
			fObserverY = REAL_ZERO; // Stops VC4 warning.
			Fatal_error("Illegal value %d for eEyeMode passed into _line_of_sight::ObjectToObject()", eEyeMode);
		};
	} else {
		fObserverX = pObserver->prop_xyz.x;
		fObserverY = pObserver->prop_xyz.y;
		fObserverZ = pObserver->prop_xyz.z;
		fObserverDirection = FLOAT_ZERO; // Do this for VC4 compiler.
	}

	// Set target's position.
	if (bTargetIsActor) {
		fTargetX = pTarget->mega->actor_xyz.x;
		fTargetZ = pTarget->mega->actor_xyz.z;

		if (pTarget->mega->Is_crouched())
			fTargetY = pTarget->mega->actor_xyz.y + ACTOR_CROUCHED_HEIGHT;
		else
			fTargetY = pTarget->mega->actor_xyz.y + ACTOR_EYE_HEIGHT;
	} else {
		fTargetX = pTarget->prop_xyz.x;
		fTargetY = pTarget->prop_xyz.y;
		fTargetZ = pTarget->prop_xyz.z;
	}

	// Here we check the height of the two objects against each other.  If this test fails, we don't need to do
	// any of the heavier calculations.  Note that this test can can now be overriden by the caller.
	if (!bOverrideHeightLimit) {
		if ((fTargetY > fObserverY + m_pfHeightOfView[nObserverID]) || (fTargetY < fObserverY - m_pfHeightOfView[nObserverID])) {
			// Target cannot be in view because it is not in the right height range.
			m_eImpactType = NO_IMPACT;
			m_oImpactPoint = oFrom;
			return (FALSE8);
		}
	}

	// Have a maximum range that objects can see
	PXreal dx = fTargetX - fObserverX;
	PXreal dy = fTargetY - fObserverY;
	PXreal dz = fTargetZ - fObserverZ;
	const PXreal max_range = (PXreal)m_pnSeeingDistance[nObserverID];
	const PXreal max_range_sqr = max_range * max_range;

	// Try on size of dx, dy, dz first
	if (((PXreal)PXfabs(dx) > max_range) || ((PXreal)PXfabs(dy) > max_range) || ((PXreal)PXfabs(dz) > max_range)) {
		m_eImpactType = NO_IMPACT;
		m_oImpactPoint = oFrom;
		return (FALSE8);
	}

	// Now try on squared distance
	if ((dx * dx + dy * dy + dz * dz) > max_range_sqr) {
		m_eImpactType = NO_IMPACT;
		m_oImpactPoint = oFrom;
		return (FALSE8);
	}

	// Object is in object's field of view.  Now we need to check if anything is
	// in the way.
	oFrom.SetX(fObserverX);
	oFrom.SetY(fObserverY);
	oFrom.SetZ(fObserverZ);

	oTo.SetX(fTargetX);
	oTo.SetY(fTargetY);
	oTo.SetZ(fTargetZ);

	// If the observer is an actor, we must first check field of view.
	if (bObserverIsActor) {
		// The obesrver is an actor, so we need to retrieve the current field-of-view setting for them.
		nFieldOfView = m_pnFieldOfView[nObserverID];

		// Check field of view only if not 360.
		if (nFieldOfView != 360) {
			// Need to check field-of-view properly for 1-180 degrees.
			if (InFieldOfView(fObserverX, fObserverZ, fObserverDirection, fTargetX, fTargetZ, nFieldOfView)) {
				if (bCanSeeUs)
					return (TRUE8);
				int32 john_test_time = GetMicroTimer();
				nRetVal = g_oTracer.Trace(oFrom, oTo, eRayType, m_oImpactPoint, m_eImpactType);
				john_test_time = GetMicroTimer() - john_test_time;
				john_total_traces += john_test_time;
				john_number_traces++;
				return (nRetVal);
			} else {
				// Observer is an actor but target is not in field-of-view.
				m_eImpactType = NO_IMPACT;
				m_oImpactPoint = oFrom;
				return (FALSE8);
			}
		} else {
			// Don't need to apply field-of-view for 360-degree vision.
			int32 john_test_time = g_system->getMillis();
			nRetVal = g_oTracer.Trace(oFrom, oTo, eRayType, m_oImpactPoint, m_eImpactType);
			john_test_time = g_system->getMillis() - john_test_time;
			john_total_traces += john_test_time;
			john_number_traces++;
			return (nRetVal);
		}
	} else {
		// Objects are assumed to have 360-degree vision so we just have to do a line-of-sight.
		int32 john_test_time = g_system->getMillis();
		nRetVal = g_oTracer.Trace(oFrom, oTo, eRayType, m_oImpactPoint, m_eImpactType);
		john_test_time = g_system->getMillis() - john_test_time;
		john_total_traces += john_test_time;
		john_number_traces++;
		return (nRetVal);
	}
}

void _line_of_sight::SetFieldOfView(uint32 nID, uint32 nFieldOfView) {
	// Check ID of object is valid.
	if (nID >= m_nNumObjects)
		Fatal_error("ID %d out-of-range (%d objects) in _line_of_sight::SetFieldOfView()", nID, m_nNumObjects);

	Zdebug("Setting FOV for %d to %d", nID, nFieldOfView);

	// If object is non-mega, do nothing.
	if (MS->logic_structs[nID]->image_type != VOXEL)
		return;

	// Right, ID is a valid mega.
	m_pnFieldOfView[nID] = nFieldOfView;
}

void _line_of_sight::SetSightRange(uint32 nID, uint32 nRange) {
	// Check ID of object is valid.
	if (nID >= m_nNumObjects)
		Fatal_error("ID %d out-of-range (%d objects) in _line_of_sight::SetSightRange()", nID, m_nNumObjects);

	Zdebug("Setting sight range for %d to %d", nID, nRange);

	// Set the range for the object.
	m_pnSeeingDistance[nID] = nRange;
}

void _line_of_sight::SetSightHeight(uint32 nID, uint32 nHeight) {
	// Check ID of object is valid.
	if (nID >= m_nNumObjects)
		Fatal_error("ID %d out-of-range (%d objects) in _line_of_sight::SetSightHeight()", nID, m_nNumObjects);

	Zdebug("Setting sight height for %d to %d", nID, nHeight);

	// Set the sight height for the object.
	m_pfHeightOfView[nID] = (PXfloat)nHeight;
}

void _line_of_sight::Suspend(uint32 nObserverID) {
	uint32 i;

	// Set the suspended flag for the object.
	m_pbSuspended[nObserverID] = TRUE8;

	// So engine doesn't have to wait for the LOS engine to chug round, wipe the truth table row for the object now:
	// it will not see anything.  Also wipe the column so object becomes invisible to other objects.
	for (i = 0; i < m_nNumObjects; ++i) {
		SetPackedBit(m_pnTable, i, nObserverID, FALSE8);
		SetPackedBit(m_pnTable, nObserverID, i, FALSE8);
	}
}

void _line_of_sight::WhatSeesWhat() {
	uint32 i, j;
	bool8 nResult;
	bool8 bSubscribed;
	uint32 nProcessed = 0;
	int32 *pnHowManyTimesObjectHasSubscribed = m_pnSubscribeNum + m_nFirstSubscriber;
	bool8 bTargetRequiresLOSProcessing;
	uint32 nPlayerID;

	// Cache player ID.
	nPlayerID = MS->player.Fetch_player_id();

	// Do the player line of sight every game cycle.  Only do any checking if the player has subscribed
	// to look for something.  And don't bother if the player is suspended.
	if ((m_pnSubscribeNum[nPlayerID] > 0) && !m_pbSuspended[nPlayerID]) {
		// Don't try to do anything if the object has been shut down.
		// Hmmm, if the player has been shut-down then in big trouble
		if (MS->logic_structs[nPlayerID]->ob_status != OB_STATUS_HELD) {
			// Loop for each possible observed target.
			for (j = 0; j < m_nNumObjects; ++j) {
				// Player must ignore himself.
				if (j != nPlayerID) {
					// Check to make sure target is not shut down and check to see if the
					// observer has registered for line-of-sight with the target.
					// Help the lazy logic evaluation : m_pnSubscribers more likely to be false
					bSubscribed = GetPackedBit(m_pnSubscribers, nPlayerID, j);
					if (bSubscribed && (MS->logic_structs[j]->ob_status != OB_STATUS_HELD)) {
						// Yes, this observer wants to know about line-of-sight for this target.
						nResult = ObjectToObject(nPlayerID, j, LIGHT, FALSE8, USE_OBJECT_VALUE); // Pass false as bCanSeeUs, this means tracer
						                                                                         // will always get called for player->guard type
						                                                                         // seeing...
						if (nResult != GetPackedBit(m_pnTable, nPlayerID, j)) {
							SetPackedBit(m_pnTable, nPlayerID, j, nResult);

							if (nResult) {
								// Entries differ, so a character has either moved in-to or out-of view.
								g_oEventManager->PostNamedEventToObject(EVENT_LINE_OF_SIGHT, nPlayerID, j);
							}
						}
					} else {
						// Not registered or object shutdown so LOS = false
						SetPackedBit(m_pnTable, nPlayerID, j, FALSE8);
					}
				}
			}
		}
	}

	// This loop does line-of-sight for all other megas.  These don't get updated as often as the player.
	i = m_nFirstSubscriber;

	bool8 quittingOut = 0;

	// Loop for as many objects as are going to be processed.
	while ((!quittingOut) && (nProcessed < m_nSubsPerCycle) && (nProcessed < m_nTotalCurrentSubscribers)) {
		// Only do any checking if this object has subscribed to look for something.  And don't check if
		// the object is suspended.
		if ((i != nPlayerID) && (*pnHowManyTimesObjectHasSubscribed > 0) && !m_pbSuspended[i]) {
			// Count the objects processed this call.
			++nProcessed;

			// There are a number of engine states that can be set for an object that mean it no longer
			// requires LOS processing.
			if ((MS->logic_structs[i]->ob_status != OB_STATUS_HELD) && (MS->logic_structs[i]->big_mode != __MEGA_SLICE_HELD) &&
			    (MS->logic_structs[i]->big_mode != __MEGA_PLAYER_FLOOR_HELD) && (MS->logic_structs[i]->big_mode != __MEGA_INITIAL_FLOOR_HELD)) {
				// Loop for each possible observed target.
				for (j = 0; j < m_nNumObjects; ++j) {
					// Ignore ourselves.
					if (i != j) {
						if ((MS->logic_structs[j]->ob_status != OB_STATUS_HELD) && (MS->logic_structs[j]->big_mode != __MEGA_SLICE_HELD) &&
						    (MS->logic_structs[j]->big_mode != __MEGA_PLAYER_FLOOR_HELD) && (MS->logic_structs[j]->big_mode != __MEGA_INITIAL_FLOOR_HELD)) {
							bTargetRequiresLOSProcessing = TRUE8;
						} else {
							bTargetRequiresLOSProcessing = FALSE8;
						}

						// Check to make sure target is not shut down and check to see if the observer has
						// registered for line-of-sight with the target.
						// Help the lazy logic evaluation : m_pnSubscribers more likely to be false
						bSubscribed = GetPackedBit(m_pnSubscribers, i, j);
						if (bSubscribed && !m_pbSuspended[j] && bTargetRequiresLOSProcessing) {
							// Yes, this observer wants to know about line-of-sight for this target.
							if (j == nPlayerID)
								nResult = ObjectToObject(i, j, LIGHT, LineOfSight(nPlayerID, i), USE_OBJECT_VALUE);
							else
								nResult = ObjectToObject(i, j, LIGHT, FALSE8, USE_OBJECT_VALUE);

							if (nResult != GetPackedBit(m_pnTable, i, j)) {
								SetPackedBit(m_pnTable, i, j, nResult);

								if (nResult) {
									// Entries differ, so a character has either moved in-to or out-of
									// view.
									g_oEventManager->PostNamedEventToObject(EVENT_LINE_OF_SIGHT, i, j);
								} else {
								}
							}
						} else {
							// Not registered or object shutdown so LOS = false
							SetPackedBit(m_pnTable, i, j, FALSE8);
						}
					} // end if
				}         // end for
			}                 // end if
		}                         // end if

		// Move i onto next object, but because we didn't start at the beginning of the list we must watch for
		// wrapping at the end.  Similarly, we must move on in the number of subscriber per object array.
		++i;
		if (i == m_nNumObjects) {
			i = 0;
			pnHowManyTimesObjectHasSubscribed = m_pnSubscribeNum;
		} else {
			++pnHowManyTimesObjectHasSubscribed;
		}

		// if we have looped round whole thing and found nothing to do then get out...
		if (i == m_nFirstSubscriber)
			quittingOut = 1;
	} // end for

	// Update which subscriber to start at next DutyCycle around.
	m_nFirstSubscriber = i;
}

bool8 _line_of_sight::InFieldOfView(PXreal fLookingX, PXreal fLookingZ, PXfloat fLookingDirection, PXreal fObservedX, PXreal fObservedZ, uint32 nFieldOfView) const {
	PXfloat fDirection;
	PXreal fDirectionX, fDirectionZ;
	PXreal fTargetX, fTargetZ;
	PXfloat fDotProduct;
	PXreal fBoundLeftX, fBoundLeftZ, fBoundRightX, fBoundRightZ;
	PXfloat fBoundLeft, fBoundRight;
	PXfloat fBoundLeftDotProduct, fBoundRightDotProduct;
	PXfloat fFieldOfView;

	// First calculate the direction we're looking in radians.
	fDirection = fLookingDirection * TWO_PI;

	// Now work out the vector of the target point relative to the character doing the looking.  Note that we
	// don't need the unit vector, as we're only interested in sign.
	fTargetX = fObservedX - fLookingX;
	fTargetZ = fObservedZ - fLookingZ;

	// If it is 180-degree, we can use the fast version.
	if (nFieldOfView == 180) {
		// The line that demarks what can be seen and what is behind the character is normal to the direction
		// they are looking (180-degree vision, don't forget).  This is good, because it is actually the normal
		// that we need to calculate which side of the demarkation line the target point is.  The unit vector
		// of the direction we are looking is simply given by sine and cosine if you think about it.
		fDirectionX = (PXreal)PXsin((PXdouble)fDirection);
		fDirectionZ = (PXreal)PXcos((PXdouble)fDirection);

		// The sign of the dot product tells us whether or not the point can be seen.  Note that we don't have
		// to calculate the unit vector of the target point, since we are only interested in sign.  And the sign
		// is reversed because the Z axis is reversed.
		fDotProduct = fDirectionX * fTargetX + fDirectionZ * fTargetZ;

		// We'll treat a zero result as point-not-in-view, so characters will have a tiny bit less than
		// 180-degree vision.
		if (fDotProduct >= REAL_ZERO)
			return (TRUE8);
		else
			return (FALSE8);
	} else {
		// We must use the slower version for non-180 vision.  Work out the direction of the two bounding lines
		// on the field of view.
		fFieldOfView = (PXfloat)DEGREES_TO_RADIANS(nFieldOfView);

		fBoundLeft = fDirection + fFieldOfView / REAL_TWO;
		fBoundRight = fDirection - fFieldOfView / REAL_TWO;

		// Work out the unit vector of the normals to the bounding lines.
		fBoundLeftX = -(PXreal)PXcos((PXdouble)fBoundLeft);
		fBoundLeftZ = (PXreal)PXsin((PXdouble)fBoundLeft);

		fBoundRightX = (PXreal)PXcos((PXdouble)fBoundRight);
		fBoundRightZ = -(PXreal)PXsin((PXdouble)fBoundRight);

		// And work out dot product for target relative to field-of-view bounding lines.
		fBoundLeftDotProduct = fBoundLeftX * fTargetX + fBoundLeftZ * fTargetZ;
		fBoundRightDotProduct = fBoundRightX * fTargetX + fBoundRightZ * fTargetZ;

		// Check signs to see if target is in view.
		if ((fBoundLeftDotProduct > REAL_ZERO) && (fBoundRightDotProduct > REAL_ZERO))
			return (TRUE8);
		else
			return (FALSE8);
	}
}

} // End of namespace ICB
