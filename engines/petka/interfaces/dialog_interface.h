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

#ifndef PETKA_DIALOG_INTERFACE_H
#define PETKA_DIALOG_INTERFACE_H

#include "common/str.h"

namespace Petka {

class QMessageObject;

class DialogInterface {
public:
	DialogInterface();

	void start(uint a, QMessageObject *sender);

	void saveCursorState();
	void restoreCursorState();

	void sub_4155D0(int a);

public:
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
	int _field18;
	int _field24;
	Common::String _soundName;
	int16 _savedCursorActType;
	int16 _savedCursorId;
	bool _wasCursorShown;
	bool _wasCursorAnim;
	QMessageObject *_talker;
	QMessageObject *_sender;

};

} // End of namespace Petka

#endif