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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef INTRO_H
#define INTRO_H

namespace Hugo {

enum seqTextIntro {
	kIntro1 = 0,
	kIntro2 = 1,
	kIntro3 = 2
};

class IntroHandler {
public:
	IntroHandler(HugoEngine &vm);
	virtual ~IntroHandler();

	virtual void preNewGame() = 0;
	virtual void introInit() = 0;
	virtual bool introPlay() = 0;

protected:
	HugoEngine &_vm;
	int16 introTicks;                               // Count calls to introPlay()
};

class intro_1w : public IntroHandler {
public:
	intro_1w(HugoEngine &vm);
	~intro_1w();

	void preNewGame();
	void introInit();
	bool introPlay();
};

class intro_1d : public IntroHandler {
public:
	intro_1d(HugoEngine &vm);
	~intro_1d();

	void preNewGame();
	void introInit();
	bool introPlay();
};

class intro_2w : public IntroHandler {
public:
	intro_2w(HugoEngine &vm);
	~intro_2w();

	void preNewGame();
	void introInit();
	bool introPlay();
};

class intro_2d : public IntroHandler {
public:
	intro_2d(HugoEngine &vm);
	~intro_2d();

	void preNewGame();
	void introInit();
	bool introPlay();
};

class intro_3w : public IntroHandler {
public:
	intro_3w(HugoEngine &vm);
	~intro_3w();

	void preNewGame();
	void introInit();
	bool introPlay();
};

class intro_3d : public IntroHandler {
public:
	intro_3d(HugoEngine &vm);
	~intro_3d();

	void preNewGame();
	void introInit();
	bool introPlay();
};


} // Namespace Hugo
#endif
