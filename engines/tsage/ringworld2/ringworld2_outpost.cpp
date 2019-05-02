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

#include "graphics/cursorman.h"
#include "tsage/dialogs.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_outpost.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 1337 - Card game
 *
 *--------------------------------------------------------------------------*/

Scene1337::Card::Card() {
	_cardId = 0;
	_stationPos = Common::Point(0, 0);
}

void Scene1337::Card::synchronize(Serializer &s) {
	_card.synchronize(s);

	s.syncAsSint16LE(_cardId);
	s.syncAsSint16LE(_stationPos.x);
	s.syncAsSint16LE(_stationPos.y);
}

bool Scene1337::Card::isIn(Common::Point pt) {
	if ((_stationPos.x > pt.x) || (_stationPos.x + 24 < pt.x))
		return false;

	if ((_stationPos.y > pt.y) || (_stationPos.y + 24 < pt.y))
		return false;

	return true;
}

Scene1337::GameBoardSide::GameBoardSide() {
	_card1Pos = Common::Point(0, 0);
	_card2Pos = Common::Point(0, 0);
	_card3Pos = Common::Point(0, 0);
	_card4Pos = Common::Point(0, 0);
	_frameNum = 0;
}

void Scene1337::GameBoardSide::synchronize(Serializer &s) {
	SceneHotspot::synchronize(s);

	for (int i = 0; i < 4; i++)
		_handCard[i].synchronize(s);

	for (int i = 0; i < 8; i++)
		_outpostStation[i].synchronize(s);

	_delayCard.synchronize(s);
	_emptyStationPos.synchronize(s);

	s.syncAsSint16LE(_card1Pos.x);
	s.syncAsSint16LE(_card1Pos.y);
	s.syncAsSint16LE(_card2Pos.x);
	s.syncAsSint16LE(_card2Pos.y);
	s.syncAsSint16LE(_card3Pos.x);
	s.syncAsSint16LE(_card3Pos.y);
	s.syncAsSint16LE(_card4Pos.x);
	s.syncAsSint16LE(_card4Pos.y);
	s.syncAsSint16LE(_frameNum);
}

Scene1337::Scene1337() {
	_autoplay = false;
	_cardsAvailableNumb = 0;
	_currentDiscardIndex = 0;

	for (int i = 0; i < 100; i++)
		_availableCardsPile[i] = 0;

	_shuffleEndedFl = false;
	_currentPlayerNumb = 0;
	_actionPlayerIdx = 0;
	_actionVictimIdx = 0;
	_showPlayerTurn = false;
	_displayHelpFl = false;
	_winnerId = -1;
	_instructionsDisplayedFl = false;
	_instructionsWaitCount = 0;

	_delayedFunction = nullptr;
	_actionCard1 = nullptr;
	_actionCard2 = nullptr;
	_actionCard3 = nullptr;

	_cursorCurRes = 0;
	_cursorCurStrip = 0;
	_cursorCurFrame = 0;
}

void Scene1337::synchronize(Serializer &s) {
	_actionCard1->synchronize(s);
	_actionCard2->synchronize(s);
	_actionCard3->synchronize(s);
	_animatedCard.synchronize(s);
	_shuffleAnimation.synchronize(s);
	_discardedPlatformCard.synchronize(s);
	_selectedCard.synchronize(s);
	_discardPile.synchronize(s);
	_stockCard.synchronize(s);
	_aSound1.synchronize(s);
	_aSound2.synchronize(s);
	_helpIcon.synchronize(s);
	_stockPile.synchronize(s);
	_actionItem.synchronize(s);
	_currentPlayerArrow.synchronize(s);

	for (int i = 0; i < 4; i++)
		_gameBoardSide[i].synchronize(s);

	for (int i = 0; i < 8; i++) {
		_upperDisplayCard[i].synchronize(s);
		_lowerDisplayCard[i].synchronize(s);
	}

	// TODO s.syncPointer(_delayedFunction);
	s.syncAsByte(_autoplay);
	s.syncAsByte(_shuffleEndedFl);
	s.syncAsByte(_showPlayerTurn);
	s.syncAsByte(_displayHelpFl);
	s.syncAsByte(_instructionsDisplayedFl);
	s.syncAsSint16LE(_currentDiscardIndex);
	s.syncAsSint16LE(_cardsAvailableNumb);
	s.syncAsSint16LE(_currentPlayerNumb);
	s.syncAsSint16LE(_actionPlayerIdx);
	s.syncAsSint16LE(_actionVictimIdx);
	s.syncAsSint16LE(_winnerId);
	s.syncAsSint16LE(_instructionsWaitCount);
	s.syncAsSint16LE(_cursorCurRes);
	s.syncAsSint16LE(_cursorCurStrip);
	s.syncAsSint16LE(_cursorCurFrame);

	for (int i = 0; i < 100; i++)
		s.syncAsSint16LE(_availableCardsPile[i]);

}

void Scene1337::Action1337::waitFrames(int32 frameCount) {
	uint32 firstFrameNumber = g_globals->_events.getFrameNumber();
	uint32 curFrame = firstFrameNumber;
	uint32 destFrame = firstFrameNumber + frameCount;

	while ((curFrame < destFrame) && !g_vm->shouldQuit()) {
		TsAGE::Event event;
		g_globals->_events.getEvent(event);
		curFrame = g_globals->_events.getFrameNumber();
	}

	// CHECKME: The original is calling _eventManager.waitEvent();
}

/**
 * Display instructions
 */
