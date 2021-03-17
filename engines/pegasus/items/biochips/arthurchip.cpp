/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/arthurchip.h"

namespace Common {
DECLARE_SINGLETON(Pegasus::ArthurManager);
}

namespace Pegasus {

static const char *kArthurWisdomMovies[] = {
	"Images/AI/Globals/XGLOBA01",
	"Images/AI/Globals/XGLOBA03",
	"Images/AI/Globals/XGLOBA06",
	"Images/AI/Globals/XGLOBA07",
	"Images/AI/Globals/XGLOBA09",
	"Images/AI/Globals/XGLOBA17",
	"Images/AI/Globals/XGLOBA18",
	"Images/AI/Globals/XGLOBA25",
	"Images/AI/Globals/XGLOBA26",
	"Images/AI/Globals/XGLOBA27",
	"Images/AI/Globals/XGLOBA28",
	"Images/AI/Globals/XGLOBA30",
	"Images/AI/Globals/XGLOBA34",
	"Images/AI/Globals/XGLOBA35",
	"Images/AI/Globals/XGLOBA43",
	"Images/AI/Globals/XGLOBA50",
	"Images/AI/Globals/XGLOBA56",
	"Images/AI/Globals/XGLOBA59",
	"Images/AI/Globals/XGLOBA63",
	"Images/AI/Globals/XGLOBB10",
	"Images/AI/Globals/XGLOBB11",
	"Images/AI/Globals/XGLOBB12",
	"Images/AI/Globals/XGLOBB13",
	"Images/AI/Globals/XGLOBB14",
	"Images/AI/Globals/XGLOBB15",
	"Images/AI/Globals/XGLOBB16",
	"Images/AI/Globals/XGLOBB17",
	"Images/AI/Globals/XGLOBB18",
	"Images/AI/Globals/XGLOBB19",
	"Images/AI/Globals/XGLOBB20",
	"Images/AI/Globals/XGLOBB21",
	"Images/AI/Globals/XGLOBB22",
	"Images/AI/Globals/XGLOBB23",
	"Images/AI/Globals/XGLOBB24",
	"Images/AI/Globals/XGLOBB25",
	"Images/AI/Globals/XGLOBB26",
	"Images/AI/Globals/XGLOBB27",
	"Images/AI/Globals/XGLOBB28",
	"Images/AI/Globals/XGLOBB29",
	"Images/AI/Globals/XGLOBB30",
	"Images/AI/Globals/XGLOBB31",
	"Images/AI/Globals/XGLOBB32",
	"Images/AI/Globals/XGLOBB33",
	"Images/AI/Globals/XGLOBB34",
	"Images/AI/Globals/XGLOBB35",
	"Images/AI/Globals/XGLOBB36",
	"Images/AI/Globals/XGLOBB37",
	"Images/AI/Globals/XGLOBB38",
	"Images/AI/Globals/XGLOBB39",
	"Images/AI/Globals/XGLOBB43",
	"Images/AI/Globals/XGLOBB44",
	"Images/AI/Globals/XGLOBA62"
};

ArthurChip *g_arthurChip = 0;

ArthurChip::ArthurChip(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) :
		BiochipItem(id, neighborhood, room, direction), _arthurWisdomHotspot(kArthurWisdomSpotID),
		_chattyArthurHotspot(kChattyArthurSpotID), _chattyAIHotspot(kChattyAISpotID),
		_arthurHeadHotspot(kArthurHeadSpotID) {
	_lastArthurMovie = "";

	_arthurWisdomHotspot.setArea(Common::Rect(kAIMiddleAreaLeft + 20, kAIMiddleAreaTop + 27, kAIMiddleAreaLeft + 20 + 161, kAIMiddleAreaTop + 27 + 30));
	_arthurWisdomHotspot.setHotspotFlags(kArthurBiochipSpotFlag);
	g_allHotspots.push_back(&_arthurWisdomHotspot);

	_chattyArthurHotspot.setArea(Common::Rect(kAIMiddleAreaLeft + 100, kAIMiddleAreaTop + 67, kAIMiddleAreaLeft + 100 + 20, kAIMiddleAreaTop + 67 + 20));
	_chattyArthurHotspot.setHotspotFlags(kArthurBiochipSpotFlag);
	g_allHotspots.push_back(&_chattyArthurHotspot);

	_chattyAIHotspot.setArea(Common::Rect(kAIMiddleAreaLeft + 130, kAIMiddleAreaTop + 67, kAIMiddleAreaLeft + 130 + 20, kAIMiddleAreaTop + 67 + 20));
	_chattyAIHotspot.setHotspotFlags(kArthurBiochipSpotFlag);
	g_allHotspots.push_back(&_chattyAIHotspot);

	_arthurHeadHotspot.setArea(Common::Rect(kAIRightAreaLeft, kAIRightAreaTop, kAIRightAreaLeft + kAIRightAreaWidth, kAIRightAreaTop + kAIRightAreaHeight));
	_arthurHeadHotspot.setHotspotFlags(kArthurBiochipSpotFlag);
	g_allHotspots.push_back(&_arthurHeadHotspot);

	setItemState(kArthur000);

	g_arthurChip = this;
}

ArthurChip::~ArthurChip() {
	g_arthurChip = NULL;

	g_allHotspots.removeOneHotspot(kArthurWisdomSpotID);
	g_allHotspots.removeOneHotspot(kChattyArthurSpotID);
	g_allHotspots.removeOneHotspot(kChattyAISpotID);
	g_allHotspots.removeOneHotspot(kArthurHeadSpotID);
}

void ArthurChip::select() {
	BiochipItem::select();
	setUpArthurChip();
}

void ArthurChip::setUpArthurChip() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;
	ItemState state = getItemState();

