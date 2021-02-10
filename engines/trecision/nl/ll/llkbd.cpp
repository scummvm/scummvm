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

#include <stdio.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/SysDef.h"
#include "trecision/nl/Struct.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

#include <common/file.h>

#define KEY_BUFFER          0x60
#define KEY_CONTROL         0x61
#define INT_CONTROL         0x20
#define KEYBOARD_INT        0x09

// make and break codes for the arrow keys

#define MAKE_RIGHT      77
#define MAKE_LEFT       75
#define MAKE_UP         72
#define MAKE_DOWN       80
#define MAKE_A          30
#define MAKE_S          31
#define MAKE_D          32
#define MAKE_W          17
#define MAKE_PLUS       78
#define MAKE_MINUS      74
#define MAKE_B          48
#define MAKE_N          49
#define MAKE_ENTER      28
#define MAKE_P          25
#define MAKE_L          38

#define BREAK_RIGHT     205
#define BREAK_LEFT      203
#define BREAK_UP        200
#define BREAK_DOWN      208
#define BREAK_A         158
#define BREAK_S         159
#define BREAK_D         160
#define BREAK_W         145
#define BREAK_PLUS      206
#define BREAK_MINUS     202
#define BREAK_B         176
#define BREAK_ENTER     156

namespace Common {
class File;
}

