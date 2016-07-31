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

#ifndef TITANIC_BARBOT_H
#define TITANIC_BARBOT_H

#include "titanic/npcs/true_talk_npc.h"
#include "titanic/messages/messages.h"

namespace Titanic {

class CBarbot : public CTrueTalkNPC {
	bool EnterRoomMsg(CEnterRoomMsg *msg);
private:
	static int _v0;
private:
	int _field108;
	int _field10C;
	int _field110;
	int _field114;
	int _field118;
	int _field11C;
	int _field120;
	int _field124;
	int _field128;
	int _field12C;
	int _field130;
	int _field134;
	int _field138;
	int _field13C;
	int _field140;
	int _field144;
	int _field148;
	int _field14C;
	int _field150;
	int _field154;
	int _field158;
	int _field15C;
	int _field160;
	int _field164;
	int _field168;
	int _field16C;
	int _field170;
	int _field174;
	int _field178;
	int _field17C;
	int _field180;
	int _field184;
	int _field188;
	int _field18C;
	int _field190;
	int _field194;
	int _field198;
	int _field19C;
	int _field1A0;
	int _field1A4;
	int _field1A8;
	int _field1AC;
	int _field1B0;
	int _field1B4;
	int _field1B8;
	int _field1BC;
	int _field1C0;
	int _field1C4;
	int _field1C8;
	int _field1CC;
	int _field1D0;
	int _field1D4;
	int _field1D8;
	int _field1E0;
	int _field1E4;
	int _field1E8;
	int _field1EC;
	int _field1F0;
	int _field1F4;
	int _field1F8;
	int _field1FC;
	int _field200;
	int _field204;
	int _field208;
	int _field20C;
	int _field210;
	int _field214;
	int _field218;
	int _field21C;
	int _field220;
	int _field224;
	int _field228;
	int _field22C;
	int _field230;
	int _field234;
	int _field238;
	int _field23C;
	int _field240;
	int _field244;
	int _field248;
	int _field24C;
	int _field250;
	int _field254;
	int _field258;
	int _field25C;
	int _field260;
	int _field264;
	int _field268;
	int _field26C;
	int _field270;
	int _field274;
	int _field278;
	int _field27C;
	int _field280;
	int _field284;
	int _field288;
	int _field28C;
	int _field290;
	int _field294;
	int _field298;
	int _field29C;
	int _field2A0;
	int _field2A4;
	int _field2A8;
	int _field2AC;
	int _field2B0;
	int _field2B4;
	int _field2B8;
	int _field2BC;
	int _field2C0;
	int _field2C4;
	int _field2C8;
	int _field2CC;
	int _field2D0;
	int _field2D4;
	int _field2D8;
	int _field2E0;
	int _field2E4;
	int _field2E8;
	int _field2EC;
	int _field2F0;
	int _field2F4;
	int _field2F8;
	int _field2FC;
	int _field300;
	int _field304;
	int _field308;
	int _field30C;
	int _field310;
	int _field314;
	int _field318;
	int _field31C;
	int _field320;
	int _field324;
	int _field328;
	int _field32C;
	int _field330;
	int _field334;
	int _field338;
	int _field33C;
	int _field340;
public:
	CLASSDEF;
	CBarbot();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

} // End of namespace Titanic

#endif /* TITANIC_BARBOT_H */