	if (vm->isChattyArthur()) {
		if (g_AIArea && vm->isChattyAI()) {
			if (state != kArthur002)
				setItemState(kArthur000);
		} else if (state != kArthur102) {
			setItemState(kArthur100);
		}
	} else {
		if (g_AIArea && vm->isChattyAI()) {
			if (state != kArthur012)
				setItemState(kArthur010);
		} else if (state != kArthur112) {
			setItemState(kArthur110);
		}
	}
}

void ArthurChip::activateArthurHotspots() {
	_arthurWisdomHotspot.setActive();
	_chattyArthurHotspot.setActive();
	_chattyAIHotspot.setActive();
	_arthurHeadHotspot.setActive();
}

void ArthurChip::clickInArthurHotspot(HotSpotID id) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;
	ItemState state, newState;

	if (id == kArthurHeadSpotID) {
		if (_lastArthurMovie != "")
			playArthurMovie(_lastArthurMovie);
		return;
	}

	newState = state = getItemState();
	switch (state) {
	case kArthur000:
		switch (id) {
		case kArthurWisdomSpotID:
			newState = kArthur002;
			break;
		case kChattyArthurSpotID:
			newState = kArthur010;
			break;
		case kChattyAISpotID:
			newState = kArthur100;
			break;
		}
		break;
	case kArthur010:
		switch (id) {
		case kArthurWisdomSpotID:
			newState = kArthur012;
			break;
		case kChattyArthurSpotID:
			newState = kArthur000;
			break;
		case kChattyAISpotID:
			newState = kArthur110;
			break;
		}
		break;
	case kArthur100:
		switch (id) {
		case kArthurWisdomSpotID:
			newState = kArthur102;
			break;
		case kChattyArthurSpotID:
			newState = kArthur110;
			break;
		case kChattyAISpotID:
			newState = kArthur010;
			break;
		}
		break;
	case kArthur110:
		switch (id) {
		case kArthurWisdomSpotID:
			newState = kArthur112;
			break;
		case kChattyArthurSpotID:
			newState = kArthur100;
			break;
		case kChattyAISpotID:
			newState = kArthur010;
			break;
		}
		break;
	}
	setItemState(newState);
	switch (id) {
	case kArthurWisdomSpotID:
		playArthurMovie(kArthurWisdomMovies[((PegasusEngine *)g_engine)->getRandomNumber((
						sizeof(kArthurWisdomMovies) / sizeof(const char *)) - 1)]);
		break;
	case kChattyArthurSpotID:
		vm->setChattyArthur(!vm->isChattyArthur());
		break;
	case kChattyAISpotID:
		vm->setChattyAI(!vm->isChattyAI());
		break;
	}

	setItemState(state);
}

void ArthurChip::playArthurMovie(const Common::String &movieName) {
	if (g_AIArea) {
		g_AIArea->playAIMovie(kRightAreaSignature, movieName, false, kHintInterruption);
		if (movieName != "Images/AI/Globals/XGLOB00" &&
			movieName != "Images/AI/Globals/XGLOB01" &&
			movieName != "Images/AI/Globals/XGLOBAA0" &&
			movieName != "Images/AI/Globals/XGLOBAA1" &&
			movieName != "Images/AI/Globals/XGLOBAA2")
			_lastArthurMovie = movieName;
	}
}

bool ArthurChip::playArthurMovieForEvent(const Common::String &movieName, ArthurEvent event) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	if (vm->isDVD() && vm->playerHasItemID(kArthurBiochip) &&
		vm->isChattyArthur() && !Arthur._arthurFlags.getFlag(event)) {
		Arthur._arthurFlags.setFlag(event, true);
		playArthurMovie(movieName);
		return true;
	} else {
		return false;
	}
}

void ArthurManager::resetArthurState() {
	bool savedGameFlag = _arthurFlags.getFlag(kArthurLoadedSavedGame);
	_arthurFlags.clearAllFlags();
	_arthurFlags.setFlag(kArthurLoadedSavedGame, savedGameFlag);
}

} // End of namespace Pegasus
