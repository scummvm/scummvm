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

#ifndef BLADERUNNER_SCRIPT_KIA_H
#define BLADERUNNER_SCRIPT_KIA_H

#include "bladerunner/script/script.h"

namespace BladeRunner {

class BladeRunnerEngine;

class ScriptVK : ScriptBase {
public:
	ScriptVK(BladeRunnerEngine *vm)
		: ScriptBase(vm) {
	}

	bool SCRIPT_VK_DLL_Initialize(int a1);
	void SCRIPT_VK_DLL_Calibrate(int a1);
	bool SCRIPT_VK_DLL_Begin_Test();
	void SCRIPT_VK_DLL_McCoy_Asks_Question(int a1, int a2);
	void SCRIPT_VK_DLL_Question_Asked(int a1, int a2);
 	void SCRIPT_VK_DLL_Shutdown(int a1, signed int a2, signed int a3);

private:
	int unknown1;
	int unknown2;

	void sub_402604(int a1);
	void sub_404B44(int a1);
	void sub_406088(int a1);
	void sub_407CF8(int a1);
	void sub_40897C(int a1);
	void sub_40A300(int a1, int a2);
	void sub_40A350(int a1, int a2);
	void sub_40A3A0(int a1, int a2);
	void sub_40A3F0(int a1);
	void sub_40A470(int a1);
	void sub_40A510(int a1);
};

} // End of namespace BladeRunner

#endif