void Scene1337::Action1::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 1: {
		scene->actionDisplay(1331, 6, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		R2_GLOBALS._sceneObjects->draw();
		scene->actionDisplay(1331, 7, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		scene->actionDisplay(1331, 8, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[1]._outpostStation[0]._cardId = 2;
		scene->_gameBoardSide[1]._outpostStation[0]._card.postInit();
		scene->_gameBoardSide[1]._outpostStation[0]._card.setVisage(1332);
		scene->_gameBoardSide[1]._outpostStation[0]._card.setPosition(scene->_gameBoardSide[1]._outpostStation[0]._stationPos, 0);
		scene->_gameBoardSide[1]._outpostStation[0]._card.setStrip(2);
		scene->_gameBoardSide[1]._outpostStation[0]._card.setFrame(scene->_gameBoardSide[1]._outpostStation[0]._cardId);
		scene->_gameBoardSide[1]._outpostStation[0]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[1]._outpostStation[0]);

		scene->_gameBoardSide[1]._outpostStation[1]._cardId = 3;
		scene->_gameBoardSide[1]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[1]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[1]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[1]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[1]._outpostStation[1]._card.setStrip(2);
		scene->_gameBoardSide[1]._outpostStation[1]._card.setFrame(scene->_gameBoardSide[1]._outpostStation[1]._cardId);
		scene->_gameBoardSide[1]._outpostStation[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[1]._outpostStation[1]);

		scene->_gameBoardSide[2]._outpostStation[0]._cardId = 4;
		scene->_gameBoardSide[2]._outpostStation[0]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[0]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[0]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[0]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[0]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[0]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[0]._cardId);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[0]);

		scene->_gameBoardSide[3]._outpostStation[0]._cardId = 5;
		scene->_gameBoardSide[3]._outpostStation[0]._card.postInit();
		scene->_gameBoardSide[3]._outpostStation[0]._card.setVisage(1332);
		scene->_gameBoardSide[3]._outpostStation[0]._card.setPosition(scene->_gameBoardSide[3]._outpostStation[0]._stationPos, 0);
		scene->_gameBoardSide[3]._outpostStation[0]._card.setStrip(2);
		scene->_gameBoardSide[3]._outpostStation[0]._card.setFrame(scene->_gameBoardSide[3]._outpostStation[0]._cardId);
		scene->_gameBoardSide[3]._outpostStation[0]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[3]._outpostStation[0]);

		scene->_gameBoardSide[3]._outpostStation[1]._cardId = 6;
		scene->_gameBoardSide[3]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[3]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[3]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[3]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[3]._outpostStation[1]._card.setStrip(2);
		scene->_gameBoardSide[3]._outpostStation[1]._card.setFrame(scene->_gameBoardSide[3]._outpostStation[1]._cardId);
		scene->_gameBoardSide[3]._outpostStation[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[3]._outpostStation[1]);

		scene->_gameBoardSide[3]._outpostStation[2]._cardId = 7;
		scene->_gameBoardSide[3]._outpostStation[2]._card.postInit();
		scene->_gameBoardSide[3]._outpostStation[2]._card.setVisage(1332);
		scene->_gameBoardSide[3]._outpostStation[2]._card.setPosition(scene->_gameBoardSide[3]._outpostStation[2]._stationPos, 0);
		scene->_gameBoardSide[3]._outpostStation[2]._card.setStrip(2);
		scene->_gameBoardSide[3]._outpostStation[2]._card.setFrame(scene->_gameBoardSide[3]._outpostStation[2]._cardId);
		scene->_gameBoardSide[3]._outpostStation[2]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[3]._outpostStation[2]);

		scene->_gameBoardSide[0]._outpostStation[0]._cardId = 8;
		scene->_gameBoardSide[0]._outpostStation[0]._card.postInit();
		scene->_gameBoardSide[0]._outpostStation[0]._card.setVisage(1332);
		scene->_gameBoardSide[0]._outpostStation[0]._card.setPosition(scene->_gameBoardSide[0]._outpostStation[0]._stationPos, 0);
		scene->_gameBoardSide[0]._outpostStation[0]._card.setStrip(2);
		scene->_gameBoardSide[0]._outpostStation[0]._card.setFrame(scene->_gameBoardSide[0]._outpostStation[0]._cardId);
		scene->_gameBoardSide[0]._outpostStation[0]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[0]._outpostStation[0]);

		scene->_gameBoardSide[0]._outpostStation[1]._cardId = 9;
		scene->_gameBoardSide[0]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[0]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[0]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[0]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[0]._outpostStation[1]._card.setStrip(2);
		scene->_gameBoardSide[0]._outpostStation[1]._card.setFrame(scene->_gameBoardSide[0]._outpostStation[1]._cardId);
		scene->_gameBoardSide[0]._outpostStation[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[0]._outpostStation[1]);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 9, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[2]._outpostStation[1]._cardId = 2;
		scene->_gameBoardSide[2]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[1]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[1]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[1]._cardId);
		scene->_gameBoardSide[2]._outpostStation[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[1]);

		scene->_gameBoardSide[2]._outpostStation[2]._cardId = 3;
		scene->_gameBoardSide[2]._outpostStation[2]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[2]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[2]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[2]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[2]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[2]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[2]._cardId);
		scene->_gameBoardSide[2]._outpostStation[2]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[2]);

		scene->_gameBoardSide[2]._outpostStation[3]._cardId = 5;
		scene->_gameBoardSide[2]._outpostStation[3]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[3]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[3]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[3]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[3]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[3]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[3]._cardId);
		scene->_gameBoardSide[2]._outpostStation[3]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[3]);

		scene->_gameBoardSide[2]._outpostStation[4]._cardId = 6;
		scene->_gameBoardSide[2]._outpostStation[4]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[4]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[4]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[4]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[4]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[4]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[4]._cardId);
		scene->_gameBoardSide[2]._outpostStation[4]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[4]);

		scene->_gameBoardSide[2]._outpostStation[5]._cardId = 7;
		scene->_gameBoardSide[2]._outpostStation[5]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[5]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[5]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[5]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[5]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[5]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[5]._cardId);
		scene->_gameBoardSide[2]._outpostStation[5]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[5]);

		scene->_gameBoardSide[2]._outpostStation[6]._cardId = 8;
		scene->_gameBoardSide[2]._outpostStation[6]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[6]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[6]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[6]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[6]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[6]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[6]._cardId);
		scene->_gameBoardSide[2]._outpostStation[6]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[6]);

		scene->_gameBoardSide[2]._outpostStation[7]._cardId = 9;
		scene->_gameBoardSide[2]._outpostStation[7]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[7]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[7]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[7]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[7]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[7]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[7]._cardId);
		scene->_gameBoardSide[2]._outpostStation[7]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[7]);

		scene->_aSound1.play(62);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(120);
		scene->_gameBoardSide[2]._outpostStation[0]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[1]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[2]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[3]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[4]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[5]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[6]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[7]._card.remove();

		scene->_gameBoardSide[1]._outpostStation[0]._card.remove();
		scene->_gameBoardSide[1]._outpostStation[1]._card.remove();

		scene->_gameBoardSide[3]._outpostStation[0]._card.remove();
		scene->_gameBoardSide[3]._outpostStation[1]._card.remove();
		scene->_gameBoardSide[3]._outpostStation[2]._card.remove();

		scene->_gameBoardSide[0]._outpostStation[0]._card.remove();
		scene->_gameBoardSide[0]._outpostStation[1]._card.remove();

		scene->_stockPile.setup(1332, 5, 1);
		scene->_stockPile.setPosition(Common::Point(162, 95));
		scene->_stockPile.setPriority(110);
		scene->_stockPile._effect = EFFECT_SHADED;
		scene->_stockPile.show();

		scene->_gameBoardSide[1]._handCard[0]._card.postInit();
		scene->_gameBoardSide[1]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[0]._card.setPosition(scene->_gameBoardSide[1]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[0]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[0]._card.fixPriority(170);

		scene->_gameBoardSide[1]._handCard[1]._card.postInit();
		scene->_gameBoardSide[1]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[1]._card.setPosition(scene->_gameBoardSide[1]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[1]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[1]._card.fixPriority(170);

		scene->_gameBoardSide[1]._handCard[2]._card.postInit();
		scene->_gameBoardSide[1]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[2]._card.setPosition(scene->_gameBoardSide[1]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[2]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[2]._card.fixPriority(170);

		scene->_gameBoardSide[2]._handCard[0]._cardId = 30;
		scene->_gameBoardSide[2]._handCard[0]._card.postInit();
		scene->_gameBoardSide[2]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[0]._card.setPosition(scene->_gameBoardSide[2]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[2]._handCard[0]._card.setFrame(2);
		scene->_gameBoardSide[2]._handCard[0]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._handCard[0]);

		scene->_gameBoardSide[2]._handCard[1]._cardId = 16;
		scene->_gameBoardSide[2]._handCard[1]._card.postInit();
		scene->_gameBoardSide[2]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[1]._card.setPosition(scene->_gameBoardSide[2]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[2]._handCard[1]._card.setFrame(2);
		scene->_gameBoardSide[2]._handCard[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._handCard[1]);

		scene->_gameBoardSide[2]._handCard[2]._cardId = 1;
		scene->_gameBoardSide[2]._handCard[2]._card.postInit();
		scene->_gameBoardSide[2]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[2]._card.setPosition(scene->_gameBoardSide[2]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[2]._handCard[2]._card.setFrame(2);
		scene->_gameBoardSide[2]._handCard[2]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._handCard[2]);

		scene->_gameBoardSide[3]._handCard[0]._card.postInit();
		scene->_gameBoardSide[3]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[0]._card.setPosition(scene->_gameBoardSide[3]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[0]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[0]._card.fixPriority(170);

		scene->_gameBoardSide[3]._handCard[1]._card.postInit();
		scene->_gameBoardSide[3]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[1]._card.setPosition(scene->_gameBoardSide[3]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[1]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[1]._card.fixPriority(170);

		scene->_gameBoardSide[3]._handCard[2]._card.postInit();
		scene->_gameBoardSide[3]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[2]._card.setPosition(scene->_gameBoardSide[3]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[2]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[2]._card.fixPriority(170);

		scene->_gameBoardSide[0]._handCard[0]._card.postInit();
		scene->_gameBoardSide[0]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[0]._card.setPosition(scene->_gameBoardSide[0]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[0]._handCard[0]._card.setFrame(2);
		scene->_gameBoardSide[0]._handCard[0]._card.fixPriority(170);

		scene->_gameBoardSide[0]._handCard[1]._card.postInit();
		scene->_gameBoardSide[0]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[1]._card.setPosition(scene->_gameBoardSide[0]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[0]._handCard[1]._card.setFrame(2);
		scene->_gameBoardSide[0]._handCard[1]._card.fixPriority(170);

		scene->_gameBoardSide[0]._handCard[2]._card.postInit();
		scene->_gameBoardSide[0]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[2]._card.setPosition(scene->_gameBoardSide[0]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[0]._handCard[2]._card.setFrame(2);
		scene->_gameBoardSide[0]._handCard[2]._card.fixPriority(170);

		R2_GLOBALS._sceneObjects->draw();

		scene->actionDisplay(1331, 10, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		scene->_animatedCard._card.setPosition(Common::Point(162, 95), 0);
		scene->_animatedCard._card.show();
		scene->_aSound2.play(61);

		Common::Point pt(91, 174);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);
		}
		break;
	case 2: {
		scene->_gameBoardSide[2]._handCard[3]._cardId = 2;
		scene->_gameBoardSide[2]._handCard[3]._card.postInit();
		scene->_gameBoardSide[2]._handCard[3]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[3]._card.setPosition(scene->_gameBoardSide[2]._handCard[3]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[3]._card.setStrip(1);
		scene->_gameBoardSide[2]._handCard[3]._card.setFrame(2);
		scene->_gameBoardSide[2]._handCard[3]._card.fixPriority(170);

		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._handCard[3]);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		scene->actionDisplay(1331, 12, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[2]._outpostStation[1]._cardId = 1;
		scene->_gameBoardSide[2]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[1]._card.hide();

		scene->_animatedCard._card.setStrip(scene->_gameBoardSide[2]._handCard[2]._card._strip);
		scene->_animatedCard._card.setFrame(scene->_gameBoardSide[2]._handCard[2]._card._frame);
		scene->_animatedCard._card.animate(ANIM_MODE_NONE, NULL);

		scene->_gameBoardSide[2]._handCard[2]._cardId = 0;
		scene->_gameBoardSide[2]._handCard[2]._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._handCard[2]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._outpostStation[1]._stationPos, this);
		}
		break;
	case 3: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[1]);
		scene->_aSound1.play(59);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 13, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[2]._outpostStation[1]._cardId = scene->_gameBoardSide[2]._handCard[3]._cardId;

		scene->_animatedCard._card.setStrip(scene->_gameBoardSide[2]._handCard[3]._card._strip);
		scene->_animatedCard._card.setFrame(scene->_gameBoardSide[2]._handCard[3]._card._frame);

		scene->_gameBoardSide[2]._handCard[3]._cardId = 0;
		scene->_gameBoardSide[2]._handCard[3]._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._handCard[3]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._outpostStation[1]._stationPos, this);
		}
		break;
	case 4: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[1]);
		scene->_aSound1.play(59);

		scene->_discardPile._cardId = 1;
		scene->_discardPile._card.hide();

		scene->_animatedCard._card.setStrip(5);
		scene->_animatedCard._card.setFrame(1);
		scene->_animatedCard._card.animate(ANIM_MODE_2, NULL);
		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._outpostStation[1]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_discardPile._stationPos, this);
		}
		break;
	case 5: {
		scene->_animatedCard._card.hide();

		scene->_discardPile._card.postInit();
		scene->_discardPile._card.setVisage(1332);
		scene->_discardPile._card.setPosition(scene->_discardPile._stationPos, 0);
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 14, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[2]._delayCard._card.postInit();
		scene->_gameBoardSide[2]._delayCard._card.setVisage(1332);
		scene->_gameBoardSide[2]._delayCard._card.setPosition(scene->_gameBoardSide[2]._delayCard._stationPos, 0);
		scene->_gameBoardSide[2]._delayCard._card.hide();

		scene->_gameBoardSide[3]._handCard[2]._cardId = 0;
		scene->_gameBoardSide[3]._handCard[2].remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[3]._handCard[2]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._delayCard._stationPos, this);
		}
		break;
	case 6: {
		scene->_animatedCard._card.hide();
		scene->_gameBoardSide[2]._delayCard._cardId = 21;
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._delayCard);
		scene->_aSound1.play(57);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 15, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		int tmpVal = 15;
		int i = -1;

		for (i = 0; i <= 7; i++) {
			tmpVal += 29;

			scene->_upperDisplayCard[i].postInit();
			scene->_upperDisplayCard[i].setVisage(1332);
			scene->_upperDisplayCard[i].setPosition(Common::Point(tmpVal, 90), 0);
			scene->_upperDisplayCard[i].setStrip(3);
			scene->_upperDisplayCard[i].fixPriority(190);

			scene->_lowerDisplayCard[i].postInit();
			scene->_lowerDisplayCard[i].setVisage(1332);
			scene->_lowerDisplayCard[i].setPosition(Common::Point(tmpVal, 90), 0);
			scene->_lowerDisplayCard[i].setStrip(7);
			scene->_lowerDisplayCard[i].setFrame(1);
			scene->_lowerDisplayCard[i].fixPriority(180);
		}

		scene->_upperDisplayCard[0].setFrame(1);
		scene->_upperDisplayCard[1].setFrame(3);
		scene->_upperDisplayCard[2].setFrame(6);
		scene->_upperDisplayCard[3].setFrame(8);
		scene->_upperDisplayCard[4].setFrame(9);
		scene->_upperDisplayCard[5].setFrame(10);
		scene->_upperDisplayCard[6].setFrame(11);
		scene->_upperDisplayCard[7].setFrame(12);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(240);

		scene->_upperDisplayCard[0].remove();
		scene->_upperDisplayCard[1].remove();
		scene->_upperDisplayCard[2].remove();
		scene->_upperDisplayCard[3].remove();
		scene->_upperDisplayCard[4].remove();
		scene->_upperDisplayCard[5].remove();
		scene->_upperDisplayCard[6].remove();
		scene->_upperDisplayCard[7].remove();

		scene->_lowerDisplayCard[0].remove();
		scene->_lowerDisplayCard[1].remove();
		scene->_lowerDisplayCard[2].remove();
		scene->_lowerDisplayCard[3].remove();
		scene->_lowerDisplayCard[4].remove();
		scene->_lowerDisplayCard[5].remove();
		scene->_lowerDisplayCard[6].remove();
		scene->_lowerDisplayCard[7].remove();

		scene->_discardPile._cardId = scene->_gameBoardSide[2]._delayCard._cardId;

		scene->_gameBoardSide[2]._delayCard._cardId = 0;
		scene->_gameBoardSide[2]._delayCard._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._delayCard._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_discardPile._stationPos, this);
		}
		break;
	case 7: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);

		R2_GLOBALS._sceneObjects->draw();

		scene->_gameBoardSide[2]._delayCard._card.postInit();
		scene->_gameBoardSide[2]._delayCard._card.setVisage(1332);
		scene->_gameBoardSide[2]._delayCard._card.setPosition(scene->_gameBoardSide[2]._delayCard._stationPos, 0);
		scene->_gameBoardSide[2]._delayCard._card.hide();

		scene->_gameBoardSide[3]._handCard[1]._cardId = 0;
		scene->_gameBoardSide[3]._handCard[1].remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[3]._handCard[1]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._delayCard._stationPos, this);
		}
		break;
	case 8: {
		scene->_animatedCard._card.hide();
		scene->_gameBoardSide[2]._delayCard._cardId = 14;
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._delayCard);
		scene->_aSound1.play(57);

		R2_GLOBALS._sceneObjects->draw();

		scene->actionDisplay(1331, 16, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		int tmpVal = 72;
		int i = -1;

		for (i = 0; i <= 3; i++) {
			tmpVal += 29;
			scene->_upperDisplayCard[i].postInit();
			scene->_upperDisplayCard[i].setVisage(1332);
			scene->_upperDisplayCard[i].setPosition(Common::Point(tmpVal, 71), 0);
			scene->_upperDisplayCard[i].setStrip(3);
			scene->_upperDisplayCard[i].fixPriority(190);

			scene->_lowerDisplayCard[i].postInit();
			scene->_lowerDisplayCard[i].setVisage(1332);
			scene->_lowerDisplayCard[i].setPosition(Common::Point(tmpVal, 71), 0);
			scene->_lowerDisplayCard[i].setStrip(7);
			scene->_lowerDisplayCard[i].setFrame(1);
			scene->_lowerDisplayCard[i].fixPriority(180);
		}

		scene->_upperDisplayCard[0].setFrame(2);
		scene->_upperDisplayCard[1].setFrame(5);
		scene->_upperDisplayCard[2].setFrame(7);
		scene->_upperDisplayCard[3].setFrame(15);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(240);
		scene->actionDisplay(1331, 17, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		tmpVal = 72;
		for (i = 4; i <= 7; i++) {
			tmpVal += 29;

			scene->_upperDisplayCard[i].postInit();
			scene->_upperDisplayCard[i].setVisage(1332);
			scene->_upperDisplayCard[i].setPosition(Common::Point(tmpVal, 100), 0);
			scene->_upperDisplayCard[i].setStrip(4);
			scene->_upperDisplayCard[i].fixPriority(190);

			scene->_lowerDisplayCard[i].postInit();
			scene->_lowerDisplayCard[i].setVisage(1332);
			scene->_lowerDisplayCard[i].setPosition(Common::Point(tmpVal, 100), 0);
			scene->_lowerDisplayCard[i].setStrip(7);
			scene->_lowerDisplayCard[i].setFrame(1);
			scene->_lowerDisplayCard[i].fixPriority(180);
		}

		scene->_upperDisplayCard[4].setFrame(1);
		scene->_upperDisplayCard[5].setFrame(5);
		scene->_upperDisplayCard[6].setFrame(7);
		scene->_upperDisplayCard[7].setFrame(3);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(240);

		scene->_upperDisplayCard[0].remove();
		scene->_upperDisplayCard[1].remove();
		scene->_upperDisplayCard[2].remove();
		scene->_upperDisplayCard[3].remove();
		scene->_upperDisplayCard[4].remove();
		scene->_upperDisplayCard[5].remove();
		scene->_upperDisplayCard[6].remove();
		scene->_upperDisplayCard[7].remove();

		scene->_lowerDisplayCard[0].remove();
		scene->_lowerDisplayCard[1].remove();
		scene->_lowerDisplayCard[2].remove();
		scene->_lowerDisplayCard[3].remove();
		scene->_lowerDisplayCard[4].remove();
		scene->_lowerDisplayCard[5].remove();
		scene->_lowerDisplayCard[6].remove();
		scene->_lowerDisplayCard[7].remove();

		scene->_discardPile._cardId = scene->_gameBoardSide[2]._handCard[0]._cardId;

		scene->_animatedCard._card.setStrip(scene->_gameBoardSide[2]._handCard[0]._card._strip);
		scene->_animatedCard._card.setFrame(scene->_gameBoardSide[2]._handCard[0]._card._frame);
		scene->_animatedCard._card.animate(ANIM_MODE_NONE, NULL);

		scene->_gameBoardSide[2]._handCard[0]._cardId = 0;
		scene->_gameBoardSide[2]._handCard[0]._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._handCard[0]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._delayCard._stationPos, this);
		}
		break;
	case 9: {
		scene->_aSound1.play(58);
		scene->_gameBoardSide[2]._delayCard._cardId = 0;
		scene->_gameBoardSide[2]._delayCard.remove();
		scene->_animatedCard._card.setStrip(5);
		scene->_animatedCard._card.setFrame(1);
		scene->_animatedCard._card.animate(ANIM_MODE_2, NULL);
		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._delayCard._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_discardPile._stationPos, this);
		}
		break;
	case 10: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);

		R2_GLOBALS._sceneObjects->draw();
		scene->actionDisplay(1331, 18, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_upperDisplayCard[0].postInit();
		scene->_upperDisplayCard[0].setVisage(1332);
		scene->_upperDisplayCard[0].setPosition(Common::Point(131, 71), 0);
		scene->_upperDisplayCard[0].fixPriority(190);
		scene->_upperDisplayCard[0].setStrip(3);
		scene->_upperDisplayCard[0].setFrame(4);

		scene->_lowerDisplayCard[0].postInit();
		scene->_lowerDisplayCard[0].setVisage(1332);
		scene->_lowerDisplayCard[0].setPosition(Common::Point(131, 71), 0);
		scene->_lowerDisplayCard[0].setStrip(7);
		scene->_lowerDisplayCard[0].setFrame(1);
		scene->_lowerDisplayCard[0].fixPriority(180);

		scene->_upperDisplayCard[1].postInit();
		scene->_upperDisplayCard[1].setVisage(1332);
		scene->_upperDisplayCard[1].setPosition(Common::Point(160, 71), 0);
		scene->_upperDisplayCard[1].fixPriority(190);
		scene->_upperDisplayCard[1].setStrip(3);
		scene->_upperDisplayCard[1].setFrame(16);

		scene->_lowerDisplayCard[1].postInit();
		scene->_lowerDisplayCard[1].setVisage(1332);
		scene->_lowerDisplayCard[1].setPosition(Common::Point(160, 71), 0);
		scene->_lowerDisplayCard[1].setStrip(7);
		scene->_lowerDisplayCard[1].setFrame(1);
		scene->_lowerDisplayCard[1].fixPriority(180);

		scene->_upperDisplayCard[2].postInit();
		scene->_upperDisplayCard[2].setVisage(1332);
		scene->_upperDisplayCard[2].setPosition(Common::Point(131, 100), 0);
		scene->_upperDisplayCard[2].fixPriority(190);
		scene->_upperDisplayCard[2].setStrip(4);
		scene->_upperDisplayCard[2].setFrame(4);

		scene->_lowerDisplayCard[2].postInit();
		scene->_lowerDisplayCard[2].setVisage(1332);
		scene->_lowerDisplayCard[2].setPosition(Common::Point(131, 100), 0);
		scene->_lowerDisplayCard[2].setStrip(7);
		scene->_lowerDisplayCard[2].setFrame(1);
		scene->_lowerDisplayCard[2].fixPriority(180);

		scene->_upperDisplayCard[3].postInit();
		scene->_upperDisplayCard[3].setVisage(1332);
		scene->_upperDisplayCard[3].setPosition(Common::Point(160, 100), 0);
		scene->_upperDisplayCard[3].fixPriority(190);
		scene->_upperDisplayCard[3].setStrip(4);
		scene->_upperDisplayCard[3].setFrame(2);

		scene->_lowerDisplayCard[3].postInit();
		scene->_lowerDisplayCard[3].setVisage(1332);
		scene->_lowerDisplayCard[3].setPosition(Common::Point(160, 100), 0);
		scene->_lowerDisplayCard[3].setStrip(7);
		scene->_lowerDisplayCard[3].setFrame(1);
		scene->_lowerDisplayCard[3].fixPriority(180);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(240);

		scene->_upperDisplayCard[0].remove();
		scene->_upperDisplayCard[1].remove();
		scene->_upperDisplayCard[2].remove();
		scene->_upperDisplayCard[3].remove();

		scene->_lowerDisplayCard[0].remove();
		scene->_lowerDisplayCard[1].remove();
		scene->_lowerDisplayCard[2].remove();
		scene->_lowerDisplayCard[3].remove();

		scene->_currentPlayerArrow.setFrame(1);
		scene->_currentPlayerArrow.show();
		scene->_currentPlayerArrow.animate(ANIM_MODE_2, NULL);

		R2_GLOBALS._sceneObjects->draw();

		scene->actionDisplay(1331, 19, 159, 10, 1, 220, 0, 7, 0, 154, 154);

		scene->_currentPlayerArrow.hide();

		scene->actionDisplay(1331, 20, 159, 10, 1, 220, 0, 7, 0, 154, 154);
		scene->actionDisplay(1331, 21, 159, 10, 1, 220, 0, 7, 0, 154, 154);

		scene->_discardPile._cardId = scene->_gameBoardSide[2]._handCard[1]._cardId;

		scene->_animatedCard._card.setStrip(scene->_gameBoardSide[2]._handCard[1]._card._strip);
		scene->_animatedCard._card.setFrame(scene->_gameBoardSide[2]._handCard[1]._card._frame);
		scene->_animatedCard._card.animate(ANIM_MODE_NONE, NULL);

		scene->_gameBoardSide[2]._handCard[1]._cardId = 0;
		scene->_gameBoardSide[2]._handCard[1]._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._handCard[1]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_discardPile._stationPos, this);
		}
		break;
	case 11: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);
		scene->_animatedCard._card.setStrip(5);
		scene->_animatedCard._card.setFrame(1);
		scene->_animatedCard._card.animate(ANIM_MODE_2, NULL);

		R2_GLOBALS._sceneObjects->draw();

		scene->actionDisplay(1331, 22, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		int i = -1;
		for (i = 0; i <= 3; i ++) {
			scene->_gameBoardSide[3]._handCard[i]._cardId = 0;
			scene->_gameBoardSide[3]._handCard[i]._card.remove();

			scene->_gameBoardSide[2]._handCard[i]._cardId = 0;
			scene->_gameBoardSide[2]._handCard[i]._card.remove();

			scene->_gameBoardSide[0]._handCard[i]._cardId = 0;
			scene->_gameBoardSide[0]._handCard[i]._card.remove();

			scene->_gameBoardSide[1]._handCard[i]._cardId = 0;
			scene->_gameBoardSide[1]._handCard[i]._card.remove();
		}

		for (i = 0; i <= 7; i++) {
			scene->_gameBoardSide[3]._outpostStation[i]._cardId = 0;
			scene->_gameBoardSide[3]._outpostStation[i]._card.remove();

			scene->_gameBoardSide[2]._outpostStation[i]._cardId = 0;
			scene->_gameBoardSide[2]._outpostStation[i]._card.remove();

			scene->_gameBoardSide[0]._outpostStation[i]._cardId = 0;
			scene->_gameBoardSide[0]._outpostStation[i]._card.remove();

			scene->_gameBoardSide[1]._outpostStation[i]._cardId = 0;
			scene->_gameBoardSide[1]._outpostStation[i]._card.remove();
		}

		scene->_gameBoardSide[2]._delayCard._cardId = 0;
		scene->_gameBoardSide[2]._delayCard._card.remove();

		scene->_discardPile._cardId = 0;
		scene->_discardPile._card.remove();

		scene->_stockPile.remove();
		}
		// fall through
	case 0:
		R2_GLOBALS._sceneObjects->draw();
		signal();
		break;
	case 12:
		scene->suggestInstructions();
		remove();
		break;
	default:
		break;
	}
}

/**
 * Shuffle cards animation
 */
void Scene1337::Action2::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_shuffleAnimation._card.postInit();
		scene->_shuffleAnimation._card.setVisage(1332);
		scene->_shuffleAnimation._card.setStrip(8);
		scene->_shuffleAnimation._card.setFrame(1);
		scene->_shuffleAnimation._card.fixPriority(300);
		scene->_shuffleAnimation._card.setPosition(Common::Point(156, 108));

		scene->_discardPile._card.remove();
		scene->_discardPile._cardId = 0;

		scene->_aSound1.play(60);
		scene->_shuffleAnimation._card.animate(ANIM_MODE_5, this);
		break;
	case 1:
		scene->_shuffleAnimation._card.setFrame(1);

		scene->_aSound1.play(60);
		scene->_shuffleAnimation._card.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		Common::Point pt(156, 108);
		NpcMover *mover = new NpcMover();
		scene->_shuffleAnimation._card.addMover(mover, &pt, this);
		}
		break;
	case 3:
		scene->_shuffleAnimation._card.remove();
		scene->_stockPile.setup(1332, 5, 1);
		scene->_stockPile.setPosition(Common::Point(162, 95));
		scene->_stockPile.setPriority(110);
		scene->_stockPile._effect = EFFECT_SHADED;
		scene->_stockPile.show();
		scene->_shuffleEndedFl = true;
		break;
	default:
		break;
	}
}

/**
 * Deal cards
 */
