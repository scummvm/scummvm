/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_RIDDLE_TRIGGERS_H
#define M4_RIDDLE_TRIGGERS_H

#include "m4/wscript/ws_machine.h"

namespace M4 {
namespace Riddle {

extern void sendWSMessage_multi(const char *name);

/**
 * Sets a bunch of globals. Sooooo many globals.
 */
extern void setGlobals1(int series, int val2, int val3, int val4, int val5,
	int val6 = 0, int val7 = 0, int val8 = 0, int val9 = 0, int val10 = 0,
	int val11 = 0, int val12 = 0, int val13 = 0, int val14 = 0, int val15 = 0,
	int val16 = 0, int val17 = 0, int val18 = 0, int val19 = 0, int val20 = 0,
	int val21 = 0);
extern void setGlobals3(int series, int val1, int val2);
extern void setGlobals4(int series, int val2, int val3, int val4);

extern void sendWSMessage_10000(int val1, machine *mach, int val2, int val3,
	int val4, int trigger, int series2, int val6, int val7, int val8);
extern void sendWSMessage_10000(machine *mach, int destX, int destY, int facing,
	int trigger, int complete_walk);

machine *triggerMachineByHash_3000(int myHash, int dataHash,
	int normalDir, int shadowDir, int param1, int param2,
	int index, MessageCB intrMsg, const char *machName);
extern void sendWSMessage_60000(machine *mach);
extern void sendWSMessage_80000(machine *mach);
extern void sendWSMessage_B0000(machine *mach, int trigger);
extern void sendWSMessage_B0000(int trigger);
extern void sendWSMessage_C0000(machine *mach, int trigger);
extern void sendWSMessage_C0000(int trigger);
extern void sendWSMessage_D0000(machine *mach);
extern void sendWSMessage_D0000();
extern void sendWSMessage_E0000(machine *mach);
extern void sendWSMessage_E0000();
extern void sendWSMessage_F0000(machine *mach, int trigger);
extern void sendWSMessage_F0000(int trigger);
extern void sendWSMessage_110000(machine *mach, int trigger);
extern void sendWSMessage_110000(int trigger);
extern void sendWSMessage_120000(int trigger);
extern void sendWSMessage_120000(machine *mach, int trigger);
extern void sendWSMessage_130000(machine *recv, int val1);
extern void sendWSMessage_130000(int val1);
extern void sendWSMessage_140000(machine *mach, int trigger);
extern void sendWSMessage_140000(int trigger);
extern void sendWSMessage_150000(machine *mach, int trigger);
extern void sendWSMessage_150000(int trigger);
extern void sendWSMessage_160000(machine *mach, int val1, int trigger);
extern void sendWSMessage_160000(int val1, int trigger);
extern void sendWSMessage_180000(machine *recv, int trigger);
extern void sendWSMessage_180000(int trigger);
extern void sendWSMessage_190000(machine *recv, int trigger);
extern void sendWSMessage_190000(int trigger);
extern void sendWSMessage_1a0000(machine *recv, int trigger);
extern void sendWSMessage_1e0000(machine *recv, int val1, int val2);
extern void sendWSMessage_1e0000(int val1, int val2);
extern void sendWSMessage_1f0000(machine *recv, int val1, int val2);
extern void sendWSMessage_1f0000(int val1, int val2);
extern void sendWSMessage_200000(machine *recv, int trigger);
extern void sendWSMessage_210000(machine *recv, int trigger);

extern void sendWSMessage_29a0000(machine *recv, int val1);
extern void sendWSMessage_29a0000(int val1);

extern void intrMsgNull(frac16 myMessage, machine *sender);
extern void triggerMachineByHashCallback(frac16 myMessage, machine *sender = nullptr);
extern void triggerMachineByHashCallback3000(frac16 myMessage, machine *sender = nullptr);
extern void triggerMachineByHashCallbackAlways(frac16 myMessage, machine *sender = nullptr);

} // namespace Riddle
} // namespace M4

#endif
