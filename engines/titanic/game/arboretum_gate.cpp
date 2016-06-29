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

#include "titanic/game/arboretum_gate.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CArboretumGate, CBackground)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

int CArboretumGate::_v1;
int CArboretumGate::_v2;
int CArboretumGate::_v3;

CArboretumGate::CArboretumGate() : CBackground() {
	_string1 = "NULL";
	_string2 = "NULL";
	_fieldE0 = 0;
	_fieldF0 = 0;
	_fieldF4 = 244;
	_fieldF8 = 304;
	_fieldFC = 122;
	_field100 = 182;
	_field104 = 183;
	_field108 = 243;
	_field10C = 665;
	_field110 = 724;
	_field114 = 61;
	_field118 = 121;
	_field11C = 0;
	_field120 = 60;
	_field124 = 485;
	_field128 = 544;
	_field12C = 425;
	_field130 = 484;
	_field134 = 545;
	_field138 = 604;
	_field13C = 605;
	_field140 = 664;
	_field144 = 305;
	_field148 = 364;
	_field14C = 365;
	_field150 = 424;
}

void CArboretumGate::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_v3, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_fieldF4, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_fieldFC, indent);
	file->writeNumberLine(_field100, indent);
	file->writeNumberLine(_field104, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field10C, indent);
	file->writeNumberLine(_field110, indent);
	file->writeNumberLine(_field114, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);
	file->writeNumberLine(_field128, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);
	file->writeNumberLine(_field134, indent);
	file->writeNumberLine(_field138, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_field140, indent);
	file->writeNumberLine(_field144, indent);
	file->writeNumberLine(_field148, indent);
	file->writeNumberLine(_field14C, indent);
	file->writeNumberLine(_field150, indent);
	file->writeQuotedLine(_string2, indent);

	CBackground::save(file, indent);
}

void CArboretumGate::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_v3 = file->readNumber();
	_string1 = file->readString();
	_fieldF0 = file->readNumber();
	_fieldF4 = file->readNumber();
	_fieldF8 = file->readNumber();
	_fieldFC = file->readNumber();
	_field100 = file->readNumber();
	_field104 = file->readNumber();
	_field108 = file->readNumber();
	_field10C = file->readNumber();
	_field110 = file->readNumber();
	_field114 = file->readNumber();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_field120 = file->readNumber();
	_field124 = file->readNumber();
	_field128 = file->readNumber();
	_field12C = file->readNumber();
	_field130 = file->readNumber();
	_field134 = file->readNumber();
	_field138 = file->readNumber();
	_field13C = file->readNumber();
	_field140 = file->readNumber();
	_field144 = file->readNumber();
	_field148 = file->readNumber();
	_field14C = file->readNumber();
	_field150 = file->readNumber();
	_string2 = file->readString();

	CBackground::load(file);
}

bool CArboretumGate::ActMsg(CActMsg *msg) { return false; }
bool CArboretumGate::LeaveViewMsg(CLeaveViewMsg *msg) { return false; }
bool CArboretumGate::TurnOff(CTurnOff *msg) { return false; }
bool CArboretumGate::MouseButtonDownMsg(CMouseButtonDownMsg *msg) { return false; }

bool CArboretumGate::EnterViewMsg(CEnterViewMsg *msg) {
	warning("CArboretumGate::handleEvent");
	return false;
}

bool CArboretumGate::TurnOn(CTurnOn *msg) { return false; }
bool CArboretumGate::MovieEndMsg(CMovieEndMsg *msg) { return false; }

} // End of namespace Titanic