void Scene1337::Action3::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	scene->_animatedCard._card.setPosition(Common::Point(162, 95), 0);

	switch (_actionIndex++) {
	case 0: {
		scene->_animatedCard._card._moveDiff = Common::Point(30, 30);
		scene->_animatedCard._card.setVisage(1332);
		scene->_animatedCard._card.setStrip(5);
		scene->_animatedCard._card.setFrame(1);
		scene->_animatedCard._card.fixPriority(400);
		scene->_animatedCard._card.animate(ANIM_MODE_2, NULL);
		scene->_aSound2.play(61);

		Common::Point pt(283, 146);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_animatedCard._card.show();
		scene->_gameBoardSide[1]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 1: {
		scene->_gameBoardSide[1]._handCard[0]._card.postInit();
		scene->_gameBoardSide[1]._handCard[0]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[1]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[0]._card.setPosition(scene->_gameBoardSide[1]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[0]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[0]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(10, 174);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[2]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 2: {
		scene->_gameBoardSide[2]._handCard[0]._card.postInit();
		scene->_gameBoardSide[2]._handCard[0]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[2]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[0]._card.setPosition(scene->_gameBoardSide[2]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[0]._card.fixPriority(170);
		if (scene->_gameBoardSide[2]._handCard[0]._cardId > 25) {
			scene->_gameBoardSide[2]._handCard[0]._card.setStrip(4);
			scene->_gameBoardSide[2]._handCard[0]._card.setFrame(scene->_gameBoardSide[2]._handCard[0]._cardId - 25);
		} else if (scene->_gameBoardSide[2]._handCard[0]._cardId > 9) {
			scene->_gameBoardSide[2]._handCard[0]._card.setStrip(3);
			scene->_gameBoardSide[2]._handCard[0]._card.setFrame(scene->_gameBoardSide[2]._handCard[0]._cardId - 9);
		} else {
			scene->_gameBoardSide[2]._handCard[0]._card.setStrip(2);
			scene->_gameBoardSide[2]._handCard[0]._card.setFrame(scene->_gameBoardSide[2]._handCard[0]._cardId);
		}
		scene->_aSound2.play(61);

		Common::Point pt(14, 14);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[3]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 3: {
		scene->_gameBoardSide[3]._handCard[0]._card.postInit();
		scene->_gameBoardSide[3]._handCard[0]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[3]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[0]._card.setPosition(scene->_gameBoardSide[3]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[0]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[0]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(280, 5);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[0]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 4: {
		scene->_gameBoardSide[0]._handCard[0]._card.postInit();
		scene->_gameBoardSide[0]._handCard[0]._card._moveDiff = Common::Point(30,30);
		scene->_gameBoardSide[0]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[0]._card.setPosition(scene->_gameBoardSide[0]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[0]._card.setStrip(5);
		scene->_gameBoardSide[0]._handCard[0]._card.setFrame(1);
		scene->_gameBoardSide[0]._handCard[0]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(283, 124);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[1]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 5: {
		scene->_gameBoardSide[1]._handCard[1]._card.postInit();
		scene->_gameBoardSide[1]._handCard[1]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[1]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[1]._card.setPosition(scene->_gameBoardSide[1]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[1]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[1]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(37, 174);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[2]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 6: {
		scene->_gameBoardSide[2]._handCard[1]._card.postInit();
		scene->_gameBoardSide[2]._handCard[1]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[2]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[1]._card.setPosition(scene->_gameBoardSide[2]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[1]._card.fixPriority(170);

		if (scene->_gameBoardSide[2]._handCard[1]._cardId > 25) {
			scene->_gameBoardSide[2]._handCard[1]._card.setStrip(4);
			scene->_gameBoardSide[2]._handCard[1]._card.setFrame(scene->_gameBoardSide[2]._handCard[1]._cardId - 25);
		} else if (scene->_gameBoardSide[2]._handCard[1]._cardId > 9) {
			scene->_gameBoardSide[2]._handCard[1]._card.setStrip(3);
			scene->_gameBoardSide[2]._handCard[1]._card.setFrame(scene->_gameBoardSide[2]._handCard[1]._cardId - 9);
		} else {
			scene->_gameBoardSide[2]._handCard[1]._card.setStrip(2);
			scene->_gameBoardSide[2]._handCard[1]._card.setFrame(scene->_gameBoardSide[2]._handCard[1]._cardId);
		}

		scene->_aSound2.play(61);

		Common::Point pt(14, 36);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[3]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 7: {
		scene->_gameBoardSide[3]._handCard[1]._card.postInit();
		scene->_gameBoardSide[3]._handCard[1]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[3]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[1]._card.setPosition(scene->_gameBoardSide[3]._handCard[1]._stationPos);
		scene->_gameBoardSide[3]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[1]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[1]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(253, 5);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[0]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 8: {
		scene->_gameBoardSide[0]._handCard[1]._card.postInit();
		scene->_gameBoardSide[0]._handCard[1]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[0]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[1]._card.setPosition(scene->_gameBoardSide[0]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[1]._card.setStrip(5);
		scene->_gameBoardSide[0]._handCard[1]._card.setFrame(1);
		scene->_gameBoardSide[0]._handCard[1]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(283, 102);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[1]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 9: {
		scene->_gameBoardSide[1]._handCard[2]._card.postInit();
		scene->_gameBoardSide[1]._handCard[2]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[1]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[2]._card.setPosition(scene->_gameBoardSide[1]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[2]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[2]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(64, 174);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[2]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 10: {
		scene->_gameBoardSide[2]._handCard[2]._card.postInit();
		scene->_gameBoardSide[2]._handCard[2]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[2]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[2]._card.setPosition(scene->_gameBoardSide[2]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[2]._card.fixPriority(170);

		if (scene->_gameBoardSide[2]._handCard[2]._cardId > 25) {
			scene->_gameBoardSide[2]._handCard[2]._card.setStrip(4);
			scene->_gameBoardSide[2]._handCard[2]._card.setFrame(scene->_gameBoardSide[2]._handCard[2]._cardId - 25);
		} else if (scene->_gameBoardSide[2]._handCard[2]._cardId > 9) {
			scene->_gameBoardSide[2]._handCard[2]._card.setStrip(3);
			scene->_gameBoardSide[2]._handCard[2]._card.setFrame(scene->_gameBoardSide[2]._handCard[2]._cardId - 9);
		} else {
			scene->_gameBoardSide[2]._handCard[2]._card.setStrip(2);
			scene->_gameBoardSide[2]._handCard[2]._card.setFrame(scene->_gameBoardSide[2]._handCard[2]._cardId);
		}

		scene->_aSound2.play(61);

		Common::Point pt(14, 58);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[3]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 11: {
		scene->_gameBoardSide[3]._handCard[2]._card.postInit();
		scene->_gameBoardSide[3]._handCard[2]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[3]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[2]._card.setPosition(scene->_gameBoardSide[3]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[2]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[2]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(226, 5);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[0]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 12:
		scene->_gameBoardSide[0]._handCard[2]._card.postInit();
		scene->_gameBoardSide[0]._handCard[2]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[0]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[2]._card.setPosition(scene->_gameBoardSide[0]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[2]._card.setStrip(5);
		scene->_gameBoardSide[0]._handCard[2]._card.setFrame(1);
		scene->_gameBoardSide[0]._handCard[2]._card.fixPriority(170);
		scene->_animatedCard._card.hide();
	default:
		break;
	}

	if (_actionIndex > 12) {
		scene->_currentPlayerNumb = 0;
		R2_GLOBALS._sceneObjects->draw();
		scene->actionDisplay(1330, 0, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		scene->handleNextTurn();
	} else if (_actionIndex >= 1) {
		scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
		scene->_cardsAvailableNumb--;
	}
}

/**
 * Action used to handle the other players' turn
 */
void Scene1337::Action4::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if ( (scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._cardId == 0)
		  && (!scene->isStationCard(scene->_gameBoardSide[scene->_currentPlayerNumb]._delayCard._cardId))) {
			if (scene->_cardsAvailableNumb < 0)
				scene->shuffleCards();
			scene->_animatedCard._card.setPosition(Common::Point(162, 95), 0);
			scene->_animatedCard._card.show();
			scene->_aSound2.play(61);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[scene->_currentPlayerNumb]._card1Pos, this);

			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
			scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
			scene->_cardsAvailableNumb--;

			if (scene->_cardsAvailableNumb < 0)
				scene->_stockPile.remove();
		} else {
			// Self call, forcing next actionIndex
			signal();
		}
		break;
	case 1:
		if ( (scene->_animatedCard._card._position.x == scene->_gameBoardSide[scene->_currentPlayerNumb]._card1Pos.x)
		  && (scene->_animatedCard._card._position.y == scene->_gameBoardSide[scene->_currentPlayerNumb]._card1Pos.y) ) {
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.postInit();
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card._moveDiff = Common::Point(30, 30);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.setVisage(1332);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.setPosition(scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._stationPos, 0);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.setStrip(1);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.setFrame(scene->_gameBoardSide[scene->_currentPlayerNumb]._frameNum);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.fixPriority(170);
		}

		if ((R2_GLOBALS._debugCardGame) || (scene->_currentPlayerNumb == 2))
			scene->setAnimationInfo(&scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]);

		scene->_animatedCard._card.hide();
		if ( (scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._cardId == 0)
		  && (!scene->isStationCard(scene->_gameBoardSide[scene->_currentPlayerNumb]._delayCard._cardId))) {
			if (scene->_cardsAvailableNumb < 0)
				scene->shuffleCards();
			scene->_animatedCard._card.setPosition(Common::Point(162, 95));
			scene->_animatedCard._card.show();

			scene->_aSound2.play(61);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[scene->_currentPlayerNumb]._card2Pos, this);

			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
			scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
			scene->_cardsAvailableNumb--;
			if (scene->_cardsAvailableNumb < 0)
				scene->_stockPile.remove();
		} else
			signal();
		break;
	case 2:
		if ( (scene->_animatedCard._card._position.x == scene->_gameBoardSide[scene->_currentPlayerNumb]._card2Pos.x)
		  && (scene->_animatedCard._card._position.y == scene->_gameBoardSide[scene->_currentPlayerNumb]._card2Pos.y) ) {
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.postInit();
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card._moveDiff = Common::Point(30, 30);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.setVisage(1332);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.setPosition(scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._stationPos, 0);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.setStrip(1);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.setFrame(scene->_gameBoardSide[scene->_currentPlayerNumb]._frameNum);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.fixPriority(170);
		}

		if ((R2_GLOBALS._debugCardGame) || (scene->_currentPlayerNumb == 2))
			scene->setAnimationInfo(&scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]);

		scene->_animatedCard._card.hide();
		if ( (scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._cardId == 0)
		  && (!scene->isStationCard(scene->_gameBoardSide[scene->_currentPlayerNumb]._delayCard._cardId))) {
			if (scene->_cardsAvailableNumb < 0)
				scene->shuffleCards();
			scene->_animatedCard._card.setPosition(Common::Point(162, 95));
			scene->_animatedCard._card.show();

			scene->_aSound2.play(61);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[scene->_currentPlayerNumb]._card3Pos, this);

			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
			scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
			scene->_cardsAvailableNumb--;
			if (scene->_cardsAvailableNumb < 0)
				scene->_stockPile.remove();
		} else
			signal();
		break;
	case 3:
		if ( (scene->_animatedCard._card._position.x == scene->_gameBoardSide[scene->_currentPlayerNumb]._card3Pos.x)
		  && (scene->_animatedCard._card._position.y == scene->_gameBoardSide[scene->_currentPlayerNumb]._card3Pos.y) ) {
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.postInit();
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card._moveDiff = Common::Point(30, 30);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.setVisage(1332);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.setPosition(scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._stationPos, 0);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.setStrip(1);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.setFrame(scene->_gameBoardSide[scene->_currentPlayerNumb]._frameNum);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.fixPriority(170);
		}

		if ((R2_GLOBALS._debugCardGame) || (scene->_currentPlayerNumb == 2))
			scene->setAnimationInfo(&scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]);

		scene->_animatedCard._card.hide();
		if ( (scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._cardId == 0)
		  && (!scene->isStationCard(scene->_gameBoardSide[scene->_currentPlayerNumb]._delayCard._cardId))) {
			if (scene->_cardsAvailableNumb < 0)
				scene->shuffleCards();
			scene->_animatedCard._card.setPosition(Common::Point(162, 95));
			scene->_animatedCard._card.show();

			scene->_aSound2.play(61);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[scene->_currentPlayerNumb]._card4Pos, this);

			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
			scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
			scene->_cardsAvailableNumb--;
			if (scene->_cardsAvailableNumb < 0)
				scene->_stockPile.remove();
		} else
			signal();
		break;
	case 4:
		if ( (scene->_animatedCard._card._position.x == scene->_gameBoardSide[scene->_currentPlayerNumb]._card4Pos.x)
		  && (scene->_animatedCard._card._position.y == scene->_gameBoardSide[scene->_currentPlayerNumb]._card4Pos.y) ) {
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.postInit();
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card._moveDiff = Common::Point(30, 30);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.setVisage(1332);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.setPosition(scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._stationPos, 0);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.setStrip(1);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.setFrame(scene->_gameBoardSide[scene->_currentPlayerNumb]._frameNum);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.fixPriority(170);
		}

		if ((R2_GLOBALS._debugCardGame) || (scene->_currentPlayerNumb == 2))
			scene->setAnimationInfo(&scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]);

		scene->_animatedCard._card.hide();
		switch (scene->_currentPlayerNumb) {
		case 0:
			scene->handlePlayer0();
			break;
		case 1:
			scene->handlePlayer1();
			break;
		case 2:
			scene->handleAutoplayPlayer2();
			break;
		case 3:
			scene->handlePlayer3();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

/**
 * Animations for discarding a card
 */
void Scene1337::Action5::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard1->_cardId;
		scene->_currentDiscardIndex--;
		if (!g_globals->_sceneObjects->contains(&scene->_discardPile._card)) {
			// The first discarded card makes the pile appear
			scene->_discardPile._card.postInit();
			scene->_discardPile._card.hide();
			scene->_discardPile._card.setVisage(1332);
			scene->_discardPile._card.setPosition(scene->_discardPile._stationPos, 0);
			scene->_discardPile._card.fixPriority(170);
		}

		scene->_discardPile._cardId = scene->_actionCard1->_cardId;
		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}
		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();
		Common::Point pt(128, 95);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);
		scene->handleNextTurn();
		break;
	default:
		break;
	}
}

/**
 * Animations for playing a platform card
 */
void Scene1337::Action6::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard2->_cardId = 1;
		scene->_actionCard2->_card.postInit();
		scene->_actionCard2->_card.hide();
		scene->_actionCard2->_card.setVisage(1332);
		scene->_actionCard2->_card.setPosition(scene->_actionCard2->_stationPos);
		scene->_actionCard2->_card.fixPriority(170);

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(59);
		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}
		scene->handleNextTurn();
		break;
	default:
		break;
	}
}

/**
 * Upgrade platform to station by playing a station card on top of it
 */
void Scene1337::Action7::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(59);
		scene->_discardedPlatformCard._cardId = 1;
		scene->_discardedPlatformCard._stationPos = scene->_actionCard2->_stationPos;
		scene->_discardedPlatformCard._card.postInit();
		scene->_discardedPlatformCard._card.hide();
		scene->_discardedPlatformCard._card._flags = OBJFLAG_HIDING;

		scene->discardCard(&scene->_discardedPlatformCard);
		break;
	default:
		break;
	}
}

// Remove a delay card
void Scene1337::Action8::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard2->_cardId;
		scene->_currentDiscardIndex--;

		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();

		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(58);
		scene->discardCard(scene->_actionCard2);
		break;
	default:
		break;
	}
}

// Play delay card
void Scene1337::Action9::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;
		scene->_actionCard2->_card.postInit();
		scene->_actionCard2->_card.hide();
		scene->_actionCard2->_card.setVisage(1332);
		scene->_actionCard2->_card.setPosition(scene->_actionCard2->_stationPos, 0);
		scene->_actionCard2->_card.fixPriority(170);

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(57);

		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}

		scene->handleNextTurn();
		break;
	default:
		break;
	}
}

// Play a card on the central outpost.
// This card is either a counter-trick card or a meteor card
void Scene1337::Action10::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard3->_card.postInit();
		scene->_actionCard3->_card.hide();
		scene->_actionCard3->_card.setVisage(1332);
		scene->_actionCard3->_card.setPosition(scene->_actionCard3->_stationPos, 0);
		scene->_actionCard3->_card.fixPriority(170);
		scene->_actionCard3->_cardId = scene->_actionCard1->_cardId;

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard3->_stationPos, this);
		}
		break;
	case 1: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard3);
		scene->_aSound1.play(57);

		bool found = false;
		int indexFound = -1;

		switch (scene->_actionPlayerIdx) {
		case 0:
			for (indexFound = 0; indexFound < 3; indexFound++) {
				// Check for the presence of an interceptor card
				if (scene->_gameBoardSide[0]._handCard[indexFound]._cardId == 29) {
					found = true;
					break;
				}
			}
			break;
		case 1:
			for (indexFound = 0; indexFound < 3; indexFound++) {
				// Check for the presence of an interceptor card
				if (scene->_gameBoardSide[1]._handCard[indexFound]._cardId == 29) {
					found = true;
					break;
				}
			}
			break;
		case 2:
			for (indexFound = 0; indexFound < 3; indexFound++) {
				// Check for the presence of an interceptor card
				if (scene->_gameBoardSide[2]._handCard[indexFound]._cardId == 29) {
					found = true;
					break;
				}
			}
			break;
		case 3:
			for (indexFound = 0; indexFound < 3; indexFound++) {
				// Check for the presence of an interceptor card
				if (scene->_gameBoardSide[3]._handCard[indexFound]._cardId == 29) {
					found = true;
					break;
				}
			}
			break;
		default:
			break;
		}

		bool found2 = false;

		if (found) {
			switch (scene->_actionPlayerIdx) {
			case 0:
				scene->playInterceptorCard(&scene->_gameBoardSide[0]._handCard[indexFound], scene->_actionCard3);
				found2 = true;
				break;
			case 1:
				scene->playInterceptorCard(&scene->_gameBoardSide[1]._handCard[indexFound], scene->_actionCard3);
				found2 = true;
				break;
			case 2:
				scene->subC4CD2();
				if (MessageDialog::show(USE_INTERCEPTOR, NO_MSG, YES_MSG) == 0)
					scene->subC4CEC();
				else {
					scene->playInterceptorCard(&scene->_gameBoardSide[2]._handCard[indexFound], scene->_actionCard3);
					found2 = true;
				}
				break;
			case 3:
				scene->playInterceptorCard(&scene->_gameBoardSide[3]._handCard[indexFound], scene->_actionCard3);
				found2 = true;
				break;
			default:
				break;
			}
		}

		if (found2)
			break;

		if (scene->_actionPlayerIdx == 2) {
			int stationCount = 0;
			for (int i = 0; i <= 7; i++) {
				if (scene->_gameBoardSide[2]._outpostStation[i]._cardId != 0)
					++stationCount;
			}

			if (stationCount <= 1) {
				for (int i = 0; i <= 7; i++) {
					if (scene->_gameBoardSide[2]._outpostStation[i]._cardId != 0) {
						scene->_actionCard2 = &scene->_gameBoardSide[2]._outpostStation[i];
						break;
					}
				}
			} else {
				scene->subC4CD2();

				found2 = false;
				while (!found2) {
					scene->actionDisplay(1330, 130, 159, 10, 1, 200, 0, 7, 0, 154, 154);

					// Wait for a mouse or keypress
					Event event;
					while (!g_globals->_events.getEvent(event, EVENT_BUTTON_DOWN | EVENT_KEYPRESS) && !g_vm->shouldQuit()) {
						g_globals->_scenePalette.signalListeners();
						R2_GLOBALS._sceneObjects->draw();
						g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
					}

					scene->_selectedCard._stationPos = event.mousePos;

					for (int i = 0; i <= 7; i++) {
						if (scene->_gameBoardSide[2]._outpostStation[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[2]._outpostStation[i]._cardId != 0)) {
							scene->_actionCard2 = &scene->_gameBoardSide[2]._outpostStation[i];
							found2 = true;
							break;
						}
					}
				}
				scene->subC4CEC();
			}
		}

		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard2->_cardId;
		scene->_currentDiscardIndex--;
		scene->_actionCard2->_cardId = 0;
		scene->_actionCard2->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard2->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard3->_stationPos, this);
		}
		break;
	case 2:
		scene->_animatedCard._card.hide();
		scene->discardCard(scene->_actionCard3);
		break;
	default:
		break;
	}
}

