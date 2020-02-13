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

#ifndef TSAGE_RINGWORLD2_OUTPOST_H
#define TSAGE_RINGWORLD2_OUTPOST_H

#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class Scene1337 : public SceneExt {
	class OptionsDialog: public GfxDialog {
	private:
		GfxButton _autoplay;
		GfxButton _restartGame;
		GfxButton _quitGame;
		GfxButton _continueGame;

		OptionsDialog();
		~OptionsDialog() override {}
		virtual GfxButton *execute(GfxButton *defaultButton);
	public:
		static void show();
	};

	class Card: public SceneHotspot {
	public:
		SceneObject _card;

		int _cardId;
		Common::Point _stationPos;

		Card();
		void synchronize(Serializer &s) override;
		bool isIn(Common::Point pt);
	};

	class GameBoardSide: public SceneHotspot {
	public:
		Card _handCard[4];
		Card _outpostStation[8];
		Card _delayCard;
		Card _emptyStationPos;

		Common::Point _card1Pos;
		Common::Point _card2Pos;
		Common::Point _card3Pos;
		Common::Point _card4Pos;
		int _frameNum;

		GameBoardSide();
		void synchronize(Serializer &s) override;
	};

	class Action1337: public Action {
	public:
		void waitFrames(int32 frameCount);
	};

	class Action1: public Action1337 {
	public:
		void signal() override;
	};
	class Action2: public Action1337 {
	public:
		void signal() override;
	};
	class Action3: public Action1337 {
	public:
		void signal() override;
	};
	class Action4: public Action1337 {
	public:
		void signal() override;
	};
	class Action5: public Action1337 {
	public:
		void signal() override;
	};
	class Action6: public Action1337 {
	public:
		void signal() override;
	};
	class Action7: public Action1337 {
	public:
		void signal() override;
	};
	class Action8: public Action1337 {
	public:
		void signal() override;
	};
	class Action9: public Action1337 {
	public:
		void signal() override;
	};
	class Action10: public Action1337 {
	public:
		void signal() override;
	};
	class Action11: public Action1337 {
	public:
		void signal() override;
	};
	class Action12: public Action1337 {
	public:
		void signal() override;
	};
	class Action13: public Action1337 {
	public:
		void signal() override;
	};
public:
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	Action9 _action9;
	Action10 _action10;
	Action11 _action11;
	Action12 _action12;
	Action13 _action13;

	typedef void (Scene1337::*FunctionPtrType)();
	FunctionPtrType _delayedFunction;

	bool _autoplay;
	bool _shuffleEndedFl;
	bool _showPlayerTurn;
	bool _displayHelpFl;
	bool _instructionsDisplayedFl;

	// Discarded cards are put in the available cards pile, with an higher index so there no conflict
	int _currentDiscardIndex;
	int _availableCardsPile[100];
	int _cardsAvailableNumb;
	int _currentPlayerNumb;
	int _actionPlayerIdx;
	int _actionVictimIdx;
	int _winnerId;
	int _instructionsWaitCount;
	int _cursorCurRes;
	int _cursorCurStrip;
	int _cursorCurFrame;

	ASound _aSound1;
	ASound _aSound2;
	GameBoardSide _gameBoardSide[4];
	SceneActor _helpIcon;
	SceneActor _stockPile;
	SceneItem _actionItem;
	SceneObject _currentPlayerArrow;

	Card *_actionCard1;
	Card *_actionCard2;
	Card *_actionCard3;
	Card _animatedCard;
	Card _shuffleAnimation;
	Card _discardedPlatformCard;
	Card _selectedCard;
	Card _discardPile;
	Card _stockCard;

	SceneObject _upperDisplayCard[8];
	SceneObject _lowerDisplayCard[8];

	Scene1337();
	void synchronize(Serializer &s) override;

	void actionDisplay(int resNum, int lineNum, int x, int y, int keepOnScreen, int width, int textMode, int fontNum, int colFG, int colBGExt, int colFGExt);
	void setAnimationInfo(Card *card);
	void handleNextTurn();
	void handlePlayerTurn();
	bool isStationCard(int cardId);
	bool isStopConstructionCard(int cardId);
	int  getStationId(int playerId, int handCardId);
	int  findPlatformCardInHand(int playerId);
	int  findMeteorCardInHand(int playerId);
	int  findThieftCardInHand(int playerId);
	int  isDelayCard(int cardId);
	int  getStationCardId(int cardId);
	void handlePlayer01Discard(int playerId);
	void playThieftCard(int playerId, Card *card, int victimId);
	int  getPreventionCardId(int cardId);
	bool isAttackPossible(int victimId, int cardId);
	int  getPlayerWithOutpost(int playerId);
	bool checkAntiDelayCard(int delayCardId, int cardId);
	void playStationCard(Card *station, Card *platform);
	void playDelayCard(Card *card, Card *dest);
	void playPlatformCard(Card *card, Card *dest);
	void playAntiDelayCard(Card *card, Card *dest);
	Card *getStationCard(int arg1);
	void playCentralOutpostCard(Card *card, int playerId);
	int  getRandomCardFromHand(int playerId);
	void discardCard(Card *card);
	void subC4CD2();
	void subC4CEC();
	void playInterceptorCard(Card *subObj1, Card *subObj2);
	void displayDialog(int dialogNumb);
	void subPostInit();
	void displayInstructions();
	void suggestInstructions();
	void shuffleCards();
	void dealCards();
	void showOptionsDialog();
	void handleClick(int arg1, Common::Point pt);
	void handlePlayer0();
	void handlePlayer1();
	void handlePlayer2();
	void handlePlayer3();
	void handleAutoplayPlayer2();
	void updateCursorId(int arg1, bool arg2);
	void setCursorData(int resNum, int rlbNum, int frameNum);
	void subD18F5();
	void subD1917();
	void subD1940(bool flag);
	void subD1975(int arg1, int arg2);

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void process(Event &event) override;
	void dispatch() override;
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
