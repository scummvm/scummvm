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

#ifndef M4_RIDDLE_ROOMS_SECTION3_ROOM305_H
#define M4_RIDDLE_ROOMS_SECTION3_ROOM305_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room305 : public Room {
private:
	bool _showWalker = false;
	bool _drawerOpen = false;
	bool _hideCartoon = false;
	bool _unused = false;
	int _ripleyMode = 0;
	int _ripleyShould = 0;
	int _fengMode = 0;
	int _fengShould = 0;
	int _trigger1 = -1;
	KernelTriggerType _triggerMode1 = KT_DAEMON;
	KernelTriggerType _triggerMode2 = KT_DAEMON;
	int _ripMedHigh = 0;
	int _ripLooksDown = 0;
	machine *_turtle = nullptr;
	int _shrunkenHead1 = 0;
	int _incenseHolder1 = 0;
	int _crystalSkull1 = 0;
	int _whaleboneHorn1 = 0;
	int _wheeledToy1 = 0;
	int _butterfly1 = 0;
	int _amulet1 = 0;
	int _knife1 = 0;
	int _banknote1 = 0;
	int _stamp1 = 0;
	int _map1 = 0;
	int _emerald1 = 0;
	machine *_shrunkenHead2 = nullptr;
	machine *_incenseHolder2 = nullptr;
	machine *_crystalSkull2 = nullptr;
	machine *_whaleboneHorn2 = nullptr;
	machine *_wheeledToy2 = nullptr;
	machine *_butterfly2 = nullptr;
	machine *_amulet2 = nullptr;
	machine *_knife2 = nullptr;
	machine *_banknote2 = nullptr;
	machine *_stamp2 = nullptr;
	machine *_map2 = nullptr;
	machine *_emerald2 = nullptr;

	int _easterIslandCartoon = 0;
	int _chinshiCartoon = 0;
	int _tabletsCartoon = 0;
	int _epitaphCartoon = 0;
	int _graveyardCartoon = 0;
	int _castleCartoon = 0;
	int _mocaMocheCartoon = 0;
	int _templeCartoon = 0;
	int _emeraldCartoon = 0;
	int _jellyBeans = 0;
	int _feng1 = 0;
	int _feng2 = 0;
	int _feng3 = 0;
	int _rip1 = 0;
	int _rip2 = 0;
	int _rip3 = 0;
	int _rip4 = 0;
	int _suit1 = 0;
	int _suit2 = 0;
	int _suit3 = 0;
	machine *_stander = nullptr;
	machine *_shadow5 = nullptr;
	machine *_rip5 = nullptr;
	machine *_rip6 = nullptr;
	machine *_openDrawer = nullptr;
	machine *_openDrawerTreats = nullptr;
	machine *_cartoonMach = nullptr;
	int _cartoon = 0;
	int _lookUp = 0;

	void setupSign();
	void setShadow5(bool active);
	void conv305a();
	bool walkToObject();
	int getXAreaNum() const;
	const char *getXAreaCartoon() const;
	const char *getXAreaSeries() const;
	Common::String getXAreaDigi() const;
	Common::String getXAreaDigi2() const;
	int getItemX(int seriesHash) const;
	int getItemY(int seriesHash) const;
	void openDrawer();
	void closeDrawer();
	void takeTurtleTreats();

public:
	Room305() : Room() {}
	~Room305() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
	void syncGame(Common::Serializer &s) override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