namespace Trecision {

char *TextPtr;

/* -----------------19/01/98 11.11-------------------
 * 					GetNextSent
 * --------------------------------------------------*/
char *GetNextSent() {
	while (*TextPtr) {
		*TextPtr = ~(*TextPtr);
		TextPtr++;
	}

	TextPtr++;
	return ((char *)(TextPtr));
}

/* -----------------19/01/98 11.11-------------------
 * 						LoadAll
 * --------------------------------------------------*/
void LoadAll() {
	Common::File dataNl;
	if (!dataNl.open("DATA.NL"))
		error("LoadAll : Couldn't open DATA.NL");

	for (int i = 0; i < MAXROOMS; ++i) {
		for (int j = 0; j < 4; ++j)
			g_vm->_room[i]._baseName[j] = dataNl.readSByte();
		g_vm->_room[i]._flag = dataNl.readByte();
		dataNl.readByte(); // Padding
		g_vm->_room[i]._bkgAnim = dataNl.readUint16LE();
		for (int j = 0; j < MAXOBJINROOM; ++j)
			g_vm->_room[i]._object[j] = dataNl.readUint16LE();
		for (int j = 0; j < MAXSOUNDSINROOM; ++j)
			g_vm->_room[i]._sounds[j] = dataNl.readUint16LE();
		for (int j = 0; j < MAXACTIONINROOM; ++j)
			g_vm->_room[i]._actions[j] = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXOBJ; ++i) {
		g_vm->_obj[i]._dx = dataNl.readUint16LE();
		g_vm->_obj[i]._dy = dataNl.readUint16LE();
		g_vm->_obj[i]._px = dataNl.readUint16LE();
		g_vm->_obj[i]._py = dataNl.readUint16LE();

		for (int j = 0; j < 4; ++j)
			g_vm->_obj[i]._lim[j] = dataNl.readUint16LE();

		g_vm->_obj[i]._position = dataNl.readSByte();
		dataNl.readByte(); // Padding
		g_vm->_obj[i]._name = dataNl.readUint16LE();
		g_vm->_obj[i]._examine = dataNl.readUint16LE();
		g_vm->_obj[i]._action = dataNl.readUint16LE();
		g_vm->_obj[i]._goRoom = dataNl.readByte();
		g_vm->_obj[i]._nbox = dataNl.readByte();
		g_vm->_obj[i]._ninv = dataNl.readByte();
		g_vm->_obj[i]._mode = dataNl.readByte();
		g_vm->_obj[i]._flag = dataNl.readByte();
		dataNl.readByte(); // Padding
		g_vm->_obj[i]._anim = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXINVENTORY; ++i) {
		g_vm->_inventoryObj[i]._name = dataNl.readUint16LE();
		g_vm->_inventoryObj[i]._examine = dataNl.readUint16LE();
		g_vm->_inventoryObj[i]._action = dataNl.readUint16LE();
		g_vm->_inventoryObj[i]._flag = dataNl.readByte();
		dataNl.readByte(); // Padding
		g_vm->_inventoryObj[i]._anim = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXSAMPLE; ++i) {
		for (int j = 0; j < 14; ++j)
			GSample[i]._name[j] = dataNl.readByte();
		GSample[i]._volume = dataNl.readByte();
		GSample[i]._flag = dataNl.readByte();
		GSample[i]._panning = dataNl.readSByte();
	}

	for (int i = 0; i < MAXSCRIPTFRAME; ++i) {
		ScriptFrame[i]._class = dataNl.readByte();
		ScriptFrame[i]._event = dataNl.readByte();
		ScriptFrame[i]._byteParam = dataNl.readByte();
		dataNl.readByte(); // Padding
		ScriptFrame[i]._wordParam1 = dataNl.readUint16LE();
		ScriptFrame[i]._wordParam2 = dataNl.readUint16LE();
		ScriptFrame[i]._longParam = dataNl.readUint16LE();
		ScriptFrame[i]._noWait = !(dataNl.readByte() == 0);
		dataNl.readByte();
	}

	for (int i = 0; i < MAXSCRIPT; ++i) {
		Script[i]._firstFrame = dataNl.readUint16LE();
		Script[i]._flag = !(dataNl.readByte() == 0);
		dataNl.readByte();
	}

	for (int i = 0; i < MAXANIM; ++i) {
		for (int j = 0; j < 14; ++j)
			AnimTab[i]._name[j] = dataNl.readByte();

		AnimTab[i]._flag = dataNl.readUint16LE();

		for (int j = 0; j < MAXCHILD; ++j) {
			AnimTab[i]._lim[j][0] = dataNl.readUint16LE();
			AnimTab[i]._lim[j][1] = dataNl.readUint16LE();
			AnimTab[i]._lim[j][2] = dataNl.readUint16LE();
			AnimTab[i]._lim[j][3] = dataNl.readUint16LE();
		}

		AnimTab[i]._nbox = dataNl.readByte();
		dataNl.readByte(); // Padding

		for (int j = 0; j < MAXATFRAME; ++j) {
			AnimTab[i]._atFrame[j]._type = dataNl.readByte();
			AnimTab[i]._atFrame[j]._child = dataNl.readByte();
			AnimTab[i]._atFrame[j]._numFrame = dataNl.readUint16LE();
			AnimTab[i]._atFrame[j]._index = dataNl.readUint16LE();
		}
	}

	for (int i = 0; i < MAXDIALOG; ++i) {
		_dialog[i]._flag = dataNl.readUint16LE();
		_dialog[i]._interlocutor = dataNl.readUint16LE();

		for (int j = 0; j < 14; ++j)
			_dialog[i]._startAnim[j] = dataNl.readByte();

		_dialog[i]._startLen = dataNl.readUint16LE();
		_dialog[i]._firstChoice = dataNl.readUint16LE();
		_dialog[i]._choiceNumb = dataNl.readUint16LE();

		for (int j = 0; j < MAXNEWSMKPAL; ++j)
			_dialog[i]._newPal[j] = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXCHOICE; ++i) {
		_choice[i]._flag = dataNl.readUint16LE();
		_choice[i]._sentenceIndex = dataNl.readUint16LE();
		_choice[i]._firstSubTitle = dataNl.readUint16LE();
		_choice[i]._subTitleNumb = dataNl.readUint16LE();

		for (int j = 0; j < MAXDISPSCELTE; ++j)
			_choice[i]._on[j] = dataNl.readUint16LE();

		for (int j = 0; j < MAXDISPSCELTE; ++j)
			_choice[i]._off[j] = dataNl.readUint16LE();

		_choice[i]._startFrame = dataNl.readUint16LE();
		_choice[i]._nextDialog = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXSUBTITLES; ++i) {
		_subTitles[i]._sentence = dataNl.readUint16LE();
		_subTitles[i]._x = dataNl.readUint16LE();
		_subTitles[i]._y = dataNl.readUint16LE();
		_subTitles[i]._color = dataNl.readUint16LE();
		_subTitles[i]._startFrame = dataNl.readUint16LE();
		_subTitles[i]._length = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXACTION; ++i)
		g_vm->_actionLen[i] = dataNl.readByte();

	NumFileRef = dataNl.readSint32LE();

	for (int i = 0; i < NumFileRef; ++i) {
		for (int j = 0; j < 12; ++j)
			FileRef[i].name[j] = dataNl.readByte();

		FileRef[i].offset = dataNl.readSint32LE();
	}

	dataNl.read(TextArea, MAXTEXTAREA);

	TextPtr = (char *)TextArea;

	for (int a = 0; a < MAXOBJNAME; a++)
		g_vm->_objName[a] = GetNextSent();

	for (int a = 0; a < MAXSENTENCE; a++)
		g_vm->_sentence[a] = GetNextSent();

	for (int a = 0; a < MAXSYSTEXT; a++)
		g_vm->_sysText[a] = GetNextSent();

	dataNl.close();
}

} // End of namespace Trecision
