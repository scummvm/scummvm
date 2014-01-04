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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VOYEUR_DEBUGGER_H
#define VOYEUR_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace Voyeur {

class VoyeurEngine;

class Debugger : public GUI::Debugger {
private:
	VoyeurEngine *_vm;
public:
	/**
	 * Specifies whether time should pass, and the video camera's batteries go down
	 * @default true
	 */
	bool _isTimeActive;
protected:
	bool Cmd_Time(int argc, const char **argv);

public:
	Debugger();
	virtual ~Debugger() {}
	void setVm(VoyeurEngine *vm) { _vm = vm; }

};

} // End of namespace Voyeur

#endif
