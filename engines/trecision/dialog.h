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

#ifndef TRECISION_DIALOG_H
#define TRECISION_DIALOG_H

#include "trecision/trecision.h"

namespace Trecision {
class TrecisionEngine;

class DialogManager {
	TrecisionEngine *_vm;

	void ShowChoices(uint16 i);
	void PlayChoice(uint16 i);

public:
	DialogManager(TrecisionEngine *vm);
	~DialogManager();

	void DialogPrint(int x, int y, int c, const char *txt);
	void UpdateChoices(int16 dmx, int16 dmy);
	void SelectChoice(int16 dmx, int16 dmy);
	void PlayDialog(uint16 i);
	void afterChoice(int numframe);
	void DialogHandler(int numframe);
	void doDialog();

	Dialog _dialog[MAXDIALOG];
	uint16 _curDialog, _curChoice;
	DialogSubTitle _subTitles[MAXSUBTITLES];
	uint16 _curSubTitle;
	uint16 DispChoice[MAXDISPCHOICES];
	uint16 CurDispChoice;
	int16 CurPos;
	int16 LastPos;

};
; // end of class

} // end of namespace
#endif

