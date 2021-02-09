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
#include "trecision/trecision.h"

namespace Trecision {

/*-------------------------------------------------------------------------*/
/*                             INIT OBJ NAMES                              */
/*-------------------------------------------------------------------------*/
void initNames() {
	g_vm->_sysSentence[1] = "NightLong was not properly installed!\nRun Autorun.exe from the CD-Rom."; // llscreen
	g_vm->_sysSentence[2] = "Not enough memory!\nYou need %d bytes more.\n";                           // llscreen
	g_vm->_sysSentence[3] = "Unknown error\n";                                                         // maindos
	g_vm->_sysSentence[4] = "Please insert CD number %c and press return.";                            // llmouse
	g_vm->_sysSentence[5] = "Unknown error";                                                           // llscreen
	g_vm->_sysSentence[6] = "VESA Not Supported.\n";                                                   // maindos
	g_vm->_sysSentence[7] = "Error reading file.";                                                     // lldec
	g_vm->_sysSentence[8] = "Mouse not found error!\n";                                                // maindos
	g_vm->_sysSentence[9] = "SAVE POSITION";                                                           // llmouse ###
	g_vm->_sysSentence[10] = "EMPTY SLOT";                                                             // llmouse ###
	g_vm->_sysSentence[11] = "LOAD POSITION";                                                          // llmouse ###
	g_vm->_sysSentence[12] = "Error reading saved-game";                                               // llmouse
	g_vm->_sysSentence[13] = "Are you sure that you want to quit (y/n)?";                              // llmouse ###
	g_vm->_sysSentence[14] = "Unknown error\n";                                                        // soundw95
	g_vm->_sysSentence[15] = "Sample handle not available\n";                                          // sounddos
	g_vm->_sysSentence[16] = "Run NL to select an appropriate digital audio driver\n";                 // sounddos
	g_vm->_sysSentence[17] = "This demo is over.";                                                     // llmouse ###
	g_vm->_sysSentence[18] = "NightLong";                                                              // win name - mainw95
	g_vm->_sysSentence[19] = "ERROR!";                                                                 // llmouse
	g_vm->_sysSentence[20] = "Unsupported pixel format.";                                              // mainw95
	g_vm->_sysSentence[21] = "DirectX Error";                                                          // mainw95
	g_vm->_sysSentence[22] = "NightLong Warning";                                                      // mainw95
	g_vm->_sysSentence[23] = "Use ";                                                                   // string
	g_vm->_sysSentence[24] = " with ";                                                                 // string
	g_vm->_sysSentence[25] = "Go to ";                                                                 // string
	g_vm->_sysSentence[26] = "Go to ... ";                                                             // string
	// The following are missing: FastFile, Init, Do, Utility

	g_vm->_sentence[0] = "          "; // Use it like a buffer !!!!
	g_vm->_objName[0] = " ";
}

} // End of namespace Trecision
