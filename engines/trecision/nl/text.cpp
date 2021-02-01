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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"

namespace Trecision {

extern const char *_sysSent[];
/*-------------------------------------------------------------------------*/
/*                             INIT OBJ NAMES                              */
/*-------------------------------------------------------------------------*/
void initNames() {
	_sysSent[1] = "NightLong was not properly installed!\nRun Autorun.exe from the CD-Rom."; // llscreen
	_sysSent[2] = "Not enough memory!\nYou need %d bytes more.\n";                           // llscreen
	_sysSent[3] = "Unknown error\n";                                                         // maindos
	_sysSent[4] = "Please insert CD number %c and press return.";                            // llmouse
	_sysSent[5] = "Unknown error";                                                           // llscreen
	_sysSent[6] = "VESA Not Supported.\n";                                                   // maindos
	_sysSent[7] = "Error reading file.";                                                     // lldec
	_sysSent[8] = "Mouse not found error!\n";                                                // maindos
	_sysSent[9] = "SAVE POSITION";                                                           // llmouse ###
	_sysSent[10] = "EMPTY SLOT";                                                             // llmouse ###
	_sysSent[11] = "LOAD POSITION";                                                          // llmouse ###
	_sysSent[12] = "Error reading saved-game";                                               // llmouse
	_sysSent[13] = "Are you sure that you want to quit (y/n)?";                              // llmouse ###
	_sysSent[14] = "Unknown error\n";                                                        // soundw95
	_sysSent[15] = "Sample handle not available\n";                                          // sounddos
	_sysSent[16] = "Run NL to select an appropriate digital audio driver\n";                 // sounddos
	_sysSent[17] = "This demo is over.";                                                     // llmouse ###
	_sysSent[18] = "NightLong";                                                              // win name - mainw95
	_sysSent[19] = "ERROR!";                                                                 // llmouse
	_sysSent[20] = "Unsupported pixel format.";                                              // mainw95
	_sysSent[21] = "DirectX Error";                                                          // mainw95
	_sysSent[22] = "NightLong Warning";                                                      // mainw95
	_sysSent[23] = "Use ";                                                                   // string
	_sysSent[24] = " with ";                                                                 // string
	_sysSent[25] = "Go to ";                                                                 // string
	_sysSent[26] = "Go to ... ";                                                             // string
	// The following are missing: FastFile, Init, Do, Utility

	Sentence[0] = "          "; // Use it like a buffer !!!!
	ObjName[0] = " ";
}

} // End of namespace Trecision
