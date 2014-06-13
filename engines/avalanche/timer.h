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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* Original name: TIMEOUT	The scheduling unit. */

#ifndef AVALANCHE_TIMER_H
#define AVALANCHE_TIMER_H

namespace Avalanche {
class AvalancheEngine;

class Timer {
public:
	// Reason runs between 1 and 28.
	enum Reason {
		kReasonDrawbridgeFalls = 2,
		kReasonAvariciusTalks = 3,
		kReasonGoToToilet = 4,
		kReasonExplosion = 5,
		kReasonBrummieStairs = 6,
		kReasonCardiffsurvey = 7,
		kReasonCwytalotInHerts = 8,
		kReasonGettingTiedUp = 9,
		kReasonHangingAround = 10, // Tied to the tree in Nottingham.
		kReasonJacquesWakingUp = 11,
		kReasonNaughtyDuke = 12,
		kReasonJumping = 13,
		kReasonSequencer = 14,
		kReasonCrapulusSaysSpludwickOut = 15,
		kReasonDawndelay = 16,
		kReasonDrinks = 17,
		kReasonDuLustieTalks = 18,
		kReasonFallingDownOubliette = 19,
		kReasonMeetingAvaroid = 20,
		kReasonRisingUpOubliette = 21,
		kReasonRobinHoodAndGeida = 22,
		kReasonSittingDown = 23,
		kReasonGhostRoomPhew = 1,
		kReasonArkataShouts = 24,
		kReasonWinning = 25,
		kReasonFallingOver = 26,
		kReasonSpludwickWalk = 27,
		kReasonGeidaSings = 28
	};

	// Proc runs between 1 and 41.
	enum Proc {
		kProcOpenDrawbridge = 3,
		kProcAvariciusTalks = 4,
		kProcUrinate = 5,
		kProcToilet = 6,
		kProcBang = 7,
		kProcBang2 = 8,
		kProcStairs = 9,
		kProcCardiffSurvey = 10,
		kProcCardiffReturn = 11,
		kProcCwytalotInHerts = 12,
		kProcGetTiedUp = 13,
		kProcGetTiedUp2 = 1,
		kProcHangAround = 14,
		kProcHangAround2 = 15,
		kProcAfterTheShootemup = 32,
		kProcJacquesWakesUp = 16,
		kProcNaughtyDuke = 17,
		kProcNaughtyDuke2 = 18,
		kProcNaughtyDuke3 = 38,
		kProcJump = 19,
		kProcSequence = 20,
		kProcCrapulusSpludOut = 21,
		kProcDawnDelay = 22,
		kProcBuyDrinks = 23,
		kProcBuyWine = 24,
		kProcCallsGuards = 25,
		kProcGreetsMonk = 26,
		kProcFallDownOubliette = 27,
		kProcMeetAvaroid = 28,
		kProcRiseUpOubliette = 29,
		kProcRobinHoodAndGeida = 2,
		kProcRobinHoodAndGeidaTalk = 30,
		kProcAvalotReturns = 31,
		kProcAvvySitDown = 33, // In Nottingham.
		kProcGhostRoomPhew = 34,
		kProcArkataShouts = 35,
		kProcWinning = 36,
		kProcAvalotFalls = 37,
		kProcSpludwickGoesToCauldron = 39,
		kProcSpludwickLeavesCauldron = 40,
		kProcGiveLuteToGeida = 41
	};

	struct TimerType {
		int32 _timeLeft;
		byte _action;
		byte _reason;
	};

	TimerType _times[7];

	Timer(AvalancheEngine *vm);

	void resetVariables();
	void addTimer(int32 duration, byte action, byte reason);
	void updateTimer();
	void loseTimer(byte which);

	// Procedures to do things at the end of amounts of time:
	void openDrawbridge();
	void avariciusTalks();
	void urinate();
	void toilet();
	void bang();
	void bang2();
	void stairs();
	void cardiffSurvey();
	void cardiffReturn();
	void cwytalotInHerts();
	void getTiedUp();
	void getTiedUp2();
	void hangAround();
	void hangAround2();
	void afterTheShootemup();
	void jacquesWakesUp();
	void naughtyDuke();
	void naughtyDuke2();
	void naughtyDuke3();
	void jump();
	void crapulusSaysSpludOut();
	void buyDrinks();
	void buyWine();
	void callsGuards();
	void greetsMonk();
	void fallDownOubliette();
	void meetAvaroid();
	void riseUpOubliette();
	void robinHoodAndGeida();
	void robinHoodAndGeidaTalk();
	void avalotReturns();
	void avvySitDown();
	void ghostRoomPhew();
	void arkataShouts();
	void winning();
	void avalotFalls();
	void spludwickGoesToCauldron();
	void spludwickLeavesCauldron();
	void giveLuteToGeida();

private:
	AvalancheEngine *_vm;
	byte _shootEmUpScore;
};

} // End of namespace Avalanche.

#endif // AVALANCHE_TIMER_H
