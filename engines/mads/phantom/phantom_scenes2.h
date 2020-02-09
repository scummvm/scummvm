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

#ifndef MADS_PHANTOM_SCENES2_H
#define MADS_PHANTOM_SCENES2_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

namespace Phantom {

class Scene2xx : public PhantomScene {
protected:
	/**
	 * Plays an appropriate sound when entering a scene
	 */
	void sceneEntrySound();

	/**
	 *Sets the AA file to use for the scene
	 */
	void setAAName();

	/**
	 * Updates the prefix used for getting player sprites for the scene
	 */
	void setPlayerSpritesPrefix();
public:
	Scene2xx(MADSEngine *vm) : PhantomScene(vm) {}
};

class Scene201 : public Scene2xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _needHoldFl;
	int _sellerCount;
	int _sellerStatus;
	int _sellerFrame;
	int _raoulFrame;
	int _raoulStatus;

	void handleSellerAnimation();
	void handleRaoulAnimation();
	void handleConversation();

public:
	Scene201(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene202 : public Scene2xx {
private:
	bool _ticketGivenFl;
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _skipWalkFl;
	int _chandeliersPosX[5];
	int _chandeliersHotspotId[5];
	int _conversationCount;
	int _usherStatus;
	int _usherFrame;
	int _usherCount;
	int _degasStatus;
	int _degasFrame;

	void handleConversation1();
	void handleConversation2();
	void handleUsherAnimation();
	void handleDegasAnimation();
	void handleChandeliersPositions();

public:
	Scene202(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene203 : public Scene2xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;
	bool _anim3ActvFl;
	bool _showNoteFl;

	int _brieStatus;
	int _brieFrame;
	int _brieCount;
	int _raoulStatus;
	int _raoulFrame;
	int _raoulCount;
	int _richardStatus;
	int _richardFrame;
	int _daaeStatus;
	int _daaeFrame;
	int _conversationCount;

	void handleBrieConversation();
	void handleRichardConversation();
	void handleRichardAndDaaeConversation();
	void handleBrieAnimation();
	void handleRichardAnimation();
	void handleRaoulAnimation();
	void handleDaaeAnimation();

public:
	Scene203(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene204 : public Scene2xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;
	bool _anim3ActvFl;
	bool _raoulDown;
	bool _florentGone;
	bool _skip1Fl;
	bool _skip2Fl;
	bool _skip3Fl;
	bool _endGameFl;

	int _brieStatus;
	int _brieFrame;
	int _florStatus;
	int _florFrame;
	int _raoulStatus;
	int _raoulFrame;
	int _raoulCount;

	void handleConversation();
	void handleBrieAnimation();
	void handleFlorAnimation();
	void handleRaoulAnimation();
	void handleEndAnimation();

public:
	Scene204(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene205 : public Scene2xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _noConversationHold;
	bool _giveTicketFl;

	int _richardFrame;
	int _richardStatus;
	int _richardCount;
	int _giryFrame;
	int _giryStatus;
	int _giryCount;
	int _conversationCounter;
	int _lastRandom;

	void handleConversation18();
	void handleConversation10();
	void handleConversation11();
	void handleRichardAnimation();
	void handleGiryAnimation();

public:
	Scene205(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene206 : public Scene2xx {
private:
	bool _anim0ActvFl;
	bool _skip1Fl;
	bool _skip2Fl;

public:
	Scene206(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene207 : public Scene2xx {
private:
	bool _skip1Fl;
	bool _anim0ActvFl;

public:
	Scene207(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene208 : public Scene2xx {
private:
	bool _skip1Fl;
	bool _skip2Fl;

	int _topLeftPeopleFrame;
	int _topRightPeopleFrame;
	int _middleLeftPeopleFrame;
	int _centerPeopleFrame;
	int _middleRightPeopleFrame;
	int _bottomLeftPeopleFrame;
	int _bottomMiddlePeopleFrame;
	int _bottomRightPeopleFrame;
	int _direction;

	void animateTopLeftPeople();
	void animateTopRightPeople();
	void animateMiddleLeftPeople();
	void animateCenterPeople();
	void animateMiddleRightPeople();
	void animateBottomLeftPeople();
	void animateBottomMiddlePeople();
	void animateBottomRightPeople();

public:
	Scene208(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene250 : public Scene2xx {
public:
	Scene250(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

} // End of namespace Phantom
} // End of namespace MADS

#endif /* MADS_PHANTOM_SCENES2_H */
