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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of Penumbra Overture.
 */

#ifndef GAME_RADIO_HANDLER_H
#define GAME_RADIO_HANDLER_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;

class cRadioMessage {
public:
	cRadioMessage(const tWString &asText, const tString &asSound);

	void Update(float afTimeStep);

	tWString msText;
	tString msSound;

	iSoundChannel *mpChannel;
};

typedef std::list<cRadioMessage *> tRadioMessageList;
typedef tRadioMessageList::iterator tRadioMessageListIt;

//---------------------------------------

class cRadioHandler : public iUpdateable {
public:
	cRadioHandler(cInit *apInit);
	~cRadioHandler();

	void Add(const tWString &asText, const tString &asSound);

	void Update(float afTimeStep);
	void OnDraw();
	void Reset();

	void SetOnEndCallback(const tString &asFunc) { msOnEndCallback = asFunc; }
	tString GetOnEndCallback() { return msOnEndCallback; }

	bool IsActive();

private:
	cInit *mpInit;
	cSoundHandler *mpSoundHandler;
	iFontData *mpFont;

	cRadioMessage *mpCurrentMessage;

	tWString msCurrentText;
	tWString msPrevText;

	tString msOnEndCallback;

	float mfAlpha;

	tRadioMessageList mlstMessages;
};

#endif // GAME_RADIO_HANDLER_H