// Use Thief card (#25) and pick a card from the opponent
void Scene1337::Action11::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard2->_card.postInit();
		scene->_actionCard2->_card.hide();
		scene->_actionCard2->_card.setVisage(1332);
		scene->_actionCard2->_card.setPosition(scene->_actionCard2->_stationPos, 0);
		scene->_actionCard2->_card.fixPriority(170);
		scene->_actionCard2->_cardId = 25;

		if (scene->_actionPlayerIdx == 2) {
			scene->_animatedCard._card.setPosition(scene->_actionCard2->_stationPos, 0);
			scene->setCursorData(5, 1, 4);
		} else {
			scene->_actionCard1->_cardId = 0;
			scene->_actionCard1->_card.remove();
			scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		}
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(57);

		bool found = false;
		bool noAction = true;

		int i = -1;

		switch (scene->_actionVictimIdx) {
		case 0:
			for (i = 0; i <= 3; i++) {
				if (scene->_gameBoardSide[0]._handCard[i]._cardId == 27) {
					found = true;
					break;
				}
			}

			if ((found) && (scene->getRandomCardFromHand(scene->_actionPlayerIdx) != -1)) {
				scene->_actionCard1 = &scene->_gameBoardSide[0]._handCard[i];
				scene->_actionCard2 = &scene->_gameBoardSide[0]._emptyStationPos;
				if (scene->_actionPlayerIdx != 0) {
					int tmpVal = scene->getRandomCardFromHand(scene->_actionPlayerIdx);
					scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionPlayerIdx]._handCard[tmpVal];
				}
				scene->_actionItem.setAction(&scene->_action12);
				noAction = false;
			}
			break;
		case 1:
			for (i = 0; i <= 3; i++) {
				if (scene->_gameBoardSide[1]._handCard[i]._cardId == 27) {
					found = true;
					break;
				}
			}

			if ((found) && (scene->getRandomCardFromHand(scene->_actionPlayerIdx) != -1)) {
				scene->_actionCard1 = &scene->_gameBoardSide[1]._handCard[i];
				scene->_actionCard2 = &scene->_gameBoardSide[1]._emptyStationPos;
				if (scene->_actionPlayerIdx != 1) {
					int tmpVal = scene->getRandomCardFromHand(scene->_actionPlayerIdx);
					scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionPlayerIdx]._handCard[tmpVal];
				}
				scene->_actionItem.setAction(&scene->_action12);
				noAction = false;
			}
			break;
		case 2:
			for (i = 0; i <= 3; i++) {
				if (scene->_gameBoardSide[2]._handCard[i]._cardId == 27) {
					found = true;
					break;
				}
			}

			if ((found) && (scene->getRandomCardFromHand(scene->_actionPlayerIdx) != -1)) {
				scene->subC4CD2();
				if (MessageDialog::show(USE_DOUBLE_AGENT, NO_MSG, YES_MSG) == 0)
					scene->subC4CEC();
				else {
					scene->subC4CEC();
					scene->_actionCard1 = &scene->_gameBoardSide[2]._handCard[i];
					scene->_actionCard2 = &scene->_gameBoardSide[2]._emptyStationPos;
					if (scene->_actionPlayerIdx != 2) {
						int tmpVal = scene->getRandomCardFromHand(scene->_actionPlayerIdx);
						scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionPlayerIdx]._handCard[tmpVal];
					}
					scene->_actionItem.setAction(&scene->_action12);
					noAction = false;
				}
			}
			break;
		case 3:
			for (i = 0; i <= 3; i++) {
				if (scene->_gameBoardSide[3]._handCard[i]._cardId == 27) {
					found = true;
					break;
				}
			}

			if ((found) && (scene->getRandomCardFromHand(scene->_actionPlayerIdx) != -1)) {
				scene->_actionCard1 = &scene->_gameBoardSide[3]._handCard[i];
				scene->_actionCard2 = &scene->_gameBoardSide[3]._emptyStationPos;
				if (scene->_actionPlayerIdx != 3) {
					int tmpVal = scene->getRandomCardFromHand(scene->_actionPlayerIdx);
					scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionPlayerIdx]._handCard[tmpVal];
				}
				scene->_actionItem.setAction(&scene->_action12);
				noAction = false;
			}
			break;
		default:
			break;
		}

		if (!noAction)
			break;

		if (scene->_actionPlayerIdx == 2) {
			int count = 0;
			if (scene->_actionVictimIdx != 2) {
				for (i = 0; i <= 3; i++) {
					if (scene->_gameBoardSide[scene->_actionVictimIdx]._handCard[i]._cardId != 0)
						++count;
				}
			}

			if (count > 1) {
				scene->subC4CD2();

				found = false;
				while (!found) {
					switch (scene->_actionVictimIdx) {
					case 0:
						scene->actionDisplay(1330, 131, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					case 1:
						scene->actionDisplay(1330, 132, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					case 3:
						scene->actionDisplay(1330, 133, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					default:
						break;
					}

					Event event;
					while (!g_globals->_events.getEvent(event, EVENT_BUTTON_DOWN | EVENT_KEYPRESS) && !g_vm->shouldQuit()) {
						g_globals->_scenePalette.signalListeners();
						R2_GLOBALS._sceneObjects->draw();
						g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
					}

					scene->_selectedCard._stationPos = event.mousePos;

					found = false;

					if (scene->_actionVictimIdx != 2) {
						for (i = 0; i <= 3; i++) {
							if (scene->_gameBoardSide[scene->_actionVictimIdx]._handCard[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[scene->_actionVictimIdx]._handCard[i]._cardId != 0)) {
								scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionVictimIdx]._handCard[i];
								found = true;
								break;
							}
						}
					}
				} // while
				scene->_displayHelpFl = true;
				scene->subC4CEC();
			} else if (scene->_actionVictimIdx != 2) {
				int tmpVal = scene->getRandomCardFromHand(scene->_actionVictimIdx);
				scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionVictimIdx]._handCard[tmpVal];
			}
		}

		scene->_actionCard1->_card.postInit();
		scene->_actionCard1->_card.hide();
		scene->_actionCard1->_card.setVisage(1332);
		scene->_actionCard1->_card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_actionCard1->_card.fixPriority(170);
		scene->_actionCard1->_card.setStrip2(1);
		scene->_actionCard1->_cardId = scene->_actionCard3->_cardId;

		scene->_actionCard3->_cardId = 0;
		scene->_actionCard3->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard3->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard1->_stationPos, this);
		}
		break;
	case 2:
		scene->_animatedCard._card.hide();
		switch (scene->_actionPlayerIdx) {
		case 0:
			scene->_actionCard1->_card.setFrame2(2);
			scene->_actionCard1->_card.show();
			break;
		case 1:
			scene->_actionCard1->_card.setFrame2(4);
			scene->_actionCard1->_card.show();
			break;
		case 3:
			scene->_actionCard1->_card.setFrame2(3);
			scene->_actionCard1->_card.show();
			break;
		default:
			scene->setAnimationInfo(scene->_actionCard1);
			break;
		}

		scene->_currentPlayerNumb--;
		scene->_showPlayerTurn = false;
		scene->discardCard(scene->_actionCard2);
		break;
	default:
		break;
	}
}

// Pick a card in opponent hand
void Scene1337::Action12::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		signal();
		break;
	case 1: {
		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard2->_cardId;
		scene->_currentDiscardIndex++;
		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;
		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();
		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 2:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(58);
		if (scene->_actionVictimIdx == 2) {
			int count = 0;
			int i = -1;
			switch (scene->_actionPlayerIdx) {
			case 0:
				for (i = 0; i <= 3; i++) {
					if (scene->_gameBoardSide[0]._handCard[i]._cardId != 0)
						++count;
				}
				break;
			case 1:
				for (i = 0; i <= 3; i++) {
					// The original game was counting in the hand of player 3, which is obviously wrong
					if (scene->_gameBoardSide[1]._handCard[i]._cardId != 0)
						++count;
				}
				break;
			case 3:
				for (i = 0; i <= 3; i++) {
					if (scene->_gameBoardSide[3]._handCard[i]._cardId != 0)
						++count;
				}
				break;
			default:
				break;
			}

			if (count > 1) {
				scene->subC4CD2();

				bool found = false;

				while (!found) {
					switch (scene->_actionPlayerIdx) {
					case 0:
						scene->actionDisplay(1330, 131, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					case 1:
						scene->actionDisplay(1330, 132, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					case 3:
						scene->actionDisplay(1330, 133, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					default:
						break;
					}

					Event event;
					while (!g_globals->_events.getEvent(event, EVENT_BUTTON_DOWN | EVENT_KEYPRESS) && !g_vm->shouldQuit()) {
						g_globals->_scenePalette.signalListeners();
						R2_GLOBALS._sceneObjects->draw();
						g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
					}

					scene->_selectedCard._stationPos = event.mousePos;

					switch (scene->_actionPlayerIdx) {
					case 0:
						for (i = 0; i <= 3; i++) {
							if (scene->_gameBoardSide[0]._handCard[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[0]._handCard[i]._cardId != 0)) {
								found = true;
								scene->_actionCard3 = &scene->_gameBoardSide[0]._handCard[i];
								break;
							}
						}
						break;
					case 1:
						for (i = 0; i <= 3; i++) {
							if (scene->_gameBoardSide[1]._handCard[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[1]._handCard[i]._cardId != 0)) {
								found = true;
								scene->_actionCard3 = &scene->_gameBoardSide[1]._handCard[i];
								break;
							}
						}
						break;
					case 3:
						for (i = 0; i <= 3; i++) {
							if (scene->_gameBoardSide[3]._handCard[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[3]._handCard[i]._cardId != 0)) {
								found = true;
								scene->_actionCard3 = &scene->_gameBoardSide[3]._handCard[i];
								break;
							}
						}
						break;
					default:
						break;
					}
				}
				scene->subC4CEC();
			} else {
				switch (scene->_actionPlayerIdx) {
				case 0:
					scene->_actionCard3 = &scene->_gameBoardSide[0]._handCard[scene->getRandomCardFromHand(0)];
					break;
				case 1:
					scene->_actionCard3 = &scene->_gameBoardSide[1]._handCard[scene->getRandomCardFromHand(1)];
					break;
				case 3:
					scene->_actionCard3 = &scene->_gameBoardSide[3]._handCard[scene->getRandomCardFromHand(3)];
					break;
				default:
					break;
				}
			}

			scene->_actionCard1->_card.postInit();
			scene->_actionCard1->_card.hide();
			scene->_actionCard1->_card.setVisage(1332);
			scene->_actionCard1->_card.setPosition(scene->_actionCard1->_stationPos);
			scene->_actionCard1->_card.fixPriority(170);
			scene->_actionCard1->_card.setStrip2(1);
			scene->_actionCard1->_cardId = scene->_actionCard3->_cardId;

			scene->_actionCard3->_cardId = 0;
			scene->_actionCard3->_card.remove();

			scene->_animatedCard._card.setPosition(scene->_actionCard3->_stationPos);
			scene->_animatedCard._card.show();
			scene->_aSound1.play(57);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_actionCard1->_stationPos, this);
		}
		break;
	case 3:
		scene->_animatedCard._card.hide();
		switch (scene->_actionVictimIdx) {
		case 0:
			scene->_actionCard1->_card.setFrame2(2);
			scene->_actionCard1->_card.show();
			break;
		case 1:
			scene->_actionCard1->_card.setFrame2(4);
			scene->_actionCard1->_card.show();
			break;
		case 3:
			scene->_actionCard1->_card.setFrame2(3);
			scene->_actionCard1->_card.show();
			break;
		default:
			scene->setAnimationInfo(scene->_actionCard1);
			break;
		}
		scene->discardCard(scene->_actionCard2);
		break;
	default:
		break;
	}
}

// Handle the animations of the interceptor card
void Scene1337::Action13::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard2->_cardId;
		scene->_currentDiscardIndex--;

		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(58);
		signal();
		break;
	case 2:
		scene->discardCard(scene->_actionCard2);
		break;
	default:
		break;
	}
}

void Scene1337::postInit(SceneObjectList *OwnerList) {
// In the original, may be found in subPostInit.
// Without it, enableControl asserts
	loadScene(1330);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
//

	// Hide the user interface
	BF_GLOBALS._interfaceY = SCREEN_HEIGHT;
	R2_GLOBALS._uiElements._visible = false;

	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player._canWalk = false;
	R2_GLOBALS._player._uiEnabled = false;

	_delayedFunction = nullptr;

	_actionCard1 = nullptr;
	_actionCard2 = nullptr;
	_actionCard3 = nullptr;

	_gameBoardSide[2]._handCard[0]._stationPos = Common::Point(10, 174);
	_gameBoardSide[2]._handCard[1]._stationPos = Common::Point(37, 174);
	_gameBoardSide[2]._handCard[2]._stationPos = Common::Point(64, 174);
	_gameBoardSide[2]._handCard[3]._stationPos = Common::Point(91, 174);

	_gameBoardSide[2]._outpostStation[0]._stationPos = Common::Point(119, 174);
	_gameBoardSide[2]._outpostStation[1]._stationPos = Common::Point(119, 148);
	_gameBoardSide[2]._outpostStation[2]._stationPos = Common::Point(119, 122);
	_gameBoardSide[2]._outpostStation[3]._stationPos = Common::Point(145, 122);
	_gameBoardSide[2]._outpostStation[4]._stationPos = Common::Point(171, 122);
	_gameBoardSide[2]._outpostStation[5]._stationPos = Common::Point(171, 148);
	_gameBoardSide[2]._outpostStation[6]._stationPos = Common::Point(171, 174);
	_gameBoardSide[2]._outpostStation[7]._stationPos = Common::Point(145, 174);

	_gameBoardSide[2]._delayCard._stationPos = Common::Point(199, 174);

	_gameBoardSide[2]._emptyStationPos._stationPos = Common::Point(145, 148);

	_gameBoardSide[2]._card1Pos = Common::Point(10, 174);
	_gameBoardSide[2]._card2Pos = Common::Point(37, 174);
	_gameBoardSide[2]._card3Pos = Common::Point(64, 174);
	_gameBoardSide[2]._card4Pos = Common::Point(91, 174);
	_gameBoardSide[2]._frameNum = 2;

	_gameBoardSide[3]._handCard[0]._stationPos = Common::Point(14, 14);
	_gameBoardSide[3]._handCard[1]._stationPos = Common::Point(14, 36);
	_gameBoardSide[3]._handCard[2]._stationPos = Common::Point(14, 58);
	_gameBoardSide[3]._handCard[3]._stationPos = Common::Point(14, 80);

	_gameBoardSide[3]._outpostStation[0]._stationPos = Common::Point(37, 66);
	_gameBoardSide[3]._outpostStation[1]._stationPos = Common::Point(63, 66);
	_gameBoardSide[3]._outpostStation[2]._stationPos = Common::Point(89, 66);
	_gameBoardSide[3]._outpostStation[3]._stationPos = Common::Point(89, 92);
	_gameBoardSide[3]._outpostStation[4]._stationPos = Common::Point(89, 118);
	_gameBoardSide[3]._outpostStation[5]._stationPos = Common::Point(63, 118);
	_gameBoardSide[3]._outpostStation[6]._stationPos = Common::Point(37, 118);
	_gameBoardSide[3]._outpostStation[7]._stationPos = Common::Point(37, 92);

	_gameBoardSide[3]._delayCard._stationPos = Common::Point(37, 145);

	_gameBoardSide[3]._emptyStationPos._stationPos = Common::Point(63, 92);

	_gameBoardSide[3]._card1Pos = Common::Point(14, 14);
	_gameBoardSide[3]._card2Pos = Common::Point(14, 36);
	_gameBoardSide[3]._card3Pos = Common::Point(14, 58);
	_gameBoardSide[3]._card4Pos = Common::Point(14, 80);
	_gameBoardSide[3]._frameNum = 3;

	_gameBoardSide[0]._handCard[0]._stationPos = Common::Point(280, 5);
	_gameBoardSide[0]._handCard[1]._stationPos = Common::Point(253, 5);
	_gameBoardSide[0]._handCard[2]._stationPos = Common::Point(226, 5);
	_gameBoardSide[0]._handCard[3]._stationPos = Common::Point(199, 5);

	_gameBoardSide[0]._outpostStation[0]._stationPos = Common::Point(171, 16);
	_gameBoardSide[0]._outpostStation[1]._stationPos = Common::Point(171, 42);
	_gameBoardSide[0]._outpostStation[2]._stationPos = Common::Point(171, 68);
	_gameBoardSide[0]._outpostStation[3]._stationPos = Common::Point(145, 68);
	_gameBoardSide[0]._outpostStation[4]._stationPos = Common::Point(119, 68);
	_gameBoardSide[0]._outpostStation[5]._stationPos = Common::Point(119, 42);
	_gameBoardSide[0]._outpostStation[6]._stationPos = Common::Point(119, 16);
	_gameBoardSide[0]._outpostStation[7]._stationPos = Common::Point(145, 16);

	_gameBoardSide[0]._delayCard._stationPos = Common::Point(91, 16);

	_gameBoardSide[0]._emptyStationPos._stationPos = Common::Point(145, 42);

	_gameBoardSide[0]._card1Pos = Common::Point(280, 5);
	_gameBoardSide[0]._card2Pos = Common::Point(253, 5);
	_gameBoardSide[0]._card3Pos = Common::Point(226, 5);
	_gameBoardSide[0]._card4Pos = Common::Point(199, 5);
	_gameBoardSide[0]._frameNum = 2;

	_gameBoardSide[1]._handCard[0]._stationPos = Common::Point(283, 146);
	_gameBoardSide[1]._handCard[1]._stationPos = Common::Point(283, 124);
	_gameBoardSide[1]._handCard[2]._stationPos = Common::Point(283, 102);
	_gameBoardSide[1]._handCard[3]._stationPos = Common::Point(283, 80);

	_gameBoardSide[1]._outpostStation[0]._stationPos = Common::Point(253, 122);
	_gameBoardSide[1]._outpostStation[1]._stationPos = Common::Point(227, 122);
	_gameBoardSide[1]._outpostStation[2]._stationPos = Common::Point(201, 122);
	_gameBoardSide[1]._outpostStation[3]._stationPos = Common::Point(201, 96);
	_gameBoardSide[1]._outpostStation[4]._stationPos = Common::Point(201, 70);
	_gameBoardSide[1]._outpostStation[5]._stationPos = Common::Point(227, 70);
	_gameBoardSide[1]._outpostStation[6]._stationPos = Common::Point(253, 70);
	_gameBoardSide[1]._outpostStation[7]._stationPos = Common::Point(253, 96);

	_gameBoardSide[1]._delayCard._stationPos = Common::Point(253, 43);

	_gameBoardSide[1]._emptyStationPos._stationPos = Common::Point(227, 96);

	_gameBoardSide[1]._card1Pos = Common::Point(283, 146);
	_gameBoardSide[1]._card2Pos = Common::Point(283, 124);
	_gameBoardSide[1]._card3Pos = Common::Point(283, 102);
	_gameBoardSide[1]._card4Pos = Common::Point(283, 80);
	_gameBoardSide[1]._frameNum = 4;

	subPostInit();

	_stockPile.postInit();
}

