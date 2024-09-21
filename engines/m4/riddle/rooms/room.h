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

#ifndef M4_RIDDLE_ROOMS_ROOM_H
#define M4_RIDDLE_ROOMS_ROOM_H

#include "m4/core/rooms.h"
#include "m4/adv_r/conv_io.h"
#include "m4/core/imath.h"
#include "m4/graphics/gr_series.h"
#include "m4/adv_r/adv_hotspot.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room : public M4::Room {
private:
	static int _ripSketching;

protected:
	static void intrMsgNull(frac16 myMessage, machine *sender) {}
	static void triggerMachineByHashCallback(frac16 myMessage, machine *sender = nullptr);
	static void triggerMachineByHashCallbackNegative(frac16 myMessage, machine *sender = nullptr);
	static void triggerMachineByHashCallback3000(frac16 myMessage, machine *sender = nullptr);
	static void triggerMachineByHashCallbackAlways(frac16 myMessage, machine *sender = nullptr);

	void restoreAutosave();

	/**
	 * Checks various game flags for updates
	 * @param flag		If set, does extra checks
	 * @return	A count of the flag changes done
	 */
	int checkFlags(bool flag);

	void setFlag45();

	/**
	 * Sets a bunch of globals. Sooooo many globals.
	 */
	void setGlobals1(int val1, int val2, int val3, int val4, int val5,
		int val6 = 0, int val7 = 0, int val8 = 0, int val9 = 0, int val10 = 0,
		int val11 = 0, int val12 = 0, int val13 = 0, int val14 = 0, int val15 = 0,
		int val16 = 0, int val17 = 0, int val18 = 0, int val19 = 0, int val20 = 0,
		int val21 = 0);
	void setGlobals3(int series, int val1, int val2);
	void setGlobals4(int val1, int val2, int val3, int val4);

	void sendWSMessage_10000(int val1, machine *recv, int val2, int val3,
		int val4, int trigger, int val9, int val6, int val7, int val8);
	void sendWSMessage_10000(machine *recv, int val1, int val2, int val3,
		int trigger, int val4);

	machine *triggerMachineByHash_3000(int val1, int val2, const int16 *normalDirs,
		const int16 *shadowDirs, int val3, int val4, int val5,
		MessageCB intrMsg, const char *machName);
	void sendWSMessage_60000(machine *mach);
	void sendWSMessage_80000(machine *mach);
	void sendWSMessage_B0000(machine *mach, int trigger);
	void sendWSMessage_B0000(int trigger);
	void sendWSMessage_C0000(machine *mach, int trigger);
	void sendWSMessage_C0000(int trigger);
	void sendWSMessage_D0000(machine *mach);
	void sendWSMessage_D0000();
	void sendWSMessage_E0000(machine *mach);
	void sendWSMessage_E0000();
	void sendWSMessage_F0000(machine *mach, int trigger);
	void sendWSMessage_F0000(int trigger);
	void sendWSMessage_110000(machine *mach, int trigger);
	void sendWSMessage_110000(int trigger);
	void sendWSMessage_120000(int trigger);
	void sendWSMessage_120000(machine *mach, int trigger);
	void sendWSMessage_130000(machine *recv, int val1);
	void sendWSMessage_130000(int val1);
	void sendWSMessage_140000(machine *mach, int trigger);
	void sendWSMessage_140000(int trigger);
	void sendWSMessage_150000(machine *mach, int trigger);
	void sendWSMessage_150000(int trigger);
	void sendWSMessage_160000(machine *mach, int val1, int trigger);
	void sendWSMessage_160000(int val1, int trigger);
	void sendWSMessage_180000(machine *recv, int trigger);
	void sendWSMessage_180000(int trigger);
	void sendWSMessage_190000(machine *recv, int trigger);
	void sendWSMessage_190000(int trigger);
	void sendWSMessage_1a0000(machine *recv, int trigger);
	void sendWSMessage_1e0000(machine *recv, int val1, int val2);
	void sendWSMessage_1e0000(int val1, int val2);
	void sendWSMessage_1f0000(machine *recv, int val1, int val2);
	void sendWSMessage_1f0000(int val1, int val2);
	void sendWSMessage_200000(machine *recv, int trigger);
	void sendWSMessage_210000(machine *recv, int trigger);

	void sendWSMessage_29a0000(machine *recv, int val1);
	void sendWSMessage_29a0000(int val1);
	void sendWSMessage_multi(const char *name);

	/**
	 * Get the number of key items placed in room 305 (display room)
	 */
	int getNumKeyItemsPlaced() const;

	bool setItemsPlacedFlags();
	const char *getItemsPlacedDigi() const;

	/**
	 * Sets all the hotspots to be inactive
	 */
	void disableHotspots();

	/**
	 * Sets all the hotspots to be active
	 */
	void enableHotspots();

	bool checkStrings() const;

public:
	Room() : M4::Room() {}
	~Room() override {}

	void preload() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
