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

#include "engines/icb/p4.h"
#include "engines/icb/common/px_globalvariables.h"

namespace ICB {

CpxGlobalScriptVariables::CpxGlobalScriptVariables() {
	m_no_vars = 0;
	m_sorted = 0;

	int i;
	for (i = 0; i < MAX_global_vars; i++) {
		m_vars[i].hash = 666;
		m_vars[i].value = 666;
		m_varInit[i] = GLOBAL_VAR_NOT_SET;
	}
}

int32 CpxGlobalScriptVariables::FindVariable(uint32 hash) {
	int index = -1;

	if (m_sorted == 0) {
		SortVariables();
	} else {
		// Use binary search system to find the variables
		// The variables are stored in ascending hash sorted order
		int min = 0;
		int max = m_no_vars;
		index = ((max - min) >> 1);
		CpxVariable *pvar = m_vars + index;

		// Start at the middle of the list
		while (pvar->hash != hash) {
			// Not found
			if ((index == min) || (index == max)) {
				index = -1;
				break;
			}

			// Is it further up the table ?
			if (hash > pvar->hash) {
				min = index;
				// Go down from min so we creep towards maximum
				index = max - ((max - min) >> 1);
			} else {
				max = index;
				// Go up from min so we creep towards minimum
				index = min + ((max - min) >> 1);
			}
			pvar = m_vars + index;

			if (max == min)
				Fatal_error("GlobalVars Binary search failed max==min %d number %d", max, m_no_vars);
		}

//#define CHECK_BINARY_SEARCH
#ifdef CHECK_BINARY_SEARCH
		uint i;
		int32 oldi = 0;
		// Check the binarry search worked
		for (i = 0; i < m_no_vars; i++) {
			if (m_vars[i].hash == hash) {
				oldi = i;
				break;
			}
		}
		if (oldi != index) {
			Fatal_error("Binary search failed");
		}
#endif
	}

	return index;
}

void CpxGlobalScriptVariables::InitVariable(uint32 hash, int32 value, const char *name) {
	// If the variable exists then it has already been initialised

	int32 i = FindVariable(hash);
	if (i == -1) {
		m_vars[m_no_vars].hash = hash;
		m_vars[m_no_vars].value = value;
		if (name) {
			Tdebug("gtable.txt", "%s , %d , 0x%X", name, value, hash);
		}
		m_no_vars++;
		// The list is no int32er sorted !
		m_sorted = 0;
	} else {
		// The variable has been set, so initing it is an error
		//		Fatal_error( "Global variable with hash 0x%08x has already been initialised",hash); // Game engine error
		m_vars[i].value = value;
	}
}

void CpxGlobalScriptVariables::SetVariable(uint32 hash, int32 value) {
	// Set a variable
	// You can't set a variable that has not been initialised
	// Has the variable been defined already
	int32 i = FindVariable(hash);
	if (i != -1) {
		// Once a variable is set then flag it as such
		m_varInit[i] = GLOBAL_VAR_SET;

		// Ok, just return the value
		m_vars[i].value = value;
	} else {
		// The variable hasn't been set, so accessing it is an error
		Fatal_error("SetVariable::Global variable with hash 0x%08x has been accessed before it was initialised", hash); // Game engine error
	}
}

// warn = 1
// Give a warning if someone Get's the value before Setting it
// i.e. they are relying on the initialisation code which is bad
// warn = 0
// Don't give the warning - this is needed for save games which Get all global's !
int32 CpxGlobalScriptVariables::GetVariable(uint32 hash, const char *name, int warn) {
	// Has the variable been defined already
	int32 i = FindVariable(hash);
	if (i != -1) {
		if (warn == 1) {
			// Give a warning if someone Get's the value before Setting it
			// i.e. they are relying on the initialisation code which is bad
			// Once a variable is set then flag it as such
			if (m_varInit[i] == GLOBAL_VAR_NOT_SET) {
				// Only give the warning once
				m_varInit[i] = GLOBAL_VAR_SET;

				if (name) {
					{ Message_box("GetVariable::Global variable '%s' hash 0x%08X value %d accessed before it was set", name, hash, m_vars[i].value); }
				} else {
					{ Message_box("GetVariable::Global variable hash 0x%08X value %d accessed before it was set", hash, m_vars[i].value); }
				}
			}
		}

		// Ok, just return the value
		return m_vars[i].value;
	} else {
		// The variable hasn't been set, so accessing it is an error
		Fatal_error("GetVariable::Global variable with hash 0x%08X has been accessed before it was initialised", hash); // Game engine error
		return 0;
	}
}

//
// Sort the variables so searching for them is quicker !
// The search method can then use a binary chop system
//
void CpxGlobalScriptVariables::SortVariables() {
	uint32 i;
	uint32 j;
	CpxVariable temp;
	uint8 temp8;

	// Sort the variable in ascending order
	// e.g. lowest -> highest
	for (i = 0; i < m_no_vars; i++) {
		for (j = i + 1; j < m_no_vars; j++) {
			if (m_vars[i].hash > m_vars[j].hash) {
				temp = m_vars[i];
				m_vars[i] = m_vars[j];
				m_vars[j] = temp;

				temp8 = m_varInit[i];
				m_varInit[i] = m_varInit[j];
				m_varInit[j] = temp8;
			}
		}
	}
	// The list is now sorted
	m_sorted = 1;
}

} // End of namespace ICB
