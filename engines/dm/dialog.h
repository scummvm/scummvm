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

#define k0_DIALOG_SET_VIEWPORT 0
#define k1_DIALOG_SET_SCREEN   1
#define k2_DIALOG_SET_UNKNOWN  2
#define k1_ONE_CHOICE          1
#define k2_TWO_CHOICES         2
#define k4_FOUR_CHOICES        4
#define k0_DIALOG_CHOICE_NONE  0
#define k1_DIALOG_CHOICE_1     1
#define k2_DIALOG_CHOICE_2     2
#define k3_DIALOG_CHOICE_3     3
#define k4_DIALOG_CHOICE_4     4

class DialogMan {
	DMEngine *_vm;
public:
	uint16 _selectedDialogChoice; // @ G0335_ui_SelectedDialogChoice
	explicit DialogMan(DMEngine *vm);
	void dialogDraw(const char *msg1, const char *msg2, const char *choice1, const char *choice2,
						 const char *choice3, const char *choice4, bool screenDialog, bool clearScreen, bool fading); // @ F0427_DIALOG_Draw
	void printCenteredChoice(byte *bitmap, const char *str, int16 posX, int16 posY); // @ F0425_DIALOG_PrintCenteredChoice
	bool isMessageOnTwoLines(const char *str, char *part1, char *part2); // @ F0426_DIALOG_IsMessageOnTwoLines
	int16 getChoice(uint16 choiceCount, uint16 dialogSetIndex, int16 driveType, int16 automaticChoiceIfFlopyInDrive); // @ F0424_DIALOG_GetChoice
};

}