void Scene1337::remove() {
	if (R2_GLOBALS._v57709 > 1) {
		subD1917();
		subD1940(false);
	}

	R2_GLOBALS._uiElements._active = true;
	R2_GLOBALS._uiElements._visible = true;
	SceneExt::remove();
}

void Scene1337::process(Event &event) {
	if (event.eventType == EVENT_BUTTON_DOWN) {
		if (event.btnState == BTNSHIFT_RIGHT) {
			updateCursorId(R2_GLOBALS._mouseCursorId, true);
			event.handled = true;
		} else if (_delayedFunction) {
			FunctionPtrType tmpFctPtr = _delayedFunction;
			_delayedFunction = nullptr;
			(this->*tmpFctPtr)();
			event.handled = true;
		}
	} else if (event.eventType == EVENT_KEYPRESS) {
		if (event.kbd.keycode == Common::KEYCODE_SPACE) {
			if (_delayedFunction) {
				FunctionPtrType tmpFctPtr = _delayedFunction;
				_delayedFunction = nullptr;
				(this->*tmpFctPtr)();
				event.handled = true;
			}
		} else
			warning("Fixme: Find proper keycode value");
	}

	if (!event.handled)
		Scene::process(event);
}

void Scene1337::dispatch() {
	if (!_instructionsDisplayedFl) {
		++_instructionsWaitCount;
		if (_instructionsWaitCount == 4) {
			_instructionsDisplayedFl = true;
			suggestInstructions();
		}
	}

	// The following code is in the original in sceneHandler::process(),
	// which is terrible as it's checked in every scene of the game.
	setCursorData(5, _cursorCurStrip, _cursorCurFrame);
	//

	Scene::dispatch();
}

void Scene1337::actionDisplay(int resNum, int lineNum, int x, int y, int keepOnScreen, int width, int textMode, int fontNum, int colFG, int colBGExt, int colFGExt) {
	// TODO: Check if it's normal that arg5 is unused and replaced by an hardcoded 0 value
	// May hide an original bug

	SceneItem::display(resNum, lineNum, SET_X, x, SET_Y, y, SET_KEEP_ONSCREEN, 0,
		               SET_WIDTH, width, SET_POS_MODE, -1, SET_TEXT_MODE, textMode,
					   SET_FONT, fontNum, SET_FG_COLOR, colFG, SET_EXT_BGCOLOR, colBGExt,
					   SET_EXT_FGCOLOR, colFGExt, LIST_END);
}

void Scene1337::setAnimationInfo(Card *card) {
	if (!card)
		return;

	if (card->_cardId > 25) {
		card->_card.setStrip2(4);
		card->_card.setFrame(card->_cardId - 25);
	} else if (card->_cardId > 9) {
		card->_card.setStrip2(3);
		card->_card.setFrame(card->_cardId - 9);
	} else {
		card->_card.setStrip2(2);
		card->_card.setFrame(card->_cardId);
	}

	card->_card.show();
	R2_GLOBALS._sceneObjects->draw();
}

