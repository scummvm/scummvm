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

#ifndef ADL_HIRES0_H
#define ADL_HIRES0_H

#include "adl/adl_v2.h"

namespace Adl {

#define IDS_HR0_DISK_IMAGE "MISSION.NIB"

#define IDI_HR0_NUM_ROOMS 43
#define IDI_HR0_NUM_MESSAGES 142
#define IDI_HR0_NUM_VARS 40
#define IDI_HR0_NUM_ITEM_PICS 2
#define IDI_HR0_NUM_ITEM_OFFSETS 16

// Messages used outside of scripts
#define IDI_HR0_MSG_CANT_GO_THERE      110
#define IDI_HR0_MSG_DONT_UNDERSTAND    112
#define IDI_HR0_MSG_ITEM_DOESNT_MOVE   114
#define IDI_HR0_MSG_ITEM_NOT_HERE      115
#define IDI_HR0_MSG_THANKS_FOR_PLAYING 113

class HiRes0Engine : public AdlEngine_v2 {
public:
	HiRes0Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v2(syst, gd) { }
	~HiRes0Engine() { }

private:
	// AdlEngine
	void init();
	void initGameState();
};

} // End of namespace Adl

#endif
