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

enum GlassGiven { GG_DEFAULT = -1, GG_EMPTY = 0, GG_CONDIMENT = 1, GG_BIRD = 3 };

class CBarbot : public CTrueTalkNPC {
	struct FrameRange {
		int _startFrame;
		int _endFrame;
		FrameRange() : _startFrame(0), _endFrame(0) {}
	};
	class FrameRanges : public Common::Array<FrameRange> {
	public:
		FrameRanges();
	};

	DECLARE_MESSAGE_MAP;
	bool ActMsg(CActMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool TurnOn(CTurnOn *msg);
	bool TurnOff(CTurnOff *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool TrueTalkSelfQueueAnimSetMsg(CTrueTalkSelfQueueAnimSetMsg *msg);
	bool TrueTalkQueueUpAnimSetMsg(CTrueTalkQueueUpAnimSetMsg *msg);
	bool TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg);
	bool TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg);
	bool FrameMsg(CFrameMsg *msg);
	bool LoadSuccessMsg(CLoadSuccessMsg *msg);
	bool MovieFrameMsg(CMovieFrameMsg *msg);
	bool EnterRoomMsg(CEnterRoomMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
private:
	static int _timesCalled;
private:
	FrameRanges _frames;
	int _field108;
	int _field10C;
	int _field110;
	bool _addedLemon;
	bool _addedTV;
	bool _addedPuret;
	int _field120;
	int _field124;
	bool _visCenterOnCounter;
	bool _addedVodka;
	bool _gottenDrunk;
	int _field134;
	int _field138;
	int _field13C;
	int _volume;
	int _frameNum;
	int _field148;
	int _field14C;
	int _field150;
	int _field154;
	GlassGiven _glassContent;
	bool _drunkFlag;
	int _field160;
private:
	/**
	 * Plays a given range of movie frames
	 */
	void playRange(const FrameRange &range, uint flags = 0);

	/**
	 * Returns true if one of the ingredients (with the exception of Vodka)
	 * is missing from the concoction
	 */
	bool areIngredientsMissing() const {
		return !_addedPuret || !_addedLemon || !_addedVodka;
	}
public:
	CLASSDEF;
	CBarbot();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

} // End of namespace Titanic

#endif /* TITANIC_BARBOT_H */
