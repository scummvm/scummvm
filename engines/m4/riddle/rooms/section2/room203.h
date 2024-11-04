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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM203_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM203_H

#include "m4/riddle/rooms/section2/section2_room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room203 : public Section2Room {
private:
	int _val1 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val8 = 0;
	bool _showWalker = false;
	bool _ripley80000 = false;
	int _val11 = 0;
	const char *_digiName1 = nullptr;
	int _digiTrigger1 = 0;
	Common::String _digiName2;
	const char *_digiName3 = nullptr;
	int _digiTrigger3 = -1;
	int _trigger1 = -1;
	int _trigger2 = -1;
	int _trigger3 = -1;
	int _trigger4 = -1;
	int _trigger5 = -1;
	int _shadow3 = 0;
	machine *_ripley = nullptr;
	machine *_stream1 = nullptr;
	int _ripHeadTurn = 0;
	int _ripLooksAtHeads = 0;
	int _ripYouSeeToIt = 0;
	int _ripLookAtHeadsTalkMei = 0;
	int _ripKneeling = 0;
	int _ripKneelingTalk = 0;
	int _ripGivesPhoto = 0;
	int _ripClimbsAndBacksDown = 0;
	int _ripPointsAtHelmet = 0;
	int _ripHandsBehBack = 0;
	int _ripHandTalk = 0;
	int _ripTalker = 0;
	int _ripArmsX = 0;
	int _ripHeadDownTalkOff = 0;
	int _ripLookDown = 0;
	int _ripTugsAtCollar = 0;
	int _ripLooksThroughHole = 0;
	int _ripTossesHelmet = 0;
	int _ripTossesBucket = 0;
	machine *_ripsh1 = nullptr;
	machine *_mei = nullptr;
	int _meiTalkToRip = 0;
	int _meiTurnAndTalk = 0;
	int _meiRightHandOut = 0;
	int _meiCheekLine = 0;
	int _meiHallOfClassics = 0;
	machine *_gk = nullptr;
	int _gkFrame = 0;
	int _gkManyDoNeedPass = 0;
	int _gkMayNotPass = 0;
	int _gkMode = 0, _gkShould = 0;
	machine *_oldLady = nullptr;
	int _oldLadyFrame = 0;
	int _oldLadyNoHelmet = 0;
	int _oldLadyFeedingBirds = 0;
	int _oldLadyMode = 0, _oldLadyShould = 0;
	int _ripleyShould = 0;
	int _oldLady1 = 0;
	int _oldLadyPointsToPhoto = 0;
	int _oldLadyPhotoPopup = 0;
	int _oldLadyProtectsHelmet = 0;
	int _oldLadyShowsPhoto = 0;
	machine *_pigeons1 = nullptr;
	machine *_pigeons3 = nullptr;
	int _pigeonsSeries1 = 0;
	int _pigeonsSeries3 = 0;
	machine *_official = nullptr;
	machine *_officialShadow = nullptr;
	int _officialStander = 0;
	int _officialMode = 0, _officialShould = 0;
	int _officialTurn9_11 = 0;
	int _officialTurn9_11_pointGun = 0;
	int _officialTurn11_3 = 0;
	int _officialTurn3_7 = 0;
	int _officialMoveAlong = 0;
	int _officialThroughThere = 0;
	int _officialHalt = 0;
	machine *_peasant = nullptr;
	machine *_peasantShadow = nullptr;
	int _peasantSeries = 0;
	int _peasantSeriesShadow = 0;
	int _peasantRocks = 0;
	int _peasantRocksShadow = 0;
	int _peskyYellsThief = 0;
	int _peskyBegLoop = 0;
	int _peskyAction = 0;
	int _peasantMode = 0, _peasantShould = 0;
	int _peasantMode2 = 0;
	int _peasantX = 0;
	int _peasantY = 0;
	int _peasantScale = 0;
	int _peasantLayer = 0;
	int _peasantSquat9 = 0;
	int _peasantSquat3 = 0;
	int _peasantSquatTo9 = 0;
	int _peasantFromSquat3 = 0;
	bool _flag1 = false;
	bool _flag2 = false;
	machine *_sg = nullptr;
	int _203sg01 = 0;
	machine *_g1 = nullptr;
	machine *_g2 = nullptr;
	int _ripleyMode = 0;
	int _unkShould = 0;
	int _oneFrameBucket = 0;

	int _ctr1 = 0;

	void setupHelmetHotspot();
	void setupPeasantHotspot(int mode);
	void setupGk();
	void setupOldLady();
	void setupPigeons();
	void setupOfficial();
	void setupPeasant();
	void peasantWalk();
	void peasantAnim1();
	void peasantAnim2();
	void conv203c();
	void conv203d();
	void conv203e();
	void playSound(const char *digiName, int trigger);
	void lookThroughHole(const char *digiName, int trigger);
	void lookAtHeads(const char *digiName, int trigger);

public:
	Room203() : Section2Room() {}
	~Room203() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