void Scene1337::handleNextTurn() {
	switch (_winnerId) {
	case -1:
		++_currentPlayerNumb;
		if (_currentPlayerNumb > 3)
			_currentPlayerNumb = 0;

		if (_showPlayerTurn) {
			_currentPlayerArrow.show();
			switch (_currentPlayerNumb) {
			case 0:
				_currentPlayerArrow.setStrip(3);
				break;
			case 1:
				_currentPlayerArrow.setStrip(4);
				break;
			case 2:
				subD1975(174, 107);
				_currentPlayerArrow.setStrip(1);
				break;
			case 3:
				subC4CEC();
				_currentPlayerArrow.setStrip(2);
				break;
			default:
				break;
			}

			if (!_autoplay)
				_delayedFunction = &Scene1337::handlePlayerTurn;
			else
				handlePlayerTurn();
		} else {
			handlePlayerTurn();
		}
		break;
	case 0:
		_aSound2.play(62);
		actionDisplay(1330, 135, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 121, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		actionDisplay(1330, 122, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		R2_GLOBALS._sceneObjects->draw();
		actionDisplay(1330, 123, 159, 134, 1, 200, 0, 7, 0, 105, 105);
		break;
	case 1:
		_aSound2.play(62);
		actionDisplay(1330, 151, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		actionDisplay(1330, 118, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		actionDisplay(1330, 119, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		R2_GLOBALS._sceneObjects->draw();
		actionDisplay(1330, 120, 159, 134, 1, 200, 0, 7, 0, 105, 105);
		break;
	case 2:
		_aSound2.play(62);
		actionDisplay(1330, 134, 159, 134, 1, 200, 0, 7, 0, 105, 105);
		actionDisplay(1330, 124, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		actionDisplay(1330, 126, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		R2_GLOBALS._sceneObjects->draw();
		actionDisplay(1330, 125, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		break;
	case 3:
		_aSound2.play(62);
		actionDisplay(1330, 150, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		actionDisplay(1330, 115, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		actionDisplay(1330, 116, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		R2_GLOBALS._sceneObjects->draw();
		actionDisplay(1330, 117, 159, 134, 1, 200, 0, 7, 0, 105, 105);
		break;
	default:
		break;
	}

	if (_winnerId != -1)
		R2_GLOBALS._sceneManager.changeScene(125);

}

void Scene1337::handlePlayerTurn() {
	if (_showPlayerTurn)
		_currentPlayerArrow.hide();

	switch (_currentPlayerNumb) {
	case 2:
		subC4CD2();
		if (_displayHelpFl)
			actionDisplay(1330, 114, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		_displayHelpFl = false;
		// fall through
	case 0:
		// fall through
	case 1:
		// fall through
	case 3:
		_actionItem.setAction(&_action4);
	default:
		break;
	}

	_showPlayerTurn = true;

}

bool Scene1337::isStationCard(int cardId) {
	switch (cardId) {
	case 10:
	// No break on purpose
	case 12:
	// No break on purpose
	case 15:
	// No break on purpose
	case 17:
	// No break on purpose
	case 18:
	// No break on purpose
	case 19:
	// No break on purpose
	case 20:
	// No break on purpose
	case 21:
		return true;
	default:
		return false;
	}
}

bool Scene1337::isStopConstructionCard(int cardId) {
	switch (cardId) {
	case 11:
	// No break on purpose
	case 14:
	// No break on purpose
	case 16:
	// No break on purpose
	case 24:
		return true;
	default:
		return false;
	}
}

int Scene1337::getStationId(int playerId, int handCardId) {
	if ((_gameBoardSide[playerId]._handCard[handCardId]._cardId > 1) && (_gameBoardSide[playerId]._handCard[handCardId]._cardId <= 9))
		return handCardId;

	return -1;
}

int Scene1337::findPlatformCardInHand(int playerId) {
	for (int i = 0; i <= 3; i++) {
		if (_gameBoardSide[playerId]._handCard[i]._cardId == 1)
			return i;
	}

	return -1;
}

int Scene1337::findMeteorCardInHand(int playerId) {
	for (int i = 0; i <= 3; i++) {
		if (_gameBoardSide[playerId]._handCard[i]._cardId == 13)
			return i;
	}

	return -1;
}

int Scene1337::findThieftCardInHand(int playerId) {
	for (int i = 0; i <= 3; i++) {
		if (_gameBoardSide[playerId]._handCard[i]._cardId == 25)
			return i;
	}

	return -1;
}

int Scene1337::isDelayCard(int cardId) {
	switch (cardId) {
	case 11:
	// No break on purpose
	case 14:
	// No break on purpose
	case 16:
	// No break on purpose
	case 24:
		return cardId;
		break;
	default:
		return -1;
		break;
	}
}

int Scene1337::getStationCardId(int cardId) {
	switch (cardId) {
	case 10:
	// No break on purpose
	case 12:
	// No break on purpose
	case 15:
	// No break on purpose
	case 17:
	// No break on purpose
	case 18:
	// No break on purpose
	case 19:
	// No break on purpose
	case 20:
	// No break on purpose
	case 21:
		return cardId;
	default:
		return -1;
	}
}

void Scene1337::handlePlayer01Discard(int playerId) {
	switch (playerId) {
	case 0:
		for (int i = 0; i <= 3; i++) {
			if (getStationCardId(_gameBoardSide[playerId]._handCard[i]._cardId) != -1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (isDelayCard(_gameBoardSide[playerId]._handCard[i]._cardId) != -1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			// Outpost Card
			if ((_gameBoardSide[playerId]._handCard[i]._cardId > 1) && (_gameBoardSide[playerId]._handCard[i]._cardId <= 9)) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if ((_gameBoardSide[playerId]._handCard[i]._cardId >= 26) && (_gameBoardSide[playerId]._handCard[i]._cardId <= 33)) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			// Station Card
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			// Thief card
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 25) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			// Meteor Card
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 13) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}
		break;
	case 1:
		for (int i = 0; i <= 3; i++) {
			if ((_gameBoardSide[playerId]._handCard[i]._cardId >= 26) && (_gameBoardSide[playerId]._handCard[i]._cardId <= 33)) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			// Station Card
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			// Outpost Card
			if ((_gameBoardSide[playerId]._handCard[i]._cardId > 1) && (_gameBoardSide[playerId]._handCard[i]._cardId <= 9)) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (getStationCardId(_gameBoardSide[playerId]._handCard[i]._cardId) != -1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (isDelayCard(_gameBoardSide[playerId]._handCard[i]._cardId) != -1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			// Thief card
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 25) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			// Meteor Card
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 13) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		break;
	default:
		break;
	}
}

void Scene1337::playThieftCard(int playerId, Card *card, int victimId) {
	_actionPlayerIdx = playerId;
	_actionVictimIdx = victimId;

	int randIndx;

	for (;;) {
		randIndx = R2_GLOBALS._randomSource.getRandomNumber(3);
		if (_gameBoardSide[victimId]._handCard[randIndx]._cardId != 0)
			break;
	}

	_actionCard1 = card;
	_actionCard2 = &_gameBoardSide[victimId]._emptyStationPos;
	_actionCard3 = &_gameBoardSide[victimId]._handCard[randIndx];

	_actionItem.setAction(&_action11);
}

int Scene1337::getPreventionCardId(int cardId) {
	int retVal;

	switch (cardId) {
	case 10:
		retVal = 2;
		break;
	case 12:
		retVal = 3;
		break;
	case 15:
		retVal = 5;
		break;
	case 17:
		retVal = 9;
		break;
	case 18:
		retVal = 6;
		break;
	case 19:
		retVal = 4;
		break;
	case 20:
		retVal = 8;
		break;
	case 21:
		retVal = 7;
		break;
	default:
		retVal = -1;
	}

	return retVal;
}

bool Scene1337::isAttackPossible(int victimId, int cardId) {
	if (victimId < 0 || victimId >= ARRAYSIZE(_gameBoardSide))
		error("Scene1337::isAttackPossible() victimId:%d out of range 0 to %d", victimId, ARRAYSIZE(_gameBoardSide)-1);

	for (int i = 0; i <= 7; i++) {
		if (_gameBoardSide[victimId]._outpostStation[i]._cardId != 0) {
			if (getPreventionCardId(cardId) == _gameBoardSide[victimId]._outpostStation[i]._cardId)
				return false;
		}
	}
	return true;
}

int Scene1337::getPlayerWithOutpost(int playerId) {
	int randPlayerId = R2_GLOBALS._randomSource.getRandomNumber(3);

	for (int i = 0; i <= 3; i++) {
		if (randPlayerId != playerId) {
			for (int j = 0; j <= 7; j++) {
				if (_gameBoardSide[randPlayerId]._outpostStation[j]._cardId != 0)
					return randPlayerId;
			}
		}

		if (playerId == 1) {
			randPlayerId--;
			if (randPlayerId < 0)
				randPlayerId = 3;
		} else {
			++randPlayerId;
			if (randPlayerId > 3)
				randPlayerId = 0;
		}
	}

	return -1;
}

bool Scene1337::checkAntiDelayCard(int delayCardId, int cardId) {
	if ((delayCardId == 11) && (cardId == 26)) // Diplomacy
		return true;

	if ((delayCardId == 14) && (cardId == 30)) // Cure
		return true;

	if ((delayCardId == 16) && (cardId == 32)) // Agreement
		return true;

	if ((delayCardId == 24) && (cardId == 28)) // Innovation
		return true;

	return false;
}

void Scene1337::playStationCard(Card *station, Card *platform) {
	_actionCard1 = station;
	_actionCard2 = platform;
	_actionItem.setAction(&_action7);
}

int Scene1337::getRandomCardFromHand(int playerId) {
	if ( (_gameBoardSide[playerId]._handCard[0]._cardId == 0)
	  && (_gameBoardSide[playerId]._handCard[1]._cardId == 0)
	  && (_gameBoardSide[playerId]._handCard[2]._cardId == 0)
	  && (_gameBoardSide[playerId]._handCard[3]._cardId == 0))
	  return -1;

	int randIndx;
	for (;;) {
		randIndx = R2_GLOBALS._randomSource.getRandomNumber(3);
		if (_gameBoardSide[playerId]._handCard[randIndx]._cardId != 0)
			break;
	}

	return randIndx;
}

void Scene1337::playPlatformCard(Card *card, Card *dest) {
	_actionCard1 = card;
	_actionCard2 = dest;

	_actionItem.setAction(&_action6);
}

void Scene1337::playDelayCard(Card *card, Card *dest) {
	_actionCard1 = card;
	_actionCard2 = dest;

	_actionItem.setAction(&_action9);
}

void Scene1337::playAntiDelayCard(Card *card, Card *dest) {
	_actionCard1 = card;
	_actionCard2 = dest;

	_actionItem.setAction(&_action8);

	// WORKAROUND: Restore the default cursor and for a call to signal.
	// This works around the cursor caching we got rid of, and avoid
	// the game ends in an eternal loop when a player reacts to another player
	// attack.
	setCursorData(5, 1, 4);
	signal();
}


Scene1337::Card *Scene1337::getStationCard(int playerId) {
	for (int i = 0; i <= 7; i++) {
		if ((_gameBoardSide[playerId]._outpostStation[i]._cardId >= 1) && (_gameBoardSide[playerId]._outpostStation[i]._cardId <= 9))
			return &_gameBoardSide[playerId]._outpostStation[i];
	}

	return nullptr;
}

void Scene1337::playCentralOutpostCard(Card *card, int playerId) {
	_actionCard1 = card;
	_actionCard2 = getStationCard(playerId);
	_actionCard3 = &_gameBoardSide[playerId]._emptyStationPos;
	_actionPlayerIdx = playerId;
	_actionItem.setAction(&_action10);
}

void Scene1337::discardCard(Card *card) {
	_actionCard1 = card;

	_actionItem.setAction(&_action5);
}

void Scene1337::subC4CD2() {
	if (R2_GLOBALS._v57709 > 0) {
		subD1917();
		subD1940(false); // _v5780C--
	}
}

void Scene1337::subC4CEC() {
	if (R2_GLOBALS._v57709 == 0) {
		subD18F5();
		subD1940(true); // _v5780C++
	}
}

// Play Interceptor card
void Scene1337::playInterceptorCard(Card *subObj1, Card *subObj2) {
	_actionCard1 = subObj1;
	_actionCard2 = subObj2;

	_actionItem.setAction(&_action13);
}

void Scene1337::displayDialog(int dialogNumb) {
	switch (dialogNumb - 1) {
	case 0:
		actionDisplay(1330, 53, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 1:
		actionDisplay(1330, 57, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 2:
		actionDisplay(1330, 58, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 3:
		actionDisplay(1330, 59, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 4:
		actionDisplay(1330, 60, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 5:
		actionDisplay(1330, 61, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 6:
		actionDisplay(1330, 62, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 7:
		actionDisplay(1330, 63, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 8:
		actionDisplay(1330, 64, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 9:
		actionDisplay(1330, 65, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 10:
		actionDisplay(1330, 67, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 11:
		actionDisplay(1330, 69, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 12:
		actionDisplay(1330, 71, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 72, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 73, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 13:
		actionDisplay(1330, 79, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 14:
		actionDisplay(1330, 81, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 15:
		actionDisplay(1330, 83, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 16:
		actionDisplay(1330, 85, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 17:
		actionDisplay(1330, 87, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 18:
		actionDisplay(1330, 89, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 19:
		actionDisplay(1330, 91, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 20:
		actionDisplay(1330, 93, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 23:
		actionDisplay(1330, 95, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 24:
		actionDisplay(1330, 97, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 25:
		actionDisplay(1330, 104, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 26:
		actionDisplay(1330, 105, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 106, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 27:
		actionDisplay(1330, 110, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 28:
		actionDisplay(1330, 108, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 109, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 29:
		actionDisplay(1330, 111, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 31:
		actionDisplay(1330, 112, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	default:
		break;
	}
}

void Scene1337::subPostInit() {
	R2_GLOBALS._v57709 = 0;
	R2_GLOBALS._v5780C = 0;
	updateCursorId(1, false);
	subD1940(true); // _v5780C++
	subD18F5();

//	loadScene(1330);
//	SceneExt::postInit();

	R2_GLOBALS._scenePalette.addRotation(224, 235, 1);

	_availableCardsPile[0] = 1;
	_availableCardsPile[1] = 1;
	_availableCardsPile[2] = 1;
	_availableCardsPile[3] = 1;
	_availableCardsPile[4] = 1;
	_availableCardsPile[5] = 1;
	_availableCardsPile[6] = 1;
	_availableCardsPile[7] = 1;
	_availableCardsPile[8] = 26;
	_availableCardsPile[9] = 2;
	_availableCardsPile[10] = 2;
	_availableCardsPile[11] = 2;
	_availableCardsPile[12] = 2;
	_availableCardsPile[13] = 2;
	_availableCardsPile[14] = 26;
	_availableCardsPile[15] = 3;
	_availableCardsPile[16] = 3;
	_availableCardsPile[17] = 3;
	_availableCardsPile[18] = 3;
	_availableCardsPile[19] = 3;
	_availableCardsPile[20] = 28;
	_availableCardsPile[21] = 4;
	_availableCardsPile[22] = 4;
	_availableCardsPile[23] = 4;
	_availableCardsPile[24] = 4;
	_availableCardsPile[25] = 4;
	_availableCardsPile[26] = 28;
	_availableCardsPile[27] = 5;
	_availableCardsPile[28] = 5;
	_availableCardsPile[29] = 5;
	_availableCardsPile[30] = 5;
	_availableCardsPile[31] = 5;
	_availableCardsPile[32] = 30;
	_availableCardsPile[33] = 6;
	_availableCardsPile[34] = 6;
	_availableCardsPile[35] = 6;
	_availableCardsPile[36] = 6;
	_availableCardsPile[37] = 6;
	_availableCardsPile[38] = 30;
	_availableCardsPile[39] = 7;
	_availableCardsPile[40] = 7;
	_availableCardsPile[41] = 7;
	_availableCardsPile[42] = 7;
	_availableCardsPile[43] = 7;
	_availableCardsPile[44] = 32;
	_availableCardsPile[45] = 8;
	_availableCardsPile[46] = 8;
	_availableCardsPile[47] = 8;
	_availableCardsPile[48] = 8;
	_availableCardsPile[49] = 8;
	_availableCardsPile[50] = 32;
	_availableCardsPile[51] = 9;
	_availableCardsPile[52] = 9;
	_availableCardsPile[53] = 9;
	_availableCardsPile[54] = 9;
	_availableCardsPile[55] = 9;
	_availableCardsPile[56] = 10;
	_availableCardsPile[57] = 11;
	_availableCardsPile[58] = 12;
	_availableCardsPile[59] = 13;
	_availableCardsPile[60] = 13;
	_availableCardsPile[61] = 14;
	_availableCardsPile[62] = 15;
	_availableCardsPile[63] = 16;
	_availableCardsPile[64] = 17;
	_availableCardsPile[65] = 18;
	_availableCardsPile[66] = 19;
	_availableCardsPile[67] = 20;
	_availableCardsPile[68] = 21;
	_availableCardsPile[69] = 26;
	_availableCardsPile[70] = 28;
	_availableCardsPile[71] = 24;
	_availableCardsPile[72] = 25;
	_availableCardsPile[73] = 25;
	_availableCardsPile[74] = 25;
	_availableCardsPile[75] = 25;
	_availableCardsPile[76] = 26;
	_availableCardsPile[77] = 26;
	_availableCardsPile[78] = 26;
	_availableCardsPile[79] = 27;
	_availableCardsPile[80] = 27;
	_availableCardsPile[81] = 28;
	_availableCardsPile[82] = 28;
	_availableCardsPile[83] = 28;
	_availableCardsPile[84] = 29;
	_availableCardsPile[85] = 29;
	_availableCardsPile[86] = 29;
	_availableCardsPile[87] = 30;
	_availableCardsPile[88] = 30;
	_availableCardsPile[89] = 30;
	_availableCardsPile[90] = 30;
	_availableCardsPile[91] = 32;
	_availableCardsPile[92] = 1;
	_availableCardsPile[93] = 32;
	_availableCardsPile[94] = 32;
	_availableCardsPile[95] = 32;
	_availableCardsPile[96] = 1;
	_availableCardsPile[97] = 1;
	_availableCardsPile[98] = 1;
	_availableCardsPile[99] = 0;

	_cardsAvailableNumb = 98;
	_currentDiscardIndex = 98; // CHECKME: Would make more sense at pos 99

	_discardPile._cardId = 0;
	_discardPile._stationPos = Common::Point(128, 95);

	_stockCard._cardId = 0;
	_stockCard._stationPos = Common::Point(162, 95);

	_selectedCard._cardId = 0;

	_animatedCard._card.postInit();
	_animatedCard._card.setVisage(1332);
	_animatedCard._card.setStrip(5);
	_animatedCard._card.setFrame(1);
	_animatedCard._card._moveDiff = Common::Point(10, 10);
	_animatedCard._card.fixPriority(400);
	_animatedCard._card.setPosition(Common::Point(128, 95), 0);
	_animatedCard._card.animate(ANIM_MODE_2, NULL);
	_animatedCard._card.hide();

	_currentPlayerArrow.postInit();
	_currentPlayerArrow.setVisage(1334);
	_currentPlayerArrow.setStrip(1);
	_currentPlayerArrow.setFrame(1);
	_currentPlayerArrow._numFrames = 12;
	_currentPlayerArrow.fixPriority(500);
	_currentPlayerArrow.setPosition(Common::Point(174, 107), 0);
	_currentPlayerArrow.animate(ANIM_MODE_2, NULL);
	_currentPlayerArrow.hide();

	_showPlayerTurn = true;
	_displayHelpFl = false;
	_winnerId = -1;

	_helpIcon.postInit();
	_helpIcon.setup(9531, 1, 1);
	_helpIcon.setPosition(Common::Point(249, 168));
	_helpIcon.setPriority(155);
	_helpIcon._effect = EFFECT_NONE;
	_helpIcon.show();

	_autoplay = false;
	_instructionsDisplayedFl = false;
	_instructionsWaitCount = 0;
}

void Scene1337::suggestInstructions() {
	if (R2_GLOBALS._v57709 > 0)
		subD1917();

	if (MessageDialog::show(NEED_INSTRUCTIONS, NO_MSG, YES_MSG) == 0) {
		if (R2_GLOBALS._v57709 == 0)
			subD18F5();
		dealCards();
	} else {
		if (R2_GLOBALS._v57709 == 0)
			subD18F5();
		displayInstructions();
	}
}

void Scene1337::displayInstructions() {
	_actionItem.setAction(&_action1);
}

void Scene1337::shuffleCards() {
	R2_GLOBALS._sceneObjects->draw();

	// Remove holes in card pile
	for (int i = 0; i <= 98; i++) {
		if (_availableCardsPile[i] == 0) {
			for (int j = i + 1; j <= 98; j ++) {
				if (_availableCardsPile[j] != 0) {
					_availableCardsPile[i] = _availableCardsPile[j];
					_availableCardsPile[j] = 0;
					break;
				}
			}
		}
	}

	// Compute the number of available cards
	for (int i = 0; i <= 99; i ++) {
		if (_availableCardsPile[i] == 0) {
			// CHECKME: This will fail if i == 0, which shouldn't happen
			// as we don't shuffle cards when no card is available.
			_cardsAvailableNumb = i - 1;
			_currentDiscardIndex = 98;  // CHECKME: Would make more sense at pos 99
			break;
		}
	}

	for (int i = 0; i < 2000; i ++) {
		int randIndx = R2_GLOBALS._randomSource.getRandomNumber(_cardsAvailableNumb);
		int swap = _availableCardsPile[0];
		_availableCardsPile[0] = _availableCardsPile[randIndx];
		_availableCardsPile[randIndx] = swap;
	}

	_shuffleEndedFl = false;

	// Shuffle cards
	_animatedCard._card.setAction(&_action2);

	while(!_shuffleEndedFl && !g_vm->shouldQuit()) {
		g_globals->_sceneObjects->recurse(SceneHandler::dispatchObject);
		g_globals->_scenePalette.signalListeners();
		R2_GLOBALS._sceneObjects->draw();
		g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
	}
}

void Scene1337::dealCards() {
	_animatedCard._card._moveDiff = Common::Point(30, 30);
	shuffleCards();

	// Deal cards
	_actionItem.setAction(&_action3);
}

void Scene1337::showOptionsDialog() {
	// Display menu with "Auto Play", "New Game", "Quit" and "Continue"
	OptionsDialog::show();
}

void Scene1337::handleClick(int arg1, Common::Point pt) {
	int curReg = R2_GLOBALS._sceneRegions.indexOf(g_globals->_events._mousePos);

	if (arg1 == 3) {
		bool found = false;
		int i;
		for (i = 0; i <= 7; i++) {
			if ( _gameBoardSide[2]._outpostStation[i].isIn(pt)
			  || _gameBoardSide[0]._outpostStation[i].isIn(pt)
			  || _gameBoardSide[1]._outpostStation[i].isIn(pt)
			  || _gameBoardSide[3]._outpostStation[i].isIn(pt) ) {
				found = true;
				break;
			}
		}

		if (found) {
			switch (curReg) {
			case 5:
				if (_gameBoardSide[2]._outpostStation[i]._cardId != 0)
					displayDialog(_gameBoardSide[2]._outpostStation[i]._cardId);
				else
					actionDisplay(1330, 20, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 10:
				if (_gameBoardSide[3]._outpostStation[i]._cardId != 0)
					displayDialog(_gameBoardSide[3]._outpostStation[i]._cardId);
				else
					actionDisplay(1330, 22, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 15:
				if (_gameBoardSide[0]._outpostStation[i]._cardId != 0)
					displayDialog(_gameBoardSide[0]._outpostStation[i]._cardId);
				else
					actionDisplay(1330, 21, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 20:
				if (_gameBoardSide[1]._outpostStation[i]._cardId != 0)
					displayDialog(_gameBoardSide[1]._outpostStation[i]._cardId);
				else
					actionDisplay(1330, 23, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				break;
			}
		} else if ( _gameBoardSide[2]._delayCard.isIn(pt)
			  || _gameBoardSide[0]._delayCard.isIn(pt)
			  || _gameBoardSide[1]._delayCard.isIn(pt)
			  || _gameBoardSide[3]._delayCard.isIn(pt) ) {
			switch (curReg) {
			case 5:
				if (_gameBoardSide[2]._delayCard._cardId != 0)
					displayDialog(_gameBoardSide[2]._delayCard._cardId);
				else
					actionDisplay(1330, 10, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 10:
				if (_gameBoardSide[3]._delayCard._cardId != 0)
					displayDialog(_gameBoardSide[3]._delayCard._cardId);
				else
					actionDisplay(1330, 16, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 15:
				if (_gameBoardSide[0]._delayCard._cardId != 0)
					displayDialog(_gameBoardSide[0]._delayCard._cardId);
				else
					actionDisplay(1330, 13, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 20:
				if (_gameBoardSide[1]._delayCard._cardId != 0)
					displayDialog(_gameBoardSide[1]._delayCard._cardId);
				else
					actionDisplay(1330, 18, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				break;
			}
		} else if (_discardPile.isIn(pt)) {
			if (_discardPile._cardId != 0)
				displayDialog(_discardPile._cardId);
			else
				actionDisplay(1330, 7, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		} else if (_helpIcon._bounds.contains(pt))
			actionDisplay(1330, 43, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if (_stockCard.isIn(pt))
			actionDisplay(1330, 4, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if ( (_gameBoardSide[2]._emptyStationPos.isIn(pt))
			     || (_gameBoardSide[3]._emptyStationPos.isIn(pt))
			     || (_gameBoardSide[0]._emptyStationPos.isIn(pt))
			     || (_gameBoardSide[1]._emptyStationPos.isIn(pt)) )
			actionDisplay(1330, 32, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if (_gameBoardSide[2]._handCard[0].isIn(pt))
			displayDialog(_gameBoardSide[2]._handCard[0]._cardId);
		else if (_gameBoardSide[2]._handCard[1].isIn(pt))
			displayDialog(_gameBoardSide[2]._handCard[1]._cardId);
		else if (_gameBoardSide[2]._handCard[2].isIn(pt))
			displayDialog(_gameBoardSide[2]._handCard[2]._cardId);
		else if (_gameBoardSide[2]._handCard[3].isIn(pt))
			displayDialog(_gameBoardSide[2]._handCard[3]._cardId);
		else if ((curReg >= 6) && (curReg <= 9))
			actionDisplay(1330, 29, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if ((curReg >= 11) && (curReg <= 14))
			actionDisplay(1330, 31, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if ((curReg >= 16) && (curReg <= 19))
			actionDisplay(1330, 30, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else {
			switch (curReg) {
			case 0:
				actionDisplay(1330, 2, 159, 134, 1, 200, 0, 7, 0, 105, 105);
				break;
			case 5:
				actionDisplay(1330, 25, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 10:
				actionDisplay(1330, 27, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 15:
				actionDisplay(1330, 26, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 20:
				actionDisplay(1330, 28, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 21:
				actionDisplay(1330, 24, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				break;
			}
		}
	}

	if (arg1 != 1)
		return;

	for (int i = 0; i <= 7; i++) {
		if (_gameBoardSide[2]._outpostStation[i].isIn(pt)) {
			switch (_gameBoardSide[2]._outpostStation[i]._cardId) {
			case 0:
				actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 1:
				actionDisplay(1330, 54, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				actionDisplay(1330, 34, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			}
			return;
		}
		if (_gameBoardSide[0]._outpostStation[i].isIn(pt)) {
			switch (_gameBoardSide[0]._outpostStation[i]._cardId) {
			case 0:
				actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				actionDisplay(1330, 1, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			}
			return;
		}
		if (_gameBoardSide[1]._outpostStation[i].isIn(pt)) {
			switch (_gameBoardSide[1]._outpostStation[i]._cardId) {
			case 0:
				actionDisplay(1330, 146, 300, 99, 1, 136, 0, 7, 0, 117, 117);
				break;
			default:
				actionDisplay(1330, 144, 300, 99, 1, 136, 0, 7, 0, 117, 117);
				break;
			}
			return;
		}
		if (_gameBoardSide[3]._outpostStation[i].isIn(pt)) {
			switch (_gameBoardSide[3]._outpostStation[i]._cardId) {
			case 0:
				actionDisplay(1330, 147, 20, 99, 1, 136, 0, 7, 0, 172, 172);
				break;
			default:
				actionDisplay(1330, 145, 20, 99, 1, 136, 0, 7, 0, 172, 172);
				break;
			}
			return;
		}
	}

	if (_gameBoardSide[2]._delayCard.isIn(pt)) {
		// The original uses _gameBoardSide[0], which is obviously a bug.
		if (_gameBoardSide[2]._delayCard._cardId != 0)
			actionDisplay(1330, 39, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else
			actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		return;
	}
	if (_gameBoardSide[3]._delayCard.isIn(pt)) {
		if (_gameBoardSide[3]._delayCard._cardId != 0)
			actionDisplay(1330, 145, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		else
			actionDisplay(1330, 147, 20, 99, 1, 136, 0, 7, 0, 172, 172);

		return;
	}
	if (_gameBoardSide[1]._delayCard.isIn(pt)) {
		if (_gameBoardSide[1]._delayCard._cardId != 0)
			actionDisplay(1330, 144, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		else
			actionDisplay(1330, 146, 300, 99, 1, 136, 0, 7, 0, 117, 117);

		return;
	}
	if (_gameBoardSide[0]._delayCard.isIn(pt)) {
		if (_gameBoardSide[0]._delayCard._cardId != 0)
			actionDisplay(1330, 1, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else
			actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		return;
	}
	if (_gameBoardSide[3]._emptyStationPos.isIn(pt)) {
		actionDisplay(1330, 147, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		return;
	}
	if (_gameBoardSide[1]._emptyStationPos.isIn(pt)) {
		actionDisplay(1330, 146, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		return;
	}
	if (_gameBoardSide[0]._emptyStationPos.isIn(pt)) {
		actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		return;
	}

	if (_helpIcon._bounds.contains(pt)) {
		showOptionsDialog();
		return;
	}

	if (_discardPile.isIn(pt))
		actionDisplay(1330, 9, 159, 10, 1, 200, 0, 7, 0, 154, 154);
	else if (_stockCard.isIn(pt))
		actionDisplay(1330, 5, 159, 10, 1, 200, 0, 7, 0, 154, 154);
	else {
		switch (curReg) {
		case 0:
			actionDisplay(1330, 3, 159, 10, 1, 200, 0, 7, 0, 154, 154);
			break;
		case 6:
		// no break on purpose
		case 7:
		// no break on purpose
		case 8:
		// no break on purpose
		case 9:
			actionDisplay(1330, 145, 20, 99, 1, 136, 0, 7, 0, 172, 172);
			break;
		case 10:
			actionDisplay(1330, 147, 20, 99, 1, 136, 0, 7, 0, 172, 172);
			break;
		case 11:
		// no break on purpose
		case 12:
		// no break on purpose
		case 13:
		// no break on purpose
		case 14:
			actionDisplay(1330, 1, 159, 10, 1, 200, 0, 7, 0, 154, 154);
			break;
		case 16:
		// no break on purpose
		case 17:
		// no break on purpose
		case 18:
		// no break on purpose
		case 19:
			actionDisplay(1330, 144, 300, 99, 1, 136, 0, 7, 0, 117, 117);
			break;
		case 20:
			actionDisplay(1330, 146, 300, 99, 1, 136, 0, 7, 0, 117, 117);
			break;
		default:
			actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
			break;
		}
	}
}

void Scene1337::handlePlayer0() {
	if (_gameBoardSide[0]._delayCard._cardId != 0) {
		switch (_gameBoardSide[0]._delayCard._cardId) {
		case 10:
		//No break on purpose
		case 12:
		//No break on purpose
		case 15:
		//No break on purpose
		case 17:
		//No break on purpose
		case 18:
		//No break on purpose
		case 19:
		//No break on purpose
		case 20:
		//No break on purpose
		case 21:
			discardCard(&_gameBoardSide[0]._delayCard);
			return;
		default:
			for (int i = 0; i <= 3; i++) {
				if (checkAntiDelayCard(_gameBoardSide[0]._delayCard._cardId, _gameBoardSide[0]._handCard[i]._cardId)) {
					playAntiDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[0]._delayCard);
					return;
				}
			}

			break;
		}
	}

	for (int i = 0; i <= 3; i++) {
		int tmpVal = getStationId(0, i);

		if (tmpVal != -1) {
			bool stationAlreadyPresentFl = false;
			for (int j = 0; j <= 7; j++) {
				if (_gameBoardSide[0]._outpostStation[j]._cardId == _gameBoardSide[0]._handCard[tmpVal]._cardId) {
					stationAlreadyPresentFl = true;
					break;
				}
			}

			if (!stationAlreadyPresentFl) {
				for (int j = 0; j <= 7; j++) {
					if ((_gameBoardSide[0]._outpostStation[j]._cardId == 1) && !isStopConstructionCard(_gameBoardSide[0]._delayCard._cardId)) {
						int stationCount = 0;
						for (int k = 0; k <= 7; k++) {
							if ((_gameBoardSide[0]._outpostStation[k]._cardId > 1) && (_gameBoardSide[0]._outpostStation[k]._cardId <= 9)) {
								++stationCount;
							}
						}

						if (stationCount == 7)
							_winnerId = 0;

						playStationCard(&_gameBoardSide[0]._handCard[tmpVal], &_gameBoardSide[0]._outpostStation[j]);
						return;
					}
				}
			}
		}
	}

	int tmpVal = findPlatformCardInHand(0);

	if (tmpVal != -1) {
		for (int i = 0; i <= 7; i++) {
			if ((_gameBoardSide[0]._outpostStation[i]._cardId == 0) && !isStopConstructionCard(_gameBoardSide[0]._delayCard._cardId)) {
				playPlatformCard(&_gameBoardSide[0]._handCard[tmpVal], &_gameBoardSide[0]._outpostStation[i]);
				return;
			}
		}
	}

	int meteorCardId = findMeteorCardInHand(0);
	if (meteorCardId != -1) {
		for (int i = 0; i <= 7; i++) {
			if (_gameBoardSide[2]._outpostStation[i]._cardId != 0) {
				playCentralOutpostCard(&_gameBoardSide[0]._handCard[meteorCardId], 2);
				return;
			}
		}
	}

	int thieftId = findThieftCardInHand(0);
	if (thieftId != -1) {
		if ( (_gameBoardSide[2]._handCard[0]._cardId != 0)
		  || (_gameBoardSide[2]._handCard[1]._cardId != 0)
		  || (_gameBoardSide[2]._handCard[2]._cardId != 0)
		  || (_gameBoardSide[2]._handCard[3]._cardId != 0) ) {
			playThieftCard(0, &_gameBoardSide[0]._handCard[thieftId], 2);
			return;
		}
	}

	for (int i = 0; i <= 3; i++) {
		if ((isDelayCard(_gameBoardSide[0]._handCard[i]._cardId) != -1)
		&&  (_gameBoardSide[2]._delayCard._cardId == 0)
		&&  isAttackPossible(2, _gameBoardSide[0]._handCard[i]._cardId)) {
			playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[2]._delayCard);
			return;
		}
	}

	for (int i = 0; i <= 3; i++) {
		if ((getStationCardId(_gameBoardSide[0]._handCard[i]._cardId) != -1)
		&&  (_gameBoardSide[2]._delayCard._cardId == 0)
		&& isAttackPossible(2, _gameBoardSide[0]._handCard[i]._cardId)) {
			playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[2]._delayCard);
			return;
		}
	}

	meteorCardId = findMeteorCardInHand(0);
	int victimId = getPlayerWithOutpost(0);

	if ((meteorCardId != -1) && (victimId != -1)) {
		playCentralOutpostCard(&_gameBoardSide[0]._handCard[meteorCardId], victimId);
		return;
	}

	thieftId = findThieftCardInHand(0);
	if (thieftId != -1) {
		if ( (_gameBoardSide[1]._handCard[0]._cardId != 0)
		  || (_gameBoardSide[1]._handCard[1]._cardId != 0)
		  || (_gameBoardSide[1]._handCard[2]._cardId != 0)
		  || (_gameBoardSide[1]._handCard[3]._cardId != 0) ) {
			playThieftCard(0, &_gameBoardSide[0]._handCard[thieftId], 1);
			return;
		}
	}

	for (int i = 0; i <= 3; i++) {
		if (getStationCardId(_gameBoardSide[0]._handCard[i]._cardId) != -1) {
			if ((_gameBoardSide[1]._delayCard._cardId == 0) && isAttackPossible(1, _gameBoardSide[0]._handCard[i]._cardId)) {
				playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[1]._delayCard);
				return;
			}

			if ((_gameBoardSide[3]._delayCard._cardId == 0) && isAttackPossible(3, _gameBoardSide[0]._handCard[i]._cardId)) {
				playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[3]._delayCard);
				return;
			}
		}
	}

	for (int i = 0; i <= 3; i++) {
		tmpVal = isDelayCard(_gameBoardSide[0]._handCard[i]._cardId);
		if (tmpVal != -1) {
			if ((_gameBoardSide[1]._delayCard._cardId == 0) && isAttackPossible(1, _gameBoardSide[0]._handCard[i]._cardId)) {
				playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[1]._delayCard);
				return;
			}

			if ((_gameBoardSide[3]._delayCard._cardId == 0) && isAttackPossible(3, _gameBoardSide[0]._handCard[i]._cardId)) {
				playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[3]._delayCard);
				return;
			}
		}
	}

	handlePlayer01Discard(0);
}

void Scene1337::handlePlayer1() {
	if (this->_gameBoardSide[1]._delayCard._cardId != 0) {
		switch (_gameBoardSide[1]._delayCard._cardId) {
		case 10:
			// No break on purpose
		case 12:
			// No break on purpose
		case 15:
			// No break on purpose
		case 17:
			// No break on purpose
		case 18:
			// No break on purpose
		case 19:
			// No break on purpose
		case 20:
			// No break on purpose
		case 21:
			discardCard(&_gameBoardSide[1]._delayCard);
			return;
		default:
			for (int i = 0; i <= 3; i++) {
				if (checkAntiDelayCard(_gameBoardSide[1]._delayCard._cardId, _gameBoardSide[1]._handCard[i]._cardId)) {
					playAntiDelayCard(&_gameBoardSide[1]._handCard[i], &_gameBoardSide[1]._delayCard);
					return;
				}
			}
			break;
		}
	}

	for (int i = 0; i <= 3; i++) {
		int tmpIndx = getStationId(1, i);
		if (tmpIndx == -1)
			break;

		int tmpVal = 0;
		for (int j = 0; j <= 7; j++) {
			if (_gameBoardSide[1]._outpostStation[j]._cardId == _gameBoardSide[1]._handCard[tmpIndx]._cardId) {
				tmpVal = 1;
				break;
			}
		}

		if (tmpVal == 0)
			break;

		for (int j = 0; j <= 7; j++) {
			if ((_gameBoardSide[1]._outpostStation[j]._cardId == 1) && !isStopConstructionCard(_gameBoardSide[1]._delayCard._cardId)) {
				int stationCount = 0;
				for (int k = 0; k <= 7; k++) {
					if ((_gameBoardSide[1]._outpostStation[k]._cardId > 1) && (_gameBoardSide[1]._outpostStation[k]._cardId <= 9))
						++stationCount;
				}

				if (stationCount == 7)
					_winnerId = 1;

				playStationCard(&_gameBoardSide[1]._handCard[tmpIndx], &_gameBoardSide[1]._outpostStation[j]);
				return;
			}
		}
	}

	int normalCardId = findPlatformCardInHand(1);
	if (normalCardId != -1) {
		for (int i = 0; i <= 7; i++) {
			if ((_gameBoardSide[1]._outpostStation[i]._cardId == 0) && !isStopConstructionCard(_gameBoardSide[1]._delayCard._cardId)) {
				playPlatformCard(&_gameBoardSide[1]._handCard[normalCardId], &_gameBoardSide[1]._outpostStation[i]);
				return;
			}
		}
	}

	int meterorCardId = findMeteorCardInHand(1);
	int victimId = getPlayerWithOutpost(1);

	if ((meterorCardId != -1) && (victimId != -1)) {
		playCentralOutpostCard(&_gameBoardSide[1]._handCard[meterorCardId], victimId);
		return;
	}

	int thieftId = findThieftCardInHand(1);
	if (thieftId != -1) {
		int playerIdFound = -1;
		int rndVal = R2_GLOBALS._randomSource.getRandomNumber(3);
		for (int i = 0; i <= 3; i++) {
			if (rndVal != 1) {
				if (  (_gameBoardSide[rndVal]._handCard[0]._cardId != 0)
					|| (_gameBoardSide[rndVal]._handCard[1]._cardId != 0)
					|| (_gameBoardSide[rndVal]._handCard[2]._cardId != 0)
					|| (_gameBoardSide[rndVal]._handCard[3]._cardId != 0)) {
						playerIdFound = rndVal;
						break;
				}
			}
			// The original was only updating in the rndVal block,
			// which was a bug as the checks were stopping at this point
			rndVal--;
			if (rndVal < 0)
				rndVal = 3;
		}

		if (playerIdFound != -1) {
			playThieftCard(1, &_gameBoardSide[1]._handCard[thieftId], playerIdFound);
			return;
		}
	}

	for (int i = 0; i <= 3; i++) {
		int tmpVal = isDelayCard(_gameBoardSide[1]._handCard[i]._cardId);
		if (tmpVal != -1) {
			victimId = -1;
			int rndVal = R2_GLOBALS._randomSource.getRandomNumber(3);

			for (int j = 0; j <= 3; j++) {
				if (rndVal != 1) {
					if ((_gameBoardSide[rndVal]._delayCard._cardId == 0) && isAttackPossible(rndVal, _gameBoardSide[1]._handCard[i]._cardId))
						victimId = rndVal;
				}

				if (victimId != -1) {
					playDelayCard(&_gameBoardSide[1]._handCard[i], &_gameBoardSide[victimId]._delayCard);
					return;
				} else {
					rndVal--;
					if (rndVal < 0)
						rndVal = 3;
				}
			}
		}
	}

	for (int j = 0; j <= 3; j++) {
		if (getStationCardId(_gameBoardSide[1]._handCard[j]._cardId) != -1) {
			victimId = -1;
			int rndVal = R2_GLOBALS._randomSource.getRandomNumber(3);
			for (int l = 0; l <= 3; l++) {
				if (rndVal != 1) {
					if ((_gameBoardSide[rndVal]._delayCard._cardId == 0) && (_gameBoardSide[1]._handCard[j]._cardId == 1))
						victimId = rndVal;
				}
				if (victimId != -1) {
					playDelayCard(&_gameBoardSide[1]._handCard[j], &_gameBoardSide[victimId]._delayCard);
					return;
				} else {
					rndVal--;
					if (rndVal < 0)
						rndVal = 3;
				}
			}
		}
	}

	handlePlayer01Discard(1);
}

void Scene1337::handlePlayer3() {
	if (_gameBoardSide[3]._delayCard._cardId != 0) {
		switch (_gameBoardSide[3]._delayCard._cardId) {
		case 10:
			// No break on purpose
		case 12:
			// No break on purpose
		case 15:
			// No break on purpose
		case 17:
			// No break on purpose
		case 18:
			// No break on purpose
		case 19:
			// No break on purpose
		case 20:
			// No break on purpose
		case 21:
			discardCard(&_gameBoardSide[3]._delayCard);
			return;
		default:
			for (int i = 0; i <= 3; i++) {
				if (checkAntiDelayCard(_gameBoardSide[3]._delayCard._cardId, _gameBoardSide[3]._handCard[i]._cardId)) {
					playAntiDelayCard(&_gameBoardSide[3]._handCard[i], &_gameBoardSide[3]._delayCard);
					return;
				}
			}
			break;
		}
	}

	int randIndx = R2_GLOBALS._randomSource.getRandomNumber(3);
	if (_gameBoardSide[3]._handCard[randIndx]._cardId == 1) {
		// Station Card
		for (int i = 0; i <= 7; i++) {
			if ((_gameBoardSide[3]._outpostStation[i]._cardId == 0) && !isStopConstructionCard(_gameBoardSide[3]._delayCard._cardId)) {
				playPlatformCard(&_gameBoardSide[3]._handCard[randIndx], &_gameBoardSide[3]._outpostStation[i]);
				return;
			}
		}
	} else if (_gameBoardSide[3]._handCard[randIndx]._cardId <= 9) {
		// Outpost Card
		for (int i = 0; i <= 7; i++) {
			if (_gameBoardSide[3]._outpostStation[i]._cardId == _gameBoardSide[3]._handCard[randIndx]._cardId) {
				discardCard(&_gameBoardSide[3]._handCard[randIndx]);
				return;
			}
		}

		for (int i = 0; i <= 7; i++) {
			if ((_gameBoardSide[3]._outpostStation[i]._cardId == 1) && !isStopConstructionCard(_gameBoardSide[3]._delayCard._cardId)) {
				int stationCount = 0;
				for (int j = 0; j <= 7; j++) {
					if ((_gameBoardSide[3]._outpostStation[j]._cardId > 1) && (_gameBoardSide[3]._outpostStation[j]._cardId <= 9))
						++stationCount;
				}

				if (stationCount == 7)
					_winnerId = 3;

				playStationCard(&_gameBoardSide[3]._handCard[randIndx], &_gameBoardSide[3]._outpostStation[i]);
				return;
			}
		}
	} else if (_gameBoardSide[3]._handCard[randIndx]._cardId == 13) {
		// Meteor Card
		int victimId = getPlayerWithOutpost(3);

		if (victimId != -1) {
			playCentralOutpostCard(&_gameBoardSide[3]._handCard[randIndx], victimId);
			return;
		}
	} else if (_gameBoardSide[3]._handCard[randIndx]._cardId == 25) {
		// Thief card
		int victimId = -1;
		int tmpRandIndx = R2_GLOBALS._randomSource.getRandomNumber(3);

		for (int i = 0; i <= 3; i++) {
			if (  (tmpRandIndx != 3)
				&& (  (_gameBoardSide[tmpRandIndx]._handCard[0]._cardId != 0)
				|| (_gameBoardSide[tmpRandIndx]._handCard[1]._cardId != 0)
				|| (_gameBoardSide[tmpRandIndx]._handCard[2]._cardId != 0)
				|| (_gameBoardSide[tmpRandIndx]._handCard[3]._cardId != 0) )) {
					victimId = tmpRandIndx;
					break;
			}

			++tmpRandIndx;
			if (tmpRandIndx > 3)
				tmpRandIndx = 0;
		}

		if (victimId != -1) {
			playThieftCard(3, &_gameBoardSide[3]._handCard[randIndx], victimId);
			return;
		}
	} else {
		switch (_gameBoardSide[3]._handCard[randIndx]._cardId) {
		case 10:
			// No break on purpose
		case 11:
			// No break on purpose
		case 12:
			// No break on purpose
		case 14:
			// No break on purpose
		case 15:
			// No break on purpose
		case 16:
			// No break on purpose
		case 17:
			// No break on purpose
		case 18:
			// No break on purpose
		case 19:
			// No break on purpose
		case 20:
			// No break on purpose
		case 21:
			// No break on purpose
		case 24: {
			int victimId = -1;
			int tmpRandIndx = R2_GLOBALS._randomSource.getRandomNumber(3);

			for (int i = 0; i <= 3; i++) {
				if (tmpRandIndx != 3) {
					if ((_gameBoardSide[tmpRandIndx]._delayCard._cardId == 0)
					&&  isAttackPossible(tmpRandIndx, _gameBoardSide[3]._handCard[randIndx]._cardId))
						victimId = tmpRandIndx;
				}

				++tmpRandIndx;
				if (tmpRandIndx > 3)
					tmpRandIndx = 0;

				if (victimId != -1)
					break;
			}

			if (victimId != -1) {
				// Useless second identical check skipped
				playDelayCard(&_gameBoardSide[3]._handCard[randIndx], &_gameBoardSide[victimId]._delayCard);
				return;
			}
				 }
		default:
			break;
		}
	}

	discardCard(&_gameBoardSide[3]._handCard[randIndx]);
}

void Scene1337::handleAutoplayPlayer2() {
	if (getStationCardId(this->_gameBoardSide[2]._delayCard._cardId) == -1)
		_delayedFunction = &Scene1337::handlePlayer2;
	else
		discardCard(&_gameBoardSide[2]._delayCard);
}

void Scene1337::handlePlayer2() {
	_selectedCard._stationPos = g_globals->_events._mousePos;

	if (R2_GLOBALS._v57810 == 200) {
		// Hand
		int i;
		for (i = 0; i < 4; i++) {
			if ((_gameBoardSide[2]._handCard[i].isIn(_selectedCard._stationPos)) && (_gameBoardSide[2]._handCard[i]._cardId != 0)) {
				Card *handcard = &_gameBoardSide[2]._handCard[i];
				_selectedCard._cardId = handcard->_cardId;
				_selectedCard._stationPos = handcard->_stationPos;
				//warning("_selectedCard._actorName = handcard->_actorName;");
				//warning("_selectedCard._fieldE = handcard->_fieldE;");
				//warning("_selectedCard._field10 = handcard->_field10;");
				//warning("_selectedCard._field12 = handcard->_field12;");
				//warning("_selectedCard._field14 = handcard->_field14;");
				//warning("_selectedCard._field16 = handcard->_field16;");
				_selectedCard._sceneRegionId = handcard->_sceneRegionId;
				_selectedCard._position = handcard->_position;
				_selectedCard._yDiff = handcard->_yDiff;
				_selectedCard._bounds = handcard->_bounds;
				_selectedCard._resNum = handcard->_resNum;
				_selectedCard._lookLineNum = handcard->_lookLineNum;
				_selectedCard._talkLineNum = handcard->_talkLineNum;
				_selectedCard._useLineNum = handcard->_useLineNum;
				_selectedCard._action = handcard->_action;
				//warning("_selectedCard._field0 = handcard->_field0;");
				_selectedCard._card._updateStartFrame = handcard->_card._updateStartFrame;
				_selectedCard._card._walkStartFrame = handcard->_card._walkStartFrame;
				_selectedCard._card._oldPosition = handcard->_card._oldPosition;
				_selectedCard._card._percent = handcard->_card._percent;
				_selectedCard._card._priority = handcard->_card._priority;
				_selectedCard._card._angle = handcard->_card._angle;
				_selectedCard._card._flags = handcard->_card._flags;
				_selectedCard._card._xe = handcard->_card._xe;
				_selectedCard._card._xs = handcard->_card._xs;
				_selectedCard._card._paneRects[0] = handcard->_card._paneRects[0];
				_selectedCard._card._paneRects[1] = handcard->_card._paneRects[1];
				_selectedCard._card._visage = handcard->_card._visage;
				_selectedCard._card._objectWrapper = handcard->_card._objectWrapper;
				_selectedCard._card._strip = handcard->_card._strip;
				_selectedCard._card._animateMode = handcard->_card._animateMode;
				_selectedCard._card._frame = handcard->_card._frame;
				_selectedCard._card._endFrame = handcard->_card._endFrame;
				_selectedCard._card._loopCount = handcard->_card._loopCount;
				_selectedCard._card._frameChange = handcard->_card._frameChange;
				_selectedCard._card._numFrames = handcard->_card._numFrames;
				_selectedCard._card._regionIndex = handcard->_card._regionIndex;
				_selectedCard._card._mover = handcard->_card._mover;
				_selectedCard._card._moveDiff = handcard->_card._moveDiff;
				_selectedCard._card._moveRate = handcard->_card._moveRate;
				_selectedCard._card._actorDestPos = handcard->_card._actorDestPos;
				_selectedCard._card._endAction = handcard->_card._endAction;
				_selectedCard._card._regionBitList = handcard->_card._regionBitList;
				// _selectedCard._object1._actorName = handcard->_object1._actorName;
				//warning("_selectedCard._card._fieldE = handcard->_card._fieldE;");
				//warning("_selectedCard._card._field10 = handcard->_card._field10;");
				//warning("_selectedCard._card._field12 = handcard->_card._field12;");
				//warning("_selectedCard._card._field14 = handcard->_card._field14;");
				//warning("_selectedCard._card._field16 = handcard->_card._field16;");

				_gameBoardSide[2]._handCard[i]._cardId = 0;
				_gameBoardSide[2]._handCard[i]._card.remove();
				break;
			}
		}

		if (i == 4) {
			handleClick(1, _selectedCard._stationPos);
			handleAutoplayPlayer2();
			return;
		} else {
			setCursorData(1332, _selectedCard._card._strip, _selectedCard._card._frame);
			R2_GLOBALS._sceneObjects->draw();
		}
	} else if (R2_GLOBALS._v57810 == 300) {
		// Eye
		handleClick(3, _selectedCard._stationPos);
		handleAutoplayPlayer2();
		return;
	} else {
		// The original code is calling a function full of dead code.
		// Only this message remains after a cleanup.
		MessageDialog::show(WRONG_ANSWER_MSG, OK_BTN_STRING);
		//
		handleAutoplayPlayer2();
		return;
	}

	Event event;
	bool found;
	for (;;) {
		if ( ((g_globals->_events.getEvent(event, EVENT_BUTTON_DOWN)) && (event.btnState == BTNSHIFT_RIGHT))
			|| (g_globals->_events.getEvent(event, EVENT_KEYPRESS)) ){
			_selectedCard._stationPos = g_globals->_events._mousePos;
			found = false;

			for (int i = 0; i <= 3; i ++) {
				if (_gameBoardSide[2]._handCard[i].isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					if (_gameBoardSide[2]._handCard[i]._cardId == 0) {
						_gameBoardSide[2]._handCard[i]._cardId = _selectedCard._cardId;
						_gameBoardSide[2]._handCard[i]._card.postInit();
						_gameBoardSide[2]._handCard[i]._card.hide();
						_gameBoardSide[2]._handCard[i]._card.setVisage(1332);
						_gameBoardSide[2]._handCard[i]._card.setPosition(_gameBoardSide[2]._handCard[i]._stationPos, 0);
						_gameBoardSide[2]._handCard[i]._card.fixPriority(170);
						setAnimationInfo(&_gameBoardSide[2]._handCard[i]);
						setCursorData(5, 1, 4);
						_currentPlayerNumb--;
						_showPlayerTurn = false;
						handleNextTurn();
						return;
					} else {
						actionDisplay(1330, 127, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						found = true;
					}
					break;
				}
			}

			if (!found) {
				if (_discardPile.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					discardCard(&_selectedCard);
					return;
				} else if (_selectedCard._cardId == 1) {
					bool isInCardFl = false;
					int i;
					for (i = 0; i <= 7; i++) {
						if (_gameBoardSide[2]._outpostStation[i].isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							isInCardFl = true;
							break;
						}
					}

					if ((isInCardFl) && (_gameBoardSide[2]._outpostStation[i]._cardId == 0)) {
						if (isDelayCard(_gameBoardSide[2]._delayCard._cardId) != -1) {
							actionDisplay(1330, 55, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else {
							playPlatformCard(&_selectedCard, &_gameBoardSide[2]._outpostStation[i]);
							return;
						}
					} else {
						actionDisplay(1330, 56, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					}
				} else if (_selectedCard._cardId <= 9) {
					bool isInCardFl = false;
					int i;
					for (i = 0; i <= 7; i++) {
						if (_gameBoardSide[2]._outpostStation[i].isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							isInCardFl = true;
							break;
						}
					}
					if ((isInCardFl) && (_gameBoardSide[2]._outpostStation[i]._cardId == 1)) {
						isInCardFl = false;
						for (int j = 0; j <= 7; j++) {
							if (_selectedCard._cardId == _gameBoardSide[2]._outpostStation[j]._cardId) {
								isInCardFl = true;
								break;
							}
						}
						if (isInCardFl) {
							// This station is already in place
							actionDisplay(1330, 34, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else if (isDelayCard(_gameBoardSide[2]._delayCard._cardId) != -1) {
							// You must eliminate your delay before you can play a station
							actionDisplay(1330, 35, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else {
							int stationCount = 0;
							for (int k = 0; k <= 7; k++) {
								if ((_gameBoardSide[2]._outpostStation[k]._cardId > 1) && (_gameBoardSide[2]._outpostStation[k]._cardId <= 9))
									++stationCount;
							}

							if (stationCount == 7)
								_winnerId = 2;

							playStationCard(&_selectedCard, &_gameBoardSide[2]._outpostStation[i]);
							return;
						}
					} else {
						actionDisplay(1330, 37, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					}
				} else if ((_selectedCard._cardId == 26) || (_selectedCard._cardId == 30) ||(_selectedCard._cardId == 32) || (_selectedCard._cardId == 28)) {
					// Check anti-delay card (26 = Diplomacy, 28 = Innovation, 30 = Cure, 32 = Agreement)
					if (_gameBoardSide[2]._delayCard.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
						actionDisplay(1330, 42, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (checkAntiDelayCard(_gameBoardSide[2]._delayCard._cardId, _selectedCard._cardId)) {
						playAntiDelayCard(&_selectedCard, &_gameBoardSide[2]._delayCard);
						return;
					} else {
						if (_gameBoardSide[2]._delayCard._cardId != 0) {
							switch (_gameBoardSide[2]._delayCard._cardId) {
							case 11:
								actionDisplay(1330, 68, 159, 10, 1, 200, 0, 7, 0, 154, 154);
								break;
							case 14:
								actionDisplay(1330, 80, 159, 10, 1, 200, 0, 7, 0, 154, 154);
								break;
							case 16:
								actionDisplay(1330, 84, 159, 10, 1, 200, 0, 7, 0, 154, 154);
								break;
							case 24:
								actionDisplay(1330, 96, 159, 10, 1, 200, 0, 7, 0, 154, 154);
								break;
							default:
								break;
							}
						} else {
							actionDisplay(1330, 41, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						}
					}
				} else if ((getStationCardId(_selectedCard._cardId) == -1) && (isDelayCard(_selectedCard._cardId) == -1)) {
					if (_selectedCard._cardId == 13) {
						// Meteor Card
						if (_gameBoardSide[0]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							for (int k = 0; k <= 7; k++) {
								if (_gameBoardSide[0]._outpostStation[k]._cardId != 0) {
									playCentralOutpostCard(&_selectedCard, 0);
									return;
								}
							}
							actionDisplay(1330, 74, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else if (_gameBoardSide[3]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							for (int k = 0; k <= 7; k++) {
								if (_gameBoardSide[3]._outpostStation[k]._cardId != 0) {
									playCentralOutpostCard(&_selectedCard, 3);
									return;
								}
							}
							actionDisplay(1330, 74, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else if (_gameBoardSide[1]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							for (int k = 0; k <= 7; k++) {
								if (_gameBoardSide[1]._outpostStation[k]._cardId == 0) {
									playCentralOutpostCard(&_selectedCard, 1);
									return;
								}
							}
							actionDisplay(1330, 74, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else {
							actionDisplay(1330, 128, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						}
					} else if (_selectedCard._cardId == 25) {
						// Thief card
						if (_gameBoardSide[0]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							if ( (_gameBoardSide[0]._handCard[0]._cardId != 0)
								|| (_gameBoardSide[0]._handCard[1]._cardId != 0)
								|| (_gameBoardSide[0]._handCard[2]._cardId != 0)
								|| (_gameBoardSide[0]._handCard[3]._cardId != 0) ) {
									int k;
									for (k = 0; k <= 3; k++){
										if (_gameBoardSide[2]._handCard[k]._cardId == 0)
											break;
									}
									playThieftCard(2, &_gameBoardSide[2]._handCard[k], 0);
									return;
							} else {
								actionDisplay(1330, 99, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							}
						} else if (_gameBoardSide[1]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							if ( (_gameBoardSide[1]._handCard[0]._cardId != 0)
								|| (_gameBoardSide[1]._handCard[1]._cardId != 0)
								|| (_gameBoardSide[1]._handCard[2]._cardId != 0)
								|| (_gameBoardSide[1]._handCard[3]._cardId != 0) ) {
									int k;
									for (k = 0; k <= 3; k++){
										if (_gameBoardSide[2]._handCard[k]._cardId == 0)
											break;
									}
									playThieftCard(2, &_gameBoardSide[2]._handCard[k], 1);
									return;
							} else {
								actionDisplay(1330, 99, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							}
						}

						if (_gameBoardSide[3]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							if ( (_gameBoardSide[3]._handCard[0]._cardId != 0)
								|| (_gameBoardSide[3]._handCard[1]._cardId != 0)
								|| (_gameBoardSide[3]._handCard[2]._cardId != 0)
								|| (_gameBoardSide[3]._handCard[3]._cardId != 0) ) {
									int k;
									for (k = 0; k <= 3; k++){
										if (_gameBoardSide[2]._handCard[k]._cardId == 0)
											break;
									}
									playThieftCard(2, &_gameBoardSide[2]._handCard[k], 3);
									return;
							} else {
								actionDisplay(1330, 99, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							}
						} else {
							actionDisplay(1330, 129, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						}
					} else if (_selectedCard._cardId == 29) {
						// Interceptor cards are used to prevent collision
						actionDisplay(1330, 136, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (_selectedCard._cardId == 27) {
						actionDisplay(1330, 137, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					}
				} else if (_gameBoardSide[0]._delayCard.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					if (_gameBoardSide[0]._delayCard._cardId != 0) {
						actionDisplay(1330, 15, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (!isAttackPossible(0, _selectedCard._cardId)) {
						switch (_selectedCard._cardId) {
						case 10:
							actionDisplay(1330, 66, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 12:
							actionDisplay(1330, 70, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 15:
							actionDisplay(1330, 82, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 17:
							actionDisplay(1330, 86, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 18:
							actionDisplay(1330, 88, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 19:
							actionDisplay(1330, 90, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 20:
							actionDisplay(1330, 92, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 21:
							actionDisplay(1330, 94, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						default:
							break;
						}
					} else {
						playDelayCard(&_selectedCard, &_gameBoardSide[0]._delayCard);
						return;
					}
				} else if (_gameBoardSide[3]._delayCard.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					if (_gameBoardSide[3]._delayCard._cardId != 0) {
						actionDisplay(1330, 17, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (!isAttackPossible(3, _selectedCard._cardId)) {
						switch (_selectedCard._cardId) {
						case 10:
							actionDisplay(1330, 66, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 12:
							actionDisplay(1330, 70, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 15:
							actionDisplay(1330, 82, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 17:
							actionDisplay(1330, 86, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 18:
							actionDisplay(1330, 88, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 19:
							actionDisplay(1330, 90, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 20:
							actionDisplay(1330, 92, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 21:
							actionDisplay(1330, 94, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						default:
							break;
						}
					} else {
						playDelayCard(&_selectedCard, &_gameBoardSide[3]._delayCard);
						return;
					}
				} else if (_gameBoardSide[1]._delayCard.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					if (_gameBoardSide[1]._delayCard._cardId != 0) {
						actionDisplay(1330, 19, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (!isAttackPossible(1, _selectedCard._cardId)) {
						switch (_selectedCard._cardId) {
						case 10:
							actionDisplay(1330, 66, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 12:
							actionDisplay(1330, 70, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 15:
							actionDisplay(1330, 82, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 17:
							actionDisplay(1330, 86, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 18:
							actionDisplay(1330, 88, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 19:
							actionDisplay(1330, 90, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 20:
							actionDisplay(1330, 92, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 21:
							actionDisplay(1330, 94, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						default:
							break;
						}
					} else {
						playDelayCard(&_selectedCard, &_gameBoardSide[1]._delayCard);
						return;
					}
				} else {
					actionDisplay(1330, 38, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				}
			}
		} else {
			g_globals->_scenePalette.signalListeners();
			R2_GLOBALS._sceneObjects->draw();
			g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
		}

		g_globals->_sceneObjects->recurse(SceneHandler::dispatchObject);
	}
}

void Scene1337::updateCursorId(int cursorId, bool updateFl) {
	if ((R2_GLOBALS._v57709 != 0) || (R2_GLOBALS._v5780C != 0))
		return;

	R2_GLOBALS._mouseCursorId = cursorId;

	if (updateFl) {
		R2_GLOBALS._mouseCursorId++;

		if (R2_GLOBALS._mouseCursorId < 1)
			R2_GLOBALS._mouseCursorId = 2;

		if (R2_GLOBALS._mouseCursorId > 2)
			R2_GLOBALS._mouseCursorId = 1;
	}

	// The original was using an intermediate function to call setCursorData.
	// It has been removed to improve readability
	if (R2_GLOBALS._mouseCursorId == 1) {
		R2_GLOBALS._v57810 = 200;
		setCursorData(5, 1, 4);
	} else if (R2_GLOBALS._mouseCursorId == 2) {
		R2_GLOBALS._v57810 = 300;
		setCursorData(5, 1, 5);
	} else {
		R2_GLOBALS._v57810 = 0;
		setCursorData(5, 0, 0);
	}
}

void Scene1337::setCursorData(int resNum, int rlbNum, int frameNum) {
	_cursorCurRes = resNum;
	_cursorCurStrip = rlbNum;
	_cursorCurFrame = frameNum;

	if (!frameNum) {
		// Should be a hardcoded cursor displaying only a dot.
		// FIXME: Use another cursor when possible
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
	} else {
		// TODO: The original was using some resource caching, which was useless and complex
		// and which has been removed. This cursor behavior clearly made intensive use of this caching...
		// We now have to find a way to cache these cursor pointers and avoid loading them multiple times per seconds
		uint size;
		byte *cursor = g_resourceManager->getSubResource(resNum, rlbNum, frameNum, &size);
		// Decode the cursor
		GfxSurface s = surfaceFromRes(cursor);

		Graphics::Surface surface = s.lockSurface();
		const byte *cursorData = (const byte *)surface.getPixels();
		CursorMan.replaceCursor(cursorData, surface.w, surface.h, s._centroid.x, s._centroid.y, s._transColor);
		s.unlockSurface();

		DEALLOCATE(cursor);
	}
}

void Scene1337::subD18F5() {
	if (R2_GLOBALS._v57709 == 0)
		// The original restores a copy of the default cursor (the hand), which isn't possible with our implementation
		// We reload that cursor instead.
		setCursorData(5, 1, 4);

	++R2_GLOBALS._v57709;
}

void Scene1337::subD1917() {
	if (R2_GLOBALS._v57709 != 0) {
		R2_GLOBALS._v57709--;
		if (R2_GLOBALS._v57709 == 0) {
			// The original was using an intermediate function to call setCursorData.
			// It has been removed to improve readability
			setCursorData(5, _cursorCurStrip, _cursorCurFrame);
		}
	}
}

void Scene1337::subD1940(bool flag) {
	if (flag)
		++R2_GLOBALS._v5780C;
	else if (R2_GLOBALS._v5780C != 0)
		--R2_GLOBALS._v5780C;
}

void Scene1337::subD1975(int arg1, int arg2) {
	// No implementation required in ScummVM: Mouse handling with tons of caching
}

void Scene1337::OptionsDialog::show() {
	OptionsDialog *dlg = new OptionsDialog();
	dlg->draw();

	// Show the dialog
	GfxButton *btn = dlg->execute(NULL);

	// Figure out the new selected character
	if (btn == &dlg->_quitGame)
		R2_GLOBALS._sceneManager.changeScene(125);
	else if (btn == &dlg->_restartGame)
		R2_GLOBALS._sceneManager.changeScene(1330);

	// Remove the dialog
	dlg->remove();
	delete dlg;
}

Scene1337::OptionsDialog::OptionsDialog() {
	// Set the elements text
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;
	_autoplay.setText(scene->_autoplay ? AUTO_PLAY_ON : AUTO_PLAY_OFF);
	_restartGame.setText(START_NEW_CARD_GAME);
	_quitGame.setText(QUIT_CARD_GAME);
	_continueGame.setText(CONTINUE_CARD_GAME);

	// Set position of the elements
	_autoplay._bounds.moveTo(5, 2);
	_restartGame._bounds.moveTo(5, _autoplay._bounds.bottom + 2);
	_quitGame._bounds.moveTo(5, _restartGame._bounds.bottom + 2);
	_continueGame._bounds.moveTo(5, _quitGame._bounds.bottom + 2);

	// Add the items to the dialog
	addElements(&_autoplay, &_restartGame, &_quitGame, &_continueGame, NULL);

	// Set the dialog size and position
	frame();
	_bounds.collapse(-6, -6);
	setCenter(160, 100);
}

GfxButton *Scene1337::OptionsDialog::execute(GfxButton *defaultButton) {
	_gfxManager.activate();

	// Event loop
	GfxButton *selectedButton = NULL;

	bool breakFlag = false;
	while (!g_vm->shouldQuit() && !breakFlag) {
		Event event;
		while (g_globals->_events.getEvent(event) && !breakFlag) {
			// Adjust mouse positions to be relative within the dialog
			event.mousePos.x -= _gfxManager._bounds.left;
			event.mousePos.y -= _gfxManager._bounds.top;

			for (GfxElementList::iterator i = _elements.begin(); i != _elements.end(); ++i) {
				if ((*i)->process(event))
					selectedButton = static_cast<GfxButton *>(*i);
			}

			if (selectedButton == &_autoplay) {
				// Toggle Autoplay
				selectedButton = NULL;
				Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;
				scene->_autoplay = !scene->_autoplay;

				_autoplay.setText(scene->_autoplay ? AUTO_PLAY_ON : AUTO_PLAY_OFF);
				_autoplay.draw();
			} else if (selectedButton) {
				breakFlag = true;
				break;
			} else if (!event.handled) {
				if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
					selectedButton = NULL;
					breakFlag = true;
					break;
				}
			}
		}

		g_system->delayMillis(10);
		GLOBALS._screen.update();
	}

	_gfxManager.deactivate();
	return selectedButton;
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
