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

#include "dm.h"


namespace DM {

class DialogMan {
	DMEngine *_vm;
public:
	uint16 _g335_selectedDialogChoice; // @ G0335_ui_SelectedDialogChoice
	explicit DialogMan(DMEngine *vm);
	void f427_dialogDraw(char *msg1, char *msg2, char *choice1, char *choice2,
						 char *choice3, char *choice4, bool screenDialog, bool clearScreen, bool fading); // @ F0427_DIALOG_Draw
	void f425_printCenteredChoice(byte *bitmap, char *str, int16 posX, int16 posY); // @ F0425_DIALOG_PrintCenteredChoice
	bool f426_isMessageOnTwoLines(char *str, char *part1, char *part2); // @ F0426_DIALOG_IsMessageOnTwoLines
};

}