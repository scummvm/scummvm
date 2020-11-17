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

#ifndef AGDS_PROCESS_EXIT_CODE_H
#define AGDS_PROCESS_EXIT_CODE_H

namespace AGDS {

	enum ProcessExitCode {
		kExitCodeDestroy						= 2,
		kExitCodeSuspend						= 5,
		kExitCodeSetNextScreen					= 6,
		kExitCodeSetNextScreenSaveInHistory		= 7,
		kExitCodeLoadScreenObject				= 8,
		kExitCodeLoadScreenObjectAs				= 9,
		kExitCodeLoadInventoryObject			= 10,
		kExitCodeMouseAreaChange				= 11,
		kExitCodeRunDialog						= 12,
		kExitCodeCreatePatchLoadResources		= 13,
		kExitCodeLoadGame						= 14,
		kExitCodeExitScreen						= 15,
		kExitCodeCloseInventory					= 16,
		kExitCodeSaveGame						= 17,

		kExitCodeLoadPreviousScreenObject		= 99
	};

}

#endif
