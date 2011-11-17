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

#include "dreamweb/dreamweb.h"

namespace DreamGen {


typedef void (DreamGenContext::*UseCallback)(void);

// Note: The callback pointer has been placed before the
// ID to keep MSVC happy (otherwise, it throws warnings
// that alignment of a member was sensitive to packing)
struct UseListEntry {
	UseCallback callback;
	const char *id;
};

void DreamGenContext::useroutine() {

	static const UseListEntry kUseList[] = {
		{ &DreamGenContext::usemon,            "NETW" },
		{ &DreamGenContext::useelevator1,      "ELVA" },
		{ &DreamGenContext::useelevator2,      "ELVB" },
		{ &DreamGenContext::useelevator3,      "ELVC" },
		{ &DreamGenContext::useelevator4,      "ELVE" },
		{ &DreamGenContext::useelevator5,      "ELVF" },
		{ &DreamGenContext::usechurchgate,     "CGAT" },
		{ &DreamGenContext::usestereo,         "REMO" },
		{ &DreamGenContext::usebuttona,        "BUTA" },
		{ &DreamGenContext::usewinch,          "CBOX" },
		{ &DreamGenContext::uselighter,        "LITE" },
		{ &DreamGenContext::useplate,          "PLAT" },
		{ &DreamGenContext::usecontrol,        "LIFT" },
		{ &DreamGenContext::usewire,           "WIRE" },
		{ &DreamGenContext::usehandle,         "HNDL" },
		{ &DreamGenContext::usehatch,          "HACH" },
		{ &DreamGenContext::useelvdoor,        "DOOR" },
		{ &DreamGenContext::usecashcard,       "CSHR" },
		{ &DreamGenContext::usegun,            "GUNA" },
		{ &DreamGenContext::usecardreader1,    "CRAA" },
		{ &DreamGenContext::usecardreader2,    "CRBB" },
		{ &DreamGenContext::usecardreader3,    "CRCC" },
		{ &DreamGenContext::sitdowninbar,      "SEAT" },
		{ &DreamGenContext::usemenu,           "MENU" },
		{ &DreamGenContext::usecooker,         "COOK" },
		{ &DreamGenContext::callhotellift,     "ELCA" },
		{ &DreamGenContext::calledenslift,     "EDCA" },
		{ &DreamGenContext::calledensdlift,    "DDCA" },
		{ &DreamGenContext::usealtar,          "ALTR" },
		{ &DreamGenContext::openhoteldoor,     "LOKA" },
		{ &DreamGenContext::openhoteldoor2,    "LOKB" },
		{ &DreamGenContext::openlouis,         "ENTA" },
		{ &DreamGenContext::openryan,          "ENTB" },
		{ &DreamGenContext::openpoolboss,      "ENTE" },
		{ &DreamGenContext::openyourneighbour, "ENTC" },
		{ &DreamGenContext::openeden,          "ENTD" },
		{ &DreamGenContext::opensarters,       "ENTH" },
		{ &DreamGenContext::wearwatch,         "WWAT" },
		{ &DreamGenContext::usepoolreader,     "POOL" },
		{ &DreamGenContext::wearshades,        "WSHD" },
		{ &DreamGenContext::grafittidoor,      "GRAF" },
		{ &DreamGenContext::trapdoor,          "TRAP" },
		{ &DreamGenContext::edenscdplayer,     "CDPE" },
		{ &DreamGenContext::opentvdoor,        "DLOK" },
		{ &DreamGenContext::usehole,           "HOLE" },
		{ &DreamGenContext::usedryer,          "DRYR" },
		{ &DreamGenContext::usechurchhole,     "HOLY" },
		{ &DreamGenContext::usewall,           "WALL" },
		{ &DreamGenContext::usediary,          "BOOK" },
		{ &DreamGenContext::useaxe,            "AXED" },
		{ &DreamGenContext::useshield,         "SHLD" },
		{ &DreamGenContext::userailing,        "BCNY" },
		{ &DreamGenContext::usecoveredbox,     "LIDC" },
		{ &DreamGenContext::useclearbox,       "LIDU" },
		{ &DreamGenContext::useopenbox,        "LIDO" },
		{ &DreamGenContext::usepipe,           "PIPE" },
		{ &DreamGenContext::usebalcony,        "BALC" },
		{ &DreamGenContext::usewindow,         "WIND" },
		{ &DreamGenContext::viewfolder,        "PAPR" },
		{ &DreamGenContext::usetrainer,        "UWTA" },
		{ &DreamGenContext::usetrainer,        "UWTB" },
		{ &DreamGenContext::entersymbol,       "STAT" },
		{ &DreamGenContext::opentomb,          "TLID" },
		{ &DreamGenContext::useslab,           "SLAB" },
		{ &DreamGenContext::usecart,           "CART" },
		{ &DreamGenContext::usefullcart,       "FCAR" },
		{ &DreamGenContext::slabdoora,         "SLBA" },
		{ &DreamGenContext::slabdoorb,         "SLBB" },
		{ &DreamGenContext::slabdoorc,         "SLBC" },
		{ &DreamGenContext::slabdoord,         "SLBD" },
		{ &DreamGenContext::slabdoore,         "SLBE" },
		{ &DreamGenContext::slabdoorf,         "SLBF" },
		{ &DreamGenContext::useplinth,         "PLIN" },
		{ &DreamGenContext::useladder,         "LADD" },
		{ &DreamGenContext::useladderb,        "LADB" },
		{ &DreamGenContext::chewy,             "GUMA" },
		{ &DreamGenContext::wheelsound,        "SQEE" },
		{ &DreamGenContext::runtap,            "TAPP" },
		{ &DreamGenContext::playguitar,        "GUIT" },
		{ &DreamGenContext::hotelcontrol,      "CONT" },
		{ &DreamGenContext::hotelbell,         "BELL" },
	};

	if (data.byte(kReallocation) >= 50) {
		if (data.byte(kPointerpower) == 0)
			return;
		data.byte(kPointerpower) = 0;
	}

	getanyad();
	const uint8 *id = es.ptr(bx + 12, 4);

	for (size_t i = 0; i < sizeof(kUseList)/sizeof(UseListEntry); ++i) {
		const UseListEntry &entry = kUseList[i];
		if (('A' + id[0] == entry.id[0]) && ('A' + id[1] == entry.id[1]) && ('A' + id[2] == entry.id[2]) && ('A' + id[3] == entry.id[3])) {
			(this->*entry.callback)();
			return;
		}
	}

	delpointer();
	uint8 *obText = getobtextstartCPP();
	if (findnextcolon(&obText) != 0) {
		if (findnextcolon(&obText) != 0) {
			if (*obText != 0) {
				usetext(obText);
				hangonp(400);
				putbackobstuff();
				return;
			}
		}
	}

	createpanel();
	showpanel();
	showman();
	showexit();
	obicons();
	printmessage(33, 100, 63, 241, true);
	worktoscreenm();
	hangonp(50);
	putbackobstuff();
	data.byte(kCommandtype) = 255;
}

void DreamGenContext::usetext() {
	usetext(es.ptr(si, 0));
}

void DreamGenContext::usetext(const uint8 *string) {
	createpanel();
	showpanel();
	showman();
	showexit();
	obicons();
	printdirect(string, 36, 104, 241, true);
	worktoscreenm();
}

} /*namespace dreamgen */

