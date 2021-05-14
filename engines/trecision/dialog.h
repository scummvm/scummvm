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

	void showChoices(uint16 i);
	void playChoice(uint16 i);

	int16 _curPos;
	int16 _lastPos;
	uint16 _dispChoice[MAXDISPCHOICES];
	uint16 _curDispChoice;
	DialogSubTitle _subTitles[MAXSUBTITLES];

public:
	DialogManager(TrecisionEngine *vm);
	~DialogManager();

	void dialogPrint(int x, int y, int c, const Common::String &txt);
	void updateChoices(int16 dmx, int16 dmy);
	void selectChoice(int16 dmx, int16 dmy);
	void playDialog(uint16 i);
	void afterChoice();
	void dialogHandler(int numFrame);
	void doDialog();
	bool showCharacterAfterDialog() const;

	void syncGameStream(Common::Serializer &ser);
	void loadData(Common::File *file);

	Dialog _dialog[MAXDIALOG];
	DialogChoice _choice[MAXCHOICE];

	uint16 _curDialog;
	uint16 _curChoice;
	uint16 _curSubTitle;
};
; // end of class

} // end of namespace
#endif

