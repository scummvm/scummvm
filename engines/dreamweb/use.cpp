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

struct UseListEntry {
	uint8  id[5]; // 0-terminal because it is easier syntatically to initialize the array
	UseCallback callback;
};

void DreamGenContext::useroutine() {

	static const UseListEntry kUseList[] = {
		{ "NETW", &DreamGenContext::usemon },
		{ "ELVA", &DreamGenContext::useelevator1 },
		{ "ELVB", &DreamGenContext::useelevator2 },
		{ "ELVC", &DreamGenContext::useelevator3 },
		{ "ELVE", &DreamGenContext::useelevator4 },
		{ "ELVF", &DreamGenContext::useelevator5 },
		{ "CGAT", &DreamGenContext::usechurchgate },
		{ "REMO", &DreamGenContext::usestereo },
		{ "BUTA", &DreamGenContext::usebuttona },
		{ "CBOX", &DreamGenContext::usewinch },
		{ "LITE", &DreamGenContext::uselighter },
		{ "PLAT", &DreamGenContext::useplate },
		{ "LIFT", &DreamGenContext::usecontrol },
		{ "WIRE", &DreamGenContext::usewire },
		{ "HNDL", &DreamGenContext::usehandle },
		{ "HACH", &DreamGenContext::usehatch },
		{ "DOOR", &DreamGenContext::useelvdoor },
		{ "CSHR", &DreamGenContext::usecashcard },
		{ "GUNA", &DreamGenContext::usegun },
		{ "CRAA", &DreamGenContext::usecardreader1 },
		{ "CRBB", &DreamGenContext::usecardreader2 },
		{ "CRCC", &DreamGenContext::usecardreader3 },
		{ "SEAT", &DreamGenContext::sitdowninbar },
		{ "MENU", &DreamGenContext::usemenu },
		{ "COOK", &DreamGenContext::usecooker },
		{ "ELCA", &DreamGenContext::callhotellift },
		{ "EDCA", &DreamGenContext::calledenslift },
		{ "DDCA", &DreamGenContext::calledensdlift },
		{ "ALTR", &DreamGenContext::usealtar },
		{ "LOKA", &DreamGenContext::openhoteldoor },
		{ "LOKB", &DreamGenContext::openhoteldoor2 },
		{ "ENTA", &DreamGenContext::openlouis },
		{ "ENTB", &DreamGenContext::openryan },
		{ "ENTE", &DreamGenContext::openpoolboss },
		{ "ENTC", &DreamGenContext::openyourneighbour },
		{ "ENTD", &DreamGenContext::openeden },
		{ "ENTH", &DreamGenContext::opensarters },
		{ "WWAT", &DreamGenContext::wearwatch },
		{ "POOL", &DreamGenContext::usepoolreader },
		{ "WSHD", &DreamGenContext::wearshades },
		{ "GRAF", &DreamGenContext::grafittidoor },
		{ "TRAP", &DreamGenContext::trapdoor },
		{ "CDPE", &DreamGenContext::edenscdplayer },
		{ "DLOK", &DreamGenContext::opentvdoor },
		{ "HOLE", &DreamGenContext::usehole },
		{ "DRYR", &DreamGenContext::usedryer },
		{ "HOLY", &DreamGenContext::usechurchhole },
		{ "WALL", &DreamGenContext::usewall },
		{ "BOOK", &DreamGenContext::usediary },
		{ "AXED", &DreamGenContext::useaxe },
		{ "SHLD", &DreamGenContext::useshield },
		{ "BCNY", &DreamGenContext::userailing },
		{ "LIDC", &DreamGenContext::usecoveredbox },
		{ "LIDU", &DreamGenContext::useclearbox },
		{ "LIDO", &DreamGenContext::useopenbox },
		{ "PIPE", &DreamGenContext::usepipe },
		{ "BALC", &DreamGenContext::usebalcony },
		{ "WIND", &DreamGenContext::usewindow },
		{ "PAPR", &DreamGenContext::viewfolder },
		{ "UWTA", &DreamGenContext::usetrainer },
		{ "UWTB", &DreamGenContext::usetrainer },
		{ "STAT", &DreamGenContext::entersymbol },
		{ "TLID", &DreamGenContext::opentomb },
		{ "SLAB", &DreamGenContext::useslab },
		{ "CART", &DreamGenContext::usecart },
		{ "FCAR", &DreamGenContext::usefullcart },
		{ "SLBA", &DreamGenContext::slabdoora },
		{ "SLBB", &DreamGenContext::slabdoorb },
		{ "SLBC", &DreamGenContext::slabdoorc },
		{ "SLBD", &DreamGenContext::slabdoord },
		{ "SLBE", &DreamGenContext::slabdoore },
		{ "SLBF", &DreamGenContext::slabdoorf },
		{ "PLIN", &DreamGenContext::useplinth },
		{ "LADD", &DreamGenContext::useladder },
		{ "LADB", &DreamGenContext::useladderb },
		{ "GUMA", &DreamGenContext::chewy },
		{ "SQEE", &DreamGenContext::wheelsound },
		{ "TAPP", &DreamGenContext::runtap },
		{ "GUIT", &DreamGenContext::playguitar },
		{ "CONT", &DreamGenContext::hotelcontrol },
		{ "BELL", &DreamGenContext::hotelbell },
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

